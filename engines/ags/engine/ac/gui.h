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

#ifndef AGS_ENGINE_AC_GUI_H
#define AGS_ENGINE_AC_GUI_H

#include "ags/engine/ac/dynobj/script_gui.h"
#include "ags/shared/gui/gui_main.h"

namespace AGS3 {

using AGS::Shared::GUIMain;
using AGS::Shared::GUIObject;

ScriptGUI *GUI_AsTextWindow(ScriptGUI *tehgui);
int     GUI_GetPopupStyle(ScriptGUI *tehgui);
void    GUI_SetVisible(ScriptGUI *tehgui, int isvisible);
int     GUI_GetVisible(ScriptGUI *tehgui);
int     GUI_GetX(ScriptGUI *tehgui);
void    GUI_SetX(ScriptGUI *tehgui, int xx);
int     GUI_GetY(ScriptGUI *tehgui);
void    GUI_SetY(ScriptGUI *tehgui, int yy);
void    GUI_SetPosition(ScriptGUI *tehgui, int xx, int yy);
void    GUI_SetSize(ScriptGUI *sgui, int widd, int hitt);
int     GUI_GetWidth(ScriptGUI *sgui);
int     GUI_GetHeight(ScriptGUI *sgui);
void    GUI_SetWidth(ScriptGUI *sgui, int newwid);
void    GUI_SetHeight(ScriptGUI *sgui, int newhit);
void    GUI_SetZOrder(ScriptGUI *tehgui, int z);
int     GUI_GetZOrder(ScriptGUI *tehgui);
void    GUI_SetClickable(ScriptGUI *tehgui, int clickable);
int     GUI_GetClickable(ScriptGUI *tehgui);
int     GUI_GetID(ScriptGUI *tehgui);
GUIObject *GUI_GetiControls(ScriptGUI *tehgui, int idx);
int     GUI_GetControlCount(ScriptGUI *tehgui);
void    GUI_SetPopupYPos(ScriptGUI *tehgui, int newpos);
int     GUI_GetPopupYPos(ScriptGUI *tehgui);
void    GUI_SetTransparency(ScriptGUI *tehgui, int trans);
int     GUI_GetTransparency(ScriptGUI *tehgui);
void    GUI_Centre(ScriptGUI *sgui);
void    GUI_SetBackgroundGraphic(ScriptGUI *tehgui, int slotn);
int     GUI_GetBackgroundGraphic(ScriptGUI *tehgui);
void    GUI_SetBackgroundColor(ScriptGUI *tehgui, int newcol);
int     GUI_GetBackgroundColor(ScriptGUI *tehgui);
void    GUI_SetBorderColor(ScriptGUI *tehgui, int newcol);
int     GUI_GetBorderColor(ScriptGUI *tehgui);
void    GUI_SetTextColor(ScriptGUI *tehgui, int newcol);
int     GUI_GetTextColor(ScriptGUI *tehgui);
void    GUI_SetTextPadding(ScriptGUI *tehgui, int newpos);
int     GUI_GetTextPadding(ScriptGUI *tehgui);
ScriptGUI *GetGUIAtLocation(int xx, int yy);

void    remove_popup_interface(int ifacenum);
void    process_interface_click(int ifce, int btn, int mbut);
void    replace_macro_tokens(const char *text, AGS::Shared::String &fixed_text);
void    update_gui_zorder();
void    export_gui_controls(int ee);
void    unexport_gui_controls(int ee);
void    update_gui_disabled_status();
int     adjust_x_for_guis(int x, int y, bool assume_blocking = false);
int     adjust_y_for_guis(int y, bool assume_blocking = false);

int     gui_get_interactable(int x, int y);
int     gui_on_mouse_move(const int mx, const int my);
void    gui_on_mouse_hold(const int wasongui, const int wasbutdown);
void    gui_on_mouse_up(const int wasongui, const int wasbutdown, const int mx, const int my);
void    gui_on_mouse_down(const int guin, const int mbut, const int mx, const int my);

} // namespace AGS3

#endif
