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

#include <engines/pink/constants.h>
#include "actor.h"
#include "engines/pink/objects/pages/game_page.h"
#include "lead_actor.h"
#include "engines/pink/objects/actions/action.h"
#include "pink/cursor_mgr.h"

namespace Pink {

void Actor::deserialize(Archive &archive) {
    NamedObject::deserialize(archive);
    _page = static_cast<GamePage*>(archive.readObject());
    archive >> _actions;
}

void Actor::toConsole() {
    debug("Actor: _name = %s", _name.c_str());
    for (int i = 0; i < _actions.size(); ++i) {
        _actions[i]->toConsole();
    }
}

Sequencer *Actor::getSequencer() const {
    return _page->getSequencer();
}

Action *Actor::findAction(const Common::String &name) {
    Action ** action = Common::find_if(_actions.begin(), _actions.end(), [&name]
            (Action* action) {
        return name == action->getName();
    });;
    if (!action)
        return nullptr;
    return *action;
}

GamePage *Actor::getPage() const {
    return _page;
}

void Actor::init(bool unk) {
    if (!_action) {
        _action = findAction({"Idle"});
    }

    if (!_action) {
        _isActionEnded = 1;
    }
    else {
        _isActionEnded = 0;
        _action->start(unk);
    }
}

void Actor::hide() {
    setAction({"Hide"});
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
    if (newAction) {
        _isActionEnded = 0;
        _action = newAction;
        _action->start(0);
    }
}

void Actor::setAction(Action *newAction, bool unk) {
    if (unk){
        assert(0); // want to see this
        _isActionEnded = 1;
        _action = newAction;
    }
    else setAction(newAction);
}

Action *Actor::getAction() const {
    return _action;
}

bool Actor::isPlaying() {
    return !_isActionEnded;
}

bool Actor::initPallete(Director *director) {
    for (int i = 0; i < _actions.size(); ++i) {
        if (_actions[i]->initPalette(director))
            return true;
    }
    return false;
}

void Actor::onMouseOver(Common::Point point, CursorMgr *mgr) {
    mgr->setCursor(kDefaultCursor, point);
}

Actor::~Actor() {
    for (int i = 0; i < _actions.size(); ++i) {
        delete _actions[i];
    }
}

} // End of namespace Pink
