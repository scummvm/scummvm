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

#include "sci/sci_memory.h"
#include "sci/gfx/operations.h"
#include "sci/gfx/font.h"
#include "sci/sci.h"	// for g_engine, evil hack

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

gfx_pixmap_color_t default_colors[DEFAULT_COLORS_NR] = {{GFX_COLOR_SYSTEM, 0x00, 0x00, 0x00}, {GFX_COLOR_SYSTEM, 0x00, 0x00, 0xaa},
	{GFX_COLOR_SYSTEM, 0x00, 0xaa, 0x00}, {GFX_COLOR_SYSTEM, 0x00, 0xaa, 0xaa},
	{GFX_COLOR_SYSTEM, 0xaa, 0x00, 0x00}, {GFX_COLOR_SYSTEM, 0xaa, 0x00, 0xaa},
	{GFX_COLOR_SYSTEM, 0xaa, 0x55, 0x00}, {GFX_COLOR_SYSTEM, 0xaa, 0xaa, 0xaa},
	{GFX_COLOR_SYSTEM, 0x55, 0x55, 0x55}, {GFX_COLOR_SYSTEM, 0x55, 0x55, 0xff},
	{GFX_COLOR_SYSTEM, 0x55, 0xff, 0x55}, {GFX_COLOR_SYSTEM, 0x55, 0xff, 0xff},
	{GFX_COLOR_SYSTEM, 0xff, 0x55, 0x55}, {GFX_COLOR_SYSTEM, 0xff, 0x55, 0xff},
	{GFX_COLOR_SYSTEM, 0xff, 0xff, 0x55}, {GFX_COLOR_SYSTEM, 0xff, 0xff, 0xff}
}; // "Normal" EGA

#define POINTER_VISIBLE_BUT_CLIPPED 2

// Performs basic checks that apply to most functions
#define BASIC_CHECKS(error_retval) \
if (!state) { \
	GFXERROR("Null state!\n"); \
	return error_retval; \
} \
if (!state->driver) { \
	GFXERROR("GFX driver invalid!\n"); \
	return error_retval; \
}

// How to determine whether colors have to be allocated
#define PALETTE_MODE state->driver->mode->palette

//#define GFXOP_DEBUG_DIRTY

// Internal operations

static void _gfxop_scale_rect(rect_t *rect, gfx_mode_t *mode) {
	int xfact = mode->xfact;
	int yfact = mode->yfact;

	rect->x *= xfact;
	rect->y *= yfact;
	rect->width *= xfact;
	rect->height *= yfact;
}

static void _gfxop_scale_point(Common::Point *point, gfx_mode_t *mode) {
	int xfact = mode->xfact;
	int yfact = mode->yfact;

	point->x *= xfact;
	point->y *= yfact;
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

static int _gfxop_grab_pixmap(gfx_state_t *state, gfx_pixmap_t **pxmp, int x, int y,
							  int xl, int yl, int priority, rect_t *zone) {
	// Returns 1 if the resulting data size was zero, GFX_OK or an error code otherwise */
	int xfact = state->driver->mode->xfact;
	int yfact = state->driver->mode->yfact;
	int unscaled_xl = (xl + xfact - 1) / xfact;
	int unscaled_yl = (yl + yfact - 1) / yfact;
	*zone = gfx_rect(x, y, xl, yl);

	if (_gfxop_clip(zone, gfx_rect(0, 0, 320 * state->driver->mode->xfact, 200 * state->driver->mode->yfact)))
		return GFX_ERROR;

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
		gfx_pixmap_alloc_data(*pxmp, state->driver->mode);
	}
	return state->driver->grab_pixmap(state->driver, *zone, *pxmp, priority ? GFX_MASK_PRIORITY : GFX_MASK_VISUAL);
}

#define DRAW_LOOP(condition)										\
{													\
	rect_t drawrect = gfx_rect(pos.x, pos.y, pxm->index_width, pxm->index_height);				\
	int offset, base_offset;									\
	int read_offset, base_read_offset;								\
	int x,y;											\
													\
	if (!pxm->index_data) {										\
		GFXERROR("Attempt to draw control color %d on pixmap %d/%d/%d without index data", color, pxm->ID, pxm->loop, pxm->cel);						\
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

static int _gfxop_install_pixmap(gfx_driver_t *driver, gfx_pixmap_t *pxm) {
	if (!driver->mode->palette) return GFX_OK;
	if (!pxm->palette) return GFX_OK;

	assert(pxm->palette->getParent() == driver->mode->palette);

	if (!driver->mode->palette->isDirty()) return GFX_OK;

	// TODO: We probably want to only update the colours used by this pixmap
	// here. This will require updating the 'dirty' system.
	for (unsigned int i = 0; i < driver->mode->palette->size(); ++i) {
		const PaletteEntry& c = (*driver->mode->palette)[i];
		driver->set_palette(driver, i, c.r, c.g, c.b);
	}

	driver->install_palette(driver, driver->mode->palette);
	driver->mode->palette->markClean();
	return GFX_OK;
}

static int _gfxop_draw_pixmap(gfx_driver_t *driver, gfx_pixmap_t *pxm, int priority, int control,
	rect_t src, rect_t dest, rect_t clip, int static_buf, gfx_pixmap_t *control_map, gfx_pixmap_t *priority_map) {
	int error;
	rect_t clipped_dest = gfx_rect(dest.x, dest.y, dest.width, dest.height);

	if (control >= 0 || priority >= 0) {
		Common::Point original_pos = Common::Point(dest.x / driver->mode->xfact, dest.y / driver->mode->yfact);

		if (control >= 0)
			_gfxop_draw_control(control_map, pxm, control, original_pos);

#ifdef PRECISE_PRIORITY_MAP
		if (priority >= 0)
			_gfxop_draw_priority(priority_map, pxm, priority, original_pos);
#endif
	}

	if (_gfxop_clip(&clipped_dest, clip))
		return GFX_OK;

	src.x += clipped_dest.x - dest.x;
	src.y += clipped_dest.y - dest.y;
	src.width = clipped_dest.width;
	src.height = clipped_dest.height;

	error = _gfxop_install_pixmap(driver, pxm);
	if (error)
		return error;

	DDIRTY(stderr, "\\-> Drawing to actual %d %d %d %d\n", clipped_dest.x / driver->mode->xfact,
	       clipped_dest.y / driver->mode->yfact, clipped_dest.width / driver->mode->xfact, clipped_dest.height / driver->mode->yfact);

	error = driver->draw_pixmap(driver, pxm, priority, src, clipped_dest, static_buf ? GFX_BUFFER_STATIC : GFX_BUFFER_BACK);

	if (error) {
		GFXERROR("driver->draw_pixmap() returned error!\n");
		return error;
	}

	return GFX_OK;
}

static void _gfxop_full_pointer_refresh(gfx_state_t *state) {
	state->pointer_pos.x = state->driver->pointer_x / state->driver->mode->xfact;
	state->pointer_pos.y = state->driver->pointer_y / state->driver->mode->yfact;
}

static int _gfxop_buffer_propagate_box(gfx_state_t *state, rect_t box, gfx_buffer_t buffer);

gfx_pixmap_t *_gfxr_get_cel(gfx_state_t *state, int nr, int *loop, int *cel, int palette) {
	gfxr_view_t *view = gfxr_get_view(state->resstate, nr, loop, cel, palette);
	gfxr_loop_t *indexed_loop;

	if (!view)
		return NULL;

	if (*loop >= view->loops_nr || *loop < 0) {
		GFXWARN("Attempt to get cel from loop %d/%d inside view %d\n", *loop, view->loops_nr, nr);
		return NULL;
	}
	indexed_loop = view->loops + *loop;

	if (*cel >= indexed_loop->cels_nr || *cel < 0) {
		GFXWARN("Attempt to get cel %d/%d from view %d/%d\n", *cel, indexed_loop->cels_nr, nr, *loop);
		return NULL;
	}

	return indexed_loop->cels[*cel]; // Yes, view->cels uses a malloced pointer list.
}

//** Dirty rectangle operations **

static int _gfxop_update_box(gfx_state_t *state, rect_t box) {
	int retval;
	_gfxop_scale_rect(&box, state->driver->mode);

	if ((retval = _gfxop_buffer_propagate_box(state, box, GFX_BUFFER_FRONT))) {
		GFXERROR("Error occured while propagating box (%d,%d,%d,%d) to front buffer\n", box.x, box.y, box.width, box.height);
		return retval;
	}
	return GFX_OK;
}

static gfx_dirty_rect_t *_rect_create(rect_t box) {
	gfx_dirty_rect_t *rect;

	rect = (gfx_dirty_rect_t *)sci_malloc(sizeof(gfx_dirty_rect_t));
	rect->next = NULL;
	rect->rect = box;

	return rect;
}

gfx_dirty_rect_t *gfxdr_add_dirty(gfx_dirty_rect_t *base, rect_t box, int strategy) {
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
		return base;

	switch (strategy) {

	case GFXOP_DIRTY_FRAMES_ONE:
		if (base) {
			Common::Rect tmp = toCommonRect(box);
			tmp.extend(toCommonRect(base->rect));
			base->rect = toSCIRect(tmp);
		} else {
			base = _rect_create(box);
		}
		break;

	case GFXOP_DIRTY_FRAMES_CLUSTERS: {
		gfx_dirty_rect_t **rectp = &(base);

		while (*rectp) {
			if (gfx_rects_overlap((*rectp)->rect, box)) {
				gfx_dirty_rect_t *next = (*rectp)->next;
				Common::Rect tmp = toCommonRect((*rectp)->rect);
				tmp.extend(toCommonRect(box));
				box = toSCIRect(tmp);

				free(*rectp);
				*rectp = next;
			} else
				rectp = &((*rectp)->next);
		}
		*rectp = _rect_create(box);

	}
	break;

	default:
		GFXERROR("Attempt to use invalid dirty frame mode %d!\nPlease refer to gfx_options.h.", strategy);

	}

	return base;
}

static void _gfxop_add_dirty(gfx_state_t *state, rect_t box) {
	if (state->disable_dirty)
		return;

	state->dirty_rects = gfxdr_add_dirty(state->dirty_rects, box, state->options->dirty_frames);
}

static void _gfxop_add_dirty_x(gfx_state_t *state, rect_t box) {
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

static int _gfxop_clear_dirty_rec(gfx_state_t *state, gfx_dirty_rect_t *rect) {
	int retval;

	if (!rect)
		return GFX_OK;

#ifdef GFXOP_DEBUG_DIRTY
	fprintf(stderr, "\tClearing dirty (%d %d %d %d)\n",
	        GFX_PRINT_RECT(rect->rect));
#endif
	if (!state->fullscreen_override)
		retval = _gfxop_update_box(state, rect->rect);
	else
		retval = GFX_OK;

	retval |= _gfxop_clear_dirty_rec(state, rect->next);

	free(rect);
	return retval;
}

//** Exported operations **

static void init_aux_pixmap(gfx_pixmap_t **pixmap) {
	*pixmap = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320, 200, GFX_RESID_NONE, 0, 0));
	//  FIXME: don't duplicate this palette for every aux_pixmap
	(*pixmap)->palette = new Palette(default_colors, DEFAULT_COLORS_NR);
}

static int _gfxop_init_common(gfx_state_t *state, gfx_options_t *options, ResourceManager *resManager) {
	gfxr_init_static_palette();

	state->options = options;

	if (!((state->resstate = gfxr_new_resource_manager(state->version, state->options, state->driver, resManager)))) {
		GFXERROR("Failed to initialize resource manager!\n");
		return GFX_FATAL;
	}

	state->static_palette = NULL;

	if (state->version < SCI_VERSION_01_VGA) {
		state->static_palette = gfx_sci0_pic_colors->getref();
	} else if (state->version == SCI_VERSION_1_1 || state->version == SCI_VERSION_32) {
		GFXDEBUG("Palettes are not yet supported in this SCI version\n");
	} else {
		Resource *res = state->resstate->resManager->findResource(kResourceTypePalette, 999, 0);
		if (res && res->data)
			state->static_palette = gfxr_read_pal1(res->id, res->data, res->size);
	}

	state->visible_map = GFX_MASK_VISUAL;
	state->fullscreen_override = NULL; // No magical override
	gfxop_set_clip_zone(state, gfx_rect(0, 0, 320, 200));

	init_aux_pixmap(&(state->control_map));
	init_aux_pixmap(&(state->priority_map));
	init_aux_pixmap(&(state->static_priority_map));

	state->options = options;
	state->disable_dirty = 0;
	state->_events.clear();

	state->pic = state->pic_unscaled = NULL;

	state->pic_nr = -1; // Set background pic number to an invalid value

	state->tag_mode = 0;

	state->dirty_rects = NULL;

	return GFX_OK;
}

int gfxop_init_default(gfx_state_t *state, gfx_options_t *options, ResourceManager *resManager) {
	BASIC_CHECKS(GFX_FATAL);
	if (state->driver->init(state->driver))
		return GFX_FATAL;

	return _gfxop_init_common(state, options, resManager);
}

int gfxop_init(gfx_state_t *state, int xfact, int yfact, gfx_color_mode_t bpp,
	gfx_options_t *options, ResourceManager *resManager) {
	int color_depth = bpp ? bpp : 1;
	int initialized = 0;

	BASIC_CHECKS(GFX_FATAL);

	do {
		if (!state->driver->init_specific(state->driver, xfact, yfact, color_depth))
			initialized = 1;
		else
			color_depth++;
	} while (!initialized && color_depth < 9 && !bpp);

	if (!initialized)
		return GFX_FATAL;

	return _gfxop_init_common(state, options, resManager);
}

int gfxop_set_parameter(gfx_state_t *state, char *attribute, char *value) {
	BASIC_CHECKS(GFX_FATAL);

	return state->driver->set_parameter(state->driver, attribute, value);
}

int gfxop_exit(gfx_state_t *state) {
	BASIC_CHECKS(GFX_ERROR);
	gfxr_free_resource_manager(state->resstate);

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

	state->driver->exit(state->driver);

	return GFX_OK;
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

int gfxop_scan_bitmask(gfx_state_t *state, rect_t area, gfx_map_mask_t map) {
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

int gfxop_set_clip_zone(gfx_state_t *state, rect_t zone) {
	int xfact, yfact;
	BASIC_CHECKS(GFX_ERROR);

	DDIRTY(stderr, "-- Setting clip zone %d %d %d %d\n", GFX_PRINT_RECT(zone));

	xfact = state->driver->mode->xfact;
	yfact = state->driver->mode->yfact;

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

	return GFX_OK;
}

int gfxop_set_color(gfx_state_t *state, gfx_color_t *color, int r, int g, int b, int a, int priority, int control) {
	int mask = ((r >= 0 && g >= 0 && b >= 0) ? GFX_MASK_VISUAL : 0) | ((priority >= 0) ? GFX_MASK_PRIORITY : 0)
	           | ((control >= 0) ? GFX_MASK_CONTROL : 0);

	BASIC_CHECKS(GFX_FATAL);

	if (PALETTE_MODE && a >= GFXOP_ALPHA_THRESHOLD)
		mask &= ~GFX_MASK_VISUAL;

	color->mask = mask;

	color->priority = priority;
	color->control = control;

	if (mask & GFX_MASK_VISUAL) {
		color->visual.r = r;
		color->visual.g = g;
		color->visual.b = b;
		color->alpha = a;

		if (PALETTE_MODE) {
			color->visual.parent_index = state->driver->mode->palette->findNearbyColor(r,g,b,true);
		}
	}

	return GFX_OK;
}

// Wrapper for gfxop_set_color
int gfxop_set_color(gfx_state_t *state, gfx_color_t *colorOut, gfx_color_t &colorIn) {
	if (colorIn.mask & GFX_MASK_VISUAL)
		return gfxop_set_color(state, colorOut, colorIn.visual.r, colorIn.visual.g, colorIn.visual.b,
			colorIn.alpha, colorIn.priority, colorIn.control);
	else
		return gfxop_set_color(state, colorOut, -1, -1, -1, colorIn.alpha,
			colorIn.priority, colorIn.control);
}

int gfxop_set_system_color(gfx_state_t *state, unsigned int index, gfx_color_t *color) {
	BASIC_CHECKS(GFX_FATAL);

	if (!PALETTE_MODE)
		return GFX_OK;

	if (index >= state->driver->mode->palette->size()) {
		GFXERROR("Attempt to set invalid color index %02x as system color\n", color->visual.parent_index);
		return GFX_ERROR;
	}

	state->driver->mode->palette->makeSystemColor(index, color->visual);

	return GFX_OK;
}

int gfxop_free_color(gfx_state_t *state, gfx_color_t *color) {
	// FIXME: implement. (And call in the appropriate places!)
	return GFX_OK;
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
			float start = 0.0, end = 1.0;
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

			return (start > 1.0 || end < 0.0);
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

static void draw_line_to_control_map(gfx_state_t *state, Common::Point start, Common::Point end, gfx_color_t color) {
	if (color.mask & GFX_MASK_CONTROL)
		if (!point_clip(&start, &end, state->clip_zone_unscaled, 0, 0))
			gfx_draw_line_pixmap_i(state->control_map, start, end, color.control);
}

static int simulate_stippled_line_draw(gfx_driver_t *driver, int skipone, Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode) {
	// Draws a stippled line if this isn't supported by the driver (skipone is ignored ATM)
	int xl = end.x - start.x;
	int yl = end.y - start.y;
	int stepwidth = (xl) ? driver->mode->xfact : driver->mode->yfact;
	int dbl_stepwidth = 2 * stepwidth;
	int linelength = (line_mode == GFX_LINE_MODE_FINE) ? stepwidth - 1 : 0;
	int *posvar;
	int length;
	int delta;
	int length_left;

	if (!xl) { // xl = 0, so we move along yl
		posvar = (int *) &start.y;
		length = yl;
		delta = (yl < 0) ? -dbl_stepwidth : dbl_stepwidth;
	} else {
		assert(!yl);  // We don't do diagonals; that's not needed ATM.
		posvar = (int *) &start.x;
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
		int retval;
		Common::Point nextpos = Common::Point(start.x + xl, start.y + yl);

		if ((retval = driver->draw_line(driver, start, nextpos, color, line_mode, GFX_LINE_STYLE_NORMAL))) {
			GFXERROR("Failed to draw partial stippled line (%d,%d) -- (%d,%d)\n", start.x, start.y, nextpos.x, nextpos.y);
			return retval;
		}
		*posvar += delta;
	}

	if (length_left) {
		int retval;
		Common::Point nextpos;

		if (length_left > stepwidth)
			length_left = stepwidth;

		if (xl)
			xl = length_left;
		else
			if (yl)
				yl = length_left;

		nextpos = Common::Point(start.x + xl, start.y + yl);

		if ((retval = driver->draw_line(driver, start, nextpos, color, line_mode, GFX_LINE_STYLE_NORMAL))) {
			GFXERROR("Failed to draw partial stippled line (%d,%d) -- (%d,%d)\n", start.x, start.y, nextpos.x, nextpos.y);
			return retval;
		}
	}

	return GFX_OK;
}

static int _gfxop_draw_line_clipped(gfx_state_t *state, Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode,
	gfx_line_style_t line_style) {
	int retval;
	int skipone = (start.x ^ end.y) & 1; // Used for simulated line stippling

	BASIC_CHECKS(GFX_FATAL);
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
		if (point_clip(&start, &end, state->clip_zone, state->driver->mode->xfact - 1, state->driver->mode->yfact - 1))
			return GFX_OK; // Clipped off

	if (line_style == GFX_LINE_STYLE_STIPPLED) {
		if (start.x != end.x && start.y != end.y) {
			GFXWARN("Attempt to draw stippled line which is neither an hbar nor a vbar: (%d,%d) -- (%d,%d)\n", start.x, start.y, end.x, end.y);
			return GFX_ERROR;
		}
		if (!(state->driver->capabilities & GFX_CAPABILITY_STIPPLED_LINES))
			return simulate_stippled_line_draw(state->driver, skipone, start, end, color, line_mode);
	}

	if ((retval = state->driver->draw_line(state->driver, start, end, color, line_mode, line_style))) {
		GFXERROR("Failed to draw line (%d,%d) -- (%d,%d)\n", start.x, start.y, end.x, end.y);
		return retval;
	}

	return GFX_OK;
}

int gfxop_draw_line(gfx_state_t *state, Common::Point start, Common::Point end,
	gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	int xfact, yfact;

	BASIC_CHECKS(GFX_FATAL);
	_gfxop_add_dirty_x(state, gfx_rect(start.x, start.y, end.x - start.x, end.y - start.y));

	xfact = state->driver->mode->xfact;
	yfact = state->driver->mode->yfact;

	draw_line_to_control_map(state, start, end, color);

	_gfxop_scale_point(&start, state->driver->mode);
	_gfxop_scale_point(&end, state->driver->mode);

	if (line_mode == GFX_LINE_MODE_FINE) {
		start.x += xfact >> 1;
		start.y += yfact >> 1;

		end.x += xfact >> 1;
		end.y += yfact >> 1;
	}

	if (color.visual.parent_index == GFX_COLOR_INDEX_UNMAPPED)
		gfxop_set_color(state, &color, color);
	return _gfxop_draw_line_clipped(state, start, end, color, line_mode, line_style);
}

int gfxop_draw_rectangle(gfx_state_t *state, rect_t rect, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	int retval = 0;
	int xfact, yfact;
	int xunit, yunit;
	int x, y, xl, yl;
	Common::Point upper_left_u, upper_right_u, lower_left_u, lower_right_u;
	Common::Point upper_left, upper_right, lower_left, lower_right;

	BASIC_CHECKS(GFX_FATAL);
	_gfxop_full_pointer_refresh(state);

	xfact = state->driver->mode->xfact;
	yfact = state->driver->mode->yfact;

	if (line_mode == GFX_LINE_MODE_FINE) {
		xunit = yunit = 1;
		xl = 1 + (rect.width - 1) * xfact;
		yl = 1 + (rect.height - 1) * yfact;
		x = rect.x * xfact + (xfact - 1);
		y = rect.y * yfact + (yfact - 1);
	} else {
		xunit = xfact;
		yunit = yfact;
		xl = rect.width * xfact;
		yl = rect.height * yfact;
		x = rect.x * xfact;
		y = rect.y * yfact;
	}

	upper_left_u = Common::Point(rect.x, rect.y);
	upper_right_u = Common::Point(rect.x + rect.width, rect.y);
	lower_left_u = Common::Point(rect.x, rect.y + rect.height);
	lower_right_u = Common::Point(rect.x + rect.width, rect.y + rect.height);

	upper_left = Common::Point(x, y);
	upper_right = Common::Point(x + xl, y);
	lower_left = Common::Point(x, y + yl);
	lower_right = Common::Point(x + xl, y + yl);

#define PARTIAL_LINE(pt1, pt2)									\
	retval |= _gfxop_draw_line_clipped(state, pt1, pt2, color, line_mode, line_style);	\
	draw_line_to_control_map(state, pt1##_u, pt2##_u, color);				\
	_gfxop_add_dirty_x(state, gfx_rect(pt1##_u.x, pt1##_u.y, pt2##_u.x - pt1##_u.x, pt2##_u.y - pt1##_u.y))

	PARTIAL_LINE(upper_left, upper_right);
	PARTIAL_LINE(upper_right, lower_right);
	PARTIAL_LINE(lower_right, lower_left);
	PARTIAL_LINE(lower_left, upper_left);

#undef PARTIAL_LINE
	if (retval) {
		GFXERROR("Failed to draw rectangle (%d,%d)+(%d,%d)\n", rect.x, rect.y, rect.width, rect.height);
		return retval;
	}

	return GFX_OK;
}


#define COLOR_MIX(type, dist) ((color1.type * dist) + (color2.type * (1.0 - dist)))

int gfxop_draw_box(gfx_state_t *state, rect_t box, gfx_color_t color1, gfx_color_t color2, gfx_box_shade_t shade_type) {
	gfx_driver_t *drv = state->driver;
	int reverse = 0; // switch color1 and color2
	float mod_offset = 0.0, mod_breadth = 1.0; // 0.0 to 1.0: Color adjustment
	gfx_rectangle_fill_t driver_shade_type;
	rect_t new_box;

	BASIC_CHECKS(GFX_FATAL);
	_gfxop_full_pointer_refresh(state);

	if (PALETTE_MODE || !(state->driver->capabilities & GFX_CAPABILITY_SHADING))
		shade_type = GFX_BOX_SHADE_FLAT;


	_gfxop_add_dirty(state, box);

	if (color1.mask & GFX_MASK_CONTROL) {
		// Write control block, clipped by 320x200
		memcpy(&new_box, &box, sizeof(rect_t));
		_gfxop_clip(&new_box, gfx_rect(0, 0, 320, 200));

		gfx_draw_box_pixmap_i(state->control_map, new_box, color1.control);
	}

	_gfxop_scale_rect(&box, state->driver->mode);

	if (!(color1.mask & (GFX_MASK_VISUAL | GFX_MASK_PRIORITY)))
		return GFX_OK;

	if (box.width <= 1 || box.height <= 1) {
		GFXDEBUG("Attempt to draw box with size %dx%d\n", box.width, box.height);
		return GFX_OK;
	}

	memcpy(&new_box, &box, sizeof(rect_t));
	if (_gfxop_clip(&new_box, state->clip_zone))
		return GFX_OK;

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
		GFXERROR("Invalid shade type: %d\n", shade_type);
		return GFX_ERROR;
	}


	if (reverse)
		mod_offset = (float)(1.0 - (mod_offset + mod_breadth));
	// Reverse offset if we have to interpret colors inversely

	if (shade_type == GFX_BOX_SHADE_FLAT) {
		color1.priority = 0;
		color1.control = 0;
		if (color1.visual.parent_index == GFX_COLOR_INDEX_UNMAPPED)
			gfxop_set_color(state, &color1, color1);
		return drv->draw_filled_rect(drv, new_box, color1, color1, GFX_SHADE_FLAT);
	} else {
		if (PALETTE_MODE) {
			GFXWARN("Attempting to draw shaded box in palette mode!\n");
			return GFX_ERROR;
		}

		gfx_color_t draw_color1; // CHECKME
		gfx_color_t draw_color2;
		gfxop_set_color(state, &draw_color1, 0, 0, 0, 0, 0, 0);
		gfxop_set_color(state, &draw_color2, 0, 0, 0, 0, 0, 0);

		draw_color1.mask = draw_color2.mask = color1.mask;
		draw_color1.priority = draw_color2.priority = color1.priority;

		if (draw_color1.mask & GFX_MASK_VISUAL) {
			draw_color1.visual.r = (uint8) COLOR_MIX(visual.r, mod_offset);
			draw_color1.visual.g = (uint8) COLOR_MIX(visual.g, mod_offset);
			draw_color1.visual.b = (uint8) COLOR_MIX(visual.b, mod_offset);
			draw_color1.alpha = (uint8) COLOR_MIX(alpha, mod_offset);

			mod_offset += mod_breadth;

			draw_color2.visual.r = (uint8) COLOR_MIX(visual.r, mod_offset);
			draw_color2.visual.g = (uint8) COLOR_MIX(visual.g, mod_offset);
			draw_color2.visual.b = (uint8) COLOR_MIX(visual.b, mod_offset);
			draw_color2.alpha = (uint8) COLOR_MIX(alpha, mod_offset);
		}
		if (reverse)
			return drv->draw_filled_rect(drv, new_box, draw_color2, draw_color1, driver_shade_type);
		else
			return drv->draw_filled_rect(drv, new_box, draw_color1, draw_color2, driver_shade_type);
	}
}
#undef COLOR_MIX

int gfxop_fill_box(gfx_state_t *state, rect_t box, gfx_color_t color) {
	return gfxop_draw_box(state, box, color, color, GFX_BOX_SHADE_FLAT);
}

static int _gfxop_buffer_propagate_box(gfx_state_t *state, rect_t box, gfx_buffer_t buffer) {
	int error;

	if (_gfxop_clip(&box, gfx_rect(0, 0, 320 * state->driver->mode->xfact, 200 * state->driver->mode->yfact)))
		return GFX_OK;

	if ((error = state->driver->update(state->driver, box, Common::Point(box.x, box.y), buffer))) {
		GFXERROR("Error occured while updating region (%d,%d,%d,%d) in buffer %d\n", box.x, box.y, box.width, box.height, buffer);
		return error;
	}

	return GFX_OK;
}

extern int sci0_palette;

int gfxop_clear_box(gfx_state_t *state, rect_t box) {
	BASIC_CHECKS(GFX_FATAL);
	_gfxop_full_pointer_refresh(state);
	_gfxop_add_dirty(state, box);
	DDIRTY(stderr, "[]  clearing box %d %d %d %d\n", GFX_PRINT_RECT(box));
	if (box.x == 29 && box.y == 77 && (sci0_palette == 1)) {
		BREAKPOINT();
	}

	_gfxop_clip(&box, gfx_rect(0, 0, 320, 200));
#ifdef PRECISE_PRIORITY_MAP
	if (state->pic_unscaled)
		gfx_copy_pixmap_box_i(state->priority_map, state->static_priority_map, box);
#endif

	_gfxop_scale_rect(&box, state->driver->mode);

	return _gfxop_buffer_propagate_box(state, box, GFX_BUFFER_BACK);
}

int gfxop_set_visible_map(gfx_state_t *state, gfx_map_mask_t visible_map) {
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
		fprintf(stderr, "Invalid display map %d selected!\n", visible_map);
		return GFX_ERROR;
	}

	state->visible_map = visible_map;

	return GFX_OK;
}

int gfxop_update(gfx_state_t *state) {
	int retval;

	BASIC_CHECKS(GFX_FATAL);

	retval = _gfxop_clear_dirty_rec(state, state->dirty_rects);

	state->dirty_rects = NULL;

	if (state->fullscreen_override) {
		// We've been asked to re-draw the active full-screen image, essentially.
		rect_t rect = gfx_rect(0, 0, 320, 200);
		gfx_xlate_pixmap(state->fullscreen_override, state->driver->mode, GFX_XLATE_FILTER_NONE);
		gfxop_draw_pixmap(state, state->fullscreen_override, rect, Common::Point(0, 0));
		retval |= _gfxop_update_box(state, rect);
	}

	if (retval) {
		GFXERROR("Clearing the dirty rectangles failed!\n");
	}

	if (state->tag_mode) {
		// This usually happens after a pic and all resources have been drawn
		gfxr_free_tagged_resources(state->resstate);
		state->tag_mode = 0;
	}

	return retval;
}

int gfxop_update_box(gfx_state_t *state, rect_t box) {
	BASIC_CHECKS(GFX_FATAL);

	if (state->disable_dirty)
		_gfxop_update_box(state, box);
	else
		_gfxop_add_dirty(state, box);

	return gfxop_update(state);
}

int gfxop_enable_dirty_frames(gfx_state_t *state) {
	BASIC_CHECKS(GFX_ERROR);
	state->disable_dirty = 0;

	return GFX_OK;
}

int gfxop_disable_dirty_frames(gfx_state_t *state) {
	BASIC_CHECKS(GFX_ERROR);

	state->disable_dirty = 1;

	return GFX_OK;
}


// Pointer and IO ops
int gfxop_sleep(gfx_state_t *state, uint32 msecs) {
	BASIC_CHECKS(GFX_FATAL);

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

	return GFX_OK;
}

int _gfxop_set_pointer(gfx_state_t *state, gfx_pixmap_t *pxm) {
	BASIC_CHECKS(GFX_FATAL);

	state->driver->set_pointer(state->driver, pxm);

	return GFX_OK;
}

int gfxop_set_pointer_cursor(gfx_state_t *state, int nr) {
	gfx_pixmap_t *new_pointer = NULL;

	BASIC_CHECKS(GFX_FATAL);

	if (nr == GFXOP_NO_POINTER)
		new_pointer = NULL;
	else {
		// FIXME: the initialization of the GFX resource manager should
		// be pushed up, and it shouldn't occur here
		GfxResManager *_gfx = new GfxResManager(state->resstate);
		new_pointer = _gfx->getCursor(nr);
		delete _gfx;

		if (!new_pointer) {
			GFXWARN("Attempt to set invalid pointer #%d\n", nr);
		}
	}

	return _gfxop_set_pointer(state, new_pointer);
}

int gfxop_set_pointer_view(gfx_state_t *state, int nr, int loop, int cel, Common::Point  *hotspot) {
	int real_loop = loop;
	int real_cel = cel;
	gfx_pixmap_t *new_pointer = NULL;

	BASIC_CHECKS(GFX_FATAL);

	new_pointer = _gfxr_get_cel(state, nr, &real_loop, &real_cel, 0); // FIXME: For now, don't palettize pointers

	if (hotspot) {
		new_pointer->xoffset = hotspot->x;
		new_pointer->yoffset = hotspot->y;
	}

	if (!new_pointer) {
		GFXWARN("Attempt to set invalid pointer #%d\n", nr);
		return GFX_ERROR;
	} else {
		if (real_loop != loop || real_cel != cel) {
			GFXDEBUG("Changed loop/cel from %d/%d to %d/%d in view %d\n", loop, cel, real_loop, real_cel, nr);
		}
		return _gfxop_set_pointer(state, new_pointer);
	}
}

int gfxop_set_pointer_position(gfx_state_t *state, Common::Point pos) {
	BASIC_CHECKS(GFX_ERROR);

	state->pointer_pos = pos;

	if (pos.x > 320 || pos.y > 200) {
		GFXWARN("Attempt to place pointer at invalid coordinates (%d, %d)\n", pos.x, pos.y);
		return 0; // Not fatal
	}

	state->driver->pointer_x = pos.x * state->driver->mode->xfact;
	state->driver->pointer_y = pos.y * state->driver->mode->yfact;
	g_system->warpMouse(state->driver->pointer_x, state->driver->pointer_y);

	_gfxop_full_pointer_refresh(state);
	return 0;
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

	switch (c) {
	case SCI_K_F1 :
		return SCI_K_SHIFT_F1;
	case SCI_K_F2 :
		return SCI_K_SHIFT_F2;
	case SCI_K_F3 :
		return SCI_K_SHIFT_F3;
	case SCI_K_F4 :
		return SCI_K_SHIFT_F4;
	case SCI_K_F5 :
		return SCI_K_SHIFT_F5;
	case SCI_K_F6 :
		return SCI_K_SHIFT_F6;
	case SCI_K_F7 :
		return SCI_K_SHIFT_F7;
	case SCI_K_F8 :
		return SCI_K_SHIFT_F8;
	case SCI_K_F9 :
		return SCI_K_SHIFT_F9;
	case SCI_K_F10 :
		return SCI_K_SHIFT_F10;
	}

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

static sci_event_t scummvm_get_event(gfx_driver_t *drv) {
	static int _modifierStates = 0;	// FIXME: EVIL HACK
	sci_event_t input = { SCI_EVT_NONE, 0, 0, 0 };

	Common::EventManager *em = g_system->getEventManager();
	Common::Event ev;

	bool found = em->pollEvent(ev);
	Common::Point p = ev.mouse;

	// Don't generate events for mouse movement
	while (found && ev.type == Common::EVENT_MOUSEMOVE) {
		drv->pointer_x = ev.mouse.x;
		drv->pointer_y = ev.mouse.y;
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

				((Sci::SciEngine*)g_engine)->_console->attach();
				((Sci::SciEngine*)g_engine)->_console->onFrame();

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
					input.character = input.data + SCI_K_SHIFT_F1 - SCI_K_F1;
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
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;
		case Common::EVENT_RBUTTONDOWN:
			input.type = SCI_EVT_MOUSE_PRESS;
			input.data = 2;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;
		case Common::EVENT_LBUTTONUP:
			input.type = SCI_EVT_MOUSE_RELEASE;
			input.data = 1;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;
		case Common::EVENT_RBUTTONUP:
			input.type = SCI_EVT_MOUSE_RELEASE;
			input.data = 2;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
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

sci_event_t gfxop_get_event(gfx_state_t *state, unsigned int mask) {
	sci_event_t error_event = { SCI_EVT_ERROR, 0, 0, 0 };
	sci_event_t event = { 0, 0, 0, 0 };

	BASIC_CHECKS(error_event);
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

int gfxop_lookup_view_get_loops(gfx_state_t *state, int nr) {
	int loop = 0, cel = 0;
	gfxr_view_t *view = NULL;

	BASIC_CHECKS(GFX_ERROR);

	view = gfxr_get_view(state->resstate, nr, &loop, &cel, 0);

	if (!view) {
		GFXWARN("Attempt to retrieve number of loops from invalid view %d\n", nr);
		return 0;
	}

	return view->loops_nr;
}

int gfxop_lookup_view_get_cels(gfx_state_t *state, int nr, int loop) {
	int real_loop = loop, cel = 0;
	gfxr_view_t *view = NULL;

	BASIC_CHECKS(GFX_ERROR);

	view = gfxr_get_view(state->resstate, nr, &real_loop, &cel, 0);

	if (!view) {
		GFXWARN("Attempt to retrieve number of cels from invalid/broken view %d\n", nr);
		return 0;
	} else if (real_loop != loop) {
		GFXWARN("Loop number was corrected from %d to %d in view %d\n", loop, real_loop, nr);
	}

	return view->loops[real_loop].cels_nr;
}

int gfxop_check_cel(gfx_state_t *state, int nr, int *loop, int *cel) {
	BASIC_CHECKS(GFX_ERROR);

	if (!gfxr_get_view(state->resstate, nr, loop, cel, 0)) {
		GFXWARN("Attempt to verify loop/cel values for invalid view %d\n", nr);
		return GFX_ERROR;
	}

	return GFX_OK;
}

int gfxop_overflow_cel(gfx_state_t *state, int nr, int *loop, int *cel) {
	int loop_v = *loop;
	int cel_v = *cel;
	BASIC_CHECKS(GFX_ERROR);

	if (!gfxr_get_view(state->resstate, nr, &loop_v, &cel_v, 0)) {
		GFXWARN("Attempt to verify loop/cel values for invalid view %d\n", nr);
		return GFX_ERROR;
	}

	if (loop_v != *loop)
		*loop = 0;

	if (loop_v != *loop
	        || cel_v != *cel)
		*cel = 0;

	return GFX_OK;
}

int gfxop_get_cel_parameters(gfx_state_t *state, int nr, int loop, int cel, int *width, int *height, Common::Point *offset) {
	gfxr_view_t *view = NULL;
	gfx_pixmap_t *pxm = NULL;
	BASIC_CHECKS(GFX_ERROR);

	if (!(view = gfxr_get_view(state->resstate, nr, &loop, &cel, 0))) {
		GFXWARN("Attempt to get cel parameters for invalid view %d\n", nr);
		return GFX_ERROR;
	}

	pxm = view->loops[loop].cels[cel];
	*width = pxm->index_width;
	*height = pxm->index_height;
	offset->x = pxm->xoffset;
	offset->y = pxm->yoffset;

	return GFX_OK;
}

static int _gfxop_draw_cel_buffer(gfx_state_t *state, int nr, int loop, int cel, Common::Point pos, gfx_color_t color, int static_buf, int palette) {
	int priority = (color.mask & GFX_MASK_PRIORITY) ? color.priority : -1;
	int control = (color.mask & GFX_MASK_CONTROL) ? color.control : -1;
	gfxr_view_t *view = NULL;
	gfx_pixmap_t *pxm = NULL;
	int old_x, old_y;
	BASIC_CHECKS(GFX_FATAL);

	if (!(view = gfxr_get_view(state->resstate, nr, &loop, &cel, palette))) {
		GFXWARN("Attempt to draw loop/cel %d/%d in invalid view %d\n", loop, cel, nr);
		return GFX_ERROR;
	}
	pxm = view->loops[loop].cels[cel];

	old_x = pos.x -= pxm->xoffset;
	old_y = pos.y -= pxm->yoffset;

	pos.x *= state->driver->mode->xfact;
	pos.y *= state->driver->mode->yfact;

	if (!static_buf)
		_gfxop_add_dirty(state, gfx_rect(old_x, old_y, pxm->index_width, pxm->index_height));

	return _gfxop_draw_pixmap(state->driver, pxm, priority, control, gfx_rect(0, 0, pxm->width, pxm->height),
	                          gfx_rect(pos.x, pos.y, pxm->width, pxm->height), state->clip_zone, static_buf , state->control_map,
	                          static_buf ? state->static_priority_map : state->priority_map);
}

int gfxop_draw_cel(gfx_state_t *state, int nr, int loop, int cel, Common::Point pos, gfx_color_t color, int palette) {
	return _gfxop_draw_cel_buffer(state, nr, loop, cel, pos, color, 0, palette);
}

int gfxop_draw_cel_static(gfx_state_t *state, int nr, int loop, int cel, Common::Point pos, gfx_color_t color, int palette) {
	int retval;
	rect_t oldclip = state->clip_zone;

	state->clip_zone = gfx_rect_fullscreen;
	_gfxop_scale_rect(&(state->clip_zone), state->driver->mode);
	retval = gfxop_draw_cel_static_clipped(state, nr, loop, cel, pos, color, palette);
	// Except that the area it's clipped against is... unusual ;-)
	state->clip_zone = oldclip;

	return retval;
}

int gfxop_draw_cel_static_clipped(gfx_state_t *state, int nr, int loop, int cel, Common::Point pos, gfx_color_t color, int palette) {
	return _gfxop_draw_cel_buffer(state, nr, loop, cel, pos, color, 1, palette);
}

// Pic operations

static int _gfxop_set_pic(gfx_state_t *state) {
	gfx_copy_pixmap_box_i(state->control_map, state->pic->control_map, gfx_rect(0, 0, 320, 200));
	gfx_copy_pixmap_box_i(state->priority_map, state->pic_unscaled->priority_map, gfx_rect(0, 0, 320, 200));
	gfx_copy_pixmap_box_i(state->static_priority_map, state->pic_unscaled->priority_map, gfx_rect(0, 0, 320, 200));

	_gfxop_install_pixmap(state->driver, state->pic->visual_map);

	if (state->options->pic0_unscaled)
		state->pic->priority_map = gfx_pixmap_scale_index_data(state->pic->priority_map, state->driver->mode);
	return state->driver->set_static_buffer(state->driver, state->pic->visual_map, state->pic->priority_map);
}

int *gfxop_get_pic_metainfo(gfx_state_t *state) {
	return (state->pic) ? state->pic->priorityTable : NULL;
}

int gfxop_new_pic(gfx_state_t *state, int nr, int flags, int default_palette) {
	BASIC_CHECKS(GFX_FATAL);

	// FIXME: the initialization of the GFX resource manager should
	// be pushed up, and it shouldn't occur here
	GfxResManager *_gfx = new GfxResManager(state->resstate);
	_gfx->tagResources();
	state->tag_mode = 1;
	state->palette_nr = default_palette;
	state->pic = _gfx->getPic(nr, GFX_MASK_VISUAL, flags, default_palette, true);
	delete _gfx;

	if (state->driver->mode->xfact == 1 && state->driver->mode->yfact == 1) {
		state->pic_unscaled = state->pic;
	} else {
		// FIXME: the initialization of the GFX resource manager should
		// be pushed up, and it shouldn't occur here
		GfxResManager *_gfx = new GfxResManager(state->resstate);
		state->pic = _gfx->getPic(nr, GFX_MASK_VISUAL, flags, default_palette, false);
		delete _gfx;
	}

	if (!state->pic || !state->pic_unscaled) {
		GFXERROR("Could not retrieve background pic %d!\n", nr);
		if (state->pic) {
			GFXERROR("  -- Inconsistency: scaled pic _was_ retrieved!\n");
		}

		if (state->pic_unscaled) {
			GFXERROR("  -- Inconsistency: unscaled pic _was_ retrieved!\n");
		}

		state->pic = state->pic_unscaled = NULL;
		return GFX_ERROR;
	}

	state->pic_nr = nr;

	return _gfxop_set_pic(state);
}

int gfxop_add_to_pic(gfx_state_t *state, int nr, int flags, int default_palette) {
	BASIC_CHECKS(GFX_FATAL);

	if (!state->pic) {
		GFXERROR("Attempt to add to pic with no pic active!\n");
		return GFX_ERROR;
	}

	if (!(state->pic = gfxr_add_to_pic(state->resstate, state->pic_nr, nr, GFX_MASK_VISUAL, flags, state->palette_nr, default_palette, 1))) {
		GFXERROR("Could not add pic #%d to pic #%d!\n", state->pic_nr, nr);
		return GFX_ERROR;
	}
	state->pic_unscaled = gfxr_add_to_pic(state->resstate, state->pic_nr, nr, GFX_MASK_VISUAL, flags,
	                                      state->palette_nr, default_palette, 1);

	return _gfxop_set_pic(state);
}

// Text operations

// FIXME: only the resstate member of state is used -- inline the reference by:
// replacing gfx_state_t* state parameter with gfx_resstate_t* gfxResourceState and adjust callers accordingly
int gfxop_get_font_height(gfx_state_t *state, int font_nr) {
	gfx_bitmap_font_t *font;
	BASIC_CHECKS(GFX_FATAL);

	// FIXME: the initialization of the GFX resource manager should
	// be pushed up, and it shouldn't occur here
	GfxResManager *_gfx = new GfxResManager(state->resstate);
	font = _gfx->getFont(font_nr);
	delete _gfx;

	if (!font)
		return GFX_ERROR;

	return font->line_height;
}

int gfxop_get_text_params(gfx_state_t *state, int font_nr, const char *text, int maxwidth, int *width, int *height, int text_flags,
						  int *lines_nr, int *lineheight, int *lastline_width) {
	text_fragment_t *textsplits;
	gfx_bitmap_font_t *font;

	BASIC_CHECKS(GFX_FATAL);

	// FIXME: the initialization of the GFX resource manager should
	// be pushed up, and it shouldn't occur here
	GfxResManager *_gfx = new GfxResManager(state->resstate);
	font = _gfx->getFont(font_nr);
	delete _gfx;

	if (!font) {
		GFXERROR("Attempt to calculate text size with invalid font #%d\n", font_nr);
		*width = *height = 0;
		return GFX_ERROR;
	}

	textsplits = gfxr_font_calculate_size(font, maxwidth, text, width, height, lines_nr, lineheight, lastline_width,
	                                      (state->options->workarounds & GFX_WORKAROUND_WHITESPACE_COUNT) | text_flags);

	if (!textsplits) {
		GFXERROR("Could not calculate text size!");
		*width = *height = 0;
		return GFX_ERROR;
	}

	free(textsplits);

	return GFX_OK;
}

gfx_text_handle_t *gfxop_new_text(gfx_state_t *state, int font_nr, char *text, int maxwidth, gfx_alignment_t halign,
								  gfx_alignment_t valign, gfx_color_t color1, gfx_color_t color2, gfx_color_t bg_color, int flags) {
	gfx_text_handle_t *handle;
	gfx_bitmap_font_t *font;
	int i, error = 0;
	BASIC_CHECKS(NULL);

	// mapping text colors to palette
	error |= gfxop_set_color(state, &color1, color1);
	error |= gfxop_set_color(state, &color2, color2);
	error |= gfxop_set_color(state, &bg_color, bg_color);
	if (error) {
		GFXERROR("Unable to set up colors");
		return NULL;
	}

	// FIXME: the initialization of the GFX resource manager should
	// be pushed up, and it shouldn't occur here
	GfxResManager *_gfx = new GfxResManager(state->resstate);
	font = _gfx->getFont(font_nr);
	delete _gfx;

	if (!font) {
		GFXERROR("Attempt to draw text with invalid font #%d\n", font_nr);
		return NULL;
	}

	handle = (gfx_text_handle_t *)sci_malloc(sizeof(gfx_text_handle_t));

	handle->text = (char *)sci_malloc(strlen(text) + 1);
	strcpy(handle->text, text);
	handle->halign = halign;
	handle->valign = valign;
	handle->line_height = font->line_height;

	handle->lines = gfxr_font_calculate_size(font, maxwidth, handle->text, &(handle->width), &(handle->height), &(handle->lines_nr),
	                             NULL, NULL, ((state->options->workarounds & GFX_WORKAROUND_WHITESPACE_COUNT) ?
	                              GFXR_FONT_FLAG_COUNT_WHITESPACE : 0) | flags);

	if (!handle->lines) {
		free(handle->text);
		free(handle);
		GFXERROR("Could not calculate text parameters in font #%d\n", font_nr);
		return NULL;
	}

	if (flags & GFXR_FONT_FLAG_NO_NEWLINES) {
		handle->lines_nr = 1;
		handle->lines->length = strlen(text);
	}

	handle->text_pixmaps = (gfx_pixmap_t **)sci_malloc(sizeof(gfx_pixmap_t *) * handle->lines_nr);

	for (i = 0; i < handle->lines_nr; i++) {
		int chars_nr = handle->lines[i].length;

		handle->text_pixmaps[i] = gfxr_draw_font(font, handle->lines[i].offset, chars_nr,
		                          (color1.mask & GFX_MASK_VISUAL) ? &color1.visual : NULL,
		                          (color2.mask & GFX_MASK_VISUAL) ? &color2.visual : NULL,
		                          (bg_color.mask & GFX_MASK_VISUAL) ? &bg_color.visual : NULL);

		if (!handle->text_pixmaps[i]) {
			int j;

			for (j = 0; j < i; j++)
				gfx_free_pixmap(handle->text_pixmaps[j]);

			free(handle->text_pixmaps);
			free(handle->text);
			free(handle->lines);
			GFXERROR("Failed to draw text pixmap for line %d/%d\n", i, handle->lines_nr);
			free(handle);
			return NULL;
		}
	}

	handle->font = font;

	handle->priority = (color1.mask & GFX_MASK_PRIORITY) ? color1.priority : -1;
	handle->control = (color1.mask & GFX_MASK_CONTROL) ? color1.control : -1;

	return handle;
}

int gfxop_free_text(gfx_state_t *state, gfx_text_handle_t *handle) {
	int j;

	BASIC_CHECKS(GFX_ERROR);

	if (handle->text_pixmaps) {
		for (j = 0; j < handle->lines_nr; j++)
			gfx_free_pixmap(handle->text_pixmaps[j]);
		free(handle->text_pixmaps);
	}

	free(handle->text);
	free(handle->lines);
	free(handle);

	return GFX_OK;
}

int gfxop_draw_text(gfx_state_t *state, gfx_text_handle_t *handle, rect_t zone) {
	int line_height;
	rect_t pos;
	int i;
	BASIC_CHECKS(GFX_FATAL);
	_gfxop_full_pointer_refresh(state);

	if (!handle) {
		GFXERROR("Attempt to draw text with NULL handle!\n");
		return GFX_ERROR;
	}

	if (!handle->lines_nr) {
		GFXDEBUG("Skipping draw_text operation because number of lines is zero\n");
		return GFX_OK;
	}

	_gfxop_scale_rect(&zone, state->driver->mode);

	line_height = handle->line_height * state->driver->mode->yfact;

	pos.y = zone.y;

	switch (handle->valign) {

	case ALIGN_TOP:
		break;

	case ALIGN_CENTER:
		pos.y += (zone.height - (line_height * handle->lines_nr)) >> 1;
		break;

	case ALIGN_BOTTOM:
		pos.y += (zone.height - (line_height * handle->lines_nr));
		break;

	default:
		GFXERROR("Invalid vertical alignment %d!\n", handle->valign);
		return GFX_FATAL; // Internal error...
	}

	for (i = 0; i < handle->lines_nr; i++) {

		gfx_pixmap_t *pxm = handle->text_pixmaps[i];

		if (!pxm->data) {
			gfx_xlate_pixmap(pxm, state->driver->mode, state->options->text_xlate_filter);
			gfxr_endianness_adjust(pxm, state->driver->mode); // FIXME: resmgr layer!
		}
		if (!pxm) {
			GFXERROR("Could not find text pixmap %d/%d\n", i, handle->lines_nr);
			return GFX_ERROR;
		}

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
			GFXERROR("Invalid vertical alignment %d!\n", handle->valign);
			return GFX_FATAL; // Internal error...
		}

		pos.width = pxm->width;
		pos.height = pxm->height;

		_gfxop_add_dirty(state, pos);
		_gfxop_draw_pixmap(state->driver, pxm, handle->priority, handle->control,
		                   gfx_rect(0, 0, pxm->width, pxm->height), pos, state->clip_zone, 0, state->control_map, state->priority_map);

		pos.y += line_height;
	}

	return GFX_OK;
}

gfx_pixmap_t *gfxop_grab_pixmap(gfx_state_t *state, rect_t area) {
	gfx_pixmap_t *pixmap = NULL;
	rect_t resultzone; // Ignored for this application
	BASIC_CHECKS(NULL);
	_gfxop_full_pointer_refresh(state);

	_gfxop_scale_rect(&area, state->driver->mode);
	if (_gfxop_grab_pixmap(state, &pixmap, area.x, area.y, area.width, area.height, 0, &resultzone))
		return NULL; // area CUT the visual screen had a null or negative size

	return pixmap;
}

int gfxop_draw_pixmap(gfx_state_t *state, gfx_pixmap_t *pxm, rect_t zone, Common::Point pos) {
	rect_t target;
	BASIC_CHECKS(GFX_ERROR);

	if (!pxm) {
		GFXERROR("Attempt to draw NULL pixmap!\n");
		return GFX_ERROR;
	}

	_gfxop_full_pointer_refresh(state);

	target = gfx_rect(pos.x, pos.y, zone.width, zone.height);

	_gfxop_add_dirty(state, target);

	if (!pxm) {
		GFXERROR("Attempt to draw_pixmap with pxm=NULL\n");
		return GFX_ERROR;
	}

	_gfxop_scale_rect(&zone, state->driver->mode);
	_gfxop_scale_rect(&target, state->driver->mode);

	return _gfxop_draw_pixmap(state->driver, pxm, -1, -1, zone, target, gfx_rect(0, 0, 320*state->driver->mode->xfact,
	                                   200*state->driver->mode->yfact), 0, NULL, NULL);
}

int gfxop_free_pixmap(gfx_state_t *state, gfx_pixmap_t *pxm) {
	BASIC_CHECKS(GFX_ERROR);
	gfx_free_pixmap(pxm);
	return GFX_OK;
}

} // End of namespace Sci
