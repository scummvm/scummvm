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

#ifndef WATCHMAKER_GEOMETRY_H
#define WATCHMAKER_GEOMETRY_H

#include "watchmaker/t3d.h"

#define MAX_RECURSION_LEVEL 10

#define MAX_PARTICLES   10

namespace Watchmaker {

#define LEFTCLIP        0
#define RIGHTCLIP       1
#define TOPCLIP         2
#define BOTTOMCLIP      3
#define NUMCLIPPLANES   4

extern t3dNORMAL   ClipPlanes[NUMCLIPPLANES];

extern t3dBODY      *t3dCurRoom, *t3dOrigRoom;
extern t3dCAMERA    *t3dCurCamera;
extern t3dM3X3F     t3dCurViewMatrix;
extern t3dBODY      *PortalCrossed;
extern t3dCHARACTER *t3dCurCharacter;
extern uint32       t3d_NumMeshesVisible;
extern t3dMESH      *t3d_VisibleMeshes[];

extern t3dBODY      *t3dRxt;
extern t3dBODY      *t3dSky;
//s extern t3dBODY      *t3dSun;

extern uint8        FloorHit;
extern t3dV3F       t3d3dMousePos;
extern t3dV3F       FloorHitCoords;
extern uint32       StatNumTris, StatNumVerts;

struct WindowInfo;

uint16 t3dBackfaceCulling(NormalList &normals, uint32 NumNormals, t3dV3F  *eye);

void t3dReleaseBody(t3dBODY *b);
void t3dReleaseCharacter(t3dCHARACTER *b);
unsigned char t3dCreateSmokeParticle(unsigned int Num, unsigned char Type, unsigned int Opacity);
void t3dCalcRejectedMeshFromPortal(t3dBODY *body);
void t3dSetViewport(t3dCAMERA *cam, WindowInfo &info, t3dF32 fov, uint8 sup);
t3dF32 t3dCheckWithFloor();
void t3dCreateProceduralSky();
void t3dCalcMeshBones(t3dMESH *mesh, int32 last);
void t3dResetPipeline();
void t3dRotateMoveCamera(t3dCAMERA *cam, t3dF32 AngleX, t3dF32 AngleY, t3dF32 AngleSpeed);
void t3dReleaseParticles();
void t3dResetMesh(t3dMESH *mesh);
void t3dShowBoundingBox(t3dBODY *b);
void t3dShowBounds(t3dPAN *p, uint32 numpan);
bool t3dTransformBody(t3dBODY *b);
void t3dTransformSky();
bool t3dTransformCharacter(t3dCHARACTER *c);
void t3dProcessPortals();
t3dBODY *t3dCheckPortalCrossed(t3dV3F *a);
void t3dSortMeshes();
void QueueMaterialList(MaterialTable &MatList, unsigned int NumMat, signed short int ViewMatrixNum);
void ProcessMaterialList();
void t3dAddTriangle(t3dF32 x1, t3dF32 y1, t3dF32 x2, t3dF32 y2, t3dF32 x3, t3dF32 y3,
                    int32 r, int32 g, int32 b, int32 a);
void t3dAddQuad(t3dF32 x1, t3dF32 y1, t3dF32 x2, t3dF32 y2, t3dF32 x3, t3dF32 y3, t3dF32 x4, t3dF32 y4,
                int32 r, int32 g, int32 b, int32 a);
} // End of namespace Watchmaker

#endif // WATCHMAKER_GEOMETRY_H
