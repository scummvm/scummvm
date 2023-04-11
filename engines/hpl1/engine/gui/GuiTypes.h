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

#ifndef HPL_GUI_TYPES_H
#define HPL_GUI_TYPES_H

#include "common/keyboard.h"
#include "common/list.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/input/InputTypes.h"
#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

//--------------------------------

enum eGuiMouseButton {
	eGuiMouseButton_Left = 0x00000001,
	eGuiMouseButton_Middle = 0x00000002,
	eGuiMouseButton_Right = 0x00000004,
	eGuiMouseButton_LastEnum = 4
};

//--------------------------------

enum eGuiArrowKey {
	eGuiArrowKey_Left,
	eGuiArrowKey_Right,
	eGuiArrowKey_Up,
	eGuiArrowKey_Down,
	eGuiArrowKey_LastEnum
};

//--------------------------------

enum eGuiMaterial {
	eGuiMaterial_Diffuse,
	eGuiMaterial_Alpha,
	eGuiMaterial_FontNormal,
	eGuiMaterial_Additive,
	eGuiMaterial_Modulative,
	eGuiMaterial_LastEnum,
};

//--------------------------------

enum eWidgetType {
	eWidgetType_Root,
	eWidgetType_Window,
	eWidgetType_Button,
	eWidgetType_Frame,
	eWidgetType_Label,
	eWidgetType_Slider,
	eWidgetType_TextBox,
	eWidgetType_CheckBox,
	eWidgetType_Image,
	eWidgetType_ListBox,
	eWidgetType_ComboBox,

	eWidgetType_User,
	eWidgetType_LastEnum,
};

//--------------------------------

enum eGuiSkinFont {
	eGuiSkinFont_Default,
	eGuiSkinFont_Disabled,
	eGuiSkinFont_WindowLabel,

	eGuiSkinFont_LastEnum
};

//--------------------------------

enum eGuiSkinAttribute {
	eGuiSkinAttribute_WindowLabelTextOffset,

	eGuiSkinAttribute_ButtonPressedContentOffset,

	eGuiSkinAttribute_SliderButtonSize,

	eGuiSkinAttribute_ListBoxSliderWidth,

	eGuiSkinAttribute_ComboBoxButtonWidth,
	eGuiSkinAttribute_ComboBoxSliderWidth,

	eGuiSkinAttribute_LastEnum
};
//--------------------------------

enum eGuiSkinGfx {
	///////////////////////////////////
	// Pointer
	eGuiSkinGfx_PointerNormal,
	eGuiSkinGfx_PointerText,

	///////////////////////////////////
	// Window
	eGuiSkinGfx_WindowBorderRight,
	eGuiSkinGfx_WindowBorderLeft,
	eGuiSkinGfx_WindowBorderUp,
	eGuiSkinGfx_WindowBorderDown,

	eGuiSkinGfx_WindowCornerLU,
	eGuiSkinGfx_WindowCornerRU,
	eGuiSkinGfx_WindowCornerRD,
	eGuiSkinGfx_WindowCornerLD,

	eGuiSkinGfx_WindowLabel,
	eGuiSkinGfx_WindowBackground,

	///////////////////////////////////
	// Frame
	eGuiSkinGfx_FrameBorderRight,
	eGuiSkinGfx_FrameBorderLeft,
	eGuiSkinGfx_FrameBorderUp,
	eGuiSkinGfx_FrameBorderDown,

	eGuiSkinGfx_FrameCornerLU,
	eGuiSkinGfx_FrameCornerRU,
	eGuiSkinGfx_FrameCornerRD,
	eGuiSkinGfx_FrameCornerLD,

	eGuiSkinGfx_FrameBackground,

	///////////////////////////////////
	// Check Box
	eGuiSkinGfx_CheckBoxEnabledUnchecked,
	eGuiSkinGfx_CheckBoxEnabledChecked,
	eGuiSkinGfx_CheckBoxDisabledUnchecked,
	eGuiSkinGfx_CheckBoxDisabledChecked,

	///////////////////////////////////
	// Text Box
	eGuiSkinGfx_TextBoxBackground,
	eGuiSkinGfx_TextBoxSelectedTextBack,
	eGuiSkinGfx_TextBoxMarker,

	///////////////////////////////////
	// List Box
	eGuiSkinGfx_ListBoxBackground,

	///////////////////////////////////
	// Combo Box
	eGuiSkinGfx_ComboBoxButtonIcon,

	eGuiSkinGfx_ComboBoxBorderRight,
	eGuiSkinGfx_ComboBoxBorderLeft,
	eGuiSkinGfx_ComboBoxBorderUp,
	eGuiSkinGfx_ComboBoxBorderDown,

	eGuiSkinGfx_ComboBoxCornerLU,
	eGuiSkinGfx_ComboBoxCornerRU,
	eGuiSkinGfx_ComboBoxCornerRD,
	eGuiSkinGfx_ComboBoxCornerLD,

	eGuiSkinGfx_ComboBoxBackground,

	///////////////////////////////////
	// Slider
	eGuiSkinGfx_SliderVertArrowUp,
	eGuiSkinGfx_SliderVertArrowDown,
	eGuiSkinGfx_SliderVertBackground,

	eGuiSkinGfx_SliderHoriArrowUp,
	eGuiSkinGfx_SliderHoriArrowDown,
	eGuiSkinGfx_SliderHoriBackground,

	///////////////////////////////////
	// Button
	eGuiSkinGfx_ButtonUpBorderRight,
	eGuiSkinGfx_ButtonUpBorderLeft,
	eGuiSkinGfx_ButtonUpBorderUp,
	eGuiSkinGfx_ButtonUpBorderDown,

	eGuiSkinGfx_ButtonUpCornerLU,
	eGuiSkinGfx_ButtonUpCornerRU,
	eGuiSkinGfx_ButtonUpCornerRD,
	eGuiSkinGfx_ButtonUpCornerLD,

	eGuiSkinGfx_ButtonUpBackground,

	eGuiSkinGfx_ButtonDownBorderRight,
	eGuiSkinGfx_ButtonDownBorderLeft,
	eGuiSkinGfx_ButtonDownBorderUp,
	eGuiSkinGfx_ButtonDownBorderDown,

	eGuiSkinGfx_ButtonDownCornerLU,
	eGuiSkinGfx_ButtonDownCornerRU,
	eGuiSkinGfx_ButtonDownCornerRD,
	eGuiSkinGfx_ButtonDownCornerLD,

	eGuiSkinGfx_ButtonDownBackground,

	eGuiSkinGfx_LastEnum,
};

//---------------------------------------------

enum eWidgetSliderOrientation {
	eWidgetSliderOrientation_Horisontal,
	eWidgetSliderOrientation_Vertical,
	eWidgetSliderOrientation_LastEnum
};

//--------------------------------

enum eGuiMessage {
	eGuiMessage_MouseMove,        // pos= mouse pos, lVal=buttons down
	eGuiMessage_MouseDown,        // pos= mouse pos, lVal=button
	eGuiMessage_MouseUp,          // pos= mouse pos, lVal=button
	eGuiMessage_MouseDoubleClick, // pos= mouse pos, lVal=button
	eGuiMessage_MouseEnter,       // pos= mouse pos, lVal=buttons down
	eGuiMessage_MouseLeave,       // pos= mouse pos, lVal=buttons down

	eGuiMessage_GotFocus,  // pos= mouse pos, lVal=buttons down
	eGuiMessage_LostFocus, // pos= mouse pos, lVal=buttons down

	eGuiMessage_OnDraw, // fVal = TimeStep, data = clipregion pointer

	eGuiMessage_ButtonPressed, // pos= mouse pos, lVal=buttons down

	eGuiMessage_TextChange, // nothing

	eGuiMessage_CheckChange, // lVal = checked or not.

	eGuiMessage_KeyPress, // pos= mouse pos, lVal=char code.

	eGuiMessage_SliderMove, // val=slider value

	eGuiMessage_SelectionChange, // val=selected val

	eGuiMessage_LastEnum,
};

//--------------------------------

struct cGuiMessageData {
	cGuiMessageData() {}
	cGuiMessageData(const cVector2f &avPos, const cVector2f &avRel) {
		mvPos = avPos;
		mvRel = avRel;
	}
	cGuiMessageData(const cVector2f &avPos, const cVector2f &avRel, int alVal) {
		mvPos = avPos;
		mvRel = avRel;
		mlVal = alVal;
	}
	cGuiMessageData(int alVal) {
		mlVal = alVal;
	}
	cGuiMessageData(float afVal) {
		mfVal = afVal;
	}
	cGuiMessageData(Common::KeyState aKeyPress) {
		mKeyPress = aKeyPress;
	}

	cVector2f mvPos;
	cVector2f mvRel;
	int mlVal;
	Common::KeyState mKeyPress;
	float mfVal;
	void *mpData;
	eGuiMessage mMessage;
};

//--------------------------------

class iWidget;

typedef bool (*tGuiCallbackFunc)(void *, iWidget *, cGuiMessageData &);

#define kGuiCalllbackDeclarationEnd(FuncName) \
	static bool FuncName##_static_gui(void *apObject, iWidget *apWidget, cGuiMessageData &aData)

#define kGuiCalllbackDeclaredFuncEnd(ThisClass, FuncName)                                              \
	bool ThisClass::FuncName##_static_gui(void *apObject, iWidget *apWidget, cGuiMessageData &aData) { \
		return ((ThisClass *)apObject)->FuncName(apWidget, aData);                                     \
	}

#define kGuiCalllbackFuncEnd(ThisClass, FuncName)                                                  \
	static bool FuncName##_static_gui(void *apObject, iWidget *apWidget, cGuiMessageData &aData) { \
		return ((ThisClass *)apObject)->FuncName(apWidget, aData);                                 \
	}

#define kGuiCallback(FuncName) &FuncName##_static_gui

//--------------------------------

typedef Common::List<iWidget *> tWidgetList;
typedef tWidgetList::iterator tWidgetListIt;

//--------------------------------

class cGuiGfxElement;

typedef Common::List<cGuiGfxElement *> tGuiGfxElementList;
typedef tGuiGfxElementList::iterator tGuiGfxElementListIt;

//--------------------------------

} // namespace hpl

#endif // HPL_GAME_TYPES_H
