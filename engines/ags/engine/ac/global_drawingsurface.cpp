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
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_drawingsurface.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/game/roomstruct.h"
#include "ags/shared/gui/guidefines.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/shared/gfx/gfx_def.h"
#include "ags/engine/gfx/gfx_util.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern Bitmap *raw_saved_screen;
extern RoomStruct thisroom;
extern GameState play;
extern SpriteCache spriteset;
extern GameSetupStruct game;

// Raw screen writing routines - similar to old CapturedStuff
#define RAW_START() play.raw_drawing_surface = thisroom.BgFrames[play.bg_frame].Graphic; play.raw_modified[play.bg_frame] = 1
#define RAW_END()
#define RAW_SURFACE() (play.raw_drawing_surface.get())

// RawSaveScreen: copy the current screen to a backup bitmap
void RawSaveScreen() {
	if (raw_saved_screen != nullptr)
		delete raw_saved_screen;
	PBitmap source = thisroom.BgFrames[play.bg_frame].Graphic;
	raw_saved_screen = BitmapHelper::CreateBitmapCopy(source.get());
}
// RawRestoreScreen: copy backup bitmap back to screen; we
// deliberately don't free the Bitmap *cos they can multiple restore
// and it gets freed on room exit anyway
void RawRestoreScreen() {
	if (raw_saved_screen == nullptr) {
		debug_script_warn("RawRestoreScreen: unable to restore, since the screen hasn't been saved previously.");
		return;
	}
	PBitmap deston = thisroom.BgFrames[play.bg_frame].Graphic;
	deston->Blit(raw_saved_screen, 0, 0, 0, 0, deston->GetWidth(), deston->GetHeight());
	invalidate_screen();
	mark_current_background_dirty();
}
// Restores the backup bitmap, but tints it to the specified level
void RawRestoreScreenTinted(int red, int green, int blue, int opacity) {
	if (raw_saved_screen == nullptr) {
		debug_script_warn("RawRestoreScreenTinted: unable to restore, since the screen hasn't been saved previously.");
		return;
	}
	if ((red < 0) || (green < 0) || (blue < 0) ||
		(red > 255) || (green > 255) || (blue > 255) ||
		(opacity < 1) || (opacity > 100))
		quit("!RawRestoreScreenTinted: invalid parameter. R,G,B must be 0-255, opacity 1-100");

	debug_script_log("RawRestoreTinted RGB(%d,%d,%d) %d%%", red, green, blue, opacity);

	PBitmap deston = thisroom.BgFrames[play.bg_frame].Graphic;
	tint_image(deston.get(), raw_saved_screen, red, green, blue, opacity);
	invalidate_screen();
	mark_current_background_dirty();
}

void RawDrawFrameTransparent(int frame, int translev) {
	if ((frame < 0) || ((size_t)frame >= thisroom.BgFrameCount) ||
		(translev < 0) || (translev > 99))
		quit("!RawDrawFrameTransparent: invalid parameter (transparency must be 0-99, frame a valid BG frame)");

	PBitmap bg = thisroom.BgFrames[frame].Graphic;
	if (bg->GetColorDepth() <= 8)
		quit("!RawDrawFrameTransparent: 256-colour backgrounds not supported");

	if (frame == play.bg_frame)
		quit("!RawDrawFrameTransparent: cannot draw current background onto itself");

	RAW_START();
	if (translev == 0) {
		// just draw it over the top, no transparency
		RAW_SURFACE()->Blit(bg.get(), 0, 0, 0, 0, bg->GetWidth(), bg->GetHeight());
	} else {
		// Draw it transparently
		GfxUtil::DrawSpriteWithTransparency(RAW_SURFACE(), bg.get(), 0, 0,
			GfxDef::Trans100ToAlpha255(translev));
	}
	invalidate_screen();
	mark_current_background_dirty();
	RAW_END();
}

void RawClear(int clr) {
	RAW_START();
	clr = RAW_SURFACE()->GetCompatibleColor(clr);
	RAW_SURFACE()->Clear(clr);
	invalidate_screen();
	mark_current_background_dirty();
}
void RawSetColor(int clr) {
	// set the colour at the appropriate depth for the background
	play.raw_color = MakeColor(clr);
}
void RawSetColorRGB(int red, int grn, int blu) {
	if ((red < 0) || (red > 255) || (grn < 0) || (grn > 255) ||
		(blu < 0) || (blu > 255))
		quit("!RawSetColorRGB: colour values must be 0-255");

	play.raw_color = makecol_depth(thisroom.BgFrames[play.bg_frame].Graphic->GetColorDepth(), red, grn, blu);
}
void RawPrint(int xx, int yy, const char *text) {
	RAW_START();
	// don't use wtextcolor because it will do a 16->32 conversion
	color_t text_color = play.raw_color;
	if ((RAW_SURFACE()->GetColorDepth() <= 8) && (play.raw_color > 255)) {
		text_color = RAW_SURFACE()->GetCompatibleColor(1);
		debug_script_warn("RawPrint: Attempted to use hi-color on 256-col background");
	}
	data_to_game_coords(&xx, &yy);
	wouttext_outline(RAW_SURFACE(), xx, yy, play.normal_font, text_color, text);
	// we must invalidate the entire screen because these are room
	// co-ordinates, not screen co-ords which it works with
	invalidate_screen();
	mark_current_background_dirty();
	RAW_END();
}
void RawPrintMessageWrapped(int xx, int yy, int wid, int font, int msgm) {
	char displbuf[3000];
	int linespacing = getfontspacing_outlined(font);
	data_to_game_coords(&xx, &yy);
	wid = data_to_game_coord(wid);

	get_message_text(msgm, displbuf);
	// it's probably too late but check anyway
	if (strlen(displbuf) > 2899)
		quit("!RawPrintMessageWrapped: message too long");
	if (break_up_text_into_lines(displbuf, Lines, wid, font) == 0)
		return;

	RAW_START();
	color_t text_color = play.raw_color;
	for (size_t i = 0; i < Lines.Count(); i++)
		wouttext_outline(RAW_SURFACE(), xx, yy + linespacing * i, font, text_color, Lines[i]);
	invalidate_screen();
	mark_current_background_dirty();
	RAW_END();
}

void RawDrawImageCore(int xx, int yy, int slot, int alpha) {
	if ((slot < 0) || (spriteset[slot] == nullptr))
		quit("!RawDrawImage: invalid sprite slot number specified");
	RAW_START();

	if (spriteset[slot]->GetColorDepth() != RAW_SURFACE()->GetColorDepth()) {
		debug_script_warn("RawDrawImage: Sprite %d colour depth %d-bit not same as background depth %d-bit", slot, spriteset[slot]->GetColorDepth(), RAW_SURFACE()->GetColorDepth());
	}

	draw_sprite_slot_support_alpha(RAW_SURFACE(), false, xx, yy, slot, kBlendMode_Alpha, alpha);
	invalidate_screen();
	mark_current_background_dirty();
	RAW_END();
}

void RawDrawImage(int xx, int yy, int slot) {
	data_to_game_coords(&xx, &yy);
	RawDrawImageCore(xx, yy, slot);
}

void RawDrawImageTrans(int xx, int yy, int slot, int alpha) {
	data_to_game_coords(&xx, &yy);
	RawDrawImageCore(xx, yy, slot, alpha);
}

void RawDrawImageOffset(int xx, int yy, int slot) {
	// This function takes coordinates in real game coordinates as opposed to script coordinates
	defgame_to_finalgame_coords(xx, yy);
	RawDrawImageCore(xx, yy, slot);
}

void RawDrawImageTransparent(int xx, int yy, int slot, int legacy_transparency) {
	if ((legacy_transparency < 0) || (legacy_transparency > 100))
		quit("!RawDrawImageTransparent: invalid transparency setting");

	// WARNING: the previous versions of AGS actually had a bug:
	// although manual stated that RawDrawImageTransparent takes % of transparency
	// as an argument, that value was used improperly when setting up an Allegro's
	// trans_blender, which caused it to act about as % of opacity instead, but
	// with a twist.
	//
	// It was converted to 255-ranged "transparency" parameter:
	// int transparency = (trans * 255) / 100;
	//
	// Note by CJ:
	// Transparency is a bit counter-intuitive
	// 0=not transparent, 255=invisible, 1..254 barely visible .. mostly visible
	//
	// In order to support this backward-compatible behavior, we convert the
	// opacity into proper alpha this way:
	// 0      => alpha 255
	// 100    => alpha 0
	// 1 - 99 => alpha 1 - 244
	//
	RawDrawImageTrans(xx, yy, slot, GfxDef::LegacyTrans100ToAlpha255(legacy_transparency));

	update_polled_stuff_if_runtime();  // this operation can be slow so stop music skipping
}
void RawDrawImageResized(int xx, int yy, int gotSlot, int width, int height) {
	if ((gotSlot < 0) || (spriteset[gotSlot] == nullptr))
		quit("!RawDrawImageResized: invalid sprite slot number specified");
	// very small, don't draw it
	if ((width < 1) || (height < 1))
		return;

	data_to_game_coords(&xx, &yy);
	data_to_game_coords(&width, &height);

	// resize the sprite to the requested size
	Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, spriteset[gotSlot]->GetColorDepth());
	newPic->StretchBlt(spriteset[gotSlot],
		RectWH(0, 0, game.SpriteInfos[gotSlot].Width, game.SpriteInfos[gotSlot].Height),
		RectWH(0, 0, width, height));

	RAW_START();
	if (newPic->GetColorDepth() != RAW_SURFACE()->GetColorDepth())
		quit("!RawDrawImageResized: image colour depth mismatch: the background image must have the same colour depth as the sprite being drawn");

	GfxUtil::DrawSpriteWithTransparency(RAW_SURFACE(), newPic, xx, yy);
	delete newPic;
	invalidate_screen();
	mark_current_background_dirty();
	update_polled_stuff_if_runtime();  // this operation can be slow so stop music skipping
	RAW_END();
}
void RawDrawLine(int fromx, int fromy, int tox, int toy) {
	data_to_game_coords(&fromx, &fromy);
	data_to_game_coords(&tox, &toy);

	play.raw_modified[play.bg_frame] = 1;
	int ii, jj;
	// draw a line thick enough to look the same at all resolutions
	PBitmap bg = thisroom.BgFrames[play.bg_frame].Graphic;
	color_t draw_color = play.raw_color;
	for (ii = 0; ii < get_fixed_pixel_size(1); ii++) {
		for (jj = 0; jj < get_fixed_pixel_size(1); jj++)
			bg->DrawLine(Line(fromx + ii, fromy + jj, tox + ii, toy + jj), draw_color);
	}
	invalidate_screen();
	mark_current_background_dirty();
}
void RawDrawCircle(int xx, int yy, int rad) {
	data_to_game_coords(&xx, &yy);
	rad = data_to_game_coord(rad);

	play.raw_modified[play.bg_frame] = 1;
	PBitmap bg = thisroom.BgFrames[play.bg_frame].Graphic;
	bg->FillCircle(Circle(xx, yy, rad), play.raw_color);
	invalidate_screen();
	mark_current_background_dirty();
}
void RawDrawRectangle(int x1, int y1, int x2, int y2) {
	play.raw_modified[play.bg_frame] = 1;
	data_to_game_coords(&x1, &y1);
	data_to_game_round_up(&x2, &y2);

	PBitmap bg = thisroom.BgFrames[play.bg_frame].Graphic;
	bg->FillRect(Rect(x1, y1, x2, y2), play.raw_color);
	invalidate_screen();
	mark_current_background_dirty();
}
void RawDrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
	play.raw_modified[play.bg_frame] = 1;
	data_to_game_coords(&x1, &y1);
	data_to_game_coords(&x2, &y2);
	data_to_game_coords(&x3, &y3);

	PBitmap bg = thisroom.BgFrames[play.bg_frame].Graphic;
	bg->DrawTriangle(Triangle(x1, y1, x2, y2, x3, y3), play.raw_color);
	invalidate_screen();
	mark_current_background_dirty();
}

} // namespace AGS3
