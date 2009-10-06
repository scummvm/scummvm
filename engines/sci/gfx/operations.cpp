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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Graphical operations, called from the widget state manager

#include "sci/sci.h"
#include "sci/gfx/operations.h"
#include "sci/gfx/font.h"
#include "sci/console.h"

#include "common/system.h"
#include "common/events.h"

namespace Sci {

#define PRECISE_PRIORITY_MAP // Duplicate all operations on the local priority map as appropriate

#undef GFXW_DEBUG_DIRTY
// Enable to debug stuff relevant for dirty rectsin widget management

#ifdef GFXW_DEBUG_DIRTY
#  define DDIRTY fprintf(stderr, "%s:%5d| ", __FILE__, __LINE__); fprintf
#else
#  define DDIRTY if (0) fprintf
#endif

// Default color maps
#define DEFAULT_COLORS_NR 16

PaletteEntry default_colors[DEFAULT_COLORS_NR] = {
	PaletteEntry(0x00, 0x00, 0x00), PaletteEntry(0x00, 0x00, 0xaa),
	PaletteEntry(0x00, 0xaa, 0x00), PaletteEntry(0x00, 0xaa, 0xaa),
	PaletteEntry(0xaa, 0x00, 0x00), PaletteEntry(0xaa, 0x00, 0xaa),
	PaletteEntry(0xaa, 0x55, 0x00), PaletteEntry(0xaa, 0xaa, 0xaa),
	PaletteEntry(0x55, 0x55, 0x55), PaletteEntry(0x55, 0x55, 0xff),
	PaletteEntry(0x55, 0xff, 0x55), PaletteEntry(0x55, 0xff, 0xff),
	PaletteEntry(0xff, 0x55, 0x55), PaletteEntry(0xff, 0x55, 0xff),
	PaletteEntry(0xff, 0xff, 0x55), PaletteEntry(0xff, 0xff, 0xff)
}; // "Normal" EGA

#define POINTER_VISIBLE_BUT_CLIPPED 2

// How to determine whether colors have to be allocated
#define SCREEN_PALETTE state->driver->getMode()->palette

//#define GFXOP_DEBUG_DIRTY

// Internal operations

static void _gfxop_scale_rect(rect_t *rect, gfx_mode_t *mode) {
	rect->x *= mode->scaleFactor;
	rect->y *= mode->scaleFactor;
	rect->width *= mode->scaleFactor;
	rect->height *= mode->scaleFactor;
}

static void _gfxop_scale_point(Common::Point *point, gfx_mode_t *mode) {
	point->x *= mode->scaleFactor;
	point->y *= mode->scaleFactor;
}

int _gfxop_clip(rect_t *rect, rect_t clipzone) {
// Returns 1 if nothing is left */
#if 0
	printf("Clipping (%d, %d) size (%d, %d)  by (%d,%d)(%d,%d)\n", rect->x, rect->y, rect->width, rect->height,
	       clipzone.x, clipzone.y, clipzone.width, clipzone.height);
#endif

	if (rect->x < clipzone.x) {
		rect->width -= (clipzone.x - rect->x);
		rect->x = clipzone.x;
	}

	if (rect->y < clipzone.y) {
		rect->height -= (clipzone.y - rect->y);
		rect->y = clipzone.y;
	}

	if (rect->x + rect->width > clipzone.x + clipzone.width)
		rect->width = (clipzone.x + clipzone.width) - rect->x;

	if (rect->y + rect->height > clipzone.y + clipzone.height)
		rect->height = (clipzone.y + clipzone.height) - rect->y;

	if (rect->width < 0)
		rect->width = 0;
	if (rect->height < 0)
		rect->height = 0;

#if 0
	printf(" => (%d, %d) size (%d, %d)\n", rect->x, rect->y, rect->width, rect->height);
#endif
	return (rect->width <= 0 || rect->height <= 0);
}

static void _gfxop_grab_pixmap(GfxState *state, gfx_pixmap_t **pxmp, int x, int y,
							  int xl, int yl, int priority, rect_t *zone) {
	// Returns 1 if the resulting data size was zero, GFX_OK or an error code otherwise */
	int xfact = state->driver->getMode()->scaleFactor;
	int yfact = state->driver->getMode()->scaleFactor;
	int unscaled_xl = (xl + xfact - 1) / xfact;
	int unscaled_yl = (yl + yfact - 1) / yfact;
	*zone = gfx_rect(x, y, xl, yl);

	if (_gfxop_clip(zone, gfx_rect(0, 0, 320 * state->driver->getMode()->scaleFactor, 200 * state->driver->getMode()->scaleFactor)))
		error("_gfxop_grab_pixmap: zone was empty");

	if (!*pxmp)
		*pxmp = gfx_new_pixmap(unscaled_xl, unscaled_yl, GFX_RESID_NONE, 0, 0);
	else
		if (xl * yl > (*pxmp)->width * (*pxmp)->height) {
			gfx_pixmap_free_data(*pxmp);
			(*pxmp)->data = NULL;
		}

	if (!(*pxmp)->data) {
		(*pxmp)->index_width = unscaled_xl + 1;
		(*pxmp)->index_height = unscaled_yl + 1;
		gfx_pixmap_alloc_data(*pxmp, state->driver->getMode());
	}
	state->driver->grabPixmap(*zone, *pxmp, priority ? GFX_MASK_PRIORITY : GFX_MASK_VISUAL);
}

#define DRAW_LOOP(condition)										\
{													\
	rect_t drawrect = gfx_rect(pos.x, pos.y, pxm->index_width, pxm->index_height);				\
	int offset, base_offset;									\
	int read_offset, base_read_offset;								\
	int x,y;											\
													\
	if (!pxm->index_data) {										\
		error("Attempt to draw control color %d on pixmap %d/%d/%d without index data", color, pxm->ID, pxm->loop, pxm->cel);						\
		return;											\
	}												\
													\
	if (_gfxop_clip(&drawrect, gfx_rect(0, 0, 320, 200)))						\
		return;											\
													\
	offset = base_offset = drawrect.x + drawrect.y * 320;						\
	read_offset = base_read_offset = (drawrect.x - pos.x) + ((drawrect.y - pos.y) * pxm->index_width);	\
													\
	for (y = 0; y < drawrect.height; y++) {								\
		for (x = 0; x < drawrect.width; x++)							\
			if (pxm->index_data[read_offset++] != pxm->color_key) {				\
				if (condition)								\
					map->index_data[offset++] = color;				\
				else									\
					++offset;							\
			} else										\
					++offset;								\
													\
		offset = base_offset += 320;								\
		read_offset = base_read_offset += pxm->index_width;					\
	}												\
}

static void _gfxop_draw_control(gfx_pixmap_t *map, gfx_pixmap_t *pxm, int color, Common::Point pos)
DRAW_LOOP(1) // Always draw

#ifdef PRECISE_PRIORITY_MAP
static void _gfxop_draw_priority(gfx_pixmap_t *map, gfx_pixmap_t *pxm, int color, Common::Point pos)
DRAW_LOOP(map->index_data[offset] < color) // Draw only lower priority
#endif

#undef DRAW_LOOP

static void _gfxop_install_pixmap(GfxDriver *driver, gfx_pixmap_t *pxm) {
	if (!driver->getMode()->palette)
		return;
	if (!pxm->palette)
		return;

	pxm->palette->mergeInto(driver->getMode()->palette);
	assert(pxm->palette->getParent() == driver->getMode()->palette);

	if (pxm->palette_revision != pxm->palette->getRevision())
		gfx_xlate_pixmap(pxm, driver->getMode());

	if (!driver->getMode()->palette->isDirty())
		return;

	// TODO: We probably want to only update the colours used by this pixmap
	// here. This will require updating the 'dirty' system.
	byte paletteData[4*256];
	const uint paletteSize = driver->getMode()->palette->size();
	for (uint i = 0; i < paletteSize; ++i) {
		const PaletteEntry& c = (*driver->getMode()->palette)[i];
		paletteData[4*i+0] = c.r;
		paletteData[4*i+1] = c.g;
		paletteData[4*i+2] = c.b;
		paletteData[4*i+3] = 255;
	}

	g_system->setPalette(paletteData, 0, paletteSize);
	driver->getMode()->palette->markClean();
}

static void _gfxop_draw_pixmap(GfxDriver *driver, gfx_pixmap_t *pxm, int priority, int control,
	rect_t src, rect_t dest, rect_t clip, int static_buf, gfx_pixmap_t *control_map, gfx_pixmap_t *priority_map) {
	rect_t clipped_dest = gfx_rect(dest.x, dest.y, dest.width, dest.height);

	if (control >= 0 || priority >= 0) {
		Common::Point original_pos = Common::Point(dest.x / driver->getMode()->scaleFactor, dest.y / driver->getMode()->scaleFactor);

		if (control >= 0)
			_gfxop_draw_control(control_map, pxm, control, original_pos);

#ifdef PRECISE_PRIORITY_MAP
		if (priority >= 0)
			_gfxop_draw_priority(priority_map, pxm, priority, original_pos);
#endif
	}

	if (_gfxop_clip(&clipped_dest, clip))
		return;

	src.x += clipped_dest.x - dest.x;
	src.y += clipped_dest.y - dest.y;
	src.width = clipped_dest.width;
	src.height = clipped_dest.height;

	_gfxop_install_pixmap(driver, pxm);

	DDIRTY(stderr, "\\-> Drawing to actual %d %d %d %d\n", clipped_dest.x / driver->getMode()->scaleFactor,
	       clipped_dest.y / driver->getMode()->scaleFactor, clipped_dest.width / driver->getMode()->scaleFactor, clipped_dest.height / driver->getMode()->scaleFactor);

	driver->drawPixmap(pxm, priority, src, clipped_dest, static_buf ? GFX_BUFFER_STATIC : GFX_BUFFER_BACK);
}

static void _gfxop_full_pointer_refresh(GfxState *state) {
	bool clipped = false;
	Common::Point mousePoint = g_system->getEventManager()->getMousePos();

	state->pointer_pos.x = mousePoint.x / state->driver->getMode()->scaleFactor;
	state->pointer_pos.y = mousePoint.y / state->driver->getMode()->scaleFactor;

	if (state->pointer_pos.x < state->pointerZone.left) {
		state->pointer_pos.x = state->pointerZone.left;
		clipped = true;
	} else if (state->pointer_pos.x >= state->pointerZone.right) {
		state->pointer_pos.x = state->pointerZone.right - 1;
		clipped = true;
	}

	if (state->pointer_pos.y < state->pointerZone.top) {
		state->pointer_pos.y = state->pointerZone.top;
		clipped = true;
	} else if (state->pointer_pos.y >= state->pointerZone.bottom) {
		state->pointer_pos.y = state->pointerZone.bottom - 1;
		clipped = true;
	}

	// FIXME: Do this only when mouse is grabbed?
	if (clipped)
		g_system->warpMouse(state->pointer_pos.x * state->driver->getMode()->scaleFactor,
							state->pointer_pos.y * state->driver->getMode()->scaleFactor);
}

static void _gfxop_buffer_propagate_box(GfxState *state, rect_t box, gfx_buffer_t buffer);

gfx_pixmap_t *_gfxr_get_cel(GfxState *state, int nr, int *loop, int *cel, int palette) {
	gfxr_view_t *view = state->gfxResMan->getView(nr, loop, cel, palette);

	gfxr_loop_t *indexed_loop;

	if (!view)
		return NULL;

	if (*loop >= view->loops_nr || *loop < 0) {
		warning("[GFX] Attempt to get cel from loop %d/%d inside view %d", *loop, view->loops_nr, nr);
		return NULL;
	}
	indexed_loop = view->loops + *loop;

	if (*cel >= indexed_loop->cels_nr || *cel < 0) {
		warning("[GFX] Attempt to get cel %d/%d from view %d/%d", *cel, indexed_loop->cels_nr, nr, *loop);
		return NULL;
	}

	return indexed_loop->cels[*cel]; // Yes, view->cels uses a malloced pointer list.
}

//** Dirty rectangle operations **

static void _gfxop_update_box(GfxState *state, rect_t box) {
	_gfxop_scale_rect(&box, state->driver->getMode());
	_gfxop_buffer_propagate_box(state, box, GFX_BUFFER_FRONT);
}

void gfxdr_add_dirty(DirtyRectList &list, rect_t box) {
	if (box.width < 0) {
		box.x += box.width;
		box.width = - box.width;
	}

	if (box.height < 0) {
		box.y += box.height;
		box.height = - box.height;
	}
#ifdef GFXOP_DEBUG_DIRTY
	fprintf(stderr, "Adding new dirty (%d %d %d %d)\n",
	        GFX_PRINT_RECT(box));
#endif
	if (_gfxop_clip(&box, gfx_rect(0, 0, 320, 200)))
		return;

	DirtyRectList::iterator dirty = list.begin();
	while (dirty != list.end()) {
		if (gfx_rects_overlap(*dirty, box)) {
			Common::Rect tmp = toCommonRect(box);
			tmp.extend(toCommonRect(*dirty));
			box = toSCIRect(tmp);

			dirty = list.erase(dirty);
		} else
			++dirty;
	}
	list.push_back(box);
}

static void _gfxop_add_dirty(GfxState *state, rect_t box) {
	if (state->disable_dirty)
		return;

	gfxdr_add_dirty(state->_dirtyRects, box);
}

static void _gfxop_add_dirty_x(GfxState *state, rect_t box) {
	// Extends the box size by one before adding (used for lines)
	if (box.width < 0)
		box.width--;
	else
		box.width++;

	if (box.height < 0)
		box.height--;
	else
		box.height++;

	_gfxop_add_dirty(state, box);
}

static void _gfxop_clear_dirty_rec(GfxState *state, DirtyRectList &dirtyRects) {
	DirtyRectList::iterator dirty = dirtyRects.begin();
	while (dirty != dirtyRects.end()) {

	#ifdef GFXOP_DEBUG_DIRTY
		fprintf(stderr, "\tClearing dirty (%d %d %d %d)\n", GFX_PRINT_RECT(*dirty));
	#endif
		if (!state->fullscreen_override)
			_gfxop_update_box(state, *dirty);
		++dirty;
	}
	dirtyRects.clear();
}

//** Exported operations **

static void init_aux_pixmap(gfx_pixmap_t **pixmap) {
	*pixmap = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320, 200, GFX_RESID_NONE, 0, 0));
	//  FIXME: don't duplicate this palette for every aux_pixmap
	(*pixmap)->palette = new Palette(default_colors, DEFAULT_COLORS_NR);
}

void gfxop_init(GfxState *state,
				gfx_options_t *options, ResourceManager *resMan,
				SciGuiScreen *screen, SciGuiPalette *palette, int scaleFactor) {
	state->options = options;
	state->visible_map = GFX_MASK_VISUAL;
	state->fullscreen_override = NULL; // No magical override
	state->options = options;
	state->disable_dirty = 0;
	state->_events.clear();
	state->pic = state->pic_unscaled = NULL;
	state->pic_nr = -1; // Set background pic number to an invalid value
	state->tag_mode = 0;
	state->pic_port_bounds = gfx_rect(0, 10, 320, 190);
	state->_dirtyRects.clear();

	state->driver = new GfxDriver(screen, scaleFactor);

	state->gfxResMan = new GfxResManager(state->options, state->driver, resMan, screen, palette);

	gfxop_set_clip_zone(state, gfx_rect(0, 0, 320, 200));
	state->pointerZone = Common::Rect(0, 0, 320, 200);

	init_aux_pixmap(&(state->control_map));
	init_aux_pixmap(&(state->priority_map));
	init_aux_pixmap(&(state->static_priority_map));
}

void gfxop_exit(GfxState *state) {
	state->gfxResMan->freeResManager();

	if (state->control_map) {
		gfx_free_pixmap(state->control_map);
		state->control_map = NULL;
	}

	if (state->priority_map) {
		gfx_free_pixmap(state->priority_map);
		state->priority_map = NULL;
	}

	if (state->static_priority_map) {
		gfx_free_pixmap(state->static_priority_map);
		state->static_priority_map = NULL;
	}

	delete state->driver;
}

static int _gfxop_scan_one_bitmask(gfx_pixmap_t *pixmap, rect_t zone) {
	int retval = 0;
	int pixmap_xscale = pixmap->index_width / 320;
	int pixmap_yscale = pixmap->index_height / 200;
	int line_width = pixmap_yscale * pixmap->index_width;
	int startindex = (line_width * zone.y) + (zone.x * pixmap_xscale);

	startindex += pixmap_xscale >> 1; // Center on X
	startindex += (pixmap_yscale >> 1) * pixmap->index_width; // Center on Y

	if (_gfxop_clip(&zone, gfx_rect(0, 0, pixmap->index_width, pixmap->index_height)))
		return 0;

	while (zone.height--) {
		int i;
		for (i = 0; i < (zone.width * pixmap_xscale); i += pixmap_xscale)
			retval |= (1 << ((pixmap->index_data[startindex + i]) & 0xf));

		startindex += line_width;
	}

	return retval;
}

int gfxop_scan_bitmask(GfxState *state, rect_t area, gfx_map_mask_t map) {
	gfxr_pic_t *pic = (state->pic_unscaled) ? state->pic_unscaled : state->pic;
	int retval = 0;

	_gfxop_clip(&area, gfx_rect(0, 10, 320, 200));

	if (area.width <= 0
	        || area.height <= 0)
		return 0;

	if (map & GFX_MASK_VISUAL)
		retval |= _gfxop_scan_one_bitmask(pic->visual_map, area);

	if (map & GFX_MASK_PRIORITY)
		retval |= _gfxop_scan_one_bitmask(state->priority_map, area);
	if (map & GFX_MASK_CONTROL)
		retval |= _gfxop_scan_one_bitmask(state->control_map, area);

	return retval;
}

#define MIN_X 0
#define MIN_Y 0
#define MAX_X 319
#define MAX_Y 199

void gfxop_set_clip_zone(GfxState *state, rect_t zone) {
	int xfact, yfact;

	DDIRTY(stderr, "-- Setting clip zone %d %d %d %d\n", GFX_PRINT_RECT(zone));

	xfact = state->driver->getMode()->scaleFactor;
	yfact = state->driver->getMode()->scaleFactor;

	if (zone.x < MIN_X) {
		zone.width -= (zone.x - MIN_X);
		zone.x = MIN_X;
	}

	if (zone.y < MIN_Y) {
		zone.height -= (zone.y - MIN_Y);
		zone.y = MIN_Y;
	}

	if (zone.x + zone.width > MAX_X)
		zone.width = MAX_X + 1 - zone.x;

	if (zone.y + zone.height > MAX_Y)
		zone.height = MAX_Y + 1 - zone.y;

	memcpy(&(state->clip_zone_unscaled), &zone, sizeof(rect_t));

	state->clip_zone.x = state->clip_zone_unscaled.x * xfact;
	state->clip_zone.y = state->clip_zone_unscaled.y * yfact;
	state->clip_zone.width = state->clip_zone_unscaled.width * xfact;
	state->clip_zone.height = state->clip_zone_unscaled.height * yfact;
}

void gfxop_set_color(GfxState *state, gfx_color_t *color, int r, int g, int b, int a, int priority, int control) {
	int mask = ((r >= 0 && g >= 0 && b >= 0) ? GFX_MASK_VISUAL : 0) | ((priority >= 0) ? GFX_MASK_PRIORITY : 0)
	           | ((control >= 0) ? GFX_MASK_CONTROL : 0);

	if (a >= GFXOP_ALPHA_THRESHOLD)
		mask &= ~GFX_MASK_VISUAL;

	color->mask = mask;

	color->priority = priority;
	color->control = control;

	if (mask & GFX_MASK_VISUAL) {
		color->visual.r = r;
		color->visual.g = g;
		color->visual.b = b;
		color->alpha = a;
		color->visual._parentIndex = SCREEN_PALETTE->findNearbyColor(r,g,b,true);
	}
}

// Wrapper for gfxop_set_color
void gfxop_set_color(GfxState *state, gfx_color_t *colorOut, gfx_color_t &colorIn) {
	gfxop_set_color(state, colorOut,
	              (colorIn.mask & GFX_MASK_VISUAL) ? colorIn.visual.r : -1,
	              (colorIn.mask & GFX_MASK_VISUAL) ? colorIn.visual.g : -1,
	              (colorIn.mask & GFX_MASK_VISUAL) ? colorIn.visual.b : -1,
	              (colorIn.mask & GFX_MASK_VISUAL) ? colorIn.alpha : -1,
	              (colorIn.mask & GFX_MASK_PRIORITY) ? colorIn.priority : -1,
	              (colorIn.mask & GFX_MASK_CONTROL) ? colorIn.control : -1);
}

void gfxop_set_system_color(GfxState *state, unsigned int index, gfx_color_t *color) {
	if (index >= SCREEN_PALETTE->size()) {
		error("Attempt to set invalid color index %02x as system color", color->visual.getParentIndex());
	}

	SCREEN_PALETTE->makeSystemColor(index, color->visual);
}

void gfxop_free_color(GfxState *state, gfx_color_t *color) {
	// FIXME: implement. (And call in the appropriate places!)
}


// Generic drawing operations

static int line_check_bar(int *start, int *length, int clipstart, int cliplength) {
	int overlength;

	if (*start < clipstart) {
		*length -= (clipstart - *start);
		*start = clipstart;
	}

	overlength = 1 + (*start + *length) - (clipstart + cliplength);

	if (overlength > 0)
		*length -= overlength;

	return (*length < 0);
}

static void clip_line_partial(float *start, float *end, float delta_val, float pos_val, float start_val, float end_val) {
	float my_start = (start_val - pos_val) * delta_val;
	float my_end = (end_val - pos_val) * delta_val;

	if (my_end < *end)
		*end = my_end;
	if (my_start > *start)
		*start = my_start;
}

static int line_clip(rect_t *line, rect_t clip, int xfact, int yfact) {
	// returns 1 if nothing is left, or 0 if part of the line is in the clip window
	// Compensate for line thickness (should match precisely)
	clip.width -= xfact;
	clip.height -= yfact;

	if (!line->width) { // vbar
		if (line->x < clip.x || line->x >= (clip.x + clip.width))
			return 1;

		return line_check_bar(&(line->y), &(line->height), clip.y, clip.height);

	} else {
		if (!line->height) {// hbar
			if (line->y < clip.y || line->y >= (clip.y + clip.height))
				return 1;

			return line_check_bar(&(line->x), &(line->width), clip.x, clip.width);

		} else { // "normal" line
			float start = 0.0f, end = 1.0f;
			float xv = (float)line->width;
			float yv = (float)line->height;

			if (line->width < 0)
				clip_line_partial(&start, &end, (float)(1.0 / xv), (float)line->x, (float)(clip.x + clip.width), (float)clip.x);
			else
				clip_line_partial(&start, &end, (float)(1.0 / xv), (float)line->x, (float)clip.x, (float)(clip.x + clip.width));

			if (line->height < 0)
				clip_line_partial(&start, &end, (float)(1.0 / yv), (float)line->y, (float)(clip.y + clip.height), (float)clip.y);
			else
				clip_line_partial(&start, &end, (float)(1.0 / yv), (float)line->y, (float)clip.y, (float)(clip.y + clip.height));

			line->x += (int)(xv * start);
			line->y += (int)(yv * start);

			line->width = (int)(xv * (end - start));
			line->height = (int)(yv * (end - start));

			return (start > 1.0f || end < 0.0f);
		}
	}

	return 0;
}

static int point_clip(Common::Point *start, Common::Point *end, rect_t clip, int xfact, int yfact) {
	rect_t line = gfx_rect(start->x, start->y, end->x - start->x, end->y - start->y);
	int retval = line_clip(&line, clip, xfact, yfact);

	start->x = line.x;
	start->y = line.y;

	end->x = line.x + line.width;
	end->y = line.y + line.height;

	return retval;
}

static void draw_line_to_control_map(GfxState *state, Common::Point start, Common::Point end, gfx_color_t color) {
	if (color.mask & GFX_MASK_CONTROL)
		if (!point_clip(&start, &end, state->clip_zone_unscaled, 0, 0))
			gfx_draw_line_pixmap_i(state->control_map, start, end, color.control);
}

static void simulate_stippled_line_draw(GfxDriver *driver, int skipone, Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode) {
	// Draws a stippled line if this isn't supported by the driver (skipone is ignored ATM)
	int xl = end.x - start.x;
	int yl = end.y - start.y;
	int stepwidth = (xl) ? driver->getMode()->scaleFactor : driver->getMode()->scaleFactor;
	int dbl_stepwidth = 2 * stepwidth;
	int linelength = (line_mode == GFX_LINE_MODE_FINE) ? stepwidth - 1 : 0;
	int16 *posvar;
	int length;
	int delta;
	int length_left;

	if (!xl) { // xl = 0, so we move along yl
		posvar = &start.y;
		length = yl;
		delta = (yl < 0) ? -dbl_stepwidth : dbl_stepwidth;
	} else {
		assert(!yl);  // We don't do diagonals; that's not needed ATM.
		posvar = &start.x;
		length = xl;
		delta = (xl < 0) ? -dbl_stepwidth : dbl_stepwidth;
	}

	length_left = length;

	if (skipone) {
		length_left -= stepwidth;
		*posvar += stepwidth;
	}

	length /= delta;

	length_left -= length * dbl_stepwidth;

	if (xl)
		xl = linelength;
	else
		yl = linelength;

	while (length--) {
		Common::Point nextpos = Common::Point(start.x + xl, start.y + yl);
		driver->drawLine(start, nextpos, color, line_mode, GFX_LINE_STYLE_NORMAL);
		*posvar += delta;
	}

	if (length_left) {
		Common::Point nextpos;

		if (length_left > stepwidth)
			length_left = stepwidth;

		if (xl)
			xl = length_left;
		else
			if (yl)
				yl = length_left;

		nextpos = Common::Point(start.x + xl, start.y + yl);

		driver->drawLine(start, nextpos, color, line_mode, GFX_LINE_STYLE_NORMAL);
	}
}

static void _gfxop_draw_line_clipped(GfxState *state, Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode,
	gfx_line_style_t line_style) {
	int skipone = (start.x ^ end.y) & 1; // Used for simulated line stippling

	_gfxop_full_pointer_refresh(state);

	// First, make sure that the line is normalized
	if (start.y > end.y) {
		Common::Point swap = start;
		start = end;
		end = swap;
	}

	if (start.x < state->clip_zone.x
	        || start.y < state->clip_zone.y
	        || end.x >= (state->clip_zone.x + state->clip_zone.width)
	        || end.y >= (state->clip_zone.y + state->clip_zone.height))
		if (point_clip(&start, &end, state->clip_zone, state->driver->getMode()->scaleFactor - 1, state->driver->getMode()->scaleFactor - 1))
			return; // Clipped off

	if (line_style == GFX_LINE_STYLE_STIPPLED) {
		if (start.x != end.x && start.y != end.y)
			error("[GFX] Attempt to draw stippled line which is neither an hbar nor a vbar: (%d,%d) -- (%d,%d)", start.x, start.y, end.x, end.y);
		simulate_stippled_line_draw(state->driver, skipone, start, end, color, line_mode);
	}

	state->driver->drawLine(start, end, color, line_mode, line_style);
}

void gfxop_draw_line(GfxState *state, Common::Point start, Common::Point end,
	gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	int xfact, yfact;

	_gfxop_add_dirty_x(state, gfx_rect(start.x, start.y, end.x - start.x, end.y - start.y));

	xfact = state->driver->getMode()->scaleFactor;
	yfact = state->driver->getMode()->scaleFactor;

	draw_line_to_control_map(state, start, end, color);

	_gfxop_scale_point(&start, state->driver->getMode());
	_gfxop_scale_point(&end, state->driver->getMode());

	if (line_mode == GFX_LINE_MODE_FINE) {
		start.x += xfact >> 1;
		start.y += yfact >> 1;

		end.x += xfact >> 1;
		end.y += yfact >> 1;
	}

	if (color.visual.getParentIndex() == -1)
		gfxop_set_color(state, &color, color);
	_gfxop_draw_line_clipped(state, start, end, color, line_mode, line_style);
}

void gfxop_draw_rectangle(GfxState *state, rect_t rect, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	int xfact, yfact;
	int x, y, xl, yl;
	Common::Point upper_left_u, upper_right_u, lower_left_u, lower_right_u;
	Common::Point upper_left, upper_right, lower_left, lower_right;

	_gfxop_full_pointer_refresh(state);

	xfact = state->driver->getMode()->scaleFactor;
	yfact = state->driver->getMode()->scaleFactor;

	int offset = line_mode == GFX_LINE_MODE_FINE ? 1 : 0;
	x = rect.x * xfact + (xfact - 1) * offset;
	y = rect.y * yfact + (yfact - 1) * offset;
	xl = offset + (rect.width - offset) * xfact;
	yl = offset + (rect.height - offset) * yfact;

	upper_left_u = Common::Point(rect.x, rect.y);
	upper_right_u = Common::Point(rect.x + rect.width, rect.y);
	lower_left_u = Common::Point(rect.x, rect.y + rect.height);
	lower_right_u = Common::Point(rect.x + rect.width, rect.y + rect.height);

	upper_left = Common::Point(x, y);
	upper_right = Common::Point(x + xl, y);
	lower_left = Common::Point(x, y + yl);
	lower_right = Common::Point(x + xl, y + yl);

#define PARTIAL_LINE(pt1, pt2)									\
	_gfxop_draw_line_clipped(state, pt1, pt2, color, line_mode, line_style);	\
	draw_line_to_control_map(state, pt1##_u, pt2##_u, color);				\
	_gfxop_add_dirty_x(state, gfx_rect(pt1##_u.x, pt1##_u.y, pt2##_u.x - pt1##_u.x, pt2##_u.y - pt1##_u.y))

	PARTIAL_LINE(upper_left, upper_right);
	PARTIAL_LINE(upper_right, lower_right);
	PARTIAL_LINE(lower_right, lower_left);
	PARTIAL_LINE(lower_left, upper_left);

#undef PARTIAL_LINE
}


void gfxop_draw_box(GfxState *state, rect_t box, gfx_color_t color1, gfx_color_t color2, gfx_box_shade_t shade_type) {
	GfxDriver *drv = state->driver;
	int reverse = 0; // switch color1 and color2
	float mod_offset = 0.0f, mod_breadth = 1.0f; // 0.0 to 1.0: Color adjustment
	gfx_rectangle_fill_t driver_shade_type;
	rect_t new_box;

	_gfxop_full_pointer_refresh(state);

	shade_type = GFX_BOX_SHADE_FLAT;


	_gfxop_add_dirty(state, box);

	if (color1.mask & GFX_MASK_CONTROL) {
		// Write control block, clipped by 320x200
		memcpy(&new_box, &box, sizeof(rect_t));
		_gfxop_clip(&new_box, gfx_rect(0, 0, 320, 200));

		gfx_draw_box_pixmap_i(state->control_map, new_box, color1.control);
	}

	_gfxop_scale_rect(&box, state->driver->getMode());

	if (!(color1.mask & (GFX_MASK_VISUAL | GFX_MASK_PRIORITY)))
		return;

	if (box.width <= 1 || box.height <= 1) {
		debugC(2, kDebugLevelGraphics, "Attempt to draw box with size %dx%d", box.width, box.height);
		return;
	}

	memcpy(&new_box, &box, sizeof(rect_t));
	if (_gfxop_clip(&new_box, state->clip_zone))
		return;

	switch (shade_type) {

	case GFX_BOX_SHADE_FLAT:
		driver_shade_type = GFX_SHADE_FLAT;
		break;

	case GFX_BOX_SHADE_LEFT:
		reverse = 1;
	case GFX_BOX_SHADE_RIGHT:
		driver_shade_type = GFX_SHADE_HORIZONTALLY;
		mod_offset = (float)(((new_box.x - box.x) * 1.0) / (box.width * 1.0));
		mod_breadth = (float)((new_box.width * 1.0) / (box.width * 1.0));
		break;

	case GFX_BOX_SHADE_UP:
		reverse = 1;
	case GFX_BOX_SHADE_DOWN:
		driver_shade_type = GFX_SHADE_VERTICALLY;
		mod_offset = (float)(((new_box.y - box.y) * 1.0) / (box.height * 1.0));
		mod_breadth = (float)((new_box.height * 1.0) / (box.height * 1.0));
		break;

	default:
		error("Invalid shade type: %d", shade_type);
	}


	if (reverse)
		mod_offset = (float)(1.0 - (mod_offset + mod_breadth));
	// Reverse offset if we have to interpret colors inversely

	if (shade_type == GFX_BOX_SHADE_FLAT) {
		color1.priority = 0;
		color1.control = 0;
		if (color1.visual.getParentIndex() == -1)
			gfxop_set_color(state, &color1, color1);
		drv->drawFilledRect(new_box, color1, color1, GFX_SHADE_FLAT);
		return;
	}
}

void gfxop_fill_box(GfxState *state, rect_t box, gfx_color_t color) {
	gfxop_draw_box(state, box, color, color, GFX_BOX_SHADE_FLAT);
}

static void _gfxop_buffer_propagate_box(GfxState *state, rect_t box, gfx_buffer_t buffer) {
	if (_gfxop_clip(&box, gfx_rect(0, 0, 320 * state->driver->getMode()->scaleFactor, 200 * state->driver->getMode()->scaleFactor)))
		return;

	state->driver->update(box, Common::Point(box.x, box.y), buffer);
}

extern int sci0_palette;

void gfxop_clear_box(GfxState *state, rect_t box) {
	_gfxop_full_pointer_refresh(state);
	_gfxop_add_dirty(state, box);
	DDIRTY(stderr, "[]  clearing box %d %d %d %d\n", GFX_PRINT_RECT(box));

	_gfxop_clip(&box, gfx_rect(0, 0, 320, 200));
#ifdef PRECISE_PRIORITY_MAP
	if (state->pic_unscaled)
		gfx_copy_pixmap_box_i(state->priority_map, state->static_priority_map, box);
#endif

	_gfxop_scale_rect(&box, state->driver->getMode());

	_gfxop_buffer_propagate_box(state, box, GFX_BUFFER_BACK);
}

void gfxop_set_visible_map(GfxState *state, gfx_map_mask_t visible_map) {
	switch (visible_map) {

	case GFX_MASK_VISUAL:
		state->fullscreen_override = NULL;
		if (visible_map != state->visible_map) {
			rect_t rect = gfx_rect(0, 0, 320, 200);
			gfxop_clear_box(state, rect);
			gfxop_update_box(state, rect);
		}
		break;

	case GFX_MASK_PRIORITY:
		state->fullscreen_override = state->priority_map;
		break;

	case GFX_MASK_CONTROL:
		state->fullscreen_override = state->control_map;
		break;

	default:
		warning("Invalid display map %d selected", visible_map);
		return;
	}

	state->visible_map = visible_map;
}

void gfxop_update(GfxState *state) {
	_gfxop_clear_dirty_rec(state, state->_dirtyRects);

	if (state->fullscreen_override) {
		// We've been asked to re-draw the active full-screen image, essentially.
		rect_t rect = gfx_rect(0, 0, 320, 200);
		gfx_xlate_pixmap(state->fullscreen_override, state->driver->getMode());
		gfxop_draw_pixmap(state, state->fullscreen_override, rect, Common::Point(0, 0));
		_gfxop_update_box(state, rect);
	}

	if (state->tag_mode) {
		// This usually happens after a pic and all resources have been drawn
		state->gfxResMan->freeTaggedResources();
		state->tag_mode = 0;
	}
}

void gfxop_update_box(GfxState *state, rect_t box) {
	if (state->disable_dirty)
		_gfxop_update_box(state, box);
	else
		_gfxop_add_dirty(state, box);

	gfxop_update(state);
}

void gfxop_enable_dirty_frames(GfxState *state) {
	state->disable_dirty = 0;
}

void gfxop_disable_dirty_frames(GfxState *state) {
	state->disable_dirty = 1;
}


// Pointer and IO ops
void gfxop_sleep(GfxState *state, uint32 msecs) {
	uint32 time;
	const uint32 wakeup_time = g_system->getMillis() + msecs;

	while (true) {
		// let backend process events and update the screen
		gfxop_get_event(state, SCI_EVT_PEEK);
		g_system->updateScreen();
		time = g_system->getMillis();
		if (time + 10 < wakeup_time) {
			g_system->delayMillis(10);
		} else {
			if (time < wakeup_time)
				g_system->delayMillis(wakeup_time - time);
			break;
		}

	}
}

void gfxop_set_pointer_cursor(GfxState *state, int nr) {
	if (nr == GFXOP_NO_POINTER) {
		state->driver->setPointer(NULL, NULL);
		return;
	}

	gfx_pixmap_t *new_pointer = state->gfxResMan->getCursor(nr);

	if (!new_pointer) {
		warning("[GFX] Attempt to set invalid pointer #%d\n", nr);
		return;
	}

	Common::Point p = Common::Point(new_pointer->xoffset, new_pointer->yoffset);
	state->driver->setPointer(new_pointer, &p);
}

void gfxop_set_pointer_view(GfxState *state, int nr, int loop, int cel, Common::Point *hotspot) {
	// FIXME: For now, don't palettize pointers
	gfx_pixmap_t *new_pointer = state->gfxResMan->getView(nr, &loop, &cel, 0)->loops[loop].cels[cel];

	// Eco Quest 1 uses a 1x1 transparent cursor to hide the cursor from the user. Some scalers don't seem to support this.
	if (new_pointer->width < 2 || new_pointer->height < 2) {
		state->driver->setPointer(NULL, NULL);
		return;
	}

	if (hotspot)
		state->driver->setPointer(new_pointer, hotspot);
	else {
		// Compute hotspot from xoffset/yoffset
		Common::Point p = Common::Point(new_pointer->xoffset + (new_pointer->width >> 1), new_pointer->yoffset + new_pointer->height - 1);
		state->driver->setPointer(new_pointer, &p);
	}
}

void gfxop_set_pointer_zone(GfxState *state, Common::Rect rect) {
	state->pointerZone = rect;
}

#define SCANCODE_ROWS_NR 3

struct scancode_row {
	int offset;
	const char *keys;
} scancode_rows[SCANCODE_ROWS_NR] = {
	{0x10, "QWERTYUIOP[]"},
	{0x1e, "ASDFGHJKL;'\\"},
	{0x2c, "ZXCVBNM,./"}
};

static int _gfxop_scancode(int ch) {
	// Calculates a PC keyboard scancode from a character */
	int row;
	int c = toupper((char)ch);

	for (row = 0; row < SCANCODE_ROWS_NR; row++) {
		const char *keys = scancode_rows[row].keys;
		int offset = scancode_rows[row].offset;

		while (*keys) {
			if (*keys == c)
				return offset << 8;

			offset++;
			keys++;
		}
	}

	return ch;
}

int _gfxop_shiftify(int c) {
	char shifted_numbers[] = ")!@#$%^&*(";

	if (c < 256) {
		c = toupper((char)c);

		if (c >= 'A' && c <= 'Z')
			return c;

		if (c >= '0' && c <= '9')
			return shifted_numbers[c-'0'];

		switch (c) {
		case SCI_K_TAB:
			return SCI_K_SHIFT_TAB;
		case ']':
			return '}';
		case '[':
			return '{';
		case '`':
			return '~';
		case '-':
			return '_';
		case '=':
			return '+';
		case ';':
			return ':';
		case '\'':
			return '"';
		case '\\':
			return '|';
		case ',':
			return '<';
		case '.':
			return '>';
		case '/':
			return '?';
		default:
			return c; // No match
		}
	}

	if (c >= SCI_K_F1 && c <= SCI_K_F10)
		return c + 25;

	return c;
}

static int _gfxop_numlockify(int c) {
	switch (c) {
	case SCI_K_DELETE:
		return '.';
	case SCI_K_INSERT:
		return '0';
	case SCI_K_END:
		return '1';
	case SCI_K_DOWN:
		return '2';
	case SCI_K_PGDOWN:
		return '3';
	case SCI_K_LEFT:
		return '4';
	case SCI_K_CENTER:
		return '5';
	case SCI_K_RIGHT:
		return '6';
	case SCI_K_HOME:
		return '7';
	case SCI_K_UP:
		return '8';
	case SCI_K_PGUP:
		return '9';
	default:
		return c; // Unchanged
	}
}

static sci_event_t scummvm_get_event(GfxDriver *drv) {
	static int _modifierStates = 0;	// FIXME: Avoid non-const global vars
	sci_event_t input = { SCI_EVT_NONE, 0, 0, 0 };

	Common::EventManager *em = g_system->getEventManager();
	Common::Event ev;

	bool found = em->pollEvent(ev);
	Common::Point p = ev.mouse;

	// Don't generate events for mouse movement
	while (found && ev.type == Common::EVENT_MOUSEMOVE) {
		found = em->pollEvent(ev);
	}

	if (found && !ev.synthetic && ev.type != Common::EVENT_MOUSEMOVE) {
		int modifiers = em->getModifierState();

		// We add the modifier key status to buckybits
		// SDL sends a keydown event if a modifier key is turned on and a keyup event if it's off
		//
		// FIXME: This code is semi-bogus. It only records the modifier key being *pressed*.
		// It does not track correctly whether capslock etc. is active. To do that, we
		// would have to record the fact that the modifier was pressed in global var,
		// and also watch for Common::EVENT_KEYUP events.
		// But this is still not quite good enough, because not all events might
		// pass through here (e.g. the GUI might be running with its own event loop).
		//
		// The best solution likely would be to add code to the EventManager class
		// for tracking which keys are pressed and which are not...
		if (ev.type == Common::EVENT_KEYDOWN || ev.type == Common::EVENT_KEYUP) {
			switch (ev.kbd.keycode) {
			case Common::KEYCODE_CAPSLOCK:
				if (ev.type == Common::EVENT_KEYDOWN) {
					_modifierStates |= SCI_EVM_CAPSLOCK;
				} else {
					_modifierStates &= ~SCI_EVM_CAPSLOCK;
				}
				break;
			case Common::KEYCODE_NUMLOCK:
				if (ev.type == Common::EVENT_KEYDOWN) {
					_modifierStates |= SCI_EVM_NUMLOCK;
				} else {
					_modifierStates &= ~SCI_EVM_NUMLOCK;
				}
				break;
			case Common::KEYCODE_SCROLLOCK:
				if (ev.type == Common::EVENT_KEYDOWN) {
					_modifierStates |= SCI_EVM_SCRLOCK;
				} else {
					_modifierStates &= ~SCI_EVM_SCRLOCK;
				}
				break;
			default:
				break;
			}
		}
		//TODO: SCI_EVM_INSERT

		input.buckybits =
		    ((modifiers & Common::KBD_ALT) ? SCI_EVM_ALT : 0) |
		    ((modifiers & Common::KBD_CTRL) ? SCI_EVM_CTRL : 0) |
		    ((modifiers & Common::KBD_SHIFT) ? SCI_EVM_LSHIFT | SCI_EVM_RSHIFT : 0) |
			_modifierStates;

		switch (ev.type) {
			// Keyboard events
		case Common::EVENT_KEYDOWN:
			input.data = ev.kbd.keycode;
			input.character = ev.kbd.ascii;

			// Debug console
			if (ev.kbd.flags == Common::KBD_CTRL && ev.kbd.keycode == Common::KEYCODE_d) {
				// Open debug console
				Console *con = ((Sci::SciEngine*)g_engine)->getSciDebugger();
				con->attach();

				// Clear keyboard event
				input.type = SCI_EVT_NONE;
				input.character = 0;
				input.data = 0;
				input.buckybits = 0;

				return input;
			}

			if (!(input.data & 0xFF00)) {
				// Directly accept most common keys without conversion
				input.type = SCI_EVT_KEYBOARD;
				if (input.data == Common::KEYCODE_TAB) {
					// Tab
					input.type = SCI_EVT_KEYBOARD;
					input.data = SCI_K_TAB;
					if (input.buckybits & (SCI_EVM_LSHIFT | SCI_EVM_RSHIFT))
						input.character = SCI_K_SHIFT_TAB;
					else
						input.character = SCI_K_TAB;
				}
			} else if ((input.data >= Common::KEYCODE_F1) && input.data <= Common::KEYCODE_F10) {
				// F1-F10
				input.type = SCI_EVT_KEYBOARD;
				// SCI_K_F1 == 59 << 8
				// SCI_K_SHIFT_F1 == 84 << 8
				input.data = SCI_K_F1 + ((input.data - Common::KEYCODE_F1)<<8);
				if (input.buckybits & (SCI_EVM_LSHIFT | SCI_EVM_RSHIFT))
					input.character = input.data + 25;
				else
					input.character = input.data;
			} else {
				// Special keys that need conversion
				input.type = SCI_EVT_KEYBOARD;
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_UP:
					input.data = SCI_K_UP;
					break;
				case Common::KEYCODE_DOWN:
					input.data = SCI_K_DOWN;
					break;
				case Common::KEYCODE_RIGHT:
					input.data = SCI_K_RIGHT;
					break;
				case Common::KEYCODE_LEFT:
					input.data = SCI_K_LEFT;
					break;
				case Common::KEYCODE_INSERT:
					input.data = SCI_K_INSERT;
					break;
				case Common::KEYCODE_HOME:
					input.data = SCI_K_HOME;
					break;
				case Common::KEYCODE_END:
					input.data = SCI_K_END;
					break;
				case Common::KEYCODE_PAGEUP:
					input.data = SCI_K_PGUP;
					break;
				case Common::KEYCODE_PAGEDOWN:
					input.data = SCI_K_PGDOWN;
					break;
				case Common::KEYCODE_DELETE:
					input.data = SCI_K_DELETE;
					break;
				// Keypad keys
				case Common::KEYCODE_KP8:	// up
					if (!(_modifierStates & SCI_EVM_NUMLOCK))
						input.data = SCI_K_UP;
					break;
				case Common::KEYCODE_KP2:	// down
					if (!(_modifierStates & SCI_EVM_NUMLOCK))
						input.data = SCI_K_DOWN;
					break;
				case Common::KEYCODE_KP6:	// right
					if (!(_modifierStates & SCI_EVM_NUMLOCK))
						input.data = SCI_K_RIGHT;
					break;
				case Common::KEYCODE_KP4:	// left
					if (!(_modifierStates & SCI_EVM_NUMLOCK))
						input.data = SCI_K_LEFT;
					break;
				case Common::KEYCODE_KP5:	// center
					if (!(_modifierStates & SCI_EVM_NUMLOCK))
						input.data = SCI_K_CENTER;
					break;
				default:
					input.type = SCI_EVT_NONE;
					break;
				}
				input.character = input.data;
			}
			break;

			// Mouse events
		case Common::EVENT_LBUTTONDOWN:
			input.type = SCI_EVT_MOUSE_PRESS;
			input.data = 1;
			break;
		case Common::EVENT_RBUTTONDOWN:
			input.type = SCI_EVT_MOUSE_PRESS;
			input.data = 2;
			break;
		case Common::EVENT_LBUTTONUP:
			input.type = SCI_EVT_MOUSE_RELEASE;
			input.data = 1;
			break;
		case Common::EVENT_RBUTTONUP:
			input.type = SCI_EVT_MOUSE_RELEASE;
			input.data = 2;
			break;

			// Misc events
		case Common::EVENT_QUIT:
			input.type = SCI_EVT_QUIT;
			break;

		default:
			break;
		}
	}

	return input;
}

sci_event_t gfxop_get_event(GfxState *state, unsigned int mask) {
	//sci_event_t error_event = { SCI_EVT_ERROR, 0, 0, 0 };
	sci_event_t event = { 0, 0, 0, 0 };

	_gfxop_full_pointer_refresh(state);

	// Update the screen here, since it's called very often
	g_system->updateScreen();

	// Get all queued events from graphics driver
	do {
		event = scummvm_get_event(state->driver);
		if (event.type != SCI_EVT_NONE)
			state->_events.push_back(event);
	} while (event.type != SCI_EVT_NONE);

	// Search for matching event in queue
	Common::List<sci_event_t>::iterator iter = state->_events.begin();
	while (iter != state->_events.end() && !((*iter).type & mask))
		++iter;

	if (iter != state->_events.end()) {
		// Event found
		event = *iter;

		// If not peeking at the queue, remove the event
		if (!(mask & SCI_EVT_PEEK)) {
			state->_events.erase(iter);
		}
	} else {
		// No event found: we must return a SCI_EVT_NONE event.

		// Because event.type is SCI_EVT_NONE already here,
		// there is no need to change it.
	}

	_gfxop_full_pointer_refresh(state);

	if (event.type == SCI_EVT_KEYBOARD) {
		// Do we still have to translate the key?

		event.character = event.data;

		// Scancodify if appropriate
		if (event.buckybits & SCI_EVM_ALT)
			event.character = _gfxop_scancode(event.character);

		// Shift if appropriate
		else if (((event.buckybits & (SCI_EVM_RSHIFT | SCI_EVM_LSHIFT)) && !(event.buckybits & SCI_EVM_CAPSLOCK))
		         || (!(event.buckybits & (SCI_EVM_RSHIFT | SCI_EVM_LSHIFT)) && (event.buckybits & SCI_EVM_CAPSLOCK)))
			event.character = _gfxop_shiftify(event.character);

		// Numlockify if appropriate
		else if (event.buckybits & SCI_EVM_NUMLOCK)
			event.data = _gfxop_numlockify(event.data);
	}

	return event;
}

// View operations

int gfxop_lookup_view_get_loops(GfxState *state, int nr) {
	int loop = 0, cel = 0;
	gfxr_view_t *view = NULL;

	view = state->gfxResMan->getView(nr, &loop, &cel, 0);

	if (!view) {
		error("[GFX] Attempt to retrieve number of loops from invalid view %d", nr);
	}

	return view->loops_nr;
}

int gfxop_lookup_view_get_cels(GfxState *state, int nr, int loop) {
	int real_loop = loop, cel = 0;
	gfxr_view_t *view = NULL;

	view = state->gfxResMan->getView(nr, &real_loop, &cel, 0);

	if (!view) {
		warning("[GFX] Attempt to retrieve number of cels from invalid/broken view %d", nr);
		return 0;
	} else if (real_loop != loop) {
		warning("[GFX] Loop number was corrected from %d to %d in view %d", loop, real_loop, nr);
	}

	return view->loops[real_loop].cels_nr;
}

void gfxop_check_cel(GfxState *state, int nr, int *loop, int *cel) {
	gfxr_view_t *testView = state->gfxResMan->getView(nr, loop, cel, 0);

	if (!testView)
		error("[GFX] Attempt to verify loop/cel values for invalid view %d", nr);
}

void gfxop_get_cel_parameters(GfxState *state, int nr, int loop, int cel, int *width, int *height, Common::Point *offset) {
	gfxr_view_t *view = NULL;
	gfx_pixmap_t *pxm = NULL;

	view = state->gfxResMan->getView(nr, &loop, &cel, 0);

	if (!view)
		error("[GFX] Attempt to get cel parameters for invalid view %d", nr);

	pxm = view->loops[loop].cels[cel];
	*width = pxm->index_width;
	*height = pxm->index_height;
	offset->x = pxm->xoffset;
	offset->y = pxm->yoffset;
}

static void _gfxop_draw_cel_buffer(GfxState *state, int nr, int loop, int cel, Common::Point pos, gfx_color_t color, int static_buf, int palette) {
	int priority = (color.mask & GFX_MASK_PRIORITY) ? color.priority : -1;
	int control = (color.mask & GFX_MASK_CONTROL) ? color.control : -1;
	gfxr_view_t *view = NULL;
	gfx_pixmap_t *pxm = NULL;
	int old_x, old_y;

	view = state->gfxResMan->getView(nr, &loop, &cel, palette);

	if (!view) {
		warning("[GFX] Attempt to draw loop/cel %d/%d in invalid view %d\n", loop, cel, nr);
		return;
	}
	pxm = view->loops[loop].cels[cel];

	old_x = pos.x -= pxm->xoffset;
	old_y = pos.y -= pxm->yoffset;

	pos.x *= state->driver->getMode()->scaleFactor;
	pos.y *= state->driver->getMode()->scaleFactor;

	if (!static_buf)
		_gfxop_add_dirty(state, gfx_rect(old_x, old_y, pxm->index_width, pxm->index_height));

	_gfxop_draw_pixmap(state->driver, pxm, priority, control, gfx_rect(0, 0, pxm->width, pxm->height),
	                          gfx_rect(pos.x, pos.y, pxm->width, pxm->height), state->clip_zone, static_buf , state->control_map,
	                          static_buf ? state->static_priority_map : state->priority_map);
}

void gfxop_draw_cel(GfxState *state, int nr, int loop, int cel, Common::Point pos, gfx_color_t color, int palette) {
	_gfxop_draw_cel_buffer(state, nr, loop, cel, pos, color, 0, palette);
}

void gfxop_draw_cel_static(GfxState *state, int nr, int loop, int cel, Common::Point pos, gfx_color_t color, int palette) {
	rect_t oldclip = state->clip_zone;

	state->clip_zone = gfx_rect_fullscreen;
	_gfxop_scale_rect(&(state->clip_zone), state->driver->getMode());
	gfxop_draw_cel_static_clipped(state, nr, loop, cel, pos, color, palette);
	// Except that the area it's clipped against is... unusual ;-)
	state->clip_zone = oldclip;
}

void gfxop_draw_cel_static_clipped(GfxState *state, int nr, int loop, int cel, Common::Point pos, gfx_color_t color, int palette) {
	_gfxop_draw_cel_buffer(state, nr, loop, cel, pos, color, 1, palette);
}

// Pic operations

static void _gfxop_set_pic(GfxState *state) {
	gfx_copy_pixmap_box_i(state->control_map, state->pic->control_map, gfx_rect(0, 0, 320, 200));
	gfx_copy_pixmap_box_i(state->priority_map, state->pic_unscaled->priority_map, gfx_rect(0, 0, 320, 200));
	gfx_copy_pixmap_box_i(state->static_priority_map, state->pic_unscaled->priority_map, gfx_rect(0, 0, 320, 200));

	// Reset global palette to this PIC's palette
	// FIXME: The _gfxop_install_pixmap call below updates the OSystem palette.
	// This is too soon, since it causes brief palette corruption until the
	// screen is updated too. (Possibly related: EngineState::pic_not_valid .)
	if (state->pic->visual_map->palette) {
		state->pic->visual_map->palette->forceInto(SCREEN_PALETTE);
		_gfxop_install_pixmap(state->driver, state->pic->visual_map);
	}

#ifdef CUSTOM_GRAPHICS_OPTIONS
	if (state->options->pic0_unscaled)
#endif
		state->pic->priority_map = gfx_pixmap_scale_index_data(state->pic->priority_map, state->driver->getMode());
	state->driver->setStaticBuffer(state->pic->visual_map, state->pic->priority_map);
}

int *gfxop_get_pic_metainfo(GfxState *state) {
	return (state->pic) ? state->pic->priorityTable : NULL;
}

void gfxop_new_pic(GfxState *state, int nr, int flags, int default_palette) {
	state->gfxResMan->tagResources();
	state->tag_mode = 1;
	state->palette_nr = default_palette;
	state->pic = state->gfxResMan->getPic(nr, GFX_MASK_VISUAL, flags, default_palette, true);

	if (state->driver->getMode()->scaleFactor == 1 && state->driver->getMode()->scaleFactor == 1) {
		state->pic_unscaled = state->pic;
	} else {
		state->pic_unscaled = state->gfxResMan->getPic(nr, GFX_MASK_VISUAL, flags, default_palette, false);
	}

	if (!state->pic || !state->pic_unscaled) {
		warning("Could not retrieve background pic %d", nr);
		if (state->pic) {
			warning("  -- Inconsistency: scaled pic _was_ retrieved!");
		}

		if (state->pic_unscaled) {
			warning("  -- Inconsistency: unscaled pic _was_ retrieved!");
		}

		error("Error occured in gfxop_new_pic()");
		state->pic = state->pic_unscaled = NULL;
	}

	state->pic_nr = nr;

	_gfxop_set_pic(state);
}

void gfxop_add_to_pic(GfxState *state, int nr, int flags, int default_palette) {
	if (!state->pic)
		error("Attempt to add to pic with no pic active");

	state->pic = state->gfxResMan->addToPic(state->pic_nr, nr, flags, state->palette_nr, default_palette);

	if (!state->pic)
		error("Could not add pic #%d to pic #%d", state->pic_nr, nr);

	state->pic_unscaled = state->gfxResMan->addToPic(state->pic_nr, nr, flags, state->palette_nr, default_palette);

	_gfxop_set_pic(state);
}

// Text operations

// FIXME: only the resstate member of state is used -- inline the reference by:
// replacing GfxState* state parameter with gfx_resstate_t* gfxResourceState and adjust callers accordingly
int gfxop_get_font_height(GfxState *state, int font_nr) {
	gfx_bitmap_font_t *font = state->gfxResMan->getFont(font_nr);

	if (!font)
		error("gfxop_get_font_height(): Font number %d not found", font_nr);

	return font->line_height;
}

void gfxop_get_text_params(GfxState *state, int font_nr, const char *text, int maxwidth, int *width, int *height, int text_flags,
						  int *lines_nr, int *lineheight, int *lastline_width) {
	Common::Array<TextFragment> fragments;
	bool textsplits;
	gfx_bitmap_font_t *font;

	font = state->gfxResMan->getFont(font_nr);

	if (!font)
		error("Attempt to calculate text size with invalid font #%d", font_nr);

#ifdef CUSTOM_GRAPHICS_OPTIONS
	textsplits = gfxr_font_calculate_size(fragments, font, maxwidth, text, width, height, lineheight, lastline_width,
	                                      (state->options->workarounds & GFX_WORKAROUND_WHITESPACE_COUNT) | text_flags);
#else
	textsplits = gfxr_font_calculate_size(fragments, font, maxwidth, text, width, height, lineheight, lastline_width, text_flags);
#endif

	if (!textsplits)
		error("Could not calculate text size");

	if (lines_nr)
		*lines_nr = fragments.size();
}

TextHandle *gfxop_new_text(GfxState *state, int font_nr, const Common::String &text, int maxwidth, gfx_alignment_t halign,
								  gfx_alignment_t valign, gfx_color_t color1, gfx_color_t color2, gfx_color_t bg_color, int flags) {
	TextHandle *handle;
	gfx_bitmap_font_t *font;

	// mapping text colors to palette
	gfxop_set_color(state, &color1, color1);
	gfxop_set_color(state, &color2, color2);
	gfxop_set_color(state, &bg_color, bg_color);

	font = state->gfxResMan->getFont(font_nr);

	if (!font) {
		error("Attempt to draw text with invalid font #%d", font_nr);
		return NULL;
	}

	handle = new TextHandle();

	handle->_text = text;
	handle->halign = halign;
	handle->valign = valign;
	handle->line_height = font->line_height;

	bool result;
#ifdef CUSTOM_GRAPHICS_OPTIONS
	result = gfxr_font_calculate_size(handle->lines, font, maxwidth, handle->_text.c_str(), &(handle->width), &(handle->height),
	                             NULL, NULL, ((state->options->workarounds & GFX_WORKAROUND_WHITESPACE_COUNT) ?
	                              kFontCountWhitespace : 0) | flags);
#else
	result = gfxr_font_calculate_size(handle->lines, font, maxwidth, handle->_text.c_str(), &(handle->width), &(handle->height),
	                             NULL, NULL, flags);
#endif

	if (!result) {
		error("Could not calculate text parameters in font #%d", font_nr);
		delete handle;
		return NULL;
	}

	if (flags & kFontNoNewlines) {
		handle->lines.resize(1);
		handle->lines[0].length = text.size();
	}

	handle->text_pixmaps.resize(handle->lines.size());

	for (uint i = 0; i < handle->lines.size(); i++) {
		int chars_nr = handle->lines[i].length;

		handle->text_pixmaps[i] = gfxr_draw_font(font, handle->lines[i].offset, chars_nr,
		                          (color1.mask & GFX_MASK_VISUAL) ? &color1.visual : NULL,
		                          (color2.mask & GFX_MASK_VISUAL) ? &color2.visual : NULL,
		                          (bg_color.mask & GFX_MASK_VISUAL) ? &bg_color.visual : NULL);

		if (!handle->text_pixmaps[i]) {
			error("Failed to draw text pixmap for line %d/%d", i, handle->lines.size());
			delete handle;
			return NULL;
		}
	}

	handle->font = font;

	handle->priority = (color1.mask & GFX_MASK_PRIORITY) ? color1.priority : -1;
	handle->control = (color1.mask & GFX_MASK_CONTROL) ? color1.control : -1;

	return handle;
}

void gfxop_free_text(GfxState *state, TextHandle *handle) {
	delete handle;
}

TextHandle::TextHandle() {
	line_height = 0;
	font = 0;

	width = height = 0;

	priority = control = 0;
	halign = valign = ALIGN_BOTTOM;
}

TextHandle::~TextHandle() {
	for (uint j = 0; j < text_pixmaps.size(); j++)
		if (text_pixmaps[j])
			gfx_free_pixmap(text_pixmaps[j]);
}

void gfxop_draw_text(GfxState *state, TextHandle *handle, rect_t zone) {
	int line_height;
	rect_t pos;
	_gfxop_full_pointer_refresh(state);

	if (!handle)
		error("Attempt to draw text with NULL handle");

	if (handle->lines.empty()) {
		debugC(2, kDebugLevelGraphics, "Skipping draw_text operation because number of lines is zero\n");
		return;
	}

	_gfxop_scale_rect(&zone, state->driver->getMode());

	line_height = handle->line_height * state->driver->getMode()->scaleFactor;

	pos.y = zone.y;

	switch (handle->valign) {

	case ALIGN_TOP:
		break;

	case ALIGN_CENTER:
		pos.y += (zone.height - (line_height * handle->lines.size())) >> 1;
		break;

	case ALIGN_BOTTOM:
		pos.y += (zone.height - (line_height * handle->lines.size()));
		break;

	default:
		error("Invalid vertical alignment %d", handle->valign);
	}

	for (uint i = 0; i < handle->lines.size(); i++) {

		gfx_pixmap_t *pxm = handle->text_pixmaps[i];

		if (!pxm->data) {
			gfx_xlate_pixmap(pxm, state->driver->getMode());
		}
		if (!pxm)
			error("Could not find text pixmap %d/%d", i, handle->lines.size());

		pos.x = zone.x;

		switch (handle->halign) {

		case ALIGN_LEFT:
			break;

		case ALIGN_CENTER:
			pos.x += (zone.width - pxm->width) >> 1;
			break;

		case ALIGN_RIGHT:
			pos.x += (zone.width - pxm->width);
			break;

		default:
			error("Invalid vertical alignment %d", handle->valign);
		}

		pos.width = pxm->width;
		pos.height = pxm->height;

		_gfxop_add_dirty(state, pos);
		_gfxop_draw_pixmap(state->driver, pxm, handle->priority, handle->control,
		                   gfx_rect(0, 0, pxm->width, pxm->height), pos, state->clip_zone, 0, state->control_map, state->priority_map);

		pos.y += line_height;
	}
}

gfx_pixmap_t *gfxop_grab_pixmap(GfxState *state, rect_t area) {
	gfx_pixmap_t *pixmap = NULL;
	rect_t resultzone; // Ignored for this application
	_gfxop_full_pointer_refresh(state);

	_gfxop_scale_rect(&area, state->driver->getMode());
	_gfxop_grab_pixmap(state, &pixmap, area.x, area.y, area.width, area.height, 0, &resultzone);

	return pixmap;
}

void gfxop_draw_pixmap(GfxState *state, gfx_pixmap_t *pxm, rect_t zone, Common::Point pos) {
	rect_t target = gfx_rect(pos.x, pos.y, zone.width, zone.height);

	if (!pxm)
		error("Attempt to draw NULL pixmap");

	_gfxop_full_pointer_refresh(state);
	_gfxop_add_dirty(state, target);

	_gfxop_scale_rect(&zone, state->driver->getMode());
	_gfxop_scale_rect(&target, state->driver->getMode());

	return _gfxop_draw_pixmap(state->driver, pxm, -1, -1, zone, target, gfx_rect(0, 0, 320*state->driver->getMode()->scaleFactor,
	                                   200*state->driver->getMode()->scaleFactor), 0, NULL, NULL);
}

void gfxop_free_pixmap(GfxState *state, gfx_pixmap_t *pxm) {
	gfx_free_pixmap(pxm);
}

} // End of namespace Sci
