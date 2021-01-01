/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_UI_DROPDOWN_H
#define BLADERUNNER_UI_DROPDOWN_H

#include "bladerunner/color.h"
#include "bladerunner/ui/ui_component.h"

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace BladeRunner {

class BladeRunnerEngine;
class UIImagePicker;
class UIScrollBox;

typedef void UIDropDownLineSelectedCallback(void *callbackData, void *source, int data, int mouseButton);
typedef void UIDropDownGenericCallback(void *callbackData, void *source);

class UIDropDown : public UIComponent {

	static const int   kDropDownButtonShapeWidth   =  15;
	static const int   kDropDownButtonShapeHeight  =  10;
	static const uint8 kFrameRectPaddingPx         =   2;
	static const int   kFurthestLeftForScrollBar   = 495;

	static const Color256 kColors[];

	int            _controlLeftX;
	Common::String _labelStr;

	bool           _isVisible;

	int            _lineSelectedId;
	Common::String _lineSelectedStr;
	UIScrollBox   *_lineSelectorScrollBox;
	UIImagePicker *_lineDropdownBtn;
	//int            _lineDropdownBtnTopY;
	//int            _lineDropdownBtnHeight;

	Common::Rect   _lineSelectorFrameRect;
	int            _lineSelectorFrameRectColor;
	bool           _lineSelectorFrameRectHasFocus;
	int            _lineSelectorScrollBoxMaxLineWidth;

	UIDropDownLineSelectedCallback   *_ddlLineSelectedCallback;
	UIDropDownGenericCallback        *_ddlCancelledCallback;
	UIDropDownGenericCallback        *_ddlTopFrameClickCallback;
	void                             *_callbackData;

	int            _mouseX;
	int            _mouseY;

public:
	UIDropDown(BladeRunnerEngine *vm, UIDropDownLineSelectedCallback *ddlLineSelectedCallback,
	                                  UIDropDownGenericCallback *ddlCancelledCallback,
	                                  UIDropDownGenericCallback *ddlTopFrameClickCallback,
	                                  void *callbackData,
		                              Common::String labelStr,
	                                  int controlLeftX,
	                                  int controlTopY,
	                                  int scrollBoxMaxLineCount);
	~UIDropDown() override;

	void draw(Graphics::Surface &surface) override;

	void handleMouseMove(int mouseX, int mouseY) override;
	void handleMouseDown(bool alternateButton) override;
	void handleMouseUp(bool alternateButton) override;
	void handleMouseScroll(int direction) override;

	void show();
	void hide();
	bool isVisible();
	bool isDropDownMenuExpanded();

	void activate();
	void deactivate();

	void clearLines();
	void addLine(const Common::String &text, int lineData);
	void addLine(const char *text, int lineData);

	void sortLines();

	void setLabelStr(Common::String newLabel);
	void setControlLeft(int controlLeftX);

	Common::String getLineSelectedStr();

private:
	static void mouseDownLDBCallback(int buttonId, void *callbackData);
	static void scrollBoxLineSelectCallback(void *callbackData, void *source, int lineData, int mouseButton);

	void onButtonPressed(int buttonId);

	void showSelectionDropdown(bool showToggle);
};

} // End of namespace BladeRunner

#endif
