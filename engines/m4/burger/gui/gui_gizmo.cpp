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

#include "m4/burger/gui/gui_gizmo.h"
#include "m4/burger/vars.h"
#include "m4/core/errors.h"
#include "m4/adv_r/adv_background.h"
#include "m4/adv_r/adv_control.h"
#include "m4/graphics/gr_sprite.h"
#include "m4/gui/gui_event.h"
#include "m4/mem/mem.h"
#include "m4/platform/keys.h"

namespace M4 {
namespace Burger {
namespace GUI {

static void gizmo_dispose_gui();
static void gizmo_restore_interface(bool fade);
static void gizmo_free_gui(ScreenContext *screenContext);
static bool gizmo_load_sprites(const char *name, size_t count);
static void gizmo_free_sprites();
static void gizmo_draw_sprite(M4sprite *sprite, Buffer *dest, int destX, int destY);
static ScreenContext *gui_create_gizmo(M4sprite *sprite, int sx, int sy, uint scrnFlags);

void gizmo_anim(RGB8 *pal) {
	if (!_GIZMO(initialized))
		gizmo_initialize(pal);

	if (gizmo_load_sprites("500gizmo", 58)) {
		_GIZMO(gui) = gui_create_gizmo(_GIZMO(sprites)[0], 0, 0, 505);

		if (_GIZMO(gui)) {
			// TODO
		}
	}
}

void gizmo_initialize(RGB8 *pal) {
	if (!_GIZMO(initialized)) {
		_GIZMO(initialized) = true;

		// Remember whether interface was visible when gizmo was initialized
		if (INTERFACE_VISIBLE) {
			_GIZMO(interface_visible) = true;
			interface_hide();
		} else {
			_GIZMO(interface_visible) = false;
		}

		_GIZMO(palette) = pal;
		krn_fade_to_grey(pal, 5, 1);
	}
}

void gizmo_shutdown(void *, void *) {
	gizmo_dispose_gui();
	gizmo_restore_interface(true);
}

static void gizmo_restore_interface(bool fade) {
	if (_GIZMO(initialized)) {
		_GIZMO(val1) = 0;

		if (_GIZMO(lowMemory2)) {
			if (!adv_restoreBackground())
				error_show(FL, 0, "unable to restore background");
		}

		if (_GIZMO(lowMemory1)) {
			if (!adv_restoreCodes())
				error_show(FL, 0, "unable to restore screen codes");
		}

		krn_fade_from_grey(_GIZMO(palette), 5, 1, fade ? 1 : 2);

		if (_GIZMO(interface_visible))
			interface_show();

		mouse_unlock_sprite();
		game_pause(false);
		_GIZMO(initialized) = false;
	}
}

static void gizmo_dispose_gui() {
	if (_GIZMO(gui)) {
		vmng_screen_dispose(_GIZMO(gui));
		gizmo_free_gui(_GIZMO(gui));
		gizmo_free_sprites();
		_GIZMO(gui) = 0;
	}
}

static void gizmo_free_gui(ScreenContext *screenContext) {
	// TODO
}

static bool gizmo_load_sprites(const char *name, size_t count) {
	if (LoadSpriteSeries(name, &_GIZMO(seriesHandle), &_GIZMO(celsOffset),
			&_GIZMO(palOffset), _GIZMO(palette)) > 0) {
		gr_pal_set_range(_GIZMO(palette), 64, 192);
		_GIZMO(assetName) = mem_strdup(name);

		_GIZMO(spriteCount) = count;
		_GIZMO(sprites) = (M4sprite **)mem_alloc(count * sizeof(M4sprite *), "*sprites array");

		for (size_t idx = 0; idx < count; ++idx) {
			_GIZMO(sprites)[idx] = CreateSprite(_GIZMO(seriesHandle), _GIZMO(celsOffset),
				idx, nullptr, nullptr);
			if (!_GIZMO(sprites)[idx])
				return false;
		}

		return true;
	} else {
		return false;
	}
}

static void gizmo_free_sprites() {
	if (_GIZMO(assetName)) {
		rtoss(_GIZMO(assetName));
		mem_free(_GIZMO(assetName));

		_GIZMO(assetName) = nullptr;
		_GIZMO(seriesHandle) = 0;
		_GIZMO(celsOffset) = -1;
		_GIZMO(palOffset) = -1;

		for (int idx = 0; idx < _GIZMO(spriteCount); ++idx)
			mem_free(_GIZMO(sprites)[idx]);

		mem_free(_GIZMO(sprites));
		_GIZMO(sprites) = nullptr;
		_GIZMO(spriteCount) = 0;
	}
}

void gizmo_draw_sprite(M4sprite *sprite, Buffer *dest, int destX, int destY) {
	Buffer src;
	DrawRequest dr;

	if (sprite && dest) {
		HLock(sprite->sourceHandle);
		sprite->data = (uint8 *)((intptr)*sprite->sourceHandle + sprite->sourceOffset);
		src.stride = src.w = sprite->w;
		src.h = sprite->h;
		src.encoding = sprite->encoding & 0x7f;
		src.data = sprite->data;

		dr.Src = &src;
		dr.Dest = dest;
		dr.x = destX;
		dr.y = destY;
		dr.scaleX = dr.scaleY = 100;
		dr.srcDepth = 0;
		dr.depthCode = nullptr;
		dr.Pal = nullptr;
		dr.ICT = nullptr;
		gr_sprite_draw(&dr);
		HUnLock(sprite->sourceHandle);
	}
}

static void gui_gizmo_show(ScreenContext *s, RectList *r, Buffer *dest, int32 destX, int32 destY) {
	if (!s)
		return;
	void *scrnContent = s->scrnContent;
	if (!scrnContent)
		return;

	GrBuff *buf = (GrBuff *)s->scrnContent;
	if (!buf)
		return;
	Buffer *src = buf->get_buffer();
	if (!src)
		return;

	if (dest) {
		for (RectList *rect = r; rect; rect = rect->next) {
			vmng_refresh_video(rect->x1, rect->y1,
				rect->x1 - s->x1, rect->y1 - s->y1, rect->x2 - s->x1, rect->y2 - s->y1,
				src);
		}
	} else {
		for (RectList *rect = r; rect; rect = rect->next) {
			gr_buffer_rect_copy_2(src, dest,
				rect->x1 - s->x1, rect->y1 - s->y1, destX, destY,
				rect->x2 - rect->x1 + 1, rect->y2 - rect->y1 + 1);
		}
	}
}

static bool gui_gizmo_eventHandler(void *s, int32 eventType, int32 event, int32 x, int32 y, bool *z) {
	ScreenContext *srcBuffer = (ScreenContext *)s;
	*z = false;
	/*
	int32 status = 0;
	ScreenContext *ctx = vmng_screen_find(s, &status);
	if (!ctx || status != 1)
		return false;

	if (eventType == EVENT_KEY && event == KEY_ESCAPE && myScreen->y1 != 0) {
		_GIZMO(val1) = 0;

	}
	*/
	// TODO: event handler
	return false;
}

static ScreenContext *gui_create_gizmo(M4sprite *sprite, int sx, int sy, uint scrnFlags) {
	if (!sprite)
		return nullptr;

	Gizmo *gui = (Gizmo *)mem_alloc(sizeof(Gizmo), "*gui gizmo");
	if (!gui)
		return nullptr;

	GrBuff *grBuff = new GrBuff(sprite->w, sprite->h);
	gui->_grBuff = grBuff;
	gui->_eventHandler = gui_gizmo_eventHandler;

	Buffer *dest = gui->_grBuff->get_buffer();
	Buffer *src = _G(gameDrawBuff)->get_buffer();

	if ((_G(gameDrawBuff)->h - sy) >= dest->h) {
		gr_buffer_rect_copy_2(src, dest, sx, sy, 0, 0, dest->w, dest->h);
	} else {
		gr_buffer_rect_copy_2(src, dest, sx, sy, 0, 0, dest->w, _G(gameDrawBuff)->h - sy);
	}

	_G(gameDrawBuff)->release();
	if (sprite->sourceHandle)
		gizmo_draw_sprite(sprite, dest, 0, 0);

	gui->_grBuff->release();

	return vmng_screen_create(sx, sy, sx + sprite->w, sy + sprite->h,
		69, scrnFlags, gui, (RefreshFunc)gui_gizmo_show, gui_gizmo_eventHandler);
}

} // namespace GUI
} // namespace Burger
} // namespace M4
