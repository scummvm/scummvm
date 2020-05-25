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

#include "ultima/shared/std/containers.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/nuvie/fonts/font_manager.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/screen/screen.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/core/anim_manager.h"

namespace Ultima {
namespace Nuvie {

#define MESG_ANIM_HIT_WORLD ANIM_CB_HIT_WORLD
#define MESG_ANIM_HIT       ANIM_CB_HIT
#define MESG_ANIM_DONE      ANIM_CB_DONE

static float get_relative_degrees(sint16 sx, sint16 sy, float angle_up = 0);
struct tossanim_tile_shifts_s tossanim_tile_shifts[] = {
	{ TILE_U6_BOLT, 4 },
	{ TILE_U6_ARROW, 4 },
	{ 0, 0}
}; // end-list pointer


/* Convert a non-normalized relative direction (difference) from any center
 * point to degrees, where direction 0,-1 (up) is `angle_up' degrees.
 * Returns the angle.
 * FIXME: this should only be a few lines
 */
static float get_relative_degrees(sint16 sx, sint16 sy, float angle_up) {
	float angle = 0;
	uint16 x = abs(sx), y = abs(sy);

	if (sx > 0 && sy < 0)
		angle = (x > y) ? 68 : (y > x) ? 23 : 45;
	else if (sx > 0 && sy == 0)
		angle = 90;
	else if (sx > 0 && sy > 0)
		angle = (x > y) ? 113 : (y > x) ? 158 : 125;
	else if (sx == 0 && sy > 0)
		angle = 180;
	else if (sx < 0 && sy > 0)
		angle = (x > y) ? 248 : (y > x) ? 203 : 225;
	else if (sx < 0 && sy == 0)
		angle = 270;
	else if (sx < 0 && sy < 0)
		angle = (x > y) ? 293 : (y > x) ? 338 : 315;
	// (sx == 0 && sy < 0) = 0

	angle += angle_up;
	if (angle >= 360)
		angle -= 360;
	return (angle);
}


AnimManager::AnimManager(sint16 x, sint16 y, Screen *screen, Common::Rect *clipto)
	: next_id(0) {
	map_window = Game::get_game()->get_map_window();
	tile_pitch = 16;

	viewsurf = screen;
	if (clipto)
		viewport = *clipto;

	mapwindow_x_offset = x;
	mapwindow_y_offset = y;
}


/* Returns an iterator to the animation with requested id_n.
 */
AnimIterator AnimManager::get_anim_iterator(uint32 anim_id) {
	AnimIterator i = anim_list.begin();
	while (i != anim_list.end()) {
		if ((*i)->id_n == anim_id)
			return (i);
		++i;
	}
	return (anim_list.end());
}


/* Returns animation with requested id_n.
 */
NuvieAnim *AnimManager::get_anim(uint32 anim_id) {
	AnimIterator i = get_anim_iterator(anim_id);
	if (i != anim_list.end())
		return (*i);
	return (NULL);
}


/* Update all animations.
 */
void AnimManager::update() {
	AnimIterator i = anim_list.begin();
	while (i != anim_list.end()) {
		(*i)->updated = (*i)->update();
		++i;
	}

	// remove completed animations
	i = anim_list.begin();
	while (i != anim_list.end())
		if (!(*i)->running) {
			destroy_anim(*i);
			i = anim_list.begin();
		} else ++i;
}


/* Draw all animations that have been updated.
 */
void AnimManager::display(bool top_anims) {
	AnimIterator i = anim_list.begin();
	while (i != anim_list.end()) {
		if ((*i)->updated && ((top_anims && (*i)->top_anim) || !(*i)->top_anim)) {
			(*i)->display();
			(*i)->updated = false;
		}
		++i;
	}
}


/* Add animation pointer to managed list.
 * Returns id_n of new animation for future reference.
 */
sint32 AnimManager::new_anim(NuvieAnim *new_anim) {
	if (new_anim) {
		new_anim->id_n = next_id++;
		new_anim->anim_manager = this;
		anim_list.push_back(new_anim);
		new_anim->start();
		return ((uint32)new_anim->id_n);
	}
	DEBUG(0, LEVEL_ERROR, "Anim: tried to add NULL anim\n");
	return (-1);
}


/* Delete all animations.
 */
void AnimManager::destroy_all() {
	while (!anim_list.empty())
		if (!destroy_anim(*anim_list.begin()))
			break;
}


/* Delete an animation.
 */
bool AnimManager::destroy_anim(uint32 anim_id) {
	return (destroy_anim(get_anim(anim_id)));
}


/* Delete an animation.
 */
bool AnimManager::destroy_anim(NuvieAnim *anim_pt) {
	AnimIterator i = get_anim_iterator(anim_pt->id_n);
	if (i != anim_list.end()) {
//            (*i)->message(MESG_ANIM_DONE); // FIXME: for now Anims send this for various reasons
		if ((*i)->safe_to_delete)
			delete *i;
		anim_list.erase(i);
		return (true);
	}
	DEBUG(0, LEVEL_ERROR, "Anim: error deleting %d\n", anim_pt->id_n);
	return (false);
}


/* Draw tile on viewsurf at x,y.
 */
void AnimManager::drawTile(Tile *tile, uint16 x, uint16 y) {
	viewsurf->blit(mapwindow_x_offset + x, mapwindow_y_offset + y, tile->data, 8, tile_pitch, tile_pitch, 16,
	               tile->transparent, &viewport);
}

void AnimManager::drawText(Font *font, const char *text, uint16 x, uint16 y) {
	font->drawString(viewsurf, text, mapwindow_x_offset + x, mapwindow_y_offset + y);
}

/* Draw tile on viewsurf at map location wx,wy (offset by add_x,add_y).
 */
void AnimManager::drawTileAtWorldCoords(Tile *tile, uint16 wx, uint16 wy,
                                        uint16 add_x, uint16 add_y) {
	sint16 cur_x = map_window->cur_x;
	sint16 cur_y = map_window->cur_y;
	drawTile(tile, ((wx - cur_x) * tile_pitch) + add_x,
	         ((wy - cur_y) * tile_pitch) + add_y);
}


/*** NuvieAnim ***/
NuvieAnim::NuvieAnim() {
	anim_manager = NULL;

	id_n = 0;

	vel_x = vel_y = 0;
	px = py = 0;

	safe_to_delete = true;
	updated = true;
	running = true;

	last_move_time = SDL_GetTicks();
	paused = false;
	top_anim = false;
}


NuvieAnim::~NuvieAnim() {

}


/* Set velocity (x,y) for moving towards a target at distance (xdir,ydir).
 */
void NuvieAnim::set_velocity_for_speed(sint16 xdir, sint16 ydir, uint32 spd) {
	uint16 max_dist, min_dist;
	uint16 xdist = abs(xdir), ydist = abs(ydir);
	sint32 xvel = spd, yvel = spd;

	if (xdir == 0 || ydir == 0) {
		if (xdir == 0) xvel = 0;
		if (ydir == 0) yvel = 0;
	} else if (xdist > ydist) { // MAX_DIR = xdir
		max_dist = xdist;
		min_dist = ydist;
		yvel = xvel / (max_dist / min_dist); // max_vel/min_vel = max_dir/min_dir
	} else { // MAX_DIR = ydir
		min_dist = xdist;
		max_dist = ydist;
		xvel = yvel / (max_dist / min_dist);
	}
	set_velocity((xdir == 0) ? 0 : (xdir > 0) ? xvel : -xvel,
	             (ydir == 0) ? 0 : (ydir > 0) ? yvel : -yvel);
}


/* Move per velocity.
 */
void NuvieAnim::update_position() {
	uint32 this_time = SDL_GetTicks();

	if (this_time - last_move_time >= 100) { // only move every 10th sec
		sint32 vel_x_incr = vel_x / 10, vel_y_incr = vel_y / 10;
		if (vel_x && !vel_x_incr) // move even if fps was < 10
			vel_x_incr = (vel_x < 0) ? -1 : 1;
		if (vel_y && !vel_y_incr)
			vel_y_incr = (vel_y < 0) ? -1 : 1;

		shift(vel_x_incr, vel_y_incr);
		last_move_time = this_time;
	}
}


/*** TileAnim ***/
TileAnim::TileAnim() {
	_mapWindow = Game::get_game()->get_map_window();
	_tx = _ty = _px = _py = 0;
}


TileAnim::~TileAnim() {
	while (_tiles.size())
		remove_tile();
}


/* Find a tile in the animation and return its id or list position.
 * Returns -1 if it isn't found.
 */
sint32 TileAnim::get_tile_id(PositionedTile *find_tile) {
	uint32 tile_count = _tiles.size();
	for (uint32 t = 0; t < tile_count; t++)
		if (find_tile == _tiles[t])
			return (t);
	return (-1);
}


/* Display ordered tile list.
 */
void TileAnim::display() {
	uint32 tcount = _tiles.size();
	for (sint32 t = (tcount - 1); t >= 0; t--)
		anim_manager->drawTileAtWorldCoords(_tiles[t]->tile,
		                                    _tx + _tiles[t]->pos_x, _ty + _tiles[t]->pos_y,
		                                    _px + _tiles[t]->px, _py + _tiles[t]->py);
}


/* Add tile to list for this Anim, relative to the Anim coordinates.
 */
PositionedTile *TileAnim::add_tile(Tile *tile, sint16 x, sint16 y,
                                   uint16 add_x, uint16 add_y) {
	PositionedTile *new_tile = new PositionedTile;
	new_tile->tile = tile;
	new_tile->pos_x = x;
	new_tile->pos_y = y;
	new_tile->px = add_x;
	new_tile->py = add_y;
	_tiles.insert(_tiles.begin(), new_tile);
	return (new_tile);
}


/* Remove tile from list position `i'. (default = last added)
 */
void TileAnim::remove_tile(uint32 i) {
	if (i < _tiles.size()) {
		//vector<PositionedTile*>::iterator ti = &tiles[i];
		Std::vector<PositionedTile *>::iterator ti = _tiles.begin();
		for (uint32 j = 0; j < i; j++)
			ti++;

		delete *ti;
		_tiles.erase(ti);
	}
}

void TileAnim::remove_tile(PositionedTile *p_tile) {
	Std::vector<PositionedTile *>::iterator ti = _tiles.begin();
	for (; ti != _tiles.end(); ti++) {
		if (*ti == p_tile) {
			delete *ti;
			_tiles.erase(ti);
			break;
		}
	}

	return;
}


/* Move tile(s) by the pixel, in direction sx,sy.
 */
void TileAnim::shift(sint32 sx, sint32 sy) {
	uint8 tile_pitch = anim_manager->get_tile_pitch();
	uint32 total_px = (_tx * tile_pitch) + _px, total_py = (_ty * tile_pitch) + _py;
	total_px += sx;
	total_py += sy;
	move(total_px / tile_pitch, total_py / tile_pitch,
	     total_px % tile_pitch, total_py % tile_pitch);
}


/* Move a tile by the pixel, relative to the animation, in direction sx,sy.
 */
void TileAnim::shift_tile(uint32 ptile_num, sint32 sx, sint32 sy) {
	if (_tiles.size() <= ptile_num)
		return;

	uint8 tile_pitch = anim_manager->get_tile_pitch();
	uint16 vtx = _tiles[ptile_num]->pos_x, vty = _tiles[ptile_num]->pos_y;
	uint16 vpx = _tiles[ptile_num]->px, vpy = _tiles[ptile_num]->py;
	uint32 total_px = (vtx * tile_pitch) + vpx, total_py = (vty * tile_pitch) + vpy;
	total_px += sx;
	total_py += sy;
	// move
	_tiles[ptile_num]->pos_x = total_px / tile_pitch;
	_tiles[ptile_num]->pos_y = total_py / tile_pitch;
	_tiles[ptile_num]->px = total_px % tile_pitch;
	_tiles[ptile_num]->py = total_py % tile_pitch;
}

void TileAnim::move_tile(PositionedTile *ptile, uint32 x, uint32 y) {
	uint8 tile_pitch = anim_manager->get_tile_pitch();

	ptile->pos_x = x / tile_pitch;
	ptile->pos_y = y / tile_pitch;
	ptile->px = x % tile_pitch;
	ptile->py = y % tile_pitch;
}

/*** HitAnim ***/

/* Construct TimedEvent with effect duration as time.
 */
HitAnim::HitAnim(MapCoord *loc) {
	hit_actor = NULL;
	add_tile(_mapWindow->get_tile_manager()->get_tile(257), // U6 HIT_EFFECT
	         0, 0);
	move(loc->x, loc->y);
}


HitAnim::HitAnim(Actor *actor) {
	hit_actor = actor;
	add_tile(_mapWindow->get_tile_manager()->get_tile(257), // U6 HIT_EFFECT
	         0, 0);
	MapCoord loc = hit_actor->get_location();
	move(loc.x, loc.y);
}


/* Keep effect over actor. */
bool HitAnim::update() {
	if (hit_actor) {
		MapCoord loc = hit_actor->get_location();
		move(loc.x, loc.y);
	}
	return (true);
}

uint16 HitAnim::callback(uint16 msg, CallBack *caller, void *msg_data) {
	if (msg == MESG_TIMED) {
		message(MESG_ANIM_DONE);
		stop();
	}

	return (0);
}


/*** TextAnim ***/

TextAnim::TextAnim(Std::string t, MapCoord loc, uint32 dur) {
	_px = loc.x;
	_py = loc.y;
	duration = dur;
	font = Game::get_game()->get_font_manager()->get_conv_font();
	text = t;
	top_anim = true;
}

TextAnim::~TextAnim() {

}

void TextAnim::display() {
	if (is_paused())
		return;

	anim_manager->drawText(font, text.c_str(), _px, _py);
}

uint16 TextAnim::callback(uint16 msg, CallBack *caller, void *msg_data) {
	if (msg == MESG_TIMED) {
		message(MESG_ANIM_DONE);
		stop();
	}

	return (0);
}

/*** TossAnim ***/
TossAnim::TossAnim(Tile *tile, const MapCoord &start, const MapCoord &stop, uint16 pixels_per_sec, uint8 stop_flags) {
	tile_center = 0;
	actor_manager = Game::get_game()->get_actor_manager();
	obj_manager = Game::get_game()->get_obj_manager();
	map = Game::get_game()->get_game_map();

	init(tile, 0, start, stop, pixels_per_sec, stop_flags);
}
TossAnim::TossAnim(Obj *obj, uint16 degrees, const MapCoord &start, const MapCoord &stop, uint16 pixels_per_sec, uint8 stop_flags) {
	tile_center = 0;
	actor_manager = Game::get_game()->get_actor_manager();
	obj_manager = Game::get_game()->get_obj_manager();
	map = Game::get_game()->get_game_map();

	init(obj_manager->get_obj_tile(obj->obj_n, obj->frame_n), degrees,
	     start, stop, pixels_per_sec, stop_flags);
}

TossAnim::~TossAnim() {
	if (running) {
		DEBUG(0, LEVEL_WARNING, "deleting active TossAnim!\n");
		stop();
	}

	delete src;
	delete target;
	delete toss_tile;
}

void TossAnim::init(Tile *tile, uint16 degrees, const MapCoord &start, const MapCoord &stop, uint16 pixels_per_sec, uint8 stop_flags) {
	src = new MapCoord(start);
	target = new MapCoord(stop);
	blocking = stop_flags;
	speed = pixels_per_sec;
	_mapWindow->get_level(&mapwindow_level);

	TileManager *tile_mgr = Game::get_game()->get_tile_manager();
	MapCoord direction; // relative direction from->to
	direction.sx = target->x - src->x;
	direction.sy = target->y - src->y;
	toss_tile = tile_mgr->get_rotated_tile(tile, get_relative_degrees(direction.sx, direction.sy, degrees));

	// these are set in start()
	start_px = start_py = target_px = target_py = 0;
	tanS = 0;
	old_relpos = 0;
	x_left = 0;
	y_left = 0;
	x_dist = 0;
	y_dist = 0;
}


/* Start tile at source. Get a tangent for velocity. */
void TossAnim::start() {
	uint8 pitch = anim_manager->get_tile_pitch(); // AnimManager not set until start
	tile_center = (pitch / 2) - 1;

	add_tile(toss_tile, 0, 0);
	move(src->x, src->y);

	// pixel-line movement constants
	start_px = src->x * pitch;
	start_py = src->y * pitch;
	target_px = target->x * pitch;
	target_py = target->y * pitch;
	x_dist = abs(sint32(target_px - start_px));
	y_dist = abs(sint32(target_py - start_py));
	if (x_dist) // will move in X direction (and possibly diagonally)
		tanS = float(sint32(target_py - start_py)) / sint32(target_px - start_px);
//DEBUG(0,LEVEL_DEBUGGING,"start: tanS = %d / %d = %f\n", target_py-start_py, target_px-start_px, tanS);
	Game::get_game()->dont_wait_for_interval();

// adjust tile appearance but not anim location
	uint16 t = 0;
	while (tossanim_tile_shifts[t].tile_num != 0) {
		if (tossanim_tile_shifts[t].tile_num == toss_tile->tile_num) {
			sint8 shift = tossanim_tile_shifts[t].shift;
			if (sint32(target_px - start_px) < 0) // going left
				shift_tile(0, 0, -shift);
			if (sint32(target_px - start_px) > 0) // going right
				shift_tile(0, 0, shift);
			if (sint32(target_py - start_py) < 0) // going up
				shift_tile(0, shift, 0);
			if (sint32(target_py - start_py) > 0) // going down
				shift_tile(0, -shift, 0);
		}
		++t;
	}
}


/* Return to normal Events timing.
 */
void TossAnim::stop() {
	if (running) { // were stop() called twice, Events might start waiting even when more animations are left
		TileAnim::stop();
		Game::get_game()->wait_for_interval();
	}
}


/* Returns location of tile biased by movement. If moving left or up, the
 * center of the tile must be left or above the center of its location.
 */
MapCoord TossAnim::get_location() {
	MapCoord loc(_tx, _ty, 0);
	if (src->x > target->x) { // moving left
		if (_px > 0) loc.x += 1;
	}
	if (src->y > target->y) { // moving up
		if (_px > 0) loc.y += 1;
	}
	return (loc);
}


/* Slide tile. Check map for interception. (stop if hit target)
 * Possible things to hit are actors and objects. Possible locations to hit are
 * boundary tiles (blocking) and the target.
 */
bool TossAnim::update() {
	uint32 moves_left = 0;
	if (!running || is_paused())
		return true;

	do { // move (no more than) one tile at a time, and check for intercept
		MapCoord old_loc = get_location(); // record old location
		moves_left = update_position(anim_manager->get_tile_pitch()); /** MOVE **/
		MapCoord new_loc = get_location();

		// test the current location, check for intercept
		if (new_loc != old_loc) {
//DEBUG(0,LEVEL_DEBUGGING,"Toss: %x,%x,%x->%x,%x,%x\n",old_loc.x,old_loc.y,old_loc.z,new_loc.x,new_loc.y,new_loc.z);
			// hit actor or object? (can be one or the other)
			Actor *hitActor = actor_manager->get_actor(new_loc.x, new_loc.y, mapwindow_level);
			Obj *hitObj = obj_manager->get_obj(new_loc.x, new_loc.y, mapwindow_level);

			// blocking tile
			if (!map->is_passable(new_loc.x, new_loc.y, mapwindow_level)) {
				if (!hitActor) // NOTE: no effect if actor is also at location
					hit_blocking(MapCoord(new_loc.x, new_loc.y, mapwindow_level));
				else
					stop(); // this mimics U6 in appearance and results in no effect
			} else if (hitActor)
				hit_actor(hitActor);
			else if (hitObj)
				hit_object(hitObj);

			if (running) { // one of the interceptors could have stopped us
				// target (mandatory stop)
				if (new_loc.x == target->x && new_loc.y == target->y)
					hit_target();
				else if (new_loc.distance(*src) > src->distance(*target)) {
					// overshot target (move and stop)
					DEBUG(0, LEVEL_WARNING, "TossAnim missed the target\n");
					move(target->x, target->y);
					hit_target();
				}
			}
		}
	} while (running && moves_left > 0);
	return (true);
}

void TossAnim::display() {
	if (is_paused())
		return;

	TileAnim::display();
}

void TossAnim::hit_target() {
	assert(running == true);

	stop();
	message(MESG_ANIM_DONE, NULL);
}

void TossAnim::hit_object(Obj *obj) {
	assert(running == true);

	MapEntity obj_ent(obj);
	if (blocking & TOSS_TO_OBJECT)
		message(MESG_ANIM_HIT, &obj_ent);
}

void TossAnim::hit_actor(Actor *actor) {
	assert(running == true);

	MapEntity actor_ent(actor);
	if (blocking & TOSS_TO_ACTOR)
		message(MESG_ANIM_HIT, &actor_ent);
}

void TossAnim::hit_blocking(MapCoord obj_loc) {
	assert(running == true);

	if (blocking & TOSS_TO_BLOCKING)
		message(MESG_ANIM_HIT_WORLD, &obj_loc);
}


/* Get velocity from tangent & speed, and move the tile.
 * FIXME: for diagonal movement the speed is only applied to X-axis, which is modified for Y-axis (so some tosses look faster than others)
 */
uint32 TossAnim::update_position(uint32 max_move) {
	uint32 ms_passed = 0;
	uint32 fraction = divide_time(SDL_GetTicks(), last_move_time, &ms_passed);

	/* get number of moves(pixels) this fraction, and movement direction */
	uint32 moves_left = 0;
	float moves = (float)speed / (fraction > 0 ? fraction : 1);
	if (moves > max_move) { // WARNING: this drops the decimal value, but it doesn't matter if moving more than 16px per call (also I'm not even saving the remainder now)
		moves_left = (uint32)floorf(moves) - max_move;
		moves = max_move;
	}
	sint8 xdir = (start_px == target_px) ? 0 : (start_px < target_px) ? 1 : -1;
	sint8 ydir = (start_py == target_py) ? 0 : (start_py < target_py) ? 1 : -1;
	sint32 x_move = (uint32)floorf(moves) * xdir; // modifies per direction
	sint32 y_move = (uint32)floorf(moves) * ydir;

	// collect the remainder (amount of movement less than 1)
	accumulate_moves(moves, x_move, y_move, xdir, ydir);
//DEBUG(0,LEVEL_DEBUGGING,"(%d) moves:%f x_move:%d y_move:%d x_left:%f y_left:%f\n",ms_passed,moves,x_move,y_move,x_left,y_left);
	// too slow for movement, just return
	if (x_move == 0 && y_move == 0)
		return (moves_left);

	if (x_move != 0) {
		if (x_dist >= y_dist) { // Y=X*tangent
			uint32 xpos = _tx * 16 + _px; // pixel location
			sint32 relx = (xpos + x_move) - start_px; // new relative position
			sint32 rely = (uint32)roundf(relx * tanS);
			shift(x_move, rely - old_relpos); // **MOVE**
			old_relpos = rely;
		} else { // X=Y/tangent
			uint32 ypos = _ty * 16 + _py; // pixel location
			sint32 rely = (ypos + y_move) - start_py; // new relative position
			sint32 relx = (uint32)roundf(rely / tanS);
			shift(relx - old_relpos, y_move); // **MOVE**
			old_relpos = relx;
		}
	} else // only moving along Y
		shift(0, y_move); // **MOVE**
	return (moves_left);
}


/* Update x_left/y_left from incremental moves adding the decimal of
 * moves to them. If any integer value has been collected, move it to
 * the passed x_move/y_move references.
 */
void TossAnim::accumulate_moves(float moves, sint32 &x_move, sint32 &y_move, sint8 xdir, sint8 ydir) {
	x_left += (float)moves - (uint32)floorf(moves); // get decimal component
	y_left += (float)moves - (uint32)floorf(moves);
	if (x_left >= 1.0) {
		x_move += (uint32)floorf(x_left) * xdir; // add integer to movement
		x_left -= (uint32)floorf(x_left); // remove from collection
	}
	if (y_left >= 1.0) {
		y_move += (uint32)floorf(y_left) * ydir; // add integer to movement
		y_left -= (uint32)floorf(y_left); // remove from collection
	}
}


/*** ExplosiveAnim ***/
ExplosiveAnim::ExplosiveAnim(MapCoord *start, uint32 size) {
	exploding_tile_num = 393; // U6 FIREBALL_EFFECT

	center = *start;
	radius = size;
}


/* Delete the rotated tileset.
 */
ExplosiveAnim::~ExplosiveAnim() {
// removed - this is deleted in TileAnim
//    for(uint32 i = 0; i < tiles.size(); i++)
//        delete tiles[i]->tile;
}


/* Set up how many fireballs shoot out and in what directions. Start them at
 * the center.
 */
void ExplosiveAnim::start() {
	TileManager *tile_manager = _mapWindow->get_tile_manager();
	sint32 s = 8; // how many pixels each new fireball is from the previous

	move(center.x, center.y);
	flame.resize(radius * 5);

	for (uint32 t = 0; t < flame.size(); t++) {
		flame[t].travelled = 0;
		flame[t].tile = add_tile(tile_manager->get_tile(exploding_tile_num), 0, 0);

		uint8 dir = (t < 8) ? t : NUVIE_RAND() % 8;
		switch (dir) {
		case NUVIE_DIR_N:
			flame[t].direction = MapCoord(0, -s);
			break;
		case NUVIE_DIR_NE:
			flame[t].direction = MapCoord(s, -s);
			break;
		case NUVIE_DIR_E:
			flame[t].direction = MapCoord(s, 0);
			break;
		case NUVIE_DIR_SE:
			flame[t].direction = MapCoord(s, s);
			break;
		case NUVIE_DIR_S:
			flame[t].direction = MapCoord(0, s);
			break;
		case NUVIE_DIR_SW:
			flame[t].direction = MapCoord(-s, s);
			break;
		case NUVIE_DIR_W:
			flame[t].direction = MapCoord(-s, 0);
			break;
		case NUVIE_DIR_NW:
			flame[t].direction = MapCoord(-s, -s);
		}
		if (t >= 8) { // after the cardinals, randomly alter direction
			flame[t].direction.sx += ((NUVIE_RAND() % 7) - 3);
			flame[t].direction.sy += ((NUVIE_RAND() % 7) - 3);
		}

		// rotate sprite
		if (!(flame[t].direction.sx == 0 && flame[t].direction.sy == 0)) {
			Tile *rot_tile = NULL;
			rot_tile = tile_manager->get_rotated_tile(flame[t].tile->tile,
			           get_relative_degrees(flame[t].direction.sx, flame[t].direction.sy));
			flame[t].tile->tile = rot_tile;
		}
	}

	// start timer: larger explosions are faster
	uint32 delay;
	if (radius <= 3)
		delay = 15;
	else if (radius <= 5)
		delay = 5;
	else
		delay = 4;
	start_timer(delay);
}


/* On TIMED: Expand explosion. End when explosion is finished.
 */
uint16 ExplosiveAnim::callback(uint16 msg, CallBack *caller, void *msg_data) {
	bool animating = false; // true = still exploding
	uint32 flame_size = flame.size();

	if (msg != MESG_TIMED)
		return (0);

	for (uint32 t = 0; t < flame_size; t++) { // update each line of fire
		uint32 r = radius;
		if ((r > 1) && flame[t].direction.sx && flame[t].direction.sy) // diagonal
			r -= 1;
		if (flame[t].travelled < r) { // keep exploding; add another fireball
			flame[t].tile = add_tile(flame[t].tile->tile,
			                         flame[t].tile->pos_x, flame[t].tile->pos_y,
			                         flame[t].tile->px, flame[t].tile->py);
			shift_tile(0, flame[t].direction.sx, flame[t].direction.sy);
			flame[t].travelled++; // flame travel distance from center
			animating = true;
		}
	}

	if (!animating) { // all flames stopped moving, so we're done
		message(MESG_ANIM_DONE); // FIXME: in the future make all Anims send when deleted
		stop();
	}
	return (0);
}


/* Check map for interception with any flames. Stop exploding in direction of
 * a blocked tile. Hit actors & volatile objects (powder kegs).
 */
bool ExplosiveAnim::update() {
	Map *map = Game::get_game()->get_game_map();
	uint8 mapwindow_level;
	LineTestResult lt;

	_mapWindow->get_level(&mapwindow_level);
	for (uint32 t = 0; t < flame.size(); t++) { // update each line of fire
		if (flame[t].direction.sx == 0 && flame[t].direction.sy == 0)
			continue; // not moving; check next flames
		// (absolute) location of last fireball (the one that matters)
		MapCoord edge(flame[t].tile->pos_x + center.x,
		              flame[t].tile->pos_y + center.y, mapwindow_level);
		uint16 edge_px = flame[t].tile->px; // pixel offset
		uint16 edge_py = flame[t].tile->py;
		// effective edge is location of the CENTER of this fireball/tile, if it
		// is moving down or right, but I'm not sure why direction affects it
		if (flame[t].direction.sx > 0 && edge_px >= 8) edge.x += 1;
		if (flame[t].direction.sy > 0 && edge_py >= 8) edge.y += 1;

		if (map->lineTest(center.x, center.y, edge.x, edge.y, mapwindow_level,
		                  LT_HitActors, lt, 1)
		        && !already_hit(MapEntity(lt.hitActor)))
			hit_actor(lt.hitActor);
		else if (map->lineTest(center.x, center.y, edge.x, edge.y, mapwindow_level,
		                       LT_HitObjects, lt, 1)
		         && !already_hit(MapEntity(lt.hitObj)))
			hit_object(lt.hitObj);

		// stop in this direction
		// FIXME: more things than boundaries (walls) stop explosions
		if (map->is_boundary(edge.x, edge.y, mapwindow_level) && edge != center)
			flame[t].direction.sx = flame[t].direction.sy = 0;
//        get_shifted_location(edge.x, edge.y, edge_px, edge_py,
//                             flame[t].direction.sx, flame[t].direction.sy);
//        if(map->lineTest(center.x, center.y, edge.x, edge.y, mapwindow_level,
//                         LT_HitUnpassable, lt, 1))
//            flame[t].direction.sx = flame[t].direction.sy = 0;
	}

	return (true);
}


/* Returns true if the explosion has already the particular thing this MapEntity
 * points to. (and shouldn't hit it again)
 */
bool ExplosiveAnim::already_hit(MapEntity ent) {
	for (uint32 e = 0; e < hit_items.size(); e++)
		if (hit_items[e].entity_type == ent.entity_type)
			if (hit_items[e].data == ent.data)
				return (true);
	return (false);
}


/* Also adds object to hit_items list for already_hit() to check. */
void ExplosiveAnim::hit_object(Obj *obj) {
	if (!running)
		return;
	MapEntity obj_ent(obj);
	hit_items.push_back(obj_ent);
	message(MESG_ANIM_HIT, &obj_ent);
}


/* Also adds actor to hit_items list for already_hit() to check. */
void ExplosiveAnim::hit_actor(Actor *actor) {
	if (!running)
		return;
	MapEntity actor_ent(actor);
	hit_items.push_back(actor_ent);
	message(MESG_ANIM_HIT, &actor_ent);
}


/* Get the location something on the map would be at were it
 * shifted by sx,sy pixels from x,y+px,py.
 * FIXME: duplicated code
 */
void ExplosiveAnim::get_shifted_location(uint16 &x, uint16 &y, uint16 &px_, uint16 &py_, uint32 sx, uint32 sy) {
	uint8 tile_pitch = anim_manager->get_tile_pitch();
	uint32 total_px = (x * tile_pitch) + px_, total_py = (y * tile_pitch) + py_;
	total_px += sx;
	total_py += sy;

	x = total_px / tile_pitch;
	y = total_py / tile_pitch;
	px_ = total_px % tile_pitch;
	py_ = total_py % tile_pitch;
}


/*** ProjectileAnim ***/
ProjectileAnim::ProjectileAnim(uint16 tileNum, MapCoord *start, vector<MapCoord> target, uint8 animSpeed, bool leaveTrail, uint16 initialTileRotation, uint16 rotationAmount, uint8 src_y_offset) {
	tile_num = tileNum; //382; // U6 FIREBALL_EFFECT
	src_tile_y_offset = src_y_offset;
	src = *start;
	line.resize(target.size());

	for (uint16 i = 0; i < line.size(); i++) {
		line[i].target = target[i];
		line[i].lineWalker = new U6LineWalker(src.x * 16, src.y * 16, target[i].x * 16, target[i].y * 16);
		line[i].rotation = initialTileRotation;
		line[i].rotation_amount = rotationAmount;
	}

	stopped_count = 0;

	speed = animSpeed;
	if (speed == 0)
		speed = 1;

	leaveTrailFlag = leaveTrail;
}


/* Delete the rotated tileset.
 */
ProjectileAnim::~ProjectileAnim() {

	for (uint32 i = 0; i < line.size(); i++) {
		delete line[i].lineWalker;
		delete line[i].p_tile->tile; //because we made a new rotated tile.
	}
}


/* Set up how many fireballs shoot out and in what directions. Start them at
 * the center.
 */
void ProjectileAnim::start() {
	TileManager *tile_manager = _mapWindow->get_tile_manager();
	Tile *t = tile_manager->get_tile(tile_num);

	move(0, 0);

	for (uint16 i = 0; i < line.size(); i++) {
		uint32 x, y;
		line[i].lineWalker->next(&x, &y);
		line[i].current_deg = get_relative_degrees(line[i].target.x - src.x, line[i].target.y - src.y) - line[i].rotation;
		if (line[i].current_deg < 0)
			line[i].current_deg += 360;

		line[i].p_tile = add_tile(tile_manager->get_rotated_tile(t, line[i].current_deg, src_tile_y_offset), x / 16, y / 16, x % 16, y % 16);
		line[i].update_idx = 0;
		line[i].isRunning = true;
	}
}



/* Check map for interception with any flames. Stop exploding in direction of
 * a blocked tile. Hit actors & volatile objects (powder kegs).
 */
bool ProjectileAnim::update() {
	TileManager *tile_manager = _mapWindow->get_tile_manager();
	Map *map = Game::get_game()->get_game_map();
	uint8 level;
	LineTestResult lt;

	_mapWindow->get_level(&level);

	for (uint16 i = 0; i < line.size(); i++) {
		if (line[i].isRunning) {
			uint32 x = 0, y = 0;
			bool canContinue = false;
			//printf("step. speed=%d, time=%d\n",speed,SDL_GetTicks());
			for (uint16 j = 0; j < speed * 2; j++) {
				canContinue = line[i].lineWalker->next(&x, &y);
				if (!canContinue) { //FIXME explosion boundary checks. || map->is_boundary((x + 10)/16,(y + 10)/16,level))
					canContinue = false;
					break;
				}
			}

			if (line[i].rotation_amount != 0) {
				line[i].current_deg += line[i].rotation_amount;
				if (line[i].current_deg >= 360)
					line[i].current_deg -= 360;

				tile_manager->get_rotated_tile(tile_manager->get_tile(tile_num), line[i].p_tile->tile, line[i].current_deg);
			}

			if (leaveTrailFlag == true) {
				add_tile(line[i].p_tile->tile, x / 16, y / 16, x % 16, y % 16);
			} else {
				move_tile(line[i].p_tile, x, y);
			}

			if (map->testIntersection(x / 16, y / 16, level, LT_HitActors, lt)
			        && !already_hit(MapEntity(lt.hitActor)))
				hit_entity(MapEntity(lt.hitActor));
			else if (map->testIntersection(x / 16, y / 16, level, LT_HitObjects, lt)
			         && !already_hit(MapEntity(lt.hitObj)))
				hit_entity(MapEntity(lt.hitObj));

			if (canContinue == false) {
				line[i].isRunning = false;
				stopped_count++;
			}
		}

	}

	if (stopped_count == line.size()) {
		message(MESG_ANIM_DONE);
		stop();
	}

	return (true);
}

/* Also adds actor/object to hit_items list for already_hit() to check. */
void ProjectileAnim::hit_entity(MapEntity entity) {
	if (!running)
		return;
	hit_items.push_back(entity);
	message(MESG_ANIM_HIT, &entity);
}

/* Returns true if the explosion has already the particular thing this MapEntity
 * points to. (and shouldn't hit it again)
 */
bool ProjectileAnim::already_hit(MapEntity ent) {
	for (uint32 e = 0; e < hit_items.size(); e++)
		if (hit_items[e].entity_type == ent.entity_type)
			if (hit_items[e].data == ent.data)
				return (true);
	return (false);
}

/*** WingAnim ***/
WingAnim::WingAnim(MapCoord t) {
	TileManager *tile_manager = _mapWindow->get_tile_manager();

	p_tile_top = NULL;
	p_tile_bottom = NULL;
	target = t;
	y = target.y * 16;

	uint16 mx, my, win_w, win_h;
	_mapWindow->get_pos(&mx, &my);
	_mapWindow->get_windowSize(&win_w, &win_h);

	uint16 off_x;

	off_x = target.x - mx;

	if (off_x <=  win_w / 2) {
		//right, left
		finish_x = (mx - 1) * 16;
		x_inc = -4;
		x = (mx + win_w + 1) * 16;
		wing_top[0] = tile_manager->get_tile(1792 + 32 + 6);
		wing_top[1] = tile_manager->get_tile(1792 + 32 + 7);

		wing_bottom[0] = tile_manager->get_tile(1792 + 24 + 6);
		wing_bottom[1] = tile_manager->get_tile(1792 + 24 + 7);
	} else {
		//left, right
		finish_x = (mx + win_w + 1) * 16;
		x_inc = 4;
		x = (mx - 1) * 16;
		wing_top[0] = tile_manager->get_tile(1792 + 24 + 2);
		wing_top[1] = tile_manager->get_tile(1792 + 24 + 3);

		wing_bottom[0] = tile_manager->get_tile(1792 + 32 + 2);
		wing_bottom[1] = tile_manager->get_tile(1792 + 32 + 3);
	}

	unpause();
}


WingAnim::~WingAnim() {

}


void WingAnim::start() {
	move(0, 0);

	p_tile_top = add_tile(wing_top[0], x / 16, (y - 16) / 16, x % 16, (y - 16) % 16);
	p_tile_bottom = add_tile(wing_bottom[0], x / 16, y / 16, x % 16, y % 16);
}



bool WingAnim::update() {
	if (is_paused())
		return true;

	x += x_inc;

	if (x == finish_x || Game::get_game()->get_script()->call_is_avatar_dead()) {
		message(MESG_ANIM_DONE);
		stop();
		return true;
	}

	move_tile(p_tile_top, x, y - 16);
	move_tile(p_tile_bottom, x, y);

	if (x == target.x * 16) {
		pause(); //we pause our anim here to avoid recursion from the hit effect called by our effect class parent.
		message(MESG_ANIM_HIT);
		unpause();
	}

	if (x % 16 == 0) { //flap wings.
		if (p_tile_top->tile == wing_top[0]) {
			p_tile_top->tile = wing_top[1];
			p_tile_bottom->tile = wing_bottom[1];
		} else {
			p_tile_top->tile = wing_top[0];
			p_tile_bottom->tile = wing_bottom[0];
		}
	}

	return (true);
}

HailstormAnim::HailstormAnim(MapCoord t) {
	target = t;
	hailstone_tile = Game::get_game()->get_tile_manager()->get_tile(0x18e); //hailstone tile.

	num_active = 0;
	for (int i = 0; i < HAILSTORM_ANIM_MAX_STONES; i++) {
		hailstones[i].length_left = 0;
		hailstones[i].p_tile = NULL;
	}

	num_hailstones_left = (NUVIE_RAND() % 20) + 10;

	unpause();
}

HailstormAnim::~HailstormAnim() {
}

void HailstormAnim::start() {
	move(0, 0);
}

bool HailstormAnim::update() {
	if (is_paused())
		return true;

	if (num_active < 6 && num_hailstones_left > 0) {
		if (NUVIE_RAND() % 2 == 0) {
			sint8 ret = find_free_hailstone();
			if (ret >= 0) {
				const uint8 idx = (uint8)ret;

				hailstones[idx].x = target.x * 16 + (NUVIE_RAND() % 30) + (NUVIE_RAND() % 30) - 23 - 52;
				hailstones[idx].y = target.y * 16 + (NUVIE_RAND() % 30) + (NUVIE_RAND() % 30) - 23 - 52;

				hailstones[idx].p_tile = add_tile(hailstone_tile, hailstones[idx].x / 16, hailstones[idx].y / 16, hailstones[idx].x % 16, hailstones[idx].y % 16);
				hailstones[idx].length_left = 52;
				num_hailstones_left--;
				num_active++;
			}
		}
	}

	for (int i = 0; i < HAILSTORM_ANIM_MAX_STONES; i++) {
		if (hailstones[i].length_left > 0) {
			hailstones[i].x += 4;
			hailstones[i].y += 4;
			move_tile(hailstones[i].p_tile, hailstones[i].x, hailstones[i].y);
			hailstones[i].length_left -= 4;

			if (hailstones[i].length_left == 0) {
				num_active--;
				remove_tile(hailstones[i].p_tile);
				hailstones[i].p_tile = NULL;

				uint8 z = 0;
				_mapWindow->get_level(&z);
				Actor *actor = Game::get_game()->get_actor_manager()->get_actor(hailstones[i].x / 16, hailstones[i].y / 16, z);
				if (actor) {
					pause();
					message(MESG_ANIM_HIT, actor);
					unpause();
					if (Game::get_game()->get_script()->call_is_avatar_dead()) {
						message(MESG_ANIM_DONE);
						stop();
						return true;
					}
				} else {
					//FIXME add sfx for hailstone hitting ground.
					Game::get_game()->get_sound_manager()->playSfx(NUVIE_SFX_HAIL_STONE);
				}
			}
		}
	}

	if (num_active == 0 && num_hailstones_left == 0) {
		message(MESG_ANIM_DONE);
		stop();
	}

	return true;
}

sint8 HailstormAnim::find_free_hailstone() {
	for (int i = 0; i < HAILSTORM_ANIM_MAX_STONES; i++) {
		if (hailstones[i].length_left == 0)
			return i;
	}

	return -1;
}

TileFadeAnim::TileFadeAnim(MapCoord *loc, Tile *from, Tile *to, uint16 speed) {
	init(speed);

	if (from != NULL) {
		anim_tile = new Tile(*from);
	} else {
		anim_tile = new Tile();
		memset(anim_tile->data, 0xff, TILE_DATA_SIZE);
	}

	anim_tile->transparent = true;

	if (to == NULL) {
		to_tile = new Tile();
		to_tile->transparent = true;
		memset(to_tile->data, 0xff, TILE_DATA_SIZE);
		should_delete_to_tile = true;
	} else {
		to_tile = to;
	}

	add_tile(anim_tile, 0, 0);
	move(loc->x, loc->y);
}

TileFadeAnim::TileFadeAnim(MapCoord *loc, Tile *from, uint8 color_from, uint8 color_to, bool reverse, uint16 speed) {
	init(speed);

	if (reverse) {
		to_tile = from;
		anim_tile = new Tile(*from);
		for (int i = 0; i < TILE_DATA_SIZE; i++) {
			if (anim_tile->data[i] == color_from)
				anim_tile->data[i] = color_to;
		}
	} else {
		to_tile = new Tile(*from);
		should_delete_to_tile = true;
		for (int i = 0; i < TILE_DATA_SIZE; i++) {
			if (to_tile->data[i] == color_from)
				to_tile->data[i] = color_to;
		}

		anim_tile = new Tile(*from);
	}

	add_tile(anim_tile, 0, 0);
	move(loc->x, loc->y);
}

TileFadeAnim::~TileFadeAnim() {
	delete anim_tile; //FIXME do we need to unlink from anim manager here!

	if (should_delete_to_tile)
		delete to_tile;
}

void TileFadeAnim::init(uint16 speed) {
	pixels_per_update = speed;
	pixel_count = 0;
	memset(mask, 0, 256);
	should_delete_to_tile = false;
}

bool TileFadeAnim::update() {
	for (int i = 0; i < pixels_per_update && pixel_count < TILE_DATA_SIZE;) {
		uint16 x = NUVIE_RAND() % TILE_WIDTH,
		       y = NUVIE_RAND() % TILE_HEIGHT;

		uint8 idx = y * TILE_WIDTH + x;

		if (mask[idx] == 0) {
			anim_tile->data[idx] = to_tile->data[idx];
			mask[idx] = 1;
			pixel_count++;
			i++;
		}
	}

	if (pixel_count == TILE_DATA_SIZE) {
		stop();
		message(MESG_ANIM_DONE);
	}

	return true;
}

} // End of namespace Nuvie
} // End of namespace Ultima
