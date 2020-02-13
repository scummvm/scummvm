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

#include "ultima/nuvie/misc/iavl_tree.h"
#include "ultima/nuvie/core/nuvie_defs.h"

namespace Ultima {
namespace Nuvie {

static iAVLNode *iAVLCloseSearchNode(iAVLTree const *avltree, iAVLKey key);
static void iAVLRebalanceNode(iAVLTree *avltree, iAVLNode *avlnode);
static void iAVLFreeBranch(iAVLNode *avlnode, void (freeitem)(void *item));
static void iAVLFillVacancy(iAVLTree *avltree,
                            iAVLNode *origparent, iAVLNode **superparent,
                            iAVLNode *left, iAVLNode *right);

#define MAX(x, y)      ((x) > (y) ? (x) : (y))
#define MIN(x, y)      ((x) < (y) ? (x) : (y))
#define L_DEPTH(n)     ((n)->left ? (n)->left->depth : 0)
#define R_DEPTH(n)     ((n)->right ? (n)->right->depth : 0)
#define CALC_DEPTH(n)  (MAX(L_DEPTH(n), R_DEPTH(n)) + 1)


/*
 * AVLAllocTree:
 * Allocate memory for a new AVL tree and set the getkey function for
 * that tree.  The getkey function should take an item and return an
 * AVLKey that is to be used for indexing this object in the AVL tree.
 * On success, a pointer to the malloced AVLTree is returned.  If there
 * was a malloc failure, then NULL is returned.
 */
iAVLTree *iAVLAllocTree(iAVLKey(*getkey)(void const *item)) {
	iAVLTree *rc;

	rc = (iAVLTree *)malloc(sizeof(iAVLTree));
	if (rc == NULL)
		return NULL;

	rc->top = NULL;
	rc->count = 0;
	rc->getkey = getkey;
	return rc;
}


/*
 * AVLFreeTree:
 * Free all memory used by this AVL tree.  If freeitem is not NULL, then
 * it is assumed to be a destructor for the items reference in the AVL
 * tree, and they are deleted as well.
 */
void iAVLFreeTree(iAVLTree *avltree, void (freeitem)(void *item)) {
	iAVLCleanTree(avltree, freeitem);
	free(avltree);
}

void iAVLCleanTree(iAVLTree *avltree, void (freeitem)(void *item)) {
	if (avltree->top)
		iAVLFreeBranch(avltree->top, freeitem);

	avltree->top = NULL;
	avltree->count = 0;
}

/*
 * AVLInsert:
 * Create a new node and insert an item there.
 *
 * Returns  0 on success,
 *         -1 on malloc failure,
 *          3 if duplicate key.
 */
int iAVLInsert(iAVLTree *avltree, void *item) {
	iAVLNode *newnode;
	iAVLNode *node;
	iAVLNode *balnode;
	iAVLNode *nextbalnode;

	newnode = (iAVLNode *)malloc(sizeof(iAVLNode));
	if (newnode == NULL)
		return -1;

	newnode->key = avltree->getkey(item);
	newnode->item = item;
	newnode->depth = 1;
	newnode->left = NULL;
	newnode->right = NULL;
	newnode->parent = NULL;

	if (avltree->top != NULL) {
		node = iAVLCloseSearchNode(avltree, newnode->key);

		if (!iAVLKey_cmp(avltree, node->key, newnode->key)) {
			free(newnode);
			return 3;
		}

		newnode->parent = node;

		if (iAVLKey_cmp(avltree, newnode->key, node->key) < 0) {
			node->left = newnode;
			node->depth = CALC_DEPTH(node);
		}

		else {
			node->right = newnode;
			node->depth = CALC_DEPTH(node);
		}

		for (balnode = node->parent; balnode; balnode = nextbalnode) {
			nextbalnode = balnode->parent;
			iAVLRebalanceNode(avltree, balnode);
		}
	}

	else {
		avltree->top = newnode;
	}

	avltree->count++;
	return 0;
}


/*
 * iAVLSearch:
 * Return a pointer to the item with the given key in the AVL tree.  If
 * no such item is in the tree, then NULL is returned.
 */
void *iAVLSearch(iAVLTree const *avltree, iAVLKey key) {
	iAVLNode *node;

	node = iAVLCloseSearchNode(avltree, key);

	if (node && !iAVLKey_cmp(avltree, node->key, key))
		return node->item;

	return NULL;
}


/*
 * iAVLDelete:
 * Deletes the node with the given key.  Does not delete the item at
 * that key.  Returns 0 on success and -1 if a node with the given key
 * does not exist.
 */
int iAVLDelete(iAVLTree *avltree, iAVLKey key) {
	iAVLNode *avlnode;
	iAVLNode *origparent;
	iAVLNode **superparent;

	avlnode = iAVLCloseSearchNode(avltree, key);
	if (avlnode == NULL || iAVLKey_cmp(avltree, avlnode->key, key))
		return -1;

	origparent = avlnode->parent;

	if (origparent) {
		if (iAVLKey_cmp(avltree, avlnode->key, avlnode->parent->key) < 0)
			superparent = &(avlnode->parent->left);
		else
			superparent = &(avlnode->parent->right);
	} else
		superparent = &(avltree->top);

	iAVLFillVacancy(avltree, origparent, superparent,
	                avlnode->left, avlnode->right);
	free(avlnode);
	avltree->count--;
	return 0;
}


/*
 * iAVLFirst:
 * Initializes an iAVLCursor object and returns the item with the lowest
 * key in the iAVLTree.
 */
void *iAVLFirst(iAVLCursor *avlcursor, iAVLTree const *avltree) {
	const iAVLNode *avlnode;

	avlcursor->avltree = avltree;

	if (avltree->top == NULL) {
		avlcursor->curnode = NULL;
		return NULL;
	}

	for (avlnode = avltree->top;
	        avlnode->left != NULL;
	        avlnode = avlnode->left);
	avlcursor->curnode = avlnode;
	return avlnode->item;
}


/*
 * iAVLNext:
 * Called after an iAVLFirst() call, this returns the item with the least
 * key that is greater than the last item returned either by iAVLFirst()
 * or a previous invokation of this function.
 */
void *iAVLNext(iAVLCursor *avlcursor) {
	const iAVLNode *avlnode;

	avlnode = avlcursor->curnode;

	if (avlnode->right != NULL) {
		for (avlnode = avlnode->right;
		        avlnode->left != NULL;
		        avlnode = avlnode->left);
		avlcursor->curnode = avlnode;
		return avlnode->item;
	}

	while (avlnode->parent && avlnode->parent->left != avlnode) {
		avlnode = avlnode->parent;
	}

	if (avlnode->parent == NULL) {
		avlcursor->curnode = NULL;
		return NULL;
	}

	avlcursor->curnode = avlnode->parent;
	return avlnode->parent->item;
}


/*
 * iAVLCloseSearchNode:
 * Return a pointer to the node closest to the given key.
 * Returns NULL if the AVL tree is empty.
 */
iAVLNode *iAVLCloseSearchNode(iAVLTree const *avltree, iAVLKey key) {
	iAVLNode *node;

	node = avltree->top;

	if (!node)
		return NULL;

	for (;;) {
		if (!iAVLKey_cmp(avltree, node->key, key))
			return node;

		if (iAVLKey_cmp(avltree, node->key, key) < 0) {
			if (node->right)
				node = node->right;
			else
				return node;
		}

		else {
			if (node->left)
				node = node->left;
			else
				return node;
		}
	}
}


/*
 * iAVLRebalanceNode:
 * Rebalances the AVL tree if one side becomes too heavy.  This function
 * assumes that both subtrees are AVL trees with consistant data.  This
 * function has the additional side effect of recalculating the depth of
 * the tree at this node.  It should be noted that at the return of this
 * function, if a rebalance takes place, the top of this subtree is no
 * longer going to be the same node.
 */
void iAVLRebalanceNode(iAVLTree *avltree, iAVLNode *avlnode) {
	long depthdiff;
	iAVLNode *child;
	iAVLNode *gchild;
	iAVLNode *origparent;
	iAVLNode **superparent;

	origparent = avlnode->parent;

	if (origparent) {
		if (iAVLKey_cmp(avltree, avlnode->key, avlnode->parent->key) < 0)
			superparent = &(avlnode->parent->left);
		else
			superparent = &(avlnode->parent->right);
	} else
		superparent = &(avltree->top);

	depthdiff = R_DEPTH(avlnode) - L_DEPTH(avlnode);

	if (depthdiff <= -2) {
		child = avlnode->left;

		if (L_DEPTH(child) >= R_DEPTH(child)) {
			avlnode->left = child->right;
			if (avlnode->left != NULL)
				avlnode->left->parent = avlnode;
			avlnode->depth = CALC_DEPTH(avlnode);
			child->right = avlnode;
			if (child->right != NULL)
				child->right->parent = child;
			child->depth = CALC_DEPTH(child);
			*superparent = child;
			child->parent = origparent;
		}

		else {
			gchild = child->right;
			avlnode->left = gchild->right;
			if (avlnode->left != NULL)
				avlnode->left->parent = avlnode;
			avlnode->depth = CALC_DEPTH(avlnode);
			child->right = gchild->left;
			if (child->right != NULL)
				child->right->parent = child;
			child->depth = CALC_DEPTH(child);
			gchild->right = avlnode;
			if (gchild->right != NULL)
				gchild->right->parent = gchild;
			gchild->left = child;
			if (gchild->left != NULL)
				gchild->left->parent = gchild;
			gchild->depth = CALC_DEPTH(gchild);
			*superparent = gchild;
			gchild->parent = origparent;
		}
	}

	else if (depthdiff >= 2) {
		child = avlnode->right;

		if (R_DEPTH(child) >= L_DEPTH(child)) {
			avlnode->right = child->left;
			if (avlnode->right != NULL)
				avlnode->right->parent = avlnode;
			avlnode->depth = CALC_DEPTH(avlnode);
			child->left = avlnode;
			if (child->left != NULL)
				child->left->parent = child;
			child->depth = CALC_DEPTH(child);
			*superparent = child;
			child->parent = origparent;
		}

		else {
			gchild = child->left;
			avlnode->right = gchild->left;
			if (avlnode->right != NULL)
				avlnode->right->parent = avlnode;
			avlnode->depth = CALC_DEPTH(avlnode);
			child->left = gchild->right;
			if (child->left != NULL)
				child->left->parent = child;
			child->depth = CALC_DEPTH(child);
			gchild->left = avlnode;
			if (gchild->left != NULL)
				gchild->left->parent = gchild;
			gchild->right = child;
			if (gchild->right != NULL)
				gchild->right->parent = gchild;
			gchild->depth = CALC_DEPTH(gchild);
			*superparent = gchild;
			gchild->parent = origparent;
		}
	}

	else {
		avlnode->depth = CALC_DEPTH(avlnode);
	}
}


/*
 * iAVLFreeBranch:
 * Free memory used by this node and its item.  If the freeitem argument
 * is not NULL, then that function is called on the items to free their
 * memory as well.  In other words, the freeitem function is a
 * destructor for the items in the tree.
 */
void iAVLFreeBranch(iAVLNode *avlnode, void (freeitem)(void *item)) {
	if (avlnode->left)
		iAVLFreeBranch(avlnode->left, freeitem);
	if (avlnode->right)
		iAVLFreeBranch(avlnode->right, freeitem);
	if (freeitem != NULL)
		freeitem(avlnode->item);
	free(avlnode);
}


/*
 * iAVLFillVacancy:
 * Given a vacancy in the AVL tree by it's parent, children, and parent
 * component pointer, fill that vacancy.
 */
void iAVLFillVacancy(iAVLTree *avltree,
                     iAVLNode *origparent, iAVLNode **superparent,
                     iAVLNode *left, iAVLNode *right) {
	iAVLNode *avlnode;
	iAVLNode *balnode;
	iAVLNode *nextbalnode;

	if (left == NULL) {
		if (right)
			right->parent = origparent;

		*superparent = right;
		balnode = origparent;
	}

	else {
		for (avlnode = left; avlnode->right != NULL; avlnode = avlnode->right);

		if (avlnode == left) {
			balnode = avlnode;
		} else {
			balnode = avlnode->parent;
			balnode->right = avlnode->left;
			if (balnode->right != NULL)
				balnode->right->parent = balnode;
			avlnode->left = left;
			left->parent = avlnode;
		}

		avlnode->right = right;
		if (right != NULL)
			right->parent = avlnode;
		*superparent = avlnode;
		avlnode->parent = origparent;
	}

	for (; balnode; balnode = nextbalnode) {
		nextbalnode = balnode->parent;
		iAVLRebalanceNode(avltree, balnode);
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
