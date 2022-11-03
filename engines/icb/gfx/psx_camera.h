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

#ifndef ICB_PSX_CAMERA_H
#define ICB_PSX_CAMERA_H

#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/common/px_common.h"

namespace ICB {

#define PSX_CAMERA_ID "CAM"
#define PSX_CAMERA_SCHEMA 2

#define PSX_WEATHER_NONE 0
#define PSX_WEATHER_SNOW 1
#define PSX_WEATHER_RAIN 2

typedef struct {
	MATRIX view;
	uint16 focLen;
} psxPCcameraOld;

typedef struct {
	MATRIX view;
	uint16 focLen;
	int32 pos[3];
} psxPCcamera;

typedef struct {
	float m[3][3];
	float t[3];
	float fov;
	int32 width;
	int32 height;
} psxFloatCamera;

typedef struct {
	char id[4];
	int32 schema;
	MATRIX view;
	int32 pan;
	uint16 focLen;
	int32 weatherType;
	int32 lightning;
	int32 windx;
	int32 windy;
	int32 windz;
	int32 particleCount;
} psxCamera;

// Useful camera and co-ordinate system based maths

void makeLSmatrix(MATRIX *ws, MATRIX *lw, MATRIX *ls);

void psxWorldToFilm(const PXvector_PSX &worldpos, const psxCamera &camera, bool8 &is_onfilm, PXvector_PSX &filmpos);

} // End of namespace ICB

#endif // #ifndef PSX_CAMERA_H
