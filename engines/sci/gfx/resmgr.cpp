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

void gfxr_free_all_resources(gfx_resstate_t *state) {
	for (int type = 0; type < GFX_RESOURCE_TYPES_NR; ++type) {
		for (IntResMap::iterator iter = state->_resourceMaps[type].begin(); iter != state->_resourceMaps[type].end(); ++iter) {
			gfxr_free_resource(iter->_value, type);
			iter->_value = 0;
		}
	}
}

void gfxr_free_resource_manager(gfx_resstate_t *state) {
	gfxr_free_all_resources(state);
	delete state;
}

void gfxr_free_tagged_resources(gfx_resstate_t *state) {
	// Current heuristics: free tagged views and old pics

	IntResMap::iterator iter;
	int type;
	const int tmp = state->tag_lock_counter;

	type = GFX_RESOURCE_TYPE_VIEW;
	for (iter = state->_resourceMaps[type].begin(); iter != state->_resourceMaps[type].end(); ++iter) {
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
	for (iter = state->_resourceMaps[type].begin(); iter != state->_resourceMaps[type].end(); ++iter) {
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

	state->tag_lock_counter = 0;
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
	gfx_resource_type_t restype = GFX_RESOURCE_TYPE_PIC;
	IntResMap &resMap = _state->_resourceMaps[restype];
	gfx_resource_t *res = NULL;
	int hash = gfxr_interpreter_options_hash(restype, _state->version, _state->options, 0);
	int must_post_process_pic = 0;
	int need_unscaled = (_state->driver->mode->xfact != 1 || _state->driver->mode->yfact != 1);

	hash |= (flags << 20) | ((default_palette & 0x7) << 28);

	res = resMap.contains(num) ? resMap[num] : NULL;

	if (!res || res->mode != hash) {
		gfxr_pic_t *pic;
		gfxr_pic_t *unscaled_pic = NULL;

		if (_state->options->pic0_unscaled) {
			need_unscaled = 0;
			pic = gfxr_init_pic(&mode_1x1_color_index, GFXR_RES_ID(restype, num), _state->version >= SCI_VERSION_01_VGA);
		} else
			pic = gfxr_init_pic(_state->driver->mode, GFXR_RES_ID(restype, num), _state->version >= SCI_VERSION_01_VGA);
		if (!pic) {
			GFXERROR("Failed to allocate scaled pic!\n");
			return NULL;
		}

		gfxr_clear_pic0(pic, SCI_TITLEBAR_SIZE);

		if (need_unscaled) {
			unscaled_pic = gfxr_init_pic(&mode_1x1_color_index, GFXR_RES_ID(restype, num), _state->version >= SCI_VERSION_01_VGA);
			if (!unscaled_pic) {
				GFXERROR("Failed to allocate unscaled pic!\n");
				return NULL;
			}
			gfxr_clear_pic0(pic, SCI_TITLEBAR_SIZE);
		}
		if (gfxr_interpreter_calculate_pic(_state, pic, unscaled_pic, flags, default_palette, num)) {
			gfxr_free_pic(pic);
			if (unscaled_pic)
				gfxr_free_pic(unscaled_pic);
			return NULL;
		}
		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->ID = GFXR_RES_ID(restype, num);
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

gfxr_pic_t *gfxr_add_to_pic(gfx_resstate_t *state, int old_nr, int new_nr, int maps, int flags,
							int old_default_palette, int default_palette, int scaled) {
	gfx_resource_type_t restype = GFX_RESOURCE_TYPE_PIC;
	IntResMap &resMap = state->_resourceMaps[restype];
	gfxr_pic_t *pic = NULL;
	gfx_resource_t *res = NULL;
	int hash = gfxr_interpreter_options_hash(restype, state->version, state->options, 0);
	int need_unscaled = !(state->options->pic0_unscaled) && (state->driver->mode->xfact != 1 || state->driver->mode->yfact != 1);

	res = resMap.contains(old_nr) ? resMap[old_nr] : NULL;

	if (!res || (res->mode != MODE_INVALID && res->mode != hash)) {
		// FIXME: the initialization of the GFX resource manager should
		// be pushed up, and it shouldn't occur here
		GfxResManager *_gfx = new GfxResManager(state);
		_gfx->getPic(old_nr, 0, flags, old_default_palette, scaled);
		delete _gfx;

		res = resMap.contains(old_nr) ? resMap[old_nr] : NULL;

		if (!res) {
			GFXWARN("Attempt to add pic %d to non-existing pic %d\n", new_nr, old_nr);
			return NULL;
		}
	}

	if (state->options->pic0_unscaled) // Unscale priority map, if we scaled it earlier
		_gfxr_unscale_pixmap_index_data(res->scaled_data.pic->priority_map, state->driver->mode);

	if (scaled) {
		res->lock_sequence_nr = state->options->buffer_pics_nr;

		gfxr_interpreter_calculate_pic(state, res->scaled_data.pic, need_unscaled ? res->unscaled_data.pic : NULL,
		                               flags | DRAWPIC01_FLAG_OVERLAID_PIC, default_palette, new_nr);
	}

	res->mode = MODE_INVALID; // Invalidate

	if (state->options->pic0_unscaled) // Scale priority map again, if needed
		res->scaled_data.pic->priority_map = gfx_pixmap_scale_index_data(res->scaled_data.pic->priority_map, state->driver->mode);
	{
		int old_ID = get_pic_id(res);
		set_pic_id(res, GFXR_RES_ID(restype, new_nr)); // To ensure that our graphical translation optoins work properly
		pic = gfxr_pic_xlate_common(res, maps, scaled, 1, state->driver->mode, state->options->pic_xlate_filter, 1, state->options);
		set_pic_id(res, old_ID);
	}

	return pic;
}


gfxr_view_t *gfxr_get_view(gfx_resstate_t *state, int nr, int *loop, int *cel, int palette) {
	gfx_resource_type_t restype = GFX_RESOURCE_TYPE_VIEW;
	IntResMap &resMap = state->_resourceMaps[restype];
	gfx_resource_t *res = NULL;
	int hash = gfxr_interpreter_options_hash(restype, state->version, state->options, palette);
	gfxr_view_t *view = NULL;
	gfxr_loop_t *loop_data = NULL;
	gfx_pixmap_t *cel_data = NULL;

	res = resMap.contains(nr) ? resMap[nr] : NULL;

	if (!res || res->mode != hash) {
		view = gfxr_interpreter_get_view(*(state->resManager), nr, palette, state->static_palette, state->version);

		if (!view)
			return NULL;

		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->scaled_data.view = NULL;
			res->ID = GFXR_RES_ID(restype, nr);
			res->lock_sequence_nr = state->tag_lock_counter;
			res->mode = hash;
			resMap[nr] = res;
		} else {
			gfxr_free_view(res->unscaled_data.view);
		}

		res->mode = hash;
		res->unscaled_data.view = view;

	} else {
		res->lock_sequence_nr = state->tag_lock_counter; // Update lock counter
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
		gfx_get_res_config(state->options, cel_data);
		gfx_xlate_pixmap(cel_data, state->driver->mode, state->options->view_xlate_filter);
		gfxr_endianness_adjust(cel_data, state->driver->mode);
	}

	return view;
}

gfx_bitmap_font_t *GfxResManager::getFont(int num, bool scaled) {
	gfx_resource_type_t restype = GFX_RESOURCE_TYPE_FONT;
	IntResMap &resMap = _state->_resourceMaps[restype];
	gfx_resource_t *res = NULL;
	int hash;

	hash = gfxr_interpreter_options_hash(restype, _state->version, _state->options, 0);

	res = resMap.contains(num) ? resMap[num] : NULL;

	if (!res || res->mode != hash) {
		Resource *fontRes = _state->resManager->findResource(kResourceTypeFont, num, 0);
		if (!fontRes || !fontRes->data)
			return NULL;

		gfx_bitmap_font_t *font = gfxr_read_font(fontRes->id, fontRes->data, fontRes->size);

		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->scaled_data.font = NULL;
			res->ID = GFXR_RES_ID(restype, num);
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
	gfx_resource_type_t restype = GFX_RESOURCE_TYPE_CURSOR;
	IntResMap &resMap = _state->_resourceMaps[restype];
	gfx_resource_t *res = NULL;
	int hash = gfxr_interpreter_options_hash(restype, _state->version, _state->options, 0);

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
			res->ID = GFXR_RES_ID(restype, num);
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
