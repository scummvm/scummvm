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

#include "pink/constants.h"
#include "pink/cursor_mgr.h"
#include "pink/pink.h"
#include "pink/objects/actions/action_cel.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

Actor::Actor()
		: _page(nullptr), _action(nullptr),
		  _isActionEnded(1) {}

Actor::~Actor() {
	for (uint i = 0; i < _actions.size(); ++i) {
		delete _actions[i];
	}
}

void Actor::deserialize(Archive &archive) {
	NamedObject::deserialize(archive);
	_page = static_cast<Page *>(archive.readObject());
	_actions.deserialize(archive);
}

void Actor::loadState(Archive &archive) {
	_action = findAction(archive.readString());
}

void Actor::saveState(Archive &archive) {
	Common::String actionName;

	if (_action)
		actionName = _action->getName();

	archive.writeString(actionName);
}

void Actor::init(bool paused) {
	if (!_action)
		_action = findAction(kIdleAction);

	if (!_action) {
		_isActionEnded = 1;
	} else {
		_isActionEnded = 0;
		_action->start();
		_action->pause(paused);
	}
}

bool Actor::initPalette(Director *director) {
	for (uint i = 0; i < _actions.size(); ++i) {
		if (_actions[i]->initPalette(director))
			return true;
	}
	return false;
}

void Actor::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "Actor: _name = %s", _name.c_str());
	for (uint i = 0; i < _actions.size(); ++i) {
		_actions[i]->toConsole();
	}
}

void Actor::pause(bool paused) {
	if (_action)
		_action->pause(paused);
}

void Actor::onMouseOver(Common::Point point, CursorMgr *mgr) {
	mgr->setCursor(kDefaultCursor, point, Common::String());
}

void Actor::onMouseOverWithItem(Common::Point point, const Common::String &itemName, CursorMgr *cursorMgr) {
	cursorMgr->setCursor(kHoldingItemCursor, point, itemName);
}

Action *Actor::findAction(const Common::String &name) {
	for (uint i = 0; i < _actions.size(); ++i) {
		if (_actions[i]->getName() == name)
			return _actions[i];
	}
	return nullptr;
}

Common::String Actor::getLocation() const {
	return Common::String();
}

void Actor::setAction(Action *newAction) {
	if (_action) {
		_isActionEnded = 1;
		_action->end();
	}
	_action = newAction;
	if (newAction) {
		_isActionEnded = 0;
		_action->start();
	}
}

void Actor::setAction(Action *newAction, bool loadingSave) {
	if (loadingSave) {
		_isActionEnded = 1;
		_action = newAction;
	} else {
		setAction(newAction);
	}
}

InventoryMgr *Actor::getInventoryMgr() const {
	return _page->getModule()->getInventoryMgr();
}

Common::String Actor::getPDALink() const {
	return Common::String();
}

} // End of namespace Pink
