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

#include "hpl1/penumbra-overture/SaveTypes.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerState_Interact.h"

//////////////////////////////////////////////////////////////////////////
// BODY
//////////////////////////////////////////////////////////////////////////

void cEngineBody_SaveData::FromBody(iPhysicsBody *pBody) {
	mbActive = pBody->IsActive();
	mbCollideCharacter = pBody->GetCollideCharacter();
	mvLinearVelocity = pBody->GetLinearVelocity();
	mvAngularVelocity = pBody->GetAngularVelocity();
	m_mtxTransform = pBody->GetWorldMatrix();

	if (gpInit->mpPlayer->GetState() == ePlayerState_Grab &&
		gpInit->mpPlayer->GetPushBody() == pBody &&
		gpInit->mpPlayer->mbUseNormalMass == false) {
		mfMass = pBody->GetMass() * cPlayerState_Grab::mfMassDiv;
	} else {
		mfMass = pBody->GetMass();
	}
}
void cEngineBody_SaveData::ToBody(iPhysicsBody *pBody) {
	pBody->SetActive(mbActive);
	pBody->SetMass(mfMass);
	pBody->SetCollideCharacter(mbCollideCharacter);
	pBody->SetAngularVelocity(mvAngularVelocity);
	pBody->SetLinearVelocity(mvLinearVelocity);
	pBody->SetMatrix(m_mtxTransform);
}

kBeginSerializeBase(cEngineBody_SaveData)
	kSerializeVar(mfMass, eSerializeType_Float32)
		kSerializeVar(mbActive, eSerializeType_Bool)
			kSerializeVar(mbCollideCharacter, eSerializeType_Bool)
				kSerializeVar(mvLinearVelocity, eSerializeType_Vector3f)
					kSerializeVar(mvAngularVelocity, eSerializeType_Vector3f)
						kSerializeVar(m_mtxTransform, eSerializeType_Matrixf)
							kEndSerialize()

	//////////////////////////////////////////////////////////////////////////
	// JOINT
	//////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------
	void cEngineJoint_SaveData::FromJoint(iPhysicsJoint *apJoint) {
	msName = apJoint->GetName();

	///////////////////////////////
	// Callback
	cScriptJointCallback *pCallback = static_cast<cScriptJointCallback *>(apJoint->GetCallback());
	if (pCallback) {
		msOnMaxCallback = pCallback->msMaxFunc;
		msOnMinCallback = pCallback->msMinFunc;
	} else {
		msOnMaxCallback = "";
		msOnMinCallback = "";
	}

	///////////////////////////////
	// Controllers
	cPhysicsControllerIterator ctrlIt = apJoint->GetControllerIterator();
	while (ctrlIt.HasNext()) {
		iPhysicsController *pCtrl = ctrlIt.Next();
		cEngineJointController_SaveData saveCtrl;

		saveCtrl.msName = pCtrl->GetName();
		saveCtrl.mfDestValue = pCtrl->GetDestValue();
		saveCtrl.mbActive = pCtrl->IsActive();

		mvControllers.Add(saveCtrl);
	}

	///////////////////////////////
	// Type specific
	switch (apJoint->GetType()) {
	case ePhysicsJointType_Ball: {
		iPhysicsJointBall *pBallJoint = static_cast<iPhysicsJointBall *>(apJoint);
		mfMaxLimit = pBallJoint->GetMaxConeAngle();
		mfMinLimit = pBallJoint->GetMaxTwistAngle();
		break;
	}
	case ePhysicsJointType_Hinge: {
		iPhysicsJointHinge *pHingeJoint = static_cast<iPhysicsJointHinge *>(apJoint);
		mfMaxLimit = pHingeJoint->GetMaxAngle();
		mfMinLimit = pHingeJoint->GetMinAngle();
		break;
	}
	case ePhysicsJointType_Screw: {
		iPhysicsJointScrew *pScrewJoint = static_cast<iPhysicsJointScrew *>(apJoint);
		mfMinLimit = pScrewJoint->GetMinDistance();
		mfMaxLimit = pScrewJoint->GetMaxDistance();
		break;
	}
	case ePhysicsJointType_Slider: {
		iPhysicsJointSlider *pSliderJoint = static_cast<iPhysicsJointSlider *>(apJoint);
		mfMinLimit = pSliderJoint->GetMinDistance();
		mfMaxLimit = pSliderJoint->GetMaxDistance();
		break;
	}
	default:
		break;
	}
}

//------------------------------------------------------------------------

void cEngineJoint_SaveData::ToJoint(iPhysicsJoint *apJoint) {
	///////////////////////////////
	// Callback
	cScriptJointCallback *pCallback = static_cast<cScriptJointCallback *>(apJoint->GetCallback());
	if (pCallback == NULL) {
		pCallback = hplNew(cScriptJointCallback, (gpInit->mpGame->GetScene()));
		apJoint->SetCallback(pCallback, true);

		if (msOnMaxCallback != "" || msOnMinCallback != "") {
			pCallback->msMaxFunc = msOnMaxCallback;
			pCallback->msMinFunc = msOnMinCallback;
		}
	} else {
		pCallback->msMaxFunc = msOnMaxCallback;
		pCallback->msMinFunc = msOnMinCallback;
	}

	///////////////////////////////
	// Controllers
	for (int i = 0; i < (int)mvControllers.Size(); ++i) {
		iPhysicsController *pCtrl = apJoint->GetController(mvControllers[i].msName);

		pCtrl->SetDestValue(mvControllers[i].mfDestValue);
		pCtrl->SetActive(mvControllers[i].mbActive);
	}

	///////////////////////////////
	// Type specific
	switch (apJoint->GetType()) {
	case ePhysicsJointType_Ball: {
		iPhysicsJointBall *pBallJoint = static_cast<iPhysicsJointBall *>(apJoint);
		pBallJoint->SetConeLimits(pBallJoint->GetConePin(), mfMaxLimit, mfMinLimit);
		break;
	}
	case ePhysicsJointType_Hinge: {
		iPhysicsJointHinge *pHingeJoint = static_cast<iPhysicsJointHinge *>(apJoint);
		pHingeJoint->SetMaxAngle(mfMaxLimit);
		pHingeJoint->SetMinAngle(mfMinLimit);
		break;
	}
	case ePhysicsJointType_Screw: {
		iPhysicsJointScrew *pScrewJoint = static_cast<iPhysicsJointScrew *>(apJoint);
		pScrewJoint->SetMinDistance(mfMinLimit);
		pScrewJoint->SetMaxDistance(mfMaxLimit);
		break;
	}
	case ePhysicsJointType_Slider: {
		iPhysicsJointSlider *pSliderJoint = static_cast<iPhysicsJointSlider *>(apJoint);
		pSliderJoint->SetMinDistance(mfMinLimit);
		pSliderJoint->SetMaxDistance(mfMaxLimit);
		break;
	}
	default:
		break;
	}
}

//------------------------------------------------------------------------

kBeginSerializeBase(cEngineJointController_SaveData)
	kSerializeVar(msName, eSerializeType_String)
		kSerializeVar(mfDestValue, eSerializeType_Float32)
			kSerializeVar(mbActive, eSerializeType_Bool)
				kEndSerialize()

					kBeginSerializeBase(cEngineJoint_SaveData)
						kSerializeVar(msName, eSerializeType_String)
							kSerializeVar(mfMinLimit, eSerializeType_Float32)
								kSerializeVar(mfMaxLimit, eSerializeType_Float32)
									kSerializeVar(msOnMinCallback, eSerializeType_String)
										kSerializeVar(msOnMaxCallback, eSerializeType_String)
											kSerializeClassContainer(mvControllers, cEngineJointController_SaveData, eSerializeType_Class)
												kEndSerialize()
	//------------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PS
	//////////////////////////////////////////////////////////////////////////

	//----------------------------------------------------------
	void cEnginePS_SaveData::FromPS(cParticleSystem3D *apPS) {
	if (apPS) {
		msName = apPS->GetName();

		msType = apPS->GetDataName();
		mvSize = apPS->GetDataSize();
		m_mtxTransform = apPS->GetWorldMatrix();

		mvEmitterActive.Resize(apPS->GetEmitterNum());
		for (int i = 0; i < apPS->GetEmitterNum(); ++i) {
			iParticleEmitter *pEmitter = apPS->GetEmitter(i);

			if (pEmitter->IsDying() || pEmitter->IsDead())
				mvEmitterActive[i].mbActive = false;
			else
				mvEmitterActive[i].mbActive = true;
		}
	} else {
		msName = "NULL";
		msType = "";
		mvEmitterActive.Clear();
	}
}
void cEnginePS_SaveData::ToPS(cParticleSystem3D *apPS) {
	if (apPS == NULL)
		return;

	for (size_t i = 0; i < mvEmitterActive.Size(); ++i) {
		iParticleEmitter *pEmitter = apPS->GetEmitter((int)i);

		if (mvEmitterActive[i].mbActive == false) {
			pEmitter->KillInstantly();
		}
	}
}

//----------------------------------------------------------

kBeginSerializeBase(cEnginePSEmitter_SaveData)
	kSerializeVar(mbActive, eSerializeType_Bool)
		kEndSerialize()

			kBeginSerializeBase(cEnginePS_SaveData)
				kSerializeVar(msName, eSerializeType_String)
					kSerializeVar(msType, eSerializeType_String)
						kSerializeVar(mvSize, eSerializeType_Vector3f)
							kSerializeVar(m_mtxTransform, eSerializeType_Matrixf)
								kSerializeClassContainer(mvEmitterActive, cEnginePSEmitter_SaveData, eSerializeType_Class)
									kEndSerialize()

	//----------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// BEAM
	//////////////////////////////////////////////////////////////////////////

	//----------------------------------------------------------
	void cEngineBeam_SaveData::FromBeam(cBeam *apBeam) {
	msName = apBeam->GetName();
	msFile = apBeam->GetFileName();

	mvStartPos = apBeam->GetLocalPosition();
	mvEndPos = apBeam->GetEnd()->GetLocalPosition();
}

void cEngineBeam_SaveData::ToBeam(cBeam *apBeam) {
	apBeam->LoadXMLProperties(msFile);

	apBeam->SetPosition(mvStartPos);
	apBeam->GetEnd()->SetPosition(mvEndPos);
}

//----------------------------------------------------------

kBeginSerializeBase(cEngineBeam_SaveData)
	kSerializeVar(msName, eSerializeType_String)
		kSerializeVar(msFile, eSerializeType_String)

			kSerializeVar(mvStartPos, eSerializeType_Vector3f)
				kSerializeVar(mvEndPos, eSerializeType_Vector3f)
					kEndSerialize()

	//----------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// SOUND
	//////////////////////////////////////////////////////////////////////////

	void cEngineSound_SaveData::FromSound(cSoundEntity *apSound) {
	msName = apSound->GetName();
	mbActive = apSound->IsActive();
	mbStopped = apSound->IsStopped();
}

void cEngineSound_SaveData::ToSound(cSoundEntity *apSound) {
	apSound->SetActive(mbActive);
	if (mbStopped)
		apSound->Stop(false);
	if (_fading)
		apSound->FadeOut(_fadeSpeed);
}

kBeginSerializeBase(cEngineSound_SaveData)
	kSerializeVar(msName, eSerializeType_String)
		kSerializeVar(mbActive, eSerializeType_Bool)
			kSerializeVar(mbStopped, eSerializeType_Bool)
				kSerializeVar(_fading, eSerializeType_Bool)
					kSerializeVar(_fadeSpeed, eSerializeType_Float32)
						kEndSerialize()

	//////////////////////////////////////////////////////////////////////////
	// LIGHT
	//////////////////////////////////////////////////////////////////////////

	kBeginSerializeBase(cEngineLightAttachBB_SaveData)
		kSerializeVar(msName, eSerializeType_String)
			kEndSerialize()

				void cEngineLight_SaveData::FromLight(iLight3D *apLight) {
	msName = apLight->GetName();

	if (apLight->IsFading() && apLight->GetFlickerActive() == false) {
		apLight->SetDiffuseColor(apLight->GetDestColor());
		apLight->SetFarAttenuation(apLight->GetDestRadius());
	}

	mbActive = apLight->IsActive();
	mbVisible = apLight->IsVisible();

	if (apLight->GetFlickerActive()) {
		mDiffuseColor = apLight->GetFlickerOnColor();
		mfFarAttenuation = apLight->GetFlickerOnRadius();
	} else {
		mDiffuseColor = apLight->GetDiffuseColor();
		mfFarAttenuation = apLight->GetFarAttenuation();
	}

	Common::Array<cBillboard *> *pBillboardVec = apLight->GetBillboardVec();
	for (size_t i = 0; i < pBillboardVec->size(); ++i) {
		cBillboard *pBillboard = (*pBillboardVec)[i];
		cEngineLightAttachBB_SaveData attachedBB;
		attachedBB.msName = pBillboard->GetName();
		mvBillboards.Add(attachedBB);
	}

	mbFlickering = apLight->GetFlickerActive();
	msFlickerOffSound = apLight->GetFlickerOffSound();
	msFlickerOnSound = apLight->GetFlickerOnSound();
	msFlickerOffPS = apLight->GetFlickerOffPS();
	msFlickerOnPS = apLight->GetFlickerOnPS();
	mfFlickerOnMinLength = apLight->GetFlickerOnMinLength();
	mfFlickerOffMinLength = apLight->GetFlickerOffMinLength();
	mfFlickerOnMaxLength = apLight->GetFlickerOnMaxLength();
	mfFlickerOffMaxLength = apLight->GetFlickerOffMaxLength();
	mFlickerOffColor = apLight->GetFlickerOffColor();
	mfFlickerOffRadius = apLight->GetFlickerOffRadius();
	mbFlickerFade = apLight->GetFlickerFade();
	mfFlickerOnFadeLength = apLight->GetFlickerOnFadeLength();
	mfFlickerOffFadeLength = apLight->GetFlickerOffFadeLength();
}

void cEngineLight_SaveData::ToLight(iLight3D *apLight) {
	apLight->SetActive(mbActive);
	apLight->SetVisible(mbVisible);
	apLight->SetDiffuseColor(mDiffuseColor);
	apLight->SetFarAttenuation(mfFarAttenuation);

	cWorld3D *pWorld = gpInit->mpGame->GetScene()->GetWorld3D();
	for (size_t i = 0; i < mvBillboards.Size(); ++i) {
		cBillboard *pBillboard = pWorld->GetBillboard(mvBillboards[i].msName);
		if (pBillboard)
			apLight->AttachBillboard(pBillboard);
		else
			Warning("Could not find billboard %s\n", mvBillboards[i].msName.c_str());
	}

	apLight->SetFlickerActive(mbFlickering);
	apLight->SetFlicker(mFlickerOffColor, mfFlickerOffRadius,
						mfFlickerOnMinLength, mfFlickerOnMaxLength,
						msFlickerOnSound, msFlickerOnPS,
						mfFlickerOffMinLength, mfFlickerOffMaxLength,
						msFlickerOffSound, msFlickerOffPS,
						mbFlickerFade, mfFlickerOnFadeLength, mfFlickerOffFadeLength);
}

kBeginSerializeBase(cEngineLight_SaveData)
	kSerializeVar(msName, eSerializeType_String)
		kSerializeVar(mbActive, eSerializeType_Bool)
			kSerializeVar(mbVisible, eSerializeType_Bool)

				kSerializeVar(mDiffuseColor, eSerializeType_Color)
					kSerializeVar(mfFarAttenuation, eSerializeType_Float32)

						kSerializeClassContainer(mvBillboards, cEngineLightAttachBB_SaveData, eSerializeType_Class)

							kSerializeVar(mbFlickering, eSerializeType_Bool)
								kSerializeVar(msFlickerOffSound, eSerializeType_String)
									kSerializeVar(msFlickerOnSound, eSerializeType_String)
										kSerializeVar(msFlickerOffPS, eSerializeType_String)
											kSerializeVar(msFlickerOnPS, eSerializeType_String)
												kSerializeVar(mfFlickerOnMinLength, eSerializeType_Float32)
													kSerializeVar(mfFlickerOffMinLength, eSerializeType_Float32)
														kSerializeVar(mfFlickerOnMaxLength, eSerializeType_Float32)
															kSerializeVar(mfFlickerOffMaxLength, eSerializeType_Float32)
																kSerializeVar(mFlickerOffColor, eSerializeType_Color)
																	kSerializeVar(mfFlickerOffRadius, eSerializeType_Float32)
																		kSerializeVar(mbFlickerFade, eSerializeType_Bool)
																			kSerializeVar(mfFlickerOnFadeLength, eSerializeType_Float32)
																				kSerializeVar(mfFlickerOffFadeLength, eSerializeType_Float32)
																					kEndSerialize()
