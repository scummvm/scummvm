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
	conv005_exit_b_b = 16
};

enum {
	conv006_wares_greeting   = 0,
	conv006_choices_chat     = 2,
	conv006_choices_sorry    = 3,
	conv006_exit_b_b         = 5,
	conv006_how_only         = 6,
	conv006_thanks_only      = 7,
	conv006_wise_only        = 8,
};

enum {
	conv008_query_wish       =  3,
	conv008_query_respect    =  4,
	conv008_query_norespect  =  5,
	conv008_query_humans     =  6,
	conv008_query_why_annoy  =  7,
	conv008_query_king       =  8,
	conv008_query_where      =  9,
	conv008_banter_only      = 11
};

enum {
	conv009_hello1a_only     = 1,
	conv009_hello2a_only     = 3,
	conv009_choices_trip     = 4,
	conv009_choices_never    = 5,
	conv009_choices_someday  = 6,
	conv009_exiter_leave     = 8
};

enum {
	conv010_hold_only  =  3,
	conv010_leave_b_b  =  6,
	conv010_fix_b_b    = 22,
	conv010_die_b_b    = 31
};

enum {
	conv012_gash_b_b   = 14
};

enum {
	conv014_heehee_only      = 0,
	conv014_query_wish       = 3,
	conv014_query_respect    = 4,
	conv014_query_humans     = 6,
	conv014_query_why_annoy  = 7,
	conv014_banter_only      = 8
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
