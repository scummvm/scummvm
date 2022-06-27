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

#include "hpl1/penumbra-overture/GameMessageHandler.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/Notebook.h"
#include "hpl1/penumbra-overture/NumericalPanel.h"
#include "hpl1/penumbra-overture/Player.h"

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameMessageHandler::cGameMessageHandler(cInit *apInit) : iUpdateable("GameMessageHandler") {
	mpInit = apInit;

	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");

	msOverCallback = "";

	mbFocusIsedUsed = false;

	mbBlackText = false;
}

//-----------------------------------------------------------------------

cGameMessageHandler::~cGameMessageHandler(void) {
	STLDeleteAll(mlstMessages);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// GAME MESSAGE
//////////////////////////////////////////////////////////////////////////

cGameMessage::cGameMessage(const tWString &asText, cGameMessageHandler *apMessHandler) {
	mpMessHandler = apMessHandler;

	msText = asText;
	mfFade = 0;
	mbActive = false;
	mfFadeAdd = 1.3f;
}

//-----------------------------------------------------------------------

void cGameMessage::Update(float afTimeStep) {
	if (mbActive) {
		mfFade += mfFadeAdd * afTimeStep;

		if (mfFadeAdd < 0) {
			if (mfFade <= 0) {
				mfFade = 0;
				mbActive = 0;
			}
		} else {
			if (mfFade > 1) {
				mfFade = 1;
			}
		}
	}
}

//-----------------------------------------------------------------------

void cGameMessage::Draw(FontData *apFont) {
	if (mbActive == false)
		return;

	float fSide = 30;
	if (mpMessHandler->mbBlackText) {
		apFont->drawWordWrap(cVector3f(fSide, 300, 152), 800 - fSide * 2, 16, 14, cColor(0, mfFade),
							 eFontAlign_Left, msText);
	} else {
		apFont->drawWordWrap(cVector3f(fSide, 300, 152), 800 - fSide * 2, 16, 14, cColor(1, 1, 1, mfFade),
							 eFontAlign_Left, msText);
		apFont->drawWordWrap(cVector3f(fSide, 300, 151) + cVector3f(1, 1, -1), 800 - fSide * 2, 16, 14, cColor(0, 0, 0, mfFade),
							 eFontAlign_Left, msText);
		apFont->drawWordWrap(cVector3f(fSide, 300, 151) + cVector3f(-1, -1, -1), 800 - fSide * 2, 16, 14, cColor(0, 0, 0, mfFade),
							 eFontAlign_Left, msText);
		apFont->drawWordWrap(cVector3f(fSide, 300, 151) + cVector3f(1, -1, -1), 800 - fSide * 2, 16, 14, cColor(0, 0, 0, mfFade),
							 eFontAlign_Left, msText);
		apFont->drawWordWrap(cVector3f(fSide, 300, 151) + cVector3f(-1, 1, -1), 800 - fSide * 2, 16, 14, cColor(0, 0, 0, mfFade),
							 eFontAlign_Left, msText);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameMessageHandler::Add(const tWString &asText) {
	if (mpInit->mpPlayer->GetHealth() <= 0)
		return;

	cGameMessage *pMess = hplNew(cGameMessage, (asText, this));
	mlstMessages.push_back(pMess);

	if (mpInit->mpPlayer->GetState() != ePlayerState_Message) {
		if (mpInit->mpInventory->IsActive())
			mpInit->mpInventory->SetActive(false);
		if (mpInit->mpNotebook->IsActive())
			mpInit->mpNotebook->SetActive(false);
		if (mpInit->mpNumericalPanel->IsActive())
			mpInit->mpNumericalPanel->SetActive(false);

		mLastState = mpInit->mpPlayer->GetState();
		mpInit->mpPlayer->ChangeState(ePlayerState_Message);

		pMess->mbActive = true;
	}
}

//-----------------------------------------------------------------------

void cGameMessageHandler::ShowNext() {
	bool bFoundMess = false;
	cGameMessage *pPrevMess = NULL;

	// wait till the message is fully visible.
	if (mlstMessages.empty() == false) {
		cGameMessage *pMess = mlstMessages.front();
		if (pMess->mfFade < 0.2f && pMess->mfFadeAdd > 0)
			return;
	}

	tGameMessageListIt it = mlstMessages.begin();
	for (; it != mlstMessages.end(); ++it) {
		cGameMessage *pMess = *it;
		if (pMess->mbActive == false) {
			pMess->mbActive = true;
			bFoundMess = true;
			if (pPrevMess)
				pPrevMess->mfFadeAdd = -pPrevMess->mfFadeAdd;
			break;
		}
		pPrevMess = pMess;
	}

	// Check if this is the last message.
	if (bFoundMess == false) {
		if (pPrevMess)
			pPrevMess->mfFadeAdd = -pPrevMess->mfFadeAdd;

		if (mLastState != ePlayerState_Grab &&
			mLastState != ePlayerState_Push &&
			mLastState != ePlayerState_Move &&
			mLastState != ePlayerState_UseItem) {
			mpInit->mpPlayer->ChangeState(mLastState);
		} else {
			mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
		}

		if (mbFocusIsedUsed) {
			mbFocusIsedUsed = false;
			mpInit->mpEffectHandler->GetDepthOfField()->SetActive(false, 2.0f);
		}

		// Check if there is a script callback
		if (msOverCallback != "") {
			tString sCommand = msOverCallback + "()";
			msOverCallback = "";

			mpInit->RunScriptCommand(sCommand);
		} else {
		}
	}
}

//-----------------------------------------------------------------------

void cGameMessageHandler::Update(float afTimeStep) {
	if (mpInit->mpPlayer->IsDead()) {
		STLDeleteAll(mlstMessages);
		mlstMessages.clear();
		return;
	}

	int lCount = 0;
	tGameMessageListIt it = mlstMessages.begin();
	for (; it != mlstMessages.end(); ++lCount) {
		cGameMessage *pMess = *it;

		pMess->Update(afTimeStep);

		if (lCount == 0 && pMess->mbActive == false) {
			hplDelete(pMess);
			it = mlstMessages.erase(it);
		} else {
			++it;
		}
	}
}

//-----------------------------------------------------------------------

void cGameMessageHandler::OnWorldLoad() {
}

//-----------------------------------------------------------------------

void cGameMessageHandler::OnWorldExit() {
	Reset();
	mpInit->mpEffectHandler->GetDepthOfField()->SetActive(false, 1.0f);
}

//-----------------------------------------------------------------------

void cGameMessageHandler::OnDraw() {
	tGameMessageListIt it = mlstMessages.begin();
	for (; it != mlstMessages.end(); ++it) {
		cGameMessage *pMess = *it;
		pMess->Draw(mpFont);
	}
}

//-----------------------------------------------------------------------

void cGameMessageHandler::SetOnMessagesOverCallback(const tString &asFunction) {
	msOverCallback = asFunction;
}

//-----------------------------------------------------------------------

void cGameMessageHandler::Reset() {
	STLDeleteAll(mlstMessages);
	mlstMessages.clear();

	mbFocusIsedUsed = false;
}

//-----------------------------------------------------------------------
