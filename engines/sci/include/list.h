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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)queue.h	8.5 (Berkeley) 8/20/94
 */

/* 2006-04-21 Modified by Walter van Niftrik. */

#ifndef _SCI_LIST_H
#define _SCI_LIST_H

#include "common/scummsys.h"

namespace Sci {

/* List definitions. */
#define LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;	/* first element */			\
}

#define LIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define LIST_ENTRY(type)						\
struct {								\
	struct type *le_next;	/* next element */			\
	struct type **le_prev;	/* address of previous next element */	\
}

#define LIST_INIT(head) do {						\
	(head)->lh_first = NULL;					\
} while (0)

#define LIST_INSERT_AFTER(listelm, elm, field) do {			\
	if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)	\
		(listelm)->field.le_next->field.le_prev =		\
		    &(elm)->field.le_next;				\
	(listelm)->field.le_next = (elm);				\
	(elm)->field.le_prev = &(listelm)->field.le_next;		\
} while (0)

#define LIST_INSERT_BEFORE(listelm, elm, field) do {			\
	(elm)->field.le_prev = (listelm)->field.le_prev;		\
	(elm)->field.le_next = (listelm);				\
	*(listelm)->field.le_prev = (elm);				\
	(listelm)->field.le_prev = &(elm)->field.le_next;		\
} while (0)

#define LIST_INSERT_HEAD(head, elm, field) do {				\
	if (((elm)->field.le_next = (head)->lh_first) != NULL)		\
		(head)->lh_first->field.le_prev = &(elm)->field.le_next;\
	(head)->lh_first = (elm);					\
	(elm)->field.le_prev = &(head)->lh_first;			\
} while (0)

#define LIST_REMOVE(elm, field) do {					\
	if ((elm)->field.le_next != NULL)				\
		(elm)->field.le_next->field.le_prev =			\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = (elm)->field.le_next;			\
} while (0)

#define LIST_FOREACH(var, head, field)					\
	for ((var) = ((head)->lh_first);				\
		(var);							\
		(var) = ((var)->field.le_next))

/* List access methods. */
#define LIST_EMPTY(head)		((head)->lh_first == NULL)
#define LIST_FIRST(head)		((head)->lh_first)
#define LIST_NEXT(elm, field)		((elm)->field.le_next)

/* Circular list definitions. */

#define CLIST_HEAD(name, type)						\
struct name {								\
	struct type *clh_first;	/* first element. */			\
}

#define CLIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define CLIST_ENTRY(type)						\
struct {								\
	struct type *cle_next;	/* next element. */			\
	struct type *cle_prev;	/* previous element */			\
}

#define CLIST_INIT(head) do {						\
	(head)->clh_first = NULL;					\
} while (0)

#define CLIST_INSERT_HEAD(head, elm, field) do {			\
	if ((head)->clh_first == NULL)					\
		(elm)->field.cle_next = (elm)->field.cle_prev = (elm);	\
	else {								\
		(elm)->field.cle_next = (head)->clh_first;		\
		(elm)->field.cle_prev =					\
		    (head)->clh_first->field.cle_prev;			\
		(head)->clh_first->field.cle_prev = (elm);		\
		(elm)->field.cle_prev->field.cle_next = (elm);		\
	}								\
	(head)->clh_first = (elm);					\
} while (0)

#define CLIST_INSERT_AFTER(listelm, elm, field) do {			\
	(elm)->field.cle_prev = (listelm);				\
	(elm)->field.cle_next = (listelm)->field.cle_next;		\
	(listelm)->field.cle_next->field.cle_prev = (elm);		\
	(listelm)->field.cle_next = (elm);				\
} while (0)

#define CLIST_REMOVE(head, elm, field) do {				\
	if ((elm)->field.cle_next == (elm))				\
		(head)->clh_first = NULL;				\
	else {								\
		if ((head)->clh_first == (elm))				\
			(head)->clh_first = (elm)->field.cle_next;	\
		(elm)->field.cle_prev->field.cle_next =			\
		    (elm)->field.cle_next;				\
		(elm)->field.cle_next->field.cle_prev =			\
		    (elm)->field.cle_prev;				\
	}								\
} while (0)

#define CLIST_FOREACH(var, head, field)					\
	for ((var) = (head)->clh_first;					\
		(var);							\
		(var) = ((var)->field.cle_next == (head)->clh_first ?	\
		    NULL : (var)->field.cle_next))

/* Circular list access methods. */
#define CLIST_EMPTY(head)		((head)->clh_first == NULL)
#define CLIST_FIRST(head)		((head)->clh_first)
#define CLIST_NEXT(elm, field)		((elm)->field.cle_next)
#define CLIST_PREV(elm, field)		((elm)->field.cle_prev)

} // End of namespace Sci

#endif /* !_SCI_LIST_H */
