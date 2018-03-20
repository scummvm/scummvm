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

#include "actor.h"
#include "../page.h"
#include "lead_actor.h"
#include "../actions/action.h"

namespace Pink {

void Actor::deserialize(Archive &archive) {
    NamedObject::deserialize(archive);
    _page = static_cast<GamePage*>(archive.readObject());
    if (dynamic_cast<LeadActor*>(this))
        debug("LeadActor: _name = %s", _name.c_str());
    else debug("Actor: _name = %s", _name.c_str());
    archive >> _actions;
}

Sequencer *Actor::getSequencer() {
    return _page->getSequencer();
}

Action *Actor::findAction(const Common::String &name) {
    return *Common::find_if(_actions.begin(), _actions.end(), [&name]
            (Action* action) {
        return name == action->getName();
    });;
}

GamePage *Actor::getPage() const {
    return _page;
}

void Actor::init(bool unk) {
    if (!_action) {
        _action = findAction({"Idle"});
    }

    if (!_action) {
        _isActionEnd = 1;
    }
    else {
        _isActionEnd = 0;
        _action->play(unk);
    }
}

void Actor::hide() {
    setAction({"Hide"});
}

void Actor::endAction() {
    _isActionEnd = 1;
}

void Actor::setAction(const Common::String &name) {
    Action *newAction = findAction(name);
    if (_action) {
        _isActionEnd = 1;
        _action->end();
    }
    if (newAction) {
        _isActionEnd = 0;
        _action = newAction;
        _action->play(0);
    }
}

} // End of namespace Pink
