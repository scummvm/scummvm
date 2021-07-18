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

#include "ags/plugins/core/gui.h"
#include "ags/engine/ac/gui.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void GUI::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(GUI::Centre^0, Centre);
	SCRIPT_METHOD_EXT(GUI::GetAtScreenXY^2, GetGUIAtLocation);
	SCRIPT_METHOD_EXT(GUI::SetPosition^2, SetPosition);
	SCRIPT_METHOD_EXT(GUI::SetSize^2, SetSize);
	SCRIPT_METHOD_EXT(GUI::get_BackgroundGraphic, GetBackgroundGraphic);
	SCRIPT_METHOD_EXT(GUI::set_BackgroundGraphic, SetBackgroundGraphic);
	SCRIPT_METHOD_EXT(GUI::get_Clickable, GetClickable);
	SCRIPT_METHOD_EXT(GUI::set_Clickable, SetClickable);
	SCRIPT_METHOD_EXT(GUI::get_ControlCount, GetControlCount);
	SCRIPT_METHOD_EXT(GUI::geti_Controls, GetiControls);
	SCRIPT_METHOD_EXT(GUI::get_Height, GetHeight);
	SCRIPT_METHOD_EXT(GUI::set_Height, SetHeight);
	SCRIPT_METHOD_EXT(GUI::get_ID, GetID);
	SCRIPT_METHOD_EXT(GUI::get_Transparency, GetTransparency);
	SCRIPT_METHOD_EXT(GUI::set_Transparency, SetTransparency);
	SCRIPT_METHOD_EXT(GUI::get_Visible, GetVisible);
	SCRIPT_METHOD_EXT(GUI::set_Visible, SetVisible);
	SCRIPT_METHOD_EXT(GUI::get_Width, GetWidth);
	SCRIPT_METHOD_EXT(GUI::set_Width, SetWidth);
	SCRIPT_METHOD_EXT(GUI::get_X, GetX);
	SCRIPT_METHOD_EXT(GUI::set_X, SetX);
	SCRIPT_METHOD_EXT(GUI::get_Y, GetY);
	SCRIPT_METHOD_EXT(GUI::set_Y, SetY);
	SCRIPT_METHOD_EXT(GUI::get_ZOrder, GetZOrder);
	SCRIPT_METHOD_EXT(GUI::set_ZOrder, SetZOrder);
}

void GUI::Centre(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	AGS3::GUI_Centre(sgui);
}

void GUI::GetGUIAtLocation(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetGUIAtLocation(xx, yy);
}

void GUI::SetPosition(ScriptMethodParams &params) {
	PARAMS3(ScriptGUI *, tehgui, int, xx, int, yy);
	AGS3::GUI_SetPosition(tehgui, xx, yy);
}

void GUI::SetSize(ScriptMethodParams &params) {
	PARAMS3(ScriptGUI *, sgui, int, widd, int, hitt);
	AGS3::GUI_SetSize(sgui, widd, hitt);
}

void GUI::GetBackgroundGraphic(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetBackgroundGraphic(sgui);

}

void GUI::SetBackgroundGraphic(ScriptMethodParams &params) {
	PARAMS2(ScriptGUI *, tehgui, int, slotn);
	AGS3::GUI_SetBackgroundGraphic(tehgui, slotn);
}

void GUI::GetClickable(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetClickable(sgui);
}

void GUI::SetClickable(ScriptMethodParams &params) {
	PARAMS2(ScriptGUI *, tehgui, int, clickable);
	AGS3::GUI_SetClickable(tehgui, clickable);
}

void GUI::GetControlCount(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetControlCount(sgui);
}

void GUI::GetiControls(ScriptMethodParams &params) {
	PARAMS2(ScriptGUI *, sgui, int, idx);
	params._result = AGS3::GUI_GetiControls(sgui, idx);
}

void GUI::GetHeight(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetHeight(sgui);
}

void GUI::SetHeight(ScriptMethodParams &params) {
	PARAMS2(ScriptGUI *, sgui, int, newhit);
	AGS3::GUI_SetHeight(sgui, newhit);
}

void GUI::GetID(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetID(sgui);
}

void GUI::GetTransparency(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetTransparency(sgui);
}

void GUI::SetTransparency(ScriptMethodParams &params) {
	PARAMS2(ScriptGUI *, tehgui, int, trans);
	AGS3::GUI_SetTransparency(tehgui, trans);
}

void GUI::GetVisible(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetVisible(sgui);
}

void GUI::SetVisible(ScriptMethodParams &params) {
	PARAMS2(ScriptGUI *, tehgui, int, isvisible);
	AGS3::GUI_SetVisible(tehgui, isvisible);
}

void GUI::GetWidth(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetWidth(sgui);

}

void GUI::SetWidth(ScriptMethodParams &params) {
	PARAMS2(ScriptGUI *, sgui, int, newwid);
	AGS3::GUI_SetWidth(sgui, newwid);
}

void GUI::GetX(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetX(sgui);
}

void GUI::SetX(ScriptMethodParams &params) {
	PARAMS2(ScriptGUI *, tehgui, int, xx);
	AGS3::GUI_SetX(tehgui, xx);
}

void GUI::GetY(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetY(sgui);

}

void GUI::SetY(ScriptMethodParams &params) {
	PARAMS2(ScriptGUI *, tehgui, int, yy);
	AGS3::GUI_SetY(tehgui, yy);
}

void GUI::GetZOrder(ScriptMethodParams &params) {
	PARAMS1(ScriptGUI *, sgui);
	params._result = AGS3::GUI_GetZOrder(sgui);
}

void GUI::SetZOrder(ScriptMethodParams &params) {
	PARAMS2(ScriptGUI *, tehgui, int, z);
	AGS3::GUI_SetZOrder(tehgui, z);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
