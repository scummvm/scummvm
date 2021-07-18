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

#include "ags/shared/ac/common.h"
#include "ags/engine/ac/gui_control.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_inv.h"
#include "ags/shared/gui/gui_label.h"
#include "ags/shared/gui/gui_listbox.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_slider.h"
#include "ags/shared/gui/gui_textbox.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/ac/dynobj/cc_gui.h"
#include "ags/engine/ac/dynobj/cc_gui_object.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

GUIObject *GetGUIControlAtLocation(int xx, int yy) {
	int guinum = GetGUIAt(xx, yy);
	if (guinum == -1)
		return nullptr;

	data_to_game_coords(&xx, &yy);

	int oldmousex = _G(mousex), oldmousey = _G(mousey);
	_G(mousex) = xx - _GP(guis)[guinum].X;
	_G(mousey) = yy - _GP(guis)[guinum].Y;
	int toret = _GP(guis)[guinum].FindControlUnderMouse(0, false);
	_G(mousex) = oldmousex;
	_G(mousey) = oldmousey;
	if (toret < 0)
		return nullptr;

	return _GP(guis)[guinum].GetControl(toret);
}

int GUIControl_GetVisible(GUIObject *guio) {
	return guio->IsVisible();
}

void GUIControl_SetVisible(GUIObject *guio, int visible) {
	const bool on = visible != 0;
	if (on != guio->IsVisible()) {
		guio->SetVisible(on);
		_GP(guis)[guio->ParentId].OnControlPositionChanged();
	}
}

int GUIControl_GetClickable(GUIObject *guio) {
	if (guio->IsClickable())
		return 1;
	return 0;
}

void GUIControl_SetClickable(GUIObject *guio, int enabled) {
	if (enabled)
		guio->SetClickable(true);
	else
		guio->SetClickable(false);

	_GP(guis)[guio->ParentId].OnControlPositionChanged();
}

int GUIControl_GetEnabled(GUIObject *guio) {
	return guio->IsEnabled() ? 1 : 0;
}

void GUIControl_SetEnabled(GUIObject *guio, int enabled) {
	const bool on = enabled != 0;
	if (on != guio->IsEnabled()) {
		guio->SetEnabled(on);
		_GP(guis)[guio->ParentId].OnControlPositionChanged();
	}
}


int GUIControl_GetID(GUIObject *guio) {
	return guio->Id;
}

ScriptGUI *GUIControl_GetOwningGUI(GUIObject *guio) {
	return &_G(scrGui)[guio->ParentId];
}

GUIButton *GUIControl_GetAsButton(GUIObject *guio) {
	if (_GP(guis)[guio->ParentId].GetControlType(guio->Id) != kGUIButton)
		return nullptr;

	return (GUIButton *)guio;
}

GUIInvWindow *GUIControl_GetAsInvWindow(GUIObject *guio) {
	if (_GP(guis)[guio->ParentId].GetControlType(guio->Id) != kGUIInvWindow)
		return nullptr;

	return (GUIInvWindow *)guio;
}

GUILabel *GUIControl_GetAsLabel(GUIObject *guio) {
	if (_GP(guis)[guio->ParentId].GetControlType(guio->Id) != kGUILabel)
		return nullptr;

	return (GUILabel *)guio;
}

GUIListBox *GUIControl_GetAsListBox(GUIObject *guio) {
	if (_GP(guis)[guio->ParentId].GetControlType(guio->Id) != kGUIListBox)
		return nullptr;

	return (GUIListBox *)guio;
}

GUISlider *GUIControl_GetAsSlider(GUIObject *guio) {
	if (_GP(guis)[guio->ParentId].GetControlType(guio->Id) != kGUISlider)
		return nullptr;

	return (GUISlider *)guio;
}

GUITextBox *GUIControl_GetAsTextBox(GUIObject *guio) {
	if (_GP(guis)[guio->ParentId].GetControlType(guio->Id) != kGUITextBox)
		return nullptr;

	return (GUITextBox *)guio;
}

int GUIControl_GetX(GUIObject *guio) {
	return game_to_data_coord(guio->X);
}

void GUIControl_SetX(GUIObject *guio, int xx) {
	guio->X = data_to_game_coord(xx);
	_GP(guis)[guio->ParentId].OnControlPositionChanged();
}

int GUIControl_GetY(GUIObject *guio) {
	return game_to_data_coord(guio->Y);
}

void GUIControl_SetY(GUIObject *guio, int yy) {
	guio->Y = data_to_game_coord(yy);
	_GP(guis)[guio->ParentId].OnControlPositionChanged();
}

int GUIControl_GetZOrder(GUIObject *guio) {
	return guio->ZOrder;
}

void GUIControl_SetZOrder(GUIObject *guio, int zorder) {
	_GP(guis)[guio->ParentId].SetControlZOrder(guio->Id, zorder);
}

void GUIControl_SetPosition(GUIObject *guio, int xx, int yy) {
	GUIControl_SetX(guio, xx);
	GUIControl_SetY(guio, yy);
}


int GUIControl_GetWidth(GUIObject *guio) {
	return game_to_data_coord(guio->Width);
}

void GUIControl_SetWidth(GUIObject *guio, int newwid) {
	guio->Width = data_to_game_coord(newwid);
	guio->OnResized();
	_GP(guis)[guio->ParentId].OnControlPositionChanged();
}

int GUIControl_GetHeight(GUIObject *guio) {
	return game_to_data_coord(guio->Height);
}

void GUIControl_SetHeight(GUIObject *guio, int newhit) {
	guio->Height = data_to_game_coord(newhit);
	guio->OnResized();
	_GP(guis)[guio->ParentId].OnControlPositionChanged();
}

void GUIControl_SetSize(GUIObject *guio, int newwid, int newhit) {
	if ((newwid < 2) || (newhit < 2))
		quit("!SetGUIObjectSize: new size is too small (must be at least 2x2)");

	debug_script_log("SetGUIObject %d,%d size %d,%d", guio->ParentId, guio->Id, newwid, newhit);
	GUIControl_SetWidth(guio, newwid);
	GUIControl_SetHeight(guio, newhit);
}

void GUIControl_SendToBack(GUIObject *guio) {
	_GP(guis)[guio->ParentId].SendControlToBack(guio->Id);
}

void GUIControl_BringToFront(GUIObject *guio) {
	_GP(guis)[guio->ParentId].BringControlToFront(guio->Id);
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// void (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_BringToFront(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(GUIObject, GUIControl_BringToFront);
}

// GUIObject *(int xx, int yy)
RuntimeScriptValue Sc_GetGUIControlAtLocation(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(GUIObject, _GP(ccDynamicGUIObject), GetGUIControlAtLocation);
}

// void (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_SendToBack(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(GUIObject, GUIControl_SendToBack);
}

// void (GUIObject *guio, int xx, int yy)
RuntimeScriptValue Sc_GUIControl_SetPosition(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(GUIObject, GUIControl_SetPosition);
}

// void (GUIObject *guio, int newwid, int newhit)
RuntimeScriptValue Sc_GUIControl_SetSize(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(GUIObject, GUIControl_SetSize);
}

// GUIButton* (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetAsButton(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUIObject, GUIButton, _GP(ccDynamicGUI), GUIControl_GetAsButton);
}

// GUIInvWindow* (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetAsInvWindow(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUIObject, GUIInvWindow, _GP(ccDynamicGUI), GUIControl_GetAsInvWindow);
}

// GUILabel* (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetAsLabel(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUIObject, GUILabel, _GP(ccDynamicGUI), GUIControl_GetAsLabel);
}

// GUIListBox* (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetAsListBox(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUIObject, GUIListBox, _GP(ccDynamicGUI), GUIControl_GetAsListBox);
}

// GUISlider* (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetAsSlider(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUIObject, GUISlider, _GP(ccDynamicGUI), GUIControl_GetAsSlider);
}

// GUITextBox* (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetAsTextBox(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUIObject, GUITextBox, _GP(ccDynamicGUI), GUIControl_GetAsTextBox);
}

// int (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetClickable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIObject, GUIControl_GetClickable);
}

// void (GUIObject *guio, int enabled)
RuntimeScriptValue Sc_GUIControl_SetClickable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIObject, GUIControl_SetClickable);
}

// int (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetEnabled(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIObject, GUIControl_GetEnabled);
}

// void (GUIObject *guio, int enabled)
RuntimeScriptValue Sc_GUIControl_SetEnabled(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIObject, GUIControl_SetEnabled);
}

// int (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIObject, GUIControl_GetHeight);
}

// void (GUIObject *guio, int newhit)
RuntimeScriptValue Sc_GUIControl_SetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIObject, GUIControl_SetHeight);
}

// int (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIObject, GUIControl_GetID);
}

// ScriptGUI* (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetOwningGUI(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUIObject, ScriptGUI, _GP(ccDynamicGUI), GUIControl_GetOwningGUI);
}

// int (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetVisible(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIObject, GUIControl_GetVisible);
}

// void (GUIObject *guio, int visible)
RuntimeScriptValue Sc_GUIControl_SetVisible(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIObject, GUIControl_SetVisible);
}

// int (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIObject, GUIControl_GetWidth);
}

// void (GUIObject *guio, int newwid)
RuntimeScriptValue Sc_GUIControl_SetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIObject, GUIControl_SetWidth);
}

// int (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIObject, GUIControl_GetX);
}

// void (GUIObject *guio, int xx)
RuntimeScriptValue Sc_GUIControl_SetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIObject, GUIControl_SetX);
}

// int (GUIObject *guio)
RuntimeScriptValue Sc_GUIControl_GetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIObject, GUIControl_GetY);
}

// void (GUIObject *guio, int yy)
RuntimeScriptValue Sc_GUIControl_SetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIObject, GUIControl_SetY);
}

RuntimeScriptValue Sc_GUIControl_GetZOrder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIObject, GUIControl_GetZOrder);
}

RuntimeScriptValue Sc_GUIControl_SetZOrder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIObject, GUIControl_SetZOrder);
}



void RegisterGUIControlAPI() {
	ccAddExternalObjectFunction("GUIControl::BringToFront^0", Sc_GUIControl_BringToFront);
	ccAddExternalStaticFunction("GUIControl::GetAtScreenXY^2", Sc_GetGUIControlAtLocation);
	ccAddExternalObjectFunction("GUIControl::SendToBack^0", Sc_GUIControl_SendToBack);
	ccAddExternalObjectFunction("GUIControl::SetPosition^2", Sc_GUIControl_SetPosition);
	ccAddExternalObjectFunction("GUIControl::SetSize^2", Sc_GUIControl_SetSize);
	ccAddExternalObjectFunction("GUIControl::get_AsButton", Sc_GUIControl_GetAsButton);
	ccAddExternalObjectFunction("GUIControl::get_AsInvWindow", Sc_GUIControl_GetAsInvWindow);
	ccAddExternalObjectFunction("GUIControl::get_AsLabel", Sc_GUIControl_GetAsLabel);
	ccAddExternalObjectFunction("GUIControl::get_AsListBox", Sc_GUIControl_GetAsListBox);
	ccAddExternalObjectFunction("GUIControl::get_AsSlider", Sc_GUIControl_GetAsSlider);
	ccAddExternalObjectFunction("GUIControl::get_AsTextBox", Sc_GUIControl_GetAsTextBox);
	ccAddExternalObjectFunction("GUIControl::get_Clickable", Sc_GUIControl_GetClickable);
	ccAddExternalObjectFunction("GUIControl::set_Clickable", Sc_GUIControl_SetClickable);
	ccAddExternalObjectFunction("GUIControl::get_Enabled", Sc_GUIControl_GetEnabled);
	ccAddExternalObjectFunction("GUIControl::set_Enabled", Sc_GUIControl_SetEnabled);
	ccAddExternalObjectFunction("GUIControl::get_Height", Sc_GUIControl_GetHeight);
	ccAddExternalObjectFunction("GUIControl::set_Height", Sc_GUIControl_SetHeight);
	ccAddExternalObjectFunction("GUIControl::get_ID", Sc_GUIControl_GetID);
	ccAddExternalObjectFunction("GUIControl::get_OwningGUI", Sc_GUIControl_GetOwningGUI);
	ccAddExternalObjectFunction("GUIControl::get_Visible", Sc_GUIControl_GetVisible);
	ccAddExternalObjectFunction("GUIControl::set_Visible", Sc_GUIControl_SetVisible);
	ccAddExternalObjectFunction("GUIControl::get_Width", Sc_GUIControl_GetWidth);
	ccAddExternalObjectFunction("GUIControl::set_Width", Sc_GUIControl_SetWidth);
	ccAddExternalObjectFunction("GUIControl::get_X", Sc_GUIControl_GetX);
	ccAddExternalObjectFunction("GUIControl::set_X", Sc_GUIControl_SetX);
	ccAddExternalObjectFunction("GUIControl::get_Y", Sc_GUIControl_GetY);
	ccAddExternalObjectFunction("GUIControl::set_Y", Sc_GUIControl_SetY);
	ccAddExternalObjectFunction("GUIControl::get_ZOrder", Sc_GUIControl_GetZOrder);
	ccAddExternalObjectFunction("GUIControl::set_ZOrder", Sc_GUIControl_SetZOrder);
}

} // namespace AGS3
