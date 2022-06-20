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

#ifndef GAME_NUMERICAL_PANEL_H
#define GAME_NUMERICAL_PANEL_H

#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameTypes.h"

using namespace hpl;

class cInit;

//---------------------------
class cNumericalPanel;

class cNumericalButton {
public:
	cNumericalButton(cInit *apInit, cNumericalPanel *apPanel, cVector2f avPos, cVector2f avSize, int alNum);
	~cNumericalButton();

	void OnUpdate(float afTimeStep);

	void OnDraw();

	void OnMouseDown();
	void OnMouseUp();
	void OnMouseOver(bool abOver);

	const cRect2f &GetRect() { return mRect; }

private:
	cGfxObject *mpGfxUp;
	cGfxObject *mpGfxDown;

	float mfAlpha;
	bool mbOver;

	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;
	cNumericalPanel *mpPanel;

	cVector3f mvPositon;
	cRect2f mRect;

	int mlNum;
};

typedef std::list<cNumericalButton *> tNumericalButtonList;
typedef tNumericalButtonList::iterator tNumericalButtonListIt;

//---------------------------------------------

class cNumericalPanel : public iUpdateable {
	friend class cNumericalButton;

public:
	cNumericalPanel(cInit *apInit);
	~cNumericalPanel();

	void Reset();

	void OnDraw();

	void Update(float afTimeStep);

	void SetMousePos(const cVector2f &avPos);
	void AddMousePos(const cVector2f &avRel);
	cVector2f GetMousePos() { return mvMousePos; }

	void OnMouseDown(eMButton aButton);
	void OnMouseUp(eMButton aButton);

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

	void OnExit();

	void AddDigit(int alNum);

	void SetUp(const tString &asName, const tString &asCallback);

	void SetCode(tIntVec &avCode);

private:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	cGfxObject *mpGfxBackground;
	cGfxObject *mpGfxPanel;

	bool mbMouseIsDown;

	bool mbActive;
	float mfAlpha;

	cVector2f mvMousePos;

	eCrossHairState mLastCrossHairState;

	tNumericalButtonList mlstButtons;

	tString msName;
	tString msCallback;

	tIntVec mvDigits;
	tIntVec mvCode;
};

//---------------------------------------------

#endif // GAME_NUMERICAL_PANEL_H
