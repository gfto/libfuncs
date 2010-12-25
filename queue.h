/*
 * IPTV.bg Media Proxy
 * Request queue header file
 * 
 * Copyright (C) 2006 Unix Solutions Ltd.
 * Written by Luben Karavelov (luben@unixsol.org)
 * 
 */
#ifndef QUEUE_H
# define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct QNODE {
	struct QNODE *next;
	void *data;
} QNODE;

typedef struct QUEUE {
	QNODE *head;
	QNODE *tail;
	pthread_mutex_t *mutex;	// queue's mutex.
	pthread_cond_t  *cond;	// queue's condition variable.
	int items;				// number of messages in queue
} QUEUE;

QUEUE *queue_new		();
void queue_free			(QUEUE **q);

void queue_add			(QUEUE *q, void *data);
void *queue_get			(QUEUE *q);
void *queue_get_nowait	(QUEUE *q);
void queue_wakeup		(QUEUE *q);

#ifdef __cplusplus
}
#endif

#endif
