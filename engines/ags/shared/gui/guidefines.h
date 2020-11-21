//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#ifndef __AC_GUIDEFINES_H
#define __AC_GUIDEFINES_H

#define GUIMAGIC          0xcafebeef
#define MAX_GUIOBJ_EVENTS 10
#define TEXTWINDOW_PADDING_DEFAULT  3
//#define MAX_OBJ_EACH_TYPE 251

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
//
//=============================================================================

enum GuiVersion
{
    // TODO: find out all corresponding engine version numbers
    kGuiVersion_Initial     = 0,
    kGuiVersion_214         = 100,
    kGuiVersion_222         = 101,
    kGuiVersion_230         = 102,
    kGuiVersion_unkn_103    = 103,
    kGuiVersion_unkn_104    = 104,
    kGuiVersion_260         = 105,
    kGuiVersion_unkn_106    = 106,
    kGuiVersion_unkn_107    = 107,
    kGuiVersion_unkn_108    = 108,
    kGuiVersion_unkn_109    = 109,
    kGuiVersion_270         = 110,
    kGuiVersion_272a        = 111,
    kGuiVersion_272b        = 112,
    kGuiVersion_272c        = 113,
    kGuiVersion_272d        = 114,
    kGuiVersion_272e        = 115,
    
    kGuiVersion_330         = 116,
    kGuiVersion_331         = 117,
    kGuiVersion_340         = 118,
    kGuiVersion_350         = 119,
    kGuiVersion_Current     = kGuiVersion_350,
};

namespace AGS
{
namespace Common
{

// GUIMain's style and behavior flags
enum GUIMainFlags
{
    kGUIMain_Clickable  = 0x0001,
    kGUIMain_TextWindow = 0x0002,
    kGUIMain_Visible    = 0x0004,
    kGUIMain_Concealed  = 0x0008,

    // NOTE: currently default state is Visible to keep this backwards compatible;
    // check later if this is still a wanted behavior
    kGUIMain_DefFlags   = kGUIMain_Clickable | kGUIMain_Visible,
    // flags that had inverse meaning in old formats
    kGUIMain_OldFmtXorMask = kGUIMain_Clickable
};

// GUIMain's legacy flags, now converted to GUIMainFlags on load
enum GUIMainLegacyFlags
{
    kGUIMain_LegacyTextWindow = 5
};

// GUIMain's style of getting displayed on screen
enum GUIPopupStyle
{
    // Normal GUI
    kGUIPopupNormal           = 0,
    // Shown when the mouse cursor moves to the top of the screen
    kGUIPopupMouseY           = 1,
    // Same as Normal, but pauses the game when shown
    kGUIPopupModal            = 2,
    // Same as Normal, but is not removed when interface is off
    kGUIPopupNoAutoRemove     = 3,
    // (legacy option) Normal GUI, initially off
    // converts to kGUIPopupNormal with Visible = false
    kGUIPopupLegacyNormalOff  = 4
};

// The type of GUIControl
enum GUIControlType
{
    kGUIControlUndefined = -1,
    kGUIButton      = 1,
    kGUILabel       = 2,
    kGUIInvWindow   = 3,
    kGUISlider      = 4,
    kGUITextBox     = 5,
    kGUIListBox     = 6
};

// GUIControl general style and behavior flags
enum GUIControlFlags
{
    kGUICtrl_Default    = 0x0001, // only button
    kGUICtrl_Cancel     = 0x0002, // unused
    kGUICtrl_Enabled    = 0x0004,
    kGUICtrl_TabStop    = 0x0008, // unused
    kGUICtrl_Visible    = 0x0010,
    kGUICtrl_Clip       = 0x0020, // only button
    kGUICtrl_Clickable  = 0x0040,
    kGUICtrl_Translated = 0x0080, // 3.3.0.1132
    kGUICtrl_Deleted    = 0x8000, // unused (probably remains from the old editor?)

    kGUICtrl_DefFlags   = kGUICtrl_Enabled | kGUICtrl_Visible | kGUICtrl_Clickable,
    // flags that had inverse meaning in old formats
    kGUICtrl_OldFmtXorMask = kGUICtrl_Enabled | kGUICtrl_Visible | kGUICtrl_Clickable
};

// GUIListBox style and behavior flags
enum GUIListBoxFlags
{
    kListBox_ShowBorder = 0x01,
    kListBox_ShowArrows = 0x02,
    kListBox_SvgIndex   = 0x04,

    kListBox_DefFlags   = kListBox_ShowBorder | kListBox_ShowArrows,
    // flags that had inverse meaning in old formats
    kListBox_OldFmtXorMask = kListBox_ShowBorder | kListBox_ShowArrows
};

// GUITextBox style and behavior flags
enum GUITextBoxFlags
{
    kTextBox_ShowBorder = 0x0001,

    kTextBox_DefFlags   = kTextBox_ShowBorder,
    // flags that had inverse meaning in old formats
    kTextBox_OldFmtXorMask = kTextBox_ShowBorder
};

// Savegame data format
// TODO: move to the engine code
enum GuiSvgVersion
{
    kGuiSvgVersion_Initial  = 0,
    kGuiSvgVersion_350      = 1
};

} // namespace Common
} // namespace AGS

extern int guis_need_update;

#endif // __AC_GUIDEFINES_H
