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

#include "hpl1/engine/gui/WidgetWindow.h"

#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/gui/Gui.h"
#include "hpl1/engine/gui/GuiGfxElement.h"
#include "hpl1/engine/gui/GuiSet.h"
#include "hpl1/engine/gui/GuiSkin.h"

#include "hpl1/engine/resources/FontManager.h"
#include "hpl1/engine/resources/Resources.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cWidgetWindow::cWidgetWindow(cGuiSet *apSet, cGuiSkin *apSkin) : iWidget(eWidgetType_Window, apSet, apSkin) {
	mvRelMousePos = 0;
	mbMoving = false;
	mbStatic = false;
}

//-----------------------------------------------------------------------

cWidgetWindow::~cWidgetWindow() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWidgetWindow::SetStatic(bool abX) {
	mbStatic = abX;

	if (mbStatic == false) {
		mbMoving = false;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWidgetWindow::OnLoadGraphics() {
	mpGfxBackground = mpSkin->GetGfx(eGuiSkinGfx_WindowBackground);

	mpGfxLabel = mpSkin->GetGfx(eGuiSkinGfx_WindowLabel);

	mvGfxBorders[0] = mpSkin->GetGfx(eGuiSkinGfx_WindowBorderRight);
	mvGfxBorders[1] = mpSkin->GetGfx(eGuiSkinGfx_WindowBorderLeft);
	mvGfxBorders[2] = mpSkin->GetGfx(eGuiSkinGfx_WindowBorderUp);
	mvGfxBorders[3] = mpSkin->GetGfx(eGuiSkinGfx_WindowBorderDown);

	mvGfxCorners[0] = mpSkin->GetGfx(eGuiSkinGfx_WindowCornerLU);
	mvGfxCorners[1] = mpSkin->GetGfx(eGuiSkinGfx_WindowCornerRU);
	mvGfxCorners[2] = mpSkin->GetGfx(eGuiSkinGfx_WindowCornerRD);
	mvGfxCorners[3] = mpSkin->GetGfx(eGuiSkinGfx_WindowCornerLD);

	mpLabelFont = mpSkin->GetFont(eGuiSkinFont_WindowLabel);

	mvLabelTextOffset = mpSkin->GetAttribute(eGuiSkinAttribute_WindowLabelTextOffset);
}

//-----------------------------------------------------------------------

void cWidgetWindow::OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion) {
	////////////////////////////////
	// Calc label size
	cVector2f vLabelSize;
	vLabelSize.x = mvSize.x - (mvGfxCorners[0]->GetActiveSize().x + mvGfxCorners[1]->GetActiveSize().x);
	vLabelSize.y = mpLabelFont->mvSize.y + mvLabelTextOffset.y * 2;

	////////////////////////////////
	// Label
	mpSet->DrawGfx(mpGfxLabel, GetGlobalPosition() + cVector3f(mvGfxCorners[0]->GetActiveSize().x, mvGfxCorners[0]->GetActiveSize().y, 0.2f),
				   vLabelSize);

	////////////////////////////////
	// Label text
	DrawSkinText(msText, eGuiSkinFont_WindowLabel, GetGlobalPosition() + cVector3f(mvGfxCorners[0]->GetActiveSize().x + mvLabelTextOffset.x, mvGfxCorners[0]->GetActiveSize().y + mvLabelTextOffset.y, 0.4f));

	////////////////////////////////
	// Borders and background
	DrawBordersAndCorners(mpGfxBackground, mvGfxBorders, mvGfxCorners, GetGlobalPosition(), mvSize);
}

//-----------------------------------------------------------------------

bool cWidgetWindow::OnMouseMove(cGuiMessageData &aData) {
	if (mbMoving)
		SetGlobalPosition(mvRelMousePos + cVector3f(aData.mvPos.x, aData.mvPos.y, 0));
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetWindow::OnMouseDown(cGuiMessageData &aData) {
	if (mbStatic)
		return false;

	////////////////////////////////
	// Calculate label rectangle
	cRect2f labelRect;
	labelRect.w = mvSize.x - (mvGfxCorners[0]->GetActiveSize().x + mvGfxCorners[1]->GetActiveSize().x);
	labelRect.h = mpLabelFont->mvSize.y + mvLabelTextOffset.y * 2;
	labelRect.x = GetGlobalPosition().x + mvGfxCorners[0]->GetActiveSize().x;
	labelRect.y = GetGlobalPosition().y + mvGfxCorners[0]->GetActiveSize().y;

	////////////////////////////////
	// Check for collision
	if (cMath::PointBoxCollision(aData.mvPos, labelRect) && aData.mlVal & eGuiMouseButton_Left) {
		mbMoving = true;
		mvRelMousePos = GetPosRelativeToMouse(aData);
		mvRelMousePos.z = GetGlobalPosition().z;
	}

	return true;
}

//-----------------------------------------------------------------------

bool cWidgetWindow::OnMouseUp(cGuiMessageData &aData) {
	if (aData.mlVal & eGuiMouseButton_Left)
		mbMoving = false;

	return true;
}

//-----------------------------------------------------------------------

bool cWidgetWindow::OnMouseEnter(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetWindow::OnMouseLeave(cGuiMessageData &aData) {
	cVector3f vLastGlobal = GetGlobalPosition();

	if (mbMoving)
		SetGlobalPosition(mvRelMousePos + cVector3f(aData.mvPos.x, aData.mvPos.y, 0));

	// Check so that mouse is not outside of clip area.
	if (PointIsInside(aData.mvPos, false) == false) {
		SetGlobalPosition(vLastGlobal);
		mbMoving = false;
	}

	return false;
}

//-----------------------------------------------------------------------

} // namespace hpl
