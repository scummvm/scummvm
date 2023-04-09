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

#include "hpl1/penumbra-overture/MainMenu.h"

#include "common/savefile.h"
#include "hpl1/debug.h"
#include "hpl1/graphics.h"
#include "hpl1/hpl1.h"
#include "hpl1/penumbra-overture/ButtonHandler.h"
#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/GraphicsHelper.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/IntroStory.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHelper.h"
#include "hpl1/penumbra-overture/SaveHandler.h"

float gfMenuFadeAmount;
bool gbMustRestart = false;

static eMainMenuState gvMenuBackStates[] = {
	eMainMenuState_Exit,  // eMainMenuState_Start,
	eMainMenuState_Start, // eMainMenuState_NewGame,
	eMainMenuState_Start, // eMainMenuState_Exit,
	eMainMenuState_Start, // eMainMenuState_Continue,
	eMainMenuState_Start, // eMainMenuState_Resume,

	eMainMenuState_Start, // eMainMenuState_LoadGameSpot,
	eMainMenuState_Start, // eMainMenuState_LoadGameAuto,
	eMainMenuState_Start, // eMainMenuState_LoadGameFavorite,

	eMainMenuState_Start,           // eMainMenuState_Options,
	eMainMenuState_Options,         // eMainMenuState_OptionsGraphics,
	eMainMenuState_OptionsGraphics, // eMainMenuState_OptionsGraphicsAdvanced,
	eMainMenuState_Options,         // eMainMenuState_OptionsControls,
	eMainMenuState_Options,         // eMainMenuState_OptionsGame,
	eMainMenuState_Options,         // eMainMenuState_OptionsSound,
	eMainMenuState_OptionsControls, // eMainMenuState_OptionsKeySetupMove,
	eMainMenuState_OptionsControls, // eMainMenuState_OptionsKeySetupAction,
	eMainMenuState_OptionsControls, // eMainMenuState_OptionsKeySetupMisc,

	eMainMenuState_Options, // eMainMenuState_GraphicsRestart,

	eMainMenuState_Start, // eMainMenuState_FirstStart,
};

//////////////////////////////////////////////////////////////////////////
// WIDGET
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget::cMainMenuWidget(cInit *apInit, const cVector3f &avPos, const cVector2f &avSize) {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	mvPositon = cVector3f(avPos.x, avPos.y, 40);

	mRect.w = avSize.x;
	mRect.h = avSize.y;
	mRect.x = avPos.x - mRect.w / 2;
	mRect.y = avPos.y;

	mbActive = true;
}

cMainMenuWidget::~cMainMenuWidget() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// MAIN BUTTON
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget_MainButton::cMainMenuWidget_MainButton(cInit *apInit, const cVector3f &avPos,
													   const tWString &asText, eMainMenuState aNextState)
	: cMainMenuWidget(apInit, avPos, cVector2f(1, 1)) {
	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("font_menu_small.fnt", 30);

	msText = asText;
	mvFontSize = 35;
	mbOver = false;

	mfOverTimer = 0;
	mfAlpha = 0;

	mRect.w = mpFont->getLength(mvFontSize, msText.c_str());
	mRect.h = mvFontSize.y + 8;
	mRect.x = avPos.x - mRect.w / 2;
	mRect.y = avPos.y + 3;

	mNextState = aNextState;

	msTip = _W("");
}

cMainMenuWidget_MainButton::~cMainMenuWidget_MainButton() {
}

void cMainMenuWidget_MainButton::OnUpdate(float afTimeStep) {
	mfOverTimer += afTimeStep * 1.3f;

	if (mbOver) {
		mfAlpha += 1.8f * afTimeStep;
		if (mfAlpha > 1)
			mfAlpha = 1;
	} else {
		mfAlpha -= 1.3f * afTimeStep;
		if (mfAlpha < 0)
			mfAlpha = 0;
	}
}

//-----------------------------------------------------------------------

void cMainMenuWidget_MainButton::OnMouseOver(bool abOver) {
	mbOver = abOver;

	if (abOver) {
		mpInit->mpMainMenu->SetButtonTip(msTip);
	}
}

//-----------------------------------------------------------------------

void cMainMenuWidget_MainButton::OnMouseDown(eMButton aButton) {
	mpInit->mpMainMenu->SetState(mNextState);
	mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_menu_click", false, 1);
}

//-----------------------------------------------------------------------

void cMainMenuWidget_MainButton::OnDraw() {
	mpFont->draw(mvPositon, mvFontSize, cColor(0.62f + mfAlpha * 0.3f, 1), eFontAlign_Center, msText);

	float fAdd = sin(mfOverTimer) * 16.0f;

	if (mfAlpha > 0) {
		mpFont->draw(mvPositon + cVector3f(fAdd, 0, -1), mvFontSize, cColor(0.56f, 0.35f * mfAlpha), eFontAlign_Center, msText);
		mpFont->draw(mvPositon + cVector3f(-fAdd, 0, -1), mvFontSize, cColor(0.56f, 0.35f * mfAlpha), eFontAlign_Center, msText);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// BUTTON
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget_Button::cMainMenuWidget_Button(cInit *apInit, const cVector3f &avPos,
											   const tWString &asText, eMainMenuState aNextState,
											   cVector2f avFontSize, eFontAlign aAlignment)
	: cMainMenuWidget(apInit, avPos, cVector2f(1, 1)) {
	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("font_menu_small.fnt", 30);

	msText = asText;
	mvFontSize = avFontSize;
	mbOver = false;

	mAlignment = aAlignment;

	mfAlpha = 0;
	mfOverTimer = 0;

	mRect.w = mpFont->getLength(mvFontSize, msText.c_str());
	mRect.h = mvFontSize.y + 3;
	mRect.y = avPos.y + 3;

	if (mAlignment == eFontAlign_Center) {
		mRect.x = avPos.x - mRect.w / 2;
	} else if (mAlignment == eFontAlign_Left) {
		mRect.x = avPos.x;
	} else if (mAlignment == eFontAlign_Right) {
		mRect.x = avPos.x - mRect.w;
	}

	mNextState = aNextState;
}

cMainMenuWidget_Button::~cMainMenuWidget_Button() {
}

void cMainMenuWidget_Button::OnUpdate(float afTimeStep) {
	if (mbOver) {
		mfAlpha += 1.8f * afTimeStep;
		if (mfAlpha > 1)
			mfAlpha = 1;
	} else {
		mfAlpha -= 1.3f * afTimeStep;
		if (mfAlpha < 0)
			mfAlpha = 0;
	}

	mfOverTimer += afTimeStep * 0.4f;
}

//-----------------------------------------------------------------------

void cMainMenuWidget_Button::OnMouseOver(bool abOver) {
	mbOver = abOver;

	if (mbOver) {
		mpInit->mpMainMenu->SetButtonTip(msTip);
	}
}

//-----------------------------------------------------------------------

void cMainMenuWidget_Button::OnMouseDown(eMButton aButton) {
	mpInit->mpMainMenu->SetState(mNextState);
	mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_menu_click", false, 1);
}

//-----------------------------------------------------------------------

void cMainMenuWidget_Button::OnDraw() {
	mpFont->draw(mvPositon, mvFontSize, cColor(0.62f, 1), mAlignment, msText);

	if (mfAlpha > 0) {
		float fX = 0.8f + sin(mfOverTimer) * 0.2f;

		mpFont->draw(mvPositon + cVector3f(0, 0, 1), mvFontSize, cColor(0.9f, 0.95f, 1.0f, mfAlpha * fX), mAlignment, msText);
		mpFont->draw(mvPositon + cVector3f(2, 2, -1), mvFontSize, cColor(0.1f, 0.32f, 1.0f, mfAlpha * fX), mAlignment, msText);
		mpFont->draw(mvPositon + cVector3f(-2, -2, -1), mvFontSize, cColor(0.1f, 0.32f, 1.0f, mfAlpha * fX), mAlignment, msText);
		mpFont->draw(mvPositon + cVector3f(3, 3, -2), mvFontSize, cColor(0.1f, 0.32f, 1.0f, mfAlpha * 0.5f * fX), mAlignment, msText);
		mpFont->draw(mvPositon + cVector3f(-3, -3, -2), mvFontSize, cColor(0.1f, 0.32f, 1.0f, mfAlpha * 0.5f * fX), mAlignment, msText);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// TEXT
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget_Text::cMainMenuWidget_Text(cInit *apInit, const cVector3f &avPos, const tWString &asText,
										   cVector2f avFontSize, eFontAlign aAlignment,
										   cMainMenuWidget *apExtra, float afMaxWidth)
	: cMainMenuWidget(apInit, avPos, cVector2f(1, 1)) {
	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("font_menu_small.fnt", 30);

	mfMaxWidth = afMaxWidth;

	msText = asText;
	mvFontSize = avFontSize;

	mAlignment = aAlignment;

	mRect.w = mpFont->getLength(mvFontSize, msText.c_str());
	mRect.h = mvFontSize.y + 3;
	mRect.y = avPos.y + 3;

	if (mAlignment == eFontAlign_Center) {
		mRect.x = avPos.x - mRect.w / 2;
	} else if (mAlignment == eFontAlign_Left) {
		mRect.x = avPos.x;
	} else if (mAlignment == eFontAlign_Right) {
		mRect.x = avPos.x - mRect.w;
	}

	mpExtra = apExtra;

	mbOver = false;
}

cMainMenuWidget_Text::~cMainMenuWidget_Text() {
}

//-----------------------------------------------------------------------

void cMainMenuWidget_Text::UpdateSize() {
	mRect.w = mpFont->getLength(mvFontSize, msText.c_str());
}

void cMainMenuWidget_Text::OnDraw() {
	if (mfMaxWidth <= 0)
		mpFont->draw(mvPositon, mvFontSize, cColor(0.9f, 1), mAlignment, msText);
	else
		mpFont->drawWordWrap(mvPositon, mfMaxWidth, mvFontSize.y + 1,
							 mvFontSize, cColor(0.9f, 1), mAlignment, msText);
}

//-----------------------------------------------------------------------

void cMainMenuWidget_Text::OnMouseDown(eMButton aButton) {
	if (mpExtra) {
		mpExtra->OnMouseDown(aButton);
	}
}

void cMainMenuWidget_Text::OnMouseOver(bool abOver) {
	// if(abOver == mbOver) return;

	mbOver = abOver;

	if (mpExtra) {
		if (mpExtra->mbOver == false)
			mpExtra->OnMouseOver(abOver);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// IMAGE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget_Image::cMainMenuWidget_Image(cInit *apInit, const cVector3f &avPos, const cVector2f &avSize,
											 const tString &asImageFile, const tString &asImageMat,
											 const cColor &aColor)
	: cMainMenuWidget(apInit, avPos, avSize) {
	mpImage = mpDrawer->CreateGfxObject(asImageFile, asImageMat);

	mColor = aColor;

	mvSize = avSize;
}

cMainMenuWidget_Image::~cMainMenuWidget_Image() {
}

//-----------------------------------------------------------------------

void cMainMenuWidget_Image::OnDraw() {
	mpDrawer->DrawGfxObject(mpImage, mvPositon, mvSize, mColor);
}

//////////////////////////////////////////////////////////////////////////
// LIST
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget_List::cMainMenuWidget_List(cInit *apInit, const cVector3f &avPos, const cVector2f &avSize,
										   cVector2f avFontSize)
	: cMainMenuWidget(apInit, avPos, avSize) {
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("font_menu_small.fnt", 30);

	mpBackGfx = mpDrawer->CreateGfxObject("effect_white.jpg", "diffalpha2d");

	mpDownGfx = mpDrawer->CreateGfxObject("menu_list_down.bmp", "diffalpha2d");
	mpUpGfx = mpDrawer->CreateGfxObject("menu_list_up.bmp", "diffalpha2d");
	mpSlideGfx = mpDrawer->CreateGfxObject("menu_list_slide.bmp", "diffalpha2d");
	mpBorderLeftGfx = mpDrawer->CreateGfxObject("menu_list_border_left.bmp", "diffalpha2d");
	mpBorderTopGfx = mpDrawer->CreateGfxObject("menu_list_border_top.bmp", "diffalpha2d");
	mpBorderBottomGfx = mpDrawer->CreateGfxObject("menu_list_border_bottom.bmp", "diffalpha2d");
	mpSlideButtonGfx = mpDrawer->CreateGfxObject("menu_list_slider_button.bmp", "diffalpha2d");

	mvFontSize = avFontSize;

	mRect.x = avPos.x;
	mRect.y = avPos.y;

	mRect.w = avSize.x;
	mRect.h = avSize.y;

	mlMaxRows = (int)floor((avSize.y - 6) / (avFontSize.y + 2));

	mvPosition = avPos;
	mvSize = avSize;

	mlSelected = -1;

	mlFirstRow = 0;

	mbSlideButtonPressed = false;
	mfSlideButtonMove = 0;

	mvLastMousePos = mpInit->mpMainMenu->GetMousePos();
}
cMainMenuWidget_List::~cMainMenuWidget_List() {
}

//-----------------------------------------------------------------------

void cMainMenuWidget_List::OnUpdate(float afTimeStep) {
	////////////////////////////////
	// Slide button

	// Size
	float fT = (float)mlMaxRows / (float)mvEntries.size();
	if (fT > 1)
		fT = 1;
	mfSlideButtonSize = (mvSize.y - 28) * fT;

	// Pos
	mfSlideButtonPos = 0;
	if ((int)mvEntries.size() > mlMaxRows) {
		mfSlideButtonPos = ((mvSize.y - 28) - mfSlideButtonSize) *
						   (float)mlFirstRow / (float)(mvEntries.size() - mlMaxRows);
	}

	/////////////////////////////////
	// Slide button pressed
	if (mbSlideButtonPressed && (int)mvEntries.size() > mlMaxRows) {
		float fMinStep = ((mvSize.y - 28) - mfSlideButtonSize) / (float)mvEntries.size();
		cVector2f vRelMouse = mpInit->mpMainMenu->GetMousePos() - mvLastMousePos;

		mfSlideButtonMove += vRelMouse.y;

		while (mfSlideButtonMove <= -fMinStep && mlFirstRow > 0) {
			mlFirstRow--;
			mfSlideButtonMove += fMinStep;
		}
		while (mfSlideButtonMove >= fMinStep && mlFirstRow < (int)mvEntries.size() - mlMaxRows) {
			mlFirstRow++;
			mfSlideButtonMove -= fMinStep;
		}
	}

	mvLastMousePos = mpInit->mpMainMenu->GetMousePos();
}

//-----------------------------------------------------------------------

void cMainMenuWidget_List::OnMouseOver(bool abOver) {
	if (abOver == false) {
		mbSlideButtonPressed = false;
		mfSlideButtonMove = 0;
	}
}

//-----------------------------------------------------------------------

void cMainMenuWidget_List::OnDraw() {
	mpDrawer->DrawGfxObject(mpBackGfx, mvPositon - cVector3f(0, 0, 1), mvSize, cColor(0.05f, 0.05f, 0.1f, 1));

	// Up
	mpDrawer->DrawGfxObject(mpUpGfx,
							cVector3f(mvPositon.x + mvSize.x - 14, mvPositon.y, mvPositon.z + 1),
							cVector2f(14, 14), cColor(1, 1));
	// Down
	mpDrawer->DrawGfxObject(mpDownGfx,
							cVector3f(mvPositon.x + mvSize.x - 14, mvPositon.y + mvSize.y - 14, mvPositon.z + 1),
							cVector2f(14, 14), cColor(1, 1));
	// Slide
	mpDrawer->DrawGfxObject(mpSlideGfx,
							cVector3f(mvPositon.x + mvSize.x - 14, mvPositon.y + 14, mvPositon.z + 1),
							cVector2f(14, mvSize.y - 28), cColor(1, 1));

	// Border Top
	mpDrawer->DrawGfxObject(mpBorderTopGfx,
							cVector3f(mvPositon.x + 3, mvPositon.y, mvPositon.z + 1),
							cVector2f(mvSize.x - 17, 3), cColor(1, 1));
	// Border Bottom
	mpDrawer->DrawGfxObject(mpBorderBottomGfx,
							cVector3f(mvPositon.x + 3, mvPositon.y + mvSize.y - 3, mvPositon.z + 1),
							cVector2f(mvSize.x - 17, 3), cColor(1, 1));

	// Border Left
	mpDrawer->DrawGfxObject(mpBorderLeftGfx,
							cVector3f(mvPositon.x, mvPositon.y, mvPositon.z + 1),
							cVector2f(3, mvSize.y), cColor(1, 1));

	// Slider Button
	cVector3f vButtonStart = cVector3f(mvPositon.x + mvSize.x - 14, mvPositon.y + 14, mvPositon.z + 2);
	mpDrawer->DrawGfxObject(mpSlideButtonGfx, vButtonStart + cVector3f(0, mfSlideButtonPos, 0),
							cVector2f(14, mfSlideButtonSize), cColor(1, 1));

	cVector3f vPos = mvPositon + cVector3f(5, 3, 0);

	for (size_t i = mlFirstRow; i < mvEntries.size(); ++i) {
		if ((int)i - mlFirstRow >= mlMaxRows)
			break;

		if (mlSelected == (int)i) {
			mpFont->draw(vPos, mvFontSize, cColor(0.95f, 1), eFontAlign_Left, mvEntries[i]);
			mpDrawer->DrawGfxObject(mpBackGfx, vPos + cVector3f(0, 2, -1),
									cVector2f(mvSize.x - 5, mvFontSize.y),
									cColor(0.0f, 0.0f, 0.73f, 1));
		} else
			mpFont->draw(vPos, mvFontSize, cColor(0.7f, 1), eFontAlign_Left, mvEntries[i]);

		vPos.y += mvFontSize.y + 2;
	}
}

//-----------------------------------------------------------------------

void cMainMenuWidget_List::OnMouseDown(eMButton aButton) {
	cVector2f vLocalMouse = mpInit->mpMainMenu->GetMousePos() -
							cVector2f(mvPositon.x, mvPositon.y);

	// Scrollbar
	if (vLocalMouse.x > mvSize.x - 14) {
		// Up Arrow
		if (vLocalMouse.y <= 14) {
			if (mlFirstRow > 0)
				mlFirstRow--;
		}
		// Down Arrow
		else if (vLocalMouse.y >= mvSize.y - 14) {
			if (mlFirstRow < (int)mvEntries.size() - mlMaxRows)
				mlFirstRow++;
		}
		// Press slide button
		else if (vLocalMouse.y >= mfSlideButtonPos &&
				 vLocalMouse.y <= mfSlideButtonPos + mfSlideButtonSize) {
			mbSlideButtonPressed = true;
		}
	}
	// Entries
	else {
		int lSelected = mlFirstRow + (int)floor((vLocalMouse.y - 3) / (mvFontSize.y + 2));
		if (lSelected < (int)mvEntries.size())
			mlSelected = lSelected;
	}
}

void cMainMenuWidget_List::OnMouseUp(eMButton aButton) {
	mbSlideButtonPressed = false;
	mfSlideButtonMove = 0;
}

//-----------------------------------------------------------------------

void cMainMenuWidget_List::AddEntry(const tWString &asText) {
	mvEntries.push_back(asText);
	if (mlSelected == -1 && mvEntries.size() == 1)
		mlSelected = 0;
}

//-----------------------------------------------------------------------

const tWString &cMainMenuWidget_List::GetSelectedEntry() {
	return mvEntries[0];
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// NEW GAME
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget_NewGame::cMainMenuWidget_NewGame(cInit *apInit, const cVector3f &avPos,
												 const tWString &asText,
												 cVector2f avFontSize, eFontAlign aAlignment,
												 eGameDifficulty aDiffuculty)
	: cMainMenuWidget_Button(apInit, avPos, asText,
							 eMainMenuState_LastEnum, avFontSize, aAlignment) {
	mDiffuculty = aDiffuculty;

	switch (mDiffuculty) {
	case eGameDifficulty_Easy:
		msTip = kTranslate("MainMenu", "TipDifficultyEasy");
		break;
	case eGameDifficulty_Normal:
		msTip = kTranslate("MainMenu", "TipDifficultyNormal");
		break;
	case eGameDifficulty_Hard:
		msTip = kTranslate("MainMenu", "TipDifficultyHard");
		break;
	default:
		break;
	}
}
//-----------------------------------------------------------------------

void cMainMenuWidget_NewGame::OnMouseDown(eMButton aButton) {
	mpInit->mpGraphicsHelper->DrawLoadingScreen("");

	mpInit->mpMainMenu->SetActive(false);
	mpInit->ResetGame(true);

	mpInit->mDifficulty = mDiffuculty;

	if (mpInit->mbShowIntro) {
		mpInit->mpIntroStory->SetActive(true);
	} else {
		mpInit->mpGame->GetUpdater()->SetContainer("Default");
		mpInit->mpGame->GetScene()->SetDrawScene(true);

		mpInit->mpMapHandler->Load(mpInit->msStartMap, mpInit->msStartLink);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONTINUE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget_Continue::cMainMenuWidget_Continue(cInit *apInit, const cVector3f &avPos,
												   const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
	: cMainMenuWidget_Button(apInit, avPos, asText,
							 eMainMenuState_LastEnum, avFontSize, aAlignment) {
}
//-----------------------------------------------------------------------

void cMainMenuWidget_Continue::OnMouseDown(eMButton aButton) {
	// mpInit->mpGraphicsHelper->DrawLoadingScreen("other_loading.jpg");

	mpInit->mpMainMenu->SetActive(false);

	tWString latestSave = mpInit->mpSaveHandler->GetLatest(_W("????:*"));
	if (latestSave != _W(""))
		mpInit->mpSaveHandler->LoadGameFromFile(latestSave);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// QUIT
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget_Quit::cMainMenuWidget_Quit(cInit *apInit, const cVector3f &avPos,
										   const tWString &asText,
										   cVector2f avFontSize, eFontAlign aAlignment)
	: cMainMenuWidget_Button(apInit, avPos,
							 asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
}
//-----------------------------------------------------------------------

void cMainMenuWidget_Quit::OnMouseDown(eMButton aButton) {
	mpInit->mpGame->Exit();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// RESUME
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget_Resume::cMainMenuWidget_Resume(cInit *apInit, const cVector3f &avPos,
											   const tWString &asText)
	: cMainMenuWidget_MainButton(apInit, avPos, asText, eMainMenuState_LastEnum) {
}
//-----------------------------------------------------------------------

void cMainMenuWidget_Resume::OnMouseDown(eMButton aButton) {
	mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_menu_click", false, 1);

	mpInit->mpMainMenu->SetActive(false);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// LOADGAME
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

tWStringVec *gvSaveGameFileVec;

class cMainMenuWidget_SaveGameList : public cMainMenuWidget_List {
public:
	cMainMenuWidget_SaveGameList(cInit *apInit, const cVector3f &avPos, const cVector2f &avSize,
								 cVector2f avFontSize, tWString asDir, int alNum)
		: cMainMenuWidget_List(apInit, avPos, avSize, avFontSize) {
		msDir = asDir;
		mlNum = alNum;
	}

	void OnDoubleClick(eMButton aButton) {
		if (mlSelected < 0)
			return;

		tWString sFile = gvSaveGameFileVec[mlNum][mlSelected];

		mpInit->mpMainMenu->SetActive(false);
		mpInit->ResetGame(true);

		mpInit->mpSaveHandler->LoadGameFromFile(sFile);
	}

private:
	tWString msDir;
	int mlNum;
};

cMainMenuWidget_SaveGameList *gpSaveGameList[3] = {NULL, NULL, NULL};

class cMainMenuWidget_LoadSaveGame : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_LoadSaveGame(cInit *apInit, const cVector3f &avPos, const tWString &asText,
								 cVector2f avFontSize, eFontAlign aAlignment,
								 tWString asDir, int alNum)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msDir = asDir;
		mlNum = alNum;
	}

	void OnMouseDown(eMButton aButton) {
		int lSelected = gpSaveGameList[mlNum]->GetSelectedIndex();
		if (lSelected < 0)
			return;

		tWString sFile = msDir + Common::U32String("/") + gvSaveGameFileVec[mlNum][lSelected];

		mpInit->mpMainMenu->SetActive(false);
		mpInit->ResetGame(true);

		mpInit->mpSaveHandler->LoadGameFromFile(sFile);
	}

	tWString msDir;
	int mlNum;
};

class cMainMenuWidget_RemoveSaveGame : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_RemoveSaveGame(cInit *apInit, const cVector3f &avPos, const tWString &asText,
								   cVector2f avFontSize, eFontAlign aAlignment,
								   tWString asDir, int alNum)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msDir = asDir;
		mlNum = alNum;
	}

	void OnMouseDown(eMButton aButton) {
		int lSelected = gpSaveGameList[mlNum]->GetSelectedIndex();
		if (lSelected < 0)
			return;

		tWString sFile = gvSaveGameFileVec[mlNum][lSelected];
		Hpl1::g_engine->removeSaveFile(sFile);
		mpInit->mpMainMenu->UpdateWidgets();
	}

	tWString msDir;
	int mlNum;
};

class cMainMenuWidget_FavoriteSaveGame : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_FavoriteSaveGame(cInit *apInit, const cVector3f &avPos, const tWString &asText,
									 cVector2f avFontSize, eFontAlign aAlignment, int alNum)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		mlNum = alNum;
	}

	void OnMouseDown(eMButton aButton) {
		int lSelected = gpSaveGameList[mlNum]->GetSelectedIndex();
		if (lSelected < 0)
			return;

		tWString originalName = gvSaveGameFileVec[mlNum][lSelected];
		tWString newName = _W("favorite-") + cString::SubW(originalName, originalName.find('.') + 1);
		Hpl1::logInfo(Hpl1::kDebugSaves, "adding save %S to favourites\n", newName.encode().c_str());
		Common::String originalFile(Hpl1::g_engine->mapInternalSaveToFile(originalName));
		Common::String newFile(Hpl1::g_engine->createSaveFile(newName));
		g_engine->getSaveFileManager()->copySavefile(originalFile, newFile);
		mpInit->mpMainMenu->UpdateWidgets();
	}

	int mlNum;
};

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// OPTIONS CONTROLS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenuWidget_Text *gpInvertMouseYText = NULL;
cMainMenuWidget_Text *gpMouseSensitivityText = NULL;
cMainMenuWidget_Text *gpToggleCrouchText = NULL;
cMainMenuWidget_Text *gpWidgetInteractModeCameraSpeedText = NULL;
cMainMenuWidget_Text *gpWidgetActionModeCameraSpeedText = NULL;
cMainMenuWidget_Text *gpWidgetWeightForceScaleText = NULL;

//-----------------------------------------------------------------------

class cMainMenuWidget_InvertMouseY : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_InvertMouseY(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipControlsInvertMouseY");
	}

	void OnMouseDown(eMButton aButton) {
		mpInit->mpButtonHandler->mbInvertMouseY = !mpInit->mpButtonHandler->mbInvertMouseY;
		gpInvertMouseYText->msText = mpInit->mpButtonHandler->mbInvertMouseY ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

//------------------------------------------------------------

class cMainMenuWidget_MouseSensitivity : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_MouseSensitivity(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipControlsMouseSensitivity");
	}

	void OnMouseDown(eMButton aButton) {
		if (aButton == eMButton_Left) {
			mpInit->mpButtonHandler->mfMouseSensitivity += 0.2f;
			if (mpInit->mpButtonHandler->mfMouseSensitivity > 5.0f)
				mpInit->mpButtonHandler->mfMouseSensitivity = 5.0f;
		} else if (aButton == eMButton_Right) {
			mpInit->mpButtonHandler->mfMouseSensitivity -= 0.2f;
			if (mpInit->mpButtonHandler->mfMouseSensitivity < 0.2f)
				mpInit->mpButtonHandler->mfMouseSensitivity = 0.2f;
		}

		char sTempVec[256];
		snprintf(sTempVec, 256, "%.1f", mpInit->mpButtonHandler->mfMouseSensitivity);
		gpMouseSensitivityText->msText = cString::To16Char(sTempVec);
	}
};

//------------------------------------------------------------

class cMainMenuWidget_ToggleCrouch : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_ToggleCrouch(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipControlsToggleCrouch");
	}

	void OnMouseDown(eMButton aButton) {
		mpInit->mpButtonHandler->mbToggleCrouch = !mpInit->mpButtonHandler->mbToggleCrouch;
		gpToggleCrouchText->msText = mpInit->mpButtonHandler->mbToggleCrouch ? kTranslate("MainMenu", "On")
																			 : kTranslate("MainMenu", "Off");
	}
};

//////////////////////////////////////////////////////////////////////////
// OPTIONS GAME
//////////////////////////////////////////////////////////////////////////

cMainMenuWidget_Text *gpLanguageText = NULL;
cMainMenuWidget_Text *gpSubtitlesText = NULL;

cMainMenuWidget_Text *gpSimpleSwingText = NULL;
cMainMenuWidget_Text *gpAllowQuickSaveText = NULL;
cMainMenuWidget_Text *gpDisablePersonalText = NULL;
cMainMenuWidget_Text *gpDifficultyText = NULL;
cMainMenuWidget_Text *gpFlashItemsText = NULL;
cMainMenuWidget_Text *gpShowCrossHairText = NULL;

class cMainMenuWidget_ShowCrossHair : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_ShowCrossHair(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = _W("");
	}

	void OnMouseDown(eMButton aButton) {
		mpInit->mbShowCrossHair = !mpInit->mbShowCrossHair;

		gpShowCrossHairText->msText = mpInit->mbShowCrossHair ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

class cMainMenuWidget_FlashItems : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_FlashItems(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGameFlashText");
	}

	void OnMouseDown(eMButton aButton) {
		mpInit->mbFlashItems = !mpInit->mbFlashItems;

		gpFlashItemsText->msText = mpInit->mbFlashItems ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

constexpr const char *gvDifficultyLevel[] = {"Easy", "Normal", "Hard"};
constexpr int glDifficultyLevelNum = 3;

class cMainMenuWidget_Difficulty : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_Difficulty(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGameDifficulty");
	}

	void OnMouseDown(eMButton aButton) {
		int lCurrent = (int)mpInit->mDifficulty;
		if (aButton == eMButton_Left) {
			lCurrent++;
			if (lCurrent >= glDifficultyLevelNum)
				lCurrent = 0;
		} else if (aButton == eMButton_Right) {
			lCurrent--;
			if (lCurrent < 0)
				lCurrent = glDifficultyLevelNum - 1;
		}

		gpDifficultyText->msText = kTranslate("MainMenu", gvDifficultyLevel[lCurrent]);
		mpInit->mDifficulty = (eGameDifficulty)lCurrent;
	}
};

class cMainMenuWidget_SimpleSwing : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_SimpleSwing(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
	}

	void OnMouseDown(eMButton aButton) {
		mpInit->mbSimpleWeaponSwing = !mpInit->mbSimpleWeaponSwing;

		gpSimpleSwingText->msText = mpInit->mbSimpleWeaponSwing ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

class cMainMenuWidget_AllowQuickSave : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_AllowQuickSave(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
	}

	void OnMouseDown(eMButton aButton) {
		mpInit->mbAllowQuickSave = !mpInit->mbAllowQuickSave;

		gpAllowQuickSaveText->msText = mpInit->mbAllowQuickSave ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

class cMainMenuWidget_DisablePersonal : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_DisablePersonal(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGameDisablePersonal");
	}

	void OnMouseDown(eMButton aButton) {
		mpInit->mbDisablePersonalNotes = !mpInit->mbDisablePersonalNotes;

		gpDisablePersonalText->msText = mpInit->mbDisablePersonalNotes ? kTranslate("MainMenu", "Off") : kTranslate("MainMenu", "On");
	}
};

class cMainMenuWidget_Subtitles : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_Subtitles(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGameSubtitles");
	}

	void OnMouseDown(eMButton aButton) {
		mpInit->mbSubtitles = !mpInit->mbSubtitles;

		gpSubtitlesText->msText = mpInit->mbSubtitles ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

class cMainMenuWidget_Language : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_Language(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		tStringList lstStrings;
		apInit->mpGame->GetResources()->GetLowLevel()->findFilesInDir(lstStrings, "config", "*.lang");

		mlCurrentFile = 0;
		int lIdx = 0;
		for (tStringListIt it = lstStrings.begin(); it != lstStrings.end(); ++it) {
			mvFiles.push_back(*it);
			if (cString::ToLowerCase(apInit->msLanguageFile) ==
				cString::ToLowerCase(*it)) {
				mlCurrentFile = lIdx;
			}

			++lIdx;
		}

		msTip = kTranslate("MainMenu", "TipGameLanguage");
	}

	void OnMouseDown(eMButton aButton) {
		if (aButton == eMButton_Left) {
			mlCurrentFile++;
			if (mlCurrentFile >= (int)mvFiles.size())
				mlCurrentFile = 0;
		} else if (aButton == eMButton_Right) {
			mlCurrentFile--;
			if (mlCurrentFile < 0)
				mlCurrentFile = (int)mvFiles.size() - 1;
		}

		gpLanguageText->msText = cString::To16Char(cString::SetFileExt(mvFiles[mlCurrentFile], ""));
		mpInit->msLanguageFile = mvFiles[mlCurrentFile];

		if (mpInit->mpMapHandler->GetCurrentMapName() != "") {
			gbMustRestart = true;
		} else {
			mpInit->mpGame->GetResources()->ClearResourceDirs();
			mpInit->mpGame->GetResources()->AddResourceDir("core/programs");
			mpInit->mpGame->GetResources()->AddResourceDir("core/textures");
			mpInit->mpGame->GetResources()->LoadResourceDirsFile("resources.cfg");

			mpInit->mpGame->GetResources()->SetLanguageFile(mpInit->msLanguageFile);

			mpInit->mpMainMenu->UpdateWidgets();
		}
	}

	Common::Array<tString> mvFiles;
	int mlCurrentFile;
};

//------------------------------------------------------------

//------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// OPTIONS GRAPHICS
//////////////////////////////////////////////////////////////////////////

cMainMenuWidget_Text *gpResolutionText = NULL;
cMainMenuWidget_Text *gpPostEffectsText = NULL;
cMainMenuWidget_Text *gpBloomText = NULL;
cMainMenuWidget_Text *gpMotionBlurText = NULL;
cMainMenuWidget_Text *gpTextureQualityText = NULL;
cMainMenuWidget_Text *gpShaderQualityText = NULL;
cMainMenuWidget_Text *gpShadowsText = NULL;
cMainMenuWidget_Text *gpTextureFilterText = NULL;
cMainMenuWidget_Text *gpTextureAnisotropyText = NULL;
cMainMenuWidget_Text *gpGammaText = NULL;
cMainMenuWidget_Text *gpGammaText2 = NULL;
cMainMenuWidget_Text *gpFSAAText = NULL;
cMainMenuWidget_Text *gpDoFText = NULL;

constexpr cVector2l gvResolutions[] = {cVector2l(640, 480), cVector2l(800, 600), cVector2l(1024, 768),
									   cVector2l(1152, 864), cVector2l(1280, 720), cVector2l(1280, 768),
									   cVector2l(1280, 800), cVector2l(1280, 960), cVector2l(1280, 1024),
									   cVector2l(1360, 768), cVector2l(1360, 1024), cVector2l(1400, 1050),
									   cVector2l(1440, 900), cVector2l(1680, 1050), cVector2l(1600, 1200),
									   cVector2l(1920, 1080), cVector2l(1920, 1200)};
int glResolutionNum = 17;

constexpr const char *gvTextureQuality[] = {"High", "Medium", "Low"};
constexpr int glTextureQualityNum = 3;
constexpr const char *gvShaderQuality[] = {"Very Low", "Low", "Medium", "High"};
constexpr int glShaderQualityNum = 4;

cMainMenuWidget_Text *gpNoiseFilterText = NULL;

//------------------------------------------------------------

class cMainMenuWidget_Gamma : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_Gamma(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment,
						  int alGNum)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		mfMax = 3.0f;
		mfMin = 0.1f;
		mfStep = 0.1f;

		mlGNum = alGNum;

		msTip = kTranslate("MainMenu", "TipGraphicsGamma");
	}

	void OnMouseDown(eMButton aButton) {
		mfGamma = mpInit->mpGame->GetGraphics()->GetLowLevel()->GetGammaCorrection();

		if (aButton == eMButton_Left) {
			mfGamma += mfStep;
			if (mfGamma > mfMax)
				mfGamma = mfMax;
		} else if (aButton == eMButton_Right) {
			mfGamma -= mfStep;
			if (mfGamma < mfMin)
				mfGamma = mfMin;
		}

		mpInit->mpGame->GetGraphics()->GetLowLevel()->SetGammaCorrection(mfGamma);

		char sTempVec[256];
		snprintf(sTempVec, 256, "%.1f", mfGamma);

		gpGammaText->msText = cString::To16Char(sTempVec);
		if (mlGNum == 1)
			gpGammaText2->msText = cString::To16Char(sTempVec);
	}

	float mfGamma;
	float mfMax;
	float mfMin;
	float mfStep;
	int mlGNum;
};

//------------------------------------------------------------

class cMainMenuWidget_NoiseFilter : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_NoiseFilter(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGraphicsNoiseFilter");
	}

	void OnMouseDown(eMButton aButton) {
		bool bX = mpInit->mpPlayer->GetNoiseFilter()->IsActive();
		mpInit->mpPlayer->GetNoiseFilter()->SetActive(!bX);

		gpNoiseFilterText->msText = mpInit->mpPlayer->GetNoiseFilter()->IsActive() ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

//------------------------------------------------------------

constexpr const char *gvShadowTypes[] = {"On", "Only Static", "Off"};

class cMainMenuWidget_Shadows : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_Shadows(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		mlCurrent = apInit->mpGame->GetGraphics()->GetRenderer3D()->GetShowShadows();
		msTip = kTranslate("MainMenu", "TipGraphicsShadows");
	}

	void OnMouseDown(eMButton aButton) {
		if (aButton == eMButton_Left) {
			mlCurrent++;
			if (mlCurrent >= 3)
				mlCurrent = 0;
		} else if (aButton == eMButton_Right) {
			mlCurrent--;
			if (mlCurrent < 0)
				mlCurrent = 2;
		}

		gpShadowsText->msText = kTranslate("MainMenu", gvShadowTypes[mlCurrent]);
		mpInit->mpGame->GetGraphics()->GetRenderer3D()->SetShowShadows((eRendererShowShadows)mlCurrent);
	}

	int mlCurrent;
};

//------------------------------------------------------------

constexpr const char *gvTextureFilter[] = {"Bilinear", "Trilinear"};

class cMainMenuWidget_TextureFilter : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_TextureFilter(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		mlCurrent = apInit->mpGame->GetResources()->GetMaterialManager()->GetTextureFilter();
		msTip = kTranslate("MainMenu", "TipGraphicsTextureFilter");
	}

	void OnMouseDown(eMButton aButton) {
		if (aButton == eMButton_Left) {
			mlCurrent--;
			if (mlCurrent < 0)
				mlCurrent = 1;
		} else if (aButton == eMButton_Right) {
			mlCurrent++;
			if (mlCurrent > 1)
				mlCurrent = 0;
		}

		gpTextureFilterText->msText = kTranslate("MainMenu", gvTextureFilter[mlCurrent]);
		mpInit->mpGame->GetResources()->GetMaterialManager()->SetTextureFilter((eTextureFilter)mlCurrent);
	}

	int mlCurrent;
};

//------------------------------------------------------------

class cMainMenuWidget_TextureAnisotropy : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_TextureAnisotropy(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		mlMax = mpInit->mpGame->GetGraphics()->GetLowLevel()->GetCaps(eGraphicCaps_MaxAnisotropicFiltering);
		msTip = kTranslate("MainMenu", "TipGraphicsTextureAnisotropy");
	}

	void OnMouseDown(eMButton aButton) {
		int lX = (int)mpInit->mpGame->GetResources()->GetMaterialManager()->GetTextureAnisotropy();

		if (aButton == eMButton_Left) {
			lX *= 2;
			if (lX > mlMax)
				lX = 1;
		} else if (aButton == eMButton_Right) {
			lX /= 2;
			if (lX < 1)
				lX = mlMax;
		}

		if (lX != 1)
			gpTextureAnisotropyText->msText = cString::To16Char(cString::ToString(lX) + "x");
		else
			gpTextureAnisotropyText->msText = kTranslate("MainMenu", "Off");

		mpInit->mpGame->GetResources()->GetMaterialManager()->SetTextureAnisotropy((float)lX);
	}

	int mlMax;
};

//------------------------------------------------------------

class cMainMenuWidget_FSAA : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_FSAA(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		mlMax = 4;
		msTip = kTranslate("MainMenu", "TipGraphicsFSAA");
	}

	void OnMouseDown(eMButton aButton) {
		int lX = mpInit->mlFSAA;

		if (aButton == eMButton_Left) {
			if (lX == 0)
				lX = 2;
			else
				lX *= 2;

			if (lX > mlMax)
				lX = 0;
		} else if (aButton == eMButton_Right) {
			if (lX == 2)
				lX = 0;
			else if (lX == 0)
				lX = -1;
			else
				lX /= 2;

			if (lX < 0)
				lX = mlMax;
		}

		if (lX != 0)
			gpFSAAText->msText = cString::To16Char(cString::ToString(lX) + "x");
		else
			gpFSAAText->msText = kTranslate("MainMenu", "Off");

		mpInit->mlFSAA = lX;

		gbMustRestart = true;
	}

	int mlMax;
	int mlCurrent;
};

class cMainMenuWidget_DOF : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_DOF(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGraphicsDOF");
	}

	void OnMouseDown(eMButton aButton) {
		bool bX = mpInit->mpEffectHandler->GetDepthOfField()->IsDisabled();
		mpInit->mpEffectHandler->GetDepthOfField()->SetDisabled(!bX);

		gpDoFText->msText = bX ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

//------------------------------------------------------------

class cMainMenuWidget_ShaderQuality : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_ShaderQuality(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGraphicsShaderQuality");
	}

	void OnMouseDown(eMButton aButton) {
		if (!Hpl1::areShadersAvailable())
			return;
		int lCurrent = iMaterial::GetQuality();
		if (aButton == eMButton_Left) {
			lCurrent++;
			if (lCurrent >= glShaderQualityNum)
				lCurrent = 0;
		} else if (aButton == eMButton_Right) {
			lCurrent--;
			if (lCurrent < 0)
				lCurrent = glShaderQualityNum - 1;
		}

		gpShaderQualityText->msText = kTranslate("MainMenu", gvShaderQuality[lCurrent]);
		iMaterial::SetQuality((eMaterialQuality)lCurrent);

		if (mpInit->mpMapHandler->GetCurrentMapName() != "")
			gbMustRestart = true;
	}
};

//------------------------------------------------------------

class cMainMenuWidget_TextureQuality : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_TextureQuality(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		mlCurrent = apInit->mpGame->GetResources()->GetMaterialManager()->GetTextureSizeLevel();
		msTip = kTranslate("MainMenu", "TipGraphicsTextureQuality");
	}

	void OnMouseDown(eMButton aButton) {
		if (aButton == eMButton_Right) {
			mlCurrent++;
			if (mlCurrent >= glTextureQualityNum)
				mlCurrent = 0;
		} else if (aButton == eMButton_Left) {
			mlCurrent--;
			if (mlCurrent < 0)
				mlCurrent = glTextureQualityNum - 1;
		}

		gpTextureQualityText->msText = kTranslate("MainMenu", gvTextureQuality[mlCurrent]);
		mpInit->mpGame->GetResources()->GetMaterialManager()->SetTextureSizeLevel(mlCurrent);

		gbMustRestart = true;
	}

	int mlCurrent;
};

//------------------------------------------------------------

class cMainMenuWidget_Resolution : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_Resolution(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGraphicsResolution");
	}

	void OnMouseDown(eMButton aButton) {
		int lCurrentNum = 0;

		// get current num
		for (int i = 0; i < glResolutionNum; ++i) {
			if (gvResolutions[i] == mpInit->mvScreenSize) {
				lCurrentNum = i;
				break;
			}
		}

		if (aButton == eMButton_Left) {
			lCurrentNum++;
			if (lCurrentNum >= glResolutionNum)
				lCurrentNum = 0;
		} else if (aButton == eMButton_Right) {
			lCurrentNum--;
			if (lCurrentNum < 0)
				lCurrentNum = glResolutionNum - 1;
		}

		mpInit->mvScreenSize = gvResolutions[lCurrentNum];

		char sTempVec[256];
		snprintf(sTempVec, 256, "%d x %d", mpInit->mvScreenSize.x, mpInit->mvScreenSize.y);
		gpResolutionText->msText = cString::To16Char(sTempVec);

		gbMustRestart = true;
	}
};

//-----------------------------------------------------------

class cMainMenuWidget_PostEffects : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_PostEffects(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGraphicsPostEffects");
	}

	void OnMouseDown(eMButton aButton) {
		bool bX = mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->GetActive();
		mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetActive(!bX);
		mpInit->mbPostEffects = !bX;

		gpPostEffectsText->msText = mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->GetActive() ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

//-----------------------------------------------------------

class cMainMenuWidget_Bloom : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_Bloom(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGraphicsBloom");
	}

	void OnMouseDown(eMButton aButton) {
		bool bX = mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->GetBloomActive();
		mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetBloomActive(!bX);

		gpBloomText->msText = mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->GetBloomActive() ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

//------------------------------------------------------------

class cMainMenuWidget_MotionBlur : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_MotionBlur(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		msTip = kTranslate("MainMenu", "TipGraphicsMotionBlur");
	}

	void OnMouseDown(eMButton aButton) {
		bool bX = mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->GetMotionBlurActive();
		mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetMotionBlurActive(!bX);

		gpMotionBlurText->msText = mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->GetMotionBlurActive() ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	}
};

//------------------------------------------------------------

class cMainMenuWidget_GfxBack : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_GfxBack(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize, eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
	}

	void OnMouseDown(eMButton aButton) {
		mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_menu_click", false, 1);
		if (gbMustRestart) {
			mpInit->mpMainMenu->SetState(eMainMenuState_GraphicsRestart);
			gbMustRestart = false;
		} else {
			mpInit->mpMainMenu->SetState(eMainMenuState_Options);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// OPTIONS KEY CONFIG
//////////////////////////////////////////////////////////////////////////

class cMainMenuWidget_KeyButton : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_KeyButton(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize,
							  eFontAlign aAlignment, cMainMenuWidget_Text *apKeyWiget,
							  const tString &asActionName)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
		mpKeyWidget = apKeyWiget;
		msActionName = asActionName;

		iAction *pAction = mpInit->mpGame->GetInput()->GetAction(asActionName);
		if (pAction) {
			tString sKeyName = pAction->GetInputName();
			mpKeyWidget->msText = kTranslate("ButtonNames", sKeyName);

			// If translation is missing, set to default
			if (mpKeyWidget->msText == _W(""))
				mpKeyWidget->msText = cString::To16Char(sKeyName);

		} else {
			mpKeyWidget->msText = kTranslate("MainMenu", "Empty");
			// FatalError("Action for %s button does not exist!\n",msText.c_str());
		}

		mpKeyWidget->SetExtraWidget(this);
		mpKeyWidget->UpdateSize();
	}

	void OnMouseDown(eMButton aButton) {
		mpKeyWidget->msText = _W(".....");
		mpInit->mpMainMenu->SetInputToAction(msActionName, mpKeyWidget);
	}

	void Reset() {
		iAction *pAction = mpInit->mpGame->GetInput()->GetAction(msActionName);

		if (pAction) {
			tString sKeyName = pAction->GetInputName();
			mpKeyWidget->msText = kTranslate("ButtonNames", sKeyName);

			// If translation is missing, set to default
			if (mpKeyWidget->msText == _W(""))
				mpKeyWidget->msText = cString::To16Char(sKeyName);

			mpKeyWidget->UpdateSize();
		} else {
			mpKeyWidget->msText = kTranslate("MainMenu", "Empty");
		}
	}

private:
	cMainMenuWidget_Text *mpKeyWidget;
	tString msActionName;
};

//------------------------------------------------------------

class cMainMenuWidget_KeyReset : public cMainMenuWidget_Button {
public:
	cMainMenuWidget_KeyReset(cInit *apInit, const cVector3f &avPos, const tWString &asText, cVector2f avFontSize,
							 eFontAlign aAlignment)
		: cMainMenuWidget_Button(apInit, avPos, asText, eMainMenuState_LastEnum, avFontSize, aAlignment) {
	}

	void OnMouseDown(eMButton aButton) {
		// Log("Setting deafult keys!\n");
		mpInit->mpButtonHandler->SetDefaultKeys();
		mpInit->mpMainMenu->ResetWidgets(eMainMenuState_OptionsKeySetupMove);
		mpInit->mpMainMenu->ResetWidgets(eMainMenuState_OptionsKeySetupAction);
		mpInit->mpMainMenu->ResetWidgets(eMainMenuState_OptionsKeySetupMisc);
	}

private:
	// cMainMenuWidget_Text *mpKeyWidget;
	tString msActionName;
};

//------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMainMenu::cMainMenu(cInit *apInit) : iUpdateable("MainMenu") {
	gvSaveGameFileVec = new tWStringVec[3];
	mState = eMainMenuState_Start;

	mpLogo = NULL;
	mpBackground = NULL;

	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	// Load graphics
	mpGfxBlackQuad = mpDrawer->CreateGfxObject("effect_black.bmp", "diffalpha2d");
	mpGfxMouse = mpDrawer->CreateGfxObject("player_crosshair_pointer.bmp", "diffalpha2d");

	mpGfxRainDrop = mpDrawer->CreateGfxObject("menu_rain_drop.jpg", "diffadditive2d");
	mpGfxRainSplash = mpDrawer->CreateGfxObject("menu_rain_splash.jpg", "diffadditive2d");
	mpGfxSnowFlake = mpDrawer->CreateGfxObject("menu_snow_flake.jpg", "diffadditive2d");

	// Init effects
	mvRainDrops.resize(70);
	mvRainSplashes.resize(180);
	for (size_t i = 0; i < mvRainSplashes.size(); ++i) {
		mvRainSplashes[i].mCol = cColor(1, 0);
		mvRainSplashes[i].mpGfx = mpGfxRainSplash;
	}
	for (size_t i = 0; i < mvRainDrops.size(); ++i) {
		mvRainDrops[i].mCol = cColor(1, 0);
		mvRainDrops[i].mpGfx = mpGfxRainDrop;
	}

	mvSnowFlakes.resize(80);
	for (size_t i = 0; i < mvSnowFlakes.size(); ++i) {
		mvSnowFlakes[i].mvPos = cVector3f(cMath::RandRectf(350, 800), cMath::RandRectf(200, 550), 20);
		mvSnowFlakes[i].mvVel = cVector3f(0, cMath::RandRectf(15, 40), 0);
		mvSnowFlakes[i].mvSize = cMath::RandRectf(2, 10);
		mvSnowFlakes[i].mpGfx = mpGfxSnowFlake;
	}

	// load fonts
	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("font_menu_small.fnt", 20, 32, 255);
	mpTipFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");

	//////////////////////////////////
	// Init widgets

	mvState.resize(eMainMenuState_LastEnum);

	Reset();
}

//-----------------------------------------------------------------------

cMainMenu::~cMainMenu(void) {
	STLDeleteAll(mlstWidgets);
	delete[] gvSaveGameFileVec;

	mpDrawer->DestroyGfxObject(mpGfxBlackQuad);
	mpDrawer->DestroyGfxObject(mpGfxMouse);
	mpDrawer->DestroyGfxObject(mpGfxRainDrop);
	mpDrawer->DestroyGfxObject(mpGfxRainSplash);
	mpDrawer->DestroyGfxObject(mpGfxSnowFlake);

	if (mpLogo)
		mpInit->mpGame->GetResources()->GetTextureManager()->Destroy(mpLogo);
	if (mpBackground)
		mpInit->mpGame->GetResources()->GetTextureManager()->Destroy(mpBackground);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cMainMenu::Reset() {
	mbActive = false;
	mbFadeIn = false;
	mfAlpha = 0;
	mfFadeAmount = 0;

	mbMouseIsDown = false;

	mbUpdateWidgets = false;

	mpCurrentActionText = NULL;

	mbGameActive = false;

	// Effects:
	mfRainDropCount = 0;
}

//-----------------------------------------------------------------------

void cMainMenu::OnPostSceneDraw() {
	mpInit->mpGraphicsHelper->ClearScreen(cColor(0, 0));

	mpInit->mpGraphicsHelper->DrawTexture(mpLogo, 0, cVector3f(800, 180, 30), cColor(1, 1));
	mpInit->mpGraphicsHelper->DrawTexture(mpBackground, cVector3f(0, 180, 0), cVector3f(800, 420, 0), cColor(1, 1));

	////////////////////////////////
	// Fade in
	if (mbFadeIn) {
		mpDrawer->DrawGfxObject(mpGfxBlackQuad, cVector3f(0, 0, 120), cVector2f(800, 600), cColor(1, 1 - mfFadeAmount));
	}
}

//-----------------------------------------------------------------------

void cMainMenu::OnDraw() {
	////////////////////////////////
	// Draw widgets
	tMainMenuWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		cMainMenuWidget *pWidget = *it;

		if (pWidget->IsActive())
			pWidget->OnDraw();
	}

	DrawBackground();

	////////////////////////////
	// Draw tip
	if (msButtonTip != _W("")) {
		mpTipFont->drawWordWrap(cVector3f(10, 570, 150), 780, 13, 12, cColor(1, 1),
								eFontAlign_Left, msButtonTip);
	}

	////////////////////////////////
	// Draw mouse
	if (mpCurrentActionText)
		return;
	cResourceImage *pImage = mpGfxMouse->GetMaterial()->GetImage(eMaterialTexture_Diffuse);
	cVector2l vSize = pImage->GetSize();
	cVector2f vPosAdd(((float)vSize.x) / 2.0f, ((float)vSize.y) / 2.0f);
	mpDrawer->DrawGfxObject(mpGfxMouse, cVector3f(0, 0, 100) + (mvMousePos - vPosAdd));
}

//-----------------------------------------------------------------------

static void DrawParticle(cGraphicsDrawer *apDrawer, cMainMenuParticle *apParticle) {
	apDrawer->DrawGfxObject(apParticle->mpGfx,
							apParticle->mvPos - apParticle->mvSize / 2,
							apParticle->mvSize,
							apParticle->mCol);
}

void cMainMenu::DrawBackground() {
	if (mbGameActive) {
		for (size_t i = 0; i < mvSnowFlakes.size(); ++i)
			DrawParticle(mpDrawer, &mvSnowFlakes[i]);
	} else {
		for (size_t i = 0; i < mvRainDrops.size(); ++i)
			DrawParticle(mpDrawer, &mvRainDrops[i]);
		for (size_t i = 0; i < mvRainSplashes.size(); ++i)
			DrawParticle(mpDrawer, &mvRainSplashes[i]);
	}
}

//-----------------------------------------------------------------------

void cMainMenu::Update(float afTimeStep) {
	if (mbFadeIn) {
		if (mfFadeAmount < 1)
			mfFadeAmount += 0.5f * afTimeStep;
		else {
			mbFadeIn = false;
			mfFadeAmount = 0;
		}
	}

	if (mbUpdateWidgets) {
		mbUpdateWidgets = false;
		CreateWidgets();
		SetState(mState);
	}

	if (mpCurrentActionText) {
		cInput *pInput = mpInit->mpGame->GetInput();

		if (CheckForInput()) {
			// Log("Creating action '%s'\n",msCurrentActionName.c_str());
			iAction *pAction = pInput->InputToAction(msCurrentActionName);

			mpCurrentActionText->msText = kTranslate("ButtonNames", pAction->GetInputName());

			// If translation is missing, set to default
			if (mpCurrentActionText->msText == _W(""))
				mpCurrentActionText->msText = cString::To16Char(pAction->GetInputName());

			mpCurrentActionText->UpdateSize();

			tString sAction = mpInit->mpButtonHandler->GetActionName(pAction->GetInputName(), msCurrentActionName);
			if (sAction != "") {
				pInput->DestroyAction(sAction);

				mpInit->mpMainMenu->ResetWidgets(eMainMenuState_OptionsKeySetupMove);
				mpInit->mpMainMenu->ResetWidgets(eMainMenuState_OptionsKeySetupAction);
				mpInit->mpMainMenu->ResetWidgets(eMainMenuState_OptionsKeySetupMisc);
			}

			mpCurrentActionText = NULL;
			Log("Reset check for input!\n");
		}

	} else {
	}

	////////////////////////////////
	// Update effect
	if (mbGameActive) {
		for (size_t i = 0; i < mvSnowFlakes.size(); ++i) {
			cMainMenuParticle &flake = mvSnowFlakes[i];

			if (flake.mvPos.y >= 600 - (150 - mvSnowFlakes[i].mvSize.x * 15)) {
				mvSnowFlakes[i].mvPos = cVector3f(cMath::RandRectf(350, 800), 200, 20);
				mvSnowFlakes[i].mvVel = cVector3f(0, cMath::RandRectf(15, 40), 0);
				mvSnowFlakes[i].mvSize = cMath::RandRectf(2, 10);
			}

			flake.mvPos += flake.mvVel * afTimeStep;
			flake.mvVel.x += cMath::RandRectf(-2, 2);
			if (flake.mvVel.x < -25)
				flake.mvVel.x = -25;
			if (flake.mvVel.x > 25)
				flake.mvVel.x = 25;

			float fDist = cMath::Vector3Dist(flake.mvPos, cVector3f(550, 550, 20));
			float fAlpha = 1 - fDist / 380;
			if (fAlpha < 0)
				fAlpha = 0;
			flake.mCol = cColor(1, fAlpha);
		}
	} else {
		// Rainsplashes
		for (size_t i = 0; i < mvRainSplashes.size(); ++i) {
			cMainMenuParticle &splash = mvRainSplashes[i];
			if (splash.mCol.a <= 0) {
				splash.mCol.a = 1;

				float fX, fY;

				if (i < 120) {
					fX = cMath::RandRectf(400, 700);
					fY = cMath::RandRectf(500, 550);
				} else if (i < 140) {
					fX = cMath::RandRectf(480, 605);
					fY = cMath::RandRectf(278, 320);
				} else if (i < 160) {
					fX = cMath::RandRectf(360, 460);
					fY = cMath::RandRectf(288, 330);
				} else {
					fX = cMath::RandRectf(615, 760);
					fY = cMath::RandRectf(258, 310);
				}

				splash.mvPos = cVector3f(fX, fY, 20);
				splash.mvSize = cMath::RandRectf(3, 16);

				splash.mpGfx = mpGfxRainSplash;

				splash.mvVel.x = cMath::RandRectf(1, 5);
			} else {
				splash.mCol.a -= afTimeStep * splash.mvVel.x;
				if (splash.mCol.a < 0)
					splash.mCol.a = 0;
			}
		}

		// Rain drops
		if (mfRainDropCount <= 0) {
			for (size_t i = 0; i < mvRainDrops.size(); ++i) {
				float fX = cMath::RandRectf(150, 800);
				float fY = cMath::RandRectf(180, 600);
				mvRainDrops[i].mvPos = cVector3f(fX, fY, 20);
				mvRainDrops[i].mvSize = cMath::RandRectf(28, 34);

				float fDist = cMath::Vector3Dist(mvRainDrops[i].mvPos, cVector3f(550, 400, 20));
				float fAlpha = 1 - fDist / 380;
				if (fAlpha < 0)
					fAlpha = 0;
				mvRainDrops[i].mCol = cColor(1, fAlpha);

				mvRainDrops[i].mpGfx = mpGfxRainDrop;
			}

			mfRainDropCount = 1.0f / 37.0f;
		} else {
			mfRainDropCount -= afTimeStep;
		}
	}

	////////////////////////////////
	// Update buttons
	msButtonTip = _W("");
	tMainMenuWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		cMainMenuWidget *pWidget = *it;

		if (pWidget->IsActive())
			pWidget->OnUpdate(afTimeStep);

		if (cMath::PointBoxCollision(mvMousePos, pWidget->GetRect())) {
			if (pWidget->IsActive())
				pWidget->OnMouseOver(true);
		} else {
			if (pWidget->IsActive())
				pWidget->OnMouseOver(false);
		}
	}
}

//-----------------------------------------------------------------------

void cMainMenu::AddMousePos(const cVector2f &avRel) {
	if (mpCurrentActionText)
		return;

	mvMousePos += avRel;

	if (mvMousePos.x < 0)
		mvMousePos.x = 0;
	if (mvMousePos.x >= 800)
		mvMousePos.x = 800;
	if (mvMousePos.y < 0)
		mvMousePos.y = 0;
	if (mvMousePos.y >= 600)
		mvMousePos.y = 600;
}

void cMainMenu::SetMousePos(const cVector2f &avPos) {
	if (mpCurrentActionText)
		return;

	mvMousePos = avPos;
}

//-----------------------------------------------------------------------

void cMainMenu::OnMouseDown(eMButton aButton) {
	if (mpCurrentActionText)
		return;

	////////////////////////////////
	// Update buttons
	tMainMenuWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		cMainMenuWidget *pWidget = *it;

		if (cMath::PointBoxCollision(mvMousePos, pWidget->GetRect())) {
			if (pWidget->IsActive()) {
				pWidget->OnMouseDown(aButton);
				break;
			}
		}
	}

	mbMouseIsDown = true;
}

void cMainMenu::OnMouseUp(eMButton aButton) {
	if (mpCurrentActionText)
		return;

	////////////////////////////////
	// Update buttons
	tMainMenuWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		cMainMenuWidget *pWidget = *it;

		if (cMath::PointBoxCollision(mvMousePos, pWidget->GetRect())) {
			if (pWidget->IsActive())
				pWidget->OnMouseUp(aButton);
		}
	}

	mbMouseIsDown = false;
}

//-----------------------------------------------------------------------

void cMainMenu::OnMouseDoubleClick(eMButton aButton) {
	if (mpCurrentActionText)
		return;

	////////////////////////////////
	// Update buttons
	tMainMenuWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		cMainMenuWidget *pWidget = *it;

		if (cMath::PointBoxCollision(mvMousePos, pWidget->GetRect())) {
			if (pWidget->IsActive())
				pWidget->OnDoubleClick(aButton);
		}
	}

	mbMouseIsDown = false;
}

//-----------------------------------------------------------------------

void cMainMenu::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;

	if (mbActive) {
		if (!mpInit->mbFullScreen) {
			mpInit->mpGame->GetInput()->GetLowLevel()->LockInput(false);
		}

		mpInit->mpGame->GetUpdater()->SetContainer("MainMenu");
		mpInit->mpGame->GetScene()->SetDrawScene(false);
		mpInit->mpGame->GetScene()->SetUpdateMap(false);

		mpInit->mpButtonHandler->ChangeState(eButtonHandlerState_MainMenu);

		///////////////////////////////
		// Init menu
		CreateWidgets();
		cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

		if (mpInit->mpMapHandler->GetCurrentMapName() != "") {
			mpInit->mpGame->GetSound()->GetSoundHandler()->PauseAll(eSoundDest_World | eSoundDest_Gui);
			mpInit->mpGame->GetSound()->GetMusicHandler()->Pause();

			mbGameActive = true;
			pSoundHandler->PlayGui("gui_wind1", true, 1);
		} else {
			mpInit->mpGame->GetSound()->GetMusicHandler()->Play("music_theme.ogg", 1, 5.0f, false);

			if (pSoundHandler->IsPlaying("gui_rain1") == false)
				pSoundHandler->PlayGui("gui_rain1", true, 1);

			mbGameActive = false;
			mbFadeIn = true;
		}

		bool bFirstStart = mpInit->mpConfig->GetBool("Game", "FirstStart", true);
		if (bFirstStart) {
			SetState(eMainMenuState_FirstStart);
			mLastState = eMainMenuState_FirstStart;

			mpInit->mpConfig->SetBool("Game", "FirstStart", false);
		} else {
			SetState(eMainMenuState_Start);
			mLastState = eMainMenuState_Start;
		}

		gbMustRestart = false;

		mpCurrentActionText = NULL;

		mpLogo = mpInit->mpGame->GetResources()->GetTextureManager()->Create2D("menu_logo.jpg", false);

		if (mbGameActive)
			mpBackground = mpInit->mpGame->GetResources()->GetTextureManager()->Create2D("menu_background_ingame.jpg", false);
		else
			mpBackground = mpInit->mpGame->GetResources()->GetTextureManager()->Create2D("menu_background.jpg", false);

	} else {
		if (!mpInit->mbFullScreen) {
			mpInit->mpGame->GetInput()->GetLowLevel()->LockInput(true);
		}

		cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

		if (mpInit->mpMapHandler->GetCurrentMapName() != "") {
			if (pSoundHandler->IsPlaying("gui_wind1"))
				pSoundHandler->Stop("gui_wind1");

			pSoundHandler->ResumeAll(eSoundDest_World | eSoundDest_Gui);
			mpInit->mpGame->GetSound()->GetMusicHandler()->Resume();
		} else {
			if (pSoundHandler->IsPlaying("gui_rain1"))
				pSoundHandler->Stop("gui_rain1");
			mpInit->mpGame->GetSound()->GetMusicHandler()->Stop(0.3f);
		}

		mpInit->mpGame->GetUpdater()->SetContainer("Default");
		mpInit->mpGame->GetScene()->SetDrawScene(true);
		mpInit->mpGame->GetScene()->SetUpdateMap(true);
		mpInit->mpButtonHandler->ChangeState(eButtonHandlerState_Game);

		if (mpLogo)
			mpInit->mpGame->GetResources()->GetTextureManager()->Destroy(mpLogo);
		mpLogo = NULL;
		if (mpBackground)
			mpInit->mpGame->GetResources()->GetTextureManager()->Destroy(mpBackground);
		mpBackground = NULL;
	}
}
//-----------------------------------------------------------------------

void cMainMenu::Exit() {
	if (mpCurrentActionText) {
		mpCurrentActionText = NULL;
	} else if (mState == eMainMenuState_Start && mpInit->mpMapHandler->GetCurrentMapName() != "") {
		SetActive(false);
	} else if ((mState == eMainMenuState_OptionsGraphics ||
				mState == eMainMenuState_OptionsSound ||
				mState == eMainMenuState_OptionsGame) &&
			   gbMustRestart) {
		SetState(eMainMenuState_GraphicsRestart);

		mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_menu_click", false, 1);

		gbMustRestart = false;
	} else {
		SetState(gvMenuBackStates[mState]);

		mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_menu_click", false, 1);
	}
}

//-----------------------------------------------------------------------

void cMainMenu::OnExit() {
	SetActive(false);
}

//-----------------------------------------------------------------------

void cMainMenu::SetState(eMainMenuState aState) {
	mLastState = mState;

	mState = aState;

	// Set all widgets as not active.
	tMainMenuWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		cMainMenuWidget *pWidget = *it;
		pWidget->SetActive(false);
	}

	// Set all widgets for current state as active.
	it = mvState[aState].begin();
	for (; it != mvState[aState].end(); ++it) {
		cMainMenuWidget *pWidget = *it;
		pWidget->SetActive(true);
	}
}

//-----------------------------------------------------------------------

void cMainMenu::SetInputToAction(const tString &asActionName, cMainMenuWidget_Text *apText) {
	msCurrentActionName = asActionName;
	mpCurrentActionText = apText;
	InitCheckInput();
}

//-----------------------------------------------------------------------

void cMainMenu::InitCheckInput() {
	cInput *pInput = mpInit->mpGame->GetInput();

	for (int i = 0; i < Common::KEYCODE_LAST; ++i) {
		mvKeyPressed[i] = pInput->GetKeyboard()->KeyIsDown(static_cast<Common::KeyCode>(i));
	}

	for (int i = 0; i < eMButton_LastEnum; ++i) {
		mvMousePressed[i] = pInput->GetMouse()->ButtonIsDown((eMButton)i);
	}
}

//-----------------------------------------------------------------------

bool cMainMenu::CheckForInput() {
	cInput *pInput = mpInit->mpGame->GetInput();

	////////////////////
	// Keyboard
	for (int i = 0; i < Common::KEYCODE_LAST; ++i) {
		if (pInput->GetKeyboard()->KeyIsDown(static_cast<Common::KeyCode>(i))) {
			if (mvKeyPressed[i] == false)
				return true;
		} else {
			mvKeyPressed[i] = false;
		}
	}

	////////////////////
	// Mouse
	for (int i = 0; i < eMButton_LastEnum; ++i) {
		if (pInput->GetMouse()->ButtonIsDown((eMButton)i)) {
			if (mvMousePressed[i] == false)
				return true;
		} else {
			mvMousePressed[i] = false;
		}
	}

	return false;
}

//-----------------------------------------------------------------------

void cMainMenu::ResetWidgets(eMainMenuState aState) {
	tMainMenuWidgetListIt it = mvState[aState].begin();
	for (; it != mvState[aState].end(); ++it) {
		cMainMenuWidget *pWidget = *it;
		pWidget->Reset();
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

class cTempFileAndData {
public:
	cTempFileAndData(const tWString &asFile, const cDate &aDate) {
		msFile = asFile;
		mDate = aDate;
	}

	bool operator<(const cTempFileAndData &aA) const {
		return mDate < aA.mDate;
	}
	bool operator>(const cTempFileAndData &aA) const {
		return mDate > aA.mDate;
	}
	bool operator==(const cTempFileAndData &aA) const {
		return mDate == aA.mDate;
	}

	tWString msFile;
	cDate mDate;
};

typedef Hpl1::Std::set<cTempFileAndData, Common::Greater<cTempFileAndData> > tTempFileAndDataSet;
typedef tTempFileAndDataSet::iterator tTempFileAndDataSetIt;

//-----------------------------------------------------------------------

void cMainMenu::CreateWidgets() {
	cVector3f vPos;
	tWString sText;
	cMainMenuWidget_Text *pTempTextWidget;
	char sTempVec[256];

	cVector3f vTextStart(220, 230, 40);

	///////////////////////////////
	// Erase all previous
	STLDeleteAll(mlstWidgets);
	for (size_t i = 0; i < eMainMenuState_LastEnum; ++i)
		mvState[i].clear();

	///////////////////////////////
	// First start
	//////////////////////////////
	bool bFirstStart = mpInit->mpConfig->GetBool("Game", "FirstStart", true);
	if (bFirstStart) {
		vPos = cVector3f(40, 190, 40);
		AddWidgetToState(eMainMenuState_FirstStart, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "Welcome"), 15, eFontAlign_Left)));
		vPos.y += 18;
		AddWidgetToState(eMainMenuState_FirstStart, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "Too Improve"), 15, eFontAlign_Left)));
		vPos.y += 28;
		AddWidgetToState(eMainMenuState_FirstStart, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "StartTip1"), 15, eFontAlign_Left)));
		vPos.y += 18;
		AddWidgetToState(eMainMenuState_FirstStart, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "StartTip2"), 15, eFontAlign_Left)));
		vPos.y += 18;
		AddWidgetToState(eMainMenuState_FirstStart, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "StartTip3"), 15, eFontAlign_Left)));
		vPos.y += 28;
		vPos.x = 395;
		cMainMenuWidget *pGammaFirstButton = hplNew(cMainMenuWidget_Gamma, (mpInit, vPos, kTranslate("MainMenu", "Gamma:"), 20, eFontAlign_Right, 1));
		AddWidgetToState(eMainMenuState_FirstStart, pGammaFirstButton);
		vPos.x = 405;
		snprintf(sTempVec, 256, "%.1f", mpInit->mpGame->GetGraphics()->GetLowLevel()->GetGammaCorrection());
		sText = cString::To16Char(sTempVec);
		gpGammaText2 = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
		AddWidgetToState(eMainMenuState_FirstStart, gpGammaText2);
		gpGammaText2->SetExtraWidget(pGammaFirstButton);

		vPos.y += 31;
		AddWidgetToState(eMainMenuState_FirstStart, hplNew(cMainMenuWidget_Image, (mpInit,
																				   cVector3f(250, vPos.y, 30),
																				   cVector2f(300, 200),
																				   "menu_gamma.bmp",
																				   "diffalpha2d",
																				   cColor(1, 1))));
		vPos.y += 205;
		// AddWidgetToState(eMainMenuState_FirstStart,hplNew( cMainMenuWidget_Text(mpInit,vPos,kTranslate("MainMenu", "StartTip4"),15,eFontAlign_Left));
		// vPos.y += 28;
		AddWidgetToState(eMainMenuState_FirstStart, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "OK"), eMainMenuState_Start, 20, eFontAlign_Center)));
	}

	///////////////////////////////
	// Start menu:
	//////////////////////////////
	vPos = vTextStart; // cVector3f(400, 260, 40);

	if (mpInit->mpMapHandler->GetCurrentMapName() != "") {
		AddWidgetToState(eMainMenuState_Start, hplNew(cMainMenuWidget_Resume, (mpInit, vPos, kTranslate("MainMenu", "Resume"))));
		vPos.y += 60;
	} else {
		tWString latestSave = mpInit->mpSaveHandler->GetLatest(_W("????:*"));

		if (latestSave != _W("")) {
			AddWidgetToState(eMainMenuState_Start, hplNew(cMainMenuWidget_MainButton, (mpInit, vPos, kTranslate("MainMenu", "Continue"), eMainMenuState_Continue)));
			vPos.y += 51;
		}
	}

	AddWidgetToState(eMainMenuState_Start, hplNew(cMainMenuWidget_MainButton, (mpInit, vPos, kTranslate("MainMenu", "New Game"), eMainMenuState_NewGame)));
	vPos.y += 51;
	AddWidgetToState(eMainMenuState_Start, hplNew(cMainMenuWidget_MainButton, (mpInit, vPos, kTranslate("MainMenu", "Load Game"), eMainMenuState_LoadGameSpot)));
	vPos.y += 51;
	AddWidgetToState(eMainMenuState_Start, hplNew(cMainMenuWidget_MainButton, (mpInit, vPos, kTranslate("MainMenu", "Options"), eMainMenuState_Options)));
	vPos.y += 51;
	AddWidgetToState(eMainMenuState_Start, hplNew(cMainMenuWidget_MainButton, (mpInit, vPos, kTranslate("MainMenu", "Exit"), eMainMenuState_Exit)));

	///////////////////////////////////
	// New Game
	///////////////////////////////////

	vPos = vTextStart; // cVector3f(400, 260, 40);
	// AddWidgetToState(eMainMenuState_NewGame,hplNew( cMainMenuWidget_Text(mpInit,vPos,kTranslate("MainMenu","StartNewGame"),24,eFontAlign_Center));
	// vPos.y += 34;
	AddWidgetToState(eMainMenuState_NewGame, hplNew(cMainMenuWidget_NewGame, (mpInit, vPos, kTranslate("MainMenu", "Easy"), 24, eFontAlign_Center, eGameDifficulty_Easy)));
	vPos.y += 30;
	AddWidgetToState(eMainMenuState_NewGame, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "EasyDesc"), 16, eFontAlign_Center)));
	vPos.y += 42;
	AddWidgetToState(eMainMenuState_NewGame, hplNew(cMainMenuWidget_NewGame, (mpInit, vPos, kTranslate("MainMenu", "Normal"), 24, eFontAlign_Center, eGameDifficulty_Normal)));
	vPos.y += 30;
	AddWidgetToState(eMainMenuState_NewGame, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "NormalDesc"), 16, eFontAlign_Center)));
	vPos.y += 42;
	AddWidgetToState(eMainMenuState_NewGame, hplNew(cMainMenuWidget_NewGame, (mpInit, vPos, kTranslate("MainMenu", "Hard"), 24, eFontAlign_Center, eGameDifficulty_Hard)));
	vPos.y += 30;
	AddWidgetToState(eMainMenuState_NewGame, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "HardDesc"), 16, eFontAlign_Center)));
	vPos.y += 46;
	AddWidgetToState(eMainMenuState_NewGame, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Back"), eMainMenuState_Start, 22, eFontAlign_Center)));

	///////////////////////////////////
	// Continue
	///////////////////////////////////
	vPos = vTextStart; // cVector3f(400, 260, 40);
	AddWidgetToState(eMainMenuState_Continue, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "ContinueLastSave"), 20, eFontAlign_Center)));
	vPos.y += 34;
	AddWidgetToState(eMainMenuState_Continue, hplNew(cMainMenuWidget_Continue, (mpInit, vPos, kTranslate("MainMenu", "Yes"), 20, eFontAlign_Center)));
	vPos.y += 29;
	AddWidgetToState(eMainMenuState_Continue, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "No"), eMainMenuState_Start, 20, eFontAlign_Center)));

	///////////////////////////////////
	// Load Game
	///////////////////////////////////

	for (size_t i = 0; i < 3; ++i) {
		vPos = vTextStart; // cVector3f(400, 260, 40);

		eMainMenuState state = (eMainMenuState)(eMainMenuState_LoadGameSpot + i);

		///////////////////////////
		// Head
		AddWidgetToState(state, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "Load Game"), 27, eFontAlign_Center)));
		vPos.y += 42;
		vPos.x -= 110;

		///////////////////////////
		// Buttons
		AddWidgetToState(state, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Saved Games"), eMainMenuState_LoadGameSpot, 25, eFontAlign_Center)));
		vPos.y += 32;
		AddWidgetToState(state, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Auto Saves"), eMainMenuState_LoadGameAuto, 25, eFontAlign_Center)));
		vPos.y += 32;
		AddWidgetToState(state, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Favorites"), eMainMenuState_LoadGameFavorite, 25, eFontAlign_Center)));

		///////////////////////////
		// Back
		vPos.y += 150;
		vPos.x += 130;
		vPos.y += 32;
		AddWidgetToState(state, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Back"), eMainMenuState_Start, 23, eFontAlign_Center)));

		///////////////////////////
		// Load type
		vPos = vTextStart; // cVector3f(400, 260, 40);
		vPos.y += 42;
		vPos.x += 185;
		tString sLoadType = "Saved Games";
		if (i == 1)
			sLoadType = "Auto Saves";
		if (i == 2)
			sLoadType = "Favorites";
		AddWidgetToState(state, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", sLoadType) + _W(":"), 21, eFontAlign_Center)));

		///////////////////////////
		// Saved games list

		// Set up
		vPos = vTextStart; // cVector3f(400, 260 , 40);
		vPos.y += 46 + 30;
		vPos.x += 15;

		tWString sDir("spot:");
		if (i == 1)
			sDir = _W("auto:");
		else if (i == 2)
			sDir = _W("favorite:");

		gpSaveGameList[i] = hplNew(cMainMenuWidget_SaveGameList, (
																	 mpInit, vPos, cVector2f(355, 170), 15, sDir, (int)i));
		AddWidgetToState(state, gpSaveGameList[i]);

		tTempFileAndDataSet setTempFiles;
		Common::StringArray saves = Hpl1::g_engine->listInternalSaves(sDir + "*");
		for (auto &s : saves) {
			tWString sFile = cString::To16Char(s.c_str());
			cDate date = cSaveHandler::parseDate(s);
			setTempFiles.insert(cTempFileAndData(sFile, date));
		}

		// Go through the sorted array and add to File vector and as list entries
		gvSaveGameFileVec[i].clear();
		tTempFileAndDataSetIt dateIt = setTempFiles.begin();
		for (; dateIt != setTempFiles.end(); ++dateIt) {
			const cTempFileAndData &temp = *dateIt;

			tWString sFile = temp.msFile;

			gvSaveGameFileVec[i].push_back(sFile);

			sFile = cString::SubW(sFile, sFile.find(':') + 1);
			gpSaveGameList[i]->AddEntry(sFile);
			// gpSaveGameList[i]->AddEntry(sFile);
		}

		///////////////////////////
		// Save game buttons
		vPos.y += 170;
		vPos.x = vTextStart.x + 20;

		AddWidgetToState(state, hplNew(cMainMenuWidget_LoadSaveGame, (mpInit, vPos, kTranslate("MainMenu", "Load"), 17, eFontAlign_Left, sDir, (int)i)));

		vPos.x += 70;
		if (i != 2)
			AddWidgetToState(state, hplNew(cMainMenuWidget_FavoriteSaveGame, (mpInit, vPos, kTranslate("MainMenu", "Add To Favorites"), 17, eFontAlign_Left, (int)i)));

		vPos.x += 205;
		AddWidgetToState(state, hplNew(cMainMenuWidget_RemoveSaveGame, (mpInit, vPos, kTranslate("MainMenu", "Remove"), 17, eFontAlign_Left, sDir, (int)i)));
	}

	///////////////////////////////////
	// Quit
	///////////////////////////////////
	vPos = vTextStart; // cVector3f(400, 260, 40);
	AddWidgetToState(eMainMenuState_Exit, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "SureQuit"), 20, eFontAlign_Center)));
	vPos.y += 34;
	AddWidgetToState(eMainMenuState_Exit, hplNew(cMainMenuWidget_Quit, (mpInit, vPos, kTranslate("MainMenu", "Yes"), 20, eFontAlign_Center)));
	vPos.y += 29;
	AddWidgetToState(eMainMenuState_Exit, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "No"), eMainMenuState_Start, 20, eFontAlign_Center)));

	///////////////////////////////////
	// Options
	///////////////////////////////////
	vPos = vTextStart; // cVector3f(400, 260, 40);
	AddWidgetToState(eMainMenuState_Options, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Controls"), eMainMenuState_OptionsControls, 25, eFontAlign_Center)));
	vPos.y += 37;
	AddWidgetToState(eMainMenuState_Options, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Game"), eMainMenuState_OptionsGame, 25, eFontAlign_Center)));
	vPos.y += 37;
	AddWidgetToState(eMainMenuState_Options, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Graphics"), eMainMenuState_OptionsGraphics, 25, eFontAlign_Center)));
	vPos.y += 37;
	AddWidgetToState(eMainMenuState_Options, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Back"), eMainMenuState_Start, 25, eFontAlign_Center)));

	///////////////////////////////////
	// Options Controls
	///////////////////////////////////
	vPos = vTextStart; // cVector3f(400, 260, 40);
	// Head
	AddWidgetToState(eMainMenuState_OptionsControls, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "Controls"), 25, eFontAlign_Center)));
	vPos.y += 37;
	// Buttons
	cMainMenuWidget *pWidgetInvertMouseY = hplNew(cMainMenuWidget_InvertMouseY, (mpInit, vPos, kTranslate("MainMenu", "Invert Mouse Y:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsControls, pWidgetInvertMouseY);
	vPos.y += 29;
	cMainMenuWidget *pWidgetMouseSensitivity = hplNew(cMainMenuWidget_MouseSensitivity, (mpInit, vPos, kTranslate("MainMenu", "Mouse Sensitivity:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsControls, pWidgetMouseSensitivity);
	vPos.y += 29;
	cMainMenuWidget *pWidgetToggleCrouch = hplNew(cMainMenuWidget_ToggleCrouch, (mpInit, vPos, kTranslate("MainMenu", "Toggle Crouch:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsControls, pWidgetToggleCrouch);
	vPos.y += 29;
	// cMainMenuWidget *pWidgetChangeKeyConf = hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Change Key Mapping"), eMainMenuState_OptionsKeySetupMove, 20, eFontAlign_Center));
	// AddWidgetToState(eMainMenuState_OptionsControls, pWidgetChangeKeyConf);
	// vPos.y += 35;
	AddWidgetToState(eMainMenuState_OptionsControls, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Back"), eMainMenuState_Options, 23, eFontAlign_Center)));

	// Text
	vPos = cVector3f(vTextStart.x + 12, vTextStart.y + 37, vTextStart.z);

	sText = mpInit->mpButtonHandler->mbInvertMouseY ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	gpInvertMouseYText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left, pWidgetInvertMouseY));
	AddWidgetToState(eMainMenuState_OptionsControls, gpInvertMouseYText);

	vPos.y += 29;
	snprintf(sTempVec, 256, "%.1f", mpInit->mpButtonHandler->mfMouseSensitivity);
	sText = cString::To16Char(sTempVec);
	gpMouseSensitivityText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left, pWidgetMouseSensitivity));
	AddWidgetToState(eMainMenuState_OptionsControls, gpMouseSensitivityText);

	vPos.y += 29;
	sText = mpInit->mpButtonHandler->mbToggleCrouch ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	gpToggleCrouchText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left, pWidgetToggleCrouch));
	AddWidgetToState(eMainMenuState_OptionsControls, gpToggleCrouchText);

	///////////////////////////////////
	// Options Key Setup General stuff
	///////////////////////////////////
	for (int i = 0; i < 3; ++i) {
		eMainMenuState state = (eMainMenuState)(i + eMainMenuState_OptionsKeySetupMove);
		cVector3f vPos2 = vTextStart; // cVector3f(400, 260, 40);
		// Head
		AddWidgetToState(state, hplNew(cMainMenuWidget_Text, (mpInit, vPos2, kTranslate("MainMenu", "Configure Keys"), 25, eFontAlign_Center)));
		vPos2.y += 42;
		vPos2.x -= 110;
		// Buttons
		AddWidgetToState(state, hplNew(cMainMenuWidget_Button, (mpInit, vPos2, kTranslate("MainMenu", "Movement"), eMainMenuState_OptionsKeySetupMove, 25, eFontAlign_Center)));
		vPos2.y += 32;
		AddWidgetToState(state, hplNew(cMainMenuWidget_Button, (mpInit, vPos2, kTranslate("MainMenu", "Actions"), eMainMenuState_OptionsKeySetupAction, 25, eFontAlign_Center)));
		vPos2.y += 32;
		AddWidgetToState(state, hplNew(cMainMenuWidget_Button, (mpInit, vPos2, kTranslate("MainMenu", "Misc"), eMainMenuState_OptionsKeySetupMisc, 25, eFontAlign_Center)));
		// Back
		vPos2.y += 150;
		vPos2.x += 130;
		AddWidgetToState(state, hplNew(cMainMenuWidget_KeyReset, (mpInit, vPos2, kTranslate("MainMenu", "Reset to defaults"), 23, eFontAlign_Center)));
		vPos2.y += 32;
		AddWidgetToState(state, hplNew(cMainMenuWidget_Button, (mpInit, vPos2, kTranslate("MainMenu", "Back"), eMainMenuState_OptionsControls, 23, eFontAlign_Center)));
	}

	///////////////////////////////////
	// Options Key Setup Move
	///////////////////////////////////

	cMainMenuWidget *pWidgetKeyButton;
	float fKeyTextXAdd = 195;

	// Key buttons
	/*cInput *pInput = */ mpInit->mpGame->GetInput();
	vPos = vTextStart; // cVector3f(400, 260, 40);
	vPos.y += 46;
	vPos.x += 15;

	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pTempTextWidget);
	pWidgetKeyButton = hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Forward:"),
														  18, eFontAlign_Left, pTempTextWidget, "Forward"));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pWidgetKeyButton);

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pTempTextWidget);
	pWidgetKeyButton = hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Backward:"),
														  18, eFontAlign_Left, pTempTextWidget, "Backward"));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pWidgetKeyButton);

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pTempTextWidget);
	pWidgetKeyButton = hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Strafe Left:"),
														  18, eFontAlign_Left, pTempTextWidget, "Left"));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pWidgetKeyButton);

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pTempTextWidget);
	pWidgetKeyButton = hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Strafe Right:"),
														  18, eFontAlign_Left, pTempTextWidget, "Right"));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pWidgetKeyButton);

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pTempTextWidget);
	pWidgetKeyButton = hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Run:"),
														  18, eFontAlign_Left, pTempTextWidget, "Run"));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pWidgetKeyButton);

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pTempTextWidget);
	pWidgetKeyButton = hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Crouch:"),
														  18, eFontAlign_Left, pTempTextWidget, "Crouch"));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pWidgetKeyButton);

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pTempTextWidget);
	pWidgetKeyButton = hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Jump:"),
														  18, eFontAlign_Left, pTempTextWidget, "Jump"));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pWidgetKeyButton);

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pTempTextWidget);
	pWidgetKeyButton = hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Lean Left:"),
														  18, eFontAlign_Left, pTempTextWidget, "LeanLeft"));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pWidgetKeyButton);

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pTempTextWidget);
	pWidgetKeyButton = hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Lean Right:"),
														  18, eFontAlign_Left, pTempTextWidget, "LeanRight"));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMove, pWidgetKeyButton);

	///////////////////////////////////
	// Options Key Setup Action
	///////////////////////////////////
	// Key buttons
	vPos = vTextStart; // cVector3f(400, 260, 40);
	vPos.y += 46;
	vPos.x += 15;

	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupAction, pTempTextWidget);
	AddWidgetToState(eMainMenuState_OptionsKeySetupAction, hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Interact:"),
																							  18, eFontAlign_Left, pTempTextWidget, "Interact")));

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupAction, pTempTextWidget);
	AddWidgetToState(eMainMenuState_OptionsKeySetupAction, hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Examine:"),
																							  18, eFontAlign_Left, pTempTextWidget, "Examine")));

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupAction, pTempTextWidget);
	AddWidgetToState(eMainMenuState_OptionsKeySetupAction, hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "InteractMode:"),
																							  18, eFontAlign_Left, pTempTextWidget, "InteractMode")));

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupAction, pTempTextWidget);
	AddWidgetToState(eMainMenuState_OptionsKeySetupAction, hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Holster:"),
																							  18, eFontAlign_Left, pTempTextWidget, "Holster")));

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupAction, pTempTextWidget);
	AddWidgetToState(eMainMenuState_OptionsKeySetupAction, hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "LookMode:"),
																							  18, eFontAlign_Left, pTempTextWidget, "LookMode")));

	///////////////////////////////////
	// Options Key Setup Misc
	///////////////////////////////////
	// Key buttons
	vPos = vTextStart; // cVector3f(400, 260, 40);
	vPos.y += 46;
	vPos.x += 15;

	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMisc, pTempTextWidget);
	AddWidgetToState(eMainMenuState_OptionsKeySetupMisc, hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Inventory:"),
																							18, eFontAlign_Left, pTempTextWidget, "Inventory")));

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMisc, pTempTextWidget);
	AddWidgetToState(eMainMenuState_OptionsKeySetupMisc, hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Notebook:"),
																							18, eFontAlign_Left, pTempTextWidget, "NoteBook")));

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMisc, pTempTextWidget);
	AddWidgetToState(eMainMenuState_OptionsKeySetupMisc, hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Pers. Notes:"),
																							18, eFontAlign_Left, pTempTextWidget, "PersonalNotes")));

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMisc, pTempTextWidget);
	AddWidgetToState(eMainMenuState_OptionsKeySetupMisc, hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Flashlight:"),
																							18, eFontAlign_Left, pTempTextWidget, "Flashlight")));

	vPos.y += 23;
	pTempTextWidget = hplNew(cMainMenuWidget_Text, (mpInit, vPos + cVector3f(fKeyTextXAdd, 0, 0), Common::U32String(), 18, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsKeySetupMisc, pTempTextWidget);
	AddWidgetToState(eMainMenuState_OptionsKeySetupMisc, hplNew(cMainMenuWidget_KeyButton, (mpInit, vPos, kTranslate("MainMenu", "Glowstick:"),
																							18, eFontAlign_Left, pTempTextWidget, "GlowStick")));

	///////////////////////////////////
	// Options Game
	///////////////////////////////////
	vPos = vTextStart; // cVector3f(400, 260, 40);
	// Head
	AddWidgetToState(eMainMenuState_OptionsGame, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "Game"), 25, eFontAlign_Center)));
	vPos.y += 37;
	// Buttons
	cMainMenuWidget *pWidgetLanguage = hplNew(cMainMenuWidget_Language, (mpInit, vPos, kTranslate("MainMenu", "Language:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGame, pWidgetLanguage);
	vPos.y += 29;
	cMainMenuWidget *pWidgetSubtitles = hplNew(cMainMenuWidget_Subtitles, (mpInit, vPos, kTranslate("MainMenu", "Subtitle:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGame, pWidgetSubtitles);
	vPos.y += 50;
	cMainMenuWidget *pWidgetDifficulty = hplNew(cMainMenuWidget_Difficulty, (mpInit, vPos, kTranslate("MainMenu", "Difficulty:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGame, pWidgetDifficulty);
	vPos.y += 29;
	if (mpInit->mbSimpleSwingInOptions) {
		AddWidgetToState(eMainMenuState_OptionsGame, hplNew(cMainMenuWidget_SimpleSwing, (mpInit, vPos, kTranslate("MainMenu", "SimpleSwing:"), 20, eFontAlign_Right)));
		vPos.y += 29;
	}
	// AddWidgetToState(eMainMenuState_OptionsGame,hplNew( cMainMenuWidget_AllowQuickSave(mpInit,vPos,kTranslate("MainMenu","AllowQuickSave:"),20,eFontAlign_Right));
	// vPos.y += 29;
	tWString sCrosshairText = kTranslate("MainMenu", "Show Crosshair:");
	if (sCrosshairText == _W(""))
		sCrosshairText = _W("Show Crosshair:");
	cMainMenuWidget *pWidgetShowCrossHair = hplNew(cMainMenuWidget_ShowCrossHair, (mpInit, vPos, sCrosshairText, 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGame, pWidgetShowCrossHair);
	vPos.y += 29;

	cMainMenuWidget *pWidgetFlashItems = hplNew(cMainMenuWidget_FlashItems, (mpInit, vPos, kTranslate("MainMenu", "FlashItems:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGame, pWidgetFlashItems);
	vPos.y += 29;

	cMainMenuWidget *pWidgetDisablePersonal = hplNew(cMainMenuWidget_DisablePersonal, (mpInit, vPos, kTranslate("MainMenu", "DisablePersonal:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGame, pWidgetDisablePersonal);
	vPos.y += 35;
	AddWidgetToState(eMainMenuState_OptionsGame, hplNew(cMainMenuWidget_GfxBack, (mpInit, vPos, kTranslate("MainMenu", "Back"), 23, eFontAlign_Center)));

	// Text
	vPos = cVector3f(vTextStart.x + 12, vTextStart.y + 37, vTextStart.z);

	sText = cString::To16Char(cString::SetFileExt(mpInit->msLanguageFile, ""));
	gpLanguageText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGame, gpLanguageText);
	gpLanguageText->SetExtraWidget(pWidgetLanguage);

	vPos.y += 29;
	sText = mpInit->mbSubtitles ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	gpSubtitlesText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGame, gpSubtitlesText);
	gpSubtitlesText->SetExtraWidget(pWidgetSubtitles);

	vPos.y += 25;
	AddWidgetToState(eMainMenuState_OptionsGame, hplNew(cMainMenuWidget_Text, (mpInit, cVector3f(vPos - cVector3f(12, 0, 0)),
																			   kTranslate("MainMenu", "VoiceLanguange:"), 12, eFontAlign_Right)));
	AddWidgetToState(eMainMenuState_OptionsGame, hplNew(cMainMenuWidget_Text, (mpInit, cVector3f(vPos),
																			   kTranslate("MainMenu", "SetThisToLanguageOfVoice"), 12, eFontAlign_Left)));
	vPos.y += 25;

	sText = kTranslate("MainMenu", gvDifficultyLevel[mpInit->mDifficulty]);
	gpDifficultyText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGame, gpDifficultyText);
	gpDifficultyText->SetExtraWidget(pWidgetDifficulty);
	vPos.y += 29;

	if (mpInit->mbSimpleSwingInOptions) {
		sText = mpInit->mbSimpleWeaponSwing ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
		gpSimpleSwingText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
		AddWidgetToState(eMainMenuState_OptionsGame, gpSimpleSwingText);
		vPos.y += 29;
	}

	// sText = mpInit->mbAllowQuickSave ? kTranslate("MainMenu","On") : kTranslate("MainMenu","Off");
	// gpAllowQuickSaveText = hplNew( cMainMenuWidget_Text, (mpInit,vPos,sText,20,eFontAlign_Left) );
	// AddWidgetToState(eMainMenuState_OptionsGame,gpAllowQuickSaveText);
	// vPos.y += 29;
	sText = mpInit->mbShowCrossHair ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	gpShowCrossHairText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGame, gpShowCrossHairText);
	gpShowCrossHairText->SetExtraWidget(pWidgetShowCrossHair);
	vPos.y += 29;

	sText = mpInit->mbFlashItems ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	gpFlashItemsText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGame, gpFlashItemsText);
	gpFlashItemsText->SetExtraWidget(pWidgetFlashItems);

	vPos.y += 29;
	sText = mpInit->mbDisablePersonalNotes ? kTranslate("MainMenu", "Off") : kTranslate("MainMenu", "On");
	gpDisablePersonalText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGame, gpDisablePersonalText);
	gpDisablePersonalText->SetExtraWidget(pWidgetDisablePersonal);

	///////////////////////////////////
	// Options Graphics
	///////////////////////////////////
	vPos = vTextStart; // cVector3f(400, 230, 40);
	// Head
	AddWidgetToState(eMainMenuState_OptionsGraphics, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "Graphics"), 25, eFontAlign_Center)));
	vPos.y += 37;
	/*AddWidgetToState(eMainMenuState_OptionsGraphics,hplNew( cMainMenuWidget_Image, (mpInit,
													cVector3f(400,vPos.y,30),
													cVector2f(200,150),
													"menu_gamma.bmp",
													"diffalpha2d",
													cColor(1,1))) );

	//Buttons
	vPos.x -= 130;*/
	cMainMenuWidget *pWidgetResolution = hplNew(cMainMenuWidget_Resolution, (mpInit, vPos, kTranslate("MainMenu", "Resolution:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphics, pWidgetResolution);
	vPos.y += 29;
	cMainMenuWidget *pWidgetNoiseFilter = hplNew(cMainMenuWidget_NoiseFilter, (mpInit, vPos, kTranslate("MainMenu", "Noise Filter:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphics, pWidgetNoiseFilter);
	vPos.y += 29;
	cMainMenuWidget *pWidgetBloom = hplNew(cMainMenuWidget_Bloom, (mpInit, vPos, kTranslate("MainMenu", "Bloom:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphics, pWidgetBloom);
	vPos.y += 29;
	cMainMenuWidget *pWidgetGamma = hplNew(cMainMenuWidget_Gamma, (mpInit, vPos, kTranslate("MainMenu", "Gamma:"), 20, eFontAlign_Right, 0));
	AddWidgetToState(eMainMenuState_OptionsGraphics, pWidgetGamma);
	vPos.y += 29;
	cMainMenuWidget *pWidgetShaderQuality = hplNew(cMainMenuWidget_ShaderQuality, (mpInit, vPos, kTranslate("MainMenu", "Shader Quality:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphics, pWidgetShaderQuality);

	// vPos.x = 400;
	// vPos.y = 230 + 150;
	vPos.y += 35;
	AddWidgetToState(eMainMenuState_OptionsGraphics, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Advanced"), eMainMenuState_OptionsGraphicsAdvanced, 23, eFontAlign_Center)));

	vPos.y += 35;
	AddWidgetToState(eMainMenuState_OptionsGraphics, hplNew(cMainMenuWidget_GfxBack, (mpInit, vPos, kTranslate("MainMenu", "Back"), 23, eFontAlign_Center)));

	// Text
	vPos = cVector3f(vTextStart.x + 12, vTextStart.y + 37, vTextStart.z);

	snprintf(sTempVec, 256, "%d x %d", mpInit->mvScreenSize.x, mpInit->mvScreenSize.y);
	sText = cString::To16Char(sTempVec);
	gpResolutionText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphics, gpResolutionText);
	gpResolutionText->SetExtraWidget(pWidgetResolution);

	vPos.y += 29;
	sText = mpInit->mpPlayer->GetNoiseFilter()->IsActive() ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	gpNoiseFilterText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphics, gpNoiseFilterText);
	gpNoiseFilterText->SetExtraWidget(pWidgetNoiseFilter);

	vPos.y += 29;
	sText = mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->GetBloomActive() ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	gpBloomText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphics, gpBloomText);
	gpBloomText->SetExtraWidget(pWidgetBloom);

	vPos.y += 29;
	snprintf(sTempVec, 256, "%.1f", mpInit->mpGame->GetGraphics()->GetLowLevel()->GetGammaCorrection());
	sText = cString::To16Char(sTempVec);
	gpGammaText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphics, gpGammaText);
	gpGammaText->SetExtraWidget(pWidgetGamma);

	vPos.y += 29;
	sText = kTranslate("MainMenu", gvShaderQuality[iMaterial::GetQuality()]);
	gpShaderQualityText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphics, gpShaderQualityText);
	gpShaderQualityText->SetExtraWidget(pWidgetShaderQuality);

	///////////////////////////////////
	// Options Advanced Graphics
	///////////////////////////////////
	vPos = vTextStart + cVector3f(40, 0, 0); // cVector3f(400, 260, 40);
	// Head
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "Advanced Graphics"), 25, eFontAlign_Center)));
	vPos.y += 37;

	// Buttons
	cMainMenuWidget *pTextureQualityButton = hplNew(cMainMenuWidget_TextureQuality, (mpInit, vPos, kTranslate("MainMenu", "Texture Quality:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, pTextureQualityButton);
	vPos.y += 29;
	cMainMenuWidget *pShadowsButton = hplNew(cMainMenuWidget_Shadows, (mpInit, vPos, kTranslate("MainMenu", "Shadows:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, pShadowsButton);
	vPos.y += 29;
	cMainMenuWidget *pPostEffectsButton = hplNew(cMainMenuWidget_PostEffects, (mpInit, vPos, kTranslate("MainMenu", "Post Effects:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, pPostEffectsButton);
	vPos.y += 29;
	cMainMenuWidget *pMotionBlurButton = hplNew(cMainMenuWidget_MotionBlur, (mpInit, vPos, kTranslate("MainMenu", "Motion Blur:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, pMotionBlurButton);
	vPos.y += 29;
	cMainMenuWidget *pTextureFilterButton = hplNew(cMainMenuWidget_TextureFilter, (mpInit, vPos, kTranslate("MainMenu", "Texture Filter:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, pTextureFilterButton);
	vPos.y += 29;
	cMainMenuWidget *pTextureAnisotropyButton = hplNew(cMainMenuWidget_TextureAnisotropy, (mpInit, vPos, kTranslate("MainMenu", "Anisotropy:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, pTextureAnisotropyButton);
	vPos.y += 29;
	cMainMenuWidget *pFSAAButton = hplNew(cMainMenuWidget_FSAA, (mpInit, vPos, kTranslate("MainMenu", "Anti-Aliasing:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, pFSAAButton);
	vPos.y += 29;
	cMainMenuWidget *pDOFButton = hplNew(cMainMenuWidget_DOF, (mpInit, vPos, kTranslate("MainMenu", "Depth of Field:"), 20, eFontAlign_Right));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, pDOFButton);
	vPos.y += 35;
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "Back"), eMainMenuState_OptionsGraphics, 23, eFontAlign_Center)));

	// Text
	vPos = cVector3f(vTextStart.x + 12, vTextStart.y + 37, vTextStart.z) + cVector3f(40, 0, 0);

	sText = kTranslate("MainMenu", gvTextureQuality[mpInit->mpGame->GetResources()->GetMaterialManager()->GetTextureSizeLevel()]);
	gpTextureQualityText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, gpTextureQualityText);
	gpTextureQualityText->SetExtraWidget(pTextureQualityButton);

	vPos.y += 29;
	sText = kTranslate("MainMenu", gvShadowTypes[mpInit->mpGame->GetGraphics()->GetRenderer3D()->GetShowShadows()]);
	gpShadowsText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, gpShadowsText);
	gpShadowsText->SetExtraWidget(pShadowsButton);

	vPos.y += 29;
	sText = mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->GetActive() ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	gpPostEffectsText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, gpPostEffectsText);
	gpPostEffectsText->SetExtraWidget(pPostEffectsButton);

	vPos.y += 29;
	sText = mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->GetMotionBlurActive() ? kTranslate("MainMenu", "On") : kTranslate("MainMenu", "Off");
	gpMotionBlurText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, gpMotionBlurText);
	gpMotionBlurText->SetExtraWidget(pMotionBlurButton);

	vPos.y += 29;
	sText = kTranslate("MainMenu", gvTextureFilter[mpInit->mpGame->GetResources()->GetMaterialManager()->GetTextureFilter()]);
	gpTextureFilterText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, gpTextureFilterText);
	gpTextureFilterText->SetExtraWidget(pTextureFilterButton);

	vPos.y += 29;
	int lAniDeg = (int)mpInit->mpGame->GetResources()->GetMaterialManager()->GetTextureAnisotropy();
	if (lAniDeg != 1)
		sText = cString::To16Char(cString::ToString(lAniDeg) + "x");
	else
		sText = kTranslate("MainMenu", "Off");
	gpTextureAnisotropyText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, gpTextureAnisotropyText);
	gpTextureAnisotropyText->SetExtraWidget(pTextureAnisotropyButton);

	vPos.y += 29;
	int lFSAA = mpInit->mlFSAA;
	if (lFSAA != 0)
		sText = cString::To16Char(cString::ToString(lFSAA) + "x");
	else
		sText = kTranslate("MainMenu", "Off");
	gpFSAAText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, gpFSAAText);
	gpFSAAText->SetExtraWidget(pFSAAButton);

	vPos.y += 29;
	sText = mpInit->mpEffectHandler->GetDepthOfField()->IsDisabled() ? kTranslate("MainMenu", "Off") : kTranslate("MainMenu", "On");

	gpDoFText = hplNew(cMainMenuWidget_Text, (mpInit, vPos, sText, 20, eFontAlign_Left));
	AddWidgetToState(eMainMenuState_OptionsGraphicsAdvanced, gpDoFText);
	gpDoFText->SetExtraWidget(pDOFButton);

	///////////////////////////////////
	// Graphics Restart
	///////////////////////////////////

	vPos = vTextStart; // cVector3f(400, 260, 40);
	AddWidgetToState(eMainMenuState_GraphicsRestart, hplNew(cMainMenuWidget_Text, (mpInit, vPos, kTranslate("MainMenu", "GraphicsRestart"), 16, eFontAlign_Center,
																				   NULL, 400)));
	vPos.y += 42;
	AddWidgetToState(eMainMenuState_GraphicsRestart, hplNew(cMainMenuWidget_Button, (mpInit, vPos, kTranslate("MainMenu", "OK"), eMainMenuState_Options, 22, eFontAlign_Center)));
}

//-----------------------------------------------------------------------

void cMainMenu::AddWidgetToState(eMainMenuState aState, cMainMenuWidget *apWidget) {
	mlstWidgets.push_back(apWidget);
	mvState[aState].push_back(apWidget);
}

//---------------------------------------------------------------------
