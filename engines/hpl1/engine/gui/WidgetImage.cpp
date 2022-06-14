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

#include "hpl1/engine/gui/WidgetImage.h"

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

cWidgetImage::cWidgetImage(cGuiSet *apSet, cGuiSkin *apSkin) : iWidget(eWidgetType_Image, apSet, apSkin) {
	mpGfxImage = NULL;
	mColor = cColor(1, 1);
}

//-----------------------------------------------------------------------

cWidgetImage::~cWidgetImage() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWidgetImage::SetImage(cGuiGfxElement *apGfx) {
	if (mpGfxImage == apGfx)
		return;

	mpGfxImage = apGfx;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWidgetImage::OnLoadGraphics() {
}

//-----------------------------------------------------------------------

void cWidgetImage::OnChangeSize() {
}

//-----------------------------------------------------------------------

void cWidgetImage::OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion) {
	////////////////////////////////
	// Background
	if (mpGfxImage) {
		mpSet->DrawGfx(mpGfxImage, GetGlobalPosition(), mvSize, mColor);
	}
}

//-----------------------------------------------------------------------

bool cWidgetImage::OnMouseMove(cGuiMessageData &aData) {
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetImage::OnMouseDown(cGuiMessageData &aData) {
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetImage::OnMouseUp(cGuiMessageData &aData) {
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetImage::OnMouseEnter(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetImage::OnMouseLeave(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

} // namespace hpl
