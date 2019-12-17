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
#include "ultima/ultima8/convert/u8/convert_shape_u8.h"

namespace Ultima {
namespace Ultima8 {

// Shape format configuration for Ultima8
const ConvertShapeFormat U8ShapeFormat = {
	"Ultima8",
	6,		// header
	"",		// ident
	0,		// bytes_ident 
	0,		// bytes_special
	4,		// header_unk
	2,		// num_frames

	6,		// frameheader
	3,		// frame_offset
	1,		// frameheader_unk
	2,		// frame_length
	0,		// frame_length_kludge

	18,		// frameheader2
	8,		// frame_unknown
	2,		// frame_compression
	2,		// frame_width
	2,		// frame_height
	2,		// frame_xoff
	2,		// frame_yoff

	2,		// line_offset
	0		// line_offset_absolute
};

// Shape format configuration for Ultima8 2D interface components
const ConvertShapeFormat U82DShapeFormat = {
	"Ultima8 2D",
	6,		// header
	"",		// ident
	0,		// bytes_ident 
	0,		// bytes_special
	4,		// header_unk
	2,		// num_frames

	6,		// frameheader
	3,		// frame_offset
	1,		// frameheader_unk
	2,		// frame_length
	8,		// frame_length_kludge

	18,		// frameheader2
	8,		// frame_unknown
	2,		// frame_compression
	2,		// frame_width
	2,		// frame_height
	2,		// frame_xoff
	2,		// frame_yoff

	2,		// line_offset
	0		// line_offset_absolute
};

// Shape format configuration for Ultima8 SKF
const ConvertShapeFormat U8SKFShapeFormat = {
	"Ultima8 SKF",
	2,		// header
	"\2",	// ident
	2,		// bytes_ident 
	0,		// bytes_special
	0,		// header_unk
	0,		// num_frames

	0,		// frameheader
	0,		// frame_offset
	0,		// frameheader_unk
	0,		// frame_length
	0,		// frame_length_kludge

	10,		// frameheader2
	0,		// frame_unknown
	2,		// frame_compression
	2,		// frame_width
	2,		// frame_height
	2,		// frame_xoff
	2,		// frame_yoff

	2,		// line_offset
	0		// line_offset_absolute
};

// Shape format configuration for Compressed Ultima 8 shapes
const ConvertShapeFormat U8CMPShapeFormat = {
	"Ultima8 CMP",
	11,		// header
	"",		// ident
	0,		// bytes_ident 
	5,		// bytes_special
	4,		// header_unk
	2,		// num_frames

	6,		// frameheader
	4,		// frame_offset
	2,		// frameheader_unk
	0,//2,		// frame_length
	0,//-16,	// frame_length_kludge

	10,		// frameheader2
	0,		// frame_unknown
	2,		// frame_compression
	2,		// frame_width
	2,		// frame_height
	2,		// frame_xoff
	2,		// frame_yoff

	0,		// line_offset
	0		// line_offset_absolute
};

} // End of namespace Ultima8
} // End of namespace Ultima
