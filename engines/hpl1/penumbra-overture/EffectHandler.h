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

#ifndef GAME_EFFECT_HANDLER_H
#define GAME_EFFECT_HANDLER_H

#include "hpl1/engine/engine.h"

using namespace hpl;

class cInit;

//-----------------------------------------

class cEffect_Underwater {
public:
	cEffect_Underwater(cInit *apInit, cGraphicsDrawer *apDrawer);
	~cEffect_Underwater();

	void Update(float afTimeStep);
	void OnDraw();
	void Reset();

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

	void SetColor(const cColor &aColor) { mColor = aColor; }

private:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	cGfxObject *mpWhiteGfx;

	bool mbActive;

	cColor mColor;
};

//-----------------------------------------

class cEffect_ShakeScreen_Shake {
public:
	float mfMaxSize;
	float mfSize;
	float mfTime;
	float mfFadeInTime;
	float mfMaxFadeInTime;
	float mfFadeOutTime;
	float mfMaxFadeOutTime;
};

class cEffect_ShakeScreen {
public:
	cEffect_ShakeScreen(cInit *apInit);
	~cEffect_ShakeScreen();

	void Start(float afAmount, float afTime, float afFadeInTime, float afFadeOutTime);

	void Update(float afTimeStep);
	void Reset();

	cVector3f GetScreenAdd() { return mvAdd; }

private:
	cInit *mpInit;

	cVector3f mvAdd;

	iHapticForce *mpForce;

	std::list<cEffect_ShakeScreen_Shake> mlstShakes;
};

//-----------------------------------------
class cGameSaveArea;
class cEffect_SaveEffect {
public:
	cEffect_SaveEffect(cInit *apInit, cGraphicsDrawer *apDrawer);
	~cEffect_SaveEffect();

	void NormalSave(const cVector3f &avPos, cGameSaveArea *apSaveArea);
	void AutoSave();

	void Update(float afTimeStep);
	void OnDraw();
	void Reset();

private:
	void NormalSaveUpdate(float afTimeStep);
	void AutoSaveUpdate(float afTimeStep);

	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	cGfxObject *mpFlashGfx;

	cGameSaveArea *mpSaveArea;

	tWString msMessage;

	bool mbActive;
	int mlState;
	bool mbAutoSave;

	cVector3f mvPosition;

	float mfFlashAlpha;
	cColor mFlashColor;

	float mfTime;

	float mfStartFov;
	float mfFov;
};

//-----------------------------------------

class cEffect_DepthOfField {
public:
	cEffect_DepthOfField(cInit *apInit);
	~cEffect_DepthOfField();

	void SetDisabled(bool abX);
	bool IsDisabled() { return mbDisabled; }

	void SetActive(bool abX, float afFadeTime);
	bool IsActive() { return mbActive; }

	void SetUp(float afNearPlane, float afFocalPlane, float afFarPlane);

	void SetFocusBody(iPhysicsBody *apBody) { mpFocusBody = apBody; }
	void FocusOnBody(iPhysicsBody *apBody);

	void Update(float afTimeStep);

	void Reset();

private:
	cInit *mpInit;
	cRendererPostEffects *mpPostEffects;

	iPhysicsBody *mpFocusBody;

	bool mbActive;
	bool mbDisabled;

	float mfFadeSpeed;

	float mfMaxBlur;
};
//-----------------------------------------

class cEffect_Flash {
public:
	cEffect_Flash(cInit *apInit, cGraphicsDrawer *apDrawer);
	~cEffect_Flash();

	void Start(float afFadeIn, float afWhite, float afFadeOut);

	void Update(float afTimeStep);
	void OnDraw();
	void Reset();

private:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	cGfxObject *mpWhiteGfx;

	bool mbActive;
	float mfAlpha;

	int mlStep;

	float mfCount;

	float mfFadeInSpeed;
	float mfWhiteSpeed;
	float mfFadeOutSpeed;
};

//-----------------------------------------

class cSubTitle {
public:
	tWString msMessage;
	float mfCount;
	float mfAlpha;
	bool mbActive;
};

typedef std::list<cSubTitle> tSubTitleList;
typedef tSubTitleList::iterator tSubTitleListIt;

class cEffect_SubTitle {
public:
	cEffect_SubTitle(cInit *apInit, cGraphicsDrawer *apDrawer);
	~cEffect_SubTitle();

	void Add(const tWString &asMessage, float afTime, bool abRemovePrevious);

	void Update(float afTimeStep);
	void OnDraw();
	void Reset();

private:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	iFontData *mpFont;

	tSubTitleList mlstSubTitles;
};

//-----------------------------------------

class cEffect_WaveGravity {
public:
	cEffect_WaveGravity(cInit *apInit);
	~cEffect_WaveGravity();

	void SetActive(bool abX);
	// alDir: 0=Xaxis 1=Zaxis
	void Setup(float afMaxAngle, float afSwingLength, float afGravitySize, int alDir);

	void Update(float afTimeStep);
	void Reset();

private:
	cInit *mpInit;

	bool mbActive;

	float mfMaxAngle;
	float mfSwingLength;
	float mfSize;
	int mlDir;

	float mfTime;
};

//-----------------------------------------

class cEffectHandler : public iUpdateable {
public:
	cEffectHandler(cInit *apInit);
	~cEffectHandler();

	void OnStart();
	void Update(float afTimeStep);
	void Reset();
	void OnDraw();

	cEffect_Flash *GetFlash() { return mpFlash; }
	cEffect_WaveGravity *GetWaveGravity() { return mpWaveGravity; }
	cEffect_SubTitle *GetSubTitle() { return mpSubTitle; }
	cEffect_DepthOfField *GetDepthOfField() { return mpDepthOfField; }
	cEffect_SaveEffect *GetSaveEffect() { return mpSaveEffect; }
	cEffect_ShakeScreen *GetShakeScreen() { return mpShakeScreen; }
	cEffect_Underwater *GetUnderwater() { return mpUnderwater; }

private:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	cEffect_Flash *mpFlash;
	cEffect_WaveGravity *mpWaveGravity;
	cEffect_SubTitle *mpSubTitle;
	cEffect_DepthOfField *mpDepthOfField;
	cEffect_SaveEffect *mpSaveEffect;
	cEffect_ShakeScreen *mpShakeScreen;
	cEffect_Underwater *mpUnderwater;
};

#endif // GAME_EFFECT_HANDLER_H
