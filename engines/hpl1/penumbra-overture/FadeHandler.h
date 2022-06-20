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

#ifndef GAME_FADE_HANDLER_H
#define GAME_FADE_HANDLER_H

#include "hpl1/engine/engine.h"

using namespace hpl;

class cInit;

class cFadeHandler : public iUpdateable {
public:
	cFadeHandler(cInit *apInit);
	~cFadeHandler();

	void FadeOut(float afTime);
	void FadeIn(float afTime);
	bool IsActive();

	void SetWideScreenActive(bool abX);

	void OnStart();
	void Update(float afTimeStep);
	void Reset();
	void OnDraw();

private:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	cGfxObject *mpBlackGfx;

	bool mbActive;
	float mfAlpha;
	float mfAlphaAdd;

	bool mbWideScreenActive;
	float mfWideScreenAlpha;
};

#endif // GAME_FADE_HANDLER_H
