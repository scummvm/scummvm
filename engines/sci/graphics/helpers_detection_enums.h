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

#ifndef SCI_HELPERS_DETECTION_ENUMS_H
#define SCI_HELPERS_DETECTION_ENUMS_H

namespace Sci {
// Game view types, sorted by the number of colors
enum ViewType {
	kViewUnknown,   // uninitialized, or non-SCI
	kViewEga,       // EGA SCI0/SCI1 and Amiga SCI0/SCI1 ECS 16 colors
	kViewAmiga,     // Amiga SCI1 ECS 32 colors
	kViewAmiga64,   // Amiga SCI1 AGA 64 colors (i.e. Longbow)
	kViewVga,       // VGA SCI1 256 colors
	kViewVga11      // VGA SCI1.1 and newer 256 colors
};

} // End of namespace Sci

#endif // SCI_HELPERS_DETECTION_ENUMS_H
