/*
 * UX IO functions
 * Copyright (C) 2006-2009 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "libfuncs.h"
#include "log.h"

static int io_report_errors = 1;

void set_log_io_errors(int report_io_errors) {
	io_report_errors = report_io_errors;
}

char * chomp(char *x) {
	int i=strlen(x)-1;
	while ((i>=0)&&((x[i]=='\n')||(x[i]=='\r')||(x[i]==' ')||(x[i]=='\t'))){
		x[i--]=0;
	}
	return x;
}

ssize_t safe_read(int fd, void *buf, size_t len) {
	ssize_t readen;
	do {
		readen = read(fd, buf, len);
		if ((readen < 0) && (errno == EAGAIN || errno == EINTR))
			continue;
		return readen;
	} while (1);
}

ssize_t safe_write(int fd, const void *buf, size_t len) {
	ssize_t written;
	do {
		written = write(fd, buf, len);
		if ((written < 0) && (errno == EAGAIN || errno == EINTR))
			continue;
		return written;
	} while (1);
}

void shutdown_fd(int *in_fd) {
	if (*in_fd > -1) {
		shutdown(*in_fd, SHUT_RDWR);
		close(*in_fd);
		*in_fd = -1;
	}
}

ssize_t fdgetline(int fd, char *buf, size_t buf_size) {
	size_t i=0;
	int intrs = 0;
	int num_timeouts = 0;
	struct pollfd fdset[1];
	fdset[0].fd = fd;
	fdset[0].events = POLLIN;
	if (buf_size <= 0 || fd == -1)
		return 0;
	while (1) {
		int fdready = poll(fdset, 1, FDGETLINE_TIMEOUT);
		if (fdready == -1) {
			if (errno == EINTR) {
				if (++intrs < FDGETLINE_RETRIES)
					continue;
				else
					return i;
			}
			if (num_timeouts++ <= FDGETLINE_RETRIES) {
				continue;
			} else {
				if (io_report_errors)
					log_perror("fdgetline() timeout", errno);
			}
		}
		if (fdready == 0 || fdready == -1) { /* Timeout || error */
			if (num_timeouts++ <= FDGETLINE_RETRIES)
				continue;
			return 0;
		}
		if (safe_read(fd,buf+i,1)!=1) /* Try to read 1 char */
			break;
		i++;
		if (buf[i-1]=='\n')
			break;
		if (i==buf_size) /* End of buffer reached, get out a here */
			return 0;
	}
	buf[i]='\0';
	return i;
}

ssize_t fdread_ex(int fd, char *buf, size_t buf_size, int timeout, int retries, int waitfull) {
	ssize_t rbytes = 0;
	int intrs = 0;
	int num_timeouts = 0;
	struct pollfd fdset[1];
	fdset[0].fd = fd;
	fdset[0].events = POLLIN;
	if (buf_size <= 0 || fd == -1)
		return 0;
	while (1) {
		int fdready = poll(fdset, 1, timeout);
		if (fdready == -1 || fdready == 0) { /* Timeout || error */
			if (errno == EINTR) {
				if (++intrs < retries)
					continue;
				else
					return rbytes;
			}
			if (num_timeouts++ <= retries) {
				continue;
			} else {
				if (timeout && io_report_errors) {
					log_perror("fdread() timeout", errno);
				}
				return rbytes > 0 ? rbytes : -1;
			}
		}
		ssize_t j = safe_read(fd, buf + rbytes, buf_size - rbytes);
		if (j < 0) // Error reading
			return j;
		if (j == 0) { // No data, retry?
			if (num_timeouts++ > retries) {
				return rbytes;
			}
			continue;
		}
		rbytes += j;
		if (!waitfull)
			return rbytes;
		if (rbytes == (ssize_t)buf_size)
			return rbytes;
	}
	return 0;
}

ssize_t fdread(int fd, char *buf, size_t buf_size) {
	return fdread_ex(fd, buf, buf_size, FDREAD_TIMEOUT, FDREAD_RETRIES, 1);
}

ssize_t fdread_nowaitfull(int fd, char *buf, size_t buf_size) {
	return fdread_ex(fd, buf, buf_size, FDREAD_TIMEOUT, FDREAD_RETRIES, 0);
}

ssize_t fdwrite(int fd, char *buf, size_t buf_size) {
	ssize_t wbytes=0;
	int intrs = 0;
	int num_timeouts = 0;
	struct pollfd fdset[1];
	fdset[0].fd = fd;
	fdset[0].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL | POLLRDHUP;

	if (buf_size <= 0 || fd == -1)
		return 0;
	while (1) {
		int fdready = poll(fdset, 1, FDWRITE_TIMEOUT);
		if (fdready == -1 || fdready == 0) { /* Timeout || error */
			if (errno == EINTR) {
				if (++intrs < FDWRITE_RETRIES)
					continue;
				else
					return wbytes;
			}
			if (num_timeouts++ <= FDWRITE_RETRIES) {
				continue;
			} else {
				if (io_report_errors)
					log_perror("fdwrite() timeout", errno);
				return wbytes > 0 ? wbytes : -1;
			}
		}
		if (fdready < 1 || (fdready == 1 && fdset[0].revents != POLLOUT)) /* Timeout || error */
			return -1;
		ssize_t j = safe_write(fd, buf+wbytes, buf_size-wbytes);
		if (j < 0) // Error writing
			return j;
		if (j == 0) { // No data, retry?
			if (num_timeouts++ > FDREAD_RETRIES) {
				return wbytes;
			}
			continue;
		}
		wbytes += j;
		if (wbytes == (ssize_t)buf_size)
			return wbytes;
	}
	return 0;
}

int fdputs(int fd, char *msg) {
	return fdwrite(fd, msg, strlen(msg));
}

int fdputsf(int fd, char *fmt, ...) {
	char msg[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, sizeof(msg)-1, fmt, args);
	va_end(args);
	return fdwrite(fd, msg, strlen(msg));
}


void set_sock_nonblock(int sockfd) {
	int arg = fcntl(sockfd, F_GETFL, NULL);
	arg |= O_NONBLOCK; 
	fcntl(sockfd, F_SETFL, arg);
}

void set_sock_block(int sockfd) {
	int arg = fcntl(sockfd, F_GETFL, NULL);
	arg &= (~O_NONBLOCK);
	fcntl(sockfd, F_SETFL, arg);
}


int do_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen, int timeout) {
	set_sock_nonblock(sockfd);
	// Trying to connect with timeout
	int result = connect(sockfd, serv_addr, addrlen);
	if (result < 0) { // Not connected
		if (errno != EINPROGRESS) { // Not in progress
			return -1;
		} else { // Wait a timeout ms for socket to become ready to write into
			struct pollfd fdset[1];
			fdset[0].fd = sockfd;
			fdset[0].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL | POLLRDHUP;
			do {
				int fdready = poll(fdset, 1, timeout);
				if (fdready == -1 && errno == EINTR)
					continue;
				if (fdready < 1 || (fdready == 1 && fdset[0].revents != POLLOUT)) { // Timeout || error
					// Get socket error
					unsigned int err_val=0, sz = sizeof(int); 
					getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void*)(&err_val), &sz);
					errno = err_val;
					if (!errno) // It can't be success, so simulate timeout
						errno = ENETDOWN;
					return -1;
				}
				break;
			} while (1);
		}
	}
	set_sock_block(sockfd);
	return 0;
}
