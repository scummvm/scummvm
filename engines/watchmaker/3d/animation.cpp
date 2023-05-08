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

#define FORBIDDEN_SYMBOL_EXCEPTION_strcat
#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy

#include "watchmaker/3d/animation.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/file_utils.h"
#include "watchmaker/game.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/t3d.h"
#include "watchmaker/types.h"
#include "watchmaker/utils.h"
#include "watchmaker/windows_hacks.h"

/* -----------------16/12/98 10.32-------------------
 *              PRELOADEDANIMS
 * --------------------------------------------------*/
#define MAX_BONES               40
#define MAX_PRELOADED_ANIMS     8
#define A3DFILEVERSION          5
#define SCALE_DEFAULT_ANIM      1
#define SCALE_ANIM              3

namespace Watchmaker {

struct t3dLOADBONE {
	t3dV3F *Trasl;
	t3dV3F *Euler;
	uint32 NumBone;
};

struct t3dLOADANIM {
	Common::String name;
	uint32 NumFrames = 0, NumBones = 0, HiBone = 0, LastTime = 0;
	t3dF32 *Dist = nullptr;
	t3dLOADBONE Bone[MAX_BONES] = {};
};

t3dLOADANIM PreloadedAnim[MAX_PRELOADED_ANIMS];

/* -----------------30/12/98 10.56-------------------
 *                  t3dMatRotXYZ
 * --------------------------------------------------*/
void t3dMatRotXYZ(t3dM3X3F *dest, t3dF32 x, t3dF32 y, t3dF32 z) {
	t3dM3X3F    matrix, matrix_x, matrix_y, matrix_z;

	t3dMatIdentity(&matrix_x);
	t3dMatIdentity(&matrix_y);
	t3dMatIdentity(&matrix_z);

	matrix_x.M[4] = (float)cos(x);
	matrix_x.M[5] = (float)sin(x);
	matrix_x.M[7] = -(float)sin(x);
	matrix_x.M[8] = (float)cos(x);

	matrix_y.M[0] = (float)cos(y);
	matrix_y.M[2] = -(float)sin(y);
	matrix_y.M[6] = (float)sin(y);
	matrix_y.M[8] = (float)cos(y);

	matrix_z.M[0] = (float)cos(z);
	matrix_z.M[1] = (float)sin(z);
	matrix_z.M[3] = -(float)sin(z);
	matrix_z.M[4] = (float)cos(z);

	t3dMatMul(&matrix, &matrix_x, &matrix_y);
	t3dMatMul(&matrix, &matrix, &matrix_z);

	dest->M[0] = matrix.M[0];
	dest->M[2] = matrix.M[1];
	dest->M[1] = matrix.M[2];
	dest->M[6] = matrix.M[3];
	dest->M[8] = matrix.M[4];
	dest->M[7] = matrix.M[5];
	dest->M[3] = matrix.M[6];
	dest->M[5] = matrix.M[7];
	dest->M[4] = matrix.M[8];
}

Common::Array<t3dPLIGHT> t3dBODY::getPositionalLight(uint8 pos) {
	Common::Array<t3dPLIGHT> result;
	for (auto light : PosLightTable) {
		if (light.Num == pos) {
			result.push_back(light);
		}
	}
	return result;
}

/* -----------------04/07/98 15.52-------------------
 *                  GetLightPosition
 * --------------------------------------------------*/
uint8 GetLightPosition(t3dV3F *dest, uint8 pos) {
	if (!pos) return 0;

	auto pLights = t3dCurRoom->getPositionalLight(pos);
	dest->y = CurFloorY;
	for (auto light : pLights) {
		if (light.Pos.x && light.Pos.z) {
			dest->x = light.Pos.x;
			dest->z = light.Pos.z;
			return pos;
		}
	}
	if (pos != 99)
		warning("Can't find lpos %d in %s", pos, t3dCurRoom->name.c_str());
	return 0;
}

/* -----------------04/07/98 15.52-------------------
 *                  GetLightPosition
 * --------------------------------------------------*/
uint8 GetLightDirection(t3dV3F *dest, uint8 pos) {
	if (!pos) return 0;

	auto pLights = t3dCurRoom->getPositionalLight(pos);
	dest->y = CurFloorY;
	for (auto light : pLights) {
		if (light.Dir.x && light.Dir.z) {
			dest->x = light.Dir.x;
			dest->z = light.Dir.x;
			return pos;
		}
	}
	if (pos != 99)
		warning("Can't find ldir %d in %s", pos, t3dCurRoom->name.c_str());
	return 0;
}


/* -----------------15/12/98 16.26-------------------
 *                  t3dLoadAnimation
 * --------------------------------------------------*/
int8 t3dLoadAnimation(WGame &game, const char *s, t3dMESH *mesh, uint16 Flag) {
	uint32 nf, nb, i, k, h, older, ScaleAnim, CurPreloadedAnim;
	uint32 j = 0;
	t3dLOADANIM *p;
	t3dLOADBONE *bone;
	t3dBONEANIM *db;
	t3dBONE *b;
	t3dV3F t;
	t3dF32 c;

//	Prova a vedere se l'ho gia' precaricata
	for (CurPreloadedAnim = 0; CurPreloadedAnim < MAX_PRELOADED_ANIMS; CurPreloadedAnim++)
		if (PreloadedAnim[CurPreloadedAnim].NumFrames)
			if (PreloadedAnim[CurPreloadedAnim].name.equalsIgnoreCase(s))
				break;
//	Se la devo precaricare, cerco quella piu' vecchia e la scarico
	if (CurPreloadedAnim >= MAX_PRELOADED_ANIMS) {
		older = 0;
//		Prima cerco se ci sono ancora degli slot liberi
		for (CurPreloadedAnim = 0; CurPreloadedAnim < MAX_PRELOADED_ANIMS; CurPreloadedAnim++) {
			if (!PreloadedAnim[CurPreloadedAnim].NumFrames)
				break;
			else if (!(older) || (older > PreloadedAnim[CurPreloadedAnim].LastTime))
				older = PreloadedAnim[j = CurPreloadedAnim].LastTime;
		}
//		Se non c'erano slot liberi, rilascia vecchia animazione precaricata
		if (CurPreloadedAnim >= MAX_PRELOADED_ANIMS) {
			CurPreloadedAnim = j;
//t         DebugFile( "Precarico animazione %s nello slot %d occupato da %s", s, CurPreloadedAnim, PreloadedAnim[j].Name );
//			Disalloca tutto
			for (i = 0; i < MAX_BONES; i++) {
				t3dFree(PreloadedAnim[j].Bone[i].Trasl);
				t3dFree(PreloadedAnim[j].Bone[i].Euler);
			}
			delete[] PreloadedAnim[j].Dist;
			PreloadedAnim[j] = t3dLOADANIM();
		}
//t     else
//t         DebugFile( "Precarico animazione %s nello slot libero %d", s, CurPreloadedAnim );

		p = &PreloadedAnim[CurPreloadedAnim];
		p->name = s;

//		Carica la nuova animazione
		Common::String name = game.workDirs._a3dDir + replaceExtension(s, "a3d");

		{
			auto stream = game.resolveFile(name.c_str());
			if (!stream) {
				warning("File %s not found", name.c_str());
				return -1;
			}

			if ((i = stream->readByte()) != A3DFILEVERSION) {
				warning("%s file incompatible: current version: %d.\tFile version: %d", name.c_str(), A3DFILEVERSION, i);
				return -1;
			}

			nb = stream->readSint16LE();
			nf = stream->readSint16LE();
			if (nf == 0) {
				warning("%s has N0 frames!", name.c_str());
				return -1;
			}
			if (nb >= MAX_BONES) {
				warning("%s has too many bones (%d, MAX is %d)!", name.c_str(), j, MAX_BONES);
				return -1;
			}
			p->NumBones = nb;
			p->NumFrames = nf;

			for (i = 0; i < nb; i++) {
				j = (uint32)(stream->readByte());
				if (!(p->HiBone) || (p->HiBone < j))
					p->HiBone = j;

				bone = &p->Bone[i];
				bone->NumBone = j;
				bone->Euler = t3dCalloc<t3dV3F>(nf);
				bone->Trasl = t3dCalloc<t3dV3F>(nf);

				for (k = 0; k < nf; k++) {
					bone->Euler[k] = t3dV3F(*stream);
				}

				for (k = 0; k < nf; k++) {
					bone->Trasl[k] = t3dV3F(*stream);
				}
			}
			if (stream->readByte()) {
				p->Dist = new t3dF32[nf]{};
				for (k = 0; k < nf; k++)
					p->Dist[k] = stream->readFloatLE();
			}
		} // Close file
	}
//t else
//t     DebugFile( "Animazione %s gia' precaricata nello slot %d", s, CurPreloadedAnim );

	p = &PreloadedAnim[CurPreloadedAnim];
//	Scrive l'ultima volta che l'ho usata
	p->LastTime = t3dReadTime();

//	Finalmente copia l'animazione precaricata nella mesh
	if (Flag & T3D_MESH_DEFAULTANIM) {
		db = &mesh->DefaultAnim;
		mesh->Flags |= T3D_MESH_DEFAULTANIM;
		if (db) mesh->releaseAnim(T3D_MESH_DEFAULTANIM);
		if (db) mesh->releaseAnim(0);
		ScaleAnim = SCALE_DEFAULT_ANIM;
		db->NumFrames = p->NumFrames;
	} else {
		db = &mesh->Anim;
		mesh->Flags &= ~T3D_MESH_DEFAULTANIM;
		if (db) mesh->releaseAnim(0);
		ScaleAnim = SCALE_ANIM;
		db->NumFrames = (p->NumFrames - 2) * ScaleAnim + 2;
	}
	if (db->BoneTable) mesh->releaseAnim(0);
	mesh->NumNormals = 0;
	db->NumBones = 0;
	db->BoneTable = nullptr;
	db->BoneTable = t3dCalloc<t3dBONE>(p->HiBone + 1);
	db->NumBones = p->HiBone + 1;

	c = 1.0f / (t3dF32)(ScaleAnim);
	for (i = 0; i < p->NumBones; i++) {
		bone = &p->Bone[i];
		b = &db->BoneTable[bone->NumBone];

		b->Matrix = t3dCalloc<t3dM3X3F>(db->NumFrames);
		b->Trasl = t3dCalloc<t3dV3F>(db->NumFrames);

		for (k = 0; k < db->NumFrames; k++) {
			j = ((k - 1) / ScaleAnim) + 1;
			h = ((k - 1) % ScaleAnim);

			if ((!h) || (k < 1)) {
				if (k < 1)
					j = k;
				t3dMatRotXYZ(&b->Matrix[k], bone->Euler[j].x, bone->Euler[j].y, bone->Euler[j].z);
				memcpy(&b->Trasl[k], &bone->Trasl[j], sizeof(t3dV3F));
			} else {
				t3dVectSub(&t, &bone->Euler[j + 1], &bone->Euler[j]);
				if ((t.x <  T3D_2PI) && (t.x >  T3D_PI)) t.x = t.x - T3D_2PI;
				if ((t.x > -T3D_2PI) && (t.x < -T3D_PI)) t.x = t.x + T3D_2PI;
				if ((t.y <  T3D_2PI) && (t.y >  T3D_PI)) t.y = t.y - T3D_2PI;
				if ((t.y > -T3D_2PI) && (t.y < -T3D_PI)) t.y = t.y + T3D_2PI;
				if ((t.z <  T3D_2PI) && (t.z >  T3D_PI)) t.z = t.z - T3D_2PI;
				if ((t.z > -T3D_2PI) && (t.z < -T3D_PI)) t.z = t.z + T3D_2PI;
				t *= (c * (t3dF32)(h));
				t3dVectAdd(&t, &bone->Euler[j], &t);
				t3dMatRotXYZ(&b->Matrix[k], t.x, t.y, t.z);

				t3dVectSub(&t, &bone->Trasl[j + 1], &bone->Trasl[j]);
				t *= (c * (t3dF32)(h));
				t3dVectAdd(&b->Trasl[k], &bone->Trasl[j], &t);
			}

			/*      if(!(mesh->Flags&T3D_MESH_CHARACTER))
			        DebugFile("%3d;%3d;%9f;%9f;%9f;%9f;%9f;%9f;%9f;%9f;%9f;%9f;%9f;%9f;",k,i,
			            (bone->Euler[j].x)*180.0f/T3D_PI,(bone->Euler[j].y)*180.0f/T3D_PI,(bone->Euler[j].z)*180.0f/T3D_PI,
			            b->Matrix[k].M[0],b->Matrix[k].M[1],b->Matrix[k].M[2],
			            b->Matrix[k].M[3],b->Matrix[k].M[4],b->Matrix[k].M[5],
			            b->Matrix[k].M[6],b->Matrix[k].M[7],b->Matrix[k].M[8] );
			*/
		}

		b->ModVertices.clear();
//		Poi inserisce tutti i vertici modificati nell'array gia' alloocato della dimensione giusta
		for (auto &modVertices : mesh->ModVertices) {
			if (modVertices.NumBone == bone->NumBone) {
				b->ModVertices.push_back(modVertices.NumVert);
			}
		}
	}
	if (p->Dist) {
		db->Dist = new t3dF32[db->NumFrames]{};
		for (k = 0; k < db->NumFrames; k++)
			db->Dist[k] = p->Dist[k];
	}
	return 1;
}

/* -----------------30/12/98 11.27-------------------
 *                  FixupAnim
 * --------------------------------------------------*/
void FixupAnim(t3dMESH *mesh, uint8 pos, const char *room) {
	t3dBONEANIM *db;
	t3dBONE *bone, *bone0;
	t3dV3F lp, ld, Frame0Trasl, cc, tmp, tmp1, tmp2, zero;
	t3dM3X3F lm, mx, BoneInitMatrix;
	uint32 i, k, frame;
	t3dBODY *OldCurRoom = t3dCurRoom;

	if (mesh->Flags & T3D_MESH_DEFAULTANIM) {
		db = &mesh->DefaultAnim;
		pos = 0;
	} else {
		db = &mesh->Anim;
		if (pos) {
			if (room && (room[0] != '\0')) {
				t3dBODY *roomPtr = _vm->_roomManager->getRoomIfLoaded(room);
				if (room) {
					t3dCurRoom = roomPtr;
				}
			}
			if (!GetLightPosition(&lp, pos) || (lp.x == 0.0f) || (lp.z == 0.0f)) pos = 0;
			if (!GetLightDirection(&ld, pos) || (ld.x == 0.0f) || (ld.z == 0.0f)) pos = 0;
			t3dCurRoom = OldCurRoom;

			t3dVectSub(&ld, &ld, &lp);
			ld.z = -ld.z;
			t3dVectAdd(&ld, &ld, &lp);
			t3dMatView(&lm, &lp, &ld);

			if ((!pos) || (mesh->Flags & (T3D_MESH_ABS_ANIM | T3D_MESH_CHARACTER))) {
				t3dVectCopy(&lp, &mesh->Trasl);
				t3dMatCopy(&lm, &mesh->Matrix);
				pos = 99;
			}

			if (mesh->Flags & T3D_MESH_ABS_ANIM)
				t3dVectTransform(&cc, &CharCorrection, &lm);
		}
	}
//	Ora sistema tutte le altre bones 1..32 (mesh) e 33/34 (camera)
	for (i = 1; i < db->NumBones; i++) {
		if (!(bone = &db->BoneTable[i]) || !(bone->Trasl) || !(bone->Matrix)) continue;
//		Salva la prima matrice di ogni bone
		t3dMatCopy(&BoneInitMatrix, &bone->Matrix[0]);
//		Calcola scostamento iniziale bone per azioni assolute personaggi
		if (i == 1) t3dVectSub(&Frame0Trasl, &bone->Trasl[1], &bone->Trasl[0]);

		for (k = 0; k < db->NumFrames; k++) {
//			Tutte le matrici diventano relative al frame 0
			t3dMatMulInv(&bone->Matrix[k], &bone->Matrix[k], &BoneInitMatrix);
//			Aggiunge la correzione a:
//			- Azioni di default (tutti frames personaggi)
//			- Azioni relative (tutti frames, personaggi e oggetti)
//			- Azioni assolute (personaggi frame 0)
			if ((mesh->Flags & T3D_MESH_DEFAULTANIM) || !(mesh->Flags & T3D_MESH_ABS_ANIM) ||
			        ((!k) && (mesh->Flags & T3D_MESH_ABS_ANIM) && (mesh->Flags & T3D_MESH_CHARACTER)))
				t3dVectAdd(&bone->Trasl[k], &CharCorrection, &bone->Trasl[k]);

			if (pos) {
//				Oggetti relativi
				if (!(mesh->Flags & T3D_MESH_CHARACTER) && !(mesh->Flags & T3D_MESH_ABS_ANIM)) {
					t3dVectTransform(&ld, &bone->Trasl[k], &lm);
					t3dVectAdd(&bone->Trasl[k], &ld, &lp);
				}
//				Personaggi assoluti
				else if ((mesh->Flags & T3D_MESH_ABS_ANIM) && (mesh->Flags & T3D_MESH_CHARACTER) && (k)) {
					t3dVectSub(&bone->Trasl[k], &bone->Trasl[k], &Frame0Trasl);
					t3dVectTransformInv(&bone->Trasl[k], &bone->Trasl[k], &lm);
					t3dVectAdd(&bone->Trasl[k], &bone->Trasl[k], &CharCorrection);

					t3dMatMul(&bone->Matrix[k], &bone->Matrix[k], &lm);
				}
			}
			/*      if(!(mesh->Flags&T3D_MESH_CHARACTER))
			        DebugFile("%3d;%3d;%9f;%9f;%9f;%9f;%9f;%9f;%9f;%9f;%9f;",k,i,
			            bone->Matrix[k].M[0],bone->Matrix[k].M[1],bone->Matrix[k].M[2],
			            bone->Matrix[k].M[3],bone->Matrix[k].M[4],bone->Matrix[k].M[5],
			            bone->Matrix[k].M[6],bone->Matrix[k].M[7],bone->Matrix[k].M[8] );
			*/
		}
	}

	if (db->Dist)
		for (k = 0; k < db->NumFrames; k++)
			if ((mesh->Flags & T3D_MESH_CHARACTER) && ((!k) || (mesh->Flags & T3D_MESH_DEFAULTANIM)))
				db->Dist[k] -= CharCorrection.z;

	if (!(bone0 = &db->BoneTable[0]) || !(bone0->Trasl) || !(bone0->Matrix)) {
		bone0->Matrix = t3dCalloc<t3dM3X3F>(db->NumFrames);
		bone0->Trasl = t3dCalloc<t3dV3F>(db->NumFrames);
	} else
		warning("Guarda che il bone0 e' gia' stato allocato nella mesh %s", mesh->name.c_str());

	for (k = 0; k < db->NumFrames; k++) {
		t3dVectCopy(&bone0->Trasl[k], &mesh->Trasl);
		t3dMatCopy(&bone0->Matrix[k], &mesh->Matrix);
	}

	if ((mesh->Flags & T3D_MESH_CHARACTER) && !(mesh->Flags & T3D_MESH_DEFAULTANIM)) {
		if (!(bone = &db->BoneTable[1]) || !(bone->Trasl) || !(bone->Matrix))
			return ;

		t3dVectInit(&tmp1, bone->Trasl[1].x, 0.0f, bone->Trasl[1].z);
		t3dVectInit(&tmp, 0.0f, 0.0f, 1.0f);
		t3dVectTransform(&tmp, &tmp, &bone->Matrix[1]);
		tmp.y = 0;
		t3dVectFill(&zero, 0.0f);
		t3dMatView(&mx, &zero, &tmp);
		t3dVectTransform(&cc, &CharCorrection, &mx);
		t3dVectSub(&tmp1, &tmp1, &cc);

		for (frame = 0; frame < db->NumFrames; frame++) {
			t3dVectInit(&tmp2, bone->Trasl[frame].x, 0.0f, bone->Trasl[frame].z);
			t3dVectInit(&tmp, 0.0f, 0.0f, 1.0f);
			t3dVectTransform(&tmp, &tmp, &bone->Matrix[frame]);
			tmp.y = 0;
			t3dVectFill(&zero, 0.0f);
			t3dMatView(&mx, &zero, &tmp);
			t3dVectTransform(&cc, &CharCorrection, &mx);
			t3dVectSub(&tmp2, &tmp2, &cc);

			t3dVectSub(&tmp, &tmp2, &tmp1);
//			t3dVectTransform( &tmp, &tmp, &mesh->Matrix );

			tmp.x += bone->Trasl[1].x - bone->Trasl[0].x;
			tmp.z += bone->Trasl[1].z - bone->Trasl[0].z;

			t3dVectCopy(&bone0->Trasl[frame], &tmp);
			t3dVectInit(&tmp, 0.0f, 0.0f, -1.0f);
			t3dVectTransform(&tmp, &tmp, &bone->Matrix[frame]);
			tmp.z = -tmp.z;
			tmp.y = 0;
			t3dVectFill(&zero, 0.0f);
			t3dMatView(&bone0->Matrix[frame], &zero, &tmp);
		}

		for (i = 1; i < db->NumBones; i++) {
			if (!(bone = &db->BoneTable[i]) || !(bone->Trasl) || !(bone->Matrix))
				continue;

			for (k = 0; k < db->NumFrames; k++) {
				t3dVectSub(&bone->Trasl[k], &bone->Trasl[k], &bone0->Trasl[k]);
				t3dVectTransform(&bone->Trasl[k], &bone->Trasl[k], &bone0->Matrix[k]);
				t3dMatMulInv(&bone->Matrix[k], &bone->Matrix[k], &bone0->Matrix[k]);
			}
		}

		for (k = 0; k < db->NumFrames; k++) {
			t3dMatMulInv(&bone0->Matrix[k], &mesh->Matrix, &bone0->Matrix[k]);
			t3dVectTransform(&tmp, &bone0->Trasl[k], &mesh->Matrix);
			t3dVectAdd(&bone0->Trasl[k], &mesh->Trasl,  &tmp);
		}
	}
}

/* -----------------13/04/99 14.57-------------------
 *                  LoadShadowMeshes
 * --------------------------------------------------*/
t3dBODY *LoadShadowMeshes(WGame &game, const char *pname, t3dBODY *Body) {
	uint16  ref;
	char    Name[255];
	t3dBODY *shadow = new t3dBODY();
	gVertex *Original = Body->MeshTable[0].VertexBuffer;
	t3dF32  dist, rez;

	strcpy(Name, pname);
	strncpy(&Name[strlen(pname) - 4], "_Shadow.t3d\0", 12);
	uint16 numBodys = 0;
	shadow = _vm->_roomManager->loadRoom(Name, shadow, &numBodys, (T3D_NOLIGHTMAPS | T3D_NORECURSION | T3D_NOVOLUMETRICLIGHTS | T3D_NOCAMERAS | T3D_NOBOUNDS | T3D_STATIC_SET0 | T3D_STATIC_SET1));
	if (!shadow) return nullptr;

	for (uint16 i = 0; i < shadow->NumMeshes(); i++) {
		t3dMESH &m = shadow->MeshTable[i];
		m.VBptr = m.VertexBuffer;
		for (uint16 j = 0; j < m.NumFaces(); j++) {
			t3dFACE &f = m.FList[j];
			for (uint16 n = 0; n < 3; n++) {
				t3dV3F  pnt;
				pnt.x = m.VBptr[f.VertexIndex[n]].x;
				pnt.y = m.VBptr[f.VertexIndex[n]].y;
				pnt.z = m.VBptr[f.VertexIndex[n]].z;

				ref = 0;
				dist = 999999999.0f;
				for (uint16 k = 0; k < Body->MeshTable[0].NumVerts; k++) {
					t3dV3F  tpnt;
					tpnt.x = Original[k].x;
					tpnt.y = Original[k].y;
					tpnt.z = Original[k].z;
					if ((rez = t3dVectDistance(&pnt, &tpnt)) < dist) {
						dist = rez;
						ref = k;
					}
				}
				f.VertexIndex[n] = ref;
			}
		}
		m.VBptr = nullptr;
		delete[] m.VertexBuffer;
		m.VertexBuffer = nullptr;
		delete[] m.OldVertexBuffer;
		m.OldVertexBuffer = nullptr;
		delete[] m.SavedVertexBuffer;
		m.SavedVertexBuffer = nullptr;

		m.VertexBuffer = Body->MeshTable[0].VertexBuffer;
		m.NumVerts = Body->MeshTable[0].NumVerts;
		m.Flags |= T3D_MESH_CHARACTER; //this is a character
	}
	return shadow;
}


/* -----------------30/12/98 11.27-------------------
 *                  t3dLoadCharacter
 * --------------------------------------------------*/
t3dCHARACTER *t3dLoadCharacter(WGame &game, const char *pname, uint16 num) {
	warning("LoadCharacter(%s)", pname);
	uint8   Mirror = 1;
	uint16  numBody = 0, f;
	t3dV3F tmp;
	//  gVertex *v;

	t3dCHARACTER *b = new t3dCHARACTER[1] {};
	b->Body = _vm->_roomManager->loadRoom(pname, b->Body, &numBody, (T3D_NOLIGHTMAPS | T3D_NORECURSION | T3D_NOVOLUMETRICLIGHTS | T3D_NOCAMERAS | T3D_STATIC_SET0 | T3D_STATIC_SET1));
	if (!b->Body) {
		delete[] b;
		return nullptr;
	}
	b->Mesh = &b->Body->MeshTable[0];
	b->CurRoom = t3dCurRoom;
	b->Flags = T3D_CHARACTER_HIDE | T3D_CHARACTER_REALTIMELIGHTING;
	if (num >= 2) b->Flags |= T3D_CHARACTER_BNDHIDE;
	//Try to load animation
	if (t3dLoadAnimation(game, pname, b->Mesh, T3D_MESH_DEFAULTANIM) == -1) {
		warning("t3dLoadCharacter: Error loading %s", pname);
		delete[] b;
		return nullptr;
	}
	FixupAnim(b->Mesh, 0, "");

	// Zero's all the Normals vars, 'cause I recalc all the normals runtime...
	b->Body->NumNormals = 0;
	b->Body->NumVerticesNormals = 0;

	for (uint16 i = 0; i < b->Body->NumMeshes(); i++) {
		t3dMESH &mesh = b->Body->MeshTable[i];
		for (f = 0; f < mesh.NumFaces(); f++) {
			mesh.FList[f].n = nullptr;
		}
//sb
//sb        mesh->Flags|=T3D_MESH_CASTREALTIMESHADOWS;
//sb
	}
	b->Body->NList.clear();
//sb
//sb    b->Flags|=T3D_CHARACTER_CASTREALTIMESHADOWS;
//sb
//	Per gli specchi
	if (Mirror) {                                                                               // Ogni personaggio potrebbe apparire in uno specchio
		b->Body->MirrorMatTable.resize(b->Body->NumMaterials());

		rCopyMaterialList(b->Body->MirrorMatTable, b->Body->MatTable, b->Body->NumMaterials());
	}

//	Per le ombre, altezza e raggio del cilindro
	b->Height = (t3dF32)sqrt(b->Mesh->BBox[0].p.x * b->Mesh->BBox[0].p.x + b->Mesh->BBox[0].p.z * b->Mesh->BBox[0].p.z);
	b->Radius = (t3dF32)sqrt(b->Mesh->BBox[5].p.x * b->Mesh->BBox[5].p.x + b->Mesh->BBox[5].p.z * b->Mesh->BBox[5].p.z);
	if (b->Radius < b->Height) b->Radius = b->Height;
	b->Height = (b->Mesh->BBox[0].p.y - b->Mesh->BBox[2].p.y) * 1.2f;

	// No bounding box detection
	/*  t3dVectFill(&b->Mesh->BBox[0].p,0.0f);
	    t3dVectFill(&b->Mesh->BBox[1].p,0.0f);
	    t3dVectFill(&b->Mesh->BBox[2].p,0.0f);
	    t3dVectFill(&b->Mesh->BBox[3].p,0.0f);
	    t3dVectFill(&b->Mesh->BBox[4].p,0.0f);
	    t3dVectFill(&b->Mesh->BBox[5].p,0.0f);
	    t3dVectFill(&b->Mesh->BBox[6].p,0.0f);
	    t3dVectFill(&b->Mesh->BBox[7].p,0.0f);
	    b->Mesh->Flags|=T3D_MESH_NOBOUNDBOX;
	*/
	for (uint16 i = 0; i < b->Body->NumMeshes(); i++) {
		b->Body->MeshTable[i].Flags |= T3D_MESH_CHARACTER;
		b->Body->MeshTable[i].Flags &= ~T3D_MESH_MIRROR;
	}

	t3dVectFill(&b->Pos, 0.0f);
	t3dVectInit(&tmp, 0.0f, 0.0f, -1.0f);
	t3dVectAdd(&tmp, &b->Pos, &tmp);
	t3dMatView(&b->Mesh->Matrix, &b->Pos, &tmp);

	b->Mesh->Matrix.Flags &= ~T3D_MATRIX_IDENTITY;
	b->Mesh->CurFrame = 4;
	b->Mesh->LastFrame = 0;
	b->Mesh->BlendPercent = 255;

	b->Walk.OldPanel = -1;
	b->Walk.CurPanel = -1;
	b->Walk.NumPathNodes = -1;

	t3dVectInit(&b->Dir, 0.0f, 0.0f, -1.0f);
	t3dVectTransform(&b->Dir, &b->Dir, &b->Mesh->Matrix);    //rotate by Character angle

//sb
	if (num < 2)
		b->Shadow = LoadShadowMeshes(game, pname, b->Body);
	else
		b->Shadow = nullptr;
//sb
	return b;
}

/* -----------------25/09/98 16.07-------------------
 *                  GetFullLightPosition
 * --------------------------------------------------*/
uint8 GetFullLightDirection(t3dV3F *dest, uint8 pos) {
	if (!pos) return 0;

	auto pLights = t3dCurRoom->getPositionalLight(pos);
	for (auto light : pLights) {
		if (light.Dir.x && light.Dir.z) {
			*dest = light.Dir;
			return pos;
		}
	}

	if (pos != 99)
		DebugLogFile("Can't find fldir %d in %s", pos, t3dCurRoom->name.c_str());
	return 0;
}

/* -----------------21/12/98 16.40-------------------
 *                  ReleasePreloadedAnims
 * --------------------------------------------------*/
void ReleasePreloadedAnims() {
	int32 i, j;

	for (j = 0; j < MAX_PRELOADED_ANIMS; j++) {
//		Disalloca tutto
		for (i = 0; i < MAX_BONES; i++) {
			t3dFree(PreloadedAnim[j].Bone[i].Trasl);
			t3dFree(PreloadedAnim[j].Bone[i].Euler);
		}
		delete[] PreloadedAnim[j].Dist;
		PreloadedAnim[j] = t3dLOADANIM();
	}
}

/* -----------------02/05/00 9.30--------------------
 *              CompareLightPosition
 * --------------------------------------------------*/
uint8 CompareLightPosition(char *roomname, uint8 pos1, t3dV3F *pos2, t3dF32 acceptable_dist) {
	t3dV3F p1;
	t3dBODY *t;

	if ((pos1 <= 0) || (pos2 == nullptr)) return FALSE;

//	cerco la stanza
	t = nullptr;
	if (roomname && (roomname[0] != '\0')) {
		t = _vm->_roomManager->getRoomIfLoaded(roomname);
	} else t = t3dCurRoom;

	if (!t) return FALSE;

	auto pLights = t->getPositionalLight(pos1);
	bool foundLight = false;
	for (auto &light : pLights) {
		if (light.Pos.x && light.Pos.z) {
			p1.x = light.Pos.x;
			p1.y = light.Pos.y;
			p1.z = light.Pos.z;
			foundLight = true;
			break;
		}
	}
	if (!foundLight) return FALSE;

	if (t3dVectSquaredDistance(&p1, pos2) <= acceptable_dist) return TRUE;

	return FALSE;
}

} // End of namespace Watchmaker
