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

#include "hpl1/engine/gui/WidgetLabel.h"

#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/graphics/font_data.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/gui/Gui.h"
#include "hpl1/engine/gui/GuiGfxElement.h"
#include "hpl1/engine/gui/GuiSet.h"
#include "hpl1/engine/gui/GuiSkin.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cWidgetLabel::cWidgetLabel(cGuiSet *apSet, cGuiSkin *apSkin) : iWidget(eWidgetType_Label, apSet, apSkin) {
	mbWordWrap = false;
	mTextAlign = eFontAlign_Left;

	mlMaxCharacters = -1;
}

//-----------------------------------------------------------------------

cWidgetLabel::~cWidgetLabel() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

void cWidgetLabel::SetMaxTextLength(int alLength) {
	if (mlMaxCharacters == alLength)
		return;

	mlMaxCharacters = alLength;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWidgetLabel::OnLoadGraphics() {
}

//-----------------------------------------------------------------------

void cWidgetLabel::OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion) {
	////////////////////////////////
	// Texts
	cVector3f vOffset = 0;
	// if(mTextAlign == eFontAlign_Center)		vOffset.x += mvSize.x/2;
	// else
	if (mTextAlign == eFontAlign_Right)
		vOffset.x += mvSize.x;

	if (mbWordWrap) {
		int lChars = 0;
		bool bEnabled = IsEnabled();
		float fHeight = mvDefaultFontSize.y + 2;
		tWStringVec vRows;
		mpDefaultFontType->getWordWrapRows(mvSize.x, fHeight,
										   mvDefaultFontSize, msText,
										   &vRows);

		for (size_t i = 0; i < vRows.size(); ++i) {
			bool bBreak = false;
			if (mlMaxCharacters >= 0) {
				if (lChars + (int)vRows[i].size() > mlMaxCharacters) {
					vRows[i] = cString::SubW(vRows[i], 0, mlMaxCharacters - lChars);
					bBreak = true;
				}
				lChars += (int)vRows[i].size();
			}

			if (bEnabled)
				DrawDefaultText(vRows[i], GetGlobalPosition() + vOffset, mTextAlign);
			else
				DrawSkinText(vRows[i], eGuiSkinFont_Disabled, GetGlobalPosition() + vOffset, mTextAlign);
			vOffset.y += fHeight;

			if (bBreak)
				break;
		}
	} else {
		if (mlMaxCharacters >= 0 && (int)msText.size() > mlMaxCharacters) {
			if (IsEnabled())
				DrawDefaultText(cString::SubW(msText, 0, mlMaxCharacters), GetGlobalPosition() + vOffset, mTextAlign);
			else
				DrawSkinText(cString::SubW(msText, 0, mlMaxCharacters), eGuiSkinFont_Disabled, GetGlobalPosition() + vOffset, mTextAlign);
		} else {
			if (IsEnabled())
				DrawDefaultText(msText, GetGlobalPosition() + vOffset, mTextAlign);
			else
				DrawSkinText(msText, eGuiSkinFont_Disabled, GetGlobalPosition() + vOffset, mTextAlign);
		}
	}
}

//-----------------------------------------------------------------------

bool cWidgetLabel::OnMouseMove(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetLabel::OnMouseDown(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetLabel::OnMouseUp(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetLabel::OnMouseEnter(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetLabel::OnMouseLeave(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

} // namespace hpl
