/*
 * Server functions
 * Copyright (C) 2006-2010 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */
#ifndef SERVER_H
# define SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <netinet/in.h>

void daemonize(char *pidfile);
void init_server_socket(char *bind_addr, int bind_port, struct sockaddr_in *server, int *server_socket);

#ifdef __cplusplus
}
#endif

#endif
