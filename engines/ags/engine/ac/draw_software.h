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
// Software drawing component. Optimizes drawing for software renderer using
// dirty rectangles technique.
//
//=============================================================================

#ifndef __AGS_EE_AC__DRAWSOFTWARE_H
#define __AGS_EE_AC__DRAWSOFTWARE_H

#include "gfx/bitmap.h"
#include "gfx/ddb.h"
#include "util/geometry.h"

// Inits dirty rects array for the given room camera/viewport pair
// View_index indicates the room viewport (>= 0) or the main viewport (-1)
void init_invalid_regions(int view_index, const Size &surf_size, const Rect &viewport);
// Deletes dirty rects for particular index
void delete_invalid_regions(int view_index);
// Disposes dirty rects arrays
void dispose_invalid_regions(bool room_only);
// Update the coordinate transformation for the particular dirty rects object
void set_invalidrects_cameraoffs(int view_index, int x, int y);
// Mark the whole screen dirty
void invalidate_all_rects();
// Mark the whole camera surface dirty
void invalidate_all_camera_rects(int view_index);
void invalidate_rect_ds(int x1, int y1, int x2, int y2, bool in_room);
// Paints the black screen background in the regions marked as dirty
void update_black_invreg_and_reset(AGS::Common::Bitmap *ds);
// Copies the room regions marked as dirty from source (src) to destination (ds) with the given offset (x, y)
// no_transform flag tells the system that the regions should be plain copied to the ds.
void update_room_invreg_and_reset(int view_index, AGS::Common::Bitmap *ds, AGS::Common::Bitmap *src, bool no_transform);

#endif // __AGS_EE_AC__DRAWSOFTWARE_H
