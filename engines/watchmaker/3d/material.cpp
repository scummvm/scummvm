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

#include "common/util.h"
#include "watchmaker/3d/material.h"
#include "watchmaker/render.h"

namespace Watchmaker {

MaterialPtr rAddMaterial(gMaterial &Material, const Common::String &TextName, int NumFaces, unsigned int LoaderFlags) {
	// TODO: This is duplicated in opengl_3d.cpp
	warning("TODO: Fix rAddMaterial");
#if 0
	bool			AlreadyLoaded=FALSE;
	int				len=strlen(TextName);

	if (((TextName[len-1-0]=='i')|| (TextName[len-1-0]=='I')) &&
		((TextName[len-1-1]=='v')|| (TextName[len-1-1]=='V')) &&
		((TextName[len-1-2]=='a')|| (TextName[len-1-2]=='A')) )
	{
		if( (Material.Movie=gLoadMovie(TextName)) == NULL )
			return NULL;
		if( (Material.Texture=gUserTexture(	64,
												128)) == NULL )
//		if( (Material->Texture=gUserTexture(	Material->Movie->g_psiStreamInfo.rcFrame.right,
//										Material->Movie->g_psiStreamInfo.rcFrame.bottom)) == NULL )
			return NULL;
		Material.Flags|=T3D_MATERIAL_MOVIE;
	}
	else
	{
		if( (Material.Texture=gLoadTexture(TextName,LoaderFlags)) == NULL )
			return NULL;
	}

//f
//f	Material->FacesList=(WORD *)t3dRealloc(Material->FacesList,sizeof(WORD)*3*NumFaces+1);
//f	Material->NumAllocatedFaces+=3*NumFaces+1;
	Material.FacesList.resize(Material.FacesList.size() + NumFaces * 3 );
	Material.NumAllocatedFaces+=NumFaces*3;
//f
	Material.Flags|=T3D_MATERIAL_NOLIGHTMAP;
	return Material;
#endif
	return nullptr;
}

MaterialPtr rAddMaterial(MaterialTable &MList, const Common::String &TextName, int NumFaces, unsigned int LoaderFlags) {
	MaterialPtr &Material=MList[0];
	rAddMaterial(*Material, TextName, NumFaces, LoaderFlags);
}

void gMaterial::addProperty(int flag) {
	this->Flags |= flag;
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
	if( !num || !am )
		return false;

	Common::SharedPtr<gMaterial> cm;
	int i;
	for ( i=0; i<this->NumAddictionalMaterial; i++ ) {
		cm = this->AddictionalMaterial[i];
		if (cm->Texture->ID == am->Texture->ID)
			break;
	}

	if( i == this->NumAddictionalMaterial )
	{
		this->AddictionalMaterial.push_back(Common::SharedPtr<gMaterial>(new gMaterial(*am)));
		cm = this->AddictionalMaterial.back();
		cm->NumAllocatedFaces=0;
		cm->FacesList.clear();
		this->NumAddictionalMaterial++;
	}

	cm->FacesList.resize(cm->FacesList.size() + num*3);
	cm->NumAllocatedFaces+=num*3;
	return true;
}

bool gMaterial::addNumFaces(unsigned int num) {
	if (num == 0)
		return false;

	FacesList.resize(FacesList.size() + num * 3);
	NumAllocatedFaces += num * 3;
	return true;
}


MaterialPtr rMergeMaterial(MaterialPtr Mat1, MaterialPtr Mat2) {
	if ( !Mat1 || !Mat2 )
		return nullptr;

	for ( int i=0; i<Mat2->NumAddictionalMaterial; i++ ) {
		Mat1->addNumFacesAdditionalMaterial(Mat2->AddictionalMaterial[i],
									   Mat2->AddictionalMaterial[i]->NumAllocatedFaces);
	}
	Mat1->addNumFaces(Mat2->NumAllocatedFaces);

	//reset mat2
	rRemoveMaterial(Mat2);
	*Mat2 = gMaterial();

	return Mat1;
}

void rRemoveMaterials(Common::Array<Common::SharedPtr<gMaterial>> &m) {
	for (auto &material : m) {
		m.clear();
	}
}

Common::SharedPtr<gMaterial> rCopyMaterial(Common::SharedPtr<gMaterial> Mat1, Common::SharedPtr<gMaterial> Mat2) {
	int i;

	if (!Mat1 || !Mat2)
		return nullptr;

	Mat1->FacesList.clear();
	Mat1->AddictionalMaterial.clear();
	Mat1->FacesList.clear();
	delete[] Mat1->VertsList;
	Mat1->VertsList = nullptr;
	//t3dFree(Mat1->FlagsList);
	*Mat1 = gMaterial();

	if (Mat2->NumAllocatedFaces) {
		Mat1->addNumFaces(Mat2->NumAllocatedFaces);
		for (int i = 0; i < Mat2->NumAllocatedFaces; i++) {
			Mat1->FacesList.push_back(Mat2->FacesList[i]);
		}
	}
	if (Mat2->NumAllocatedVerts) {
		Mat1->VertsList = new gVertex*[Mat2->NumAllocatedVerts]{};
		memcpy(Mat1->VertsList, Mat2->VertsList, sizeof(gVertex *)*Mat2->NumAllocatedVerts);
	}
	if (Mat2->NumAllocatedMesh) {
		Mat1->FlagsList = Mat2->FlagsList;
	}

	Mat1->Texture = Mat2->Texture;
	Mat1->Movie = Mat2->Movie;
	Mat1->Flags = Mat2->Flags;
	Mat1->NumFaces = Mat2->NumFaces;
	Mat1->NumAllocatedFaces = Mat2->NumAllocatedFaces;
	Mat1->NumAllocatedVerts = Mat2->NumAllocatedVerts;
	Mat1->VBO = Mat2->VBO;
	Mat1->NumAllocatedMesh = Mat2->NumAllocatedMesh;
	Mat1->r = Mat2->r;
	Mat1->g = Mat2->g;
	Mat1->b = Mat2->b;
	Mat1->NumAddictionalMaterial = Mat2->NumAddictionalMaterial;

	if (Mat2->NumAddictionalMaterial)
		Mat1->AddictionalMaterial.resize(Mat2->NumAddictionalMaterial); // TODO: Does this mean that we don't copy any extras?

	for (i = 0; i < Mat2->NumAddictionalMaterial; i++)
		rCopyMaterial(Mat1->AddictionalMaterial[i], Mat2->AddictionalMaterial[i]);

	return Mat1;
}

void gMaterial::clear() {
	if (Movie)
	{
		Movie = nullptr;
	}
	FacesList.clear();
	delete[] VertsList;
	VertsList=nullptr;
	FlagsList.clear();
// 	rDeleteVertexBuffer(m->VB);
	VBO=0;

	for ( int j=0; j<NumAddictionalMaterial; j++)
	{
		Common::SharedPtr<gMaterial> cm = AddictionalMaterial[j];
		cm->FacesList.clear();
		delete[] cm->VertsList;
		cm->VertsList=nullptr;
		cm->FlagsList.clear();
// 		rDeleteVertexBuffer(cm->VB);
		cm->VBO=0;
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
	//warning("Stubbed: rAddToMaterialList");
	//D3DVERTEXBUFFERDESC VBDesc;
	gMaterial *cm;
	gBatchBlock *bb;
	int j;
#if 0
	if (!mat)
		return;

	if ((mat.Flags & T3D_MATERIAL_MOVIE))
		gUpdateMovie(mat);
#endif
	if ((mat.NumFaces >= 3) && (mat.VBO)) {
		bb = rNewBatchBlock(mat.Texture->ID, mat.Flags, 0, 0);
		bb->ViewMatrixNum = ViewMatrixNum;
		bb->NumFaces = (unsigned short int) mat.NumFaces;
		bb->FacesList = mat.FacesList;
		bb->VBO = mat.VBO;
		bb->NumVerts = (unsigned short int) mat.NumAllocatedVerts;
		mat.NumFaces = 0;
//		if ( bb->VB == g_lpD3DUserVertexBuffer )
//			DebugLogFile("User VB %s with %d verts",mat->Texture->Name,bb->NumVerts);
#if 0
		if ((bb->NumVerts == 0) && (bb->VBO)) {
			if (bb->VB->GetVertexBufferDesc(&VBDesc) != D3D_OK)
				DebugLogFile("Can't get VB information for %s", mat->Texture->Name);
			else
				bb->NumVerts = (unsigned short int) VBDesc.dwNumVertices;
//			DebugLogFile("Saving VB %s with %d verts",mat->Texture->Name,bb->NumVerts);
		}
#endif
	}

	for (auto &cm : mat.AddictionalMaterial) {
		if (cm->NumFaces < 3) continue;
		if (cm->VBO == NULL) continue;
		bb = rNewBatchBlock(mat.Texture->ID, mat.Flags, cm->Texture->ID, cm->Flags);
		bb->ViewMatrixNum = ViewMatrixNum;
		bb->NumFaces = (unsigned short int) cm->NumFaces;
		bb->FacesList = cm->FacesList;
		bb->VBO = cm->VBO;
		bb->NumVerts = (unsigned short int) cm->NumAllocatedVerts;
		cm->NumFaces = 0;
		if (bb->NumVerts == 0) {
#if 0
			if (bb->VBO->GetVertexBufferDesc(&VBDesc) != D3D_OK)
				DebugLogFile("Can't get VB information for %s", mat->Texture->Name);
			else
				bb->NumVerts = (unsigned short int) VBDesc.dwNumVertices;
//			DebugLogFile("Saving VB %s with %d verts",mat->Texture->Name,bb->NumVerts);
#endif
		}
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

	for (auto &mat: MatList) {
		rAddToMaterialList(mat, ViewMatrixNum);
	}
}

MaterialTable rCreateMaterialList(int num) {
	MaterialTable list;
	list.reserve(num);
	for (int i = 0; i < num; i++) {
		list.push_back(Common::SharedPtr<gMaterial>(new gMaterial()));
	}
	return list;
}

} // End of namespace Watchmaker
