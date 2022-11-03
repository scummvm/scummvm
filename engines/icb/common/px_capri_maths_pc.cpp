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

#include "engines/icb/psx_config.h"
#include "engines/icb/gfx/psx_pcdefines.h"

#if _PSX_ON_PC == 1
#include "engines/icb/common/px_capri_maths_pc.h"
#endif

namespace ICB {

MATRIXPC *gterot_pc;
MATRIXPC *gtetrans_pc;
MATRIXPC *gtecolour_pc;
MATRIXPC *gtelight_pc;
int32 gteback_pc[3];
int32 gtegeomscrn_pc;
int32 gtescreenscaleshift_pc = 0;

} // End of namespace ICB
