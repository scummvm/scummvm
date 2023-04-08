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
void HideRoomMeshes(Init &init, t3dBODY *body);
void t3dUpdateArrow(t3dMESH *m, t3dF32 len);
bool t3dSetSpecialAnimFrame(WGame &game, const char *name, t3dMESH *mesh, int32 nf);
void ChangeMeshFlags(t3dMESH *m, int8 add, uint32 newflags);
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

// TODO: This could perhaps be PIMPLd, as we don't really need to expose the implementation.
struct SMeshModifier {
	Common::String meshName;
private:
	int32 Flags = 0;
	uint32 AddFlags = 0;
	uint32 RemoveFlags = 0;
	uint32 AddMatFlags = 0;
	uint32 RemoveMatFlags = 0;
	int32 MatFrame = 0;
	uint16 BndLevel = 0;
	int8 HaloesStatus = 0;
public:
	Common::String animName;
	SMeshModifier() = default;
	SMeshModifier(const char *name, int16 com, void *p);
	SMeshModifier(Common::SeekableReadStream &stream);
	void configure(const char *name, int16 com, void *p);
	void modifyMesh(WGame &game, t3dMESH *mesh);
	uint16 getBndLevel() const { return BndLevel; }
	int32 getFlags() const { return Flags; }
	int8 getHaloesStatus() const { return HaloesStatus; }
};

class MeshModifiers {
	SMeshModifier MMList[MAX_MODIFIED_MESH] = {};
public:
	MeshModifiers() = default;
	MeshModifiers(Common::SeekableReadStream &stream) {
		for (int i = 0; i < MAX_MODIFIED_MESH; i++) {
			MMList[i] = SMeshModifier(stream);
		}
	}
	void addMeshModifier(const Common::String &name, int16 com, void *p);
	void applyAllMeshModifiers(WGame &game, t3dBODY *b);
	void modifyMesh(WGame &game, t3dMESH *mesh);
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_LL_MESH_H
