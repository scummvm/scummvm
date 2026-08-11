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

#ifndef BAGEL_HODJNPODJ_GLOBALS_H
#define BAGEL_HODJNPODJ_GLOBALS_H

#include "bagel/boflib/palette.h"

namespace Bagel {
namespace HodjNPodj {

/*
 * Hodj 'n' Podj .INI file name
 */
#define INI_FILENAME    "HODJPODJ.INI"

#define PATHSPECSIZE                256

/*
* Main Window positioning constants
*/
#define GAME_WIDTH                  640
#define GAME_HEIGHT                 480
#define GAME_TOP_BORDER_WIDTH        24
#define GAME_BOTTOM_BORDER_WIDTH     16
#define GAME_LEFT_BORDER_WIDTH       16
#define GAME_RIGHT_BORDER_WIDTH      16

// Scroll button size and positioning information
#define SCROLL_BUTTON_X     250
#define SCROLL_BUTTON_Y     0
#define SCROLL_BUTTON_DX    140
#define SCROLL_BUTTON_DY    23
#define SCROLLUP_BMP        "ART/SCROLLUP.BMP"
#define SCROLLDOWN_BMP      "ART/SCROLLDN.BMP"
#define SCROLLUP            461
#define SCROLLDOWN          462

// New Game button area
#define NEWGAME_LOCATION_X   15
#define NEWGAME_LOCATION_Y    0
#define NEWGAME_WIDTH       217
#define NEWGAME_HEIGHT       20

} // namespace HodjNPodj
} // namespace Bagel

#endif
