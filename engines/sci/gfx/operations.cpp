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

/* Graphical operations, called from the widget state manager */

#include "sci/include/sci_memory.h"
#include "sci/include/gfx_operations.h"

#include <ctype.h>

#define PRECISE_PRIORITY_MAP /* Duplicate all operations on the local priority map as appropriate */

#undef GFXW_DEBUG_DIRTY /* Enable to debug stuff relevant for dirty rects
** in widget management  */

#ifdef GFXW_DEBUG_DIRTY
#  define DDIRTY fprintf(stderr, "%s:%5d| ", __FILE__, __LINE__); fprintf
#else
#  define DDIRTY if (0) fprintf
#endif

/* Default color maps */
#define DEFAULT_COLORS_NR 16
gfx_pixmap_color_t default_colors[DEFAULT_COLORS_NR] = {{GFX_COLOR_SYSTEM, 0x00, 0x00, 0x00}, {GFX_COLOR_SYSTEM, 0x00, 0x00, 0xaa},
	{GFX_COLOR_SYSTEM, 0x00, 0xaa, 0x00}, {GFX_COLOR_SYSTEM, 0x00, 0xaa, 0xaa},
	{GFX_COLOR_SYSTEM, 0xaa, 0x00, 0x00}, {GFX_COLOR_SYSTEM, 0xaa, 0x00, 0xaa},
	{GFX_COLOR_SYSTEM, 0xaa, 0x55, 0x00}, {GFX_COLOR_SYSTEM, 0xaa, 0xaa, 0xaa},
	{GFX_COLOR_SYSTEM, 0x55, 0x55, 0x55}, {GFX_COLOR_SYSTEM, 0x55, 0x55, 0xff},
	{GFX_COLOR_SYSTEM, 0x55, 0xff, 0x55}, {GFX_COLOR_SYSTEM, 0x55, 0xff, 0xff},
	{GFX_COLOR_SYSTEM, 0xff, 0x55, 0x55}, {GFX_COLOR_SYSTEM, 0xff, 0x55, 0xff},
	{GFX_COLOR_SYSTEM, 0xff, 0xff, 0x55}, {GFX_COLOR_SYSTEM, 0xff, 0xff, 0xff}
}; /* "Normal" EGA */


#define POINTER_VISIBLE_BUT_CLIPPED 2

/* Performs basic checks that apply to most functions */
#define BASIC_CHECKS(error_retval) \
if (!state) { \
	GFXERROR("Null state!\n"); \
	return error_retval; \
} \
if (!state->driver) { \
	GFXERROR("GFX driver invalid!\n"); \
	return error_retval; \
}

/* How to determine whether colors have to be allocated */
#define PALETTE_MODE state->driver->mode->palette

#define DRAW_POINTER { int __x = _gfxop_draw_pointer(state); if (__x) { GFXERROR("Drawing the mouse pointer failed!\n"); return __x;} }
#define REMOVE_POINTER { int __x = _gfxop_remove_pointer(state); if (__x) { GFXERROR("Removing the mouse pointer failed!\n"); return __x;} }

/* #define GFXOP_DEBUG_DIRTY */

/* Internal operations */

static void
_gfxop_scale_rect(rect_t *rect, gfx_mode_t *mode) {
	int xfact = mode->xfact;
	int yfact = mode->yfact;

	rect->x *= xfact;
	rect->y *= yfact;
	rect->xl *= xfact;
	rect->yl *= yfact;
}

static void
_gfxop_scale_point(Common::Point *point, gfx_mode_t *mode) {
	int xfact = mode->xfact;
	int yfact = mode->yfact;

	point->x *= xfact;
	point->y *= yfact;
}

static void
_gfxop_alloc_colors(gfx_state_t *state, gfx_pixmap_color_t *colors, int colors_nr) {
	int i;

	if (!PALETTE_MODE)
		return;

	for (i = 0; i < colors_nr; i++)
		gfx_alloc_color(state->driver->mode->palette, colors + i);
}

#if 0
// Unreferenced - removed
static void
_gfxop_free_colors(gfx_state_t *state, gfx_pixmap_color_t *colors, int colors_nr) {
	int i;

	if (!PALETTE_MODE)
		return;

	for (i = 0; i < colors_nr; i++)
		gfx_free_color(state->driver->mode->palette, colors + i);
}
#endif

int _gfxop_clip(rect_t *rect, rect_t clipzone)
/* Returns 1 if nothing is left */
{
#if 0
	printf("Clipping (%d, %d) size (%d, %d)  by (%d,%d)(%d,%d)\n", rect->x, rect->y, rect->xl, rect->yl,
	       clipzone.x, clipzone.y, clipzone.xl, clipzone.yl);
#endif

	if (rect->x < clipzone.x) {
		rect->xl -= (clipzone.x - rect->x);
		rect->x = clipzone.x;
	}

	if (rect->y < clipzone.y) {
		rect->yl -= (clipzone.y - rect->y);
		rect->y = clipzone.y;
	}

	if (rect->x + rect->xl > clipzone.x + clipzone.xl)
		rect->xl = (clipzone.x + clipzone.xl) - rect->x;

	if (rect->y + rect->yl > clipzone.y + clipzone.yl)
		rect->yl = (clipzone.y + clipzone.yl) - rect->y;

	if (rect->xl < 0)
		rect->xl = 0;
	if (rect->yl < 0)
		rect->yl = 0;

#if 0
	printf(" => (%d, %d) size (%d, %d)\n", rect->x, rect->y, rect->xl, rect->yl);
#endif
	return (rect->xl <= 0 || rect->yl <= 0);
}

static int
_gfxop_grab_pixmap(gfx_state_t *state, gfx_pixmap_t **pxmp, int x, int y,
                   int xl, int yl, int priority, rect_t *zone)
/* Returns 1 if the resulting data size was zero, GFX_OK or an error code otherwise */
{
	int xfact = state->driver->mode->xfact;
	int yfact = state->driver->mode->yfact;
	int unscaled_xl = (xl + xfact - 1) / xfact;
	int unscaled_yl = (yl + yfact - 1) / yfact;
	*zone = gfx_rect(x, y, xl, yl);

	if (_gfxop_clip(zone, gfx_rect(0, 0,
	                               320 * state->driver->mode->xfact,
	                               200 * state->driver->mode->yfact)))
		return GFX_ERROR;

	if (!*pxmp)
		*pxmp = gfx_new_pixmap(unscaled_xl, unscaled_yl, GFX_RESID_NONE, 0, 0);
	else
		if (xl * yl > (*pxmp)->xl * (*pxmp)->yl) {
			gfx_pixmap_free_data(*pxmp);
			(*pxmp)->data = NULL;
		}

	if (!(*pxmp)->data) {
		(*pxmp)->index_xl = unscaled_xl + 1;
		(*pxmp)->index_yl = unscaled_yl + 1;
		gfx_pixmap_alloc_data(*pxmp, state->driver->mode);
	}
	return state->driver->grab_pixmap(state->driver, *zone, *pxmp,
	                                  priority ? GFX_MASK_PRIORITY : GFX_MASK_VISUAL);
}


#define DRAW_LOOP(condition)										\
{													\
	rect_t drawrect = gfx_rect(pos.x, pos.y, pxm->index_xl, pxm->index_yl);				\
	int offset, base_offset;									\
	int read_offset, base_read_offset;								\
	int x,y;											\
													\
	if (!pxm->index_data) {										\
		GFXERROR("Attempt to draw control color %d on pixmap %d/%d/%d without index data!\n",	\
			 color, pxm->ID, pxm->loop, pxm->cel);						\
		return;											\
	}												\
													\
	if (_gfxop_clip(&drawrect, gfx_rect(0, 0, 320, 200)))						\
		return;											\
													\
	offset = base_offset = drawrect.x + drawrect.y * 320;						\
	read_offset = base_read_offset = (drawrect.x - pos.x) + ((drawrect.y - pos.y) * pxm->index_xl);	\
													\
	for (y = 0; y < drawrect.yl; y++) {								\
		for (x = 0; x < drawrect.xl; x++)							\
			if (pxm->index_data[read_offset++] != pxm->color_key) {				\
				if (condition)								\
					map->index_data[offset++] = color;				\
				else									\
					++offset;							\
			} else ++offset;								\
													\
		offset = base_offset += 320;								\
		read_offset = base_read_offset += pxm->index_xl;					\
	}												\
}

static void
_gfxop_draw_control(gfx_pixmap_t *map, gfx_pixmap_t *pxm, int color, Common::Point pos)
DRAW_LOOP(1) /* Always draw */

#ifdef PRECISE_PRIORITY_MAP
static void
_gfxop_draw_priority(gfx_pixmap_t *map, gfx_pixmap_t *pxm, int color, Common::Point pos)
DRAW_LOOP(map->index_data[offset] < color) /* Draw only lower priority */
#endif

#undef DRAW_LOOP

static int
_gfxop_install_pixmap(gfx_driver_t *driver, gfx_pixmap_t *pxm) {
	int error;

	if (driver->mode->palette &&
	        (!(pxm->flags & GFX_PIXMAP_FLAG_PALETTE_SET))) {
		int i;

		for (i = 0; i < pxm->colors_nr; i++) {
			if ((error = driver->set_palette(driver, pxm->colors[i].global_index,
			                                 pxm->colors[i].r,
			                                 pxm->colors[i].g,
			                                 pxm->colors[i].b))) {

				GFXWARN("driver->set_palette(%d, %02x/%02x/%02x) failed!\n",
				        pxm->colors[i].global_index,
				        pxm->colors[i].r,
				        pxm->colors[i].g,
				        pxm->colors[i].b);

				if (error == GFX_FATAL)
					return GFX_FATAL;
			}
		}

		pxm->flags |= GFX_PIXMAP_FLAG_PALETTE_SET;
	}
	return GFX_OK;
}

static int
_gfxop_draw_pixmap(gfx_driver_t *driver, gfx_pixmap_t *pxm, int priority, int control,
                   rect_t src, rect_t dest, rect_t clip, int static_buf, gfx_pixmap_t *control_map,
                   gfx_pixmap_t *priority_map) {
	int error;
	rect_t clipped_dest = gfx_rect(dest.x, dest.y, dest.xl, dest.yl);

	if (control >= 0 || priority >= 0) {
		Common::Point original_pos = Common::Point(dest.x / driver->mode->xfact,
		                                 dest.y / driver->mode->yfact);

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
	src.xl = clipped_dest.xl;
	src.yl = clipped_dest.yl;

	error = _gfxop_install_pixmap(driver, pxm);
	if (error) return error;

	DDIRTY(stderr, "\\-> Drawing to actual %d %d %d %d\n",
	       clipped_dest.x / driver->mode->xfact,
	       clipped_dest.y / driver->mode->yfact,
	       clipped_dest.xl / driver->mode->xfact,
	       clipped_dest.yl / driver->mode->yfact);

	error = driver->draw_pixmap(driver, pxm, priority, src, clipped_dest,
	                            static_buf ? GFX_BUFFER_STATIC : GFX_BUFFER_BACK);

	if (error) {
		GFXERROR("driver->draw_pixmap() returned error!\n");
		return error;
	}
	return GFX_OK;
}

static int
_gfxop_remove_pointer(gfx_state_t *state) {
	if (state->mouse_pointer_visible
	        && !state->mouse_pointer_in_hw
	        && state->mouse_pointer_bg) {
		int retval;

		if (state->mouse_pointer_visible == POINTER_VISIBLE_BUT_CLIPPED) {
			state->mouse_pointer_visible = 0;
			state->pointer_pos.x = state->driver->pointer_x / state->driver->mode->xfact;
			state->pointer_pos.y = state->driver->pointer_y / state->driver->mode->yfact;
			return GFX_OK;
		}

		state->mouse_pointer_visible = 0;

		retval = state->driver->draw_pixmap(state->driver, state->mouse_pointer_bg, GFX_NO_PRIORITY,
		                                    gfx_rect(0, 0, state->mouse_pointer_bg->xl, state->mouse_pointer_bg->yl),
		                                    state->pointer_bg_zone,
		                                    GFX_BUFFER_BACK);

		state->pointer_pos.x = state->driver->pointer_x / state->driver->mode->xfact;
		state->pointer_pos.y = state->driver->pointer_y / state->driver->mode->yfact;

		return retval;

	} else {
		state->pointer_pos.x = state->driver->pointer_x / state->driver->mode->xfact;
		state->pointer_pos.y = state->driver->pointer_y / state->driver->mode->yfact;
		return GFX_OK;
	}
}

static int /* returns 1 if there are no pointer bounds, 0 otherwise */
_gfxop_get_pointer_bounds(gfx_state_t *state, rect_t *rect) {
	gfx_pixmap_t *ppxm = state->mouse_pointer;

	if (!ppxm)
		return 1;

	rect->x = state->driver->pointer_x - ppxm->xoffset * (state->driver->mode->xfact);
	rect->y = state->driver->pointer_y - ppxm->yoffset * (state->driver->mode->yfact);
	rect->xl = ppxm->xl;
	rect->yl = ppxm->yl;

	return (_gfxop_clip(rect, gfx_rect(0, 0, 320 * state->driver->mode->xfact,
	                                   200 * state->driver->mode->yfact)));
}

static int
_gfxop_buffer_propagate_box(gfx_state_t *state, rect_t box, gfx_buffer_t buffer);

static int
_gfxop_draw_pointer(gfx_state_t *state) {
	if (state->mouse_pointer_visible || !state->mouse_pointer || state->mouse_pointer_in_hw)
		return GFX_OK;
	else {
		int retval;
		gfx_pixmap_t *ppxm = state->mouse_pointer;
		int xfact, yfact;
		int x = state->driver->pointer_x - ppxm->xoffset * (xfact = state->driver->mode->xfact);
		int y = state->driver->pointer_y - ppxm->yoffset * (yfact = state->driver->mode->yfact);
		int error;

		state->mouse_pointer_visible = 1;

		state->old_pointer_draw_pos.x = x;
		state->old_pointer_draw_pos.y = y;

		/* FIXME: we are leaking the mouse_pointer_bg, but freeing it causes weirdness in jones
		 * we should reuse the buffer instead of malloc/free for better performance */

		retval = _gfxop_grab_pixmap(state, &(state->mouse_pointer_bg), x, y,
		                            ppxm->xl, ppxm->yl, 0,
		                            &(state->pointer_bg_zone));

		if (retval == GFX_ERROR) {
			state->pointer_bg_zone = gfx_rect(0, 0, 320, 200);
			state->mouse_pointer_visible = POINTER_VISIBLE_BUT_CLIPPED;
			return GFX_OK;
		}

		if (retval)
			return retval;

		error = _gfxop_draw_pixmap(state->driver, ppxm, -1, -1,
		                           gfx_rect(0, 0, ppxm->xl, ppxm->yl),
		                           gfx_rect(x, y, ppxm->xl, ppxm->yl),
		                           gfx_rect(0, 0, xfact * 320 , yfact * 200),
		                           0, state->control_map, state->priority_map);

		if (error)
			return error;


		return GFX_OK;
	}
}

gfx_pixmap_t *
_gfxr_get_cel(gfx_state_t *state, int nr, int *loop, int *cel, int palette) {
	gfxr_view_t *view = gfxr_get_view(state->resstate, nr, loop, cel, palette);
	gfxr_loop_t *indexed_loop;

	if (!view)
		return NULL;

	if (*loop >= view->loops_nr
	        || *loop < 0) {
		GFXWARN("Attempt to get cel from loop %d/%d inside view %d\n", *loop,
		        view->loops_nr, nr);
		return NULL;
	}
	indexed_loop = view->loops + *loop;

	if (*cel >= indexed_loop->cels_nr
	        || *cel < 0) {
		GFXWARN("Attempt to get cel %d/%d from view %d/%d\n", *cel, indexed_loop->cels_nr,
		        nr, *loop);
		return NULL;
	}

	return indexed_loop->cels[*cel]; /* Yes, view->cels uses a malloced pointer list. */
}

/*** Dirty rectangle operations ***/

static inline int
_gfxop_update_box(gfx_state_t *state, rect_t box) {
	int retval;
	_gfxop_scale_rect(&box, state->driver->mode);

	if ((retval = _gfxop_buffer_propagate_box(state, box, GFX_BUFFER_FRONT))) {
		GFXERROR("Error occured while propagating box (%d,%d,%d,%d) to front buffer\n",
		         box.x, box.y, box.xl, box.yl);
		return retval;
	}
	return GFX_OK;
}


static struct _dirty_rect *
			_rect_create(rect_t box) {
	struct _dirty_rect *rect;

	rect = (struct _dirty_rect*)sci_malloc(sizeof(struct _dirty_rect));
	rect->next = NULL;
	rect->rect = box;

	return rect;
}


gfx_dirty_rect_t *
gfxdr_add_dirty(gfx_dirty_rect_t *base, rect_t box, int strategy) {
	if (box.xl < 0) {
		box.x += box.xl;
		box.xl = - box.xl;
	}

	if (box.yl < 0) {
		box.y += box.yl;
		box.yl = - box.yl;
	}
#ifdef GFXOP_DEBUG_DIRTY
	fprintf(stderr, "Adding new dirty (%d %d %d %d)\n",
	        GFX_PRINT_RECT(box));
#endif
	if (_gfxop_clip(&box, gfx_rect(0, 0, 320, 200)))
		return base;

	switch (strategy) {

	case GFXOP_DIRTY_FRAMES_ONE:
		if (base)
			base->rect = gfx_rects_merge(box, base->rect);
		else
			base = _rect_create(box);
		break;

	case GFXOP_DIRTY_FRAMES_CLUSTERS: {
		struct _dirty_rect **rectp = &(base);

		while (*rectp) {
			if (gfx_rects_overlap((*rectp)->rect, box)) {
				struct _dirty_rect *next = (*rectp)->next;
				box = gfx_rects_merge((*rectp)->rect, box);
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

static void
_gfxop_add_dirty(gfx_state_t *state, rect_t box) {
	if (state->disable_dirty)
		return;

	state->dirty_rects = gfxdr_add_dirty(state->dirty_rects, box, state->options->dirty_frames);
}

static inline void
_gfxop_add_dirty_x(gfx_state_t *state, rect_t box)
/* Extends the box size by one before adding (used for lines) */
{
	if (box.xl < 0)
		box.xl--;
	else
		box.xl++;

	if (box.yl < 0)
		box.yl--;
	else
		box.yl++;

	_gfxop_add_dirty(state, box);
}

static int
_gfxop_clear_dirty_rec(gfx_state_t *state, struct _dirty_rect *rect) {
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


/*** Exported operations ***/

static void
init_aux_pixmap(gfx_pixmap_t **pixmap) {
	*pixmap = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320, 200, GFX_RESID_NONE, 0, 0));
	(*pixmap)->flags |= GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
	(*pixmap)->colors_nr = DEFAULT_COLORS_NR;
	(*pixmap)->colors = default_colors;
}

static int
_gfxop_init_common(gfx_state_t *state, gfx_options_t *options, void *misc_payload) {
	state->options = options;

	if (!((state->resstate = gfxr_new_resource_manager(state->version,
	                         state->options,
	                         state->driver,
	                         misc_payload)))) {
		GFXERROR("Failed to initialize resource manager!\n");
		return GFX_FATAL;
	}

	if ((state->static_palette =
	            gfxr_interpreter_get_static_palette(state->resstate,
	                                                state->version,
	                                                &(state->static_palette_entries),
	                                                misc_payload)))
		_gfxop_alloc_colors(state, state->static_palette, state->static_palette_entries);

	state->visible_map = GFX_MASK_VISUAL;
	state->fullscreen_override = NULL; /* No magical override */
	gfxop_set_clip_zone(state, gfx_rect(0, 0, 320, 200));

	state->mouse_pointer = state->mouse_pointer_bg = NULL;
	state->mouse_pointer_visible = 0;

	init_aux_pixmap(&(state->control_map));
	init_aux_pixmap(&(state->priority_map));
	init_aux_pixmap(&(state->static_priority_map));

	state->options = options;
	state->mouse_pointer_in_hw = 0;
	state->disable_dirty = 0;
	state->events = NULL;

	state->pic = state->pic_unscaled = NULL;

	state->pic_nr = -1; /* Set background pic number to an invalid value */

	state->tag_mode = 0;

	state->dirty_rects = NULL;

	state->old_pointer_draw_pos.x = -1;
	state->old_pointer_draw_pos.y = -1;

	return GFX_OK;
}

int
gfxop_init_default(gfx_state_t *state, gfx_options_t *options, void *misc_info) {
	BASIC_CHECKS(GFX_FATAL);
	if (state->driver->init(state->driver))
		return GFX_FATAL;

	return _gfxop_init_common(state, options, misc_info);
}


int
gfxop_init(gfx_state_t *state, int xfact, int yfact, gfx_color_mode_t bpp,
           gfx_options_t *options, void *misc_info) {
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

	return _gfxop_init_common(state, options, misc_info);
}


int
gfxop_set_parameter(gfx_state_t *state, char *attribute, char *value) {
	BASIC_CHECKS(GFX_FATAL);

	return state->driver->set_parameter(state->driver, attribute, value);
}


int
gfxop_exit(gfx_state_t *state) {
	BASIC_CHECKS(GFX_ERROR);
	gfxr_free_resource_manager(state->driver, state->resstate);

	if (state->control_map) {
		gfx_free_pixmap(state->driver, state->control_map);
		state->control_map = NULL;
	}

	if (state->priority_map) {
		gfx_free_pixmap(state->driver, state->priority_map);
		state->priority_map = NULL;
	}

	if (state->static_priority_map) {
		gfx_free_pixmap(state->driver, state->static_priority_map);
		state->static_priority_map = NULL;
	}

	if (state->mouse_pointer_bg) {
		gfx_free_pixmap(state->driver, state->mouse_pointer_bg);
		state->mouse_pointer_bg = NULL;
	}

	state->driver->exit(state->driver);
	return GFX_OK;
}


static int
_gfxop_scan_one_bitmask(gfx_pixmap_t *pixmap, rect_t zone) {
	int retval = 0;
	int pixmap_xscale = pixmap->index_xl / 320;
	int pixmap_yscale = pixmap->index_yl / 200;
	int line_width = pixmap_yscale * pixmap->index_xl;
	int startindex = (line_width * zone.y) + (zone.x * pixmap_xscale);

	startindex += pixmap_xscale >> 1; /* Center on X */
	startindex += (pixmap_yscale >> 1) * pixmap->index_xl; /* Center on Y */

	if (_gfxop_clip(&zone, gfx_rect(0, 0, pixmap->index_xl, pixmap->index_yl)))
		return 0;

	while (zone.yl--) {
		int i;
		for (i = 0; i < (zone.xl * pixmap_xscale); i += pixmap_xscale)
			retval |= (1 << ((pixmap->index_data[startindex + i]) & 0xf));

		startindex += line_width;
	}

	return retval;
}

int
gfxop_scan_bitmask(gfx_state_t *state, rect_t area, gfx_map_mask_t map) {
	gfxr_pic_t *pic = (state->pic_unscaled) ? state->pic_unscaled : state->pic;
	int retval = 0;

	_gfxop_clip(&area, gfx_rect(0, 10, 320, 200));

	if (area.xl <= 0
	        || area.yl <= 0)
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

int
gfxop_set_clip_zone(gfx_state_t *state, rect_t zone) {
	int xfact, yfact;
	BASIC_CHECKS(GFX_ERROR);

	DDIRTY(stderr, "-- Setting clip zone %d %d %d %d\n", GFX_PRINT_RECT(zone));

	xfact = state->driver->mode->xfact;
	yfact = state->driver->mode->yfact;

	if (zone.x < MIN_X) {
		zone.xl -= (zone.x - MIN_X);
		zone.x = MIN_X;
	}

	if (zone.y < MIN_Y) {
		zone.yl -= (zone.y - MIN_Y);
		zone.y = MIN_Y;
	}

	if (zone.x + zone.xl > MAX_X)
		zone.xl = MAX_X + 1 - zone.x;

	if (zone.y + zone.yl > MAX_Y)
		zone.yl = MAX_Y + 1 - zone.y;

	memcpy(&(state->clip_zone_unscaled), &zone, sizeof(rect_t));

	state->clip_zone.x = state->clip_zone_unscaled.x * xfact;
	state->clip_zone.y = state->clip_zone_unscaled.y * yfact;
	state->clip_zone.xl = state->clip_zone_unscaled.xl * xfact;
	state->clip_zone.yl = state->clip_zone_unscaled.yl * yfact;

	return GFX_OK;
}

int
gfxop_set_color(gfx_state_t *state, gfx_color_t *color, int r, int g, int b, int a,
                int priority, int control) {
	gfx_pixmap_color_t pixmap_color = {0, 0, 0, 0};
	int error_code;
	int mask = ((r >= 0 && g >= 0 && b >= 0) ? GFX_MASK_VISUAL : 0)
	           | ((priority >= 0) ? GFX_MASK_PRIORITY : 0)
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
			pixmap_color.r = r;
			pixmap_color.g = g;
			pixmap_color.b = b;
			pixmap_color.global_index = GFX_COLOR_INDEX_UNMAPPED;
			if ((error_code = gfx_alloc_color(state->driver->mode->palette, &pixmap_color))) {
				if (error_code < 0) {
					GFXWARN("Could not get color entry for %02x/%02x/%02x\n", r, g, b);
					return error_code;
				} else if ((error_code = state->driver->set_palette(state->driver, pixmap_color.global_index, (byte) r, (byte) g, (byte) b))) {
					GFXWARN("Graphics driver failed to set color index %d to (%02x/%02x/%02x)\n",
					        pixmap_color.global_index, r, g, b);
					return error_code;
				}
			}
			color->visual.global_index = pixmap_color.global_index;
		}
	}
	return GFX_OK;
}

int
gfxop_set_system_color(gfx_state_t *state, gfx_color_t *color) {
	gfx_palette_color_t *palette_colors;
	BASIC_CHECKS(GFX_FATAL);

	if (!PALETTE_MODE)
		return GFX_OK;

	if (color->visual.global_index < 0
	        || color->visual.global_index >= state->driver->mode->palette->max_colors_nr) {
		GFXERROR("Attempt to set invalid color index %02x as system color\n", color->visual.global_index);
		return GFX_ERROR;
	}

	palette_colors = state->driver->mode->palette->colors;
	palette_colors[color->visual.global_index].lockers = GFX_COLOR_SYSTEM;

	return GFX_OK;
}

int
gfxop_free_color(gfx_state_t *state, gfx_color_t *color) {
	gfx_palette_color_t *palette_color	= 0;
	gfx_pixmap_color_t pixmap_color		= {0, 0, 0, 0};
	int error_code;
	BASIC_CHECKS(GFX_FATAL);

	if (!PALETTE_MODE)
		return GFX_OK;

	if (color->visual.global_index < 0
	        || color->visual.global_index >= state->driver->mode->palette->max_colors_nr) {
		GFXERROR("Attempt to free invalid color index %02x\n", color->visual.global_index);
		return GFX_ERROR;
	}

	pixmap_color.global_index = color->visual.global_index;
	palette_color = state->driver->mode->palette->colors + pixmap_color.global_index;
	pixmap_color.r = palette_color->r;
	pixmap_color.g = palette_color->g;
	pixmap_color.b = palette_color->b;

	if ((error_code = gfx_free_color(state->driver->mode->palette, &pixmap_color))) {
		GFXWARN("Failed to free color with color index %02x\n", color->visual.global_index);
		return error_code;
	}

	return GFX_OK;
}

/******************************/
/* Generic drawing operations */
/******************************/


static int
line_check_bar(int *start, int *length, int clipstart, int cliplength) {
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

static void
clip_line_partial(float *start, float *end, float delta_val, float pos_val, float start_val, float end_val) {
	float my_start = (start_val - pos_val) * delta_val;
	float my_end = (end_val - pos_val) * delta_val;

	if (my_end < *end)
		*end = my_end;
	if (my_start > *start)
		*start = my_start;
}

static int
line_clip(rect_t *line, rect_t clip, int xfact, int yfact)
/* returns 1 if nothing is left, or 0 if part of the line is in the clip window */
{
	/* Compensate for line thickness (should match precisely) */
	clip.xl -= xfact;
	clip.yl -= yfact;

	if (!line->xl) {/* vbar */
		if (line->x < clip.x || line->x >= (clip.x + clip.xl))
			return 1;

		return line_check_bar(&(line->y), &(line->yl), clip.y, clip.yl);

	} else

		if (!line->yl) {/* hbar */
			if (line->y < clip.y || line->y >= (clip.y + clip.yl))
				return 1;

			return line_check_bar(&(line->x), &(line->xl), clip.x, clip.xl);

		} else { /* "normal" line */
			float start = 0.0, end = 1.0;
			float xv = (float) line->xl;
			float yv = (float) line->yl;

			if (line->xl < 0)
				clip_line_partial(&start, &end, (float)(1.0 / xv), (float) line->x, (float)(clip.x + clip.xl), (float) clip.x);
			else
				clip_line_partial(&start, &end, (float)(1.0 / xv), (float) line->x, (float) clip.x, (float)(clip.x + clip.xl));

			if (line->yl < 0)
				clip_line_partial(&start, &end, (float)(1.0 / yv), (float) line->y, (float)(clip.y + clip.yl), (float) clip.y);
			else
				clip_line_partial(&start, &end, (float)(1.0 / yv), (float) line->y, (float) clip.y, (float)(clip.y + clip.yl));

			line->x += (int)(xv * start);
			line->y += (int)(yv * start);

			line->xl = (int)(xv * (end - start));
			line->yl = (int)(yv * (end - start));

			return (start > 1.0 || end < 0.0);
		}
	return 0;
}

static int
point_clip(Common::Point *start, Common::Point *end, rect_t clip, int xfact, int yfact) {
	rect_t line = gfx_rect(start->x, start->y, end->x - start->x, end->y - start->y);
	int retval = line_clip(&line, clip, xfact, yfact);

	start->x = line.x;
	start->y = line.y;

	end->x = line.x + line.xl;
	end->y = line.y + line.yl;

	return retval;
}



static void
draw_line_to_control_map(gfx_state_t *state, Common::Point start, Common::Point end, gfx_color_t color) {
	if (color.mask & GFX_MASK_CONTROL)
		if (!point_clip(&start, &end, state->clip_zone_unscaled, 0, 0))
			gfx_draw_line_pixmap_i(state->control_map, start, end, color.control);
}

static int
simulate_stippled_line_draw(gfx_driver_t *driver, int skipone, Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode)
/* Draws a stippled line if this isn't supported by the driver (skipone is ignored ATM) */
{
	int xl = end.x - start.x;
	int yl = end.y - start.y;
	int stepwidth = (xl) ? driver->mode->xfact : driver->mode->yfact;
	int dbl_stepwidth = 2 * stepwidth;
	int linelength = (line_mode == GFX_LINE_MODE_FINE) ? stepwidth - 1 : 0;
	int *posvar;
	int length;
	int delta;
	int length_left;

	if (!xl) { /* xl = 0, so we move along yl */
		posvar = (int *) &start.y;
		length = yl;
		delta = (yl < 0) ? -dbl_stepwidth : dbl_stepwidth;
	} else {
		assert(!yl);  /* We don't do diagonals; that's not needed ATM. */
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

		if ((retval = driver->draw_line(driver, start, nextpos,
		                                color, line_mode, GFX_LINE_STYLE_NORMAL))) {
			GFXERROR("Failed to draw partial stippled line (%d,%d) -- (%d,%d)\n",
			         start.x, start.y, nextpos.x, nextpos.y);
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
			GFXERROR("Failed to draw partial stippled line (%d,%d) -- (%d,%d)\n",
			         start.x, start.y, nextpos.x, nextpos.y);
			return retval;
		}
	}

	return GFX_OK;
}


static int
_gfxop_draw_line_clipped(gfx_state_t *state, Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode,
                         gfx_line_style_t line_style) {
	int retval;
	int skipone = (start.x ^ end.y) & 1; /* Used for simulated line stippling */

	BASIC_CHECKS(GFX_FATAL);
	REMOVE_POINTER;

	/* First, make sure that the line is normalized */
	if (start.y > end.y) {
		Common::Point swap = start;
		start = end;
		end = swap;
	}

	if (start.x < state->clip_zone.x
	        || start.y < state->clip_zone.y
	        || end.x >= (state->clip_zone.x + state->clip_zone.xl)
	        || end.y >= (state->clip_zone.y + state->clip_zone.yl))
		if (point_clip(&start, &end, state->clip_zone, state->driver->mode->xfact - 1,
		               state->driver->mode->yfact - 1))
			return GFX_OK; /* Clipped off */

	if (line_style == GFX_LINE_STYLE_STIPPLED) {
		if (start.x != end.x && start.y != end.y) {
			GFXWARN("Attempt to draw stippled line which is neither an hbar nor a vbar: (%d,%d) -- (%d,%d)\n",
			        start.x, start.y, end.x, end.y);
			return GFX_ERROR;
		}
		if (!(state->driver->capabilities & GFX_CAPABILITY_STIPPLED_LINES))
			return simulate_stippled_line_draw(state->driver, skipone, start, end, color, line_mode);
	}

	if ((retval = state->driver->draw_line(state->driver, start, end, color, line_mode, line_style))) {
		GFXERROR("Failed to draw line (%d,%d) -- (%d,%d)\n",
		         start.x, start.y, end.x, end.y);
		return retval;
	}
	return GFX_OK;
}

int
gfxop_draw_line(gfx_state_t *state, Common::Point start, Common::Point end,
                gfx_color_t color, gfx_line_mode_t line_mode,
                gfx_line_style_t line_style) {
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

	return _gfxop_draw_line_clipped(state, start, end, color, line_mode, line_style);
}

int
gfxop_draw_rectangle(gfx_state_t *state, rect_t rect, gfx_color_t color, gfx_line_mode_t line_mode,
                     gfx_line_style_t line_style) {
	int retval = 0;
	int xfact, yfact;
	int xunit, yunit;
	int x, y, xl, yl;
	Common::Point upper_left_u, upper_right_u, lower_left_u, lower_right_u;
	Common::Point upper_left, upper_right, lower_left, lower_right;

	BASIC_CHECKS(GFX_FATAL);
	REMOVE_POINTER;

	xfact = state->driver->mode->xfact;
	yfact = state->driver->mode->yfact;

	if (line_mode == GFX_LINE_MODE_FINE) {
		xunit = yunit = 1;
		xl = 1 + (rect.xl - 1) * xfact;
		yl = 1 + (rect.yl - 1) * yfact;
		x = rect.x * xfact + (xfact - 1);
		y = rect.y * yfact + (yfact - 1);
	} else {
		xunit = xfact;
		yunit = yfact;
		xl = rect.xl * xfact;
		yl = rect.yl * yfact;
		x = rect.x * xfact;
		y = rect.y * yfact;
	}

	upper_left_u = Common::Point(rect.x, rect.y);
	upper_right_u = Common::Point(rect.x + rect.xl, rect.y);
	lower_left_u = Common::Point(rect.x, rect.y + rect.yl);
	lower_right_u = Common::Point(rect.x + rect.xl, rect.y + rect.yl);

	upper_left = Common::Point(x, y);
	upper_right = Common::Point(x + xl, y);
	lower_left = Common::Point(x, y + yl);
	lower_right = Common::Point(x + xl, y + yl);

#define PARTIAL_LINE(pt1, pt2)	 								\
	retval |= _gfxop_draw_line_clipped(state, pt1, pt2, color, line_mode, line_style);	\
	draw_line_to_control_map(state, pt1##_u, pt2##_u, color);				\
	_gfxop_add_dirty_x(state,								\
			   gfx_rect(pt1##_u.x, pt1##_u.y, pt2##_u.x - pt1##_u.x, pt2##_u.y - pt1##_u.y))

	PARTIAL_LINE(upper_left, upper_right);
	PARTIAL_LINE(upper_right, lower_right);
	PARTIAL_LINE(lower_right, lower_left);
	PARTIAL_LINE(lower_left, upper_left);

#undef PARTIAL_LINE
	if (retval) {
		GFXERROR("Failed to draw rectangle (%d,%d)+(%d,%d)\n", rect.x, rect.y, rect.xl, rect.yl);
		return retval;
	}
	return GFX_OK;
}


#define COLOR_MIX(type, dist) ((color1.type * dist) + (color2.type * (1.0 - dist)))


int
gfxop_draw_box(gfx_state_t *state, rect_t box, gfx_color_t color1, gfx_color_t color2,
               gfx_box_shade_t shade_type) {
	gfx_driver_t *drv = state->driver;
	int reverse = 0; /* switch color1 and color2 */
	float mod_offset = 0.0, mod_breadth = 1.0; /* 0.0 to 1.0: Color adjustment */
	gfx_rectangle_fill_t driver_shade_type;
	rect_t new_box;

	BASIC_CHECKS(GFX_FATAL);
	REMOVE_POINTER;

	if (PALETTE_MODE || !(state->driver->capabilities & GFX_CAPABILITY_SHADING))
		shade_type = GFX_BOX_SHADE_FLAT;


	_gfxop_add_dirty(state, box);

	if (color1.mask & GFX_MASK_CONTROL) {
		/* Write control block, clipped by 320x200 */
		memcpy(&new_box, &box, sizeof(rect_t));
		_gfxop_clip(&new_box, gfx_rect(0, 0, 320, 200));

		gfx_draw_box_pixmap_i(state->control_map, new_box, color1.control);
	}

	_gfxop_scale_rect(&box, state->driver->mode);

	if (!(color1.mask & (GFX_MASK_VISUAL | GFX_MASK_PRIORITY)))
		return GFX_OK; /* So long... */

	if (box.xl <= 1 || box.yl <= 1) {
		GFXDEBUG("Attempt to draw box with size %dx%d\n", box.xl, box.yl);
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
		mod_offset = (float)(((new_box.x - box.x) * 1.0) / (box.xl * 1.0));
		mod_breadth = (float)((new_box.xl * 1.0) / (box.xl * 1.0));
		break;

	case GFX_BOX_SHADE_UP:
		reverse = 1;
	case GFX_BOX_SHADE_DOWN:
		driver_shade_type = GFX_SHADE_VERTICALLY;
		mod_offset = (float)(((new_box.y - box.y) * 1.0) / (box.yl * 1.0));
		mod_breadth = (float)((new_box.yl * 1.0) / (box.yl * 1.0));
		break;

	default:
		GFXERROR("Invalid shade type: %d\n", shade_type);
		return GFX_ERROR;
	}


	if (reverse)
		mod_offset = (float)(1.0 - (mod_offset + mod_breadth));
	/* Reverse offset if we have to interpret colors inversely */

	if (shade_type == GFX_BOX_SHADE_FLAT)
		return drv->draw_filled_rect(drv, new_box, color1, color1, GFX_SHADE_FLAT);
	else {
		if (PALETTE_MODE) {
			GFXWARN("Attempting to draw shaded box in palette mode!\n");
			return GFX_ERROR;
		}

		gfx_color_t draw_color1 = {{0, 0, 0, 0}, 0, 0, 0, 0};
		gfx_color_t draw_color2 = {{0, 0, 0, 0}, 0, 0, 0, 0};

		draw_color1.mask = draw_color2.mask = color1.mask;
		draw_color1.priority = draw_color2.priority = color1.priority;

		if (draw_color1.mask & GFX_MASK_VISUAL) {
			draw_color1.visual.r = (guint8) COLOR_MIX(visual.r, mod_offset);
			draw_color1.visual.g = (guint8) COLOR_MIX(visual.g, mod_offset);
			draw_color1.visual.b = (guint8) COLOR_MIX(visual.b, mod_offset);
			draw_color1.alpha = (guint8) COLOR_MIX(alpha, mod_offset);

			mod_offset += mod_breadth;

			draw_color2.visual.r = (guint8) COLOR_MIX(visual.r, mod_offset);
			draw_color2.visual.g = (guint8) COLOR_MIX(visual.g, mod_offset);
			draw_color2.visual.b = (guint8) COLOR_MIX(visual.b, mod_offset);
			draw_color2.alpha = (guint8) COLOR_MIX(alpha, mod_offset);
		}
		if (reverse)
			return drv->draw_filled_rect(drv, new_box, draw_color2, draw_color1, driver_shade_type);
		else
			return drv->draw_filled_rect(drv, new_box, draw_color1, draw_color2, driver_shade_type);
	}
}
#undef COLOR_MIX


int
gfxop_fill_box(gfx_state_t *state, rect_t box, gfx_color_t color) {
	return gfxop_draw_box(state, box, color, color, GFX_BOX_SHADE_FLAT);
}



static int
_gfxop_buffer_propagate_box(gfx_state_t *state, rect_t box, gfx_buffer_t buffer) {
	int error;

	if (_gfxop_clip(&box, gfx_rect(0, 0, 320 * state->driver->mode->xfact, 200 * state->driver->mode->yfact)))
		return GFX_OK;

	if ((error = state->driver->update(state->driver, box, Common::Point(box.x, box.y), buffer))) {
		GFXERROR("Error occured while updating region (%d,%d,%d,%d) in buffer %d\n",
		         box.x, box.y, box.xl, box.yl, buffer);
		return error;
	}
	return GFX_OK;
}

extern int sci0_palette;
int
gfxop_clear_box(gfx_state_t *state, rect_t box) {
	BASIC_CHECKS(GFX_FATAL);
	REMOVE_POINTER;
	_gfxop_add_dirty(state, box);
	DDIRTY(stderr, "[]  clearing box %d %d %d %d\n", GFX_PRINT_RECT(box));
	if (box.x == 29
	        && box.y == 77
	        && (sci0_palette == 1)) {
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

int
gfxop_set_visible_map(gfx_state_t *state, gfx_map_mask_t visible_map) {
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

int
gfxop_update(gfx_state_t *state) {
	int retval;

	BASIC_CHECKS(GFX_FATAL);
	DRAW_POINTER;

	retval = _gfxop_clear_dirty_rec(state, state->dirty_rects);

	state->dirty_rects = NULL;

	if (state->fullscreen_override) {
		/* We've been asked to re-draw the active full-screen image, essentially. */
		rect_t rect = gfx_rect(0, 0, 320, 200);
		gfx_xlate_pixmap(state->fullscreen_override, state->driver->mode, GFX_XLATE_FILTER_NONE);
		gfxop_draw_pixmap(state, state->fullscreen_override, rect, Common::Point(0, 0));
		retval |= _gfxop_update_box(state, rect);
	}

	if (retval) {
		GFXERROR("Clearing the dirty rectangles failed!\n");
	}

	if (state->tag_mode) {
		/* This usually happens after a pic and all resources have been drawn */
		gfxr_free_tagged_resources(state->driver, state->resstate);
		state->tag_mode = 0;
	}

	return retval;
}


int
gfxop_update_box(gfx_state_t *state, rect_t box) {
	BASIC_CHECKS(GFX_FATAL);
	DRAW_POINTER;

	if (state->disable_dirty)
		_gfxop_update_box(state, box);
	else
		_gfxop_add_dirty(state, box);

	return gfxop_update(state);
}

int
gfxop_enable_dirty_frames(gfx_state_t *state) {
	BASIC_CHECKS(GFX_ERROR);
	state->disable_dirty = 0;

	return GFX_OK;
}

int
gfxop_disable_dirty_frames(gfx_state_t *state) {
	BASIC_CHECKS(GFX_ERROR);

	state->disable_dirty = 1;

	return GFX_OK;
}


/**********************/
/* Pointer and IO ops */
/**********************/

#define SECONDS_OF_DAY (24*60*60)
#define MILLION 1000000
/* Sure, this may seem silly, but it's too easy to miss a zero...) */


#define GFXOP_FULL_POINTER_REFRESH if (_gfxop_full_pointer_refresh(state)) { GFXERROR("Failed to do full pointer refresh!\n"); return GFX_ERROR; }

static int
_gfxop_full_pointer_refresh(gfx_state_t *state) {
	rect_t pointer_bounds;
	rect_t old_pointer_bounds	= {0, 0, 0, 0};
	int new_x			= state->driver->pointer_x;
	int new_y			= state->driver->pointer_y;

	if (new_x != state->old_pointer_draw_pos.x
	        || new_y != state->old_pointer_draw_pos.y) {
		Common::Point pp_new = Common::Point(new_x / state->driver->mode->xfact,
		                           new_y / state->driver->mode->yfact);

		if (!_gfxop_get_pointer_bounds(state, &pointer_bounds)) {
			memcpy(&old_pointer_bounds, &(state->pointer_bg_zone), sizeof(rect_t));
			REMOVE_POINTER;
			state->pointer_pos = pp_new;

			DRAW_POINTER;
			if (_gfxop_buffer_propagate_box(state, pointer_bounds, GFX_BUFFER_FRONT)) return 1;
			if (_gfxop_buffer_propagate_box(state, old_pointer_bounds, GFX_BUFFER_FRONT)) return 1;

			state->old_pointer_draw_pos = Common::Point(new_x, new_y);
		} else
			state->pointer_pos = pp_new;
	}
	return 0;
}

int
gfxop_usleep(gfx_state_t *state, long usecs) {
	long time, utime;
	long wakeup_time, wakeup_utime;
	long add_seconds;
	int retval = GFX_OK;

	BASIC_CHECKS(GFX_FATAL);

	sci_gettime(&wakeup_time, &wakeup_utime);
	wakeup_utime += usecs;

	add_seconds = (wakeup_utime / MILLION);
	wakeup_time += add_seconds;
	wakeup_utime -= (MILLION * add_seconds);

	do {
		GFXOP_FULL_POINTER_REFRESH;
		sci_gettime(&time, &utime);
		usecs = (wakeup_time - time) * MILLION + wakeup_utime - utime;
	} while ((usecs > 0) && !(retval = state->driver->usec_sleep(state->driver, usecs)));

	if (retval) {
		GFXWARN("Waiting failed\n");
	}

	return retval;
}


int
_gfxop_set_pointer(gfx_state_t *state, gfx_pixmap_t *pxm) {
	rect_t old_pointer_bounds = {0, 0, 0, 0};
	rect_t pointer_bounds = {0, 0, 0, 0};
	int retval = -1;
	int draw_old;
	int draw_new = 0;

	BASIC_CHECKS(GFX_FATAL);

	draw_old = state->mouse_pointer != NULL;

	draw_new = 0;
	state->driver->set_pointer(state->driver, pxm);
	state->mouse_pointer_in_hw = 1;

	if (!state->mouse_pointer_in_hw)
		draw_old = state->mouse_pointer != NULL;


	if (draw_old) {
		_gfxop_get_pointer_bounds(state, &old_pointer_bounds);
		REMOVE_POINTER;
	}


	if (draw_new) {
		state->mouse_pointer = pxm;
		DRAW_POINTER;
		_gfxop_get_pointer_bounds(state, &pointer_bounds);
	}

	if (draw_new && state->mouse_pointer)
		_gfxop_buffer_propagate_box(state, pointer_bounds, GFX_BUFFER_FRONT);

	if (draw_old)
		_gfxop_buffer_propagate_box(state, old_pointer_bounds, GFX_BUFFER_FRONT);

	if (state->mouse_pointer == NULL)
		state->mouse_pointer_visible = 0;
	else if (!state->mouse_pointer_visible)
		state->mouse_pointer_visible = 1;
	/* else don't touch it, as it might be VISIBLE_BUT_CLIPPED! */

	return GFX_OK;
}


int
gfxop_set_pointer_cursor(gfx_state_t *state, int nr) {
	gfx_pixmap_t *new_pointer = NULL;

	BASIC_CHECKS(GFX_FATAL);

	if (nr == GFXOP_NO_POINTER)
		new_pointer = NULL;
	else {
		new_pointer = gfxr_get_cursor(state->resstate, nr);

		if (!new_pointer) {
			GFXWARN("Attempt to set invalid pointer #%d\n", nr);
		}
	}

	return _gfxop_set_pointer(state, new_pointer);
}


int
gfxop_set_pointer_view(gfx_state_t *state, int nr, int loop, int cel, Common::Point  *hotspot) {
	int real_loop = loop;
	int real_cel = cel;
	gfx_pixmap_t *new_pointer = NULL;

	BASIC_CHECKS(GFX_FATAL);

	new_pointer = _gfxr_get_cel(state, nr, &real_loop, &real_cel,
	                            0); /* FIXME: For now, don't palettize pointers */

	if (hotspot) {
		new_pointer->xoffset = hotspot->x;
		new_pointer->yoffset = hotspot->y;
	}

	if (!new_pointer) {
		GFXWARN("Attempt to set invalid pointer #%d\n", nr);
		return GFX_ERROR;
	} else {
		if (real_loop != loop || real_cel != cel) {
			GFXDEBUG("Changed loop/cel from %d/%d to %d/%d in view %d\n",
			         loop, cel, real_loop, real_cel, nr);
		}
		return _gfxop_set_pointer(state, new_pointer);
	}
}

int
gfxop_set_pointer_position(gfx_state_t *state, Common::Point pos) {
	BASIC_CHECKS(GFX_ERROR);

	state->pointer_pos = pos;

	if (pos.x > 320 || pos.y > 200) {
		GFXWARN("Attempt to place pointer at invalid coordinates (%d, %d)\n", pos.x, pos.y);
		return 0; /* Not fatal */
	}

	state->driver->pointer_x = pos.x * state->driver->mode->xfact;
	state->driver->pointer_y = pos.y * state->driver->mode->yfact;

	GFXOP_FULL_POINTER_REFRESH;
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

static int
_gfxop_scancode(int ch)
/* Calculates a PC keyboard scancode from a character */
{
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

	return ch; /* not found */
}

/* static */
int
_gfxop_shiftify(int c) {
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
			return c; /* No match */
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

static int
_gfxop_numlockify(int c) {
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
		return c; /* Unchanged */
	}
}


sci_event_t
gfxop_get_event(gfx_state_t *state, unsigned int mask) {
	sci_event_t error_event = { SCI_EVT_ERROR, 0, 0, 0 };
	sci_event_t event = { 0, 0, 0, 0 };;
	gfx_input_event_t **seekerp = &(state->events);

	BASIC_CHECKS(error_event);
	if (_gfxop_remove_pointer(state)) {
		GFXERROR("Failed to remove pointer before processing event!\n");
	}

	while (*seekerp && !((*seekerp)->event.type & mask))
		seekerp = &((*seekerp)->next);

	if (*seekerp) {
		gfx_input_event_t *goner = *seekerp;
		event = goner->event;
		*seekerp = goner->next;
		free(goner);
	} else {
		event.type = 0;

		if (!(mask & SCI_EVT_NONBLOCK)) {
			do {
				if (event.type) {
					*seekerp = (gfx_input_event_t*)sci_malloc(sizeof(gfx_input_event_t));
					(*seekerp)->next = NULL;

					event.data = (char)(event.data);
					/* Clip illegal bits */

					(*seekerp)->event = event;
					seekerp = &((*seekerp)->next);
				}
				event = state->driver->get_event(state->driver);

			} while (event.type && !(event.type & mask));
		}
	}

	if (_gfxop_full_pointer_refresh(state)) {
		GFXERROR("Failed to update the mouse pointer!\n");
		return error_event;
	}

	if (event.type == SCI_EVT_KEYBOARD) {
		/* Do we still have to translate the key? */

		event.character = event.data;

		/* Scancodify if appropriate */
		if (event.buckybits & SCI_EVM_ALT)
			event.character = _gfxop_scancode(event.character);

		/* Shift if appropriate */
		else if (((event.buckybits & (SCI_EVM_RSHIFT | SCI_EVM_LSHIFT))
		          && !(event.buckybits & SCI_EVM_CAPSLOCK))
		         ||
		         (!(event.buckybits & (SCI_EVM_RSHIFT | SCI_EVM_LSHIFT))
		          && (event.buckybits & SCI_EVM_CAPSLOCK)))
			event.character = _gfxop_shiftify(event.character);

		/* Numlockify if appropriate */
		else if (event.buckybits & SCI_EVM_NUMLOCK)
			event.data = _gfxop_numlockify(event.data);
	}

	return event;
}


/*******************/
/* View operations */
/*******************/

int
gfxop_lookup_view_get_loops(gfx_state_t *state, int nr) {
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


int
gfxop_lookup_view_get_cels(gfx_state_t *state, int nr, int loop) {
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


int
gfxop_check_cel(gfx_state_t *state, int nr, int *loop, int *cel) {
	BASIC_CHECKS(GFX_ERROR);

	if (!gfxr_get_view(state->resstate, nr, loop, cel, 0)) {
		GFXWARN("Attempt to verify loop/cel values for invalid view %d\n", nr);
		return GFX_ERROR;
	}

	return GFX_OK;
}

int
gfxop_overflow_cel(gfx_state_t *state, int nr, int *loop, int *cel) {
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


int
gfxop_get_cel_parameters(gfx_state_t *state, int nr, int loop, int cel,
                         int *width, int *height, Common::Point *offset) {
	gfxr_view_t *view = NULL;
	gfx_pixmap_t *pxm = NULL;
	BASIC_CHECKS(GFX_ERROR);

	if (!(view = gfxr_get_view(state->resstate, nr, &loop, &cel, 0))) {
		GFXWARN("Attempt to get cel parameters for invalid view %d\n", nr);
		return GFX_ERROR;
	}

	pxm = view->loops[loop].cels[cel];
	*width = pxm->index_xl;
	*height = pxm->index_yl;
	offset->x = pxm->xoffset;
	offset->y = pxm->yoffset;

	return GFX_OK;
}


static int
_gfxop_draw_cel_buffer(gfx_state_t *state, int nr, int loop, int cel,
                       Common::Point pos, gfx_color_t color, int static_buf,
                       int palette) {
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
		_gfxop_add_dirty(state, gfx_rect(old_x, old_y, pxm->index_xl, pxm->index_yl));

	return _gfxop_draw_pixmap(state->driver, pxm, priority, control,
	                          gfx_rect(0, 0, pxm->xl, pxm->yl),
	                          gfx_rect(pos.x, pos.y, pxm->xl, pxm->yl),
	                          state->clip_zone,
	                          static_buf , state->control_map,
	                          static_buf
	                          ? state->static_priority_map
	                          : state->priority_map);
}


int
gfxop_draw_cel(gfx_state_t *state, int nr, int loop, int cel, Common::Point pos,
               gfx_color_t color, int palette) {
	return _gfxop_draw_cel_buffer(state, nr, loop, cel, pos, color, 0, palette);
}


int
gfxop_draw_cel_static(gfx_state_t *state, int nr, int loop, int cel, Common::Point pos,
                      gfx_color_t color, int palette) {
	int retval;
	rect_t oldclip = state->clip_zone;

	state->clip_zone = gfx_rect_fullscreen;
	_gfxop_scale_rect(&(state->clip_zone), state->driver->mode);
	retval = gfxop_draw_cel_static_clipped(state, nr, loop, cel, pos, color,
	                                       palette);
	/* Except that the area it's clipped against is... unusual ;-) */
	state->clip_zone = oldclip;

	return retval;
}


int
gfxop_draw_cel_static_clipped(gfx_state_t *state, int nr, int loop, int cel,
                              Common::Point pos, gfx_color_t color, int palette) {
	return _gfxop_draw_cel_buffer(state, nr, loop, cel, pos, color, 1, palette);
}


/******************/
/* Pic operations */
/******************/

static int
_gfxop_set_pic(gfx_state_t *state) {
	gfx_copy_pixmap_box_i(state->control_map, state->pic->control_map, gfx_rect(0, 0, 320, 200));
	gfx_copy_pixmap_box_i(state->priority_map, state->pic_unscaled->priority_map, gfx_rect(0, 0, 320, 200));
	gfx_copy_pixmap_box_i(state->static_priority_map, state->pic_unscaled->priority_map, gfx_rect(0, 0, 320, 200));

	_gfxop_install_pixmap(state->driver, state->pic->visual_map);

	if (state->options->pic0_unscaled)
		state->pic->priority_map = gfx_pixmap_scale_index_data(state->pic->priority_map, state->driver->mode);
	return state->driver->set_static_buffer(state->driver, state->pic->visual_map, state->pic->priority_map);
}


void *
gfxop_get_pic_metainfo(gfx_state_t *state) {
	return (state->pic) ? state->pic->internal : NULL;
}


int
gfxop_new_pic(gfx_state_t *state, int nr, int flags, int default_palette) {
	BASIC_CHECKS(GFX_FATAL);

	gfxr_tag_resources(state->resstate);
	state->tag_mode = 1;
	state->palette_nr = default_palette;

	state->pic = gfxr_get_pic(state->resstate, nr, GFX_MASK_VISUAL, flags, default_palette, 1);

	if (state->driver->mode->xfact == 1 && state->driver->mode->yfact == 1)
		state->pic_unscaled = state->pic;
	else
		state->pic_unscaled = gfxr_get_pic(state->resstate, nr, GFX_MASK_VISUAL, flags, default_palette, 0);

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


int
gfxop_add_to_pic(gfx_state_t *state, int nr, int flags, int default_palette) {
	BASIC_CHECKS(GFX_FATAL);

	if (!state->pic) {
		GFXERROR("Attempt to add to pic with no pic active!\n");
		return GFX_ERROR;
	}

	if (!(state->pic = gfxr_add_to_pic(state->resstate, state->pic_nr, nr,
	                                   GFX_MASK_VISUAL, flags, state->palette_nr, default_palette, 1))) {
		GFXERROR("Could not add pic #%d to pic #%d!\n", state->pic_nr, nr);
		return GFX_ERROR;
	}
	state->pic_unscaled = gfxr_add_to_pic(state->resstate, state->pic_nr, nr,
	                                      GFX_MASK_VISUAL, flags,
	                                      state->palette_nr,
	                                      default_palette, 1);

	return _gfxop_set_pic(state);
}


/*******************/
/* Text operations */
/*******************/


int
gfxop_get_font_height(gfx_state_t *state, int font_nr) {
	gfx_bitmap_font_t *font;
	BASIC_CHECKS(GFX_FATAL);

	font = gfxr_get_font(state->resstate, font_nr, 0);
	if (!font)
		return GFX_ERROR;

	return font->line_height;
}

int
gfxop_get_text_params(gfx_state_t *state, int font_nr, const char *text,
                      int maxwidth, int *width, int *height, int text_flags,
                      int *lines_nr, int *lineheight, int *lastline_width) {
	text_fragment_t *textsplits;
	gfx_bitmap_font_t *font;

	BASIC_CHECKS(GFX_FATAL);

	font = gfxr_get_font(state->resstate, font_nr, 0);

	if (!font) {
		GFXERROR("Attempt to calculate text size with invalid font #%d\n", font_nr);
		*width = *height = 0;
		return GFX_ERROR;
	}

	textsplits = gfxr_font_calculate_size(font, maxwidth, text, width,
	                                      height, lines_nr,
	                                      lineheight, lastline_width,
	                                      (state->options->workarounds & GFX_WORKAROUND_WHITESPACE_COUNT)
	                                      | text_flags);


	if (!textsplits) {
		GFXERROR("Could not calculate text size!");
		*width = *height = 0;
		return GFX_ERROR;
	}

	free(textsplits);
	return GFX_OK;
}


#define COL_XLATE(des,src) \
  des = src.visual; /* The new gfx_color_t structure makes things a lot easier :-) */ /* \
  if (gfxop_set_color(state, &src, \
		      src.visual.r, \
		      src.visual.g, \
		      src.visual.b, \
		      src.alpha, \
		      src.priority, \
		      src.control)) \
  { \
	  GFXERROR("Unable to set up colors"); \
	  return NULL; \
  }
*/

gfx_text_handle_t *
gfxop_new_text(gfx_state_t *state, int font_nr, char *text, int maxwidth,
               gfx_alignment_t halign, gfx_alignment_t valign,
               gfx_color_t color1, gfx_color_t color2, gfx_color_t bg_color,
               int flags) {
	gfx_text_handle_t *handle;
	gfx_bitmap_font_t *font;
	int i;
	gfx_pixmap_color_t pxm_col1, pxm_col2, pxm_colbg;
	BASIC_CHECKS(NULL);

	COL_XLATE(pxm_col1, color1);
	COL_XLATE(pxm_col2, color2);
	COL_XLATE(pxm_colbg, bg_color);

	font = gfxr_get_font(state->resstate, font_nr, 0);

	if (!font) {
		GFXERROR("Attempt to draw text with invalid font #%d\n", font_nr);
		return NULL;
	}

	handle = (gfx_text_handle_t*)sci_malloc(sizeof(gfx_text_handle_t));

	handle->text = (char*)sci_malloc(strlen(text) + 1);
	strcpy(handle->text, text);
	handle->halign = halign;
	handle->valign = valign;
	handle->line_height = font->line_height;

	handle->lines =
	    gfxr_font_calculate_size(font, maxwidth, handle->text, &(handle->width), &(handle->height),
	                             &(handle->lines_nr),
	                             NULL, NULL,
	                             ((state->options->workarounds & GFX_WORKAROUND_WHITESPACE_COUNT) ?
	                              GFXR_FONT_FLAG_COUNT_WHITESPACE : 0)
	                             | flags);

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

	handle->text_pixmaps = (gfx_pixmap_t**)sci_malloc(sizeof(gfx_pixmap_t *) * handle->lines_nr);

	for (i = 0; i < handle->lines_nr; i++) {
		int chars_nr = handle->lines[i].length;

		handle->text_pixmaps[i] = gfxr_draw_font(font, handle->lines[i].offset, chars_nr,
		                          (color1.mask & GFX_MASK_VISUAL) ? &pxm_col1 : NULL,
		                          (color2.mask & GFX_MASK_VISUAL) ? &pxm_col2 : NULL,
		                          (bg_color.mask & GFX_MASK_VISUAL) ? &pxm_colbg : NULL);

		if (!handle->text_pixmaps[i]) {
			int j;

			for (j = 0; j < i; j++)
				gfx_free_pixmap(state->driver, handle->text_pixmaps[j]);
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


int
gfxop_free_text(gfx_state_t *state, gfx_text_handle_t *handle) {
	int j;

	BASIC_CHECKS(GFX_ERROR);

	if (handle->text_pixmaps) {
		for (j = 0; j < handle->lines_nr; j++)
			gfx_free_pixmap(state->driver, handle->text_pixmaps[j]);
		free(handle->text_pixmaps);
	}

	free(handle->text);
	free(handle->lines);
	free(handle);
	return GFX_OK;
}


int
gfxop_draw_text(gfx_state_t *state, gfx_text_handle_t *handle, rect_t zone) {
	int line_height;
	rect_t pos;
	int i;
	BASIC_CHECKS(GFX_FATAL);
	REMOVE_POINTER;

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
		pos.y += (zone.yl - (line_height * handle->lines_nr)) >> 1;
		break;

	case ALIGN_BOTTOM:
		pos.y += (zone.yl - (line_height * handle->lines_nr));
		break;

	default:
		GFXERROR("Invalid vertical alignment %d!\n", handle->valign);
		return GFX_FATAL; /* Internal error... */
	}

	for (i = 0; i < handle->lines_nr; i++) {

		gfx_pixmap_t *pxm = handle->text_pixmaps[i];

		if (!pxm->data) {
			gfx_xlate_pixmap(pxm, state->driver->mode, state->options->text_xlate_filter);
			gfxr_endianness_adjust(pxm, state->driver->mode); /* FIXME: resmgr layer! */
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
			pos.x += (zone.xl - pxm->xl) >> 1;
			break;

		case ALIGN_RIGHT:
			pos.x += (zone.xl - pxm->xl);
			break;

		default:
			GFXERROR("Invalid vertical alignment %d!\n", handle->valign);
			return GFX_FATAL; /* Internal error... */
		}

		pos.xl = pxm->xl;
		pos.yl = pxm->yl;

		_gfxop_add_dirty(state, pos);

		_gfxop_draw_pixmap(state->driver, pxm, handle->priority, handle->control,
		                   gfx_rect(0, 0, pxm->xl, pxm->yl), pos, state->clip_zone, 0,
		                   state->control_map, state->priority_map);

		pos.y += line_height;
	}

	return GFX_OK;
}


gfx_pixmap_t *
gfxop_grab_pixmap(gfx_state_t *state, rect_t area) {
	gfx_pixmap_t *pixmap = NULL;
	rect_t resultzone; /* Ignored for this application */
	BASIC_CHECKS(NULL);
	if (_gfxop_remove_pointer(state)) {
		GFXERROR("Could not remove pointer!\n");
		return NULL;
	}

	_gfxop_scale_rect(&area, state->driver->mode);
	if (_gfxop_grab_pixmap(state, &pixmap, area.x, area.y, area.xl, area.yl, 0, &resultzone))
		return NULL; /* area CUT the visual screen had a null or negative size */

	pixmap->flags |= GFX_PIXMAP_FLAG_PALETTE_SET | GFX_PIXMAP_FLAG_DONT_UNALLOCATE_PALETTE;

	return pixmap;
}

int
gfxop_draw_pixmap(gfx_state_t *state, gfx_pixmap_t *pxm, rect_t zone, Common::Point pos) {
	rect_t target;
	BASIC_CHECKS(GFX_ERROR);

	if (!pxm) {
		GFXERROR("Attempt to draw NULL pixmap!\n");
		return GFX_ERROR;
	}

	REMOVE_POINTER;

	target = gfx_rect(pos.x, pos.y, zone.xl, zone.yl);

	_gfxop_add_dirty(state, target);

	if (!pxm) {
		GFXERROR("Attempt to draw_pixmap with pxm=NULL\n");
		return GFX_ERROR;
	}

	_gfxop_scale_rect(&zone, state->driver->mode);
	_gfxop_scale_rect(&target, state->driver->mode);

	return _gfxop_draw_pixmap(state->driver, pxm, -1, -1, zone, target,
	                          gfx_rect(0, 0, 320*state->driver->mode->xfact,
	                                   200*state->driver->mode->yfact), 0, NULL, NULL);
}

int
gfxop_free_pixmap(gfx_state_t *state, gfx_pixmap_t *pxm) {
	BASIC_CHECKS(GFX_ERROR);
	gfx_free_pixmap(state->driver, pxm);
	return GFX_OK;
}

