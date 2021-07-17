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

#include "ags/plugins/core/gui_control.h"
#include "ags/engine/ac/gui_control.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void GUIControl::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(GUIControl::BringToFront^0, BringToFront);
	SCRIPT_METHOD_EXT(GUIControl::GetAtScreenXY^2, GetGUIControlAtLocation);
	SCRIPT_METHOD_EXT(GUIControl::SendToBack^0, SendToBack);
	SCRIPT_METHOD_EXT(GUIControl::SetPosition^2, SetPosition);
	SCRIPT_METHOD_EXT(GUIControl::SetSize^2, SetSize);
	SCRIPT_METHOD_EXT(GUIControl::get_AsButton, GetAsButton);
	SCRIPT_METHOD_EXT(GUIControl::get_AsInvWindow, GetAsInvWindow);
	SCRIPT_METHOD_EXT(GUIControl::get_AsLabel, GetAsLabel);
	SCRIPT_METHOD_EXT(GUIControl::get_AsListBox, GetAsListBox);
	SCRIPT_METHOD_EXT(GUIControl::get_AsSlider, GetAsSlider);
	SCRIPT_METHOD_EXT(GUIControl::get_AsTextBox, GetAsTextBox);
	SCRIPT_METHOD_EXT(GUIControl::get_Clickable, GetClickable);
	SCRIPT_METHOD_EXT(GUIControl::set_Clickable, SetClickable);
	SCRIPT_METHOD_EXT(GUIControl::get_Enabled, GetEnabled);
	SCRIPT_METHOD_EXT(GUIControl::set_Enabled, SetEnabled);
	SCRIPT_METHOD_EXT(GUIControl::get_Height, GetHeight);
	SCRIPT_METHOD_EXT(GUIControl::set_Height, SetHeight);
	SCRIPT_METHOD_EXT(GUIControl::get_ID, GetID);
	SCRIPT_METHOD_EXT(GUIControl::get_OwningGUI, GetOwningGUI);
	SCRIPT_METHOD_EXT(GUIControl::get_Visible, GetVisible);
	SCRIPT_METHOD_EXT(GUIControl::set_Visible, SetVisible);
	SCRIPT_METHOD_EXT(GUIControl::get_Width, GetWidth);
	SCRIPT_METHOD_EXT(GUIControl::set_Width, SetWidth);
	SCRIPT_METHOD_EXT(GUIControl::get_X, GetX);
	SCRIPT_METHOD_EXT(GUIControl::set_X, SetX);
	SCRIPT_METHOD_EXT(GUIControl::get_Y, GetY);
	SCRIPT_METHOD_EXT(GUIControl::set_Y, SetY);
}

void GUIControl::BringToFront(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	AGS3::GUIControl_BringToFront(guio);
}

void GUIControl::GetGUIControlAtLocation(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetGUIControlAtLocation(xx, yy);
}

void GUIControl::SendToBack(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	AGS3::GUIControl_SendToBack(guio);
}

void GUIControl::SetPosition(ScriptMethodParams &params) {
	PARAMS3(GUIObject *, guio, int, xx, int, yy);
	AGS3::GUIControl_SetPosition(guio, xx, yy);
}

void GUIControl::SetSize(ScriptMethodParams &params) {
	PARAMS3(GUIObject *, guio, int, newwid, int, newhit);
	AGS3::GUIControl_SetSize(guio, newwid, newhit);
}

void GUIControl::GetAsButton(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetAsButton(guio);
}

void GUIControl::GetAsInvWindow(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetAsInvWindow(guio);
}

void GUIControl::GetAsLabel(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetAsLabel(guio);
}

void GUIControl::GetAsListBox(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetAsListBox(guio);
}

void GUIControl::GetAsSlider(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetAsSlider(guio);
}

void GUIControl::GetAsTextBox(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetAsTextBox(guio);
}

void GUIControl::GetClickable(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetClickable(guio);
}

void GUIControl::SetClickable(ScriptMethodParams &params) {
	PARAMS2(GUIObject *, guio, int, enabled);
	AGS3::GUIControl_SetClickable(guio, enabled);
}

void GUIControl::GetEnabled(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetEnabled(guio);
}

void GUIControl::SetEnabled(ScriptMethodParams &params) {
	PARAMS2(GUIObject *, guio, int, enabled);
	AGS3::GUIControl_SetEnabled(guio, enabled);
}

void GUIControl::GetHeight(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetHeight(guio);
}

void GUIControl::SetHeight(ScriptMethodParams &params) {
	PARAMS2(GUIObject *, guio, int, newhit);
	AGS3::GUIControl_SetHeight(guio, newhit);
}

void GUIControl::GetID(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetID(guio);

}

void GUIControl::GetOwningGUI(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetOwningGUI(guio);
}

void GUIControl::GetVisible(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetVisible(guio);

}

void GUIControl::SetVisible(ScriptMethodParams &params) {
	PARAMS2(GUIObject *, guio, int, visible);
	AGS3::GUIControl_SetVisible(guio, visible);
}

void GUIControl::GetWidth(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetWidth(guio);
}

void GUIControl::SetWidth(ScriptMethodParams &params) {
	PARAMS2(GUIObject *, guio, int, newwid);
	AGS3::GUIControl_SetWidth(guio, newwid);
}

void GUIControl::GetX(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetX(guio);
}

void GUIControl::SetX(ScriptMethodParams &params) {
	PARAMS2(GUIObject *, guio, int, xx);
	AGS3::GUIControl_SetX(guio, xx);
}

void GUIControl::GetY(ScriptMethodParams &params) {
	PARAMS1(GUIObject *, guio);
	params._result = AGS3::GUIControl_GetY(guio);
}

void GUIControl::SetY(ScriptMethodParams &params) {
	PARAMS2(GUIObject *, guio, int, yy);
	AGS3::GUIControl_SetY(guio, yy);
}


} // namespace Core
} // namespace Plugins
} // namespace AGS3
