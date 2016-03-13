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

#include "titanic/core/tree_item.h"
#include "titanic/core/dont_save_file_item.h"
#include "titanic/core/file_item.h"
#include "titanic/core/named_item.h"

namespace Titanic {

CTreeItem::CTreeItem() : _parent(nullptr), _firstChild(nullptr),
	_nextSibling(nullptr), _priorSibling(nullptr), _field14(0) {
}

void CTreeItem::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
	CMessageTarget::save(file, indent);
}

void CTreeItem::load(SimpleFile *file) {
	file->readNumber();
	CMessageTarget::load(file);
}

CGameManager *CTreeItem::getGameManager() const {
	return _parent ? _parent->getGameManager() : nullptr;
}

CTreeItem *CTreeItem::getRoot() const {
	CTreeItem *parent = getParent();

	if (parent) {
		do {
			parent = parent->getParent();
		} while (parent->getParent());
	}

	return parent;
}

CTreeItem *CTreeItem::getLastSibling() {
	CTreeItem *item = this;
	while (item->getNextSibling())
		item = item->getNextSibling();

	return item;
}

CTreeItem *CTreeItem::getLastChild() const {
	if (!_firstChild)
		return nullptr;
	return _firstChild->getLastSibling();
}

CTreeItem *CTreeItem::scan(CTreeItem *item) const {
	if (_firstChild)
		return _firstChild;

	const CTreeItem *treeItem = this;
	while (treeItem != item) {
		if (treeItem->_nextSibling)
			return treeItem->_nextSibling;

		treeItem = treeItem->_parent;
		if (!treeItem)
			break;
	}

	return nullptr;
}

CTreeItem *CTreeItem::findChildInstanceOf(ClassDef *classDef) const {
	for (CTreeItem *treeItem = _firstChild; treeItem; treeItem = treeItem->getNextSibling()) {
		if (treeItem->isInstanceOf(classDef))
			return treeItem;
	}

	return nullptr;
}

CTreeItem *CTreeItem::findNextInstanceOf(ClassDef *classDef, CTreeItem *startItem) const {
	CTreeItem *treeItem = startItem ? startItem->getNextSibling() : getFirstChild();

	for (; treeItem; treeItem = treeItem->getNextSibling()) {
		if (treeItem->isInstanceOf(classDef))
			return treeItem;
	}

	return nullptr;
}

void CTreeItem::addUnder(CTreeItem *newParent) {
	if (newParent->_firstChild)
		addSibling(newParent->getLastSibling());
	else
		setParent(newParent);
}

void CTreeItem::setParent(CTreeItem *newParent) {
	_parent = newParent;
	_priorSibling = nullptr;
	_nextSibling = newParent->_firstChild;

	if (newParent->_firstChild)
		newParent->_firstChild->_priorSibling = this;
	newParent->_firstChild = this;
}

void CTreeItem::addSibling(CTreeItem *item) {
	_priorSibling = item->_nextSibling;
	_nextSibling = item->_nextSibling;
	_parent = item->_parent;

	if (item->_nextSibling)
		item->_nextSibling->_priorSibling = this;
	item->_nextSibling = this;
}

void CTreeItem::destroyAll() {
	destroyOthers();
	detach();
	delete this;
}

int CTreeItem::destroyOthers() {
	if (!_firstChild)
		return 0;

	CTreeItem *item = this, *child, *nextSibling;
	int total = 0;

	do {
		child = item->_firstChild;
		nextSibling = item->_nextSibling;

		if (child)
			total += child->destroyOthers();
		child->detach();
		delete child;
		++total;
	} while ((item = nextSibling) != nullptr);

	return total;
}

void CTreeItem::detach() {
	// Delink this item from any prior and/or next siblings
	if (_priorSibling)
		_priorSibling->_nextSibling = _nextSibling;
	if (_nextSibling)
		_nextSibling->_priorSibling = _priorSibling;

	if (_parent && _parent->_firstChild == this)
		_parent->_firstChild = _nextSibling;

	_priorSibling = _nextSibling = _parent = nullptr;
}

CNamedItem *CTreeItem::findByName(const CString &name, int maxLen) {
	CString nameLower = name;
	nameLower.toLowercase();

	for (CTreeItem *treeItem = this; treeItem; treeItem = scan(treeItem)) {
		CString nodeName = treeItem->getName();
		nodeName.toLowercase();

		if (maxLen) {
			if (nodeName.left(maxLen).compareTo(nameLower))
				return dynamic_cast<CNamedItem *>(treeItem);
		} else {
			if (nodeName.compareTo(nameLower))
				return dynamic_cast<CNamedItem *>(treeItem);
		}
	}

	return nullptr;
}

} // End of namespace Titanic
