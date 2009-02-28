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

// The interpreter-specific part of the resource manager, for SCI

#include "sci/sci_memory.h"
#include "sci/scicore/resource.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_resmgr.h"
#include "sci/gfx/gfx_options.h"

#include "common/util.h"

namespace Sci {

int gfxr_interpreter_options_hash(gfx_resource_type_t type, int version, gfx_options_t *options, void *internal, int palette) {
	switch (type) {
	case GFX_RESOURCE_TYPE_VIEW:
		return palette;

	case GFX_RESOURCE_TYPE_PIC:
		if (version >= SCI_VERSION_01_VGA)
			return options->pic_port_bounds.y;
		else
			return (options->pic0_unscaled) ? 0x10000 : (options->pic0_dither_mode << 12)
			       | (options->pic0_dither_pattern << 8) | (options->pic0_brush_mode << 4) | (options->pic0_line_mode);

	case GFX_RESOURCE_TYPE_FONT:
		return 0;

	case GFX_RESOURCE_TYPE_CURSOR:
		return 0;

	case GFX_RESOURCE_TYPES_NR:
	default:
		GFXERROR("Invalid resource type: %d\n", type);
		return -1;
	}
}

gfxr_pic_t *gfxr_interpreter_init_pic(int version, gfx_mode_t *mode, int ID, void *internal) {
	return gfxr_init_pic(mode, ID, version >= SCI_VERSION_01_VGA);
}

void gfxr_interpreter_clear_pic(int version, gfxr_pic_t *pic, void *internal) {
	gfxr_clear_pic0(pic, SCI_TITLEBAR_SIZE);
}

int gfxr_interpreter_calculate_pic(gfx_resstate_t *state, gfxr_pic_t *scaled_pic, gfxr_pic_t *unscaled_pic,
	int flags, int default_palette, int nr, void *internal) {
	ResourceManager *resmgr = (ResourceManager *)state->misc_payload;
	Resource *res = resmgr->findResource(sci_pic, nr, 0);
	int need_unscaled = unscaled_pic != NULL;
	gfxr_pic0_params_t style, basic_style;

	basic_style.line_mode = GFX_LINE_MODE_CORRECT;
	basic_style.brush_mode = GFX_BRUSH_MODE_SCALED;
	basic_style.pic_port_bounds = state->options->pic_port_bounds;

	style.line_mode = state->options->pic0_line_mode;
	style.brush_mode = state->options->pic0_brush_mode;
	style.pic_port_bounds = state->options->pic_port_bounds;

	if (!res || !res->data)
		return GFX_ERROR;

	if (state->version >= SCI_VERSION_01_VGA) {
		if (need_unscaled) {
			if (state->version == SCI_VERSION_1_1)
				gfxr_draw_pic11(unscaled_pic, flags, default_palette, res->size, res->data, &basic_style, res->id,
				                state->static_palette, state->static_palette_entries);
			else
				gfxr_draw_pic01(unscaled_pic, flags, default_palette, res->size, res->data, &basic_style, res->id, 1,
				                state->static_palette, state->static_palette_entries);
		}
		if (scaled_pic && scaled_pic->undithered_buffer)
			memcpy(scaled_pic->visual_map->index_data, scaled_pic->undithered_buffer, scaled_pic->undithered_buffer_size);

		if (state->version == SCI_VERSION_1_1)
			gfxr_draw_pic11(scaled_pic, flags, default_palette, res->size, res->data, &style, res->id,
			                state->static_palette, state->static_palette_entries);
		else
			gfxr_draw_pic01(scaled_pic, flags, default_palette, res->size, res->data, &style, res->id, state->version,
			                state->static_palette, state->static_palette_entries);
	} else {
		if (need_unscaled)
			gfxr_draw_pic01(unscaled_pic, flags, default_palette, res->size, res->data, &basic_style, res->id, 0,
			                state->static_palette, state->static_palette_entries);

		if (scaled_pic && scaled_pic->undithered_buffer)
			memcpy(scaled_pic->visual_map->index_data, scaled_pic->undithered_buffer, scaled_pic->undithered_buffer_size);

		gfxr_draw_pic01(scaled_pic, flags, default_palette, res->size, res->data, &style, res->id, 0,
		                state->static_palette, state->static_palette_entries);
		if (need_unscaled)
			gfxr_remove_artifacts_pic0(scaled_pic, unscaled_pic);

		if (!scaled_pic->undithered_buffer)
			scaled_pic->undithered_buffer = sci_malloc(scaled_pic->undithered_buffer_size);

		memcpy(scaled_pic->undithered_buffer, scaled_pic->visual_map->index_data, scaled_pic->undithered_buffer_size);

		gfxr_dither_pic0(scaled_pic, state->options->pic0_dither_mode, state->options->pic0_dither_pattern);
	}

	// Mark default palettes
	if (scaled_pic)
		scaled_pic->visual_map->loop = default_palette;

	if (unscaled_pic)
		unscaled_pic->visual_map->loop = default_palette;

	return GFX_OK;
}

void gfxr_palettize_view(gfxr_view_t *view, gfx_pixmap_color_t *source, int source_entries) {
	int i;

	for (i = 0;i < MIN(view->colors_nr, source_entries);i++) {
		if ((view->colors[i].r == 0) && (view->colors[i].g == 0) && (view->colors[i].b == 0)) {
			view->colors[i] = source[i];
		}
	}
}

gfxr_view_t *gfxr_draw_view11(int id, byte *resource, int size);

gfxr_view_t *gfxr_interpreter_get_view(gfx_resstate_t *state, int nr, void *internal, int palette) {
	ResourceManager *resmgr = (ResourceManager *) state->misc_payload;
	Resource *res = resmgr->findResource(sci_view, nr, 0);
	int resid = GFXR_RES_ID(GFX_RESOURCE_TYPE_VIEW, nr);
	gfxr_view_t *result = 0;

	if (!res || !res->data)
		return NULL;

	if (state->version < SCI_VERSION_01) palette = -1;

	switch (state->version) {
	case SCI_VERSION_0:
	case SCI_VERSION_01:
		result = gfxr_draw_view0(resid, res->data, res->size, palette);
		break;
	case SCI_VERSION_01_VGA:
	case SCI_VERSION_01_VGA_ODD:
	case SCI_VERSION_1_EARLY:
	case SCI_VERSION_1_LATE:
		result = gfxr_draw_view1(resid, res->data, res->size, state->static_palette, state->static_palette_entries);
		break;
	case SCI_VERSION_1_1:
	case SCI_VERSION_32:
		result = gfxr_draw_view11(resid, res->data, res->size);
		break;
	}

	if (state->version >= SCI_VERSION_01_VGA) {
		if (!result->colors) {
			result->colors = (gfx_pixmap_color_t *)sci_malloc(sizeof(gfx_pixmap_color_t) * state->static_palette_entries);
			memset(result->colors, 0, sizeof(gfx_pixmap_color_t) * state->static_palette_entries);
			result->colors_nr = state->static_palette_entries;
		}
		gfxr_palettize_view(result, state->static_palette, state->static_palette_entries);
	}
	return result;
}

gfx_bitmap_font_t *gfxr_interpreter_get_font(gfx_resstate_t *state, int nr, void *internal) {
	ResourceManager *resmgr = (ResourceManager *)state->misc_payload;
	Resource *res = resmgr->findResource(sci_font, nr, 0);
	if (!res || !res->data)
		return NULL;

	return gfxr_read_font(res->id, res->data, res->size);
}

gfx_pixmap_t *gfxr_interpreter_get_cursor(gfx_resstate_t *state, int nr, void *internal) {
	ResourceManager *resmgr = (ResourceManager *) state->misc_payload;
	Resource *res = resmgr->findResource(sci_cursor, nr, 0);
	int resid = GFXR_RES_ID(GFX_RESOURCE_TYPE_CURSOR, nr);

	if (!res || !res->data)
		return NULL;

	if (state->version >= SCI_VERSION_1_1) {
		GFXWARN("Attempt to retrieve cursor in SCI1.1 or later\n");
		return NULL;
	}

	if (state->version == SCI_VERSION_0)
		return gfxr_draw_cursor0(resid, res->data, res->size);
	else
		return gfxr_draw_cursor01(resid, res->data, res->size);
}

int *gfxr_interpreter_get_resources(gfx_resstate_t *state, gfx_resource_type_t type, int version, int *entries_nr, void *internal) {
	ResourceManager *resmgr = (ResourceManager *) state->misc_payload;
	int restype;
	int *resources;
	int count = 0;
	int top = sci_max_resource_nr[version] + 1;
	int i;
	switch (type) {

	case GFX_RESOURCE_TYPE_VIEW:
		restype = sci_view;
		break;

	case GFX_RESOURCE_TYPE_PIC:
		restype = sci_pic;
		break;

	case GFX_RESOURCE_TYPE_CURSOR:
		restype = sci_cursor;
		break;

	case GFX_RESOURCE_TYPE_FONT:
		restype = sci_font;
		break;

	default:
		GFXDEBUG("Unsupported resource %d\n", type);
		return NULL;
	}

	resources = (int *)sci_malloc(sizeof(int) * top);

	for (i = 0; i < top; i++)
		if (resmgr->testResource(restype, i))
			resources[count++] = i;

	*entries_nr = count;

	return resources;
}

gfx_pixmap_color_t *gfxr_interpreter_get_static_palette(gfx_resstate_t *state, int version, int *colors_nr, void *internal) {
	if (version >= SCI_VERSION_01_VGA)
		return gfxr_interpreter_get_palette(state, version, colors_nr, internal, 999);

	*colors_nr = GFX_SCI0_PIC_COLORS_NR;
	return gfx_sci0_pic_colors;
}

gfx_pixmap_color_t *gfxr_interpreter_get_palette(gfx_resstate_t *state, int version, int *colors_nr, void *internal, int nr) {
	ResourceManager *resmgr = (ResourceManager *)state->misc_payload;
	Resource *res;

	if (version < SCI_VERSION_01_VGA)
		return NULL;

	res = resmgr->findResource(sci_palette, nr, 0);
	if (!res || !res->data)
		return NULL;

	switch (version) {
	case SCI_VERSION_01_VGA :
	case SCI_VERSION_01_VGA_ODD :
	case SCI_VERSION_1_EARLY :
	case SCI_VERSION_1_LATE :
		return gfxr_read_pal1(res->id, colors_nr, res->data, res->size);
	case SCI_VERSION_1_1 :
	case SCI_VERSION_32 :
		GFXDEBUG("Palettes are not yet supported in this SCI version\n");
		return NULL;

	default:
		BREAKPOINT();
		return NULL;
	}
}

int gfxr_interpreter_needs_multicolored_pointers(int version, void *internal) {
	return (version > SCI_VERSION_1);
}

} // End of namespace Sci
