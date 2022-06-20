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

#ifndef GAME_SAVE_TYPES_H
#define GAME_SAVE_TYPES_H

#include "hpl1/engine/engine.h"

using namespace hpl;

//------------------------------------------

class cEngineBody_SaveData : public iSerializable {
	kSerializableClassInit(cEngineBody_SaveData) public : void FromBody(iPhysicsBody *pBody);
	void ToBody(iPhysicsBody *pBody);

	float mfMass;
	bool mbActive;
	bool mbCollideCharacter;
	cVector3f mvLinearVelocity;
	cVector3f mvAngularVelocity;
	cMatrixf m_mtxTransform;
};

//------------------------------------------

class cEngineJointController_SaveData : public iSerializable {
	kSerializableClassInit(cEngineJointController_SaveData) public : tString msName;

	float mfDestValue;
	bool mbActive;
};

//------------------------------------------

class cEngineJoint_SaveData : public iSerializable {
	kSerializableClassInit(cEngineJoint_SaveData) public : void FromJoint(iPhysicsJoint *apJoint);
	void ToJoint(iPhysicsJoint *apJoint);

	tString msName;
	float mfMinLimit;
	float mfMaxLimit;

	tString msOnMinCallback;
	tString msOnMaxCallback;

	cContainerVec<cEngineJointController_SaveData> mvControllers;
};

//------------------------------------------

class cEnginePSEmitter_SaveData : public iSerializable {
	kSerializableClassInit(cEnginePSEmitter_SaveData) public : bool mbActive;
};

class cEnginePS_SaveData : public iSerializable {
	kSerializableClassInit(cEnginePS_SaveData) public : void FromPS(cParticleSystem3D *apPS);
	void ToPS(cParticleSystem3D *apPS);

	tString msName;
	tString msType;
	cVector3f mvSize;
	cMatrixf m_mtxTransform;

	cContainerVec<cEnginePSEmitter_SaveData> mvEmitterActive;
};

//------------------------------------------

class cEngineBeam_SaveData : public iSerializable {
	kSerializableClassInit(cEngineBeam_SaveData) public : void FromBeam(cBeam *apBeam);
	void ToBeam(cBeam *apBeam);

	tString msName;
	tString msFile;

	cVector3f mvStartPos;
	cVector3f mvEndPos;
};

//------------------------------------------

class cEngineSound_SaveData : public iSerializable {
	kSerializableClassInit(cEngineSound_SaveData) public : void FromSound(cSoundEntity *apSound);
	void ToSound(cSoundEntity *apSound);

	tString msName;
	bool mbActive;
	bool mbStopped;
};

//------------------------------------------

class cEngineLightAttachBB_SaveData : public iSerializable {
	kSerializableClassInit(cEngineLightAttachBB_SaveData) public : tString msName;
};

class cEngineLight_SaveData : public iSerializable {
	kSerializableClassInit(cEngineLight_SaveData) public : void FromLight(iLight3D *apLight);
	void ToLight(iLight3D *apLight);

	tString msName;
	bool mbActive;
	bool mbVisible;

	cColor mDiffuseColor;
	float mfFarAttenuation;

	cContainerVec<cEngineLightAttachBB_SaveData> mvBillboards;

	bool mbFlickering;
	tString msFlickerOffSound;
	tString msFlickerOnSound;
	tString msFlickerOffPS;
	tString msFlickerOnPS;
	float mfFlickerOnMinLength;
	float mfFlickerOffMinLength;
	float mfFlickerOnMaxLength;
	float mfFlickerOffMaxLength;
	cColor mFlickerOffColor;
	float mfFlickerOffRadius;
	bool mbFlickerFade;
	float mfFlickerOnFadeLength;
	float mfFlickerOffFadeLength;
};

//------------------------------------------

#endif // GAME_SAVE_TYPES_H
