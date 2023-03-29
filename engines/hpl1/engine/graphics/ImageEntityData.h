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

#ifndef HPL_IMAGE_ENTITY_DATA_H
#define HPL_IMAGE_ENTITY_DATA_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/resources/ResourceBase.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/std/map.h"

class TiXmlElement;

namespace hpl {

#define kMaxImageEntityFrameWidth (1024)

class cResources;
class cGraphics;
class iMaterial;
class cMesh2D;

class cImageFrame {
public:
	iMaterial *mpMaterial;
	tVertexVec mvVtx;
};

typedef Common::Array<cImageFrame> tImageFrameVec;
typedef tImageFrameVec::iterator tImageFrameVecIt;

class cImageAnimation {
public:
	tString msName;
	int mlHandle;
	float mfSpeed;
	tIntVec mvFrameNums;
	bool mbCollidable;
	tString msSound;
};

typedef Hpl1::Std::map<tString, cImageAnimation> tImageAnimationMap;
typedef tImageAnimationMap::iterator tImageAnimationMapIt;

class cImageEntityData : public iResourceBase {
public:
	cImageEntityData(tString asName, cGraphics *apGraphics, cResources *apResources);
	~cImageEntityData();

	// resource stuff:
	bool reload() { return false; }
	void unload() {}
	void destroy() {}

	bool CreateFromFile(const tString &asFile, tIntVec &avImageHandle);

	cImageFrame *GetImageFrame(int alFrame);
	tUIntVec *GetIndexVec() { return &mvIdxVec; }

	int GetFrameNum() { return mlFrameNum; }
	cImageAnimation *GetAnimationByName(const tString &asName);
	cImageAnimation *GetAnimationByHandle(int alHandle);
	int GetAnimationNum() { return (int)m_mapAnimations.size(); }
	tImageAnimationMap *GetAnimationMap() { return &m_mapAnimations; }

	const tString &GetType() { return msType; }
	const tString &GetSubType() { return msSubType; }
	const tString &GetDataName() { return msDataName; }

	cVector2f GetImageSize() { return mvImageSize; }

	cMesh2D *GetCollideMesh() { return mpCollideMesh; }

	bool IsCollidable() { return mbCollidable; }
	bool GetCollides() { return mbCollides; }
	bool GetCastShadows() { return mbCastShadows; }

private:
	int mlFrameNum;
	cResources *mpResources;
	cGraphics *mpGraphics;
	cVector2l mvFrameSize;
	cMesh2D *mpMesh;
	cMesh2D *mpCollideMesh;
	tUIntVec mvIdxVec;
	cVector2f mvImageSize;

	tString msDataName;
	tString msType;
	tString msSubType;

	bool mbCastShadows;
	bool mbCollidable;
	bool mbCollides;
	bool mbLit;

	tImageFrameVec mvImageFrames;
	tImageAnimationMap m_mapAnimations;

	void GetFrameNum(TiXmlElement *apElement);
};

} // namespace hpl

#endif // HPL_IMAGE_ENTITY_DATA_H
