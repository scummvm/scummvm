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
 * This file is part of Penumbra Overture.
 */

#ifndef GAME_HAPTIC_GAME_CAMERA_H
#define GAME_HAPTIC_GAME_CAMERA_H

#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameTypes.h"

#include <list>

using namespace hpl;

class cInit;
class cPlayer;

enum eHapticGameCameraType {
	eHapticGameCameraType_Frame,
	eHapticGameCameraType_Centre,
	eHapticGameCameraType_LastEnum
};

//----------------------------------------

class cHapticGameCamera {
public:
	cHapticGameCamera(cInit *apInit, cPlayer *apPlayer);
	~cHapticGameCamera();

	void Update(float afTimeStep);

	void OnWorldLoad();
	void OnWorldExit();
	void Reset();

	void OnPostSceneDraw();

	bool IsTurning() { return mbIsTurning; }

	void SetActive(bool abX);
	void SetCamMoveActive(bool abX) { mbCamMove = abX; }

	bool ShowCrosshair();

	void SetType(eHapticGameCameraType aType);

	cMeshEntity *GetHandEntity() { return mpHandEntity; }
	iCollideShape *GetHandShape() { return mpHandShape; }

	void SetHandVisible(bool abX);
	bool GetHandVisible();

	float GetHandReachRadius();

	void SetRenderActive(bool abX);

	void SetInteractModeCameraSpeed(float afSpeed);
	void SetActionModeCameraSpeed(float afSpeed);
	float GetInteractModeCameraSpeed() { return mfInteractModeCameraSpeed; }
	float GetActionModeCameraSpeed() { return mfActionModeCameraSpeed; }

	void SetUseFrame(bool abX) { mbUseFrame = abX; }

private:
	void UpdateCameraOrientation(float afTimeStep);
	void UpdateHand(float afTimeStep);
	void UpdateProxyCovered(float afTimeStep);
	void UpdateProxyInteraction(float afTimeStep);

	bool mbActive;

	bool mbRenderActive;

	cVector3f mvPrevPrxoyPos;

	int mlContactCount;

	bool mbUseFrame;

	bool mbHandVisible;

	cVector3f mvCentrePos;

	tHapticShapeList mlstActiveShapes;

	float mfInteractModeCameraSpeed;
	float mfActionModeCameraSpeed;

	float mfFrameW;
	float mfFrameH;

	float mfFrameForceStrength;
	float mfFrameMaxForce;
	float mfFrameMoveSpeedX;
	float mfFrameMoveSpeedY;
	float mfFrameMaxMoveSpeed;

	bool mbCamMove;

	eHapticGameCameraType mType;

	cInit *mpInit;
	cPlayer *mpPlayer;
	iLowLevelHaptic *mpLowLevelHaptic;
	cScene *mpScene;

	bool mbHapticWasActive;
	bool mbIsTurning;
	bool mbAtEdge;

	iHapticForce *mpForce;

	cMeshEntity *mpHandEntity;
	iCollideShape *mpHandShape;

	iGpuProgram *mpVtxProgram;

	std::list<cVector3f> mlstProxyPos;
};
//----------------------------------------

#endif // GAME_HAPTIC_GAME_CAMERA_H
