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

#include "watchmaker/3d/light.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/file_utils.h"
#include "watchmaker/game.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/renderer.h"
#include "watchmaker/t3d.h"
#include "watchmaker/types.h"
#include "watchmaker/utils.h"

namespace Watchmaker {

/* -----------------29/05/99 11.41-------------------
 *              Illumina un t3dVERTEX (per WmGen)
 * --------------------------------------------------*/
uint8 LightVertex(t3dVERTEX *vv, t3dV3F *v, t3dLIGHT *light) {
	t3dF32  dist, direction, ang, deg, Intensity1 = 1.0f, Intensity2 = 1.0f;
	t3dV3F  vdist, dir, finallight;
	t3dF32  half_hotspot = DEGREE_TO_RADIANS(light->HotSpot) * 0.5f,
	        half_falloff = DEGREE_TO_RADIANS(light->FallOff) * 0.5f;

	t3dVectSub(&vdist, v, &light->Source);
	dist = t3dVectMod(&vdist);

	t3dVectFill(&finallight, 0.0f);

	if ((light->Type & T3D_LIGHT_SPOTLIGHT)) {
		t3dVectSub(&dir, &light->Target, &light->Source);                  // Serve per Spot
		direction = t3dVectMod(&dir);

		ang = (float)acos(t3dVectDot(&vdist, &dir) / (direction * dist));
		if ((ang > (half_hotspot))) {
			if (ang > (half_falloff)) {
				Intensity2 = 0.0f;
				Intensity1 = 0.0f;
			} else {
				Intensity1 = 1.0;
				deg = half_hotspot + (ang - half_hotspot);
				Intensity2 = (half_falloff - deg) / (half_falloff - half_hotspot);
			}
		} else {
			Intensity1 = 1.0;
			Intensity2 = 1.0;
		}

		if ((light->Type & T3D_LIGHT_ATTENUATION)) {
			if ((dist > light->NearRange)) {
				if (dist > light->FarRange) {
					Intensity1 = 0.0f;
					Intensity2 = 0.0f;
				} else {
					dist = light->NearRange + (dist - light->NearRange);
					Intensity2 *= (light->FarRange - dist) / (light->FarRange - light->NearRange);
					Intensity1 *= 1.0f;
				}
			} else {
				Intensity1 *= 1.0f;
				Intensity2 *= 1.0f;
			}
		}
	} else {
		if ((light->Type & T3D_LIGHT_ATTENUATION)) {
			if ((dist < light->NearRange)) {
				Intensity1 = 1.0f;
				Intensity2 = 1.0f;
			} else {
				Intensity1 = 1.0f;

				if (dist > light->FarRange)
					Intensity2 = 0.0f;
				else {
					dist = light->NearRange + (dist - light->NearRange);
					Intensity2 *= (light->FarRange - dist) / (light->FarRange - light->NearRange);
					Intensity1 *= 1.0f;
				}
			}
		} else {
			Intensity1 = 1.0;
			Intensity2 = 1.0;
		}
	}

	if ((Intensity1 == 0.0f) || (Intensity2 == 0.0f)) {
		return 0;
	}

	finallight = light->Color * (Intensity1 * Intensity2 * light->Multiplier);

	if (finallight.x > 255.0f) finallight.x = 255.0f;
	if (finallight.y > 255.0f) finallight.y = 255.0f;
	if (finallight.z > 255.0f) finallight.z = 255.0f;
	vv->r = (uint8)finallight.x;
	vv->g = (uint8)finallight.y;
	vv->b = (uint8)finallight.z;

	return 1;
}

/* -----------------29/05/99 11.41-------------------
 *              Illumina un gVertex (per game)
 * --------------------------------------------------*/
uint8 LightgVertex(gVertex *v, t3dLIGHT *light) {
	t3dVERTEX vv;
	t3dV3F vt;

	vt.x = v->x;
	vt.y = v->y;
	vt.z = v->z;

	if (LightVertex(&vv, &vt, light))
		return true;
	else
		return false;
}

/* -----------------29/05/99 11.55-------------------
 *              setDirectoryAndName
 * --------------------------------------------------*/
Common::String setDirectoryAndName(const Common::String &path, const Common::String &name) {
	int32  len = name.size();

	auto backSlashPos = name.findFirstOf("\\");

	return path + name.substr(backSlashPos + 1, name.size() - (backSlashPos + 1));
}

/* -----------------29/05/99 12.03-------------------
 *                  LoadVolumetricMap
 * --------------------------------------------------*/
void LoadVolumetricMap(WorkDirs &workDirs, const char *pname, t3dBODY *b) {
	uint32  i, j, k;

	auto stream = workDirs.resolveFile(pname);
	if (!(stream)) {
//t     DebugLogWindow("File %s not found: assuming no volumetriclights informations",pname);
		return ;
	}

	if ((i = stream->readSint32LE()) != VOLLIGHTFILEVERSION) {
		warning("Invalid File version: %s file version is: %d\t You need the version: %d", pname, i, VOLLIGHTFILEVERSION);
		return ;
	}

	b->VolumetricLights = Common::SharedPtr<t3dVolLights>(new t3dVolLights());
	b->VolumetricLights->CellsSize = stream->readFloatLE();
	b->VolumetricLights->xcells = stream->readSint32LE();
	b->VolumetricLights->ycells = stream->readSint32LE();
	b->VolumetricLights->zcells = stream->readSint32LE();

	b->VolumetricLights->VolMap.resize(b->VolumetricLights->ycells * b->VolumetricLights->xcells * b->VolumetricLights->zcells);
	for (i = 0; i < b->VolumetricLights->ycells - 1; i++) {
		for (j = 0; j < b->VolumetricLights->zcells - 1; j++) {
			for (k = 0; k < b->VolumetricLights->xcells - 1; k++) {
				b->VolumetricLights->VolMap[(k) + ((j)*b->VolumetricLights->xcells) + ((i)*b->VolumetricLights->xcells * b->VolumetricLights->zcells)] = stream->readByte();
			}
		}
	}
}

/* -----------------29/05/99 12.04-------------------
 *              t3dLoadOutdoorLights
 * --------------------------------------------------*/
void t3dLoadOutdoorLights(const char *pname, t3dBODY *b, int32 ora) {
	warning("STUBBED, t3dLoadOutdoorLights");
#if 0
	t3dU32      i, j, k;
	t3dMESH     *m;
	gVertex     *gv;
	t3dU32      nverts;
	char        Name[T3D_NAMELEN];
	t3dU32      len;
	DWORD       *Buf, *t;
	t3dLIGHT    *l;

	if (!pname || !b) return;

	i = 0;
	if (ora >= 1130) i++;
	if ((ora >= 1300) && (ora <= 1310)) i++;
	if (ora >= 1800) i++;
	if (ora >= 2030) i++;

	if (i == (t3dU32)t3dCurOliSet) return;

	len = strlen(pname);
	memset(Name, 0, sizeof(Name));
	strncpy(Name, pname, len - 4);
	strcat(Name, ".oli");

	if (!(t3dOpenFile(Name))) {
		DebugLogWindow("Unable to open OLI file %s", Name);
		return ;
	}

	if ((i = t3dRead32()) != OUTDOORLIGHTSFILEVERSION) {
		DebugLogWindow("Invalid File version: %s file version is: %d\t You need the version: %d", pname, i, VOLLIGHTFILEVERSION);
		return ;
	}

	if ((nverts = t3dRead32()) != b->NumTotVerts) {
		DebugLogWindow("Old OLI File %s!", pname);
		t3dCurOliSet = i;
		t3dCreateProceduralSky();
		return ;
	}

	Buf = t = (DWORD *)t3dMalloc(nverts * sizeof(DWORD));

	t3dCurOliSet = 0;
	t3dCurTime = ora;
	if (ora >= 1130) {
		t3dReadData(Buf, nverts * sizeof(DWORD));
		t3dCurOliSet++;
	}
	if ((ora >= 1300) && (ora <= 1310)) {
		t3dReadData(Buf, nverts * sizeof(DWORD));
		t3dCurOliSet++;
	}
	if (ora >= 1800) {
		t3dReadData(Buf, nverts * sizeof(DWORD));
		t3dCurOliSet++;
	}
	if (ora >= 2030) {
		t3dReadData(Buf, nverts * sizeof(DWORD));
		t3dCurOliSet++;
	}

	t3dReadData(Buf, nverts * sizeof(DWORD));
	t3dCloseFile();

	for (i = 0, m = b->MeshTable; i < b->NumMeshes; i++, m++) {
#ifndef WMGEN
		m->VBptr = m->VertexBuffer;
#endif
		for (j = 0, gv = m->VBptr; j < m->NumVerts; j++, gv++, t++) {
			gv->diffuse = *t;
		}
		m->Flags |= T3D_MESH_UPDATEVB;
#ifndef WMGEN
		m->VBptr = nullptr;
#endif
	}
	t3dFree(Buf);

	t3dVectCopy(&b->AmbientLight, &OliAmbient[t3dCurOliSet]);

	l = b->LightTable;
	for (k = 0; k < b->NumLights; k++, l++) {
		if (l->Type & T3D_LIGHT_SOLARVARIATION) {
			if (l->Type & T3D_LIGHT_SUN)
				t3dVectCopy(&l->Source, &l->SolarPos[t3dCurOliSet]);
			t3dVectCopy(&l->Color, &l->SolarColor[t3dCurOliSet]);

			l->Type |= T3D_LIGHT_LIGHTON;
			if ((t3dCurOliSet == 0) && (l->Type & T3D_LIGHT_OFF_MORNING))     l->Type &= ~T3D_LIGHT_LIGHTON;
			if ((t3dCurOliSet == 1) && (l->Type & T3D_LIGHT_OFF_AFTERNOON))   l->Type &= ~T3D_LIGHT_LIGHTON;
			if ((t3dCurOliSet == 2) && (l->Type & T3D_LIGHT_OFF_EVENING))     l->Type &= ~T3D_LIGHT_LIGHTON;
			if ((t3dCurOliSet == 3) && (l->Type & T3D_LIGHT_OFF_NIGHT))       l->Type &= ~T3D_LIGHT_LIGHTON;

			if (l->Color.x > 255.0f) l->Color.x = 255.0f;
			if (l->Color.y > 255.0f) l->Color.y = 255.0f;
			if (l->Color.z > 255.0f) l->Color.z = 255.0f;
			if (l->Color.x < 0.0f) l->Color.x = 0.0f;
			if (l->Color.y < 0.0f) l->Color.y = 0.0f;
			if (l->Color.z < 0.0f) l->Color.z = 0.0f;
		}
	}

	t3dCreateProceduralSky();
#endif
}

/* -----------------29/05/99 12.01-------------------
 *                  GetBoundaries
 * --------------------------------------------------*/
void GetBoundaries(t3dBODY *b, t3dF32 *minx, t3dF32 *miny, t3dF32 *minz, t3dF32 *maxx, t3dF32 *maxy, t3dF32 *maxz) {
	gVertex *gv;

	*minx = *miny = *minz = 999999999.9f;
	*maxx = *maxy = *maxz = -999999999.9f;

	for (uint32 i = 0; i < b->NumMeshes(); i++) {
#ifndef WMGEN
		gv = b->MeshTable[i].VertexBuffer;
#else
		gv = (gVertex *)(m->VBptr);
#endif

		for (uint32 j = 0; j < b->MeshTable[i].NumVerts; j++, gv++) {
			if (gv->x < *minx) *minx = gv->x;
			if (gv->y < *miny) *miny = gv->y;
			if (gv->z < *minz) *minz = gv->z;

			if (gv->x > *maxx) *maxx = gv->x;
			if (gv->y > *maxy) *maxy = gv->y;
			if (gv->z > *maxz) *maxz = gv->z;
		}

	}
}

t3dLIGHT::t3dLIGHT(WGame &game, t3dBODY *b, WorkDirs &workDirs, Common::SeekableReadStream &stream) {
	Type = stream.readUint32LE();                                                        // Legge tipo
	//		DebugFile("%d: SPOT %X ATTEN %X SHAD %X",light,Light[light].Type&T3D_LIGHT_SPOTLIGHT,Light[light].Type&T3D_LIGHT_ATTENUATION,Light[light].Type&T3D_LIGHT_CASTSHADOWS);
	Source = t3dV3F(stream) * SCALEFACTOR;                                    // Legge Source
	Target = t3dV3F(stream) * SCALEFACTOR;                                    // Legge Target

	HotSpot = stream.readFloatLE() * SCALEFACTOR;
	FallOff = stream.readFloatLE() * SCALEFACTOR;

	Color = t3dV3F(stream);                                                   // Legge Color

	NearRange = stream.readFloatLE() * SCALEFACTOR;
	FarRange = stream.readFloatLE() * SCALEFACTOR;

	Multiplier = stream.readFloatLE();
	Flicker = stream.readByte();

	t3dBackfaceCulling(b->NList, (uint16)(b->NumNormals + b->NumVerticesNormals), &Source); // Setta le facce che sono backface
//f     t3dPreLigthVertices(b->NList, b->VList, (t3dU16)b->NumVerts, &Light[light]);            // Illumina facce che non sono backface


	if (Flicker) {                                                             // Aggiorna Vertici visibili per flicker
		Type |= T3D_LIGHT_PULSE;
		setupVisibleVerticesFromLight(b);
		AnimLight.LastRandomizer = 0;
	}

	if (Type & T3D_LIGHT_FLARE) {                                              // Se ha una flare
//f         Light[light].Type&=~T3D_LIGHT_LIGHTON;                                              // La spegne
		FlareSize = stream.readFloatLE() * SCALEFACTOR;                               // Legge il size della flare

		Common::String name = readT3dString(stream); // Legge nome dellaq texture
		Common::String appo;
#ifndef WMGEN
		if (hasFileExtension(name, "avi")) {
			appo = workDirs._moviesDir + name;                                                              // altrimenti prende quello di default
		} else {
			appo = workDirs._mapsDir + name;;                                                            // altrimenti prende quello di default
		}
#else
		strcpy(Appo, WmMapsDir);
			strcat(Appo, Name);
#endif

#ifndef WMGEN
		if (!(game._renderer->addMaterial(Material[0], appo, 15, 0))) {      // Aggiunge il materiale
			warning("File %s not found", appo.c_str());
			Material[0].Texture = nullptr;
			assert(false);
		}
//f         rAddNumFacesMaterial(&Light[light].Material[0], /*f2*/7);                           // Aggiunge 7 facce ???
		Material[0].addProperty(T3D_MATERIAL_FLARE);                // Assegna al materiale la prop flare
		warning("TODO!, Implement the user vertex buffer");
#if 0
		Light[light].Material[0].VB = rGetUserVertexBuffer();
#endif
		warning("TODO: Lights");
		//Material[0].NumAllocatedVerts += 45;
#endif
	}

	Particle = nullptr;
	if (Type & T3D_LIGHT_CANDLESMOKE) {                                        // Se ha una smoke-particle
		Particle = Common::SharedPtr<t3dParticle>(new t3dParticle(stream));              // Legge data
	}
	if (Type & T3D_LIGHT_SOLARVARIATION) {                                     // Se ha una variazione in base al sole
		if (!(Type & T3D_LIGHT_OFF_MORNING)) {
			SolarPos[0] = t3dV3F(stream);
			SolarColor[0] = t3dV3F::fromStreamAsBytes(stream);
		}
		if (!(Type & T3D_LIGHT_OFF_AFTERNOON)) {
			t3dVectCopy(&SolarPos[1], &Source);
			t3dVectCopy(&SolarColor[1], &Color);
		}
		if (!(Type & T3D_LIGHT_OFF_EVENING)) {
			SolarPos[2] = t3dV3F(stream);
			SolarColor[2] = t3dV3F::fromStreamAsBytes(stream);
		}
		if (!(Type & T3D_LIGHT_OFF_NIGHT)) {
			SolarPos[3] = t3dV3F(stream);
			SolarColor[3] = t3dV3F::fromStreamAsBytes(stream);
		}
	}
}


/* -----------------10/06/99 16.02-------------------
 *              SetVisibleFromLight
 * --------------------------------------------------*/
void t3dLIGHT::SetVisibleFromLight(gVertex *v) {
	AnimLight.VisVerts.push_back(v);
}


/* -----------------10/06/99 16.02-------------------
 *          setupVisibleVerticesFromLight
 * --------------------------------------------------*/
void t3dLIGHT::setupVisibleVerticesFromLight(t3dBODY *b) {
	gVertex     *gv;
	for (int k = 0; k < b->NumMeshes(); k++) {
		t3dMESH &m = b->MeshTable[k];
#ifndef WMGEN
		m.VBptr = m.VertexBuffer;
		gv = m.VBptr;
#else
		gv = m->VBptr;
#endif
		for (int j = 0; j < m.NumVerts; j++, gv++)
			if (LightgVertex(gv, this))
				SetVisibleFromLight(gv);

#ifndef WMGEN
		gv = nullptr;
		m.VBptr = nullptr;
#endif
	}
}

t3dPLIGHT::t3dPLIGHT(Common::SeekableReadStream &stream) {
	Num = stream.readByte();                                                         // Legge numero pos

	Pos.x = stream.readFloatLE();                                                    // Legge Pos
	Pos.y = stream.readFloatLE();
	Pos.z = stream.readFloatLE();

	Dir.x = stream.readFloatLE();                                                    // Legge Dir
	Dir.y = stream.readFloatLE();
	Dir.z = stream.readFloatLE();
}

} // End of namespace Watchmaker
