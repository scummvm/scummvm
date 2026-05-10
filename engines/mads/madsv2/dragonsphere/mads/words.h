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

#ifndef MADS_DRAGONSPHERE_MADS_WORDS_H
#define MADS_DRAGONSPHERE_MADS_WORDS_H

#include "common/scummsys.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {

// Hardcoded verb IDs (VOCABH.DB, IDs 1-13)
enum {
	words_look      =  3,
	words_take      =  4,
	words_push      =  5,
	words_open      =  6,
	words_talk_to   =  8,
	words_pull      = 10,
	words_walk_to   = 13
};

// Vocabulary word IDs (verified against release disassembly)
enum {
	words_floor              =  16,
	words_wall               =  20,
	words_window             =  24,
	words_tapestry           =  26,
	words_look_at            =  30,
	words_walk_through       =  37,
	words_goblet             =  59,
	words_bone               =  62,
	words_brazier            = 175,
	words_door_to_throne_room = 176,
	words_door_to_hallway    = 197,
	words_ceiling            = 196,
	words_chair              = 202,
	words_dividing_wall      = 208,
	words_bucket             = 225,
	words_sconce             = 329,
	words_dining_table       = 178,
	words_scullery_maid      = 266
};

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS

#endif
