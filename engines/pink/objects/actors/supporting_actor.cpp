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

#include "supporting_actor.h"
#include <engines/pink/archive.h>
#include <engines/pink/objects/actions/action.h>
#include <common/debug.h>

namespace Pink {

void SupportingActor::deserialize(Archive &archive) {
    Actor::deserialize(archive);
    archive >> _location >> _pdaLink >> _cursor;
    _handlerMgr.deserialize(archive);
}

void SupportingActor::toConsole() {
    debug("SupportingActor: _name = %s, _location=%s, _pdaLink=%s, _cursor=%s", _name.c_str());
    for (int i = 0; i < _actions.size(); ++i) {
        _actions[i]->toConsole();
    }
    _handlerMgr.toConsole();
}

} // End of namespace Pink