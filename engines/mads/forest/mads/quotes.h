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

#ifndef MADS_FOREST_MADS_QUOTES_H
#define MADS_FOREST_MADS_QUOTES_H

#include "common/scummsys.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

enum {
	quote_menu_done          = 1,
	quote_menu_cancel        = 2,
	quote_menu_save          = 3,
	quote_menu_restore       = 4,
	quote_menu_clear         = 5,
	quote_menu_ok            = 8,
	quote_menu_empty         = 9,
	quote_menu_unnamed       = 10,
	quote_main_save          = 12,
	quote_main_restore       = 13,
	quote_main_resume        = 17,
	quote_main_exit          = 18,
	quote_options_music      = 20,
	quote_options_fade       = 23,
	quote_options_music_on   = 26,
	quote_options_music_off  = 27,
	quote_options_fade_1     = 32,
	quote_options_fade_2     = 33,
	quote_options_fade_3     = 34,
	quote_save_title         = 47,
	quote_restore_title      = 48,
	quote_alert_save_ok      = 52,
	quote_alert_restore_ok   = 53,
	quote_alert_save_fail    = 54,
	quote_alert_restore_fail = 55
};

} // namespace Forest
} // namespace MADSV2
} // namespace MADS

#endif
