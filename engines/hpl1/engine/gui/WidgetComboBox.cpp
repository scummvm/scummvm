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

#include "hpl1/engine/gui/WidgetComboBox.h"

#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/gui/Gui.h"
#include "hpl1/engine/gui/GuiGfxElement.h"
#include "hpl1/engine/gui/GuiSet.h"
#include "hpl1/engine/gui/GuiSkin.h"

#include "hpl1/engine/gui/WidgetButton.h"
#include "hpl1/engine/gui/WidgetSlider.h"
#include "hpl1/engine/gui/WidgetTextBox.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cWidgetComboBox::cWidgetComboBox(cGuiSet *apSet, cGuiSkin *apSkin) : iWidget(eWidgetType_ComboBox, apSet, apSkin) {
	mfButtonWidth = mpSkin->GetAttribute(eGuiSkinAttribute_ComboBoxButtonWidth).x;
	mfSliderWidth = mpSkin->GetAttribute(eGuiSkinAttribute_ComboBoxSliderWidth).x;

	mpText = NULL;
	mpButton = NULL;
	mpSlider = NULL;

	mbMenuOpen = false;

	// mbConnectedToChildren = false;

	mfMenuHeight = 0;

	mlSelectedItem = -1;
	mlMouseOverSelection = -1;

	mlFirstItem = 0;
	mlMaxItems = 12;
	mlItemsShown = 0;
}

//-----------------------------------------------------------------------

cWidgetComboBox::~cWidgetComboBox() {
	if (mpSet->IsDestroyingSet() == false) {
		mpSet->DestroyWidget(mpText);
		mpSet->DestroyWidget(mpButton);
		mpSet->DestroyWidget(mpSlider);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWidgetComboBox::SetSelectedItem(int alX, bool abMoveList) {
	if (mlSelectedItem == alX && (mlSelectedItem < 0 ||
								  mpText->GetText() == mvItems[mlSelectedItem]))
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

	if (mlSelectedItem >= 0)
		SetText(mvItems[mlSelectedItem]);

	cGuiMessageData data = cGuiMessageData(mlSelectedItem);
	ProcessMessage(eGuiMessage_SelectionChange, data);
}

//-----------------------------------------------------------------------

void cWidgetComboBox::SetCanEdit(bool abX) {
	mpText->SetCanEdit(abX);
}

bool cWidgetComboBox::GetCanEdit() {
	return mpText->GetCanEdit();
}

//-----------------------------------------------------------------------

void cWidgetComboBox::SetMaxShownItems(int alX) {
	mlMaxItems = alX;

	UpdateProperties();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWidgetComboBox::UpdateProperties() {
	if ((int)mvItems.size() <= mlMaxItems)
		mlItemsShown = (int)mvItems.size();
	else
		mlItemsShown = mlMaxItems;

	mfMenuHeight = 2 + (mvDefaultFontSize.y + 2) * (float)mlItemsShown + 2;

	OnChangeSize();
}

//-----------------------------------------------------------------------

void cWidgetComboBox::OpenMenu() {
	if (mvItems.empty())
		return;

	if (mbMenuOpen)
		return;
	mpSet->SetAttentionWidget(this);
	mbMenuOpen = true;

	mlMouseOverSelection = mlSelectedItem;

	mvSize = cVector2f(mvSize.x, mvSize.y + mfMenuHeight);

	if ((int)mvItems.size() > mlMaxItems) {
		mpSlider->SetEnabled(true);
		mpSlider->SetVisible(true);

		mpSlider->SetPosition(cVector3f(mvSize.x - 20 - mvGfxBorders[1]->GetActiveSize().x,
										(mvSize.y - mfMenuHeight) + mvGfxBorders[1]->GetActiveSize().y,
										1.2f));
		mpSlider->SetSize(cVector2f(20, mfMenuHeight - mvGfxBorders[1]->GetActiveSize().y -
											mvGfxBorders[2]->GetActiveSize().y));

		mpSlider->SetBarValueSize(mlMaxItems);
		mpSlider->SetMaxValue((int)mvItems.size() - mlMaxItems);

		mpSet->SetFocusedWidget(mpSlider);
	} else {
		mpSet->SetFocusedWidget(this);
	}

	mbClipsGraphics = true;
}

//-----------------------------------------------------------------------

void cWidgetComboBox::CloseMenu() {
	if (mbMenuOpen == false)
		return;

	mpSet->SetAttentionWidget(NULL);
	mbMenuOpen = false;

	mvSize = mpText->GetSize();

	mpSlider->SetEnabled(false);
	mpSlider->SetVisible(false);

	mbClipsGraphics = false;
}

//-----------------------------------------------------------------------

bool cWidgetComboBox::ButtonPress(iWidget *apWidget, cGuiMessageData &aData) {
	if (mbMenuOpen)
		CloseMenu();
	else
		OpenMenu();

	return true;
}
kGuiCalllbackDeclaredFuncEnd(cWidgetComboBox, ButtonPress)

	//-----------------------------------------------------------------------

	bool cWidgetComboBox::SliderMove(iWidget *apWidget, cGuiMessageData &aData) {
	mlFirstItem = aData.mlVal;

	return true;
}
kGuiCalllbackDeclaredFuncEnd(cWidgetComboBox, SliderMove)

	bool cWidgetComboBox::SliderLostFocus(iWidget *apWidget, cGuiMessageData &aData) {
	if (mbMenuOpen && GetMouseIsOver() == false) {
		CloseMenu();
	}

	return false;
}
kGuiCalllbackDeclaredFuncEnd(cWidgetComboBox, SliderLostFocus)

	//-----------------------------------------------------------------------

	void cWidgetComboBox::OnInit() {
	mpText = mpSet->CreateWidgetTextBox(0, mvSize, Common::U32String(), this);
	mpText->SetText(msText);
	mpText->SetCanEdit(false);

	mpButton = mpSet->CreateWidgetButton(0, 0, Common::U32String(), this);
	mpButton->SetImage(mpSkin->GetGfx(eGuiSkinGfx_ComboBoxButtonIcon), false);
	mpButton->AddCallback(eGuiMessage_MouseDown, this, kGuiCallback(ButtonPress));

	mpSlider = mpSet->CreateWidgetSlider(eWidgetSliderOrientation_Vertical, 0, 0, 0, this);
	mpSlider->AddCallback(eGuiMessage_SliderMove, this, kGuiCallback(SliderMove));
	mpSlider->AddCallback(eGuiMessage_LostFocus, this, kGuiCallback(SliderLostFocus));
	mpSlider->SetEnabled(false);
	mpSlider->SetVisible(false);

	AddCallback(eGuiMessage_OnDraw, this, kGuiCallback(DrawText));

	OnChangeSize();
}

//-----------------------------------------------------------------------

void cWidgetComboBox::OnChangeSize() {
	if (mpText && mpButton && mpSlider) {
		mpText->SetSize(mvSize);
		mvSize = mpText->GetSize();

		cVector2f vBackSize = mpText->GetBackgroundSize();

		mpButton->SetSize(cVector2f(mfButtonWidth, vBackSize.y));
		mpButton->SetPosition(cVector3f(mvSize.x - (mvSize.x - vBackSize.x) / 2 - mfButtonWidth,
										(mvSize.y - vBackSize.y) / 2, 0.3f));

		mpSlider->SetBarValueSize(mlMaxItems);
		mpSlider->SetMaxValue((int)mvItems.size() - mlMaxItems);
	}
}

//-----------------------------------------------------------------------

void cWidgetComboBox::OnChangeText() {
	if (mpText)
		mpText->SetText(msText);
}

//-----------------------------------------------------------------------

void cWidgetComboBox::OnLoadGraphics() {
	mpGfxBackground = mpSkin->GetGfx(eGuiSkinGfx_ComboBoxBackground);

	mpGfxSelection = mpSkin->GetGfx(eGuiSkinGfx_TextBoxSelectedTextBack);

	mvGfxBorders[0] = mpSkin->GetGfx(eGuiSkinGfx_ComboBoxBorderRight);
	mvGfxBorders[1] = mpSkin->GetGfx(eGuiSkinGfx_ComboBoxBorderLeft);
	mvGfxBorders[2] = mpSkin->GetGfx(eGuiSkinGfx_ComboBoxBorderUp);
	mvGfxBorders[3] = mpSkin->GetGfx(eGuiSkinGfx_ComboBoxBorderDown);

	mvGfxCorners[0] = mpSkin->GetGfx(eGuiSkinGfx_ComboBoxCornerLU);
	mvGfxCorners[1] = mpSkin->GetGfx(eGuiSkinGfx_ComboBoxCornerRU);
	mvGfxCorners[2] = mpSkin->GetGfx(eGuiSkinGfx_ComboBoxCornerRD);
	mvGfxCorners[3] = mpSkin->GetGfx(eGuiSkinGfx_ComboBoxCornerLD);
}

//-----------------------------------------------------------------------

void cWidgetComboBox::OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion) {
	if (mbMenuOpen) {
		mpSet->DrawGfx(mpGfxBackground, GetGlobalPosition() + cVector3f(0, mvSize.y - mfMenuHeight, 1),
					   cVector2f(mvSize.x, mfMenuHeight));

		////////////////////////////////
		// Background and borders
		DrawBordersAndCorners(NULL, mvGfxBorders, mvGfxCorners,
							  GetGlobalPosition() + cVector3f(0, mvSize.y - mfMenuHeight, 1.4f),
							  cVector2f(mvSize.x, mfMenuHeight));
	}
}

//-----------------------------------------------------------------------

bool cWidgetComboBox::DrawText(iWidget *apWidget, cGuiMessageData &aData) {
	if (mbMenuOpen == false)
		return false;

	cVector3f vPos = GetGlobalPosition() +
					 cVector3f(mvGfxBorders[0]->GetActiveSize().x + 3, mpText->GetSize().y + 2, 1.2f);

	for (int i = mlFirstItem; i < (int)mvItems.size(); ++i) {
		if (i - mlFirstItem >= mlMaxItems)
			break;

		if (i == mlMouseOverSelection) {
			mpSet->DrawGfx(mpGfxSelection, vPos - cVector3f(3, 0, 0),
						   cVector2f(mvSize.x, mvDefaultFontSize.y));
		}

		DrawDefaultText(mvItems[i], vPos, eFontAlign_Left);
		vPos.y += mvDefaultFontSize.y + 2;
	}

	return true;
}
kGuiCalllbackDeclaredFuncEnd(cWidgetComboBox, DrawText)

	//-----------------------------------------------------------------------

	bool cWidgetComboBox::OnMouseMove(cGuiMessageData &aData) {
	if (mbMenuOpen == false)
		return false;

	if (GetMouseIsOver() == false)
		return false;

	cVector3f vLocalPos = WorldToLocalPosition(aData.mvPos);

	if (vLocalPos.y <= mpText->GetSize().y)
		return false;
	if (mpSlider->IsEnabled() && vLocalPos.x >= mvSize.x - 20)
		return false;

	float fToTextStart = 2 + mpText->GetSize().y + mvGfxCorners[0]->GetActiveSize().y;
	int lSelection = (int)((vLocalPos.y - fToTextStart) / (mvDefaultFontSize.y + 2));
	if (lSelection < 0)
		lSelection = 0;

	lSelection = lSelection + mlFirstItem;

	if (lSelection >= (int)mvItems.size())
		lSelection = (int)mvItems.size() - 1;

	mlMouseOverSelection = lSelection;

	return true;
}

//-----------------------------------------------------------------------

bool cWidgetComboBox::OnMouseDown(cGuiMessageData &aData) {
	if (mbMenuOpen == false)
		return false;

	cVector3f vLocal = WorldToLocalPosition(aData.mvPos);

	if (vLocal.y < mpText->GetSize().y) {
		CloseMenu();
	}

	return true;
}

//-----------------------------------------------------------------------

bool cWidgetComboBox::OnMouseUp(cGuiMessageData &aData) {
	if (mbMenuOpen == false)
		return false;

	cVector3f vLocal = WorldToLocalPosition(aData.mvPos);

	if (vLocal.y > mpText->GetSize().y) {
		SetSelectedItem(mlMouseOverSelection);
		CloseMenu();
	}

	return true;
}

//-----------------------------------------------------------------------

bool cWidgetComboBox::OnMouseEnter(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetComboBox::OnMouseLeave(cGuiMessageData &aData) {
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetComboBox::OnLostFocus(cGuiMessageData &aData) {
	/*cVector3f vLocal = */ WorldToLocalPosition(aData.mvPos);

	if (mbMenuOpen && mpSlider->IsEnabled() == false) {
		CloseMenu();
	}
	return false;
}

//-----------------------------------------------------------------------

} // namespace hpl
