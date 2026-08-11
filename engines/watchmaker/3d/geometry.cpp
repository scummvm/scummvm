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

#include "watchmaker/3d/mem_management.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/light.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/t3d_face.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/globvar.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/render.h"
#include "watchmaker/renderer.h"
#include "watchmaker/t3d.h"
#include "watchmaker/utils.h"
#include "watchmaker/windows_hacks.h"

#define SKY_SPEED               0.00005f
#define EXPRESSION_SET_LEN      14

namespace Watchmaker {

t3dCAMERA   Camera;
t3dCAMERA   *t3dCurCamera = &Camera;
t3dBODY     *t3dCurRoom, *t3dOrigRoom;
t3dBODY     *PortalCrossed = nullptr;
t3dCHARACTER    *t3dCurCharacter;

t3dM3X3F    t3dCurViewMatrix;
t3dV3F      t3dCurTranslation;
int16      t3dCurUserViewMatrix, t3dOldUserViewMatrix, t3dOrigUserViewMatrix;

uint32      t3dStartIndex, t3dNumVertices;

uint8       FloorHit;
t3dV3F      FloorHitCoords;

t3dV3F      t3d3dMousePos;

t3dBODY     *t3dRxt = nullptr;
t3dBODY     *t3dSky = nullptr;
//s t3dBODY     *t3dSun= nullptr;

uint32      t3d_NumMeshesVisible;
t3dMESH     *t3d_VisibleMeshes[255];

t3dNORMAL   ClipPlanes[NUMCLIPPLANES];

t3dMESH     *t3dPortalList[MAX_RECURSION_LEVEL];
uint32      t3dNumPortals;
uint8       bOrigRoom;
uint8       bPortalRoom;
uint8       bViewOnlyPortal;
uint8       bDisableMirrors;
uint8       bNoLightmapsCalc;
uint8       LastFlaresVisible;
t3dMESH     *t3dGlobalMirrorList[MAX_MIRRORS];
uint32      t3dNumGlobalMirrors;

uint32      StatNumTris, StatNumVerts;

int32      MaxSetDimX, MaxSetDimY;

uint32      skyval1, skyval2;
gTexture    *SkySurface;
gTexture    *SmokeSurface, *NoiseSmokeSurface;

#define T3D_PARTICLE_ACTIVE     (1<<0)
#define T3D_PARTICLE_REGEN      (1<<1)

struct Particle {
	t3dV3F      Pos;
	t3dF32      Size;
	uint32      Flags;
	t3dV3F      Color;
};

struct _PARTICLESTRUCT {
	Particle    *ParticlePos;
	uint32      NumParticles;
	uint32      FirstParticleRendered;
	gMaterial   Material;
	int8       RandTable[16];
	uint8       ParticleWait;
} Particles[MAX_PARTICLES];
uint32 t3dNumParticles = 0;

#define MAX_MATERIAL_LISTS      50

struct _MaterialList {
	MaterialTable *mat_list; // This is brittle, as there is no ownership flagging.
	unsigned int num_mat;
	signed short int matrix;
} t3dMaterialList[MAX_MATERIAL_LISTS];

uint32 t3dNumMaterialLists = 0;

/* -----------------10/06/99 15.45-------------------
 *              t3dCheckBoundSphere
 * --------------------------------------------------*/
uint8 t3dCheckBoundSphere(t3dMESH &mesh) {
	t3dV3F  pos;
	t3dF32  d1;

	if (mesh.Flags & T3D_MESH_ALWAYSVISIBLE)
		return 1;

	t3dVectAdd(&pos, &mesh.Pos, &mesh.Trasl);
	t3dVectSub(&pos, &pos, &t3dCurCamera->Source);
	t3dVectTransform(&pos, &pos, &t3dCurViewMatrix); // rotate by the mesh matrix

	if ((pos.z < 0) && (pos.z < -mesh.Radius))
		return 0;

	d1 = t3dVectDot(&pos, &ClipPlanes[RIGHTCLIP].n) + ClipPlanes[RIGHTCLIP].dist;
	if ((d1 > 0) || (mesh.Radius > (-d1))) {
		d1 = ClipPlanes[LEFTCLIP].dist + t3dVectDot(&pos, &ClipPlanes[LEFTCLIP].n);
		if ((d1 > 0) || (mesh.Radius > (-d1))) {
			d1 = ClipPlanes[TOPCLIP].dist + t3dVectDot(&pos, &ClipPlanes[TOPCLIP].n);
			if ((d1 > 0) || (mesh.Radius > (-d1))) {
				d1 = ClipPlanes[BOTTOMCLIP].dist + t3dVectDot(&pos, &ClipPlanes[BOTTOMCLIP].n);
				if ((d1 > 0) || (mesh.Radius > (-d1))) {
					return 1;
				}
			}
		}
	}

	return 0;
}


/* -----------------10/06/99 15.45-------------------
 *                  t3dCheckBoundBox
 * --------------------------------------------------*/
uint8 t3dCheckBoundBox(t3dMESH &mesh) {
	t3dV3F  TBBox[8], inter;
	t3dF32  d1;
	uint32  in = 0, i;

	if (mesh.Flags & T3D_MESH_ALWAYSVISIBLE)
		return 1;

	for (i = 0; i < 8; i++) {
		t3dVectAdd(&TBBox[i], &mesh.BBox[i].p, &mesh.Trasl);
		t3dVectSub(&TBBox[i], &TBBox[i], &t3dCurCamera->Source);
		t3dVectTransform(&TBBox[i], &TBBox[i], &t3dCurViewMatrix); // rotate by the mesh matrix
	}

	for (i = 0; i < 8; i++)
		if ((d1 = t3dVectDot(&TBBox[i], &ClipPlanes[RIGHTCLIP].n) + ClipPlanes[RIGHTCLIP].dist) > 0)
			if ((d1 = ClipPlanes[LEFTCLIP].dist + t3dVectDot(&TBBox[i], &ClipPlanes[LEFTCLIP].n)) > 0)
				if ((d1 = ClipPlanes[TOPCLIP].dist + t3dVectDot(&TBBox[i], &ClipPlanes[TOPCLIP].n)) > 0)
					if ((d1 = ClipPlanes[BOTTOMCLIP].dist + t3dVectDot(&TBBox[i], &ClipPlanes[BOTTOMCLIP].n)) > 0)
						in++;

	if (in) return 1;

	for (i = 0; i < NUMCLIPPLANES; i++) {
		if (t3dVectPlaneIntersection(&inter, TBBox[0], TBBox[1], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[1], TBBox[5], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[5], TBBox[4], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[4], TBBox[0], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[2], TBBox[3], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[3], TBBox[7], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[7], TBBox[6], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[6], TBBox[2], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[0], TBBox[2], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[1], TBBox[3], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[5], TBBox[7], ClipPlanes[i])) return 1;
		if (t3dVectPlaneIntersection(&inter, TBBox[4], TBBox[6], ClipPlanes[i])) return 1;
	}

	return 0;
}

/* -----------------11/06/99 12.23-------------------
 *              t3dCheckBlockMesh
 * --------------------------------------------------*/
void t3dCheckBlockMesh(Common::Array<t3dMESH> &mt, uint32 NumMeshes, t3dMESH *blockmesh) {
	t3dF32  xa, za, xb, zb, xc, zc, xd, zd;
	t3dF32  r, s, divisor;

	uint8   ref[4] = {2, 3, 6, 7};

	if (!blockmesh || mt.empty() || !blockmesh->VertexBuffer) return;

	xc = t3dCurCamera->Source.x;
	zc = t3dCurCamera->Source.z;

	xa = za =  9999999.9f;
	xb = zb = -9999999.9f;
	blockmesh->VBptr = blockmesh->VertexBuffer;
	for (uint32 j = 0; j < blockmesh->NumFaces(); j++) {
		t3dFACE &f = blockmesh->FList[j];
		if (!f.n) continue;

		for (uint32 i = 0; i < 3; i++) {
			if (blockmesh->VBptr[f.VertexIndex[i]].x < xa) {
				xa = blockmesh->VBptr[f.VertexIndex[i]].x;
				za = blockmesh->VBptr[f.VertexIndex[i]].z;
			}
			if (blockmesh->VBptr[f.VertexIndex[i]].x > xb) {
				xb = blockmesh->VBptr[f.VertexIndex[i]].x;
				zb = blockmesh->VBptr[f.VertexIndex[i]].z;
			}
		}
	}
	blockmesh->VBptr = nullptr;

	for (uint32 i = 0; i < NumMeshes; i++) {
		t3dMESH &mesh = mt[i];
		if ((mesh.Flags & T3D_MESH_HIDDEN) || (mesh.Flags & T3D_MESH_INVISIBLEFROMSECT))
			continue;

		uint32 j;
		for (j = 0; j < 4; j++) {
			xd = mesh.Trasl.x + mesh.BBox[ref[j]].p.x;
			zd = mesh.Trasl.z + mesh.BBox[ref[j]].p.z;

			divisor = (t3dF32)((xb - xa) * (zd - zc) - (zb - za) * (xd - xc));
			if (!divisor) divisor = 0.000001f;
			r = (t3dF32)((za - zc) * (xd - xc) - (xa - xc) * (zd - zc)) / divisor;
			s = (t3dF32)((za - zc) * (xb - xa) - (xa - xc) * (zb - za)) / divisor;

			if ((r < 0.0f) || (r > (1.0f)) || (s < 0.0f) || (s > (1.0f)))
				break;
		}
		if (j >= 4) mesh.Flags |= T3D_MESH_INVISIBLEFROMSECT;
	}
}


/* -----------------10/06/99 15.53-------------------
 *          t3dCalcRejectedMeshFromPortal
 * --------------------------------------------------*/
void t3dCalcRejectedMeshFromPortal(t3dBODY *body) {
	t3dV3F      appo, c0;
	t3dV3F      ppv1, ppv2, ppv3, ppv4, Source, Target;
	uint32      p;
	t3dNORMAL   OldClipPlanes[NUMCLIPPLANES];
	t3dF32      minx, miny, minz, maxx, maxy, maxz;

	for (uint32 i = 0; i < body->NumMeshes(); i++) {
		t3dMESH &mesh = body->MeshTable[i];
		if ((mesh.Flags & T3D_MESH_PREPROCESSPORTAL) && (!(mesh.Flags & T3D_MESH_NOPORTALCHECK)) &&
		        (mesh.PortalList) && (mesh.RejectedMeshes.empty())) {
			mesh.VBptr = mesh.VertexBuffer;
			ppv1.x = mesh.VBptr[mesh.FList[0].VertexIndex[0]].x;
			ppv1.y = mesh.VBptr[mesh.FList[0].VertexIndex[0]].y;
			ppv1.z = mesh.VBptr[mesh.FList[0].VertexIndex[0]].z;
			ppv2.x = mesh.VBptr[mesh.FList[0].VertexIndex[1]].x;
			ppv2.y = mesh.VBptr[mesh.FList[0].VertexIndex[1]].y;
			ppv2.z = mesh.VBptr[mesh.FList[0].VertexIndex[1]].z;
			ppv3.x = mesh.VBptr[mesh.FList[0].VertexIndex[2]].x;
			ppv3.y = mesh.VBptr[mesh.FList[0].VertexIndex[2]].y;
			ppv3.z = mesh.VBptr[mesh.FList[0].VertexIndex[2]].z;

			for (p = 0; p < 3; p++) {
				appo.x = mesh.VBptr[mesh.FList[1].VertexIndex[p]].x;
				appo.y = mesh.VBptr[mesh.FList[1].VertexIndex[p]].y;
				appo.z = mesh.VBptr[mesh.FList[1].VertexIndex[p]].z;

				if ((ppv1 != appo) && (ppv2 != appo) && (ppv3 != appo))
					memcpy(&ppv4, &appo, sizeof(t3dV3F));
			}

			memcpy(OldClipPlanes, ClipPlanes, sizeof(t3dNORMAL)*NUMCLIPPLANES);

			minx = miny = minz = 9999999.9f;
			maxx = maxy = maxz = -9999999.9f;
			if (ppv1.x < minx) minx = ppv1.x;
			if (ppv1.y < miny) miny = ppv1.y;
			if (ppv1.z < minz) minz = ppv1.z;
			if (ppv1.x > maxx) maxx = ppv1.x;
			if (ppv1.y > maxy) maxy = ppv1.y;
			if (ppv1.z > maxz) maxz = ppv1.z;

			if (ppv2.x < minx) minx = ppv2.x;
			if (ppv2.y < miny) miny = ppv2.y;
			if (ppv2.z < minz) minz = ppv2.z;
			if (ppv2.x > maxx) maxx = ppv2.x;
			if (ppv2.y > maxy) maxy = ppv2.y;
			if (ppv2.z > maxz) maxz = ppv2.z;

			if (ppv3.x < minx) minx = ppv3.x;
			if (ppv3.y < miny) miny = ppv3.y;
			if (ppv3.z < minz) minz = ppv3.z;
			if (ppv3.x > maxx) maxx = ppv3.x;
			if (ppv3.y > maxy) maxy = ppv3.y;
			if (ppv3.z > maxz) maxz = ppv3.z;

			if (ppv4.x < minx) minx = ppv4.x;
			if (ppv4.y < miny) miny = ppv4.y;
			if (ppv4.z < minz) minz = ppv4.z;
			if (ppv4.x > maxx) maxx = ppv4.x;
			if (ppv4.y > maxy) maxy = ppv4.y;
			if (ppv4.z > maxz) maxz = ppv4.z;

			Source.x = minx + ((maxx - minx) / 2.0f) + mesh.NList[0]->n.x * 100;
			Source.y = miny + ((maxy - miny) / 2.0f) + mesh.NList[0]->n.y * 100;
			Source.z = minz + ((maxz - minz) / 2.0f) + mesh.NList[0]->n.z * 100;
			Target.x = minx + ((maxx - minx) / 2.0f);
			Target.y = miny + ((maxy - miny) / 2.0f);
			Target.z = minz + ((maxz - minz) / 2.0f);

			t3dMatView(&t3dCurViewMatrix, &Source, &Target);


			t3dVectSub(&ppv4, &ppv4, &Source);
			t3dVectSub(&ppv3, &ppv3, &Source);
			t3dVectSub(&ppv2, &ppv2, &Source);
			t3dVectSub(&ppv1, &ppv1, &Source);
			t3dVectTransform(&ppv1, &ppv1, &t3dCurViewMatrix);
			t3dVectTransform(&ppv2, &ppv2, &t3dCurViewMatrix);
			t3dVectTransform(&ppv3, &ppv3, &t3dCurViewMatrix);
			t3dVectTransform(&ppv4, &ppv4, &t3dCurViewMatrix);
			t3dVectFill(&c0, 0.0f);

			//  t3dVectCopy(&c0,&cam->Source);

			t3dPlaneNormal(&ClipPlanes[RIGHTCLIP],  &c0, &ppv3, &ppv4);
			t3dPlaneNormal(&ClipPlanes[LEFTCLIP],   &c0, &ppv1, &ppv2);
			t3dPlaneNormal(&ClipPlanes[TOPCLIP],    &c0, &ppv4, &ppv1);
			t3dPlaneNormal(&ClipPlanes[BOTTOMCLIP], &c0, &ppv2, &ppv3);

			mesh.VBptr = nullptr;

			t3dVectCopy(&t3dCurCamera->Source, &Source);
			t3dVectCopy(&t3dCurCamera->Target, &Target);

			for (uint32 j = 0; j < mesh.PortalList->NumMeshes(); j++) {
				t3dMESH *m = &mesh.PortalList->MeshTable[j];
				if (!t3dCheckBoundSphere(*m)) {
					mesh.RejectedMeshes.push_back(m);
				}
			}
		}
	}
}

/* -----------------10/06/99 15.39-------------------
 *                  t3dReleaseBody
 * --------------------------------------------------*/
void t3dReleaseBody(t3dBODY *b) {
	if (!b) return;

	for (auto &mesh : b->MeshTable) {
		mesh.release();
	}
	b->MeshTable.clear();

	if (!b->MatTable.empty())
		rRemoveMaterials(b->MatTable);
	b->MatTable.clear();

	if (b->LightmapTable.size() > 0)
		rRemoveMaterials(b->LightmapTable);
	b->LightmapTable.clear();

	if (b->MirrorMatTable.size() > 0) {
		rRemoveMaterials(b->MirrorMatTable);
	}
	b->MirrorMatTable.clear();


	b->clearVBTable();

	for (int i = 0; i < T3D_MAX_LEVELS; i++) {
		delete[] b->Panel[i];
		b->Panel[i] = nullptr;
	}

	// Not at all necessary
	b->CameraGrid.Grid.clear();
	b->CameraPath.clear();
	b->CameraTable.clear();
	b->LightTable.clear();
	b->PosLightTable.clear();
	b->NList.clear();
	b->VolumetricLights.reset();
	for (int i = 0; i < T3D_MAX_BLOCK_MESHES; i++)
		b->BlockMeshes[i] = nullptr;
	t3dFree(b);
}


/* -----------------10/06/99 15.40-------------------
 *                  t3dReleaseCharacter
 * --------------------------------------------------*/
void t3dReleaseCharacter(t3dCHARACTER *b) {
	warning("STUBBED: t3dReleaseCharacter");
#if 0
	t3dU32  i, j;

	if (!b)
		return;

//	if ( b->Body )
//		t3dReleaseBody(b->Body);
	b->Body = nullptr;

	if (b->Shadow) {
		for (i = 0; i < b->Shadow->NumMeshes; i++)
			b->Shadow->MeshTable[i].VertexBuffer = nullptr;
//m     t3dReleaseBody(b->Shadow);
	}
	b->Shadow = nullptr;

	for (i = 0; i < T3D_MAX_SHADOWBOX_PER_CHAR; i++) {
		if (b->ShadowBox[i]) {
			for (j = 0; j < MAX_SHADOWS_PER_LIGHT; j++) {
				rDeleteVertexBuffer(b->ShadowBox[i]->ShadowsList[j].VB);
				b->ShadowBox[i]->ShadowsList[j].VB = nullptr;
				rDeleteVertexBuffer(b->ShadowBox[i]->ShadowsList[j].ProjVertsVB);
				b->ShadowBox[i]->ShadowsList[j].ProjVertsVB = nullptr;

				if (b->ShadowBox[i]->ShadowsList[j].pwShadVolIndices != b->ShadowBox[i]->ShadowsList[j].pwShadVolSideIndices)
					t3dFree(b->ShadowBox[i]->ShadowsList[j].pwShadVolIndices);
				b->ShadowBox[i]->ShadowsList[j].pwShadVolIndices = nullptr;
				t3dFree(b->ShadowBox[i]->ShadowsList[j].pwShadVolSideIndices);
				b->ShadowBox[i]->ShadowsList[j].pwShadVolSideIndices = nullptr;

				t3dFree(b->ShadowBox[i]->ShadowsList[j].pwShadVolCapIndices);
				b->ShadowBox[i]->ShadowsList[j].pwShadVolCapIndices = nullptr;
			}
			rDeleteVertexBuffer(b->ShadowBox[i]->VB);
			b->ShadowBox[i]->VB = nullptr;
			t3dFree(b->ShadowBox[i]->pwIndices);
			b->ShadowBox[i]->pwIndices = nullptr;
			t3dFree(b->ShadowBox[i]);
			b->ShadowBox[i] = nullptr;
		}
	}

	t3dFree(b);
	b = nullptr;
#endif
}

/* -----------------10/06/99 15.40-------------------
 *                  t3dBackfaceCulling
 * --------------------------------------------------*/
uint16 t3dBackfaceCulling(NormalList &normals, uint32 NumNormals, t3dV3F  *eye) {
	uint32 NumVisiNorm = 0;
	// Attiva solo le normali che si vedono
	for (uint32 i = 0; i < NumNormals; i++) {
		auto n = normals[i];
		n->tras_n = n->dist - t3dVectDot(&n->n, eye);
		if (n->tras_n > 0)
			n->flag = T3D_NORMAL_INVISIBLE;
		else
			n->flag = T3D_NORMAL_VISIBLE;
	}

	return NumVisiNorm;
}

/* -----------------10/06/99 15.55-------------------
 *                      ssqrt
 * --------------------------------------------------*/
inline long ssqrt(long i) {
	error("TODO: ssqrt");
#if 0
	long r = rand(), rnew = 1, rold = r;

	do {
		rold = r;
		r = rnew;
		rnew = (r + (i / r));
		rnew >>= 1;
	} while (rold != rnew);
	return rnew;
#endif
}

/* -----------------10/06/99 15.55-------------------
 *                      isprime
 * --------------------------------------------------*/
inline int isprime(long i) {
	long si, j;

	if (i == 0)
		return 0;

	si = ssqrt(i);

	for (j = 2; (j <= si); j++) {
		if (i % j == 0)
			return 0;
	}

	return 1;

}

/* -----------------10/06/99 15.55-------------------
 *                  Cosine_Interpolate
 * --------------------------------------------------*/
t3dF32 Cosine_Interpolate(t3dF32 a, t3dF32 b, t3dF32 x) {
	t3dF32  ft = x * T3D_PI;
	t3dF32  f = (1.0f - (t3dF32)cos(ft)) * 0.5f;
	return  a * (1.0f - f) + b * f;

//return  a*(1.0f-x) + b*x;
}

/* -----------------10/06/99 15.55-------------------
 *                  SelectRandomNumbers
 * --------------------------------------------------*/
void SelectRandomNumbers() {
	error("TODO: SelectRandomNumbers");
#if 0
	while (!isprime(skyval1 = (uint32)rand() % 100));
	while (!isprime(skyval2 = (uint32)(skyval1 - rand() % 100) % 15));
#endif
}

/* -----------------10/06/99 15.56-------------------
 *                      Noise1
 * --------------------------------------------------*/
t3dF32 Noise1(int32 x, int32 y) {
	int32  n;
	t3dF32  f;

	/*    n = x + y * 57;
	    n = (n<<13)^n;
	    f=(t3dF32)((n * (n * n * 15731 + 789221) + 1 376 312 589) & 0x7fffffff);
	    return ( 1.0f - f / 1 073 741 824.0f);
	*/
	skyval1 = 13;
	skyval2 = 131;
	n = x + y * skyval1;
	n = (n << skyval2)^n;
	f = (t3dF32)((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff);
	return (1.0f - f / 1073741824.0f);
//	f=(t3dF32)((n * (n * n * skyval3 + skyval4) + skyval5) & 0x7fffffff);
	//return ( 1.0f - f / (t3dF32)skyval6);
}

/* -----------------10/06/99 15.56-------------------
 *                  SmoothNoise_1
 * --------------------------------------------------*/
t3dF32 SmoothNoise_1(int32 x, int32 y) {
	t3dF32  corners = (Noise1(x - 1, y - 1) + Noise1(x + 1, y - 1) + Noise1(x - 1, y + 1) + Noise1(x + 1, y + 1)) / 16;
	t3dF32  sides   = (Noise1(x - 1, y)  + Noise1(x + 1, y)  + Noise1(x, y - 1)  + Noise1(x, y + 1)) /  8;
	t3dF32  center  =  Noise1(x, y) / 4;
	return corners + sides + center;

//	return ((Noise1(x-1, y-1)+Noise1(x+1, y-1)+Noise1(x-1, y+1)+Noise1(x+1, y+1))+( Noise1(x-1, y)  +Noise1(x+1, y)  +Noise1(x, y-1)  +Noise1(x, y+1) )+Noise1(x, y))/9.0f;
}

/* -----------------10/06/99 15.56-------------------
 *              InterpolatedNoise_1
 * --------------------------------------------------*/
t3dF32 InterpolatedNoise_1(t3dF32 x, t3dF32 y) {
	int32  integer_X    = (int32)(x);
	t3dF32  fractional_X = x - (t3dF32)integer_X;

	int32  integer_Y    = (int32)(y);
	t3dF32  fractional_Y = y - (t3dF32)integer_Y;

	t3dF32  v1 = SmoothNoise_1(integer_X,     integer_Y);
	t3dF32  v2 = SmoothNoise_1(integer_X + 1, integer_Y);
	t3dF32  v3 = SmoothNoise_1(integer_X,     integer_Y + 1);
	t3dF32  v4 = SmoothNoise_1(integer_X + 1, integer_Y + 1);

	t3dF32  i1 = Cosine_Interpolate(v1, v2, fractional_X);
	t3dF32  i2 = Cosine_Interpolate(v3, v4, fractional_X);

	return Cosine_Interpolate(i1, i2, fractional_Y);
}

/* -----------------10/06/99 15.56-------------------
 *                  CloudExpCurve
 * --------------------------------------------------*/
t3dF32 CloudExpCurve(t3dF32 v, t3dF32 CloudCover, t3dF32 CloudSharpness) {
	t3dF32 c = v - CloudCover;
	if (c < 0.0f)
		c = 0;

	return (255 - ((t3dF32)pow(CloudSharpness, c) * 255));
}

/* -----------------10/06/99 15.57-------------------
 *                  PerlinNoise_2D
 * --------------------------------------------------*/
t3dF32 PerlinNoise_2D(t3dF32 x, t3dF32 y, t3dF32 persistence, uint32 NumberOfOctaves) {
	uint32  i;
	t3dF32  total = 0.0f;
	t3dF32  frequency;
	t3dF32  amplitude = 1;

	for (i = 0; i < NumberOfOctaves; i++) {
		frequency = /*1.0f*/(t3dF32)pow(2, i);
		amplitude = /*1.0f*/((t3dF32)pow(persistence, i));

		total += InterpolatedNoise_1((x * frequency), (y * frequency)) * amplitude;
	}
//	DebugFile("perl: %f",(total));
	return total;
}

/* -----------------10/06/99 15.57-------------------
 *              t3dCreateProceduralSky
 * --------------------------------------------------*/
void t3dCreateProceduralSky() {
	unsigned short *p;
	int32  i, j, pitch;
	t3dF32  f, x, y, xstep, ystep;
	uint32  cr, cb, cg;
	t3dF32  CloudCover[4] = { 2.0f, 5.0f, 2.0f, 1.0f };
	t3dF32  CloudSharpness[4] = { 0.98f, 0.95f, 0.98f, 0.98f };
	t3dF32  CloudR[4] = { 143.0f, 131.0f, 145.0f,  45.0f };
	t3dF32  CloudG[4] = { 155.0f, 129.0f, 130.0f,  49.0f };
	t3dF32  CloudB[4] = { 174.0f, 100.0f, 148.0f,  50.0f };
	uint32  Refr[4] = {88, 110, 110, 36 };
	uint32  Refg[4] = {106, 150, 106, 35 };
	uint32  Refb[4] = {164, 240, 134, 42 };
	uint8   *tab;
	uint32  MARGIN;

	if (t3dSky == nullptr) return;
	if ((t3dCurOliSet < 0) || (t3dCurOliSet > 3)) t3dCurOliSet = 0;

	//set a random seed
	error("TODO:t3dCreateProceduralSky");
#if 0
	srand((unsigned)t3dReadTime());
	SelectRandomNumbers();
#endif
	MaxSetDimX = MaxSetDimY = 256;
	MARGIN = MaxSetDimX / 2;
	tab = new uint8[MaxSetDimX * MaxSetDimY * 3]{};

	if (!SkySurface)  // rReleaseBitmapDirect( SkySurface );
		SkySurface = (gTexture *)rCreateSurfaceP(256, 256, rTEXTURESURFACE);

	p = (unsigned short *)rLockSurfaceDirect(SkySurface, (unsigned int *)&pitch);
	pitch /= 2;

	x = y = 0;
	ystep = xstep = (1.0f / (256.0f));
	for (j = 0; j < MaxSetDimY; j++, y += ystep) {
		x = 0.0f;
		for (i = 0; i < MaxSetDimX; i++, x += xstep) {
			f = (PerlinNoise_2D(x * 2.0f, y * 4.0f, 0.65f, 8));
			if (f < 0.0f)
				f = -f;

			cr = (uint32)CloudExpCurve(f * CloudR[t3dCurOliSet], CloudCover[t3dCurOliSet], CloudSharpness[t3dCurOliSet]);
			cg = (uint32)CloudExpCurve(f * CloudG[t3dCurOliSet], CloudCover[t3dCurOliSet], CloudSharpness[t3dCurOliSet]);
			cb = (uint32)CloudExpCurve(f * CloudB[t3dCurOliSet], CloudCover[t3dCurOliSet], CloudSharpness[t3dCurOliSet]);

			if (cr < Refr[t3dCurOliSet]) {
				cr += Refr[t3dCurOliSet];
				cr /= 2;
			}
			if (cg < Refg[t3dCurOliSet]) {
				cg += Refg[t3dCurOliSet];
				cg /= 2;
			}
			if (cb < Refb[t3dCurOliSet]) {
				cb += Refb[t3dCurOliSet];
				cb /= 2;
			}
			if (cr > 255)
				cr = 255 - (cr - 255);
			if (cg > 255)
				cg = 255 - (cg - 255);
			if (cb > 255)
				cb = 255 - (cb - 255);

			if (cr > 255)
				cr = 255;
			if (cg > 255)
				cg = 255;
			if (cb > 255)
				cb = 255;

			tab[i * 3 + 0 + j * 3 * MaxSetDimX] = cr;
			tab[i * 3 + 1 + j * 3 * MaxSetDimX] = cg;
			tab[i * 3 + 2 + j * 3 * MaxSetDimX] = cb;
		}
	}

	ystep = 1.0f / MARGIN;
	y = 1.0f;
	for (j = 0; j < (int32)MARGIN; j++, y -= ystep) {
		for (i = 0; i < MaxSetDimX; i++) {
			cr = (uint32)Cosine_Interpolate(tab[i * 3 + 0 + j * 3 * MaxSetDimX], tab[i * 3 + 0 + (255 - j) * 3 * MaxSetDimX], y);
			cg = (uint32)Cosine_Interpolate(tab[i * 3 + 1 + j * 3 * MaxSetDimX], tab[i * 3 + 1 + (255 - j) * 3 * MaxSetDimX], y);
			cb = (uint32)Cosine_Interpolate(tab[i * 3 + 2 + j * 3 * MaxSetDimX], tab[i * 3 + 2 + (255 - j) * 3 * MaxSetDimX], y);
			if (cr > 255)
				cr = 255;
			if (cg > 255)
				cg = 255;
			if (cb > 255)
				cb = 255;
			tab[i * 3 + 0 + j * 3 * MaxSetDimX] = cr;
			tab[i * 3 + 1 + j * 3 * MaxSetDimX] = cg;
			tab[i * 3 + 2 + j * 3 * MaxSetDimX] = cb;
		}
	}
	xstep = 1.0f / MARGIN;
	x = 1.0f;
	for (j = 0; j < MaxSetDimY; j++) {
		x = 1.0f;
		for (i = 0; i < (int32)MARGIN; i++, x -= xstep) {
			cr = (uint32)Cosine_Interpolate(tab[i * 3 + 0 + j * 3 * MaxSetDimX], tab[(255 - i) * 3 + 0 + (j) * 3 * MaxSetDimX], x);
			cg = (uint32)Cosine_Interpolate(tab[i * 3 + 1 + j * 3 * MaxSetDimX], tab[(255 - i) * 3 + 1 + (j) * 3 * MaxSetDimX], x);
			cb = (uint32)Cosine_Interpolate(tab[i * 3 + 2 + j * 3 * MaxSetDimX], tab[(255 - i) * 3 + 2 + (j) * 3 * MaxSetDimX], x);
			if (cr > 255)
				cr = 255;
			if (cg > 255)
				cg = 255;
			if (cb > 255)
				cb = 255;
			tab[i * 3 + 0 + j * 3 * MaxSetDimX] = cr;
			tab[i * 3 + 1 + j * 3 * MaxSetDimX] = cg;
			tab[i * 3 + 2 + j * 3 * MaxSetDimX] = cb;
		}
	}


	for (j = 0; j < MaxSetDimY; j++) {
		for (i = 0; i < MaxSetDimX; i++) {
			cr = tab[i * 3 + 0 + j * 3 * MaxSetDimX];
			cg = tab[i * 3 + 1 + j * 3 * MaxSetDimX];
			cb = tab[i * 3 + 2 + j * 3 * MaxSetDimX];
			p[i + j * pitch] = (short)rRGBAToTextureFormat(cr, cg, cb, 255);
		}
	}

	rUnlockSurfaceDirect(SkySurface);
	delete[] tab;

	if (t3dSky && t3dSky->MatTable.size() >= 2)
		t3dSky->MatTable[1]->Texture = (SkySurface);
}

/* -----------------10/06/99 15.57-------------------
 *              t3dCreateSmokeParticle
 * --------------------------------------------------*/
uint8 t3dCreateSmokeParticle(uint32 Num, uint8 Type, uint32 Opacity) {
	error("TODO: t3dCreateSmokeParticle");
#if 0
	t3dF32  v1, v2;
	int32  i, j, pitch, color;
	unsigned short *p;

	Particles[t3dNumParticles].Material = gMaterial();

	if ((SmokeSurface == nullptr) && (Type == 1)) {
		SmokeSurface = (gTexture *)rCreateSurface(16, 16, rTEXTURESURFACE);

		p = (unsigned short *)rLockSurfaceDirect(SmokeSurface, (unsigned int *)&pitch);
		pitch /= 2;

		for (j = 0; j < 16; j++) {
			for (i = 0; i < 16; i++) {
				v1 = (t3dF32)fabs(sin(DEGREE_TO_RADIANS(360 * i / 32)));
				v2 = (t3dF32)fabs(sin(DEGREE_TO_RADIANS(360 * j / 32)));
				color = (uint32)(v1 * v2 * Opacity);
//				color-=(rand()%(color+1)/2);
				if (color <= 3)
					p[i + j * pitch] = (short)rRGBAToTextureFormat(color, color, color, 0);
				else
					p[i + j * pitch] = (short)rRGBAToTextureFormat(color, color, color, 255);
			}
		}

		rUnlockSurfaceDirect(SmokeSurface);
	} else if ((NoiseSmokeSurface == nullptr) && (Type == 2)) {
		NoiseSmokeSurface = (gTexture *)rCreateSurface(16, 16, rTEXTURESURFACE);

		p = (unsigned short *)rLockSurfaceDirect(NoiseSmokeSurface, (unsigned int *)&pitch);
		pitch /= 2;

		for (j = 0; j < 16; j++) {
			for (i = 0; i < 16; i++) {
				v1 = (t3dF32)fabs(sin(DEGREE_TO_RADIANS(360 * i / 32)));
				v2 = (t3dF32)fabs(sin(DEGREE_TO_RADIANS(360 * j / 32)));
				color = (uint32)(v1 * v2 * Opacity);
				color -= (rand() % (color + 1) / 2);
				if (color <= 3)
					p[i + j * pitch] = (short)rRGBAToTextureFormat(color, color, color, 0);
				else
					p[i + j * pitch] = (short)rRGBAToTextureFormat(color, color, color, 255);
			}
		}

		rUnlockSurfaceDirect(NoiseSmokeSurface);
	}

	warning("Partially stubbed t3dCreateSmokeParticle");
#if 0
	if (Type == 1)
		Particles[t3dNumParticles].Material.Texture = (SmokeSurface);
	else
		Particles[t3dNumParticles].Material.Texture = (NoiseSmokeSurface);
	Particles[t3dNumParticles].Material.VB = rGetUserVertexBuffer();
#endif
	for (i = 0; i < 16; i++)
		Particles[t3dNumParticles].RandTable[i] = rand() % 3 - 1;

	Particles[t3dNumParticles].Material.addNumFaces(Num);
	Particles[t3dNumParticles].Material.addProperty(T3D_MATERIAL_SMOKE | T3D_MATERIAL_NOLIGHTMAP);
	Particles[t3dNumParticles].ParticlePos = (Particle *)t3dMalloc(sizeof(Particle) * Num);
	memset(Particles[t3dNumParticles].ParticlePos, 0, sizeof(Particle)*Num);
	Particles[t3dNumParticles].NumParticles = Num / 3;
	t3dNumParticles++;

#endif
	return t3dNumParticles - 1;
}

/* -----------------10/06/99 15.44-------------------
 *              t3dCalcVertsInterpolants
 * --------------------------------------------------*/
void t3dCalcVertsInterpolants(gVertex *oldptr, gVertex *newptr, t3dV3F *VI, uint32 num) {
#define NUMINTERPOLANTS (1.0f/255.0f)
	uint32 i;

	if (!oldptr || !newptr || !VI)
		return ;

	for (i = 0; i < num; i++, oldptr++, newptr++, VI++) {
		VI->x = (newptr->x - oldptr->x) * NUMINTERPOLANTS;
		VI->y = (newptr->y - oldptr->y) * NUMINTERPOLANTS;
		VI->z = (newptr->z - oldptr->z) * NUMINTERPOLANTS;
//		newptr->diffuse=oldptr->diffuse;
	}
}

/* -----------------10/06/99 15.45-------------------
 *                  t3dAddExpression
 * --------------------------------------------------*/
void t3dAddExpression(t3dMESH *mesh, uint32 Frame, gVertex *v) {
	if (Frame >= mesh->MorphFrames.size()) {
		if (!mesh->MorphFrames.empty())
			warning("t3dAddExpression() mesh %s  frame %d  mesh->NumMorphFrames %d", mesh->name.c_str(), Frame, mesh->MorphFrames.size());

		Frame = 0;
		return;
	}

	mesh->LastExpressionFrame = mesh->ExpressionFrame;

	for (auto &mv : mesh->MorphFrames[Frame]._morphModVertices) {
		gVertex *ap = &v[mv._index];
		ap->x = mv._v.x;
		ap->y = mv._v.y;
		ap->z = mv._v.z;
	}
}

/* -----------------10/06/99 15.45-------------------
 *                  CalcBones
 * --------------------------------------------------*/
void CalcBones(t3dMESH *mesh, t3dBONEANIM *Anim, int32 Frame) {
	t3dBONE     *bone;
	t3dM3X3F    *Matrix;
	t3dV3F      *Trasl, Appov;
	int32      i, cv;
	gVertex     *Newptr;
	uint8       *Average/*,first=0*/;
	uint32      memalloc = 0;
	t3dF32      InvAvg;
	gVertex     *gv;
	gVertex     *v;
//	t3dF32   x,y,z;

	if (Frame > Anim->NumFrames - 1)
		return;

	memalloc = (sizeof(uint8) * mesh->NumVerts);
	Average = (uint8 *)t3dAlloc(memalloc);
	memset(Average, 0, memalloc);

	if (!(Newptr = mesh->SavedVertexBuffer))
		return ;

	t3dAddExpression(mesh, mesh->ExpressionFrame, Newptr);

	//Search first valid vertex
	cv = 99999999;
	bone = Anim->BoneTable;
	for (i = 0; i < Anim->NumBones; i++, bone++) {
		if (!bone->Trasl || !bone->Matrix || (bone->ModVertices.size() > mesh->NumVerts)) continue;

		for (auto &modVertex : bone ->ModVertices) {
			if (modVertex < cv) {
				cv = modVertex;
			}
		}
	}
	if (cv == 99999999) {
		warning("Questa animazione non modifica vertici in %s", mesh->name.c_str());
		t3dDealloc(memalloc);
		return ;
	}

	/*  for ( i=0; i<mesh->NumVerts; i++ )
	    {
	        memcpy(&Newptr[i],&mesh->VBptr[cv+i].p,sizeof(t3dV3F));
	    }*/

	bone = Anim->BoneTable;
//	DebugFile("\nFrame %d",Frame);
	for (i = 0; i < Anim->NumBones; i++, bone++) {
		if (!bone->Trasl || !bone->Matrix || (bone->ModVertices.size() > mesh->NumVerts)) continue;

//		DebugFile("Bone %d",i);
		Matrix = &bone->Matrix[Frame];
		Trasl = &bone->Trasl[Frame];


		for (auto &modVertex : bone->ModVertices) {
//			DebugFile("%d",bone->ModVertices[k].Vertex);
			gv = &mesh->VBptr[modVertex];

//			t3dVectCopy(&Appov,&Newptr[bone->ModVertices[k].Vertex-cv]);
//			t3dVectSub(&Appov, &Appov, &bone->Trasl[0]);

			Appov.x = Newptr[modVertex - cv].x - bone->Trasl[0].x;
			Appov.y = Newptr[modVertex - cv].y - bone->Trasl[0].y;
			Appov.z = Newptr[modVertex - cv].z - bone->Trasl[0].z;
			t3dVectTransformInv(&Appov, &Appov, Matrix);

			if (Average[modVertex - cv] == 0) {
				gv->x = Appov.x + Trasl->x;
				gv->y = Appov.y + Trasl->y;
				gv->z = Appov.z + Trasl->z;
			} else {
				gv->x += Appov.x + Trasl->x;
				gv->y += Appov.y + Trasl->y;
				gv->z += Appov.z + Trasl->z;
			}
			Average[modVertex - cv] ++;
		}
	}



	v = &mesh->VBptr[0];
	for (i = 0; i < mesh->NumVerts; i++, v++, Average++, Newptr++) {
		if (!(mesh->Flags & T3D_MESH_CHARACTER))
			v->diffuse = Newptr->diffuse;
//		v->diffuse=RGBA_MAKE(255,255,255,255);
		if ((*Average) <= 1)
			continue;

		InvAvg = 1.0f / (*Average);

		v->x *= InvAvg;
		v->y *= InvAvg;
		v->z *= InvAvg;
	}

	/*  v=&mesh->VBptr[0];
	    for (i=0; i<mesh->NumVerts; i++, v++, Average++)
	    {
	        x = (v->x * mesh->Matrix.M[0]) + (v->y * mesh->Matrix.M[1]) + (v->z * mesh->Matrix.M[2]);
	        y = (v->x * mesh->Matrix.M[3]) + (v->y * mesh->Matrix.M[4]) + (v->z * mesh->Matrix.M[5]);
	        z = (v->x * mesh->Matrix.M[6]) + (v->y * mesh->Matrix.M[7]) + (v->z * mesh->Matrix.M[8]);
	        v->x=x;
	        v->y=y;
	        v->z=z;
	    }*/

	t3dDealloc(memalloc);

	t3dCalcVertsInterpolants(mesh->OldVertexBuffer, mesh->VBptr, mesh->VertsInterpolants, mesh->NumVerts);
}

/* -----------------10/06/99 15.46-------------------
 *                  t3dAddBlend
 * --------------------------------------------------*/
void t3dAddBlend(t3dF32 AddPercent, t3dMESH *mesh) {
	gVertex *gv;
	t3dV3F  *VI;
	uint32  i;

	if (!mesh->VBptr)
		return;

	gv = mesh->VBptr;
	VI = mesh->VertsInterpolants;
	for (i = 0; i < mesh->NumVerts; i++, gv++, VI++) {
		gv->x += VI->x * AddPercent;
		gv->y += VI->y * AddPercent;
		gv->z += VI->z * AddPercent;
	}
}

/* -----------------10/06/99 15.46-------------------
 *                  t3dCalcMeshBones
 * --------------------------------------------------*/
void t3dCalcMeshBones(t3dMESH *mesh, int32 last) {
	int16 TempFrame = 0;
	if (!mesh) return;

	if (last) {
		TempFrame = mesh->CurFrame;
		if (mesh->Flags & T3D_MESH_DEFAULTANIM)
			mesh->CurFrame = mesh->DefaultAnim.NumFrames - 1;
		else
			mesh->CurFrame = mesh->Anim.NumFrames - 1;
	}

	if ((mesh->CurFrame > 0) || (mesh->ExpressionFrame != mesh->LastExpressionFrame)) {
		if ((mesh->LastFrame != mesh->CurFrame) || (mesh->ExpressionFrame != mesh->LastExpressionFrame) ||
		        ((mesh->Flags & T3D_MESH_LAST_DEFAULTANIM) && !(mesh->Flags & T3D_MESH_DEFAULTANIM)) ||
		        (!(mesh->Flags & T3D_MESH_LAST_DEFAULTANIM) && (mesh->Flags & T3D_MESH_DEFAULTANIM))) {
			if (mesh->LastFrame != mesh->CurFrame)
				mesh->LastFrame = mesh->CurFrame;

			if (!(mesh->VBptr = mesh->VertexBuffer) || !mesh->OldVertexBuffer)
				return;

			memcpy(mesh->OldVertexBuffer, mesh->VertexBuffer, mesh->NumVerts * sizeof(gVertex));

			mesh->Flags |= T3D_MESH_UPDATEVB;
			if (mesh->Flags & T3D_MESH_DEFAULTANIM) {
				mesh->Flags |= T3D_MESH_LAST_DEFAULTANIM;
				CalcBones(mesh, &mesh->DefaultAnim, mesh->CurFrame);
				if (mesh->CurFrame > mesh->DefaultAnim.NumFrames - 1)
					mesh->CurFrame = 0;
			} else {
				mesh->Flags &= ~T3D_MESH_LAST_DEFAULTANIM;
				CalcBones(mesh, &mesh->Anim, mesh->CurFrame);
				if (mesh->CurFrame > mesh->Anim.NumFrames - 1)
					mesh->CurFrame = 0;
			}

			mesh->LastBlendPercent = 255;
			mesh->VBptr = nullptr;
		}

		if (mesh->LastBlendPercent != mesh->BlendPercent) {
			if (!(mesh->VBptr = mesh->VertexBuffer))
				return;

			mesh->Flags |= T3D_MESH_UPDATEVB;
			if (!mesh->BlendPercent) {
				gVertex *OldVBptr;
				if (!(OldVBptr = mesh->OldVertexBuffer))
					return ;
				memcpy(mesh->VBptr, OldVBptr, sizeof(gVertex)*mesh->NumVerts);
			} else
				t3dAddBlend((t3dF32)(mesh->BlendPercent - mesh->LastBlendPercent), mesh);

			mesh->LastBlendPercent = mesh->BlendPercent;
			mesh->VBptr = nullptr;
		}
	}

	if (last) mesh->CurFrame = TempFrame;
}

/* -----------------10/06/99 15.49-------------------
 *              t3dRotateMoveCamera
 * --------------------------------------------------*/
void t3dRotateMoveCamera(t3dCAMERA *cam, t3dF32 AngleX, t3dF32 AngleY, t3dF32 AngleSpeed) {
	t3dV3F      dir, sdir, tmp;
//	t3dF32       /*angley,angle*/;
	t3dM3X3F    mx;

	if ((AngleX == 0.0f) && (AngleY == 0.0f) && (!AngleSpeed)) return;
	/*
	    t3dVectSub(&dir,&cam->Target,&cam->Source);
	    angle=(t3dF32)sqrt(dir.x*dir.x+dir.z*dir.z);

	    cam->Target.y = cam->Source.y + dir.y*(t3dF32)cos(-AngleX) + angle*(t3dF32)sin(-AngleX);
	//  cam->Target.x = cam->Source.x - dir.y*(t3dF32)sin(-AngleX) + angle*(t3dF32)cos(-AngleX);
	//  cam->Target.z = cam->Source.z - dir.y*(t3dF32)sin(-AngleX) + angle*(t3dF32)cos(-AngleX);

	    cam->Target.x = cam->Source.x + dir.x*(t3dF32)cos(AngleY) + dir.z*(t3dF32)sin(AngleY);
	    cam->Target.z = cam->Source.z - dir.x*(t3dF32)sin(AngleY) + dir.z*(t3dF32)cos(AngleY);
	*/

	t3dMatView(&cam->Matrix, &cam->Source, &cam->Target);
	t3dMatRot(&mx, AngleX, AngleY, 0);
	t3dVectSub(&dir, &cam->Target, &cam->Source);
	t3dVectTransform(&sdir, &dir, &cam->Matrix);
	t3dVectTransform(&sdir, &sdir, &mx);
	t3dVectTransformInv(&sdir, &sdir, &cam->Matrix);
	t3dVectNormalize(&sdir);
	t3dVectCopy(&tmp, &sdir);
	sdir *= AngleSpeed;
	t3dVectAdd(&cam->Source, &cam->Source, &sdir);
	t3dVectAdd(&cam->Target, &cam->Source, &tmp);
}

/* -----------------10/06/99 15.49-------------------
 *                  t3dResetPipeline
 * --------------------------------------------------*/
void t3dResetPipeline() {
	t3dMESH *m;
	uint32 i, j;

	rResetPipeline();
	t3dStartIndex = t3dNumVertices = 0;
	t3d_NumMeshesVisible = 0;
	t3dNumGlobalMirrors = 0;
	t3dNumMaterialLists = 0;
	StatNumTris = 0;
	StatNumVerts = 0;

	for (i = 0; i < t3dNumPortals; i++) {
		if ((!t3dPortalList[i]) || (!t3dPortalList[i]->PortalList)) continue;

		for (j = 0, m = &t3dPortalList[i]->PortalList->MeshTable[0]; j < t3dPortalList[i]->PortalList->NumMeshes(); j++, m++)
			m->Flags &= ~T3D_MESH_PORTALPROCESSED;
	}

	t3dNumPortals = 0;
	if (PortalCrossed) {
		t3dCurRoom = PortalCrossed;
		PortalCrossed = nullptr;
	}
}


/* -----------------10/06/99 15.58-------------------
 *              t3dReleaseParticles
 * --------------------------------------------------*/
void t3dReleaseParticles() {
	for (int i = 0; i < MAX_PARTICLES; i++) {
		t3dFree(Particles[i].ParticlePos);
		Particles[i].Material.clear();
	}
}

/* -----------------10/06/99 15.46-------------------
 *                  t3dResetMesh
 * --------------------------------------------------*/
void t3dResetMesh(t3dMESH *mesh) {
	if (!mesh->VertexBuffer || !mesh->OldVertexBuffer || !mesh->SavedVertexBuffer)
		return ;

	memcpy(mesh->VertexBuffer, mesh->SavedVertexBuffer, sizeof(gVertex)*mesh->NumVerts);
	memcpy(mesh->OldVertexBuffer, mesh->SavedVertexBuffer, sizeof(gVertex)*mesh->NumVerts);
	mesh->Flags |= T3D_MESH_UPDATEVB;
}

/* -----------------10/06/99 15.49-------------------
 *                      CheckAndClip
 * --------------------------------------------------*/
uint8 CheckAndClip(t3dV3F *start, t3dV3F *end, t3dNORMAL *n) {
	t3dF32 divi;
	t3dF32 d1 = t3dVectDot(start, &n->n) - n->dist;
	t3dF32 d2 = t3dVectDot(end, &n->n) - n->dist;

	if ((d1 < 0) && (d2 < 0))
		return 0;
	else if ((d1 >= 0) && (d2 >= 0))
		return 1;


	if ((d1 < 0) && (d2 >= 0)) {
		d2 = d2 - d1;
		divi = -d1 / d2;
		start->x = start->x + divi * (end->x - start->x);
		start->y = start->y + divi * (end->y - start->y);
		start->z = start->z + divi * (end->z - start->z);
		return 1;
	} else {
		d1 = d1 - d2;
		divi = -d2 / d1;
		end->x = end->x + divi * (start->x - end->x);
		end->y = end->y + divi * (start->y - end->y);
		end->z = end->z + divi * (start->z - end->z);
		return 2;
	}
}

/* -----------------10/06/99 15.50-------------------
 *                  t3dCheckWithFloor
 * --------------------------------------------------*/
t3dF32 t3dCheckWithFloor() {
	extern t3dF32 CurFloorY;
	t3dVERTEX a, b, c;
	t3dNORMAL bbn;
	t3dV3F  StartDir, EndDir, Sight, pos, en, st;
	t3dF32  MaxX, AverageZ = -99999999999.9f;

	a.p.x = -10;
	a.p.y = CurFloorY + 15;
	a.p.z = 10;
	b.p.x = 10;
	b.p.y = CurFloorY + 15;
	b.p.z = 10;
	c.p.x = 10;
	c.p.y = CurFloorY + 15;
	c.p.z = -10;
	t3dPlaneNormal(&bbn, &a.p, &b.p, &c.p); //floor

	t3dVectCopy(&StartDir, &t3dCurCamera->Source);
	t3dVectTransformInv(&EndDir, &t3d3dMousePos, &t3dCurViewMatrix);
	t3dVectAdd(&EndDir, &EndDir, &t3dCurCamera->Source);
//	t3dVectSub(&EndDir,&EndDir,&mesh->Pos);

	t3dVectSub(&Sight, &EndDir, &StartDir);
	t3dVectNormalize(&Sight);
	Sight *= 2000000.0f;
	t3dVectAdd(&EndDir, &StartDir, &Sight);


	if (CheckAndClip(&StartDir, &EndDir, &bbn)) {
		pos.x = - t3dCurCamera->Source.x;   // posizione centro mesh dalla camera
		pos.y = - t3dCurCamera->Source.y;
		pos.z = - t3dCurCamera->Source.z;
		t3dVectTransform(&pos, &pos, &t3dCurViewMatrix); // rotate by the mesh matrix

//		t3dVectTransform(&StartDir,&StartDir,&t3dCurCamera->Matrix);
//		t3dVectTransform(&EndDir,&EndDir,&t3dCurCamera->Matrix);

		t3dVectCopy(&st, &StartDir);
		t3dVectCopy(&en, &EndDir);
		t3dVectTransform(&StartDir, &StartDir, &t3dCurViewMatrix);
		t3dVectTransform(&EndDir, &EndDir, &t3dCurViewMatrix);
		t3dVectAdd(&StartDir, &StartDir, &pos);
		t3dVectAdd(&EndDir, &EndDir, &pos);

		if ((MaxX = t3dPointSquaredDistance(&StartDir)) > AverageZ) {
			t3dVectCopy(&FloorHitCoords, &st);
			AverageZ = MaxX;
		}
		if ((MaxX = t3dPointSquaredDistance(&EndDir)) > AverageZ) {
			t3dVectCopy(&FloorHitCoords, &en);
			AverageZ = MaxX;
		}

		return AverageZ;
	}

	return 999999999999.9f;
}

/* -----------------10/06/99 15.43-------------------
 *                  t3dProcessMirror
 * --------------------------------------------------*/
void t3dProcessMirror(t3dMESH *mesh, t3dCAMERA *cam) {
	warning("STUBBED: t3dProcessMirror");
#if 0
//	t3dBODY      *NewBody=NULL;
	t3dV3F      appo, c0;
	t3dV3F      ppv1, ppv2, ppv3, ppv4/*,eye,pos,oldtrasl,oldpos*/;
	t3dU32      p/*,k*/;
	t3dNORMAL   OldClipPlanes[NUMCLIPPLANES];
	t3dM3X3F    ReflMatrix;
	t3dV3F      mirrorpos;
//	t3dMESH      *m;
	t3dM3X3F    OldViewMatrix;
	t3dCAMERA   OldCurCamera;
	gMaterial   *OldMaterialTable;

	if (!mesh->VertexBuffer)
		return ;

	mesh->VBptr = mesh->VertexBuffer;
	ppv1.x = mesh->VBptr[mesh->FList[0].VertexIndex[0]].x;
	ppv1.y = mesh->VBptr[mesh->FList[0].VertexIndex[0]].y;
	ppv1.z = mesh->VBptr[mesh->FList[0].VertexIndex[0]].z;
	ppv2.x = mesh->VBptr[mesh->FList[0].VertexIndex[1]].x;
	ppv2.y = mesh->VBptr[mesh->FList[0].VertexIndex[1]].y;
	ppv2.z = mesh->VBptr[mesh->FList[0].VertexIndex[1]].z;
	ppv3.x = mesh->VBptr[mesh->FList[0].VertexIndex[2]].x;
	ppv3.y = mesh->VBptr[mesh->FList[0].VertexIndex[2]].y;
	ppv3.z = mesh->VBptr[mesh->FList[0].VertexIndex[2]].z;

	for (p = 0; p < 3; p++) {
		appo.x = mesh->VBptr[mesh->FList[1].VertexIndex[p]].x;
		appo.y = mesh->VBptr[mesh->FList[1].VertexIndex[p]].y;
		appo.z = mesh->VBptr[mesh->FList[1].VertexIndex[p]].z;

		if ((!t3dVectCmp(&ppv1, &appo)) &&
		        (!t3dVectCmp(&ppv2, &appo)) &&
		        (!t3dVectCmp(&ppv3, &appo)))
			memcpy(&ppv4, &appo, sizeof(t3dV3F));
	}

	memcpy(OldClipPlanes, ClipPlanes, sizeof(t3dNORMAL)*NUMCLIPPLANES);

	t3dVectSub(&ppv4, &ppv4, &cam->Source);
	t3dVectSub(&ppv3, &ppv3, &cam->Source);
	t3dVectSub(&ppv2, &ppv2, &cam->Source);
	t3dVectSub(&ppv1, &ppv1, &cam->Source);
	t3dVectTransform(&ppv1, &ppv1, &t3dCurViewMatrix);
	t3dVectTransform(&ppv2, &ppv2, &t3dCurViewMatrix);
	t3dVectTransform(&ppv3, &ppv3, &t3dCurViewMatrix);
	t3dVectTransform(&ppv4, &ppv4, &t3dCurViewMatrix);
	t3dVectFill(&c0, 0.0f);

//	t3dVectCopy(&c0,&cam->Source);

	t3dPlaneNormal(&ClipPlanes[RIGHTCLIP],  &c0, &ppv3, &ppv4);
	t3dPlaneNormal(&ClipPlanes[LEFTCLIP],   &c0, &ppv1, &ppv2);
	t3dPlaneNormal(&ClipPlanes[TOPCLIP],    &c0, &ppv4, &ppv1);
	t3dPlaneNormal(&ClipPlanes[BOTTOMCLIP], &c0, &ppv2, &ppv3);

	mesh->VBptr = NULL;

	t3dMatReflect(&ReflMatrix, &mirrorpos, mesh->FList[0].n);
//	t3dVectAdd(&mirrorpos,&mirrorpos,&mesh->Pos);

	bDisableMirrors = 1;
	t3dMatCopy(&OldViewMatrix, &t3dCurViewMatrix);
	memcpy(&OldCurCamera, t3dCurCamera, sizeof(t3dCAMERA));
	{
		t3dU32 i, j;
		t3dMESH *m;
		t3dFACE *f;
		for (i = 0, m = t3dCurRoom->MeshTable; i < t3dCurRoom->NumMeshes; i++, m++)
			for (j = 0, f = m->FList; j < m->NumFaces; j++, f++)
				if (f->mat)
					f->mat = (gMaterial *)((t3dU8 *)f->mat - (t3dU8 *)t3dCurRoom->MatTable + (t3dU8 *)t3dCurRoom->MirrorMatTable);
		OldMaterialTable = t3dCurRoom->MatTable;
		t3dCurRoom->MatTable = t3dCurRoom->MirrorMatTable;
	}

	t3dVectAdd(&t3dCurCamera->Source, &t3dCurCamera->Source, &mirrorpos);
	t3dVectTransform(&t3dCurCamera->Source, &t3dCurCamera->Source, &ReflMatrix);
	t3dVectAdd(&t3dCurCamera->Target, &t3dCurCamera->Target, &mirrorpos);
	t3dVectTransform(&t3dCurCamera->Target, &t3dCurCamera->Target, &ReflMatrix);

	t3dMatMul(&t3dCurViewMatrix, &t3dCurViewMatrix, &ReflMatrix);


	if (!t3dTransformBody(t3dCurRoom)) {
		DebugLogWindow("Can't transform %s", mesh->PortalList->Name);
	}

	bDisableMirrors = 0;
	t3dMatCopy(&t3dCurViewMatrix, &OldViewMatrix);
	memcpy(ClipPlanes, OldClipPlanes, sizeof(t3dNORMAL)*NUMCLIPPLANES);
	{
		t3dU32 i, j;
		t3dMESH *m;
		t3dFACE *f;
		t3dCurRoom->MatTable = OldMaterialTable;
		for (i = 0, m = t3dCurRoom->MeshTable; i < t3dCurRoom->NumMeshes; i++, m++)
			for (j = 0, f = m->FList; j < m->NumFaces; j++, f++)
				if (f->mat)
					f->mat = (gMaterial *)((t3dU8 *)f->mat - (t3dU8 *)t3dCurRoom->MirrorMatTable + (t3dU8 *)t3dCurRoom->MatTable);
	}
	memcpy(t3dCurCamera, &OldCurCamera, sizeof(t3dCAMERA));
#endif
}

/* -----------------10/06/99 15.51-------------------
 *                  t3dProcessMirrors
 * --------------------------------------------------*/
void t3dProcessMirrors(t3dMESH **MirrorList, uint32 NumMirrors) {
	uint32 i;

	for (i = 0; i < NumMirrors; i++)
		t3dProcessMirror(MirrorList[i], t3dCurCamera);
}

#define _ADDLINES(x)    *ptr=num+x; ptr++;  rAddLinesArray()
/* -----------------02/06/99 16.53-------------------
 *                  t3dShowBoundingBox
 * --------------------------------------------------*/
void t3dShowBoundingBox(t3dBODY *b) {
	t3dV3F  tmp;
	uint16    *ptr;
	uint16  num;

	gVertex *VertPointer = (gVertex *)rLockPointArray();

	ptr = rGetLinesArrayPtr();
	num = rGetNumPointArray();

	tmp.x = -t3dCurCamera->Source.x;
	tmp.y = -t3dCurCamera->Source.y;
	tmp.z = -t3dCurCamera->Source.z;
	t3dVectTransform(&tmp, &tmp, &t3dCurViewMatrix);
	rBuildLinesViewMatrix(t3dCurViewMatrix, tmp);

	for (uint32 i = 0; i < b->NumMeshes(); i++) {
		t3dMESH &mesh = b->MeshTable[i];
		if (mesh.Flags & (T3D_MESH_PORTAL | T3D_MESH_HIDDEN)) {
			continue;
		}

		for (uint32 j = 0; j < 8; j++) {
			VertPointer->x = mesh.BBox[j].p.x + mesh.Trasl.x;
			VertPointer->y = mesh.BBox[j].p.y + mesh.Trasl.y;
			VertPointer->z = mesh.BBox[j].p.z + mesh.Trasl.z;
			VertPointer->diffuse = RGBA_MAKE(0, 0, 255, 255);
			rAddPointArray();
			VertPointer++;
//			t3dNumVertices++;
		}

		_ADDLINES(0);
		_ADDLINES(1);

		_ADDLINES(1);
		_ADDLINES(3);

		_ADDLINES(3);
		_ADDLINES(2);

		_ADDLINES(2);
		_ADDLINES(0);

		_ADDLINES(4);
		_ADDLINES(5);

		_ADDLINES(5);
		_ADDLINES(7);

		_ADDLINES(7);
		_ADDLINES(6);

		_ADDLINES(6);
		_ADDLINES(4);

		_ADDLINES(4);
		_ADDLINES(0);

		_ADDLINES(6);
		_ADDLINES(2);

		_ADDLINES(7);
		_ADDLINES(3);

		_ADDLINES(5);
		_ADDLINES(1);

		num += 8;
	}

	rUnlockPointArray();
}

/* -----------------02/06/99 16.54-------------------
 *                  t3dShowBounds
 * --------------------------------------------------*/
void t3dShowBounds(t3dPAN *p, uint32 numpan) {
	t3dV3F  tmp;
	uint32  j;
	uint16    *ptr;
	uint16  num;
	gVertex *VertPointer = (gVertex *)rLockPointArray();
	ptr = rGetLinesArrayPtr();
	num = rGetNumPointArray();

	tmp.x = -t3dCurCamera->Source.x;
	tmp.y = -t3dCurCamera->Source.y;
	tmp.z = -t3dCurCamera->Source.z;
	t3dVectTransform(&tmp, &tmp, &t3dCurViewMatrix);
	rBuildLinesViewMatrix(t3dCurViewMatrix, tmp);

	for (j = 0; j < numpan; j++) {
		VertPointer->x = p[j].a.x;
		VertPointer->y = CurFloorY + 1.0f;
		VertPointer->z = p[j].a.z;
		VertPointer->diffuse = RGBA_MAKE(250, 0, 0, 255);
		rAddPointArray();
		VertPointer++;
//		t3dNumVertices++;

		VertPointer->x = p[j].b.x;
		VertPointer->y = CurFloorY + 1.0f;
		VertPointer->z = p[j].b.z;
		VertPointer->diffuse = RGBA_MAKE(250, 0, 0, 255);
		rAddPointArray();
		VertPointer++;
//		t3dNumVertices++;

		VertPointer->x = p[j].backA.x;
		VertPointer->y = CurFloorY + 1.0f;
		VertPointer->z = p[j].backA.z;
		VertPointer->diffuse = RGBA_MAKE(250, 0, 0, 255);
		rAddPointArray();
		VertPointer++;
//		t3dNumVertices++;

		VertPointer->x = p[j].backB.x;
		VertPointer->y = CurFloorY + 1.0f;
		VertPointer->z = p[j].backB.z;
		VertPointer->diffuse = RGBA_MAKE(250, 0, 0, 255);
		rAddPointArray();
		VertPointer++;
//		t3dNumVertices++;

		_ADDLINES(0);
		_ADDLINES(1);

		_ADDLINES(0);
		_ADDLINES(2);

		_ADDLINES(1);
		_ADDLINES(3);

		num += 4;

	}

	rUnlockPointArray();
}

/* -----------------10/06/99 15.51-------------------
 *              t3dCalcVolumetricLights
 * --------------------------------------------------*/
void t3dCalcVolumetricLights(t3dMESH *m, t3dBODY *body) {
	uint32  i;
	t3dF32  invcellssize;
	int32  x, y, z;
	uint8   val;
//	t3dS32   val1,val2,val3,tval;
	int32  xcells;
	int32  ycells;
	int32  zcells;
	t3dF32  xcells_rest, ycells_rest, zcells_rest;
	gVertex *gv;
	uint8   r, g, b;
	t3dV3F  vect, sco;

	if (!body->VolumetricLights) return;
	if (!(gv = m->VBptr)) return;

	invcellssize = 1.0f / body->VolumetricLights->CellsSize;
	xcells = body->VolumetricLights->xcells;
	ycells = body->VolumetricLights->ycells;
	zcells = body->VolumetricLights->zcells;
	t3dVectSub(&sco, &m->Trasl, &body->MinPos);

	for (i = 0; i < m->NumVerts; i++, gv++) {
		t3dVectInit(&vect, gv->x, gv->y, gv->z);
		t3dVectTransform(&vect, &vect, &m->Matrix);
		t3dVectAdd(&vect, &vect, &sco);

		x = (int32)(xcells_rest = (vect.x * invcellssize));
		y = (int32)(ycells_rest = (vect.y * invcellssize));
		z = (int32)(zcells_rest = (vect.z * invcellssize));

		if ((x < 0) || (y < 0) || (z < 0) || (x > xcells) || (y > ycells) || (z > zcells)) {
			gv->diffuse = RGBA_MAKE(128, 128, 128, 255);
			continue;
		}

		val = body->VolumetricLights->VolMap[x + (z * xcells) + (y * xcells * zcells)];

		r = (uint8)(((RGBA_GETRED(gv->diffuse) * val) >> 8) & 0xFF);
		g = (uint8)(((RGBA_GETGREEN(gv->diffuse) * val) >> 8) & 0xFF);
		b = (uint8)(((RGBA_GETBLUE(gv->diffuse) * val) >> 8) & 0xFF);

		gv->diffuse = RGBA_MAKE(r, g, b, RGBA_GETALPHA(gv->diffuse));
	}
}


/* -----------------10/06/99 15.53-------------------
 *                  t3dLightCharacter
 * --------------------------------------------------*/
void t3dLightCharacter(t3dCHARACTER *Ch) {
	t3dMESH     *mesh;
	uint32      j;
	t3dV3F      l;
	t3dF32      dist, far_range, near_range, AttenIntensity;
	t3dF32      ang, half_hotspot, half_falloff, SpotIntensity;
	t3dF32      nlight, Intensity;
	t3dV3F      ppos;
	t3dV3F      *normal, dir;
	int32      rr, gg, bb, arr, agg, abb;
	gVertex     *gv;
	DWORD       amb;

	mesh = Ch->Mesh;

	if (!mesh || !mesh->VertexBuffer)
		return ;

	gv = mesh->VBptr = mesh->VertexBuffer;
	mesh->Flags |= T3D_MESH_UPDATEVB;

	arr = (uint32)(Ch->Body->AmbientLight.x + t3dCurRoom->AmbientLight.x);
	agg = (uint32)(Ch->Body->AmbientLight.y + t3dCurRoom->AmbientLight.y);
	abb = (uint32)(Ch->Body->AmbientLight.z + t3dCurRoom->AmbientLight.z);
	if (arr > 255) arr = 255;
	if (agg > 255) agg = 255;
	if (abb > 255) abb = 255;
	amb = RGBA_MAKE(arr, agg, abb, 255);
	for (j = 0; j < mesh->NumVerts; j++, gv++)
		gv->diffuse = amb;

	const auto &light = Ch->CurRoom->LightTable[0];
	if (light.Type & T3D_LIGHT_ALLLIGHTSOFF) {
		mesh->VBptr = nullptr;
		return;
	}
	t3dVectAdd(&ppos, &mesh->Trasl, &mesh->Pos);

	//for (uint32 i = 0; i < Ch->CurRoom->NumLights(); i++, lt++) {
	for (auto &lt : Ch->CurRoom->LightTable) {
		if (!(lt.Type & T3D_LIGHT_LIGHTON)) continue;
		if (!(lt.Type & T3D_LIGHT_REALTIME)) continue;
		if ((lt.Type & T3D_LIGHT_FLARE)) continue;

		AttenIntensity = 1.0f;
		if (lt.Type & T3D_LIGHT_ATTENUATION) {
			near_range = lt.NearRange;
			far_range = lt.FarRange;

			dist = t3dVectDistance(&lt.Source, &ppos);
			if (dist >  far_range)
				continue;
			else if (dist > near_range)
				AttenIntensity = (1.0f - ((dist - near_range) / (far_range - near_range)));
		}
		SpotIntensity = 1.0f;
		if (lt.Type & T3D_LIGHT_SPOTLIGHT) {
			half_hotspot = DEGREE_TO_RADIANS(lt.HotSpot) * 0.5f;
			half_falloff = DEGREE_TO_RADIANS(lt.FallOff) * 0.5f;

			t3dVectSub(&l, &ppos, &lt.Source);
			t3dVectNormalize(&l);
			t3dVectSub(&dir, &lt.Target, &lt.Source);
			t3dVectNormalize(&dir);

			ang = (t3dF32)acos(t3dVectDot(&dir, &l));
			if (ang > half_falloff)
				continue;
			else if (ang > half_hotspot)
				SpotIntensity = (1.0f - ((ang - half_hotspot) / (half_falloff - half_hotspot)));
		}

		t3dVectSub(&l, &lt.Source, &ppos);
		t3dVectTransformInv(&l, &l, &mesh->Matrix);
		t3dVectNormalize(&l);

		Intensity = AttenIntensity * SpotIntensity;
		gv = mesh->VBptr;
		for (j = 0; j < mesh->NumVerts; j++, gv++) {
			normal = &mesh->NList[j]->n;
			if ((nlight = t3dVectDot(normal, &l)) >= 0) {
				nlight *= Intensity;
				rr = t3dFloatToInt(lt.Color.x * nlight) + RGBA_GETRED(gv->diffuse);
				gg = t3dFloatToInt(lt.Color.y * nlight) + RGBA_GETGREEN(gv->diffuse);
				bb = t3dFloatToInt(lt.Color.z * nlight) + RGBA_GETBLUE(gv->diffuse);
				if (rr > 255) rr = 255;
				if (gg > 255) gg = 255;
				if (bb > 255) bb = 255;
				gv->diffuse = RGBA_MAKE(rr, gg, bb, 255);
			}
		}
	}

	if (Ch->Flags & T3D_CHARACTER_VOLUMETRICLIGHTING)
		t3dCalcVolumetricLights(mesh, Ch->CurRoom);

	mesh->VBptr = nullptr;
}

/* -----------------10/06/99 15.50-------------------
 *                  CheckInBoundBox
 * --------------------------------------------------*/
uint8 CheckInBoundBox(t3dV3F *start, t3dV3F *end, t3dVERTEX *bbv, t3dNORMAL *bbn) {
	if (!CheckAndClip(start, end, &bbn[0])) return 0;
	if (!CheckAndClip(start, end, &bbn[1])) return 0;
	if (!CheckAndClip(start, end, &bbn[2])) return 0;
	if (!CheckAndClip(start, end, &bbn[3])) return 0;
	if (!CheckAndClip(start, end, &bbn[4])) return 0;
	if (!CheckAndClip(start, end, &bbn[5])) return 0;

	return 1;
}

/* -----------------10/06/99 15.50-------------------
 *                  t3dDetectObj
 * --------------------------------------------------*/
uint8 t3dDetectObj(t3dCAMERA *cam, t3dMESH *mesh, t3dM3X3F *matrix) {
	t3dV3F  StartDir, EndDir, Sight, pos;
	t3dF32  MaxX;

	if (mesh->Flags & T3D_MESH_NOBOUNDBOX)
		return 0;

	t3dVectTransformInv(&EndDir, &t3d3dMousePos, &t3dCurViewMatrix);
	t3dVectAdd(&EndDir, &EndDir, &cam->Source);

	t3dVectCopy(&StartDir, &cam->Source);
	t3dVectSub(&Sight, &EndDir, &StartDir);
	t3dVectNormalize(&Sight);
	Sight *= 2000000.0f;
	t3dVectAdd(&EndDir, &StartDir, &Sight);
	mesh->BBoxAverageZ = 9999999999.9f;

	t3dVectInit(&pos, 0.0f, 0.0f, 0.0f);
	if (mesh->Flags & T3D_MESH_CHARACTER) {
		t3dVectInit(&pos, mesh->Trasl.x, CurFloorY, mesh->Trasl.z);
		t3dVectSub(&StartDir, &StartDir, &pos);
		t3dVectSub(&EndDir, &EndDir, &pos);
	}

	if (CheckInBoundBox(&StartDir, &EndDir, &mesh->BBox[0], &mesh->BBoxNormal[0]) && (t3d_NumMeshesVisible < 255)) {
		t3dV3F  st, en;

		pos.x -= cam->Source.x; // posizione centro mesh dalla camera
		pos.y -= cam->Source.y;
		pos.z -= cam->Source.z;
		t3dVectTransform(&pos, &pos, &t3dCurViewMatrix); // rotate by the mesh matrix

		t3dVectCopy(&st, &StartDir);
		t3dVectCopy(&en, &EndDir);
		t3dVectTransform(&StartDir, &StartDir, &t3dCurViewMatrix);
		t3dVectTransform(&EndDir, &EndDir, &t3dCurViewMatrix);
		t3dVectAdd(&StartDir, &StartDir, &pos);
		t3dVectAdd(&EndDir, &EndDir, &pos);
		MaxX = t3dPointSquaredDistance(&StartDir);
		if ((MaxX) < mesh->BBoxAverageZ) {
			mesh->BBoxAverageZ = MaxX;
			t3dVectCopy(&mesh->Intersection, &st);
		}
		MaxX = t3dPointSquaredDistance(&EndDir);
		if ((MaxX) < mesh->BBoxAverageZ) {
			mesh->BBoxAverageZ = MaxX;
			t3dVectCopy(&mesh->Intersection, &en);
		}

		t3d_NumMeshesVisible++;
		t3d_VisibleMeshes[t3d_NumMeshesVisible - 1] = mesh;

		return 1;
	} else
		return 0;
}

/* -----------------10/06/99 15.42-------------------
 *              t3dSetVisibileVertex
 * --------------------------------------------------*/
void t3dSetVisibileVertex(t3dMESH &mesh) {
//	if (mesh->Flags&T3D_MESH_PORTAL)
//		return ;

	for (uint32 i = 0; i < mesh.NumFaces() ; i++) {
		t3dFACE &f = mesh.FList[i];
		if ((/*(!(f->flags&T3D_MATERIAL_OPACITY)) &&*/ (!(f.flags & T3D_MATERIAL_CLIPMAP)) &&
		        (!(f.flags & T3D_MATERIAL_ADDITIVE))) &&
		        (!(f.flags & T3D_MATERIAL_GLASS))) {
			if (f.isVisible())
				f.flags |= T3D_FACE_VISIBLE;
			else
				f.flags &= ~(T3D_FACE_VISIBLE);                                                // continue
			/*          if ((f->flags&T3D_MATERIAL_PORTAL))
			                if (f->flags&T3D_FACE_VISIBLE)
			                    f->flags&=~T3D_FACE_VISIBLE;
			                else
			                    f->flags|=T3D_FACE_VISIBLE;*/
		} else
			f.flags |= T3D_FACE_VISIBLE;
	}
}

/* -----------------18/06/99 10.46-------------------
 *              t3dAddTextureBufferShadow
 * --------------------------------------------------*/
void t3dAddTextureBufferShadow(t3dCHARACTER *c, uint32 CurShadowBox, t3dLIGHT *light) {
	warning("TODO: t3dAddTextureBufferShadow");
	return;
#if 0
	SHADOWBOX   *sb = c->ShadowBox[CurShadowBox];
	SHADOW      *shad = &sb->ShadowsList[0];
	uint32      i, k, ff, kkk, StartVert;
	uint8       *ProjectedPolys, *pp;
	t3dMESH     *cm = c->Mesh;
	t3dBODY     *cr = t3dCurRoom; //c->CurRoom;
	pVert       tempv, *pv;
	WORD        *ptr = nullptr;
	gVertex     *gv, *pc;
	t3dV3F      v, tmp, MinSco, MaxSco, Aspect, BBox[8], dir;

	if (!cm || !cm->VertexBuffer)
		return ;
//tb
	t3dMatView(&t3dCurViewMatrix, &light->Source, &cm->Trasl, 0.0f);                             // punta la luce ai piedi
	t3dVectSub(&t3dCurTranslation, &cr->MeshTable[0].Trasl, &light->Source);
	t3dVectTransform(&t3dCurTranslation, &t3dCurTranslation, &t3dCurViewMatrix);                 // rotate by the camera matrix

	t3dBackfaceCulling(cr->NList, cr->NumNormals, &light->Source);                           // setta le normali che sono backface la luce

	if (shad->totalverts < cr->NumTotVerts) {
		rDeleteVertexBuffer(shad->ProjVertsVB);
		if (shad->pwShadVolCapIndices) t3dFree(shad->pwShadVolCapIndices);
		shad->ProjVertsVB = rCreateVertexBuffer(cr->NumTotVerts);
		shad->pwShadVolCapIndices = t3dCalloc<WORD>(cr->NumTotVerts * 3);
		shad->totalverts = cr->NumTotVerts;
	} else {
		if (!shad->ProjVertsVB) shad->ProjVertsVB = rCreateVertexBuffer(cr->NumTotVerts);
		if (!shad->pwShadVolCapIndices) shad->pwShadVolCapIndices = t3dCalloc<WORD>(cr->NumTotVerts * 3);
		shad->totalverts = cr->NumTotVerts;
	}
	ProjectedPolys = (t3dU8 *)t3dAlloc(sizeof(t3dU8) * cr->NumTotVerts);
	rMakeProjectiveShadow(sb, nullptr, cm->NumVerts);
	StartVert = 0;

	for (i = 0; i < 8; i++) t3dVectCopy(&BBox[i], &cm->Trasl);
	for (i = 0; i < 4; i++) BBox[i + 4].y += c->Height;
	t3dVectSub(&tmp, &cm->Trasl, &light->Source);
	tmp.y = 0.0f;
	t3dVectNormalize(&tmp);
	t3dVectScale(&tmp, &tmp, c->Radius * 2.0f);

	t3dVectAdd(&BBox[0], &BBox[0], &tmp);
	t3dVectAdd(&BBox[4], &BBox[4], &tmp);
	t3dVectSub(&BBox[1], &BBox[1], &tmp);
	t3dVectSub(&BBox[5], &BBox[5], &tmp);
	tmp.y = tmp.x;
	tmp.x = -tmp.z;
	tmp.z = tmp.y;
	tmp.y = 0.0f;
	t3dVectAdd(&BBox[2], &BBox[2], &tmp);
	t3dVectAdd(&BBox[6], &BBox[6], &tmp);
	t3dVectSub(&BBox[3], &BBox[3], &tmp);
	t3dVectSub(&BBox[7], &BBox[7], &tmp);

	t3dVectFill(&MinSco, 9999999.9f);
	t3dVectFill(&MaxSco, -9999999.9f);
	for (i = 0; i < 8; i++) {
		t3dVectTransform(&v, &BBox[i], &t3dCurViewMatrix);
		t3dVectAdd(&v, &v, &t3dCurTranslation);
		v.x /= v.z;
		v.y = -v.y / v.z;
		if (v.x < MinSco.x) MinSco.x = v.x;
		if (v.y < MinSco.y) MinSco.y = v.y;
		if (v.z < MinSco.z) MinSco.z = v.z;
		if (v.x > MaxSco.x) MaxSco.x = v.x;
		if (v.y > MaxSco.y) MaxSco.y = v.y;
		if (v.z > MaxSco.z) MaxSco.z = v.z;
	}
	Aspect.x = 1.0f / (MaxSco.x - MinSco.x);
	Aspect.y = 1.0f / (MaxSco.y - MinSco.y);
	t3dVectSub(&dir, &cm->Trasl, &light->Source);
	t3dVectNormalize(&dir);
//	Aspect.x *= (1.0-0.1f*fabs(dir.x));
//	Aspect.y *= (1.0-0.1f*fabs(dir.y));
	tempv.rhw   =  1.0f / t3dVectDistance(&light->Source, &cm->Trasl);

	pc = rLockVertexPtr(shad->ProjVertsVB, rVBLOCK_NOSYSLOCK);

	for (k = 0; k < cr->NumMeshes(); k++) {                                                  // scorre tutte le mesh della stanza
		t3dMESH &mm = cr->MeshTable[k];
		if (!(mm.Flags & T3D_MESH_VISIBLE) || (mm.Flags & T3D_MESH_HIDDEN)) continue;        // salta se e' nascosta o se non e' visibile
		if ((mm.Flags & T3D_MESH_MIRROR) || (mm.Flags & T3D_MESH_PORTAL)) continue;          // salta se e' un portale o uno specchio

		t3dSetVisibileVertex(mm);                                                               // segna le facce che sono backface la luce

		gv = mm.VertexBuffer;
		pp = &ProjectedPolys[StartVert];

		for (i = 0; i < mm.NumVerts; i++, pp++, gv++, pc++) {
			memcpy(pc, gv, sizeof(gVertex));
			pc->diffuse = 0xFFFFFFFF;

			t3dVectInit(&v, gv->x, gv->y, gv->z);
			t3dVectTransform(&v, &v, &t3dCurViewMatrix);
			t3dVectAdd(&v, &v, &t3dCurTranslation);

//			tempv.rhw  =  1.0f/MinSco.z;
			tempv.x     =  v.x * tempv.rhw;
			tempv.y     = -v.y * tempv.rhw;
			tempv.z     =  v.z;
			pc->u1      = (tempv.x - MinSco.x) * Aspect.x;
			pc->v1      = (tempv.y - MinSco.y) * Aspect.y;

			*pp = 0;
			if (tempv.x < MinSco.x * 4.0f) *pp |= (1 << 1);
			if (tempv.x > MaxSco.x * 4.0f) *pp |= (1 << 2);
			if (tempv.y < MinSco.y * 4.0f) *pp |= (1 << 3);
			if (tempv.y > MaxSco.y * 4.0f) *pp |= (1 << 4);
			if (tempv.z < MinSco.z) *pp |= (1 << 5);
		}

		pp = &ProjectedPolys[StartVert];
		ptr = &shad->pwShadVolCapIndices[shad->num_cap_indices];
		for (t3dU32 ff = 0; ff < mm.NumFaces(); ff++) {
			t3dFACE &face = mm.FList[ff];
			if (!(face.flags & T3D_FACE_VISIBLE))
				continue;

			if ((pp[face.VertexIndex[0]]) && (pp[face.VertexIndex[1]]) && (pp[face.VertexIndex[2]]))
				continue;

			*(ptr++) = face.VertexIndex[0] + StartVert;
			*(ptr++) = face.VertexIndex[1] + StartVert;
			*(ptr++) = face.VertexIndex[2] + StartVert;
			shad->num_cap_indices += 3;
		}

		StartVert += mm.NumVerts;
	}
	t3dDealloc(sizeof(t3dU8)*cr->NumTotVerts);
	rUnlockVertexPtr(shad->ProjVertsVB);
	pc = nullptr;

	t3dVectSub(&t3dCurTranslation, &cm->Trasl, &light->Source);                                  // punta la luce sull'omino
	t3dVectTransform(&t3dCurTranslation, &t3dCurTranslation, &t3dCurViewMatrix);
	t3dMatMul(&t3dCurViewMatrix, &t3dCurViewMatrix, &cm->Matrix);

	if (shad->pwShadVolSideIndices) {
		kkk = 0;
		for (i = 0; i < c->Shadow->NumMeshes(); i++) {
			t3dMESH &mm = c->Shadow->MeshTable[i];
			for (k = 0; k < mm.NumFaces(); k++) {
				shad->pwShadVolSideIndices[kkk++] = mm.FList[k].VertexIndex[0];
				shad->pwShadVolSideIndices[kkk++] = mm.FList[k].VertexIndex[1];
				shad->pwShadVolSideIndices[kkk++] = mm.FList[k].VertexIndex[2];
			}
		}
		/*f     for( k=0; k<cm->NumFaces; k++ )
		        {
		            shad->pwShadVolSideIndices[kkk++] = cm->FList[k].VertexIndex[0];
		            shad->pwShadVolSideIndices[kkk++] = cm->FList[k].VertexIndex[1];
		            shad->pwShadVolSideIndices[kkk++] = cm->FList[k].VertexIndex[2];
		        }f*/
		shad->num_side_indices = kkk;
	}

	if (shad->VB) {
		gv = cm->VertexBuffer;
		pv = (pVert *)rLockVertexPtr(shad->VB, rVBLOCK_NOSYSLOCK);

		for (i = 0; i < cm->NumVerts; i++, pv++, gv++) {
			t3dVectInit(&v, gv->x, gv->y, gv->z);
//f         if ( v.y < 60.0f ) v.y -= 60.0f;
			t3dVectTransform(&v, &v, &t3dCurViewMatrix);
			t3dVectAdd(&v, &v, &t3dCurTranslation);

			pv->rhw = 1.0f / v.z;
			pv->x   = (v.x * pv->rhw - MinSco.x) * Aspect.x * 256.0f;
			pv->y   = (-v.y * pv->rhw - MinSco.y) * Aspect.y * 256.0f;
			pv->z   =  v.z * (1.0f / 30000.0f);
//			pv->diffuse = RGBA_MAKE(1,1,1,sb->Intensity);
			assert(false);
#if 0
			pv->diffuse = RGB_MAKE(255 - sb->Intensity, 255 - sb->Intensity, 255 - sb->Intensity);
#endif
//			pv->diffuse = RGB_MAKE(128,128,128);
		}
		rUnlockVertexPtr(shad->VB);
	}
//tb
#endif
}

/* -----------------18/06/99 10.47-------------------
 *              t3dAddStencilBufferShadow
 * --------------------------------------------------*/
void t3dAddStencilBufferShadow(t3dCHARACTER *c, uint32 CurShadowBox, t3dLIGHT *light) {
	SHADOWBOX   *sb = c->ShadowBox[CurShadowBox];
	uint32  i, j, k, f, NumVerts;
	t3dMESH *cm = c->Mesh;
	t3dV3F  tmp, NormLight;
	t3dM3X3F lm;

	if (!cm || !cm->VertexBuffer)
		return ;
//sb
	t3dVectSub(&NormLight, &cm->Trasl, &light->Source);
	t3dVectNormalize(&NormLight);
	t3dVectTransformInv(&NormLight, &NormLight, &cm->Matrix);
	NormLight.y = - NormLight.y;
	NormLight.z = - NormLight.z;
	NormLight.x = - NormLight.x;

	t3dVectSub(&tmp, &cm->Trasl, &t3dCurCamera->Source);
	tmp.y = 0.0f;
	t3dVectNormalize(&tmp);

	tmp = tmp * 2.0f;
	tmp *= c->Radius * 2.0f;
	t3dVectTransformInv(&tmp, &tmp, &cm->Matrix);

	if (rMakeShadowBox(sb, tmp.x, c->Height, tmp.z, sb->Intensity))
		return ;

	gVertex *VBptr = cm->VertexBuffer;
	for (i = 0; i < c->Shadow->NumMeshes(); i++) {
		t3dMESH &mm = c->Shadow->MeshTable[i];
		uint32 *FaceList = (uint32 *)t3dAlloc(sizeof(uint32) * mm.NumFaces() * 3);
		gVertex *VertList = (gVertex *)t3dAlloc(sizeof(gVertex) * mm.NumFaces() * 3);
		NumVerts = 0;
		for (k = 0; k < mm.NumFaces(); k++) {
			for (f = 0; f < 3; f++) {
				FaceList[NumVerts] = mm.FList[k].VertexIndex[f];
				for (j = 0; j < NumVerts; j++)
					if (FaceList[j] == FaceList[NumVerts]) break;
				if (j >= NumVerts) NumVerts++;
			}
		}
		for (j = 0; j < NumVerts; j++)
			VertList[j] = VBptr[FaceList[j]];

		t3dVectFill(&tmp, 0.0f);
		t3dMatView(&lm, &tmp, &NormLight);
		rMakeShadowVolume(sb, VertList, NumVerts, lm.M);

		t3dDealloc(sizeof(gVertex)*mm.NumFaces() * 3);
		t3dDealloc(sizeof(uint32) * mm.NumFaces() * 3);
	}
//sb
}


/* -----------------10/06/99 15.49-------------------
 *              t3dTransformCharacter
 * --------------------------------------------------*/
bool t3dTransformCharacter(t3dCHARACTER *c) {
	uint32      OldMeshFlags, l, CurShadowBox;
	t3dV3F      tmp, SavedTrasl;
	t3dBODY     *OldCurRoom;
	t3dM3X3F    SavedMat;
	t3dVERTEX   vv;
	int16      intens;

	if (!c || !c->Body || !c->Mesh) return FALSE;
	if (c->Flags & T3D_CHARACTER_HIDE)
		goto enabledinmirror;

	c->CurRoom = t3dCurRoom;
	if (c->Flags & T3D_CHARACTER_REALTIMELIGHTING)
		t3dLightCharacter(c);

	bDisableMirrors = 1;
	if (c->Mesh->ExpressionFrame >= EXPRESSION_SET_LEN) {
		DebugLogWindow("Errore Espressione");
		c->Mesh->ExpressionFrame = 0;
	}
	c->Mesh->ExpressionFrame += c->CurExpressionSet * EXPRESSION_SET_LEN;
	if (!t3dTransformBody(c->Body))
		DebugLogWindow("Can't transform %s", c->Body->name.c_str());
	c->Mesh->ExpressionFrame -= c->CurExpressionSet * EXPRESSION_SET_LEN;
	if (c->Mesh) t3dDetectObj(t3dCurCamera, c->Mesh, &t3dCurViewMatrix);
	bDisableMirrors = 0;

	if ((c->Flags & T3D_CHARACTER_CASTREALTIMESHADOWS) && !bDisableMirrors && (!(LoaderFlags & T3D_NOSHADOWS))) {
		CurShadowBox = 0;
		t3dOldUserViewMatrix = t3dCurUserViewMatrix;
		rSaveViewMatrix();

		t3dVectCopy(&SavedTrasl, &t3dCurTranslation);
		t3dMatCopy(&SavedMat, &t3dCurViewMatrix);
		for (l = 0; l < t3dCurRoom->NumLights(); l++) {
			if (CurShadowBox > 1) continue;
			if (!(t3dCurRoom->LightTable[l].Type & T3D_LIGHT_REALTIME)) continue;
			if (!(t3dCurRoom->LightTable[l].Type & T3D_LIGHT_CASTSHADOWS)) continue;
			if (!(t3dCurRoom->LightTable[l].Type & T3D_LIGHT_LIGHTON)) continue;
			if ((t3dCurRoom->LightTable[l].Type & T3D_LIGHT_FLARE)) continue;

			t3dVectAdd(&tmp, &c->Body->MeshTable[0].Trasl, &c->Body->MeshTable[0].Pos);
			if (t3dCurRoom->LightTable[l].Source.y < tmp.y) continue;
			if (!LightVertex(&vv, &tmp, &t3dCurRoom->LightTable[l])) continue;
			if ((vv.r + vv.g + vv.b) < 30) continue;

			if (c->ShadowBox[CurShadowBox] == nullptr)
				c->ShadowBox[CurShadowBox] = t3dCalloc<SHADOWBOX>(1);
			c->ShadowBox[CurShadowBox]->NumShadowsList = 0;
			intens = (int16)(vv.r + vv.g + vv.b /*- t3dCurRoom->AmbientLight.x - t3dCurRoom->AmbientLight.y - t3dCurRoom->AmbientLight.z*/) / 4;
			if (intens < 0)    intens = 0;
			if (intens > 255)  intens = 255;
			c->ShadowBox[CurShadowBox]->Intensity = intens;

			if (!rGetStencilBitDepth()) {
				c->ShadowBox[CurShadowBox]->ViewMatrixNum = t3dOrigUserViewMatrix;
				t3dAddTextureBufferShadow(c, CurShadowBox++, &t3dCurRoom->LightTable[l]);
			} else {
				c->ShadowBox[CurShadowBox]->ViewMatrixNum = t3dCurUserViewMatrix;
				t3dAddStencilBufferShadow(c, CurShadowBox++, &t3dCurRoom->LightTable[l]);
			}
		}
		t3dMatCopy(&t3dCurViewMatrix, &SavedMat);
		t3dVectCopy(&t3dCurTranslation, &SavedTrasl);
		rRestoreViewMatrix();
		t3dCurUserViewMatrix = t3dOldUserViewMatrix;
	}

enabledinmirror:
	if ((c->Flags & T3D_CHARACTER_ENABLEDINMIRROR) || !(c->Flags & T3D_CHARACTER_HIDE)) {
		OldMeshFlags = c->Flags;
		c->Flags &= ~T3D_CHARACTER_HIDE;

		OldCurRoom = t3dCurRoom;
		t3dCurRoom = c->Body;
		t3dProcessMirrors(t3dGlobalMirrorList, t3dNumGlobalMirrors);
		t3dCurRoom = OldCurRoom;

		c->Flags = OldMeshFlags;
	}

	return TRUE;
}

/* -----------------10/06/99 15.58-------------------
 *                  ProcessWater
 * --------------------------------------------------*/
void ProcessWater(t3dMESH &mesh, int32 CurPass, int32 MaxPass) {
	MaterialPtr mat = mesh.FList[0].getMaterial();
	uint32  dimx = mat->Texture->DimX;
	uint32  dimy = mat->Texture->DimY;
	int32  *dest = mesh.WaterBuffer2 + dimx * 2, *source = mesh.WaterBuffer1 + dimx * 2;
	uint32  i;

	for (i = dimx; i < (dimx * dimy) - dimx; i++, dest++, source++) {
		if (((int32)i % MaxPass) != CurPass) continue;                                               // divide il renderig in piu' passi

		*dest = (((*(source - 1) +
		           * (source + 1) +
		           * (source - dimx) +
		           * (source + dimx))  >> 1)) - (*dest);
		*dest -= ((*dest) >> 10);
	}
}

/* -----------------10/06/99 15.58-------------------
 *                  t3dRenderWater
 * --------------------------------------------------*/
void t3dRenderWater(t3dMESH &mesh, uint32 Type) {
	MaterialPtr mat = mesh.FList[0].getMaterial();
	int32  pitch, Xoffset, Yoffset;
	int32  dimx = (int32)mat->Texture->DimX;
	int32  dimy = (int32)mat->Texture->DimY;
	uint16  *texturesource, *texturedest, *textsource;
	int32  *WaterBuffer = mesh.WaterBuffer2 + dimx, *AppoWaterBuffer;
	static  int32  WaterPass = 0;
	int32  R, G, B, A;

//w
	return ;
//w
	if (mat->AddictionalMaterial.empty() || !mat->AddictionalMaterial[0]->Texture)
		return;

	if (--WaterPass < 0)
		WaterPass = 4;
	else {
		ProcessWater(mesh, WaterPass, 4);
		return ;
	}

	textsource = texturesource = (uint16 *)rLockSurfaceDirect(mat->AddictionalMaterial[0]->Texture, (unsigned int *)&pitch);
	texturedest = (uint16 *)rLockSurfaceDirect(mat->Texture, (unsigned int *)&pitch);

	pitch /= 2;
	/*  for (i=0; i<(dimx*dimy); i++, texturedest+=2,texturesource+=2, WaterBuffer++)
	    {
	        Xoffset=(*(WaterBuffer-1))-(*(WaterBuffer+1));
	        Yoffset=(*(WaterBuffer-dimx))-(*(WaterBuffer+dimx));
	        *texturedest=*(texturesource+((Xoffset*2+Yoffset*pitch)&(dimy*dimy*2-1)));
	    }*/

	for (int32 j = 0; j < dimy; j++)
		for (int32 i = 0; i < dimx; i++, texturedest++, WaterBuffer++, textsource++) {
			if (Type & T3D_MESH_POOLWATER) {
				Xoffset = ((*(WaterBuffer - 1)) - (*(WaterBuffer + 1))) >> 16;
				Yoffset = ((*(WaterBuffer - dimx)) - (*(WaterBuffer + dimx))) >> 16;
				*texturedest = (texturesource[((i + Xoffset) & (dimx - 1)) + (((j + Yoffset) & (dimy - 1)) * pitch)]);
			} else if (Type & T3D_MESH_RECEIVERIPPLES) {
				R = ((*textsource) & 0x1f);
				G = ((*textsource) >> 5) & 0x1f;
				B = ((*textsource) >> 10) & 0x1f;
				A = ((*textsource) >> 15) & 1;
				//          if(*WaterBuffer>0)
				{
					R += ((*WaterBuffer) >> 16);
					G += ((*WaterBuffer) >> 16);
					B += ((*WaterBuffer) >> 16);
					if (R > 30)   R = 30;
					if (G > 30)   G = 30;
					if (B > 30)   B = 30;
				}

				*texturedest = (R & 0x1f) | ((G & 0x1f) << 5) | ((B & 0x1f) << 10) | (A << 15);
			}
		}


	rUnlockSurfaceDirect(mat->AddictionalMaterial[0]->Texture);
	rUnlockSurfaceDirect(mat->Texture);

	AppoWaterBuffer = mesh.WaterBuffer1;
	mesh.WaterBuffer1 = mesh.WaterBuffer2;
	mesh.WaterBuffer2 = AppoWaterBuffer;

//	DebugLogWindow("--------");

	/*  for every pixel (x,y) in the buffer
	        Xoffset = buffer(x-1, y) - buffer(x+1, y)
	        Yoffset = buffer(x, y-1) - buffer(x, y+1)
	        Shading = Xoffset
	        t = texture(x+Xoffset, y+Yoffset)
	        p = t + Shading
	        plot pixel at (x,y) from texture(x+Xoffset, y+Yoffset)
	    end loop*/
}

/* -----------------10/06/99 15.58-------------------
 *                  t3dRenderWaves
 * --------------------------------------------------*/
void t3dRenderWaves(gVertex *gv, uint32 NumVerts, t3dF32 mul, t3dF32 Y) {
	static  uint16  Posit = 0;
	uint32  i;
	t3dF32  v1, v2, y;
//	gVertex *ogv=gv;

	/*  mul=0.00009f;
	    for (i=0; i<NumVerts; i++, ogv++)
	    {
	        if (ogv->u1>2.0f || ogv->v1>2.0f)
	            mul=0.005f;
	    }
	*/
	for (i = 0; i < NumVerts; i++, gv++) {
		if (i & 0x1) {
			v1 = (t3dF32)sin(DEGREE_TO_RADIANS(Posit)) * mul;
			v2 = (t3dF32)cos(DEGREE_TO_RADIANS(Posit)) * mul;
			y = (t3dF32)sin(DEGREE_TO_RADIANS(Posit)) * Y;
		} else {
			v1 = (t3dF32)cos(DEGREE_TO_RADIANS(Posit)) * mul;
			v2 = (t3dF32)sin(DEGREE_TO_RADIANS(Posit)) * mul;
			y = (t3dF32)cos(DEGREE_TO_RADIANS(Posit)) * Y;
		}


		gv->v1 += v1;
		gv->v2 += v2;
		gv->u1 += v1;
		gv->u2 += v2;
		gv->y += y;
	}

	Posit++;
	if (Posit > 360)
		Posit -= 360;

//	Posit=Posit>360 ? 0 : Posit++;
}

/* -----------------10/06/99 15.58-------------------
 *                  t3dMoveTexture
 * --------------------------------------------------*/
void t3dMoveTexture(gVertex *gv, uint32 NumVerts, t3dF32 XInc, t3dF32 YInc) {
	uint32  i;

	for (i = 0; i < NumVerts; i++, gv++) {
		gv->u1 += XInc;
		gv->v1 += YInc;
	}
}

void t3dSetFaceVisibilityPortal(t3dMESH *mesh) {
	if (bOrigRoom && !bDisableMirrors && (!(mesh->Flags & T3D_MESH_NOPORTALCHECK)) && (mesh->PortalList)) {
		for (uint32 pl = 0; pl < t3dNumPortals; pl++)
			if (t3dPortalList[pl] == mesh)
				return;
		t3dPortalList[t3dNumPortals++] = mesh;                                              // aggiunge a lista portali
	}
}

void t3dSetFaceVisibilityMirror(t3dMESH *mesh) {
	if (bOrigRoom && !bDisableMirrors)
		if ((t3dNumGlobalMirrors + 1) < MAX_MIRRORS)
			t3dGlobalMirrorList[t3dNumGlobalMirrors++] = mesh;                              // aggiunge alla lista
}

/* -----------------10/06/99 15.44-------------------
 *                  t3dSetFaceVisibility
 * --------------------------------------------------*/
void t3dSetFaceVisibility(t3dMESH *mesh, t3dCAMERA *cam) {
	if (mesh->Flags & T3D_MESH_PORTAL) {                                                         // se e' un portale
		t3dSetFaceVisibilityPortal(mesh);
		return;                                                                                // ed esce
	} else if (mesh->Flags & T3D_MESH_MIRROR) {                                                         // se e' uno specchio
		t3dSetFaceVisibilityMirror(mesh);
		return;                                                                                 // ed esce
	}

	int16 LastT1, LastT2, T1, T2;
	LastT1 = LastT2 = -2;
	T1 = T2 = -1;

	MaterialPtr target = nullptr;
	for (uint32 i = 0; i < mesh->NumFaces(); i++) {
		t3dFACE &f = mesh->FList[i];

		if (!f.getMaterial() || !(f.flags & T3D_FACE_VISIBLE))
			continue;

		f.checkVertices();
		MaterialPtr Material = f.getMaterial();
		T1 = Material->Texture->ID;

		if (!bNoLightmapsCalc && f.lightmap) {
			T2 = f.lightmap->Texture->ID;
		} else
			T2 = -1;

		if (Material->hasFlag(T3D_MATERIAL_ENVIRONMENT)) {                                       // se ha l'environnement
			t3dM3X3F    m;
			t3dMatMul(&m, &mesh->Matrix, &t3dCurViewMatrix);

			if (!(mesh->VBptr = mesh->VertexBuffer))
				continue;

			for (int j = 0; j < 3; j++) {
				t3dV3F  v;
				gVertex *gv = &mesh->VBptr[f.VertexIndex[j]];
				t3dV3F *n = &mesh->NList[f.VertexIndex[j]]->n;
				t3dVectTransform(&v, n, &m);
				gv->u1 = (v.x);
				gv->v1 = (v.y);
			}

			mesh->VBptr = nullptr;
			mesh->Flags |= T3D_MESH_UPDATEVB;
		}

		if ((T1 != LastT1) || (T2 != LastT2) || !target) {
			if (T2 > 0) {
				if (!Material->NumAddictionalMaterial)
					DebugLogWindow("Error no Sub Material found in %s!", mesh->name.c_str());
				int32 j = -1;
				for (j = 0; j < Material->NumAddictionalMaterial; j++)
					if (Material->AddictionalMaterial[j]->Texture->ID == T2)
						break;

				if (j == Material->NumAddictionalMaterial) {
					DebugLogWindow("Sub Material not found in %s!", mesh->name.c_str());
					DebugLogWindow("%d %d | %d", Material->NumAddictionalMaterial, T2, f.lightmap->Texture->ID);
					for (j = 0; j < Material->NumAddictionalMaterial; j++)
						DebugLogWindow("%d", Material->AddictionalMaterial[j]->Texture->ID);
					continue;

				}
				target = Material->AddictionalMaterial[j];
			} else {
				target = Material;
			}
			LastT1 = T1;
			LastT2 = T2;
		}

		// Something is wrong here, as the original game seems to be just writing to NumFaces + 0,1,2, as if there
		// was space allocated there.
		for (int v = 0; v < 3; v++) {
			target->addFace(f.getMatVertexIndex(v));
		}

		StatNumTris++;
	}
}


/* -----------------10/06/99 15.46-------------------
 *              t3dTransformMesh
 * --------------------------------------------------*/
void t3dTransformMesh(t3dMESH &mesh) {
	if ((mesh.Flags & (T3D_MESH_HIDDEN)) &&
	        ((mesh.Flags & T3D_MESH_CHARACTER)))
		return;

	if ((bDisableMirrors == 0) && (bViewOnlyPortal == 0)) {
		t3dDetectObj(t3dCurCamera, &mesh, &t3dCurViewMatrix);

//s     if (t3dSun)
//s         t3dCheckIfFlaresRayVisible(t3dCurCamera->Source,t3dSun->LightTable[0].Source, mesh);
	}

	t3dSetVisibileVertex(mesh);                                                                 // setta tutti i vertici visibili della mesh
	t3dCalcMeshBones(&mesh, 0);                                                                      // calcola eventuali bones

	//setup indices array, texture mapping and materials states
	if ((!(mesh.Flags & T3D_MESH_HIDDEN)))
		t3dSetFaceVisibility(&mesh, t3dCurCamera);

	if (bOrigRoom && !bDisableMirrors) {                                         // Solo se e' nella stanza principale
		if (mesh.Flags & T3D_MESH_RECEIVERIPPLES)
			t3dRenderWater(mesh, T3D_MESH_RECEIVERIPPLES);
		else if (mesh.Flags & T3D_MESH_POOLWATER)
			t3dRenderWater(mesh, T3D_MESH_POOLWATER);

		if (mesh.Flags & T3D_MESH_WAVESTEXTURE || (mesh.XInc) || (mesh.YInc)) {
			if (!(mesh.VBptr = mesh.VertexBuffer))
				return;

			if (mesh.Flags & T3D_MESH_WAVESTEXTURE)
				t3dRenderWaves(mesh.VBptr, mesh.NumVerts, mesh.WavesSpeed, mesh.YSpeed);

			if ((mesh.XInc) || (mesh.YInc))
				t3dMoveTexture(mesh.VBptr, mesh.NumVerts, mesh.XInc, mesh.YInc);

			mesh.Flags |= T3D_MESH_UPDATEVB;
			mesh.VBptr = nullptr;
		}
	}
}

void t3dCAMERA::normalizedSight() {
	t3dV3F normalizedSightOfView = this->Target - this->Source;
	t3dVectNormalize(&normalizedSightOfView);
	this->NormalizedDir = normalizedSightOfView;
}

/* -----------------10/06/99 15.48-------------------
 *                  t3dCalcHalos
 * --------------------------------------------------*/
void t3dCalcHalos(t3dBODY *b) {
	// The userVertexBuffer stuff is not ready yet, giving us nullptr writes.
	warning("TODO: t3dCalcHalos");
	return;

	gMaterial   *Material;
	uint32      uvbc;
	uint32      i;
	gVertex     *gv;
	t3dF32      size;
	//uint16      *fp;
	t3dV3F      v0, v1, v2, v3, tmp;

	for (i = 0; i < b->NumLights(); i++) {
		t3dLIGHT &l = b->LightTable[i];
		if (!(l.Type & T3D_LIGHT_LIGHTON)) continue;

		if (l.Type & T3D_LIGHT_FLARE)
			break;
	}
	if (i >= b->NumLights()) return ;

	t3dOldUserViewMatrix = t3dCurUserViewMatrix;
	rSaveViewMatrix();
	tmp.x = -t3dCurCamera->Source.x;
	tmp.y = -t3dCurCamera->Source.y;
	tmp.z = -t3dCurCamera->Source.z;
	t3dVectTransform(&tmp, &tmp, &t3dCurViewMatrix);
	rSetViewMatrix(t3dCurViewMatrix, tmp);
	t3dCurUserViewMatrix = rAddUserViewMatrix(t3dCurViewMatrix, tmp);

	uvbc = rGetUserVertexBufferCounter();
	gv = rLockVertexPtr(rGetUserVertexBuffer(), rVBLOCK_NOSYSLOCK);
	gv += uvbc;

	for (i = 0; i < b->NumLights(); i++) {
		t3dLIGHT &l = b->LightTable[i];
		if (!(l.Type & T3D_LIGHT_LIGHTON)) continue;

		if (l.Type & T3D_LIGHT_FLARE) {
			Material = &l.Material[0];

			size = l.FlareSize;

			v0.x = -size;
			v0.y = size;
			v0.z = 0.0f;
			t3dVectTransformInv(&v0, &v0, &t3dCurViewMatrix);

			v1.x = size;
			v1.y = size;
			v1.z = 0.0f;
			t3dVectTransformInv(&v1, &v1, &t3dCurViewMatrix);

			v2.x = -size;
			v2.y = -size;
			v2.z = 0.0f;
			t3dVectTransformInv(&v2, &v2, &t3dCurViewMatrix);

			v3.x = size;
			v3.y = -size;
			v3.z = 0.0f;
			t3dVectTransformInv(&v3, &v3, &t3dCurViewMatrix);

			for (int f = 0; f < 3; f++) {
				Material->addFace(f + uvbc);
			}
			StatNumTris++;

			Material->addFace(2 + uvbc);
			Material->addFace(3 + uvbc);
			Material->addFace(1 + uvbc);
			StatNumTris++;

			gv[0].x = v0.x + l.Source.x;
			gv[0].y = v0.y + l.Source.y;
			gv[0].z = v0.z + l.Source.z;
			gv[0].u2 = gv[0].u1 = 0.0f;
			gv[0].v2 = gv[0].v1 = 0.0f;
			gv[1].x = v1.x + l.Source.x;
			gv[1].y = v1.y + l.Source.y;
			gv[1].z = v1.z + l.Source.z;
			gv[1].u2 = gv[1].u1 = 1.0f;
			gv[1].v2 = gv[1].v1 = 0.0f;
			gv[2].x = v2.x + l.Source.x;
			gv[2].y = v2.y + l.Source.y;
			gv[2].z = v2.z + l.Source.z;
			gv[2].u2 = gv[2].u1 = 0.0f;
			gv[2].v2 = gv[2].v1 = 1.0f;
			gv[3].x = v3.x + l.Source.x;
			gv[3].y = v3.y + l.Source.y;
			gv[3].z = v3.z + l.Source.z;
			gv[3].u2 = gv[3].u1 = 1.0f;
			gv[3].v2 = gv[3].v1 = 1.0f;

			gv[0].diffuse = gv[1].diffuse = gv[2].diffuse = gv[3].diffuse = RGBA_MAKE((uint32)l.Color.x, (uint32)l.Color.y, (uint32)l.Color.z, 255);

//			rProcessVertices(rGetUserVertexBuffer(), t3dStartIndex,4);
			t3dStartIndex += 4;
			StatNumVerts += 4;
			uvbc += 4;
			gv += 4;

			rAddToMaterialList(l.Material[0], t3dCurUserViewMatrix);
		}
	}
	rUnlockVertexPtr(rGetUserVertexBuffer());
	rSetUserVertexBufferCounter(uvbc);

	rRestoreViewMatrix();
	t3dCurUserViewMatrix = t3dOldUserViewMatrix;
}

/* -----------------10/06/99 15.53-------------------
 *                  t3dAddParticle
 * --------------------------------------------------*/
void t3dAddParticle(gMaterial *Material, t3dV3F *Source, t3dF32 size, t3dV3F Color, uint32 num) {
	warning("TODO: t3dAddParticle");
	return;
#if 0
	int16      T1;
	uint32      uvbc;
	gVertex     *gv;
	t3dV3F      v0, v1, v2, v3;
	uint32      rr, gg, bl, i;

	uvbc = rGetUserVertexBufferCounter();
	gv = rLockVertexPtr(rGetUserVertexBuffer(), rVBLOCK_NOSYSLOCK);
	gv += uvbc;

	T1 = Material->Texture->ID;

	uint32 faceOffset = Material->NumFaces;

	for (i = 0; i < num; i++, gv += 4) {
		v0.x = -size;
		v0.y = size;
		v0.z = 0.0f;
		t3dVectTransformInv(&v0, &v0, &t3dCurViewMatrix);

		v1.x = size;
		v1.y = size;
		v1.z = 0.0f;
		t3dVectTransformInv(&v1, &v1, &t3dCurViewMatrix);

		v2.x = -size;
		v2.y = -size;
		v2.z = 0.0f;
		t3dVectTransformInv(&v2, &v2, &t3dCurViewMatrix);

		v3.x = size;
		v3.y = -size;
		v3.z = 0.0f;
		t3dVectTransformInv(&v3, &v3, &t3dCurViewMatrix);

		Material->FacesList[faceOffset++] = 0 + uvbc;
		Material->FacesList[faceOffset++] = 1 + uvbc;
		Material->FacesList[faceOffset++] = 2 + uvbc;
		StatNumTris++;
		Material->NumFaces += 3;

		Material->FacesList[faceOffset++] = 2 + uvbc;
		Material->FacesList[faceOffset++] = 3 + uvbc;
		Material->FacesList[faceOffset++] = 1 + uvbc;
		StatNumTris++;
		Material->NumFaces += 3;

		gv[0].x = v0.x + Source->x;
		gv[0].y = v0.y + Source->y;
		gv[0].z = v0.z + Source->z;
		gv[0].u2 = gv[0].u1 = 0.0f;
		gv[0].v2 = gv[0].v1 = 0.0f;
		gv[1].x = v1.x + Source->x;
		gv[1].y = v1.y + Source->y;
		gv[1].z = v1.z + Source->z;
		gv[1].u2 = gv[1].u1 = 1.0f;
		gv[1].v2 = gv[1].v1 = 0.0f;
		gv[2].x = v2.x + Source->x;
		gv[2].y = v2.y + Source->y;
		gv[2].z = v2.z + Source->z;
		gv[2].u2 = gv[2].u1 = 0.0f;
		gv[2].v2 = gv[2].v1 = 1.0f;
		gv[3].x = v3.x + Source->x;
		gv[3].y = v3.y + Source->y;
		gv[3].z = v3.z + Source->z;
		gv[3].u2 = gv[3].u1 = 1.0f;
		gv[3].v2 = gv[3].v1 = 1.0f;

		rr = (uint32)(Color.x);
		gg = (uint32)(Color.y);
		bl = (uint32)(Color.z);

		gv[0].diffuse = gv[1].diffuse = gv[2].diffuse = gv[3].diffuse = RGBA_MAKE((uint32)rr, gg, bl, rand() % 255);

		StatNumVerts += 4;
		uvbc += 4;
	}

//	rProcessVertices(rGetUserVertexBuffer(), t3dStartIndex,num*4);
	t3dStartIndex += num * 4;

	rUnlockVertexPtr(rGetUserVertexBuffer());
	rSetUserVertexBufferCounter(uvbc);
#endif
}

/* -----------------10/06/99 15.57-------------------
 *                  t3dRaiseSmoke
 * --------------------------------------------------*/
void t3dRaiseSmoke(t3dV3F *pos, t3dParticle *p) {
	uint32      i, j = p->ParticleIndex;
	Particle    *pPos;
//	t3dV3F       Color;

//	if (Particles[j].NumParticles<NumParticles)
//		Particles[j].NumParticles++;

//	if (j==0)
//		return ;//Particles[j].NumParticles=0;
	pPos = &Particles[j].ParticlePos[Particles[j].FirstParticleRendered];
	for (i = Particles[j].FirstParticleRendered; i < Particles[j].NumParticles; i++, pPos++) {
		if (pPos->Flags & T3D_PARTICLE_ACTIVE)
			t3dAddParticle(&Particles[j].Material, &pPos->Pos, pPos->Size, pPos->Color, 1);
	}
	pPos = Particles[j].ParticlePos;
	for (i = 0; i < Particles[j].FirstParticleRendered; i++, pPos++) {
		if (pPos->Flags & T3D_PARTICLE_ACTIVE)
			t3dAddParticle(&Particles[j].Material, &pPos->Pos, pPos->Size, pPos->Color, 1);
	}

	pPos = Particles[j].ParticlePos;
	for (i = 0; i < Particles[j].NumParticles; i++, pPos++) {
		if (pPos->Flags & T3D_PARTICLE_ACTIVE) {
//			t3dAddParticle(&Particles[j].Material, &pPos->Pos, pPos->Size, pPos->Color, 1);
			pPos->Pos.y += p->Speed;
			if (pPos->Pos.y < pos->y + p->Seg1) {
				pPos->Size += p->Dim1;
				pPos->Pos.x += p->Caos1 * Particles[j].RandTable[(uint32)pPos->Pos.y & 0xf];
				if ((pPos->Color.x += p->R2) > 255.0f) pPos->Color.x = 255.0f;
				if ((pPos->Color.y += p->G2) > 255.0f) pPos->Color.y = 255.0f;
				if ((pPos->Color.z += p->B2) > 255.0f) pPos->Color.z = 255.0f;
			} else {
				if ((pPos->Color.x += p->R3) > 255.0f) pPos->Color.x = 255.0f;
				if ((pPos->Color.y += p->G3) > 255.0f) pPos->Color.y = 255.0f;
				if ((pPos->Color.z += p->B3) > 255.0f) pPos->Color.z = 255.0f;
				pPos->Pos.x += p->Caos2 * Particles[j].RandTable[(uint32)pPos->Pos.y & 0xf];
				if (pPos->Pos.y > pos->y + p->Seg2)
					pPos->Size -= p->Dim2;
				else
					pPos->Size += p->Dim2;
			}
			if (pPos->Pos.y > pos->y + p->Lung) {
				pPos->Flags &= ~T3D_PARTICLE_ACTIVE;
				pPos->Flags |= T3D_PARTICLE_REGEN;

				if (Particles[j].FirstParticleRendered <= Particles[j].NumParticles - 1)
					Particles[j].FirstParticleRendered++;
				else
					Particles[j].FirstParticleRendered = 0;
			}
		} else {
			if ((!(Particles[j].ParticleWait) || (pPos->Flags & T3D_PARTICLE_REGEN))) {
				error("TODO: Complete particle support");
#if 0
				pPos->Flags &= ~T3D_PARTICLE_REGEN;
				pPos->Flags |= T3D_PARTICLE_ACTIVE;
				pPos->Pos.x = pos->x;
				pPos->Pos.y = pos->y;
				pPos->Pos.z = pos->z;
				pPos->Size = p->Size;
				pPos->Color.x = p->R1;
				pPos->Color.y = p->G1;
				pPos->Color.z = p->B1;
				Particles[j].RandTable[i & 0xf] = rand() % 3 - 1;
				Particles[j].ParticleWait = p->Delay;
#endif
			}
		}
	}

	Particles[j].ParticleWait--;

	rAddToMaterialList(Particles[j].Material, t3dCurUserViewMatrix);
}

/* -----------------10/06/99 15.48-------------------
 *                  t3dAnimLights
 * --------------------------------------------------*/
void t3dAnimLights(t3dBODY *b) {
//	t3dU8        exist=0;
	gVertex     *vv;
	int32      rr, gg, bb, aa;
	uint8       bProcessLights;

	if (b->LightTable.empty())
		return;

	if (b->LightTable[0].Type & T3D_LIGHT_ALLLIGHTSOFF)
		return;

	t3dCalcHalos(b); // Calculate flare/halos for lights

	bProcessLights = 0;
	uint32 light = 0;
	while ((light < b->NumLights())) {
		t3dLIGHT &l = b->LightTable[light];
		if (l.Type & T3D_LIGHT_CANDLESMOKE)
			t3dRaiseSmoke(&l.Source, l.Particle.get());
		else if (l.Type & T3D_LIGHT_PULSE) {
			error("TODO: Pulsing lights");
#if 0
		bProcessLights = 1;
		l.LightRandomizer = -rand() % ((int32)(l.Color.x + l.Color.y + l.Color.z) / 12);
#endif
		}
		light++;
	}

	if (bProcessLights) {
		//lock all vertexbuffers
		for (uint i = 0; i < b->NumMeshes(); i++) {
			t3dMESH &m = b->MeshTable[i];
			m.VBptr = m.VertexBuffer;
			m.Flags |= T3D_MESH_UPDATEVB;
		}

		for (auto &l : b->LightTable) {
			if (!(l.Type & T3D_LIGHT_PULSE) || !(l.Type & T3D_LIGHT_LIGHTON)) continue;

			if (l.FlickerDelay > (l.Flicker)) {
				l.FlickerDelay = 0;
				for (int j = 0; j < l.AnimLight.NumVisVerts(); j++) {
					vv = l.AnimLight.VisVerts[j];
					rr = RGBA_GETRED(vv->diffuse);
					gg = RGBA_GETGREEN(vv->diffuse);
					bb = RGBA_GETBLUE(vv->diffuse);
					aa = RGBA_GETALPHA(vv->diffuse);
					rr += l.AnimLight.LastRandomizer + l.LightRandomizer;
					gg += l.AnimLight.LastRandomizer + l.LightRandomizer;
					bb += l.AnimLight.LastRandomizer + l.LightRandomizer;
					if (rr < 0)   rr = 0;
					if (gg < 0)   gg = 0;
					if (bb < 0)   bb = 0;
					if (rr > 255) rr = 255;
					if (gg > 255) gg = 255;
					if (bb > 255) bb = 255;

					vv->diffuse = RGBA_MAKE(rr, gg, bb, aa);
				}
				l.AnimLight.LastRandomizer = -l.LightRandomizer;
			}
			l.FlickerDelay++;
		}
		//lock all vertexbuffers
		for (uint i = 0; i < b->NumMeshes(); i++) {
			b->MeshTable[i].VBptr = nullptr;
		}
	}
}

/* -----------------29/07/99 16.13-------------------
 *              t3dCheckMaterialVB
 * --------------------------------------------------*/
void t3dCheckMaterialVB(MaterialPtr mat) {
	if (!mat || (mat->NumAllocatedVerts() < 3))
		return ;

	{
		int i = 0;
		for (; i < mat->NumAllocatedMesh; i++)
			if (*(mat->FlagsList[i]) & T3D_MESH_UPDATEVB)
				break;

		if (i >= mat->NumAllocatedMesh)
			return;
	}
	//warning("TODO: Implement t3dCheckMaterialVB");
//	gv = rLockVertexPtr(mat->VB, DDLOCK_WRITEONLY | DDLOCK_NOSYSLOCK);
	mat->VBO->_buffer.clear();
	for (int i = 0; i < mat->NumAllocatedVerts(); i++) {
		mat->VBO->_buffer.push_back(*mat->VertsList[i]);
		//memcpy(gv, mat->VertsList[i], sizeof(gVertex));
	}

//	rUnlockVertexPtr(mat->VB);
}

/* -----------------10/06/99 15.49-------------------
 *              t3dTransformBody
 * --------------------------------------------------*/
bool t3dTransformBody(t3dBODY *b) {
	if (b == nullptr) return false;

	if ((t3dOrigRoom == b) && !(bDisableMirrors)) // If I'm in the main room
		bOrigRoom = 1;
	else
		bOrigRoom = 0;

	//Normalize view (normalized too many times; can be optimized)
	t3dCurCamera->normalizedSight();

	//Check if all light are turned off
	bNoLightmapsCalc = 0;
	if (!b->LightTable.empty() && b->LightTable[0].Type & T3D_LIGHT_ALLLIGHTSOFF)
		bNoLightmapsCalc = 1;

	//Do primary backface culling
//f
	t3dV3F      Sight;
	t3dVectTransformInv(&Sight, &t3dCurCamera->Source, &b->MeshTable[0].Matrix);
#if 0
	t3dBackfaceCulling(b->NList, b->NumNormals/*+b->NumVerticesNormals*/, &Sight);
#endif
//	t3dBackfaceCulling(b->NList, b->NumNormals/*+b->NumVerticesNormals*/, &t3dCurCamera->Source);
//f
	//Setting up transformation matrices. Mesh-Translation for moved/moveable objects
	t3dCurTranslation = -t3dCurCamera->Source + b->MeshTable[0].Trasl;
	t3dVectTransform(&t3dCurTranslation, &t3dCurTranslation, &t3dCurViewMatrix); // rotate by the camera matrix
//f
	t3dM3X3F    LastViewMatrix;
	t3dMatCopy(&LastViewMatrix, &t3dCurViewMatrix);                                             // Save original view-matrix
	t3dMatMul(&t3dCurViewMatrix, &t3dCurViewMatrix, &b->MeshTable[0].Matrix);
//f
	rSetViewMatrix(t3dCurViewMatrix, t3dCurTranslation);
	t3dCurUserViewMatrix = rAddUserViewMatrix(t3dCurViewMatrix, t3dCurTranslation);
	if (bOrigRoom)
		t3dOrigUserViewMatrix = t3dCurUserViewMatrix;

	//Lights Animations (flicker, pulse etc etc)
	t3dAnimLights(b);
//f
	t3dMatCopy(&t3dCurViewMatrix, &LastViewMatrix);                                             // Restore original matrix
//f

	// Remove the meshes that remain behind BlockMesh
	if (bOrigRoom)
		for (uint32 j = 0; j < T3D_MAX_BLOCK_MESHES; j++)
			t3dCheckBlockMesh(b->MeshTable, b->NumMeshes(), b->BlockMeshes[j]);

	//Main loop through the meshes
	for (uint32 i = 0; i < b->NumMeshes(); i++) {
		t3dMESH &m = b->MeshTable[i];
		if ((!(m.Flags & (T3D_MESH_HIDDEN | T3D_MESH_INVISIBLEFROMSECT | T3D_MESH_PORTALPROCESSED))) &&                 // hidden or invisible
		        (bDisableMirrors || !(m.Flags & T3D_MESH_ONLYINMIRRORS)) &&                     // only in mirrors
		        (t3dCheckBoundSphere(m)) &&                                                          // check bound-sphere
		        !((m.NumFaces() > 16) && !t3dCheckBoundBox(m))) {                                     // check bound-box
			// Set visible
			m.Flags |= T3D_MESH_VISIBLE;
			// If viewed from a portal, mark it as processed to avoid double rendering
			if (bPortalRoom) m.Flags |= T3D_MESH_PORTALPROCESSED;
			// Transform mesh. This function also apply bones deformation and other mesh modifications
			t3dTransformMesh(m);

			// Now send vertices to render.dll for transformations
			if ((!(m.Flags & T3D_MESH_HIDDEN) && (!(m.Flags & T3D_MESH_MIRROR)) && (!(m.Flags & T3D_MESH_PORTAL)))) {
				t3dStartIndex += m.NumVerts;
				StatNumVerts += m.NumVerts;
			}
		} else
			m.Flags &= ~(T3D_MESH_VISIBLE | T3D_MESH_INVISIBLEFROMSECT); //Mesh is invisible
	}

	//Now build up the material lists. Check also if lightmaps are active;if active process lightmaps
	for (auto &mat : b->MatTable) {
		if (!mat) continue;

		t3dCheckMaterialVB(mat);

		for (auto &cm : mat->AddictionalMaterial) {
			t3dCheckMaterialVB(cm);
		}
//f     rAddToMaterialList( mat, t3dCurUserViewMatrix );
	}
	QueueMaterialList(b->MatTable, b->NumMaterials(), t3dCurUserViewMatrix);

	for (uint32 i = 0; i < b->NumMeshes(); i++) {
		b->MeshTable[i].Flags &= ~T3D_MESH_UPDATEVB;
	}

	//if there are mirrors in the body is now time to process them...
	if (!bDisableMirrors && bOrigRoom)
		t3dProcessMirrors(t3dGlobalMirrorList, t3dNumGlobalMirrors);

	bOrigRoom = 0;
	return true;
}

/* -----------------10/06/99 15.53-------------------
 *                  t3dAddLensFlare
 * --------------------------------------------------*/
void t3dAddLensFlare(gMaterial *Material, t3dV3F *Source, t3dF32 size, t3dF32 perc2, t3dV3F Color) {
	warning("TODO: t3dAddLensFlare");
	return;
#if 0
	uint32      uvbc;
	gVertex     *gv;
	t3dV3F      v0, v1, v2, v3;
	int32      rr, gg, bb;

	uvbc = rGetUserVertexBufferCounter();
	gv = rLockVertexPtr(rGetUserVertexBuffer(), rVBLOCK_NOSYSLOCK);
	gv += uvbc;

//	for (i=0; i<num; i++)
	{


		v0.x = -size;
		v0.y = size;
		v0.z = 0.0f;
		t3dVectTransformInv(&v0, &v0, &t3dCurViewMatrix);

		v1.x = size;
		v1.y = size;
		v1.z = 0.0f;
		t3dVectTransformInv(&v1, &v1, &t3dCurViewMatrix);

		v2.x = -size;
		v2.y = -size;
		v2.z = 0.0f;
		t3dVectTransformInv(&v2, &v2, &t3dCurViewMatrix);

		v3.x = size;
		v3.y = -size;
		v3.z = 0.0f;
		t3dVectTransformInv(&v3, &v3, &t3dCurViewMatrix);

		int faceIndex = Material->NumFaces;
		Material->FacesList[faceIndex++] = 0 + uvbc;
		Material->FacesList[faceIndex++] = 1 + uvbc;
		Material->FacesList[faceIndex++] = 2 + uvbc;
		StatNumTris++;
		Material->NumFaces += 3;

		Material->FacesList[faceIndex++] = 2 + uvbc;
		Material->FacesList[faceIndex++] = 3 + uvbc;
		Material->FacesList[faceIndex++] = 1 + uvbc;
		StatNumTris++;
		Material->NumFaces += 3;

		gv[0].x = v0.x + Source->x;
		gv[0].y = v0.y + Source->y;
		gv[0].z = v0.z + Source->z;
		gv[0].u2 = gv[0].u1 = 0.0f;
		gv[0].v2 = gv[0].v1 = 0.0f;
		gv[1].x = v1.x + Source->x;
		gv[1].y = v1.y + Source->y;
		gv[1].z = v1.z + Source->z;
		gv[1].u2 = gv[1].u1 = 1.0f;
		gv[1].v2 = gv[1].v1 = 0.0f;
		gv[2].x = v2.x + Source->x;
		gv[2].y = v2.y + Source->y;
		gv[2].z = v2.z + Source->z;
		gv[2].u2 = gv[2].u1 = 0.0f;
		gv[2].v2 = gv[2].v1 = 1.0f;
		gv[3].x = v3.x + Source->x;
		gv[3].y = v3.y + Source->y;
		gv[3].z = v3.z + Source->z;
		gv[3].u2 = gv[3].u1 = 1.0f;
		gv[3].v2 = gv[3].v1 = 1.0f;

		rr = (uint32)(Color.x - perc2);
		gg = (uint32)(Color.y - perc2);
		bb = (uint32)(Color.z - perc2);
		if (rr < 0)    rr = 0;
		if (gg < 0)    gg = 0;
		if (bb < 0)    bb = 0;
		if (rr > 255)  rr = 255;
		if (gg > 255)  gg = 255;
		if (bb > 255)  bb = 255;

		gv[0].diffuse = gv[1].diffuse = gv[2].diffuse = gv[3].diffuse = RGBA_MAKE((uint32)rr, gg, bb, rand() % 255);

//		rProcessVertices(rGetUserVertexBuffer(), t3dStartIndex,4);
		t3dStartIndex += 4;
		StatNumVerts += 4;
		uvbc += 4;
	}

	rUnlockVertexPtr(rGetUserVertexBuffer());
	rSetUserVertexBufferCounter(uvbc);
#endif
}

/* -----------------10/06/99 15.54-------------------
 *
 * --------------------------------------------------*/
uint8 t3dPointInFrustum(t3dV3F pos) {
	t3dF32 d1;

	t3dVectSub(&pos, &pos, &t3dCurCamera->Source);
	t3dVectTransform(&pos, &pos, &t3dCurViewMatrix); // rotate by the mesh matrix

	d1 = t3dVectDot(&pos, &ClipPlanes[RIGHTCLIP].n) + ClipPlanes[RIGHTCLIP].dist;
	if ((d1 > 0)) {
		d1 = ClipPlanes[LEFTCLIP].dist + t3dVectDot(&pos, &ClipPlanes[LEFTCLIP].n);
		if ((d1 > 0)) {
			d1 = ClipPlanes[TOPCLIP].dist + t3dVectDot(&pos, &ClipPlanes[TOPCLIP].n);
			if ((d1 > 0)) {
				d1 = ClipPlanes[BOTTOMCLIP].dist + t3dVectDot(&pos, &ClipPlanes[BOTTOMCLIP].n);
				if ((d1 > 0)) {
					return 1;
				}
			}
		}
	}

	return 0;
}


/*
 * -----------------10/06/99 15.54-------------------
 *                  t3dTransformSky
 * --------------------------------------------------*/
void t3dTransformSky() {
	gVertex *gv;
	t3dBODY *sky = t3dSky;
	t3dBODY *sun = nullptr; //s t3dSun;
	t3dV3F  SunDir, color, inc, pos, tmp;
	t3dF32  perc;
	static  t3dV3F  OldSunPosition;
//	static t3dU32    value;

	if (sky) {
		for (uint32 i = 0; i < sky->NumMeshes(); i++) {
			t3dVectCopy(&sky->MeshTable[i].Trasl, &t3dCurCamera->Source);
		}
		if (!t3dTransformBody(sky)) {
			DebugLogWindow("Can't transform %s", t3dSky->name.c_str());
		}

		gv = t3dSky->MeshTable[0].VertexBuffer;
		for (uint32 i = 0; i < t3dSky->MeshTable[0].NumVerts; i++, gv++) {
			gv->u1 += SKY_SPEED;
			gv->v1 += SKY_SPEED;
		}
		t3dSky->MeshTable[0].Flags |= T3D_MESH_UPDATEVB;
	}

	if (sun) {
		t3dF32      size;
//		t3dV3F      v1;
//		unsigned short  *p;
//		t3dS32      pitch;
		t3dF32      perc2;
		uint32      rr = 50, gg = 50, bb = 50/*,aa*/;

		/*      p=(unsigned short*)rLockSurface(SkySurface, &pitch);
		        pitch/=2;

		        v1.x=SunPositionInTexture.x;
		        v1.y=SunPositionInTexture.y;
		        if ((v1.x<0.0f) || (v1.y<0.0f) || (v1.x>1.0f) || (v1.x>1.0f))
		            return;
		        rSurfaceFormatToRGBA((WORD)p[(t3dU32)(v1.x*256.0f)+(t3dU32)(v1.y*256.0f)*pitch],
		                                &rr,&gg,&bb,&aa);
		//      p[(t3dU32)(v1.x*256.0f)+(t3dU32)(v1.y*256.0f)*pitch]=(short)rRGBAToSurfaceFormat(rand(),rand(),rand(),255);
		//      p[(t3dU32)(0.5f*256.0f)+(t3dU32)(0.5f*256.0f)*pitch]=(short)rRGBAToSurfaceFormat(rand(),rand(),rand(),aa);
		        rUnlockSurface(SkySurface);
		*/

		t3dSky->MatTable[1]->Texture = (SkySurface);


		perc2 = ((((rr + gg + bb) * (1.0f / 3.0f))));

		t3dOldUserViewMatrix = t3dCurUserViewMatrix;
		rSaveViewMatrix();
		tmp = -t3dCurCamera->Source;
		t3dVectTransform(&tmp, &tmp, &t3dCurViewMatrix);

		rSetViewMatrix(t3dCurViewMatrix, tmp);
		t3dCurUserViewMatrix = rAddUserViewMatrix(t3dCurViewMatrix, tmp);

		auto &l = sun->LightTable[0];

		t3dVectSub(&SunDir, &l.Source, &t3dCurCamera->Source);
		t3dVectNormalize(&SunDir);
		perc = t3dVectDot(&SunDir, &t3dCurCamera->NormalizedDir);

		if ((l.Type & T3D_LIGHT_FLARE) && (perc > 0)) {
			t3dV3F  appo, appo2;

			size = (perc * perc2) * 180; //f era: 500

			t3dVectSub(&appo, &t3dCurCamera->Source, &OldSunPosition);
			t3dVectCopy(&OldSunPosition, &t3dCurCamera->Source);
			appo2.x = l.Source.x + appo.x;
			appo2.y = 8000;     //f era: 7000
			appo2.z = l.Source.z + appo.z;

			if (t3dPointInFrustum(appo2) && (LastFlaresVisible)) {
				//Draw Lensflares
				l.Source.y = 8000; //f era: 7000

				t3dVectCopy(&pos, &t3dCurCamera->Source);
				t3dVectCopy(&inc, &t3dCurCamera->NormalizedDir);
				inc *= 2200.0f;
				t3dVectAdd(&pos, &pos, &inc);
				t3dVectSub(&SunDir, &l.Source, &pos);
				inc.x = (SunDir.x / 3);
				inc.y = (SunDir.y / 3);
				inc.z = (SunDir.z / 3);

				perc = 1.0f - ((1.0f - perc) / 0.15f);

				t3dVectCopy(&pos, &l.Source);

				color.x = 0 * perc;
				color.y = 155 * perc;
				color.z = 155 * perc;
				t3dVectSub(&pos, &pos, &inc);
				t3dAddLensFlare(&l.Material[1], &pos, 500, 0, color);

				color.x = 0 * perc;
				color.y = 155 * perc;
				color.z = 200 * perc;
				t3dVectSub(&pos, &pos, &inc);
				t3dAddLensFlare(&l.Material[1], &pos, 400, 0, color);

				color.x = 0 * perc;
				color.y = 55 * perc;
				color.z = 155 * perc;
				inc *= 0.8f;
				t3dVectSub(&pos, &pos, &inc);
				t3dAddLensFlare(&l.Material[1], &pos, 100, 0, color);

				color.x = 255 * (perc);
				color.y = 222 * (perc);
				color.z = 2  * (perc);
				inc *= 0.5f;
				t3dVectSub(&pos, &pos, &inc);
				t3dAddLensFlare(&l.Material[1], &pos, 200, 0, color);

				rAddToMaterialList(l.Material[1], t3dCurUserViewMatrix);
			}

			LastFlaresVisible = 1;

			//Draw Sun
			t3dAddLensFlare(&l.Material[0], &l.Source, size, perc2, l.Color);
			rAddToMaterialList(l.Material[0], t3dCurUserViewMatrix);
		}
		rRestoreViewMatrix();
		t3dCurUserViewMatrix = t3dOldUserViewMatrix;
	}
}


/* -----------------10/06/99 15.43-------------------
 *              t3dCheckPortalCrossed
 * --------------------------------------------------*/
t3dBODY *t3dCheckPortalCrossed(t3dV3F *a) {
	NormalPtr   n;
	uint32      i;
	t3dMESH     *mesh;
	t3dV3F      line, inter, scale, v0, v1, v2;
	t3dF32      d;

	for (i = 0; i < t3dNumPortals; i++) {
		mesh = t3dPortalList[i];

		if (mesh->Flags & T3D_MESH_VIEWONLYPORTAL) continue;

		n = mesh->FList[0].n;
		d = n->dist - (t3dVectDot(a, &n->n));
		if (d > -1.0f) {
			if (!(mesh->VBptr = mesh->VertexBuffer))
				return nullptr;

			t3dVectCopy(&line, a);
			line.y += 10;
			t3dVectCopy(&scale, &n->n);
			scale *= 20000.0f;
			t3dVectAdd(&line, &line, &scale);

			v0.x = mesh->VBptr[mesh->FList[0].VertexIndex[0]].x;
			v0.y = mesh->VBptr[mesh->FList[0].VertexIndex[0]].y;
			v0.z = mesh->VBptr[mesh->FList[0].VertexIndex[0]].z;
			v1.x = mesh->VBptr[mesh->FList[0].VertexIndex[1]].x;
			v1.y = mesh->VBptr[mesh->FList[0].VertexIndex[1]].y;
			v1.z = mesh->VBptr[mesh->FList[0].VertexIndex[1]].z;
			v2.x = mesh->VBptr[mesh->FList[0].VertexIndex[2]].x;
			v2.y = mesh->VBptr[mesh->FList[0].VertexIndex[2]].y;
			v2.z = mesh->VBptr[mesh->FList[0].VertexIndex[2]].z;

			if (t3dVectTriangleIntersection(&inter, line, *a, v0, v1, v2, *n)) { //return 1 if the vector from start to end pass trought the triangle v1,v2,v3
				mesh->VBptr = nullptr;
				return mesh->PortalList;
			}
			v0.x = mesh->VBptr[mesh->FList[1].VertexIndex[0]].x;
			v0.y = mesh->VBptr[mesh->FList[1].VertexIndex[0]].y;
			v0.z = mesh->VBptr[mesh->FList[1].VertexIndex[0]].z;
			v1.x = mesh->VBptr[mesh->FList[1].VertexIndex[1]].x;
			v1.y = mesh->VBptr[mesh->FList[1].VertexIndex[1]].y;
			v1.z = mesh->VBptr[mesh->FList[1].VertexIndex[1]].z;
			v2.x = mesh->VBptr[mesh->FList[1].VertexIndex[2]].x;
			v2.y = mesh->VBptr[mesh->FList[1].VertexIndex[2]].y;
			v2.z = mesh->VBptr[mesh->FList[1].VertexIndex[2]].z;

			if (t3dVectTriangleIntersection(&inter, line, *a, v0, v1, v2, *n)) { //return 1 if the vector from start to end pass trought the triangle v1,v2,v3
				mesh->VBptr = nullptr;
				return mesh->PortalList;
			}
			mesh->VBptr = nullptr;
		}
	}

	return nullptr;
}

/* -----------------08/03/2000 15.08-----------------
 *                  QueueMaterialList
 * --------------------------------------------------*/
void QueueMaterialList(MaterialTable &MatList, unsigned int NumMat, signed short int ViewMatrixNum) {
	uint32 i;

	if (!NumMat) return;

	for (i = 0; i < t3dNumMaterialLists; i++)
		if (t3dMaterialList[i].mat_list == &MatList)
			break;

	if ((i >= t3dNumMaterialLists) || (t3dMaterialList[i].num_mat != NumMat) || (t3dMaterialList[i].matrix != ViewMatrixNum)) {
		if ((t3dNumMaterialLists + 1) >= MAX_MATERIAL_LISTS) {
			DebugLogWindow("Too mach material lists! %d MAX is %d", t3dNumMaterialLists + 1, MAX_MATERIAL_LISTS);
			return ;
		}

		t3dMaterialList[t3dNumMaterialLists].mat_list = &MatList;
		t3dMaterialList[t3dNumMaterialLists].num_mat = NumMat;
		t3dMaterialList[t3dNumMaterialLists].matrix = ViewMatrixNum;
		t3dNumMaterialLists ++;
	}
}

/* -----------------08/03/2000 15.13-----------------
 *                  ProcessMaterialList
 * --------------------------------------------------*/
void ProcessMaterialList() {
	for (uint32 i = 0; i < t3dNumMaterialLists; i++) {
		rBuildMaterialList(*t3dMaterialList[i].mat_list, t3dMaterialList[i].num_mat, t3dMaterialList[i].matrix);
	}
}

/* -----------------10/06/99 15.50-------------------
 *                  SortMesh
 * --------------------------------------------------*/
static int SortMesh(const void *element1, const void *element2) {
	const t3dMESH * const *mm1 = (const t3dMESH * const *)(element1);
	const t3dMESH * const *mm2 = (const t3dMESH * const *)(element2);

	const t3dMESH *m1 = *mm1;
	const t3dMESH *m2 = *mm2;

	/*  DebugLogWindow("%s",m1->Name);
	    DebugLogWindow("%s",m2->Name);
	    DebugLogWindow("----------------");*/
	if (m1->BBoxAverageZ > m2->BBoxAverageZ)
		return 1;
	else if (m1->BBoxAverageZ < m2->BBoxAverageZ)
		return -1;
	else
		return 0;
}

/* -----------------10/06/99 15.51-------------------
 *                  t3dSortMeshes
 * --------------------------------------------------*/
void t3dSortMeshes() {
	qsort((void *)&t3d_VisibleMeshes[0], (size_t)t3d_NumMeshesVisible, sizeof(t3dMESH *),
	      SortMesh);
}

/* -----------------10/06/99 15.43-------------------
 *                  t3dProcessPortal
 * --------------------------------------------------*/
void t3dProcessPortal(t3dMESH *mesh, t3dCAMERA *cam) {
	t3dV3F      appo, c0;
	t3dV3F      ppv1, ppv2, ppv3, ppv4;
	uint32      p;
	t3dNORMAL   OldClipPlanes[NUMCLIPPLANES];

	if (!mesh->VertexBuffer)
		return ;

	mesh->VBptr = mesh->VertexBuffer;
	ppv1.x = mesh->VBptr[mesh->FList[0].VertexIndex[0]].x;
	ppv1.y = mesh->VBptr[mesh->FList[0].VertexIndex[0]].y;
	ppv1.z = mesh->VBptr[mesh->FList[0].VertexIndex[0]].z;
	ppv2.x = mesh->VBptr[mesh->FList[0].VertexIndex[1]].x;
	ppv2.y = mesh->VBptr[mesh->FList[0].VertexIndex[1]].y;
	ppv2.z = mesh->VBptr[mesh->FList[0].VertexIndex[1]].z;
	ppv3.x = mesh->VBptr[mesh->FList[0].VertexIndex[2]].x;
	ppv3.y = mesh->VBptr[mesh->FList[0].VertexIndex[2]].y;
	ppv3.z = mesh->VBptr[mesh->FList[0].VertexIndex[2]].z;

	for (p = 0; p < 3; p++) {
		appo.x = mesh->VBptr[mesh->FList[1].VertexIndex[p]].x;
		appo.y = mesh->VBptr[mesh->FList[1].VertexIndex[p]].y;
		appo.z = mesh->VBptr[mesh->FList[1].VertexIndex[p]].z;

		if ((ppv1 != appo) && (ppv2 != appo) && (ppv3 != appo))
			memcpy(&ppv4, &appo, sizeof(t3dV3F));
	}

	memcpy(OldClipPlanes, ClipPlanes, sizeof(t3dNORMAL)*NUMCLIPPLANES);

	t3dVectSub(&ppv4, &ppv4, &cam->Source);
	t3dVectSub(&ppv3, &ppv3, &cam->Source);
	t3dVectSub(&ppv2, &ppv2, &cam->Source);
	t3dVectSub(&ppv1, &ppv1, &cam->Source);
	t3dVectTransform(&ppv1, &ppv1, &t3dCurViewMatrix);
	t3dVectTransform(&ppv2, &ppv2, &t3dCurViewMatrix);
	t3dVectTransform(&ppv3, &ppv3, &t3dCurViewMatrix);
	t3dVectTransform(&ppv4, &ppv4, &t3dCurViewMatrix);
	t3dVectFill(&c0, 0.0f);

//	t3dVectCopy(&c0,&cam->Source);

	t3dPlaneNormal(&ClipPlanes[RIGHTCLIP],  &c0, &ppv3, &ppv4);
	t3dPlaneNormal(&ClipPlanes[LEFTCLIP],   &c0, &ppv1, &ppv2);
	t3dPlaneNormal(&ClipPlanes[TOPCLIP],    &c0, &ppv4, &ppv1);
	t3dPlaneNormal(&ClipPlanes[BOTTOMCLIP], &c0, &ppv2, &ppv3);

	mesh->VBptr = nullptr;

	for (auto &rejectedMesh : mesh->RejectedMeshes) {
		rejectedMesh->Flags |= T3D_MESH_INVISIBLEFROMSECT;
	}

	if (mesh->Flags & T3D_MESH_VIEWONLYPORTAL) bViewOnlyPortal = TRUE;
	if (!t3dTransformBody(mesh->PortalList)) {
		DebugLogWindow("Can't transform %s", mesh->PortalList->name.c_str());
	}
	bViewOnlyPortal = FALSE;

	for (auto &rejectedMesh : mesh->RejectedMeshes) {
		rejectedMesh->Flags &= ~T3D_MESH_INVISIBLEFROMSECT;
	}

	memcpy(ClipPlanes, OldClipPlanes, sizeof(t3dNORMAL)*NUMCLIPPLANES);
}


/* -----------------10/06/99 15.51-------------------
 *                  t3dProcessPortals
 * --------------------------------------------------*/
void t3dProcessPortals() {
	uint32 i;

	if (t3dNumPortals > 5)
		DebugLogWindow("Current numportals: %d", t3dNumPortals);

	bPortalRoom = TRUE;
	for (i = 0; i < t3dNumPortals; i++)
		t3dProcessPortal(t3dPortalList[i], t3dCurCamera);
	bPortalRoom = FALSE;
}

/* -----------------10/06/99 15.47-------------------
 *                  t3dAddTriangle
 * --------------------------------------------------*/
void t3dAddTriangle(t3dF32 x1, t3dF32 y1, t3dF32 x2, t3dF32 y2, t3dF32 x3, t3dF32 y3,
                    int32 r, int32 g, int32 b, int32 a) {
	rAddTrianglesArray(x1, y1, r, g, b, a);
	rAddTrianglesArray(x2, y2, r, g, b, a);
	rAddTrianglesArray(x3, y3, r, g, b, a);
}

/* -----------------10/06/99 15.47-------------------
 *                  t3dAddQuad
 * --------------------------------------------------*/
void t3dAddQuad(t3dF32 x1, t3dF32 y1, t3dF32 x2, t3dF32 y2, t3dF32 x3, t3dF32 y3, t3dF32 x4, t3dF32 y4,
                int32 r, int32 g, int32 b, int32 a) {
	t3dAddTriangle(x1, y1, x2, y2, x3, y3, r, g, b, a);
	t3dAddTriangle(x2, y2, x4, y4, x3, y3, r, g, b, a);
}

} // End of namespace Watchmaker
