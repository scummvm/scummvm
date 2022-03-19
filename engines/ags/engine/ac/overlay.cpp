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

#include "ags/engine/ac/overlay.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/character_extras.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_overlay.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

void Overlay_Remove(ScriptOverlay *sco) {
	sco->Remove();
}

void Overlay_SetText(ScriptOverlay *scover, int wii, int fontid, int text_color, const char *text) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!Overlay.SetText: invalid overlay ID specified");
	int xx = game_to_data_coord(_GP(screenover)[ovri].x) - scover->borderWidth;
	int yy = game_to_data_coord(_GP(screenover)[ovri].y) - scover->borderHeight;

	RemoveOverlay(scover->overlayId);
	const int disp_type = scover->overlayId;

	if (CreateTextOverlay(xx, yy, wii, fontid, text_color, get_translation(text), disp_type) != scover->overlayId)
		quit("SetTextOverlay internal error: inconsistent type ids");
}

int Overlay_GetX(ScriptOverlay *scover) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");

	int tdxp, tdyp;
	get_overlay_position(_GP(screenover)[ovri], &tdxp, &tdyp);

	return game_to_data_coord(tdxp);
}

void Overlay_SetX(ScriptOverlay *scover, int newx) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");

	_GP(screenover)[ovri].x = data_to_game_coord(newx);
}

int Overlay_GetY(ScriptOverlay *scover) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");

	int tdxp, tdyp;
	get_overlay_position(_GP(screenover)[ovri], &tdxp, &tdyp);

	return game_to_data_coord(tdyp);
}

void Overlay_SetY(ScriptOverlay *scover, int newy) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");

	_GP(screenover)[ovri].y = data_to_game_coord(newy);
}

int Overlay_GetWidth(ScriptOverlay *scover) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");
	return game_to_data_coord(_GP(screenover)[ovri].pic->GetWidth());
}

int Overlay_GetHeight(ScriptOverlay *scover) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");
	return game_to_data_coord(_GP(screenover)[ovri].pic->GetHeight());
}

int Overlay_GetValid(ScriptOverlay *scover) {
	if (scover->overlayId == -1)
		return 0;

	if (!IsOverlayValid(scover->overlayId)) {
		scover->overlayId = -1;
		return 0;
	}

	return 1;
}

ScriptOverlay *Overlay_CreateGraphical(int x, int y, int slot, int transparent) {
	ScriptOverlay *sco = new ScriptOverlay();
	sco->overlayId = CreateGraphicOverlay(x, y, slot, transparent);
	sco->borderHeight = 0;
	sco->borderWidth = 0;
	sco->isBackgroundSpeech = 0;

	ccRegisterManagedObject(sco, sco);
	return sco;
}

ScriptOverlay *Overlay_CreateTextual(int x, int y, int width, int font, int colour, const char *text) {
	ScriptOverlay *sco = new ScriptOverlay();

	data_to_game_coords(&x, &y);
	width = data_to_game_coord(width);

	sco->overlayId = CreateTextOverlayCore(x, y, width, font, colour, text, DISPLAYTEXT_NORMALOVERLAY, 0);

	int ovri = find_overlay_of_type(sco->overlayId);
	sco->borderWidth = game_to_data_coord(_GP(screenover)[ovri].x - x);
	sco->borderHeight = game_to_data_coord(_GP(screenover)[ovri].y - y);
	sco->isBackgroundSpeech = 0;

	ccRegisterManagedObject(sco, sco);
	return sco;
}

int Overlay_GetTransparency(ScriptOverlay *scover) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");

	return GfxDef::LegacyTrans255ToTrans100(_GP(screenover)[ovri].transparency);
}

void Overlay_SetTransparency(ScriptOverlay *scover, int trans) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");
	if ((trans < 0) | (trans > 100))
		quit("!SetTransparency: transparency value must be between 0 and 100");

	_GP(screenover)[ovri].transparency = GfxDef::Trans100ToLegacyTrans255(trans);
}

int Overlay_GetZOrder(ScriptOverlay *scover) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");

	return _GP(screenover)[ovri].zorder;
}

void Overlay_SetZOrder(ScriptOverlay *scover, int zorder) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");

	_GP(screenover)[ovri].zorder = zorder;
}

//=============================================================================

// Creates and registers a managed script object for existing overlay object
ScriptOverlay *create_scriptobj_for_overlay(ScreenOverlay &over) {
	ScriptOverlay *scover = new ScriptOverlay();
	scover->overlayId = over.type;
	int handl = ccRegisterManagedObject(scover, scover);
	over.associatedOverlayHandle = handl;
	return scover;
}

// Creates managed script object for overlay and adds internal engine's reference to it,
// so that it does not get disposed even if there are no user references in script.
static ScriptOverlay *create_scriptobj_addref(ScreenOverlay &over) {
	ScriptOverlay *scover = create_scriptobj_for_overlay(over);
	ccAddObjectReference(over.associatedOverlayHandle);
	return scover;
}

// Invalidates existing script object to let user know that previous overlay is gone,
// and releases engine's internal reference (script object may exist while there are user refs)
static void invalidate_and_subref(ScreenOverlay &over, ScriptOverlay *&scover) {
	scover->overlayId = -1;
	scover = nullptr;
	ccReleaseObjectReference(over.associatedOverlayHandle);
}

// Frees overlay resources and disposes script object if there are no more refs
static void dispose_overlay(ScreenOverlay &over) {
	delete over.pic;
	over.pic = nullptr;
	if (over.bmp != nullptr)
		_G(gfxDriver)->DestroyDDB(over.bmp);
	over.bmp = nullptr;
	if (over.associatedOverlayHandle) // dispose script object if there are no more refs
		ccAttemptDisposeObject(over.associatedOverlayHandle);
}

void remove_screen_overlay_index(size_t over_idx) {
	ScreenOverlay &over = _GP(screenover)[over_idx];
	// TODO: move these custom settings outside of this function
	if (over.type == _GP(play).complete_overlay_on) {
		_GP(play).complete_overlay_on = 0;
	} else if (over.type == _GP(play).text_overlay_on) {
		if (_GP(play).speech_text_scover)
			invalidate_and_subref(over, _GP(play).speech_text_scover);
		_GP(play).text_overlay_on = 0;
	} else if (over.type == OVER_PICTURE) {
		if (_GP(play).speech_face_scover)
			invalidate_and_subref(over, _GP(play).speech_face_scover);
		_G(face_talking) = -1;
	}
	dispose_overlay(over);
	_GP(screenover).erase(_GP(screenover).begin() + over_idx);
	// if an overlay before the sierra-style speech one is removed, update the index
	// TODO: this is bad, need more generic system to store overlay references
	if ((size_t)_G(face_talking) > over_idx)
		_G(face_talking)--;
}

void remove_screen_overlay(int type) {
	for (size_t i = 0; i < _GP(screenover).size();) {
		if (type < 0 || _GP(screenover)[i].type == type)
			remove_screen_overlay_index(i);
		else
			i++;
	}
}

int find_overlay_of_type(int type) {
	for (size_t i = 0; i < _GP(screenover).size(); ++i) {
		if (_GP(screenover)[i].type == type) return i;
	}
	return -1;
}

size_t add_screen_overlay(int x, int y, int type, Bitmap *piccy, bool alphaChannel) {
	return add_screen_overlay(x, y, type, piccy, 0, 0, alphaChannel);
}

size_t add_screen_overlay(int x, int y, int type, Shared::Bitmap *piccy, int pic_offx, int pic_offy, bool alphaChannel) {
	if (type == OVER_CUSTOM) {
		// find an unused custom ID; TODO: find a better approach!
		for (int id = OVER_CUSTOM + 1; (size_t)id <= _GP(screenover).size() + OVER_CUSTOM + 1; ++id) {
			if (find_overlay_of_type(id) == -1) {
				type = id; break;
			}
		}
	}

	ScreenOverlay over;
	over.pic = piccy;
	over.bmp = _G(gfxDriver)->CreateDDBFromBitmap(piccy, alphaChannel);
	over.x = x;
	over.y = y;
	over._offsetX = pic_offx;
	over._offsetY = pic_offy;
	// by default draw speech and portraits over GUI, and the rest under GUI
	over.zorder = (type == OVER_TEXTMSG || type == OVER_PICTURE || type == OVER_TEXTSPEECH) ?
		INT_MAX : INT_MIN;
	over.type = type;
	over.timeout = 0;
	over.bgSpeechForChar = -1;
	over.associatedOverlayHandle = 0;
	over.hasAlphaChannel = alphaChannel;
	over.positionRelativeToScreen = true;
	// TODO: move these custom settings outside of this function
	if (type == OVER_COMPLETE) _GP(play).complete_overlay_on = type;
	else if (type == OVER_TEXTMSG || type == OVER_TEXTSPEECH) {
		_GP(play).text_overlay_on = type;
		// only make script object for blocking speech now, because messagebox blocks all script
		// and therefore cannot be accessed, so no practical reason for that atm
		if (type == OVER_TEXTSPEECH)
			_GP(play).speech_text_scover = create_scriptobj_addref(over);
	} else if (type == OVER_PICTURE) {
		_GP(play).speech_face_scover = create_scriptobj_addref(over);
	}

	_GP(screenover).push_back(std::move(over));
	return _GP(screenover).size() - 1;
}

void get_overlay_position(const ScreenOverlay &over, int *x, int *y) {
	int tdxp, tdyp;
	const Rect &ui_view = _GP(play).GetUIViewport();

	if (over.x == OVR_AUTOPLACE) {
		// auto place on character
		int charid = over.y;

		auto view = FindNearestViewport(charid);
		const int charpic = _GP(views)[_GP(game).chars[charid].view].loops[_GP(game).chars[charid].loop].frames[0].pic;
		const int height = (_G(charextra)[charid].height < 1) ? _GP(game).SpriteInfos[charpic].Height : _G(charextra)[charid].height;
		Point screenpt = view->RoomToScreen(
		                     data_to_game_coord(_GP(game).chars[charid].x),
		                     data_to_game_coord(_GP(game).chars[charid].get_effective_y()) - height).first;
		tdxp = screenpt.X - over.pic->GetWidth() / 2;
		if (tdxp < 0) tdxp = 0;
		tdyp = screenpt.Y - get_fixed_pixel_size(5);
		tdyp -= over.pic->GetHeight();
		if (tdyp < 5) tdyp = 5;

		if ((tdxp + over.pic->GetWidth()) >= ui_view.GetWidth())
			tdxp = (ui_view.GetWidth() - over.pic->GetWidth()) - 1;
		if (_GP(game).chars[charid].room != _G(displayed_room)) {
			tdxp = ui_view.GetWidth() / 2 - over.pic->GetWidth() / 2;
			tdyp = ui_view.GetHeight() / 2 - over.pic->GetHeight() / 2;
		}
	} else {
		// Note: the internal offset is only needed when x,y coordinates are specified
		// and only in the case where the overlay is using a GUI. See issue #1098
		tdxp = over.x + over._offsetX;
		tdyp = over.y + over._offsetY;

		if (!over.positionRelativeToScreen) {
			Point tdxy = _GP(play).RoomToScreen(tdxp, tdyp);
			tdxp = tdxy.X;
			tdyp = tdxy.Y;
		}
	}
	*x = tdxp;
	*y = tdyp;
}

void recreate_overlay_ddbs() {
	for (auto &over : _GP(screenover)) {
		if (over.bmp)
			_G(gfxDriver)->DestroyDDB(over.bmp);
		if (over.pic)
			over.bmp = _G(gfxDriver)->CreateDDBFromBitmap(over.pic, false);
		else
			over.bmp = nullptr;
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// ScriptOverlay* (int x, int y, int slot, int transparent)
RuntimeScriptValue Sc_Overlay_CreateGraphical(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT4(ScriptOverlay, Overlay_CreateGraphical);
}

// ScriptOverlay* (int x, int y, int width, int font, int colour, const char* text, ...)
RuntimeScriptValue Sc_Overlay_CreateTextual(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(Overlay_CreateTextual, 6);
	ScriptOverlay *overlay = Overlay_CreateTextual(params[0].IValue, params[1].IValue, params[2].IValue,
	                         params[3].IValue, params[4].IValue, scsf_buffer);
	return RuntimeScriptValue().SetDynamicObject(overlay, overlay);
}

// void (ScriptOverlay *scover, int wii, int fontid, int clr, char*texx, ...)
RuntimeScriptValue Sc_Overlay_SetText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_SCRIPT_SPRINTF(Overlay_SetText, 4);
	Overlay_SetText((ScriptOverlay *)self, params[0].IValue, params[1].IValue, params[2].IValue, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void (ScriptOverlay *sco)
RuntimeScriptValue Sc_Overlay_Remove(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptOverlay, Overlay_Remove);
}

// int (ScriptOverlay *scover)
RuntimeScriptValue Sc_Overlay_GetValid(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetValid);
}

// int (ScriptOverlay *scover)
RuntimeScriptValue Sc_Overlay_GetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetX);
}

// void (ScriptOverlay *scover, int newx)
RuntimeScriptValue Sc_Overlay_SetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetX);
}

// int (ScriptOverlay *scover)
RuntimeScriptValue Sc_Overlay_GetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetY);
}

// void (ScriptOverlay *scover, int newy)
RuntimeScriptValue Sc_Overlay_SetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetY);
}

RuntimeScriptValue Sc_Overlay_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetWidth);
}

RuntimeScriptValue Sc_Overlay_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetHeight);
}

RuntimeScriptValue Sc_Overlay_GetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetTransparency);
}

RuntimeScriptValue Sc_Overlay_SetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetTransparency);
}

RuntimeScriptValue Sc_Overlay_GetZOrder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetZOrder);
}

RuntimeScriptValue Sc_Overlay_SetZOrder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetZOrder);
}

//=============================================================================
//
// Exclusive API for Plugins
//
//=============================================================================

// void (ScriptOverlay *scover, int wii, int fontid, int clr, char*texx, ...)
void ScPl_Overlay_SetText(ScriptOverlay *scover, int wii, int fontid, int clr, char *texx, ...) {
	API_PLUGIN_SCRIPT_SPRINTF(texx);
	Overlay_SetText(scover, wii, fontid, clr, scsf_buffer);
}


void RegisterOverlayAPI() {
	ccAddExternalStaticFunction("Overlay::CreateGraphical^4", Sc_Overlay_CreateGraphical);
	ccAddExternalStaticFunction("Overlay::CreateTextual^106", Sc_Overlay_CreateTextual);
	ccAddExternalObjectFunction("Overlay::SetText^104", Sc_Overlay_SetText);
	ccAddExternalObjectFunction("Overlay::Remove^0", Sc_Overlay_Remove);
	ccAddExternalObjectFunction("Overlay::get_Valid", Sc_Overlay_GetValid);
	ccAddExternalObjectFunction("Overlay::get_X", Sc_Overlay_GetX);
	ccAddExternalObjectFunction("Overlay::set_X", Sc_Overlay_SetX);
	ccAddExternalObjectFunction("Overlay::get_Y", Sc_Overlay_GetY);
	ccAddExternalObjectFunction("Overlay::set_Y", Sc_Overlay_SetY);
	ccAddExternalObjectFunction("Overlay::get_Width", Sc_Overlay_GetWidth);
	ccAddExternalObjectFunction("Overlay::get_Height", Sc_Overlay_GetHeight);
	ccAddExternalObjectFunction("Overlay::get_Transparency", Sc_Overlay_GetTransparency);
	ccAddExternalObjectFunction("Overlay::set_Transparency", Sc_Overlay_SetTransparency);
	ccAddExternalObjectFunction("Overlay::get_ZOrder", Sc_Overlay_GetZOrder);
	ccAddExternalObjectFunction("Overlay::set_ZOrder", Sc_Overlay_SetZOrder);
}

} // namespace AGS3
