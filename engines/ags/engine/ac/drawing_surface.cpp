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

#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/drawing_surface.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character_cache.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/object_cache.h"
#include "ags/engine/ac/room_object.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/string.h"
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
	if (sds->dynamicSpriteNumber >= 0) {
		if (sds->modified) {
			game_sprite_updated(sds->dynamicSpriteNumber);
		}

		sds->dynamicSpriteNumber = -1;
	}
	if (sds->dynamicSurfaceNumber >= 0) {
		delete _G(dynamicallyCreatedSurfaces)[sds->dynamicSurfaceNumber];
		_G(dynamicallyCreatedSurfaces)[sds->dynamicSurfaceNumber] = nullptr;
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
			_G(dynamicallyCreatedSurfaces)[i] = BitmapHelper::CreateBitmapCopy(sourceBitmap);
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

void DrawingSurface_DrawSurface(ScriptDrawingSurface *target, ScriptDrawingSurface *source, int translev) {
	if ((translev < 0) || (translev > 99))
		quit("!DrawingSurface.DrawSurface: invalid parameter (transparency must be 0-99)");

	Bitmap *ds = target->StartDrawing();
	Bitmap *surfaceToDraw = source->GetBitmapSurface();

	if (surfaceToDraw == target->GetBitmapSurface())
		quit("!DrawingSurface.DrawSurface: cannot draw surface onto itself");

	if (translev == 0) {
		// just draw it over the top, no transparency
		ds->Blit(surfaceToDraw, 0, 0, 0, 0, surfaceToDraw->GetWidth(), surfaceToDraw->GetHeight());
		target->FinishedDrawing();
		return;
	}

	if (surfaceToDraw->GetColorDepth() <= 8)
		quit("!DrawingSurface.DrawSurface: 256-colour surfaces cannot be drawn transparently");

	// Draw it transparently
	GfxUtil::DrawSpriteWithTransparency(ds, surfaceToDraw, 0, 0,
	                                    GfxDef::Trans100ToAlpha255(translev));
	target->FinishedDrawing();
}

void DrawingSurface_DrawImage(ScriptDrawingSurface *sds, int xx, int yy, int slot, int trans, int width, int height) {
	if ((slot < 0) || (_GP(spriteset)[slot] == nullptr))
		quit("!DrawingSurface.DrawImage: invalid sprite slot number specified");

	if ((trans < 0) || (trans > 100))
		quit("!DrawingSurface.DrawImage: invalid transparency setting");

	// 100% transparency, don't draw anything
	if (trans == 100)
		return;

	Bitmap *sourcePic = _GP(spriteset)[slot];
	bool needToFreeBitmap = false;

	if (width != SCR_NO_VALUE) {
		// Resize specified

		if ((width < 1) || (height < 1))
			return;

		sds->SizeToGameResolution(&width, &height);

		// resize the sprite to the requested size
		Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, sourcePic->GetColorDepth());

		newPic->StretchBlt(sourcePic,
		                   RectWH(0, 0, _GP(game).SpriteInfos[slot].Width, _GP(game).SpriteInfos[slot].Height),
		                   RectWH(0, 0, width, height));

		sourcePic = newPic;
		needToFreeBitmap = true;
		update_polled_stuff_if_runtime();
	}

	Bitmap *ds = sds->StartDrawing();
	sds->PointToGameResolution(&xx, &yy);

	if (sourcePic->GetColorDepth() != ds->GetColorDepth()) {
		debug_script_warn("RawDrawImage: Sprite %d colour depth %d-bit not same as background depth %d-bit", slot, _GP(spriteset)[slot]->GetColorDepth(), ds->GetColorDepth());
	}

	draw_sprite_support_alpha(ds, sds->hasAlphaChannel != 0, xx, yy, sourcePic, (_GP(game).SpriteInfos[slot].Flags & SPF_ALPHACHANNEL) != 0,
	                          kBlendMode_Alpha, GfxDef::Trans100ToAlpha255(trans));

	sds->FinishedDrawing();

	if (needToFreeBitmap)
		delete sourcePic;
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
	wouttext_outline(ds, xx, yy, font, text_color, text);
	sds->FinishedDrawing();
}

void DrawingSurface_DrawStringWrapped_Old(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int alignment, const char *msg) {
	DrawingSurface_DrawStringWrapped(sds, xx, yy, wid, font, ConvertLegacyScriptAlignment((LegacyScriptAlignment)alignment), msg);
}

void DrawingSurface_DrawStringWrapped(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int alignment, const char *msg) {
	int linespacing = getfontspacing_outlined(font);
	sds->PointToGameResolution(&xx, &yy);
	sds->SizeToGameResolution(&wid);

	if (break_up_text_into_lines(msg, _GP(Lines), wid, font) == 0)
		return;

	Bitmap *ds = sds->StartDrawing();
	color_t text_color = sds->currentColour;

	for (size_t i = 0; i < _GP(Lines).Count(); i++) {
		int drawAtX = xx;

		if (alignment & kMAlignHCenter) {
			drawAtX = xx + ((wid / 2) - wgettextwidth(_GP(Lines)[i].GetCStr(), font) / 2);
		} else if (alignment & kMAlignRight) {
			drawAtX = (xx + wid) - wgettextwidth(_GP(Lines)[i].GetCStr(), font);
		}

		wouttext_outline(ds, drawAtX, yy + linespacing * i, font, text_color, _GP(Lines)[i].GetCStr());
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
	unsigned int rawPixel = ds->GetPixel(x, y);
	unsigned int maskColor = ds->GetMaskColor();
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
RuntimeScriptValue Sc_DrawingSurface_DrawImage(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT6(ScriptDrawingSurface, DrawingSurface_DrawImage);
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
	API_OBJCALL_VOID_PINT5_POBJ(ScriptDrawingSurface, DrawingSurface_DrawStringWrapped, const char);
}

// void (ScriptDrawingSurface* target, ScriptDrawingSurface* source, int translev)
RuntimeScriptValue Sc_DrawingSurface_DrawSurface(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ_PINT(ScriptDrawingSurface, DrawingSurface_DrawSurface, ScriptDrawingSurface);
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
// Exclusive API for Plugins
//
//=============================================================================

// void (ScriptDrawingSurface *sds, int xx, int yy, int font, const char* texx, ...)
void ScPl_DrawingSurface_DrawString(ScriptDrawingSurface *sds, int xx, int yy, int font, const char *texx, ...) {
	API_PLUGIN_SCRIPT_SPRINTF(texx);
	DrawingSurface_DrawString(sds, xx, yy, font, scsf_buffer);
}

void RegisterDrawingSurfaceAPI(ScriptAPIVersion base_api, ScriptAPIVersion compat_api) {
	ccAddExternalObjectFunction("DrawingSurface::Clear^1", Sc_DrawingSurface_Clear);
	ccAddExternalObjectFunction("DrawingSurface::CreateCopy^0", Sc_DrawingSurface_CreateCopy);
	ccAddExternalObjectFunction("DrawingSurface::DrawCircle^3", Sc_DrawingSurface_DrawCircle);
	ccAddExternalObjectFunction("DrawingSurface::DrawImage^6", Sc_DrawingSurface_DrawImage);
	ccAddExternalObjectFunction("DrawingSurface::DrawLine^5", Sc_DrawingSurface_DrawLine);
	ccAddExternalObjectFunction("DrawingSurface::DrawMessageWrapped^5", Sc_DrawingSurface_DrawMessageWrapped);
	ccAddExternalObjectFunction("DrawingSurface::DrawPixel^2", Sc_DrawingSurface_DrawPixel);
	ccAddExternalObjectFunction("DrawingSurface::DrawRectangle^4", Sc_DrawingSurface_DrawRectangle);
	ccAddExternalObjectFunction("DrawingSurface::DrawString^104", Sc_DrawingSurface_DrawString);
	if (base_api < kScriptAPI_v350)
		ccAddExternalObjectFunction("DrawingSurface::DrawStringWrapped^6", Sc_DrawingSurface_DrawStringWrapped_Old);
	else
		ccAddExternalObjectFunction("DrawingSurface::DrawStringWrapped^6", Sc_DrawingSurface_DrawStringWrapped);
	ccAddExternalObjectFunction("DrawingSurface::DrawSurface^2", Sc_DrawingSurface_DrawSurface);
	ccAddExternalObjectFunction("DrawingSurface::DrawTriangle^6", Sc_DrawingSurface_DrawTriangle);
	ccAddExternalObjectFunction("DrawingSurface::GetPixel^2", Sc_DrawingSurface_GetPixel);
	ccAddExternalObjectFunction("DrawingSurface::Release^0", Sc_DrawingSurface_Release);
	ccAddExternalObjectFunction("DrawingSurface::get_DrawingColor", Sc_DrawingSurface_GetDrawingColor);
	ccAddExternalObjectFunction("DrawingSurface::set_DrawingColor", Sc_DrawingSurface_SetDrawingColor);
	ccAddExternalObjectFunction("DrawingSurface::get_Height", Sc_DrawingSurface_GetHeight);
	ccAddExternalObjectFunction("DrawingSurface::get_UseHighResCoordinates", Sc_DrawingSurface_GetUseHighResCoordinates);
	ccAddExternalObjectFunction("DrawingSurface::set_UseHighResCoordinates", Sc_DrawingSurface_SetUseHighResCoordinates);
	ccAddExternalObjectFunction("DrawingSurface::get_Width", Sc_DrawingSurface_GetWidth);

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("DrawingSurface::Clear^1", (void *)DrawingSurface_Clear);
	ccAddExternalFunctionForPlugin("DrawingSurface::CreateCopy^0", (void *)DrawingSurface_CreateCopy);
	ccAddExternalFunctionForPlugin("DrawingSurface::DrawCircle^3", (void *)DrawingSurface_DrawCircle);
	ccAddExternalFunctionForPlugin("DrawingSurface::DrawImage^6", (void *)DrawingSurface_DrawImage);
	ccAddExternalFunctionForPlugin("DrawingSurface::DrawLine^5", (void *)DrawingSurface_DrawLine);
	ccAddExternalFunctionForPlugin("DrawingSurface::DrawMessageWrapped^5", (void *)DrawingSurface_DrawMessageWrapped);
	ccAddExternalFunctionForPlugin("DrawingSurface::DrawPixel^2", (void *)DrawingSurface_DrawPixel);
	ccAddExternalFunctionForPlugin("DrawingSurface::DrawRectangle^4", (void *)DrawingSurface_DrawRectangle);
	ccAddExternalFunctionForPlugin("DrawingSurface::DrawString^104", (void *)ScPl_DrawingSurface_DrawString);
	if (base_api < kScriptAPI_v350)
		ccAddExternalFunctionForPlugin("DrawingSurface::DrawStringWrapped^6", (void *)DrawingSurface_DrawStringWrapped_Old);
	else
		ccAddExternalFunctionForPlugin("DrawingSurface::DrawStringWrapped^6", (void *)DrawingSurface_DrawStringWrapped);
	ccAddExternalFunctionForPlugin("DrawingSurface::DrawSurface^2", (void *)DrawingSurface_DrawSurface);
	ccAddExternalFunctionForPlugin("DrawingSurface::DrawTriangle^6", (void *)DrawingSurface_DrawTriangle);
	ccAddExternalFunctionForPlugin("DrawingSurface::GetPixel^2", (void *)DrawingSurface_GetPixel);
	ccAddExternalFunctionForPlugin("DrawingSurface::Release^0", (void *)DrawingSurface_Release);
	ccAddExternalFunctionForPlugin("DrawingSurface::get_DrawingColor", (void *)DrawingSurface_GetDrawingColor);
	ccAddExternalFunctionForPlugin("DrawingSurface::set_DrawingColor", (void *)DrawingSurface_SetDrawingColor);
	ccAddExternalFunctionForPlugin("DrawingSurface::get_Height", (void *)DrawingSurface_GetHeight);
	ccAddExternalFunctionForPlugin("DrawingSurface::get_UseHighResCoordinates", (void *)DrawingSurface_GetUseHighResCoordinates);
	ccAddExternalFunctionForPlugin("DrawingSurface::set_UseHighResCoordinates", (void *)DrawingSurface_SetUseHighResCoordinates);
	ccAddExternalFunctionForPlugin("DrawingSurface::get_Width", (void *)DrawingSurface_GetWidth);
}

} // namespace AGS3
