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
#include "pink/objects/actions/action.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

void Actor::deserialize(Archive &archive) {
	NamedObject::deserialize(archive);
	_page = static_cast<Page*>(archive.readObject());
	_actions.deserialize(archive);
}

void Actor::toConsole() {
	debug("Actor: _name = %s", _name.c_str());
	for (uint i = 0; i < _actions.size(); ++i) {
		_actions[i]->toConsole();
	}
}

Sequencer *Actor::getSequencer() const {
	return _page->getSequencer();
}

Action *Actor::findAction(const Common::String &name) {
	for (uint i = 0; i < _actions.size(); ++i) {
		if (_actions[i]->getName() == name)
			return _actions[i];
	}
	return nullptr;
}

Page *Actor::getPage() const {
	return _page;
}

void Actor::init(bool unk) {
	if (!_action)
		_action = findAction(kIdleAction);

	if (!_action) {
		_isActionEnded = 1;
	} else {
		_isActionEnded = 0;
		_action->start(unk);
	}
}

void Actor::hide() {
	setAction(kHideAction);
}

void Actor::endAction() {
	_isActionEnded = 1;
}

void Actor::setAction(const Common::String &name) {
	Action *newAction = findAction(name);
	setAction(newAction);
}

void Actor::setAction(Action *newAction) {
	if (_action) {
		_isActionEnded = 1;
		_action->end();
	}
	_action = newAction;
	if (newAction) {
		_isActionEnded = 0;
		_action->start(0);
	}
}

void Actor::setAction(Action *newAction, bool unk) {
	if (unk) {
		//assert(0); // want to see this
		_isActionEnded = 1;
		_action = newAction;
	} else {
		setAction(newAction);
	}
}

Action *Actor::getAction() const {
	return _action;
}

bool Actor::isPlaying() {
	return !_isActionEnded;
}

bool Actor::initPallete(Director *director) {
	for (uint i = 0; i < _actions.size(); ++i) {
		if (_actions[i]->initPalette(director))
			return true;
	}
	return false;
}

void Actor::onMouseOver(Common::Point point, CursorMgr *mgr) {
	mgr->setCursor(kDefaultCursor, point, Common::String());
}

Actor::~Actor() {
	for (uint i = 0; i < _actions.size(); ++i) {
		delete _actions[i];
	}
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

void Actor::pause() {
	if (_action)
		_action->pause();
}

void Actor::unpause() {
	if (_action)
		_action->unpause();
}

void Actor::onHover(Common::Point point, const Common::String &itemName, CursorMgr *cursorMgr) {
	cursorMgr->setCursor(kHoldingItemCursor, point, itemName);
}

void Actor::setPage(Page *page) {
	_page = page;
}

} // End of namespace Pink
