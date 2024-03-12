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
#include "m4/core/imath.h"
#include "m4/adv_r/adv_background.h"
#include "m4/adv_r/adv_control.h"
#include "m4/graphics/gr_sprite.h"
#include "m4/gui/gui_event.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/mem/mem.h"
#include "m4/platform/keys.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace GUI {

static void gizmo_dispose_gui();
static void gizmo_restore_interface(bool fade);
static void gizmo_free_gui(Gizmo *gizmo);
static bool gizmo_load_sprites(const char *name, size_t count);
static void gizmo_free_sprites();
static void gizmo_draw_sprite(M4sprite *sprite, Buffer *dest, int destX, int destY);
static Gizmo *gui_create_gizmo(M4sprite *sprite, int sx, int sy, uint scrnFlags);
static void gizmo_digi_daemon(int trigger);
static void gizmo_daemon(int trigger);

Gizmo_Globals::Gizmo_Globals() {
	Common::fill(_roomFlags, _roomFlags + 15, false);
}

Gizmo_Globals::~Gizmo_Globals() {
	mem_free(_gui);
}

static void gizmo_sound() {
	digi_read_another_chunk();
	midi_loop();

	g_system->updateScreen();
	g_system->delayMillis(10);

	g_events->process();
	gui_system_event_handler();
}

void gizmo_digi_play(const char *name, int vol, bool &done) {
	if (!done) {
		done = true;
		digi_play(name, 2, vol);
		digi_read_another_chunk();
		player_set_commands_allowed(false);

		while (!g_engine->shouldQuit() && digi_play_state(2))
			gizmo_sound();

		player_set_commands_allowed(true);
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

static void gizmo_digi_daemon(int trigger) {
	switch (trigger) {
	case 5000:
		if (player_been_here(503)) {
			if (_G(flags)[kBORK_STATE] == 16)
				gizmo_digi_play("510w005", 255, _GIZMO(roomFlags)[3]);
			else
				gizmo_digi_play("510w004", 255, _GIZMO(roomFlags)[2]);
		}
		break;

	case 5001:
		if (player_been_here(507)) {
			if (_G(flags)[V223] == 2)
				gizmo_digi_play("510w008", 255, _GIZMO(roomFlags)[5]);
			else
				gizmo_digi_play("510w007", 255, _GIZMO(roomFlags)[4]);
		}
		break;

	case 5002:
		if (player_been_here(504)) {
			if (_G(flags)[V210] == 5002)
				gizmo_digi_play("510w011", 255, _GIZMO(roomFlags)[7]);
			else
				gizmo_digi_play("510w010", 255, _GIZMO(roomFlags)[6]);
		}
		break;

	case 5003:
		if (player_been_here(508)) {
			if (_G(flags)[V227] != 0)
				gizmo_digi_play("510w014", 255, _GIZMO(roomFlags)[9]);
			else
				gizmo_digi_play("510w013", 255, _GIZMO(roomFlags)[8]);
		}
		break;

	case 5004:
		if (player_been_here(506)) {
			if (_G(flags)[V218] == 5003)
				gizmo_digi_play("510w017", 255, _GIZMO(roomFlags)[11]);
			else
				gizmo_digi_play("510w016", 255, _GIZMO(roomFlags)[10]);
		}
		break;

	case 5005:
		if (_G(flags)[kStairsBorkState] == 5003)
			gizmo_digi_play("510w019", 255, _GIZMO(roomFlags)[13]);
		else
			gizmo_digi_play("510w018", 255, _GIZMO(roomFlags)[12]);
		break;

	case 5006:
		gizmo_digi_play("510w020", 255, _GIZMO(roomFlags)[14]);
		break;

	default:
		break;
	}
}

static void gizmo_restore_sprite(int spriteIndex) {
	if (!_GIZMO(gui))
		return;

	Buffer *dest = _GIZMO(gui)->_grBuff->get_buffer();
	if (!dest)
		return;

	if (spriteIndex >= 22)
		gizmo_draw_sprite(_GIZMO(sprites)[spriteIndex], dest, 270, 38);

	int32 status = 0;
	ScreenContext *ctx = vmng_screen_find(_GIZMO(gui), &status);

	if (ctx && status == 1)
		RestoreScreens(270, 38, 381, 93);
}

static void gizmo_digi_wait(int spriteIndex1, int spriteIndex2) {
	player_set_commands_allowed(false);
	digi_read_another_chunk();

	int spriteNum = spriteIndex1;
	while (digi_play_state(2)) {
		// Cycle displayed sprite
		gizmo_restore_sprite(spriteNum);
		spriteNum = (spriteNum == spriteIndex2) ? spriteIndex1 : spriteNum + 1;

		uint32 timer = timer_read_60();

		while (!g_engine->shouldQuit() && (timer_read_60() - timer) < 6)
			gizmo_sound();
	}

	gizmo_restore_sprite(57);
	player_set_commands_allowed(true);
}

static void gizmo_daemon(int trigger) {
	switch (trigger) {
	case 5000:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("510b001a", 2, 255, -1);
			break;
		case 2:
			digi_play("510b001b", 2, 255, -1);
			break;
		default:
			digi_play("510b001c", 2, 255, -1);
			break;
		}

		gizmo_digi_wait(32, 36);
		break;

	case 5001:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("510b002a", 2, 255, -1);
			break;
		case 2:
			digi_play("510b002b", 2, 255, -1);
			break;
		default:
			digi_play("510b002c", 2, 255, -1);
			break;
		}

		gizmo_digi_wait(27, 31);
		break;

	case 5002:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("510b003a", 2, 255, -1);
			break;
		case 2:
			digi_play("510b003b", 2, 255, -1);
			break;
		default:
			digi_play("510b003c", 2, 255, -1);
			break;
		}

		gizmo_digi_wait(32, 36);
		break;

	case 5003:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			digi_play("510b004a", 2, 255, -1);
			break;
		case 2:
			digi_play("510b004b", 2, 255, -1);
			break;
		default:
			break;
		}

		gizmo_digi_wait(37, 41);
		break;

	case 5004:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("510b005a", 2, 255, -1);
			break;
		case 2:
			digi_play("510b005b", 2, 255, -1);
			break;
		default:
			digi_play("510b005c", 2, 255, -1);
			break;
		}

		gizmo_digi_wait(42, 46);
		break;

	case 5005:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("510b006a", 2, 255, -1);
			break;
		case 2:
			digi_play("510b006b", 2, 255, -1);
			break;
		default:
			digi_play("510b006c", 2, 255, -1);
			break;
		}

		gizmo_digi_wait(47, 51);
		break;

	case 5006:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("510b007a", 2, 255, -1);
			break;
		case 2:
			digi_play("510b007b", 2, 255, -1);
			break;
		default:
			digi_play("510b007c", 2, 255, -1);
			break;
		}

		gizmo_digi_wait(52, 56);
		break;

	default:
		break;
	}

	gizmo_digi_daemon(trigger);
}

static void gizmo_restore_interface(bool fade) {
	if (_GIZMO(initialized)) {
		_GIZMO(currentItem) = 0;

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

static void gizmo_free_gui(Gizmo *gizmo) {
	if (!gizmo)
		return;

	for (GizmoItem *item = gizmo->_items; item; item = gizmo->_items) {
		gizmo->_items = item->_next;
		(*item->_fnFree)(item);
	}

	GrBuff *grBuff = gizmo->_grBuff;
	if (grBuff)
		delete grBuff;

	mem_free(gizmo);
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

static void gizmo_draw(ScreenContext *s, RectList *r, Buffer *dest, int32 destX, int32 destY) {
	if (!s)
		return;
	void *scrnContent = s->scrnContent;
	if (!scrnContent)
		return;

	Gizmo *gizmo = (Gizmo *)s->scrnContent;
	if (!gizmo)
		return;
	Buffer *src = gizmo->_grBuff->get_buffer();
	if (!src)
		return;

	if (dest) {
		for (RectList *rect = r; rect; rect = rect->next) {
			gr_buffer_rect_copy_2(src, dest,
				rect->x1 - s->x1, rect->y1 - s->y1, destX, destY,
				rect->x2 - rect->x1 + 1, rect->y2 - rect->y1 + 1);
		}
	} else {
		for (RectList *rect = r; rect; rect = rect->next) {
			vmng_refresh_video(rect->x1, rect->y1,
				rect->x1 - s->x1, rect->y1 - s->y1, rect->x2 - s->x1, rect->y2 - s->y1,
				src);
		}
	}
}

static bool gizmo_eventHandler(void *s, int32 eventType, int32 event, int32 x, int32 y, bool *z) {
	Gizmo *gizmo = (Gizmo *)s;
	bool flag = false;
	int32 status = 0;
	ScreenContext *ctx = vmng_screen_find(s, &status);

	// WORKAROUND: gui_system_event_handler may call without z being set
	bool dummyZ = false;
	if (!z)
		z = &dummyZ;
	*z = false;
	
	if (!ctx || status != 1)
		return false;

	if (eventType == EVENT_KEY) {
		if (event == KEY_ESCAPE && gizmo->_fnEscape) {
			_GIZMO(currentItem) = nullptr;
			(*gizmo->_fnEscape)();
			return true;
		} else if (event == KEY_RETURN && gizmo->_fnEnter) {
			_GIZMO(currentItem) = nullptr;
			(*gizmo->_fnEnter)();
			return true;
		}
	}

	int xs = x + ctx->x1;
	int ys = y + ctx->y1;

	if (_GIZMO(currentItem)) {
		flag = (*_GIZMO(currentItem)->_fnEvents)(_GIZMO(currentItem),
			eventType, event, xs, ys, &_GIZMO(currentItem));

		if (_GIZMO(currentItem))
			*z = true;

		if (flag)
			return true;
	}

	if (eventType == EVENT_MOUSE) {
		GizmoItem *item;
		for (item = gizmo->_items; item; item = item->_next) {
			if (item->_btnRect.contains(xs, ys))
				break;
		}

		if (item && item->_fnEvents) {
			item->_fnEvents(item, eventType, event, xs, ys, &_GIZMO(currentItem));

			if (_GIZMO(currentItem))
				*z = true;

			return true;
		}

	} else if (eventType == EVENT_KEY) {
		GizmoItem *item;
		for (item = gizmo->_items; item && !flag; item = item->_next) {
			if (item->_fnEvents)
				flag = (*item->_fnEvents)(item, eventType, event, -1, -1, nullptr);
		}

		return flag;
	}

	switch (event) {
	case 2:
	case 14:
		if (!(ctx->scrnFlags & SF_IMMOVABLE)) {
			*z = true;
			_GIZMO(savedFlag) = true;
			_GIZMO(savedX) = x;
			_GIZMO(savedY) = y;
		}

		if (xs < 230 || xs > 420 || ys > 374)
			gizmo_shutdown();
		break;

	case 4:
	case 16:
		if (_GIZMO(savedFlag)) {
			MoveScreenDelta(ctx, _GIZMO(savedX), _GIZMO(savedY));
			_GIZMO(savedX) = x;
			_GIZMO(savedY) = y;
		}
		break;

	case 5:
	case 17:
		*z = false;
		_GIZMO(savedFlag) = false;
		break;

	default:
		break;
	}

	return true;
}

static void gizmo_item_draw(GizmoItem *item, Gizmo *gizmo, int x, int y) {
	if (!item || !item->_button || !gizmo)
		return;

	Buffer *src = nullptr;
	if (item->_hasBuffer) {
		if (!item->_grBuff)
			return;

		src = item->_grBuff->get_buffer();
		if (!src)
			return;
	}

	Buffer *dest = gizmo->_grBuff->get_buffer();
	if (!dest)
		return;

	GizmoButton *btn = item->_button;
	M4sprite *sprite = nullptr;

	switch (btn->_state) {
	case IN_CONTROL:
		sprite = _GIZMO(sprites)[15 + btn->_index];
		break;
	case OVER_CONTROL:
		sprite = _GIZMO(sprites)[8 + btn->_index];
		break;
	default:
		sprite = _GIZMO(sprites)[1 + btn->_index];
		break;
	}

	if (src) {
		gr_buffer_rect_copy_2(src, dest, 0, 0, x, y, src->w, src->h);
		item->_grBuff->release();
	}

	gizmo_draw_sprite(sprite, dest, x, y);
	gizmo->_grBuff->release();
}

static void gizmo_item_free(GizmoItem *item) {
	if (!item)
		return;

	if (item->_grBuff)
		delete item->_grBuff;

	if (item->_button)
		mem_free(item->_button);
	mem_free(item);
}

static GrBuff *gizmo_create_buffer(Gizmo *gizmo, int sx, int sy, int w, int h) {
	if (!gizmo || !gizmo->_grBuff)
		return nullptr;

	GrBuff *grBuff = new GrBuff(w, h);
	Buffer *src = gizmo->_grBuff->get_buffer();
	Buffer *dest = grBuff->get_buffer();
	assert(src && dest);

	gr_buffer_rect_copy_2(src, dest, sx, sy, 0, 0, w, h);
	gizmo->_grBuff->release();
	grBuff->release();

	return grBuff;
}

static bool gizmo_item_contains(GizmoItem *item, int x, int y) {
	return item->_btnRect.contains(x, y);
}

static GizmoItem *gizmo_findItem(int id, Gizmo *gizmo) {
	for (GizmoItem *item = gizmo->_items; item; item = item->_next) {
		if (item->_id == id)
			return item;
	}

	return nullptr;
}

static bool gizmo_item_events(GizmoItem *item, int eventType, int event, int x, int y,
		GizmoItem **currentItem) {
	if (!item || !item->_button || eventType != EVENT_MOUSE)
		return false;

	GizmoButton *btn = item->_button;
	if (btn->_state == SELECTED)
		return false;

	bool flag1 = false;
	bool flag2 = false;
	bool flag3 = false;

	switch (event) {
	case _ME_move:
		if (gizmo_item_contains(item, x, y)) {
			if (!player_commands_allowed())
				return false;

			*currentItem = item;

			if (btn->_state != IN_CONTROL) {
				btn->_state = IN_CONTROL;
				flag1 = true;
			}
		} else {
			*currentItem = nullptr;

			if (btn->_state != NOTHING) {
				btn->_state = NOTHING;
				flag1 = true;
				flag3 = false;
			}
		}
		break;

	case _ME_L_click:
	case _ME_doubleclick:
		if (gizmo_item_contains(item, x, y)) {
			if (!player_commands_allowed())
				return false;

			btn->_state = OVER_CONTROL;
			*currentItem = item;
			flag1 = true;

		} else {
			*currentItem = nullptr;

			if (btn->_state != NOTHING) {
				btn->_state = NOTHING;
				flag1 = true;
			}
		}
		break;

	case _ME_L_hold:
	case _ME_doubleclick_hold:
		break;

	case _ME_L_drag:
	case _ME_doubleclick_drag:
		if (!*currentItem)
			return true;

		if (gizmo_item_contains(item, x, y)) {
			if (!player_commands_allowed())
				return false;

			if (btn->_state != OVER_CONTROL) {
				btn->_state = OVER_CONTROL;
				flag1 = true;
			}
		} else {
			if (btn->_state != IN_CONTROL) {
				btn->_state = IN_CONTROL;
				flag1 = true;
			}
		}
		break;

	case _ME_L_release:
	case _ME_doubleclick_release:
		if (gizmo_item_contains(item, x, y)) {
			if (!player_commands_allowed())
				return false;

			if (*currentItem)
				flag2 = true;
			else
				*currentItem = item;
		} else {
			*currentItem = nullptr;
			btn->_state = NOTHING;
			flag1 = true;
			flag3 = false;
		}
		break;

	default:
		break;
	}

	if (flag1) {
		(*item->_fnDraw)(item, item->_gizmo, item->_bounds.left, item->_bounds.top);

		int32 status;
		ScreenContext *ctx = vmng_screen_find(item->_gizmo, &status);

		if (ctx && status == 1)
			RestoreScreens(item->_bounds.left + ctx->x1, item->_bounds.top + ctx->y1,
				item->_bounds.right + ctx->x1, item->_bounds.bottom + ctx->y1);
	}

	if (flag2 && item->_select) {
		(*item->_select)();

		int32 status;
		ScreenContext *ctx = vmng_screen_find(item->_gizmo, &status);

		if (ctx && status == 1) {
			if (!gizmo_findItem(item->_id, item->_gizmo))
				*currentItem = nullptr;

		} else {
			*currentItem = nullptr;
		}
	}

	return flag3;
}

static GizmoItem *gizmo_add_item(Gizmo *gizmo, int id,
		int boundsX, int boundsY, int boundsW, int boundsH,
		int rect1X, int rect1Y, int rect1W, int rect1H,
		GizmoItemFnSelect select, int btnIndex, bool selected = false,
		bool hasBuffer = false, int arg9 = 0,
		GizmoItemFnEvents events = gizmo_item_events) {
	if (!gizmo)
		return nullptr;

	// Create new item
	GizmoItem *item = (GizmoItem *)mem_alloc(sizeof(GizmoItem), "*gui gizmo item");
	assert(item);

	// Put the new item at the head of the list
	item->_next = gizmo->_items;
	item->_prior = nullptr;
	item->_gizmo = gizmo;

	if (gizmo->_items)
		gizmo->_items->_prior = item;
	gizmo->_items = item;

	// Set fields
	item->_id = id;
	item->_bounds = Common::Rect(boundsX, boundsY, boundsX + boundsW - 1,
		boundsY + boundsH - 1);
	item->_btnRect = Common::Rect(rect1X, rect1Y, rect1X + rect1W - 1, rect1Y + rect1H - 1);
	item->_select = select;

	item->_hasBuffer = hasBuffer;
	if (hasBuffer) {
		item->_grBuff = gizmo_create_buffer(gizmo, rect1X, rect1Y, rect1W, rect1H);
	} else {
		item->_grBuff = nullptr;
	}

	GizmoButton *btn = (GizmoButton *)mem_alloc(sizeof(GizmoButton), "*gizmo button");
	assert(btn);

	btn->_state = selected ? SELECTED : NOTHING;
	btn->_index = btnIndex;
	btn->_field8 = arg9;
	btn->_field10 = id - 1000;
	item->_button = btn;

	item->_fnDraw = gizmo_item_draw;
	item->_fnFree = gizmo_item_free;
	item->_fnEvents = events;
	(*item->_fnDraw)(item, gizmo, boundsX, boundsY);

	int32 status = 0;
	ScreenContext *ctx = vmng_screen_find(gizmo, &status);

	if (ctx && status == 1) {
		RestoreScreens(ctx->x1 + item->_bounds.left, ctx->y1 + item->_bounds.top,
			ctx->x1 + item->_bounds.right, ctx->y1 + item->_bounds.bottom);
	}

	return item;
}

static Gizmo *gui_create_gizmo(M4sprite *sprite, int sx, int sy, uint scrnFlags) {
	if (!sprite)
		return nullptr;

	Gizmo *gui = (Gizmo *)mem_alloc(sizeof(Gizmo), "*gui gizmo");
	if (!gui)
		return nullptr;

	GrBuff *grBuff = new GrBuff(sprite->w, sprite->h);
	gui->_grBuff = grBuff;
	gui->_items = nullptr;
	gui->_fnEnter = nullptr;
	gui->_fnEscape = nullptr;
	gui->_fnEvents = gizmo_eventHandler;

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

	ScreenContext *ctx = vmng_screen_create(sx, sy, sx + sprite->w, sy + sprite->h,
		69, scrnFlags, gui, (RefreshFunc)gizmo_draw, gizmo_eventHandler);
	return ctx ? gui : nullptr;
}

void gizmo_anim(RGB8 *pal) {
	if (!_GIZMO(initialized))
		gizmo_initialize(pal);
	if (!gizmo_load_sprites("500gizmo", 58))
		return;

	Gizmo *gizmo = gui_create_gizmo(_GIZMO(sprites)[0], 0, 0, 505);
	_GIZMO(gui) = gizmo;
	assert(gizmo);

	gizmo_add_item(gizmo, 1, 234, 153, 178, 48, 251, 163, 144, 25,
		[]() { gizmo_daemon(5000); }, 0);
	gizmo_add_item(gizmo, 2, 234, 178, 178, 48, 254, 188, 136, 25,
		[]() { gizmo_daemon(5001); }, 1);
	gizmo_add_item(gizmo, 3, 235, 203, 178, 48, 256, 212, 128, 27,
		[]() { gizmo_daemon(5002); }, 2);
	gizmo_add_item(gizmo, 6, 234, 230, 178, 48, 260, 239, 119, 25,
		[]() { gizmo_daemon(5003); }, 3);
	gizmo_add_item(gizmo, 5, 234, 255, 178, 48, 264, 264, 109, 25,
		[]() { gizmo_daemon(5004); }, 4);
	gizmo_add_item(gizmo, 6, 234, 278, 178, 48, 266, 289, 102, 25,
		[]() { gizmo_daemon(5005); }, 5);
	gizmo_add_item(gizmo, 7, 234, 300, 178, 48, 268, 314, 96, 25,
		[]() { gizmo_daemon(5006); }, 6);

	vmng_screen_show(gizmo);
	mouse_lock_sprite(0);
}

} // namespace GUI
} // namespace Burger
} // namespace M4
