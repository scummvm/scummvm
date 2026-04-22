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

#ifndef MADS_PHANTOM_MADS_QUOTES_H
#define MADS_PHANTOM_MADS_QUOTES_H

#include "common/scummsys.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

enum {
	quote_menu_done				=  1,
	quote_menu_cancel			=  2,
	quote_menu_save				=  3,
	quote_menu_restore			=  4,
	quote_menu_clear			=  5,
	quote_menu_yes				=  6,
	quote_menu_no				=  7,
	quote_menu_ok				=  8,
	quote_menu_empty			=  9,
	quote_menu_unnamed			= 10,
	quote_main_title			= 11,
	quote_main_item1			= 12,
	quote_main_item2			= 13,
	quote_main_item3			= 15,
	quote_main_item4			= 16,
	quote_main_item5			= 17,
	quote_main_item6			= 14,
	quote_options_title			= 18,
	quote_options_item1			= 19,
	quote_options_item2			= 20,
	quote_options_item3			= 21,
	quote_options_item4			= 22,
	quote_options_item5			= 23,
	quote_options_item6			= 24,
	quote_options_item1a		= 25,
	quote_options_item1b		= 26,
	quote_options_item2a		= 27,
	quote_options_item2b		= 28,
	quote_options_item3a		= 29,
	quote_options_item3b		= 30,
	quote_options_item4a		= 31,
	quote_options_item4b		= 32,
	quote_options_item4c		= 33,
	quote_options_item5a		= 34,
	quote_options_item5b		= 35,
	quote_options_item5c		= 36,
	quote_options_item6a		= 37,
	quote_options_item6b		= 38,
	quote_save_title			= 39,
	quote_restore_title			= 40,
	quote_difficulty_title		= 41,
	quote_difficulty_item1		= 42,
	quote_difficulty_item2		= 43,
	quote_save_successful		= 44,
	quote_restore_successful	= 45,
	quote_save_failed			= 46,
	quote_restore_failed		= 47,
	quote_emergency_save_1a		= 48,
	quote_emergency_save_1b		= 49,
	quote_emergency_save_success = 50,
	quote_emergency_save_failure = 51,
	quote_emergency_save_attempt = 52,
	quote_emergency_save_resume = 53,
	quote_score_1				= 54,
	quote_score_2				= 55,
	quote_score_3				= 56,
	quote_score_4				= 57,
	quote_score_rank_1			= 58,
	quote_score_rank_2			= 59,
	quote_score_rank_3			= 60,
	quote_score_rank_4			= 61,
	quote_score_rank_5			= 62,
	quote_score_rank_6			= 63,
	quote_score_rank_7			= 64,
	quote_score_rank_8			= 65,
	quote_mainmenu_phantom_1	= 66,
	quote_305a0					= 100,
	quote_305a1					= 101,
	quote_310a0					= 102,
	quote_310a1					= 103,
	quote_310a2					= 104,
	quote_310b0					= 105,
	quote_310b1					= 106,
	quote_310b2					= 107,
	quote_310c0					= 108,
	quote_310c1					= 109,
	quote_310d0					= 110,
	quote_310d1					= 111,
	quote_310d2					= 112,
	quote_310e0					= 113,
	quote_310e1					= 114,
	quote_310e2					= 115,
	quote_310f0					= 116,
	quote_204a0					= 117
};

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
