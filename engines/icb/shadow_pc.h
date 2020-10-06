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

#ifndef ICB_SHADOW_PC_H
#define ICB_SHADOW_PC_H

#include "engines/icb/common/px_common.h"
#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/light_pc.h"
#include "engines/icb/gfx/psx_pxactor.h"
#include "engines/icb/gfx/psx_camera.h"
#include "engines/icb/gfx/rap_api.h"
#include "engines/icb/gfx/rab_api.h"
#include "engines/icb/shadow.h"

namespace ICB {

void DrawShadow1PC(rap_API *srap, int poseBone, MATRIXPC *lw, MATRIXPC *world2screen, MATRIXPC *local2world, int nShadows, SVECTORPC *ldirs, CVECTOR *lcolours, SVECTORPC *p_n,
                   int *p_d, int debug, SVECTOR **shadowBox, SVECTOR *shadowBoxMin, SVECTOR *shadowBoxMax);

} // End of namespace ICB

#endif // #ifndef SHADOW_PC_H
