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

#ifndef NUVIE_ACTORS_WOU_ACTOR_H
#define NUVIE_ACTORS_WOU_ACTOR_H

#include "ultima/nuvie/actors/actor.h"

namespace Ultima {
namespace Nuvie {

class WOUActor: public Actor {
protected:

public:

	WOUActor(Map *m, ObjManager *om, GameClock *c) : Actor(m, om, c) { }
	~WOUActor() override { }

	bool can_carry_object(uint16 obj_n, uint32 qty = 0) override;
	bool can_carry_object(Obj *obj) override;

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
