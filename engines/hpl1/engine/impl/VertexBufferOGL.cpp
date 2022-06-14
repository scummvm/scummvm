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
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hpl1/engine/impl/VertexBufferOGL.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/LowLevelSystem.h"

//#include <GL/GLee.h>

#include <string.h>

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cVertexBufferOGL::cVertexBufferOGL(iLowLevelGraphics *apLowLevelGraphics, tVertexFlag aFlags,
								   eVertexBufferDrawType aDrawType, eVertexBufferUsageType aUsageType,
								   int alReserveVtxSize, int alReserveIdxSize) : iVertexBuffer(apLowLevelGraphics, aFlags, aDrawType, aUsageType, alReserveVtxSize, alReserveIdxSize) {
	if (alReserveVtxSize > 0) {
		for (int i = 0; i < klNumOfVertexFlags; i++) {
			if (aFlags & kvVertexFlags[i]) {
				mvVertexArray[i].reserve(alReserveVtxSize * kvVertexElements[i]);
			}
		}
	}

	if (alReserveIdxSize > 0)
		mvIndexArray.reserve(alReserveIdxSize);

	mbTangents = false;

	mbHasShadowDouble = false;

	mpLowLevelGraphics = apLowLevelGraphics;
}

cVertexBufferOGL::~cVertexBufferOGL() {
	for (int i = 0; i < klNumOfVertexFlags; i++)
		mvVertexArray[i].clear();

	mvIndexArray.clear();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cVertexBufferOGL::AddVertex(tVertexFlag aType, const cVector3f &avVtx) {
	int idx = cMath::Log2ToInt((int)aType);

	mvVertexArray[idx].push_back(avVtx.x);
	mvVertexArray[idx].push_back(avVtx.y);
	mvVertexArray[idx].push_back(avVtx.z);
	if (kvVertexElements[idx] == 4)
		mvVertexArray[idx].push_back(1);
}

//-----------------------------------------------------------------------

void cVertexBufferOGL::AddColor(tVertexFlag aType, const cColor &aColor) {
	int idx = cMath::Log2ToInt((int)aType);

	mvVertexArray[idx].push_back(aColor.r);
	mvVertexArray[idx].push_back(aColor.g);
	mvVertexArray[idx].push_back(aColor.b);
	mvVertexArray[idx].push_back(aColor.a);
}

//-----------------------------------------------------------------------

void cVertexBufferOGL::AddIndex(unsigned int alIndex) {
	mvIndexArray.push_back(alIndex);
}

//-----------------------------------------------------------------------

bool cVertexBufferOGL::Compile(tVertexCompileFlag aFlags) {
	if (aFlags & eVertexCompileFlag_CreateTangents) {
		mbTangents = true;

		mVertexFlags |= eVertexFlag_Texture1;

		int idx = cMath::Log2ToInt((int)eVertexFlag_Texture1);

		int lSize = GetVertexNum() * 4;
		mvVertexArray[idx].resize(lSize);

		cMath::CreateTriTangentVectors(&(mvVertexArray[cMath::Log2ToInt((int)eVertexFlag_Texture1)][0]),
									   &mvIndexArray[0], GetIndexNum(),

									   &(mvVertexArray[cMath::Log2ToInt((int)eVertexFlag_Position)][0]),
									   kvVertexElements[cMath::Log2ToInt((int)eVertexFlag_Position)],

									   &(mvVertexArray[cMath::Log2ToInt((int)eVertexFlag_Texture0)][0]),
									   &(mvVertexArray[cMath::Log2ToInt((int)eVertexFlag_Normal)][0]),
									   GetVertexNum());
	}

	return true;
}

//-----------------------------------------------------------------------

void cVertexBufferOGL::UpdateData(tVertexFlag aTypes, bool abIndices) {
}

//-----------------------------------------------------------------------

void cVertexBufferOGL::CreateShadowDouble(bool abUpdateData) {
	int lIdx = cMath::Log2ToInt(eVertexFlag_Position);

	// Set to new size.
	int lSize = (int)mvVertexArray[lIdx].size();
	mvVertexArray[lIdx].reserve(lSize * 2);

	int lCount = lSize / 4;
	for (int i = 0; i < lCount; i++) {
		mvVertexArray[lIdx].push_back(mvVertexArray[lIdx][i * 4 + 0]);
		mvVertexArray[lIdx].push_back(mvVertexArray[lIdx][i * 4 + 1]);
		mvVertexArray[lIdx].push_back(mvVertexArray[lIdx][i * 4 + 2]);
		mvVertexArray[lIdx].push_back(0); // 0);
	}

	mbHasShadowDouble = true;

	if (abUpdateData) {
		UpdateData(eVertexFlag_Position, false);
	}
}

//-----------------------------------------------------------------------

void cVertexBufferOGL::Transform(const cMatrixf &a_mtxTransform) {
	float *pPosArray = GetArray(eVertexFlag_Position);
	float *pNormalArray = GetArray(eVertexFlag_Normal);
	float *pTangentArray = NULL;
	if (mbTangents)
		pTangentArray = GetArray(eVertexFlag_Texture1);

	int lVtxNum = GetVertexNum();

	cMatrixf mtxRot = a_mtxTransform.GetRotation();

	int lVtxStride = kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)];

	int lOffset = GetVertexNum() * 4;

	for (int i = 0; i < lVtxNum; i++) {
		float *pPos = &pPosArray[i * lVtxStride];
		float *pNorm = &pNormalArray[i * 3];
		float *pTan = NULL;
		if (mbTangents)
			pTan = &pTangentArray[i * 4];

		cVector3f vPos = cMath::MatrixMul(a_mtxTransform, cVector3f(pPos[0], pPos[1], pPos[2]));
		pPos[0] = vPos.x;
		pPos[1] = vPos.y;
		pPos[2] = vPos.z;

		if (mbHasShadowDouble) {
			float *pExtraPos = &pPosArray[i * lVtxStride + lOffset];
			pExtraPos[0] = vPos.x;
			pExtraPos[1] = vPos.y;
			pExtraPos[2] = vPos.z;
		}

		cVector3f vNorm = cMath::MatrixMul(mtxRot, cVector3f(pNorm[0], pNorm[1], pNorm[2]));
		vNorm.Normalise();
		pNorm[0] = vNorm.x;
		pNorm[1] = vNorm.y;
		pNorm[2] = vNorm.z;

		if (mbTangents) {
			cVector3f vTan = cMath::MatrixMul(mtxRot, cVector3f(pTan[0], pTan[1], pTan[2]));
			vTan.Normalise();
			pTan[0] = vTan.x;
			pTan[1] = vTan.y;
			pTan[2] = vTan.z;
		}
	}

	if (mbTangents)
		UpdateData(eVertexFlag_Position | eVertexFlag_Normal | eVertexFlag_Texture1, false);
	else
		UpdateData(eVertexFlag_Position | eVertexFlag_Normal, false);
}

//-----------------------------------------------------------------------

void cVertexBufferOGL::Draw(eVertexBufferDrawType aDrawType) {
#if 0
  		eVertexBufferDrawType drawType = aDrawType == eVertexBufferDrawType_LastEnum ? mDrawType : aDrawType;

		///////////////////////////////
		//Get the draw type
		GLenum mode = GL_TRIANGLES;
		if(drawType==eVertexBufferDrawType_Quad)		mode = GL_QUADS;
		else if(drawType==eVertexBufferDrawType_Lines)	mode = GL_LINE_STRIP;


		int lSize = mlElementNum;
		if(mlElementNum<0) lSize = GetIndexNum();

		glDrawElements(mode,lSize,GL_UNSIGNED_INT, &mvIndexArray[0]);
#endif
}

void cVertexBufferOGL::DrawIndices(unsigned int *apIndices, int alCount,
								   eVertexBufferDrawType aDrawType) {
#if 0
  		eVertexBufferDrawType drawType = aDrawType == eVertexBufferDrawType_LastEnum ? mDrawType : aDrawType;

		///////////////////////////////
		//Get the draw type
		GLenum mode = GL_TRIANGLES;
		if(drawType==eVertexBufferDrawType_Quad)		mode = GL_QUADS;
		else if(drawType==eVertexBufferDrawType_Lines)	mode = GL_LINE_STRIP;

		//////////////////////////////////
		//Bind and draw the buffer
		glDrawElements(mode, alCount, GL_UNSIGNED_INT, apIndices);
#endif
}

//-----------------------------------------------------------------------

void cVertexBufferOGL::Bind() {
	SetVertexStates(mVertexFlags);
}

//-----------------------------------------------------------------------

void cVertexBufferOGL::UnBind() {
}

//-----------------------------------------------------------------------

iVertexBuffer *cVertexBufferOGL::CreateCopy(eVertexBufferUsageType aUsageType) {
	cVertexBufferOGL *pVtxBuff = hplNew(cVertexBufferOGL, (mpLowLevelGraphics,
														   mVertexFlags, mDrawType, aUsageType,
														   GetVertexNum(), GetIndexNum()));

	// Copy the vertices to the new buffer.
	for (int i = 0; i < klNumOfVertexFlags; i++) {
		if (kvVertexFlags[i] & mVertexFlags) {
			int lElements = kvVertexElements[i];
			if (mbTangents && kvVertexFlags[i] == eVertexFlag_Texture1)
				lElements = 4;

			pVtxBuff->ResizeArray(kvVertexFlags[i], (int)mvVertexArray[i].size());

			memcpy(pVtxBuff->GetArray(kvVertexFlags[i]),
				   &mvVertexArray[i][0], mvVertexArray[i].size() * sizeof(float));
		}
	}

	// Copy indices to the new buffer
	pVtxBuff->ResizeIndices(GetIndexNum());
	memcpy(pVtxBuff->GetIndices(), GetIndices(), GetIndexNum() * sizeof(unsigned int));

	pVtxBuff->mbTangents = mbTangents;
	pVtxBuff->mbHasShadowDouble = mbHasShadowDouble;

	pVtxBuff->Compile(0);

	return pVtxBuff;
}

//-----------------------------------------------------------------------

cBoundingVolume cVertexBufferOGL::CreateBoundingVolume() {
	cBoundingVolume bv;

	int lNum = cMath::Log2ToInt((int)eVertexFlag_Position);

	bv.AddArrayPoints(&(mvVertexArray[lNum][0]), GetVertexNum());
	bv.CreateFromPoints(kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)]);

	return bv;
}

float *cVertexBufferOGL::GetArray(tVertexFlag aType) {
	int idx = cMath::Log2ToInt((int)aType);

	return &mvVertexArray[idx][0];
}

//-----------------------------------------------------------------------

unsigned int *cVertexBufferOGL::GetIndices() {
	return &mvIndexArray[0];
}

//-----------------------------------------------------------------------

void cVertexBufferOGL::ResizeArray(tVertexFlag aType, int alSize) {
	int idx = cMath::Log2ToInt((int)aType);

	mvVertexArray[idx].resize(alSize);
}

//-----------------------------------------------------------------------

void cVertexBufferOGL::ResizeIndices(int alSize) {
	mvIndexArray.resize(alSize);
}

//-----------------------------------------------------------------------

int cVertexBufferOGL::GetVertexNum() {
	int idx = cMath::Log2ToInt((int)eVertexFlag_Position);
	int lSize = (int)mvVertexArray[idx].size() / kvVertexElements[idx];

	// If there is a shadow double, just return the length of the first half.
	if (mbHasShadowDouble)
		return lSize / 2;
	else
		return lSize;
}

int cVertexBufferOGL::GetIndexNum() {
	return (int)mvIndexArray.size();
}

cVector3f cVertexBufferOGL::GetVector3(tVertexFlag aType, unsigned alIdx) {
	if (!(aType & mVertexFlags))
		return cVector3f(0, 0, 0);

	int idx = cMath::Log2ToInt((int)aType);
	int pos = alIdx * kvVertexElements[idx];

	return cVector3f(mvVertexArray[idx][pos + 0], mvVertexArray[idx][pos + 1],
					 mvVertexArray[idx][pos + 2]);
}
cVector3f cVertexBufferOGL::GetVector4(tVertexFlag aType, unsigned alIdx) {
	if (!(aType & mVertexFlags))
		return cVector3f(0, 0, 0);

	int idx = cMath::Log2ToInt((int)aType);
	int pos = alIdx * 4; // kvVertexElements[idx];

	return cVector3f(mvVertexArray[idx][pos + 0], mvVertexArray[idx][pos + 1],
					 mvVertexArray[idx][pos + 2]);
}
cColor cVertexBufferOGL::GetColor(tVertexFlag aType, unsigned alIdx) {
	if (!(aType & mVertexFlags))
		return cColor();

	int idx = cMath::Log2ToInt((int)aType);
	int pos = alIdx * kvVertexElements[idx];

	return cColor(mvVertexArray[idx][pos + 0], mvVertexArray[idx][pos + 1],
				  mvVertexArray[idx][pos + 2], mvVertexArray[idx][pos + 3]);
}
unsigned int cVertexBufferOGL::GetIndex(tVertexFlag aType, unsigned alIdx) {
	return mvIndexArray[alIdx];
}

//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
/////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cVertexBufferOGL::SetVertexStates(tVertexFlag aFlags) {
#if 0
  		/// POSITION /////////////////////////
		if(aFlags & eVertexFlag_Position){
			glEnableClientState(GL_VERTEX_ARRAY );
			int idx = cMath::Log2ToInt(eVertexFlag_Position);
			glVertexPointer(kvVertexElements[idx],GL_FLOAT, sizeof(float)*kvVertexElements[idx], &mvVertexArray[idx][0]);
		}
		else
		{
			glDisableClientState(GL_VERTEX_ARRAY );
		}

		/// COLOR 0 /////////////////////////
		if(aFlags & eVertexFlag_Color0)
		{
			glEnableClientState(GL_COLOR_ARRAY );
			int idx = cMath::Log2ToInt(eVertexFlag_Color0);
			glColorPointer(kvVertexElements[idx],GL_FLOAT, sizeof(float)*kvVertexElements[idx], &mvVertexArray[idx][0]);
		}
		else
		{
			glDisableClientState(GL_COLOR_ARRAY );
		}

		/// NORMAL /////////////////////////
		if(aFlags & eVertexFlag_Normal)
		{
			glEnableClientState(GL_NORMAL_ARRAY );
			glNormalPointer(GL_FLOAT, sizeof(float)*3, &mvVertexArray[cMath::Log2ToInt(eVertexFlag_Normal)][0]);
		}
		else
		{
			glDisableClientState(GL_NORMAL_ARRAY );
		}

		/// TEXTURE 0 /////////////////////////
		if(aFlags & eVertexFlag_Texture0)
		{
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY );
			int idx =  cMath::Log2ToInt(eVertexFlag_Texture0);
			glTexCoordPointer(kvVertexElements[idx],GL_FLOAT,sizeof(float)*kvVertexElements[idx],&mvVertexArray[idx][0] );
		}
		else {
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY );
		}

		/// TEXTURE 1 /////////////////////////
		if(aFlags & eVertexFlag_Texture1){
			glClientActiveTextureARB(GL_TEXTURE1_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY );
			int idx =  cMath::Log2ToInt(eVertexFlag_Texture1);

			if(mbTangents)
				glTexCoordPointer(4,GL_FLOAT,sizeof(float)*4,&mvVertexArray[idx][0] );
			else
				glTexCoordPointer(kvVertexElements[idx],GL_FLOAT,sizeof(float)*kvVertexElements[idx],&mvVertexArray[idx][0] );
		}
		else {
			glClientActiveTextureARB(GL_TEXTURE1_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY );
		}

		/// TEXTURE 2 /////////////////////////
		if(aFlags & eVertexFlag_Texture2){
			glClientActiveTextureARB(GL_TEXTURE2_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY );
			int idx =  cMath::Log2ToInt(eVertexFlag_Texture2);
			glTexCoordPointer(kvVertexElements[idx],GL_FLOAT,sizeof(float)*kvVertexElements[idx],&mvVertexArray[idx][0] );
		}
		else {
			glClientActiveTextureARB(GL_TEXTURE2_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY );
		}

		/// TEXTURE 3 /////////////////////////
		if(aFlags & eVertexFlag_Texture3){
			glClientActiveTextureARB(GL_TEXTURE3_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY );
			int idx =  cMath::Log2ToInt(eVertexFlag_Texture3);
			glTexCoordPointer(kvVertexElements[idx],GL_FLOAT,sizeof(float)*kvVertexElements[idx],&mvVertexArray[idx][0] );
		}
		else {
			glClientActiveTextureARB(GL_TEXTURE3_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY );
		}

		/// TEXTURE 4 /////////////////////////
		if(aFlags & eVertexFlag_Texture4){
			glClientActiveTextureARB(GL_TEXTURE4_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY );
			int idx =  cMath::Log2ToInt(eVertexFlag_Texture4);
			glTexCoordPointer(kvVertexElements[idx],GL_FLOAT,sizeof(float)*kvVertexElements[idx],&mvVertexArray[idx][0] );
		}
		else {
			glClientActiveTextureARB(GL_TEXTURE4_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY );
		}
#endif
}

//-----------------------------------------------------------------------

} // namespace hpl
