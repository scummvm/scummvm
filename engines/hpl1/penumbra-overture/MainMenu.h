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

#ifndef GAME_MAIN_MENU_H
#define GAME_MAIN_MENU_H

#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameTypes.h"

using namespace hpl;

class cInit;

enum eMainMenuState {
	eMainMenuState_Start,
	eMainMenuState_NewGame,
	eMainMenuState_Exit,
	eMainMenuState_Continue,
	eMainMenuState_Resume,

	eMainMenuState_LoadGameSpot,
	eMainMenuState_LoadGameAuto,
	eMainMenuState_LoadGameFavorite,

	eMainMenuState_Options,
	eMainMenuState_OptionsGraphics,
	eMainMenuState_OptionsGraphicsAdvanced,
	eMainMenuState_OptionsControls,
	eMainMenuState_OptionsGame,
	eMainMenuState_OptionsSound,
	eMainMenuState_OptionsKeySetupMove,
	eMainMenuState_OptionsKeySetupAction,
	eMainMenuState_OptionsKeySetupMisc,

	eMainMenuState_GraphicsRestart,

	eMainMenuState_FirstStart,

	eMainMenuState_LastEnum
};

//---------------------------

class cMainMenuWidget {
public:
	cMainMenuWidget(cInit *apInit, const cVector3f &avPos, const cVector2f &avSize);
	virtual ~cMainMenuWidget();

	virtual void OnUpdate(float afTimeStep) {}

	virtual void OnDraw() {}

	virtual void OnMouseDown(eMButton aButton){};
	virtual void OnMouseUp(eMButton aButton){};

	virtual void OnDoubleClick(eMButton aButton){};

	virtual void OnMouseOver(bool abOver) {}

	const cRect2f &GetRect() { return mRect; }

	virtual void Reset() {}

	virtual void OnActivate() {}

	bool IsActive() { return mbActive; }
	void SetActive(bool abX) {
		if (mbActive == abX)
			return;

		mbActive = abX;
		if (mbActive)
			OnActivate();
	}

	bool mbOver;

protected:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	cVector3f mvPositon;
	cRect2f mRect;

	float mfAlpha;
	bool mbActive;
};

//---------------------------------------------

class cMainMenuWidget_MainButton : public cMainMenuWidget {
public:
	cMainMenuWidget_MainButton(cInit *apInit, const cVector3f &avPos, const tWString &asText,
							   eMainMenuState aNextState);
	~cMainMenuWidget_MainButton();

	void OnUpdate(float afTimeStep);

	void OnMouseOver(bool abOver);

	void OnDraw();

	virtual void OnMouseDown(eMButton aButton);
	virtual void OnMouseUp(eMButton aButton){};

	void OnActivate() {
		mfAlpha = 0;
		mfOverTimer = 0;
		mbOver = false;
	}

protected:
	tWString msText;
	tWString msTip;
	FontData *mpFont;
	bool mbOver;
	cVector2f mvFontSize;
	eMainMenuState mNextState;

	float mfOverTimer;
};

//---------------------------------------------

class cMainMenuWidget_Button : public cMainMenuWidget {
public:
	cMainMenuWidget_Button(cInit *apInit, const cVector3f &avPos, const tWString &asText,
						   eMainMenuState aNextState, cVector2f avFontSize, eFontAlign aAlignment);
	~cMainMenuWidget_Button();

	void OnUpdate(float afTimeStep);
	void OnMouseOver(bool abOver);
	void OnDraw();

	virtual void OnMouseDown(eMButton aButton);
	virtual void OnMouseUp(eMButton aButton){};

	void OnActivate() {
		mfAlpha = 0;
		mfOverTimer = 0;
		mbOver = false;
	}

	tWString msText;

protected:
	FontData *mpFont;

	cVector2f mvFontSize;
	eMainMenuState mNextState;
	tWString msTip;

	eFontAlign mAlignment;

	float mfOverTimer;
};

//---------------------------------------------

class cMainMenuWidget_Text : public cMainMenuWidget {
public:
	cMainMenuWidget_Text(cInit *apInit, const cVector3f &avPos, const tWString &asText,
						 cVector2f avFontSize, eFontAlign aAlignment,
						 cMainMenuWidget *apExtra = NULL, float afMaxWidth = 0);
	~cMainMenuWidget_Text();

	void OnDraw();
	void OnMouseDown(eMButton aButton);
	void OnMouseOver(bool abOver);

	void SetExtraWidget(cMainMenuWidget *apExtra) { mpExtra = apExtra; }
	cMainMenuWidget *GetExtraWidget() { return mpExtra; }

	void UpdateSize();

	tWString msText;

protected:
	FontData *mpFont;

	eFontAlign mAlignment;

	cVector2f mvFontSize;

	float mfMaxWidth;

	cMainMenuWidget *mpExtra;
};

//---------------------------------------------

class cMainMenuWidget_Image : public cMainMenuWidget {
public:
	cMainMenuWidget_Image(cInit *apInit, const cVector3f &avPos, const cVector2f &avSize,
						  const tString &asImageFile, const tString &asImageMat,
						  const cColor &aColor);
	~cMainMenuWidget_Image();

	void OnDraw();

protected:
	cGfxObject *mpImage;
	cColor mColor;

	cVector2f mvSize;
};

//---------------------------------------------

class cMainMenuWidget_List : public cMainMenuWidget {
public:
	cMainMenuWidget_List(cInit *apInit, const cVector3f &avPos, const cVector2f &avSize,
						 cVector2f avFontSize);

	~cMainMenuWidget_List();

	void OnUpdate(float afTimeStep);
	void OnMouseOver(bool abOver);
	void OnDraw();

	void OnMouseDown(eMButton aButton);
	void OnMouseUp(eMButton aButton);

	void AddEntry(const tWString &asText);

	const tWString &GetSelectedEntry();
	int GetSelectedIndex() { return mlSelected; }
	void SetSelectedIndex(int alIdx) { mlSelected = alIdx; }

protected:
	cGraphicsDrawer *mpDrawer;
	FontData *mpFont;
	cGfxObject *mpBackGfx;

	cGfxObject *mpDownGfx;
	cGfxObject *mpUpGfx;
	cGfxObject *mpSlideGfx;
	cGfxObject *mpSlideButtonGfx;
	cGfxObject *mpBorderLeftGfx;
	cGfxObject *mpBorderTopGfx;
	cGfxObject *mpBorderBottomGfx;

	float mfSlideButtonSize;
	float mfSlideButtonPos;
	bool mbSlideButtonPressed;
	float mfSlideButtonMove;

	bool mbOver;
	cVector2f mvFontSize;
	cVector3f mvPosition;
	cVector2f mvSize;

	eFontAlign mAlignment;

	float mfOverTimer;

	int mlMaxRows;

	int mlFirstRow;

	int mlSelected;
	tWStringVec mvEntries;

	cVector2f mvLastMousePos;
};
//---------------------------------------------

class cMainMenuWidget_NewGame : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_NewGame(cInit *apInit, const cVector3f &avPos, const tWString &asText,
							cVector2f avFontSize, eFontAlign aAlignment,
							eGameDifficulty aDiffuculty);

	virtual void OnMouseDown(eMButton aButton);

private:
	eGameDifficulty mDiffuculty;
};

//---------------------------------------------

class cMainMenuWidget_Continue : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_Continue(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment);

	virtual void OnMouseDown(eMButton aButton);
};

//---------------------------------------------

class cMainMenuWidget_Quit : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_Quit(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment);

	virtual void OnMouseDown(eMButton aButton);
};

//---------------------------------------------

class cMainMenuWidget_Resume : public cMainMenuWidget_MainButton {
public:
	cMainMenuWidget_Resume(cInit *apInit, const cVector3f &avPos, const tWString &asText);

	virtual void OnMouseDown(eMButton aButton);
};

//---------------------------------------------

typedef Common::List<cMainMenuWidget *> tMainMenuWidgetList;
typedef tMainMenuWidgetList::iterator tMainMenuWidgetListIt;

//---------------------------------------------

class cMainMenuParticle {
public:
	cVector3f mvPos;
	cVector3f mvVel;
	cVector2f mvSize;
	cColor mCol;

	cGfxObject *mpGfx;
};

//---------------------------------------------

class cMainMenu : public iUpdateable {
	friend class cMainMenuWidget;

public:
	cMainMenu(cInit *apInit);
	~cMainMenu();

	void Reset();

	void OnPostSceneDraw();

	void OnDraw();

	void Update(float afTimeStep);

	void SetButtonTip(const tWString &asString) { msButtonTip = asString; }

	void SetMousePos(const cVector2f &avPos);
	void AddMousePos(const cVector2f &avRel);
	cVector2f GetMousePos() { return mvMousePos; }

	void OnMouseDown(eMButton aButton);
	void OnMouseUp(eMButton aButton);

	void OnMouseDoubleClick(eMButton aButton);

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

	void OnExit();

	void Exit();

	void SetState(eMainMenuState aState);

	void SetInputToAction(const tString &asActionName, cMainMenuWidget_Text *apText);

	void InitCheckInput();
	bool CheckForInput();

	void ResetWidgets(eMainMenuState aState);

	void UpdateWidgets() { mbUpdateWidgets = true; }

private:
	void DrawBackground();

	void CreateWidgets();

	void AddWidgetToState(eMainMenuState aState, cMainMenuWidget *apWidget);

	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	cGfxObject *mpGfxBlackQuad;
	cGfxObject *mpGfxMouse;

	cGfxObject *mpGfxRainDrop;
	cGfxObject *mpGfxRainSplash;
	cGfxObject *mpGfxSnowFlake;

	iTexture *mpLogo;
	iTexture *mpBackground;

	tString msCurrentActionName;
	cMainMenuWidget_Text *mpCurrentActionText;

	tWString msButtonTip;
	FontData *mpTipFont;

	Common::Array<cMainMenuParticle> mvRainDrops;
	float mfRainDropCount;

	Common::Array<cMainMenuParticle> mvRainSplashes;

	Common::Array<cMainMenuParticle> mvSnowFlakes;

	bool mbMouseIsDown;

	bool mbFadeIn;
	float mfFadeAmount;

	bool mbActive;
	float mfAlpha;

	bool mbUpdateWidgets;

	eMainMenuState mLastState;

	FontData *mpFont;

	cVector2f mvMousePos;

	tMainMenuWidgetList mlstWidgets;

	eMainMenuState mState;
	Common::Array<tMainMenuWidgetList> mvState;

	bool mbGameActive;

	bool mvKeyPressed[Common::KEYCODE_LAST];
	bool mvMousePressed[eMButton_LastEnum];
};

//---------------------------------------------

#endif // GAME_MAIN_MENU_H
