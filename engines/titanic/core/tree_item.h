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

#include "titanic/core/message_target.h"
#include "titanic/support/simple_file.h"

namespace Titanic {

class CGameManager;
class CMovieClipList;
class CNamedItem;
class CProjectItem;
class CScreenManager;
class CViewItem;

class CTreeItem: public CMessageTarget {
	friend class CMessage;
	DECLARE_MESSAGE_MAP;
private:
	CTreeItem *_parent;
	CTreeItem *_nextSibling;
	CTreeItem *_priorSibling;
	CTreeItem *_firstChild;
	int _field14;
public:
	CLASSDEF;
	CTreeItem();


	/**
	 * Dump the item and any of it's children
	 */
	void dump(int indent);

	/**
	 * Dump the item
	 */
	virtual CString dumpItem(int indent) const;

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;

	/**
	 * Get the game manager for the project
	 */
	virtual CGameManager *getGameManager() const;

	/**
	 * Returns true if the item is a file item
	 */
	virtual bool isFileItem() const;

	/**
	 * Returns true if the item is a room item
	 */
	virtual bool isRoomItem() const;

	/**
	 * Returns true if the item is a node item
	 */
	virtual bool isNodeItem() const;

	/**
	 * Returns true if the item is a view item
	 */
	virtual bool isViewItem() const;

	/**
	 * Returns true if the item is a link item
	 */
	virtual bool isLinkItem() const;

	/**
	 * Returns true if the item is a placeholder item
	 */
	virtual bool isPlaceHolderItem() const;

	/**
	 * Returns true if the item is a named item
	 */
	virtual bool isNamedItem() const;

	/**
	 * Returns true if the item is a game object
	 */
	virtual bool isGameObject() const;

	/**
	 * Returns true if the item is a game object desc item
	 */
	virtual bool isGameObjectDescItem() const;

	/**
	 * Gets the name of the item, if any
	 */
	virtual const CString getName() const { return CString(); }

	/**
	 * Returns true if the item's name matches a passed name
	 */
	virtual bool isEquals(const CString &name, bool startsWith = false) const{ return false; }

	/**
	 * Compares the name of the item to a passed name
	 */
	virtual int compareTo(const CString &name, int maxLen = 0) const { return false; }

	/**
	 * Returns the clip list, if any, associated with the item
	 */
	virtual const CMovieClipList *getMovieClips() const { return nullptr; }

	/**
	 * Returns true if the given item connects to another specified view
	 */
	virtual bool connectsTo(CViewItem *destView) const { return false; }

	/**
	 * Allows the item to draw itself
	 */
	virtual void draw(CScreenManager *screenManager) {}

	/**
	 * Gets the bounds occupied by the item
	 */
	virtual Rect getBounds() const { return Rect(); }

	/**
	 * Free up any surface the object used
	 */
	virtual void freeSurface() {}

	/**
	 * Get the parent for the given item
	 */
	CTreeItem *getParent() const { return _parent; }

	/**
	 * Jumps up through the parents to find the root item
	 */
	CProjectItem *getRoot() const;

	/**
	 * Get the next sibling
	 */
	CTreeItem *getNextSibling() const { return _nextSibling; }

	/**
	 * Get the prior sibling
	 */
	CTreeItem *getPriorSibling() const { return _priorSibling; }

	/**
	 * Get the last sibling of this sibling
	 */
	CTreeItem *getLastSibling();

	/**
	 * Get the first child of the item, if any
	 */
	CTreeItem *getFirstChild() const { return _firstChild; }

	/**
	 * Get the last child of the item, if any
	 */
	CTreeItem *getLastChild() const;

	/**
	 * Given all the recursive children of the tree item, gives the next
	 * item in sequence to the passed starting item
	 */
	CTreeItem *scan(CTreeItem *item) const;

	/**
	 * Find the first child item that is of a given type
	 */
	CTreeItem *findChildInstanceOf(ClassDef *classDef) const;

	/**
	 * Find the next sibling item that is of the given type
	 */
	CTreeItem *findNextInstanceOf(ClassDef *classDef, CTreeItem *startItem) const;

	/**
	 * Adds the item under another tree item
	 */
	void addUnder(CTreeItem *newParent);

	/**
	 * Sets the parent for the item
	 */
	void setParent(CTreeItem *newParent);

	/**
	 * Adds the item as a sibling of another item
	 */
	void addSibling(CTreeItem *item);

	/**
	 * Moves the tree item to be under another parent
	 */
	void moveUnder(CTreeItem *newParent);

	/**
	 * Destroys both the item as well as any of it's children
	 */
	void destroyAll();

	/**
	 * Destroys all child tree items under this one.
	 * @returns		Total number of tree items recursively removed
	 */
	int destroyChildren();

	/**
	 * Detach the tree item from any other associated tree items
	 */
	void detach();

	/**
	 * Attaches a tree item to a new node
	 */
	void attach(CTreeItem *item);

	/**
	 * Finds a tree item by name
	 * @param name		Name to find
	 * @param subMatch	If false, does an exact name match.
	 *		If false, matches any item that starts with the given name
	 */
	CNamedItem *findByName(const CString &name, bool subMatch = false);
};

} // End of namespace Titanic

#endif /* TITANIC_TREE_ITEM_H */
