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

#ifndef GAME_INTRO_STORY_H
#define GAME_INTRO_STORY_H

#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameTypes.h"

using namespace hpl;

class cInit;

#define INTRO_IMAGE_NUM (6)

//----------------------------------------

class cIntroImage {
public:
	cIntroImage();
	~cIntroImage();

	void Update(float afTimeStep);

	void OnDraw();

	void FadeBrightnessTo(float afDarkness, float afTime);
	void FadeAlphaTo(float afDarkness, float afTime);

	void MoveTo(const cVector3f &avPos, float afTime);

	void Reset();

	//////////////
	// Variables
	cVector3f mvCameraPosition;
	cVector3f mvFinalPos;
	cVector3f mvPosStep;
	float mfPosCount;
	cVector3f mvPosDistMul;

	tVector3fList mlstPrevPos;
	int mlMaxPrevPos;

	iTexture *mpTexture;

	bool mbActive;

	float mfBrightness;
	float mfBrightnessStep;
	float mfFinalBrightness;

	float mfAlpha;
	float mfAlphaStep;
	float mfFinalAlpha;

	tVertexVec mvVtxVec;

	cInit *mpInit;
	iLowLevelGraphics *mpLowGfx;
	cTextureManager *mpTexManager;
};

//----------------------------------------

class cIntroStory : public iUpdateable {
public:
	cIntroStory(cInit *apInit);
	~cIntroStory();

	void Update(float afTimeStep);
	void Reset();
	void OnDraw();
	void OnPostSceneDraw();

	void Exit();

	bool IsActive() { return mbActive; }
	void SetActive(bool abX);

private:
	void SetMessage(const tWString &asMess);

	void Image00();
	void Image01();
	void Image02();
	void Image03();
	void Image04();
	void Image05();

	bool mbSearchNext;

	cInit *mpInit;
	iLowLevelGraphics *mpLowGfx;
	cTextureManager *mpTexManager;
	cSoundHandler *mpSoundHandler;

	iTexture *mpBlackTexture;

	cIntroImage mvImages[INTRO_IMAGE_NUM];

	float mfTimerCount;
	int mlNextStop;

	float mfVoiceVol;

	FontData *mpFont;

	bool mbActive;

	tWString msMessage;
	tWString msCentreText;

	eButtonHandlerState mLastButtonState;
};

//----------------------------------------

#endif // GAME_INTRO_STORY_H
