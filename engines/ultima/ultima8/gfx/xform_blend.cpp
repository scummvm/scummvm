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

#include "ultima/ultima8/gfx/xform_blend.h"

namespace Ultima {
namespace Ultima8 {

const uint8 U8XFormPal[1024] = {
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	48, 48, 48, 80, // (green->dark grey)
	24, 24, 24, 128, // (*->vdark grey)
	64, 64, 24, 64, // (yellow)
	80, 80, 80, 80, // (white->grey)
	180, 90, 0,  80, // (red->orange)
	0,  0,  252, 40, // (blue)
	0,  0,  104, 40, // (blue)
	0,  0,  0,  0
};

// TODO: Only eyeballed these, should try and make it more exact to original.
const uint8 CruXFormPal[1024] = {
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	48, 48, 48, 80, // (green->dark grey)
	24, 24, 24, 128, // (*->vdark grey)
	64, 64, 24, 64, // (yellow)
	80, 80, 80, 80,
	48, 48, 48, 140, // (*->grey)
	24, 24, 24, 140, // (*->dark grey) 13
	10, 10, 10, 140, // (*->vdark grey) 14
	0,  0,  0,  0
};

} // End of namespace Ultima8
} // End of namespace Ultima
