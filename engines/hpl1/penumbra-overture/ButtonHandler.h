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

#ifndef GAME_BUTTON_HANDLER_H
#define GAME_BUTTON_HANDLER_H

#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameTypes.h"

using namespace hpl;

class cInit;
class cPlayer;

class cButtonHandler : public iUpdateable {
public:
	cButtonHandler(cInit *apInit);
	~cButtonHandler();

	void OnStart();
	void Update(float afTimeStep);
	void Reset();
	void OnExit();
	void OnPostSceneDraw();

	void SetDefaultKeys();

	tString GetActionName(const tString &asInputName, const tString &asSkipAction);

	void ChangeState(eButtonHandlerState aState);
	eButtonHandlerState GetState() { return mState; }

	bool GetInvertMouseY() { return mbInvertMouseY; }
	bool GetToggleCrouch() { return mbToggleCrouch; }

	// Settings
	float mfMouseSensitivity;
	bool mbInvertMouseY;
	bool mbToggleCrouch;

private:
	iAction *ActionFromTypeAndVal(const tString &asName, const tString &asType, int input);
	void TypeAndValFromAction(iAction *apAction, tString *apType, tString *apVal);

	int mlNumOfActions;

	cInit *mpInit;

	cInput *mpInput;
	iLowLevelGraphics *mpLowLevelGraphics;

	cPlayer *mpPlayer;

	eButtonHandlerState mState;
};

#endif // GAME_BUTTON_HANDLER_H
