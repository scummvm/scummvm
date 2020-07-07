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


#include "common/debug.h"

#include "pink/director.h"
#include "pink/pink.h"
#include "pink/objects/inventory.h"
#include "pink/objects/actions/action.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

InventoryMgr::InventoryMgr()
	: _lead(nullptr), _window(nullptr), _itemActor(nullptr),
	_rightArrow(nullptr), _leftArrow(nullptr), _currentItem(nullptr),
	_state(kIdle), _isClickedOnItem(false) {}

void InventoryItem::deserialize(Archive &archive) {
	NamedObject::deserialize(archive);
	_initialOwner = archive.readString();
	_currentOwner = _initialOwner;
}

void InventoryItem::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\tInventoryItem: _initialOwner=%s _currentOwner=%s", _initialOwner.c_str(), _currentOwner.c_str());
}

InventoryMgr::~InventoryMgr() {
	for (uint i = 0; i < _items.size(); ++i) {
		delete _items[i];
	}
}

void InventoryMgr::deserialize(Archive &archive) {
	_items.deserialize(archive);
}

InventoryItem *InventoryMgr::findInventoryItem(const Common::String &name) {
	for (uint i = 0; i < _items.size(); ++i) {
		if (_items[i]->getName() == name) {
			return _items[i];
		}
	}
	return nullptr;
}

void InventoryMgr::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "InventoryMgr:");
	for (uint i = 0; i < _items.size(); ++i) {
		_items[i]->toConsole();
	}
}

bool InventoryMgr::isPinkOwnsAnyItems() {
	if (_currentItem)
		return true;

	for (uint i = 0; i < _items.size(); ++i) {
		if (_items[i]->getCurrentOwner() == _lead->getName()) {
			_currentItem = _items[i];
			return true;
		}
	}

	return false;
}

void InventoryMgr::setItemOwner(const Common::String &owner, InventoryItem *item) {
	if (owner == item->getCurrentOwner())
		return;

	if (item == _currentItem && _lead->getName() != owner)
		_currentItem = nullptr;
	else if (_lead->getName() == owner)
		_currentItem = item;

	item->_currentOwner = owner;
}

bool InventoryMgr::start(bool paused) {
	if (!isPinkOwnsAnyItems())
		return false;

	_window = _lead->getPage()->findActor(kInventoryWindowActor);
	_itemActor = _lead->getPage()->findActor(kInventoryItemActor);
	_rightArrow = _lead->getPage()->findActor(kInventoryRightArrowActor);
	_leftArrow = _lead->getPage()->findActor(kInventoryLeftArrowActor);

	if (!paused) {
		_window->setAction(kOpenAction);
		_state = kOpening;
	}

	return true;
}

void InventoryMgr::update() {
	if (_window->isPlaying())
		return;

	switch (_state) {
	case kOpening:
		_state = kReady;
		_itemActor->setAction(_currentItem->getName());
		_window->setAction(kShowAction);
		_leftArrow->setAction(kShowAction);
		_rightArrow->setAction(kShowAction);
		break;
	case kClosing:
		_window->setAction(kIdleAction);
		_lead->onInventoryClosed(_isClickedOnItem);
		_state = kIdle;
		_window = nullptr;
		_itemActor = nullptr;
		_isClickedOnItem = false;
		break;
	default:
		break;
	}
}

void InventoryMgr::onClick(Common::Point point) {
	if (_state != kReady)
		return;

	Actor *actor = _lead->getActorByPoint(point);
	if (actor == _itemActor || actor == _window) {
		if (_itemActor->getAction()->getName() == "WBook") {
			_lead->loadPDA("TOC");
			return;
		}
		_isClickedOnItem = true;
		close();
	} else if (actor == _leftArrow) {
		showNextItem(kLeft);
	} else if (actor == _rightArrow) {
		showNextItem(kRight);
	} else
		close();
}

void InventoryMgr::close() {
	_state = kClosing;

	_window->setAction(kCloseAction);
	_itemActor->setAction(kIdleAction);
	_leftArrow->setAction(kIdleAction);
	_rightArrow->setAction(kIdleAction);
}

void InventoryMgr::showNextItem(bool direction) {
	int index = 0;
	for (uint i = 0; i < _items.size(); ++i) {
		if (_currentItem == _items[i]) {
			index = i + _items.size();
			break;
		}
	}

	for (uint i = 0; i < _items.size(); ++i) {
		index = (direction == kLeft) ? index - 1 : index + 1;
		if (_items[index % _items.size()]->getCurrentOwner() == _currentItem->getCurrentOwner()) {
			_currentItem = _items[index % _items.size()];
			_itemActor->setAction(_currentItem->getName());
			break;
		}
	}
}

void InventoryMgr::loadState(Archive &archive) {
	_state = (State)archive.readByte();
	_isClickedOnItem = archive.readByte();

	for (uint i = 0; i < _items.size(); ++i) {
		_items[i]->_currentOwner = archive.readString();
	}

	const Common::String currItemName = archive.readString();
	if (currItemName.empty()) {
		_currentItem = nullptr;
		_isClickedOnItem = 0;
	} else {
		_currentItem = findInventoryItem(currItemName);
	}
}

void InventoryMgr::saveState(Archive &archive) {
	archive.writeByte(_state);
	archive.writeByte(_isClickedOnItem);

	for (uint i = 0; i < _items.size(); ++i) {
		archive.writeString(_items[i]->_currentOwner);
	}

	if (_currentItem)
		archive.writeString(_currentItem->getName());
	else
		archive.writeString(Common::String());
}

} // End of namespace Pink
