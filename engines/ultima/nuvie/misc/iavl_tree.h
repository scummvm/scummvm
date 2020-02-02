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

#ifndef NUVIE_MISC_IAVL_TREE_H
#define NUVIE_MISC_IAVL_TREE_H

namespace Ultima {
namespace Nuvie {

/* typedef the keytype */
// TODO: Clean up object manager so it isn't intermixing pointers and ints
//typedef long iAVLKey;
union iAVLKey {
	void *_ptr;
	long _int;
};

/* Comparison function for integers is subtraction. */
#define iAVLKey_cmp(tree, a, b) ((a._int) - (b._int))


typedef struct _iAVLNode {
	iAVLKey key;
	long depth;
	void *item;
	struct _iAVLNode *parent;
	struct _iAVLNode *left;
	struct _iAVLNode *right;
} iAVLNode;


typedef struct {
	iAVLNode *top;
	long count;
	iAVLKey(*getkey)(const void *item);
} iAVLTree;


typedef struct {
	const iAVLTree *avltree;
	const iAVLNode *curnode;
} iAVLCursor;


extern iAVLTree *iAVLAllocTree(iAVLKey(*getkey)(void const *item));
extern void iAVLFreeTree(iAVLTree *avltree, void (freeitem)(void *item));
extern void iAVLCleanTree(iAVLTree *avltree, void (freeitem)(void *item));
extern int iAVLInsert(iAVLTree *avltree, void *item);
extern void *iAVLSearch(iAVLTree const *avltree, iAVLKey key);
extern int iAVLDelete(iAVLTree *avltree, iAVLKey key);
extern void *iAVLFirst(iAVLCursor *avlcursor, iAVLTree const *avltree);
extern void *iAVLNext(iAVLCursor *avlcursor);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
