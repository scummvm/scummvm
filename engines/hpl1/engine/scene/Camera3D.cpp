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

#include "hpl1/engine/scene/Camera3D.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/scene/Entity3D.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cCamera3D::cCamera3D() {
	mvPosition = cVector3f(0);

	mfFOV = cMath::ToRad(70.0f);
	mfAspect = 4.0f / 3.0f;
	mfFarClipPlane = 1000.0f;
	mfNearClipPlane = 0.05f;

	mfPitch = 0;
	mfYaw = 0;
	mfRoll = 0;

	mRotateMode = eCameraRotateMode_EulerAngles;
	mMoveMode = eCameraMoveMode_Fly;

	m_mtxView = cMatrixf::Identity;
	m_mtxProjection = cMatrixf::Identity;
	m_mtxMove = cMatrixf::Identity;

	mbViewUpdated = true;
	mbProjectionUpdated = true;
	mbMoveUpdated = true;

	mbInfFarPlane = true;

	mvPitchLimits = cVector2f(kPif / 2.0f, -kPif / 2.0f);
	mvYawLimits = cVector2f(0, 0);
}

//-----------------------------------------------------------------------

cCamera3D::~cCamera3D() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cCamera3D::SetPosition(const cVector3f &avPos) {
	mvPosition = avPos;
	mbViewUpdated = true;

	mNode.SetPosition(mvPosition);
}

//-----------------------------------------------------------------------

void cCamera3D::SetPitch(float afAngle) {
	mfPitch = afAngle;

	if (mvPitchLimits.x != 0 || mvPitchLimits.y != 0) {
		if (mfPitch > mvPitchLimits.x)
			mfPitch = mvPitchLimits.x;
		if (mfPitch < mvPitchLimits.y)
			mfPitch = mvPitchLimits.y;
	}

	mbViewUpdated = true;
	mbMoveUpdated = true;
}
void cCamera3D::SetYaw(float afAngle) {
	mfYaw = afAngle;

	if (mvYawLimits.x != 0 || mvYawLimits.y != 0) {
		if (mfYaw > mvYawLimits.x)
			mfYaw = mvYawLimits.x;
		if (mfYaw < mvYawLimits.y)
			mfYaw = mvYawLimits.y;
	}

	mbViewUpdated = true;
	mbMoveUpdated = true;
}
void cCamera3D::SetRoll(float afAngle) {
	mfRoll = afAngle;
	mbViewUpdated = true;
	mbMoveUpdated = true;
}

//-----------------------------------------------------------------------

void cCamera3D::AddPitch(float afAngle) {
	mfPitch += afAngle;

	if (mvPitchLimits.x != 0 || mvPitchLimits.y != 0) {
		if (mfPitch > mvPitchLimits.x)
			mfPitch = mvPitchLimits.x;
		if (mfPitch < mvPitchLimits.y)
			mfPitch = mvPitchLimits.y;
	}

	mbViewUpdated = true;
	mbMoveUpdated = true;
}
void cCamera3D::AddYaw(float afAngle) {
	mfYaw += afAngle;

	if (mvYawLimits.x != 0 || mvYawLimits.y != 0) {
		if (mfYaw > mvYawLimits.x)
			mfYaw = mvYawLimits.x;
		if (mfYaw < mvYawLimits.y)
			mfYaw = mvYawLimits.y;
	}

	mbViewUpdated = true;
	mbMoveUpdated = true;
}
void cCamera3D::AddRoll(float afAngle) {
	mfRoll += afAngle;
	mbViewUpdated = true;
	mbMoveUpdated = true;
}

//-----------------------------------------------------------------------

void cCamera3D::MoveForward(float afDist) {
	UpdateMoveMatrix();

	mvPosition += m_mtxMove.GetForward() * -afDist;

	mbViewUpdated = true;

	mNode.SetPosition(mvPosition);
}

//-----------------------------------------------------------------------

void cCamera3D::MoveRight(float afDist) {
	UpdateMoveMatrix();

	mvPosition += m_mtxMove.GetRight() * afDist;

	mbViewUpdated = true;

	mNode.SetPosition(mvPosition);
}

//-----------------------------------------------------------------------

void cCamera3D::MoveUp(float afDist) {
	UpdateMoveMatrix();

	mvPosition += m_mtxMove.GetUp() * afDist;

	mbViewUpdated = true;

	mNode.SetPosition(mvPosition);
}

//-----------------------------------------------------------------------

void cCamera3D::SetRotateMode(eCameraRotateMode aMode) {
	mRotateMode = aMode;
	mbViewUpdated = true;
	mbMoveUpdated = true;
}

//-----------------------------------------------------------------------

void cCamera3D::SetMoveMode(eCameraMoveMode aMode) {
	mMoveMode = aMode;
	mbMoveUpdated = true;
}

//-----------------------------------------------------------------------

void cCamera3D::ResetRotation() {
	mbViewUpdated = false;
	mbMoveUpdated = false;
	m_mtxMove = cMatrixf::Identity;
	m_mtxView = cMatrixf::Identity;

	mfRoll = 0;
	mfYaw = 0;
	mfPitch = 0;
}

//-----------------------------------------------------------------------

cFrustum *cCamera3D::GetFrustum() {
	// If the far plane is infinite, still have to use a number on far plane
	// to calculate the near plane in the frustm.
	bool bWasInf = false;
	if (mbInfFarPlane) {
		SetInifintiveFarPlane(false);
		bWasInf = true;
	}
	mFrustum.SetViewProjMatrix(GetProjectionMatrix(),
							   GetViewMatrix(),
							   GetFarClipPlane(), GetNearClipPlane(),
							   GetFOV(), GetAspect(), GetPosition(), mbInfFarPlane);
	if (bWasInf) {
		SetInifintiveFarPlane(true);
	}

	return &mFrustum;
}

//-----------------------------------------------------------------------

const cMatrixf &cCamera3D::GetViewMatrix() {
	if (mbViewUpdated) {
		if (mRotateMode == eCameraRotateMode_EulerAngles) {
			m_mtxView = cMatrixf::Identity;

			m_mtxView = cMath::MatrixMul(cMath::MatrixTranslate(mvPosition * -1), m_mtxView);
			m_mtxView = cMath::MatrixMul(cMath::MatrixRotateY(-mfYaw), m_mtxView);
			m_mtxView = cMath::MatrixMul(cMath::MatrixRotateX(-mfPitch), m_mtxView);
			m_mtxView = cMath::MatrixMul(cMath::MatrixRotateZ(-mfRoll), m_mtxView);
		}

		mbViewUpdated = false;
	}
	return m_mtxView;
}

//-----------------------------------------------------------------------

const cMatrixf &cCamera3D::GetProjectionMatrix() {
	if (mbProjectionUpdated) {
		float fFar = mfFarClipPlane;
		float fNear = mfNearClipPlane;
		float fTop = tan(mfFOV * 0.5f) * fNear;
		float fBottom = -fTop;
		float fRight = mfAspect * fTop;
		float fLeft = mfAspect * fBottom;

		float A = (2.0f * fNear) / (fRight - fLeft);
		float B = (2.0f * fNear) / (fTop - fBottom);
		float D = -1.0f;
		float C, Z;
		if (mbInfFarPlane) {
			C = -2.0f * fNear;
			Z = -1.0f;
		} else {
			C = -(2.0f * fFar * fNear) / (fFar - fNear);
			Z = -(fFar + fNear) / (fFar - fNear);
		}

		float X = 0; //(fRight + fLeft)/(fRight - fLeft);
		float Y = 0; //(fTop + fBottom)/(fTop - fBottom);

		m_mtxProjection = cMatrixf(
			A, 0, X, 0,
			0, B, Y, 0,
			0, 0, Z, C,
			0, 0, D, 0);

		mbProjectionUpdated = false;
	}

	return m_mtxProjection;
}

//-----------------------------------------------------------------------

const cMatrixf &cCamera3D::GetMoveMatrix() {
	UpdateMoveMatrix();

	return m_mtxMove;
}

//-----------------------------------------------------------------------

cVector3f cCamera3D::GetEyePosition() {
	return mvPosition;
}

//-----------------------------------------------------------------------

void cCamera3D::SetModelViewMatrix(iLowLevelGraphics *apLowLevel) {
}

//-----------------------------------------------------------------------

void cCamera3D::SetProjectionMatrix(iLowLevelGraphics *apLowLevel) {
}

//-----------------------------------------------------------------------

cVector3f cCamera3D::UnProject(const cVector2f &avScreenPos, iLowLevelGraphics *apLowLevel) {
	// This code uses math::unproejct which is not working currently
	/*cVector3f vPos(avScreenPos.x,avScreenPos.y,-0.1f);

	bool bWasInf=false;
	if(mbInfFarPlane){
		SetInifintiveFarPlane(false);bWasInf=true;
	}
	cMatrixf mtxViewProj = cMath::MatrixMul(GetProjectionMatrix(), GetViewMatrix());
	if(bWasInf){
		SetInifintiveFarPlane(true);
	}

	cVector2f vScreenSize = apLowLevel->GetVirtualSize();

	return cMath::Vector3UnProject(vPos, cRect2f(0,0,vScreenSize.x, vScreenSize.y), mtxViewProj);*/

	cVector2f vScreenSize = apLowLevel->GetVirtualSize();
	cVector2f vNormScreen((avScreenPos.x / vScreenSize.x) - 0.5f,
						  0.5f - (avScreenPos.y / vScreenSize.y));
	float afNormalizedSlope = tan(mfFOV * 0.5f);

	cVector2f avViewportToWorld;
	avViewportToWorld.y = afNormalizedSlope * mfNearClipPlane * 2.0f;
	avViewportToWorld.x = avViewportToWorld.y * mfAspect;

	cVector3f vDir(vNormScreen.x * avViewportToWorld.x,
				   vNormScreen.y * avViewportToWorld.y,
				   -mfNearClipPlane);

	cMatrixf mtxRot = cMath::MatrixInverse(GetViewMatrix().GetRotation());

	vDir = cMath::MatrixMul(mtxRot, vDir);
	vDir.Normalise();

	return vDir;
}

//-----------------------------------------------------------------------

void cCamera3D::AttachEntity(iEntity3D *apEntity) {
	mNode.AddEntity(apEntity);
}

void cCamera3D::RemoveEntity(iEntity3D *apEntity) {
	mNode.RemoveEntity(apEntity);
}

void cCamera3D::ClearAttachedEntities() {
	mNode.ClearEntities();
}

//-----------------------------------------------------------------------

cVector3f cCamera3D::GetForward() {
	return GetViewMatrix().GetForward() * -1.0f;
}
cVector3f cCamera3D::GetRight() {
	return GetViewMatrix().GetRight();
}
cVector3f cCamera3D::GetUp() {
	return GetViewMatrix().GetUp();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cCamera3D::UpdateMoveMatrix() {
	if (mbMoveUpdated) {
		if (mRotateMode == eCameraRotateMode_EulerAngles) {
			m_mtxMove = cMath::MatrixRotateY(-mfYaw);
			if (mMoveMode == eCameraMoveMode_Fly) {
				m_mtxMove = cMath::MatrixMul(cMath::MatrixRotateX(-mfPitch), m_mtxMove);
			}
		}

		mbMoveUpdated = false;
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
