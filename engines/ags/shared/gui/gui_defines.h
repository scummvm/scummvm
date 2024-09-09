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

#ifndef AGS_SHARED_GUI_GUI_DEFINES_H
#define AGS_SHARED_GUI_GUI_DEFINES_H

namespace AGS3 {

#define GUIMAGIC          0xcafebeef
#define MAX_GUIOBJ_EVENTS 10
#define TEXTWINDOW_PADDING_DEFAULT  3

// TODO: find out more about gui version history
//=============================================================================
// GUI Version history
//-----------------------------------------------------------------------------
//
// 2.1.4..... (100): Introduced Slider gui control. Gui count is now serialized
//                   in game file.
// 2.2.2..... (101): Introduced TextBox gui control.
// 2.3.0..... (102): Introduced ListBox gui control.
// 2.6.0..... (105): GUI custom Z-order support.
// 2.7.0..... (110): Added GUI OnClick handler.
// 2.7.2.???? (111): Added text alignment property to buttons.
// 2.7.2.???? (112): Added text alignment property to list boxes.
// 2.7.2.???? (113): Increased permitted length of GUI label text from 200 to
//                   2048 characters.
// 2.7.2.???? (114): Obsoleted savegameindex[] array, and added
//                   ListBox.SaveGameSlots[] array instead.
// 2.7.2.???? (115): Added GUI Control z-order support.
//
// 3.3.0.1132 (116): Added kGUICtrl_Translated flag.
// 3.3.1.???? (117): Added padding variable for text window GUIs.
// 3.4.0      (118): Removed GUI limits
// 3.5.0      (119): Game data contains GUI properties that previously
//                   could be set only at runtime.
// Since then format value is defined as AGS version represented as NN,NN,NN,NN
//=============================================================================

enum GuiVersion {
	// TODO: find out all corresponding engine version numbers
	kGuiVersion_Initial = 0,
	kGuiVersion_214 = 100,
	kGuiVersion_222 = 101,
	kGuiVersion_230 = 102,
	kGuiVersion_unkn_103 = 103,
	kGuiVersion_unkn_104 = 104,
	kGuiVersion_260 = 105,
	kGuiVersion_unkn_106 = 106,
	kGuiVersion_unkn_107 = 107,
	kGuiVersion_unkn_108 = 108,
	kGuiVersion_unkn_109 = 109,
	kGuiVersion_270 = 110,
	kGuiVersion_272a = 111,
	kGuiVersion_272b = 112,
	kGuiVersion_272c = 113,
	kGuiVersion_272d = 114,
	kGuiVersion_272e = 115,

	kGuiVersion_330 = 116,
	kGuiVersion_331 = 117,
	kGuiVersion_340 = 118,
	kGuiVersion_350 = 119,
	kGuiVersion_Current = kGuiVersion_350,
};

namespace AGS {
namespace Shared {

// GUIMain's style and behavior flags
enum GUIMainFlags {
	kGUIMain_Clickable = 0x0001,
	kGUIMain_TextWindow = 0x0002,
	kGUIMain_Visible = 0x0004,
	kGUIMain_Concealed = 0x0008,

	// NOTE: currently default state is Visible to keep this backwards compatible;
	// check later if this is still a wanted behavior
	kGUIMain_DefFlags = kGUIMain_Clickable | kGUIMain_Visible,
	// flags that had inverse meaning in old formats
	kGUIMain_OldFmtXorMask = kGUIMain_Clickable
};

// GUIMain's legacy flags, now converted to GUIMainFlags on load
enum GUIMainLegacyFlags {
	kGUIMain_LegacyTextWindow = 5
};

// GUIMain's style of getting displayed on screen
enum GUIPopupStyle {
	// Normal GUI
	kGUIPopupNormal = 0,
	// Shown when the mouse cursor moves to the top of the screen
	kGUIPopupMouseY = 1,
	// Same as Normal, but pauses the game when shown
	kGUIPopupModal = 2,
	// Same as Normal, but is not removed when interface is off
	kGUIPopupNoAutoRemove = 3,
	// (legacy option) Normal GUI, initially off
	// converts to kGUIPopupNormal with Visible = false
	kGUIPopupLegacyNormalOff = 4
};

// The type of GUIControl
enum GUIControlType {
	kGUIControlUndefined = -1,
	kGUIButton = 1,
	kGUILabel = 2,
	kGUIInvWindow = 3,
	kGUISlider = 4,
	kGUITextBox = 5,
	kGUIListBox = 6
};

// GUIControl general style and behavior flags
enum GUIControlFlags {
	kGUICtrl_Default = 0x0001, // only button
	kGUICtrl_Cancel = 0x0002, // unused
	kGUICtrl_Enabled = 0x0004,
	kGUICtrl_TabStop = 0x0008, // unused
	kGUICtrl_Visible = 0x0010,
	kGUICtrl_Clip = 0x0020, // only button
	kGUICtrl_Clickable = 0x0040,
	kGUICtrl_Translated = 0x0080, // 3.3.0.1132
	kGUICtrl_Deleted = 0x8000, // unused (probably remains from the old editor?)

	kGUICtrl_DefFlags = kGUICtrl_Enabled | kGUICtrl_Visible | kGUICtrl_Clickable | kGUICtrl_Translated,
	// flags that had inverse meaning in old formats
	kGUICtrl_OldFmtXorMask = kGUICtrl_Enabled | kGUICtrl_Visible | kGUICtrl_Clickable
};

// Label macro flags, define which macros are present in the Label's Text
enum GUILabelMacro {
	kLabelMacro_None = 0,
	kLabelMacro_Gamename = 0x01,
	kLabelMacro_Overhotspot = 0x02,
	kLabelMacro_Score = 0x04,
	kLabelMacro_ScoreText = 0x08,
	kLabelMacro_TotalScore = 0x10,

	kLabelMacro_AllScore = kLabelMacro_Score | kLabelMacro_ScoreText,
	kLabelMacro_All = 0xFFFF
};

// GUIListBox style and behavior flags
enum GUIListBoxFlags {
	kListBox_ShowBorder = 0x01,
	kListBox_ShowArrows = 0x02,
	kListBox_SvgIndex = 0x04,

	kListBox_DefFlags = kListBox_ShowBorder | kListBox_ShowArrows,
	// flags that had inverse meaning in old formats
	kListBox_OldFmtXorMask = kListBox_ShowBorder | kListBox_ShowArrows
};

// GUITextBox style and behavior flags
enum GUITextBoxFlags {
	kTextBox_ShowBorder = 0x0001,

	kTextBox_DefFlags = kTextBox_ShowBorder,
	// flags that had inverse meaning in old formats
	kTextBox_OldFmtXorMask = kTextBox_ShowBorder
};

// Savegame data format
// TODO: move to the engine code
enum GuiSvgVersion {
	kGuiSvgVersion_Initial = 0,
	kGuiSvgVersion_350,
	kGuiSvgVersion_36020,
	kGuiSvgVersion_36023,
	kGuiSvgVersion_36025
};

// Style of GUI drawing in disabled state
enum GuiDisableStyle {
	kGuiDis_Undefined = -1, // this is for marking not-disabled state
	kGuiDis_Greyout = 0,    // paint "gisabled" effect over controls
	kGuiDis_Blackout = 1,   // invisible controls (but guis are shown
	kGuiDis_Unchanged = 2,  // no change, display normally
	kGuiDis_Off = 3         // fully invisible guis
};

// Global GUI options
struct GuiOptions {
	// Clip GUI control's contents to the control's rectangle
	bool ClipControls = true;
	// How the GUI controls are drawn when the interface is disabled
	GuiDisableStyle DisabledStyle = kGuiDis_Unchanged;
	// Whether to graphically outline GUI controls
	bool OutlineControls = false;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
