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
#include "pink/pink.h"
#include "pink/objects/actions/action_hide.h"
#include "pink/objects/actors/actor.h"

namespace Pink {

void ActionHide::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\tActionHide: _name = %s", _name.c_str());
}

void ActionHide::start() {
	debugC(6, kPinkDebugActions, "Actor %s has now ActionHide %s", _actor->getName().c_str(), _name.c_str());
	_actor->endAction();
}

void ActionHide::end() {
	debugC(6, kPinkDebugActions, "ActionHide %s of Actor %s is ended", _name.c_str(), _actor->getName().c_str());
}

} //End of namespace Pink
