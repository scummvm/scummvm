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

#include "watchmaker/3d/material.h"
#include "common/util.h"
#include "watchmaker/3d/render/opengl_2d.h"
#include "watchmaker/render.h"

namespace Watchmaker {

MaterialPtr rAddMaterial(gMaterial &Material, const Common::String &TextName, int NumFaces, unsigned int LoaderFlags) {
	// TODO: This is duplicated in opengl_3d.cpp
	warning("TODO: Fix rAddMaterial");
#if 0
	bool            AlreadyLoaded = FALSE;
	int             len = strlen(TextName);

	if (((TextName[len - 1 - 0] == 'i') || (TextName[len - 1 - 0] == 'I')) &&
	        ((TextName[len - 1 - 1] == 'v') || (TextName[len - 1 - 1] == 'V')) &&
	        ((TextName[len - 1 - 2] == 'a') || (TextName[len - 1 - 2] == 'A'))) {
		if ((Material.Movie = gLoadMovie(TextName)) == NULL)
			return NULL;
		if ((Material.Texture = gUserTexture(64,
		                                     128)) == NULL)
//		if( (Material->Texture=gUserTexture(    Material->Movie->g_psiStreamInfo.rcFrame.right,
//										Material->Movie->g_psiStreamInfo.rcFrame.bottom)) == NULL )
			return NULL;
		Material.Flags |= T3D_MATERIAL_MOVIE;
	} else {
		if ((Material.Texture = gLoadTexture(TextName, LoaderFlags)) == NULL)
			return NULL;
	}

//f
//f Material->FacesList=(WORD *)t3dRealloc(Material->FacesList,sizeof(WORD)*3*NumFaces+1);
//f Material->NumAllocatedFaces+=3*NumFaces+1;
	Material.FacesList.resize(Material.FacesList.size() + NumFaces * 3);
	Material.NumAllocatedFaces += NumFaces * 3;
//f
	Material.Flags |= T3D_MATERIAL_NOLIGHTMAP;
	return Material;
#endif
	return nullptr;
}

void gMaterial::addProperty(int flag) {
	this->Flags |= flag;
}

void gMaterial::clearFlag(int flag) {
	this->Flags &= ~flag;
}

bool gMaterial::hasFlag(int flag) {
	return this->Flags & flag;
}

void gMaterial::addColor(unsigned char r_add, unsigned char g_add, unsigned char b_add) {
	int rr, gg, bb;

	rr = this->r;
	gg = this->g;
	bb = this->b;

	rr += r;
	gg += g;
	bb += b;

	rr = MIN(MAX(rr, 0), 255);
	gg = MIN(MAX(gg, 0), 255);
	bb = MIN(MAX(bb, 0), 255);

	this->r = (unsigned char)rr;
	this->g = (unsigned char)gg;
	this->b = (unsigned char)bb;
}

bool gMaterial::addNumFacesAdditionalMaterial(MaterialPtr am, unsigned int num) {
	if (!num || !am)
		return false;

	Common::SharedPtr<gMaterial> cm;
	int i;
	for (i = 0; i < this->NumAddictionalMaterial; i++) {
		cm = this->AddictionalMaterial[i];
		if (cm->Texture->ID == am->Texture->ID)
			break;
	}

	if (i == this->NumAddictionalMaterial) {
		this->AddictionalMaterial.push_back(Common::SharedPtr<gMaterial>(new gMaterial(*am)));
		cm = this->AddictionalMaterial.back();
		cm->FacesList.resize(0);
		this->NumAddictionalMaterial++;
	}
	return true;
}

bool gMaterial::addNumFaces(unsigned int num) {
	// TODO: Remove, as this is not necessary with a Common::Array
	return true;
}


MaterialPtr rMergeMaterial(MaterialPtr Mat1, MaterialPtr Mat2) {
	if (!Mat1 || !Mat2)
		return nullptr;

	for (int i = 0; i < Mat2->NumAddictionalMaterial; i++) {
		Mat1->addNumFacesAdditionalMaterial(Mat2->AddictionalMaterial[i],
		                                    /*Mat2->AddictionalMaterial[i]->NumAllocatedFaces*/ Mat2->AddictionalMaterial[i]->NumFaces());
	}
	//reset mat2
	rRemoveMaterial(Mat2);
	*Mat2 = gMaterial();

	return Mat1;
}

void rRemoveMaterials(Common::Array<Common::SharedPtr<gMaterial>> &m) {
	for (auto &material : m) {
		material->clear();
	}
	m.clear();
}

Common::SharedPtr<gMaterial> rCopyMaterial(Common::SharedPtr<gMaterial> Mat1, Common::SharedPtr<gMaterial> Mat2) {
	if (!Mat1 || !Mat2)
		return nullptr;

	Mat1->clearFaceList();
	Mat1->AddictionalMaterial.clear();
	Mat1->clearFaceList();
	Mat1->VertsList.clear();
	//t3dFree(Mat1->FlagsList);
	*Mat1 = gMaterial();

	if (Mat2->NumFaces()) {
		for (int i = 0; i < Mat2->NumFaces(); i++) {
			Mat1->addFace(Mat2->getFace(i));
		}
	}
	for (int i = 0; i < Mat2->NumFaces(); i++) {
		if (Mat2->getFace(i) >= Mat2->VertsList.size()) {
			warning("TODO");
		}
	}
	if (Mat2->NumAllocatedVerts()) {
		Mat1->VertsList = Mat2->VertsList;
	}
	if (Mat2->NumAllocatedMesh) {
		Mat1->FlagsList = Mat2->FlagsList;
	}

	Mat1->Texture = Mat2->Texture;
	Mat1->Movie = Mat2->Movie;
	Mat1->Flags = Mat2->Flags;
	Mat1->VBO = Mat2->VBO;
	Mat1->NumAllocatedMesh = Mat2->NumAllocatedMesh;
	Mat1->r = Mat2->r;
	Mat1->g = Mat2->g;
	Mat1->b = Mat2->b;
	Mat1->NumAddictionalMaterial = Mat2->NumAddictionalMaterial;

	rCopyMaterialList(Mat1->AddictionalMaterial, Mat2->AddictionalMaterial, Mat2->NumAddictionalMaterial); // TODO: Does this mean that we don't copy any extras?

	return Mat1;
}

void rCopyMaterialList(MaterialTable &dst, MaterialTable &src, uint count) {
	dst.resize(count);
	if (count > src.size()) {
		error("Copying more materials than there are in the src");
	}
	for (uint i = 0; i < count; i++) {
		if (!dst[i]) {
			dst[i] = Common::SharedPtr<gMaterial>(new gMaterial());
		}
		rCopyMaterial(dst[i], src[i]);
	}
}

void gMaterial::clear() {
	// TODO: This flag clearing doesn't happen in the original, but shouldn't matter as the class is instantiated again when used in Particles.
	Flags = 0;

	if (Movie) {
		Movie = nullptr;
	}
	FacesList.clear();
	VertsList.clear();
	FlagsList.clear();
//  rDeleteVertexBuffer(m->VB);
	VBO = 0;

	for (int j = 0; j < NumAddictionalMaterial; j++) {
		Common::SharedPtr<gMaterial> cm = AddictionalMaterial[j];
		cm->FacesList.clear();
		cm->VertsList.clear();
		cm->FlagsList.clear();
//      rDeleteVertexBuffer(cm->VB);
		cm->VBO = 0;
	}
	AddictionalMaterial.clear();
}


void rRemoveMaterial(Common::SharedPtr<gMaterial> &m) {
	m->clear();
}


/* -----------------29/07/99 15.53-------------------
 *  Aggiunge un materiale alla MaterialList
 * --------------------------------------------------*/
void rAddToMaterialList(gMaterial &mat, signed short int ViewMatrixNum) {
	gBatchBlock *bb = nullptr;

	if ((mat.Flags & T3D_MATERIAL_MOVIE)) {
		warning("Movie: %s %d", mat.Movie->_name.c_str(), mat.Texture->ID);
		mat.Movie->updateMovie();
	}

	if ((mat.NumFaces() >= 3) && (mat.VBO)) {
		if (mat.Texture) {
	//		if (mat.Texture->name == "./TMaps/bianco.tga")
	//			return;
		}
		bb = rNewBatchBlock(mat.Texture->ID, mat.Flags, 0, 0);
		bb->ViewMatrixNum = ViewMatrixNum;
		bb->FacesList = mat.getFacesList();
		bb->VBO = mat.VBO;
		for (uint f = 0; f < bb->FacesList.size(); f++) {
			if (bb->FacesList[f] >= bb->VBO->_buffer.size()) {
				for (uint o = 0; o < bb->FacesList.size(); o++) {
					warning("%d", bb->FacesList[o]);
				}
				warning("%d > %d (%d)", bb->FacesList[f], bb->VBO->_buffer.size(), bb->NumVerts());
			}
		}
		mat.emptyFacesList(); // We may want to keep the reservation to avoid the extra reallocs here.
	}

	for (auto &cm : mat.AddictionalMaterial) {
		if (cm->NumFaces() < 3) continue;
		if (cm->VBO == NULL) continue;
		bb = rNewBatchBlock(mat.Texture->ID, mat.Flags, cm->Texture->ID, cm->Flags);
		bb->ViewMatrixNum = ViewMatrixNum;
		bb->FacesList = cm->getFacesList();
		bb->VBO = cm->VBO;
		cm->emptyFacesList();
	}
}

void rAddToMaterialList(MaterialPtr mat, signed short int ViewMatrixNum) {
	if (mat) {
		rAddToMaterialList(*mat, ViewMatrixNum);
	}
}

/* -----------------31/05/99 10.07-------------------
 *  Costruisce la lista dei materiali ordinata
 * --------------------------------------------------*/
void rBuildMaterialList(MaterialTable &MatList, unsigned int NumMat, signed short int ViewMatrixNum) {
	if (NumMat == 0)
		return;

	for (auto &mat : MatList) {
		rAddToMaterialList(mat, ViewMatrixNum);
	}
}

MaterialTable rCreateMaterialList(int num) {
	MaterialTable list;
	// We avoid actually allocating the gMaterial-objects, as we want the size() to
	// represent the actually loaded elements.
	list.reserve(num);
	return list;
}

} // End of namespace Watchmaker
