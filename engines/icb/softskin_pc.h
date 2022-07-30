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

#ifndef ICB_SOFTSKIN_PC_H
#define ICB_SOFTSKIN_PC_H

#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/gfx/rap_api.h"

namespace ICB {

int32 softskinPC(RapAPI *rap, int32 poseBone, MATRIXPC *lw, SVECTORPC *local, int16 *xminLocal, int16 *xmaxLocal, int16 *yminLocal, int16 *ymaxLocal, int16 *zminLocal,
			   int16 *zmaxLocal, int32 screenShift);

} // End of namespace ICB

#endif // #ifndef SOFTSKIN_PC_H
