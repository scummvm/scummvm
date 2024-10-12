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

#ifndef SCUMM_MACGUI_COLORS_H

namespace Scumm {

// Colors used by the 16 color Mac games. Nothing that is ever drawn by
// 256 color games should use these!

#define kBlack         0
#define	kBlue          1
#define	kGreen         2
#define kCyan          3
#define kRed           4
#define kMagenta       5
#define kBrown         6
#define kLightGray     7
#define kDarkGray      8
#define kBrightBlue    9
#define kBrightGreen   10
#define kBrightCyan    11
#define kBrightRed     12
#define kBrightMagenta 13
#define kBrightYellow  14
#define kWhite         15

// Gray or checkerboard
#define kBackground    254

#define kTransparency  255

} // End of namespace Scumm

#endif
