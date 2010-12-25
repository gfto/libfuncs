/*
 * IPTV.bg Media Proxy
 * LOG functions header file
 * 
 * Copyright (C) 2006 Unix Solutions Ltd.
 * Written by Luben Karavelov (luben@unixsol.org)
 * 
 */

#ifndef LOG_H
# define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

void log_init  (char *host_ident, int use_syslog, int use_stderr, char *log_host, int log_port);
void log_close ();

void LOG (const char *msg);

__attribute__ ((format(printf, 1, 2)))
void LOGf(const char *fmt, ...);

void log_perror(const char *message, int _errno);

void log_set_out_fd(FILE *new_out_fd);

#ifdef DEBUG
	#define dbg_LOG  LOG
	#define dbg_LOGf LOGf
#else
	#define dbg_LOG(arg)  do { /* arg */ } while(0)
	#define dbg_LOGf(...) do { /* ... */ } while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif
