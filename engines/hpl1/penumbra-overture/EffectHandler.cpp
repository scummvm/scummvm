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

#include "hpl1/penumbra-overture/EffectHandler.h"

#include "hpl1/penumbra-overture/GameMessageHandler.h"
#include "hpl1/penumbra-overture/GameSaveArea.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHelper.h"
#include "hpl1/penumbra-overture/RadioHandler.h"
#include "hpl1/penumbra-overture/SaveHandler.h"

//////////////////////////////////////////////////////////////////////////
// UNDERWATER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEffect_Underwater::cEffect_Underwater(cInit *apInit, cGraphicsDrawer *apDrawer) {
	mpInit = apInit;
	mpDrawer = apDrawer;

	mpWhiteGfx = mpDrawer->CreateGfxObject("effect_white.jpg", "smoke2d");

	Reset();
}
//-----------------------------------------------------------------------

cEffect_Underwater::~cEffect_Underwater() {
}

//-----------------------------------------------------------------------

void cEffect_Underwater::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;
}

//-----------------------------------------------------------------------

void cEffect_Underwater::Update(float afTimeStep) {
}
//-----------------------------------------------------------------------

void cEffect_Underwater::OnDraw() {
	if (mbActive == false)
		return;

	cColor invColor(1 - mColor.r, 1 - mColor.g, 1 - mColor.b, 0);
	mpDrawer->DrawGfxObject(mpWhiteGfx, 0, cVector2f(800, 600), invColor);
}
//-----------------------------------------------------------------------

void cEffect_Underwater::Reset() {
	mbActive = false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SHAKE SCREEN
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEffect_ShakeScreen::cEffect_ShakeScreen(cInit *apInit) {
	mpInit = apInit;
}

cEffect_ShakeScreen::~cEffect_ShakeScreen() {
}

void cEffect_ShakeScreen::Start(float afAmount, float afTime, float afFadeInTime, float afFadeOutTime) {
	cEffect_ShakeScreen_Shake shake;
	shake.mfSize = afAmount;
	shake.mfMaxSize = afAmount;
	shake.mfTime = afTime;
	shake.mfFadeInTime = afFadeInTime;
	shake.mfMaxFadeInTime = afFadeInTime;
	shake.mfFadeOutTime = afFadeOutTime;
	shake.mfMaxFadeOutTime = afFadeOutTime;

	mlstShakes.push_back(shake);
}

void cEffect_ShakeScreen::Update(float afTimeStep) {
	float fLargest = 0;

	Common::List<cEffect_ShakeScreen_Shake>::iterator it = mlstShakes.begin();
	for (; it != mlstShakes.end();) {
		cEffect_ShakeScreen_Shake &shake = *it;

		if (shake.mfFadeInTime > 0) {
			shake.mfFadeInTime -= afTimeStep;
			if (shake.mfFadeInTime < 0)
				shake.mfFadeInTime = 0;
			float fT = shake.mfFadeInTime / shake.mfMaxFadeInTime;
			shake.mfSize = (1 - fT) * shake.mfMaxSize;
		} else if (shake.mfTime > 0) {
			shake.mfTime -= afTimeStep;
			if (shake.mfTime < 0)
				shake.mfTime = 0;
			shake.mfSize = shake.mfMaxSize;
		} else {
			shake.mfFadeOutTime -= afTimeStep;
			if (shake.mfFadeOutTime < 0)
				shake.mfFadeOutTime = 0;
			float fT = shake.mfFadeOutTime / shake.mfMaxFadeOutTime;
			shake.mfSize = fT * shake.mfMaxSize;
		}

		// Log("%f, %f, %f size: %f\n",shake.mfFadeInTime,shake.mfTime,shake.mfFadeOutTime,shake.mfSize);

		if (fLargest < shake.mfSize)
			fLargest = shake.mfSize;

		if (shake.mfTime <= 0 && shake.mfFadeOutTime <= 0 && shake.mfFadeInTime <= 0) {
			it = mlstShakes.erase(it);
		} else {
			++it;
		}
	}

	mvAdd.x = cMath::RandRectf(-fLargest, fLargest);
	mvAdd.y = cMath::RandRectf(-fLargest, fLargest);
	mvAdd.z = cMath::RandRectf(-fLargest, fLargest);
}

void cEffect_ShakeScreen::Reset() {
	mlstShakes.clear();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE EFFECT
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEffect_SaveEffect::cEffect_SaveEffect(cInit *apInit, cGraphicsDrawer *apDrawer) {
	mpInit = apInit;
	mpDrawer = apDrawer;

	mpFlashGfx = mpDrawer->CreateGfxObject("effect_white.jpg", "diffalpha2d");

	Reset();
}
cEffect_SaveEffect::~cEffect_SaveEffect() {
}

void cEffect_SaveEffect::NormalSave(const cVector3f &avPos, cGameSaveArea *apSaveArea) {
	mpSaveArea = apSaveArea;

	msMessage = _W("");
	if (apSaveArea->GetMessageCat() != "") {
		msMessage = kTranslate(apSaveArea->GetMessageCat(), apSaveArea->GetMessageEntry());
		if (apSaveArea->GetHasBeenUsed())
			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("horror_save", false, 1);
		else
			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("horror_flashback1", false, 1);
	} else {
		mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("horror_save", false, 1);
	}

	mvPosition = avPos;
	mbActive = true;
	mfTime = 0;

	mfStartFov = mpInit->mpPlayer->GetCamera()->GetFOV();
	mfFov = mpInit->mpPlayer->GetCamera()->GetFOV();

	mpInit->mpPlayer->SetActive(false);
	mpInit->mpPlayer->GetLookAt()->SetActive(true);
	mpInit->mpPlayer->GetLookAt()->SetTarget(avPos, 2.1f, 4);

	mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailActive(true);
	mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailAmount(0.8f);

	mfFlashAlpha = 0;
	mFlashColor = cColor(216.0f / 255.0f,
						 85.0f / 255.0f,
						 5.0f / 255.0f, 0);

	mlState = 0;

	mbAutoSave = false;
}

void cEffect_SaveEffect::AutoSave() {
	mbActive = true;
	mfTime = 0;

	// mpInit->mpPlayer->SetActive(false);

	mfFlashAlpha = 0;
	mFlashColor = cColor(216.0f / 255.0f,
						 85.0f / 255.0f,
						 5.0f / 255.0f, 0);

	mlState = 0;

	mbAutoSave = true;
}

void cEffect_SaveEffect::Update(float afTimeStep) {
	if (mbActive == false)
		return;

	mfTime += afTimeStep;

	if (mbAutoSave) {
		AutoSaveUpdate(afTimeStep);
	} else {
		NormalSaveUpdate(afTimeStep);
	}
}

void cEffect_SaveEffect::OnDraw() {
	if (mbActive == false)
		return;

	mpDrawer->DrawGfxObject(mpFlashGfx, 0, cVector2f(800, 600),
							cColor(1, 1) * mfFlashAlpha +
								mFlashColor * (1 - mfFlashAlpha));
}

void cEffect_SaveEffect::Reset() {
	mbActive = false;
}

//-----------------------------------------------------------------------

void cEffect_SaveEffect::NormalSaveUpdate(float afTimeStep) {
	switch (mlState) {
		///////////////////////
		// State 0
	case 0: {
		// Flash
		mfFlashAlpha += 0.5f * afTimeStep;
		if (mfFlashAlpha > 1.0f)
			mfFlashAlpha = 1.0f;

		// Fov
		mfFov -= (mfFov - 0.2f) * afTimeStep * 1.3f;
		mpInit->mpPlayer->GetCamera()->SetFOV(mfFov);

		if (mfTime > 3.0f) {
			mlState++;
			mpInit->mpPlayer->GetCamera()->SetFOV(mfStartFov);
		}
		break;
	}
		///////////////////////
		// State 1
	case 1: {
		if (msMessage != _W("") && mpSaveArea->GetHasBeenUsed() == false) {
			mpInit->mpGameMessageHandler->SetBlackText(true);
			mpInit->mpGameMessageHandler->Add(msMessage);
		}
		mlState++;
		break;
	}
		///////////////////////
		// State 2
	case 2: {
		if (mpInit->mpGameMessageHandler->HasMessage() == false) {
			mlState++;
		}
		break;
	}
		///////////////////////
		// State3
	case 3: {
		// Flash
		mfFlashAlpha -= 0.6f * afTimeStep;
		if (mfFlashAlpha < 0.0f) {
			mfFlashAlpha = 0.0f;
			mlState++;
		}

		break;
	}
	case 4: {
		mfFlashAlpha = 0.f;
		mFlashColor = cColor(0.f, 0.f);
		mlState++;
		break;
	}
	case 5: {
		/////////////
		// Reset all
		mbActive = false;
		mpInit->mpPlayer->SetActive(true);
		mpInit->mpPlayer->GetLookAt()->SetActive(false);
		mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailActive(false);

		/////////////
		// Display message
		tString sEntry = "AfterSave_Default";
		if (mpSaveArea->GetHasBeenUsed() == false) {
			mpInit->mpPlayer->mlStat_NumOfSaves++;
			int lNum = mpInit->mpPlayer->mlStat_NumOfSaves;
			if (lNum > 10)
				lNum = 10;
			if (lNum < 10)
				sEntry = "AfterSave_0" + cString::ToString(lNum);
			else
				sEntry = "AfterSave_" + cString::ToString(lNum);

			mpSaveArea->SetHasBeenUsed(true);
		}

		mpInit->mpGameMessageHandler->SetBlackText(false);
		mpInit->mpGameMessageHandler->Add(kTranslate("Save", sEntry));

		mpInit->mpSaveHandler->AutoSave(_W("spot"), 10);
		break;
	}
	}
}

//-----------------------------------------------------------------------

void cEffect_SaveEffect::AutoSaveUpdate(float afTimeStep) {
	switch (mlState) {
		///////////////////////
		// State 0
	case 0: {
		// Flash
		mfFlashAlpha += 1.2f * afTimeStep;
		if (mfFlashAlpha > 0.75f) {
			mfFlashAlpha = 0.75f;
			mlState++;
			mpInit->mpPlayer->SetActive(true);
		}
		break;
	}
		///////////////////////
		// State 1
	case 1: {
		// Flash
		mfFlashAlpha -= 0.8f * afTimeStep;
		if (mfFlashAlpha < 0.0f) {
			mfFlashAlpha = 0.0f;
			mlState++;
		}

		break;
	}
		///////////////////////
		// State2
	case 2: {
		mlState++;
		mFlashColor = cColor(0.0, 0.0);
		mfFlashAlpha = 0.f;
		break;
	}
	case 3: {
		Reset();
		mpInit->mpSaveHandler->AutoSave(_W("auto"), 5);
		break;
	}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// DEPTH OF FIELD
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEffect_DepthOfField::cEffect_DepthOfField(cInit *apInit) {
	mbDisabled = false;
	mpInit = apInit;
	mpPostEffects = apInit->mpGame->GetGraphics()->GetRendererPostEffects();

	Reset();
}

cEffect_DepthOfField::~cEffect_DepthOfField() {
}

void cEffect_DepthOfField::Reset() {
	mbActive = false;
	mfMaxBlur = 0;
	mpFocusBody = NULL;
}

void cEffect_DepthOfField::SetDisabled(bool abX) {
	mbDisabled = abX;

	if (mbDisabled) {
		mpPostEffects->SetDepthOfFieldActive(false);
	} else if (mbActive) {
		mpPostEffects->SetDepthOfFieldActive(true);
	}
}

void cEffect_DepthOfField::SetActive(bool abX, float afFadeTime) {
	mbActive = abX;
	if (mbDisabled)
		return;

	if (mbActive)
		mpPostEffects->SetDepthOfFieldActive(true);

	if (afFadeTime > 0)
		mfFadeSpeed = 1 / afFadeTime;
	else
		mfFadeSpeed = 100000.0f;

	Update(1.0f / 60.0f);
}

void cEffect_DepthOfField::SetUp(float afNearPlane, float afFocalPlane, float afFarPlane) {
	mpPostEffects->SetDepthOfFieldNearPlane(afNearPlane);
	mpPostEffects->SetDepthOfFieldFocalPlane(afFocalPlane);
	mpPostEffects->SetDepthOfFieldFarPlane(afFarPlane);
}

void cEffect_DepthOfField::Update(float afTimeStep) {
	/////////////////////////////////
	// Update focus to body
	if (mpFocusBody && mfMaxBlur > 0) {
		FocusOnBody(mpFocusBody);
	}

	///////////////////////////
	// Update max blur
	if (mbActive) {
		mfMaxBlur += afTimeStep * mfFadeSpeed;
		if (mfMaxBlur > 1)
			mfMaxBlur = 1;
	} else if (mfMaxBlur > 0) {
		mfMaxBlur -= afTimeStep * mfFadeSpeed;
		if (mfMaxBlur < 0) {
			mfMaxBlur = 0;
			mpPostEffects->SetDepthOfFieldActive(false);
		}
	}

	mpPostEffects->SetDepthOfFieldMaxBlur(mfMaxBlur);
}

//-----------------------------------------------------------------------

void cEffect_DepthOfField::FocusOnBody(iPhysicsBody *apBody) {
	cBoundingVolume *pBV = apBody->GetBV();
	cVector3f vCamPos = mpInit->mpPlayer->GetCamera()->GetPosition();

	// Focal plane
	float fFocalPlane = cMath::Vector3Dist(pBV->GetWorldCenter(), vCamPos);

	// Near plane
	float fNearPlane = fFocalPlane - (pBV->GetRadius() + 0.3f);
	if (fNearPlane < 0)
		fNearPlane = 0;

	// Far plane
	float fFarPlane = fFocalPlane + (pBV->GetRadius() + 0.3f);

	// float fDist = cMath::Vector3Dist(vCamPos,apBody->GetWorldPosition());
	// Log("Body: %s Dist: %f PickedDist: %f\n",apBody->GetName().c_str(), fDist,mpInit->mpPlayer->GetPickedDist());
	// Log("Setup near %f focal %f far %f Radii %f\n",fNearPlane,fFocalPlane,fFarPlane,pBV->GetRadius());

	SetUp(fNearPlane, fFocalPlane, fFarPlane);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// FLASH
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEffect_Flash::cEffect_Flash(cInit *apInit, cGraphicsDrawer *apDrawer) {
	mpInit = apInit;
	mpDrawer = apDrawer;

	mpWhiteGfx = mpDrawer->CreateGfxObject("effect_white.jpg", "diffadditive2d");

	Reset();
}
cEffect_Flash::~cEffect_Flash() {
	mpDrawer->DestroyGfxObject(mpWhiteGfx);
}

//-----------------------------------------------------------------------

void cEffect_Flash::Start(float afFadeIn, float afWhite, float afFadeOut) {
	mbActive = true;

	mlStep = 0;

	mfFadeInSpeed = 1 / afFadeIn;
	mfWhiteSpeed = 1 / afWhite;
	mfFadeOutSpeed = 1 / afFadeOut;
}

//-----------------------------------------------------------------------

void cEffect_Flash::Update(float afTimeStep) {
	if (mbActive == false)
		return;

	if (mlStep == 0) {
		mfAlpha += mfFadeInSpeed * afTimeStep;
		if (mfAlpha >= 1.0f) {
			mfAlpha = 1.0f;
			mlStep = 1;
			mfCount = 1;
		}
	} else if (mlStep == 1) {
		mfCount -= mfWhiteSpeed * afTimeStep;
		if (mfCount <= 0) {
			mlStep = 2;
		}
	} else if (mlStep == 2) {
		mfAlpha -= mfFadeOutSpeed * afTimeStep;
		if (mfAlpha <= 0.0f) {
			mbActive = false;
		}
	}
}

//-----------------------------------------------------------------------

void cEffect_Flash::OnDraw() {
	if (mbActive == false)
		return;

	mpDrawer->DrawGfxObject(mpWhiteGfx, 0, cVector2f(800, 600), cColor(1, mfAlpha));
}

//-----------------------------------------------------------------------

void cEffect_Flash::Reset() {
	mbActive = false;
}

//////////////////////////////////////////////////////////////////////////
// SUB TITLE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
cEffect_SubTitle::cEffect_SubTitle(cInit *apInit, cGraphicsDrawer *apDrawer) {
	mpInit = apInit;
	mpDrawer = apDrawer;

	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");
}
cEffect_SubTitle::~cEffect_SubTitle() {
}

void cEffect_SubTitle::Add(const tWString &asMessage, float afTime, bool abRemovePrevious) {
	if (abRemovePrevious) {
		tSubTitleListIt it = mlstSubTitles.begin();
		for (; it != mlstSubTitles.end();) {
			cSubTitle &subTitle = *it;

			if (subTitle.mbActive) {
				subTitle.mfCount = 0;
				++it;
			} else {
				it = mlstSubTitles.erase(it);
			}
		}
	}

	cSubTitle subTitle;
	subTitle.mfAlpha = 0.0f;
	subTitle.mfCount = afTime;
	subTitle.msMessage = asMessage;
	subTitle.mbActive = false;

	mlstSubTitles.push_back(subTitle);
}

void cEffect_SubTitle::Update(float afTimeStep) {
	bool bFoundFirst = false;

	tSubTitleListIt it = mlstSubTitles.begin();
	for (; it != mlstSubTitles.end();) {
		cSubTitle &subTitle = *it;

		if (subTitle.mbActive) {
			if (subTitle.mfCount > 0) {
				bFoundFirst = true;

				subTitle.mfCount -= afTimeStep;

				subTitle.mfAlpha += afTimeStep * 0.9f;
				if (subTitle.mfAlpha > 1)
					subTitle.mfAlpha = 1;
			} else {
				subTitle.mfAlpha -= afTimeStep * 0.9f;
				if (subTitle.mfAlpha <= 0) {
					it = mlstSubTitles.erase(it);
					continue;
				}
			}
		} else if (bFoundFirst == false) {
			subTitle.mbActive = true;
			bFoundFirst = true;
		}

		++it;
	}
}
void cEffect_SubTitle::OnDraw() {
	if (mpInit->mpRadioHandler->IsActive() || mpInit->mbSubtitles == false)
		return;

	tSubTitleListIt it = mlstSubTitles.begin();
	for (; it != mlstSubTitles.end(); ++it) {
		cSubTitle &subTitle = *it;

		float fAlpha = subTitle.mfAlpha * (1 - mpInit->mpInventory->GetAlpha());

		if (subTitle.mbActive) {
			mpFont->drawWordWrap(cVector3f(25, 500, 47), 750, 16, 15, cColor(1, fAlpha),
								 eFontAlign_Left, subTitle.msMessage);
			mpFont->drawWordWrap(cVector3f(25, 500, 46) + cVector3f(1, 1, 0), 750, 16, 15, cColor(0, fAlpha),
								 eFontAlign_Left, subTitle.msMessage);
			mpFont->drawWordWrap(cVector3f(25, 500, 46) + cVector3f(-1, -1, 0), 750, 16, 15, cColor(0, fAlpha),
								 eFontAlign_Left, subTitle.msMessage);
		}
	}
}
void cEffect_SubTitle::Reset() {
	mlstSubTitles.clear();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// WAVE GRAVITY
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEffect_WaveGravity::cEffect_WaveGravity(cInit *apInit) {
	mpInit = apInit;

	Reset();
}
cEffect_WaveGravity::~cEffect_WaveGravity() {
}

//-----------------------------------------------------------------------

void cEffect_WaveGravity::SetActive(bool abX) {
	mbActive = abX;
}

void cEffect_WaveGravity::Setup(float afMaxAngle, float afSwingLength, float afGravitySize, int alDir) {
	mfMaxAngle = afMaxAngle;
	mfSwingLength = afSwingLength;
	mfSize = afGravitySize;
	mlDir = alDir;
}

void cEffect_WaveGravity::Update(float afTimeStep) {
	if (mbActive == false)
		return;

	iPhysicsWorld *pWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	//////////////////////////////////
	// Set all bodies to not active

	cPhysicsBodyIterator it = pWorld->GetBodyIterator();
	while (it.HasNext()) {
		iPhysicsBody *pBody = it.Next();

		// quick fix for oscillation
		if (pBody->GetJointNum() > 0 && pBody->GetJoint(0)->GetLimitAutoSleep())
			continue;

		if (pBody->GetMass() != 0) {
			pBody->SetEnabled(true);
		}
	}

	/////////////////////////////////
	// Update gravity
	mfTime += (k2Pif / mfSwingLength) * afTimeStep;

	float afAngle = mfMaxAngle * sin(mfTime);

	cVector3f vDir(0, 0, 0);
	vDir.y = -cos(afAngle);
	if (mlDir == 0)
		vDir.x = sin(afAngle);
	else
		vDir.z = sin(afAngle);

	vDir = vDir * mfSize;

	pWorld->SetGravity(vDir);
}

void cEffect_WaveGravity::Reset() {
	mbActive = false;

	mfMaxAngle = 0;
	mfSwingLength = 1;
	mfSize = 9.8f;

	mfTime = 0;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEffectHandler::cEffectHandler(cInit *apInit) : iUpdateable("EffectHandler") {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	mpFlash = hplNew(cEffect_Flash, (mpInit, mpDrawer));
	mpWaveGravity = hplNew(cEffect_WaveGravity, (mpInit));
	mpSubTitle = hplNew(cEffect_SubTitle, (mpInit, mpDrawer));
	mpDepthOfField = hplNew(cEffect_DepthOfField, (mpInit));
	mpSaveEffect = hplNew(cEffect_SaveEffect, (mpInit, mpDrawer));
	mpShakeScreen = hplNew(cEffect_ShakeScreen, (mpInit));
	mpUnderwater = hplNew(cEffect_Underwater, (mpInit, mpDrawer));

	Reset();
}

//-----------------------------------------------------------------------

cEffectHandler::~cEffectHandler(void) {
	hplDelete(mpFlash);
	hplDelete(mpWaveGravity);
	hplDelete(mpSubTitle);
	hplDelete(mpDepthOfField);
	hplDelete(mpSaveEffect);
	hplDelete(mpShakeScreen);
	hplDelete(mpUnderwater);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cEffectHandler::OnStart() {
}

//-----------------------------------------------------------------------

void cEffectHandler::Update(float afTimeStep) {
	mpFlash->Update(afTimeStep);
	mpWaveGravity->Update(afTimeStep);
	mpSubTitle->Update(afTimeStep);
	mpDepthOfField->Update(afTimeStep);
	mpSaveEffect->Update(afTimeStep);
	mpShakeScreen->Update(afTimeStep);
	mpUnderwater->Update(afTimeStep);
}

//-----------------------------------------------------------------------

void cEffectHandler::Reset() {
	mpFlash->Reset();
	mpWaveGravity->Reset();
	mpSubTitle->Reset();
	mpDepthOfField->Reset();
	mpSaveEffect->Reset();
	mpShakeScreen->Reset();
	mpUnderwater->Reset();
}

//-----------------------------------------------------------------------

void cEffectHandler::OnDraw() {
	mpFlash->OnDraw();
	mpSubTitle->OnDraw();
	mpSaveEffect->OnDraw();
	mpUnderwater->OnDraw();
}

//-----------------------------------------------------------------------
