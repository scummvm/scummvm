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

#include "ultima/shared/core/tree_item.h"
#include "ultima/shared/core/named_item.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/early/game_base.h"

namespace Ultima {
namespace Shared {

EMPTY_MESSAGE_MAP(TreeItem, MessageTarget);

TreeItem::TreeItem() : _parent(nullptr), _firstChild(nullptr),
_nextSibling(nullptr), _priorSibling(nullptr),
_disposeAfterUse(DisposeAfterUse::NO) {
}

Game *TreeItem::getGame() {
	TreeItem *treeItem = this;

	while (treeItem->getParent()) {
		treeItem = treeItem->getParent();
	}

	return dynamic_cast<Game *>(treeItem);
}

const Game *TreeItem::getGame() const {
	const TreeItem *treeItem = this;

	while (treeItem->getParent()) {
		treeItem = treeItem->getParent();
	}

	return dynamic_cast<const Game *>(treeItem);
}

TreeItem *TreeItem::getLastSibling() {
	TreeItem *item = this;
	while (item->getNextSibling())
		item = item->getNextSibling();

	return item;
}

Gfx::VisualItem *TreeItem::getView() {
	return getGame()->getView();
}

TreeItem *TreeItem::getLastChild() const {
	if (!_firstChild)
		return nullptr;
	return _firstChild->getLastSibling();
}

TreeItem *TreeItem::scan(TreeItem *item) const {
	if (_firstChild)
		return _firstChild;

	const TreeItem *treeItem = this;
	while (treeItem != item) {
		if (treeItem->_nextSibling)
			return treeItem->_nextSibling;

		treeItem = treeItem->_parent;
		if (!treeItem)
			break;
	}

	return nullptr;
}

TreeItem *TreeItem::findChildInstanceOf(const ClassDef &classDef) const {
	for (TreeItem *treeItem = _firstChild; treeItem; treeItem = treeItem->getNextSibling()) {
		if (treeItem->isInstanceOf(classDef))
			return treeItem;
	}

	return nullptr;
}

TreeItem *TreeItem::findNextInstanceOf(const ClassDef &classDef, TreeItem *startItem) const {
	TreeItem *treeItem = startItem ? startItem->getNextSibling() : getFirstChild();

	for (; treeItem; treeItem = treeItem->getNextSibling()) {
		if (treeItem->isInstanceOf(classDef))
			return treeItem;
	}

	return nullptr;
}

void TreeItem::addUnder(TreeItem *newParent) {
	if (newParent->_firstChild)
		addSibling(newParent->_firstChild->getLastSibling());
	else
		setParent(newParent);
}

void TreeItem::addChild(TreeItem *child) {
	child->addUnder(this);
}

void TreeItem::setParent(TreeItem *newParent) {
	_parent = newParent;
	_priorSibling = nullptr;
	_nextSibling = newParent->_firstChild;

	if (newParent->_firstChild)
		newParent->_firstChild->_priorSibling = this;
	newParent->_firstChild = this;
}

void TreeItem::addSibling(TreeItem *item) {
	_priorSibling = item;
	_nextSibling = item->_nextSibling;
	_parent = item->_parent;

	if (item->_nextSibling)
		item->_nextSibling->_priorSibling = this;
	item->_nextSibling = this;
}

void TreeItem::moveUnder(TreeItem *newParent) {
	if (newParent) {
		detach();
		addUnder(newParent);
	}
}

void TreeItem::destroyAll() {
	destroyChildren();
	detach();
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete this;
}

int TreeItem::destroyChildren() {
	if (!_firstChild)
		return 0;

	TreeItem *item = _firstChild, *child, *nextSibling;
	int total = 0;

	do {
		child = item->_firstChild;
		nextSibling = item->_nextSibling;

		if (child)
			total += item->destroyChildren();
		item->detach();
		if (item->_disposeAfterUse == DisposeAfterUse::YES)
			delete item;
		++total;
	} while ((item = nextSibling) != nullptr);

	return total;
}

void TreeItem::detach() {
	// Delink this item from any prior and/or next siblings
	if (_priorSibling)
		_priorSibling->_nextSibling = _nextSibling;
	if (_nextSibling)
		_nextSibling->_priorSibling = _priorSibling;

	if (_parent && _parent->_firstChild == this)
		_parent->_firstChild = _nextSibling;

	_priorSibling = _nextSibling = _parent = nullptr;
}

void TreeItem::attach(TreeItem *item) {
	_nextSibling = item;
	_priorSibling = item->_priorSibling;
	_parent = item->_parent;

	if (item->_priorSibling)
		item->_priorSibling->_nextSibling = this;

	item->_priorSibling = this;
	if (item->_parent && !item->_parent->_firstChild)
		item->_parent->_firstChild = this;
}

NamedItem *TreeItem::findByName(const Common::String &name) {
	Common::String nameLower = name;
	nameLower.toLowercase();

	for (TreeItem *treeItem = this; treeItem; treeItem = treeItem->scan(this)) {
		Common::String itemName = treeItem->getName();
		itemName.toLowercase();

		if (!itemName.compareTo(nameLower))
			return dynamic_cast<NamedItem *>(treeItem);
	}

	return nullptr;
}

} // End of namespace Shared
} // End of namespace Ultima
