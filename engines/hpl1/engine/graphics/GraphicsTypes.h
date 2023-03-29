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

#ifndef HPL_GRAPHICSTYPES_H
#define HPL_GRAPHICSTYPES_H

#include "common/array.h"
#include "common/list.h"
#include "hpl1/engine/graphics/Color.h"
#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

#define MAX_TEXTUREUNITS (8)
#define MAX_NUM_OF_LIGHTS (30)

enum eMatrix {
	eMatrix_ModelView,
	eMatrix_Projection,
	eMatrix_Texture,
	eMatrix_LastEnum
};

enum eTileRotation {
	eTileRotation_0,
	eTileRotation_90,
	eTileRotation_180,
	eTileRotation_270,
	eTileRotation_LastEnum
};

enum ePrimitiveType {
	ePrimitiveType_Tri,
	ePrimitiveType_Quad,
	ePrimitiveType_LastEnum
};

typedef tFlag tAnimTransformFlag;

#define eAnimTransformFlag_Translate (0x00000001)
#define eAnimTransformFlag_Scale (0x00000002)
#define eAnimTransformFlag_Rotate (0x00000004)

#define klNumOfAnimTransformFlags (3)

const tAnimTransformFlag kvAnimTransformFlags[] = {eAnimTransformFlag_Translate,
												   eAnimTransformFlag_Scale, eAnimTransformFlag_Rotate};

//-----------------------------------------

enum eColorDataFormat {
	eColorDataFormat_RGB,
	eColorDataFormat_RGBA,
	eColorDataFormat_ALPHA,
	eColorDataFormat_BGR,
	eColorDataFormat_BGRA,
	eColorDataFormat_LastEnum
};

//---------------------------------------

enum eFontAlign {
	eFontAlign_Left,
	eFontAlign_Right,
	eFontAlign_Center,
	eFontAlign_LastEnum
};

//---------------------------------------

class cKeyFrame {
public:
	cVector3f trans;
	cVector3f scale;
	cQuaternion rotation;
	float time;
};

typedef Common::Array<cKeyFrame *> tKeyFramePtrVec;
typedef tKeyFramePtrVec::iterator tKeyFramePtrVecIt;

typedef Common::Array<cKeyFrame> tKeyFrameVec;
typedef tKeyFrameVec::iterator tKeyFrameVecIt;

//---------------------------------------

enum eAnimationEventType {
	eAnimationEventType_PlaySound,
	eAnimationEventType_LastEnum
};

//---------------------------------------

class cVertexBonePair {
public:
	cVertexBonePair(unsigned int aVtx, unsigned int aBone, float aW) {
		vtxIdx = aVtx;
		boneIdx = aBone;
		weight = aW;
	}
	cVertexBonePair() {}

	unsigned int vtxIdx;
	unsigned int boneIdx;
	float weight;
};

typedef Common::Array<cVertexBonePair> tVertexBonePairVec;
typedef tVertexBonePairVec::iterator tVertexBonePairVecIt;

//---------------------------------------

class cVertex {
public:
	cVertex() : pos(0), tex(0), col(0) {}
	cVertex(const cVector3f &avPos, const cVector3f &avTex, const cColor &aCol) {
		pos = avPos;
		tex = avTex;
		col = aCol;
	}

	cVertex(const cVector3f &avPos, const cColor &aCol) {
		pos = avPos;
		col = aCol;
	}

	cVector3f pos;
	cVector3f tex;
	cVector3f tan;
	cVector3f norm;
	cColor col;
};

typedef Common::List<cVertex> tVertexList;
typedef tVertexList::iterator tVertexListIt;

typedef Common::Array<cVertex> tVertexVec;
typedef tVertexVec::iterator tVertexVecIt;
} // namespace hpl

#endif // HPL_GRAPHICSTYPES_H
