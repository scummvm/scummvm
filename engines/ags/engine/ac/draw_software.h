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

//=============================================================================
//
// Software drawing component. Optimizes drawing for software renderer using
// dirty rectangles technique.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_DRAW_SOFTWARE_H
#define AGS_ENGINE_AC_DRAW_SOFTWARE_H

#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/shared/util/geometry.h"
#include "ags/shared/util/scaling.h"

namespace AGS3 {

using AGS::Shared::PlaneScaling;

// TODO: choose these values depending on game resolution?
#define MAXDIRTYREGIONS 25
#define WHOLESCREENDIRTY (MAXDIRTYREGIONS + 5)
#define MAX_SPANS_PER_ROW 4

// Dirty rects store coordinate values in the coordinate system of a camera surface,
// where coords always span from 0,0 to surface width,height.
// Converting from room to dirty rects would require subtracting room camera offsets.
struct IRSpan {
	int x1, x2;
	int mergeSpan(int tx1, int tx2);

	IRSpan();
};

struct IRRow {
	IRSpan span[MAX_SPANS_PER_ROW];
	int numSpans;

	IRRow();
};

struct DirtyRects {
	// Size of the surface managed by this dirty rects object
	Size SurfaceSize;
	// Where the surface is rendered on screen
	Rect Viewport;
	// Room -> screen coordinate transformation
	PlaneScaling Room2Screen;
	// Screen -> dirty surface rect
	// The dirty rects are saved in coordinates limited to (0,0)->(camera size) rather than room or screen coords
	PlaneScaling Screen2DirtySurf;

	std::vector<IRRow> DirtyRows;
	Rect DirtyRegions[MAXDIRTYREGIONS];
	size_t NumDirtyRegions;

	DirtyRects();
	bool IsInit() const;
	// Initialize dirty rects for the given surface size
	void Init(const Size &surf_size, const Rect &viewport);
	void SetSurfaceOffsets(int x, int y);
	// Delete dirty rects
	void Destroy();
	// Mark all surface as tidy
	void Reset();
};

// Sets global viewport offset (used for legacy letterbox)
void set_invalidrects_globaloffs(int x, int y);
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
// Mark certain rectangle dirty; in_room tells if coordinates are room viewport or screen coords
void invalidate_rect_ds(int x1, int y1, int x2, int y2, bool in_room);
// Mark rectangle dirty, treat pos as global screen coords (not offset by legacy letterbox mode)
void invalidate_rect_global(int x1, int y1, int x2, int y2);
// Paints the black screen background in the regions marked as dirty
void update_black_invreg_and_reset(AGS::Shared::Bitmap *ds);
// Copies the room regions marked as dirty from source (src) to destination (ds) with the given offset (x, y)
// no_transform flag tells the system that the regions should be plain copied to the ds.
void update_room_invreg_and_reset(int view_index, AGS::Shared::Bitmap *ds, AGS::Shared::Bitmap *src, bool no_transform);

} // namespace AGS3

#endif
