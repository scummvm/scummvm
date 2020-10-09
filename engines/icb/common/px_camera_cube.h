/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PX_CAMERA_CUBE
#define ICB_PX_CAMERA_CUBE

#include "engines/icb/common/px_common.h"

namespace ICB {

typedef struct {
	/*
	           (y)height
	          |
	          |
	          |
	          |
	          |
	 x1,y1,z1 |----------|                    y
	           \          \                   |
	            \          \                  |
	             \___________ (x)width          \---x
	              (z)depth                           \      parallel to max model axis
	                                                            z
	*/

	PXreal x1;
	PXreal y1;
	PXreal z1; // top left bottom corner  point
	PXreal width;
	PXreal depth;
	PXreal height;
	uint32 script_name_offset; // IF        this is set then we call a script
	uint32 camera_name_offset; // ELSE  offset from start of this file to ascii
	//          name of camera - and therefore set
} _camera_cube;

} // End of namespace ICB

#endif
