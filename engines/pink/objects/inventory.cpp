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


#include <common/debug.h>
#include "inventory.h"
#include "engines/pink/archive.h"
#include "pink/objects/actors/lead_actor.h"

namespace Pink {

InventoryMgr::InventoryMgr()
    : _lead(nullptr), _item(nullptr)
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

InventoryItem *InventoryMgr::findInventoryItem(Common::String &name) {
    return *Common::find_if(_items.begin(), _items.end(), [&name]
            (InventoryItem *item) {
        return name == item->getName();
    });;
}

void InventoryMgr::setLeadActor(LeadActor *lead) {
    _lead = lead;
}

void InventoryMgr::toConsole() {
    debug("InventoryMgr:");
    for (int i = 0; i < _items.size(); ++i) {
        _items[i]->toConsole();
    }
}

bool InventoryMgr::isPinkOwnsAnyItems() {
    if (_item)
        return true;

    for (int i = 0; i < _items.size(); ++i) {
        if (_items[i]->getCurrentOwner() == _lead->getName()){
            _item = _items[i];
            return true;
        }
    }

    return false;
}

void InventoryMgr::setItemOwner(const Common::String &owner, InventoryItem *item) {
    item->_currentOwner = owner;
    _item = item;
}

} // End of namespace Pink


