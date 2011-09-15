/*
 * Async DNS resolver
 * Copyright (C) 2009-2010 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */

#ifndef ASYNCDNS_H
# define ASYNCDNS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

// Returns
//   0 on success
//   1 on error
//   2 on timeout
int async_resolve_host(char *host, int port, struct sockaddr_in *sockaddr, int msec_timeout, int *active);

#endif
