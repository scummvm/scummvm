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
#include "ags/engine/ac/object.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/object.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/room_object.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/walkable_area.h"
#include "ags/shared/game/room_struct.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

void redo_walkable_areas() {
	_GP(thisroom).WalkAreaMask->Blit(_G(walkareabackup), 0, 0);
	for (int h = 0; h < _G(walkareabackup)->GetHeight(); ++h) {
		uint8_t *walls_scanline = _GP(thisroom).WalkAreaMask->GetScanLineForWriting(h);
		for (int w = 0; w < _G(walkareabackup)->GetWidth(); ++w) {
			if ((walls_scanline[w] >= sizeof(_GP(play).walkable_areas_on)) ||
				(_GP(play).walkable_areas_on[walls_scanline[w]] == 0))
				walls_scanline[w] = 0;
		}
	}
}

int get_walkable_area_pixel(int x, int y) {
	return _GP(thisroom).WalkAreaMask->GetPixel(room_to_mask_coord(x), room_to_mask_coord(y));
}

int get_area_scaling(int onarea, int xx, int yy) {

	int zoom_level = 100;
	xx = room_to_mask_coord(xx);
	yy = room_to_mask_coord(yy);

	if ((onarea >= 0) && (onarea < MAX_WALK_AREAS) &&
	        (_GP(thisroom).WalkAreas[onarea].ScalingNear != NOT_VECTOR_SCALED)) {
		// We have vector scaling!
		// In case the character is off the screen, limit the Y co-ordinate
		// to within the area range (otherwise we get silly zoom levels
		// that cause Out Of Memory crashes)
		if (yy > _GP(thisroom).WalkAreas[onarea].Bottom)
			yy = _GP(thisroom).WalkAreas[onarea].Bottom;
		if (yy < _GP(thisroom).WalkAreas[onarea].Top)
			yy = _GP(thisroom).WalkAreas[onarea].Top;
		// Work it all out without having to use floats
		// Percent = ((y - top) * 100) / (areabottom - areatop)
		// Zoom level = ((max - min) * Percent) / 100
		if (_GP(thisroom).WalkAreas[onarea].Bottom != _GP(thisroom).WalkAreas[onarea].Top) {
			int percent = ((yy - _GP(thisroom).WalkAreas[onarea].Top) * 100)
			              / (_GP(thisroom).WalkAreas[onarea].Bottom - _GP(thisroom).WalkAreas[onarea].Top);
			zoom_level = ((_GP(thisroom).WalkAreas[onarea].ScalingNear - _GP(thisroom).WalkAreas[onarea].ScalingFar) * (percent)) / 100 + _GP(thisroom).WalkAreas[onarea].ScalingFar;
		} else {
			// Special case for 1px tall walkable area: take bottom line scaling
			zoom_level = _GP(thisroom).WalkAreas[onarea].ScalingNear;
		}
		zoom_level += 100;
	} else if ((onarea >= 0) & (onarea < MAX_WALK_AREAS))
		zoom_level = _GP(thisroom).WalkAreas[onarea].ScalingFar + 100;

	if (zoom_level == 0)
		zoom_level = 100;

	return zoom_level;
}

void scale_sprite_size(int sppic, int zoom_level, int *newwidth, int *newheight) {
	newwidth[0] = (_GP(game).SpriteInfos[sppic].Width * zoom_level) / 100;
	newheight[0] = (_GP(game).SpriteInfos[sppic].Height * zoom_level) / 100;
	if (newwidth[0] < 1)
		newwidth[0] = 1;
	if (newheight[0] < 1)
		newheight[0] = 1;
}

void remove_walkable_areas_from_temp(int fromx, int cwidth, int starty, int endy) {

	fromx = room_to_mask_coord(fromx);
	cwidth = room_to_mask_coord(cwidth);
	starty = room_to_mask_coord(starty);
	endy = room_to_mask_coord(endy);

	int yyy;
	if (endy >= _G(walkable_areas_temp)->GetHeight())
		endy = _G(walkable_areas_temp)->GetHeight() - 1;
	if (starty < 0)
		starty = 0;

	for (; cwidth > 0; cwidth--) {
		for (yyy = starty; yyy <= endy; yyy++)
			_G(walkable_areas_temp)->PutPixel(fromx, yyy, 0);
		fromx++;
	}

}

int is_point_in_rect(int x, int y, int left, int top, int right, int bottom) {
	if ((x >= left) && (x < right) && (y >= top) && (y <= bottom))
		return 1;
	return 0;
}

Bitmap *prepare_walkable_areas(int sourceChar) {
	// copy the walkable areas to the temp bitmap
	_G(walkable_areas_temp)->Blit(_GP(thisroom).WalkAreaMask.get(), 0, 0, 0, 0, _GP(thisroom).WalkAreaMask->GetWidth(), _GP(thisroom).WalkAreaMask->GetHeight());
	// if the character who's moving doesn't Bitmap *, don't bother checking
	if (sourceChar < 0);
	else if (_GP(game).chars[sourceChar].flags & CHF_NOBLOCKING)
		return _G(walkable_areas_temp);

	// for each character in the current room, make the area under
	// them unwalkable
	for (int ww = 0; ww < _GP(game).numcharacters; ww++) {
		if (_GP(game).chars[ww].on != 1) continue;
		if (_GP(game).chars[ww].room != _G(displayed_room)) continue;
		if (ww == sourceChar) continue;
		if (_GP(game).chars[ww].flags & CHF_NOBLOCKING) continue;
		if (room_to_mask_coord(_GP(game).chars[ww].y) >= _G(walkable_areas_temp)->GetHeight()) continue;
		if (room_to_mask_coord(_GP(game).chars[ww].x) >= _G(walkable_areas_temp)->GetWidth()) continue;
		if ((_GP(game).chars[ww].y < 0) || (_GP(game).chars[ww].x < 0)) continue;

		CharacterInfo *char1 = &_GP(game).chars[ww];
		int cwidth, fromx;

		if (is_char_on_another(sourceChar, ww, &fromx, &cwidth))
			continue;
		if ((sourceChar >= 0) && (is_char_on_another(ww, sourceChar, nullptr, nullptr)))
			continue;

		remove_walkable_areas_from_temp(fromx, cwidth, char1->get_blocking_top(), char1->get_blocking_bottom());
	}

	// check for any blocking objects in the room, and deal with them
	// as well
	for (uint32_t ww = 0; ww < _G(croom)->numobj; ww++) {
		if (_G(objs)[ww].on != 1) continue;
		if ((_G(objs)[ww].flags & OBJF_SOLID) == 0)
			continue;
		if (room_to_mask_coord(_G(objs)[ww].y) >= _G(walkable_areas_temp)->GetHeight()) continue;
		if (room_to_mask_coord(_G(objs)[ww].x) >= _G(walkable_areas_temp)->GetWidth()) continue;
		if ((_G(objs)[ww].y < 0) || (_G(objs)[ww].x < 0)) continue;

		int x1, y1, width, y2;
		get_object_blocking_rect(ww, &x1, &y1, &width, &y2);

		// if the character is currently standing on the object, ignore
		// it so as to allow him to escape
		if ((sourceChar >= 0) &&
		        (is_point_in_rect(_GP(game).chars[sourceChar].x, _GP(game).chars[sourceChar].y,
		                          x1, y1, x1 + width, y2)))
			continue;

		remove_walkable_areas_from_temp(x1, width, y1, y2);
	}

	return _G(walkable_areas_temp);
}

// return the walkable area at the character's feet, taking into account
// that he might just be off the edge of one
int get_walkable_area_at_location(int xx, int yy) {

	int onarea = get_walkable_area_pixel(xx, yy);

	if (onarea < 0) {
		// the character has walked off the edge of the screen, so stop them
		// jumping up to full size when leaving
		if (xx >= _GP(thisroom).Width)
			onarea = get_walkable_area_pixel(_GP(thisroom).Width - 1, yy);
		else if (xx < 0)
			onarea = get_walkable_area_pixel(0, yy);
		else if (yy >= _GP(thisroom).Height)
			onarea = get_walkable_area_pixel(xx, _GP(thisroom).Height - 1);
		else if (yy < 0)
			onarea = get_walkable_area_pixel(xx, 1);
	}
	if (onarea == 0) {
		// the path finder sometimes slightly goes into non-walkable areas;
		// so check for scaling in adjacent pixels
		const int TRYGAP = 2;
		onarea = get_walkable_area_pixel(xx + TRYGAP, yy);
		if (onarea <= 0)
			onarea = get_walkable_area_pixel(xx - TRYGAP, yy);
		if (onarea <= 0)
			onarea = get_walkable_area_pixel(xx, yy + TRYGAP);
		if (onarea <= 0)
			onarea = get_walkable_area_pixel(xx, yy - TRYGAP);
		if (onarea < 0)
			onarea = 0;
	}

	return (onarea >= 0 && onarea < MAX_WALK_AREAS) ? onarea : 0;
}

int get_walkable_area_at_character(int charnum) {
	CharacterInfo *chin = &_GP(game).chars[charnum];
	return get_walkable_area_at_location(chin->x, chin->y);
}

} // namespace AGS3
