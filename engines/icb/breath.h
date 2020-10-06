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

#ifndef ICB_BREATH_H
#define ICB_BREATH_H

#include "engines/icb/common/px_common.h"

namespace ICB {

#define MAX_BREATH 8 // number of particles

#define BREATH_OFF 0
#define BREATH_ON 1
#define BREATH_SMOKE 2

class Breath {
public:
	SVECTOR position; // 8 bytes?

	short breathColour[MAX_BREATH]; // 8 bytes

	int8 breathZ[MAX_BREATH];       // 4 bytes
	int8 breathY[MAX_BREATH];       // 4 bytes
	int8 breathSize[MAX_BREATH];    // 4 bytes
	int8 breathStarted[MAX_BREATH]; // 4 bytes
	short breathEnd;                // 2 bytes
	uint8 on;                       // 1 byte
	int8 allStarted;                // 1 byte (how many times reset)

	Breath();
	void Update();
	void Init();
};

} // End of namespace ICB

#endif
