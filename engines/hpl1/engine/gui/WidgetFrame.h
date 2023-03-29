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
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_WIDGET_FRAME_H
#define HPL_WIDGET_FRAME_H

#include "hpl1/engine/gui/Widget.h"

namespace hpl {

class cGuiSkinFont;

class cWidgetFrame : public iWidget {
public:
	cWidgetFrame(cGuiSet *apSet, cGuiSkin *apSkin);
	virtual ~cWidgetFrame();

	void SetDrawFrame(bool abX) { mbDrawFrame = abX; }
	bool GetDrawFrame() { return mbDrawFrame; }

	void SetDrawBackground(bool abX) { mbDrawBackground = abX; }
	bool GetDrawBackground() { return mbDrawBackground; }

	void SetBackgroundZ(float afZ) { mfBackgroundZ = afZ; }
	float GetBackgroundZ() { return mfBackgroundZ; }

	void SetBackGroundColor(const cColor &aColor) { mBackGroundColor = aColor; }
	const cColor &GetBackGroundColor() { return mBackGroundColor; }

protected:
	/////////////////////////
	// Implemented functions
	void OnLoadGraphics();

	void OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion);
	void OnDrawAfterClip(float afTimeStep, cGuiClipRegion *apClipRegion);

	bool OnMouseMove(cGuiMessageData &aData);
	bool OnMouseDown(cGuiMessageData &aData);
	bool OnMouseUp(cGuiMessageData &aData);
	bool OnMouseEnter(cGuiMessageData &aData);
	bool OnMouseLeave(cGuiMessageData &aData);

	/////////////////////////
	// Data
	bool mbDrawFrame;
	bool mbDrawBackground;
	float mfBackgroundZ;
	cColor mBackGroundColor;

	cGuiGfxElement *mpGfxBackground;

	cGuiGfxElement *mvGfxBorders[4];
	cGuiGfxElement *mvGfxCorners[4];
};

} // namespace hpl

#endif // HPL_WIDGET_FRAME_H
