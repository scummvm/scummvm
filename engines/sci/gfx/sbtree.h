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

/* Static binary tree header file */

/* Static binary trees are used to link ints to pointers. They are not capable
** of resizing after being initialized.
*/

#ifndef _SBTREE_H_
#define _SBTREE_H_

#ifdef SBTREE_DEBUG
#  include <stdio.h>
#endif

namespace Sci {

struct sbtree_t {
	int entries_nr;
	int min_entry;
	int max_entry;
	int levels;
	int alloced_entries;
	void *data;
};


sbtree_t *sbtree_new(int size, int *keys);
/* Generates a new sbtree with the specified key set
** Parameters: (int) size: Number of entries in 'keys'
**             (int *) keys: Array of 'size' integer keys
** Returns   : (sbtree_t *) A new sbtree
** Only positive keys are allowed. Duplicates are not detected and will
** cause additional memory usage and slower access times if not removed
** beforehand.
*/

void sbtree_free(sbtree_t *tree);
/* Frees an sbtree
** Parameters: (sbtree_t *) tree: The tree to free
** Returns   : (void) Nothing at all
*/

int sbtree_set(sbtree_t *tree, int key, void *value);
/* Sets a key to a value
** Parameters: (sbtree_t *) tree: The tree to modify
**             (int) key: The key to set
**             (void *) value: The value part of the (key,value) tuple
** Returns   : (int) 0 if everything went OK, -1 if the key was invalid
** value may, of course, be NULL here.
*/

void *sbtree_get(sbtree_t *tree, int key);
/* Retreives a key
** Parameters: (sbtree_t *) tree: The tree to search in
**             (int) key: The key to retrieve
** Returns   : (void *) The value mapped to the key
** If key was not found/invalid, NULL is returned. Note that there is no
** way of distinguishing between keys mapped to NULL and invalid keys,
** short of attempting to set that key.
*/

void sbtree_foreach(sbtree_t *tree, void *args, void *(*operation)(sbtree_t *, const int,
               const void *, void *));
/* Operates once on each entry in the tree
** Parameters: (sbtree_t *) tree: The tree to operate on
**             (void *) arguments: Additional arguments to pass to 'operation'
**             (int (int, void *, void *)) operation: The operation to execute
** Returns   : (void)
** The parameters of the function that is to be executed are as follows:
** operation (sbtree_t *tree, int key, void *value, void *args)
** Parameters: (sbtree_t *) tree: The tree the operation was executed on
**             (int) key: Key of the entry the operation was invoked on
**             (void *) value: Value of the entry the operation was invoked on
**             (void *) args: The 'args' parameter passed to sbtree_foreach()
** Returns   : (void *) The new value of this entry
** This function will only work properly the original data contained no duplicate keys.
*/

} // End of namespace Sci

#endif /* !_SBTREE_H_ */





