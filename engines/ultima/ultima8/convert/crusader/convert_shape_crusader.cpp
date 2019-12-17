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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/convert/crusader/convert_shape_crusader.h"

namespace Ultima {
namespace Ultima8 {

// Shape format configuration for Crusader
const ConvertShapeFormat CrusaderShapeFormat = {
	"Crusader",
	6,		// header
	"",		// ident
	0,		// bytes_ident 
	0,		// bytes_special
	4,		// header_unk
	2,		// num_frames

	8,		// frameheader
	3,		// frame_offset
	1,		// frameheader_unk
	4,		// frame_length
	0,		// frame_length_kludge

	28,		// frameheader2        20 for mouse/gumps
	8,		// frame_unknown       0  for mouse/gumps
	4,		// frame_compression
	4,		// frame_width
	4,		// frame_height
	4,		// frame_xoff
	4,		// frame_yoff

	4,		// line_offset
	0		// line_offset_absolute
};

// Shape format configuration for 2D Crusader Shapes
const ConvertShapeFormat Crusader2DShapeFormat = {
	"Crusader 2D",
	6,		// header
	"",		// ident
	0,		// bytes_ident 
	0,		// bytes_special
	4,		// header_unk
	2,		// num_frames

	8,		// frameheader
	3,		// frame_offset
	1,		// frameheader_unk
	4,		// frame_length
	0,		// frame_length_kludge

	20,		// frameheader2        20 for mouse/gumps
	0,		// frame_unknown       0  for mouse/gumps
	4,		// frame_compression
	4,		// frame_width
	4,		// frame_height
	4,		// frame_xoff
	4,		// frame_yoff

	4,		// line_offset
	0		// line_offset_absolute
};

} // End of namespace Ultima8
} // End of namespace Ultima
