/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PSX_PXACTOR_H
#define ICB_PSX_PXACTOR_H

#include "engines/icb/gfx/psx_pcgpu.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/shadow.h"

namespace ICB {

typedef struct psxActor {
	// The true position in x,y,z space i.e. minus the render correction
	PXvector_PSX truePos;
	// The true rotation i.e. minus the render correction (pan_adjust)
	SVECTOR trueRot;

	// The actors render : local to world matrix
	MATRIX lw;
	SVECTOR rot;
	SVECTOR pos;
	int32 style;

	// Screen position
	SVECTOR sPos;

	// Bounding box on the screen
	SVECTOR bboxScrn[8];

	// Min & max corners of the bounding box
	SVECTOR minBbox;
	SVECTOR maxBbox;

	// Shadow bounding boxes
	SVECTOR shadowBox[MAX_SHADOWS][8];
	SVECTOR shadowMinBox[MAX_SHADOWS];
	SVECTOR shadowMaxBox[MAX_SHADOWS];
	uint32 nShadows;

} psxActor;

} // End of namespace ICB

#endif // #ifndef PSX_PXACTOR_H
