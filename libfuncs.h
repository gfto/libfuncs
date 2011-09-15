/*
 * UX libfuncs main header
 * Copyright (C) 2006-2010 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */

#ifndef LIBFUNCS_H
# define LIBFUNCS_H

#undef DEBUG
//#define DEBUG 1

#define DNS_RESOLVER_TIMEOUT 5000

#define FDGETLINE_TIMEOUT 500
#define FDGETLINE_RETRIES 30

#define FDREAD_TIMEOUT 1500
#define FDREAD_RETRIES 7

#define FDWRITE_TIMEOUT 1500
#define FDWRITE_RETRIES 7

#ifndef FREE
	#define FREE(x) if(x) { free(x); x=NULL; }
#endif

#ifndef POLLRDHUP
	#define POLLRDHUP 0
#endif

#define min(a,b) ((a < b) ? a : b)
#define max(a,b) ((a > b) ? a : b)

#include "asyncdns.h"
#include "http_response.h"
#include "io.h"
#include "list.h"
#include "log.h"
#include "queue.h"
#include "cbuf.h"
#include "server.h"
#include "misc.h"

#endif
