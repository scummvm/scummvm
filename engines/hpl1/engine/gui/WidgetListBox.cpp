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

#include "hpl1/engine/gui/WidgetListBox.h"

#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/gui/Gui.h"
#include "hpl1/engine/gui/GuiGfxElement.h"
#include "hpl1/engine/gui/GuiSet.h"
#include "hpl1/engine/gui/GuiSkin.h"

#include "hpl1/engine/gui/WidgetSlider.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cWidgetListBox::cWidgetListBox(cGuiSet *apSet, cGuiSkin *apSkin) : iWidget(eWidgetType_ListBox, apSet, apSkin) {
	mbClipsGraphics = true;

	mfBackgroundZ = -0.5;

	mlFirstItem = 0;
	mlMaxItems = 1;

	mlSelectedItem = -1;

	mfSliderWidth = mpSkin->GetAttribute(eGuiSkinAttribute_ListBoxSliderWidth).x;

	mpSlider = NULL;
}

//-----------------------------------------------------------------------

cWidgetListBox::~cWidgetListBox() {
	if (mpSet->IsDestroyingSet() == false) {
		mpSet->DestroyWidget(mpSlider);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

void cWidgetListBox::SetSelectedItem(int alX, bool abMoveList) {
	if (mlSelectedItem == alX)
		return;

	mlSelectedItem = alX;

	if (abMoveList && mlSelectedItem >= mlFirstItem + mlMaxItems) {
		while (mlSelectedItem >= mlFirstItem + mlMaxItems) {
			mlFirstItem++;
		}
		mpSlider->SetValue(mlFirstItem);
	}
	if (abMoveList && mlSelectedItem < mlFirstItem && mlSelectedItem >= 0) {
		while (mlSelectedItem < mlFirstItem) {
			mlFirstItem--;
		}
		mpSlider->SetValue(mlSelectedItem);
	}

	cGuiMessageData data = cGuiMessageData(mlSelectedItem);
	ProcessMessage(eGuiMessage_SelectionChange, data);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

bool cWidgetListBox::DrawText(iWidget *apWidget, cGuiMessageData &aData) {
	cVector3f vPosition = GetGlobalPosition() + cVector3f(3, 2, 0);
	for (int i = mlFirstItem; i < (int)mvItems.size(); ++i) {
		if (i - mlFirstItem > mlMaxItems)
			break;

		if (i == mlSelectedItem) {
			mpSet->DrawGfx(mpGfxSelection, vPosition - cVector3f(3, 0, 0),
						   cVector2f(mvSize.x, mvDefaultFontSize.y));
		}

		DrawDefaultText(mvItems[i], vPosition, eFontAlign_Left);
		vPosition.y += mvDefaultFontSize.y + 2;
	}

	return true;
}
kGuiCalllbackDeclaredFuncEnd(cWidgetListBox, DrawText)

	//-----------------------------------------------------------------------

	bool cWidgetListBox::MoveSlider(iWidget *apWidget, cGuiMessageData &aData) {
	mlFirstItem = aData.mlVal;

	return true;
}
kGuiCalllbackDeclaredFuncEnd(cWidgetListBox, MoveSlider)

	//-----------------------------------------------------------------------

	void cWidgetListBox::UpdateProperties() {
	mlMaxItems = (int)(mvSize.y / (mvDefaultFontSize.y + 2));

	if ((int)mvItems.size() > mlMaxItems) {
		mpSlider->SetBarValueSize(mlMaxItems);
		mpSlider->SetMaxValue((int)mvItems.size() - mlMaxItems);
	} else {
		mpSlider->SetMaxValue(0);
		mpSlider->SetBarValueSize(1);
	}
}

//-----------------------------------------------------------------------

void cWidgetListBox::OnInit() {
	mpSlider = mpSet->CreateWidgetSlider(eWidgetSliderOrientation_Vertical, 0, 0, 0, this);
	mpSlider->AddCallback(eGuiMessage_SliderMove, this, kGuiCallback(MoveSlider));

	AddCallback(eGuiMessage_OnDraw, this, kGuiCallback(DrawText));

	OnChangeSize();
}

//-----------------------------------------------------------------------

void cWidgetListBox::OnChangeSize() {
	if (mpSlider) {
		mpSlider->SetSize(cVector2f(mfSliderWidth, mvSize.y));
		mpSlider->SetPosition(cVector3f(mvSize.x - mfSliderWidth, 0, 0.2f));

		UpdateProperties();
	}
}

//-----------------------------------------------------------------------

void cWidgetListBox::OnLoadGraphics() {
	mpGfxBackground = mpSkin->GetGfx(eGuiSkinGfx_ListBoxBackground);

	mpGfxSelection = mpSkin->GetGfx(eGuiSkinGfx_TextBoxSelectedTextBack);

	mvGfxBorders[0] = mpSkin->GetGfx(eGuiSkinGfx_FrameBorderRight);
	mvGfxBorders[1] = mpSkin->GetGfx(eGuiSkinGfx_FrameBorderLeft);
	mvGfxBorders[2] = mpSkin->GetGfx(eGuiSkinGfx_FrameBorderUp);
	mvGfxBorders[3] = mpSkin->GetGfx(eGuiSkinGfx_FrameBorderDown);

	mvGfxCorners[0] = mpSkin->GetGfx(eGuiSkinGfx_FrameCornerLU);
	mvGfxCorners[1] = mpSkin->GetGfx(eGuiSkinGfx_FrameCornerRU);
	mvGfxCorners[2] = mpSkin->GetGfx(eGuiSkinGfx_FrameCornerRD);
	mvGfxCorners[3] = mpSkin->GetGfx(eGuiSkinGfx_FrameCornerLD);
}

//-----------------------------------------------------------------------

void cWidgetListBox::OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion) {
	////////////////////////////////
	// Background
	mpSet->DrawGfx(mpGfxBackground, GetGlobalPosition() + cVector3f(0, 0, mfBackgroundZ),
				   mvSize);

	////////////////////////////////
	// Borders
	DrawBordersAndCorners(NULL, mvGfxBorders, mvGfxCorners,
						  GetGlobalPosition() -
							  cVector3f(mvGfxCorners[0]->GetActiveSize().x,
										mvGfxCorners[0]->GetActiveSize().y, 0),
						  mvSize + mvGfxCorners[0]->GetActiveSize() +
							  mvGfxCorners[2]->GetActiveSize());
}

//-----------------------------------------------------------------------

bool cWidgetListBox::OnMouseMove(cGuiMessageData &aData) {
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetListBox::OnMouseDown(cGuiMessageData &aData) {
	cVector3f vLocalPos = WorldToLocalPosition(aData.mvPos);

	int lSelection = (int)((vLocalPos.y - 2) / (mvDefaultFontSize.y + 2));
	if (lSelection < 0)
		lSelection = 0;

	lSelection = lSelection + mlFirstItem;

	if (lSelection >= (int)mvItems.size())
		lSelection = (int)mvItems.size() - 1;

	SetSelectedItem(lSelection);

	return true;
}

//-----------------------------------------------------------------------

bool cWidgetListBox::OnMouseUp(cGuiMessageData &aData) {
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetListBox::OnMouseEnter(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetListBox::OnMouseLeave(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetListBox::OnKeyPress(cGuiMessageData &aData) {
	auto key = aData.mKeyPress.keycode;
	if (key == Common::KEYCODE_UP) {
		if (mlSelectedItem > 0)
			SetSelectedItem(mlSelectedItem - 1, true);
	} else if (key == Common::KEYCODE_DOWN) {
		if (mlSelectedItem < (int)mvItems.size() - 1)
			SetSelectedItem(mlSelectedItem + 1, true);
	}

	return true;
}

//-----------------------------------------------------------------------

} // namespace hpl
