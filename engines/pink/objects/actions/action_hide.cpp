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

#include "action_hide.h"
#include "engines/pink/objects/actors/actor.h"
#include <engines/pink/archive.h>
#include <common/debug.h>


namespace Pink {

void Pink::ActionHide::deserialize(Archive &archive) {
    Action::deserialize(archive);
}

void ActionHide::play(bool unk_startNow) {
    debug("ActionHide %s is now in playing state", _name.c_str());
    _actor->endAction();
}

void ActionHide::end() {
    debug("ActionHide %s is ended", _name.c_str());
}

void ActionHide::toConsole() {
    debug("\tActionHide: _name = %s", _name.c_str());
}

} //End of namespace Pink