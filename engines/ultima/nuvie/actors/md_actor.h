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

#ifndef NUVIE_ACTORS_MD_ACTOR_H
#define NUVIE_ACTORS_MD_ACTOR_H

#include "ultima/nuvie/actors/wou_actor.h"

namespace Ultima {
namespace Nuvie {

class MDActor: public WOUActor {
protected:

public:

	MDActor(Map *m, ObjManager *om, GameClock *c);
	~MDActor() override;

	bool init(uint8 unused = 0) override;
	bool will_not_talk() override;
	uint8 get_maxhp() override {
		return (((level * 24 + strength * 2) < 255) ? (level * 24 + strength * 2) : 255);
	}
	uint8 get_hp_text_color() override;
	uint8 get_str_text_color() override;
	uint8 get_dex_text_color() override;
	bool is_immobile() override;

	bool check_move(uint16 new_x, uint16 new_y, uint8 new_z, ActorMoveFlags flags = 0) override;
	uint16 get_downward_facing_tile_num() override;
	void set_direction(uint8 d) override;
	bool is_passable() override;

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
