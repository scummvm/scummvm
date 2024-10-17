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
#include "ags/engine/ac/gui_control.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/string.h"
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
#include "ags/engine/ac/dynobj/script_string.h"
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
	int32_t toret = _GP(guis)[guinum].FindControlAt(xx, yy, 0, false);

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
	}
}

int GUIControl_GetClickable(GUIObject *guio) {
	if (guio->IsClickable())
		return 1;
	return 0;
}

void GUIControl_SetClickable(GUIObject *guio, int enabled) {
	const bool on = enabled != 0;
	if (on != guio->IsClickable()) {
		guio->SetClickable(on);
	}
}

int GUIControl_GetEnabled(GUIObject *guio) {
	return guio->IsEnabled() ? 1 : 0;
}

void GUIControl_SetEnabled(GUIObject *guio, int enabled) {
	const bool on = enabled != 0;
	if (on != guio->IsEnabled()) {
		guio->SetEnabled(on);
	}
}


int GUIControl_GetID(GUIObject *guio) {
	return guio->Id;
}

const char *GUIControl_GetScriptName(GUIObject *guio) {
	return CreateNewScriptString(guio->Name);
}

ScriptGUI *GUIControl_GetOwningGUI(GUIObject *guio) {
	return &_GP(scrGui)[guio->ParentId];
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
	_GP(guis)[guio->ParentId].NotifyControlPosition(); // update control under cursor
}

int GUIControl_GetY(GUIObject *guio) {
	return game_to_data_coord(guio->Y);
}

void GUIControl_SetY(GUIObject *guio, int yy) {
	guio->Y = data_to_game_coord(yy);
	_GP(guis)[guio->ParentId].NotifyControlPosition(); // update control under cursor
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
	return game_to_data_coord(guio->GetWidth());
}

void GUIControl_SetWidth(GUIObject *guio, int newwid) {
	guio->SetWidth(data_to_game_coord(newwid));
}

int GUIControl_GetHeight(GUIObject *guio) {
	return game_to_data_coord(guio->GetHeight());
}

void GUIControl_SetHeight(GUIObject *guio, int newhit) {
	guio->SetHeight(data_to_game_coord(newhit));
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

int GUIControl_GetTransparency(GUIObject *guio) {
	return GfxDef::LegacyTrans255ToTrans100(guio->GetTransparency());
}

void GUIControl_SetTransparency(GUIObject *guio, int trans) {
	if ((trans < 0) | (trans > 100))
		quit("!SetGUITransparency: transparency value must be between 0 and 100");
	guio->SetTransparency(GfxDef::Trans100ToLegacyTrans255(trans));
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

GUIObject *GUIControl_GetByName(const char *name) {
	return static_cast<GUIObject *>(ccGetScriptObjectAddress(name, _GP(ccDynamicGUIObject).GetType()));
}

RuntimeScriptValue Sc_GUIControl_GetByName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_POBJ(GUIObject, _GP(ccDynamicGUIObject), GUIControl_GetByName, const char);
}

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

RuntimeScriptValue Sc_GUIControl_GetScriptName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUIObject, const char, _GP(myScriptStringImpl), GUIControl_GetScriptName);
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

RuntimeScriptValue Sc_GUIControl_GetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIObject, GUIControl_GetTransparency);
}

RuntimeScriptValue Sc_GUIControl_SetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIObject, GUIControl_SetTransparency);
}

void RegisterGUIControlAPI() {
	ScFnRegister guicontrol_api[] = {
		{"GUIControl::GetAtScreenXY^2", API_FN_PAIR(GetGUIControlAtLocation)},
		{"GUIControl::GetByName", API_FN_PAIR(GUIControl_GetByName)},

		{"GUIControl::BringToFront^0", API_FN_PAIR(GUIControl_BringToFront)},
		{"GUIControl::SendToBack^0", API_FN_PAIR(GUIControl_SendToBack)},
		{"GUIControl::SetPosition^2", API_FN_PAIR(GUIControl_SetPosition)},
		{"GUIControl::SetSize^2", API_FN_PAIR(GUIControl_SetSize)},
		{"GUIControl::get_AsButton", API_FN_PAIR(GUIControl_GetAsButton)},
		{"GUIControl::get_AsInvWindow", API_FN_PAIR(GUIControl_GetAsInvWindow)},
		{"GUIControl::get_AsLabel", API_FN_PAIR(GUIControl_GetAsLabel)},
		{"GUIControl::get_AsListBox", API_FN_PAIR(GUIControl_GetAsListBox)},
		{"GUIControl::get_AsSlider", API_FN_PAIR(GUIControl_GetAsSlider)},
		{"GUIControl::get_AsTextBox", API_FN_PAIR(GUIControl_GetAsTextBox)},
		{"GUIControl::get_Clickable", API_FN_PAIR(GUIControl_GetClickable)},
		{"GUIControl::set_Clickable", API_FN_PAIR(GUIControl_SetClickable)},
		{"GUIControl::get_Enabled", API_FN_PAIR(GUIControl_GetEnabled)},
		{"GUIControl::set_Enabled", API_FN_PAIR(GUIControl_SetEnabled)},
		{"GUIControl::get_Height", API_FN_PAIR(GUIControl_GetHeight)},
		{"GUIControl::set_Height", API_FN_PAIR(GUIControl_SetHeight)},
		{"GUIControl::get_ID", API_FN_PAIR(GUIControl_GetID)},
		{"GUIControl::get_OwningGUI", API_FN_PAIR(GUIControl_GetOwningGUI)},
		{"GUIControl::get_ScriptName", API_FN_PAIR(GUIControl_GetScriptName)},
		{"GUIControl::get_Visible", API_FN_PAIR(GUIControl_GetVisible)},
		{"GUIControl::set_Visible", API_FN_PAIR(GUIControl_SetVisible)},
		{"GUIControl::get_Width", API_FN_PAIR(GUIControl_GetWidth)},
		{"GUIControl::set_Width", API_FN_PAIR(GUIControl_SetWidth)},
		{"GUIControl::get_X", API_FN_PAIR(GUIControl_GetX)},
		{"GUIControl::set_X", API_FN_PAIR(GUIControl_SetX)},
		{"GUIControl::get_Y", API_FN_PAIR(GUIControl_GetY)},
		{"GUIControl::set_Y", API_FN_PAIR(GUIControl_SetY)},
		{"GUIControl::get_ZOrder", API_FN_PAIR(GUIControl_GetZOrder)},
		{"GUIControl::set_ZOrder", API_FN_PAIR(GUIControl_SetZOrder)},
		{"GUIControl::get_Transparency", API_FN_PAIR(GUIControl_GetTransparency)},
		{"GUIControl::set_Transparency", API_FN_PAIR(GUIControl_SetTransparency)},
	};

	ccAddExternalFunctions361(guicontrol_api);
}

} // namespace AGS3
