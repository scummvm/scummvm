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

#include "common/std/algorithm.h"
#include "ags/engine/ac/gui.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_inventory_item.h"
#include "ags/engine/ac/global_screen.h"
#include "ags/engine/ac/gui_control.h"
#include "ags/shared/ac/interface_button.h"
#include "ags/engine/ac/inv_window.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/dynobj/cc_gui_object.h"
#include "ags/engine/ac/dynobj/script_gui.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/script/cc_instance.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/ac/dynobj/cc_gui.h"
#include "ags/engine/ac/dynobj/cc_gui_object.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/shared/util/string_compat.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

ScriptGUI *GUI_AsTextWindow(ScriptGUI *tehgui) { // Internally both GUI and TextWindow are implemented by same class
	return _GP(guis)[tehgui->id].IsTextWindow() ? &_GP(scrGui)[tehgui->id] : nullptr;
}

int GUI_GetPopupStyle(ScriptGUI *tehgui) {
	return _GP(guis)[tehgui->id].PopupStyle;
}

void GUI_SetVisible(ScriptGUI *tehgui, int isvisible) {
	if (isvisible)
		InterfaceOn(tehgui->id);
	else
		InterfaceOff(tehgui->id);
}

int GUI_GetVisible(ScriptGUI *tehgui) {
	// Since 3.5.0 this always returns honest state of the Visible property as set by the game
	if (_G(loaded_game_file_version) >= kGameVersion_350)
		return (_GP(guis)[tehgui->id].IsVisible()) ? 1 : 0;
	// Prior to 3.5.0 PopupY guis override Visible property and set it to 0 when auto-hidden;
	// in order to simulate same behavior we only return positive if such gui is popped up:
	return (_GP(guis)[tehgui->id].IsDisplayed()) ? 1 : 0;
}

bool GUI_GetShown(ScriptGUI *tehgui) {
	return _GP(guis)[tehgui->id].IsDisplayed();
}

int GUI_GetX(ScriptGUI *tehgui) {
	return game_to_data_coord(_GP(guis)[tehgui->id].X);
}

void GUI_SetX(ScriptGUI *tehgui, int xx) {
	_GP(guis)[tehgui->id].X = data_to_game_coord(xx);
}

int GUI_GetY(ScriptGUI *tehgui) {
	return game_to_data_coord(_GP(guis)[tehgui->id].Y);
}

void GUI_SetY(ScriptGUI *tehgui, int yy) {
	_GP(guis)[tehgui->id].Y = data_to_game_coord(yy);
}

void GUI_SetPosition(ScriptGUI *tehgui, int xx, int yy) {
	GUI_SetX(tehgui, xx);
	GUI_SetY(tehgui, yy);
}

void GUI_SetSize(ScriptGUI *sgui, int widd, int hitt) {
	if ((widd < 1) || (hitt < 1))
		quitprintf("!SetGUISize: invalid dimensions (tried to set to %d x %d)", widd, hitt);

	GUIMain *tehgui = &_GP(guis)[sgui->id];
	data_to_game_coords(&widd, &hitt);

	if ((tehgui->Width == widd) && (tehgui->Height == hitt))
		return;

	tehgui->Width = widd;
	tehgui->Height = hitt;

	tehgui->MarkChanged();
}

int GUI_GetWidth(ScriptGUI *sgui) {
	return game_to_data_coord(_GP(guis)[sgui->id].Width);
}

int GUI_GetHeight(ScriptGUI *sgui) {
	return game_to_data_coord(_GP(guis)[sgui->id].Height);
}

void GUI_SetWidth(ScriptGUI *sgui, int newwid) {
	GUI_SetSize(sgui, newwid, GUI_GetHeight(sgui));
}

void GUI_SetHeight(ScriptGUI *sgui, int newhit) {
	GUI_SetSize(sgui, GUI_GetWidth(sgui), newhit);
}

void GUI_SetZOrder(ScriptGUI *tehgui, int z) {
	_GP(guis)[tehgui->id].ZOrder = z;
	update_gui_zorder();
}

int GUI_GetZOrder(ScriptGUI *tehgui) {
	return _GP(guis)[tehgui->id].ZOrder;
}

void GUI_SetClickable(ScriptGUI *tehgui, int clickable) {
	_GP(guis)[tehgui->id].SetClickable(clickable != 0);
}

int GUI_GetClickable(ScriptGUI *tehgui) {
	return _GP(guis)[tehgui->id].IsClickable() ? 1 : 0;
}

int GUI_GetID(ScriptGUI *tehgui) {
	return tehgui->id;
}

const char *GUI_GetScriptName(ScriptGUI *tehgui) {
	return CreateNewScriptString(_GP(guis)[tehgui->id].Name);
}

GUIObject *GUI_GetiControls(ScriptGUI *tehgui, int idx) {
	if ((idx < 0) || (idx >= _GP(guis)[tehgui->id].GetControlCount()))
		return nullptr;
	return _GP(guis)[tehgui->id].GetControl(idx);
}

int GUI_GetControlCount(ScriptGUI *tehgui) {
	return _GP(guis)[tehgui->id].GetControlCount();
}

int GUI_GetPopupYPos(ScriptGUI *tehgui) {
	return _GP(guis)[tehgui->id].PopupAtMouseY;
}

void GUI_SetPopupYPos(ScriptGUI *tehgui, int newpos) {
	if (!_GP(guis)[tehgui->id].IsTextWindow())
		_GP(guis)[tehgui->id].PopupAtMouseY = newpos;
}

void GUI_SetTransparency(ScriptGUI *tehgui, int trans) {
	if ((trans < 0) | (trans > 100))
		quit("!SetGUITransparency: transparency value must be between 0 and 100");

	_GP(guis)[tehgui->id].SetTransparencyAsPercentage(trans);
}

int GUI_GetTransparency(ScriptGUI *tehgui) {
	return GfxDef::LegacyTrans255ToTrans100(_GP(guis)[tehgui->id].Transparency);
}

void GUI_Centre(ScriptGUI *sgui) {
	GUIMain *tehgui = &_GP(guis)[sgui->id];
	tehgui->X = _GP(play).GetUIViewport().GetWidth() / 2 - tehgui->Width / 2;
	tehgui->Y = _GP(play).GetUIViewport().GetHeight() / 2 - tehgui->Height / 2;
}

void GUI_SetBackgroundGraphic(ScriptGUI *tehgui, int slotn) {
	if (_GP(guis)[tehgui->id].BgImage != slotn) {
		_GP(guis)[tehgui->id].BgImage = slotn;
		_GP(guis)[tehgui->id].MarkChanged();
	}
}

int GUI_GetBackgroundGraphic(ScriptGUI *tehgui) {
	if (_GP(guis)[tehgui->id].BgImage < 1)
		return 0;
	return _GP(guis)[tehgui->id].BgImage;
}

void GUI_SetBackgroundColor(ScriptGUI *tehgui, int newcol) {
	if (_GP(guis)[tehgui->id].BgColor != newcol) {
		_GP(guis)[tehgui->id].BgColor = newcol;
		_GP(guis)[tehgui->id].MarkChanged();
	}
}

int GUI_GetBackgroundColor(ScriptGUI *tehgui) {
	return _GP(guis)[tehgui->id].BgColor;
}

void GUI_SetBorderColor(ScriptGUI *tehgui, int newcol) {
	if (_GP(guis)[tehgui->id].IsTextWindow())
		return;
	if (_GP(guis)[tehgui->id].FgColor != newcol) {
		_GP(guis)[tehgui->id].FgColor = newcol;
		_GP(guis)[tehgui->id].MarkChanged();
	}
}

int GUI_GetBorderColor(ScriptGUI *tehgui) {
	if (_GP(guis)[tehgui->id].IsTextWindow())
		return 0;
	return _GP(guis)[tehgui->id].FgColor;
}

void GUI_SetTextColor(ScriptGUI *tehgui, int newcol) {
	if (!_GP(guis)[tehgui->id].IsTextWindow())
		return;
	if (_GP(guis)[tehgui->id].FgColor != newcol) {
		_GP(guis)[tehgui->id].FgColor = newcol;
		_GP(guis)[tehgui->id].MarkChanged();
	}
}

int GUI_GetTextColor(ScriptGUI *tehgui) {
	if (!_GP(guis)[tehgui->id].IsTextWindow())
		return 0;
	return _GP(guis)[tehgui->id].FgColor;
}

int GUI_GetTextPadding(ScriptGUI *tehgui) {
	return _GP(guis)[tehgui->id].Padding;
}

void GUI_SetTextPadding(ScriptGUI *tehgui, int newpos) {
	if (_GP(guis)[tehgui->id].IsTextWindow())
		_GP(guis)[tehgui->id].Padding = newpos;
}

ScriptGUI *GetGUIAtLocation(int xx, int yy) {
	int guiid = GetGUIAt(xx, yy);
	if (guiid < 0)
		return nullptr;
	return &_GP(scrGui)[guiid];
}

void GUI_Click(ScriptGUI *scgui, int mbut) {
	process_interface_click(scgui->id, -1, mbut);
}

void GUI_ProcessClick(int x, int y, int mbut) {
	int guiid = gui_get_interactable(x, y);
	if (guiid >= 0) { // simulate mouse click at the given coordinates
		_GP(guis)[guiid].Poll(x, y);
		gui_on_mouse_down(guiid, mbut, x, y);
		gui_on_mouse_up(guiid, mbut, x, y);
	}
}

//=============================================================================

void remove_popup_interface(int ifacenum) {
	if (_G(ifacepopped) != ifacenum) return;
	_G(ifacepopped) = -1;
	UnPauseGame();
	_GP(guis)[ifacenum].SetConceal(true);
	if (_G(mousey) <= _GP(guis)[ifacenum].PopupAtMouseY)
		_GP(mouse).SetPosition(Point(_G(mousex), _GP(guis)[ifacenum].PopupAtMouseY + 2));
	if ((!IsInterfaceEnabled()) && (_G(cur_cursor) == _G(cur_mode)))
		// Only change the mouse cursor if it hasn't been specifically changed first
		set_mouse_cursor(CURS_WAIT);
	else if (IsInterfaceEnabled())
		set_default_cursor();

	if (ifacenum == _G(mouse_on_iface)) _G(mouse_on_iface) = -1;
}

void process_interface_click(int ifce, int btn, int mbut) {
	if (btn < 0) {
		// click on GUI background
		RuntimeScriptValue params[]{ RuntimeScriptValue().SetScriptObject(&_GP(scrGui)[ifce], &_GP(ccDynamicGUI)),
					RuntimeScriptValue().SetInt32(mbut) };
		QueueScriptFunction(kScInstGame, _GP(guis)[ifce].OnClickHandler.GetCStr(), 2, params);
		return;
	}

	int btype = _GP(guis)[ifce].GetControlType(btn);
	int rtype = kGUIAction_None, rdata = 0;
	if (btype == kGUIButton) {
		GUIButton *gbuto = (GUIButton *)_GP(guis)[ifce].GetControl(btn);
		rtype = gbuto->ClickAction[kGUIClickLeft];
		rdata = gbuto->ClickData[kGUIClickLeft];
	} else if ((btype == kGUISlider) || (btype == kGUITextBox) || (btype == kGUIListBox))
		rtype = kGUIAction_RunScript;
	else quit("unknown GUI object triggered process_interface");

	if (rtype == kGUIAction_None);
	else if (rtype == kGUIAction_SetMode)
		set_cursor_mode(rdata);
	else if (rtype == kGUIAction_RunScript) {
		GUIObject *theObj = _GP(guis)[ifce].GetControl(btn);
		// if the object has a special handler script then run it;
		// otherwise, run interface_click
		if ((theObj->GetEventCount() > 0) &&
			(!theObj->EventHandlers[0].IsEmpty()) &&
			(!_G(gameinst)->GetSymbolAddress(theObj->EventHandlers[0].GetCStr()).IsNull())) {
			// control-specific event handler
			if (theObj->GetEventArgs(0).FindChar(',') != String::NoIndex) {
				RuntimeScriptValue params[]{ RuntimeScriptValue().SetScriptObject(theObj, &_GP(ccDynamicGUIObject)),
					RuntimeScriptValue().SetInt32(mbut) };
				QueueScriptFunction(kScInstGame, theObj->EventHandlers[0].GetCStr(), 2, params);
			} else {
				RuntimeScriptValue params[]{ RuntimeScriptValue().SetScriptObject(theObj, &_GP(ccDynamicGUIObject)) };
				QueueScriptFunction(kScInstGame, theObj->EventHandlers[0].GetCStr(), 1, params);
			}
		} else {
			RuntimeScriptValue params[]{ ifce , btn };
			QueueScriptFunction(kScInstGame, "interface_click", 2, params);
		}
	}
}

void replace_macro_tokens(const char *text, String &fixed_text) {
	const char *curptr = &text[0];
	char tmpm[3];
	const char *endat = curptr + strlen(text);
	fixed_text.Empty();
	char tempo[STD_BUFFER_SIZE];

	while (1) {
		if (curptr[0] == 0) break;
		if (curptr >= endat) break;
		if (curptr[0] == '@') {
			const char *curptrWasAt = curptr;
			char macroname[21];
			int idd = 0;
			curptr++;
			for (idd = 0; idd < 20; idd++) {
				if (curptr[0] == '@') {
					macroname[idd] = 0;
					curptr++;
					break;
				}
				// unterminated macro (eg. "@SCORETEXT"), so abort
				if (curptr[0] == 0)
					break;
				macroname[idd] = curptr[0];
				curptr++;
			}
			macroname[idd] = 0;
			tempo[0] = 0;
			if (ags_stricmp(macroname, "score") == 0)
				snprintf(tempo, sizeof(tempo), "%d", _GP(play).score);
			else if (ags_stricmp(macroname, "totalscore") == 0)
				snprintf(tempo, sizeof(tempo), "%d", MAXSCORE);
			else if (ags_stricmp(macroname, "scoretext") == 0)
				snprintf(tempo, sizeof(tempo), "%d of %d", _GP(play).score, MAXSCORE);
			else if (ags_stricmp(macroname, "gamename") == 0)
				snprintf(tempo, sizeof(tempo), "%s", _GP(play).game_name.GetCStr());
			else if (ags_stricmp(macroname, "overhotspot") == 0) {
				// While game is in Wait mode, no overhotspot text
				if (!IsInterfaceEnabled())
					tempo[0] = 0;
				else
					GetLocationName(game_to_data_coord(_G(mousex)), game_to_data_coord(_G(mousey)), tempo);
			} else { // not a macro, there's just a @ in the message
				curptr = curptrWasAt + 1;
				snprintf(tempo, sizeof(tempo), "%s", "@");
			}

			fixed_text.Append(tempo);
		} else {
			tmpm[0] = curptr[0];
			tmpm[1] = 0;
			fixed_text.Append(tmpm);
			curptr++;
		}
	}
}

bool sort_gui_less(const int g1, const int g2) {
	return (_GP(guis)[g1].ZOrder < _GP(guis)[g2].ZOrder) ||
		   ((_GP(guis)[g1].ZOrder == _GP(guis)[g2].ZOrder) && (g1 < g2));
}

void update_gui_zorder() {
	std::sort(_GP(play).gui_draw_order.begin(), _GP(play).gui_draw_order.end(), sort_gui_less);
}

void export_gui_controls(int ee) {
	for (int ff = 0; ff < _GP(guis)[ee].GetControlCount(); ff++) {
		GUIObject *guio = _GP(guis)[ee].GetControl(ff);
		if (!guio->Name.IsEmpty())
			ccAddExternalScriptObject(guio->Name, guio, &_GP(ccDynamicGUIObject));
		ccRegisterManagedObject(guio, &_GP(ccDynamicGUIObject));
	}
}

void unexport_gui_controls(int ee) {
	for (int ff = 0; ff < _GP(guis)[ee].GetControlCount(); ff++) {
		GUIObject *guio = _GP(guis)[ee].GetControl(ff);
		if (!guio->Name.IsEmpty())
			ccRemoveExternalSymbol(guio->Name);
		if (!ccUnRegisterManagedObject(guio))
			quit("unable to unregister guicontrol object");
	}
}

void update_gui_disabled_status() {
	// update GUI display status (perhaps we've gone into an interface disabled state)
	const GuiDisableStyle all_buttons_was = _G(all_buttons_disabled);
	_G(all_buttons_disabled) = IsInterfaceEnabled() ? kGuiDis_Undefined : GUI::Options.DisabledStyle;

	if (all_buttons_was != _G(all_buttons_disabled)) {
		// Mark guis for redraw and reset control-under-mouse detection
		GUI::MarkAllGUIForUpdate(GUI::Options.DisabledStyle != kGuiDis_Unchanged, true);
		if (GUI::Options.DisabledStyle != kGuiDis_Unchanged) {
			invalidate_screen();
		}
	}
}

static bool should_skip_adjust_for_gui(const GUIMain &gui) {
	return
		// not shown
		!gui.IsDisplayed() ||
		// completely offscreen
		!IsRectInsideRect(_GP(play).GetUIViewport(), RectWH(gui.X, gui.Y, gui.Width, gui.Height)) ||
		// fully transparent (? FIXME: this only checks background, but not controls)
		((gui.BgColor == 0) && (gui.BgImage < 1)) || (gui.Transparency == 255);
}

int adjust_x_for_guis(int x, int y, bool assume_blocking) {
	if ((_GP(game).options[OPT_DISABLEOFF] == kGuiDis_Off) && (_G(all_buttons_disabled) >= 0 || assume_blocking))
		return x;  // All GUI off (or will be when the message is displayed)
	// If it's covered by a GUI, move it right a bit
	for (const auto &gui : _GP(guis)) {
		if (should_skip_adjust_for_gui(gui))
			continue;
		// higher, lower or to the right from the message (?)
		if ((gui.X > x) || (gui.Y > y) || (gui.Y + gui.Height < y))
			continue;
		// try to deal with full-width GUIs across the top
		// FIXME: using a harcoded width in pixels...
		if (gui.X + gui.Width >= get_fixed_pixel_size(280))
			continue;
		// Fix coordinates if x is inside the gui
		if (x < gui.X + gui.Width)
			x = gui.X + gui.Width + 2;
	}
	return x;
}

int adjust_y_for_guis(int y, bool assume_blocking) {
	if ((_GP(game).options[OPT_DISABLEOFF] == kGuiDis_Off) && (_G(all_buttons_disabled) >= 0 || assume_blocking))
		return y;  // All GUI off (or will be when the message is displayed)
	// If it's covered by a GUI, move it down a bit
	for (const auto &gui : _GP(guis)) {
		if (should_skip_adjust_for_gui(gui))
			continue;
		// lower than the message
		if (gui.Y > y)
			continue;
		// try to deal with full-height GUIs down the left or right
		// FIXME: using a harcoded height in pixels...
		if (gui.Height > get_fixed_pixel_size(50))
			continue;
		// Fix coordinates if y is inside the gui
		if (y < gui.Y + gui.Height)
			y = gui.Y + gui.Height + 2;
	}
	return y;
}

int gui_get_interactable(int x, int y) {
	if ((_GP(game).options[OPT_DISABLEOFF] == kGuiDis_Off) && (_G(all_buttons_disabled) >= 0))
		return -1;
	return GetGUIAt(x, y);
}

int gui_on_mouse_move(const int mx, const int my) {
	int mouse_over_gui = -1;
	// If all GUIs are off, skip the loop
	if ((_GP(game).options[OPT_DISABLEOFF] == kGuiDis_Off) && (_G(all_buttons_disabled) >= 0));
	else {
		// Scan for mouse-y-pos GUIs, and pop one up if appropriate
		// Also work out the mouse-over GUI while we're at it
		// CHECKME: not sure why, but we're testing forward draw order here -
		// from farthest to nearest (this was in original code?)
		for (int guin : _GP(play).gui_draw_order) {
			if (_GP(guis)[guin].IsInteractableAt(mx, my)) mouse_over_gui = guin;

			if (_GP(guis)[guin].PopupStyle != kGUIPopupMouseY) continue;
			if (_GP(play).complete_overlay_on > 0) break;  // interfaces disabled			//    if (_GP(play).disabled_user_interface>0) break;
			if (_G(ifacepopped) == guin) continue;
			if (!_GP(guis)[guin].IsVisible()) continue;
			// Don't allow it to be popped up while skipping cutscene
			if (_GP(play).fast_forward) continue;

			if (_G(mousey) < _GP(guis)[guin].PopupAtMouseY) {
				set_mouse_cursor(CURS_ARROW);
				_GP(guis)[guin].SetConceal(false);
				_G(ifacepopped) = guin;
				PauseGame();
				break;
			}
		}
	}
	return mouse_over_gui;
}

void gui_on_mouse_hold(const int wasongui, const int wasbutdown) {
	for (int i = 0; i < _GP(guis)[wasongui].GetControlCount(); i++) {
		GUIObject *guio = _GP(guis)[wasongui].GetControl(i);
		if (!guio->IsActivated) continue;
		if (_GP(guis)[wasongui].GetControlType(i) != kGUISlider) continue;
		// GUI Slider repeatedly activates while being dragged
		guio->IsActivated = false;
		force_event(EV_IFACECLICK, wasongui, i, wasbutdown);
		break;
	}
}

void gui_on_mouse_up(const int wasongui, const int wasbutdown, const int mx, const int my) {
	_GP(guis)[wasongui].OnMouseButtonUp();

	for (int i = 0; i < _GP(guis)[wasongui].GetControlCount(); i++) {
		GUIObject *guio = _GP(guis)[wasongui].GetControl(i);
		if (!guio->IsActivated) continue;
		guio->IsActivated = false;
		if (!IsInterfaceEnabled()) break;

		int cttype = _GP(guis)[wasongui].GetControlType(i);
		if ((cttype == kGUIButton) || (cttype == kGUISlider) || (cttype == kGUIListBox)) {
			force_event(EV_IFACECLICK, wasongui, i, wasbutdown);
		} else if (cttype == kGUIInvWindow) {
			_G(mouse_ifacebut_xoffs) = mx - (guio->X) - _GP(guis)[wasongui].X;
			_G(mouse_ifacebut_yoffs) = my - (guio->Y) - _GP(guis)[wasongui].Y;
			int iit = offset_over_inv((GUIInvWindow *)guio);
			if (iit >= 0) {
				_GP(play).used_inv_on = iit;
				if (_GP(game).options[OPT_HANDLEINVCLICKS]) {
					// Let the script handle the click
					// LEFTINV is 5, RIGHTINV is 6
					force_event(EV_TEXTSCRIPT, kTS_MouseClick, wasbutdown + 4);
				} else if (wasbutdown == kMouseRight)  // right-click is always Look
					RunInventoryInteraction(iit, MODE_LOOK);
				else if (_G(cur_mode) == MODE_HAND)
					SetActiveInventory(iit);
				else
					RunInventoryInteraction(iit, _G(cur_mode));
			}
		} else quit("clicked on unknown control type");
		if (_GP(guis)[wasongui].PopupStyle == kGUIPopupMouseY)
			remove_popup_interface(wasongui);
		break;
	}

	run_on_event(GE_GUI_MOUSEUP, RuntimeScriptValue().SetInt32(wasongui));
}

void gui_on_mouse_down(const int guin, const int mbut, const int mx, const int my) {
	debug_script_log("Mouse click over GUI %d", guin);
	_GP(guis)[guin].OnMouseButtonDown(mx, my);
	// run GUI click handler if not on any control
	if ((_GP(guis)[guin].MouseDownCtrl < 0) && (!_GP(guis)[guin].OnClickHandler.IsEmpty()))
		force_event(EV_IFACECLICK, guin, -1, mbut);

	run_on_event(GE_GUI_MOUSEDOWN, RuntimeScriptValue().SetInt32(guin));
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

ScriptGUI *GUI_GetByName(const char *name) {
	return static_cast<ScriptGUI *>(ccGetScriptObjectAddress(name, _GP(ccDynamicGUI).GetType()));
}

RuntimeScriptValue Sc_GUI_GetByName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_POBJ(ScriptGUI, _GP(ccDynamicGUI), GUI_GetByName, const char);
}

// void GUI_Centre(ScriptGUI *sgui)
RuntimeScriptValue Sc_GUI_Centre(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptGUI, GUI_Centre);
}

// ScriptGUI *(int xx, int yy)
RuntimeScriptValue Sc_GetGUIAtLocation(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(ScriptGUI, _GP(ccDynamicGUI), GetGUIAtLocation);
}

// void (ScriptGUI *tehgui, int xx, int yy)
RuntimeScriptValue Sc_GUI_SetPosition(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(ScriptGUI, GUI_SetPosition);
}

// void (ScriptGUI *sgui, int widd, int hitt)
RuntimeScriptValue Sc_GUI_SetSize(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(ScriptGUI, GUI_SetSize);
}

// int (ScriptGUI *tehgui)
RuntimeScriptValue Sc_GUI_GetBackgroundGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetBackgroundGraphic);
}

// void (ScriptGUI *tehgui, int slotn)
RuntimeScriptValue Sc_GUI_SetBackgroundGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetBackgroundGraphic);
}

RuntimeScriptValue Sc_GUI_GetBackgroundColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetBackgroundColor);
}

RuntimeScriptValue Sc_GUI_SetBackgroundColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetBackgroundColor);
}

RuntimeScriptValue Sc_GUI_GetBorderColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetBorderColor);
}

RuntimeScriptValue Sc_GUI_SetBorderColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetBorderColor);
}

RuntimeScriptValue Sc_GUI_GetTextColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetTextColor);
}

RuntimeScriptValue Sc_GUI_SetTextColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetTextColor);
}

// int (ScriptGUI *tehgui)
RuntimeScriptValue Sc_GUI_GetClickable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetClickable);
}

// void (ScriptGUI *tehgui, int clickable)
RuntimeScriptValue Sc_GUI_SetClickable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetClickable);
}

// int (ScriptGUI *tehgui)
RuntimeScriptValue Sc_GUI_GetControlCount(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetControlCount);
}

// GUIObject* (ScriptGUI *tehgui, int idx)
RuntimeScriptValue Sc_GUI_GetiControls(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT(ScriptGUI, GUIObject, _GP(ccDynamicGUIObject), GUI_GetiControls);
}

// int (ScriptGUI *sgui)
RuntimeScriptValue Sc_GUI_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetHeight);
}

// void (ScriptGUI *sgui, int newhit)
RuntimeScriptValue Sc_GUI_SetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetHeight);
}

// int (ScriptGUI *tehgui)
RuntimeScriptValue Sc_GUI_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetID);
}

RuntimeScriptValue Sc_GUI_GetScriptName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptGUI, const char, _GP(myScriptStringImpl), GUI_GetScriptName);
}

RuntimeScriptValue Sc_GUI_GetPopupYPos(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetPopupYPos);
}

RuntimeScriptValue Sc_GUI_SetPopupYPos(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetPopupYPos);
}

RuntimeScriptValue Sc_GUI_GetTextPadding(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetTextPadding);
}

RuntimeScriptValue Sc_GUI_SetTextPadding(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetTextPadding);
}

// int (ScriptGUI *tehgui)
RuntimeScriptValue Sc_GUI_GetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetTransparency);
}

// void (ScriptGUI *tehgui, int trans)
RuntimeScriptValue Sc_GUI_SetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetTransparency);
}

// int (ScriptGUI *tehgui)
RuntimeScriptValue Sc_GUI_GetVisible(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetVisible);
}

// void (ScriptGUI *tehgui, int isvisible)
RuntimeScriptValue Sc_GUI_SetVisible(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetVisible);
}

// int (ScriptGUI *sgui)
RuntimeScriptValue Sc_GUI_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetWidth);
}

// void (ScriptGUI *sgui, int newwid)
RuntimeScriptValue Sc_GUI_SetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetWidth);
}

// int (ScriptGUI *tehgui)
RuntimeScriptValue Sc_GUI_GetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetX);
}

// void (ScriptGUI *tehgui, int xx)
RuntimeScriptValue Sc_GUI_SetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetX);
}

// int (ScriptGUI *tehgui)
RuntimeScriptValue Sc_GUI_GetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetY);
}

// void (ScriptGUI *tehgui, int yy)
RuntimeScriptValue Sc_GUI_SetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetY);
}

// int (ScriptGUI *tehgui)
RuntimeScriptValue Sc_GUI_GetZOrder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetZOrder);
}

// void (ScriptGUI *tehgui, int z)
RuntimeScriptValue Sc_GUI_SetZOrder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_SetZOrder);
}

RuntimeScriptValue Sc_GUI_AsTextWindow(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptGUI, ScriptGUI, _GP(ccDynamicGUI), GUI_AsTextWindow);
}

RuntimeScriptValue Sc_GUI_GetPopupStyle(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptGUI, GUI_GetPopupStyle);
}

RuntimeScriptValue Sc_GUI_Click(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptGUI, GUI_Click);
}

RuntimeScriptValue Sc_GUI_ProcessClick(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(GUI_ProcessClick);
}

RuntimeScriptValue Sc_GUI_GetShown(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(ScriptGUI, GUI_GetShown);
}

void RegisterGUIAPI() {
	ScFnRegister gui_api[] = {
		{"GUI::GetAtScreenXY^2", API_FN_PAIR(GetGUIAtLocation)},
		{"GUI::GetByName", API_FN_PAIR(GUI_GetByName)},
		{"GUI::ProcessClick^3", API_FN_PAIR(GUI_ProcessClick)},

		{"GUI::Centre^0", API_FN_PAIR(GUI_Centre)},
		{"GUI::Click^1", API_FN_PAIR(GUI_Click)},
		{"GUI::SetPosition^2", API_FN_PAIR(GUI_SetPosition)},
		{"GUI::SetSize^2", API_FN_PAIR(GUI_SetSize)},
		{"GUI::get_BackgroundGraphic", API_FN_PAIR(GUI_GetBackgroundGraphic)},
		{"GUI::set_BackgroundGraphic", API_FN_PAIR(GUI_SetBackgroundGraphic)},
		{"GUI::get_BackgroundColor", API_FN_PAIR(GUI_GetBackgroundColor)},
		{"GUI::set_BackgroundColor", API_FN_PAIR(GUI_SetBackgroundColor)},
		{"GUI::get_BorderColor", API_FN_PAIR(GUI_GetBorderColor)},
		{"GUI::set_BorderColor", API_FN_PAIR(GUI_SetBorderColor)},
		{"GUI::get_Clickable", API_FN_PAIR(GUI_GetClickable)},
		{"GUI::set_Clickable", API_FN_PAIR(GUI_SetClickable)},
		{"GUI::get_ControlCount", API_FN_PAIR(GUI_GetControlCount)},
		{"GUI::geti_Controls", API_FN_PAIR(GUI_GetiControls)},
		{"GUI::get_Height", API_FN_PAIR(GUI_GetHeight)},
		{"GUI::set_Height", API_FN_PAIR(GUI_SetHeight)},
		{"GUI::get_ID", API_FN_PAIR(GUI_GetID)},
		{"GUI::get_AsTextWindow", API_FN_PAIR(GUI_AsTextWindow)},
		{"GUI::get_PopupStyle", API_FN_PAIR(GUI_GetPopupStyle)},
		{"GUI::get_PopupYPos", API_FN_PAIR(GUI_GetPopupYPos)},
		{"GUI::set_PopupYPos", API_FN_PAIR(GUI_SetPopupYPos)},
		{"GUI::get_ScriptName", API_FN_PAIR(GUI_GetScriptName)},
		{"TextWindowGUI::get_TextColor", API_FN_PAIR(GUI_GetTextColor)},
		{"TextWindowGUI::set_TextColor", API_FN_PAIR(GUI_SetTextColor)},
		{"TextWindowGUI::get_TextPadding", API_FN_PAIR(GUI_GetTextPadding)},
		{"TextWindowGUI::set_TextPadding", API_FN_PAIR(GUI_SetTextPadding)},
		{"GUI::get_Transparency", API_FN_PAIR(GUI_GetTransparency)},
		{"GUI::set_Transparency", API_FN_PAIR(GUI_SetTransparency)},
		{"GUI::get_Visible", API_FN_PAIR(GUI_GetVisible)},
		{"GUI::set_Visible", API_FN_PAIR(GUI_SetVisible)},
		{"GUI::get_Width", API_FN_PAIR(GUI_GetWidth)},
		{"GUI::set_Width", API_FN_PAIR(GUI_SetWidth)},
		{"GUI::get_X", API_FN_PAIR(GUI_GetX)},
		{"GUI::set_X", API_FN_PAIR(GUI_SetX)},
		{"GUI::get_Y", API_FN_PAIR(GUI_GetY)},
		{"GUI::set_Y", API_FN_PAIR(GUI_SetY)},
		{"GUI::get_ZOrder", API_FN_PAIR(GUI_GetZOrder)},
		{"GUI::set_ZOrder", API_FN_PAIR(GUI_SetZOrder)},
		{"GUI::get_Shown", API_FN_PAIR(GUI_GetShown)},
	};

	ccAddExternalFunctions361(gui_api);
}

} // namespace AGS3
