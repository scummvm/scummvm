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

#include "hpl1/penumbra-overture/RadioHandler.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Player.h"

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cRadioHandler::cRadioHandler(cInit *apInit) : iUpdateable("RadioHandler") {
	mpInit = apInit;
	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");

	mpSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

	Reset();
}

//-----------------------------------------------------------------------

cRadioHandler::~cRadioHandler(void) {
	STLDeleteAll(mlstMessages);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// GAME MESSAGE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cRadioMessage::cRadioMessage(const tWString &asText, const tString &asSound) {
	msText = asText;
	msSound = asSound;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cRadioHandler::Add(const tWString &asText, const tString &asSound) {
	if (mlstMessages.empty())
		msPrevText = _W("");

	cRadioMessage *pMess = hplNew(cRadioMessage, (asText, asSound));
	mlstMessages.push_back(pMess);
}

//-----------------------------------------------------------------------

void cRadioHandler::Update(float afTimeStep) {
	if (mpInit->mpPlayer->IsDead()) {
		STLDeleteAll(mlstMessages);
		mlstMessages.clear();
		return;
	}

	/////////////////////////////
	// Update current message
	if (mpCurrentMessage) {
		if (mpSoundHandler->IsValid(mpCurrentMessage->mpChannel) == false) {
			msCurrentText = _W("");
			msPrevText = mpCurrentMessage->msText;
			hplDelete(mpCurrentMessage);
			mpCurrentMessage = NULL;

			if (mlstMessages.empty() && msOnEndCallback != "") {
				mpInit->RunScriptCommand(msOnEndCallback + "()");
				msOnEndCallback = "";
			}
		}
	}

	////////////////////////////
	// Get next message
	if (mpCurrentMessage == NULL && mlstMessages.empty() == false) {
		// Get newer message
		mpCurrentMessage = mlstMessages.front();
		mlstMessages.pop_front();

		msCurrentText = mpCurrentMessage->msText;

		mfAlpha = 0;

		// Start newer message
		mpCurrentMessage->mpChannel = mpSoundHandler->PlayStream(mpCurrentMessage->msSound, false, 1);
	}

	////////////////////////
	// Alpha
	if (mfAlpha < 1) {
		mfAlpha += afTimeStep * 2;
		if (mfAlpha > 1) {
			mfAlpha = 1;
			msPrevText = msCurrentText;
		}
	}
}

//-----------------------------------------------------------------------

bool cRadioHandler::IsActive() {
	return mpCurrentMessage != NULL;
}

//-----------------------------------------------------------------------

void cRadioHandler::OnDraw() {
	float fAlpha = mfAlpha;

	if (mpInit->mbSubtitles) {
		if (msCurrentText != _W("")) {
			mpFont->DrawWordWrap(cVector3f(25, 500, 47), 750, 16, 15, cColor(1, fAlpha),
								 eFontAlign_Left, msCurrentText);
			mpFont->DrawWordWrap(cVector3f(25, 500, 46) + cVector3f(2, 2, 0), 750, 16, 15, cColor(0, fAlpha),
								 eFontAlign_Left, msCurrentText);
		}

		if (msPrevText != _W("") && msPrevText != msCurrentText && mfAlpha < 1) {
			mpFont->DrawWordWrap(cVector3f(25, 500, 47), 750, 16, 15, cColor(1, 1 - fAlpha),
								 eFontAlign_Left, msPrevText);
			mpFont->DrawWordWrap(cVector3f(25, 500, 46) + cVector3f(2, 2, 0), 750, 16, 15, cColor(0, 1 - fAlpha),
								 eFontAlign_Left, msPrevText);
		}
	}
}

//-----------------------------------------------------------------------

void cRadioHandler::Reset() {
	STLDeleteAll(mlstMessages);
	mlstMessages.clear();

	msCurrentText = _W("");
	msPrevText = _W("");

	mfAlpha = 0;

	mpCurrentMessage = NULL;

	msOnEndCallback = "";
}

//-----------------------------------------------------------------------
