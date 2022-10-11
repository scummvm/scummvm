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

#include "hpl1/penumbra-overture/Credits.h"

#include "hpl1/penumbra-overture/ButtonHandler.h"
#include "hpl1/penumbra-overture/GameMessageHandler.h"
#include "hpl1/penumbra-overture/GameMusicHandler.h"
#include "hpl1/penumbra-overture/GraphicsHelper.h"
#include "hpl1/penumbra-overture/HapticGameCamera.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MainMenu.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHands.h"

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cCredits::cCredits(cInit *apInit) : iUpdateable("Credits") {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	// Load fonts
	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");

	// Load config file

	// Load text
	tWString sText = kTranslate("MainMenu", "CreditsText");
	mpFont->getWordWrapRows(750, 19, 17, sText, &mvTextRows);

	Reset();
}

//-----------------------------------------------------------------------

cCredits::~cCredits(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cCredits::Reset() {
	mbActive = false;

	mfYPos = 600;
}

//-----------------------------------------------------------------------

void cCredits::OnPostSceneDraw() {
	mpInit->mpGraphicsHelper->ClearScreen(cColor(0, 0));
}

//-----------------------------------------------------------------------

void cCredits::OnDraw() {
	float fSize[2] = {17, 19};
	float fY = mfYPos;
	for (size_t i = 0; i < mvTextRows.size(); ++i) {
		int lSize = 0;
		if (mvTextRows[i][0] == '*') {
			lSize = 1;
		}
		if (mvTextRows[i].size() <= 1) {
			fY += fSize[lSize];
		}

		if (fY >= -fSize[lSize]) {
			if (fY > 600)
				continue;

			float fAlpha = fY / 300;
			if (fAlpha > 1)
				fAlpha = (2 - fAlpha);

			if (lSize == 0)
				mpFont->draw(cVector3f(400, fY, 10), fSize[lSize], cColor(1, fAlpha), eFontAlign_Center,
							 mvTextRows[i]);
			else
				mpFont->draw(cVector3f(400, fY, 10), fSize[lSize], cColor(0.8f, fAlpha), eFontAlign_Center,
							 mvTextRows[i].substr(1));
		}
		fY += fSize[lSize];
	}
}

//-----------------------------------------------------------------------

void cCredits::Update(float afTimeStep) {
	// Make sure haptics is off
	if (mpInit->mbHasHaptics) {
		mpInit->mpGame->GetHaptic()->GetLowLevel()->StopAllForces();
	}

	mfYPos -= afTimeStep * 30;
	cMusicHandler *pMusicHandler = mpInit->mpGame->GetSound()->GetMusicHandler();

	// Check if the credits are over.
	float fSize[2] = {17, 19};
	float fY = mfYPos;
	for (size_t i = 0; i < mvTextRows.size(); ++i) {
		int lSize = 0;
		if (mvTextRows[i][0] == '*')
			lSize = 1;
		if (mvTextRows[i].size() <= 1)
			fY += fSize[lSize];
		fY += fSize[lSize];
	}
	if (fY < -120 && pMusicHandler->GetCurrentSong() != NULL) {
		SetActive(false);
		pMusicHandler->Stop(0.1f);
	}
	if (fY < -340) {
		SetActive(false);
	}
}

//-----------------------------------------------------------------------

void cCredits::OnMouseDown(eMButton aButton) {
	OnButtonDown();
}

//-----------------------------------------------------------------------

void cCredits::OnButtonDown() {
	SetActive(false);
}

//-----------------------------------------------------------------------

void cCredits::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;

	if (mbActive) {
		mpInit->mpGame->GetScene()->GetWorld3D()->DestroyAllSoundEntities();

		mpInit->mpGame->GetUpdater()->SetContainer("Credits");
		mpInit->mpGame->GetScene()->SetDrawScene(false);
		mpInit->mpGame->GetScene()->SetUpdateMap(false);
		if (mpInit->mbHasHaptics) {
			mpInit->mpGame->GetHaptic()->GetLowLevel()->SetUpdateShapes(false);
			mpInit->mpGame->GetHaptic()->GetLowLevel()->StopAllForces();
			mpInit->mpPlayer->GetHapticCamera()->SetActive(false);
		}
		mpInit->mpButtonHandler->ChangeState(eButtonHandlerState_Credits);

		mpInit->mpGame->GetSound()->GetMusicHandler()->Play("penumbra_music_E1_E", 1, 0.3f, false);

		Log("Set Active!\n");
	} else {
		mpInit->mpGame->GetSound()->GetMusicHandler()->Stop(0.5f);

		mpInit->ResetGame(true);
		mpInit->mpMainMenu->SetActive(true);
	}
}

//-----------------------------------------------------------------------

void cCredits::OnExit() {
	SetActive(false);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
