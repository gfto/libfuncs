/*
 * Server functions
 * Copyright (C) 2006-2010 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void daemonize(char *pidfile) {
	if (!pidfile)
		return;
	printf("Daemonizing.\n");
	pid_t pid = fork();
	if (pid > 0) {
		FILE *F = fopen(pidfile,"w");
		if (F) {
			fprintf(F,"%i\n",pid);
			fclose(F);
		}
		exit(0);
	}
	// Child process continues...
	setsid();	// request a new session (job control)
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);
}

void init_server_socket(char *bind_addr, int bind_port, struct sockaddr_in *server, int *server_socket) {
	char *binded;

	struct hostent *host_ptr;
	*server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*server_socket == -1) {
		perror("socket(server_socket)");
		exit(1);
	}
	int j = 1;
	if (setsockopt(*server_socket, SOL_SOCKET, SO_REUSEADDR,(const char *) &j, sizeof(j))<0) {
		perror("setsockopt(SO_REUSEADDR)");
		exit(1);
	}

	memset(server, 0, sizeof(struct sockaddr_in));
	if (!bind_addr) {
		binded = "*";
		server->sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		host_ptr = gethostbyname(bind_addr);
		if (!host_ptr) {
			fprintf(stderr,"Error can't resolve bind address: %s\n", bind_addr);
			exit(1);
		}
		memcpy(&server->sin_addr, host_ptr->h_addr, sizeof(server->sin_addr));
		binded = inet_ntoa(server->sin_addr);
	}

	/* Bind to server socket */
	fprintf(stderr, "Binding to %s:%i\t", binded, bind_port);
	if (strcmp(binded,"*")==0)
		fprintf(stderr,"\t");

	server->sin_family = AF_INET;
	server->sin_port   = htons(bind_port);
	if (bind(*server_socket, (struct sockaddr *)server, sizeof(struct sockaddr_in)) < 0) {
		perror("bind(server_socket)");
		exit(1);
	}
	if (listen(*server_socket, 256) < 0) {
		perror("listen()");
		exit(1);
	}

	fputs("[OK]\n",stderr);
}
