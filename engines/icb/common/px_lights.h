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

#include "engines/icb/common/px_common.h"
#include "engines/icb/gfx/psx_pcdefines.h"

namespace ICB {

enum pxLightType { OMNI_PX_LIGHT, FREE_SPOT_PX_LIGHT, TARGET_SPOT_PX_LIGHT, FREE_DIRECT_PX_LIGHT, TARGET_DIRECT_PX_LIGHT, GLOBAL_PX_LIGHT };

typedef struct {
	char name[16]; // String for user's name
	uint32 type;   // 0-5 enumerated, see above

	MATRIX PSX_matrix; // Standard PSX type, 32 bytes

	SVECTOR direction; // Standard PSX type, 8 bytes

	int32 pan;  // Fixed point angle
	int32 cant; // Fixed point angle
	int32 tilt; // Fixed point angle

	uint32 red;             // Fixed point integer
	uint32 green;           // Fixed point integer
	uint32 blue;            // Fixed point integer
	uint32 intensity;       // Fixed point integer
	uint32 hspot_size;      // Fixed point angle
	uint32 falloff_size;    // Fixed point angle
	uint32 use_nearAtten;   // 0 false, 1 true
	uint32 nearAtten_start; // Nearest integer
	uint32 nearAtten_end;   // Nearest integer
	uint32 use_farAtten;    // 0 false, 1 true
	uint32 farAtten_start;  // Nearest integer
	uint32 farAtten_end;    // Nearest integer

	uint32 shape;           // 0 rectangular, 1 circlular
	uint32 aspect;          // Fixed point integer
	uint32 overshoot;       // 0 false, 1 true
	uint32 shadow;          // 0 false, 1 true
	uint32 affect_diffuse;  // 0 false, 1 true
	uint32 affect_specular; // 0 false, 1 true

} _pxLightState; // Size 144 bytes in total

class _pxLights {
public:
	_pxLights() { ; }
	~_pxLights() { ; }

	uint32 Fetch_number_of_items() const { return num_of_lights; }
	inline const _pxLightState *Fetch_item_by_number(uint32 number) const;

private:
	uint32 num_of_lights;
};

// All lights are of the same size to skip to the start of whichever is wanted
inline const _pxLightState *_pxLights::Fetch_item_by_number(uint32 number) const {
	if (number < num_of_lights) {
		return ((_pxLightState *)(((char *)this) + (4 + (number * 144))));
	} else
		return NULL;
}

} // End of namespace ICB
