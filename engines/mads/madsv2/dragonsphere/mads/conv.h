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

#ifndef MADS_DRAGONSPHERE_MADS_CONV_H
#define MADS_DRAGONSPHERE_MADS_CONV_H

#include "common/scummsys.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {

// Conversation node IDs are added here as each conversation is ported.
// Each conversation gets its own enum block named after its .CON file.

enum {
	conv002_counter_only = 0,
	conv002_banter_random = 1,
	conv002_flirt_rand = 2
};

enum {
	conv003_replies_b_b = 2,
	conv003_exit_b_b = 8,
	conv003_replies_defeat = 5
};

enum {
	conv004_seen_only = 0,
	conv004_resolved_only = 1,
	conv004_am_only = 2,
	conv004_wait_only = 3,
	conv004_thanks_only = 4,
	conv004_nay_only = 5,
	conv004_queen_only = 6,
	conv004_fathers_only = 7,
	conv004_last_only = 8,
	conv004_exit_b_b = 10
};

enum {
	conv034_five_b_b = 4,
	conv034_seven_only = 7,
	conv034_eight_b_b = 9,
	conv034_eight_only = 10,
	conv034_nine_only = 11,
	conv034_final_only = 15,
	conv034_exit_a_a = 16,
	conv034_exit_b_b = 17
};

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS

#endif
