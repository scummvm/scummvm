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

#ifndef GAME_MAP_LOAD_TEXT_H
#define GAME_MAP_LOAD_TEXT_H

#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameTypes.h"

using namespace hpl;

class cInit;

//-------------------------------------------------

class cMapLoadText : public iUpdateable {
public:
	cMapLoadText(cInit *apInit);
	~cMapLoadText();

	void Reset();

	void OnPostSceneDraw();

	void OnDraw();

	void Update(float afTimeStep);

	void OnMouseDown(eMButton aButton);
	void OnMouseUp(eMButton aButton);

	void SetText(tString asCat, tString asEntry) {
		msTextCat = asCat;
		msTextEntry = asEntry;
	}

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

	void OnExit();

private:
	void DrawText(bool abLoading);

	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	FontData *mpFont;
	FontData *mpTextFont;

	tWStringVec mvRows;

	iTexture *mpBack;

	tString msTextCat;
	tString msTextEntry;

	float mfAlpha;
	float mfAlphaAdd;

	bool mbActive;
};

//---------------------------------------------

#endif // GAME_MAP_LOAD_TEXT_H
