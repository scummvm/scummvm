/***************************************************************************
 aatree.c Copyright (C) 2006 Walter van Niftrik


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

#include <stdlib.h>
#include <aatree.h>

#include <sci_memory.h>

struct aatree
{
	struct aatree *left, *right;
	int level;
	void *key;
};

/* Sentinel node */
static aatree_t bottom = {&bottom, &bottom, 0, NULL};

static void
skew(aatree_t **t)
{
	if ((*t)->left->level == (*t)->level) {
		/* Rotate right */
		aatree_t *temp = *t;
		*t = (*t)->left;
		temp->left = (*t)->right;
		(*t)->right = temp;
	}
}

static void
split(aatree_t **t)
{
	if ((*t)->right->right->level == (*t)->level) {
		/* Rotate left */
		aatree_t *temp = *t;
		*t = (*t)->right;
		temp->right = (*t)->left;
		(*t)->left = temp;
		(*t)->level++;
	}
}

static int
delete_node(void *x, aatree_t **t, aatree_t *deleted, int (*compar)(const void *, const void *))
{
	int retval = -1;

	if (*t != &bottom) {
		/* Search down the tree */
		aatree_t **n;

		if (compar(x, (*t)->key) < 0)
			n = &(*t)->left;
		else {
			n = &(*t)->right;
			deleted = *t;
		}

		retval = delete_node(x, n, deleted, compar);

		/* At the bottom of the tree we remove the element (if it is present) */
		if ((*n == &bottom) && (deleted != &bottom) && (compar(x, deleted->key) == 0)) {
			aatree_t *temp;
			deleted->key = (*t)->key;
			temp = *t;
			*t = (*t)->right;
			sci_free(temp);
			retval = 0;
		}
		else if (((*t)->left->level < (*t)->level - 1) || ((*t)->right->level < (*t)->level - 1)) {
			(*t)->level--;
			if ((*t)->right->level > (*t)->level)
				(*t)->right->level = (*t)->level;
			skew(t);
			skew(&(*t)->right);
			skew(&(*t)->right->right);
			split(t);
			split(&(*t)->right);
		}
	}

	return retval;
}

aatree_t *
aatree_new()
{
	return &bottom;
}

int
aatree_insert(void *x, aatree_t **t, int (*compar)(const void *, const void *))
{
	int retval = -1;
	int c;

	if (*t == &bottom) {
		*t = (aatree_t*)sci_malloc(sizeof(aatree_t));

		if (*t == NULL)
			return 1;

		(*t)->key = x;
		(*t)->left = &bottom;
		(*t)->right = &bottom;
		(*t)->level = 1;
		return 0;
	}

	c = compar(x, (*t)->key);

	if (c < 0)
		retval = aatree_insert(x, &(*t)->left, compar);
	else if (c > 0)
		retval = aatree_insert(x, &(*t)->right, compar);

	skew(t);
	split(t);
	return retval;
}

int
aatree_delete(void *x, aatree_t **t, int (*compar)(const void *, const void *))
{
	return delete_node(x, t, &bottom, compar);
}

aatree_t *
aatree_walk(aatree_t *t, int direction)
{
	if ((direction == AATREE_WALK_LEFT) && (t->left != &bottom))
		return t->left;

	if ((direction == AATREE_WALK_RIGHT) && (t->right != &bottom))
		return t->right;

	return NULL;
}

void *
aatree_get_data(aatree_t *t)
{
	return t->key;
}

void
aatree_free(aatree_t *t)
{
	if (t == &bottom)
		return;

	aatree_free(t->left);
	aatree_free(t->right);

	sci_free(t);
}
