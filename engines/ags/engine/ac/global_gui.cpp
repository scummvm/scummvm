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

#include "ac/common.h"
#include "ac/display.h"
#include "ac/draw.h"
#include "ac/gamesetupstruct.h"
#include "ac/global_game.h"
#include "ac/global_gui.h"
#include "ac/gui.h"
#include "ac/guicontrol.h"
#include "ac/mouse.h"
#include "ac/string.h"
#include "debug/debug_log.h"
#include "font/fonts.h"
#include "gui/guimain.h"
#include "script/runtimescriptvalue.h"
#include "util/string_compat.h"

using namespace AGS::Shared;

extern GameSetupStruct game;
extern ScriptGUI *scrGui;

int IsGUIOn(int guinum) {
	if ((guinum < 0) || (guinum >= game.numgui))
		quit("!IsGUIOn: invalid GUI number specified");
	return (guis[guinum].IsDisplayed()) ? 1 : 0;
}

// This is an internal script function, and is undocumented.
// It is used by the editor's automatic macro generation.
int FindGUIID(const char *GUIName) {
	for (int ii = 0; ii < game.numgui; ii++) {
		if (guis[ii].Name.IsEmpty())
			continue;
		if (strcmp(guis[ii].Name, GUIName) == 0)
			return ii;
		if ((guis[ii].Name[0u] == 'g') && (ags_stricmp(guis[ii].Name.GetCStr() + 1, GUIName) == 0))
			return ii;
	}
	quit("FindGUIID: No matching GUI found: GUI may have been deleted");
	return -1;
}

void InterfaceOn(int ifn) {
	if ((ifn < 0) | (ifn >= game.numgui))
		quit("!GUIOn: invalid GUI specified");

	EndSkippingUntilCharStops();

	if (guis[ifn].IsVisible()) {
		debug_script_log("GUIOn(%d) ignored (already on)", ifn);
		return;
	}
	guis_need_update = 1;
	guis[ifn].SetVisible(true);
	debug_script_log("GUI %d turned on", ifn);
	// modal interface
	if (guis[ifn].PopupStyle == kGUIPopupModal) PauseGame();
	// clear the cached mouse position
	guis[ifn].OnControlPositionChanged();
	guis[ifn].Poll();
}

void InterfaceOff(int ifn) {
	if ((ifn < 0) | (ifn >= game.numgui)) quit("!GUIOff: invalid GUI specified");
	if (!guis[ifn].IsVisible()) {
		debug_script_log("GUIOff(%d) ignored (already off)", ifn);
		return;
	}
	debug_script_log("GUI %d turned off", ifn);
	guis[ifn].SetVisible(false);
	if (guis[ifn].MouseOverCtrl >= 0) {
		// Make sure that the overpic is turned off when the GUI goes off
		guis[ifn].GetControl(guis[ifn].MouseOverCtrl)->OnMouseLeave();
		guis[ifn].MouseOverCtrl = -1;
	}
	guis[ifn].OnControlPositionChanged();
	guis_need_update = 1;
	// modal interface
	if (guis[ifn].PopupStyle == kGUIPopupModal) UnPauseGame();
}

void SetGUIObjectEnabled(int guin, int objn, int enabled) {
	if ((guin < 0) || (guin >= game.numgui))
		quit("!SetGUIObjectEnabled: invalid GUI number");
	if ((objn < 0) || (objn >= guis[guin].GetControlCount()))
		quit("!SetGUIObjectEnabled: invalid object number");

	GUIControl_SetEnabled(guis[guin].GetControl(objn), enabled);
}

void SetGUIObjectPosition(int guin, int objn, int xx, int yy) {
	if ((guin < 0) || (guin >= game.numgui))
		quit("!SetGUIObjectPosition: invalid GUI number");
	if ((objn < 0) || (objn >= guis[guin].GetControlCount()))
		quit("!SetGUIObjectPosition: invalid object number");

	GUIControl_SetPosition(guis[guin].GetControl(objn), xx, yy);
}

void SetGUIPosition(int ifn, int xx, int yy) {
	if ((ifn < 0) || (ifn >= game.numgui))
		quit("!SetGUIPosition: invalid GUI number");

	GUI_SetPosition(&scrGui[ifn], xx, yy);
}

void SetGUIObjectSize(int ifn, int objn, int newwid, int newhit) {
	if ((ifn < 0) || (ifn >= game.numgui))
		quit("!SetGUIObjectSize: invalid GUI number");

	if ((objn < 0) || (objn >= guis[ifn].GetControlCount()))
		quit("!SetGUIObjectSize: invalid object number");

	GUIControl_SetSize(guis[ifn].GetControl(objn), newwid, newhit);
}

void SetGUISize(int ifn, int widd, int hitt) {
	if ((ifn < 0) || (ifn >= game.numgui))
		quit("!SetGUISize: invalid GUI number");

	GUI_SetSize(&scrGui[ifn], widd, hitt);
}

void SetGUIZOrder(int guin, int z) {
	if ((guin < 0) || (guin >= game.numgui))
		quit("!SetGUIZOrder: invalid GUI number");

	GUI_SetZOrder(&scrGui[guin], z);
}

void SetGUIClickable(int guin, int clickable) {
	if ((guin < 0) || (guin >= game.numgui))
		quit("!SetGUIClickable: invalid GUI number");

	GUI_SetClickable(&scrGui[guin], clickable);
}

// pass trans=0 for fully solid, trans=100 for fully transparent
void SetGUITransparency(int ifn, int trans) {
	if ((ifn < 0) | (ifn >= game.numgui))
		quit("!SetGUITransparency: invalid GUI number");

	GUI_SetTransparency(&scrGui[ifn], trans);
}

void CentreGUI(int ifn) {
	if ((ifn < 0) | (ifn >= game.numgui))
		quit("!CentreGUI: invalid GUI number");

	GUI_Centre(&scrGui[ifn]);
}

int GetTextWidth(const char *text, int fontnum) {
	VALIDATE_STRING(text);
	if ((fontnum < 0) || (fontnum >= game.numfonts))
		quit("!GetTextWidth: invalid font number.");

	return game_to_data_coord(wgettextwidth_compensate(text, fontnum));
}

int GetTextHeight(const char *text, int fontnum, int width) {
	VALIDATE_STRING(text);
	if ((fontnum < 0) || (fontnum >= game.numfonts))
		quit("!GetTextHeight: invalid font number.");

	if (break_up_text_into_lines(text, Lines, data_to_game_coord(width), fontnum) == 0)
		return 0;
	return game_to_data_coord(getheightoflines(fontnum, Lines.Count()));
}

int GetFontHeight(int fontnum) {
	if ((fontnum < 0) || (fontnum >= game.numfonts))
		quit("!GetFontHeight: invalid font number.");
	return game_to_data_coord(getfontheight_outlined(fontnum));
}

int GetFontLineSpacing(int fontnum) {
	if ((fontnum < 0) || (fontnum >= game.numfonts))
		quit("!GetFontLineSpacing: invalid font number.");
	return game_to_data_coord(getfontspacing_outlined(fontnum));
}

void SetGUIBackgroundPic(int guin, int slotn) {
	if ((guin < 0) | (guin >= game.numgui))
		quit("!SetGUIBackgroundPic: invalid GUI number");

	GUI_SetBackgroundGraphic(&scrGui[guin], slotn);
}

void DisableInterface() {
	play.disabled_user_interface++;
	guis_need_update = 1;
	set_mouse_cursor(CURS_WAIT);
}

void EnableInterface() {
	guis_need_update = 1;
	play.disabled_user_interface--;
	if (play.disabled_user_interface < 1) {
		play.disabled_user_interface = 0;
		set_default_cursor();
	}
}
// Returns 1 if user interface is enabled, 0 if disabled
int IsInterfaceEnabled() {
	return (play.disabled_user_interface > 0) ? 0 : 1;
}

int GetGUIObjectAt(int xx, int yy) {
	GUIObject *toret = GetGUIControlAtLocation(xx, yy);
	if (toret == nullptr)
		return -1;

	return toret->Id;
}

int GetGUIAt(int xx, int yy) {
	data_to_game_coords(&xx, &yy);

	int aa, ll;
	for (ll = game.numgui - 1; ll >= 0; ll--) {
		aa = play.gui_draw_order[ll];
		if (guis[aa].IsInteractableAt(xx, yy))
			return aa;
	}
	return -1;
}

void SetTextWindowGUI(int guinum) {
	if ((guinum < -1) | (guinum >= game.numgui))
		quit("!SetTextWindowGUI: invalid GUI number");

	if (guinum < 0) ;  // disable it
	else if (!guis[guinum].IsTextWindow())
		quit("!SetTextWindowGUI: specified GUI is not a text window");

	if (play.speech_textwindow_gui == game.options[OPT_TWCUSTOM])
		play.speech_textwindow_gui = guinum;
	game.options[OPT_TWCUSTOM] = guinum;
}
