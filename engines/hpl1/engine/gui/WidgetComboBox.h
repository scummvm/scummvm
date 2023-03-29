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

#ifndef HPL_WIDGET_COMBO_BOX_H
#define HPL_WIDGET_COMBO_BOX_H

#include "hpl1/engine/gui/Widget.h"
#include "hpl1/engine/gui/WidgetBaseClasses.h"

namespace hpl {

class cGuiSkinFont;

class cWidgetTextBox;
class cWidgetButton;
class cWidgetSlider;

class cWidgetComboBox : public iWidget, public iWidgetItemContainer {
public:
	cWidgetComboBox(cGuiSet *apSet, cGuiSkin *apSkin);
	virtual ~cWidgetComboBox();

	void SetSelectedItem(int alX, bool abMoveList = false);
	int GetSelectedItem() { return mlSelectedItem; }

	void SetCanEdit(bool abX);
	bool GetCanEdit();

	void SetMaxShownItems(int alX);
	int GetMaxShownItems() { return mlMaxItems; }

protected:
	/////////////////////////
	// Own functions
	void UpdateProperties();

	void OpenMenu();
	void CloseMenu();

	bool ButtonPress(iWidget *apWidget, cGuiMessageData &aData);
	kGuiCalllbackDeclarationEnd(ButtonPress);

	bool DrawText(iWidget *apWidget, cGuiMessageData &aData);
	kGuiCalllbackDeclarationEnd(DrawText);

	bool SliderMove(iWidget *apWidget, cGuiMessageData &aData);
	kGuiCalllbackDeclarationEnd(SliderMove);

	bool SliderLostFocus(iWidget *apWidget, cGuiMessageData &aData);
	kGuiCalllbackDeclarationEnd(SliderLostFocus);

	/////////////////////////
	// Implemented functions
	void OnLoadGraphics();
	void OnChangeSize();
	void OnChangeText();
	void OnInit();

	void OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion);

	bool OnMouseMove(cGuiMessageData &aData);
	bool OnMouseDown(cGuiMessageData &aData);
	bool OnMouseUp(cGuiMessageData &aData);
	bool OnMouseEnter(cGuiMessageData &aData);
	bool OnMouseLeave(cGuiMessageData &aData);

	bool OnLostFocus(cGuiMessageData &aData);

	/////////////////////////
	// Data
	cWidgetTextBox *mpText;
	cWidgetButton *mpButton;
	cWidgetSlider *mpSlider;

	bool mbMenuOpen;
	float mfMenuHeight;

	float mfButtonWidth;
	float mfSliderWidth;

	int mlMouseOverSelection;
	int mlSelectedItem;
	int mlFirstItem;
	int mlMaxItems;
	int mlItemsShown;

	cGuiGfxElement *mpGfxBackground;

	cGuiGfxElement *mpGfxSelection;

	cGuiGfxElement *mvGfxBorders[4];
	cGuiGfxElement *mvGfxCorners[4];
};

} // namespace hpl

#endif // HPL_WIDGET_COMBO_BOX_H
