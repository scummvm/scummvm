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

#include "hpl1/engine/graphics/Bone.h"

#include "hpl1/engine/graphics/Skeleton.h"
#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cBone::cBone(const tString &asName, cSkeleton *apSkeleton) {
	msName = asName;
	mpSkeleton = apSkeleton;
	mpParent = NULL;

	mbNeedsUpdate = true;

	mlValue = 0;
}

//-----------------------------------------------------------------------

cBone::~cBone() {
	STLDeleteAll(mlstChildren);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cBone *cBone::CreateChildBone(const tString &asName) {
	cBone *pBone = hplNew(cBone, (asName, mpSkeleton));
	pBone->mpParent = this;

	mlstChildren.push_back(pBone);
	mpSkeleton->AddBone(pBone);

	return pBone;
}

//-----------------------------------------------------------------------

void cBone::SetTransform(const cMatrixf &a_mtxTransform) {
	m_mtxTransform = a_mtxTransform;

	NeedsUpdate();
}

//-----------------------------------------------------------------------

const cMatrixf &cBone::GetLocalTransform() {
	return m_mtxTransform;
}

const cMatrixf &cBone::GetWorldTransform() {
	UpdateMatrix();

	return m_mtxWorldTransform;
}

const cMatrixf &cBone::GetInvWorldTransform() {
	UpdateMatrix();

	return m_mtxInvWorldTransform;
}

//-----------------------------------------------------------------------

void cBone::Detach() {
	if (mpParent == NULL)
		return;

	tBoneListIt it = mpParent->mlstChildren.begin();
	for (; it != mpParent->mlstChildren.end(); it++) {
		if (*it == this) {
			mpParent->mlstChildren.erase(it);
			break;
		}
	}

	mpSkeleton->RemoveBone(this);
}

//-----------------------------------------------------------------------

cBoneIterator cBone::GetChildIterator() {
	return cBoneIterator(&mlstChildren);
}

//-----------------------------------------------------------------------

void cBone::UpdateMatrix() {
	if (mbNeedsUpdate == false)
		return;

	if (mpParent == NULL) {
		m_mtxWorldTransform = m_mtxTransform;
	} else {
		m_mtxWorldTransform = cMath::MatrixMul(mpParent->GetWorldTransform(), m_mtxTransform);
	}

	m_mtxInvWorldTransform = cMath::MatrixInverse(m_mtxWorldTransform);

	mbNeedsUpdate = false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cBone::NeedsUpdate() {
	mbNeedsUpdate = true;

	tBoneListIt it = mlstChildren.begin();
	for (; it != mlstChildren.end(); it++) {
		cBone *pBone = *it;

		pBone->NeedsUpdate();
	}
}
//-----------------------------------------------------------------------
} // namespace hpl
