#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

list_t *list_alloc() {
	list_t *l = malloc(sizeof(list_t));
	l->head = NULL;
  l->size = 0;
	return l;
}

void list_free(list_t *l) {
	if (l == NULL) return;
	node_t *cur = l->head;
	while (cur != NULL) {
		node_t *next = cur->next;
		free(cur);
		cur = next;
	}
	free(l);
}

void list_add_to_back(list_t *l, int val) {
	node_t *n = malloc(sizeof(node_t));
	n->val = val;
	n->next = NULL;

	if (l->head == NULL) {
		l->head = n;
	} else {
		node_t *cur = l->head;
		while(cur->next != NULL) cur = cur-> next;
		cur->next = n;
	}
	l->size++;
}

void list_add_to_front(list_t *l, int val) {
	node_t *n = malloc(sizeof(node_t));
	n->val = val;
	n->next = l->head;
	l->head = n;
	l->size++;
}

void list_add_at_index(list_t *l, int index, int val) {
	if (index <= 0 || l->head == NULL){
		list_add_to_front(l, val);
		return;
	}
	if (index >= l->size) {
		list_add_to_back(l, val);
		return;
	}

	node_t *n = malloc(sizeof(node_t));
	n->val = val;

	node_t *cur = l->head;
	for (int i = 0; i < index -1; i++) cur = cur->next;

	n->next = cur->next;
	cur->next = n;
	l->size++;
}

int list_remove_from_back(list_t *l) {
	if (l->head == NULL) return -1;
	
	int val;
	if (l->head->next == NULL) {
		val = l->head->val;
		free(l->head);
		l->head = NULL;
	} else {
		node_t *cur = l->head;
		while (cur->next->next != NULL) cur = cur->next;
		val = cur->next->val;
		free(cur->next);
		cur->next = NULL;
	}
	l->size--;
	return val;
}

int list_remove_from_front(list_t *l) {
	if (l->head == NULL) return -1;
	
	node_t *old = l->head;
	int val = old->val;
	l->head = old->next;
	free(old);
	l->size--;
	return val;
}

int list_remove_at_index(list_t *l, int index) {
	if (l->head == NULL || index < 0 || index >= l->size) return -1;

	if (index == 0) return list_remove_from_front(l);

	node_t *cur = l->head;
	for (int i = 0; i < index - 1; i++) cur = cur->next;
	
	node_t *target = cur->next;
	int val = target->val;
	cur->next = target->next;
	free(target);
	l->size--;
	return val;
}

int list_get_elem_at(list_t *l, int index) {
	if (l->head == NULL || index < 0 || index >= l->size) return -1;

	node_t *cur = l->head;
	for (int i = 0; i < index; i++) cur = cur->next;
	return cur->val;
}

int list_length(list_t *l) {
	return l->size;
}

char *listToString(list_t *l) {
	static char buf[1024];
	char tmp[32];
	buf[0] = '\0';

	strcat(buf, "[");
	node_t *cur = l->head;
	while (cur != NULL) {
		snprintf(tmp, sizeof(tmp), "%d", cur->val);
		strcat(buf, tmp);
		if (cur->next != NULL) strcat(buf, ", ");
		cur = cur->next;
	}
	strcat(buf, "]");
	return buf;
}
		
	

