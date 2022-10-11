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

#include "hpl1/penumbra-overture/NumericalPanel.h"

#include "hpl1/penumbra-overture/HapticGameCamera.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Player.h"

//////////////////////////////////////////////////////////////////////////
// BUTTON
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cNumericalButton::cNumericalButton(cInit *apInit, cNumericalPanel *apPanel, cVector2f avPos,
								   cVector2f avSize, int alNum) {
	mpInit = apInit;
	mpPanel = apPanel;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	mvPositon = cVector3f(avPos.x, avPos.y, 40);

	mRect.x = avPos.x;
	mRect.y = avPos.y;
	mRect.w = avSize.x;
	mRect.h = avSize.y;

	mpGfxUp = mpDrawer->CreateGfxObject("numpanel_button_up", "diffalpha2d");
	mpGfxDown = mpDrawer->CreateGfxObject("numpanel_button_down", "diffalpha2d");

	mlNum = alNum;

	mfAlpha = 0;
	mbOver = false;
}

cNumericalButton::~cNumericalButton() {
}

//-----------------------------------------------------------------------

void cNumericalButton::OnUpdate(float afTimeStep) {
	if (mbOver) {
		mfAlpha += 0.8f * afTimeStep;
		if (mfAlpha > 1)
			mfAlpha = 1;
	} else {
		mfAlpha -= 1.6f * afTimeStep;
		if (mfAlpha < 0)
			mfAlpha = 0;
	}
}

//-----------------------------------------------------------------------

void cNumericalButton::OnDraw() {
	if (mbOver && mpPanel->mbMouseIsDown)
		mpDrawer->DrawGfxObject(mpGfxDown, mvPositon, cVector2f(mRect.w, mRect.h), cColor(1, mpPanel->mfAlpha));
	else
		mpDrawer->DrawGfxObject(mpGfxUp, mvPositon, cVector2f(mRect.w, mRect.h), cColor(1, mpPanel->mfAlpha));

	cVector3f vFontPos = mvPositon + cVector3f(mRect.w / 2, 1, 1);

	tWString sChar;
	if (mlNum == -1)
		sChar = _W("*");
	else if (mlNum == -2)
		sChar = _W("#");
	else
		sChar = cString::To16Char(cString::ToString(mlNum));

	if (mbOver && mpPanel->mbMouseIsDown) {
		mpInit->mpDefaultFont->draw(vFontPos + cVector3f(1, 1, 0), 17, cColor(0, 0, 0, 0.9f * mpPanel->mfAlpha), eFontAlign_Center,
									sChar);
	} else {
		mpInit->mpDefaultFont->draw(vFontPos, 17, cColor(0, 0, 0, 0.9f * mpPanel->mfAlpha), eFontAlign_Center,
									sChar);
	}
}

//-----------------------------------------------------------------------

void cNumericalButton::OnMouseDown() {
	mpPanel->AddDigit(mlNum);

	mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_numpanel_button", false, 1);
}

//-----------------------------------------------------------------------

void cNumericalButton::OnMouseUp() {
}

//-----------------------------------------------------------------------

void cNumericalButton::OnMouseOver(bool abOver) {
	mbOver = abOver;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cNumericalPanel::cNumericalPanel(cInit *apInit) : iUpdateable("NumericalPanel") {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	// Load graphics (use notebook background for now).
	mpGfxBackground = mpDrawer->CreateGfxObject("notebook_background.bmp", "diffalpha2d");

	mpGfxPanel = mpDrawer->CreateGfxObject("numpanel_panel.bmp", "diffalpha2d");

	cVector2f vPos(307, 205);

	for (int i = 1; i <= 12; ++i) {
		int lNum = i;
		if (i == 10)
			lNum = -1;
		else if (i == 11)
			lNum = 0;
		else if (i == 12)
			lNum = -2;

		mlstButtons.push_back(hplNew(cNumericalButton, (mpInit, this, vPos, cVector2f(62, 30), lNum)));

		if (i % 3 == 0) {
			vPos.y += (30 + 16);
			vPos.x = 307;
		} else {
			vPos.x += 62 + 16;
		}
	}

	Reset();
}

//-----------------------------------------------------------------------

cNumericalPanel::~cNumericalPanel(void) {
	STLDeleteAll(mlstButtons);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cNumericalPanel::Reset() {
	mbActive = false;
	mfAlpha = 0;
	mbMouseIsDown = false;
}

//-----------------------------------------------------------------------

void cNumericalPanel::OnDraw() {
	if (mfAlpha == 0)
		return;

	mpDrawer->DrawGfxObject(mpGfxBackground, cVector3f(0, 0, 0), cVector2f(800, 600), cColor(1, mfAlpha));

	mpDrawer->DrawGfxObject(mpGfxPanel, cVector3f(280, 170, 10), cVector2f(270, 300), cColor(1, mfAlpha));

	////////////////////////////////
	// Update buttons
	tNumericalButtonListIt it = mlstButtons.begin();
	for (; it != mlstButtons.end(); ++it) {
		cNumericalButton *pButton = *it;

		pButton->OnDraw();
	}
}

//-----------------------------------------------------------------------

void cNumericalPanel::Update(float afTimeStep) {
	////////////////////////////////
	// Check active and fade
	if (mbActive == false) {
		mfAlpha -= 2.5f * afTimeStep;
		if (mfAlpha < 0)
			mfAlpha = 0;
		return;
	} else {
		// Bad hack fix... owell
		mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Pointer);

		mfAlpha += 2.3f * afTimeStep;
		if (mfAlpha > 1)
			mfAlpha = 1;
	}

	////////////////////////////////
	// Update buttons
	tNumericalButtonListIt it = mlstButtons.begin();
	for (; it != mlstButtons.end(); ++it) {
		cNumericalButton *pButton = *it;

		pButton->OnUpdate(afTimeStep);

		if (cMath::PointBoxCollision(mvMousePos, pButton->GetRect())) {
			pButton->OnMouseOver(true);
		} else {
			pButton->OnMouseOver(false);
		}
	}
}

//-----------------------------------------------------------------------

void cNumericalPanel::SetMousePos(const cVector2f &avPos) {
	mvMousePos = avPos;
	mpInit->mpPlayer->SetCrossHairPos(mvMousePos);
}

void cNumericalPanel::AddMousePos(const cVector2f &avRel) {
	mvMousePos += avRel;

	if (mvMousePos.x < 0)
		mvMousePos.x = 0;
	if (mvMousePos.x >= 800)
		mvMousePos.x = 800;
	if (mvMousePos.y < 0)
		mvMousePos.y = 0;
	if (mvMousePos.y >= 600)
		mvMousePos.y = 600;

	mpInit->mpPlayer->SetCrossHairPos(mvMousePos);
}

//-----------------------------------------------------------------------

void cNumericalPanel::OnMouseDown(eMButton aButton) {
	////////////////////////////////
	// Update buttons
	tNumericalButtonListIt it = mlstButtons.begin();
	for (; it != mlstButtons.end(); ++it) {
		cNumericalButton *pButton = *it;

		if (cMath::PointBoxCollision(mvMousePos, pButton->GetRect())) {
			pButton->OnMouseDown();
		}
	}

	mbMouseIsDown = true;
}

void cNumericalPanel::OnMouseUp(eMButton aButton) {
	////////////////////////////////
	// Update buttons
	tNumericalButtonListIt it = mlstButtons.begin();
	for (; it != mlstButtons.end(); ++it) {
		cNumericalButton *pButton = *it;

		if (cMath::PointBoxCollision(mvMousePos, pButton->GetRect())) {
			pButton->OnMouseUp();
		}
	}

	mbMouseIsDown = false;
}

//-----------------------------------------------------------------------

void cNumericalPanel::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;

	if (mbActive) {
		if (mpInit->mbHasHaptics)
			mpInit->mpPlayer->GetHapticCamera()->SetActive(false);

		mpInit->mpGame->GetInput()->BecameTriggerd("RightClick");
		mpInit->mpGame->GetInput()->BecameTriggerd("LeftClick");

		mLastCrossHairState = mpInit->mpPlayer->GetCrossHairState();

		mpInit->mpPlayer->SetCrossHairPos(mvMousePos);
		mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Pointer);

		mvDigits.clear();
	} else {
		if (mpInit->mbHasHaptics)
			mpInit->mpPlayer->GetHapticCamera()->SetActive(true);

		mpInit->mpPlayer->SetCrossHairState(mLastCrossHairState);
		mpInit->mpPlayer->SetCrossHairPos(cVector2f(400, 300));
	}
}

//-----------------------------------------------------------------------

void cNumericalPanel::OnExit() {
	SetActive(false);
}

//-----------------------------------------------------------------------

void cNumericalPanel::AddDigit(int alNum) {
	mvDigits.push_back(alNum);

	if (mvDigits.size() == 4) {
		bool bCorrect = true;
		for (int i = 0; i < 4; ++i) {
			if (mvCode[i] != mvDigits[i]) {
				bCorrect = false;
				break;
			}
		}

		tString sCorrect = bCorrect ? "true" : "false";

		tString sCommand = msCallback + "(\"" + msName + "\", " + sCorrect + ")";
		mpInit->RunScriptCommand(sCommand);

		SetActive(false);
	}
}

//-----------------------------------------------------------------------

void cNumericalPanel::SetUp(const tString &asName, const tString &asCallback) {
	msName = asName;
	msCallback = asCallback;
}

void cNumericalPanel::SetCode(tIntVec &avCode) {
	mvCode = avCode;
}

//-----------------------------------------------------------------------
