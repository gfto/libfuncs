/*
 * List manipulations
 * Copyright (C) 2006-2010 Unix Solutions Ltd.
 *
 * Released under MIT license.
 * See LICENSE-MIT.txt for license terms.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "libfuncs.h"
#include "list.h"

//#define LIST_DEBUG

#ifdef LIST_DEBUG
	#define Ldbg fprintf
#else
	#define Ldbg(...) do { /* ... */ } while(0)
#endif

LIST *list_new(char *name) {
	pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
	if (pthread_mutex_init(mutex,NULL) != 0) {
		perror("list_new: mutex_init");
		return NULL;
	}
	LIST *list = calloc(1, sizeof(LIST));
	if (!list)
		return NULL;
	list->mutex = mutex;
	list->name = strdup(name);

	LNODE *node = malloc(sizeof(LNODE));
	if (!node)
		return NULL;

	node->data = NULL;
	node->next = node;
	node->prev = node;

	list->head = node;	// Point to first
	list->tail = node;	// Set prev element

	Ldbg(stderr, "list_new(%s) l=%p l->head=%p l->tail=%p\n", list->name, list, list->head, list->tail);
	return list;
}

void list_free(LIST **plist, void (*free_func)(void *), void (*freep_func)(void **)) {
	LIST *list = *plist;
	if (!list)
		return;
	Ldbg(stderr, "list_free(%s)\n", list->name);
	LNODE *node, *tmp;
	list_lock(list);
	list_for_each(list, node, tmp) {
		if (free_func && node->data)
			free_func(node->data);
		if (freep_func && node->data)
			freep_func(&node->data);
		list_del_unlocked(list, &node);
	}
	FREE(list->head);
	list_unlock(list);
	pthread_mutex_destroy(list->mutex);
	FREE(list->mutex);
	FREE(list->name);
	FREE(*plist);
}

void list_lock(LIST *list) {
	pthread_mutex_lock(list->mutex);
}

void list_unlock(LIST *list) {
	pthread_mutex_unlock(list->mutex);
}

void list_add(LIST *list, void *data) {
	if (!list) {
		Ldbg(stderr, "list_add(), list == NULL\n");
		return;
	}

	if (!data) {
		Ldbg(stderr, "list_add(%s), data == NULL\n", list->name);
		return;
	}

	LNODE *node = malloc(sizeof(LNODE));
	if (!node) {
		Ldbg(stderr, "list_add(%s), can't alloc node\n", list->name);
		return;
	}

	node->data = data;

//	if (strcmp(list->name, "clients") == 0 || strcmp(list->name, "r->clients") == 0)
	Ldbg(stderr, "list_add(%s), node=%p data=%p\n", list->name, node, node->data);

	list_lock(list);

	node->prev = list->tail;
	node->next = list->head;

	list->tail->next = node;
	list->tail = node;
	list->head->prev = node;

	list->items++;
	list_unlock(list);
}

void list_dump(LIST *list) {
	if (list == NULL) {
		fprintf(stderr, "list_dump(), list is null\n");
		return;
	}
	fprintf(stderr, "list_dump(%s), nodes:%d\n", list->name, list->items);
	LNODE *node, *tmp;
	int i = 0;
	list_for_each(list, node, tmp) {
		fprintf(stderr, "   Node:%d Head:%p Tail:%p Node:%p NodeNext:%p NodePrev:%p Data:%p -> %s\n",
			i++, list->head, list->tail, node, node->next, node->prev, node->data, (char *)node->data);
	}
}

int list_del_unlocked(LIST *list, LNODE **node) {
	if (!list || !*node)
		return 0;

	LNODE *prev = (*node)->prev;
	LNODE *next = (*node)->next;

	next->prev = prev;
	prev->next = next;
	if (*node == list->tail)
		list->tail = prev;

	list->items--;

	FREE(*node);
	return 1;
}


int list_del(LIST *list, LNODE **node) {
	if (!list || !*node)
		return 0;
	list_lock(list);
	int ret = list_del_unlocked(list, node);
	list_unlock(list);
	return ret;
}

int list_del_entry(LIST *list, void *data) {
	int ret = 0;
	if (!list || !data)
		return 0;
	Ldbg(stderr, "list_del_entry(%s, %p)\n", list->name, data);
	list_lock(list);
	LNODE *node, *tmp;
	list_for_each(list, node, tmp) {
		if (node->data == data) {
			ret = list_del_unlocked(list, &node);
			break;
		}
	}
	list_unlock(list);
	return ret;
}
