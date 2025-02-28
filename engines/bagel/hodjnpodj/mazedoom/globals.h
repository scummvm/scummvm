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

#ifndef HODJNPODJ_MAZEDOOM_GLOBALS_H
#define HODJNPODJ_MAZEDOOM_GLOBALS_H

#include "common/scummsys.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

#define IDOK 10001
#define IDCANCEL 10002

// Scroll button size and positioning information
#define IDC_SCROLL			850

#define SCROLL_BUTTON_X		250
#define SCROLL_BUTTON_Y		0
#define SCROLL_BUTTON_DX	140
#define SCROLL_BUTTON_DY	23

#define SCROLLUP_BMP	"ART/SCROLLUP.BMP"
#define SCROLLDOWN_BMP	"ART/SCROLLDN.BMP"

#define IDB_HODJ_UP		"ART/HODJUP.BMP"
#define IDB_HODJ_RIGHT	"ART/HODJRT.BMP"
#define IDB_HODJ_DOWN	"ART/HODJDOWN.BMP"
#define IDB_HODJ_LEFT	"ART/HODJLF.BMP"
#define IDB_PODJ_UP		"ART/PODJUP.BMP"
#define IDB_PODJ_RIGHT	"ART/PODJRT.BMP"
#define IDB_PODJ_DOWN	"ART/PODJDOWN.BMP"
#define IDB_PODJ_LEFT	"ART/PODJLF.BMP"

#define IDB_PARTS       "ART/MODPARTS.BMP"
#define IDB_LOCALE_BMP	"ART/LOCALE.BMP"
#define IDB_BLANK_BMP	"ART/BLANK.BMP"

#define IDC_RULES						1002
#define IDC_DIFFICULTY                  1021
#define IDC_TIMELIMIT                   1022

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
