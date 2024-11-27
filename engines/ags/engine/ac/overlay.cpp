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
#include "ags/engine/ac/overlay.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/sprite_cache.h"
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
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/debugging/debug_log.h"
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

void Overlay_SetText(ScriptOverlay *scover, int width, int fontid, int text_color, const char *text) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!Overlay.SetText: invalid overlay ID specified");
	Overlay_SetText(*over, over->x, over->y, width, fontid, text_color, text);
}

void Overlay_SetText(ScreenOverlay &over, int x, int y, int width, int fontid, int text_color, const char *text) {
	// TODO: find a nice way to refactor and share these code pieces
	// from CreateTextOverlay
	width = data_to_game_coord(width);
	// allow DisplaySpeechBackground to be shrunk
	int allow_shrink = (x == OVR_AUTOPLACE) ? 1 : 0;

	// from Overlay_CreateTextCore
	if (width < 8) width = _GP(play).GetUIViewport().GetWidth() / 2;

	if (text_color == 0) text_color = 16;

	// Recreate overlay image
	int dummy_x = x, dummy_y = y, adj_x = x, adj_y = y;
	bool has_alpha = false;
	// NOTE: we pass text_color negated to let optionally use textwindow (if applicable)
	// this is a generic ugliness of _display_main args, need to refactor later.
	Bitmap *image = create_textual_image(get_translation(text), -text_color, 0, dummy_x, dummy_y, adj_x, adj_y,
										 width, fontid, allow_shrink, has_alpha);

	// Update overlay properties
	over.SetImage(std::unique_ptr<Bitmap>(image), has_alpha, adj_x - dummy_x, adj_y - dummy_y);
}

int Overlay_GetX(ScriptOverlay *scover) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");

	Point pos = get_overlay_position(*over);
	return game_to_data_coord(pos.X);
}

void Overlay_SetX(ScriptOverlay *scover, int newx) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");

	over->x = data_to_game_coord(newx);
}

int Overlay_GetY(ScriptOverlay *scover) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");

	Point pos = get_overlay_position(*over);
	return game_to_data_coord(pos.Y);
}

void Overlay_SetY(ScriptOverlay *scover, int newy) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");

	over->y = data_to_game_coord(newy);
}

int Overlay_GetGraphic(ScriptOverlay *scover) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");
	return over->GetSpriteNum();
}

void Overlay_SetGraphic(ScriptOverlay *scover, int slot) {
	if (!_GP(spriteset).DoesSpriteExist(slot)) {
		debug_script_warn("Overlay.SetGraphic: sprite %d is invalid", slot);
		slot = 0;
	}
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");
	over->SetSpriteNum(slot);
}

bool Overlay_InRoom(ScriptOverlay *scover) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");
	return over->IsRoomLayer();
}

int Overlay_GetWidth(ScriptOverlay *scover) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");
	return game_to_data_coord(over->scaleWidth);
}

int Overlay_GetHeight(ScriptOverlay *scover) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");
	return game_to_data_coord(over->scaleHeight);
}

int Overlay_GetGraphicWidth(ScriptOverlay *scover) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");
	return game_to_data_coord(over->GetGraphicSize().Width);
}

int Overlay_GetGraphicHeight(ScriptOverlay *scover) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");
	return game_to_data_coord(over->GetGraphicSize().Height);
}

void Overlay_SetScaledSize(ScreenOverlay &over, int width, int height) {
	data_to_game_coords(&width, &height);
	if (width < 1 || height < 1) {
		debug_script_warn("Overlay.SetSize: invalid dimensions: %d x %d", width, height);
		return;
	}
	if ((width == over.scaleWidth) && (height == over.scaleHeight))
		return; // no change
	over.scaleWidth = width;
	over.scaleHeight = height;
	over.MarkChanged();
}

void Overlay_SetWidth(ScriptOverlay *scover, int width) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");
	Overlay_SetScaledSize(*over, width, game_to_data_coord(over->scaleHeight));
}

void Overlay_SetHeight(ScriptOverlay *scover, int height) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");
	Overlay_SetScaledSize(*over, game_to_data_coord(over->scaleWidth), height);
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

ScreenOverlay *Overlay_CreateGraphicCore(bool room_layer, int x, int y, int slot, bool transparent, bool clone) {
	if (!_GP(spriteset).DoesSpriteExist(slot)) {
		debug_script_warn("Overlay.CreateGraphical: sprite %d is invalid", slot);
		return nullptr;
	}

	data_to_game_coords(&x, &y);
	size_t overid;
	// We clone only dynamic sprites, because it makes no sense to clone normal ones
	if (clone && (_GP(game).SpriteInfos[slot].Flags & SPF_DYNAMICALLOC) != 0) {
		Bitmap *screeno = BitmapHelper::CreateTransparentBitmap(_GP(game).SpriteInfos[slot].Width, _GP(game).SpriteInfos[slot].Height, _GP(game).GetColorDepth());
		screeno->Blit(_GP(spriteset)[slot], 0, 0, transparent ? kBitmap_Transparency : kBitmap_Copy);
		overid = add_screen_overlay(room_layer, x, y, OVER_CUSTOM, screeno,
			(_GP(game).SpriteInfos[slot].Flags & SPF_ALPHACHANNEL) != 0);
	} else {
		overid = add_screen_overlay(room_layer, x, y, OVER_CUSTOM, slot);
	}
	return overid < SIZE_MAX ? &_GP(screenover)[overid] : nullptr;
}

ScreenOverlay *Overlay_CreateTextCore(bool room_layer, int x, int y, int width, int font, int text_color,
	const char *text, int disp_type, int allow_shrink) {
	if (width < 8) width = _GP(play).GetUIViewport().GetWidth() / 2;
	if (x < 0) x = _GP(play).GetUIViewport().GetWidth() / 2 - width / 2;
	if (text_color == 0) text_color = 16;
	return display_main(x, y, width, text, disp_type, font, -text_color, 0, allow_shrink, false, room_layer);
}

ScriptOverlay *Overlay_CreateGraphicalImpl(bool room_layer, int x, int y, int slot, bool transparent, bool clone) {
	auto *over = Overlay_CreateGraphicCore(room_layer, x, y, slot, transparent, clone);
	return over ? create_scriptoverlay(*over) : nullptr;
}

ScriptOverlay *Overlay_CreateGraphical4(int x, int y, int slot, bool transparent) {
	return Overlay_CreateGraphical(x, y, slot, transparent, true /* clone */);
}

ScriptOverlay *Overlay_CreateGraphical(int x, int y, int slot, bool transparent, bool clone) {
	return Overlay_CreateGraphicalImpl(false, x, y, slot, transparent, clone);
}

ScriptOverlay *Overlay_CreateRoomGraphical(int x, int y, int slot, bool transparent, bool clone) {
	return Overlay_CreateGraphicalImpl(true, x, y, slot, transparent, clone);
}

ScriptOverlay *Overlay_CreateTextualImpl(bool room_layer, int x, int y, int width, int font, int colour, const char *text) {
	data_to_game_coords(&x, &y);
	width = data_to_game_coord(width);
	auto *over = Overlay_CreateTextCore(room_layer, x, y, width, font, colour, text, DISPLAYTEXT_NORMALOVERLAY, 0);
	return over ? create_scriptoverlay(*over) : nullptr;
}

ScriptOverlay *Overlay_CreateTextual(int x, int y, int width, int font, int colour, const char *text) {
	return Overlay_CreateTextualImpl(false, x, y, width, font, colour, text);
}

ScriptOverlay *Overlay_CreateRoomTextual(int x, int y, int width, int font, int colour, const char *text) {
	return Overlay_CreateTextualImpl(true, x, y, width, font, colour, text);
}

int Overlay_GetTransparency(ScriptOverlay *scover) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");

	return GfxDef::LegacyTrans255ToTrans100(over->transparency);
}

void Overlay_SetTransparency(ScriptOverlay *scover, int trans) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");
	if ((trans < 0) | (trans > 100))
		quit("!SetTransparency: transparency value must be between 0 and 100");

	over->transparency = GfxDef::Trans100ToLegacyTrans255(trans);
}

int Overlay_GetZOrder(ScriptOverlay *scover) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");

	return over->zorder;
}

void Overlay_SetZOrder(ScriptOverlay *scover, int zorder) {
	auto *over = get_overlay(scover->overlayId);
	if (!over)
		quit("!invalid overlay ID specified");

	over->zorder = zorder;
}

//=============================================================================

// Creates and registers a managed script object for existing overlay object
// optionally adds an internal engine reference to prevent object's disposal
ScriptOverlay *create_scriptoverlay(ScreenOverlay &over, bool internal_ref) {
	ScriptOverlay *scover = new ScriptOverlay();
	scover->overlayId = over.type;
	int handl = ccRegisterManagedObject(scover, scover);
	over.associatedOverlayHandle = handl; // save the handle for access
	if (internal_ref) // requested additional ref
		ccAddObjectReference(handl);
	return scover;
}

// Invalidates existing script object to let user know that previous overlay is gone,
// and releases engine's internal reference (script object may exist while there are user refs)
static void invalidate_and_subref(ScreenOverlay &over) {
	if (over.associatedOverlayHandle <= 0)
		return; // invalid handle

	ScriptOverlay *scover = const_cast<ScriptOverlay*>((const ScriptOverlay*)ccGetObjectAddressFromHandle(over.associatedOverlayHandle));
	if (scover) {
		scover->overlayId = -1; // invalidate script object
		ccReleaseObjectReference(over.associatedOverlayHandle);
	}
	over.associatedOverlayHandle = 0; // reset internal handle
}

// Frees overlay resources and tell to dispose script object if there are no refs left
static void dispose_overlay(ScreenOverlay &over) {
	over.SetImage(nullptr);
	// invalidate script object and dispose it if there are no more refs
	if (over.associatedOverlayHandle > 0) {
		ScriptOverlay *scover = (ScriptOverlay *)ccGetObjectAddressFromHandle(over.associatedOverlayHandle);
		if (scover) scover->overlayId = -1;
		ccAttemptDisposeObject(over.associatedOverlayHandle);
	}
}

void remove_screen_overlay(int type) {
	if (type < 0 || static_cast<uint32_t>(type) >= _GP(screenover).size() || _GP(screenover)[type].type < 0)
		return; // something is wrong

	ScreenOverlay &over = _GP(screenover)[type];
	// TODO: move these custom settings outside of this function
	if (over.type == _GP(play).complete_overlay_on) {
		_GP(play).complete_overlay_on = 0;
	} else if (over.type == _GP(play).text_overlay_on) { // release internal ref for speech text
		invalidate_and_subref(over);
		_GP(play).speech_text_schandle = 0;
		_GP(play).text_overlay_on = 0;
	} else if (over.type == OVER_PICTURE) { // release internal ref for speech face
		invalidate_and_subref(over);
		_GP(play).speech_face_schandle = 0;
		_G(face_talking) = -1;
	} else if (over.bgSpeechForChar >= 0) { // release internal ref for bg speech
		invalidate_and_subref(over);
	}
	dispose_overlay(over);

	// Don't erase vector elements, instead set invalid and record free index
	_GP(screenover)[type] = ScreenOverlay();
	if (type >= OVER_FIRSTFREE)
		_GP(over_free_ids).push(type);

    reset_drawobj_for_overlay(type);

	// If all overlays have been removed, reset creation index (helps vs overflows)
	_GP(play).overlay_count--;
	if (_GP(play).overlay_count == 0)
		_GP(play).overlay_creation_id = 0;
}

void remove_all_overlays() {
	for (auto &over : _GP(screenover))
		remove_screen_overlay(over.type);
}

ScreenOverlay *get_overlay(int type) {
	return (type >= 0 && static_cast<uint32_t>(type) < _GP(screenover).size() && _GP(screenover)[type].type >= 0) ? &_GP(screenover)[type] : nullptr;
}

size_t add_screen_overlay_impl(bool roomlayer, int x, int y, int type, int sprnum, Bitmap *piccy,
							   int pic_offx, int pic_offy, bool has_alpha) {
	if (type == OVER_CUSTOM) {
		// Find a free ID
		if (_GP(over_free_ids).size() > 0) {
			type = _GP(over_free_ids).front();
			_GP(over_free_ids).pop();
		} else {
			type = std::max((uint)OVER_FIRSTFREE, _GP(screenover).size());
		}
	}

	if (_GP(screenover).size() <= static_cast<uint32_t>(type))
		_GP(screenover).resize(type + 1);
	ScreenOverlay over;
	over.type = type;
	over.creation_id = _GP(play).overlay_creation_id++;
	if (piccy) {
		over.SetImage(std::unique_ptr<Bitmap>(piccy), has_alpha, pic_offx, pic_offy);
	} else {
		over.SetSpriteNum(sprnum, pic_offx, pic_offy);
	}
	over.x = x;
	over.y = y;
	// by default draw speech and portraits over GUI, and the rest under GUI
	over.zorder = (roomlayer || type == OVER_TEXTMSG || type == OVER_PICTURE || type == OVER_TEXTSPEECH) ?
		INT_MAX : INT_MIN;
	over.timeout = 0;
	over.bgSpeechForChar = -1;
	over.associatedOverlayHandle = 0;
	over.SetRoomLayer(roomlayer);
	// TODO: move these custom settings outside of this function
	if (type == OVER_COMPLETE) _GP(play).complete_overlay_on = type;
	else if (type == OVER_TEXTMSG || type == OVER_TEXTSPEECH) {
		_GP(play).text_overlay_on = type;
		// only make script object for blocking speech now, because messagebox blocks all script
		// and therefore cannot be accessed, so no practical reason for that atm
		if (type == OVER_TEXTSPEECH) {
			create_scriptoverlay(over, true);
			_GP(play).speech_text_schandle = over.associatedOverlayHandle;
		}
	} else if (type == OVER_PICTURE) {
		create_scriptoverlay(over, true);
		_GP(play).speech_face_schandle = over.associatedOverlayHandle;
	}
	over.MarkChanged();
	_GP(screenover)[type] = std::move(over);
	_GP(play).overlay_count++;
	return type;
}

size_t add_screen_overlay(bool roomlayer, int x, int y, int type, int sprnum) {
	return add_screen_overlay_impl(roomlayer, x, y, type, sprnum, nullptr, 0, 0, false);
}

size_t add_screen_overlay(bool roomlayer, int x, int y, int type, Bitmap *piccy, bool has_alpha) {
	return add_screen_overlay_impl(roomlayer, x, y, type, -1, piccy, 0, 0, has_alpha);
}

size_t add_screen_overlay(bool roomlayer, int x, int y, int type, Shared::Bitmap *piccy, int pic_offx, int pic_offy, bool has_alpha) {
	return add_screen_overlay_impl(roomlayer, x, y, type, -1, piccy, pic_offx, pic_offy, has_alpha);
}

Point get_overlay_position(const ScreenOverlay &over) {
	if (over.IsRoomLayer()) {
		return Point(over.x + over.offsetX, over.y + over.offsetY);
	}

	if (over.x == OVR_AUTOPLACE) {
		const Rect &ui_view = _GP(play).GetUIViewport();
		// auto place on character
		int charid = over.y;

		auto view = FindNearestViewport(charid);
		const int charpic = _GP(views)[_GP(game).chars[charid].view].loops[_GP(game).chars[charid].loop].frames[0].pic;
		const int height = (_GP(charextra)[charid].height < 1) ? _GP(game).SpriteInfos[charpic].Height : _GP(charextra)[charid].height;
		const Point screenpt = view->RoomToScreen(
			data_to_game_coord(_GP(game).chars[charid].x),
			data_to_game_coord(_GP(charextra)[charid].GetEffectiveY(&_GP(game).chars[charid])) - height).first;
		const Size pic_size = over.GetGraphicSize();
		int tdxp = std::max(0, screenpt.X - pic_size.Width / 2);
		int tdyp = screenpt.Y - get_fixed_pixel_size(5);
		tdyp -= pic_size.Height;
		tdyp = std::max(5, tdyp);

		if ((tdxp + pic_size.Width) >= ui_view.GetWidth())
			tdxp = (ui_view.GetWidth() - pic_size.Width) - 1;
		if (_GP(game).chars[charid].room != _G(displayed_room)) {
			tdxp = ui_view.GetWidth() / 2 - pic_size.Width / 2;
			tdyp = ui_view.GetHeight() / 2 - pic_size.Height / 2;
		}
		return Point(tdxp, tdyp);
	} else {
		// Note: the internal offset is only needed when x,y coordinates are specified
		// and only in the case where the overlay is using a GUI. See issue #1098
		int tdxp = over.x + over.offsetX;
		int tdyp = over.y + over.offsetY;
		if (over.IsRoomRelative())
			return _GP(play).RoomToScreen(tdxp, tdyp);
		return Point(tdxp, tdyp);
	}
}

void restore_overlays() {
	// Will have to readjust free ids records, as overlays may be restored in any random slots
	while (!_GP(over_free_ids).empty()) {
		_GP(over_free_ids).pop();
	}
	for (size_t i = 0; i < _GP(screenover).size(); ++i) {
		auto &over = _GP(screenover)[i];
		if (over.type >= 0) {
			over.MarkChanged(); // force recreate texture on next draw
		} else if (i >= OVER_FIRSTFREE) {
			_GP(over_free_ids).push(i);
		}
	}
}

std::vector<ScreenOverlay> &get_overlays() {
	return _GP(screenover);
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// ScriptOverlay* (int x, int y, int slot, int transparent)
RuntimeScriptValue Sc_Overlay_CreateGraphical4(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT3_PBOOL(ScriptOverlay, Overlay_CreateGraphical4);
}

RuntimeScriptValue Sc_Overlay_CreateGraphical(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT3_PBOOL2(ScriptOverlay, Overlay_CreateGraphical);
}

RuntimeScriptValue Sc_Overlay_CreateRoomGraphical(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT3_PBOOL2(ScriptOverlay, Overlay_CreateRoomGraphical);
}

// ScriptOverlay* (int x, int y, int width, int font, int colour, const char* text, ...)
RuntimeScriptValue Sc_Overlay_CreateTextual(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(Overlay_CreateTextual, 6);
	ScriptOverlay *overlay = Overlay_CreateTextual(params[0].IValue, params[1].IValue, params[2].IValue,
												   params[3].IValue, params[4].IValue, scsf_buffer);
	return RuntimeScriptValue().SetScriptObject(overlay, overlay);
}

RuntimeScriptValue Sc_Overlay_CreateRoomTextual(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(Overlay_CreateRoomTextual, 6);
	ScriptOverlay *overlay = Overlay_CreateRoomTextual(params[0].IValue, params[1].IValue, params[2].IValue,
													   params[3].IValue, params[4].IValue, scsf_buffer);
	return RuntimeScriptValue().SetScriptObject(overlay, overlay);
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

RuntimeScriptValue Sc_Overlay_GetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetGraphic);
}

RuntimeScriptValue Sc_Overlay_SetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetGraphic);
}

RuntimeScriptValue Sc_Overlay_InRoom(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(ScriptOverlay, Overlay_InRoom);
}

RuntimeScriptValue Sc_Overlay_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetWidth);
}

RuntimeScriptValue Sc_Overlay_SetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetWidth);
}

RuntimeScriptValue Sc_Overlay_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetHeight);
}

RuntimeScriptValue Sc_Overlay_SetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetHeight);
}

RuntimeScriptValue Sc_Overlay_GetGraphicWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetGraphicWidth);
}

RuntimeScriptValue Sc_Overlay_GetGraphicHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptOverlay, Overlay_GetGraphicHeight);
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
// Exclusive variadic API implementation for Plugins
//
//=============================================================================

ScriptOverlay *ScPl_Overlay_CreateRoomTextual(int x, int y, int width, int font, int colour, const char *text, ...) {
	API_PLUGIN_SCRIPT_SPRINTF(text);
	return Overlay_CreateRoomTextual(x, y, width, font, colour, scsf_buffer);
}

// void (ScriptOverlay *scover, int wii, int fontid, int clr, char*texx, ...)
void ScPl_Overlay_SetText(ScriptOverlay *scover, int wii, int fontid, int clr, char *texx, ...) {
	API_PLUGIN_SCRIPT_SPRINTF(texx);
	Overlay_SetText(scover, wii, fontid, clr, scsf_buffer);
}

void RegisterOverlayAPI() {
	ScFnRegister overlay_api[] = {
		{"Overlay::CreateGraphical^4", API_FN_PAIR(Overlay_CreateGraphical4)},
		{"Overlay::CreateGraphical^5", API_FN_PAIR(Overlay_CreateGraphical)},
		{"Overlay::CreateTextual^106", Sc_Overlay_CreateTextual},
		{"Overlay::CreateRoomGraphical^5", API_FN_PAIR(Overlay_CreateRoomGraphical)},
		{"Overlay::CreateRoomTextual^106", Sc_Overlay_CreateRoomTextual},

		{"Overlay::SetText^104", Sc_Overlay_SetText},
		{"Overlay::Remove^0", API_FN_PAIR(Overlay_Remove)},
		{"Overlay::get_Valid", API_FN_PAIR(Overlay_GetValid)},
		{"Overlay::get_X", API_FN_PAIR(Overlay_GetX)},
		{"Overlay::set_X", API_FN_PAIR(Overlay_SetX)},
		{"Overlay::get_Y", API_FN_PAIR(Overlay_GetY)},
		{"Overlay::set_Y", API_FN_PAIR(Overlay_SetY)},
		{"Overlay::get_Graphic", API_FN_PAIR(Overlay_GetGraphic)},
		{"Overlay::set_Graphic", API_FN_PAIR(Overlay_SetGraphic)},
		{"Overlay::get_InRoom", API_FN_PAIR(Overlay_InRoom)},
		{"Overlay::get_Width", API_FN_PAIR(Overlay_GetWidth)},
		{"Overlay::set_Width", API_FN_PAIR(Overlay_SetWidth)},
		{"Overlay::get_Height", API_FN_PAIR(Overlay_GetHeight)},
		{"Overlay::set_Height", API_FN_PAIR(Overlay_SetHeight)},
		{"Overlay::get_GraphicWidth", API_FN_PAIR(Overlay_GetGraphicWidth)},
		{"Overlay::get_GraphicHeight", API_FN_PAIR(Overlay_GetGraphicHeight)},
		{"Overlay::get_Transparency", API_FN_PAIR(Overlay_GetTransparency)},
		{"Overlay::set_Transparency", API_FN_PAIR(Overlay_SetTransparency)},
		{"Overlay::get_ZOrder", API_FN_PAIR(Overlay_GetZOrder)},
		{"Overlay::set_ZOrder", API_FN_PAIR(Overlay_SetZOrder)},
	};

	ccAddExternalFunctions361(overlay_api);
}

} // namespace AGS3
