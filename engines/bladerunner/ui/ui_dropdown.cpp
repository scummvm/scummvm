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

#include "bladerunner/ui/ui_dropdown.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/ui/ui_scroll_box.h"
#include "bladerunner/subtitles.h"
#include "bladerunner/font.h"
#include "bladerunner/game_info.h"
#include "bladerunner/shape.h"
#include "bladerunner/game_constants.h"

#include "common/debug.h"

namespace BladeRunner {

const Color256 UIDropDown::kColors[] = {
	{   0,   0,   0 }, // Black - unpressed (framing rectange)
	{  16,   8,   8 },
	{  32,  24,   8 },
	{  56,  32,  16 },
	{  72,  48,  16 },
	{  88,  56,  24 }, // Mouse-over (framing rectange)
	{ 104,  72,  32 },
	{ 128,  80,  40 },
	{ 136,  96,  48 },
	{ 152, 112,  56 },
	{ 168, 128,  72 }  // Pressed (framing rectange)
};

UIDropDown::UIDropDown(BladeRunnerEngine *vm,
	                                      UIDropDownLineSelectedCallback *ddlLineSelectedCallback,
	                                      UIDropDownGenericCallback *ddlCancelledCallback,
	                                      UIDropDownGenericCallback *ddlTopFrameClickCallback,
	                                      void *callbackData,
	                                      Common::String labelStr,
	                                      int controlLeftX,
	                                      int controlTopY,
	                                      int scrollBoxMaxLineCount) : UIComponent(vm) {

	_isVisible                         = false;

	_labelStr                          = labelStr;
	_controlLeftX                      = MAX(controlLeftX, 0);
	// TODO The id should be used to eg. grab info about the selected subtitles from an engine's subtitles object
	_lineSelectedId                    = -1;

	_lineSelectorFrameRectColor        = 0;
	_lineSelectorFrameRectHasFocus     = false;
	// A framing (outlining) rectangle to highlight the selected option field on top of the scrollbox
	controlTopY                        = CLIP(controlTopY, 0, 600);
	_lineSelectorFrameRect             = Common::Rect(0, controlTopY, 0, controlTopY + kDropDownButtonShapeHeight);

	// TODO This eventually should be set to a default probably by the outside caller class(kia_section_settings)
	//      Current explicit assignment only serves as placeholder / proof of concept
	_lineSelectedStr                   = "English (SCUMMVM) v7 [ENG]";
	_lineSelectorScrollBox             =  new UIScrollBox(_vm, scrollBoxLineSelectCallback, this, scrollBoxMaxLineCount, 2, false, Common::Rect(0, 0, 0, 55 + kFrameRectPaddingPx), Common::Rect(0, 0, 0, 55));
	_lineSelectorScrollBoxMaxLineWidth = 0;

	_lineDropdownBtn                   = new UIImagePicker(_vm, 2);

	_ddlLineSelectedCallback  = ddlLineSelectedCallback;
	_ddlCancelledCallback     = ddlCancelledCallback;
	_ddlTopFrameClickCallback = ddlTopFrameClickCallback;
	_callbackData             = callbackData;

	_mouseX = 0;
	_mouseY = 0;
}

UIDropDown::~UIDropDown() {
	delete _lineSelectorScrollBox;
	delete _lineDropdownBtn;
}

void UIDropDown::activate() {
	_lineDropdownBtn->resetImages();
	// Actual button shape
	// defineImage actually increases internally the bottom and right bounds for the rect to be inclusive (for the contains() method)
	_lineDropdownBtn->defineImage(0, Common::Rect(0, _lineSelectorFrameRect.top + 1, kDropDownButtonShapeWidth - 1, _lineSelectorFrameRect.bottom - 1), _vm->_kia->_shapes->get(73), _vm->_kia->_shapes->get(74), _vm->_kia->_shapes->get(75), nullptr);
	// Clickable Selected/Active Line Description area
	_lineDropdownBtn->defineImage(1, Common::Rect(0, _lineSelectorFrameRect.top, kDropDownButtonShapeWidth - 1, _lineSelectorFrameRect.bottom - 1), nullptr, nullptr, nullptr, nullptr);
	_lineDropdownBtn->activate(nullptr, nullptr, mouseDownLDBCallback, nullptr, this);

	_lineSelectorScrollBox->setBoxTop(_lineSelectorFrameRect.bottom);
	_lineSelectorScrollBox->setScrollbarTop(_lineSelectorFrameRect.bottom);

	_lineSelectorScrollBox->hide(); // show upon click on field or dropdown button
	show();
}


void UIDropDown::deactivate() {
	_isVisible                = false;

	_lineDropdownBtn->deactivate();
	_lineSelectorScrollBox->hide();
}

void UIDropDown::draw(Graphics::Surface &surface) {
	if (!_isVisible) {
		return;
	}

	int posStartOfSelectedLineDesc = _controlLeftX + _vm->_mainFont->getStringWidth(_labelStr) + _vm->_mainFont->getCharWidth(' ');
	_vm->_mainFont->drawString(&surface, _labelStr, _controlLeftX, _lineSelectorFrameRect.top, surface.w, surface.format.RGBToColor(232, 208, 136));
	_vm->_mainFont->drawString(&surface, _lineSelectedStr,
		                        posStartOfSelectedLineDesc,
		                        _lineSelectorFrameRect.top, surface.w, surface.format.RGBToColor(240, 232, 192));

	// TODO add a clipping for description field here
	int posEndOfSelectedLineDesc = posStartOfSelectedLineDesc + _vm->_mainFont->getStringWidth(_lineSelectedStr) + _vm->_mainFont->getCharWidth(' ');

	_lineDropdownBtn->setImageLeft(0, posEndOfSelectedLineDesc );

	_lineDropdownBtn->setImageLeft(1, posStartOfSelectedLineDesc - kFrameRectPaddingPx);
	_lineDropdownBtn->setImageWidth(1, posEndOfSelectedLineDesc + kFrameRectPaddingPx - posStartOfSelectedLineDesc);

	_lineDropdownBtn->draw(surface);
//	_lineDropdownBtn->drawTooltip(surface, _mouseX, _mouseY);

	_lineSelectorFrameRect.moveTo(posStartOfSelectedLineDesc - kFrameRectPaddingPx, _lineSelectorFrameRect.top);
	_lineSelectorFrameRect.setWidth(posEndOfSelectedLineDesc + kDropDownButtonShapeWidth + kFrameRectPaddingPx - posStartOfSelectedLineDesc);

	_lineSelectorScrollBox->draw(surface);

	int lineSelectorFrameRectTargetColor;
	if (_lineSelectorScrollBox->isVisible()) {
		lineSelectorFrameRectTargetColor = 10;
	} else if (_lineSelectorFrameRectHasFocus) {
		lineSelectorFrameRectTargetColor = 5;
	} else {
		lineSelectorFrameRectTargetColor = 0;
	}

	// Ensures animated transition of the frame's (outlining rectangle's) color to the new one
	if (_lineSelectorFrameRectColor < lineSelectorFrameRectTargetColor) {
		++_lineSelectorFrameRectColor;
	}

	// Ensures animated transition of the frame's (outlining rectangle's) color to the new one
	if (_lineSelectorFrameRectColor > lineSelectorFrameRectTargetColor) {
		--_lineSelectorFrameRectColor;
	}
	surface.frameRect(_lineSelectorFrameRect,
	                  surface.format.RGBToColor(kColors[_lineSelectorFrameRectColor].r,
	                                            kColors[_lineSelectorFrameRectColor].g,
	                                            kColors[_lineSelectorFrameRectColor].b));
}

void UIDropDown::show() {
	_isVisible = true;
}

void UIDropDown::hide() {
	_isVisible = false;
}

bool UIDropDown::isVisible() {
	return _isVisible;
}

bool UIDropDown::isDropDownMenuExpanded() {
	return _lineSelectorScrollBox->isVisible();
}

void UIDropDown::clearLines() {
	_lineSelectorScrollBox->clearLines();
	_lineSelectorScrollBoxMaxLineWidth = 0;
}

void UIDropDown::addLine(const Common::String &text, int lineData) {
	_lineSelectorScrollBox->addLine(text, lineData, 0x08);
	_lineSelectorScrollBoxMaxLineWidth = MAX(_vm->_mainFont->getStringWidth(text), _lineSelectorScrollBoxMaxLineWidth);
}

void UIDropDown::addLine(const char *text, int lineData) {
	_lineSelectorScrollBox->addLine(text, lineData, 0x08);
	_lineSelectorScrollBoxMaxLineWidth = MAX(_vm->_mainFont->getStringWidth(text), _lineSelectorScrollBoxMaxLineWidth);
}

void UIDropDown::sortLines() {
	_lineSelectorScrollBox->sortLines();
}

void UIDropDown::handleMouseMove(int mouseX, int mouseY) {
	if (!_isVisible) {
		return;
	}

	_mouseX = mouseX;
	_mouseY = mouseY;

	// contains() does not include right or bottom boundary "line"
	if (_lineSelectorFrameRect.contains(mouseX, mouseY)) {
		if (!_lineSelectorFrameRectHasFocus && !_lineSelectorScrollBox->isVisible()) {
			_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxTEXT3), 100, 0, 0, 50, 0);
		}
		_lineSelectorFrameRectHasFocus = true;
	} else {
		_lineSelectorFrameRectHasFocus = false;
	}

	_lineSelectorScrollBox->handleMouseMove(mouseX, mouseY);
	_lineDropdownBtn->handleMouseAction(mouseX, mouseY, false, false, false);
}

void UIDropDown::handleMouseDown(bool alternateButton) {
	if (!_isVisible) {
		return;
	}

	if (!alternateButton) {
		_lineSelectorScrollBox->handleMouseDown(false);
		_lineDropdownBtn->handleMouseAction(_mouseX, _mouseY, true, false, false);
		if (!_lineSelectorFrameRectHasFocus
			&& _lineSelectorScrollBox->isVisible()
			&& !_lineSelectorScrollBox->hasFocus()
		) {
			_ddlCancelledCallback(_callbackData, this);
			showSelectionDropdown(false);
		}
	}
}

void UIDropDown::handleMouseScroll(int direction) {
	if (!_isVisible) {
		return;
	}

	if (_lineSelectorScrollBox->isVisible()) {
		_lineSelectorScrollBox->handleMouseScroll(direction);
	}
}

void UIDropDown::handleMouseUp(bool alternateButton) {
	if (!_isVisible) {
		return;
	}

	if (!alternateButton) {
		_lineSelectorScrollBox->handleMouseUp(false);
		_lineDropdownBtn->handleMouseAction(_mouseX, _mouseY, false, true, false);
	}
}

void UIDropDown::scrollBoxLineSelectCallback(void *callbackData, void *source, int lineData, int mouseButton) {
	UIDropDown *self = (UIDropDown *)callbackData;

	if (source == self->_lineSelectorScrollBox && lineData >= 0) {
		Common::String selectedLangDescStr = self->_lineSelectorScrollBox->getLineText(lineData);
		self->_lineSelectedId  = lineData;
		self->_lineSelectedStr = selectedLangDescStr;
		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxELECBP1), 90, 0, 0, 50, 0);
		self->_ddlLineSelectedCallback(self->_callbackData, self, lineData, mouseButton);
		self->showSelectionDropdown(false);
		//debug("text selected: %s", selectedLangDescStr.c_str());
	}
}

// Callback from _lineDropdownBtn items
void UIDropDown::mouseDownLDBCallback(int buttonId, void *callbackData) {
	UIDropDown *self = (UIDropDown *)callbackData;
	self->onButtonPressed(buttonId);
}

void UIDropDown::onButtonPressed(int buttonId) {
	switch (buttonId) {
	case 0:
		// Pressed DDL dropdown button (0)
		// fall through
	case 1:
//		if (buttonId == 1) {
//			// Pressed DDL clickable area (1)
//			debug("Pressed DDL clickable area (1)");
//		}
		_ddlTopFrameClickCallback(_callbackData, this);
		showSelectionDropdown(!_lineSelectorScrollBox->isVisible());
		break;
	default:
		return;
	}
}

void UIDropDown::showSelectionDropdown(bool showToggle) {
	int prevDropdownBtnLeft = _lineDropdownBtn->getImageLeft(0);
	if (showToggle) {
		_lineSelectorScrollBox->setBoxTop(_lineSelectorFrameRect.bottom);
		_lineSelectorScrollBox->setBoxLeft(_lineDropdownBtn->getImageLeft(1));
		// TODO width should be retrieved from the maximum width of a language description in SUBTITLES.MIX (or a max width to clip to)
		_lineSelectorScrollBox->setBoxWidth(MAX(_lineDropdownBtn->getImageWidth(1), _lineSelectorScrollBoxMaxLineWidth + _vm->_mainFont->getCharWidth(' ')));

		if (_lineDropdownBtn->getImageLeft(0) < kFurthestLeftForScrollBar) {
			// CLIP expects the first boundary argument to be the min of the two.
			_lineSelectorScrollBox->setScrollbarLeft(CLIP( _lineSelectorScrollBox->getBoxLeft() + _lineSelectorScrollBox->getBoxWidth(), _lineDropdownBtn->getImageLeft(0), kFurthestLeftForScrollBar));
		} else {
			_lineSelectorScrollBox->setScrollbarLeft(MAX( _lineSelectorScrollBox->getBoxLeft() + _lineSelectorScrollBox->getBoxWidth(), kFurthestLeftForScrollBar));
		}

		_lineSelectorScrollBox->setScrollbarTop(_lineSelectorFrameRect.bottom);
		_lineSelectorScrollBox->setScrollbarWidth(kDropDownButtonShapeWidth);
		_lineSelectorScrollBox->show();
		// change dropdown button icon too
		_lineDropdownBtn->resetActiveImage(0);
		_lineDropdownBtn->defineImage(0, Common::Rect(prevDropdownBtnLeft, _lineSelectorFrameRect.top + 1, prevDropdownBtnLeft + kDropDownButtonShapeWidth - 1, _lineSelectorFrameRect.bottom - 1), _vm->_kia->_shapes->get(70), _vm->_kia->_shapes->get(71), _vm->_kia->_shapes->get(72), nullptr);
		_lineSelectorFrameRectColor = 10;
	} else {
		// hide scrollable area
		_lineSelectorScrollBox->hide();
		// change dropdown button icon too
		_lineDropdownBtn->resetActiveImage(0);
		_lineDropdownBtn->defineImage(0, Common::Rect(prevDropdownBtnLeft, _lineSelectorFrameRect.top + 1, prevDropdownBtnLeft + kDropDownButtonShapeWidth - 1, _lineSelectorFrameRect.bottom - 1), _vm->_kia->_shapes->get(73), _vm->_kia->_shapes->get(74), _vm->_kia->_shapes->get(75), nullptr);
		_lineSelectorFrameRectColor = 0;
	}
}

void UIDropDown::setLabelStr(Common::String newLabel) {
	_labelStr = newLabel;
}

void UIDropDown::setControlLeft(int controlLeftX) {
	_controlLeftX = controlLeftX;
}

Common::String UIDropDown::getLineSelectedStr() {
	return _lineSelectedStr;
}

}
