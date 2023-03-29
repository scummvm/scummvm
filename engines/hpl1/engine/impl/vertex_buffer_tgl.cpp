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

#include "hpl1/engine/impl/vertex_buffer_tgl.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/low_level_system.h"

#include "graphics/tinygl/tinygl.h"
#include "hpl1/debug.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

VertexBufferTGL::VertexBufferTGL(iLowLevelGraphics *apLowLevelGraphics, tVertexFlag aFlags,
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

VertexBufferTGL::~VertexBufferTGL() {
	for (int i = 0; i < klNumOfVertexFlags; i++)
		mvVertexArray[i].clear();

	mvIndexArray.clear();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void VertexBufferTGL::AddVertex(tVertexFlag aType, const cVector3f &avVtx) {
	int idx = cMath::Log2ToInt((int)aType);

	mvVertexArray[idx].push_back(avVtx.x);
	mvVertexArray[idx].push_back(avVtx.y);
	mvVertexArray[idx].push_back(avVtx.z);
	if (kvVertexElements[idx] == 4)
		mvVertexArray[idx].push_back(1);
}

//-----------------------------------------------------------------------

void VertexBufferTGL::AddColor(tVertexFlag aType, const cColor &aColor) {
	int idx = cMath::Log2ToInt((int)aType);

	mvVertexArray[idx].push_back(aColor.r);
	mvVertexArray[idx].push_back(aColor.g);
	mvVertexArray[idx].push_back(aColor.b);
	mvVertexArray[idx].push_back(aColor.a);
}

//-----------------------------------------------------------------------

void VertexBufferTGL::AddIndex(unsigned int alIndex) {
	mvIndexArray.push_back(alIndex);
}

//-----------------------------------------------------------------------

bool VertexBufferTGL::Compile(tVertexCompileFlag aFlags) {
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

void VertexBufferTGL::UpdateData(tVertexFlag aTypes, bool abIndices) {
}

//-----------------------------------------------------------------------

void VertexBufferTGL::CreateShadowDouble(bool abUpdateData) {
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

void VertexBufferTGL::Transform(const cMatrixf &a_mtxTransform) {
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

void VertexBufferTGL::Draw(eVertexBufferDrawType aDrawType) {
	eVertexBufferDrawType drawType = aDrawType == eVertexBufferDrawType_LastEnum ? mDrawType : aDrawType;

	///////////////////////////////
	// Get the draw type
	TGLenum mode = TGL_TRIANGLES;
	if (drawType == eVertexBufferDrawType_Quad)
		mode = TGL_QUADS;
	else if (drawType == eVertexBufferDrawType_Lines)
		mode = TGL_LINE_STRIP;

	int lSize = mlElementNum;
	if (mlElementNum < 0)
		lSize = GetIndexNum();

	tglDrawElements(mode, lSize, TGL_UNSIGNED_INT, &mvIndexArray[0]);
}

void VertexBufferTGL::DrawIndices(unsigned int *apIndices, int alCount,
								  eVertexBufferDrawType aDrawType) {
	eVertexBufferDrawType drawType = aDrawType == eVertexBufferDrawType_LastEnum ? mDrawType : aDrawType;

	///////////////////////////////
	// Get the draw type
	TGLenum mode = TGL_TRIANGLES;
	if (drawType == eVertexBufferDrawType_Quad)
		mode = TGL_QUADS;
	else if (drawType == eVertexBufferDrawType_Lines)
		mode = TGL_LINE_STRIP;

	//////////////////////////////////
	// Bind and draw the buffer
	tglDrawElements(mode, alCount, TGL_UNSIGNED_INT, apIndices);
}

//-----------------------------------------------------------------------

void VertexBufferTGL::Bind() {
	SetVertexStates(mVertexFlags);
}

//-----------------------------------------------------------------------

void VertexBufferTGL::UnBind() {
}

//-----------------------------------------------------------------------

iVertexBuffer *VertexBufferTGL::CreateCopy(eVertexBufferUsageType aUsageType) {
	VertexBufferTGL *pVtxBuff = hplNew(VertexBufferTGL, (mpLowLevelGraphics,
														 mVertexFlags, mDrawType, aUsageType,
														 GetVertexNum(), GetIndexNum()));

	// Copy the vertices to the new buffer.
	for (int i = 0; i < klNumOfVertexFlags; i++) {
		if (kvVertexFlags[i] & mVertexFlags) {
#if 0
			int lElements = kvVertexElements[i];
			if (mbTangents && kvVertexFlags[i] == eVertexFlag_Texture1)
				lElements = 4;
#endif

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

cBoundingVolume VertexBufferTGL::CreateBoundingVolume() {
	cBoundingVolume bv;

	int lNum = cMath::Log2ToInt((int)eVertexFlag_Position);

	bv.AddArrayPoints(&(mvVertexArray[lNum][0]), GetVertexNum());
	bv.CreateFromPoints(kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)]);

	return bv;
}

float *VertexBufferTGL::GetArray(tVertexFlag aType) {
	int idx = cMath::Log2ToInt((int)aType);

	return &mvVertexArray[idx][0];
}

//-----------------------------------------------------------------------

unsigned int *VertexBufferTGL::GetIndices() {
	return &mvIndexArray[0];
}

//-----------------------------------------------------------------------

void VertexBufferTGL::ResizeArray(tVertexFlag aType, int alSize) {
	int idx = cMath::Log2ToInt((int)aType);

	mvVertexArray[idx].resize(alSize);
}

//-----------------------------------------------------------------------

void VertexBufferTGL::ResizeIndices(int alSize) {
	mvIndexArray.resize(alSize);
}

//-----------------------------------------------------------------------

int VertexBufferTGL::GetVertexNum() {
	int idx = cMath::Log2ToInt((int)eVertexFlag_Position);
	int lSize = (int)mvVertexArray[idx].size() / kvVertexElements[idx];

	// If there is a shadow double, just return the length of the first half.
	if (mbHasShadowDouble)
		return lSize / 2;
	else
		return lSize;
}

int VertexBufferTGL::GetIndexNum() {
	return (int)mvIndexArray.size();
}

cVector3f VertexBufferTGL::GetVector3(tVertexFlag aType, unsigned alIdx) {
	if (!(aType & mVertexFlags))
		return cVector3f(0, 0, 0);

	int idx = cMath::Log2ToInt((int)aType);
	int pos = alIdx * kvVertexElements[idx];

	return cVector3f(mvVertexArray[idx][pos + 0], mvVertexArray[idx][pos + 1],
					 mvVertexArray[idx][pos + 2]);
}
cVector3f VertexBufferTGL::GetVector4(tVertexFlag aType, unsigned alIdx) {
	if (!(aType & mVertexFlags))
		return cVector3f(0, 0, 0);

	int idx = cMath::Log2ToInt((int)aType);
	int pos = alIdx * 4; // kvVertexElements[idx];

	return cVector3f(mvVertexArray[idx][pos + 0], mvVertexArray[idx][pos + 1],
					 mvVertexArray[idx][pos + 2]);
}
cColor VertexBufferTGL::GetColor(tVertexFlag aType, unsigned alIdx) {
	if (!(aType & mVertexFlags))
		return cColor();

	int idx = cMath::Log2ToInt((int)aType);
	int pos = alIdx * kvVertexElements[idx];

	return cColor(mvVertexArray[idx][pos + 0], mvVertexArray[idx][pos + 1],
				  mvVertexArray[idx][pos + 2], mvVertexArray[idx][pos + 3]);
}
unsigned int VertexBufferTGL::GetIndex(tVertexFlag aType, unsigned alIdx) {
	return mvIndexArray[alIdx];
}

//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
/////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void VertexBufferTGL::SetVertexStates(tVertexFlag aFlags) {
	/// POSITION /////////////////////////
	if (aFlags & eVertexFlag_Position) {
		tglEnableClientState(TGL_VERTEX_ARRAY);
		int idx = cMath::Log2ToInt(eVertexFlag_Position);
		tglVertexPointer(kvVertexElements[idx], TGL_FLOAT, sizeof(float) * kvVertexElements[idx], &mvVertexArray[idx][0]);
	} else {
		tglDisableClientState(TGL_VERTEX_ARRAY);
	}

	/// COLOR 0 /////////////////////////
	if (aFlags & eVertexFlag_Color0) {
		tglEnableClientState(TGL_COLOR_ARRAY);
		int idx = cMath::Log2ToInt(eVertexFlag_Color0);
		tglColorPointer(kvVertexElements[idx], TGL_FLOAT, sizeof(float) * kvVertexElements[idx], &mvVertexArray[idx][0]);
	} else {
		tglDisableClientState(TGL_COLOR_ARRAY);
	}

	/// NORMAL /////////////////////////
	if (aFlags & eVertexFlag_Normal) {
		tglEnableClientState(TGL_NORMAL_ARRAY);
		tglNormalPointer(TGL_FLOAT, sizeof(float) * 3, &mvVertexArray[cMath::Log2ToInt(eVertexFlag_Normal)][0]);
	} else {
		tglDisableClientState(TGL_NORMAL_ARRAY);
	}

	/// TEXTURE 0 /////////////////////////
	if (aFlags & eVertexFlag_Texture0) {
		tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
		int idx = cMath::Log2ToInt(eVertexFlag_Texture0);
		tglTexCoordPointer(kvVertexElements[idx], TGL_FLOAT, sizeof(float) * kvVertexElements[idx], &mvVertexArray[idx][0]);
	} else {
		tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
	}
#if 0
		/// TEXTURE 1 /////////////////////////
		if(aFlags & eVertexFlag_Texture1){
			tglClientActiveTextureARB(TGL_TEXTURE1_ARB);
			tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
			int idx =  cMath::Log2ToInt(eVertexFlag_Texture1);

			if(mbTangents)
				tglTexCoordPointer(4,TGL_FLOAT,sizeof(float)*4,&mvVertexArray[idx][0]);
			else
				tglTexCoordPointer(kvVertexElements[idx],TGL_FLOAT,sizeof(float)*kvVertexElements[idx],&mvVertexArray[idx][0]);
		}
		else {
			//tglClientActiveTextureARB(TGL_TEXTURE1_ARB);
			//tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
		}

		/// TEXTURE 2 /////////////////////////
		if(aFlags & eVertexFlag_Texture2){
			tglClientActiveTextureARB(TGL_TEXTURE2_ARB);
			tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
			int idx =  cMath::Log2ToInt(eVertexFlag_Texture2);
			tglTexCoordPointer(kvVertexElements[idx],TGL_FLOAT,sizeof(float)*kvVertexElements[idx],&mvVertexArray[idx][0]);
		}
		else {
			//tglClientActiveTextureARB(TGL_TEXTURE2_ARB);
			//tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
		}

		/// TEXTURE 3 /////////////////////////
		if(aFlags & eVertexFlag_Texture3){
			tglClientActiveTextureARB(TGL_TEXTURE3_ARB);
			tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
			int idx =  cMath::Log2ToInt(eVertexFlag_Texture3);
			tglTexCoordPointer(kvVertexElements[idx],TGL_FLOAT,sizeof(float)*kvVertexElements[idx],&mvVertexArray[idx][0]);
		}
		else {
			//tglClientActiveTextureARB(TGL_TEXTURE3_ARB);
			//tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
		}

		/// TEXTURE 4 /////////////////////////
		if(aFlags & eVertexFlag_Texture4){
			tglClientActiveTextureARB(TGL_TEXTURE4_ARB);
			tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
			int idx =  cMath::Log2ToInt(eVertexFlag_Texture4);
			tglTexCoordPointer(kvVertexElements[idx],TGL_FLOAT,sizeof(float)*kvVertexElements[idx],&mvVertexArray[idx][0]);
		}
		else {
			//tglClientActiveTextureARB(TGL_TEXTURE4_ARB);
			//tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
		}
#endif
}

//-----------------------------------------------------------------------

} // namespace hpl