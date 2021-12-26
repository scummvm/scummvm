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

#include "ags/plugins/core/gui.h"
#include "ags/engine/ac/gui.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void GUI::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(GUI::Centre^0, GUI::Centre);
	SCRIPT_METHOD(GUI::GetAtScreenXY^2, GUI::GetGUIAtLocation);
	SCRIPT_METHOD(GUI::SetPosition^2, GUI::SetPosition);
	SCRIPT_METHOD(GUI::SetSize^2, GUI::SetSize);
	SCRIPT_METHOD(GUI::get_BackgroundGraphic, GUI::GetBackgroundGraphic);
	SCRIPT_METHOD(GUI::set_BackgroundGraphic, GUI::SetBackgroundGraphic);
	SCRIPT_METHOD(GUI::get_Clickable, GUI::GetClickable);
	SCRIPT_METHOD(GUI::set_Clickable, GUI::SetClickable);
	SCRIPT_METHOD(GUI::get_ControlCount, GUI::GetControlCount);
	SCRIPT_METHOD(GUI::geti_Controls, GUI::GetiControls);
	SCRIPT_METHOD(GUI::get_Height, GUI::GetHeight);
	SCRIPT_METHOD(GUI::set_Height, GUI::SetHeight);
	SCRIPT_METHOD(GUI::get_ID, GUI::GetID);
	SCRIPT_METHOD(GUI::get_Transparency, GUI::GetTransparency);
	SCRIPT_METHOD(GUI::set_Transparency, GUI::SetTransparency);
	SCRIPT_METHOD(GUI::get_Visible, GUI::GetVisible);
	SCRIPT_METHOD(GUI::set_Visible, GUI::SetVisible);
	SCRIPT_METHOD(GUI::get_Width, GUI::GetWidth);
	SCRIPT_METHOD(GUI::set_Width, GUI::SetWidth);
	SCRIPT_METHOD(GUI::get_X, GUI::GetX);
	SCRIPT_METHOD(GUI::set_X, GUI::SetX);
	SCRIPT_METHOD(GUI::get_Y, GUI::GetY);
	SCRIPT_METHOD(GUI::set_Y, GUI::SetY);
	SCRIPT_METHOD(GUI::get_ZOrder, GUI::GetZOrder);
	SCRIPT_METHOD(GUI::set_ZOrder, GUI::SetZOrder);
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
