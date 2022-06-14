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

#include "hpl1/engine/gui/WidgetCheckBox.h"

#include "hpl1/engine/system/LowLevelSystem.h"

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

cWidgetCheckBox::cWidgetCheckBox(cGuiSet *apSet, cGuiSkin *apSkin) : iWidget(eWidgetType_CheckBox, apSet, apSkin) {
	mbChecked = false;
	mbPressed = false;

	LoadGraphics();
}

//-----------------------------------------------------------------------

cWidgetCheckBox::~cWidgetCheckBox() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWidgetCheckBox::SetChecked(bool abX) {
	if (mbChecked == abX)
		return;

	mbChecked = abX;

	cGuiMessageData data = cGuiMessageData(mbChecked);
	ProcessMessage(eGuiMessage_CheckChange, data);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWidgetCheckBox::OnLoadGraphics() {
	mvGfxBox[1][0] = mpSkin->GetGfx(eGuiSkinGfx_CheckBoxEnabledUnchecked);
	mvGfxBox[1][1] = mpSkin->GetGfx(eGuiSkinGfx_CheckBoxEnabledChecked);
	mvGfxBox[0][0] = mpSkin->GetGfx(eGuiSkinGfx_CheckBoxDisabledUnchecked);
	mvGfxBox[0][1] = mpSkin->GetGfx(eGuiSkinGfx_CheckBoxDisabledChecked);
}

//-----------------------------------------------------------------------

void cWidgetCheckBox::OnChangeSize() {
	if (mvSize.y < mvDefaultFontSize.y)
		mvSize.y = mvDefaultFontSize.y;
	if (mvSize.x < mvGfxBox[0][0]->GetActiveSize().x)
		mvSize.x = mvGfxBox[0][0]->GetActiveSize().x;
}

//-----------------------------------------------------------------------

void cWidgetCheckBox::OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion) {
	////////////////////////////////
	// Box
	mpSet->DrawGfx(mvGfxBox[IsEnabled()][mbChecked], GetGlobalPosition() +
														 cVector3f(0, mvSize.y / 2 - mvGfxBox[0][0]->GetActiveSize().y / 2, 0));

	////////////////////////////////
	// Text
	eGuiSkinFont font = IsEnabled() ? eGuiSkinFont_Default : eGuiSkinFont_Disabled;
	DrawSkinText(msText, font, GetGlobalPosition() + cVector3f(mvGfxBox[0][0]->GetActiveSize().x + 3.0f, mvSize.y / 2 - mvDefaultFontSize.y / 2, 0),
				 eFontAlign_Left);
}

//-----------------------------------------------------------------------

bool cWidgetCheckBox::OnMouseMove(cGuiMessageData &aData) {
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetCheckBox::OnMouseDown(cGuiMessageData &aData) {
	mbPressed = true;
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetCheckBox::OnMouseUp(cGuiMessageData &aData) {
	if (mbPressed)
		SetChecked(!mbChecked);

	mbPressed = false;
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetCheckBox::OnMouseEnter(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetCheckBox::OnMouseLeave(cGuiMessageData &aData) {
	mbPressed = false;
	return false;
}

//-----------------------------------------------------------------------

} // namespace hpl
