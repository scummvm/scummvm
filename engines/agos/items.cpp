/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/file.h"

#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/vga.h"

namespace AGOS {

Child *AGOSEngine::allocateChildBlock(Item *i, uint type, uint size) {
	Child *child = (Child *)allocateItem(size);
	child->next = i->children;
	i->children = child;
	child->type = type;
	return child;
}

byte *AGOSEngine::allocateItem(uint size) {
	byte *org = _itemHeapPtr;
	size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

	_itemHeapPtr += size;
	_itemHeapCurPos += size;

	if (_itemHeapCurPos > _itemHeapSize)
		error("allocateItem: Itemheap overflow");

	return org;
}

void AGOSEngine::allocItemHeap() {
	_itemHeapSize = _itemMemSize;
	_itemHeapCurPos = 0;
	_itemHeapPtr = (byte *)calloc(_itemMemSize, 1);
	if (!_itemHeapPtr)
		error("Out Of Memory - Items");
}

bool AGOSEngine::hasIcon(Item *item) {
	if (getGameType() == GType_ELVIRA1) {
		return (getUserFlag(item, 7) != 0);
	} else {
		SubObject *child = (SubObject *)findChildOfType(item, 2);
		return (child && (child->objectFlags & kOFIcon) != 0);
	}
}

uint AGOSEngine::itemGetIconNumber(Item *item) {
	if (getGameType() == GType_ELVIRA1) {
		return getUserFlag(item, 7);
	} else {
		SubObject *child = (SubObject *)findChildOfType(item, 2);
		uint offs;

		if (child == NULL || !(child->objectFlags & kOFIcon))
			return 0;

		offs = getOffsetOfChild2Param(child, 0x10);
		return child->objectFlagValue[offs];
	}
}

void AGOSEngine::setItemState(Item *item, int value) {
	item->state = value;
}

void AGOSEngine::createPlayer() {
	SubPlayer *p;

	_currentPlayer = _itemArrayPtr[1];
	_currentPlayer->adjective = -1;
	_currentPlayer->noun = 10000;

	p = (SubPlayer *)allocateChildBlock(_currentPlayer, 3, sizeof(SubPlayer));
	if (p == NULL)
		error("createPlayer: player create failure");

	p->size = 0;
	p->weight = 0;
	p->strength = 6000;
	//p->flag = xxx;
	p->level = 1;
	p->score = 0;

	setUserFlag(_currentPlayer, 0, 0);
}

Child *AGOSEngine::findChildOfType(Item *i, uint type) {
	Child *child = i->children;
	for (; child; child = child->next)
		if (child->type == type)
			return child;
	return NULL;
}

int AGOSEngine::getUserFlag(Item *item, int a) {
	SubUserFlag *subUserFlag;

	subUserFlag = (SubUserFlag *) findChildOfType(item, 9);
	if (subUserFlag == NULL)
		return 0;

	if (a < 0 || a > 7)
		return 0;

	return	subUserFlag->userFlags[a];
}

void AGOSEngine::setUserFlag(Item *item, int a, int b) {
	SubUserFlag *subUserFlag;

	subUserFlag = (SubUserFlag *) findChildOfType(item, 9);
	if (subUserFlag == NULL) {
		subUserFlag = (SubUserFlag *) allocateChildBlock(item, 9, sizeof(SubUserFlag));
	}

	if (a < 0 || a > 7)
		return;

	subUserFlag->userFlags[a] = b;
}

int AGOSEngine::getUserItem(Item *item, int n) {
	SubUserFlag *subUserFlag;

	subUserFlag = (SubUserFlag *) findChildOfType(item, 9);
	if (subUserFlag == NULL)
		return 0;

	if (n < 0 || n > 0)
		return 0;

	return	subUserFlag->userItems[n];
}

void AGOSEngine::setUserItem(Item *item, int n, int m) {
	SubUserFlag *subUserFlag;

	subUserFlag = (SubUserFlag *) findChildOfType(item, 9);
	if (subUserFlag == NULL) {
		subUserFlag = (SubUserFlag *) allocateChildBlock(item, 9, sizeof(SubUserFlag));
	}

	if (n == 0)
		subUserFlag->userItems[n] = m;
}

bool AGOSEngine::isRoom(Item *item) {
	return findChildOfType(item, 1) != NULL;
}

bool AGOSEngine::isObject(Item *item) {
	return findChildOfType(item, 2) != NULL;
}

bool AGOSEngine::isPlayer(Item *item) {
	return findChildOfType(item, 3) != NULL;
}

uint AGOSEngine::getOffsetOfChild2Param(SubObject *child, uint prop) {
	uint m = 1;
	uint offset = 0;
	while (m != prop) {
		if (child->objectFlags & m)
			offset++;
		m *= 2;
	}
	return offset;
}

Item *AGOSEngine::me() {
	if (_currentPlayer)
		return _currentPlayer;
	return _dummyItem1;
}

Item *AGOSEngine::actor() {
	error("actor: is this code ever used?");
	//if (_actorPlayer)
	//	return _actorPlayer;
	return _dummyItem1;
}

Item *AGOSEngine::getNextItemPtr() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return _subjectItem;
	case -3:
		return _objectItem;
	case -5:
		return me();
	case -7:
		return actor();
	case -9:
		return derefItem(me()->parent);
	default:
		return derefItem(a);
	}
}

Item *AGOSEngine::getNextItemPtrStrange() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return _subjectItem;
	case -3:
		return _objectItem;
	case -5:
		return _dummyItem2;
	case -7:
		return NULL;
	case -9:
		return _dummyItem3;
	default:
		return derefItem(a);
	}
}

uint AGOSEngine::getNextItemID() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return itemPtrToID(_subjectItem);
	case -3:
		return itemPtrToID(_objectItem);
	case -5:
		return getItem1ID();
	case -7:
		return 0;
	case -9:
		return me()->parent;
	default:
		return a;
	}
}

void AGOSEngine::setItemParent(Item *item, Item *parent) {
	Item *old_parent = derefItem(item->parent);

	if (item == parent)
		error("setItemParent: Trying to set item as its own parent");

	// unlink it if it has a parent
	if (old_parent)
		unlinkItem(item);
	itemChildrenChanged(old_parent);
	linkItem(item, parent);
	itemChildrenChanged(parent);
}

void AGOSEngine::itemChildrenChanged(Item *item) {
	int i;
	WindowBlock *window;

	if (_noParentNotify)
		return;

	mouseOff();

	for (i = 0; i != 8; i++) {
		window = _windowArray[i];
		if (window && window->iconPtr && window->iconPtr->itemRef == item) {
			if (_fcsData1[i]) {
				_fcsData2[i] = true;
			} else {
				_fcsData2[i] = false;
				drawIconArray(i, item, window->iconPtr->line, window->iconPtr->classMask);
			}
		}
	}

	mouseOn();
}

void AGOSEngine::unlinkItem(Item *item) {
	Item *first, *parent, *next;

	// can't unlink item without parent
	if (item->parent == 0)
		return;

	// get parent and first child of parent
	parent = derefItem(item->parent);
	first = derefItem(parent->child);

	// the node to remove is first in the parent's children?
	if (first == item) {
		parent->child = item->next;
		item->parent = 0;
		item->next = 0;
		return;
	}

	for (;;) {
		if (!first)
			error("unlinkItem: parent empty");
		if (first->next == 0)
			error("unlinkItem: parent does not contain child");

		next = derefItem(first->next);
		if (next == item) {
			first->next = next->next;
			item->parent = 0;
			item->next = 0;
			return;
		}
		first = next;
	}
}

void AGOSEngine::linkItem(Item *item, Item *parent) {
	uint id;
	// Don't allow that an item that is already linked is relinked
	if (item->parent)
		return;

	id = itemPtrToID(parent);
	item->parent = id;

	if (parent != 0) {
		item->next = parent->child;
		parent->child = itemPtrToID(item);
	} else {
		item->next = 0;
	}
}

int AGOSEngine::wordMatch(Item *item, int16 a, int16 n) {
	if ((a == -1) && (n == item->noun))
		return 1;
	if ((a == item->adjective) && (n == item->noun))
		return 1 ;

	return 0;
}

Item *AGOSEngine::derefItem(uint item) {
	if (item >= _itemArraySize) {
		debug(1, "derefItem: invalid item %d", item);
		return 0;
	}
	return _itemArrayPtr[item];
}

Item *AGOSEngine::findInByClass(Item *i, int16 m) {
	i = derefItem(i->child);

	while (i) {
		if (i->classFlags & m) {
			//_findNextPtr = derefItem(i->next);
			return i;
		}
		if (m == 0) {
			//_findNextPtr = derefItem(i->next);
			return i;
		}
		i = derefItem(i->next);
	}

	return NULL;
}

Item *AGOSEngine::findMaster(int16 a, int16 n) {
	uint j;

	for (j = 1; j < _itemArraySize; j++) {
		Item *item = derefItem(j);
		if (wordMatch(item, a, n))
			return item;
	}

	return NULL;
}

Item *AGOSEngine::nextMaster(Item *i, int16 a, int16 n) {
	uint j;
	uint first = itemPtrToID(i) + 1;

	for (j = first; j < _itemArraySize; j++) {
		Item *item = derefItem(j);
		if (wordMatch(item, a, n))
			return item;
	}

	return NULL;
}

uint AGOSEngine::itemPtrToID(Item *id) {
	uint i;
	for (i = 0; i != _itemArraySize; i++)
		if (_itemArrayPtr[i] == id)
			return i;
	error("itemPtrToID: not found");
	return 0;
}

} // End of namespace AGOS
