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

#ifndef ICB_PX_CAMERA_H
#define ICB_PX_CAMERA_H

#include "engines/icb/common/px_common.h"

namespace ICB {

typedef struct PXmatrix_PC {
	PXreal m[3][3]; /* 3x3 rotation matrix */
	PXreal t[3];    /* transfer vector */
} PXmatrix_PC;

struct PCcamera {
	char id[4];
	int schema;
	PXmatrix_PC view;
	float pan;
	float focLen;
};

} // End of namespace ICB

#endif // __px_camera_h__included__
