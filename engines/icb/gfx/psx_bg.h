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

#ifndef PSX_BG_H
#define PSX_BG_H

#include "engines/icb/gfx/psx_zlayers.h"

namespace ICB {

#define PSX_ZTRANSP 0

#ifdef _PSX

// holds the primitives for drawing the background set
// e.g. background image, any lines (e.g. barriers),
// bounding box cuboid for currently selected actor
// Each primitive has to have 2 copies : 1 for each draw buffer
typedef struct psxBGinfo {
	TSPRT bgImg[2][2];
	u_int nBGimgSprites;

	u_int nBGzfragments; // the current number being used
	u_short CLUTid;
	u_short bgTP[2];
	u_short x0, y0; // where in VRAM the origin of the image is stored
} psxBGinfo;

#endif

} // End of namespace ICB

#endif // #ifndef PSX_BG_H
