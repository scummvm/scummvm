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

#ifndef TITANIC_TREE_ITEM_H
#define TITANIC_TREE_ITEM_H

#include "titanic/objects/message_target.h"

namespace Titanic {

class CTreeItem: public CMessageTarget {
private:
	CTreeItem *_parent;
	CTreeItem *_nextSibling;
	CTreeItem *_priorSibling;
	CTreeItem *_firstChild;
	int _field14;
public:
	CTreeItem();

	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CTreeItem"; }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);

	/**
	 * Get the parent for the given item
	 */
	CTreeItem *getParent() const { return _parent; }

	/**
	 * Get the next sibling
	 */
	CTreeItem *getNextSibling() { return _nextSibling; }

	/**
	 * Get the prior sibling
	 */
	CTreeItem *getPriorSibling() { return _priorSibling; }

	/**
	 * Get the last sibling of this sibling
	 */
	CTreeItem *getLastSibling();

	/**
	 * Get the first child of the item, if any
	 */
	CTreeItem *getFirstChild() { return _firstChild; }

	/**
	 * Get the last child of the item, if any
	 */
	CTreeItem *getLastChild();
};

} // End of namespace Titanic

#endif /* TITANIC_TREE_ITEM_H */
