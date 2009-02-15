/***************************************************************************
 aatree.h Copyright (C) 2006 Walter van Niftrik


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Walter van Niftrik [w.f.b.w.v.niftrik@stud.tue.nl]

***************************************************************************/

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
