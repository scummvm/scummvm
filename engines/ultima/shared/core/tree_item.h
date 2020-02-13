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

#ifndef ULTIMA_SHARED_CORE_TREE_ITEM_H
#define ULTIMA_SHARED_CORE_TREE_ITEM_H

#include "ultima/shared/core/message_target.h"

namespace Ultima {
namespace Shared {

namespace Gfx {
class VisualItem;
} // End of namespace Gfx

namespace Maps {
class Map;
} // End of namespace Maps

class Game;
class GameManager;
class GameState;
class Events;
class Map;
class NamedItem;
class CMessage;

/**
 * This implements the base for objects can form a class hierarchy, with parents, children, and siblings
 * that messages can be passed around.
 */
class TreeItem: public MessageTarget {
	friend class CMessage;
	DECLARE_MESSAGE_MAP;
private:
	TreeItem *_parent;
	TreeItem *_nextSibling;
	TreeItem *_priorSibling;
	TreeItem *_firstChild;
	DisposeAfterUse::Flag _disposeAfterUse;
public:
	CLASSDEF;
	TreeItem();

	/**
	 * Gets the name of the item, if any
	 */
	virtual const Common::String getName() const {
		return Common::String();
	}

	/**
	 * Returns true if the item's name matches a passed name
	 */
	virtual bool isEquals(const Common::String &name, int maxLen = 0) const {
		return false;
	}

	/**
	 * Compares the name of the item to a passed name
	 */
	virtual int compareTo(const Common::String &name, int maxLen = 0) const {
		return false;
	}

	/**
	 * Called when the view changes
	 */
	virtual void viewChange() {
	}

	/**
	 * Get the parent for the given item
	 */
	TreeItem *getParent() const {
		return _parent;
	}

	/**
	 * Jumps up through the parents to find the root game
	 */
	Game *getGame();

	/**
	 * Jumps up through the parents to find the root game
	 */
	const Game *getGame() const;

	/**
	 * Returns the currently active game view
	 */
	Gfx::VisualItem *getView();

	/**
	 * Get the next sibling
	 */
	TreeItem *getNextSibling() const {
		return _nextSibling;
	}

	/**
	 * Get the prior sibling
	 */
	TreeItem *getPriorSibling() const {
		return _priorSibling;
	}

	/**
	 * Get the last sibling of this sibling
	 */
	TreeItem *getLastSibling();

	/**
	 * Get the first child of the item, if any
	 */
	TreeItem *getFirstChild() const {
		return _firstChild;
	}

	/**
	 * Get the last child of the item, if any
	 */
	TreeItem *getLastChild() const;

	/**
	 * Given all the recursive children of the tree item, gives the next
	 * item in sequence to the passed starting item
	 */
	TreeItem *scan(TreeItem *item) const;

	/**
	 * Find the first child item that is of a given type
	 */
	TreeItem *findChildInstanceOf(const ClassDef &classDef) const;

	/**
	 * Find the next sibling item that is of the given type
	 */
	TreeItem *findNextInstanceOf(const ClassDef &classDef, TreeItem *startItem) const;

	/**
	 * Adds the item under another tree item
	 */
	void addUnder(TreeItem *newParent);

	/**
	 * Adds a new child under this one
	 */
	void addChild(TreeItem *child);

	/**
	 * Sets the parent for the item
	 */
	void setParent(TreeItem *newParent);

	/**
	 * Adds the item as a sibling of another item
	 */
	void addSibling(TreeItem *item);

	/**
	 * Moves the tree item to be under another parent
	 */
	void moveUnder(TreeItem *newParent);

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
	void attach(TreeItem *item);

	/**
	 * Finds a tree item by name
	 * @param name		Name to find
	 */
	NamedItem *findByName(const Common::String &name);
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
