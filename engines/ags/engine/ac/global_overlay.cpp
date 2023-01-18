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

#include "ags/engine/ac/global_overlay.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace Shared;
using namespace Engine;

void RemoveOverlay(int ovrid) {
	if (find_overlay_of_type(ovrid) < 0) quit("!RemoveOverlay: invalid overlay id passed");
	remove_screen_overlay(ovrid);
}

int CreateGraphicOverlay(int x, int y, int slott, int trans) {
	auto *over = Overlay_CreateGraphicCore(false, x, y, slott, trans != 0, true); // always clone
	return over ? over->type : 0;
}

int CreateTextOverlay(int xx, int yy, int wii, int fontid, int text_color, const char *text, int disp_type) {
	int allowShrink = 0;

	if (xx != OVR_AUTOPLACE) {
		data_to_game_coords(&xx, &yy);
		// NOTE: this is ugly, but OVR_AUTOPLACE here suggests that width is already in game coords
		wii = data_to_game_coord(wii);
	} else  // allow DisplaySpeechBackground to be shrunk
		allowShrink = 1;

	auto *over = Overlay_CreateTextCore(false, xx, yy, wii, fontid, text_color, text, disp_type, allowShrink);
	return over ? over->type : 0;
}

void SetTextOverlay(int ovrid, int xx, int yy, int wii, int fontid, int text_color, const char *text) {
	RemoveOverlay(ovrid);
	const int disp_type = ovrid;
	int new_ovrid = CreateTextOverlay(xx, yy, wii, fontid, text_color, text, disp_type);
	if (new_ovrid != ovrid)
		quit("SetTextOverlay internal error: inconsistent type ids");
}

void MoveOverlay(int ovrid, int newx, int newy) {
	data_to_game_coords(&newx, &newy);

	int ovri = find_overlay_of_type(ovrid);
	if (ovri < 0) quit("!MoveOverlay: invalid overlay ID specified");
	_GP(screenover)[ovri].x = newx;
	_GP(screenover)[ovri].y = newy;
}

int IsOverlayValid(int ovrid) {
	if (find_overlay_of_type(ovrid) < 0)
		return 0;

	return 1;
}

} // namespace AGS3
