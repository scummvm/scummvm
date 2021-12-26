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

#ifndef ICB_NON_AD_MODULE_INCLUDED_
#define ICB_NON_AD_MODULE_INCLUDED_

#include "engines/icb/common/px_common.h"
#include "engines/icb/gfx/psx_pcdefines.h"

namespace ICB {

struct PCcamera {
	char id[4];
	int32 schema;
	MATRIX view;
	int32 pan;
	uint16 focLen;
};

void WorldToFilm(const PXvector_PC &worldpos, const PCcamera &camera, bool8 &is_onfilm, PXvector_PC &filmpos);

PXreal AngleOfVector(PXreal x, PXreal y);

} // End of namespace ICB

#endif //_NON_AD_MODULE_INCLUDED_
