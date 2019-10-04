/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/jacl/jacl.h"
#include "glk/jacl/language.h"
#include "glk/jacl/types.h"
#include "glk/jacl/prototypes.h"

namespace Glk {
namespace JACL {

extern struct object_type       *object[];
extern int                      objects;

/**************************************/
/* Queue functions                    */
/**************************************/

struct QueueNode {
	int val;
	int val2;
	struct QueueNode *next;
};

struct Queue {
	QueueNode *head;
	QueueNode *tail;
};

void qInit(Queue *q) {
	q->head = q->tail = NULL;
}

void qDelete(Queue *q) {
	QueueNode *node, *next;

	for (node = q->head; node != NULL; node = next) {
		next = node->next;
		free(node);
	}

	q->head = q->tail = NULL;
}

int qIsEmpty(Queue *q) {
	return (q->head == NULL);
}

void qDebug(Queue *q) {
	debug("Queue:");

	if (q->head == NULL) {
		debug(" empty");
	} else {
		QueueNode *node;
		for (node = q->head; node != NULL; node = node->next) {
			debug(" %d (%d)", node->val, node->val2);
		}
	}

	debug("\n");
}

void qAppend(Queue *q, int val, int val2) {
	QueueNode *node = (QueueNode *) malloc(sizeof(QueueNode));
	node->val = val;
	node->val2 = val2;
	node->next = NULL;

	if (q->head == NULL) {
		q->head = q->tail = node;
	} else {
		q->tail->next = node;
		q->tail = node;
	}
}

void qPop(Queue *q, int *val, int *val2) {
	//assert(q->head != NULL);

	*val = q->head->val;
	*val2 = q->head->val2;

	if (q->head == q->tail) {
		q->head = q->tail = NULL;
	} else {
		q->head = q->head->next;
	}
}

void qTest() {
	int val, val2;
	Queue q;

	qInit(&q);
	qDebug(&q);

	debug("\nAdd 3, 0\n");
	qAppend(&q, 3, 0);
	qDebug(&q);

	debug("\nAdd 4, 2\n");
	qAppend(&q, 4, 2);
	qDebug(&q);

	debug("\nAdd 5, 1\n");
	qAppend(&q, 5, 1);
	qDebug(&q);

	qPop(&q, &val, &val2);
	debug("\nPop %d, %d\n", val, val2);
	qDebug(&q);

	qPop(&q, &val, &val2);
	debug("\nPop %d, %d\n", val, val2);
	qDebug(&q);

	debug("\nAdd 6, 3\n");
	qAppend(&q, 6, 3);
	qDebug(&q);

	debug("\nDelete all\n");
	qDelete(&q);
	qDebug(&q);
}

/**************************************/
/* Set functions                      */
/**************************************/

/* linked list for hash table */
struct SetNode {
	int val;
	struct SetNode *next;
};

#define SET_HASHSIZE 101

struct Set {
	SetNode *node[SET_HASHSIZE];
};

void setInit(Set *set) {
	int n;

	for (n = 0; n < SET_HASHSIZE; n++) {
		set->node[n] = NULL;
	}
}

void setDelete(Set *set) {
	int n;

	for (n = 0; n < SET_HASHSIZE; n++) {
		SetNode *node, *next;

		for (node = set->node[n]; node != NULL; node = next) {
			next = node->next;
			free(node);
		}

		set->node[n] = NULL;
	}
}

void setDebug(Set *set) {
	int n;

	debug("Set:");

	for (n = 0; n < SET_HASHSIZE; n++) {
		SetNode *node;

		for (node = set->node[n]; node != NULL; node = node->next) {
			debug(" %d", node->val);
		}
	}

	debug("\n");
}

int setHash(int val) {
	return abs(val) % SET_HASHSIZE;
}

void setAdd(Set *set, int val) {
	SetNode *node;
	int n = setHash(val);

	/* check if val is already in the set */

	for (node = set->node[n]; node != NULL; node = node->next) {
		if (node->val == val) {
			return;
		}
	}

	node = (SetNode *) malloc(sizeof(SetNode));
	node->val = val;
	node->next = set->node[n];
	set->node[n] = node;
}

/* returns 1 if the set contains val, otherwise returns 0 */

int setContains(Set *set, int val) {
	SetNode *node;
	int n = setHash(val);

	for (node = set->node[n]; node != NULL; node = node->next) {
		if (node->val == val) {
			return 1;
		}
	}

	return 0;
}

void setTest() {
	Set s;

	setInit(&s);
	setDebug(&s);

	debug("\nAdd 34\n");
	setAdd(&s, 34);
	setDebug(&s);

	debug("\nAdd 56\n");
	setAdd(&s, 56);
	setDebug(&s);

	debug("\nAdd 34 again\n");
	setAdd(&s, 34);
	setDebug(&s);

	debug("\nAdd %d\n", 34 + SET_HASHSIZE);
	setAdd(&s, 34 + SET_HASHSIZE);
	setDebug(&s);

	debug("\nAdd 78\n");
	setAdd(&s, 78);
	setDebug(&s);

	debug("\nDelete all\n");
	setDelete(&s);
	setDebug(&s);
}

/**************************************/

#define DIR_NONE   -1

int find_route(int fromRoom, int toRoom, int known) {
	Set visited;
	Queue q;
	int firstTime;
	int result = DIR_NONE;

	setInit(&visited);
	qInit(&q);

	qAppend(&q, fromRoom, DIR_NONE);
	setAdd(&visited, fromRoom);
	firstTime = 1;

	while (!qIsEmpty(&q)) {
		int n, dir, firstDir;
		qPop(&q, &n, &firstDir);

		if (n == toRoom) {
			result = firstDir;
			break;
		}

		for (dir = 0; dir < 12 ; dir++) {
			int dest = object[n]->integer[dir];

			if (dest < 1 || dest > objects) continue;

			if (object[dest] == NULL) continue;

			if (dest != NOWHERE && !setContains(&visited, dest)) {
				if (!known || (object[dest]->attributes & KNOWN)) {
					qAppend(&q, dest, (firstTime ? dir : firstDir));
					setAdd(&visited, dest);
				}
			}
		}

		firstTime = 0;
	}

	setDelete(&visited);
	qDelete(&q);

	return result;
}

} // End of namespace JACL
} // End of namespace Glk
