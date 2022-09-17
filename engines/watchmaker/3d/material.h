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

#ifndef WATCHMAKER_MATERIAL_H
#define WATCHMAKER_MATERIAL_H

#include "common/array.h"
#include "common/ptr.h"
#include "watchmaker/3d/texture.h"
#include "watchmaker/3d/movie.h"
#include "watchmaker/3d/vertex.h"

namespace Watchmaker {

struct gMaterial;
typedef Common::SharedPtr<gMaterial> MaterialPtr;
typedef Common::Array<MaterialPtr> MaterialTable;

struct VertexBuffer;
// Material definition
struct gMaterial {
	gTexture                *Texture = nullptr;         // pointer to texture struct
	Common::SharedPtr<gMovie> Movie;                      // pointer to movie struct
	unsigned int            Flags = 0;                  // material flags
	int NumFaces() { return FacesList.size(); };        // current number of faces to be processed
	void addFace(uint16 face) {
		FacesList.push_back(face);
	}
	uint16 getFace(int index) const { return FacesList[index]; }
	void clearFaceList() { FacesList.clear(); }
	void emptyFacesList() { FacesList.resize(0); }
	Common::Array<uint16> getFacesList() { return FacesList; }
private:
	Common::Array<uint16>     FacesList;                  // list of verts indices
public:
	Common::Array<gVertex*> VertsList;                  // pointers to pointers to verts
	int                     NumAllocatedVerts() { return this->VertsList.size(); };      // number of allocated vertex in mat VB
	Common::SharedPtr<VertexBuffer> VBO = nullptr;
//	LPDIRECT3DVERTEXBUFFER7 VB;                         // mat VB struct
	int                     NumAllocatedMesh = 0;       // num mesh to check for modifications
	Common::Array<unsigned int *> FlagsList;              // vector of pointer to mesh flags
	unsigned char           r, g, b;                    // default material color
	int                     NumAddictionalMaterial = 0; // number of addictional material (lightmaps)
	MaterialTable		    AddictionalMaterial;        // pointer to addictional material struct
public:
	gMaterial() : r(0), g(0), b(0) {

	}
	void addColor(unsigned char r, unsigned char g, unsigned char b);
	void addProperty(int flag);
	bool hasFlag(int flag);
	void clearFlag(int flag);
	bool addNumFaces(unsigned int num);
	bool addNumFacesAdditionalMaterial(MaterialPtr am, unsigned int num);
	void clear();
};

MaterialPtr rAddMaterial(MaterialTable &MList, const Common::String &TextName, int NumFaces, unsigned int LoaderFlags);
MaterialPtr rAddMaterial(gMaterial &Material, const Common::String &TextName, int NumFaces, unsigned int LoaderFlags);
void rRemoveMaterial(MaterialPtr &m);
void rRemoveMaterials(MaterialTable &m);
MaterialPtr rCopyMaterial(MaterialPtr Mat1, MaterialPtr Mat2);
MaterialPtr rMergeMaterial(MaterialPtr Mat1, MaterialPtr Mat2);
void rAddToMaterialList(gMaterial &mat, signed short int ViewMatrixNum);


} // End of namespace Watchmaker

#endif // WATCHMAKER_MATERIAL_H
