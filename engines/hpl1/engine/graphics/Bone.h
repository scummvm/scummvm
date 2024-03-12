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

#ifndef HPL_BONE_H
#define HPL_BONE_H

#include "common/list.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "common/stablemap.h"

namespace hpl {

class cSkeleton;
class cBone;

typedef Common::List<cBone *> tBoneList;
typedef tBoneList::iterator tBoneListIt;

typedef cSTLIterator<cBone *, tBoneList, tBoneListIt> cBoneIterator;

class cBone {
	friend class cSkeleton;

public:
	cBone(const tString &asName, cSkeleton *apSkeleton);
	~cBone();

	cBone *CreateChildBone(const tString &asName);

	void SetTransform(const cMatrixf &a_mtxTransform);
	const cMatrixf &GetLocalTransform();
	const cMatrixf &GetWorldTransform();
	const cMatrixf &GetInvWorldTransform();

	const tString &GetName() { return msName; }

	cBoneIterator GetChildIterator();

	void Detach();

	cBone *GetParent() { return mpParent; }

	// Needed for some loading stuff..
	int GetValue() { return mlValue; }
	void SetValue(int alVal) { mlValue = alVal; }

private:
	void NeedsUpdate();

	void UpdateMatrix();

	tString msName;

	cMatrixf m_mtxTransform;

	cMatrixf m_mtxWorldTransform;
	cMatrixf m_mtxInvWorldTransform;

	cBone *mpParent;
	tBoneList mlstChildren;

	cSkeleton *mpSkeleton;

	bool mbNeedsUpdate;

	int mlValue;
};

} // namespace hpl

#endif // HPL_BONE_H
