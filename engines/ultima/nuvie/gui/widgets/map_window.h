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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NUVIE_CORE_MAP_WINDOW_H
#define NUVIE_CORE_MAP_WINDOW_H

#include "ultima/shared/std/containers.h"


#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/core/map.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class TileManager;
class ActorManager;
class Actor;
class AnimManager;
class Map;
class MapCoord;
class Screen;
class CallBack;
class Game;

#define MAPWINDOW_THUMBNAIL_SIZE 52
#define MAPWINDOW_THUMBNAIL_SCALE 3

#define MAP_OVERLAY_DEFAULT 0 /* just below border */
#define MAP_OVERLAY_ONTOP   1 /* cover border */

#define MAPWINDOW_ROOFTILES_IMG_W 5
#define MAPWINDOW_ROOFTILES_IMG_H 204

typedef struct {
	Tile *t;
	uint16 x, y;
} TileInfo;

typedef struct {
	Tile *eye_tile;
	uint16 prev_x, prev_y;
	uint16 moves_left;
	CallBack *caller;
} WizardEye;

enum RoofDisplayType {ROOF_DISPLAY_OFF, ROOF_DISPLAY_NORMAL, ROOF_DISPLAY_FORCE_ON };
enum InterfaceType { INTERFACE_NORMAL, INTERFACE_FULLSCREEN, INTERFACE_IGNORE_BLOCK };
enum X_RayType { X_RAY_CHEAT_OFF = -1,  X_RAY_OFF = 0, X_RAY_ON = 1, X_RAY_CHEAT_ON = 2};
enum CanDropOrMoveMsg { MSG_NOT_POSSIBLE, MSG_SUCCESS, MSG_BLOCKED, MSG_OUT_OF_RANGE, MSG_NO_TILE};

class MapWindow: public GUI_Widget {
	friend class AnimManager;
	friend class ConverseGumpWOU;
	Game *game;
	Configuration *config;
	int game_type;
	bool enable_doubleclick;
	bool walk_with_left_button;
	uint8 walk_button_mask;
	X_RayType x_ray_view;
	InterfaceType interface;

	bool custom_actor_tiles;

	Map *map;

	uint16 *tmp_map_buf; // tempory buffer for flood fill, hide rooms.
	uint16 tmp_map_width, tmp_map_height;
	Graphics::ManagedSurface *overlay; // used for visual effects
	uint8 overlay_level; // where the overlay surface is placed
	int min_brightness;

	TileManager *tile_manager;
	ObjManager *obj_manager;
	ActorManager *actor_manager;
	AnimManager *anim_manager;

	sint16 cur_x, cur_y;
	uint16 cursor_x, cursor_y, map_center_xoff;
	sint16 mousecenter_x, mousecenter_y; // location mousecursor rotates around, relative to cur_x&cur_y
	uint16 last_boundary_fill_x, last_boundary_fill_y; // start of boundary-fill in previous blacking update
	Tile *cursor_tile;
	Tile *use_tile;

	bool show_cursor;
	bool show_use_cursor;
	bool show_grid;

	unsigned char *thumbnail;
	bool new_thumbnail;

	uint16 win_width, win_height, border_width;
	uint8 cur_level;
	uint16 map_width;

	uint8 cur_x_add, cur_y_add; // pixel offset from cur_x,cur_y (set by shiftMapRelative)
	sint32 vel_x, vel_y; // velocity of automatic map movement (pixels per second)

	Common::Rect clip_rect;

	Obj *selected_obj;
	Actor *look_actor;
	Obj *look_obj;
	bool hackmove;
	bool walking;
	bool look_on_left_click;
	bool looking; // used to stop look_on_left_click from triggering during mouseup from left button walking, failed drag, or input mode

	bool window_updated;
	bool freeze_blacking_location;
	bool enable_blacking;

	bool roof_mode;
	RoofDisplayType roof_display;

	Graphics::ManagedSurface *roof_tiles;

	WizardEye wizard_eye_info;

	bool draw_brit_lens_anim;
	bool draw_garg_lens_anim;
// Std::vector<TileInfo> m_ViewableObjTiles; // shouldn't need this for in_town checks
	Std::vector<TileInfo> m_ViewableMapTiles;

	bool lighting_update_required;

public:

	MapWindow(Configuration *cfg, Map *m);
	~MapWindow() override;

	bool init(TileManager *tm, ObjManager *om, ActorManager *am);

	sint16 get_cur_x() {
		return cur_x;
	}
	sint16 get_cur_y() {
		return cur_y;
	}
	bool set_windowSize(uint16 width, uint16 height);
	void set_show_cursor(bool state);
	void set_show_use_cursor(bool state);
	void set_show_grid(bool state);
	bool is_grid_showing() {
		return show_grid;
	}
	void set_velocity(sint16 vx, sint16 vy) {
		vel_x = vx;
		vel_y = vy;
	}
	void set_overlay(Graphics::ManagedSurface *surfpt);
	void set_overlay_level(int level = MAP_OVERLAY_DEFAULT) {
		overlay_level = level;
	}
	void set_x_ray_view(X_RayType state, bool cheat_off = false);
	X_RayType get_x_ray_view() {
		return x_ray_view;
	}
	void set_freeze_blacking_location(bool state);
	void set_enable_blacking(bool state);
	void set_roof_mode(bool roofs);
	void set_roof_display_mode(enum RoofDisplayType mode) {
		roof_display = mode;
	}
	void set_walking(bool state);
	void set_walk_button_mask();
	bool will_walk_with_left_button() {
		return walk_with_left_button;
	}
	void set_walk_with_left_button(bool val) {
		walk_with_left_button = val;
		set_walk_button_mask();
	}
	void set_looking(bool state) {
		looking = state;
	}
	int get_min_brightness() {
		return min_brightness;
	}
	void set_min_brightness(int brightness) {
		min_brightness = brightness;
	}

	void moveLevel(uint8 new_level);
	void moveMap(sint16 new_x, sint16 new_y, sint8 new_level, uint8 new_x_add = 0, uint8 new_y_add = 0);
	void moveMapRelative(sint16 rel_x, sint16 rel_y);
	void shiftMapRelative(sint16 rel_x, sint16 rel_y);
	void set_mousecenter(sint16 new_x, sint16 new_y) {
		mousecenter_x = new_x;
		mousecenter_y = new_y;
	}
	void reset_mousecenter() {
		mousecenter_x = win_width / 2;
		mousecenter_y = win_height / 2;
	}
	uint16 get_win_area() {
		return win_width * win_height;
	}
	void centerMapOnActor(Actor *actor);
	void centerMap(uint16 x, uint16 y, uint8 z);
	void centerCursor();

	void moveCursor(sint16 new_x, sint16 new_y);
	void moveCursorRelative(sint16 rel_x, sint16 rel_y);

	bool is_doubleclick_enabled() {
		return enable_doubleclick;
	}
	void set_enable_doubleclick(bool val) {
		enable_doubleclick = val;
	}
	void set_look_on_left_click(bool val) {
		look_on_left_click = val;
	}
	void set_use_left_clicks();
	bool will_look_on_left_click() {
		return look_on_left_click;
	}
	bool is_on_screen(uint16 x, uint16 y, uint8 z);
	bool tile_is_black(uint16 x, uint16 y, Obj *obj = NULL); // subtracts cur_x and cur_y
	const char *look(uint16 x, uint16 y, bool show_prefix = true);
	const char *lookAtCursor(bool show_prefix = true) {
		return (look(cursor_x, cursor_y, show_prefix));
	}
	Obj *get_objAtCursor(bool for_use = false);
	Obj *get_objAtCoord(MapCoord coord, bool top_obj, bool include_ignored_objects, bool for_use = false);
	Actor *get_actorAtCursor();
	MapCoord get_cursorCoord();
	Obj *get_objAtMousePos(int x, int y);
	Actor *get_actorAtMousePos(int x, int y);
	void teleport_to_cursor();
	void select_target(int x, int y);
	void mouseToWorldCoords(int mx, int my, int &wx, int &wy);
	void get_movement_direction(uint16 mx, uint16 my, sint16 &rel_x, sint16 &rel_y, uint8 *mptr = NULL);

	TileManager *get_tile_manager() {
		return tile_manager;
	}
	AnimManager *get_anim_manager() {
		return anim_manager;
	}
	Common::Rect *get_clip_rect()       {
		return &clip_rect;
	}
	Graphics::ManagedSurface *get_overlay();

	void get_level(uint8 *level);
	void get_pos(uint16 *x, uint16 *y, uint8 *px = NULL, uint8 *py = NULL);
	void get_velocity(sint16 *vx, sint16 *vy) {
		*vx = vel_x;
		*vy = vel_y;
	}
	void get_windowSize(uint16 *width, uint16 *height);

	bool in_window(uint16 x, uint16 y, uint8 z);
	bool in_dungeon_level();
	bool in_town();
// can put object at world location x,y?
	CanDropOrMoveMsg can_drop_or_move_obj(uint16 x, uint16 y, Actor *actor, Obj *obj);
	void display_can_drop_or_move_msg(CanDropOrMoveMsg msg, Std::string msg_text = "");
	bool can_get_obj(Actor *actor, Obj *obj);
	bool blocked_by_wall(Actor *actor, Obj *obj);
	void display_move_text(Actor *target_actor, Obj *obj);
	MapCoord original_obj_loc;

	void updateBlacking();
	void updateAmbience();
	void update();
	void Display(bool full_redraw) override;

	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseMotion(int x, int y, uint8 state) override;
	GUI_status MouseDouble(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseClick(int x, int y, Shared::MouseButton button) override;
	GUI_status Idle(void) override;
	GUI_status MouseLeave(uint8 state) override;
	GUI_status MouseDelayed(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseHeld(int x, int y, Shared::MouseButton button) override;
	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status MouseWheel(sint32 x, sint32 y) override;

	void drag_drop_success(int x, int y, int message, void *data) override;
	void drag_drop_failed(int x, int y, int message, void *data) override;

	bool drag_accept_drop(int x, int y, int message, void *data) override;
	void drag_perform_drop(int x, int y, int message, void *data) override;
	bool move_on_drop(Obj *obj);
	void set_interface();
	InterfaceType get_interface();
	bool is_interface_fullscreen_in_combat();

	void drag_draw(int x, int y, int message, void *data) override;

	void update_mouse_cursor(uint32 mx, uint32 my);

	unsigned char *make_thumbnail();
	void free_thumbnail();
	Graphics::ManagedSurface *get_sdl_surface();
	Graphics::ManagedSurface *get_sdl_surface(uint16 x, uint16 y, uint16 w, uint16 h);
	Graphics::ManagedSurface *get_roof_tiles() {
		return roof_tiles;
	}

	Std::vector<Obj *> m_ViewableObjects; //^^ dodgy public buffer

	void wizard_eye_start(MapCoord location, uint16 duration, CallBack *caller);

	bool using_map_tile_lighting;

protected:
	void create_thumbnail();

	void drawActors();
	void drawAnims(bool top_anims);
	void drawObjs();
	void drawObjSuperBlock(bool draw_lowertiles, bool toptile);
	inline void drawObj(Obj *obj, bool draw_lowertiles, bool toptile);
	inline void drawTile(Tile *tile, uint16 x, uint16 y, bool toptile, bool use_tile_data = false);
	inline void drawNewTile(Tile *tile, uint16 x, uint16 y, bool toptile);
	void drawBorder();
	inline void drawTopTile(Tile *tile, uint16 x, uint16 y, bool toptile);
	inline void drawActor(Actor *actor);
	void drawRoofs();
	void drawGrid();
	void drawRain();
	inline void drawLensAnim();

	void updateLighting();
	void generateTmpMap();
	void boundaryFill(unsigned char *map_ptr, uint16 pitch, uint16 x, uint16 y);
	bool floorTilesVisible();
	bool boundaryLookThroughWindow(uint16 tile_num, uint16 x, uint16 y);

	void reshapeBoundary();
	inline bool tmpBufTileIsBlack(uint16 x, uint16 y);
	bool tmpBufTileIsBoundary(uint16 x, uint16 y);
	bool tmpBufTileIsWall(uint16 x, uint16 y, uint8 direction);

	void wizard_eye_stop();
	void wizard_eye_update();
	bool is_wizard_eye_mode() {
		if (wizard_eye_info.moves_left != 0) return true;
		else return false;
	}

	void loadRoofTiles();

private:
	void createLightOverlay();

	void AddMapTileToVisibleList(uint16 tile_num, uint16 x, uint16 y);
	bool can_display_obj(uint16 x, uint16 y, Obj *obj);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
