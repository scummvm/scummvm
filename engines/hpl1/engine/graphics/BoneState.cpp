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

#include "hpl1/engine/graphics/BoneState.h"

#include "hpl1/engine/physics/PhysicsBody.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cBoneState::cBoneState(const tString &asName, bool abAutoDeleteChildren) : cNode3D(asName, abAutoDeleteChildren) {
	mpBody = NULL;
	mpColliderBody = NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cBoneState::SetBody(iPhysicsBody *apBody) {
	mpBody = apBody;
}
iPhysicsBody *cBoneState::GetBody() {
	return mpBody;
}

//-----------------------------------------------------------------------

void cBoneState::SetColliderBody(iPhysicsBody *apBody) {
	mpColliderBody = apBody;
}
iPhysicsBody *cBoneState::GetColliderBody() {
	return mpColliderBody;
}

//-----------------------------------------------------------------------

void cBoneState::SetBodyMatrix(const cMatrixf &a_mtxBody) {
	m_mtxBody = a_mtxBody;
	m_mtxInvBody = cMath::MatrixInverse(m_mtxBody);
}

const cMatrixf &cBoneState::GetBodyMatrix() {
	return m_mtxBody;
}

const cMatrixf &cBoneState::GetInvBodyMatrix() {
	return m_mtxInvBody;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

} // namespace hpl
