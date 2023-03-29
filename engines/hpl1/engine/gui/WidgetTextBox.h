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

#ifndef HPL_WIDGET_TEXT_BOX_H
#define HPL_WIDGET_TEXT_BOX_H

#include "hpl1/engine/gui/Widget.h"

namespace hpl {

class cGuiSkinFont;

class cWidgetTextBox : public iWidget {
public:
	cWidgetTextBox(cGuiSet *apSet, cGuiSkin *apSkin);
	virtual ~cWidgetTextBox();

	void SetMaxTextLength(int alLength);
	int GetMaxTextLength() { return mlMaxCharacters; }

	cVector2f GetBackgroundSize();

	void SetMaxTextSizeNeg(float afX);

	void SetCanEdit(bool abX);
	bool GetCanEdit() { return mbCanEdit; }

protected:
	/////////////////////////
	// Own functions
	int WorldToCharPos(const cVector2f &avWorldPos);
	float CharToLocalPos(int alChar);
	void SetMarkerPos(int alPos);
	int GetLastCharInSize(int alStartPos, float afMaxSize, float afLengthAdd);
	int GetFirstCharInSize(int alStartPos, float afMaxSize, float afLengthAdd);

	/////////////////////////
	// Implemented functions
	void OnLoadGraphics();
	void OnChangeSize();
	void OnChangeText();

	void OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion);

	bool OnMouseMove(cGuiMessageData &aData);
	bool OnMouseDown(cGuiMessageData &aData);
	bool OnMouseDoubleClick(cGuiMessageData &aData);
	bool OnMouseUp(cGuiMessageData &aData);
	bool OnMouseEnter(cGuiMessageData &aData);
	bool OnMouseLeave(cGuiMessageData &aData);

	bool OnLostFocus(cGuiMessageData &aData);

	bool OnKeyPress(cGuiMessageData &aData);

	/////////////////////////
	// Data
	bool mbPressed;
	int mlMarkerCharPos;
	int mlSelectedTextEnd;
	int mlFirstVisibleChar;
	int mlVisibleCharSize;
	float mfTextMaxSize;

	int mlMaxCharacters;

	float mfMaxTextSizeNeg;

	bool mbCanEdit;

	cGuiGfxElement *mpGfxMarker;
	cGuiGfxElement *mpGfxSelectedTextBack;

	cGuiGfxElement *mpGfxBackground;

	cGuiGfxElement *mvGfxBorders[4];
	cGuiGfxElement *mvGfxCorners[4];
};

} // namespace hpl

#endif // HPL_WIDGET_TEXT_BOX_H
