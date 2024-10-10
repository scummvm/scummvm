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

#include "ags/shared/ac/common.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/gui_control.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/shared/util/string_compat.h"

namespace AGS3 {

using namespace AGS::Shared;

int IsGUIOn(int guinum) {
	if ((guinum < 0) || (guinum >= _GP(game).numgui))
		quit("!IsGUIOn: invalid GUI number specified");
	return (_GP(guis)[guinum].IsDisplayed()) ? 1 : 0;
}

// This is an internal script function, and is undocumented.
// It is used by the editor's automatic macro generation.
int FindGUIID(const char *GUIName) {
	for (int ii = 0; ii < _GP(game).numgui; ii++) {
		if (_GP(guis)[ii].Name.IsEmpty())
			continue;
		if (_GP(guis)[ii].Name == GUIName)
			return ii;
		if ((_GP(guis)[ii].Name[0u] == 'g') && (ags_stricmp(_GP(guis)[ii].Name.GetCStr() + 1, GUIName) == 0))
			return ii;
	}
	quit("FindGUIID: No matching GUI found: GUI may have been deleted");
	return -1;
}

void InterfaceOn(int ifn) {
	if ((ifn < 0) | (ifn >= _GP(game).numgui))
		quit("!GUIOn: invalid GUI specified");

	EndSkippingUntilCharStops();

	if (_GP(guis)[ifn].IsVisible()) {
		return;
	}
	_GP(guis)[ifn].SetVisible(true);
	debug_script_log("GUI %d turned on", ifn);
	// modal interface
	if (_GP(guis)[ifn].PopupStyle == kGUIPopupModal) PauseGame();
	_GP(guis)[ifn].Poll(_G(mousex), _G(mousey));
}

void InterfaceOff(int ifn) {
	if ((ifn < 0) | (ifn >= _GP(game).numgui)) quit("!GUIOff: invalid GUI specified");
	if (!_GP(guis)[ifn].IsVisible()) {
		return;
	}
	debug_script_log("GUI %d turned off", ifn);
	_GP(guis)[ifn].SetVisible(false);
	// modal interface
	if (_GP(guis)[ifn].PopupStyle == kGUIPopupModal) UnPauseGame();
}

void SetGUIObjectEnabled(int guin, int objn, int enabled) {
	if ((guin < 0) || (guin >= _GP(game).numgui))
		quit("!SetGUIObjectEnabled: invalid GUI number");
	if ((objn < 0) || (objn >= _GP(guis)[guin].GetControlCount()))
		quit("!SetGUIObjectEnabled: invalid object number");

	GUIControl_SetEnabled(_GP(guis)[guin].GetControl(objn), enabled);
}

void SetGUIObjectPosition(int guin, int objn, int xx, int yy) {
	if ((guin < 0) || (guin >= _GP(game).numgui))
		quit("!SetGUIObjectPosition: invalid GUI number");
	if ((objn < 0) || (objn >= _GP(guis)[guin].GetControlCount()))
		quit("!SetGUIObjectPosition: invalid object number");

	GUIControl_SetPosition(_GP(guis)[guin].GetControl(objn), xx, yy);
}

void SetGUIPosition(int ifn, int xx, int yy) {
	if ((ifn < 0) || (ifn >= _GP(game).numgui))
		quit("!SetGUIPosition: invalid GUI number");

	GUI_SetPosition(&_GP(scrGui)[ifn], xx, yy);
}

void SetGUIObjectSize(int ifn, int objn, int newwid, int newhit) {
	if ((ifn < 0) || (ifn >= _GP(game).numgui))
		quit("!SetGUIObjectSize: invalid GUI number");

	if ((objn < 0) || (objn >= _GP(guis)[ifn].GetControlCount()))
		quit("!SetGUIObjectSize: invalid object number");

	GUIControl_SetSize(_GP(guis)[ifn].GetControl(objn), newwid, newhit);
}

void SetGUISize(int ifn, int widd, int hitt) {
	if ((ifn < 0) || (ifn >= _GP(game).numgui))
		quit("!SetGUISize: invalid GUI number");

	GUI_SetSize(&_GP(scrGui)[ifn], widd, hitt);
}

void SetGUIZOrder(int guin, int z) {
	if ((guin < 0) || (guin >= _GP(game).numgui))
		quit("!SetGUIZOrder: invalid GUI number");

	GUI_SetZOrder(&_GP(scrGui)[guin], z);
}

void SetGUIClickable(int guin, int clickable) {
	if ((guin < 0) || (guin >= _GP(game).numgui))
		quit("!SetGUIClickable: invalid GUI number");

	GUI_SetClickable(&_GP(scrGui)[guin], clickable);
}

// pass trans=0 for fully solid, trans=100 for fully transparent
void SetGUITransparency(int ifn, int trans) {
	if ((ifn < 0) | (ifn >= _GP(game).numgui))
		quit("!SetGUITransparency: invalid GUI number");

	GUI_SetTransparency(&_GP(scrGui)[ifn], trans);
}

void CentreGUI(int ifn) {
	if ((ifn < 0) | (ifn >= _GP(game).numgui))
		quit("!CentreGUI: invalid GUI number");

	GUI_Centre(&_GP(scrGui)[ifn]);
}

int GetTextWidth(const char *text, int fontnum) {
	VALIDATE_STRING(text);
	if ((fontnum < 0) || (fontnum >= _GP(game).numfonts))
		quit("!GetTextWidth: invalid font number.");

	return game_to_data_coord(get_text_width_outlined(text, fontnum));
}

int GetTextHeight(const char *text, int fontnum, int width) {
	VALIDATE_STRING(text);
	if ((fontnum < 0) || (fontnum >= _GP(game).numfonts))
		quit("!GetTextHeight: invalid font number.");

	if (break_up_text_into_lines(text, _GP(Lines), data_to_game_coord(width), fontnum) == 0)
		return 0;
	return game_to_data_coord(get_text_lines_height(fontnum, _GP(Lines).Count()));
}

int GetFontHeight(int fontnum) {
	if ((fontnum < 0) || (fontnum >= _GP(game).numfonts))
		quit("!GetFontHeight: invalid font number.");
	return game_to_data_coord(get_font_height_outlined(fontnum));
}

int GetFontLineSpacing(int fontnum) {
	if ((fontnum < 0) || (fontnum >= _GP(game).numfonts))
		quit("!GetFontLineSpacing: invalid font number.");
	return game_to_data_coord(get_font_linespacing(fontnum));
}

void SetGUIBackgroundPic(int guin, int slotn) {
	if ((guin < 0) | (guin >= _GP(game).numgui))
		quit("!SetGUIBackgroundPic: invalid GUI number");

	GUI_SetBackgroundGraphic(&_GP(scrGui)[guin], slotn);
}

void DisableInterface() {
	// If GUI looks change when disabled, then mark all of them for redraw
	bool redraw_gui = (_GP(play).disabled_user_interface == 0) && // only if was enabled before
					  (GUI::Options.DisabledStyle != kGuiDis_Unchanged);
	GUI::MarkAllGUIForUpdate(redraw_gui, true);
	_GP(play).disabled_user_interface++;
	set_mouse_cursor(CURS_WAIT);
}

void EnableInterface() {
	_GP(play).disabled_user_interface--;
	if (_GP(play).disabled_user_interface < 1) {
		_GP(play).disabled_user_interface = 0;
		set_default_cursor();
		// If GUI looks change when disabled, then mark all of them for redraw
		GUI::MarkAllGUIForUpdate(GUI::Options.DisabledStyle != kGuiDis_Unchanged, true);
	}
}

// Returns 1 if user interface is enabled, 0 if disabled
int IsInterfaceEnabled() {
	return (_GP(play).disabled_user_interface > 0) ? 0 : 1;
}

int GetGUIObjectAt(int xx, int yy) {
	GUIObject *toret = GetGUIControlAtLocation(xx, yy);
	if (toret == nullptr)
		return -1;

	return toret->Id;
}

int GetGUIAt(int xx, int yy) {
	data_to_game_coords(&xx, &yy);

	// Test in the opposite order (from closer to further)
	for (auto g = _GP(play).gui_draw_order.crbegin();
			g < _GP(play).gui_draw_order.crend(); ++g) {
		if (_GP(guis)[*g].IsInteractableAt(xx, yy))
			return *g;
	}
	return -1;
}

void SetTextWindowGUI(int guinum) {
	if ((guinum < -1) | (guinum >= _GP(game).numgui))
		quit("!SetTextWindowGUI: invalid GUI number");

	if (guinum < 0);  // disable it
	else if (!_GP(guis)[guinum].IsTextWindow())
		quit("!SetTextWindowGUI: specified GUI is not a text window");

	if (_GP(play).speech_textwindow_gui == _GP(game).options[OPT_TWCUSTOM])
		_GP(play).speech_textwindow_gui = guinum;
	_GP(game).options[OPT_TWCUSTOM] = guinum;
}

} // namespace AGS3
