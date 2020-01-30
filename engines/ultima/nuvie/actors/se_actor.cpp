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

#include "ultima/nuvie/actors/se_actor.h"

namespace Ultima {
namespace Nuvie {

SEActor::SEActor(Map *m, ObjManager *om, GameClock *c): WOUActor(m, om, c) {
}

SEActor::~SEActor() {
}

bool SEActor::init(uint8) {
	Actor::init();
	return true;
}

bool SEActor::will_not_talk() {
	if (worktype == 0x07 || worktype == 0x8 || worktype == 0x9b)
		return true;
	return false;
}

} // End of namespace Nuvie
} // End of namespace Ultima
