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

#ifndef HPL_WIDGET_SLIDER_H
#define HPL_WIDGET_SLIDER_H

#include "hpl1/engine/gui/Widget.h"

namespace hpl {

class cGuiSkinFont;

class cWidgetButton;

//---------------------------------------------

class cWidgetSlider : public iWidget {
public:
	cWidgetSlider(cGuiSet *apSet, cGuiSkin *apSkin, eWidgetSliderOrientation aOrientation);
	virtual ~cWidgetSlider();

	int GetValue() { return mlValue; }
	void SetValue(int alValue);

	int GetMaxValue() { return mlMaxValue; }
	void SetMaxValue(int alMax);

	int GetButtonValueAdd() { return mlButtonValueAdd; }
	void SetButtonValueAdd(int alAdd);

	int GetBarValueSize() { return mlBarValueSize; }
	void SetBarValueSize(int alSize);

	float GetButtonSize() const { return mfButtonSize; }

protected:
	/////////////////////////
	// Own functions
	void UpdateBarProperties();

	bool ArrowButtonDown(iWidget *apWidget, cGuiMessageData &aData);
	kGuiCalllbackDeclarationEnd(ArrowButtonDown);

	/////////////////////////
	// Implemented functions
	void OnInit();
	void OnLoadGraphics();
	void OnChangeSize();
	void OnChangePosition();

	void OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion);

	bool OnMouseMove(cGuiMessageData &aData);
	bool OnMouseDown(cGuiMessageData &aData);
	bool OnMouseUp(cGuiMessageData &aData);
	bool OnMouseEnter(cGuiMessageData &aData);
	bool OnMouseLeave(cGuiMessageData &aData);

	bool OnLostFocus(cGuiMessageData &aData);

	/////////////////////////
	// Data
	eWidgetSliderOrientation mOrientation;

	bool mbPressed;

	float mfButtonSize;

	int mlValue;
	int mlMaxValue;
	int mlButtonValueAdd;
	int mlBarValueSize; // This is how big the bar is compared to the max value of the slider
	float mfValueStep;

	cVector3f mvBarPos;
	cVector2f mvBarSize;
	float mfSliderSize;
	cRect2f mBarRect;
	float mfMaxPos;
	float mfMinPos;

	cVector2f mvRelMousePos;

	cWidgetButton *mvButtons[2];

	cGuiGfxElement *mpGfxButtonBackground;
	cGuiGfxElement *mvGfxBorders[4];
	cGuiGfxElement *mvGfxCorners[4];

	cGuiGfxElement *mvGfxArrow[2];
	cGuiGfxElement *mpGfxBackground;
};

} // namespace hpl

#endif // HPL_WIDGET_SLIDER_H
