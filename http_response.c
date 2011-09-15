/*
 * HTTP responses
 * Copyright (C) 2006-2009 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */

#include <stdio.h>
#include <stdarg.h>
#include <netdb.h>
#include <time.h>

#include "io.h"

static char *server_signature = NULL;
static char *server_version   = NULL;

void set_http_response_server_ident(char *server, char *version) {
	server_signature = server;
	server_version   = version;
}

void send_http_response(int clientsock, const char *response) {
	char buf[128];
	time_t now = time(NULL);
	fdputsf(clientsock, "HTTP/1.0 %s\n", response);
	if (server_signature && server_version)
		fdputsf(clientsock, "Server: %s %s\n", server_signature, server_version);
	strftime(buf,sizeof(buf),"Date: %a, %d %b %Y %H:%M:%S %Z\n",gmtime(&now));
	fdputs(clientsock,buf);
	fdputs(clientsock, "Cache-Control: no-cache\n");
	fdputs(clientsock, "Connection: close\n");
	fdputs(clientsock, "Pragma: no-cache\n");
}

void send_header_textplain(int clientsock) {
	fdputs(clientsock,"Content-Type: text/plain\n");
}

void send_http_error(int clientsock, const char *code, const char *message) {
	send_http_response(clientsock, code);
	send_header_textplain(clientsock);
	if (message) {
		fdputsf(clientsock,"X-Error: %s\n", message);
	}
	fdputs(clientsock,"\n");
	fdputsf(clientsock,"%s\n", message ? message : code);
}

void send_200_ok(int clientsock) {
	send_http_response(clientsock,"200 OK");
}

void send_http_ok(int clientsock, char *message) {
	send_200_ok(clientsock);
	send_header_textplain(clientsock);
	fdputs(clientsock, "\n");
	fdputs(clientsock, message);
	fdputs(clientsock, "\n");
}


void send_http_ok_msg(int clientsock, const char *fmt, ...) {
	char msg[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, 512, fmt, args);
	va_end(args);
	send_http_ok(clientsock, msg);
}

void send_302_redirect(int clientsock, char * url) {
	send_http_response(clientsock,"302 Found");
	fdputsf(clientsock,"Location: %s\n", url);
	fdputs(clientsock,"\n");
}

void send_400_bad_request(int clientsock, const char * msg) {
	send_http_error(clientsock,"400 Bad Request", msg);
}

void send_403_forbidden_msg(int clientsock, const char *msg) {
	send_http_error(clientsock, "403 Forbidden", msg);
}

void send_403_forbidden(int clientsock) {
	send_403_forbidden_msg(clientsock, "access-denied");
}

void send_404_not_found(int clientsock) {
	send_http_error(clientsock, "404 Not Found", NULL);
}

void send_409_conflict(int clientsock, const char * msg) {
	send_http_error(clientsock, "409 Conflict", msg);
}

void send_500_internal_server_error(int clientsock, const char * msg) {
	send_http_error(clientsock, "500 Internal Server Error", msg);
}

void send_501_not_implemented(int clientsock) {
	send_http_error(clientsock, "501 Method Not Implemented", NULL);
}

void send_504_gateway_timeout(int clientsock) {
	send_http_error(clientsock, "504 Gateway Timeout", "no-signal");
}
