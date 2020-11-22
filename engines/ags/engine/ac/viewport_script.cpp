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

//=============================================================================
//
// Viewport and Camera script API.
//
//=============================================================================

#include "ac/dynobj/scriptcamera.h"
#include "ac/dynobj/scriptviewport.h"
#include "ac/dynobj/scriptuserobject.h"
#include "ac/draw.h"
#include "ac/gamestate.h"
#include "debug/debug_log.h"
#include "script/script_api.h"
#include "script/script_runtime.h"

namespace AGS3 {

using namespace AGS::Shared;

//=============================================================================
//
// Camera script API.
//
//=============================================================================

ScriptCamera *Camera_Create() {
	auto cam = play.CreateRoomCamera();
	if (!cam)
		return NULL;
	return play.RegisterRoomCamera(cam->GetID());
}

void Camera_Delete(ScriptCamera *scam) {
	play.DeleteRoomCamera(scam->GetID());
}

int Camera_GetX(ScriptCamera *scam) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.X: trying to use deleted camera");
		return 0;
	}
	int x = play.GetRoomCamera(scam->GetID())->GetRect().Left;
	return game_to_data_coord(x);
}

void Camera_SetX(ScriptCamera *scam, int x) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.X: trying to use deleted camera");
		return;
	}
	x = data_to_game_coord(x);
	auto cam = play.GetRoomCamera(scam->GetID());
	cam->LockAt(x, cam->GetRect().Top);
}

int Camera_GetY(ScriptCamera *scam) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.Y: trying to use deleted camera");
		return 0;
	}
	int y = play.GetRoomCamera(scam->GetID())->GetRect().Top;
	return game_to_data_coord(y);
}

void Camera_SetY(ScriptCamera *scam, int y) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.Y: trying to use deleted camera");
		return;
	}
	y = data_to_game_coord(y);
	auto cam = play.GetRoomCamera(scam->GetID());
	cam->LockAt(cam->GetRect().Left, y);
}

int Camera_GetWidth(ScriptCamera *scam) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.Width: trying to use deleted camera");
		return 0;
	}
	int width = play.GetRoomCamera(scam->GetID())->GetRect().GetWidth();
	return game_to_data_coord(width);
}

void Camera_SetWidth(ScriptCamera *scam, int width) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.Width: trying to use deleted camera");
		return;
	}
	width = data_to_game_coord(width);
	auto cam = play.GetRoomCamera(scam->GetID());
	cam->SetSize(Size(width, cam->GetRect().GetHeight()));
}

int Camera_GetHeight(ScriptCamera *scam) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.Height: trying to use deleted camera");
		return 0;
	}
	int height = play.GetRoomCamera(scam->GetID())->GetRect().GetHeight();
	return game_to_data_coord(height);
}

void Camera_SetHeight(ScriptCamera *scam, int height) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.Height: trying to use deleted camera");
		return;
	}
	height = data_to_game_coord(height);
	auto cam = play.GetRoomCamera(scam->GetID());
	cam->SetSize(Size(cam->GetRect().GetWidth(), height));
}

bool Camera_GetAutoTracking(ScriptCamera *scam) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.AutoTracking: trying to use deleted camera");
		return false;
	}
	return !play.GetRoomCamera(scam->GetID())->IsLocked();
}

void Camera_SetAutoTracking(ScriptCamera *scam, bool on) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.AutoTracking: trying to use deleted camera");
		return;
	}
	auto cam = play.GetRoomCamera(scam->GetID());
	if (on)
		cam->Release();
	else
		cam->Lock();
}

void Camera_SetAt(ScriptCamera *scam, int x, int y) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.SetAt: trying to use deleted camera");
		return;
	}
	data_to_game_coords(&x, &y);
	play.GetRoomCamera(scam->GetID())->LockAt(x, y);
}

void Camera_SetSize(ScriptCamera *scam, int width, int height) {
	if (scam->GetID() < 0) {
		debug_script_warn("Camera.SetSize: trying to use deleted camera");
		return;
	}
	data_to_game_coords(&width, &height);
	play.GetRoomCamera(scam->GetID())->SetSize(Size(width, height));
}

RuntimeScriptValue Sc_Camera_Create(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO(ScriptCamera, Camera_Create);
}

RuntimeScriptValue Sc_Camera_Delete(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptCamera, Camera_Delete);
}

RuntimeScriptValue Sc_Camera_GetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptCamera, Camera_GetX);
}

RuntimeScriptValue Sc_Camera_SetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptCamera, Camera_SetX);
}

RuntimeScriptValue Sc_Camera_GetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptCamera, Camera_GetY);
}

RuntimeScriptValue Sc_Camera_SetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptCamera, Camera_SetY);
}

RuntimeScriptValue Sc_Camera_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptCamera, Camera_GetWidth);
}

RuntimeScriptValue Sc_Camera_SetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptCamera, Camera_SetWidth);
}

RuntimeScriptValue Sc_Camera_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptCamera, Camera_GetHeight);
}

RuntimeScriptValue Sc_Camera_SetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptCamera, Camera_SetHeight);
}

RuntimeScriptValue Sc_Camera_GetAutoTracking(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(ScriptCamera, Camera_GetAutoTracking);
}

RuntimeScriptValue Sc_Camera_SetAutoTracking(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PBOOL(ScriptCamera, Camera_SetAutoTracking);
}

RuntimeScriptValue Sc_Camera_SetAt(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(ScriptCamera, Camera_SetAt);
}

RuntimeScriptValue Sc_Camera_SetSize(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(ScriptCamera, Camera_SetSize);
}


//=============================================================================
//
// Viewport script API.
//
//=============================================================================

ScriptViewport *Viewport_Create() {
	auto view = play.CreateRoomViewport();
	if (!view)
		return NULL;
	return play.RegisterRoomViewport(view->GetID());
}

void Viewport_Delete(ScriptViewport *scv) {
	play.DeleteRoomViewport(scv->GetID());
}

int Viewport_GetX(ScriptViewport *scv) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.X: trying to use deleted viewport");
		return 0;
	}
	int x = play.GetRoomViewport(scv->GetID())->GetRect().Left;
	return game_to_data_coord(x);
}

void Viewport_SetX(ScriptViewport *scv, int x) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.X: trying to use deleted viewport");
		return;
	}
	x = data_to_game_coord(x);
	auto view = play.GetRoomViewport(scv->GetID());
	view->SetAt(x, view->GetRect().Top);
}

int Viewport_GetY(ScriptViewport *scv) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.Y: trying to use deleted viewport");
		return 0;
	}
	int y = play.GetRoomViewport(scv->GetID())->GetRect().Top;
	return game_to_data_coord(y);
}

void Viewport_SetY(ScriptViewport *scv, int y) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.Y: trying to use deleted viewport");
		return;
	}
	y = data_to_game_coord(y);
	auto view = play.GetRoomViewport(scv->GetID());
	view->SetAt(view->GetRect().Left, y);
}

int Viewport_GetWidth(ScriptViewport *scv) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.Width: trying to use deleted viewport");
		return 0;
	}
	int width = play.GetRoomViewport(scv->GetID())->GetRect().GetWidth();
	return game_to_data_coord(width);
}

void Viewport_SetWidth(ScriptViewport *scv, int width) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.Width: trying to use deleted viewport");
		return;
	}
	width = data_to_game_coord(width);
	auto view = play.GetRoomViewport(scv->GetID());
	view->SetSize(Size(width, view->GetRect().GetHeight()));
}

int Viewport_GetHeight(ScriptViewport *scv) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.Height: trying to use deleted viewport");
		return 0;
	}
	int height = play.GetRoomViewport(scv->GetID())->GetRect().GetHeight();
	return game_to_data_coord(height);
}

void Viewport_SetHeight(ScriptViewport *scv, int height) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.Height: trying to use deleted viewport");
		return;
	}
	height = data_to_game_coord(height);
	auto view = play.GetRoomViewport(scv->GetID());
	view->SetSize(Size(view->GetRect().GetWidth(), height));
}

ScriptCamera *Viewport_GetCamera(ScriptViewport *scv) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.Camera: trying to use deleted viewport");
		return nullptr;
	}
	auto view = play.GetRoomViewport(scv->GetID());
	auto cam = view->GetCamera();
	if (!cam)
		return nullptr;
	return play.GetScriptCamera(cam->GetID());
}

void Viewport_SetCamera(ScriptViewport *scv, ScriptCamera *scam) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.Camera: trying to use deleted viewport");
		return;
	}
	if (scam != nullptr && scam->GetID() < 0) {
		debug_script_warn("Viewport.Camera: trying to link deleted camera");
		return;
	}
	auto view = play.GetRoomViewport(scv->GetID());
	// unlink previous camera
	auto cam = view->GetCamera();
	if (cam)
		cam->UnlinkFromViewport(view->GetID());
	// link new one
	if (scam != nullptr) {
		cam = play.GetRoomCamera(scam->GetID());
		view->LinkCamera(cam);
		cam->LinkToViewport(view);
	} else {
		view->LinkCamera(nullptr);
	}
}

bool Viewport_GetVisible(ScriptViewport *scv) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.Visible: trying to use deleted viewport");
		return false;
	}
	return play.GetRoomViewport(scv->GetID())->IsVisible();
}

void Viewport_SetVisible(ScriptViewport *scv, bool on) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.Visible: trying to use deleted viewport");
		return;
	}
	play.GetRoomViewport(scv->GetID())->SetVisible(on);
}

int Viewport_GetZOrder(ScriptViewport *scv) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.ZOrder: trying to use deleted viewport");
		return 0;
	}
	return play.GetRoomViewport(scv->GetID())->GetZOrder();
}

void Viewport_SetZOrder(ScriptViewport *scv, int zorder) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.ZOrder: trying to use deleted viewport");
		return;
	}
	play.GetRoomViewport(scv->GetID())->SetZOrder(zorder);
	play.InvalidateViewportZOrder();
}

ScriptViewport *Viewport_GetAtScreenXY(int x, int y) {
	data_to_game_coords(&x, &y);
	PViewport view = play.GetRoomViewportAt(x, y);
	if (!view)
		return nullptr;
	return play.GetScriptViewport(view->GetID());
}

void Viewport_SetPosition(ScriptViewport *scv, int x, int y, int width, int height) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.SetPosition: trying to use deleted viewport");
		return;
	}
	data_to_game_coords(&x, &y);
	data_to_game_coords(&width, &height);
	play.GetRoomViewport(scv->GetID())->SetRect(RectWH(x, y, width, height));
}

ScriptUserObject *Viewport_ScreenToRoomPoint(ScriptViewport *scv, int scrx, int scry, bool clipViewport) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.ScreenToRoomPoint: trying to use deleted viewport");
		return nullptr;
	}
	data_to_game_coords(&scrx, &scry);

	VpPoint vpt = play.GetRoomViewport(scv->GetID())->ScreenToRoom(scrx, scry, clipViewport);
	if (vpt.second < 0)
		return nullptr;

	game_to_data_coords(vpt.first.X, vpt.first.Y);
	return ScriptStructHelpers::CreatePoint(vpt.first.X, vpt.first.Y);
}

ScriptUserObject *Viewport_RoomToScreenPoint(ScriptViewport *scv, int roomx, int roomy, bool clipViewport) {
	if (scv->GetID() < 0) {
		debug_script_warn("Viewport.RoomToScreenPoint: trying to use deleted viewport");
		return nullptr;
	}
	data_to_game_coords(&roomx, &roomy);
	Point pt = play.RoomToScreen(roomx, roomy);
	if (clipViewport && !play.GetRoomViewport(scv->GetID())->GetRect().IsInside(pt.X, pt.Y))
		return nullptr;

	game_to_data_coords(pt.X, pt.Y);
	return ScriptStructHelpers::CreatePoint(pt.X, pt.Y);
}

RuntimeScriptValue Sc_Viewport_Create(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO(ScriptViewport, Viewport_Create);
}

RuntimeScriptValue Sc_Viewport_Delete(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptViewport, Viewport_Delete);
}

RuntimeScriptValue Sc_Viewport_GetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewport, Viewport_GetX);
}

RuntimeScriptValue Sc_Viewport_SetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptViewport, Viewport_SetX);
}

RuntimeScriptValue Sc_Viewport_GetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewport, Viewport_GetY);
}

RuntimeScriptValue Sc_Viewport_SetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptViewport, Viewport_SetY);
}

RuntimeScriptValue Sc_Viewport_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewport, Viewport_GetWidth);
}

RuntimeScriptValue Sc_Viewport_SetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptViewport, Viewport_SetWidth);
}

RuntimeScriptValue Sc_Viewport_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewport, Viewport_GetHeight);
}

RuntimeScriptValue Sc_Viewport_SetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptViewport, Viewport_SetHeight);
}

RuntimeScriptValue Sc_Viewport_GetCamera(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJAUTO(ScriptViewport, ScriptCamera, Viewport_GetCamera);
}

RuntimeScriptValue Sc_Viewport_SetCamera(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(ScriptViewport, Viewport_SetCamera, ScriptCamera);
}

RuntimeScriptValue Sc_Viewport_GetVisible(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(ScriptViewport, Viewport_GetVisible);
}

RuntimeScriptValue Sc_Viewport_SetVisible(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PBOOL(ScriptViewport, Viewport_SetVisible);
}

RuntimeScriptValue Sc_Viewport_GetZOrder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewport, Viewport_GetZOrder);
}

RuntimeScriptValue Sc_Viewport_SetZOrder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptViewport, Viewport_SetZOrder);
}

RuntimeScriptValue Sc_Viewport_GetAtScreenXY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT2(ScriptViewport, Viewport_GetAtScreenXY);
}

RuntimeScriptValue Sc_Viewport_SetPosition(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(ScriptViewport, Viewport_SetPosition);
}

RuntimeScriptValue Sc_Viewport_ScreenToRoomPoint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJAUTO_PINT2_PBOOL(ScriptViewport, ScriptUserObject, Viewport_ScreenToRoomPoint);
}

RuntimeScriptValue Sc_Viewport_RoomToScreenPoint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJAUTO_PINT2_PBOOL(ScriptViewport, ScriptUserObject, Viewport_RoomToScreenPoint);
}



void RegisterViewportAPI() {
	ccAddExternalStaticFunction("Camera::Create", Sc_Camera_Create);
	ccAddExternalObjectFunction("Camera::Delete", Sc_Camera_Delete);
	ccAddExternalObjectFunction("Camera::get_X", Sc_Camera_GetX);
	ccAddExternalObjectFunction("Camera::set_X", Sc_Camera_SetX);
	ccAddExternalObjectFunction("Camera::get_Y", Sc_Camera_GetY);
	ccAddExternalObjectFunction("Camera::set_Y", Sc_Camera_SetY);
	ccAddExternalObjectFunction("Camera::get_Width", Sc_Camera_GetWidth);
	ccAddExternalObjectFunction("Camera::set_Width", Sc_Camera_SetWidth);
	ccAddExternalObjectFunction("Camera::get_Height", Sc_Camera_GetHeight);
	ccAddExternalObjectFunction("Camera::set_Height", Sc_Camera_SetHeight);
	ccAddExternalObjectFunction("Camera::get_AutoTracking", Sc_Camera_GetAutoTracking);
	ccAddExternalObjectFunction("Camera::set_AutoTracking", Sc_Camera_SetAutoTracking);
	ccAddExternalObjectFunction("Camera::SetAt", Sc_Camera_SetAt);
	ccAddExternalObjectFunction("Camera::SetSize", Sc_Camera_SetSize);

	ccAddExternalStaticFunction("Viewport::Create", Sc_Viewport_Create);
	ccAddExternalObjectFunction("Viewport::Delete", Sc_Viewport_Delete);
	ccAddExternalObjectFunction("Viewport::get_X", Sc_Viewport_GetX);
	ccAddExternalObjectFunction("Viewport::set_X", Sc_Viewport_SetX);
	ccAddExternalObjectFunction("Viewport::get_Y", Sc_Viewport_GetY);
	ccAddExternalObjectFunction("Viewport::set_Y", Sc_Viewport_SetY);
	ccAddExternalObjectFunction("Viewport::get_Width", Sc_Viewport_GetWidth);
	ccAddExternalObjectFunction("Viewport::set_Width", Sc_Viewport_SetWidth);
	ccAddExternalObjectFunction("Viewport::get_Height", Sc_Viewport_GetHeight);
	ccAddExternalObjectFunction("Viewport::set_Height", Sc_Viewport_SetHeight);
	ccAddExternalObjectFunction("Viewport::get_Camera", Sc_Viewport_GetCamera);
	ccAddExternalObjectFunction("Viewport::set_Camera", Sc_Viewport_SetCamera);
	ccAddExternalObjectFunction("Viewport::get_Visible", Sc_Viewport_GetVisible);
	ccAddExternalObjectFunction("Viewport::set_Visible", Sc_Viewport_SetVisible);
	ccAddExternalObjectFunction("Viewport::get_ZOrder", Sc_Viewport_GetZOrder);
	ccAddExternalObjectFunction("Viewport::set_ZOrder", Sc_Viewport_SetZOrder);
	ccAddExternalObjectFunction("Viewport::GetAtScreenXY", Sc_Viewport_GetAtScreenXY);
	ccAddExternalObjectFunction("Viewport::SetPosition", Sc_Viewport_SetPosition);
	ccAddExternalObjectFunction("Viewport::ScreenToRoomPoint", Sc_Viewport_ScreenToRoomPoint);
	ccAddExternalObjectFunction("Viewport::RoomToScreenPoint", Sc_Viewport_RoomToScreenPoint);
}

} // namespace AGS3
