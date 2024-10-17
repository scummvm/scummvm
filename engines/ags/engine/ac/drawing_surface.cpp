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

#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/drawing_surface.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/room_object.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/walk_behind.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/shared/gfx/gfx_def.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

// ** SCRIPT DRAWINGSURFACE OBJECT

void DrawingSurface_Release(ScriptDrawingSurface *sds) {
	if (sds->roomBackgroundNumber >= 0) {
		if (sds->modified) {
			if (sds->roomBackgroundNumber == _GP(play).bg_frame) {
				invalidate_screen();
				mark_current_background_dirty();
			}
			_GP(play).raw_modified[sds->roomBackgroundNumber] = 1;
		}

		sds->roomBackgroundNumber = -1;
	}
	if (sds->roomMaskType > kRoomAreaNone) {
		if (sds->roomMaskType == kRoomAreaWalkBehind) {
			walkbehinds_recalc();
		}
		sds->roomMaskType = kRoomAreaNone;
	}
	if (sds->dynamicSpriteNumber >= 0) {
		if (sds->modified) {
			game_sprite_updated(sds->dynamicSpriteNumber);
		}

		sds->dynamicSpriteNumber = -1;
	}
	if (sds->dynamicSurfaceNumber >= 0) {
		_G(dynamicallyCreatedSurfaces)[sds->dynamicSurfaceNumber].reset();
		sds->dynamicSurfaceNumber = -1;
	}
	sds->modified = 0;
}

void ScriptDrawingSurface::PointToGameResolution(int *xcoord, int *ycoord) {
	ctx_data_to_game_coord(*xcoord, *ycoord, highResCoordinates != 0);
}

void ScriptDrawingSurface::SizeToGameResolution(int *width, int *height) {
	ctx_data_to_game_size(*width, *height, highResCoordinates != 0);
}

void ScriptDrawingSurface::SizeToGameResolution(int *valueToAdjust) {
	*valueToAdjust = ctx_data_to_game_size(*valueToAdjust, highResCoordinates != 0);
}

// convert actual co-ordinate back to what the script is expecting
void ScriptDrawingSurface::SizeToDataResolution(int *valueToAdjust) {
	*valueToAdjust = game_to_ctx_data_size(*valueToAdjust, highResCoordinates != 0);
}

ScriptDrawingSurface *DrawingSurface_CreateCopy(ScriptDrawingSurface *sds) {
	Bitmap *sourceBitmap = sds->GetBitmapSurface();

	for (int i = 0; i < MAX_DYNAMIC_SURFACES; i++) {
		if (_G(dynamicallyCreatedSurfaces)[i] == nullptr) {
			_G(dynamicallyCreatedSurfaces)[i].reset(BitmapHelper::CreateBitmapCopy(sourceBitmap));
			ScriptDrawingSurface *newSurface = new ScriptDrawingSurface();
			newSurface->dynamicSurfaceNumber = i;
			newSurface->hasAlphaChannel = sds->hasAlphaChannel;
			ccRegisterManagedObject(newSurface, newSurface);
			return newSurface;
		}
	}

	quit("!DrawingSurface.CreateCopy: too many copied surfaces created");
	return nullptr;
}

void DrawingSurface_DrawImageImpl(ScriptDrawingSurface *sds, Bitmap *src,
	int dst_x, int dst_y, int trans, int dst_width, int dst_height,
	int src_x, int src_y, int src_width, int src_height, int sprite_id, bool src_has_alpha) {
	Bitmap *ds = sds->GetBitmapSurface();
	if (src == ds) {
	} // ignore for now; bitmap lib supports, and may be used for effects
	/* debug_script_warn("DrawingSurface.DrawImage: drawing onto itself"); */
	if (src->GetColorDepth() != ds->GetColorDepth()) {
		if (sprite_id >= 0)
			debug_script_warn("DrawImage: Sprite %d colour depth %d-bit not same as destination depth %d-bit", sprite_id, src->GetColorDepth(), ds->GetColorDepth());
		else
			debug_script_warn("DrawImage: Source image colour depth %d-bit not same as destination depth %d-bit", src->GetColorDepth(), ds->GetColorDepth());
	}
	if ((trans < 0) || (trans > 100))
		debug_script_warn("DrawingSurface.DrawImage: invalid transparency %d, range is %d - %d", trans, 0, 100);
	trans = Math::Clamp(trans, 0, 100);

	if (trans == 100)
		return; // fully transparent
	if (dst_width < 1 || dst_height < 1 || src_width < 1 || src_height < 1)
		return; // invalid src or dest rectangles

	// Setup uninitialized arguments; convert coordinates for legacy script mode
	if (dst_width == SCR_NO_VALUE) {
		dst_width = src->GetWidth();
	} else {
		sds->SizeToGameResolution(&dst_width);
	}
	if (dst_height == SCR_NO_VALUE) {
		dst_height = src->GetHeight();
	} else {
		sds->SizeToGameResolution(&dst_height);
	}

	if (src_x == SCR_NO_VALUE) {
		src_x = 0;
	}
	if (src_y == SCR_NO_VALUE) {
		src_y = 0;
	}
	sds->PointToGameResolution(&src_x, &src_y);
	if (src_width == SCR_NO_VALUE) {
		src_width = src->GetWidth();
	} else {
		sds->SizeToGameResolution(&src_width);
	}
	if (src_height == SCR_NO_VALUE) {
		src_height = src->GetHeight();
	} else {
		sds->SizeToGameResolution(&src_height);
	}

	sds->PointToGameResolution(&dst_x, &dst_y);

	if (dst_x >= ds->GetWidth() || dst_x + dst_width <= 0 || dst_y >= ds->GetHeight() || dst_y + dst_height <= 0 ||
		src_x >= src->GetWidth() || src_x + src_width <= 0 || src_y >= src->GetHeight() || src_y + src_height <= 0)
		return; // source or destination rects lie completely off surface
	// Clamp the source rect to the valid limits to prevent exceptions (ignore dest, bitmap drawing deals with that)
	Math::ClampLength(src_x, src_width, 0, src->GetWidth());
	Math::ClampLength(src_y, src_height, 0, src->GetHeight());

	// TODO: possibly optimize by not making a stretched intermediate bitmap
	// if simpler blit/draw_sprite could be called (no translucency with alpha channel).
	std::unique_ptr<Bitmap> conv_src;
	if (dst_width != src->GetWidth() || dst_height != src->GetHeight() ||
		src_width != src->GetWidth() || src_height != src->GetHeight()) {
		// Resize and/or partial copy specified
		conv_src.reset(BitmapHelper::CreateBitmap(dst_width, dst_height, src->GetColorDepth()));
		conv_src->StretchBlt(src,
			RectWH(src_x, src_y, src_width, src_height),
			RectWH(0, 0, dst_width, dst_height));

		src = conv_src.get();
	}

	ds = sds->StartDrawing();

	draw_sprite_support_alpha(ds, sds->hasAlphaChannel != 0, dst_x, dst_y, src, src_has_alpha,
		kBlendMode_Alpha, GfxDef::Trans100ToAlpha255(trans));

	sds->FinishedDrawing();
}

void DrawingSurface_DrawImage(ScriptDrawingSurface *sds,
		int dst_x, int dst_y, int slot, int trans,
		int dst_width, int dst_height,
		int src_x, int src_y, int src_width, int src_height) {
	if ((slot < 0) || (!_GP(spriteset).DoesSpriteExist(slot)))
		quit("!DrawingSurface.DrawImage: invalid sprite slot number specified");
	DrawingSurface_DrawImageImpl(sds, _GP(spriteset)[slot], dst_x, dst_y, trans, dst_width, dst_height,
		src_x, src_y, src_width, src_height, slot, (_GP(game).SpriteInfos[slot].Flags & SPF_ALPHACHANNEL) != 0);
}

void DrawingSurface_DrawImage6(ScriptDrawingSurface *sds, int xx, int yy, int slot, int trans, int width, int height) {
	DrawingSurface_DrawImage(sds, xx, yy, slot, trans, width, height, 0, 0, SCR_NO_VALUE, SCR_NO_VALUE);
}

void DrawingSurface_DrawSurface(ScriptDrawingSurface *target, ScriptDrawingSurface *source, int trans,
		int dst_x, int dst_y, int dst_width, int dst_height,
		int src_x, int src_y, int src_width, int src_height) {
	DrawingSurface_DrawImageImpl(target, source->GetBitmapSurface(), dst_x, dst_y, trans, dst_width, dst_height,
		src_x, src_y, src_width, src_height, -1, source->hasAlphaChannel != 0);
}

void DrawingSurface_DrawSurface2(ScriptDrawingSurface *target, ScriptDrawingSurface *source, int trans) {
	DrawingSurface_DrawSurface(target, source, trans, 0, 0, SCR_NO_VALUE, SCR_NO_VALUE, 0, 0, SCR_NO_VALUE, SCR_NO_VALUE);
}

void DrawingSurface_SetDrawingColor(ScriptDrawingSurface *sds, int newColour) {
	sds->currentColourScript = newColour;
	// StartDrawing to set up ds to set the colour at the appropriate
	// depth for the background
	Bitmap *ds = sds->StartDrawing();
	if (newColour == SCR_COLOR_TRANSPARENT) {
		sds->currentColour = ds->GetMaskColor();
	} else {
		sds->currentColour = ds->GetCompatibleColor(newColour);
	}
	sds->FinishedDrawingReadOnly();
}

int DrawingSurface_GetDrawingColor(ScriptDrawingSurface *sds) {
	return sds->currentColourScript;
}

void DrawingSurface_SetUseHighResCoordinates(ScriptDrawingSurface *sds, int highRes) {
	if (_GP(game).AllowRelativeRes())
		sds->highResCoordinates = (highRes) ? 1 : 0;
}

int DrawingSurface_GetUseHighResCoordinates(ScriptDrawingSurface *sds) {
	return sds->highResCoordinates;
}

int DrawingSurface_GetHeight(ScriptDrawingSurface *sds) {
	Bitmap *ds = sds->StartDrawing();
	int height = ds->GetHeight();
	sds->FinishedDrawingReadOnly();
	sds->SizeToGameResolution(&height);
	return height;
}

int DrawingSurface_GetWidth(ScriptDrawingSurface *sds) {
	Bitmap *ds = sds->StartDrawing();
	int width = ds->GetWidth();
	sds->FinishedDrawingReadOnly();
	sds->SizeToGameResolution(&width);
	return width;
}

void DrawingSurface_Clear(ScriptDrawingSurface *sds, int colour) {
	Bitmap *ds = sds->StartDrawing();
	int allegroColor;
	if ((colour == -SCR_NO_VALUE) || (colour == SCR_COLOR_TRANSPARENT)) {
		allegroColor = ds->GetMaskColor();
	} else {
		allegroColor = ds->GetCompatibleColor(colour);
	}
	ds->Fill(allegroColor);
	sds->FinishedDrawing();
}

void DrawingSurface_DrawCircle(ScriptDrawingSurface *sds, int x, int y, int radius) {
	sds->PointToGameResolution(&x, &y);
	sds->SizeToGameResolution(&radius);

	Bitmap *ds = sds->StartDrawing();
	ds->FillCircle(Circle(x, y, radius), sds->currentColour);
	sds->FinishedDrawing();
}

void DrawingSurface_DrawRectangle(ScriptDrawingSurface *sds, int x1, int y1, int x2, int y2) {
	sds->PointToGameResolution(&x1, &y1);
	sds->PointToGameResolution(&x2, &y2);

	Bitmap *ds = sds->StartDrawing();
	ds->FillRect(Rect(x1, y1, x2, y2), sds->currentColour);
	sds->FinishedDrawing();
}

void DrawingSurface_DrawTriangle(ScriptDrawingSurface *sds, int x1, int y1, int x2, int y2, int x3, int y3) {
	sds->PointToGameResolution(&x1, &y1);
	sds->PointToGameResolution(&x2, &y2);
	sds->PointToGameResolution(&x3, &y3);

	Bitmap *ds = sds->StartDrawing();
	ds->DrawTriangle(Triangle(x1, y1, x2, y2, x3, y3), sds->currentColour);
	sds->FinishedDrawing();
}

void DrawingSurface_DrawString(ScriptDrawingSurface *sds, int xx, int yy, int font, const char *text) {
	sds->PointToGameResolution(&xx, &yy);
	Bitmap *ds = sds->StartDrawing();
	// don't use wtextcolor because it will do a 16->32 conversion
	color_t text_color = sds->currentColour;
	if ((ds->GetColorDepth() <= 8) && (_GP(play).raw_color > 255)) {
		text_color = ds->GetCompatibleColor(1);
		debug_script_warn("RawPrint: Attempted to use hi-color on 256-col background");
	}
	String res_str = GUI::ApplyTextDirection(text);
	wouttext_outline(ds, xx, yy, font, text_color, res_str.GetCStr());
	sds->FinishedDrawing();
}

void DrawingSurface_DrawStringWrapped_Old(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int alignment, const char *msg) {
	DrawingSurface_DrawStringWrapped(sds, xx, yy, wid, font, ConvertLegacyScriptAlignment((LegacyScriptAlignment)alignment), msg);
}

void DrawingSurface_DrawStringWrapped(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int alignment, const char *msg) {
	int linespacing = get_font_linespacing(font);
	sds->PointToGameResolution(&xx, &yy);
	sds->SizeToGameResolution(&wid);

	if (break_up_text_into_lines(msg, _GP(Lines), wid, font) == 0)
		return;

	Bitmap *ds = sds->StartDrawing();
	color_t text_color = sds->currentColour;

	for (size_t i = 0; i < _GP(Lines).Count(); i++) {
		GUI::DrawTextAlignedHor(ds, _GP(Lines)[i].GetCStr(), font, text_color,
			xx, xx + wid - 1, yy + linespacing * i, (FrameAlignment)alignment);
	}

	sds->FinishedDrawing();
}

void DrawingSurface_DrawMessageWrapped(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int msgm) {
	char displbuf[3000];
	get_message_text(msgm, displbuf);
	// it's probably too late but check anyway
	if (strlen(displbuf) > 2899)
		quit("!RawPrintMessageWrapped: message too long");

	DrawingSurface_DrawStringWrapped_Old(sds, xx, yy, wid, font, kLegacyScAlignLeft, displbuf);
}

void DrawingSurface_DrawLine(ScriptDrawingSurface *sds, int fromx, int fromy, int tox, int toy, int thickness) {
	sds->PointToGameResolution(&fromx, &fromy);
	sds->PointToGameResolution(&tox, &toy);
	sds->SizeToGameResolution(&thickness);
	int ii, jj, xx, yy;
	Bitmap *ds = sds->StartDrawing();
	// draw several lines to simulate the thickness
	color_t draw_color = sds->currentColour;
	for (ii = 0; ii < thickness; ii++) {
		xx = (ii - (thickness / 2));
		for (jj = 0; jj < thickness; jj++) {
			yy = (jj - (thickness / 2));
			ds->DrawLine(Line(fromx + xx, fromy + yy, tox + xx, toy + yy), draw_color);
		}
	}
	sds->FinishedDrawing();
}

void DrawingSurface_DrawPixel(ScriptDrawingSurface *sds, int x, int y) {
	sds->PointToGameResolution(&x, &y);
	int thickness = 1;
	sds->SizeToGameResolution(&thickness);
	int ii, jj;
	Bitmap *ds = sds->StartDrawing();
	// draw several pixels to simulate the thickness
	color_t draw_color = sds->currentColour;
	for (ii = 0; ii < thickness; ii++) {
		for (jj = 0; jj < thickness; jj++) {
			ds->PutPixel(x + ii, y + jj, draw_color);
		}
	}
	sds->FinishedDrawing();
}

int DrawingSurface_GetPixel(ScriptDrawingSurface *sds, int x, int y) {
	sds->PointToGameResolution(&x, &y);
	Bitmap *ds = sds->StartDrawing();
	int rawPixel = ds->GetPixel(x, y);
	int maskColor = ds->GetMaskColor();
	int colDepth = ds->GetColorDepth();

	if (rawPixel == maskColor) {
		rawPixel = (unsigned int)SCR_COLOR_TRANSPARENT;
	} else if (colDepth > 8) {
		int r = getr_depth(colDepth, rawPixel);
		int g = getg_depth(colDepth, rawPixel);
		int b = getb_depth(colDepth, rawPixel);
		rawPixel = Game_GetColorFromRGB(r, g, b);
	}

	sds->FinishedDrawingReadOnly();
	return rawPixel;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// void (ScriptDrawingSurface *sds, int colour)
RuntimeScriptValue Sc_DrawingSurface_Clear(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptDrawingSurface, DrawingSurface_Clear);
}

// ScriptDrawingSurface* (ScriptDrawingSurface *sds)
RuntimeScriptValue Sc_DrawingSurface_CreateCopy(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJAUTO(ScriptDrawingSurface, ScriptDrawingSurface, DrawingSurface_CreateCopy);
}

// void (ScriptDrawingSurface *sds, int x, int y, int radius)
RuntimeScriptValue Sc_DrawingSurface_DrawCircle(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT3(ScriptDrawingSurface, DrawingSurface_DrawCircle);
}

// void (ScriptDrawingSurface* sds, int xx, int yy, int slot, int trans, int width, int height)
RuntimeScriptValue Sc_DrawingSurface_DrawImage6(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT6(ScriptDrawingSurface, DrawingSurface_DrawImage6);
}

RuntimeScriptValue Sc_DrawingSurface_DrawImage(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_OBJ_PARAM_COUNT(METHOD, 10);
	DrawingSurface_DrawImage((ScriptDrawingSurface *)self, params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue, params[5].IValue,
		params[6].IValue, params[7].IValue, params[8].IValue, params[9].IValue);
	return RuntimeScriptValue((int32_t)0);
}

// void (ScriptDrawingSurface *sds, int fromx, int fromy, int tox, int toy, int thickness)
RuntimeScriptValue Sc_DrawingSurface_DrawLine(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT5(ScriptDrawingSurface, DrawingSurface_DrawLine);
}

// void (ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int msgm)
RuntimeScriptValue Sc_DrawingSurface_DrawMessageWrapped(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT5(ScriptDrawingSurface, DrawingSurface_DrawMessageWrapped);
}

// void (ScriptDrawingSurface *sds, int x, int y)
RuntimeScriptValue Sc_DrawingSurface_DrawPixel(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(ScriptDrawingSurface, DrawingSurface_DrawPixel);
}

// void (ScriptDrawingSurface *sds, int x1, int y1, int x2, int y2)
RuntimeScriptValue Sc_DrawingSurface_DrawRectangle(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(ScriptDrawingSurface, DrawingSurface_DrawRectangle);
}

// void (ScriptDrawingSurface *sds, int xx, int yy, int font, const char* texx, ...)
RuntimeScriptValue Sc_DrawingSurface_DrawString(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_SCRIPT_SPRINTF(DrawingSurface_DrawString, 4);
	DrawingSurface_DrawString((ScriptDrawingSurface *)self, params[0].IValue, params[1].IValue, params[2].IValue, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void (ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int alignment, const char *msg)
RuntimeScriptValue Sc_DrawingSurface_DrawStringWrapped_Old(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT5_POBJ(ScriptDrawingSurface, DrawingSurface_DrawStringWrapped_Old, const char);
}

RuntimeScriptValue Sc_DrawingSurface_DrawStringWrapped(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_SCRIPT_SPRINTF(DrawingSurface_DrawString, 6);
	DrawingSurface_DrawStringWrapped((ScriptDrawingSurface *)self, params[0].IValue, params[1].IValue, params[2].IValue,
									 params[3].IValue, params[4].IValue, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void (ScriptDrawingSurface* target, ScriptDrawingSurface* source, int translev)
RuntimeScriptValue Sc_DrawingSurface_DrawSurface2(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ_PINT(ScriptDrawingSurface, DrawingSurface_DrawSurface2, ScriptDrawingSurface);
}

RuntimeScriptValue Sc_DrawingSurface_DrawSurface(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_OBJ_PARAM_COUNT(METHOD, 10);
	DrawingSurface_DrawSurface((ScriptDrawingSurface *)self, (ScriptDrawingSurface *)params[0].Ptr,
		params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue, params[5].IValue,
		params[6].IValue, params[7].IValue, params[8].IValue, params[9].IValue);
	return RuntimeScriptValue((int32_t)0);
}

// void (ScriptDrawingSurface *sds, int x1, int y1, int x2, int y2, int x3, int y3)
RuntimeScriptValue Sc_DrawingSurface_DrawTriangle(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT6(ScriptDrawingSurface, DrawingSurface_DrawTriangle);
}

// int (ScriptDrawingSurface *sds, int x, int y)
RuntimeScriptValue Sc_DrawingSurface_GetPixel(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT2(ScriptDrawingSurface, DrawingSurface_GetPixel);
}

// void (ScriptDrawingSurface* sds)
RuntimeScriptValue Sc_DrawingSurface_Release(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptDrawingSurface, DrawingSurface_Release);
}

// int (ScriptDrawingSurface *sds)
RuntimeScriptValue Sc_DrawingSurface_GetDrawingColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDrawingSurface, DrawingSurface_GetDrawingColor);
}

// void (ScriptDrawingSurface *sds, int newColour)
RuntimeScriptValue Sc_DrawingSurface_SetDrawingColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptDrawingSurface, DrawingSurface_SetDrawingColor);
}

// int (ScriptDrawingSurface *sds)
RuntimeScriptValue Sc_DrawingSurface_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDrawingSurface, DrawingSurface_GetHeight);
}

// int (ScriptDrawingSurface *sds)
RuntimeScriptValue Sc_DrawingSurface_GetUseHighResCoordinates(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDrawingSurface, DrawingSurface_GetUseHighResCoordinates);
}

// void (ScriptDrawingSurface *sds, int highRes)
RuntimeScriptValue Sc_DrawingSurface_SetUseHighResCoordinates(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptDrawingSurface, DrawingSurface_SetUseHighResCoordinates);
}

// int (ScriptDrawingSurface *sds)
RuntimeScriptValue Sc_DrawingSurface_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDrawingSurface, DrawingSurface_GetWidth);
}

//=============================================================================
//
// Exclusive variadic API implementation for Plugins
//
//=============================================================================

void ScPl_DrawingSurface_DrawStringWrapped(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int alignment, const char *msg, ...) {
	API_PLUGIN_SCRIPT_SPRINTF(msg);
	DrawingSurface_DrawStringWrapped(sds, xx, yy, wid, font, alignment, scsf_buffer);
}

void RegisterDrawingSurfaceAPI(ScriptAPIVersion base_api, ScriptAPIVersion /*compat_api */) {
	ScFnRegister drawsurf_api[] = {
		{"DrawingSurface::Clear^1", API_FN_PAIR(DrawingSurface_Clear)},
		{"DrawingSurface::CreateCopy^0", API_FN_PAIR(DrawingSurface_CreateCopy)},
		{"DrawingSurface::DrawCircle^3", API_FN_PAIR(DrawingSurface_DrawCircle)},
		{"DrawingSurface::DrawImage^6", API_FN_PAIR(DrawingSurface_DrawImage6)},
		{"DrawingSurface::DrawImage^10", API_FN_PAIR(DrawingSurface_DrawImage)},
		{"DrawingSurface::DrawLine^5", API_FN_PAIR(DrawingSurface_DrawLine)},
		{"DrawingSurface::DrawMessageWrapped^5", API_FN_PAIR(DrawingSurface_DrawMessageWrapped)},
		{"DrawingSurface::DrawPixel^2", API_FN_PAIR(DrawingSurface_DrawPixel)},
		{"DrawingSurface::DrawRectangle^4", API_FN_PAIR(DrawingSurface_DrawRectangle)},
		{"DrawingSurface::DrawString^104", Sc_DrawingSurface_DrawString},
		{"DrawingSurface::DrawSurface^2", API_FN_PAIR(DrawingSurface_DrawSurface2)},
		{"DrawingSurface::DrawSurface^10", API_FN_PAIR(DrawingSurface_DrawSurface)},
		{"DrawingSurface::DrawTriangle^6", API_FN_PAIR(DrawingSurface_DrawTriangle)},
		{"DrawingSurface::GetPixel^2", API_FN_PAIR(DrawingSurface_GetPixel)},
		{"DrawingSurface::Release^0", API_FN_PAIR(DrawingSurface_Release)},
		{"DrawingSurface::get_DrawingColor", API_FN_PAIR(DrawingSurface_GetDrawingColor)},
		{"DrawingSurface::set_DrawingColor", API_FN_PAIR(DrawingSurface_SetDrawingColor)},
		{"DrawingSurface::get_Height", API_FN_PAIR(DrawingSurface_GetHeight)},
		{"DrawingSurface::get_UseHighResCoordinates", API_FN_PAIR(DrawingSurface_GetUseHighResCoordinates)},
		{"DrawingSurface::set_UseHighResCoordinates", API_FN_PAIR(DrawingSurface_SetUseHighResCoordinates)},
		{"DrawingSurface::get_Width", API_FN_PAIR(DrawingSurface_GetWidth)},
	};

	ccAddExternalFunctions361(drawsurf_api);

	// Few functions have to be selected based on API level
	if (base_api < kScriptAPI_v350) {
		ccAddExternalObjectFunction361("DrawingSurface::DrawStringWrapped^6", API_FN_PAIR(DrawingSurface_DrawStringWrapped_Old));
	}
	else { // old non-variadic and new variadic variants
		ccAddExternalObjectFunction361("DrawingSurface::DrawStringWrapped^6", API_FN_PAIR(DrawingSurface_DrawStringWrapped));
		ccAddExternalObjectFunction361("DrawingSurface::DrawStringWrapped^106", Sc_DrawingSurface_DrawStringWrapped, (void *)ScPl_DrawingSurface_DrawStringWrapped);
	}
}

} // namespace AGS3
