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

#ifndef HPL_SKELETON_H
#define HPL_SKELETON_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/std/map.h"

namespace hpl {

class cBone;

typedef Common::Array<cBone *> tBoneVec;
typedef tBoneVec::iterator tBoneVecIt;

typedef Hpl1::Std::map<tString, int> tBoneIdxNameMap;
typedef tBoneIdxNameMap::iterator tBoneIdxNameMapIt;

class cSkeleton {
public:
	cSkeleton();
	~cSkeleton();

	void AddBone(cBone *apBone);
	void RemoveBone(cBone *apBone);

	cBone *GetRootBone();

	cBone *GetBoneByIndex(int alIndex);
	cBone *GetBoneByName(const tString &asName);
	int GetBoneIndexByName(const tString &asName);
	int GetBoneNum();

private:
	cBone *mpRootBone;

	tBoneVec mvBones;
	tBoneIdxNameMap m_mapBonesIdxByName;
};

} // namespace hpl

#endif // HPL_SKELETON_H
