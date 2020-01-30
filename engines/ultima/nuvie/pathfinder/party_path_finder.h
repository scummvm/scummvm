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

#ifndef NUVIE_PATHFINDER_PARTY_PATH_FINDER_H
#define NUVIE_PATHFINDER_PARTY_PATH_FINDER_H

#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/map.h"

namespace Ultima {
namespace Nuvie {

/* PartyPathFinder moves the entire party at once.
 * FIXME: Move to target if one square away and unblocked. Shamino isn't in the
 * correct square after moving through a doorway.
 * FIXME: Perhaps is_contiguous() should require everyone in front of a follower
 * to also be contiguous. If more than one followers are lost, they stay
 * together more than look for the leader.
 * FIXME: When walking along the wall, the last follower doesn't move up when
 * there is a closer free square. (it sometimes behaves the same way in U6)
 * FIXME: If a higher-number follower is closer to the leader than a lower-numbered
 * one, the one with higher priority bumps him out of the way, and he loses a
 * move. This causes him to become non-contiguous. Followers should NEVER be
 * moved to non-contiguous squares. Strangely, this only happens when moving in
 * certain directions. (disable SEEK mode to check)
 * FIXME: When changing directions, followers on opposite sides of the party
 * (perpendicular to the forward direction) shouldn't exchange positions until
 * the leader stops.
 */

#define AVOID_DAMAGE_TILES true

class PartyPathFinder {
	Party *party; // friend
public:
	PartyPathFinder(Party *p);
	~PartyPathFinder();

	bool follow_passA(uint32 p); // returns true if party member p moved
	bool follow_passB(uint32 p);
	void seek_leader(uint32 p);
	void end_seek(uint32 p);

	bool move_member(uint32 member_num, sint16 relx, sint16 rely, bool ignore_position = false, bool can_bump = true, bool avoid_danger_tiles = true);
	bool bump_member(uint32 bumped_member_num, uint32 member_num);

	bool is_seeking(uint32 member_num) {
		return (get_member(member_num).actor->get_pathfinder() != 0);
	}
	bool is_contiguous(uint32 member_num, MapCoord from);
	bool is_contiguous(uint32 member_num);
	bool is_behind_target(uint32 member_num);
	bool is_at_target(uint32 p);
	void get_target_dir(uint32 p, sint8 &rel_x, sint8 &rel_y);
	void get_forward_dir(sint8 &vec_x, sint8 &vec_y);
	void get_last_move(sint8 &vec_x, sint8 &vec_y);

protected:
	bool try_moving_to_leader(uint32 p, bool ignore_position);
	bool try_moving_forward(uint32 p);
	bool try_moving_to_target(uint32 p, bool avoid_damage_tiles = false);
	bool try_all_directions(uint32 p, MapCoord target_loc);
	bool try_moving_sideways(uint32 p);

	bool leader_moved_away(uint32 p);
	bool leader_moved_diagonally();
	bool leader_moved();

	Std::vector<MapCoord> get_neighbor_tiles(MapCoord &center, MapCoord &target);

	// use party
	struct PartyMember get_member(uint32 p) {
		return (party->member[p]);
	}
	sint8 get_leader() {
		return (party->get_leader());
	}
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
