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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#include "hpl1/engine/impl/MeshLoaderMSH.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/Mesh.h"
#include "hpl1/engine/graphics/SubMesh.h"
#include "hpl1/engine/resources/MaterialManager.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"

#include "hpl1/engine/math/Math.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMeshLoaderMSH::cMeshLoaderMSH(iLowLevelGraphics *apLowLevelGraphics) : iMeshLoader(apLowLevelGraphics) {
}

//-----------------------------------------------------------------------

cMeshLoaderMSH::~cMeshLoaderMSH() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

// Definition to make the error exit not so bloated.
#define ExitLoad()      \
	hplDelete(pMesh);   \
	hplDelete(pXmlDoc); \
	return NULL;

cWorld3D *cMeshLoaderMSH::LoadWorld(const tString &asFile, cScene *apScene, tWorldLoadFlag aFlags) {
	return NULL;
}

//-----------------------------------------------------------------------

cMesh *cMeshLoaderMSH::LoadMesh(const tString &asFile, tMeshLoadFlag aFlags) {
	cMesh *pMesh = hplNew(cMesh, (cString::GetFileName(asFile), mpMaterialManager, mpAnimationManager));
	// If the mesh is animated there are some property differences, the vertex buffer
	// Must be Stream instead of static for example.
	bool bIsAnimated = false;

	/////////////////////////////////////////////////
	// LOAD THE DOCUMENT
	TiXmlDocument *pXmlDoc = hplNew(TiXmlDocument, (asFile.c_str()));
	if (pXmlDoc->LoadFile() == false) {
		Error("Couldn't load XML file '%s'!\n", asFile.c_str());
		ExitLoad();
	}

	// Get the root.
	TiXmlElement *pRootElem = pXmlDoc->RootElement();

	/////////////////////////////////////////////////
	// LOAD SUBMESHES
	// Load the root
	TiXmlElement *pSubMeshesRootElem = pRootElem->FirstChildElement("SubMeshes");
	if (pSubMeshesRootElem == NULL) {
		Error("No mesh data in XML file '%s'!\n", asFile.c_str());
		ExitLoad();
	}

	// Iterate through the sub meshes
	TiXmlElement *pSubMeshElem = pSubMeshesRootElem->FirstChildElement();
	while (pSubMeshElem) {
		//////////////////
		// Create sub mesh
		cSubMesh *pSubMesh = pMesh->CreateSubMesh(pSubMeshElem->Attribute("name"));

		//////////////////
		// Set material
		const char *pMatName = pSubMeshElem->Attribute("material");
		if (pMatName == NULL) {
			Error("No material found for mesh '%s'\n", asFile.c_str());
			ExitLoad();
		}

		iMaterial *pMaterial = mpMaterialManager->CreateMaterial(pMatName);
		pSubMesh->SetMaterial(pMaterial);

		////////////////////
		// Get the vertices
		TiXmlElement *pVtxElem = pSubMeshElem->FirstChildElement("Vertices");
		int lVtxSize = cString::ToInt(pVtxElem->Attribute("size"), 0);
		tVertexFlag vtxFlags = 0;
		bool bTangents = false;

		// Check what type of vertices are included.
		if (pVtxElem->FirstChild("Normal"))
			vtxFlags |= eVertexFlag_Normal;
		if (pVtxElem->FirstChild("Position"))
			vtxFlags |= eVertexFlag_Position;
		if (pVtxElem->FirstChild("Texture"))
			vtxFlags |= eVertexFlag_Texture0;
		if (pVtxElem->FirstChild("Color"))
			vtxFlags |= eVertexFlag_Color0;
		if (pVtxElem->FirstChild("Tangent")) {
			vtxFlags |= eVertexFlag_Texture1;
			bTangents = true;
		}

		// Create the vertex buffer
		eVertexBufferUsageType usageType = bIsAnimated ? eVertexBufferUsageType_Stream : eVertexBufferUsageType_Static;
		iVertexBuffer *pVtxBuff = mpLowLevelGraphics->CreateVertexBuffer(vtxFlags,
																		 eVertexBufferDrawType_Tri,
																		 usageType,
																		 0, 0);

		pVtxBuff->SetTangents(bTangents);

		// Fill the arrays
		for (int i = 0; i < klNumOfVertexFlags; i++) {
			if (kvVertexFlags[i] & vtxFlags) {
				int lElemPerVtx = 3;
				if (kvVertexFlags[i] & eVertexFlag_Texture1 || kvVertexFlags[i] & eVertexFlag_Color0) {
					lElemPerVtx = 4;
				}

				TiXmlElement *pElem = pVtxElem->FirstChildElement(GetVertexName(kvVertexFlags[i]));

				pVtxBuff->ResizeArray(kvVertexFlags[i], lVtxSize * lElemPerVtx);
				float *pArray = pVtxBuff->GetArray(kvVertexFlags[i]);

				// Log("TYPE: %s:\n",GetVertexName(kvVertexFlags[i]));
				FillVtxArray(pArray, pElem->Attribute("data"), lVtxSize * lElemPerVtx);
			}
		}

		////////////////////
		// Get Indices

		TiXmlElement *pIdxElem = pSubMeshElem->FirstChildElement("Indices");
		int lIdxSize = cString::ToInt(pIdxElem->Attribute("size"), 0);

		// Log("TYPE: Indices\n");
		pVtxBuff->ResizeIndices(lIdxSize);
		FillIdxArray(pVtxBuff->GetIndices(), pIdxElem->Attribute("data"), lIdxSize);

		///////////////////
		// Compile vertex buffer
		pVtxBuff->Compile(0);

		pSubMesh->SetVertexBuffer(pVtxBuff);

		/////////////////
		// Next element
		pSubMeshElem = pSubMeshesRootElem->NextSiblingElement();
	}

	/////////////////////////////////////////////////
	// LOAD SKELETON

	hplDelete(pXmlDoc);

	return pMesh;
}

//-----------------------------------------------------------------------

bool cMeshLoaderMSH::SaveMesh(cMesh *apMesh, const tString &asFile) {
	TiXmlDocument *pXmlDoc = hplNew(TiXmlDocument, (asFile.c_str()));

	// Root
	TiXmlElement XmlRoot("HPL_Mesh");
	TiXmlElement *pRootElem = static_cast<TiXmlElement *>(pXmlDoc->InsertEndChild(XmlRoot));

	// SubMeshes
	TiXmlElement XmlSubMeshes("SubMeshes");
	TiXmlElement *pSubMeshesElem = static_cast<TiXmlElement *>(pRootElem->InsertEndChild(XmlSubMeshes));

	// SubMesh
	for (int i = 0; i < apMesh->GetSubMeshNum(); i++) {
		cSubMesh *pSubMesh = apMesh->GetSubMesh(i);
		iVertexBuffer *pVtxBuff = pSubMesh->GetVertexBuffer();
		int lVtxSize = pVtxBuff->GetVertexNum();
		int lIdxSize = pVtxBuff->GetIndexNum();

		// Create element
		TiXmlElement XmlSubMesh("SubMesh");
		TiXmlElement *pSubMeshElem = static_cast<TiXmlElement *>(pSubMeshesElem->InsertEndChild(XmlSubMesh));

		// Set data
		pSubMeshElem->SetAttribute("name", pSubMesh->GetName().c_str());
		iMaterial *pMat = pSubMesh->GetMaterial();
		if (pMat)
			pSubMeshElem->SetAttribute("material", pMat->GetName().c_str());

		// Vertices
		TiXmlElement XmlVtx("Vertices");
		TiXmlElement *pVtxElem = static_cast<TiXmlElement *>(pSubMeshElem->InsertEndChild(XmlVtx));
		pVtxElem->SetAttribute("size", lVtxSize);

		for (int j = 0; j < klNumOfVertexFlags; j++) {
			if (kvVertexFlags[j] & pVtxBuff->GetVertexFlags()) {
				int lSizeMul = kvVertexElements[i];
				// Only support texture1 coordinate as tangent for now.
				if (kvVertexFlags[j] & eVertexFlag_Texture1)
					lSizeMul = 4;

				SaveFloatData(pVtxElem, lVtxSize * lSizeMul, GetVertexName(kvVertexFlags[j]),
							  pVtxBuff->GetArray(kvVertexFlags[j]));
			}
		}

		// Indices
		TiXmlElement XmlIdx("Indices");
		TiXmlElement *pIdxElem = static_cast<TiXmlElement *>(pSubMeshElem->InsertEndChild(XmlIdx));
		pIdxElem->SetAttribute("size", lIdxSize);
		SaveIntData(pIdxElem, lIdxSize, pVtxBuff->GetIndices());
	}

	bool bRet = pXmlDoc->SaveFile();
	if (bRet == false)
		Error("Couldn't save mesh to '%s'", asFile.c_str());

	hplDelete(pXmlDoc);
	return bRet;
}

//-----------------------------------------------------------------------

bool cMeshLoaderMSH::IsSupported(const tString asFileType) {
	if (asFileType == "msh")
		return true;

	return false;
}

//-----------------------------------------------------------------------

void cMeshLoaderMSH::AddSupportedTypes(tStringVec *avFileTypes) {
	avFileTypes->push_back("msh");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cMeshLoaderMSH::SaveFloatData(TiXmlElement *apRoot, int alSize, const char *asName, float *apData) {
	// Pos
	TiXmlElement XmlData(asName);
	TiXmlElement *pElem = static_cast<TiXmlElement *>(apRoot->InsertEndChild(XmlData));

	tString sData = "";
	char sTemp[20];

	for (int i = 0; i < alSize; i++) {
		float fNum = apData[i];
		int j;
		// get the number of decimals needed
		for (j = 6; j > 0; j--) {
			if (((int)(fNum * 10 * (float)j)) % 10 != 0) {
				break;
			}
		}
		int lDecimals = j;

		switch (lDecimals) {
		case 0:
			snprintf(sTemp, 20, "%.0f", apData[i]);
			break;
		case 1:
			snprintf(sTemp, 20, "%.1f", apData[i]);
			break;
		case 2:
			snprintf(sTemp, 20, "%.2f", apData[i]);
			break;
		case 3:
			snprintf(sTemp, 20, "%.3f", apData[i]);
			break;
		case 4:
			snprintf(sTemp, 20, "%.4f", apData[i]);
			break;
		case 5:
			snprintf(sTemp, 20, "%.5f", apData[i]);
			break;
		case 6:
			snprintf(sTemp, 20, "%.6f", apData[i]);
			break;
		}

		sData += sTemp;

		if (i != alSize - 1)
			sData += " ";
	}

	pElem->SetAttribute("data", sData.c_str());
}

//-----------------------------------------------------------------------

const char *cMeshLoaderMSH::GetVertexName(tVertexFlag aFlag) {
	switch (aFlag) {
	case eVertexFlag_Normal:
		return "Normal";
	case eVertexFlag_Position:
		return "Position";
	case eVertexFlag_Color0:
		return "Color";
	case eVertexFlag_Texture0:
		return "Texture";
	case eVertexFlag_Texture1:
		return "Tangent";
	}
	return "";
}

//-----------------------------------------------------------------------

void cMeshLoaderMSH::SaveIntData(TiXmlElement *apElem, int alSize, unsigned int *apData) {
	tString sData = "";
	char sTemp[10];

	for (int i = 0; i < alSize; i++) {
		snprintf(sTemp, 10, "%d", apData[i]);
		sData += sTemp;

		if (i != alSize - 1)
			sData += " ";
	}

	apElem->SetAttribute("data", sData.c_str());
}

//-----------------------------------------------------------------------

void cMeshLoaderMSH::FillVtxArray(float *apArray, const char *apString, int alSize) {
	if (apString == NULL) {
		Error("Data is NULL!\n");
		return;
	}

	char vTempChar[20];
	int lTempCharCount = 0;

	int lArrayCount = 0;
	int lStringCount = 0;

	while (lArrayCount < alSize) {
		char c = apString[lStringCount];
		// if a space is found, convert the previous characters to a float.
		if (c == ' ' || c == 0) {
			if (lTempCharCount > 0) {
				vTempChar[lTempCharCount] = 0;
				apArray[lArrayCount] = (float)atof(vTempChar);

				lTempCharCount = 0;
				lArrayCount++;
			}
		}
		// If character is not a space, add to temp char.
		else {
			vTempChar[lTempCharCount] = apString[lStringCount];
			lTempCharCount++;
		}

		lStringCount++;
	}
}

//-----------------------------------------------------------------------

void cMeshLoaderMSH::FillIdxArray(unsigned int *apArray, const char *apString, int alSize) {
	char vTempChar[20];
	int lTempCharCount = 0;

	int lArrayCount = 0;
	int lStringCount = 0;

	while (lArrayCount < alSize) {
		char c = apString[lStringCount];

		// if a space is found, convert the previous characters to a float.
		if (c == ' ' || c == 0) {
			if (lTempCharCount > 0) {
				vTempChar[lTempCharCount] = 0;
				apArray[lArrayCount] = (unsigned int)atoi(vTempChar);
				// Log("Adding: %d\n",apArray[lArrayCount]);

				lTempCharCount = 0;
				lArrayCount++;
			}
		}
		// If character is not a space, add to temp char.
		else {
			vTempChar[lTempCharCount] = c;
			lTempCharCount++;
		}

		lStringCount++;
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
