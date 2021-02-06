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

#include "ags/engine/ac/global_overlay.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/screenoverlay.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/engine/ac/system.h"
#include "ags/shared/gfx/bitmap.h"

namespace AGS3 {

using namespace Shared;
using namespace Engine;

extern SpriteCache spriteset;
extern GameSetupStruct game;



void RemoveOverlay(int ovrid) {
	if (find_overlay_of_type(ovrid) < 0) quit("!RemoveOverlay: invalid overlay id passed");
	remove_screen_overlay(ovrid);
}

int CreateGraphicOverlay(int xx, int yy, int slott, int trans) {
	data_to_game_coords(&xx, &yy);

	Bitmap *screeno = BitmapHelper::CreateTransparentBitmap(game.SpriteInfos[slott].Width, game.SpriteInfos[slott].Height, game.GetColorDepth());
	wputblock(screeno, 0, 0, spriteset[slott], trans);
	bool hasAlpha = (game.SpriteInfos[slott].Flags & SPF_ALPHACHANNEL) != 0;
	int nse = add_screen_overlay(xx, yy, OVER_CUSTOM, screeno, hasAlpha);
	return screenover[nse].type;
}

int CreateTextOverlayCore(int xx, int yy, int wii, int fontid, int text_color, const char *text, int disp_type, int allowShrink) {
	if (wii < 8) wii = play.GetUIViewport().GetWidth() / 2;
	if (xx < 0) xx = play.GetUIViewport().GetWidth() / 2 - wii / 2;
	if (text_color == 0) text_color = 16;
	return _display_main(xx, yy, wii, text, disp_type, fontid, -text_color, 0, allowShrink, false);
}

int CreateTextOverlay(int xx, int yy, int wii, int fontid, int text_color, const char *text, int disp_type) {
	int allowShrink = 0;

	if (xx != OVR_AUTOPLACE) {
		data_to_game_coords(&xx, &yy);
		wii = data_to_game_coord(wii);
	} else // allow DisplaySpeechBackground to be shrunk
		allowShrink = 1;

	return CreateTextOverlayCore(xx, yy, wii, fontid, text_color, text, disp_type, allowShrink);
}

void SetTextOverlay(int ovrid, int xx, int yy, int wii, int fontid, int text_color, const char *text) {
	RemoveOverlay(ovrid);
	const int disp_type = ovrid;
	if (CreateTextOverlay(xx, yy, wii, fontid, text_color, text, disp_type) != ovrid)
		quit("SetTextOverlay internal error: inconsistent type ids");
}

void MoveOverlay(int ovrid, int newx, int newy) {
	data_to_game_coords(&newx, &newy);

	int ovri = find_overlay_of_type(ovrid);
	if (ovri < 0) quit("!MoveOverlay: invalid overlay ID specified");
	screenover[ovri].x = newx;
	screenover[ovri].y = newy;
}

int IsOverlayValid(int ovrid) {
	if (find_overlay_of_type(ovrid) < 0)
		return 0;

	return 1;
}

} // namespace AGS3
