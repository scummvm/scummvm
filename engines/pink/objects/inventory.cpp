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

#include "pink/archive.h"
#include "pink/director.h"
#include "pink/pink.h"
#include "pink/objects/inventory.h"
#include "pink/objects/actions/action.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

InventoryMgr::InventoryMgr()
    : _lead(nullptr), _item(nullptr), _isClickedOnItem(false)
{
}

void Pink::InventoryItem::deserialize(Archive &archive) {
    NamedObject::deserialize(archive);
    _initialOwner = archive.readString();
    _currentOwner = _initialOwner;
}

Common::String &InventoryItem::getCurrentOwner() {
    return _currentOwner;
}

void InventoryItem::toConsole() {
    debug("\tInventoryItem: _initialOwner=%s _currentOwner=%s", _initialOwner, _currentOwner);
}

InventoryMgr::~InventoryMgr() {
    for (uint i = 0; i < _items.size(); ++i) {
        delete _items[i];
    }
}

void InventoryMgr::deserialize(Archive &archive) {
    archive >> _items;
}

InventoryItem *InventoryMgr::findInventoryItem(const Common::String &name) {
	for (uint i = 0; i < _items.size(); ++i) {
		if (_items[i]->getName() == name) {
			return _items[i];
		}
	}
	return nullptr;
}

void InventoryMgr::setLeadActor(LeadActor *lead) {
    _lead = lead;
}

void InventoryMgr::toConsole() {
    debug("InventoryMgr:");
    for (uint i = 0; i < _items.size(); ++i) {
        _items[i]->toConsole();
    }
}

bool InventoryMgr::isPinkOwnsAnyItems() {
    if (_item)
        return true;

    for (uint i = 0; i < _items.size(); ++i) {
        if (_items[i]->getCurrentOwner() == _lead->getName()){
            _item = _items[i];
            return true;
        }
    }

    return false;
}

void InventoryMgr::setItemOwner(const Common::String &owner, InventoryItem *item) {
    if (owner == item->getCurrentOwner())
       return;

    if (item == _item && _lead->getName() != owner)
        _item = nullptr;

    item->_currentOwner = owner;

    if (_lead->getName() == owner)
        _item = item;
}

bool InventoryMgr::start(bool playOpening) {
    if (!_item) {
        _item = findInventoryItem(_lead->getName());
        if (!_item)
            return false;
    }

    _window = _lead->getPage()->findActor(kInventoryWindowActor);
    _itemActor = _lead->getPage()->findActor(kInventoryItemActor);
    _rightArrow = _lead->getPage()->findActor(kInventoryRightArrowActor);
    _leftArrow = _lead->getPage()->findActor(kInventoryLeftArrowActor);

    if (playOpening){
        _window->setAction(kOpenAction);
        _state = kOpening;
    }

    return true;
}

void InventoryMgr::update() {
    if (_state == kOpening && !_window->isPlaying()){
        _state = kReady;
        _itemActor->setAction(_item->getName());
        _window->setAction(kShowAction);
        _leftArrow->setAction(kShowAction);
        _rightArrow->setAction(kShowAction);
    }
    else if (_state == kClosing && !_window->isPlaying()){
        _window->setAction(kIdleAction);

        _lead->onInventoryClosed(_isClickedOnItem);

        _state = kIdle;

        _window = nullptr;
        _itemActor = nullptr;
        _isClickedOnItem = false;
    }
}

void InventoryMgr::onClick(Common::Point point) {
    if (_state != kReady)
        return;

    Actor *actor = _lead->getPage()->getGame()->getDirector()->getActorByPoint(point);
    if (actor == _itemActor || actor == _window) {
        _isClickedOnItem = true;
        close();
    }
    else if (actor == _leftArrow) {
        showNextItem(kLeft);
    }
    else if (actor == _rightArrow) {
        showNextItem(kRight);
    }
    else close();
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
        if (_item == _items[i]) {
            index = i + _items.size();
            break;
        }
    }

    uint i = 0;
    do {
        index = (direction == kLeft) ? --index : ++index;
    } while(_items[index % _items.size()]->getCurrentOwner() != _item->getCurrentOwner() && ++i < _items.size());

    if (i != _items.size()) {
        _item = _items[index % _items.size()];
        _itemActor->setAction(_item->getName());
    }
}

InventoryItem *InventoryMgr::getCurrentItem() {
    return _item;
}

} // End of namespace Pink


