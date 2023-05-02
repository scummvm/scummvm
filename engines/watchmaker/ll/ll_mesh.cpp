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

#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/types.h"
#include "watchmaker/t3d.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/define.h"
#include "watchmaker/globvar.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/3d/animation.h"
#include "watchmaker/utils.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/walk/walkutil.h"
#include "watchmaker/walk/walk.h"
#include "watchmaker/ll/ll_system.h"

namespace Watchmaker {

// locals
struct t3dHEADMOVE {
	t3dV3F OldPos, DestAng, CurAng;
};

t3dHEADMOVE HeadMove[T3D_MAX_CHARACTERS];

t3dF32 OldArrowLen, OldExplosionScale;
uint32 **SavedBodyLight;
uint8 LastLightChar, LastLightRoom;
/* -----------------14/04/99 18.24-------------------
 *                  t3dUpdateArrow
 * --------------------------------------------------*/
void t3dUpdateArrow(t3dMESH *m, t3dF32 len) {
	uint32  i;

	if (!m) return;

	m->VBptr = m->VertexBuffer;
	for (i = 0; i < m->NumVerts; i++)
		if (fabs(m->VBptr[i].z) > 1.0f)
			m->VBptr[i].z += (-len + OldArrowLen);
	m->Flags |= T3D_MESH_UPDATEVB;
	m->VBptr = nullptr;

	OldArrowLen = len;
}

/* -----------------27/04/99 10.52-------------------
 *                  t3dLightRoom
 * --------------------------------------------------*/
void t3dLightRoom(Init &init, t3dBODY *b, t3dV3F *p, t3dF32 NearRange, t3dF32 FarRange, t3dF32 IperRange) {
	uint32 i, j, k, rr, gg, bb, aa, cr, cb, cg, *sbl;
	uint32 addr = 110;
	uint32 addg = 95;
	uint32 addb = 80;
	t3dMESH *m;
	t3dF32 dist;
	t3dV3F tmp;
	gVertex *gv;

	if (!b || !p) return;

	FarRange *= FarRange;
	NearRange *= NearRange;
	IperRange *= IperRange;

	if (!SavedBodyLight) {
		if (!(SavedBodyLight = (uint32 **)t3dMalloc(sizeof(uint32 *) * b->NumMeshes())))
			return ;
		m = &b->MeshTable[0];
		for (j = 0; j < b->NumMeshes(); j++, m++) {
			if (!m) continue;
			if (!(SavedBodyLight[j] = (uint32 *)t3dMalloc(sizeof(uint32) * m->NumVerts * 4)))
				continue;

			gv = m->VBptr;
			m->VBptr = m->VertexBuffer;
			for (i = 0; i < m->NumVerts; i++, gv++) {
				SavedBodyLight[j][i * 4 + 0] = RGBA_GETRED(gv->diffuse);
				SavedBodyLight[j][i * 4 + 1] = RGBA_GETGREEN(gv->diffuse);
				SavedBodyLight[j][i * 4 + 2] = RGBA_GETBLUE(gv->diffuse);
				SavedBodyLight[j][i * 4 + 3] = RGBA_GETALPHA(gv->diffuse);
			}
			m->VBptr = nullptr;
		}
	}

	LastLightRoom = (LastLightRoom + 1) > 3 ? 0 : LastLightRoom + 1;
	m = &b->MeshTable[0];
	for (j = 0; j < b->NumMeshes(); j++, m++) {
		if (!m) continue;
		if (m->name.equalsIgnoreCase("p50-cielo") || m->name.equalsIgnoreCase("p50-stelle") || m->name.equalsIgnoreCase("p50-luna")) continue;
		if (!(m->Flags & T3D_MESH_VISIBLE) && !(m->Flags & T3D_MESH_HIDDEN)) {
			m->Flags &= ~T3D_MESH_HIDDEN;
			continue;
		}
		m->Flags &= ~T3D_MESH_HIDDEN;
//		if( (j%4) != LastLightRoom ) continue;

		sbl = (uint32 *)&SavedBodyLight[j][0];
		for (k = 0; k < MAX_OBJ_MESHLINKS ; k++) {
			if (((init.Obj[oNEXTPORTAL].meshlink[k][0] != '\0') && (m->name.equalsIgnoreCase((const char *)init.Obj[oNEXTPORTAL].meshlink[k]))) ||
			        m->name.equalsIgnoreCase("p50-sentierini01") || m->name.equalsIgnoreCase("p50-sentierini02") || m->name.equalsIgnoreCase("p50-sentierini03") ||
			        m->name.equalsIgnoreCase("p50-sentierini04") || m->name.equalsIgnoreCase("p50-sentierini05") || m->name.equalsIgnoreCase("p50-sentierini06")) {
				tmp.x = m->Pos.x - p->x;
				tmp.z = m->Pos.z - p->z;
				gv = m->VBptr;
				m->VBptr = m->VertexBuffer;
				if ((dist = tmp.x * tmp.x + tmp.z * tmp.z) > (FarRange + m->Radius * m->Radius * 1.3f)) {
					if ((bGolfMode == 0) || (bGolfMode == 1))
						if (dist > (FarRange + m->Radius * m->Radius) * 2.5f) m->Flags |= T3D_MESH_HIDDEN;
					for (i = 0; i < m->NumVerts; i++, gv++) {
						rr = *sbl++;
						gg = *sbl++;
						bb = *sbl++;
						aa = *sbl++;
						gv->diffuse = RGBA_MAKE(rr, gg, bb, aa);
					}
				} else {
					for (i = 0; i < m->NumVerts; i++, gv++) {
						tmp.x = gv->x - p->x;
						tmp.z = gv->z - p->z;
						if ((dist = tmp.x * tmp.x + tmp.z * tmp.z) < IperRange) {
							rr = *sbl++ + addr * 2;
							gg = *sbl++ + addg * 2;
							bb = *sbl++ + addb * 2;
						} else if (dist < NearRange) {
							rr = *sbl++ + addr;
							gg = *sbl++ + addg;
							bb = *sbl++ + addb;
						} else if (dist < FarRange) {
							dist = 1.0f - (dist - NearRange) / (FarRange - NearRange);
							rr = *sbl++ + (uint32)((t3dF32)(addr) * dist);
							gg = *sbl++ + (uint32)((t3dF32)(addg) * dist);
							bb = *sbl++ + (uint32)((t3dF32)(addb) * dist);
						} else {
							rr = *sbl++;
							gg = *sbl++;
							bb = *sbl++;
						}
						aa = *sbl++;
						if (rr > 255) rr = 255;
						if (gg > 255) gg = 255;
						if (bb > 255) bb = 255;
						gv->diffuse = RGBA_MAKE(rr, gg, bb, aa);
					}
				}
				m->Flags |= T3D_MESH_UPDATEVB;
				m->VBptr = nullptr;
				break;
			}
		}
		if (k < MAX_OBJ_MESHLINKS) continue;

		tmp.x = m->Pos.x - p->x + m->Trasl.x;
		tmp.z = m->Pos.z - p->z + m->Trasl.z;
		if ((dist = tmp.x * tmp.x + tmp.z * tmp.z) < IperRange) {
			cr = addr * 2;
			cg = addg * 2;
			cb = addb * 2;
		} else if (dist < NearRange) {
			cr = addr;
			cg = addg;
			cb = addb;
		} else if (dist < FarRange) {
			dist = 1.0f - (dist - NearRange) / (FarRange - NearRange);
			cr = (uint32)((t3dF32)(addr) * dist);
			cg = (uint32)((t3dF32)(addg) * dist);
			cb = (uint32)((t3dF32)(addb) * dist);
		} else {
			if ((bGolfMode == 0) || (bGolfMode == 1))
				if (dist > (FarRange + m->Radius * m->Radius) * 2.5f) m->Flags |= T3D_MESH_HIDDEN;
			cr = 0;
			cg = 0;
			cb = 0;
		}
		gv = m->VBptr = m->VBptr = m->VertexBuffer;
		for (i = 0; i < m->NumVerts; i++, gv++) {
			rr = cr + *sbl++;
			gg = cg + *sbl++;
			bb = cb + *sbl++;
			aa = *sbl++;
			if (rr > 255) rr = 255;
			if (gg > 255) gg = 255;
			if (bb > 255) bb = 255;
			gv->diffuse = RGBA_MAKE(rr, gg, bb, aa);
		}
		m->Flags |= T3D_MESH_UPDATEVB;
		m->VBptr = nullptr;
	}
}


/* -----------------27/04/99 15.35-------------------
 *                  t3dLightChar
 * --------------------------------------------------*/
void t3dLightChar(t3dMESH *mesh, t3dV3F *p) {
#if 0
	int16 df;
	uint32 j, rr, gg, bb, cr, cb, cg;
	t3dF32 addr = 110 + 60;
	t3dF32 addg = 95 + 75;
	t3dF32 addb = 80 + 90;
	t3dF32 nlight;
	t3dV3F ppos, l, *normal;
	gVertex *gv;

	if (!mesh || !p) return;

	t3dVectAdd(&ppos, &mesh->Trasl, &mesh->Pos);
	gv = mesh->VBptr = mesh->VertexBuffer;

	cr = (t3dU32)t3dCurRoom->AmbientLight.x + 20;
	cg = (t3dU32)t3dCurRoom->AmbientLight.y + 20;
	cb = (t3dU32)t3dCurRoom->AmbientLight.z + 20;
	df = RGBA_MAKE(cr, cg, cb, 255);
	for (j = 0; j < mesh->NumVerts; j++, gv++)
		gv->diffuse = df;

	t3dVectSub(&l, p, &ppos);
	t3dVectTransformInv(&l, &l, &mesh->Matrix);
	t3dVectNormalize(&l);

	gv = mesh->VBptr;
	for (j = 0; j < mesh->NumVerts; j++, gv++) {
		normal = &mesh->NList[j]->n;

		nlight = t3dVectDot(normal, &l);
//		if( (nlight=t3dVectDot(normal,&l)) >= 0 )
		{
			rr = cr + t3dFloatToInt((addr * nlight));
			gg = cg + t3dFloatToInt((addg * nlight));
			bb = cb + t3dFloatToInt((addb * nlight));

			if (rr > 255) rr = 255;
			if (gg > 255) gg = 255;
			if (bb > 255) bb = 255;

			gv->diffuse = RGBA_MAKE(rr, gg, bb, 255);
		}
	}
	mesh->Flags |= T3D_MESH_UPDATEVB;
	mesh->VBptr = nullptr;
#endif
}

/* -----------------12/04/99 12.11-------------------
 *                  t3dVectMeshInters
 * --------------------------------------------------*/
uint8 t3dVectMeshInters(t3dMESH *m, t3dV3F start, t3dV3F end, t3dV3F *inters) {
	t3dV3F  v1, v2, v3;

	if (!m) return 0;
	if (!t3dVectPlaneIntersection(inters, start, end, m->BBoxNormal[3])) return 0;

	m->VBptr = m->VertexBuffer;
	for (uint32 j = 0; j < m->NumFaces(); j++) {
		t3dFACE &f = m->FList[j];
		if (!f.n) continue;

		v1.x = m->VBptr[f.VertexIndex[0]].x;
		v1.y = m->VBptr[f.VertexIndex[0]].y;
		v1.z = m->VBptr[f.VertexIndex[0]].z;
		v2.x = m->VBptr[f.VertexIndex[1]].x;
		v2.y = m->VBptr[f.VertexIndex[1]].y;
		v2.z = m->VBptr[f.VertexIndex[1]].z;
		v3.x = m->VBptr[f.VertexIndex[2]].x;
		v3.y = m->VBptr[f.VertexIndex[2]].y;
		v3.z = m->VBptr[f.VertexIndex[2]].z;

		if (t3dVectTriangleIntersection(inters, start, end, v1, v2, v3, *f.n)) {
			m->VBptr = nullptr;
			return 1;
		}
	}

	m->VBptr = nullptr;
	return 0;
}


/* -----------------19/05/00 12.43-------------------
 *              t3dMoveAndCheck1stCamera
 * --------------------------------------------------*/
bool t3dMoveAndCheck1stCamera(t3dBODY *rr, t3dCAMERA *cc, t3dV3F *mm) {
	t3dWALK *w;
	t3dV3F tmp;
	int32 i, j;

	if (!Character[ocCURPLAYER]) return FALSE;
	w = &Character[ocCURPLAYER]->Walk;

	t3dVectAdd(&tmp, &cc->Source, mm);
//	Controlla che non sia dentro un Bounding Box
	for (i = 0; i < (int32)rr->NumMeshes(); i++) {
		t3dMESH &mesh = rr->MeshTable[i];
		if (!(mesh.Flags & T3D_MESH_HIDDEN)) {
//			Se il punto di destinazione e' dentro il bound box (allargato dell'altezza del ginocchio)
			for (j = 0; j < 6; j++)
				if (t3dVectPlaneDistance(tmp, mesh.BBoxNormal[j]) < -KNEE_HEIGHT)
					break;

			if (j >= 6) {
//				Prima controlla che non sia dentro i bounds
				for (j = 0; j < w->PanelNum; j++) {
					if (PointInside(ocCURPLAYER, j, (double)tmp.x, (double)tmp.z) != 0) {
						warning("Inters %s", mesh.name.c_str());   // TODO: Debug
						return FALSE;
					}
				}
				warning("Saved by bounds");   // TODO: Debug
			}
		}
	}

//	evito che si entri nell'altro personaggio giocante
	i = (CurPlayer ^ 1);
	if (Character[i + ocDARRELL] && Character[i + ocDARRELL]->Mesh && t3dCurRoom->name.equalsIgnoreCase(PlayerStand[i].roomName)) { // Used to be stricmp
		t3dF32 d = t3dVectDistance(&tmp, &Character[i + ocDARRELL]->Mesh->Trasl);
		if (d < 435.f)  return FALSE;
	}

	t3dVectAdd(&cc->Source, &cc->Source, mm);
	t3dVectAdd(&cc->Target, &cc->Target, mm);
	return TRUE;
}

/* -----------------03/05/99 15.24-------------------
 *                  t3dClipToSurface
 * --------------------------------------------------*/
uint8 t3dClipToSurface(Init &init, t3dV3F *pt) {
	t3dV3F tmp, start, end;
	int32 i;
	t3dMESH *m;

	t3dVectCopy(&start, pt);
	start.y = 260000.0f;
	t3dVectCopy(&end, pt);
	end.y = -130000.0f;
	for (i = 0; i < 6; i++) {
		if ((m = LinkMeshToStr(init, (char *)init.Obj[oNEXTPORTAL].meshlink[i])) && (t3dVectMeshInters(m, start, end, &tmp))) {
			pt->y = tmp.y;
			return true;
		}
	}
	return false;
}

/* -----------------03/05/99 16.44-------------------
 *                  t3dUpdateExplosion
 * --------------------------------------------------*/
void t3dUpdateExplosion(t3dMESH *m, t3dF32 scale) {
//	t3dU32   i;

	if (!m) return;

	m->Matrix.M[0] = scale;
	m->Matrix.M[4] = scale;
	m->Matrix.M[8] = scale;

	/*  m->VBptr=m->VertexBuffer;
	    for (i=0; i<m->NumVerts; i++ )
	    {
	        m->VBptr[i].x *= (scale/OldExplosionScale);
	        m->VBptr[i].y *= (scale/OldExplosionScale);
	        m->VBptr[i].z *= (scale/OldExplosionScale);
	    }
	    m->Flags |= T3D_MESH_UPDATEVB;
	    m->VBptr = NULL;
	*/
	OldExplosionScale = scale;
}

/* -----------------03/09/98 17.42-------------------
 *                  UpdateBoundingBox
 * --------------------------------------------------*/
void UpdateBoundingBox(t3dMESH *mesh) {
	t3dBONEANIM *db;
	t3dBONE *bone;
	t3dV3F Appov;
	int32 i, frame;

	if (!mesh || (mesh->Flags & T3D_MESH_NOBOUNDBOX))return;

//	DebugFile("Update Bounding Box %s",mesh->Name);

	if (mesh->Flags & T3D_MESH_DEFAULTANIM)
		db = &mesh->DefaultAnim;
	else
		db = &mesh->Anim;

	frame = mesh->CurFrame;
	bone = db->BoneTable;
	for (i = 0; i < db->NumBones; i++, bone++) {
		if (!bone || !bone->Trasl || !bone->Matrix || (bone->ModVertices.size() > mesh->NumVerts)) continue;
		if ((!bone->ModVertices.empty()) && !(mesh->Flags & T3D_MESH_CHARACTER)) {
			for (i = 0; i < 8; i++) {
				t3dVectSub(&Appov, &mesh->BBox[i].p, &bone->Trasl[1]);
				t3dVectTransform(&Appov, &Appov, &bone->Matrix[1]);
				t3dVectTransformInv(&Appov, &Appov, &bone->Matrix[frame]);
				t3dVectAdd(&mesh->BBox[i].p, &Appov, &bone->Trasl[frame]);
			}

			t3dPlaneNormal(&mesh->BBoxNormal[0], &mesh->BBox[0].p, &mesh->BBox[2].p, &mesh->BBox[1].p);      //front
			t3dPlaneNormal(&mesh->BBoxNormal[1], &mesh->BBox[4].p, &mesh->BBox[5].p, &mesh->BBox[6].p);      //back
			t3dPlaneNormal(&mesh->BBoxNormal[2], &mesh->BBox[4].p, &mesh->BBox[0].p, &mesh->BBox[5].p);      //Up
			t3dPlaneNormal(&mesh->BBoxNormal[3], &mesh->BBox[6].p, &mesh->BBox[7].p, &mesh->BBox[2].p);      //Down
			t3dPlaneNormal(&mesh->BBoxNormal[4], &mesh->BBox[4].p, &mesh->BBox[6].p, &mesh->BBox[0].p);      //Left
			t3dPlaneNormal(&mesh->BBoxNormal[5], &mesh->BBox[5].p, &mesh->BBox[1].p, &mesh->BBox[7].p);      //Right
			return ;
		}
	}
}

/* -----------------08/06/00 14.51-------------------
 *              t3dSetSpecialAnimFrame
 * --------------------------------------------------*/
bool t3dSetSpecialAnimFrame(WGame &game, const char *name, t3dMESH *mesh, int32 nf) {
	if (!name || !mesh)
		return false;

	if (t3dLoadAnimation(game, name, mesh, T3D_MESH_DEFAULTANIM) <= 0)
		return false;

	mesh->Flags |= (T3D_MESH_ABS_ANIM | T3D_MESH_DEFAULTANIM);
	FixupAnim(mesh, 0, "");

	if (nf < 0) nf = mesh->DefaultAnim.NumFrames - 1;

	mesh->CurFrame = nf;
	mesh->LastFrame = -1;
	mesh->BlendPercent = 255;
	mesh->LastBlendPercent = 0;
	return true;
}

void MeshModifiers::modifyMesh(WGame &game, t3dMESH *mesh) {
	struct SMeshModifier *mm;
	int16 i;

	if (!mesh || (mesh->Flags & T3D_MESH_CHARACTER))
		return;

	// Check if there is a modifier for this mesh
	mm = &MMList[0];
	for (i = 0; i < MAX_MODIFIED_MESH; i++, mm++)
		if ((!mm->meshName.empty()) && (mm->meshName.equalsIgnoreCase(mesh->name)))
			break;

	// If there are no modifiers for this mesh or they refer to a body
	if ((i >= MAX_MODIFIED_MESH) || (mm->getFlags() & (MM_SET_BND_LEVEL | MM_SET_HALOES)))
		return;

	mm->modifyMesh(game, mesh);
}

void SMeshModifier::modifyMesh(WGame &game, t3dMESH *mesh) {
	warning("MM %s: addflags %X, removeflags %X, anim |%s|", mesh->name.c_str(), this->AddFlags, this->RemoveFlags, this->animName.c_str());
	// Update Flags
	if (this->Flags & MM_REMOVE_FLAGS)
		mesh->Flags &= ~this->RemoveFlags;
	if (this->Flags & MM_ADD_FLAGS)
		mesh->Flags |= this->AddFlags;

	// Update Materials
	if (this->Flags & MM_REMOVE_MAT_FLAGS)
		mesh->FList[0].getMaterial()->Flags &= ~this->RemoveMatFlags;
	if (this->Flags & MM_ADD_MAT_FLAGS)
		mesh->FList[0].getMaterial()->Flags |= this->AddMatFlags;
	if (this->Flags & MM_SET_MAT_FRAME)
		mesh->setMovieFrame(this->MatFrame); // This did NOT check for existing face/material before setting before.

	// Update Anim
	if ((this->Flags & MM_ANIM_BLOCK) && (!this->animName.empty()) && (!mesh->CurFrame)) {
		t3dSetSpecialAnimFrame(game, this->animName.c_str(), mesh, -1);
		t3dCalcMeshBones(mesh, 1);
		UpdateBoundingBox(mesh);
	}
}

/* -----------------15/09/98 12.04-------------------
 *                  AddMeshModifier
 * --------------------------------------------------*/
void MeshModifiers::addMeshModifier(const Common::String &name, int16 com, void *p) {
	struct SMeshModifier *mm;
	int16 i;

	warning("Not sure this is right"); // Used to check for nullptr, not 0 length.
	if (name.empty() || !p)
		return;

	//	DebugLogFile("AddMM |%s| %d",name,com);

	// Check if a modifier already exists for this mesh
	mm = &MMList[0];
	for (i = 0; i < MAX_MODIFIED_MESH; i++, mm++)
		if ((!mm->meshName.empty()) && mm->meshName.equalsIgnoreCase(name))
			break;

	// If it's a new modifier look for a free place
	if (i >= MAX_MODIFIED_MESH) {
		mm = &MMList[0];
		for (i = 0; i < MAX_MODIFIED_MESH; i++, mm++)
			if (mm->meshName.empty())
				break;
		if (i >= MAX_MODIFIED_MESH) {
			warning("Troppi Mesh modifier per %s: MAX %d", name.c_str(), MAX_MODIFIED_MESH);
			return;
		}

		*mm = SMeshModifier(name.c_str(), com, p);
	} else {
		mm->configure(name.c_str(), com, p);
	}
}

SMeshModifier::SMeshModifier(Common::SeekableReadStream &stream) {
	char stringBuffer[T3D_NAMELEN] = {};
	stream.read(stringBuffer, T3D_NAMELEN);
	meshName = stringBuffer;
	Flags = stream.readSint32LE();
	AddFlags = stream.readUint32LE();
	RemoveFlags = stream.readUint32LE();
	AddMatFlags = stream.readUint32LE();
	RemoveMatFlags = stream.readUint32LE();
	MatFrame = stream.readSint32LE();
	BndLevel = stream.readUint16LE();
	HaloesStatus = stream.readByte(); // TODO: Signed.
	stream.read(stringBuffer, T3D_NAMELEN);
	animName = stringBuffer;
}

SMeshModifier::SMeshModifier(const char *name, int16 com, void *p) {
	configure(name, com, p);
}

void SMeshModifier::configure(const char *name, int16 com, void *p) {
	this->Flags |= com;
	switch (com) {
	case MM_ADD_FLAGS:
		Flags = *((uint32 *)p);
		this->RemoveFlags &= ~Flags;
		this->AddFlags |= Flags;
		break;

	case MM_REMOVE_FLAGS:
		Flags = *((uint32 *)p);
		this->AddFlags &= ~Flags;
		this->RemoveFlags |= Flags;
		break;

	case MM_ADD_MAT_FLAGS:
		Flags = *((uint32 *)p);
		this->RemoveMatFlags &= ~Flags;
		this->AddMatFlags |= Flags;
		break;

	case MM_REMOVE_MAT_FLAGS:
		Flags = *((uint32 *)p);
		this->AddMatFlags &= ~Flags;
		this->RemoveMatFlags |= Flags;
		break;

	case MM_SET_MAT_FRAME:
		this->MatFrame = *((int32 *)p);
		break;

	case MM_ANIM_BLOCK:
		if (this->animName.empty())
			this->animName = (char *)p;
		else
			this->animName.clear();
		break;

	case MM_SET_BND_LEVEL:
		this->BndLevel = *((uint16 *)p);
		break;

	case MM_SET_HALOES:
		this->HaloesStatus = *((int8 *)p);
		break;
	}
}

void MeshModifiers::applyAllMeshModifiers(WGame &game, t3dBODY *b) {
	// Check if there is a modifier for this body
	struct SMeshModifier *mm = &MMList[0];
	for (int32 j = 0; j < MAX_MODIFIED_MESH; j++, mm++)
		if ((!mm->meshName.empty()) && b->name.equalsIgnoreCase(mm->meshName)) {
			if (mm->getFlags() & MM_SET_BND_LEVEL)
				b->CurLevel = mm->getBndLevel();

			if (mm->getFlags() & MM_SET_HALOES) {
				for (auto &l : b->LightTable) {
					if (!(l.Type & T3D_LIGHT_FLARE)) continue;

					if (mm->getHaloesStatus() > 0)
						l.Type |= T3D_LIGHT_LIGHTON;
					else
						l.Type &= ~T3D_LIGHT_LIGHTON;
				}
			}
		}

	for (int32 i = 0; i < (int32)b->NumMeshes(); i++) {
		modifyMesh(game, &b->MeshTable[i]);
	}
}

/* -----------------29/03/99 14.33-------------------
 *                  HideRoomMeshes
 * --------------------------------------------------*/
void HideRoomMeshes(Init &init, t3dBODY *body) {
	int32 cr, c, a, b, i, j, k, h, skip;
	char *str;
	t3dMESH *m;

	if (!(cr = getRoomFromStr(init, body->name))) return;
//	DebugFile("Hiding Room %s (%d)",body->Name,cr);
	for (a = 0; a < MAX_OBJS_IN_ROOM; a++) {
		if (!(c = init.Room[cr].objects[a])) continue;
		if (init.Obj[c].flags & NOUPDATE) continue;
		if (!(init.Obj[c].flags & ON) || (init.Obj[c].flags & HIDE)) {
			for (b = 0; b < MAX_OBJ_MESHLINKS; b++) {
				if (init.Obj[c].meshlink[b][0] == '\0') continue;
				m = nullptr;
				str = (char *)init.Obj[c].meshlink[b];
				for (h = 0; h < (uint16)body->NumMeshes(); h++) {
					if (body->MeshTable[h].name.equalsIgnoreCase(str)) {
						m = &body->MeshTable[h];
						break;
					}
				}
				if (m == nullptr) continue;
//				DebugFile("CandidateObj '%s'",Obj[c].meshlink[b]);

				skip = 0;
				for (i = 0; i < MAX_OBJS_IN_ROOM; i++) {
					if (!(k = init.Room[cr].objects[i]) || (k == c)) continue;
					if (!(init.Obj[k].flags & ON) || (init.Obj[k].flags & HIDE)) continue;

					for (j = 0; j < MAX_OBJ_MESHLINKS; j++) {
						if (init.Obj[k].meshlink[j][0] == '\0') continue;
						if (!Common::String((char *)init.Obj[c].meshlink[b]).equalsIgnoreCase((char *)init.Obj[k].meshlink[j])) continue;

//						DebugFile("Skipped for %d,%d",k,j);
						skip ++;
						break;
					}
				}

				if (!skip && m) {
//					DebugFile("Hiding Mesh %s",Obj[c].meshlink[b]);
					m->Flags |= T3D_MESH_HIDDEN;
				}
			}
		}
	}
}

/* -----------------28/12/98 17.43-------------------
 *                  UpdateCharHead
 * --------------------------------------------------*/
void UpdateCharHead(int32 oc, t3dV3F *dir) {
	t3dCHARACTER *Ch = Character[oc];
	t3dMESH *mesh;
	t3dHEADMOVE *t;
	t3dBONE *bone;
	t3dV3F tmp;
	int32 i, cf;
	t3dF32 s;

	if (!Ch || !(mesh = Ch->Mesh) || mHide || !dir) return;
	if ((bDialogActive) || (bT2DActive) || (InvStatus)) return;

	if (oc == ocCURPLAYER) oc = ocDARRELL + CurPlayer;
	t = &HeadMove[oc];

	if ((Player->Mesh->CurFrame > ActionStart[aSTAND]) || !(Player->Mesh->Flags & T3D_MESH_DEFAULTANIM))
		t3dVectFill(&t->DestAng, 0.0f);
	else if (t->OldPos != *dir) {
		t3dVectCopy(&tmp, &mesh->Trasl);
		tmp.y = CurFloorY + EYES_HEIGHT;
		t3dVectSub(&tmp, dir, &tmp);
		t->DestAng.x = t3dVectAngle(&tmp, &Ch->Dir);
		if (t->DestAng.x < -MAX_HEAD_ANGLE_X * 2) t->DestAng.x = -MAX_HEAD_ANGLE_X * 2;
		if (t->DestAng.x >  MAX_HEAD_ANGLE_X * 2) t->DestAng.x =  MAX_HEAD_ANGLE_X * 2;
		t->DestAng.y = -(t3dF32)asin((mPos.y - (CurFloorY + EYES_HEIGHT)) / t3dVectMod(&tmp)) * 180.0f / T3D_PI;
		if (t->DestAng.y < -MAX_HEAD_ANGLE_Y / 2) t->DestAng.y = -MAX_HEAD_ANGLE_Y / 2;
		if (t->DestAng.y >  MAX_HEAD_ANGLE_Y / 2) t->DestAng.y =  MAX_HEAD_ANGLE_Y / 2;
		t->DestAng.z = 0.0f;
		t3dVectCopy(&t->OldPos, dir);
	}

	if (t->DestAng != t->CurAng) {
		Player->Mesh->LastFrame = 0;
		if (Player->Mesh->Flags & T3D_MESH_DEFAULTANIM) {
			if (Player->Mesh->CurFrame < ActionStart[aSTAND])
				Player->Mesh->CurFrame = ActionStart[aSTAND];
			cf = 1;
			for (i = 1; i < Player->Mesh->DefaultAnim.NumBones; i++) {
				if (!(bone = &Player->Mesh->DefaultAnim.BoneTable[i]) || !(bone->Trasl) || !(bone->Matrix))
					continue;

				t3dVectCopy(&bone->Trasl[cf], &bone->Trasl[Player->Mesh->CurFrame]);
				t3dMatCopy(&bone->Matrix[cf], &bone->Matrix[Player->Mesh->CurFrame]);
			}
			bone = &Player->Mesh->DefaultAnim.BoneTable[12];
		} else {
			cf = Player->Mesh->CurFrame;
			bone = &Player->Mesh->Anim.BoneTable[12];
		}

		if ((bone) && (bone->Trasl) && (bone->Matrix)) {
			t3dVectSub(&tmp, &t->DestAng, &t->CurAng);
			s = t3dVectMod(&tmp);
			if (s > MAX_HEAD_SPEED) {
				tmp.x *= (MAX_HEAD_SPEED / s);
				tmp.y *= (MAX_HEAD_SPEED / s);
			}
			t->CurAng.x += tmp.x;
			t->CurAng.y += tmp.y;
			t->CurAng.z = 0.0f;

			t3dMatRot(&bone->Matrix[cf], (t->CurAng.y * T3D_PI) / 180.0f, (t->CurAng.x * T3D_PI) / 180.0f, 0.0f);
			bone->Matrix[cf].Flags &= ~T3D_MATRIX_IDENTITY;

//			Player->Mesh->BlendPercent = 0;
			Player->Mesh->CurFrame = cf;
		}
	}

//	se siamo in un dialogo o RTV non gli faccio muovere la testolina
	if (bDialogActive) {
		t3dVectFill(&t->CurAng, 0.0f);
		t3dVectFill(&t->DestAng, 0.0f);
		t3dVectFill(&t->OldPos, 0.0f);
	}
}

/* -----------------22/06/00 12.15-------------------
 *                  ChangeMeshFlags
 * --------------------------------------------------*/
void ChangeMeshFlags(t3dMESH *m, int8 add, uint32 newflags) {
	if (!m) return;

	if (add > 0) {
		m->Flags |= newflags;
		_vm->addMeshModifier(m->name, MM_ADD_FLAGS, &newflags);
	} else {
		m->Flags &= ~newflags;
		_vm->addMeshModifier(m->name, MM_REMOVE_FLAGS, &newflags);
	}

}

/* -----------------29/06/00 11.22-------------------
 *              ChangeHaloesStatus
 * --------------------------------------------------*/
void ChangeHaloesStatus(t3dBODY *b, int8 op) {
	if (b == nullptr) b = t3dCurRoom;
	if (b == nullptr) return;

	for (auto &l : b->LightTable) {
		if (!(l.Type & T3D_LIGHT_FLARE)) continue;

		if (op > 0)
			l.Type |= T3D_LIGHT_LIGHTON;
		else
			l.Type &= ~T3D_LIGHT_LIGHTON;
	}
	_vm->addMeshModifier(b->name.c_str(), MM_SET_HALOES, &op);
}

/* -----------------01/06/00 11.12-------------------
 *                  UpdateObjMesh
 * --------------------------------------------------*/
void UpdateObjMesh(Init &init, int32 in) {
	t3dMESH *m;
	int32 a;

	if (init.Obj[in].flags & NOUPDATE) return;

	for (a = 0; a < MAX_OBJ_MESHLINKS; a++) {
		m = LinkMeshToStr(init, (char *)init.Obj[in].meshlink[a]);
		if (m) {
			if ((init.Obj[in].flags & ON) && !(init.Obj[in].flags & HIDE))
//				m->Flags &= ~T3D_MESH_HIDDEN;
				ChangeMeshFlags(m, -1, T3D_MESH_HIDDEN);
			else
//				m->Flags |= T3D_MESH_HIDDEN;
				ChangeMeshFlags(m, +1, T3D_MESH_HIDDEN);
		} else {
			//se non la trova in memoria aggiunge solo il modifier alla lista
			uint32 newflags;

			newflags = T3D_MESH_HIDDEN;
			if (init.Obj[in].meshlink[a][0] != '\0') {
				if ((init.Obj[in].flags & ON) && !(init.Obj[in].flags & HIDE))
					_vm->addMeshModifier((char *)init.Obj[in].meshlink[a], MM_REMOVE_FLAGS, &newflags);
				else
					_vm->addMeshModifier((char *)init.Obj[in].meshlink[a], MM_ADD_FLAGS, &newflags);
			}
		}
	}//for
}


/* -----------------22/06/00 12.15-------------------
 *              SetMeshMaterialMovieFrame
 * --------------------------------------------------*/
void SetMeshMaterialMovieFrame(t3dMESH *m, int8 op, int32 newframe) {
	if (!m || m->FList.empty() || !m->FList[0].getMaterial()) return;

	if (op == 0)
		m->setMovieFrame(newframe);
	else if (op > 0)
		m->setMovieFrame(m->getMovieFrame() + newframe);
	else if (op < 0)
		m->setMovieFrame(m->getMovieFrame() - newframe);

	newframe = m->getMovieFrame();

	_vm->addMeshModifier(m->name, MM_SET_MAT_FRAME, &newframe);
}

/* -----------------22/06/00 12.15-------------------
 *              ChangeMeshMaterialFlags
 * --------------------------------------------------*/
void ChangeMeshMaterialFlag(t3dMESH *m, int8 add, uint32 newflag) {
	if (!m || m->hasFaceMaterial()) return;

	if (add > 0) {
		m->FList[0].getMaterial()->addProperty(newflag);
		_vm->addMeshModifier(m->name, MM_ADD_MAT_FLAGS, &newflag);
	} else {
		m->FList[0].getMaterial()->clearFlag(newflag);
		_vm->addMeshModifier(m->name, MM_REMOVE_MAT_FLAGS, &newflag);
	}
}

/* -----------------20/04/99 15.29-------------------
 *                  t3dProcessGolfSky
 * --------------------------------------------------*/
void t3dProcessGolfSky(t3dMESH *gs) {
	uint32  i;
	t3dF32 GolfSkySpeed = 0.00009f;
	gVertex *gv;

	if (!gs) return ;

	gv = gs->VertexBuffer;
	for (i = 0; i < gs->NumVerts; i++, gv++) {
		gv->u1 += GolfSkySpeed;
		gv->v1 += GolfSkySpeed;
	}
	gs->Flags |= T3D_MESH_UPDATEVB;
	gs->VBptr = NULL;
}

} // End of namespace Watchmaker
