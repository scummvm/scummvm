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

#include "pink/utils.h"
#include "pink/constants.h"
#include "pink/cursor_mgr.h"
#include "pink/objects/inventory.h"
#include "pink/objects/actions/action.h"
#include "pink/objects/actors/supporting_actor.h"

namespace Pink {

void SupportingActor::deserialize(Archive &archive) {
    Actor::deserialize(archive);
    _location = archive.readString();
    _pdaLink = archive.readString();
    _cursor = archive.readString();
    _handlerMgr.deserialize(archive);
}

void SupportingActor::toConsole() {
    debug("SupportingActor: _name = %s, _location=%s, _pdaLink=%s, _cursor=%s",
          _name.c_str(), _location.c_str(), _pdaLink.c_str(), _cursor.c_str());
    for (uint i = 0; i < _actions.size(); ++i) {
        _actions[i]->toConsole();
    }
    _handlerMgr.toConsole();
}

void SupportingActor::onMouseOver(Common::Point point, CursorMgr *mgr) {
    if (isLeftClickHandlers()){
        if (!_cursor.empty()){
            mgr->setCursor(_cursor, point);
        }
        else mgr->setCursor(kClickableFirstFrameCursor, point, Common::String());
    }
    else Actor::onMouseOver(point, mgr);
}

bool SupportingActor::isLeftClickHandlers() {
    return _handlerMgr.isLeftClickHandler(this);
}

bool SupportingActor::isUseClickHandlers(InventoryItem *item) {
    return _handlerMgr.isUseClickHandler(this, item->getName());
}

void SupportingActor::onTimerMessage() {
    _handlerMgr.onTimerMessage(this);
}

bool SupportingActor::onLeftClickMessage() {
    return _handlerMgr.onLeftClickMessage(this);
}

bool SupportingActor::onUseClickMessage(InventoryItem *item, InventoryMgr *mgr) {
    return _handlerMgr.onUseClickMessage(this, item, mgr);
}

const Common::String &SupportingActor::getLocation() const {
    return _location;
}

SupportingActor::~SupportingActor() {}

void SupportingActor::onHover(Common::Point point, const Common::String &itemName, CursorMgr *cursorMgr) {
    Common::String item = itemName;
    if (_handlerMgr.isUseClickHandler(this, itemName)) {
        item += kClickable;
    }
    Actor::onHover(point, item, cursorMgr);
}

} // End of namespace Pink