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

#ifndef ICB_LIGHT_H
#define ICB_LIGHT_H

#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/gfx/rlp_api.h"

namespace ICB {

// Used to pass the information from stage to actor draw
// about which lights are in teh current camera view
// also passes the state of each light

#define MAX_NUMBER_LIGHTS 32
#define MAX_NUMBER_SHADES 4

#define DEFAULT_INSHADE_PERCENTAGE 0

typedef struct PSXLampList {
	uint32 n;
	uint32 states[MAX_NUMBER_LIGHTS];
	PSXLamp *lamps[MAX_NUMBER_LIGHTS];
} PSXLampList;

typedef struct PSXShadeList {
	uint32 n;
	uint32 states[MAX_NUMBER_SHADES];
	PSXShade *shades[MAX_NUMBER_SHADES];
} PSXShadeList;

typedef struct LampInfo {
	uint32 intens;
	uint32 mult;
	uint32 index;
	uint32 bounce;
	uint32 width;
	uint32 rr;
	VECTOR direct;
	SVECTOR colour;
} LampInfo;

} // End of namespace ICB

#endif // #ifndef LIGHT_H
