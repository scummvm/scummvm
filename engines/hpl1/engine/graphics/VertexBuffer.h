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

#ifndef HPL_VERTEXBUFFER_H
#define HPL_VERTEXBUFFER_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/BoundingVolume.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cBoundingVolume;

enum eVertexBufferDrawType {
	eVertexBufferDrawType_Tri,
	eVertexBufferDrawType_Quad,
	eVertexBufferDrawType_Lines,
	eVertexBufferDrawType_LastEnum
};

enum eVertexBufferUsageType {
	eVertexBufferUsageType_Static,
	eVertexBufferUsageType_Dynamic,
	eVertexBufferUsageType_Stream,
	eVertexBufferUsageType_LastEnum
};

typedef tFlag tVertexFlag;

#define eVertexFlag_Normal (0x00000001)
#define eVertexFlag_Position (0x00000002)
#define eVertexFlag_Color0 (0x00000004)
#define eVertexFlag_Color1 (0x00000008)
#define eVertexFlag_Texture0 (0x00000010)
#define eVertexFlag_Texture1 (0x00000020)
#define eVertexFlag_Texture2 (0x00000040)
#define eVertexFlag_Texture3 (0x00000080)
#define eVertexFlag_Texture4 (0x00000100)

#define klNumOfVertexFlags (9)

const tVertexFlag kvVertexFlags[] = {eVertexFlag_Normal, eVertexFlag_Position, eVertexFlag_Color0,
									 eVertexFlag_Color1, eVertexFlag_Texture0, eVertexFlag_Texture1, eVertexFlag_Texture2,
									 eVertexFlag_Texture3, eVertexFlag_Texture4};

const int kvVertexElements[] = {
	3, // Normal
	4, // Position
	4, // Color0
	4, // Color1
	3, // Texture0
	3, // Texture1
	3, // Texture2
	3, // Texture3
	3  // Texture4
};

typedef tFlag tVertexCompileFlag;

#define eVertexCompileFlag_CreateTangents (0x00000001)

class iLowLevelGraphics;

class iVertexBuffer {
public:
	iVertexBuffer(iLowLevelGraphics *apLowLevelGraphics, tVertexFlag aFlags,
				  eVertexBufferDrawType aDrawType, eVertexBufferUsageType aUsageType,
				  int alReserveVtxSize, int alReserveIdxSize) : mVertexFlags(aFlags), mpLowLevelGraphics(apLowLevelGraphics),
																mDrawType(aDrawType), mUsageType(aUsageType), mlElementNum(-1),
																mbTangents(false) {}

	virtual ~iVertexBuffer() {}

	tVertexFlag GetFlags() { return mVertexFlags; }

	virtual void AddVertex(tVertexFlag aType, const cVector3f &avVtx) = 0;
	virtual void AddColor(tVertexFlag aType, const cColor &aColor) = 0;
	virtual void AddIndex(unsigned int alIndex) = 0;

	virtual bool Compile(tVertexCompileFlag aFlags) = 0;
	virtual void UpdateData(tVertexFlag aTypes, bool abIndices) = 0;

	/**
	 * This creates a double of the vertex array with w=0.
	 * \param abUpdateData if the hardware buffer should be updated aswell.
	 */
	virtual void CreateShadowDouble(bool abUpdateData) = 0;

	/**
	 * Transform the entire buffer with transform.
	 */
	virtual void Transform(const cMatrixf &mtxTransform) = 0;

	virtual void Draw(eVertexBufferDrawType aDrawType = eVertexBufferDrawType_LastEnum) = 0;
	virtual void DrawIndices(unsigned int *apIndices, int alCount,
							 eVertexBufferDrawType aDrawType = eVertexBufferDrawType_LastEnum) = 0;

	virtual void Bind() = 0;
	virtual void UnBind() = 0;

	virtual iVertexBuffer *CreateCopy(eVertexBufferUsageType aUsageType) = 0;

	virtual cBoundingVolume CreateBoundingVolume() = 0;

	virtual float *GetArray(tVertexFlag aType) = 0;
	virtual unsigned int *GetIndices() = 0;

	virtual int GetVertexNum() = 0;
	virtual int GetIndexNum() = 0;

	/**
	 * Resizes an array to a custom size, the size is number of elements and NOT number of vertices.
	 */
	virtual void ResizeArray(tVertexFlag aType, int alSize) = 0;
	virtual void ResizeIndices(int alSize) = 0;

	// For debugging purposes, quite slow to use.
	virtual cVector3f GetVector3(tVertexFlag aType, unsigned alIdx) = 0;
	virtual cVector3f GetVector4(tVertexFlag aType, unsigned alIdx) = 0;
	virtual cColor GetColor(tVertexFlag aType, unsigned alIdx) = 0;
	virtual unsigned int GetIndex(tVertexFlag aType, unsigned alIdx) = 0;

	/**
	 * Set the number of of elements to draw.
	 * \param alNum If < 0, draw all indices.
	 */
	void SetElementNum(int alNum) { mlElementNum = alNum; }
	int GetElementNum() { return mlElementNum; }

	tVertexFlag GetVertexFlags() { return mVertexFlags; }

	bool HasTangents() { return mbTangents; }
	void SetTangents(bool abX) { mbTangents = abX; }

protected:
	tVertexFlag mVertexFlags;
	eVertexBufferDrawType mDrawType;
	eVertexBufferUsageType mUsageType;
	iLowLevelGraphics *mpLowLevelGraphics;

	int mlElementNum;

	bool mbTangents;
};

} // namespace hpl

#endif // HPL_RENDERER3D_H
