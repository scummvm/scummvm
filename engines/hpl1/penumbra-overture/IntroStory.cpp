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

#include "hpl1/penumbra-overture/IntroStory.h"

#include "hpl1/penumbra-overture/ButtonHandler.h"
#include "hpl1/penumbra-overture/GraphicsHelper.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"

/////////////////////////
// Image 00
#define kIntro_Image00_Start 0
#define kIntro_Image00_Mess2 1

/////////////////////////
// Image 01
#define kIntro_Image01_Start 100
#define kIntro_Image01_Mess2 101
#define kIntro_Image01_Mess3 102
#define kIntro_Image01_Mess4 103

/////////////////////////
// Image 02
#define kIntro_Image02_Start 200
#define kIntro_Image02_Mess2 201
#define kIntro_Image02_Mess3 202
#define kIntro_Image02_Mess4 203
#define kIntro_Image02_Mess5 204

/////////////////////////
// Image 03
#define kIntro_Image03_Start 300
#define kIntro_Image03_Mess2 301
#define kIntro_Image03_Mess3 302
#define kIntro_Image03_Mess4 303
#define kIntro_Image03_Mess5 304

/////////////////////////
// Image 04
#define kIntro_Image04_Start 400
#define kIntro_Image04_Mess2 401

////////////////////////
// Image 05
#define kIntro_Image05_Start 500
#define kIntro_Image05_Mess2 501
#define kIntro_Image05_Mess3 502
#define kIntro_Image05_Mess4 503

//////////////////////////////////////////////////////////////////////////
// INTRO IMAGE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cIntroImage::cIntroImage() {
	mvCameraPosition = cVector3f(0, 0, 1);

	mpTexture = NULL;

	mvVtxVec.resize(4);

	mlMaxPrevPos = 35;

	Reset();
}

cIntroImage::~cIntroImage() {
}

//-----------------------------------------------------------------------

void cIntroImage::MoveTo(const cVector3f &avPos, float afTime) {
	if (afTime == 0) {
		mvCameraPosition = avPos;
		mvPosStep = 0;
		mvFinalPos = avPos;
	} else {
		mfPosCount = afTime;
		mvFinalPos = avPos;
		mvPosStep = (avPos - mvCameraPosition) / afTime;
	}
}

//-----------------------------------------------------------------------

void cIntroImage::FadeBrightnessTo(float afBrightness, float afTime) {
	if (afTime == 0) {
		mfFinalBrightness = afBrightness;
		mfBrightness = afBrightness;
		mfBrightnessStep = 0;
	} else {
		mfFinalBrightness = afBrightness;
		mfBrightnessStep = (mfFinalBrightness - mfBrightness) / afTime;
	}
}

//-----------------------------------------------------------------------

void cIntroImage::FadeAlphaTo(float afAlpha, float afTime) {
	if (afTime == 0) {
		mfFinalAlpha = afAlpha;
		mfAlpha = afAlpha;
		mfAlphaStep = 0;
		mfPosCount = 0;
	} else {
		mfFinalAlpha = afAlpha;
		mfAlphaStep = (mfFinalAlpha - mfAlpha) / afTime;
	}
}

//-----------------------------------------------------------------------

void cIntroImage::Reset() {
	mbActive = false;

	mfBrightness = 1;
	mfBrightnessStep = 0;
	mfFinalBrightness = 1;

	mfAlpha = 1;
	mfAlphaStep = 0;
	mfFinalAlpha = 1;

	mvCameraPosition = 0;
	mvFinalPos = 0;
	mvPosStep = 0;
	mfPosCount = 0;

	mvPosDistMul = 1;

	mlstPrevPos.clear();
}

//-----------------------------------------------------------------------

void cIntroImage::Update(float afTimeStep) {
	/////////////////////////////////
	// Position
	if (mvCameraPosition != mvFinalPos) {
		cVector3f vStep = mvPosStep;
		cVector3f vDist = mvFinalPos - mvCameraPosition;

		if (mvPosDistMul.x > 0)
			vStep.x = cMath::Min(vStep.x, vDist.x * mvPosDistMul.x);
		if (mvPosDistMul.y > 0)
			vStep.y = cMath::Min(vStep.y, vDist.y * mvPosDistMul.y);
		if (mvPosDistMul.z > 0)
			vStep.z = cMath::Min(vStep.z, vDist.z * mvPosDistMul.z);

		mvCameraPosition += vStep * afTimeStep;

		// X
		if (mvPosStep.x > 0 && mvCameraPosition.x > mvFinalPos.x)
			mvCameraPosition.x = mvFinalPos.x;
		if (mvPosStep.x < 0 && mvCameraPosition.x < mvFinalPos.x)
			mvCameraPosition.x = mvFinalPos.x;

		// Y
		if (mvPosStep.y > 0 && mvCameraPosition.y > mvFinalPos.y)
			mvCameraPosition.y = mvFinalPos.y;
		if (mvPosStep.y < 0 && mvCameraPosition.y < mvFinalPos.y)
			mvCameraPosition.y = mvFinalPos.y;

		// Z
		if (mvPosStep.z > 0 && mvCameraPosition.z > mvFinalPos.z)
			mvCameraPosition.z = mvFinalPos.z;
		if (mvPosStep.z < 0 && mvCameraPosition.z < mvFinalPos.z)
			mvCameraPosition.z = mvFinalPos.z;

		mlstPrevPos.push_back(mvCameraPosition);
		if (mlstPrevPos.size() > 14) {
			mlstPrevPos.pop_front();
		}
	}

	/////////////////////////////////
	// Brightness
	if (mfBrightness != mfFinalBrightness) {
		mfBrightness += mfBrightnessStep * afTimeStep;
		if (mfBrightnessStep > 0 && mfBrightness >= mfFinalBrightness) {
			mfBrightness = mfFinalBrightness;
		}
		if (mfBrightnessStep < 0 && mfBrightness <= mfFinalBrightness) {
			mfBrightness = mfFinalBrightness;
		}
	}

	/////////////////////////////////
	// Alpha
	if (mfAlpha != mfFinalAlpha) {
		mfAlpha += mfAlphaStep * afTimeStep;
		if (mfAlphaStep > 0 && mfAlpha >= mfFinalAlpha) {
			mfAlpha = mfFinalAlpha;
		}
		if (mfAlphaStep < 0 && mfAlpha <= mfFinalAlpha) {
			mfAlpha = mfFinalAlpha;
		}
	}
}

//-----------------------------------------------------------------------

void cIntroImage::OnDraw() {
	cVector3f vCamDrawPos;

	if (mlstPrevPos.empty()) {
		vCamDrawPos = mvCameraPosition;
	} else {
		vCamDrawPos = cVector3f(0, 0, 0);
		tVector3fListIt it = mlstPrevPos.begin();
		for (; it != mlstPrevPos.end(); ++it) {
			vCamDrawPos += *it;
		}

		vCamDrawPos = vCamDrawPos / (float)mlstPrevPos.size();
	}

	/////////////////////////////////
	// Set up position variables
	cVector3f vPos = cVector3f(-vCamDrawPos.x, -vCamDrawPos.y, 0);

	float fAlpha = mfAlpha;
	float fBrightness = cMath::Min(1.0f, mfBrightness);
	cVector2f vSize(1024, 784);
	vSize = vSize * (1 / vCamDrawPos.z);
	vPos = vPos * (1 / vCamDrawPos.z) + cVector3f(400, 300, 0);
	;

	///////////////////////////
	// Set up vertexes
	mvVtxVec[0] = cVertex(vPos + cVector3f(0, 0, 0),
						  cVector2f(0, 0), cColor(fBrightness, fAlpha));

	mvVtxVec[1] = cVertex(vPos + cVector3f(vSize.x, 0, 40),
						  cVector2f(1, 0), cColor(fBrightness, fAlpha));

	mvVtxVec[2] = cVertex(vPos + cVector3f(vSize.x, vSize.y, 40),
						  cVector2f(1, 1), cColor(fBrightness, fAlpha));

	mvVtxVec[3] = cVertex(vPos + cVector3f(0, vSize.y, 40),
						  cVector2f(0, 1), cColor(fBrightness, fAlpha));

	//////////////////////////
	/// Draw
	mpLowGfx->SetTexture(0, mpTexture);
	mpLowGfx->SetBlendActive(true);
	mpLowGfx->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_OneMinusSrcAlpha);

	mpLowGfx->DrawQuad(mvVtxVec);

	if (mfBrightness > 1) {
		float fWhite = mfBrightness - 1;

		mvVtxVec[0] = cVertex(cVector3f(0, 0, 0), cVector2f(0, 0), cColor(fWhite, 1));
		mvVtxVec[1] = cVertex(cVector3f(800, 0, 40), cVector2f(1, 0), cColor(fWhite, 1));
		mvVtxVec[2] = cVertex(cVector3f(800, 600, 40), cVector2f(1, 1), cColor(fWhite, 1));
		mvVtxVec[3] = cVertex(cVector3f(0, 600, 40), cVector2f(0, 1), cColor(fWhite, 1));

		mpLowGfx->SetTexture(0, NULL);

		mpLowGfx->SetBlendFunc(eBlendFunc_One, eBlendFunc_One);

		mpLowGfx->DrawQuad(mvVtxVec);
	}
	mpLowGfx->SetBlendActive(false);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cIntroStory::cIntroStory(cInit *apInit) : iUpdateable("StoryIntro") {
	mpInit = apInit;
	mpLowGfx = mpInit->mpGame->GetGraphics()->GetLowLevel();
	mpTexManager = mpInit->mpGame->GetResources()->GetTextureManager();
	mpSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

	for (int i = 0; i < INTRO_IMAGE_NUM; ++i) {
		mvImages[i].mpLowGfx = mpLowGfx;
		mvImages[i].mpTexManager = mpTexManager;
		mvImages[i].mpInit = mpInit;
		mvImages[i].mpTexture = NULL;
	}

	mpBlackTexture = NULL;

	// Load font
	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("font_computer.fnt");

	mfVoiceVol = 0.9f;

	Reset();
}

//-----------------------------------------------------------------------

cIntroStory::~cIntroStory(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cIntroStory::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;

	msCentreText = _W("");
	msMessage = _W("");

	if (mbActive) {
		mpInit->mpGame->GetUpdater()->SetContainer("Intro");
		mpInit->mpGame->GetScene()->SetDrawScene(false);
		mLastButtonState = mpInit->mpButtonHandler->GetState();
		mpInit->mpButtonHandler->ChangeState(eButtonHandlerState_Intro);

		//////////////////////////////
		// LoadImages
		for (int i = 0; i < INTRO_IMAGE_NUM; ++i) {
			mvImages[i].mpTexture = mpTexManager->Create2D("intro_image0" + cString::ToString(i + 1), false);
			if (mvImages[i].mpTexture == NULL) {
				FatalError("Couldn't load an intro image!\n");
			}
			mvImages[i].mbActive = false;
			mvImages[i].mlstPrevPos.clear();
		}

		mpBlackTexture = mpTexManager->Create2D("effect_black.bmp", false);

		mfTimerCount = 0.01f;
		mlNextStop = kIntro_Image00_Start;
		Update(1);

		mpInit->mpGame->ResetLogicTimer();
	} else {
		// mpInit->mpGame->GetUpdater()->SetContainer("Default");
		// mpInit->mpGame->GetScene()->SetDrawScene(true);
		// mpInit->mpButtonHandler->ChangeState(mLastButtonState);

		// Destroy images
		for (int i = 0; i < INTRO_IMAGE_NUM; ++i) {
			if (mvImages[i].mpTexture)
				mpTexManager->Destroy(mvImages[i].mpTexture);
			mvImages[i].mpTexture = NULL;
			mvImages[i].mbActive = false;
		}

		mpTexManager->Destroy(mpBlackTexture);
		mpBlackTexture = NULL;
	}
}

//-----------------------------------------------------------------------

void cIntroStory::Update(float afTimeStep) {
	//////////////////////////////////////
	/// Update Timer
	if (mfTimerCount > 0) {
		mfTimerCount -= afTimeStep;

		/// Update the image specific stuff
		if (mfTimerCount <= 0) {
			Image00();
			if (mbSearchNext)
				Image01();
			if (mbSearchNext)
				Image02();
			if (mbSearchNext)
				Image03();
			if (mbSearchNext)
				Image04();
			if (mbSearchNext)
				Image05();
		}
	}

	//////////////////////////////////////
	/// Update the image specific stuff
	for (int i = 0; i < INTRO_IMAGE_NUM; ++i) {
		if (mvImages[i].mbActive)
			mvImages[i].Update(afTimeStep);
	}
}

//-----------------------------------------------------------------------

void cIntroStory::Reset() {
	mbActive = false;

	mLastButtonState = eButtonHandlerState_Game;

	for (int i = 0; i < INTRO_IMAGE_NUM; ++i)
		mvImages[i].Reset();
}

//-----------------------------------------------------------------------

void cIntroStory::OnDraw() {
	cVector3f vPos = cVector3f(15, 526, 10);
	cVector2f vSize = 16;

	if (msCentreText != _W("")) {
		float fAlpha = mvImages[5].mfBrightness;
		mpFont->Draw(cVector3f(400, 300, 2), 18, cColor(1, 1, 1, fAlpha),
					 eFontAlign_Center, msCentreText.c_str());
		mpFont->Draw(cVector3f(400 + 1, 300 + 1, 1), 18, cColor(0, fAlpha),
					 eFontAlign_Center, msCentreText.c_str());
		mpFont->Draw(cVector3f(400 - 1, 300 - 1, 1), 18, cColor(0, fAlpha),
					 eFontAlign_Center, msCentreText.c_str());
		mpFont->Draw(cVector3f(400 - 1, 300 + 1, 1), 18, cColor(0, fAlpha),
					 eFontAlign_Center, msCentreText.c_str());
		mpFont->Draw(cVector3f(400 + 1, 300 - 1, 1), 18, cColor(0, fAlpha),
					 eFontAlign_Center, msCentreText.c_str());
	}

	// mpFont->DrawWordWrap(vPos + cVector3f(3,3,-1),760,21,vSize,cColor(1,0),eFontAlign_Left,msMessage);
	// mpFont->DrawWordWrap(vPos + cVector3f(-2,-2,-1),760,21,vSize,cColor(1,0),eFontAlign_Left,msMessage);

	if (mpInit->mbSubtitles) {
		mpFont->DrawWordWrap(vPos, 760, 18, vSize, cColor(1, 1, 1, 1), eFontAlign_Left, msMessage);
	}
}

//-----------------------------------------------------------------------

void cIntroStory::OnPostSceneDraw() {
	mpLowGfx->SetClearColor(cColor(0, 0, 0, 0));
	mpLowGfx->ClearScreen();

	mpLowGfx->SetDepthTestActive(false);
	mpLowGfx->PushMatrix(eMatrix_ModelView);
	mpLowGfx->SetIdentityMatrix(eMatrix_ModelView);
	mpLowGfx->SetOrthoProjection(mpLowGfx->GetVirtualSize(), -1000, 1000);

	for (int i = 0; i < INTRO_IMAGE_NUM; ++i) {
		if (mvImages[i].mbActive)
			mvImages[i].OnDraw();
	}

	mpInit->mpGraphicsHelper->DrawTexture(mpBlackTexture, cVector3f(0, 0, 140), cVector2f(800, 75), cColor(1, 1));
	mpInit->mpGraphicsHelper->DrawTexture(mpBlackTexture, cVector3f(0, 525, 140), cVector2f(800, 75), cColor(1, 1));

	mpLowGfx->PopMatrix(eMatrix_ModelView);
}

//-----------------------------------------------------------------------

void cIntroStory::SetMessage(const tWString &asMess) {
	msMessage = asMess;
}

//-----------------------------------------------------------------------

void cIntroStory::Exit() {
	mpInit->mpGame->GetSound()->GetMusicHandler()->Stop(0.3f);
	mpSoundHandler->StopAll(eSoundDest_Gui);

	SetActive(false);

	// mpInit->mpGame->Exit();
	// return;

	mpInit->mpGraphicsHelper->DrawLoadingScreen("");

	mpInit->mpGame->GetUpdater()->SetContainer("Default");
	mpInit->mpGame->GetScene()->SetDrawScene(true);
	mpInit->mpButtonHandler->ChangeState(mLastButtonState);

	mpInit->mpMapHandler->Load(mpInit->msStartMap, mpInit->msStartLink);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
void cIntroStory::Image00() {
	mbSearchNext = false;

	switch (mlNextStop) {
		//////////////////////
		// Start
	case kIntro_Image00_Start: {
		mvImages[5].mbActive = true;
		mvImages[5].FadeBrightnessTo(0, 0);
		mvImages[5].FadeBrightnessTo(1, 5);
		mvImages[5].mvCameraPosition = cVector3f(512, 384, 1.9f);
		mvImages[5].mvPosDistMul = 0.1f;

		mvImages[5].MoveTo(cVector3f(512, 384, 1.1f), 15);

		msCentreText = kTranslate("Intro", "00_01");

		mfTimerCount = 5;
		mlNextStop = kIntro_Image00_Mess2;

		break;
	}
	case kIntro_Image00_Mess2: {
		mvImages[5].FadeBrightnessTo(0, 5);
		mfTimerCount = 5;
		mlNextStop = kIntro_Image01_Start;
		break;
	}
	default:
		mbSearchNext = true;
		break;
	}
}
void cIntroStory::Image01() {
	mbSearchNext = false;

	switch (mlNextStop) {
		//////////////////////
		// Start
	case kIntro_Image01_Start: {
		msCentreText = _W("");
		mvImages[5].mbActive = false;

		mvImages[0].mbActive = true;
		mvImages[0].FadeBrightnessTo(0, 0);
		mvImages[0].FadeBrightnessTo(1, 13);
		mvImages[0].mvCameraPosition = cVector3f(500, 597, 0.5f);
		mvImages[0].mvPosDistMul = 0.1f;

		mvImages[0].MoveTo(cVector3f(540, 392, 0.8f), 28);

		mfTimerCount = 5.99f;
		mlNextStop = kIntro_Image01_Mess2;

		// Sounds to synch to rest of it.
		mpSoundHandler->PlayStream("gui_intro_ambience1.ogg", false, 0.7f);
		mpSoundHandler->PlayStream("penumbra_music_E1_I.ogg", false, 0.65f);

		mpSoundHandler->PlayStream("intro_01_01.ogg", false, mfVoiceVol);
		SetMessage(kTranslate("Intro", "01_01"));
		break;
	}
	case kIntro_Image01_Mess2: {
		mfTimerCount = 8.1f;
		mlNextStop = kIntro_Image01_Mess3;

		// mpInit->mpGame->GetSound()->GetMusicHandler()->Play("music_story.ogg",0.55f,0.05f,false);

		mpSoundHandler->PlayStream("intro_01_02.ogg", false, mfVoiceVol);

		SetMessage(kTranslate("Intro", "01_02"));
		break;
	}
	case kIntro_Image01_Mess3: {
		mfTimerCount = 6.0f;
		mlNextStop = kIntro_Image01_Mess4;

		mvImages[0].MoveTo(cVector3f(540, 342, 1.2f), 22);

		mpSoundHandler->PlayStream("intro_01_03.ogg", false, mfVoiceVol);

		SetMessage(kTranslate("Intro", "01_03"));

		break;
	}

	case kIntro_Image01_Mess4: {
		mfTimerCount = 12.0f;
		mlNextStop = kIntro_Image02_Start;

		mpSoundHandler->PlayStream("intro_01_04.ogg", false, mfVoiceVol);

		SetMessage(kTranslate("Intro", "01_04"));
		break;
	}
	default:
		mbSearchNext = true;
		break;
	}
}

//-----------------------------------------------------------------------

void cIntroStory::Image02() {
	mbSearchNext = false;

	switch (mlNextStop) {
		//////////////////////
		// Start
	case kIntro_Image02_Start: {
		mvImages[1].mbActive = true;
		mvImages[1].FadeAlphaTo(0, 0);
		mvImages[1].FadeAlphaTo(1, 5);
		mvImages[1].mvCameraPosition = cVector3f(500, 637, 0.3f);
		mvImages[1].mvPosDistMul = 0.11f;

		mvImages[1].MoveTo(cVector3f(680, 492, 0.6f), 28);

		mfTimerCount = 11.7f;
		mlNextStop = kIntro_Image02_Mess2;

		mpSoundHandler->PlayStream("intro_02_01.ogg", false, mfVoiceVol);

		SetMessage(kTranslate("Intro", "02_01"));

		break;
	}
	case kIntro_Image02_Mess2: {
		mfTimerCount = 4.7f;
		mlNextStop = kIntro_Image02_Mess3;

		mvImages[1].MoveTo(cVector3f(540, 420, 1.3f), 31);

		mpSoundHandler->PlayStream("intro_02_02.ogg", false, mfVoiceVol);

		SetMessage(kTranslate("Intro", "02_02"));

		break;
	}

	case kIntro_Image02_Mess3: {
		mvImages[0].mbActive = false;

		mfTimerCount = 12.0f;
		mlNextStop = kIntro_Image03_Start;

		mvImages[1].MoveTo(cVector3f(540, 400, 2.0f), 25);
		mvImages[1].FadeBrightnessTo(0, 12);

		mpSoundHandler->PlayStream("intro_02_03.ogg", false, mfVoiceVol);

		SetMessage(kTranslate("Intro", "02_03"));

		break;
	}

	default:
		mbSearchNext = true;
		break;
	}
}

//-----------------------------------------------------------------------

void cIntroStory::Image03() {
	mbSearchNext = false;

	switch (mlNextStop) {
		//////////////////////
		// Start
		//////////////////////
	case kIntro_Image03_Start: {
		mvImages[2].mbActive = true;
		mvImages[2].FadeAlphaTo(0, 0);
		mvImages[2].FadeAlphaTo(1, 5);
		mvImages[2].mvCameraPosition = cVector3f(630, 360, 0.5f);
		mvImages[2].mvPosDistMul = 0.1f;
		mvImages[2].mvPosDistMul.z = 0.1f;

		mvImages[2].MoveTo(cVector3f(530, 340, 0.8f), 15);

		mfTimerCount = 11.6f;
		mlNextStop = kIntro_Image03_Mess2;

		mpSoundHandler->PlayStream("intro_03_00.ogg", false, mfVoiceVol);

		SetMessage(kTranslate("Intro", "03_00"));
		break;
	}
	case kIntro_Image03_Mess2: {
		mvImages[1].mbActive = false;

		mfTimerCount = 10.3f;
		mlNextStop = kIntro_Image03_Mess3;

		mvImages[2].MoveTo(cVector3f(500, 300, 1.2f), 25);

		mpSoundHandler->PlayStream("intro_03_01.ogg", false, mfVoiceVol);

		SetMessage(kTranslate("Intro", "03_01"));
		break;
	}
	case kIntro_Image03_Mess3: {
		mfTimerCount = 9.5f;
		mlNextStop = kIntro_Image04_Start;

		mvImages[2].FadeBrightnessTo(0, 7.5f);

		mpSoundHandler->PlayStream("intro_03_02.ogg", false, mfVoiceVol);

		SetMessage(kTranslate("Intro", "03_02"));
		break;
	}
	case kIntro_Image03_Mess4: {
		Exit();
		break;
	}
	default:
		mbSearchNext = true;
		break;
	}
}

//-----------------------------------------------------------------------

void cIntroStory::Image04() {
	mbSearchNext = false;

	switch (mlNextStop) {
		//////////////////////
		// Start
	case kIntro_Image04_Start: {
		// mvImages[2].mbActive = false;

		mvImages[3].mbActive = true;
		mvImages[3].FadeBrightnessTo(0, 0);
		mvImages[3].FadeBrightnessTo(1, 10);
		mvImages[3].mvCameraPosition = cVector3f(530, 330, 0.8f);
		mvImages[3].mvPosDistMul = 0.1f;
		mvImages[3].mvPosDistMul.z = 0.3f;

		mvImages[3].MoveTo(cVector3f(560, 430, 1.25f), 15);

		mfTimerCount = 4.5f;
		mlNextStop = kIntro_Image04_Mess2;

		mpSoundHandler->PlayStream("intro_04_01.ogg", false, mfVoiceVol);
		SetMessage(kTranslate("Intro", "04_01"));
		break;
	}
	case kIntro_Image04_Mess2: {
		mfTimerCount = 9.0f;
		mlNextStop = kIntro_Image05_Start;

		mpSoundHandler->PlayStream("intro_04_02.ogg", false, mfVoiceVol);
		SetMessage(kTranslate("Intro", "04_02"));
		break;
	}
	default:
		mbSearchNext = true;
		break;
	}
}

//-----------------------------------------------------------------------

void cIntroStory::Image05() {
	mbSearchNext = false;

	switch (mlNextStop) {
		//////////////////////
		// Start
	case kIntro_Image05_Start: {
		mvImages[4].mbActive = true;
		mvImages[4].FadeAlphaTo(0, 0);
		mvImages[4].FadeAlphaTo(1, 4);
		mvImages[4].mvCameraPosition = cVector3f(730, 330, 0.8f);
		mvImages[4].mvPosDistMul = 0.1f;
		mvImages[4].mvPosDistMul.z = 0.6f;

		mvImages[4].MoveTo(cVector3f(530, 480, 1.15f), 15);

		mfTimerCount = 4.0f;
		mlNextStop = kIntro_Image05_Mess2;

		// mpSoundHandler->PlayStream("intro_04_01.ogg",false,mfVoiceVol);
		// SetMessage(kTranslate("Intro","04_01"));
		break;
	}
	case kIntro_Image05_Mess2: {
		mvImages[3].mbActive = false;

		mfTimerCount = 11.0f;
		mlNextStop = kIntro_Image05_Mess3;

		mvImages[4].FadeBrightnessTo(0, 10);

		mpSoundHandler->PlayStream("intro_05_01.ogg", false, mfVoiceVol);
		SetMessage(kTranslate("Intro", "05_01"));

		break;
	}
	case kIntro_Image05_Mess3: {
		Exit();
		break;
	}
	default:
		mbSearchNext = true;
		break;
	}
}

//-----------------------------------------------------------------------
