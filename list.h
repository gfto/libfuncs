/*
 * List manipulations header file
 * Copyright (C) 2006-2010 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */
#ifndef LIST_H
# define LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct LNODE {
	struct LNODE *prev;
	struct LNODE *next;
	void *data;
} LNODE;

typedef struct LIST {
	pthread_mutex_t *mutex;		// List's lock
	struct LNODE *head;			// Points to first element of the list
	struct LNODE *tail;			// Points to last element of the list
	unsigned int items;			// How many items are in the list
	char *name;
} LIST;

// Safe against calling list_del inside
#define list_for_each(list, elem, tmp_el) \
	for (elem = (list)->head->next, tmp_el = elem->next; elem != (list)->head && elem->data; elem = tmp_el, tmp_el = elem->next)

#define list_for_each_reverse(list, elem, tmp_el) \
	for (elem = (list)->head->prev, tmp_el = elem->prev; elem != (list)->head && elem->data; elem = tmp_el, tmp_el = elem->prev)

// list_del can not be called inside this for
#define list_for_each_unsafe(list, elem) \
	for (elem = (list)->head->next; elem != (list)->head && elem->data; elem = elem->next)

#define list_for_each_reverse_unsafe(list, elem) \
	for (elem = (list)->head->prev; elem != (list)->head && elem->data; elem = elem->prev)

LIST *list_new			(char *listname);
void list_free			(LIST **l, void (*l_free)(void *), void (*l_freep)(void **));

void list_lock			(LIST *l);
void list_unlock		(LIST *l);

void list_add			(LIST *l, void *data);

int  list_del			(LIST *l, LNODE **node);
int  list_del_unlocked	(LIST *l, LNODE **node);

int  list_del_entry		(LIST *l, void *data);

void list_dump(LIST *l);

#ifdef __cplusplus
}
#endif

#endif
