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

#ifndef AGS_SHARED_GUI_GUI_MAIN_H
#define AGS_SHARED_GUI_GUI_MAIN_H

#include "ags/lib/std/vector.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/common_defines.h" // TODO: split out gui drawing helpers
#include "ags/shared/gfx/gfx_def.h" // TODO: split out gui drawing helpers
#include "ags/shared/gui/gui_defines.h"
#include "ags/shared/util/error.h"
#include "ags/shared/util/geometry.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
}
}
using namespace AGS; // FIXME later

// There were issues when including header caused conflicts
struct GameSetupStruct;

#define LEGACY_MAX_OBJS_ON_GUI             30

#define GUIMAIN_LEGACY_RESERVED_INTS       5
#define GUIMAIN_LEGACY_NAME_LENGTH         16
#define GUIMAIN_LEGACY_EVENTHANDLER_LENGTH 20
#define GUIMAIN_LEGACY_TW_FLAGS_SIZE       4

namespace AGS {
namespace Shared {

// Legacy GUIMain visibility state, which combined Visible property and override factor
enum LegacyGUIVisState {
	kGUIVisibility_LockedOff = -1, // locked hidden (used by PopupMouseY guis)
	kGUIVisibility_Off = 0, // hidden
	kGUIVisibility_On = 1  // shown
};

class Bitmap;
class GUIObject;


class GUIMain {
public:
	static String FixupGUIName(const String &name);

public:
	GUIMain();

	void        InitDefaults();

	// Tells if the gui background supports alpha channel
	bool        HasAlphaChannel() const;
	// Tells if GUI will react on clicking on it
	bool        IsClickable() const;
	// Tells if GUI's visibility is overridden and it won't be displayed on
	// screen regardless of Visible property (until concealed mode is off).
	bool        IsConcealed() const;
	// Tells if gui is actually meant to be displayed on screen.
	// Normally Visible property determines whether GUI is allowed to be seen,
	// but there may be other settings that override it.
	bool        IsDisplayed() const;
	// Tells if given coordinates are within interactable area of gui
	// NOTE: this currently tests for actual visibility and Clickable property
	bool        IsInteractableAt(int x, int y) const;
	// Tells if gui is a text window
	bool        IsTextWindow() const;
	// Tells if GUI is *allowed* to be displayed and interacted with.
	// This does not necessarily mean that it is displayed right now, because
	// GUI may be hidden for other reasons, including overriding behavior.
	// For example GUI with kGUIPopupMouseY style will not be shown unless
	// mouse cursor is at certain position on screen.
	bool        IsVisible() const;

	// Tells if GUI has graphically changed recently
	bool        HasChanged() const;
	// Manually marks GUI as graphically changed
	void        MarkChanged();
	// Clears changed flag
	void        ClearChanged();

	int32_t FindControlUnderMouse() const;
	// this version allows some extra leeway in the Editor so that
	// the user can grab tiny controls
	int32_t FindControlUnderMouse(int leeway) const;
	int32_t FindControlUnderMouse(int leeway, bool must_be_clickable) const;
	// Gets the number of the GUI child controls
	int32_t GetControlCount() const;
	// Gets control by its child's index
	GUIObject *GetControl(int index) const;
	// Gets child control's type, looks up with child's index
	GUIControlType GetControlType(int index) const;
	// Gets child control's global ID, looks up with child's index
	int32_t GetControlID(int index) const;

	// Child control management
	// Note that currently GUIMain does not own controls (should not delete them)
	void    AddControl(GUIControlType type, int32_t id, GUIObject *control);
	void    RemoveAllControls();

	// Operations
	bool    BringControlToFront(int index);
	void    Draw(Bitmap *ds);
	void    DrawAt(Bitmap *ds, int x, int y);
	void    Poll();
	HError  RebuildArray();
	void    ResortZOrder();
	bool    SendControlToBack(int index);
	// Sets whether GUI should react to player clicking on it
	void    SetClickable(bool on);
	// Override GUI visibility; when in concealed mode GUI won't show up
	// even if Visible = true
	void    SetConceal(bool on);
	// Attempts to change control's zorder; returns if zorder changed
	bool    SetControlZOrder(int index, int zorder);
	// Changes GUI style to the text window or back
	void    SetTextWindow(bool on);
	// Sets GUI transparency as a percentage (0 - 100) where 100 = invisible
	void    SetTransparencyAsPercentage(int percent);
	// Sets whether GUI is allowed to be displayed on screen
	void    SetVisible(bool on);

	// Events
	void    OnMouseButtonDown();
	void    OnMouseButtonUp();
	void    OnControlPositionChanged();

	// Serialization
	void    ReadFromFile(Stream *in, GuiVersion gui_version);
	void    WriteToFile(Stream *out) const;
	// TODO: move to engine, into gui savegame component unit
	// (should read/write GUI properties accessing them by interface)
	void    ReadFromSavegame(Stream *in, GuiSvgVersion svg_version);
	void    WriteToSavegame(Stream *out) const;

private:
	void    DrawBlob(Bitmap *ds, int x, int y, color_t draw_color);

	// TODO: all members are currently public; hide them later
public:
	int32_t ID;             // GUI identifier
	String  Name;           // the name of the GUI

	int32_t X;
	int32_t Y;
	int32_t Width;
	int32_t Height;
	color_t BgColor;        // background color
	int32_t BgImage;        // background sprite index
	color_t FgColor;        // foreground color (used as border color in normal GUIs,
	// and text color in text windows)
	int32_t Padding;        // padding surrounding a GUI text window
	GUIPopupStyle PopupStyle; // GUI popup behavior
	int32_t PopupAtMouseY;  // popup when _G(mousey) < this
	int32_t Transparency;   // "incorrect" alpha (in legacy 255-range units)
	int32_t ZOrder;

	int32_t FocusCtrl;      // which control has the focus
	int32_t HighlightCtrl;  // which control has the bounding selection rect
	int32_t MouseOverCtrl;  // which control has the mouse cursor over it
	int32_t MouseDownCtrl;  // which control has the mouse button pressed on it
	Point   MouseWasAt;     // last mouse cursor position

	String  OnClickHandler; // script function name

	private:
	int32_t _flags;         // style and behavior flags
	bool    _hasChanged;    // flag tells whether GUI has graphically changed recently

	// Array of types and control indexes in global GUI object arrays;
	// maps GUI child slots to actual controls and used for rebuilding Controls array
	typedef std::pair<GUIControlType, int32_t> ControlRef;
	std::vector<ControlRef> _ctrlRefs;
	// Array of child control references (not exclusively owned!)
	std::vector<GUIObject *> _controls;
	// Sorted array of controls in z-order.
	std::vector<int32_t>    _ctrlDrawOrder;
};


namespace GUI {
extern GuiVersion GameGuiVersion;
extern GuiOptions Options;

// Draw standart "shading" effect over rectangle
void DrawDisabledEffect(Bitmap *ds, const Rect &rc);
// Draw text aligned inside rectangle
void DrawTextAligned(Bitmap *ds, const char *text, int font, color_t text_color, const Rect &frame, FrameAlignment align);
// Draw text aligned horizontally inside given bounds
void DrawTextAlignedHor(Bitmap *ds, const char *text, int font, color_t text_color, int x1, int x2, int y, FrameAlignment align);

// Mark all existing GUI for redraw
void MarkAllGUIForUpdate();
// Mark all GUI which use the given font for redraw
void MarkForFontUpdate(int font);
// Mark labels that acts as special text placeholders for redraw
void MarkSpecialLabelsForUpdate(GUILabelMacro macro);
// Mark inventory windows for redraw, optionally only ones linked to given character
void MarkInventoryForUpdate(int char_id, bool is_player);

// Parses the string and returns combination of label macro flags
GUILabelMacro FindLabelMacros(const String &text);

// Reads all GUIs and their controls.
// WARNING: the data is read into the global arrays (guis, guibuts, and so on)
// TODO: remove is_savegame param after dropping support for old saves
// because only they use ReadGUI to read runtime GUI data
HError ReadGUI(Stream *in, bool is_savegame = false);
// Writes all GUIs and their controls.
// WARNING: the data is written from the global arrays (guis, guibuts, and so on)
void WriteGUI(Stream *out);
// Converts legacy GUIVisibility into appropriate GUIMain properties
void ApplyLegacyVisibility(GUIMain &gui, LegacyGUIVisState vis);
}

} // namespace Shared
} // namespace AGS

extern int get_adjusted_spritewidth(int spr);
extern int get_adjusted_spriteheight(int spr);
extern bool is_sprite_alpha(int spr);

// Those function have distinct implementations in Engine and Editor
extern int get_text_width_outlined(Shared::Bitmap *ds, const char *tex, int font);

#define SET_EIP(x) set_our_eip(x);
extern void set_eip_guiobj(int eip);
extern int get_eip_guiobj();

} // namespace AGS3

#endif
