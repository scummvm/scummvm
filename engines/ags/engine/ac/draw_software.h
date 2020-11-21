//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
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
