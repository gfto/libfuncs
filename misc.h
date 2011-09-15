/*
 * Misc functions header file
 * Copyright (C) 2010 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */
#ifndef MISC_H
#define MISC_H

#include <sys/time.h>

int xstrcmp(char *a, char *b);

unsigned long long timediff_nsec(struct timespec *start_ts, struct timespec *end_ts);
unsigned long long timeval_diff_usec(struct timeval *start_ts, struct timeval *end_ts);
unsigned long long timeval_diff_msec(struct timeval *start_ts, struct timeval *end_ts);
unsigned long long timeval_diff_sec (struct timeval *start_ts, struct timeval *end_ts);

#endif
