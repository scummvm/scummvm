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
#define FORBIDDEN_SYMBOL_EXCEPTION_sprintf

#include "watchmaker/3d/t3d_body.h"
#include "common/stream.h"
#include "watchmaker/3d/light.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/t3d_face.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/file_utils.h"
#include "watchmaker/game.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/renderer.h"

namespace Watchmaker {

#define CAMFILEVERSION  2
#define BNDFILEVERSION  2

void t3dLoadMaterials(WGame &game, t3dBODY *b, Common::SeekableReadStream &stream, int numMaterials) {
	int16 loader_numtextures = 0;
	WorkDirs &workdirs = game.workDirs;
	for (uint16 material = 0; material < numMaterials; material++) {                    // Legge Materiali
		char    Name[100] = {}, Appo[100] = {};
		unsigned int        Flags = 0, flag = 0;
#ifndef WMGEN
		int len;
#endif

		unsigned char AmbientR = (uint8)(stream.readFloatLE());                                                      // legge Ambient
		unsigned char AmbientG = (uint8)(stream.readFloatLE());
		unsigned char AmbientB = (uint8)(stream.readFloatLE());

		for (uint16 kk = 0; kk < T3D_NAMELEN; kk++)                                                    // Legge noem della texture
			Name[kk] = (uint8)stream.readByte();

		Flags = stream.readSint32LE();                                                                    // Legge Flags

#ifndef WMGEN
		len = strlen(Name);
		if (((Name[len - 1] == 'i') || (Name[len - 1] == 'I')) &&
		        ((Name[len - 2] == 'v') || (Name[len - 2] == 'V')) &&
		        ((Name[len - 3] == 'a') || (Name[len - 3] == 'A'))) {
			strcpy(Appo, workdirs._moviesDir.c_str());                                                              // altrimenti prende quello di default
		} else {
			strcpy(Appo, workdirs._mapsDir.c_str());                                                            // altrimenti prende quello di default
		}
		strcat(Appo, Name);                                                                     // Attacca nome Immagine
#else
		strcpy(Appo, WmMapsDir);                                                            // altrimenti prende quello di default
		strcat(Appo, Name);                                                                     // Attacca nome TGA
#endif


//		if( LoaderFlags&T3D_HALFTEXTURESIZE )                                                   // Se ho settato texture dimezzate
//			strcpy(Appo,WmHalfMapsDir);                                                            // cambio path
//		else
//f     strcat(Appo,"grid.tga");

		if (LoaderFlags & T3D_HALFTEXTURESIZE) flag = rSURFACEHALF;                             // Se deve scalare le textures
		else flag = 0;
		MaterialPtr mat(new gMaterial());
		assert(b->MatTable.size() == material);
		b->MatTable.push_back(mat);
		//warning("Loading material %d", material);
#ifndef WMGEN
		if (!(game._renderer->addMaterial(*mat, Appo,/*f1*/0, flag))) {                                            // Carica e scala texture
			warning("Material file %s not found, ", Appo);                                               // Se non trova la texture
			mat->Texture = nullptr;
			assert(0);
			//return nullptr;
		} else
#endif
		{
			mat->addProperty(Flags);                                              // Aggiunge i Flags al materiale
//			if(LoaderFlags&T3D_NOLIGHTMAPS)
			mat->addColor(AmbientR, AmbientG, AmbientB);                          // Sovrascrive Ambient con quello gloable
			mat->addColor((uint8)b->AmbientLight.x, (uint8)b->AmbientLight.y, (uint8)b->AmbientLight.z);
			loader_numtextures++;
		}
		assert(mat->Texture);
	}//__for_material
	(void)loader_numtextures;
}

void t3dLoadMeshes(t3dBODY *b, uint32 numMeshes, t3dMESH *&ReceiveRipples, uint8 &Mirror, Common::SeekableReadStream &stream) {
	b->MeshTable.clear();
	b->MeshTable.reserve(numMeshes);
	for (uint32 mesh = 0; mesh < numMeshes; mesh++) {
		b->MeshTable.push_back(Common::move(t3dMESH(b, stream, ReceiveRipples, Mirror)));
	}
}

void t3dBODY::allocateNormals() {
	int nListSize = this->NumNormals + this->NumVerticesNormals;
	this->NList.clear();
	this->NList.reserve(nListSize);
	for (int i = 0; i < nListSize; i++) { // Alloca Normali globali
		this->NList.push_back(Common::SharedPtr<t3dNORMAL>(new t3dNORMAL()));
	}
}

void t3dBODY::initNormals(Common::SeekableReadStream &stream) {
	uint nListSize = this->NumNormals + this->NumVerticesNormals;
	assert(this->NList.size() == nListSize);
	for (uint16 normal = 0; normal < this->NList.size(); normal++) {                // Legge normali globali
		*this->NList[normal] = t3dNORMAL(stream);
	}

	for (uint16 i = 0; i < this->NumMeshes(); i++) {                                                // Aggiorna normali mesh con normali globali
		t3dMESH &Mesh = this->MeshTable[i];
#ifndef WMGEN
		Mesh.VBptr = Mesh.VertexBuffer;
#endif
		Mesh.NList.reserve(Mesh.NumVerts); // Alloca normali mesh
		for (int k = 0; k < Mesh.NumVerts; k++) {
			Mesh.NList.push_back(Common::SharedPtr<t3dNORMAL>(new t3dNORMAL()));
		}

		Mesh.NumNormals = 0;
		Mesh.NumVerticesNormals = Mesh.NumVerts;                                              // Sono tutte normali ai vertici
		for (uint16 j = 0; j < Mesh.NumVerts; j++) {
			uint16 n = (uint16)Mesh.VBptr[j].diffuse;
			*Mesh.NList[j] = *this->NList[n]; // Copia normale globale in normale mesh
			Mesh.VBptr[j].diffuse = 0;
		}
#ifndef WMGEN
		Mesh.VBptr = nullptr;
#endif
	}
}

void t3dBODY::populatePortalLists() {
	t3dMESH *Mesh = &this->MeshTable[0];
	for (uint16 mesh = 0; mesh < this->NumMeshes(); mesh++) {                                               // Cerca portali in tutte le mesh
		Mesh[mesh].PortalList = nullptr;                                                           // Azzera portale
		if (Mesh[mesh].portalName.empty())                                                 // Se non ha portale
			continue;                                                                           // continua
		if (Mesh[mesh].portalName == "castle") {                                          // Aggiunge mesh che bloccano la vista
//			DebugFile("%s: %d %d",Mesh[mesh].Name,Mesh[mesh].NumVerts,Mesh[mesh].NumFaces);
			for (uint16 i = 0; i < T3D_MAX_BLOCK_MESHES; i++) {
				if (!this->BlockMeshes[i]) {
					this->BlockMeshes[i] = &Mesh[mesh];
					break;
				}
			}
			continue;
		}

		Common::String Name = Mesh[mesh].portalName + ".t3d"; // Crea nome del nuovo portale

		Mesh[mesh].Flags |= T3D_MESH_PORTAL;                                                    // Mesh e' un portale
		Mesh[mesh].Flags |= T3D_MESH_NOBOUNDBOX;                                                // Mesh non viene vista dal mouse
//#ifndef VIEWER
//		Mesh[mesh].Flags|=T3D_MESH_NOPORTALCHECK;                                               // Spegne portale
//#endif
		// TODO: This should probably be upfactored.
		t3dBODY *rez = nullptr;
		if (((rez = _vm->_roomManager->checkIfAlreadyLoaded(Name)) == nullptr) && (!(LoaderFlags & T3D_NORECURSION))) { // Controlla se lo ha gia' caricato
			if (Name.equalsIgnoreCase("rxt.t3d"))
				_vm->_roomManager->addToLoadList(&Mesh[mesh], Name, (uint16)((LoaderFlags | T3D_NORECURSION) & ~T3D_RECURSIONLEVEL1));    // aggiunge e leva la ricorsione
//				Mesh[mesh].Flags|=T3D_MESH_NOPORTALCHECK;
			else {
				if (LoaderFlags & T3D_RECURSIONLEVEL1)
					_vm->_roomManager->addToLoadList(&Mesh[mesh], Name, (uint16)((LoaderFlags | T3D_NORECURSION) & ~T3D_RECURSIONLEVEL1));    // aggiunge e leva la ricorsione
				else
					_vm->_roomManager->addToLoadList(&Mesh[mesh], Name, (uint16)(LoaderFlags));                         // altrimenti lo agggiunge alla lista
			}
		} else
			Mesh[mesh].PortalList = rez;
	}
}

/* -----------------10/06/99 16.03-------------------
 *              SetupWaterRipples
 * --------------------------------------------------*/
void SetupWaterRipples(t3dMESH *m) {
	warning("TODO: Stubbed");
	return;
}

/* -----------------10/06/99 16.02-------------------
 *                  LoadCameras
 * --------------------------------------------------*/
void LoadCameras(WorkDirs &workDirs, const char *pname, t3dBODY *b) {
	uint8 ver;
	uint16  camera;

	auto stream = workDirs.resolveFile(pname);
	if (!stream) {
		warning("File %s not found", pname);
		return ;
	}
	if ((ver = stream->readByte()) != CAMFILEVERSION) {
		warning("CAM File Version Error: loaded %d.\tRequired %d", ver, CAMFILEVERSION);
		return ;
	}

	int numCameras = stream->readSint16LE();
	int numPaths = stream->readSint16LE();

	b->CameraTable.clear();
	b->CameraTable.reserve(numCameras);
	for (camera = 0; camera < numCameras; camera++) {
		b->CameraTable.push_back(t3dCAMERA(*stream));
	}//__for_camera

	b->CameraGrid.TopLeft.x = stream->readFloatLE() * SCALEFACTOR;
	b->CameraGrid.TopLeft.z = stream->readFloatLE() * SCALEFACTOR;
	b->CameraGrid.BottomRight.x = stream->readFloatLE() * SCALEFACTOR;
	b->CameraGrid.BottomRight.z = stream->readFloatLE() * SCALEFACTOR;

	b->CameraGrid.Row = stream->readSint16LE();
	b->CameraGrid.Col = stream->readSint16LE();

	b->CameraGrid.Grid.clear();
	b->CameraGrid.Grid.reserve(b->CameraGrid.Row * b->CameraGrid.Col);

	for (int i = 0; i < (b->CameraGrid.Row * b->CameraGrid.Col); i++) {
		b->CameraGrid.Grid.push_back(stream->readByte());
	}

	b->CameraPath.clear();
	b->CameraPath.reserve(numPaths);
	for (int i = 0; i < numPaths; i++) {
		b->CameraPath.push_back(t3dCAMERAPATH(*stream));
	}

	b->CameraGrid.CellDim.x = (b->CameraGrid.BottomRight.x - b->CameraGrid.TopLeft.x) / b->CameraGrid.Col;
	b->CameraGrid.CellDim.z = (b->CameraGrid.BottomRight.z - b->CameraGrid.TopLeft.z) / b->CameraGrid.Row;
}

/* -----------------10/06/99 16.03-------------------
 *                      loadBounds
 * --------------------------------------------------*/
void loadBounds(WorkDirs &workDirs, const char *pname, t3dBODY *b) {
	for (int i = 0; i < T3D_MAX_LEVELS; i++)
		b->Panel[i] = nullptr;
	b->CurLevel = 0;

	auto stream = workDirs.resolveFile(pname);
	if (!stream) {
		warning("File %s not found", pname);
		return ;
	}
	uint8 ver = stream->readByte();
	if (ver != BNDFILEVERSION) {
		warning("BND File Version Error: loaded %d.\tRequired %d", ver, BNDFILEVERSION);
		return;
	}
	uint16 nlev = stream->readSint16LE();
	b->NumLevels = nlev;
	if (nlev > T3D_MAX_LEVELS) {
		warning("Too much Floor Levels in %s: %d instead of %d", pname, b->NumLevels, T3D_MAX_LEVELS);
		b->NumLevels = nlev = T3D_MAX_LEVELS;
	}

	for (int j = 0; j < nlev; j++) {
		uint16 npan = stream->readSint16LE();
		b->NumPanels[j] = npan;
		b->PanelHeight[j] = stream->readFloatLE() * SCALEFACTOR;
		b->Panel[j] = new t3dPAN[npan + 4 * T3D_MAX_CHARACTERS];        // also leave some space for any additions

		for (int i = 0; i < npan; i++) {
			b->Panel[j][i].a.x = stream->readFloatLE() * SCALEFACTOR;
			b->Panel[j][i].a.z = stream->readFloatLE() * SCALEFACTOR;
			b->Panel[j][i].b.x = stream->readFloatLE() * SCALEFACTOR;
			b->Panel[j][i].b.z = stream->readFloatLE() * SCALEFACTOR;
			b->Panel[j][i].backA.x = stream->readFloatLE() * SCALEFACTOR;
			b->Panel[j][i].backA.z = stream->readFloatLE() * SCALEFACTOR;
			b->Panel[j][i].backB.x = stream->readFloatLE() * SCALEFACTOR;
			b->Panel[j][i].backB.z = stream->readFloatLE() * SCALEFACTOR;
			b->Panel[j][i].near1 = stream->readSint16LE();
			b->Panel[j][i].near2 = stream->readSint16LE();
		}
	}
}

/* -----------------10/06/99 16.03-------------------
 *                  LoadLightmaps
 * --------------------------------------------------*/
void LoadLightmaps(WGame &game, t3dBODY *b) {
	uint32  NumLightmaps;//, num;
	gVertex *gv;
	uint8   rr, gg, bb;
	int32  Map, alphaval1, alphaval2, alphaval3;
	WorkDirs &workDirs = game.workDirs;

	Common::String Appo = setDirectoryAndName(workDirs._lightmapsDir, b->name);
	Appo = replaceExtension(Appo.c_str(), "map");

	auto stream = workDirs.resolveFile(Appo);
	if (!stream) {
		warning("LoadLightmaps warning: File %s not found", Appo.c_str());
		return;
	}

	uint32 lightMapVersion = stream->readSint32LE();
	if (lightMapVersion != LIGHT_MAPVERSION) {
		warning("LoadLightmaps error: File version error: found %d, required %d", lightMapVersion, LIGHT_MAPVERSION);
		return;
	}

	b->NumLightmaps = NumLightmaps = stream->readSint32LE();
	b->LightmapTable = rCreateMaterialList(NumLightmaps);
	for (uint32 i = 0; i < NumLightmaps; i++) {
		Common::String root = setDirectoryAndName(workDirs._lightmapsDir, b->name);
		root = root.substr(0, root.size() - 4);
		Appo = Common::String::format("%s_%d.tga", root.c_str(), i);

		/*num = */stream->readSint32LE();
		MaterialPtr mat(new gMaterial());
		assert(b->LightmapTable.size() == i);
		b->LightmapTable.push_back(mat);
		if (!game._renderer->addMaterial(*b->LightmapTable[i], Appo, 0, 0))
			return;

		b->LightmapTable[i]->clear();
	}

	for (uint32 i = 0; i < b->NumNormals; i++) {
		rr = (uint8)stream->readByte();
		gg = (uint8)stream->readByte();
		bb = (uint8)stream->readByte();
		Map = (int32)stream->readSint32LE();

		Appo = setDirectoryAndName(workDirs._lightmapsDir.c_str(), b->name);
		Appo = Common::String::format("%s_%d.tga", Appo.c_str(), Map);

		if (Map >= 0) {
			for (uint32 k = 0; k < b->NumMeshes(); k++) {
				t3dMESH &m = b->MeshTable[k];
//f             if (m->Flags&T3D_MESH_HIDDEN)
//f                 continue;
				m.VBptr = m.VertexBuffer;
				gv = m.VBptr;

				if (m.Flags & T3D_MESH_NOLIGHTMAP) {
					t3dFACE &f = m.FList[0];
					for (uint32 j = 0; j < m.NumFaces(); j++) {
						if (f.n == b->NList[i]) {
							f.lightmap = nullptr;
						}
					}
					m.VBptr = nullptr;
					continue;
				}
				t3dFACE &f = m.FList[0];
				for (uint32 j = 0; j < m.NumFaces(); j++) {
//					gMaterial *newmat;
					if (f.n == b->NList[i]) {
//						f->mat->Texture=f->mat->Lightmap;
						if ((!(f.hasMaterialFlag(T3D_MATERIAL_OPACITY))) &&
						        (!(f.hasMaterialFlag(T3D_MATERIAL_CLIPMAP))) &&
						        (!(f.hasMaterialFlag(T3D_MATERIAL_BOTTLE))) &&
						        (!(f.hasMaterialFlag(T3D_MATERIAL_ADDITIVE))) &&
						        (!(f.hasMaterialFlag(T3D_MATERIAL_GLASS)))) {
							alphaval1 = RGBA_GETALPHA(gv[f.VertexIndex[0]].diffuse);
							alphaval2 = RGBA_GETALPHA(gv[f.VertexIndex[1]].diffuse);
							alphaval3 = RGBA_GETALPHA(gv[f.VertexIndex[2]].diffuse);
							gv[f.VertexIndex[0]].diffuse = RGBA_MAKE(254, 254, 254, alphaval1);
							gv[f.VertexIndex[1]].diffuse = RGBA_MAKE(254, 254, 254, alphaval2);
							gv[f.VertexIndex[2]].diffuse = RGBA_MAKE(254, 254, 254, alphaval3);
							f.lightmap = b->LightmapTable[Map];
							f.getMaterial()->addNumFacesAdditionalMaterial(f.lightmap, 1);
						} else if (((f.hasMaterialFlag(T3D_MATERIAL_OPACITY))) ||
						           ((f.hasMaterialFlag(T3D_MATERIAL_CLIPMAP))) ||
						           ((f.hasMaterialFlag(T3D_MATERIAL_BOTTLE))) ||
						           ((f.hasMaterialFlag(T3D_MATERIAL_ADDITIVE))) ||
						           ((f.hasMaterialFlag(T3D_MATERIAL_GLASS)))) {
							f.lightmap = nullptr;
						}

//						if (!f->mat->Lightmap)
						{
							//DebugLogWindow("Mesh %s: Face %d has not lightmap",m->Name,j);
						}
					}
				}
				m.VBptr = nullptr;
			}
		} else {
			for (uint32 k = 0; k < b->NumMeshes(); k++) {
				t3dMESH &m = b->MeshTable[k];
//				gMaterial *newmat;
				m.VBptr = m.VertexBuffer;
				gv = m.VBptr;

//f             if (m->Flags&T3D_MESH_HIDDEN)
//f                 continue;

				if (m.Flags & T3D_MESH_NOLIGHTMAP) {
					t3dFACE &f = m.FList[0];
					for (uint32 j = 0; j < m.NumFaces(); j++) {
						if (f.n == b->NList[i]) {
							f.lightmap = nullptr;
						}
					}
					m.VBptr = nullptr;
					continue;
				}

				t3dFACE &f = m.FList[0];
				for (uint32 j = 0; j < m.NumFaces(); j++) {
//					t3dU32 nr,ng,nb;
					if (f.n == b->NList[i]) {
						f.lightmap = nullptr;
//						if (Map==-1)
						if ((!(f.hasMaterialFlag(T3D_MATERIAL_OPACITY))) &&
						        (!(f.hasMaterialFlag(T3D_MATERIAL_CLIPMAP))) &&
						        (!(f.hasMaterialFlag(T3D_MATERIAL_BOTTLE))) &&
						        (!(f.hasMaterialFlag(T3D_MATERIAL_ADDITIVE))) &&
						        (!(f.hasMaterialFlag(T3D_MATERIAL_GLASS)))) {
							alphaval1 = RGBA_GETALPHA(gv[f.VertexIndex[0]].diffuse);
							alphaval2 = RGBA_GETALPHA(gv[f.VertexIndex[1]].diffuse);
							alphaval3 = RGBA_GETALPHA(gv[f.VertexIndex[2]].diffuse);
							gv[f.VertexIndex[0]].diffuse = RGBA_MAKE(rr, gg, bb, alphaval1);
							gv[f.VertexIndex[1]].diffuse = RGBA_MAKE(rr, gg, bb, alphaval2);
							gv[f.VertexIndex[2]].diffuse = RGBA_MAKE(rr, gg, bb, alphaval3);
						}
					}
				}
//				f->mat->Texture= nullptr;

				m.VBptr = nullptr;
			}
		}
	}

	for (uint32 i = 0; i < b->NumMeshes(); i++) {
		t3dMESH &m = b->MeshTable[i];
		m.VBptr = m.VertexBuffer;
		gv = m.VBptr;
		for (uint32 j = 0; j < m.NumVerts; j++) {
			t3dF32  u = stream->readFloatLE();
			t3dF32  v = stream->readFloatLE();

			if ((RGBA_GETRED(gv[j].diffuse) == 254) &&
			        (RGBA_GETGREEN(gv[j].diffuse) == 254) &&
			        (RGBA_GETBLUE(gv[j].diffuse) == 254)) {
				gv[j].u2 = u;
				gv[j].v2 = v;
				gv[j].diffuse = RGBA_MAKE(255, 255, 255, RGBA_GETALPHA(gv[j].diffuse));

//				gv[j].u2=(rand())%2;
//				gv[j].v2=(rand())%2;
			} else {
				gv[j].u2 = gv[j].u1;
				gv[j].v2 = gv[j].v1;
			}

//			gv[j].diffuse=0xFFFFFFFF;
//			DebugLogWindow("%f %f",gv[j].u1,gv[j].v1);
		}
		m.VBptr = nullptr;
	}
}

/* -----------------10/06/99 16.04-------------------
 *                  t3dLoadSingleRoom
 * --------------------------------------------------*/
t3dBODY *t3dBODY::loadFromStream(WGame &game, const Common::String &pname, Common::SeekableReadStream &stream, uint32 _LoaderFlags) {
	//decodeLoaderFlags(_LoaderFlags);
	//char /*t3dU8*/   Name[255];

	uint16  light;
	t3dF32  minx, miny, minz, maxx, maxy, maxz;

	WorkDirs &workdirs = game.workDirs;

	this->name = pname;
	auto _name = constructPath(workdirs._t3dDir, pname);

	this->NumTotVerts = 0;

	this->AmbientLight = t3dV3F::fromStreamAsBytes(stream);
	int numMeshes = stream.readSint16LE();                                                                 // Legge globali del body
	int numMaterials = stream.readSint16LE();
	int numLights = stream.readSint16LE();
	int numPosLights = stream.readSint16LE();

	this->NumNormals = stream.readSint32LE();
	this->NumVerticesNormals = stream.readSint32LE();
	this->allocateNormals();
	this->MatTable = rCreateMaterialList(numMaterials);                              // Crea Materiali Globali
	this->LightmapTable.clear();
//-------------------LOADING MESHES--------------------------------------
	t3dMESH *ReceiveRipples = nullptr;
	uint8   Mirror = 0;
	t3dLoadMeshes(this, numMeshes, ReceiveRipples, Mirror, stream); // TODO: We probably don't need to pass ReceiveRipples, Mirror
//-------------------END OF LOADING MESHES-------------------------------
	this->initNormals(stream);

//-------------------LOADING MATERIALS--------------------------------------
	t3dLoadMaterials(game, this, stream, numMaterials);
//-------------------LOADING LIGHTS--------------------------------------
	this->LightTable.reserve(numLights); // Alloca spazio per le luci globali
	for (light = 0; light < numLights; light++) {
		this->LightTable.push_back(t3dLIGHT(game, this, workdirs, stream)); // Azzera luce
	}//__for_light
//-------------------END OF LOADING LIGHTS-------------------------------

	this->PosLightTable.clear();
	this->PosLightTable.reserve(numPosLights);                // Alloca spazio per luci di posizione
	for (light = 0; light < numPosLights; light++) {
		this->PosLightTable.push_back(t3dPLIGHT(stream));
	}//__for_plight

//-------------------Extra optimization here-------------------------------
#ifndef WMGEN
	if (ReceiveRipples)                                                                         // Se c'era una mesh con i riples
		SetupWaterRipples(ReceiveRipples);                                                  // crea i buffers
#endif

//-------------------Prelighting body-------------------------------
	for (uint16 i = 0; i < this->NumMeshes(); i++) { // Cerca in tutte le mesh
		this->MeshTable[i].preCalcLights(this->AmbientLight);
	}

#ifndef WMGEN
	t3dPrecalcLight(this, nullptr);
#endif
//----------------END OF VERTEX PRE-LIGHTING---------------------

	populatePortalLists();

	warning("LoaderFlags late = %08X", _LoaderFlags);
	//decodeLoaderFlags(_LoaderFlags);
	if (!(_LoaderFlags & T3D_NOBOUNDS)) {                                                        // Carica Bounds
		auto bndName = workdirs.join(workdirs._bndDir, pname, "bnd");
		loadBounds(game.workDirs, bndName.c_str(), this);
	}
	if (!(_LoaderFlags & T3D_NOCAMERAS)) {                                                       // Carica Camere
		auto cameraName = constructPath(workdirs._camDir, pname, "cam");
		LoadCameras(game.workDirs, cameraName.c_str(), this);
	}
	if (!(_LoaderFlags & T3D_NOLIGHTMAPS)) {                                                     // Carica le Lightmaps
		// TODO: This looks odd
		if (!pname.equalsIgnoreCase("rxt.t3d") || !pname.equalsIgnoreCase("rxt-b.t3d") || !pname.equalsIgnoreCase("rxt-c.t3d") ||
		        !pname.equalsIgnoreCase("rxt-d.t3d") || !pname.equalsIgnoreCase("rxt-e.t3d") || !pname.equalsIgnoreCase("rxt.t3d-f"))
			LoadLightmaps(game, this);
	}
	if ((_LoaderFlags & T3D_OUTDOORLIGHTS)) {                                                    // Carica le luci per l'esterno
		if (pname.equalsIgnoreCase("rxt.t3d")) {
			auto outdoorLightsPath = constructPath(workdirs._lightmapsDir, pname);
			t3dLoadOutdoorLights(outdoorLightsPath.c_str(), this, t3dCurTime);
		}
	}
	if (!(_LoaderFlags & T3D_NOVOLUMETRICLIGHTS)) {                                              // Carica le luci volumetriche
		auto volMapPath = constructPath(workdirs._lightmapsDir, pname, "vol");
		LoadVolumetricMap(workdirs, volMapPath.c_str(), this);
	}

	GetBoundaries(this, &minx, &miny, &minz, &maxx, &maxy, &maxz);                             // Calcola occupazione stanza
	this->MinPos.x = minx;                                                                         // Salva il minimo per le luci volumetriche
	this->MinPos.y = miny;
	this->MinPos.z = minz;

#ifndef WMGEN
	for (uint16 i = 0; i < this->NumMeshes(); i++) {                                                // Scorre le mesh
		this->MeshTable[i].saveVertexBuffer();
	}
#endif

	t3dOptimizeMaterialList(this);                                                                 // Ottimizza la lista dei metriali (evitando i doppi)
#ifndef WMGEN
	t3dFinalizeMaterialList(this);                                                                 // Crea VB e indici per materiali
#endif

	if (Mirror) {                                                                               // Se c'era uno specchio
		this->MirrorMatTable = rCreateMaterialList(this->NumMaterials());
		rCopyMaterialList(this->MirrorMatTable, this->MatTable, this->NumMaterials());
	}


//f
#ifndef WMGEN
	for (uint16 i = 0; i < this->NumMeshes(); i++) {                                                     // Cancella le normali
		if (this->MeshTable[i].ModVertices.empty())                                                    // dalle mesh con Smoothing groups
			continue;
		for (uint16 j = 0; j < this->MeshTable[i].NumFaces(); j++) {
			this->MeshTable[i].FList[j].n = nullptr;
		}
	}
#endif
//f
	return this;
}

} // End of namespace Watchmaker
