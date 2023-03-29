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

#ifndef HPL_CAMERA3D_H
#define HPL_CAMERA3D_H

#include "hpl1/engine/math/Frustum.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/scene/Node3D.h"

namespace hpl {

enum eCameraMoveMode {
	eCameraMoveMode_Walk,
	eCameraMoveMode_Fly,
	eCameraMoveMode_LastEnum
};

enum eCameraRotateMode {
	eCameraRotateMode_EulerAngles,
	eCameraRotateMode_Matrix,
	eCameraRotateMode_LastEnum
};

class iLowLevelGraphics;
class iEntity3D;

class cCamera3D : public iCamera {
public:
	cCamera3D();
	virtual ~cCamera3D();

	const cVector3f &GetPosition() const { return mvPosition; }
	void SetPosition(const cVector3f &avPos);

	/**
	 * Move forward (or back) according to the move mode.
	 * \param afDist
	 */
	void MoveForward(float afDist);
	/**
	 * Move right (or left) according to the move mode.
	 * \param afDist
	 */
	void MoveRight(float afDist);
	/**
	 * Move up (or down) according to the move mode.
	 * \param afDist
	 */
	void MoveUp(float afDist);

	void SetFOV(float afAngle) {
		mfFOV = afAngle;
		mbProjectionUpdated = true;
	}
	float GetFOV() { return mfFOV; }

	void SetAspect(float afAngle) {
		mfAspect = afAngle;
		mbProjectionUpdated = true;
	}
	float GetAspect() { return mfAspect; }

	void SetFarClipPlane(float afX) {
		mfFarClipPlane = afX;
		mbProjectionUpdated = true;
	}
	float GetFarClipPlane() { return mfFarClipPlane; }

	void SetNearClipPlane(float afX) {
		mfNearClipPlane = afX;
		mbProjectionUpdated = true;
	}
	float GetNearClipPlane() { return mfNearClipPlane; }

	/**
	 * This sets the far plane so that no far clipping is made.
	 * The FarClipPlane is still used for creating bounding box and frustum and
	 * should be set to some value.
	 */
	void SetInifintiveFarPlane(bool abX) {
		mbInfFarPlane = abX;
		mbProjectionUpdated = true;
	}
	bool GetInifintiveFarPlane() { return mbInfFarPlane; }

	cFrustum *GetFrustum();

	eCameraRotateMode GetRotateMode() { return mRotateMode; }

	eCameraMoveMode GetMoveMode() { return mMoveMode; }
	/**
	 * Set the mode to calculate the rotation angles.
	 * EulerAngles: Yaw, Pitch and Roll are used.
	 * Matrix: The matrix is changed directly.
	 */
	void SetRotateMode(eCameraRotateMode aMode);
	/**
	 * Set the mode to calculate movement.
	 * Walk: only moving in the XZ plane
	 * Fly: moving in the dir the camera is facing.
	 */
	void SetMoveMode(eCameraMoveMode aMode);

	/**
	 * Resets all rotation
	 */
	void ResetRotation();

	/**
	 * Unproject the screen coordinate to world space.
	 */
	cVector3f UnProject(const cVector2f &avScreenPos, iLowLevelGraphics *apLowLevel);

	void AttachEntity(iEntity3D *apEntity);
	void RemoveEntity(iEntity3D *apEntity);
	cNode3D *GetAttachmentNode() { return &mNode; }
	void ClearAttachedEntities();

	//////////////////////////////////////////////////
	////////// EULER ANGLES ROTATION /////////////////
	//////////////////////////////////////////////////

	void SetPitch(float afAngle);
	void SetYaw(float afAngle);
	void SetRoll(float afAngle);

	void AddPitch(float afAngle);
	void AddYaw(float afAngle);
	void AddRoll(float afAngle);

	float GetPitch() const { return mfPitch; }
	float GetYaw() const { return mfYaw; }
	float GetRoll() const { return mfRoll; }

	/**
	 * Set the limits within the pitch can move
	 * \param avLimits x = high limt and y low.If both are 0 limits are disabled.
	 */
	void SetPitchLimits(cVector2f avLimits) { mvPitchLimits = avLimits; }
	const cVector2f &GetPitchLimits() { return mvPitchLimits; }
	/**
	 * Set the limits within the yaw can move
	 * \param avLimits x = high limt and y low. If both are 0 limits are disabled.
	 */
	void SetYawLimits(cVector2f avLimits) { mvYawLimits = avLimits; }
	const cVector2f &GetYawLimits() { return mvYawLimits; }

	//////////////////////////////////////////////////
	////////// EULER ANGLES ROTATION /////////////////
	//////////////////////////////////////////////////

	const cMatrixf &GetViewMatrix();
	const cMatrixf &GetProjectionMatrix();

	const cMatrixf &GetMoveMatrix();

	// iCamera stuff:
	void SetModelViewMatrix(iLowLevelGraphics *apLowLevel);
	void SetProjectionMatrix(iLowLevelGraphics *apLowLevel);
	cVector3f GetEyePosition();

	eCameraType GetType() { return eCameraType_3D; }

	cVector3f GetForward();
	cVector3f GetRight();
	cVector3f GetUp();

	//////////////////////////////////////////////////
	////////// RENDER SPECIFIC ///////////////////////
	//////////////////////////////////////////////////

	void SetPrevView(const cMatrixf &a_mtxA) { m_mtxPrevView = a_mtxA; }
	void SetPrevProjection(const cMatrixf &a_mtxA) { m_mtxPrevProjection = a_mtxA; }

	cMatrixf &GetPrevView() { return m_mtxPrevView; }
	cMatrixf &GetPrevProjection() { return m_mtxPrevProjection; }

private:
	void UpdateMoveMatrix();

	cVector3f mvPosition;

	float mfFOV;
	float mfAspect;
	float mfFarClipPlane;
	float mfNearClipPlane;

	float mfPitch;
	float mfYaw;
	float mfRoll;
	cVector2f mvPitchLimits;
	cVector2f mvYawLimits;

	eCameraRotateMode mRotateMode;
	eCameraMoveMode mMoveMode;

	cMatrixf m_mtxView;
	cMatrixf m_mtxProjection;
	cMatrixf m_mtxMove;

	cMatrixf m_mtxPrevView;
	cMatrixf m_mtxPrevProjection;

	cNode3D mNode;

	cFrustum mFrustum;

	bool mbInfFarPlane;

	bool mbViewUpdated;
	bool mbProjectionUpdated;
	bool mbMoveUpdated;
};

} // namespace hpl

#endif // HPL_CAMERA3D_H
