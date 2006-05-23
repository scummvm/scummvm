/*
 * $Id$
 *
 * List management macros from the Linux kernel
 */

#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

#include "agi/agi.h"

namespace Agi {

/**
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static INLINE void __list_add(struct list_head *tnew, struct list_head *prev, struct list_head *next) {
	next->prev = tnew;
	tnew->next = next;
	tnew->prev = prev;
	prev->next = tnew;
}

/**
 * add a new entry
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 *
 * @param new new entry to be added
 * @param head list head to add it after
 */
static INLINE void list_add(struct list_head *tnew, struct list_head *head) {
	__list_add(tnew, head, head->next);
}

/**
 * add a new entry
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 *
 * @new: new entry to be added
 * @head: list head to add it before
 */
static INLINE void list_add_tail(struct list_head *tnew, struct list_head *head) {
	__list_add(tnew, head->prev, head);
}

/**
 * Delete a list entry (makes prev/next entries point to each other)
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static INLINE void __list_del(struct list_head *prev, struct list_head *next) {
	next->prev = prev;
	prev->next = next;
}

/**
 * deletes entry from list.
 * @param entry the element to delete from the list.
 */
static INLINE void list_del(struct list_head *entry) {
	__list_del(entry->prev, entry->next);
}

/**
 * tests whether a list is empty
 * @param head the list to test.
 */
static INLINE int list_empty(struct list_head *head) {
	return head->next == head;
}

/**
 * join two lists
 * @param list the new list to add.
 * @param head the place to add it in the first list.
 */
static INLINE void list_splice(struct list_head *list, struct list_head *head) {
	struct list_head *first = list->next;

	if (first != list) {
		struct list_head *last = list->prev;
		struct list_head *at = head->next;

		first->prev = head;
		head->next = first;

		last->next = at;
		at->prev = last;
	}
}

/**
 * get the struct for this entry
 * @param ptr    the &struct list_head pointer.
 * @param type   the type of the struct this is embedded in.
 * @param member the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * iterate over a list
 * @param pos    the &struct list_head to use as a loop counter.
 * @param head   the head for your list.
 */
#define list_for_each(pos, head, next) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

}                             // End of namespace Agi

#endif
