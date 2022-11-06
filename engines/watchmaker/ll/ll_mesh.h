/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef WATCHMAKER_LL_MESH_H
#define WATCHMAKER_LL_MESH_H

#include "watchmaker/t3d.h"
#include "watchmaker/globvar.h"
#include "watchmaker/work_dirs.h"

namespace Watchmaker {

uint8 t3dClipToSurface(Init &init, t3dV3F *pt);
void t3dLightChar(t3dMESH *mesh, t3dV3F *p);
void t3dProcessGolfSky(t3dMESH *gs);
void ApplyAllMeshModifiers(WGame &game, t3dBODY *b);
void HideRoomMeshes(Init &init, t3dBODY *body);
void t3dUpdateArrow(t3dMESH *m, t3dF32 len);
bool t3dSetSpecialAnimFrame(WGame &game, const char *name, t3dMESH *mesh, int32 nf);
void ChangeMeshFlags(t3dMESH *m, int8 add, uint32 newflags);
void AddMeshModifier(const Common::String &name, int16 com, void *p);
void UpdateObjMesh(Init &init, int32 in);
void UpdateBoundingBox(t3dMESH *mesh);
void UpdateCharHead(int32 oc, t3dV3F *dir);
void SetMeshMaterialMovieFrame(t3dMESH *m, int8 op, int32 newframe);
void ChangeMeshMaterialFlag(t3dMESH *m, int8 add, uint32 newflag);
void ChangeHaloesStatus(t3dBODY *b, int8 op);
uint8 t3dVectMeshInters(t3dMESH *m, t3dV3F start, t3dV3F end, t3dV3F *inters);
void t3dLightRoom(Init &init, t3dBODY *b, t3dV3F *p, t3dF32 NearRange, t3dF32 FarRange, t3dF32 IperRange);
void t3dUpdateExplosion(t3dMESH *m, t3dF32 scale);
bool t3dMoveAndCheck1stCamera(t3dBODY *rr, t3dCAMERA *cc, t3dV3F *mm);

} // End of namespace Watchmaker

#endif // WATCHMAKER_LL_MESH_H
