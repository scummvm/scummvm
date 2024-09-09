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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/gui/widgets/msg_scroll_new_ui.h"
#include "ultima/nuvie/core/effect.h" /* for initial fade-in */
#include "ultima/nuvie/core/tile_manager.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/screen/game_palette.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/gui/widgets/command_bar.h"
#include "ultima/nuvie/views/actor_view.h"
#include "ultima/nuvie/views/inventory_view.h"
#include "ultima/nuvie/gui/widgets/background.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

#define TMP_MAP_BORDER 3

#define WRAP_VIEWP(p,p1,s) ((p1-p) < 0 ? (p1-p) + s : p1-p)

// This should make the mouse-cursor hovering identical to that in U6.
static const uint8 movement_array[9 * 9] = {
	9, 9, 2, 2, 2, 2, 2, 3, 3,
	9, 9, 9, 2, 2, 2, 3, 3, 3,
	8, 9, 9, 2, 2, 2, 3, 3, 4,
	8, 8, 8, 9, 2, 3, 4, 4, 4,
	8, 8, 8, 8, 1, 4, 4, 4, 4,
	8, 8, 8, 7, 6, 5, 4, 4, 4,
	8, 7, 7, 6, 6, 6, 5, 5, 4,
	7, 7, 7, 6, 6, 6, 5, 5, 5,
	7, 7, 6, 6, 6, 6, 6, 5, 5
};

static const Tile grid_tile = {
	0,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	0,
	//uint8 qty;
	//uint8 flags;

	0,
	0,
	0,

	{
		54, 255, 255, 255, 58, 255, 255, 255, 58, 255, 255, 255, 58, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		58, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		58, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		58, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255

	}
};

MapWindow::MapWindow(const Configuration *cfg, Map *m): GUI_Widget(nullptr, 0, 0, 0, 0), config(cfg),
		map(m), anim_manager(nullptr), cur_x(0), cur_y(0), mousecenter_x(0),
		mousecenter_y(0), cur_x_add(0), cur_y_add(0), vel_x(0), vel_y(0),
		last_boundary_fill_x(0), last_boundary_fill_y(0), cursor_x(0), cursor_y(0),
		show_cursor(false), show_use_cursor(false), show_grid(false), x_ray_view(X_RAY_OFF),
		freeze_blacking_location(false), enable_blacking(true), new_thumbnail(false),
		thumbnail(nullptr), overlay(nullptr), overlay_level(MAP_OVERLAY_DEFAULT),
		cur_level(0), tmp_map_buf(nullptr), selected_obj(nullptr), look_obj(nullptr),
		look_actor(nullptr), walking(false), looking(false),
		original_obj_loc(MapCoord(0, 0, 0)), roof_tiles(nullptr),
		draw_brit_lens_anim(false), draw_garg_lens_anim(false), window_updated(true),
		roof_display(ROOF_DISPLAY_NORMAL), lighting_update_required(true), game(nullptr),
		custom_actor_tiles(false), tmp_map_width(0), tmp_map_height(0), tile_manager(nullptr),
		obj_manager(nullptr), actor_manager(nullptr), map_center_xoff(0), cursor_tile(nullptr),
		use_tile(nullptr), win_width(0), win_height(0), border_width(0), hackmove(false),
		game_started(false), wizard_eye_info({nullptr, 0, 0, 0, nullptr}) {

	config->value("config/GameType", game_type);

	uint16 x_off = Game::get_game()->get_game_x_offset();
	uint16 y_off = Game::get_game()->get_game_y_offset();

	GUI_Widget::Init(nullptr, x_off, y_off, 0, 0);

	screen = nullptr;

	map_width = map->get_width(cur_level);

	config->value(config_get_game_key(config) + "/map_tile_lighting", using_map_tile_lighting, game_type == NUVIE_GAME_MD ? false : true);
	config->value("config/input/enable_doubleclick", enable_doubleclick, true);
	config->value("config/input/look_on_left_click", look_on_left_click, true);
	set_use_left_clicks();
	config->value("config/input/walk_with_left_button", walk_with_left_button, true);
	set_walk_button_mask();
	config->value("config/cheats/min_brightness", min_brightness, 0);

	roof_mode = Game::get_game()->is_roof_mode();

	set_interface();
}

MapWindow::~MapWindow() {
	set_overlay(nullptr); // free
	free(tmp_map_buf);
	delete anim_manager;
	if (roof_tiles)
		delete roof_tiles;
}

bool MapWindow::init(TileManager *tm, ObjManager *om, ActorManager *am) {
	game = Game::get_game();

	tile_manager = tm;
	obj_manager = om;
	actor_manager = am;
	uint16 map_w = 11, map_h = 11;
	border_width = game->get_background()->get_border_width();
	if (!game->is_orig_style()) {
		uint16 game_width = game->get_game_width();
		uint16 game_height = game->get_game_height();

		if (game->is_original_plus_cutoff_map()) {
			map_center_xoff = 0;
			game_width -= border_width; // don't go over border
		} else if (game->is_original_plus_full_map()) {
			map_center_xoff = (border_width / 16) % 16;
		} else { // new style
			map_center_xoff = 0;
		}
		map_w = game_width / 16;
		map_h = game_height / 16;
		if (game_width % 16 != 0 || map_w % 2 == 0) { // not just the right size
			map_w += 1;
			if (map_w % 2 == 0) // need odd number of tiles to center properly
				map_w += 1;
		}
		if (game_height % 16 != 0 || map_h % 2 == 0) { // not just the right size
			map_h += 1;
			if (map_h % 2 == 0) // need odd number of tiles to center properly
				map_h += 1;
		}
		offset_x -= (map_w * 16 - game_width) / 2;
		offset_y -= (map_h * 16 - game_height) / 2;
	} else
		map_center_xoff = 0;
	anim_manager = new AnimManager(offset_x, offset_y);

	cursor_tile = tile_manager->get_cursor_tile();
	use_tile = tile_manager->get_use_tile();

	area.left = offset_x;
	area.top = offset_y;

	set_windowSize(map_w, map_h);

// hide the window until game is fully loaded and does fade-in
	get_overlay(); // this allocates `overlay`
	overlay_level = MAP_OVERLAY_ONTOP;
	assert(SDL_FillRect(overlay, nullptr, game->get_palette()->get_bg_color()) == 0);

	wizard_eye_info.eye_tile = tile_manager->get_tile(TILE_U6_WIZARD_EYE);
	wizard_eye_info.moves_left = 0;
	wizard_eye_info.caller = nullptr;

	if (roof_mode)
		loadRoofTiles();

	return true;
}

void MapWindow::set_use_left_clicks() {
	if (enable_doubleclick || look_on_left_click)
		set_accept_mouseclick(true, USE_BUTTON); // allow left clicks
	else
		set_accept_mouseclick(false, USE_BUTTON); // disallow left clicks
}

bool MapWindow::set_windowSize(uint16 width, uint16 height) {
	win_width = width;
	win_height = height;
	area.setWidth(win_width * 16);
	area.setHeight(win_height * 16);

// We make the temp map +1 bigger on the top and left edges
// and +2 bigger on the bottom and right edges

// The +1 is for the boundary fill function

// The additional +1 on the right/bottom edges is needed
// to hide objects on boundarys when wall is in darkness

	tmp_map_width = win_width + (TMP_MAP_BORDER * 2);// + 1;
	tmp_map_height = win_height + (TMP_MAP_BORDER * 2);// + 1;

	tmp_map_buf = (uint16 *)nuvie_realloc(tmp_map_buf, tmp_map_width * tmp_map_height * sizeof(uint16));
	if (tmp_map_buf == nullptr)
		return false;

// if(surface != nullptr)
//   delete surface;
// surface = new Surface;

// if(surface->init(win_width*16,win_height*16) == false)
//   return false;
	if (game->is_orig_style()) {
		clip_rect.left = area.left + 8;
		clip_rect.setWidth((win_width - 1) * 16);

		if (game_type == NUVIE_GAME_U6) {
			clip_rect.top = area.top + 8;
			clip_rect.setHeight((win_height - 1) * 16);
		} else {
			clip_rect.top = area.top + 16;
			clip_rect.setHeight((win_height - 2) * 16);
		}
	} else {
		clip_rect.left = game->get_game_x_offset();
		clip_rect.top = game->get_game_y_offset();
		if (game->is_original_plus_cutoff_map())
			clip_rect.setWidth(game->get_game_width() - border_width - 1);
		else
			clip_rect.setWidth(game->get_game_width());
		clip_rect.setHeight(game->get_game_height());
	}
	anim_manager->set_area(clip_rect);

	Screen *const gameScreen = Game::get_game()->get_screen();
	assert(gameScreen);
	_mapWinSubSurf.create(*gameScreen->get_sdl_surface(), clip_rect);

	reset_mousecenter();

	updateBlacking();

	return true;
}

void MapWindow::set_walk_button_mask() {
	if (walk_with_left_button)
		walk_button_mask = (BUTTON_MASK(USE_BUTTON) | BUTTON_MASK(WALK_BUTTON));
	else
		walk_button_mask = BUTTON_MASK(WALK_BUTTON);
}

void MapWindow::set_show_cursor(bool state) {
	ActorView *actor_view = game->get_view_manager()->get_actor_view();
	InventoryView *inventory_view = game->get_view_manager()->get_inventory_view();
	if (actor_view)
		actor_view->set_show_cursor(false);
	if (inventory_view)
		inventory_view->set_show_cursor(false);

	show_cursor = state;
}

void MapWindow::set_show_use_cursor(bool state) {
	ActorView *actor_view = game->get_view_manager()->get_actor_view();
	InventoryView *inventory_view = game->get_view_manager()->get_inventory_view();
	if (actor_view)
		actor_view->set_show_cursor(false);
	if (inventory_view)
		inventory_view->set_show_cursor(false);
	show_use_cursor = state;
}

void MapWindow::set_show_grid(bool state) {
	show_grid = state;
}

/**
 * cheat_off is needed to turn the X-ray cheat off (set to X_RAY_OFF)
 * Convoluted logic is so we don't slow down boundaryFill with an extra check
 */
void MapWindow::set_x_ray_view(X_RayType state, bool cheat_off) {
	if (x_ray_view == X_RAY_CHEAT_ON) {
		if (state == X_RAY_ON) // X_RAY_CHEAT_ON takes precedence to preserve X-ray cheat
			return;
		else if (state == X_RAY_OFF) {
			if (!cheat_off) { // not turning X-ray cheat off
				if (game->are_cheats_enabled()) // don't turn off when cheats are enabled
					return;
				else  // need to preserve X-ray cheat setting when cheats are off
					state = X_RAY_CHEAT_OFF;
			}
		}
	} else if (x_ray_view == X_RAY_CHEAT_OFF) {
		if (state == X_RAY_OFF) // X_RAY_CHEAT_OFF takes precedence to preserve X-ray cheat
			return;
		else if (state == X_RAY_ON) // need to preserve X-ray cheat setting when cheats are off
			state = X_RAY_CHEAT_ON;
	}
	x_ray_view = state;
	updateBlacking();
}

void MapWindow::set_freeze_blacking_location(bool state) {
	freeze_blacking_location = state;
}

void MapWindow::set_enable_blacking(bool state) {
	enable_blacking = state;
	updateBlacking();
}

void MapWindow::set_walking(bool state) {
	if (state && game->get_view_manager()->gumps_are_active()) //we don't allow walking while gumps are active.
		return;

	walking = state;
}

void MapWindow::moveLevel(uint8 new_level) {
	cur_level = new_level;

	updateBlacking();
}

void MapWindow::moveMap(sint16 new_x, sint16 new_y, sint8 new_level, uint8 new_x_add, uint8 new_y_add) {

	map_width = map->get_width(new_level);

	if (new_x < 0) {
		new_x = map_width + new_x;
	} else {
		new_x %= map_width;
	}

//printf("cur_x = %d\n",new_x);
	cur_x = new_x;
	cur_y = new_y;
	cur_level = new_level;
	cur_x_add = new_x_add;
	cur_y_add = new_y_add;
	updateBlacking();

}

void MapWindow::moveMapRelative(sint16 rel_x, sint16 rel_y) {
	moveMap(cur_x + rel_x, cur_y + rel_y, cur_level);
}

/* Move map by relative pixel amount.
 */
void MapWindow::shiftMapRelative(sint16 rel_x, sint16 rel_y) {
	const uint8 tile_pitch = 16;
	uint32 total_px = (cur_x * tile_pitch) + cur_x_add,
	       total_py = (cur_y * tile_pitch) + cur_y_add;
	total_px += rel_x;
	total_py += rel_y;
	moveMap(total_px / tile_pitch, total_py / tile_pitch, cur_level,
	        total_px % tile_pitch, total_py % tile_pitch);
}

/* Center MapWindow on a location.
 */
void MapWindow::centerMap(uint16 x, uint16 y, uint8 z) {
	moveMap(x - ((win_width - 1 - map_center_xoff) / 2) , y - ((win_height - 1) / 2), z);
}

void MapWindow::centerMapOnActor(Actor *actor) {
	uint16 x;
	uint16 y;
	uint8 z;

	actor->get_location(&x, &y, &z);

	centerMap(x, y, z);

	return;
}

void MapWindow::centerCursor() {

	cursor_x = (win_width - 1 - map_center_xoff) / 2;
	cursor_y = (win_height - 1) / 2;

	return;
}

void MapWindow::moveCursor(sint16 new_x, sint16 new_y) {
	if (new_x < 0 || new_x >= win_width)
		return;

	if (new_y < 0 || new_y >= win_height)
		return;

	cursor_x = new_x;
	cursor_y = new_y;

	return;
}

void MapWindow::moveCursorRelative(sint16 rel_x, sint16 rel_y) {
	moveCursor(cursor_x + rel_x, cursor_y + rel_y);
}

bool MapWindow::is_on_screen(uint16 x, uint16 y, uint8 z) {
	if (z == cur_level && WRAP_VIEWP(cur_x, x, map_width) < win_width && y >= cur_y && y < cur_y + win_height) {
		if (tile_is_black(x, y) == false) {
			return true;
		}
	}

	return false;
}

/**
 * Can we display an object at this location on the tmp map buffer.
 *
 * @param x coord on the tmp buf
 * @param y coord on the tmp buf
 * @param obj object to display
 * @return
 */
bool MapWindow::can_display_obj(uint16 x, uint16 y, Obj *obj) {
	uint16 tile_num = tmp_map_buf[y * tmp_map_width + x];
	if (tile_num == 0) //don't draw object if area is in darkness.
		return false;
	else {
		if (x >= tmp_map_width - 1 || y >= tmp_map_height - 1)
			return false;

		// We don't show objects on walls if the area to the right or bottom of the wall is in darkness
		if (tmp_map_buf[y * tmp_map_width + (x + 1)] == 0 || tmp_map_buf[(y + 1)*tmp_map_width + x] == 0) {
			Tile *tile = tile_manager->get_tile(tile_num);
			if (((tile->flags1 & TILEFLAG_WALL) || (game_type == NUVIE_GAME_U6 && obj->obj_n == OBJ_U6_BARS)))
				return false;
		}
	}

	return true;
}

bool MapWindow::tile_is_black(uint16 x, uint16 y, const Obj *obj) const {
	if (game->using_hackmove())
		return false;
	if (!MapCoord(x, y, cur_level).is_visible()) // tmpBufTileIsBlack will crash if called (doesn't happen in gdb)
		return true;
	uint16 wrapped_x = WRAP_VIEWP(cur_x, x, map_width);
	if (tmpBufTileIsBlack(wrapped_x + TMP_MAP_BORDER, y - cur_y + TMP_MAP_BORDER))
		return true;
	else if (obj) {
		const Tile *tile = tile_manager->get_original_tile(obj_manager->get_obj_tile_num(obj->obj_n) + obj->frame_n);
		if (!tile || (tmpBufTileIsBlack(wrapped_x + TMP_MAP_BORDER + 1, y - cur_y + TMP_MAP_BORDER)  && !(tile->flags1 & TILEFLAG_WALL))
		        || (tmpBufTileIsBlack(wrapped_x + TMP_MAP_BORDER, y - cur_y + TMP_MAP_BORDER + 1) && !(tile->flags1 & TILEFLAG_WALL)))
			return true;
	}

	return false;
}

const char *MapWindow::look(uint16 x, uint16 y, bool show_prefix) {
	if (tmp_map_buf[(y + TMP_MAP_BORDER) * tmp_map_width + (x + TMP_MAP_BORDER)] == 0) //black area
		return "darkness."; // nothing to see here. ;)

	uint16 wrapped_x = WRAPPED_COORD(cur_x + x, cur_level);
	const Actor *actor = actor_manager->get_actor(wrapped_x, cur_y + y, cur_level);
	if (actor != nullptr && actor->is_visible())
		return actor_manager->look_actor(actor, show_prefix);

	return map->look(wrapped_x, cur_y + y, cur_level);
}


Obj *MapWindow::get_objAtCursor(bool for_use /* = false */) {
	MapCoord coord = get_cursorCoord();
	return get_objAtCoord(coord, OBJ_SEARCH_TOP, OBJ_EXCLUDE_IGNORED, for_use);
}

Obj *MapWindow::get_objAtCoord(MapCoord coord, bool top_obj, bool include_ignored_objects, bool for_use /* = false */) {
	if (tile_is_black(coord.x, coord.y))
		return nullptr; // nothing to see here. ;)

	Obj *obj = obj_manager->get_obj(coord.x, coord.y, coord.z, top_obj, include_ignored_objects);
	// Savage Empire Create Object from Tile
	if (for_use && game_type == NUVIE_GAME_SE && obj == nullptr) {
		Script *script = game->get_script();
		uint16 map_win_x = WRAP_VIEWP(cur_x, coord.x, map_width);
		uint16 map_win_y = coord.y - cur_y;
		// Check that x,y is in tmp_map_buf
		if (is_on_screen(coord.x, coord.y, coord.z)) {
			uint16 tile_n = tmp_map_buf[(map_win_y + TMP_MAP_BORDER) * tmp_map_width + (map_win_x + TMP_MAP_BORDER)];
			uint16 obj_n = script->call_get_tile_to_object_mapping(tile_n);
			if (obj_n != 0) {
				obj = obj_manager->get_tile_obj(obj_n);
				obj->x = coord.x;
				obj->y = coord.y;
				obj->z = coord.z;
			}
		}
	}
	return obj;
}

Actor *MapWindow::get_actorAtCursor() {
	if (tmp_map_buf[(cursor_y + TMP_MAP_BORDER) * tmp_map_width + (cursor_x + TMP_MAP_BORDER)] == 0) //black area
		return nullptr; // nothing to see here. ;)

	return actor_manager->get_actor(WRAPPED_COORD(cur_x + cursor_x, cur_level), WRAPPED_COORD(cur_y + cursor_y, cur_level), cur_level);
}

MapCoord MapWindow::get_cursorCoord() {
	return MapCoord(WRAPPED_COORD(cur_x + cursor_x, cur_level), WRAPPED_COORD(cur_y + cursor_y, cur_level), cur_level);
}

void MapWindow::get_level(uint8 *level) const {
	*level = cur_level;
}

void MapWindow::get_pos(uint16 *x, uint16 *y, uint8 *px, uint8 *py) const {
	*x = cur_x;
	*y = cur_y;
	if (px)
		*px = cur_x_add;
	if (py)
		*py = cur_y_add;
}

void MapWindow::get_windowSize(uint16 *width, uint16 *height) const {
	*width = win_width;
	*height = win_height;
}

/* Returns true if the location at the coordinates is visible on the map window.
 */
bool MapWindow::in_window(uint16 x, uint16 y, uint8 z) const {
	return ((z == cur_level && WRAP_VIEWP(cur_x, x, map_width) < win_width
	         && y >= cur_y && y <= (cur_y + win_height)));
}


/* Update player position if walking to mouse cursor. Update map position.
 */
void MapWindow::update() {
	GameClock *clock = game->get_clock();
	Events *event = game->get_event();
	static uint32 last_update_time = clock->get_ticks();
	uint32 update_time = clock->get_ticks();

	// do fade-in on the first update (game has loaded now)
	if (game_started == false) {
		new GameFadeInEffect(game->get_palette()->get_bg_color());
		game_started = true;
	}

	anim_manager->update(); // update animations

	if (vel_x || vel_y) { // this slides the map
		if ((update_time - last_update_time) >= 100) { // only move every 10th sec
			sint32 sx = vel_x / 10, sy = vel_y / 10;
			if (vel_x && !sx) // move even if vel_x/vel_y was < 10
				sx = (vel_x < 0) ? -1 : 1;
			if (vel_y && !sy)
				sy = (vel_y < 0) ? -1 : 1;

			shiftMapRelative(sx, sy);
			last_update_time = update_time;
		}
	}

	if (walking) {
		if (Events::get()->getButtonState() & walk_button_mask) {
			if (game->user_paused())
				return;

			int mx, my; // bit-AND buttons with mouse state to test
			screen->get_mouse_location(&mx, &my);

			if (is_wizard_eye_mode()) {
//              int wx, wy;
//              mouseToWorldCoords(mx, my, wx, wy);
				sint16 rx, ry;
				get_movement_direction((uint16)mx, (uint16)my, rx, ry);
				moveMapRelative((rx == 0) ? 0 : rx < 0 ? -1 : 1,
				                (ry == 0) ? 0 : ry < 0 ? -1 : 1);
				wizard_eye_update();
			} else {
				//DEBUG(0, LEVEL_DEBUGGING, "MOUSE WALKING...\n");
				event->walk_to_mouse_cursor((uint32)mx,
				                            (uint32)my);
			}
		} else
			walking = false;
	}

	KeyBinder *keybinder = game->get_keybinder();
	if (keybinder->is_joy_repeat_enabled() && (event->get_mode() == MOVE_MODE || is_wizard_eye_mode())
	        && keybinder->get_next_joy_repeat_time() < clock->get_ticks()) {
		Common::KeyCode key = keybinder->get_key_from_joy_walk_axes();
		if (key != Common::KEYCODE_INVALID) {
			Common::Event sdl_event;
			sdl_event.type = Common::EVENT_KEYDOWN;
			sdl_event.kbd.keycode = key;
			sdl_event.kbd.flags = 0;
			if (GUI::get_gui()->HandleEvent(&sdl_event) == GUI_PASS)
				event->handleEvent(&sdl_event);
		}
	}
}

// moved from updateBlacking() so you don't have to update all blacking (SB-X)
void MapWindow::updateAmbience() {
	lighting_update_required = true;
}

// moved from updateBlacking() so you don't have to update all blacking (SB-X)
void MapWindow::createLightOverlay() {
	//Dusk starts at 19:00
	//It's completely dark by 20:00
	//Dawn starts at 5:00
	//It's completely bright by 6:00
	//Dusk and dawn operate by changing the ambient light, not by changing the radius of the avatar's light globe

	if (!screen)
		return;

	uint8 cur_min_brightness;
	if (game->are_cheats_enabled())
		cur_min_brightness = min_brightness;
	else
		cur_min_brightness = 0;

	GameClock *clock = game->get_clock();
	Weather *weather = game->get_weather();

	bool dawn_or_dusk = false;
	int h = clock->get_hour();
	int a;
	if (x_ray_view >= X_RAY_ON)
		a = 255;
	else if (in_dungeon_level())
		a = cur_min_brightness;
	else if (weather->is_eclipse())  //solar eclipse
		a = cur_min_brightness;
	else if (h == 19) { //Dusk -- Smooth transition between 255 and min_brightness during first 59 minutes
		if (screen->get_lighting_style() == LightingSmooth) {
			dawn_or_dusk = true;
			a = 255 - (uint8)((255.0f - cur_min_brightness) * (float)clock->get_minute() / 59.0f);
		} else {
			a = 20 * (6 - clock->get_minute() / 10);
			if (a < cur_min_brightness)
				a = cur_min_brightness;
		}
	} else if (h == 5) { //Dawn -- Smooth transition between min_brightness and 255 during first 59 minutes
		if (screen->get_lighting_style() == LightingSmooth) {
			dawn_or_dusk = true;
			a = cur_min_brightness + (255.0f - cur_min_brightness) * (float)clock->get_minute() / 59.0f;
		} else {
			a = 20 * (1 +  clock->get_minute() / 10);
			if (a < cur_min_brightness)
				a = cur_min_brightness;
		}
	} else if (h > 5 && h < 19) //Day
		a = 255;
	else //Night
		a = cur_min_brightness;

	if (a > 255)
		a = 255;

	bool party_light_source;
	// smooth seems to need an enormous range in order to have smooth transitions
	if (a < (screen->get_lighting_style() == LightingSmooth ? 248 : 81) &&
	        (game->get_party()->has_light_source() || clock->get_timer(GAMECLOCK_TIMER_U6_LIGHT) != 0)) { //FIXME U6 specific
		party_light_source = true;
		if (screen->get_lighting_style() == LightingSmooth) {
			if (!dawn_or_dusk) // preserve a when dusk or dawn so we have the correct opacity
				a = cur_min_brightness;
		} else
			a = 80;
	} else
		party_light_source = false;
	screen->set_ambient(a);

	//Clear the opacity map
	screen->clearalphamap8(0, 0, win_width, win_height, screen->get_ambient(), party_light_source);

	updateLighting();

	lighting_update_required = false;
}

void MapWindow::updateLighting() {
	if (using_map_tile_lighting) {
		uint16 *ptr = tmp_map_buf;
		for (uint16 y = 0; y < tmp_map_height; y++) {
			for (uint16 x = 0; x < tmp_map_width; x++) {
				if (tmp_map_buf[x + y * tmp_map_width] != 0) {
					Tile *tile = tile_manager->get_tile(*ptr);
					if (GET_TILE_LIGHT_LEVEL(tile) > 0)
						screen->drawalphamap8globe(x - TMP_MAP_BORDER, y - TMP_MAP_BORDER, GET_TILE_LIGHT_LEVEL(tile));

					U6LList *obj_list = obj_manager->get_obj_list(cur_x - TMP_MAP_BORDER + x, cur_y - TMP_MAP_BORDER + y, cur_level); //FIXME wrapped coords.
					if (obj_list) {
						for (U6Link *link = obj_list->start(); link != nullptr; link = link->next) {
							Obj *obj = (Obj *)link->data;
							tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(obj) + obj->frame_n); //FIXME do we need to check the light for each tile in a multi-tile object.
							if (GET_TILE_LIGHT_LEVEL(tile) > 0 && can_display_obj(x, y, obj))
								screen->drawalphamap8globe(x - TMP_MAP_BORDER, y - TMP_MAP_BORDER, GET_TILE_LIGHT_LEVEL(tile));
						}
					}
				}
				ptr++;
			}
		}

		for (const TileInfo &ti : m_ViewableMapTiles) {
			if (GET_TILE_LIGHT_LEVEL(ti.t) > 0)
				screen->drawalphamap8globe(ti.x, ti.y, GET_TILE_LIGHT_LEVEL(ti.t));
		}
	}

	/* draw light coming from the actor
	   Wisps can change the light level depending on their current tile so we can't use actor->light for an actor's innate lighting.
	*/
	for (uint16 i = 0; i < 256; i++) {
		const Actor *actor = actor_manager->get_actor(i);

		if (actor->z == cur_level) {
			if (actor->x >= cur_x - TMP_MAP_BORDER && actor->x < cur_x + win_width + TMP_MAP_BORDER) {
				if (actor->y >= cur_y - TMP_MAP_BORDER && actor->y < cur_y + win_height + TMP_MAP_BORDER) {
					if (tmp_map_buf[(actor->y - cur_y + TMP_MAP_BORDER) * tmp_map_width + (actor->x - cur_x + TMP_MAP_BORDER)] != 0) {
						uint8 light = actor->get_light_level();
						if (light > 0) {
							screen->drawalphamap8globe(actor->x - cur_x, actor->y - cur_y, light);
						}
					}
				}
			}
		}
	}
}

void MapWindow::updateBlacking() {
	generateTmpMap();

	updateAmbience();

	m_ViewableObjects.clear();
/// m_ViewableObjTiles.clear();

	draw_brit_lens_anim = false;
	draw_garg_lens_anim = false;

	window_updated = true;
}

void MapWindow::Display(bool full_redraw) {
	if (lighting_update_required) {
		createLightOverlay();
	}

	uint16 *map_ptr = tmp_map_buf;
	map_ptr += (TMP_MAP_BORDER * tmp_map_width + TMP_MAP_BORDER);// * sizeof(uint16); //remember our tmp map is TMP_MAP_BORDER bigger all around.

	for (uint16 i = 0; i < win_height; i++) {
		for (uint16 j = 0; j < win_width; j++) {
			sint16 draw_x = area.left + (j * 16), draw_y = area.top + (i * 16);
			//draw_x -= (cur_x_add <= draw_x) ? cur_x_add : draw_x;
			//draw_y -= (cur_y_add <= draw_y) ? cur_y_add : draw_y;
			draw_x -= cur_x_add;
			draw_y -= cur_y_add;
			if (map_ptr[j] == 0) {
				screen->clear(draw_x, draw_y, 16, 16, &clip_rect); //blackout tile.
			} else {
				const Tile *tile;
				if (map_ptr[j] >= 16 && map_ptr[j] < 48) { //lay down the base tile for shoreline tiles
					tile = tile_manager->get_anim_base_tile(map_ptr[j]);
					screen->blit(draw_x, draw_y, (const byte *)tile->data, 8, 16, 16, 16, tile->transparent, &clip_rect);
				}

				tile = tile_manager->get_tile(map_ptr[j]);
				screen->blit(draw_x, draw_y, (const byte *)tile->data, 8, 16, 16, 16, tile->transparent, &clip_rect);

			}

		}
		//map_ptr += map_width;
		map_ptr += tmp_map_width ;//* sizeof(uint16);
	}

	drawObjs();

//drawAnims();

	if (roof_mode && roof_display != ROOF_DISPLAY_OFF) {
		drawRoofs();
	}

	if (game->get_clock()->get_timer(GAMECLOCK_TIMER_U6_STORM) != 0) //FIXME u6 specific.
		drawRain();

	if (show_grid) {
		drawGrid();
	}

	if (show_cursor) {
		screen->blit(area.left + cursor_x * 16, area.top + cursor_y * 16, (byte *)cursor_tile->data, 8, 16, 16, 16, true, &clip_rect);
	}

	if (show_use_cursor) {
		screen->blit(area.left + cursor_x * 16, area.top + cursor_y * 16, (byte *)use_tile->data, 8, 16, 16, 16, true, &clip_rect);
	}

// screen->fill(0,8,8,win_height*16-16,win_height*16-16);

	screen->blitalphamap8(area.left, area.top, &clip_rect);

	if (game->get_clock()->get_timer(GAMECLOCK_TIMER_U6_INFRAVISION) != 0)
		drawActors();

	if (overlay && overlay_level == MAP_OVERLAY_DEFAULT)
		screen->blit(area.left, area.top, (byte *)(overlay->getPixels()), overlay->format.bpp(), overlay->w, overlay->h, overlay->pitch, true, &clip_rect);

	drawAnims(true);

	if (new_thumbnail)
		create_thumbnail();

	if (is_wizard_eye_mode()) {
		uint16 we_x = mousecenter_x * 16 + area.left;
		if (game->is_original_plus_full_map())
			we_x -= ((map_center_xoff + 1) / 2) * 16;
		screen->blit(we_x, mousecenter_y * 16 + area.top, (byte *)wizard_eye_info.eye_tile->data, 8, 16, 16, 16, true, &clip_rect);
	}

	if (game->is_orig_style())
		drawBorder();

	if (overlay && overlay_level == MAP_OVERLAY_ONTOP)
		screen->blit(area.left, area.top, (byte *)(overlay->getPixels()), overlay->format.bpp(), overlay->w, overlay->h, overlay->pitch, true, &clip_rect);

// ptr = (byte *)screen->get_pixels();
// ptr += 8 * screen->get_pitch() + 8;

// screen->blit(8,8,ptr,8,(win_width-1) * 16,(win_height-1) * 16, win_width * 16, false);

	if (game->is_orig_style())
		screen->update(area.left + 8, area.top + 8, win_width * 16 - 16, win_height * 16 - 16);
	else if (game->is_original_plus_cutoff_map())
		screen->update(Game::get_game()->get_game_x_offset(), Game::get_game()->get_game_y_offset(), game->get_game_width() - border_width - 1, game->get_game_height());
	else
		screen->update(Game::get_game()->get_game_x_offset(), Game::get_game()->get_game_y_offset(), game->get_game_width(), game->get_game_height());

	if (window_updated) {
		window_updated = false;
		game->get_sound_manager()->update_map_sfx();
	}

}

void MapWindow::drawActors() {
	for (uint16 i = 0; i < 256; i++) {
		Actor *actor = actor_manager->get_actor(i);

		if (actor->z == cur_level) {
			uint8 x = WRAP_VIEWP(cur_x, actor->x, map_width);
			if (x < win_width) { //actor->x >= cur_x && actor->x < cur_x + win_width)
				if (actor->y >= cur_y && actor->y < cur_y + win_height) {
					if (tmp_map_buf[(actor->y - cur_y + TMP_MAP_BORDER) * tmp_map_width + (x + TMP_MAP_BORDER)] != 0) {
						drawActor(actor);
					}
				}
			}
		}
	}
}

//FIX need a function for multi-tile actors.
inline void MapWindow::drawActor(const Actor *actor) {
	if (actor->is_visible() /* && actor->obj_n != 0*/
	        && (!(actor->obj_flags & OBJ_STATUS_INVISIBLE) || actor->is_in_party() || actor == actor_manager->get_player())
	        && actor->get_corpser_flag() == false) {
		Tile *tile = tile_manager->get_tile(actor->get_tile_num() + actor->frame_n);
		Tile *rtile = nullptr;

		if (actor->obj_flags & OBJ_STATUS_INVISIBLE) {
			rtile = new Tile(*tile);
			for (int x = 0; x < 256; x++)
				if (rtile->data[x] != 0x00)
					rtile->data[x] = 0xFF;
				else
					rtile->data[x] = 0x0B;
		} else if (actor->status_flags & ACTOR_STATUS_PROTECTED) { // actually this doesn't appear when using a protection ring
			rtile = new Tile(*tile);
			for (int x = 0; x < 256; x++)
				if (rtile->data[x] == 0x00)
					rtile->data[x] = 0x0C;
		} else if (actor->is_cursed()) {
			rtile = new Tile(*tile);
			for (int x = 0; x < 256; x++)
				if (rtile->data[x] == 0x00)
					rtile->data[x] = 0x9;
		}
		uint16 wrapped_x = WRAP_VIEWP(cur_x, actor->x, map_width);
		if (rtile != 0) {
			drawNewTile(rtile, wrapped_x, actor->y - cur_y, false);
			drawNewTile(rtile, wrapped_x, actor->y - cur_y, true);
			delete rtile;
		} else {
			drawTile(tile, wrapped_x, actor->y - cur_y, false);
			drawTile(tile, wrapped_x, actor->y - cur_y, true);
			if (game->get_clock()->get_timer(GAMECLOCK_TIMER_U6_INFRAVISION) != 0) {
				const Std::list<Obj *> &surrounding_objs = actor->get_surrounding_obj_list();
				for (Obj *obj : surrounding_objs) {
					const Tile *t = tile_manager->get_original_tile(obj_manager->get_obj_tile_num(obj->obj_n) + obj->frame_n);
					uint16 wrapped_obj_x = WRAP_VIEWP(cur_x, obj->x, map_width);
					drawTile(t, wrapped_obj_x, obj->y - cur_y, false);
					drawTile(t, wrapped_obj_x, obj->y - cur_y, true); // doesn't seem needed but will do it anyway (for now)
				}
			}
		}
	}
}

void MapWindow::drawObjs() {
//FIXME: we need to make this more efficient.

	drawObjSuperBlock(true, false); //draw force lower objects
	drawObjSuperBlock(false, false); //draw lower objects

	drawActors();

	drawAnims(false);

	drawObjSuperBlock(false, true); //draw top objects

	drawLensAnim();
	return;
}

inline void MapWindow::drawLensAnim() {
	if (draw_brit_lens_anim) {
		if (cur_x < 0x399)
			drawTile(tile_manager->get_tile(TILE_U6_BRITANNIAN_LENS_ANIM_2), 0x398 - cur_x, 0x353 - cur_y, true);
		if (cur_x + win_width > 0x39a)
			drawTile(tile_manager->get_tile(TILE_U6_BRITANNIAN_LENS_ANIM_1), 0x39a - cur_x, 0x353 - cur_y, true);
	}

	if (draw_garg_lens_anim) {
		if (cur_x < 0x39d)
			drawTile(tile_manager->get_tile(TILE_U6_GARGOYLE_LENS_ANIM_2), 0x39c - cur_x, 0x353 - cur_y, true);
		if (cur_x + win_width > 0x39e)
			drawTile(tile_manager->get_tile(TILE_U6_GARGOYLE_LENS_ANIM_1), 0x39e - cur_x, 0x353 - cur_y, true);
	}
}

void MapWindow::drawObjSuperBlock(bool draw_lowertiles, bool toptile) {
	uint16 stop_x, stop_y;

	if (cur_x < 0)
		stop_x = 0;
	else
		stop_x = cur_x;

	if (cur_y < 0)
		stop_y = 0;
	else
		stop_y = cur_y;

	for (sint16 y = cur_y + win_height; y >= stop_y; y--) {
		for (sint16 x = cur_x + win_width; x >= stop_x; x--) {
			U6LList *obj_list = obj_manager->get_obj_list(x, y, cur_level);
			if (obj_list) {
				for (U6Link *link = obj_list->start(); link != nullptr; link = link->next) {
					Obj *obj = (Obj *)link->data;
					drawObj(obj, draw_lowertiles, toptile);
				}
			}
		}
	}

}

inline void MapWindow::drawObj(const Obj *obj, bool draw_lowertiles, bool toptile) {
	sint16 y = obj->y - cur_y;
	sint16 x = WRAP_VIEWP(cur_x, obj->x, map_width);

	if (x < 0 || y < 0)
		return;

	if (window_updated) {
		m_ViewableObjects.push_back(obj);

		if (game_type == NUVIE_GAME_U6 && cur_level == 0 && obj->y == 0x353 && tmp_map_buf[(y + TMP_MAP_BORDER)*tmp_map_width + (x + TMP_MAP_BORDER)] != 0) {
			if (obj->obj_n == 394 && obj->x == 0x399) {
				draw_brit_lens_anim = true;
			} else if (obj->obj_n == 396 && obj->x == 0x39d) {
				draw_garg_lens_anim = true;
			}
		}
	}

	//don't show invisible objects.
	if (obj->status & OBJ_STATUS_INVISIBLE)
		return;

	Tile *tile = tile_manager->get_original_tile(obj_manager->get_obj_tile_num(obj) + obj->frame_n);

	if (draw_lowertiles == false && (tile->flags3 & 0x4) && toptile == false) //don't display force lower tiles.
		return;

	if (draw_lowertiles == true && !(tile->flags3 & 0x4))
		return;

	if (tmp_map_buf[(y + TMP_MAP_BORDER)*tmp_map_width + (x + TMP_MAP_BORDER)] == 0) //don't draw object if area is in darkness.
		return;
	else {
		// We don't show objects on walls if the area to the right or bottom of the wall is in darkness
		if (tmp_map_buf[(y + TMP_MAP_BORDER)*tmp_map_width + (x + TMP_MAP_BORDER + 1)] == 0 || tmp_map_buf[(y + TMP_MAP_BORDER + 1)*tmp_map_width + (x + TMP_MAP_BORDER)] == 0) {
			if ((!(tile->flags1 & TILEFLAG_WALL) || (game_type == NUVIE_GAME_U6 && obj->obj_n == OBJ_U6_BARS)))
				return;
		}
	}

	drawTile(tile, x, obj->y - cur_y, toptile);

}

/* The pixeldata in the passed Tile pointer will be used if use_tile_data is
 * true, otherwise the current tile is derived from tile_num. This can't be
 * used with animated tiles. It only applies to the base tile in multi-tiles.
 */
inline void MapWindow::drawTile(const Tile *tile, uint16 x, uint16 y, bool toptile,
								bool use_tile_data) {
	uint16 tile_num = tile->tile_num;

	//don't show special marker tiles in MD unless "show eggs" is turned on.
	if (game_type == NUVIE_GAME_MD
	        && tile_num > 2040 && tile_num < 2048
	        && !obj_manager->is_showing_eggs())
		return;

	/* shouldn't be needed for in_town check
	 if(window_updated)
	   {
	    TileInfo ti;
	    ti.t=tile;
	    ti.left=x;
	    ti.top=y;
	    m_ViewableObjTiles.push_back(ti);
	   }
	*/
	bool dbl_width = tile->dbl_width;
	bool dbl_height = tile->dbl_height;

	if (x < win_width && y < win_height)
		drawTopTile(use_tile_data ? tile : tile_manager->get_tile(tile_num), x, y, toptile);

	if (dbl_width) {
		tile_num--;
		if (x > 0 && y < win_height) {
			tile = tile_manager->get_tile(tile_num);
			drawTopTile(tile, x - 1, y, toptile);
		}
	}

	if (dbl_height) {
		tile_num--;
		if (y > 0 && x < win_width) {
			tile = tile_manager->get_tile(tile_num);
			drawTopTile(tile, x, y - 1, toptile);
		}
	}

	if (x > 0 && dbl_width && y > 0 && dbl_height) {
		tile_num--;
		tile = tile_manager->get_tile(tile_num);
		drawTopTile(tile, x - 1, y - 1, toptile);
	}

}

inline void MapWindow::drawNewTile(const Tile *tile, uint16 x, uint16 y, bool toptile) {
	drawTile(tile, x, y, toptile, true);
}

inline void MapWindow::drawTopTile(const Tile *tile, uint16 x, uint16 y, bool toptile) {



// if(tile->boundary)
//  {
//    screen->blit(cursor_tile->data,8,x*16,y*16,16,16,false);
//   }
// FIXME: Don't use pixel offset (x_add,y_add) here, pass it via params?
	if (toptile) {
		if (tile->toptile)
//        screen->blit(x*16,y*16,tile->data,8,16,16,16,tile->transparent,&clip_rect);
			screen->blit(area.left + (x * 16) - cur_x_add, area.top + (y * 16) - cur_y_add, tile->data, 8, 16, 16, 16, tile->transparent, &clip_rect);
	} else {
		if (!tile->toptile)
//        screen->blit(x*16,y*16,tile->data,8,16,16,16,tile->transparent,&clip_rect);
			screen->blit(area.left + (x * 16) - cur_x_add, area.top + (y * 16) - cur_y_add, tile->data, 8, 16, 16, 16, tile->transparent, &clip_rect);
	}
}

void MapWindow::drawBorder() {
	const Tile *tile;
	const Tile *tile1;
	uint16 i;

	if (game_type != NUVIE_GAME_U6)
		return;

	const uint16 orig_win_w = 11;
	const uint16 orig_win_h = 11;
	uint16 x_off = Game::get_game()->get_game_x_offset();
	uint16 y_off = Game::get_game()->get_game_y_offset();

	tile = tile_manager->get_tile(432);
	screen->blit(x_off, y_off, tile->data, 8, 16, 16, 16, true, &clip_rect); // upper left corner

	tile = tile_manager->get_tile(434);
	screen->blit(x_off + (orig_win_w - 1) * 16, y_off, tile->data, 8, 16, 16, 16, true); // upper right corner (got rid of &clip_rect for original+)

	tile = tile_manager->get_tile(435);
	screen->blit(x_off, y_off + (orig_win_h - 1) * 16, tile->data, 8, 16, 16, 16, true, &clip_rect); // lower left corner

	tile = tile_manager->get_tile(437);
	screen->blit(x_off + (orig_win_w - 1) * 16, y_off + (orig_win_h - 1) * 16, tile->data, 8, 16, 16, 16, true); // lower right corner (got rid of &clip_rect for original+)

	tile = tile_manager->get_tile(433);
	tile1 = tile_manager->get_tile(436);

	for (i = 1; i < orig_win_w - 1; i++) {
		screen->blit(x_off + i * 16, y_off, tile->data, 8, 16, 16, 16, true, &clip_rect); // top row
		screen->blit(x_off + i * 16, y_off + (orig_win_h - 1) * 16, tile1->data, 8, 16, 16, 16, true, &clip_rect); // bottom row
	}

	tile = tile_manager->get_tile(438);
	tile1 = tile_manager->get_tile(439);

	for (i = 1; i < orig_win_h - 1; i++) {
		screen->blit(x_off, y_off + i * 16, tile->data, 8, 16, 16, 16, true, &clip_rect); // left column
		screen->blit(x_off + (orig_win_w - 1) * 16, y_off + i * 16, tile1->data, 8, 16, 16, 16, true); // right column (got rid of &clip_rect for original+)
	}
}

//FIXME this won't work for wrapped maps like MD.
void MapWindow::drawRoofs() {
	if (cur_y < 1 || cur_y > 760) // FIXME We need to handle this properly
		return;
	if (roof_display == ROOF_DISPLAY_NORMAL && map->has_roof(WRAPPED_COORD(cur_x + (win_width - 1 - map_center_xoff) / 2, cur_level), cur_y + (win_height - 1) / 2, cur_level)) //Don't draw roof tiles if player is underneath.
		return;
	if (x_ray_view >= X_RAY_ON)
		return;

	bool orig_style = game->is_orig_style();

	uint16 *roof_map_ptr = map->get_roof_data(cur_level);

	Common::Rect src(16, 16), dst(16, 16);

	if (roof_map_ptr) {
		roof_map_ptr += cur_y * 1024 + cur_x;
		for (uint16 i = 0; i < win_height; i++) {
			for (uint16 j = 0; j < win_width; j++) {
				if (roof_map_ptr[j] != 0) {
					dst.left = area.left + (j * 16);
					dst.top = area.top + (i * 16);
					dst.left -= cur_x_add;
					dst.top -= cur_y_add;

					src.left = (roof_map_ptr[j] % MAPWINDOW_ROOFTILES_IMG_W) * 16;
					src.top = (roof_map_ptr[j] / MAPWINDOW_ROOFTILES_IMG_W) * 16;

					if (orig_style) {
						src.setWidth(16);
						src.setHeight(16);
						dst.setWidth(16);
						dst.setHeight(16);

						if (i == 0) {
							src.top += 8;
							src.setHeight(8);
							dst.top += 8;
							dst.setHeight(8);
						} else if (i == win_height - 1) {
							src.setHeight(8);
							dst.setHeight(8);
						}

						if (j == 0) {
							src.left += 8;
							src.setWidth(8);
							dst.left += 8;
							dst.setWidth(8);
						} else if (j == win_width - 1) {
							src.setWidth(8);
							dst.setWidth(8);
						}
						SDL_BlitSurface(roof_tiles, &src, surface, &dst);
					} else {
						src.setWidth(16);
						src.setHeight(16);
						_mapWinSubSurf.blitFrom(*roof_tiles, src, Common::Point(dst.left, dst.top));
					}
				}
			}
			roof_map_ptr += 1024;
		}
	}
}

void MapWindow::drawRain() {
	int c;
	if (game->is_orig_style())
		c = win_width * win_height;
	else if (game->is_original_plus_cutoff_map())
		c = ((game->get_game_width() - border_width) * game->get_game_height()) / 256;
	else
		c = (game->get_game_width() * game->get_game_height()) / 256;
	for (int i = 0; i < c; i++) {
		uint16 x;
		uint16 y;
		if (game->is_orig_style()) {
			x = area.left + NUVIE_RAND() % ((win_width - 1) * 16 - 2) + 8;
			y = area.top + NUVIE_RAND() % ((win_height - 1) * 16 - 2) + 8;
		} else {
			if (game->is_original_plus_cutoff_map())
				x = game->get_game_x_offset() + NUVIE_RAND() % (game->get_game_width() - border_width - 2);
			else
				x = game->get_game_x_offset() + NUVIE_RAND() % (game->get_game_width() - 2);
			y = game->get_game_y_offset() + NUVIE_RAND() % (game->get_game_height() - 2);
		}

		//FIXME the original does something with the palette if a pixel is black then draw gray etc.
		//We can't do this easily here because we don't have the original 8 bit display surface.
		screen->put_pixel(118, x, y);
		screen->put_pixel(118, x + 1, y + 1);
		screen->put_pixel(0, x + 2, y + 2);
	}
}

void MapWindow::AddMapTileToVisibleList(uint16 tile_num, uint16 x, uint16 y) {
	if (x >= TMP_MAP_BORDER &&
	        y >= TMP_MAP_BORDER &&
	        x < tmp_map_width - TMP_MAP_BORDER &&
	        y < tmp_map_height - TMP_MAP_BORDER) {
		TileInfo ti;
		ti.t = tile_manager->get_tile(tile_num);
		ti.x = (uint16)(x - TMP_MAP_BORDER);
		ti.y = (uint16)(y - TMP_MAP_BORDER);
		m_ViewableMapTiles.push_back(ti);
	}
}

void MapWindow::drawGrid() {
	for (uint16 i = 0; i < win_height; i++) {
		for (uint16 j = 0; j < win_width; j++) {
			screen->blit(area.left + (j * 16) - cur_x_add, area.top + (i * 16) - cur_y_add,
				(const byte *)grid_tile.data, 8, 16, 16, 16, true);
		}
	}
}

void MapWindow::generateTmpMap() {
	uint16 x, y;

	m_ViewableMapTiles.clear();

	const byte *map_ptr = map->get_map_data(cur_level);
	uint16 pitch = map->get_width(cur_level);

	if (enable_blacking == false) {
		uint16 *ptr = tmp_map_buf;
		for (y = 0; y < tmp_map_height; y++) {
			for (x = 0; x < tmp_map_width; x++) {
				uint16 x1 = cur_x + x - TMP_MAP_BORDER;
				uint16 y1 = cur_y + y - TMP_MAP_BORDER;
				WRAP_COORD(x1, cur_level);
				WRAP_COORD(y1, cur_level);
				*ptr = map_ptr[y1 * pitch + x1];
				AddMapTileToVisibleList(*ptr, x, y);

				ptr++;
			}
		}
		return;
	}

	roof_display = ROOF_DISPLAY_NORMAL;

	memset(tmp_map_buf, 0, tmp_map_width * tmp_map_height * sizeof(uint16));

	if (freeze_blacking_location == false) {
		x = cur_x + ((win_width - 1 - map_center_xoff) / 2);
		y = cur_y + ((win_height - 1) / 2);
	} else { // SB-X
		x = last_boundary_fill_x;
		y = last_boundary_fill_y;
	}

	WRAP_COORD(x, cur_level);
	WRAP_COORD(y, cur_level);

//This is for U6. Sherry needs to pass through walls
//We shift the boundary fill start location off the wall tile so it flood
//fills correctly. We move east for vertical wall tiles and south for
//horizontal wall tiles.
	if (game_type == NUVIE_GAME_U6 && obj_manager->is_boundary(x, y, cur_level)) {
		const Tile *tile = obj_manager->get_obj_tile(x, y, cur_level, false);
		if ((tile->flags1 & TILEFLAG_WALL_MASK) == (TILEFLAG_WALL_NORTH | TILEFLAG_WALL_SOUTH))
			x = WRAPPED_COORD(x + 1, cur_level);
		else
			y = WRAPPED_COORD(y + 1, cur_level);
	}
	last_boundary_fill_x = x;
	last_boundary_fill_y = y;
	boundaryFill(map_ptr, pitch, x, y);

	reshapeBoundary();

	if (roof_mode && floorTilesVisible())
		roof_display = ROOF_DISPLAY_OFF; // hide roof if a building's floor is showing.
}

void MapWindow::boundaryFill(const byte *map_ptr, uint16 pitch, uint16 x, uint16 y) {
	uint16 pos;
	uint16 tmp_x, tmp_y;

	uint16 p_cur_x = WRAPPED_COORD(cur_x - TMP_MAP_BORDER, cur_level);
	uint16 p_cur_y = WRAPPED_COORD(cur_y - TMP_MAP_BORDER, cur_level);

	if (x == WRAPPED_COORD(p_cur_x - 1, cur_level) || x == WRAPPED_COORD(p_cur_x + tmp_map_width, cur_level))
		return;

	if (y == WRAPPED_COORD(p_cur_y - 1, cur_level) || y == WRAPPED_COORD(p_cur_y + tmp_map_height, cur_level))
		return;

	if (p_cur_y > y)
		tmp_y = pitch - p_cur_y + y;
	else
		tmp_y = y - p_cur_y;

	pos = tmp_y * tmp_map_width;

	if (p_cur_x > x)
		tmp_x = pitch - p_cur_x + x;
	else
		tmp_x = x - p_cur_x;

	pos += tmp_x;

	uint16 *ptr = &tmp_map_buf[pos];

	if (*ptr != 0)
		return;

	byte current = map_ptr[y * pitch + x];

	*ptr = (uint16)current;

	AddMapTileToVisibleList(current, tmp_x, tmp_y);

	if (x_ray_view <= X_RAY_OFF && map->is_boundary(x, y, cur_level)) { //hit the boundary wall tiles
		if (boundaryLookThroughWindow(*ptr, x, y) == false)
			return;
		else
			roof_display = ROOF_DISPLAY_OFF; //hide roof tiles if player is looking through window.
	}

	uint16 xp1 = WRAPPED_COORD(x + 1, cur_level);
	uint16 xm1 = WRAPPED_COORD(x - 1, cur_level);

	uint16 yp1 = WRAPPED_COORD(y + 1, cur_level);
	uint16 ym1 = WRAPPED_COORD(y - 1, cur_level);

	boundaryFill(map_ptr, pitch, xp1,   y);
	boundaryFill(map_ptr, pitch,   x, yp1);
	boundaryFill(map_ptr, pitch, xp1, yp1);
	boundaryFill(map_ptr, pitch, xm1, ym1);
	boundaryFill(map_ptr, pitch, xm1,   y);
	boundaryFill(map_ptr, pitch,   x, ym1);
	boundaryFill(map_ptr, pitch, xp1, ym1);
	boundaryFill(map_ptr, pitch, xm1, yp1);

	return;
}

bool MapWindow::floorTilesVisible() {
	Actor *actor = actor_manager->get_player();
	if (!actor)
		return false;

	uint16 a_x, a_y;
	uint8 a_z;
	actor->get_location(&a_x, &a_y, &a_z);

	uint16 cX = WRAPPED_COORD(a_x - 1, cur_level), eX = WRAPPED_COORD(a_x + 2, cur_level);
	uint16 cY = WRAPPED_COORD(a_y - 1, cur_level), eY = WRAPPED_COORD(a_y + 2, cur_level);

	for (; cY != eY;) {
		for (; cX != eX;) {
			if (map->has_roof(cX, cY, cur_level) && !map->is_boundary(cX, cY, cur_level)) {
				const Tile *t = obj_manager->get_obj_tile(cX, cY, cur_level, false);
				if (t && (t->flags1 & TILEFLAG_WALL))
					return true;
			}
			cX = WRAPPED_COORD(cX + 1, cur_level);
		}
		cX = WRAPPED_COORD(a_x - 1, cur_level);
		cY = WRAPPED_COORD(cY + 1, cur_level);
	}

	return false;
}

bool MapWindow::boundaryLookThroughWindow(uint16 tile_num, uint16 x, uint16 y) {
	Tile *tile = tile_manager->get_tile(tile_num);
	if (!(tile->flags2 & TILEFLAG_WINDOW)) {
		Obj *obj = obj_manager->get_objBasedAt(x, y, cur_level, true);
		if (obj) { //check for a windowed object.
			tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(obj->obj_n) + obj->frame_n);
			if (!(tile->flags2 & TILEFLAG_WINDOW))
				return false;
		} else
			return false;
	}

	Actor *actor = actor_manager->get_player();

	uint16 a_x, a_y;
	uint8 a_z;
	actor->get_location(&a_x, &a_y, &a_z);

	if (a_x == x) {
		if (a_y == WRAPPED_COORD(y - 1, cur_level) || a_y == WRAPPED_COORD(y + 1, cur_level))
			return true;
	}

	if (a_y == y) {
		if (a_x == WRAPPED_COORD(x - 1, cur_level) || a_x == WRAPPED_COORD(x + 1, cur_level))
			return true;
	}

	return false;
}

//reshape walls based on new blacked out areas.

void MapWindow::reshapeBoundary() {
	for (uint16 y = 1; y < tmp_map_height - 1; y++) {
		for (uint16 x = 1; x < tmp_map_width - 1; x++) {
			if (tmpBufTileIsBoundary(x, y)) {
				const Tile *tile = tile_manager->get_tile(tmp_map_buf[y * tmp_map_width + x]);

				uint8 flag, original_flag;
				if ((tile->tile_num >= 140 && tile->tile_num <= 187)) { //main U6 wall tiles FIX for WOU games
					flag = 0;
					original_flag = tile->flags1 & TILEFLAG_WALL_MASK;
				} else
					continue;

				//generate the required wall flags
				if (tmpBufTileIsWall(x, y - 1, NUVIE_DIR_N))
					flag |= TILEFLAG_WALL_NORTH;
				if (tmpBufTileIsWall(x + 1, y, NUVIE_DIR_E))
					flag |= TILEFLAG_WALL_EAST;
				if (tmpBufTileIsWall(x, y + 1, NUVIE_DIR_S))
					flag |= TILEFLAG_WALL_SOUTH;
				if (tmpBufTileIsWall(x - 1, y, NUVIE_DIR_W))
					flag |= TILEFLAG_WALL_WEST;

				//we want to keep existing tile if it is pointing to non-wall tiles which are not blacked
				//this is used to support cookfire walls which aren't considered walls in tileflags.

				if (tmpBufTileIsBlack(x, y - 1) == false && (original_flag & TILEFLAG_WALL_NORTH))
					flag |= TILEFLAG_WALL_NORTH;
				if (tmpBufTileIsBlack(x + 1, y) == false && (original_flag & TILEFLAG_WALL_EAST))
					flag |= TILEFLAG_WALL_EAST;
				if (tmpBufTileIsBlack(x, y + 1) == false && (original_flag & TILEFLAG_WALL_SOUTH))
					flag |= TILEFLAG_WALL_SOUTH;
				if (tmpBufTileIsBlack(x - 1, y) == false && (original_flag & TILEFLAG_WALL_WEST))
					flag |= TILEFLAG_WALL_WEST;

				if (flag == 0) //isolated border tiles
					continue;

				if (flag == 48) { // 0011 top right corner
					if (tmpBufTileIsBlack(x, y - 1) && tmpBufTileIsBlack(x + 1, y)) { //replace with blacked corner tile
						//Oh dear! this is evil. FIX
						tmp_map_buf[y * tmp_map_width + x] = 266 + 2 * (((tile->tile_num - tile->tile_num % 16) - 140) / 16);
						continue;
					}
				}

				if (flag == 192) { // 1100 bottom left corner
					if (tmpBufTileIsBlack(x, y + 1) && tmpBufTileIsBlack(x - 1, y)) { //replace with blacked corner tile
						//Oh dear! this is evil. FIX
						tmp_map_buf[y * tmp_map_width + x] = 266 + 1 + 2 * (((tile->tile_num - tile->tile_num % 16) - 140) / 16);
						continue;
					}
				}

				if ((tile->flags1 & TILEFLAG_WALL_MASK) == flag) // complete match no work needed
					continue;

				// Look for a suitable tile to transform into

				// ERIC 05/03/05 flag |= TILEFLAG_WALL_NORTH | TILEFLAG_WALL_WEST;

				if (((tile->flags1) & TILEFLAG_WALL_MASK) > flag && flag != 144) { // 1001 _| corner
					//flag |= TILEFLAG_WALL_NORTH | TILEFLAG_WALL_WEST;
					for (; ((tile->flags1) & TILEFLAG_WALL_MASK) != flag && (tile->flags1 & TILEFLAG_WALL_MASK);)
						tile = tile_manager->get_tile(tile->tile_num - 1);
				} else {
					//flag |= TILEFLAG_WALL_NORTH | TILEFLAG_WALL_WEST;
					for (; ((tile->flags1) & TILEFLAG_WALL_MASK) != flag && (tile->flags1 & TILEFLAG_WALL_MASK);)
						tile = tile_manager->get_tile(tile->tile_num + 1);
				}

				if ((tile->flags1 & TILEFLAG_WALL_MASK) == flag)
					tmp_map_buf[y * tmp_map_width + x] = tile->tile_num;
			}
		}
	}
}

inline bool MapWindow::tmpBufTileIsBlack(uint16 x, uint16 y) const {
	if (tmp_map_buf[y * tmp_map_width + x] == 0)
		return true;

	return false;
}

bool MapWindow::tmpBufTileIsBoundary(uint16 x, uint16 y) {
	uint16 tile_num = tmp_map_buf[y * tmp_map_width + x];

	if (tile_num == 0)
		return false;

	const Tile *tile = tile_manager->get_tile(tile_num);

	if (tile->boundary)
		return true;

	if (obj_manager->is_boundary(WRAPPED_COORD(cur_x - TMP_MAP_BORDER + x, cur_level), WRAPPED_COORD(cur_y - TMP_MAP_BORDER + y, cur_level), cur_level))
		return true;

	return false;
}

bool MapWindow::tmpBufTileIsWall(uint16 x, uint16 y, NuvieDir direction) {
	uint16 tile_num = tmp_map_buf[y * tmp_map_width + x];

	if (tile_num == 0)
		return false;

	uint8 mask = 0;
	switch (direction) {
	case NUVIE_DIR_N :
		mask = TILEFLAG_WALL_SOUTH;
		break;
	case NUVIE_DIR_S :
		mask = TILEFLAG_WALL_NORTH;
		break;
	case NUVIE_DIR_E :
		mask = TILEFLAG_WALL_WEST;
		break;
	case NUVIE_DIR_W :
		mask = TILEFLAG_WALL_EAST;
		break;
	default:
		error("invalid direction in MapWindow::tmpBufferIsWall");
	}

	const Tile *tile = tile_manager->get_tile(tile_num);

	if (tile->flags1 & TILEFLAG_WALL) {
		if (tile->flags1 & mask)
			return true;
	}

// if(obj_manager->is_boundary(cur_x-1+x, cur_y-1+y, cur_level))
//  return true;

	tile = obj_manager->get_obj_tile(WRAPPED_COORD(cur_x - TMP_MAP_BORDER + x, cur_level), WRAPPED_COORD(cur_y - TMP_MAP_BORDER + y, cur_level), cur_level, false);
	if (tile != nullptr) {
		if (tile->flags2 & TILEFLAG_BOUNDARY) {
			if (tile->flags1 & mask)
				return true;
		}
	}

	return false;
}

/* Returns MSG_SUCCESS if the obj can be dropped or moved by the actor at world coordinates x,y or an error msg.
 * There must be a direct path between the actor to the obj and also between the obj and destination.
 * Objs that can't be picked up will be blocked actors and unpassable areas. (z is always cur_level)
 */
CanDropOrMoveMsg MapWindow::can_drop_or_move_obj(uint16 x, uint16 y, Actor *actor, Obj *obj) {
	bool in_inventory = obj->is_in_inventory();
	if (!in_inventory && obj->x == x && obj->y == y)
		return MSG_NOT_POSSIBLE;
	if (game->using_hackmove())
		return MSG_SUCCESS;

	if (tile_is_black(x, y, obj)) {
		if (tile_is_black(x, y))
			return MSG_NOT_POSSIBLE;
		else
			return MSG_BLOCKED;
	}

	MapCoord actor_loc = actor->get_location();

	// Can only drop onto non-blocking actors
	// Message: "Blocked" if pushing, "Not possible" if dropping
	if (actor_manager->findActorAt(x, y, actor_loc.z, [](const Actor *a) {return !a->isNonBlocking();}, true, false))
		return in_inventory ? MSG_NOT_POSSIBLE : MSG_BLOCKED;

	Obj *dest_obj = nullptr;
	if (game_type == NUVIE_GAME_U6) {
		dest_obj = obj_manager->get_obj(x, y, actor_loc.z); //FIXME this might not be right. We might want to exclude obj.
	} else {
		dest_obj = obj_manager->get_obj(x, y, actor_loc.z, OBJ_SEARCH_TOP, OBJ_EXCLUDE_IGNORED, obj);
	}

	bool can_go_in_water = (game_type == NUVIE_GAME_U6
	                        && (obj->obj_n == OBJ_U6_SKIFF || obj->obj_n == OBJ_U6_RAFT));
	if (can_go_in_water && dest_obj) // it is drawn underneath so only allow on hackmove
		return MSG_NOT_POSSIBLE;

	LineTestResult lt;
	MapCoord target_loc(x, y, actor_loc.z);
	MapCoord obj_loc(obj->x, obj->y, actor_loc.z);

	if (in_inventory && !obj->get_actor_holding_obj()->is_onscreen()
	        && obj->get_actor_holding_obj()->get_location().distance(target_loc) > 5)
		return MSG_OUT_OF_RANGE;

	if (get_interface() == INTERFACE_IGNORE_BLOCK && map->can_put_obj(x, y, cur_level))
		return MSG_SUCCESS;

	if (actor_loc.distance(target_loc) > 5 && get_interface() == INTERFACE_NORMAL)
		return MSG_OUT_OF_RANGE;

	bool blocked = false;
	uint8 lt_flags = (game_type == NUVIE_GAME_U6) ? LT_HitMissileBoundary : 0; //FIXME this probably isn't quite right for MD/SE
	if (map->lineTest(actor_loc.x, actor_loc.y, x, y, actor_loc.z, lt_flags, lt, 0, obj)) {
		MapCoord hit_loc = MapCoord(lt.hit_x, lt.hit_y, lt.hit_level);
		if (in_inventory || obj_loc.distance(target_loc) != 1 || hit_loc.distance(target_loc) != 1)
			blocked = true;  // Just set a bool and don't return yet: blocker might be a suitable container.
		else // trying to push object one tile away from actor
			blocked = map->lineTest(obj->x, obj->y, x, y, actor_loc.z, lt_flags, lt, 0, obj);
	}

	const Obj* potentialContainer = nullptr;
	if (blocked) {
		if (lt.hitObj && MapCoord(lt.hitObj) == target_loc)
			potentialContainer = lt.hitObj;
	} else
		potentialContainer = dest_obj;

	if (potentialContainer && obj_manager->can_store_obj(potentialContainer, obj)) //if we are moving onto a container.
		return MSG_SUCCESS;

	if (blocked)
		return MSG_BLOCKED;

	const Tile *tile;
	if (dest_obj)
		tile = obj_manager->get_obj_tile(dest_obj->obj_n, dest_obj->frame_n);
	else
		tile = map->get_tile(x, y, actor_loc.z);

	if (!tile) // shouldn't happen
		return MSG_NO_TILE;

	if ((can_go_in_water || !map->is_water(x, y, actor_loc.z))
	        && ((tile->flags3 & TILEFLAG_CAN_PLACE_ONTOP)
	            || (tile->passable && !map->is_boundary(x, y, actor_loc.z))))
		return MSG_SUCCESS;

	return MSG_NOT_POSSIBLE;
}

void MapWindow::display_can_drop_or_move_msg(CanDropOrMoveMsg msg, string msg_text) {
	if (msg == MSG_NOT_POSSIBLE)
		msg_text += "Not possible\n";
	else if (msg == MSG_BLOCKED)
		msg_text += "Blocked\n";
	else if (msg == MSG_OUT_OF_RANGE)
		msg_text += "Out of range\n";
	/*  else if(msg == MSG_NO_TILE) // shouldn't be needed now that blacked out areas are checked first
	        msg_text += "ERROR: No tile. Report me\n";*/
	game->get_scroll()->display_string(msg_text);
}

bool MapWindow::can_get_obj(const Actor *actor, Obj *obj) {
	if (!obj)
		return false;
	if (get_interface() == INTERFACE_IGNORE_BLOCK)
		return true;
	if (obj->is_in_inventory())
		return false;
	if (obj->is_in_container())
		obj = obj->get_container_obj(true);

	if (actor->get_z() != obj->z)
		return false;

	LineTestResult lt;
	if (map->lineTest(actor->get_x(), actor->get_y(), obj->x, obj->y, obj->z, LT_HitUnpassable, lt, 0, obj)) {
		// Skip Check for SE Tile Objects - We are actually using the blocking item/tree
		Script *script = game->get_script();
		if (game_type != NUVIE_GAME_SE || !script->call_is_tile_object(obj->obj_n)) {
			return false;
		}
	}

	if (game_type == NUVIE_GAME_U6 && obj->obj_n == OBJ_U6_SECRET_DOOR)
		return true;
	return !blocked_by_wall(actor, obj);
}

/*
 * Check to make sure the obj isn't on the other side of a wall or trying to push through it.
 * The original engine didn't bother to check.
 */
bool MapWindow::blocked_by_wall(const Actor *actor, const Obj *obj) {
	if (game_type == NUVIE_GAME_U6 && obj->x == 282 && obj->y == 438 && cur_level == 0) // HACK for buggy location
		return false;
	const Tile *tile = map->get_tile(obj->x, obj->y, cur_level);
	if (((tile->flags1 & TILEFLAG_WALL) && !game->get_usecode()->is_door(obj))
	        && (((tile->flags1 & TILEFLAG_WALL_MASK) == 208 && actor->get_y() < obj->y) // can't get items that are south
	            || ((tile->flags1 & TILEFLAG_WALL_MASK) == 176 && actor->get_x() < obj->x) // can't get items that are east
	            || ((tile->flags1 & TILEFLAG_WALL_MASK) == 240 // northwest corner - used in SE (not sure if used in other games)
	                && (actor->get_y() < obj->y || actor->get_x() < obj->x))))
		return true;

	return false;
}

bool MapWindow::drag_accept_drop(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "MapWindow::drag_accept_drop()\n");

	x -= area.left;
	y -= area.top;

	x /= 16;
	y /= 16;

	GUI::get_gui()->force_full_redraw();

	if (message == GUI_DRAG_OBJ) {
		if (game->get_player()->is_in_vehicle() && !game->using_hackmove()) {
			game->get_event()->display_not_aboard_vehicle();
			return false;
		}
		uint16 mapWidth = map->get_width(cur_level);
		x = (cur_x + x) % mapWidth;
		y = (cur_y + y) % mapWidth;

		Obj *obj = (Obj *)data;
		Actor *p = actor_manager->get_player();
		Actor *target_actor = map->get_actor(x, y, cur_level);

		if (obj->is_in_inventory() == false) { //obj on map.
			if (can_get_obj(p, obj)) { //make sure there is a clear line from player to object
				if (target_actor) {
					game->get_event()->display_move_text(target_actor, obj);

					if (target_actor == p || (target_actor->is_in_party())) {
						if (obj_manager->obj_is_damaging(obj, p)) {
							game->get_player()->subtract_movement_points(3);
							return false;
						}
						if ((!game->get_usecode()->has_getcode(obj) || game->get_usecode()->get_obj(obj, target_actor))
						        && game->get_event()->can_move_obj_between_actors(obj, p, target_actor))
							return true;
						else {
							game->get_scroll()->message("\n\n");
							return false;
						}
					} else {
						game->get_scroll()->display_string("\n\nOnly within the party!");
						game->get_scroll()->message("\n\n");
						return false;
					}
				} else
					return true;
			}
		} else {
			if (game->get_usecode()->cannot_unready(obj)) {
				game->get_event()->unready(obj);
				return false;
			}
			if (target_actor) {
				Actor *owner = obj->get_actor_holding_obj();
				game->get_event()->display_move_text(target_actor, obj);

				if (game->get_event()->can_move_obj_between_actors(obj, owner, target_actor) == false) {
					game->get_scroll()->message("\n\n");
					return false;
				} else
					return true;
			} else
				return true; //throw on ground
		}
		game->get_scroll()->display_string("Move-");
		game->get_scroll()->display_string(obj_manager->look_obj(obj)); // getting obj name
		game->get_scroll()->display_string("\nto ");
		game->get_scroll()->display_string(get_direction_name(x - obj->x ,  y - obj->y));
		game->get_scroll()->message(".\n\nCan't reach it\n\n");
	}

	return false;
}

void MapWindow::drag_perform_drop(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "MapWindow::drag_perform_drop()\n");
	Events *event = game->get_event();
	uint16 mapWidth = map->get_width(cur_level);

	x -= area.left;
	y -= area.top;

	if (message == GUI_DRAG_OBJ) {
		x = (uint)(cur_x + x / 16) % mapWidth;
		y = (uint)(cur_y + y / 16) % mapWidth;
		Obj *obj = (Obj *)data;

		if (obj->obj_n == OBJ_U6_LOCK_PICK && game_type == NUVIE_GAME_U6)
			game->get_usecode()->search_container(obj, false);
		Actor *a = map->get_actor(x, y, cur_level);
		if (a && (a->is_in_party() || a == actor_manager->get_player())) {
			if (a == actor_manager->get_player()) // get
				game->get_player()->subtract_movement_points(3);
			else // get plus move
				game->get_player()->subtract_movement_points(8);
			obj_manager->moveto_inventory(obj, a);
			game->get_scroll()->message("\n\n");
		} else {
			if (!obj->is_in_inventory() && !obj->is_in_container()) { // !need is_in_container to exclude container gumps
				move_on_drop(obj); // no longer determines whether to drop or move but can call usecode
				event->newAction(PUSH_MODE);
				event->select_obj(obj);
				event->pushTo(x - obj->x, y - obj->y, PUSH_FROM_OBJECT);
				event->endAction();
				return;
			}
			CanDropOrMoveMsg can_drop; // so we can skip quantity prompt
			if ((can_drop = can_drop_or_move_obj(x, y, actor_manager->get_player(), obj)) != MSG_SUCCESS) {
				game->get_scroll()->display_string("Drop-");
				game->get_scroll()->display_string(obj_manager->look_obj(obj));
				game->get_scroll()->display_string("\n\nlocation:\n\n");
				display_can_drop_or_move_msg(can_drop, "");
				game->get_scroll()->message("\n");
				return;
			}
			// drop on ground or into a container
			event->newAction(DROP_MODE);
			event->select_obj(obj);
			if (obj->qty <= 1 || !obj_manager->is_stackable(obj))
				event->select_target(x, y);
			else
				event->set_drop_target(x, y); // pre-select target
		}
	}

}
// performs some usecode but used to decide whether to move or drop too
bool MapWindow::move_on_drop(Obj *obj) {
	bool move = (get_interface() == INTERFACE_NORMAL);
	/*  if(drop_with_move && move)
	        return true;

	    if(obj_manager->obj_is_damaging(obj))
	        return move;
	*/
	if (game->get_usecode()->has_getcode(obj) && obj->is_in_inventory() == false) {
		if (game_type == NUVIE_GAME_U6) {
			switch (obj->obj_n) {
			case OBJ_U6_CHEST:
			case OBJ_U6_LOCK_PICK:
			case OBJ_U6_MOONSTONE:
				game->get_usecode()->get_obj(obj, actor_manager->get_player());
				return false;
			case OBJ_U6_SKIFF:
				return false;
			case OBJ_U6_TORCH:
				if (obj->frame_n == 0)
					return false;
				break;
			default :
				break;
			}
		}

		return move;
	}

	return false;
}

void MapWindow::set_interface() {
	Std::string interface_str;
	config->value("config/input/interface", interface_str, "normal");
	if (interface_str == "ignore_block" ||  Game::get_game()->using_hackmove()) // game variable is not initialized
		interface = INTERFACE_IGNORE_BLOCK;
	else if (interface_str == "fullscreen")
		interface = INTERFACE_FULLSCREEN;
	else
		interface = INTERFACE_NORMAL;
}

InterfaceType MapWindow::get_interface() {
	// check is easily exploited but would be annoying if checking for nearby enemies
	if (interface == INTERFACE_FULLSCREEN && game->get_party()->is_in_combat_mode())
		return INTERFACE_NORMAL;
	return interface;
}

bool MapWindow::is_interface_fullscreen_in_combat() {
	if (interface == INTERFACE_FULLSCREEN && game->get_party()->is_in_combat_mode())
		return true;
	return false;
}

GUI_status MapWindow::Idle(void) {
	return (GUI_Widget::Idle());
}


// single-click (press and release button)
GUI_status MapWindow::MouseClick(int x, int y, Shared::MouseButton button) {
	if (button == USE_BUTTON && look_on_left_click) {
		wait_for_mouseclick(button); // see MouseDelayed
	}
	return (MouseUp(x, y, button)); // do MouseUp so selected_obj is cleared
}

// single-click; waited for double-click
GUI_status MapWindow::MouseDelayed(int x, int y, Shared::MouseButton button) {
	Events *event = game->get_event();
	if (!looking || game->user_paused() || event->cursor_mode
	        || (event->get_mode() != MOVE_MODE && event->get_mode() != EQUIP_MODE)) {
		look_obj = nullptr;
		look_actor = nullptr;
		return GUI_PASS;
	}
	game->get_scroll()->display_string("Look-");
	event->set_mode(LOOK_MODE);
	event->lookAtCursor(true, original_obj_loc.x, original_obj_loc.y, original_obj_loc.z, look_obj, look_actor);
	look_obj = nullptr;
	look_actor = nullptr;

	return (MouseUp(x, y, button)); // do MouseUp so selected_obj is cleared
}

// MouseDown; waited for MouseUp
GUI_status MapWindow::MouseHeld(int x, int y, Shared::MouseButton button) {
	looking = false;
	if (walk_with_left_button)
		set_walking(true);
	return GUI_PASS;
}

// double-click
GUI_status MapWindow::MouseDouble(int x, int y, Shared::MouseButton button) {
	Events *event = game->get_event();

	// only USE if not doing anything in event
	if (enable_doubleclick && event->get_mode() == MOVE_MODE && !is_wizard_eye_mode()) {
		int wx, wy;
		mouseToWorldCoords(x, y, wx, wy);
		event->multiuse((uint16)wx, (uint16)wy);
	}
	looking = false;
	return (MouseUp(x, y, button)); // do MouseUp so selected_obj is cleared
}

GUI_status MapWindow::MouseWheel(sint32 x, sint32 y) {
	Game *g = Game::get_game();

	if (g->is_new_style()) {
		if (y > 0)
			g->get_scroll()->move_scroll_up();
		if (y < 0)
			g->get_scroll()->move_scroll_down();
	} else {
		if (y > 0)
			g->get_scroll()->page_up();
		if (y < 0)
			g->get_scroll()->page_down();
	}
	return GUI_YUM;
}

GUI_status MapWindow::MouseDown(int x, int y, Shared::MouseButton button) {
	//DEBUG(0,LEVEL_DEBUGGING,"MapWindow::MouseDown, button = %i\n", button);
	Events *event = game->get_event();
	Actor *player = actor_manager->get_player();
	Obj *obj = get_objAtMousePos(x, y);

	if (is_wizard_eye_mode()) {
		set_walking(true);
		return GUI_YUM;
	}
	if (event->is_looking_at_spellbook()) {
		event->cancelAction();
		return GUI_YUM;
	}

	if (game->is_original_plus() && y <= Game::get_game()->get_game_y_offset() + 200
	        && x >= Game::get_game()->get_game_x_offset() + game->get_game_width() - border_width) {
		looking = false;
		return GUI_PASS;
	}
	if (event->get_mode() == MOVE_MODE || event->get_mode() == EQUIP_MODE) {
		int wx, wy;
		mouseToWorldCoords(x, y, wx, wy);

		if (button == WALK_BUTTON && game->get_command_bar()->get_selected_action() != -1) {
			if (game->get_command_bar()->try_selected_action() == false) // start new action
				return GUI_PASS; // false if new event doesn't need target
		} else if (wx == player->x && wy == player->y // PASS if Avatar is hit
		           && (button == WALK_BUTTON || !enable_doubleclick)) {
			event->cancelAction(); // MOVE_MODE, so this should work
			return GUI_PASS;
		} else if (button == WALK_BUTTON
		           || (!enable_doubleclick && button == USE_BUTTON
		               && !game->is_dragging_enabled() && !look_on_left_click)) {
			set_walking(true);
		} else if (button == USE_BUTTON) { // you can also walk by holding the USE button
			if (look_on_left_click && !event->cursor_mode) { // need to preserve location because of click delay
				looking = true;
				original_obj_loc = MapCoord(wx, wy , cur_level);
				look_actor = actor_manager->get_actor(wx , wy, cur_level);
				look_obj = obj_manager->get_obj(wx , wy, cur_level);
				moveCursor(WRAP_VIEWP(cur_x, wx, map_width), wy - cur_y);
			}
			wait_for_mousedown(button);
		}
	}

	if (event->get_mode() == INPUT_MODE || event->get_mode() == ATTACK_MODE) { // finish whatever action is being done, with mouse coordinates
		if (button != USE_BUTTON && button != WALK_BUTTON)
			return GUI_PASS;
		looking = false;
		select_target(x, y);
		return  GUI_PASS;
	} else if (event->get_mode() != MOVE_MODE && event->get_mode() != EQUIP_MODE) {
		return GUI_PASS;
	}

	if (!obj || button != DRAG_BUTTON)
		return  GUI_PASS;

	original_obj_loc = MapCoord(obj->x, obj->y, obj->z);
	int distance = player->get_location().distance(original_obj_loc);
	float weight = obj_manager->get_obj_weight(obj, OBJ_WEIGHT_EXCLUDE_CONTAINER_ITEMS);

	if ((weight == 0 || player->get_actor_num() == 0
	        || tile_is_black(obj->x, obj->y, obj)) && !game->using_hackmove())
		return  GUI_PASS;

	// checking interface directly to allow dragging in INTERFACE_FULLSCREEN when in combat
	if (distance > 1 && interface == INTERFACE_NORMAL)
		return  GUI_PASS;

	if (button == DRAG_BUTTON && game->is_dragging_enabled())
		selected_obj = obj;

	return  GUI_PASS;
}

GUI_status MapWindow::MouseUp(int x, int y, Shared::MouseButton button) {
	// cancel dragging and movement no matter what button is released
	if (selected_obj) {
		selected_obj = nullptr;
	}
	walking = false;
	dragging = false;

	return  GUI_PASS;
}

GUI_status  MapWindow::MouseMotion(int x, int y, uint8 state) {
//	Events *event = game->get_event();
	Tile    *tile;

	update_mouse_cursor((uint32)x, (uint32)y);

	//  DEBUG(0,LEVEL_DEBUGGING,"MapWindow::MouseMotion\n");

//	if(selected_obj) // We don't want to walk if we are selecting an object to move.
//		walking = false;
	if (walking) { // No, we don't want to select an object to move if we are walking.
		selected_obj = nullptr;
		dragging = false;
	}

	if (selected_obj && !dragging) {
		int wx, wy;
		// ensure that the player can reach the selected object before
		// letting them drag it
		//mouseToWorldCoords(x, y, wx, wy);
		wx = selected_obj->x;
		wy = selected_obj->y;
		LineTestResult result;
		Actor *player = actor_manager->get_player();

		if (map->lineTest(player->x, player->y, wx, wy, cur_level, LT_HitUnpassable, result)
		        && !(result.hitObj && result.hitObj->x == wx && result.hitObj->y == wy)
		        && get_interface() == INTERFACE_NORMAL)
			// something was in the way, so don't allow a drag
			return GUI_PASS;
		dragging = true;
		set_mousedown(0, DRAG_BUTTON); // cancel MouseHeld
		game->set_mouse_pointer(0); // arrow
		tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(selected_obj->obj_n) + selected_obj->frame_n);
		bool out_of_range;
		if (is_interface_fullscreen_in_combat() && player->get_location().distance(original_obj_loc) > 1)
			out_of_range = true;
		else
			out_of_range = false;
		return gui_drag_manager->start_drag(this, GUI_DRAG_OBJ, selected_obj, tile->data, 16, 16, 8, out_of_range);
	}

	return  GUI_PASS;
}

void    MapWindow::drag_drop_success(int x, int y, int message, void *data) {
	//DEBUG(0,LEVEL_DEBUGGING,"MapWindow::drag_drop_success\n");
	dragging = false;

// handled by drop target
//	if (selected_obj)
//		obj_manager->remove_obj (selected_obj);

	selected_obj = nullptr;
	Redraw();
}

void    MapWindow::drag_drop_failed(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "MapWindow::drag_drop_failed\n");
	dragging = false;
	selected_obj = nullptr;
}

// this does nothing
GUI_status MapWindow::KeyDown(const Common::KeyState &key) {
	if (is_wizard_eye_mode()) {
		KeyBinder *keybinder = Game::get_game()->get_keybinder();
		ActionType a = keybinder->get_ActionType(key);
		switch (keybinder->GetActionKeyType(a)) {
		case WEST_KEY:
			moveMapRelative(-1, 0);
			break;
		case EAST_KEY:
			moveMapRelative(1, 0);
			break;
		case SOUTH_KEY:
			moveMapRelative(0, 1);
			break;
		case NORTH_KEY:
			moveMapRelative(0, -1);
			break;
		case NORTH_EAST_KEY:
			moveMapRelative(1, -1);
			break;
		case SOUTH_EAST_KEY:
			moveMapRelative(1, 1);
			break;
		case NORTH_WEST_KEY:
			moveMapRelative(-1, -1);
			break;
		case SOUTH_WEST_KEY:
			moveMapRelative(-1, 1);
			break;
		case CANCEL_ACTION_KEY:
			wizard_eye_stop();
			break;
		default:
			keybinder->handle_always_available_keys(a);
			return GUI_YUM;
		}
		if (keybinder->GetActionKeyType(a) <= SOUTH_WEST_KEY)
			wizard_eye_update();
		return GUI_YUM;
	}
	return GUI_PASS;
}

Obj *MapWindow::get_objAtMousePos(int mx, int my) {
	int wx, wy;
	mouseToWorldCoords(mx, my, wx, wy);

	return  obj_manager->get_obj(wx, wy, cur_level);
}


Actor *MapWindow::get_actorAtMousePos(int mx, int my) {
	int wx, wy;
	mouseToWorldCoords(mx, my, wx, wy);

	return  actor_manager->get_actor(wx, wy, cur_level);
}

void MapWindow::teleport_to_cursor() {
	int mx, my, wx, wy;
	screen->get_mouse_location(&mx, &my);

	mouseToWorldCoords(mx, my, wx, wy);
	game->get_player()->move(wx, wy, cur_level, true);
}

void MapWindow::select_target(int x, int y) {
	int wx, wy;
	mouseToWorldCoords(x, y, wx, wy);
	moveCursor(WRAPPED_COORD(wx - cur_x, cur_level), WRAPPED_COORD(wy - cur_y, cur_level));
	game->get_event()->select_target(uint16(wx), uint16(wy), cur_level);
}

void MapWindow::mouseToWorldCoords(int mx, int my, int &wx, int &wy) {
	int x = mx - area.left;
	int y = my - area.top;

	int mapWidth = map->get_width(cur_level);

	wx = (uint)(cur_x + x / 16) % mapWidth;
	wy = (uint)(cur_y + y / 16) % mapWidth;
}

void MapWindow::drag_draw(int x, int y, int message, void *data) {
	Tile *tile;

	if (!selected_obj)
		return;

	tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(selected_obj) + selected_obj->frame_n);

	int nx = x - 8;
	int ny = y - 8;

	if (nx + 16 >= screen->get_width())
		nx = screen->get_width() - 17;
	else if (nx < 0)
		nx = 0;

	if (ny + 16 >= screen->get_height())
		ny = screen->get_height() - 17;
	else if (ny < 0)
		ny = 0;

	screen->blit(nx, ny, tile->data, 8, 16, 16, 16, true);
	screen->update(nx, ny, 16, 16);
}


/* Display MapWindow animations. */
void MapWindow::drawAnims(bool top_anims) {
	if (!screen) // screen should be set early on
		return;
	else if (!anim_manager->get_surface()) // screen must be assigned to AnimManager
		anim_manager->set_surface(screen);
	anim_manager->display(top_anims);
}


/* Set mouse pointer to a movement-arrow for walking, or a crosshair. */
void MapWindow::update_mouse_cursor(uint32 mx, uint32 my) {
	Events *event = game->get_event();
	int wx = 0, wy = 0;
	sint16 rel_x = 0, rel_y = 0;
	uint8 mptr = 0; // mouse-pointer is set here in get_movement_direction()

	if (event->get_mode() != MOVE_MODE && event->get_mode() != INPUT_MODE)
		return;

	// MousePos->WorldCoord->Direction&MousePointer
	if (game->is_orig_style())
		mouseToWorldCoords((int)mx, (int)my, wx, wy);
	get_movement_direction((uint16)mx, (uint16)my, rel_x, rel_y, &mptr);
	if (event->get_mode() == INPUT_MODE && mousecenter_x == (win_width / 2) && mousecenter_y == (win_height / 2)
	        && !event->dont_show_target_cursor())
		game->set_mouse_pointer(1); // crosshairs
	else if (dragging || (game->is_orig_style() && (wx == cur_x || wy == cur_y || wx == WRAP_VIEWP(cur_x, win_width - 1, map_width) || wy == (cur_y + win_height - 1)))
	         || (game->is_original_plus() && (my <= (uint32)Game::get_game()->get_game_y_offset() + 200 || game->is_original_plus_cutoff_map())
	             && mx >= (uint32)Game::get_game()->get_game_x_offset() + game->get_game_width() - border_width))
		game->set_mouse_pointer(0); // arrow
	else
		game->set_mouse_pointer(mptr); // 1=crosshairs, 2to9=arrows
}

/* Get relative movement direction from the MouseCenter coordinates to the
 * mouse coordinates mx,my, for walking with the mouse, etc. The mouse-pointer
 * number that should be used for that direction will be set to mptr.
 */
void MapWindow::get_movement_direction(uint16 mx, uint16 my, sint16 &rel_x, sint16 &rel_y, uint8 *mptr) {
	uint16 cent_x = mousecenter_x,
	       cent_y = mousecenter_y;
	if (game->is_original_plus_full_map() && game->get_event()->get_mode() != INPUT_MODE)
		cent_x -= (map_center_xoff + 1) / 2; // player is off center

	mx = (mx - area.left) / 16;
	my = (my - area.top) / 16;
	uint16 dist_x = abs(mx - cent_x), dist_y = abs(my - cent_y);

	rel_x = rel_y = 0;
	if (dist_x <= 4 && dist_y <= 4) {
		// use mapwindow coords (4,4 is center of mapwindow)
		uint8 cursor_num = movement_array[(9 * (4 + (my - cent_y))) + (4 + (mx - cent_x))];
		if (mptr) // set mouse-pointer number
			*mptr = cursor_num;
		if (cursor_num == 1) // nowhere
			return;
		if (cursor_num == 2) // up
			rel_y = -1;
		else if (cursor_num == 6) // down
			rel_y = 1;
		else if (cursor_num == 8) // left
			rel_x = -1;
		else if (cursor_num == 4) // right
			rel_x = 1;
		else if (cursor_num == 3) { // up-right
			rel_x = 1;
			rel_y = -1;
		} else if (cursor_num == 5) { // down-right
			rel_x = 1;
			rel_y = 1;
		} else if (cursor_num == 7) { // down-left
			rel_x = -1;
			rel_y = 1;
		} else if (cursor_num == 9) { // up-left
			rel_x = -1;
			rel_y = -1;
		}
	} else { // mapwindow is larger than the array; use 4 squares around center array
		if (dist_x <= 4 && my < cent_y) { // up
			rel_y = -1;
			if (mptr) *mptr = 2;
		} else if (dist_x <= 4 && my > cent_y) { // down
			rel_y = 1;
			if (mptr) *mptr = 6;
		} else if (mx < cent_x && dist_y <= 4) { // left
			rel_x = -1;
			if (mptr) *mptr = 8;
		} else if (mx > cent_x && dist_y <= 4) { // right
			rel_x = 1;
			if (mptr) *mptr = 4;
		} else if (mx > cent_x && my < cent_y) { // up-right
			rel_x = 1;
			rel_y = -1;
			if (mptr) *mptr = 3;
		} else if (mx > cent_x && my > cent_y) { // down-right
			rel_x = 1;
			rel_y = 1;
			if (mptr) *mptr = 5;
		} else if (mx < cent_x && my > cent_y) { // down-left
			rel_x = -1;
			rel_y = 1;
			if (mptr) *mptr = 7;
		} else if (mx < cent_x && my < cent_y) { // up-left
			rel_x = -1;
			rel_y = -1;
			if (mptr) *mptr = 9;
		}
	}
}


/* Revert mouse cursor to normal arrow. Stop walking. */
GUI_status MapWindow::MouseLeave(uint8 state) {
	if (game_type == NUVIE_GAME_MD) // magnifying glass - pointer 0 should be used too for some areas
		game->set_mouse_pointer(1);
	else
		game->set_mouse_pointer(0);
	walking = false;
	dragging = false;
	// NOTE: Don't clear selected_obj here! It's used to remove the object after
	// dragging.
	return GUI_PASS;
}

byte *MapWindow::make_thumbnail() {
	if (thumbnail)
		return nullptr;

	new_thumbnail = true;

	GUI::get_gui()->Display(); // this calls MapWindow::display() which in turn calls create_thumbnail(). :-)

	return thumbnail;
}

void MapWindow::create_thumbnail() {
	Common::Rect src_rect;

	src_rect.setWidth(MAPWINDOW_THUMBNAIL_SIZE * MAPWINDOW_THUMBNAIL_SCALE);
	src_rect.setHeight(src_rect.width());

	src_rect.left = area.left + win_width * 8 - (src_rect.width() / 2); // area.left + (win_width * 16) / 2 - 120 / 2
	src_rect.top = area.top + win_height * 8 - (src_rect.height() / 2); // area.top + (win_height * 16) / 2 - 120 / 2

	thumbnail = screen->copy_area(&src_rect, MAPWINDOW_THUMBNAIL_SCALE); //scale down x3

	new_thumbnail = false;
}

void MapWindow::free_thumbnail() {
	if (thumbnail) {
		delete[] thumbnail;
		thumbnail = nullptr;
	}

	return;
}


/* Returns a new 8bit copy of the mapwindow as displayed. Caller must free it. */
Graphics::ManagedSurface *MapWindow::get_sdl_surface() {
	return (get_sdl_surface(0, 0, area.width(), area.height()));
}

Graphics::ManagedSurface *MapWindow::get_sdl_surface(uint16 x, uint16 y, uint16 w, uint16 h) {
	Graphics::ManagedSurface *new_surface = nullptr;
	byte *screen_area;
	Common::Rect copy_area(area.left + x, area.top + y, area.left + x + w, area.top + y + h);

	GUI::get_gui()->Display();
	screen_area = screen->copy_area(&copy_area);

	new_surface = screen->create_sdl_surface_8(screen_area, copy_area.width(), copy_area.height());
// new_surface = screen->create_sdl_surface_from(screen_area, screen->get_bpp(),
//                                               copy_area.w, copy_area.h,
//                                               copy_area.w);
	free(screen_area);
	return new_surface;
}

/* Returns the overlay surface. A new 8bit overlay is created if necessary. */
Graphics::ManagedSurface *MapWindow::get_overlay() {
	if (!overlay)
		overlay = new Graphics::ManagedSurface(area.width(), area.height(),
			Graphics::PixelFormat::createFormatCLUT8());

	return overlay;
}

/* Set the overlay surface. The current overlay is deleted if necessary. */
void MapWindow::set_overlay(Graphics::ManagedSurface *surfpt) {
	if (overlay && (overlay != surfpt))
		delete overlay;
	overlay = surfpt;
}

/* Returns true if town tiles are within 5 tiles of the player */
bool MapWindow::in_town() const {
	const MapCoord player_loc = actor_manager->get_player()->get_location();

	for (const TileInfo &ti : m_ViewableMapTiles)
		if (MapCoord(ti.x + cur_x, ti.y + cur_y, cur_level).distance(player_loc) <= 5 && // make sure tile is close enough
		        (ti.t->flags1 & TILEFLAG_WALL) && (ti.t->flags1 & TILEFLAG_WALL_MASK)) { //only wall tiles with wall direction bits set.
			return true;
		}
	return false;
}

void MapWindow::wizard_eye_start(const MapCoord &location, uint16 duration, CallBack *caller) {
	wizard_eye_info.moves_left = duration;
	wizard_eye_info.caller = caller;

	wizard_eye_info.prev_x = cur_x;
	wizard_eye_info.prev_y = cur_y;

	set_x_ray_view(X_RAY_ON);
	sint16 map_x = location.x - (win_width / 2);
	if (game->is_original_plus_full_map())
		map_x += ((map_center_xoff + 1) / 2);
	moveMap(map_x, location.y - (win_height / 2) , cur_level); // FIXME - map should already be centered on the caster so why are we doing this?
	grab_focus();
}

void MapWindow::wizard_eye_stop() {
	if (wizard_eye_info.moves_left > 0) {
		wizard_eye_info.moves_left = 0;
		wizard_eye_update();
	}
}

void MapWindow::wizard_eye_update() {
	if (wizard_eye_info.moves_left > 0)
		wizard_eye_info.moves_left--;

	if (wizard_eye_info.moves_left == 0) {
		set_x_ray_view(X_RAY_OFF);
		moveMap(wizard_eye_info.prev_x, wizard_eye_info.prev_y, cur_level);
		wizard_eye_info.caller->callback(EFFECT_CB_COMPLETE, (CallBack *)this, nullptr);
		release_focus();
	}
}

void MapWindow::set_roof_mode(bool roofs) {
	roof_mode = roofs;
	if (roof_mode) {
		if (roof_tiles)
			return;
		else
			loadRoofTiles();
	} else {
		if (roof_tiles) {
			delete roof_tiles;
			roof_tiles = nullptr;
		}
	}
}

void MapWindow::loadRoofTiles() {
	const Common::Path imagefile = map->getRoofTilesetFilename();
	roof_tiles = SDL_LoadBMP(imagefile);
	if (roof_tiles) {
		roof_tiles->setTransparentColor(roof_tiles->format.RGBToColor(0, 0x70, 0xfc));
	}
}

bool MapWindow::in_dungeon_level() const {
	if (game_type == NUVIE_GAME_MD) {
		return (cur_level == 1 || cur_level > 3); //FIXME this should probably be moved into script.
	}
	return (cur_level != 0 && cur_level != 5);
}

} // End of namespace Nuvie
} // End of namespace Ultima
