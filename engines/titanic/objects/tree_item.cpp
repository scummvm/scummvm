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

#include "titanic/objects/tree_item.h"

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

CTreeItem *CTreeItem::getLastSibling() {
	CTreeItem *item = this;
	while (item->getNextSibling())
		item = item->getNextSibling();

	return item;
}

CTreeItem *CTreeItem::getLastChild() {
	if (!_firstChild)
		return nullptr;
	return _firstChild->getLastSibling();
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


} // End of namespace Titanic
