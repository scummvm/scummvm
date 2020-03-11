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

#ifndef ULTIMA4_TEXTCOLOR_H
#define ULTIMA4_TEXTCOLOR_H

namespace Ultima {
namespace Ultima4 {

#define TEXT_BG_INDEX 0
#define TEXT_FG_PRIMARY_INDEX 15
#define TEXT_FG_SECONDARY_INDEX 7
#define TEXT_FG_SHADOW_INDEX 80


//
// text foreground colors
//
typedef enum {
    FG_GREY   = '\023',
    FG_BLUE   = '\024',
    FG_PURPLE = '\025',
    FG_GREEN  = '\026',
    FG_RED    = '\027',
    FG_YELLOW = '\030',
    FG_WHITE  = '\031'
} ColorFG;

//
// text background colors
//
typedef enum {
    BG_NORMAL = '\032',
    BG_BRIGHT = '\033'
} ColorBG;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
