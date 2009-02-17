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

#ifndef _SCI_AATREE_H
#define _SCI_AATREE_H

/* Andersson tree implementation. Stores data pointers in a balanced binary
** tree. A user-supplied comparison function defines the ordering. For the
** semantics of this function see qsort(3)
*/

typedef struct aatree aatree_t;

/* Left child */
#define AATREE_WALK_LEFT 0
/* Right child */
#define AATREE_WALK_RIGHT 1

aatree_t *aatree_new();
/* Allocates a new aatree
** Parameters: (void)
** Returns   : (aatree_t *) A newly allocated aatree
*/

void aatree_free(aatree_t *t);
/* Deallocates an aatree
** Parameters: (aatree_t *) t: The aatree
** Returns   : (void)
*/

int aatree_delete(void *x, aatree_t **t, int (*compar)(const void *, const void *));
/* Deletes a data element from an aatree
** Parameters: (void *) x: The data element to delete
**             (aatree_t **) t: The aatree
**             compar: The comparison function
** Returns   : (int) 0 on success, -1 if x wasn't found in t
*/

int aatree_insert(void *x, aatree_t **t, int (*compar)(const void *, const void *));
/* Inserts a data element into an aatree
** Parameters: (void *) x: The data element to insert
**             (aatree_t **) t: The aatree
**             compar: The comparison function
** Returns   : (int) 0 on success, -1 if x already exists in t
*/

aatree_t *aatree_walk(aatree_t *t, int direction);
/* Walks to either the left or right child of a node
** Parameters: (aatree_t *) t: The node
**             (int) direction: AATREE_WALK_LEFT or AATREE_WALK_RIGHT
** Returns   : (aatree_t *) The requested child of t or NULL if it doesn't
**                          exist
*/

void *aatree_get_data(aatree_t *t);
/* Returns the data element of a node
** Parameters: (aatree_t *) t: The node
** Returns   : (void *) The data element
*/

#endif /* !_SCI_AATREE_H */
