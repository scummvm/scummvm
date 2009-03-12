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
#include "sci/gfx/font.h"

#include "common/util.h"

namespace Sci {

int gfxr_interpreter_options_hash(gfx_resource_type_t type, int version, gfx_options_t *options, int palette) {
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

gfxr_pic_t *gfxr_interpreter_init_pic(int version, gfx_mode_t *mode, int ID) {
	return gfxr_init_pic(mode, ID, version >= SCI_VERSION_01_VGA);
}

void gfxr_interpreter_clear_pic(int version, gfxr_pic_t *pic) {
	gfxr_clear_pic0(pic, SCI_TITLEBAR_SIZE);
}

int gfxr_interpreter_calculate_pic(gfx_resstate_t *state, gfxr_pic_t *scaled_pic, gfxr_pic_t *unscaled_pic,
	int flags, int default_palette, int nr) {
	ResourceManager& resourceManager = *(state->resManager);
	Resource *res = resourceManager.findResource(kResourceTypePic, nr, 0);
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
				gfxr_draw_pic11(unscaled_pic, flags, default_palette, res->size, res->data, &basic_style, res->id, state->static_palette);
			else
				gfxr_draw_pic01(unscaled_pic, flags, default_palette, res->size, res->data, &basic_style, res->id, 1, state->static_palette);
		}
		if (scaled_pic && scaled_pic->undithered_buffer)
			memcpy(scaled_pic->visual_map->index_data, scaled_pic->undithered_buffer, scaled_pic->undithered_buffer_size);

		if (state->version == SCI_VERSION_1_1)
			gfxr_draw_pic11(scaled_pic, flags, default_palette, res->size, res->data, &style, res->id, state->static_palette);
		else
			gfxr_draw_pic01(scaled_pic, flags, default_palette, res->size, res->data, &style, res->id, state->version, state->static_palette);
	} else {
		if (need_unscaled)
			gfxr_draw_pic01(unscaled_pic, flags, default_palette, res->size, res->data, &basic_style, res->id, 0, state->static_palette);

		if (scaled_pic && scaled_pic->undithered_buffer)
			memcpy(scaled_pic->visual_map->index_data, scaled_pic->undithered_buffer, scaled_pic->undithered_buffer_size);

		gfxr_draw_pic01(scaled_pic, flags, default_palette, res->size, res->data, &style, res->id, 0, state->static_palette);
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

void gfxr_palettize_view(gfxr_view_t *view, Palette *source) {
	for (unsigned i = 0; i < MIN(view->palette->size(), source->size()); i++) {
		const PaletteEntry& vc = view->palette->getColor(i);
		if (vc.r == 0 && vc.g == 0 && vc.b == 0) {
			const PaletteEntry& sc = source->getColor(i);
			view->palette->setColor(i, sc.r, sc.g, sc.b);
		}
	}
}

gfxr_view_t *gfxr_draw_view11(int id, byte *resource, int size);

gfxr_view_t *gfxr_interpreter_get_view(ResourceManager& resourceManager, int nr, int palette, Palette* staticPalette, int version) {
	Resource *res = resourceManager.findResource(kResourceTypeView, nr, 0);
	int resid = GFXR_RES_ID(GFX_RESOURCE_TYPE_VIEW, nr);
	gfxr_view_t *result = 0;

	if (!res || !res->data)
		return NULL;

	if (version < SCI_VERSION_01) palette = -1;

	switch (version) {
	case SCI_VERSION_0:
	case SCI_VERSION_01:
		result = gfxr_draw_view0(resid, res->data, res->size, palette);
		break;
	case SCI_VERSION_01_VGA:
	case SCI_VERSION_01_VGA_ODD:
	case SCI_VERSION_1_EARLY:
	case SCI_VERSION_1_LATE:
		result = gfxr_draw_view1(resid, res->data, res->size, staticPalette);
		break;
	case SCI_VERSION_1_1:
	case SCI_VERSION_32:
		result = gfxr_draw_view11(resid, res->data, res->size);
		break;
	}

	if (version >= SCI_VERSION_01_VGA) {
		if (!result->palette) {
			result->palette = new Palette(staticPalette->size());
			result->palette->name = "interpreter_get_view";
		}
		gfxr_palettize_view(result, staticPalette);
	}
	return result;
}

gfx_bitmap_font_t *gfxr_interpreter_get_font(ResourceManager& resourceManager, int nr) {
	Resource *res = resourceManager.findResource(kResourceTypeFont, nr, 0);
	if (!res || !res->data)
		return NULL;

	return gfxr_read_font(res->id, res->data, res->size);
}

gfx_pixmap_t *gfxr_interpreter_get_cursor(ResourceManager& resourceManager, int nr, int version) {
	Resource *res = resourceManager.findResource(kResourceTypeCursor, nr, 0);
	int resid = GFXR_RES_ID(GFX_RESOURCE_TYPE_CURSOR, nr);

	if (!res || !res->data)
		return NULL;

	if (version >= SCI_VERSION_1_1) {
		GFXWARN("Attempt to retrieve cursor in SCI1.1 or later\n");
		return NULL;
	}

	return gfxr_draw_cursor(resid, res->data, res->size, version != SCI_VERSION_0);
}

int *gfxr_interpreter_get_resources(ResourceManager& resourceManager, gfx_resource_type_t type, int version, int *entries_nr) {
	ResourceType restype;
	int *resources;
	int count = 0;
	int top = sci_max_resource_nr[version] + 1;
	int i;
	switch (type) {

	case GFX_RESOURCE_TYPE_VIEW:
		restype = kResourceTypeView;
		break;

	case GFX_RESOURCE_TYPE_PIC:
		restype = kResourceTypePic;
		break;

	case GFX_RESOURCE_TYPE_CURSOR:
		restype = kResourceTypeCursor;
		break;

	case GFX_RESOURCE_TYPE_FONT:
		restype = kResourceTypeFont;
		break;

	default:
		GFXDEBUG("Unsupported resource %d\n", type);
		return NULL;
	}

	resources = (int *)sci_malloc(sizeof(int) * top);

	for (i = 0; i < top; i++)
		if (resourceManager.testResource(restype, i))
			resources[count++] = i;

	*entries_nr = count;

	return resources;
}

Palette *gfxr_interpreter_get_static_palette(ResourceManager& resourceManager, int version, int *colors_nr) {
	if (version >= SCI_VERSION_01_VGA)
		return gfxr_interpreter_get_palette(resourceManager, version, colors_nr, 999);

	*colors_nr = GFX_SCI0_PIC_COLORS_NR;
	return gfx_sci0_pic_colors->getref();
}

Palette *gfxr_interpreter_get_palette(ResourceManager& resourceManager, int version, int *colors_nr, int nr) {
	Resource *res;

	if (version < SCI_VERSION_01_VGA)
		return NULL;

	res = resourceManager.findResource(kResourceTypePalette, nr, 0);
	if (!res || !res->data)
		return NULL;

	switch (version) {
	case SCI_VERSION_01_VGA :
	case SCI_VERSION_01_VGA_ODD :
	case SCI_VERSION_1_EARLY :
	case SCI_VERSION_1_LATE :
		return gfxr_read_pal1(res->id, res->data, res->size);
	case SCI_VERSION_1_1 :
	case SCI_VERSION_32 :
		GFXDEBUG("Palettes are not yet supported in this SCI version\n");
		return NULL;

	default:
		BREAKPOINT();
		return NULL;
	}
}

int gfxr_interpreter_needs_multicolored_pointers(int version) {
	return (version > SCI_VERSION_1);
}

} // End of namespace Sci
