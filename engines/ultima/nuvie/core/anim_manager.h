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

#ifndef NUVIE_CORE_ANIM_MANAGER_H
#define NUVIE_CORE_ANIM_MANAGER_H

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/timed_event.h"
#include "ultima/nuvie/misc/call_back.h"
#include "ultima/nuvie/misc/map_entity.h"
#include "ultima/nuvie/misc/u6_line_walker.h"

namespace Ultima {
namespace Nuvie {

using Std::list;
using Std::string;
using Std::vector;

class Actor;
class CallBack;
class AnimManager;
class NuvieAnim;
class Screen;
class Font;

#define MESG_TIMED CB_TIMED

typedef Std::list<NuvieAnim *>::iterator AnimIterator;

/* Each viewable area has it's own AnimManager. (but I can only think of
 * animations in the MapWindow using this, so that could very well change)
 */
class AnimManager {
	MapWindow *map_window;
	Screen *viewsurf;
	Common::Rect viewport; // clip anims to location
	Std::list<NuvieAnim *> anim_list; // in paint order
	uint32 next_id;

	uint8 tile_pitch;

	sint16 mapwindow_x_offset;
	sint16 mapwindow_y_offset;

	AnimIterator get_anim_iterator(uint32 anim_id);

public:
	AnimManager(sint16 x, sint16 y, Screen *screen = nullptr, Common::Rect *clipto = nullptr);
	~AnimManager() {
		destroy_all();
	}

	void update();
	void display(bool top_anims = false);

	Screen *get_surface()            {
		return viewsurf;
	}
	void set_surface(Screen *screen) {
		viewsurf = screen;
	}
	void set_area(Common::Rect clipto)  {
		viewport = clipto;
	}
	void set_tile_pitch(uint8 p)     {
		tile_pitch = p;
	}
	uint8 get_tile_pitch() const {
		return tile_pitch;
	}

//new_anim(new ExplosiveAnim(speed));
	sint32 new_anim(NuvieAnim *new_anim);
	void destroy_all();
	bool destroy_anim(uint32 anim_id);
	bool destroy_anim(NuvieAnim *anim_pt);

	NuvieAnim *get_anim(uint32 anim_id);

	void drawTile(const Tile *tile, uint16 x, uint16 y);
	void drawTileAtWorldCoords(const Tile *tile, uint16 wx, uint16 wy, uint16 add_x = 0, uint16 add_y = 0);
	void drawText(Font *font, const char *text, uint16 x, uint16 y);
};


/* Contains methods to support management, continuous display, and movement of
 * animation across viewport.
 */
/* FIXME: The return of update() is not very useful. If an anim isn't
 * redrawn then it just disappears on next MapWindow::Display(). If you don't
 * want it to appear just delete it.*/
class NuvieAnim: public CallBack {
protected:
	friend class AnimManager;
	AnimManager *anim_manager; // set by anim_manager when adding to list

	uint32 id_n; // unique

	sint32 vel_x, vel_y; // movement across viewport (pixels/second; min=10)
	uint32 px, py; // location on surface
	uint32 last_move_time; // last time when update_position() moved (ticks)

	bool safe_to_delete; // can animmgr delete me?
	bool updated; // call display
	bool running;
	bool paused;
	bool top_anim; //animate on top of mapwindow.

	// return false if animation doesn't need redraw
	virtual bool update() {
		return true;
	}
	virtual void display() = 0;

	void update_position();

public:
	NuvieAnim();
	~NuvieAnim() override;

	void pause() {
		paused = true;
	}
	void unpause() {
		paused = false;
	}
	bool is_paused() const {
		return paused;
	}

	virtual MapCoord get_location() {
		return MapCoord(px, py, 0);
	}
	uint32 get_id() const {
		return id_n;
	}

	void set_safe_to_delete(bool val)       {
		safe_to_delete = val;
	}
	void set_velocity(sint32 sx, sint32 sy) {
		vel_x = sx;
		vel_y = sy;
	}
	void set_velocity_for_speed(sint16 xdir, sint16 ydir, uint32 spd);

	virtual void stop()                     {
		updated = running = false;
	}
	virtual void start()                    { }
	uint16 message(uint16 msg, void *msg_data = nullptr, void *my_data = nullptr) {
		if (callback_target) return (CallBack::message(msg, msg_data, my_data));
		else return 0;
	}

	virtual void move(uint32 x, uint32 y, uint32 add_x = 0, uint32 add_y = 0)    {
		px = x;
		py = y;
	}
	virtual void shift(sint32 sx, sint32 sy) {
		px += sx;
		py += sy;
	}

//    void set_flags();
// ANIM_ONTOP
// ANIM_ONBOTTOM
};


/* Tile placement & data for TileAnim
 */
typedef struct {
	sint16 pos_x, pos_y; // map position relative to Anim tx,ty
	uint16 px, py; // pixel offset from pos_x,pos_y
	Tile *tile;
} PositionedTile;


/* Animation using game tiles
 */
class TileAnim : public NuvieAnim {
protected:
	MapWindow *_mapWindow;
	uint32 _tx, _ty, // location on surface: in increments of "tile_pitch"
	       _px, _py; // location on surface: pixel offset from tx,ty

	vector<PositionedTile *> _tiles;

	void display() override;

public:
	TileAnim();
	~TileAnim() override;

	MapCoord get_location() override {
		return MapCoord(_tx, _ty, 0);
	}
	void get_offset(uint32 &x_add, uint32 &y_add) const {
		x_add = _px;
		y_add = _py;
	}
	sint32 get_tile_id(PositionedTile *find_tile);

	void move(uint32 x, uint32 y, uint32 add_x = 0, uint32 add_y = 0) override {
		_tx = x;
		_ty = y;
		_px = add_x;
		_py = add_y;
	}
	void shift(sint32 sx, sint32 sy) override;
	void shift_tile(uint32 ptile_num, sint32 sx, sint32 sy);
	void move_tile(PositionedTile *ptile, uint32 x, uint32 y);


	PositionedTile *add_tile(Tile *tile, sint16 x, sint16 y, uint16 add_x = 0, uint16 add_y = 0);
	void remove_tile(uint32 i = 0);
	void remove_tile(PositionedTile *p_tile);
};


/* TileAnim using a timed event.
 */
class TimedAnim: public TileAnim {
protected:
	TimedCallback *timer;
public:
	TimedAnim()  {
		timer = nullptr;
	}
	~TimedAnim() override {
		stop_timer();
	}
	void start_timer(uint32 delay) {
		if (!timer) timer = new TimedCallback(this, nullptr, delay, true);
	}
	void stop_timer()              {
		if (timer) {
			timer->clear_target();
			timer = nullptr;
		}
	}

	void stop() override {
		stop_timer();
		NuvieAnim::stop();
	}
};


// OR these together to tell a TossAnim what to intercept
#define TOSS_TO_BLOCKING 0x01
#define TOSS_TO_ACTOR    0x02
#define TOSS_TO_OBJECT   0x04

/* A TileAnim that can intercept objects in the world. Start selected tile at
 * source, and move across viewport to target. The tile is rotated by the
 * degrees argument passed to the constructor.
 */
class TossAnim : public TileAnim {
protected:
	ActorManager *actor_manager;
	ObjManager *obj_manager;
	Map *map;

	MapCoord *src, *target;
	uint32 start_px, start_py, target_px, target_py;
	uint8 mapwindow_level; // level of map being viewed
	uint16 speed; // movement speed in pixels per second (X and Y speed can't be set independently)

	Tile *toss_tile;
	uint8 blocking; // stop_flags
	uint8 tile_center; // tile_pitch / 2
	float tanS; // Ydiff/Xdiff, between src and target (for movement velocity)
	sint16 old_relpos; // when moving diagonally, last relative position on minor axis
	float x_left, y_left; // when unable to move in a call, fractional movement values are collected here
	uint16 x_dist, y_dist; // distances from start->target on X-axis & Y-axis

	bool update() override;
	MapCoord get_location() override;

	void display() override;

public:
	TossAnim(const Tile *tile, const MapCoord &start, const MapCoord &stop, uint16 pixels_per_sec, uint8 stop_flags = 0);
	TossAnim(Obj *obj, uint16 degrees, const MapCoord &start, const MapCoord &stop, uint16 pixels_per_sec, uint8 stop_flags = 0);
	~TossAnim() override;

	void init(const Tile *tile, uint16 degrees, const MapCoord &start, const MapCoord &stop, uint16 pixels_per_sec, uint8 stop_flags);
	void start() override;
	void stop() override;
	uint32 update_position(uint32 max_move = 0);
	inline void accumulate_moves(float moves, sint32 &x_move, sint32 &y_move, sint8 xdir, sint8 ydir);

	// Virtual functions are called when the tile hits something.
	virtual void hit_target();
	virtual void hit_object(Obj *obj);
	virtual void hit_actor(Actor *actor);
	virtual void hit_blocking(const MapCoord &obj_loc);
};

// This is for off-center tiles. The tile will be moved down by the
// shift amount if moving right, and up if moving left. (and rotated)
struct tossanim_tile_shifts_s {
	uint16 tile_num;
	sint8 shift; // plus or minus vertical position
};
extern const struct tossanim_tile_shifts_s tossanim_tile_shifts[];

/* a line of fire */
typedef struct {
	PositionedTile *tile; // last associated sprite
	MapCoord direction; // where the explosion sprites are going
	uint32 travelled; // distance this fire line has travelled
} ExplosiveAnimSegment;

/* SuperBomberman! Toss fireballs in multiple directions from source out.
 */
class ExplosiveAnim : public TimedAnim {
	MapCoord center;
	uint32 radius; // num. of spaces from center
	vector<ExplosiveAnimSegment> flame; // lines of fire from the center
	uint16 exploding_tile_num; // fireball effect tile_num
	vector<MapEntity> hit_items; // things the explosion has hit

public:
	ExplosiveAnim(const MapCoord &start, uint32 size);
	~ExplosiveAnim() override;
	void start() override;
	uint16 callback(uint16 msg, CallBack *caller, void *data) override;
	bool update() override;
	bool already_hit(const MapEntity &ent);
	void hit_object(Obj *obj);
	void hit_actor(Actor *actor);
	void get_shifted_location(uint16 &x, uint16 &y, uint16 &px, uint16 &py,
	                          uint32 sx, uint32 sy);
};

typedef struct {
	MapCoord target;
	U6LineWalker *lineWalker;
	PositionedTile *p_tile;
	uint8 update_idx;
	uint16 rotation;
	uint16 rotation_amount;
	float current_deg;
	bool isRunning;
} ProjectileLine;

class ProjectileAnim : public TileAnim {
	MapCoord src;
	vector<ProjectileLine> line;
	uint16 tile_num; // fireball effect tile_num
	uint8 src_tile_y_offset; //amount to offset src_tile when rotating. Used by arrows and bolts
	vector<MapEntity> hit_items; // things the projectile has hit
	uint16 stopped_count;
	uint8 speed; //number of pixels to move in a single update.

	bool leaveTrailFlag;
public:
	ProjectileAnim(uint16 tileNum, MapCoord *start, vector<MapCoord> target, uint8 animSpeed, bool leaveTrailFlag = false, uint16 initialTileRotation = 0, uint16 rotationAmount = 0, uint8 src_y_offset = 0);
	~ProjectileAnim() override;
	void start() override;

	bool update() override;

protected:
	void hit_entity(MapEntity entity);
	bool already_hit(const MapEntity &ent);

};

class WingAnim : public TileAnim {
	MapCoord target;
	sint32 x, y, finish_x;
	sint16 x_inc;
	Tile *wing_top[2];
	Tile *wing_bottom[2];

	PositionedTile *p_tile_top;
	PositionedTile *p_tile_bottom;

public:
	WingAnim(const MapCoord &target);
	~WingAnim() override;
	void start() override;
	bool update() override;

};

typedef struct {
	uint16 x, y;
	PositionedTile *p_tile;
	uint8 length_left;
} Hailstone;

#define HAILSTORM_ANIM_MAX_STONES 6
class HailstormAnim : public TileAnim {
	MapCoord target;

	Tile *hailstone_tile;

	Hailstone hailstones[HAILSTORM_ANIM_MAX_STONES];
	uint8 num_hailstones_left;
	uint8 num_active;

public:
	HailstormAnim(const MapCoord &t);
	~HailstormAnim() override;
	void start() override;
	bool update() override;

protected:
	sint8 find_free_hailstone();

};

/* Display hit effect over an actor or location for a certain duration.
 */
class HitAnim : public TimedAnim {
	Actor *hit_actor;

	bool update() override;

public:
	HitAnim(const MapCoord &loc);
	HitAnim(Actor *actor);

	uint16 callback(uint16 msg, CallBack *caller, void *msg_data) override;
	void start() override {
		start_timer(300);
	}
};

class TextAnim : public TimedAnim {
	Std::string text;
	Font *font;
	uint32 duration;

public:
	TextAnim(Std::string text, MapCoord loc, uint32 dur);
	~TextAnim() override;
	uint16 callback(uint16 msg, CallBack *caller, void *msg_data) override;
	void start() override                    {
		start_timer(duration);
	}

	void display() override;
};

class TileFadeAnim : public TileAnim {
	uint16 pixel_count;
	Tile *anim_tile;
	Tile *to_tile;
	bool should_delete_to_tile;
	uint16 pixels_per_update; //the number of pixels to change in each update.
	unsigned char mask[256];

public:
	TileFadeAnim();
	TileFadeAnim(const MapCoord &loc, Tile *from, Tile *to, uint16 speed);
	TileFadeAnim(const MapCoord &loc, Tile *from, uint8 color_from, uint8 color_to, bool reverse, uint16 speed);
	~TileFadeAnim() override;

	bool update() override;
protected:
	void init(uint16 speed);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
