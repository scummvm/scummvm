/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/adv_r/adv_game.h"

namespace M4 {

void GameControl::syncGame(Common::Serializer &s) {
	// TODO: Ensure this matches the original structure
	for (uint i = 0; i < KERNEL_SCRATCH_SIZE; ++i)
		s.syncAsUint32LE(scratch[i]);

	s.syncAsSint16LE(room_id);
	s.syncAsSint16LE(new_room);
	s.syncAsSint16LE(previous_section);
	s.syncAsSint16LE(section_id);
	s.syncAsSint16LE(new_section);
	s.syncAsSint16LE(previous_room);

	s.syncAsUint32LE(digi_overall_volume_percent);
	s.syncAsUint32LE(midi_overall_volume_percent);
	s.syncAsByte(camera_pan_instant);
	s.syncAsByte(going);
}

} // End of namespace M4
