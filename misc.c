/*
 * Misc functions
 * Copyright (C) 2010 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */
#include <sys/time.h>
#include <string.h>
#include <errno.h>

int xstrcmp(char *a, char *b) {
	if (!a && b) return 1;
	if (!b && a) return 1;
	if (a == b) return 0;
	return strcmp(a, b);
}

unsigned long long timediff_nsec(struct timespec *start_ts, struct timespec *end_ts) {
	unsigned long long nsec;
	nsec = (end_ts->tv_sec - start_ts->tv_sec) * 1000000000;
	nsec += (end_ts->tv_nsec - start_ts->tv_nsec);
	return nsec;
}

unsigned long long timeval_diff_usec(struct timeval *start_ts, struct timeval *end_ts) {
	unsigned long long usec;
	usec = (end_ts->tv_sec - start_ts->tv_sec) * 1000000;
	usec += (end_ts->tv_usec - start_ts->tv_usec);
	return usec;
}

unsigned long long timeval_diff_msec(struct timeval *start_ts, struct timeval *end_ts) {
	unsigned long long msec;
	msec = (end_ts->tv_sec - start_ts->tv_sec) * 1000;
	msec += (end_ts->tv_usec - start_ts->tv_usec) / 1000;
	return msec;
}

unsigned long long timeval_diff_sec(struct timeval *start_ts, struct timeval *end_ts) {
	unsigned long long sec;
	sec = (end_ts->tv_sec - start_ts->tv_sec);
	sec += (end_ts->tv_usec - start_ts->tv_usec) / 1000000;
	return sec;
}
