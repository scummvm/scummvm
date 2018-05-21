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

#include "pink/archive.h"
#include "pink/objects/actors/pda_button_actor.h"

namespace Pink {

void PDAButtonActor::deserialize(Archive &archive) {
    Actor::deserialize(archive);
    _hideOnStop = (bool) archive.readDWORD();
    _opaque = (bool) archive.readDWORD();

    int comm = archive.readDWORD();
    assert(comm <= 4);
    _command = (Command) comm;
}

void PDAButtonActor::toConsole() {
    debug("PDAButtonActor: _name = %s, _x = %u _y = %u _hideOnStop = %u, _opaque = %u, _command = %u",
          _name.c_str(), _x, _y, _hideOnStop, _opaque, (int) _command);
}

} // End of namespace Pink