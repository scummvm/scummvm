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

#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/t3d.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/types.h"
#include "common/stream.h"
#include "watchmaker/utils.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/work_dirs.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/light.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/game.h"
#include "watchmaker/renderer.h"
#include "watchmaker/work_dirs.h"

namespace Watchmaker {

#define T3DFILEVERSION  11

// TODO: Globals
#define MAX_LOADED_FILES    100
RecStruct LoadedFiles[MAX_LOADED_FILES];
uint16  NumLoadedFiles = 0;

t3dV3F  CharCorrection;
t3dF32  CurFloorY;
int32  t3dCurTime = 900, t3dCurOliSet = 0;

// LoadRoom vars
#define MAX_T3D_LOADLIST_ITEMS  50
struct _t3dLOADLIST {
	Common::String pname = {};
	uint32 LoaderFlags = 0;
	t3dMESH *m = nullptr;
} t3dLoadList[MAX_T3D_LOADLIST_ITEMS] = {};

/* -----------------10/06/99 16.03-------------------
 *                  AddToLoadList
 * --------------------------------------------------*/
void AddToLoadList(t3dMESH *m, const Common::String &pname, uint32 LoaderFlags) {
	int32 a;

	if (!pname.empty()) {
		for (a = 0; a < MAX_T3D_LOADLIST_ITEMS; a++) {
			if (t3dLoadList[a].pname.empty()) {
				t3dLoadList[a].LoaderFlags = LoaderFlags;
				t3dLoadList[a].m = m;
				t3dLoadList[a].pname = pname;
				break;
			}
		}

		if (a >= MAX_T3D_LOADLIST_ITEMS)
			warning("Cannot add %s to LoadList", pname.c_str());
	} else {
		warning("Invalid parameters invoking AddToLoadList()");
		warning("Mesh (%s), pname %s", m->name.c_str(), pname.c_str());
	}
}

/* -----------------10/06/99 16.04-------------------
 *                  GetFromLoadList
 * --------------------------------------------------*/
struct _t3dLOADLIST *GetFromLoadList(void) {
	int32 a;

	for (a = 0; a < MAX_T3D_LOADLIST_ITEMS; a++) {
		if (!t3dLoadList[a].pname.empty())
			return &t3dLoadList[a];
	}

	return nullptr;
}

t3dPathCamera::t3dPathCamera(Common::SeekableReadStream &stream) {
	NumCamera = stream.readByte();
	PathIndex = stream.readByte();
	Direction = stream.readByte();
}

t3dCAMERA::t3dCAMERA(Common::SeekableReadStream &stream) {
	Index = stream.readByte();

	Source = t3dV3F(stream) * SCALEFACTOR;
	Target = t3dV3F(stream) * SCALEFACTOR;

	t3dVectCopy(&MaxTarget, &Target);

//		Camera[camera].Fov=t3dReadReal();
	Fov = RADIANS_TO_DEGREE(stream.readFloatLE());  //FOV
	NearClipPlane = stream.readFloatLE() * SCALEFACTOR;
	FarClipPlane = stream.readFloatLE() * SCALEFACTOR;

	int numPaths = stream.readByte();
	CameraPaths.reserve(numPaths);
	for (int i = 0; i < numPaths; i++) {
		CameraPaths.push_back(t3dPathCamera(stream));
	}
}

t3dCAMERAPATH::t3dCAMERAPATH(Common::SeekableReadStream &stream) {
	int numPoints = stream.readSint16LE();
	CarrelloDist = stream.readSint32LE();

	PList.resize(numPoints);
	for (int j = 0; j < numPoints; j++) {
		PList[j].x = stream.readFloatLE() * SCALEFACTOR;
		PList[j].y = stream.readFloatLE() * SCALEFACTOR;
		PList[j].z = stream.readFloatLE() * SCALEFACTOR;
	}
}
void decodeLoaderFlags(uint32 flags) {
	warning("%d: T3D_GENERATESHADOWMAPS", flags & T3D_GENERATESHADOWMAPS);
	warning("%d: T3D_NOLIGHTMAPS", flags & T3D_NOLIGHTMAPS);
	warning("%d: T3D_NORECURSION", flags & T3D_NORECURSION);
	warning("%d: T3D_HALFTEXTURESIZE", flags & T3D_HALFTEXTURESIZE);
	warning("%d: T3D_FULLSCREEN", flags & T3D_FULLSCREEN);
	warning("%d: T3D_FASTRENDERING", flags & T3D_FASTRENDERING);
	warning("%d: T3D_OUTDOORLIGHTS", flags & T3D_OUTDOORLIGHTS);
	warning("%d: T3D_NOVOLUMETRICLIGHTS", flags & T3D_NOVOLUMETRICLIGHTS);
	warning("%d: T3D_NOBOUNDS", flags & T3D_NOBOUNDS);
	warning("%d: T3D_NOCAMERAS", flags & T3D_NOCAMERAS);
	warning("%d: T3D_NONEXCLUSIVEMOUSE", flags & T3D_NONEXCLUSIVEMOUSE);
	warning("%d: T3D_RECURSIONLEVEL1", flags & T3D_RECURSIONLEVEL1);
	warning("%d: T3D_SKY", flags & T3D_SKY);
	warning("%d: T3D_PRELOAD_RXT", flags & T3D_PRELOAD_RXT);
	warning("%d: T3D_STATIC_SET0", flags & T3D_STATIC_SET0);
	warning("%d: T3D_STATIC_SET1", flags & T3D_STATIC_SET1);
	warning("%d: T3D_NOSHADOWS", flags & T3D_NOSHADOWS);
	warning("%d: T3D_NOICONS", flags & T3D_NOICONS);
	warning("%d: T3D_NOSOUND", flags & T3D_NOSOUND);
	warning("%d: T3D_PRELOADBASE", flags & T3D_PRELOADBASE);
	warning("%d: T3D_NOMUSIC", flags & T3D_NOMUSIC);
	warning("%d: T3D_DEBUGMODE", flags & T3D_DEBUGMODE);
	warning("%d: T3D_FASTFILE", flags & T3D_FASTFILE);
	warning("%d: T3D_HIPOLYPLAYERS", flags & T3D_HIPOLYPLAYERS);
	warning("%d: T3D_HIPOLYCHARACTERS", flags & T3D_HIPOLYCHARACTERS);
}

/* -----------------10/06/99 16.04-------------------
 *                  CheckIfAlreadyLoaded
 * --------------------------------------------------*/
t3dBODY *CheckIfAlreadyLoaded(const Common::String &Name) {
	if (Name.empty()) return nullptr;

	for (uint16 i = 0; i < NumLoadedFiles; i++) {
		if ((LoadedFiles[i].b != nullptr) && /*(LoadedFiles[i].Name != nullptr) &&*/ (!LoadedFiles[i].name.empty()))
			if (LoadedFiles[i].name.equalsIgnoreCase(Name))
				return LoadedFiles[i].b;
	}

	return nullptr;
}

Common::String constructPath(const Common::String &prefix, const Common::String &filename, const char *suffix) {
	Common::String Name = prefix + filename;
	uint16 len = Name.size();
	if (suffix != nullptr) {
		uint16 suffixLen = strlen(suffix);
		Name = Name.substr(0, len - suffixLen) + suffix;
		assert(suffixLen == 3);
	}
	return Common::String(Name);
}

/* -----------------10/06/99 16.04-------------------
 *                  t3dLoadSingleRoom
 * --------------------------------------------------*/
t3dBODY *t3dLoadSingleRoom(WGame &game, const Common::String &_pname, t3dBODY *b, uint16 *NumBody, uint32 LoaderFlags) {
	//warning("t3dLoadSingleRoom(workDirs, %s, b, %d, %d)", _pname, *NumBody, LoaderFlags);
	//decodeLoaderFlags(LoaderFlags);
	Common::String pname(_pname);

	WorkDirs &workdirs = game.workDirs;

	if (pname.equalsIgnoreCase("r1c.t3d"))
		if (((t3dCurTime >= 1300) && (t3dCurTime <= 1310)) || (t3dCurTime >= 1800))          //se viene cambiato l'orario cambiarlo anche in UpdateRoomVis...
			pname = "r1c-notte.t3d";
	if (pname.equalsIgnoreCase("r15.t3d"))
		if (((t3dCurTime >= 1300) && (t3dCurTime <= 1310)) || (t3dCurTime >= 1800))
			pname = "r15-notte.t3d";

	auto name = constructPath(workdirs._t3dDir, pname);

	//warning("t3dLoadSingleRoom opening(%s)", name.c_str());
	auto stream = openFile(name);
	if (!(stream)) { // Apre file
		warning("t3dLoadSingleRoom: Failed to open(%s)", name.c_str());
		return nullptr;
	}

	if (*NumBody == 0) {                                                                         // Se e' il primo body, alloca
		b = new t3dBODY;
		(*NumBody)++;
	} else
		b = (t3dBODY *)t3dRealloc((uint32 *)b, sizeof(t3dBODY) * (++(*NumBody)));                         // Altrimenti, ridimensiona

	//warning("Loading %s ...", name.c_str());
	*b = t3dBODY();	// Azzera Body

	uint16 fileVersion = stream->readByte();
	if (fileVersion != T3DFILEVERSION) {                                                   // Controlla la versione del file
		warning("%s file incompatible: current version: %d.\tFile version: %d", name.c_str(), T3DFILEVERSION, fileVersion);
		return nullptr;
	}

	{
		uint16 j = 1;
		while (LoadedFiles[j].b != nullptr) j++;
		if (j > MAX_LOADED_FILES) {
			warning("Too many t3d files loaded!");
			return nullptr;
		}
		if ((j + 1) > NumLoadedFiles) NumLoadedFiles = j + 1;

		LoadedFiles[j].name = _pname;                              // Aggiunge il file alla lista
		LoadedFiles[j].Flags = LoaderFlags;                        // Aggiunge Flags alla lista
		LoadedFiles[j].b = b;                                      // Aggiunge Body alla lista
		j = 0;
	}
	b->loadFromStream(game, pname, *stream, LoaderFlags);

	return b;
}

/* -----------------10/06/99 16.04-------------------
 *                  t3dLoadRoom
 * --------------------------------------------------*/
t3dBODY *t3dLoadRoom(WGame &game, const Common::String &pname, t3dBODY *b, uint16 *NumBody, uint32 LoaderFlags) {
	warning("t3dLoadRoom(%s, b, %d, %d)", pname.c_str(), *NumBody, LoaderFlags);
	struct _t3dLOADLIST *l;
	t3dBODY *r, *rez;
	t3dBODY *body;
	uint16 num, i;

//	azzera tutto quello che c'era prima nella load list
	for (int i = 0; i < MAX_T3D_LOADLIST_ITEMS; i++) {
		t3dLoadList[i] = _t3dLOADLIST();
	}

//	Aggiunge la stanza base alla lista di caricamenti
	AddToLoadList(nullptr, pname, LoaderFlags);

	while ((l = GetFromLoadList())) {
		num = 0;
		if (l->m) {
			if ((rez = CheckIfAlreadyLoaded(l->pname)))
				body = l->m->PortalList = rez;
			else {
//				if (l->m->Flags&T3D_MESH_PREPROCESSPORTAL)
//					body=l->m->PortalList = t3dLoadSingleRoom( l->pname, l->m->PortalList, &num, (l->LoaderFlags|T3D_HALFTEXTURESIZE) );
//				else
				body = l->m->PortalList = t3dLoadSingleRoom(game, l->pname, l->m->PortalList, &num, l->LoaderFlags);
			}
		} else
			body = r = t3dLoadSingleRoom(game, l->pname, b, NumBody, l->LoaderFlags);

		memset(l, 0, sizeof(struct _t3dLOADLIST));
	}

	if (!(LoaderFlags & T3D_NORECURSION)) {
		for (i = 0; i < NumLoadedFiles; i++)
			if (LoadedFiles[i].b)
				t3dCalcRejectedMeshFromPortal(LoadedFiles[i].b);
	}

	warning("Room loaded");
	return r;
}

t3dParticle::t3dParticle(Common::SeekableReadStream &stream) {
	t3dF32  difR1, difG1, difB1;
	t3dF32  difR2, difG2, difB2;
	Num = (uint32)stream.readFloatLE();
	Lung = stream.readFloatLE();
	Size = stream.readFloatLE();
	Seg1 = stream.readFloatLE();
	Seg2 = stream.readFloatLE();
	Dim1 = stream.readFloatLE() / 1000.0f;
	Dim2 = stream.readFloatLE() / 1000.0f;
	Speed = stream.readFloatLE() / 10.0f;
	Speed1 = stream.readFloatLE() / 10.0f;
	Speed2 = stream.readFloatLE() / 10.0f;
	Caos = stream.readFloatLE() / 10;
	Caos1 = stream.readFloatLE() / 10;
	Caos2 = stream.readFloatLE() / 10;
	Delay = (uint32)stream.readFloatLE();
	OR1 = (uint8)stream.readSint32LE();
	R1 = (uint8)stream.readSint32LE();
	G1 = (uint8)stream.readSint32LE();
	B1 = (uint8)stream.readSint32LE();
	R2 = (uint8)stream.readSint32LE();
	G2 = (uint8)stream.readSint32LE();
	B2 = (uint8)stream.readSint32LE();
	R3 = (uint8)stream.readSint32LE();
	G3 = (uint8)stream.readSint32LE();
	B3 = (uint8)stream.readSint32LE();
	Type = (uint8)stream.readSint32LE();

#ifndef WMGEN
	ParticleIndex = t3dCreateSmokeParticle(Num,
										   Type,
										   OR1);
#endif
	difR1 = (R2 - R1) / (Seg1 / Speed1);
	difG1 = (G2 - G1) / (Seg1 / Speed1);
	difB1 = (B2 - B1) / (Seg1 / Speed1);
	difR2 = (R3 - R2) / (Seg2 / Speed2);
	difG2 = (G3 - G2) / (Seg2 / Speed2);
	difB2 = (B3 - B2) / (Seg2 / Speed2);
	R2 = difR1;
	G2 = difG1;
	B2 = difB1;
	R3 = difR2;
	G3 = difG2;
	B3 = difB2;
}

/* -----------------24/05/00 10.24-------------------
 *                  t3dPrecalcLight
 * --------------------------------------------------*/
void t3dPrecalcLight(t3dBODY *b, uint8 *sun) {
	t3dV3F  tmp, l, *normal;
	t3dVERTEX vv;
	t3dF32  nlight;
	int32  k, aa, rr, gg, bb;
	uint32 i, j, cv;

	for (i = 0, cv = 0; i < b->NumMeshes(); i++) {                                            // Si ripassa tutte le mesh
		t3dMESH &Mesh = b->MeshTable[i];
#ifndef WMGEN
		Mesh.VBptr = Mesh.VertexBuffer;
#endif
		for (j = 0; j < Mesh.NumVerts; j++, cv++) {                                                // Si passa tutti i vertici
			rr = RGBA_GETRED(Mesh.VBptr[j].diffuse);
			gg = RGBA_GETGREEN(Mesh.VBptr[j].diffuse);
			bb = RGBA_GETBLUE(Mesh.VBptr[j].diffuse);
			aa = RGBA_GETALPHA(Mesh.VBptr[j].diffuse);
			tmp.x = Mesh.VBptr[j].x;
			tmp.y = Mesh.VBptr[j].y;
			tmp.z = Mesh.VBptr[j].z;
			for (k = 0; k < (int)b->NumLights(); k++) {                                           // Si passa tutte le luci
				if ((b->LightTable[k].Type & T3D_LIGHT_REALTIME) || (b->LightTable[k].Type & T3D_LIGHT_FLARE) || !(b->LightTable[k].Type & T3D_LIGHT_LIGHTON)) continue;

				t3dVectSub(&l, &b->LightTable[k].Source, &tmp);                                  // Calcola vettore luce->vertice
				t3dVectNormalize(&l);                                                            // lo normalizza

				normal = &Mesh.NList[j]->n;                                                     // Calcola normale
				if ((nlight = t3dVectDot(normal, &l)) >= 0) {
					if (LightVertex(&vv, &tmp, &b->LightTable[k])) {
						if ((sun) && (!sun[cv]) && (b->LightTable[k].Type & T3D_LIGHT_SUN)) {
							rr += t3dFloatToInt(vv.r * nlight * 0.50f);
							gg += t3dFloatToInt(vv.g * nlight * 0.35f);
							bb += t3dFloatToInt(vv.b * nlight * 0.27f);
						} else {
							rr += t3dFloatToInt(vv.r * nlight);
							gg += t3dFloatToInt(vv.g * nlight);
							bb += t3dFloatToInt(vv.b * nlight);
						}
					}
				}
			}
			if (rr < 0) rr = 0;
			if (gg < 0) gg = 0;
			if (bb < 0) bb = 0;
			if (rr > 255) rr = 255;
			if (gg > 255) gg = 255;
			if (bb > 255) bb = 255;

			Mesh.VBptr[j].diffuse = RGBA_MAKE(rr, gg, bb, aa);
		}
#ifndef WMGEN
		Mesh.VBptr = nullptr;
#endif
	}
}


/* -----------------10/06/99 16.06-------------------
 *                      t3dLoadSky
 * --------------------------------------------------*/
void t3dLoadSky(WGame &game, t3dBODY * /*body*/) {
	t3dF32  Skyminx, Skyminy, Skyminz, Skymaxx, Skymaxy, Skymaxz;
	uint16  n = 0, i;
	gVertex *gv;
//	t3dF32   Tile=1.5f;
	t3dF32  div;

	if (!(t3dSky = t3dLoadRoom(game, "sky.t3d", t3dSky, &n, T3D_NORECURSION | T3D_NOLIGHTMAPS | T3D_NOVOLUMETRICLIGHTS | T3D_NOCAMERAS | T3D_NOBOUNDS | T3D_STATIC_SET0))) {
		warning("Error during t3dLoadRoom: Sky not loaded");
	}

	GetBoundaries(t3dSky, &Skyminx, &Skyminy, &Skyminz, &Skymaxx, &Skymaxy, &Skymaxz);

	for (i = 0; i < t3dSky->NumMeshes(); i++) {
		gv = t3dSky->MeshTable[i].VertexBuffer;
		for (n = 0; n < t3dSky->MeshTable[i].NumVerts; n++, gv++) {
			gv->x -= Skyminx + ((Skymaxx - Skyminx) / 2.0f);
			gv->y -= Skyminy + ((Skymaxy - Skyminy) / 2.0f);
			gv->z -= Skyminz + ((Skymaxz - Skyminz) / 2.0f);
			div = (t3dF32)sqrt(gv->x * gv->x + gv->y * gv->y + gv->z * gv->z);
			gv->x /= div;
			gv->y /= div;
			gv->z /= div;

			gv->x *= 15000;
			gv->y *= 500;
			gv->z *= 15000;

			gv->u1 *= 1.0f;
			gv->v1 *= 1.0f;
		}

		t3dSky->MeshTable[0].Radius = 15000.0f * 2;
	}

	for (n = 0; n < t3dSky->NumNormals; n++) {
		t3dSky->NList[n]->dist *= 15000.0f;
	}

	for (n = 0; n < t3dSky->NumMaterials(); n++) {
		t3dSky->MatTable[n]->addProperty(T3D_MATERIAL_SKY);
		t3dSky->MatTable[n]->addProperty(T3D_MATERIAL_NOLIGHTMAP);
	}
}

/* -----------------02/08/99 15.40-------------------
 *                  t3dAddVertexBuffer
 * --------------------------------------------------*/
Common::SharedPtr<VertexBuffer> t3dAddVertexBuffer(t3dBODY *b, uint32 numv) {
	return b->addVertexBuffer();
}

/* -----------------10/06/99 16.03-------------------
 *              t3dOptimizeMaterialList
 * --------------------------------------------------*/
void t3dOptimizeMaterialList(t3dBODY *b) {
	for (int i = 0; i < b->NumMaterials(); i++) {                                              // Scorre tutti materilai di un body
		MaterialPtr Mat = b->MatTable[i];
		if ((Mat == nullptr) || /*(!Mat->Texture->Name) ||*/ (Mat->Movie) || (Mat->hasFlag(T3D_MATERIAL_MOVIE))) // Se non esiste o non ha texture
			continue;                                                                           // esce

		for (int j = 0; j < b->NumMaterials(); j++) {                                       // Cerca materiali uguali
			MaterialPtr CurMat = b->MatTable[j];
			if (Mat == CurMat)
				continue;

			if (CurMat == nullptr /*|| (!CurMat->Texture->Name)*/)
				continue;

			if (Mat->Texture->name.equalsIgnoreCase(CurMat->Texture->name)) {                      // Se ha lo setsso nome di texture
				//warning("TODO: Implement Material-merging");
#if 0
				// This is currently broken.
				Mat = rMergeMaterial(Mat, CurMat);                                          // Unisce i due materiali

				t3dMESH *m = b->MeshTable;
				for (int k = 0; k < b->NumMeshes; k++, m++) {                                       // Aggiorna in tutte le mesh id materiale
					t3dFACE *f = m->FList;
					for (int q = 0; q < m->NumFaces; q++, f++) {
						if (f->mat == CurMat)
							f->mat = Mat;
					}
				}
#endif
			}
		}
	}
}

/* -----------------30/07/99 10.55-------------------
 *              t3dFinalizeMaterialList
 * --------------------------------------------------*/
void t3dFinalizeMaterialList(t3dBODY *b) {
	for (uint32 i = 0; i < b->NumMeshes(); i++) {
		t3dMESH &Mesh = b->MeshTable[i];
#ifndef WMGEN
		Mesh.VBptr = Mesh.VertexBuffer;
#endif
		for (uint32 j = 0; j < Mesh.NumFaces(); j++) {
			t3dFACE &Face = Mesh.FList[j];
			MaterialPtr Mat = Face.getMaterial();
			if (Face.lightmap) {
				Mat = nullptr;
				for (auto material : Face.getMaterial()->AddictionalMaterial) {
					if (Mat->Texture->ID == Face.lightmap->Texture->ID) {
						Mat = material;
						break;
					}
				}
				if (Mat == nullptr) {
					warning("%s: Can't find Lightmap Sub-Material!", Mesh.name.c_str());
					warning("%d %d", Face.getMaterial()->NumAddictionalMaterial, Face.lightmap->Texture->ID);
					for (auto material : Face.getMaterial()->AddictionalMaterial) {
						warning("%d", material->Texture->ID);
					}
					continue;
				}
			}

			uint32 k = 0;
			for (k = 0; k < (uint32)Mat->NumAllocatedMesh; k++)
				if (Mat->FlagsList[k] == &Mesh.Flags)
					break;

			if (k >= (uint32)Mat->NumAllocatedMesh) {
				Mat->FlagsList.push_back(&Mesh.Flags);
				Mat->NumAllocatedMesh++;
				Mesh.Flags |= T3D_MESH_UPDATEVB;
			}
			Mesh.Flags |= T3D_MESH_UPDATEVB;

			for (uint32 h = 0; h < 3; h++) {
				for (k = 0; k < (uint32)Mat->NumAllocatedVerts(); k++)
					if (Mat->VertsList[k] == &Mesh.VBptr[Face.VertexIndex[h]])
						break;

				if (k >= (uint32)Mat->NumAllocatedVerts()) {
					Mat->VertsList.push_back(&Mesh.VBptr[Face.VertexIndex[h]]);
				}
				assert(k < Mat->VertsList.size());
				Face.MatVertexIndex[h] = (int16)k;
			}

		}
#ifndef WMGEN
		Mesh.VBptr = nullptr;
#endif
	}

	warning("Partially stubbed t3dFinalizeMaterialList");

	for (int i = 0; i < b->NumMaterials(); i++) {
		auto &Mat = b->MatTable[i];
		Mat->VBO = b->addVertexBuffer(); // t3dAddVertexBuffer(b, Mat->NumAllocatedVerts);
		for (int j = 0; j < (uint32)Mat->NumAddictionalMaterial; j++)
			Mat->AddictionalMaterial[j]->VBO = t3dAddVertexBuffer(b, Mat->AddictionalMaterial[j]->NumAllocatedVerts());
	}
}

} // End of namespace Watchmaker
