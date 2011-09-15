/*
 * Async DNS resolver
 * Copyright (C) 2009-2010 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "log.h"

/* FreeBSD have problems with pathread_cancel so do not use async resolver */
#ifdef __FreeBSD__

int async_resolve_host(char *host, int port, struct sockaddr_in *sockaddr, int msec_timeout, int *active) {
	msec_timeout = msec_timeout;
	active = active;
	struct hostent *hostinfo = gethostbyname(host);
	if (hostinfo == NULL) {
		int local_h_errno = h_errno;
		LOGf("gethostbyname(%s) returned %s", host, hstrerror(local_h_errno));
		return 1; // error
	}
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons(port);
	sockaddr->sin_addr = *(struct in_addr *)hostinfo->h_addr;
	// LOGf("Not using async resolver! Resolved %s to %s\n", host, inet_ntoa(sockaddr->sin_addr));
	return 0;
}

#else

struct url_resolver {
	char *host;
	int port;
	struct sockaddr_in *sockaddr;
	int *status;
};

static void resolver_cleanup(void *p) {
	if (p)
		freeaddrinfo(p);
}

static void *resolver_thread(void *indata) {
	struct url_resolver *uri = indata;
	*(uri->status) = 0;
	char *host = uri->host;
	int port = uri->port;
	struct sockaddr_in *sockaddr = uri->sockaddr;

	int h_err;
	struct addrinfo hints, *addrinfo = NULL;
	memset(&hints, 0, sizeof hints);

	int state;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &state);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &state);
	pthread_cleanup_push(resolver_cleanup, NULL);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	h_err = getaddrinfo(host, NULL, &hints, &addrinfo);

	pthread_cleanup_pop(0);

	if (h_err == 0) {
		int num_addrs = 0;
		struct addrinfo *p;
		for (p=addrinfo; p!=NULL; p=p->ai_next) {
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			sockaddr->sin_family = AF_INET;
			sockaddr->sin_port = htons(port);
			sockaddr->sin_addr = ipv4->sin_addr;
			char IP[INET_ADDRSTRLEN];
			inet_ntop(p->ai_family, &(sockaddr->sin_addr), IP, sizeof IP);
			num_addrs++;
			// LOGf("Resolved[%d] %s to %s", num_addrs, host, IP);
		}
		freeaddrinfo(addrinfo);
		*(uri->status) = 1;
	} else {
		*(uri->status) = 2;
	}

	return NULL;
}

// Returns
//   0 on success
//   1 on error
//   2 on timeout
int async_resolve_host(char *host, int port, struct sockaddr_in *sockaddr, int msec_timeout, int *active) {
	pthread_t dns_thread;
	struct url_resolver uri;
	int status = 0;
	uri.host = host;
	uri.port = port;
	uri.sockaddr = sockaddr;
	uri.status = &status;

	int lactive = 1;
	if (!active)
		active = &lactive;

	if (pthread_create(&dns_thread, NULL, resolver_thread, &uri)) {
		log_perror("Failed to create resolver thread", errno);
		return 1;
	}
	pthread_detach(dns_thread);

	#define DNS_SLEEP 20000
	long int waitcount = msec_timeout * 1000;
	while (!status) {
		if (!active) {
			// LOG("Client !active, cancel resolver");
			pthread_cancel(dns_thread);
			return 2;
		}
		usleep(DNS_SLEEP);
		waitcount = waitcount - DNS_SLEEP;
		if (waitcount <= 0) {
			// LOGf("Timed out resolving: %s", host);
			pthread_cancel(dns_thread);
			return 2; // Timed out
		}
	}

	if (status == 1)
		return 0; // Ok, resolved

	return 1; // Error resolving
}
#endif
