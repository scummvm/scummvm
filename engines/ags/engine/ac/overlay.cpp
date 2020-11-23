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

#include "ags/shared/ac/overlay.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/view.h"
#include "ags/shared/ac/character.h"
#include "ags/shared/ac/characterextras.h"
#include "ags/shared/ac/display.h"
#include "ags/shared/ac/draw.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/shared/ac/gamestate.h"
#include "ags/shared/ac/global_overlay.h"
#include "ags/shared/ac/global_translation.h"
#include "ags/shared/ac/runtime_defines.h"
#include "ags/shared/ac/screenoverlay.h"
#include "ags/shared/ac/string.h"
#include "ags/shared/gfx/graphicsdriver.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/script/runtimescriptvalue.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern int displayed_room;
extern int face_talking;
extern ViewStruct *views;
extern CharacterExtras *charextra;
extern IGraphicsDriver *gfxDriver;



std::vector<ScreenOverlay> screenover;
int is_complete_overlay = 0, is_text_overlay = 0;

void Overlay_Remove(ScriptOverlay *sco) {
	sco->Remove();
}

void Overlay_SetText(ScriptOverlay *scover, int wii, int fontid, int text_color, const char *text) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!Overlay.SetText: invalid overlay ID specified");
	int xx = game_to_data_coord(screenover[ovri].x) - scover->borderWidth;
	int yy = game_to_data_coord(screenover[ovri].y) - scover->borderHeight;

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
	get_overlay_position(screenover[ovri], &tdxp, &tdyp);

	return game_to_data_coord(tdxp);
}

void Overlay_SetX(ScriptOverlay *scover, int newx) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");

	screenover[ovri].x = data_to_game_coord(newx);
}

int Overlay_GetY(ScriptOverlay *scover) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");

	int tdxp, tdyp;
	get_overlay_position(screenover[ovri], &tdxp, &tdyp);

	return game_to_data_coord(tdyp);
}

void Overlay_SetY(ScriptOverlay *scover, int newy) {
	int ovri = find_overlay_of_type(scover->overlayId);
	if (ovri < 0)
		quit("!invalid overlay ID specified");

	screenover[ovri].y = data_to_game_coord(newy);
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
	sco->borderWidth = game_to_data_coord(screenover[ovri].x - x);
	sco->borderHeight = game_to_data_coord(screenover[ovri].y - y);
	sco->isBackgroundSpeech = 0;

	ccRegisterManagedObject(sco, sco);
	return sco;
}

//=============================================================================

void dispose_overlay(ScreenOverlay &over) {
	delete over.pic;
	over.pic = nullptr;
	if (over.bmp != nullptr)
		gfxDriver->DestroyDDB(over.bmp);
	over.bmp = nullptr;
	// if the script didn't actually use the Overlay* return
	// value, dispose of the pointer
	if (over.associatedOverlayHandle)
		ccAttemptDisposeObject(over.associatedOverlayHandle);
}

void remove_screen_overlay_index(size_t over_idx) {
	ScreenOverlay &over = screenover[over_idx];
	dispose_overlay(over);
	if (over.type == OVER_COMPLETE) is_complete_overlay--;
	if (over.type == OVER_TEXTMSG) is_text_overlay--;
	screenover.erase(screenover.begin() + over_idx);
	// if an overlay before the sierra-style speech one is removed,
	// update the index
	if (face_talking >= 0 && (size_t)face_talking > over_idx)
		face_talking--;
}

void remove_screen_overlay(int type) {
	for (size_t i = 0; i < screenover.size();) {
		if (type < 0 || screenover[i].type == type)
			remove_screen_overlay_index(i);
		else
			i++;
	}
}

int find_overlay_of_type(int type) {
	for (size_t i = 0; i < screenover.size(); ++i) {
		if (screenover[i].type == type) return i;
	}
	return -1;
}

size_t add_screen_overlay(int x, int y, int type, Bitmap *piccy, bool alphaChannel) {
	return add_screen_overlay(x, y, type, piccy, 0, 0, alphaChannel);
}

size_t add_screen_overlay(int x, int y, int type, Shared::Bitmap *piccy, int pic_offx, int pic_offy, bool alphaChannel) {
	if (type == OVER_COMPLETE) is_complete_overlay++;
	if (type == OVER_TEXTMSG) is_text_overlay++;
	if (type == OVER_CUSTOM) {
		// find an unused custom ID; TODO: find a better approach!
		for (int id = OVER_CUSTOM + 1; id < screenover.size() + OVER_CUSTOM + 1; ++id) {
			if (find_overlay_of_type(id) == -1) {
				type = id;
				break;
			}
		}
	}
	ScreenOverlay over;
	over.pic = piccy;
	over.bmp = gfxDriver->CreateDDBFromBitmap(piccy, alphaChannel);
	over.x = x;
	over.y = y;
	over._offsetX = pic_offx;
	over._offsetY = pic_offy;
	over.type = type;
	over.timeout = 0;
	over.bgSpeechForChar = -1;
	over.associatedOverlayHandle = 0;
	over.hasAlphaChannel = alphaChannel;
	over.positionRelativeToScreen = true;
	screenover.push_back(over);
	return screenover.size() - 1;
}



void get_overlay_position(const ScreenOverlay &over, int *x, int *y) {
	int tdxp, tdyp;
	const Rect &ui_view = play.GetUIViewport();

	if (over.x == OVR_AUTOPLACE) {
		// auto place on character
		int charid = over.y;

		auto view = FindNearestViewport(charid);
		const int charpic = views[game.chars[charid].view].loops[game.chars[charid].loop].frames[0].pic;
		const int height = (charextra[charid].height < 1) ? game.SpriteInfos[charpic].Height : charextra[charid].height;
		Point screenpt = view->RoomToScreen(
			data_to_game_coord(game.chars[charid].x),
			data_to_game_coord(game.chars[charid].get_effective_y()) - height).first;
		tdxp = screenpt.X - over.pic->GetWidth() / 2;
		if (tdxp < 0) tdxp = 0;
		tdyp = screenpt.Y - get_fixed_pixel_size(5);
		tdyp -= over.pic->GetHeight();
		if (tdyp < 5) tdyp = 5;

		if ((tdxp + over.pic->GetWidth()) >= ui_view.GetWidth())
			tdxp = (ui_view.GetWidth() - over.pic->GetWidth()) - 1;
		if (game.chars[charid].room != displayed_room) {
			tdxp = ui_view.GetWidth() / 2 - over.pic->GetWidth() / 2;
			tdyp = ui_view.GetHeight() / 2 - over.pic->GetHeight() / 2;
		}
	} else {
		// Note: the internal offset is only needed when x,y coordinates are specified
		// and only in the case where the overlay is using a GUI. See issue #1098
		tdxp = over.x + over._offsetX;
		tdyp = over.y + over._offsetY;

		if (!over.positionRelativeToScreen) {
			Point tdxy = play.RoomToScreen(tdxp, tdyp);
			tdxp = tdxy.X;
			tdyp = tdxy.Y;
		}
	}
	*x = tdxp;
	*y = tdyp;
}

void recreate_overlay_ddbs() {
	for (auto &over : screenover) {
		if (over.bmp)
			gfxDriver->DestroyDDB(over.bmp);
		if (over.pic)
			over.bmp = gfxDriver->CreateDDBFromBitmap(over.pic, false);
		else
			over.bmp = nullptr;
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

#include "ags/shared/debug/out.h"
#include "ags/shared/script/script_api.h"
#include "ags/shared/script/script_runtime.h"

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

//=============================================================================
//
// Exclusive API for Plugins
//
//=============================================================================

// ScriptOverlay* (int x, int y, int width, int font, int colour, const char* text, ...)
ScriptOverlay *ScPl_Overlay_CreateTextual(int x, int y, int width, int font, int colour, const char *text, ...) {
	API_PLUGIN_SCRIPT_SPRINTF(text);
	return Overlay_CreateTextual(x, y, width, font, colour, scsf_buffer);
}

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

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("Overlay::CreateGraphical^4", (void *)Overlay_CreateGraphical);
	ccAddExternalFunctionForPlugin("Overlay::CreateTextual^106", (void *)ScPl_Overlay_CreateTextual);
	ccAddExternalFunctionForPlugin("Overlay::SetText^104", (void *)ScPl_Overlay_SetText);
	ccAddExternalFunctionForPlugin("Overlay::Remove^0", (void *)Overlay_Remove);
	ccAddExternalFunctionForPlugin("Overlay::get_Valid", (void *)Overlay_GetValid);
	ccAddExternalFunctionForPlugin("Overlay::get_X", (void *)Overlay_GetX);
	ccAddExternalFunctionForPlugin("Overlay::set_X", (void *)Overlay_SetX);
	ccAddExternalFunctionForPlugin("Overlay::get_Y", (void *)Overlay_GetY);
	ccAddExternalFunctionForPlugin("Overlay::set_Y", (void *)Overlay_SetY);
}

} // namespace AGS3
