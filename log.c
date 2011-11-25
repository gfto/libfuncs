/*
 * LOG functions
 * Copyright (C) 2006-2008 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */
/* Needed for POLLRDHUP */
#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "libfuncs.h"
#include "io.h"
#include "queue.h"
#include "asyncdns.h"
#include "log.h"

static FILE *OUT_FD = NULL;

struct logger {
	int			use_stderr;
	int			use_syslog;

	char		*host_ident;

	char		*log_host;
	int			log_port;
	int			log_sock;

	int			dienow : 1,
				dying  : 1;

	QUEUE		*queue;
	pthread_t	thread;
};

static void log_connect(struct logger *l) {
	struct sockaddr_in sockname;

	if (!l->use_syslog)
		return;

	while (1) {
		if (l->dying || l->dienow)
			break;
		int active = 1;
		int dret = async_resolve_host(l->log_host, l->log_port, &sockname, 500, &active);
		if (dret != 0) {
			log_perror("Could not resolve log host.", errno);
			sleep(2);
			continue;
		}
		l->log_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (l->log_sock < 0) {
			log_perror("Could not create syslog socket", errno);
			sleep(2);
			continue;
		}
		if (connect(l->log_sock, (struct sockaddr *) &sockname, sizeof (sockname)) < 0) {
			log_perror("Could not connect to log host.", errno);
			shutdown_fd(&l->log_sock);
			sleep(2);
			continue;
		}
		int on = 1;
		setsockopt(l->log_sock, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(int));
		break;
	}
}

static void * log_thread(void *param) {
	char date[64], logline[1024], *msg=NULL;
	struct logger *l = (struct logger *)param;

	if (l->log_sock < 0)
		log_connect(l);

	while (l && !l->dienow) {
		msg = queue_get(l->queue); // Waits...
		if (!msg)
			break;

		time_t now = time(NULL);
		memset(date, 0, sizeof(date));
		struct tm ltime;
		localtime_r(&now, &ltime);
		strftime(date, sizeof(date)-1, "%b %d %H:%M:%S", &ltime);

		memset(logline, 0, sizeof(logline));
		snprintf(logline, sizeof(logline)-1, "<30>%s host %s: %s", date, l->host_ident, msg);
		logline[sizeof(logline)-2] = '\n';
		logline[sizeof(logline)-1] = '\0';

		if (l->use_stderr)
			fprintf(OUT_FD, "%s", logline+4);

		while (l->use_syslog) {
			struct pollfd fdset[1];
			int fdready;
			do {
				fdset[0].fd = l->log_sock;
				fdset[0].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL | POLLRDHUP;
				fdready = poll(fdset, 1, 5 * 1000);
			} while (fdready == -1 && errno == EINTR);
			if (fdready < 1 || (fdready == 1 && fdset[0].revents != POLLOUT)) { /* Timeout || error */
				do { /* Try to reconnect to log host */
					if (l->dienow)
						goto OUT;
					LOGf("ERROR: Lost connection to log server (%s), fd: %i\n", fdready == 1 ? "poll error" : "timeout", l->log_sock);
					shutdown_fd(&l->log_sock);
					log_connect(l);
					sleep(2);
				} while (l->log_sock < 0);
			} else {
				if (fdwrite(l->log_sock, logline, strlen(logline)) > 0)
					break;
				else
					if (l->dienow)
						goto OUT;
			}
		}

		FREE(msg);
	}
OUT:
	FREE(msg);
	pthread_exit(0);
}

static struct logger *logger = NULL;

void log_init(char *host_ident, int use_syslog, int use_stderr, char *log_host, int log_port) {
	logger = calloc(1, sizeof(struct logger));

	logger->queue = queue_new();
	logger->host_ident = strdup(host_ident);

	if (log_host)
		logger->log_host = strdup(log_host);
	logger->log_port = log_port;
	logger->log_sock = -1;

	logger->use_syslog = use_syslog;
	logger->use_stderr = use_stderr;

	pthread_create(&logger->thread, NULL , &log_thread, logger);
}

void log_set_out_fd(FILE *new_out_fd) {
	OUT_FD = new_out_fd;
}

void log_close(void) {
	logger->dying = 1;
	int count = 0;
	while (logger->queue->items && count++ < 250)
		usleep(1000);
	logger->dienow = 1;
	queue_wakeup(logger->queue);
	pthread_join(logger->thread, NULL);
	shutdown_fd(&logger->log_sock);
	queue_free(&logger->queue);
	FREE(logger->host_ident);
	FREE(logger->log_host);
	FREE(logger);
}

void LOG(const char *msg) {
	if (OUT_FD == NULL)
		OUT_FD = stderr;
	if (!logger || logger->dying) {
		fprintf(OUT_FD, "%s", msg);
	} else {
		queue_add(logger->queue, strdup(msg));
	}
}

void LOGf(const char *fmt, ...) {
	char msg[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, sizeof(msg)-1, fmt, args);
	va_end(args);
	msg[sizeof(msg)-2] = '\n';
	msg[sizeof(msg)-1] = '\0';
	LOG(msg);
}

void log_perror(const char *message, int _errno) {
	char msg[1024];
	snprintf(msg, sizeof(msg)-1, "PERROR: %s | %s\n", message, strerror(_errno));
	msg[sizeof(msg)-2] = '\n';
	msg[sizeof(msg)-1] = '\0';
	LOG(msg);
}
