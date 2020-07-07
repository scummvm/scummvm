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

#include "pink/pink.h"
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

void SupportingActor::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "SupportingActor: _name = %s, _location=%s, _pdaLink=%s, _cursor=%s",
		  _name.c_str(), _location.c_str(), _pdaLink.c_str(), _cursor.c_str());
	for (uint i = 0; i < _actions.size(); ++i) {
		_actions[i]->toConsole();
	}
	_handlerMgr.toConsole();
}

bool SupportingActor::isLeftClickHandlers() const {
	return _handlerMgr.findSuitableHandlerLeftClick(this);
}

bool SupportingActor::isUseClickHandlers(InventoryItem *item) const {
	return _handlerMgr.findSuitableHandlerUseClick(this, item->getName());
}

void SupportingActor::onMouseOver(Common::Point point, CursorMgr *mgr) {
	if (isLeftClickHandlers()) {
		if (!_cursor.empty())
			mgr->setCursor(_cursor, point);
		else
			mgr->setCursor(kClickableFirstFrameCursor, point, Common::String());
	} else
		Actor::onMouseOver(point, mgr);
}

void SupportingActor::onMouseOverWithItem(Common::Point point, const Common::String &itemName, CursorMgr *cursorMgr) {
	Common::String item = itemName;
	if (_handlerMgr.findSuitableHandlerUseClick(this, itemName))
		item += kClickable;
	cursorMgr->setCursor(kHoldingItemCursor, point, item);
}

void SupportingActor::onTimerMessage() {
	_handlerMgr.onTimerMessage(this);
}

void SupportingActor::onLeftClickMessage() {
	_handlerMgr.onLeftClickMessage(this);
}

void SupportingActor::onUseClickMessage(InventoryItem *item, InventoryMgr *mgr) {
	_handlerMgr.onUseClickMessage(this, item, mgr);
}

Common::String SupportingActor::getLocation() const {
	return _location;
}

Common::String SupportingActor::getPDALink() const {
	return _pdaLink;
}

} // End of namespace Pink
