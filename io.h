/*
 * UX IO functions header file
 * Copyright (C) 2006-2009 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */

#ifndef IO_H
# define IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <sys/types.h>

char * chomp(char *x);

ssize_t safe_read(int fd, void *buf, size_t len);
ssize_t safe_write(int fd, const void *buf, size_t len);
void shutdown_fd(int *in_fd);

ssize_t fdgetline(int fd, char *buf, size_t buf_size);
ssize_t fdread_ex(int fd, char *buf, size_t buf_size, int timeout, int retries, int waitfull);
ssize_t fdread(int fd, char *buf, size_t buf_size);
ssize_t fdread_nowaitfull(int fd, char *buf, size_t buf_size);
ssize_t fdwrite(int fd, char *buf, size_t buf_size);

int fdputs(int fd, char *msg);
int fdputsf(int fd, char *fmt, ...);

void set_log_io_errors(int report_io_errors);

void set_sock_nonblock(int sockfd);
void set_sock_block(int sockfd);

int do_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen, int timeout);

#ifdef __cplusplus
}
#endif

#endif
