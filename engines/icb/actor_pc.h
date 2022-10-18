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

#ifndef ICB_ACTOR_PC_H
#define ICB_ACTOR_PC_H

#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/gfx/psx_pcgpu.h"
#include "engines/icb/light_pc.h"
#include "engines/icb/gfx/psx_camera.h"
#include "engines/icb/gfx/psx_pxactor.h"
#include "engines/icb/gfx/psx_tman.h"
#include "engines/icb/gfx/rap_api.h"
#include "engines/icb/gfx/rab_api.h"
#include "engines/icb/gfx/rlp_api.h"
#include "engines/icb/common/px_bones.h"
#include "engines/icb/breath.h"

namespace ICB {

void ConvertToScreenCoords(SVECTORPC *local, SVECTORPC *screen, int32 nVertices);
void ConvertToScreenCoords(SVECTOR *local, SVECTOR *screen, int32 nVertices);

void DrawActor4PC(psxActor *actor, psxCamera *camera, Bone_Frame *frame, RapAPI *mesh, RapAPI *pose, RapAPI *smesh, PSXrgb *ambient, PSXLampList *lamplist,
				  PSXShadeList *shadelist, int32 nShadows, SVECTORPC *p_n, int32 *p_d, uint32 debug, int32 uvframe, BoneDeformation **boneDeforms, int32 *brightness,
				  MATRIXPC *local2screen);

void DrawActorTiePC(psxCamera *camera, SVECTORPC *pos, uint32 size, CVECTOR *colour);

// return 1 if something drawn, else return 0
int32 DrawActorSpecialEffectsPC(int32 mflash, SVECTOR *mfpos, int32 mfh, int32 mfw, int32 bullet, SVECTOR *bulletPos, int32 bulletCol, Breath *breath, MATRIXPC *local2screen, int32 brightness,
							  SVECTOR *minBBox, SVECTOR *maxBBox);

} // End of namespace ICB

#endif // #ifndef ACTOR_PC_H
