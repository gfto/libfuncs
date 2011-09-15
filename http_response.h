/*
 * HTTP responses header file
 * Copyright (C) 2006-2009 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */

#ifndef HTTP_RESPONSE_H
# define HTTP_RESPONSE_H

#ifdef __cplusplus
extern "C" {
#endif

void set_http_response_server_ident(char *server, char *version);

void send_http_response(int clientsock, const char * response);
void send_header_textplain(int clientsock);

void send_http_error(int clientsock, const char *code, const char *message);
void send_http_ok(int clientsock, const char *message);
void send_http_ok_msg(int clientsock, const char *fmt, ...);

void send_200_ok(int clientsock);
void send_302_redirect(int clientsock, const char * url);
void send_400_bad_request(int clientsock, const char *msg);
void send_403_forbidden_msg(int clientsock, const char *msg);
void send_403_forbidden(int clientsock);
void send_404_not_found(int clientsock);
void send_409_conflict(int clientsock, const char * msg);
void send_500_internal_server_error(int clientsock, const char * msg);
void send_501_not_implemented(int clientsock);
void send_504_gateway_timeout(int clientsock);

#ifdef __cplusplus
}
#endif

#endif
