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

// Resource manager core part

// FIXME/TODO: The name "(Graphics) resource manager", and the associated
// filenames, are misleading. This should be renamed to "Graphics manager"
// or something like that.

#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"
#include "sci/gfx/gfx_driver.h"
#include "sci/gfx/gfx_resmgr.h"
#include "sci/gfx/gfx_state_internal.h"
#include "sci/gfx/font.h"

#include "common/system.h"

namespace Sci {

// Invalid hash mode: Used to invalidate modified pics
#define MODE_INVALID -1

struct param_struct {
	int args[4];
	gfx_driver_t *driver;
};

#define DRAW_PIC01(pic, picStyle, isSci1) \
	gfxr_draw_pic01((pic), flags, default_palette, res->size, res->data, (picStyle), res->id, (isSci1), state->static_palette);

#define DRAW_PIC11(pic, picStyle) \
	gfxr_draw_pic11((pic), flags, default_palette, res->size, res->data, (picStyle), res->id, state->static_palette);

/* Calculate a picture
** Parameters: (gfx_resstate_t *) state: The resource state, containing options and version information
**             (gfxr_pic_t *) scaled_pic: The pic structure that is to be written to
**             (gfxr_pic_t *) unscaled_pic: The pic structure the unscaled pic is to be written to,
**                                          or NULL if it isn't needed.
**             (int) flags: Pic drawing flags (interpreter dependant)
**             (int) default_palette: The default palette to use for pic drawing (interpreter dependant)
**             (int) nr: pic resource number
** Returns   : (int) GFX_ERROR if the resource could not be found, GFX_OK otherwise
*/
int calculatePic(gfx_resstate_t *state, gfxr_pic_t *scaled_pic, gfxr_pic_t *unscaled_pic, int flags, int default_palette, int nr) {
	Resource *res = state->resManager->findResource(kResourceTypePic, nr, 0);
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

	if (need_unscaled) {
		if (state->version == SCI_VERSION_1_1)
			DRAW_PIC11(unscaled_pic, &basic_style)
		else
			DRAW_PIC01(unscaled_pic, &basic_style, state->version >= SCI_VERSION_01_VGA)
	}

	if (scaled_pic && scaled_pic->undithered_buffer)
		memcpy(scaled_pic->visual_map->index_data, scaled_pic->undithered_buffer, scaled_pic->undithered_buffer_size);

	if (state->version == SCI_VERSION_1_1)
		DRAW_PIC11(scaled_pic, &style)
	else
		DRAW_PIC01(scaled_pic, &style, state->version >= SCI_VERSION_01_VGA)

	if (state->version < SCI_VERSION_01_VGA) {
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

gfx_resstate_t *gfxr_new_resource_manager(int version, gfx_options_t *options, gfx_driver_t *driver, ResourceManager *resManager) {
	gfx_resstate_t *state = new gfx_resstate_t();

	state->version = version;
	state->options = options;
	state->driver = driver;
	state->resManager = resManager;
	state->static_palette = 0;

	state->tag_lock_counter = state->lock_counter = 0;

	return state;
}

int GfxResManager::getOptionsHash(gfx_resource_type_t type) {
	switch (type) {
	case GFX_RESOURCE_TYPE_VIEW:
		// This should never happen
		error("getOptionsHash called on a VIEW resource");

	case GFX_RESOURCE_TYPE_PIC:
		if (_state->version >= SCI_VERSION_01_VGA)
			return _state->options->pic_port_bounds.y;
		else
			return (_state->options->pic0_unscaled) ? 0x10000 : (_state->options->pic0_dither_mode << 12)
			       | (_state->options->pic0_dither_pattern << 8) | (_state->options->pic0_brush_mode << 4) 
				   | (_state->options->pic0_line_mode);

	case GFX_RESOURCE_TYPE_FONT:
	case GFX_RESOURCE_TYPE_CURSOR:
		return 0;

	case GFX_RESOURCE_TYPES_NR:
	default:
		GFXERROR("Invalid resource type: %d\n", type);
		return -1;
	}
}

#define FREEALL(freecmd, type) \
	if (resource->scaled_data.type) \
		freecmd(resource->scaled_data.type); \
	resource->scaled_data.type = NULL; \
	if (resource->unscaled_data.type) \
		freecmd(resource->unscaled_data.type); \
	resource->unscaled_data.type = NULL;

void gfxr_free_resource(gfx_resource_t *resource, int type) {
	if (!resource)
		return;

	switch (type) {

	case GFX_RESOURCE_TYPE_VIEW:
		FREEALL(gfxr_free_view, view);
		break;

	case GFX_RESOURCE_TYPE_PIC:
		FREEALL(gfxr_free_pic, pic);
		break;

	case GFX_RESOURCE_TYPE_FONT:
		FREEALL(gfxr_free_font, font);
		break;

	case GFX_RESOURCE_TYPE_CURSOR:
		FREEALL(gfx_free_pixmap, pointer);
		break;

	default:
		GFXWARN("Attempt to free invalid resource type %d\n", type);
	}

	free(resource);
}

void GfxResManager::freeAllResources() {
	for (int type = 0; type < GFX_RESOURCE_TYPES_NR; ++type) {
		for (IntResMap::iterator iter = _state->_resourceMaps[type].begin(); iter != _state->_resourceMaps[type].end(); ++iter) {
			gfxr_free_resource(iter->_value, type);
			iter->_value = 0;
		}
	}
}

void GfxResManager::freeResManager() {
	freeAllResources();
	delete _state;
}

void GfxResManager::freeTaggedResources() {
	// Current heuristics: free tagged views and old pics

	IntResMap::iterator iter;
	int type;
	const int tmp = _state->tag_lock_counter;

	type = GFX_RESOURCE_TYPE_VIEW;
	for (iter = _state->_resourceMaps[type].begin(); iter != _state->_resourceMaps[type].end(); ++iter) {
		gfx_resource_t *resource = iter->_value;

		if (resource) {
			if (resource->lock_sequence_nr < tmp) {
				gfxr_free_resource(resource, type);
				iter->_value = 0;
			} else {
				resource->lock_sequence_nr = 0;
			}
		}
	}

	type = GFX_RESOURCE_TYPE_PIC;
	for (iter = _state->_resourceMaps[type].begin(); iter != _state->_resourceMaps[type].end(); ++iter) {
		gfx_resource_t *resource = iter->_value;

		if (resource) {
			if (resource->lock_sequence_nr < 0) {
				gfxr_free_resource(resource, type);
				iter->_value = 0;
			} else {
				resource->lock_sequence_nr--;
			}
		}
	}

	_state->tag_lock_counter = 0;
}

#define XLATE_AS_APPROPRIATE(key, entry) \
	if (maps & key) { \
		if (res->unscaled_data.pic&& (force || !res->unscaled_data.pic->entry->data)) { \
				if (key == GFX_MASK_VISUAL) \
					gfx_get_res_config(options, res->unscaled_data.pic->entry); \
			        gfx_xlate_pixmap(res->unscaled_data.pic->entry, mode, filter); \
		} if (scaled && res->scaled_data.pic && (force || !res->scaled_data.pic->entry->data)) { \
				if (key == GFX_MASK_VISUAL) \
					gfx_get_res_config(options, res->scaled_data.pic->entry); \
				gfx_xlate_pixmap(res->scaled_data.pic->entry, mode, filter); \
		} \
	}

static gfxr_pic_t *gfxr_pic_xlate_common(gfx_resource_t *res, int maps, int scaled, int force, gfx_mode_t *mode,
										 gfx_xlate_filter_t filter, int endianize, gfx_options_t *options) {
	XLATE_AS_APPROPRIATE(GFX_MASK_VISUAL, visual_map);
	XLATE_AS_APPROPRIATE(GFX_MASK_PRIORITY, priority_map);
	XLATE_AS_APPROPRIATE(GFX_MASK_CONTROL, control_map);

	if (endianize && (maps & GFX_MASK_VISUAL) && res->scaled_data.pic->visual_map)
		gfxr_endianness_adjust(res->scaled_data.pic->visual_map, mode);

	return scaled ? res->scaled_data.pic : res->unscaled_data.pic;
}
#undef XLATE_AS_APPROPRIATE

gfxr_pic_t *GfxResManager::getPic(int num, int maps, int flags, int default_palette, bool scaled) {
	gfxr_pic_t *npic = NULL;
	IntResMap &resMap = _state->_resourceMaps[GFX_RESOURCE_TYPE_PIC];
	gfx_resource_t *res = NULL;
	int hash = getOptionsHash(GFX_RESOURCE_TYPE_PIC);
	int must_post_process_pic = 0;
	int need_unscaled = (_state->driver->mode->xfact != 1 || _state->driver->mode->yfact != 1);

	hash |= (flags << 20) | ((default_palette & 0x7) << 28);

	res = resMap.contains(num) ? resMap[num] : NULL;

	if (!res || res->mode != hash) {
		gfxr_pic_t *pic;
		gfxr_pic_t *unscaled_pic = NULL;

		if (_state->options->pic0_unscaled) {
			need_unscaled = 0;
			pic = gfxr_init_pic(&mode_1x1_color_index, GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, num), _state->version >= SCI_VERSION_01_VGA);
		} else
			pic = gfxr_init_pic(_state->driver->mode, GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, num), _state->version >= SCI_VERSION_01_VGA);
		if (!pic) {
			GFXERROR("Failed to allocate scaled pic!\n");
			return NULL;
		}

		gfxr_clear_pic0(pic, SCI_TITLEBAR_SIZE);

		if (need_unscaled) {
			unscaled_pic = gfxr_init_pic(&mode_1x1_color_index, GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, num), _state->version >= SCI_VERSION_01_VGA);
			if (!unscaled_pic) {
				GFXERROR("Failed to allocate unscaled pic!\n");
				return NULL;
			}
			gfxr_clear_pic0(pic, SCI_TITLEBAR_SIZE);
		}
		if (calculatePic(_state, pic, unscaled_pic, flags, default_palette, num)) {
			gfxr_free_pic(pic);
			if (unscaled_pic)
				gfxr_free_pic(unscaled_pic);
			return NULL;
		}
		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->ID = GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, num);
			res->lock_sequence_nr = _state->options->buffer_pics_nr;
			resMap[num] = res;
		} else {
			gfxr_free_pic(res->scaled_data.pic);
			if (res->unscaled_data.pic)
				gfxr_free_pic(res->unscaled_data.pic);
		}

		res->mode = hash;
		res->scaled_data.pic = pic;
		res->unscaled_data.pic = unscaled_pic;
	} else {
		res->lock_sequence_nr = _state->options->buffer_pics_nr; // Update lock counter
	}

	must_post_process_pic = res->scaled_data.pic->visual_map->data == NULL;
	// If the pic was only just drawn, we'll have to endianness-adjust it now

	npic = gfxr_pic_xlate_common(res, maps, scaled || _state->options->pic0_unscaled, 0, _state->driver->mode,
	                             _state->options->pic_xlate_filter, 0, _state->options);


	if (must_post_process_pic) {
		gfxr_endianness_adjust(npic->visual_map, _state->driver->mode);
	}

	return npic;
}

static void set_pic_id(gfx_resource_t *res, int id) {
	if (res->scaled_data.pic) {
		gfxr_pic_t *pic = res->scaled_data.pic;
		pic->control_map->ID = id;
		pic->priority_map->ID = id;
		pic->visual_map->ID = id;
	}

	if (res->unscaled_data.pic) {
		gfxr_pic_t *pic = res->unscaled_data.pic;
		pic->control_map->ID = id;
		pic->priority_map->ID = id;
		pic->visual_map->ID = id;
	}
}

static int get_pic_id(gfx_resource_t *res) {
	if (res->scaled_data.pic)
		return res->scaled_data.pic->visual_map->ID;
	else
		return res->unscaled_data.pic->visual_map->ID;
}

static void _gfxr_unscale_pixmap_index_data(gfx_pixmap_t *pxm, gfx_mode_t *mode) {
	int xmod = mode->xfact; // Step size horizontally
	int ymod = pxm->index_width * mode->yfact; // Vertical step size
	int maxpos = pxm->index_width * pxm->index_height;
	int pos;
	byte *dest = pxm->index_data;

	if (!(pxm->flags & GFX_PIXMAP_FLAG_SCALED_INDEX))
		return; // It's not scaled!

	for (pos = 0; pos < maxpos; pos += ymod) {
		int c;

		for (c = 0; c < pxm->index_width; c += xmod)
			*dest++ = pxm->index_data[pos + c];
			// No overwrite since line and offset readers move much faster (proof by in-duction, trivial
			// and left to the reader)
	}

	pxm->index_width /= mode->xfact;
	pxm->index_height /= mode->yfact;
	pxm->flags &= ~GFX_PIXMAP_FLAG_SCALED_INDEX;
}

gfxr_pic_t *GfxResManager::addToPic(int old_nr, int new_nr, int flags, int old_default_palette, int default_palette) {
	IntResMap &resMap = _state->_resourceMaps[GFX_RESOURCE_TYPE_PIC];
	gfxr_pic_t *pic = NULL;
	gfx_resource_t *res = NULL;
	int hash = getOptionsHash(GFX_RESOURCE_TYPE_PIC);
	int need_unscaled = !(_state->options->pic0_unscaled) && (_state->driver->mode->xfact != 1 || _state->driver->mode->yfact != 1);

	res = resMap.contains(old_nr) ? resMap[old_nr] : NULL;

	if (!res || (res->mode != MODE_INVALID && res->mode != hash)) {
		// FIXME: the initialization of the GFX resource manager should
		// be pushed up, and it shouldn't occur here
		GfxResManager *_gfx = new GfxResManager(_state);
		_gfx->getPic(old_nr, 0, flags, old_default_palette, 1);
		delete _gfx;

		res = resMap.contains(old_nr) ? resMap[old_nr] : NULL;

		if (!res) {
			GFXWARN("Attempt to add pic %d to non-existing pic %d\n", new_nr, old_nr);
			return NULL;
		}
	}

	if (_state->options->pic0_unscaled) // Unscale priority map, if we scaled it earlier
		_gfxr_unscale_pixmap_index_data(res->scaled_data.pic->priority_map, _state->driver->mode);

	// The following two operations are needed when returning scaled maps (which is always the case here)
	res->lock_sequence_nr = _state->options->buffer_pics_nr;
	calculatePic(_state, res->scaled_data.pic, need_unscaled ? res->unscaled_data.pic : NULL,
		                               flags | DRAWPIC01_FLAG_OVERLAID_PIC, default_palette, new_nr);

	res->mode = MODE_INVALID; // Invalidate

	if (_state->options->pic0_unscaled) // Scale priority map again, if needed
		res->scaled_data.pic->priority_map = gfx_pixmap_scale_index_data(res->scaled_data.pic->priority_map, _state->driver->mode);
	{
		int old_ID = get_pic_id(res);
		set_pic_id(res, GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, new_nr)); // To ensure that our graphical translation optoins work properly
		pic = gfxr_pic_xlate_common(res, GFX_MASK_VISUAL, 1, 1, _state->driver->mode, _state->options->pic_xlate_filter, 1, _state->options);
		set_pic_id(res, old_ID);
	}

	return pic;
}

gfxr_view_t *gfxr_draw_view11(int id, byte *resource, int size);

gfxr_view_t *GfxResManager::getView(int nr, int *loop, int *cel, int palette) {
	IntResMap &resMap = _state->_resourceMaps[GFX_RESOURCE_TYPE_VIEW];
	gfx_resource_t *res = NULL;
	int hash = palette;
	gfxr_view_t *view = NULL;
	gfxr_loop_t *loop_data = NULL;
	gfx_pixmap_t *cel_data = NULL;

	res = resMap.contains(nr) ? resMap[nr] : NULL;

	if (!res || res->mode != hash) {
		Resource *viewRes = _state->resManager->findResource(kResourceTypeView, nr, 0);
		if (!viewRes || !viewRes->data)
			return NULL;

		int resid = GFXR_RES_ID(GFX_RESOURCE_TYPE_VIEW, nr);

		if (_state->version < SCI_VERSION_01)
			view = gfxr_draw_view0(resid, viewRes->data, viewRes->size, -1);
		else if (_state->version == SCI_VERSION_01)
			view = gfxr_draw_view0(resid, viewRes->data, viewRes->size, palette);
		else if (_state->version >= SCI_VERSION_01_VGA && _state->version <= SCI_VERSION_1_LATE)
			view = gfxr_draw_view1(resid, viewRes->data, viewRes->size, _state->static_palette);
		else if (_state->version >= SCI_VERSION_1_1)
			view = gfxr_draw_view11(resid, viewRes->data, viewRes->size);

		if (_state->version >= SCI_VERSION_01_VGA) {
			if (!view->palette) {
				view->palette = new Palette(_state->static_palette->size());
				view->palette->name = "interpreter_get_view";
			}
			
			// Palettize view
			for (unsigned i = 0; i < MIN(view->palette->size(), _state->static_palette->size()); i++) {
				const PaletteEntry& vc = view->palette->getColor(i);
				if (vc.r == 0 && vc.g == 0 && vc.b == 0) {
					const PaletteEntry& sc = _state->static_palette->getColor(i);
					view->palette->setColor(i, sc.r, sc.g, sc.b);
				}
			}

		}

		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->scaled_data.view = NULL;
			res->ID = GFXR_RES_ID(GFX_RESOURCE_TYPE_VIEW, nr);
			res->lock_sequence_nr = _state->tag_lock_counter;
			res->mode = hash;
			resMap[nr] = res;
		} else {
			gfxr_free_view(res->unscaled_data.view);
		}

		res->mode = hash;
		res->unscaled_data.view = view;

	} else {
		res->lock_sequence_nr = _state->tag_lock_counter; // Update lock counter
		view = res->unscaled_data.view;
	}

	if (*loop < 0)
		*loop = 0;
	else
		if (*loop >= view->loops_nr)
			*loop = view->loops_nr - 1;

	if (*loop < 0) {
		GFXWARN("View %d has no loops\n", nr);
		return NULL;
	}

	loop_data = view->loops + (*loop);
	if (loop_data == NULL) {
		GFXWARN("Trying to load invalid loop %d of view %d\n", *loop, nr);
		return NULL;
	}

	if (*cel < 0) {
		sciprintf("Resetting cel! %d\n", *cel);
		*cel = 0;
	} else
		if (*cel >= loop_data->cels_nr)
			*cel = loop_data->cels_nr - 1;

	if (*cel < 0) {
		GFXWARN("View %d loop %d has no cels\n", nr, *loop);
		return NULL;
	}

	cel_data = loop_data->cels[*cel];
	if (loop_data == NULL) {
		GFXWARN("Trying to load invalid view/loop/cel %d/%d/%d\n", nr, *loop, *cel);
		return NULL;
	}

	if (!cel_data->data) {
		gfx_get_res_config(_state->options, cel_data);
		gfx_xlate_pixmap(cel_data, _state->driver->mode, _state->options->view_xlate_filter);
		gfxr_endianness_adjust(cel_data, _state->driver->mode);
	}

	return view;
}

gfx_bitmap_font_t *GfxResManager::getFont(int num, bool scaled) {
	IntResMap &resMap = _state->_resourceMaps[GFX_RESOURCE_TYPE_FONT];
	gfx_resource_t *res = NULL;
	int hash = getOptionsHash(GFX_RESOURCE_TYPE_FONT);

	res = resMap.contains(num) ? resMap[num] : NULL;

	if (!res || res->mode != hash) {
		Resource *fontRes = _state->resManager->findResource(kResourceTypeFont, num, 0);
		if (!fontRes || !fontRes->data)
			return NULL;

		gfx_bitmap_font_t *font = gfxr_read_font(fontRes->id, fontRes->data, fontRes->size);

		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->scaled_data.font = NULL;
			res->ID = GFXR_RES_ID(GFX_RESOURCE_TYPE_FONT, num);
			res->lock_sequence_nr = _state->tag_lock_counter;
			res->mode = hash;
			resMap[num] = res;
		} else {
			gfxr_free_font(res->unscaled_data.font);
		}

		res->unscaled_data.font = font;

		return font;
	} else {
		res->lock_sequence_nr = _state->tag_lock_counter; // Update lock counter
		if (res->unscaled_data.pointer)
			return res->unscaled_data.font;
		else
			return res->scaled_data.font;
	}
}

gfx_pixmap_t *GfxResManager::getCursor(int num) {
	IntResMap &resMap = _state->_resourceMaps[GFX_RESOURCE_TYPE_CURSOR];
	gfx_resource_t *res = NULL;
	int hash = getOptionsHash(GFX_RESOURCE_TYPE_CURSOR);

	res = resMap.contains(num) ? resMap[num] : NULL;

	if (!res || res->mode != hash) {
		Resource *cursorRes = _state->resManager->findResource(kResourceTypeCursor, num, 0);
		if (!cursorRes || !cursorRes->data)
			return NULL;

		if (_state->version >= SCI_VERSION_1_1) {
			GFXWARN("Attempt to retrieve cursor in SCI1.1 or later\n");
			return NULL;
		}

		gfx_pixmap_t *cursor = gfxr_draw_cursor(GFXR_RES_ID(GFX_RESOURCE_TYPE_CURSOR, num), 
										cursorRes->data, cursorRes->size, _state->version != SCI_VERSION_0);

		if (!cursor)
			return NULL;

		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->scaled_data.pointer = NULL;
			res->ID = GFXR_RES_ID(GFX_RESOURCE_TYPE_CURSOR, num);
			res->lock_sequence_nr = _state->tag_lock_counter;
			res->mode = hash;
			resMap[num] = res;
		} else {
			gfx_free_pixmap(res->unscaled_data.pointer);
		}
		gfx_get_res_config(_state->options, cursor);
		gfx_xlate_pixmap(cursor, _state->driver->mode, _state->options->cursor_xlate_filter);
		gfxr_endianness_adjust(cursor, _state->driver->mode);

		res->unscaled_data.pointer = cursor;

		return cursor;
	} else {
		res->lock_sequence_nr = _state->tag_lock_counter; // Update lock counter
		return res->unscaled_data.pointer;
	}
}

} // End of namespace Sci
