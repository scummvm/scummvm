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
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/anim_manager.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/tile_manager.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/core/effect_manager.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/core/player.h"

#include "backends/keymapper/keymapper.h"

namespace Ultima {
namespace Nuvie {

#define MESG_ANIM_HIT_WORLD  ANIM_CB_HIT_WORLD
#define MESG_ANIM_HIT        ANIM_CB_HIT
#define MESG_ANIM_DONE       ANIM_CB_DONE
#define MESG_EFFECT_COMPLETE EFFECT_CB_COMPLETE
//#define MESG_INPUT_READY     EVENT_CB_INPUT_READY
#define MESG_INPUT_READY     MSGSCROLL_CB_TEXT_READY

#define TRANSPARENT_COLOR 0xFF /* transparent pixel color */

static const int EXP_EFFECT_TILE_NUM = 382;

QuakeEffect *QuakeEffect::current_quake = nullptr;
FadeEffect *FadeEffect::current_fade = nullptr;


/* Add self to effect list (for future deletion).
 */
Effect::Effect() : defunct(false), retain_count(0) {
	game = Game::get_game();
	effect_manager = game->get_effect_manager();
	effect_manager->add_effect(this);
}


Effect::~Effect() {
	// FIXME: should we remove self from Callbacks' default targets?
}


/* Start managing new animation. (AnimMgr will do that actually, but we point to
 * it and can stop it when necessary)
 */
void Effect::add_anim(NuvieAnim *anim) {
	anim->set_target(this); // add self as callback target for anim
	game->get_map_window()->get_anim_manager()->new_anim(anim);
}


static const int CANNON_SPEED = 320;

/* Fire from a cannon in direction: 0=north, 1=east, 2=south, 3=west,
 *                                 -1=use cannon frame
 */
CannonballEffect::CannonballEffect(Obj *src_obj, sint8 direction)
	: obj(src_obj) {
	usecode = game->get_usecode();
	target_loc = MapCoord(obj->x, obj->y, obj->z);

	if (direction == -1)
		direction = obj->frame_n;

	uint8 target_dist = 5; // distance that cannonball will fly
	if (direction == NUVIE_DIR_N)
		target_loc.y -= target_dist;
	else if (direction == NUVIE_DIR_E)
		target_loc.x += target_dist;
	else if (direction == NUVIE_DIR_S)
		target_loc.y += target_dist;
	else if (direction == NUVIE_DIR_W)
		target_loc.x -= target_dist;

	start_anim();
}


/* Pause world & start animation. */
void CannonballEffect::start_anim() {
	MapCoord obj_loc(obj->x, obj->y, obj->z);

	game->pause_world();
	game->pause_anims();
	game->pause_user();

	anim = new TossAnim(game->get_tile_manager()->get_tile(399),
	                    obj_loc, target_loc, CANNON_SPEED, TOSS_TO_BLOCKING | TOSS_TO_ACTOR | TOSS_TO_OBJECT);
	add_anim(anim);
}


/* Handle messages from animation. Hit actors & walls. */
uint16 CannonballEffect::callback(uint16 msg, CallBack *caller, void *msg_data) {
	bool stop_effect = false;
	Actor *hit_actor = nullptr;

	switch (msg) {
	case MESG_ANIM_HIT_WORLD: {
		const MapCoord *hit_loc = static_cast<const MapCoord *>(msg_data);
		const Tile *obj_tile = game->get_obj_manager()->get_obj_tile(hit_loc->x, hit_loc->y, hit_loc->z);
		const Tile *tile = game->get_game_map()->get_tile(hit_loc->x, hit_loc->y,
		             hit_loc->z);

		if ((tile->flags2 & TILEFLAG_MISSILE_BOUNDARY)
		        || (obj_tile && (obj_tile->flags2 & TILEFLAG_MISSILE_BOUNDARY))) {
			//new ExplosiveEffect(hit_loc->x, hit_loc->y, 2);
			new ExpEffect(EXP_EFFECT_TILE_NUM, *hit_loc);
			stop_effect = true;
		}
		break;
	}
	case MESG_ANIM_HIT: {
		MapEntity *hit_ent = static_cast<MapEntity *>(msg_data);
		if (hit_ent->entity_type == ENT_ACTOR) {
			//hit_ent->actor->hit(32);
			hit_actor = hit_ent->actor;
			stop_effect = true;
		}
		if (hit_ent->entity_type == ENT_OBJ) {
			DEBUG(0, LEVEL_DEBUGGING, "hit object %d at %x,%x,%x\n", hit_ent->obj->obj_n, hit_ent->obj->x, hit_ent->obj->y, hit_ent->obj->z);
			// FIX: U6 specific
			// FIX: hit any part of ship, and reduce qty of center
			if (hit_ent->obj->obj_n == 412) {
				uint8 f = hit_ent->obj->frame_n;
				if (f == 9 || f == 15 || f == 11 || f == 13) { // directions
					if (hit_ent->obj->qty < 20) hit_ent->obj->qty = 0;
					else                      hit_ent->obj->qty -= 20;
					if (hit_ent->obj->qty == 0)
						game->get_scroll()->display_string("Ship broke!\n");
					stop_effect = true;
				}
			}
		}
		break;
	}
	case MESG_ANIM_DONE:
		//new ExplosiveEffect(target_loc.x, target_loc.y, 3);
		new ExpEffect(EXP_EFFECT_TILE_NUM, MapCoord(target_loc.x, target_loc.y, target_loc.z));
		stop_effect = true;
		break;
	}

	if (stop_effect) {
		if (hit_actor) {
			anim->pause(); //pause to avoid recursive problems when animations are called from actor_hit() lua script.
			Game::get_game()->get_script()->call_actor_hit(hit_actor, 32, true);
		}
		if (msg != MESG_ANIM_DONE) // this msg means anim stopped itself
			anim->stop();
		game->unpause_all();
		usecode->message_obj(obj, MESG_EFFECT_COMPLETE, this);
		delete_self();
	}
	return 0;
}

#define EXP_EFFECT_SPEED 3


ExpEffect::ExpEffect(uint16 tileNum, const MapCoord &location) : ProjectileEffect(),
		exp_tile_num(tileNum) {
	start_loc = location;
	start_anim();
}


/* Pause world & start animation. */
void ExpEffect::start_anim() {
	game->pause_world();
	game->pause_anims();
	game->pause_user();

	targets.resize(16);

	targets[0] = MapCoord(start_loc.x + 2, start_loc.y - 1, start_loc.z);
	targets[1] = MapCoord(start_loc.x + 1, start_loc.y + 2, start_loc.z);
	targets[2] = MapCoord(start_loc.x, start_loc.y - 2, start_loc.z);

	targets[3] = MapCoord(start_loc.x + 1, start_loc.y - 1, start_loc.z);
	targets[4] = MapCoord(start_loc.x - 1, start_loc.y + 2, start_loc.z);
	targets[5] = MapCoord(start_loc.x - 1, start_loc.y - 1, start_loc.z);

	targets[6] = MapCoord(start_loc.x - 2, start_loc.y, start_loc.z);
	targets[7] = MapCoord(start_loc.x - 1, start_loc.y + 1, start_loc.z);
	targets[8] = MapCoord(start_loc.x, start_loc.y + 2, start_loc.z);

	targets[9] = MapCoord(start_loc.x - 1, start_loc.y - 2, start_loc.z);
	targets[10] = MapCoord(start_loc.x - 2, start_loc.y - 1, start_loc.z);
	targets[11] = MapCoord(start_loc.x - 2, start_loc.y + 1, start_loc.z);

	targets[12] = MapCoord(start_loc.x + 2, start_loc.y + 1, start_loc.z);
	targets[13] = MapCoord(start_loc.x + 2, start_loc.y, start_loc.z);
	targets[14] = MapCoord(start_loc.x + 1, start_loc.y + 1, start_loc.z);
	targets[15] = MapCoord(start_loc.x + 1, start_loc.y - 2, start_loc.z);


	anim = new ProjectileAnim(exp_tile_num, &start_loc, targets, EXP_EFFECT_SPEED, true);
	add_anim(anim);

}

ProjectileEffect::ProjectileEffect(uint16 tileNum, MapCoord start, MapCoord target, uint8 speed, bool trailFlag, uint16 initialTileRotation, uint16 rotationAmount, uint8 src_y_offset) {
	vector<MapCoord> t;
	t.push_back(target);

	init(tileNum, start, t, speed, trailFlag, initialTileRotation, rotationAmount, src_y_offset);
}

ProjectileEffect::ProjectileEffect(uint16 tileNum, MapCoord start, const vector<MapCoord> &t, uint8 speed, bool trailFlag, uint16 initialTileRotation) {
	init(tileNum, start, t, speed, trailFlag, initialTileRotation, 0, 0);
}

void ProjectileEffect::init(uint16 tileNum, MapCoord start, const vector<MapCoord> &t, uint8 speed, bool trailFlag, uint16 initialTileRotation, uint16 rotationAmount, uint8 src_y_offset) {
	finished_tiles = 0;

	tile_num = tileNum;
	start_loc = start;
	anim_speed = speed;
	trail = trailFlag;
	initial_tile_rotation = initialTileRotation;
	rotation_amount = rotationAmount;

	src_tile_y_offset = src_y_offset;
	targets = t;

	start_anim();
}

/* Pause world & start animation. */
void ProjectileEffect::start_anim() {
	game->pause_world();
	//game->pause_anims();
	game->pause_user();

	add_anim(new ProjectileAnim(tile_num, &start_loc, targets, anim_speed, trail, initial_tile_rotation, rotation_amount, src_tile_y_offset));

}
/* Handle messages from animation. Hit actors & walls. */
uint16 ProjectileEffect::callback(uint16 msg, CallBack *caller, void *msg_data) {
	bool stop_effect = false;
	switch (msg) {
	case MESG_ANIM_HIT_WORLD: {
		MapCoord *hit_loc = static_cast<MapCoord *>(msg_data);
		const Tile *tile = game->get_game_map()->get_tile(hit_loc->x, hit_loc->y,
		             hit_loc->z);
		if (tile->flags1 & TILEFLAG_WALL) {
			//new ExplosiveEffect(hit_loc->x, hit_loc->y, 2);
			stop_effect = true;
		}
		break;
	}
	case MESG_ANIM_HIT: {
		MapEntity *hit_ent = static_cast<MapEntity *>(msg_data);
		hit_entities.push_back(*hit_ent);
		break;
	}
	case MESG_ANIM_DONE:
		//new ExplosiveEffect(target_loc.x, target_loc.y, 3);
		stop_effect = true;
		break;
	}

	if (stop_effect) {
		//finished_tiles++;

		if (msg != MESG_ANIM_DONE) // this msg means anim stopped itself
			((NuvieAnim *)caller)->stop();
		//if(finished_tiles == 16)
		// {
		game->unpause_world();
		game->unpause_user();
		game->unpause_anims();
		//usecode->message_obj(obj, MESG_EFFECT_COMPLETE, this);
		delete_self();
		// }
	}
	return 0;
}


/*** TimedEffect ***/
void TimedEffect::start_timer(uint32 delay) {
	if (!timer)
		timer = new TimedCallback(this, nullptr, delay, true);
}


void TimedEffect::stop_timer() {
	if (timer) {
		timer->clear_target();
		timer = nullptr;
	}
}


/*** QuakeEffect ***/
/* Shake the visible play area around for `duration' milliseconds. Magnitude
 * determines the speed of movement. An actor may be selected to keep the
 * MapWindow centered on after the Quake.
 */
QuakeEffect::QuakeEffect(uint8 magnitude, uint32 duration, Actor *keep_on)
		: strength(magnitude), orig_actor(keep_on), sx(0), sy(0),
		  map_window(nullptr), stop_time(0) {
	// single use only, so MapWindow doesn't keep moving away from center
	// ...and do nothing if magnitude isn't usable
	if (current_quake || magnitude == 0) {
		delete_self();
		return;
	}
	current_quake = this; // cleared in timer function

	map_window = game->get_map_window();
	stop_time = game->get_clock()->get_ticks() + duration;

	// get random direction (always move left-right more than up-down)
	init_directions();

	map_window->get_pos(&orig.x, &orig.y);
	map_window->get_level(&orig.z);
	map_window->set_freeze_blacking_location(true);

	start_timer(strength * 5);
}


QuakeEffect::~QuakeEffect() {
}


/* On TIMED: Move map.
 */
uint16 QuakeEffect::callback(uint16 msg, CallBack *caller, void *msg_data) {
	//  uint8 twice_strength = strength * 2;
	if (msg != MESG_TIMED)
		return 0;
	if (game->get_clock()->get_ticks() >= stop_time) {
		stop_quake();
		return 0;
	}
	recenter_map();
	map_window->shiftMapRelative(sx, sy);

	// move in opposite direction on next call
	if (sx == -(4 * strength) || sx == (4 * strength))
		sx = (sx == -(4 * strength)) ? (2 * strength) : -(2 * strength);
	else if (sx == -(2 * strength) || sx == (2 * strength))
		sx = 0;

	if (sy == -(2 * strength) || sy == (2 * strength))
		sy = 0;

	if (sx == 0 && sy == 0)
		init_directions();
	return 0;
}


/* Finish effect. Move map back to initial position.
 */
void QuakeEffect::stop_quake() {
	current_quake = nullptr;
	map_window->set_freeze_blacking_location(false);
	recenter_map();
	delete_self();
}


/* Set sx,sy to a random direction. (always move left-right more than up-down)
 */
void QuakeEffect::init_directions() {
	NuvieDir dir = static_cast<NuvieDir>(NUVIE_RAND() % 8);
	sx = 0;
	sy = 0;

	switch (dir) {
	default: // can't happen, but make the analyzer happy.
	case NUVIE_DIR_N :
		sy = -(strength * 2);
		break;
	case NUVIE_DIR_NE :
		sx = (strength * 4);
		sy = -(strength * 2);
		break;
	case NUVIE_DIR_E :
		sx = (strength * 4);
		break;
	case NUVIE_DIR_SE :
		sx = (strength * 4);
		sy = (strength * 2);
		break;
	case NUVIE_DIR_S :
		sy = (strength * 2);
		break;
	case NUVIE_DIR_SW :
		sx = -(strength * 4);
		sy = (strength * 2);
		break;
	case NUVIE_DIR_W :
		sx = -(strength * 4);
		break;
	case NUVIE_DIR_NW :
		sx = -(strength * 4);
		sy = -(strength * 2);
		break;
	}
}


/* Center map on original actor or move to original location.
 */
void QuakeEffect::recenter_map() {
	if (orig_actor)
		map_window->centerMapOnActor(orig_actor);
	else
		map_window->moveMap(orig.x, orig.y, orig.z);
}


/*** HitEffect ***/
/* Hit target actor. FIXME: implement duration and hitting a location
 */
HitEffect::HitEffect(Actor *target, uint32 duration) {
	game->pause_user();
	add_anim(new HitAnim(target));
	Game::get_game()->get_sound_manager()->playSfx(NUVIE_SFX_HIT); //FIXME use NUVIE_SFX_SAMPLE defines here.
}

HitEffect::HitEffect(const MapCoord &location) {
	game->pause_user();
	add_anim(new HitAnim(location));
	Game::get_game()->get_sound_manager()->playSfx(NUVIE_SFX_HIT); //FIXME use NUVIE_SFX_SAMPLE defines here.
}

/* On ANIM_DONE: end
 */
uint16 HitEffect::callback(uint16 msg, CallBack *caller, void *msg_data) {
	if (msg == MESG_ANIM_DONE) {
		game->unpause_user();
		delete_self();
	}
	return 0;
}

TextEffect::TextEffect(Std::string text) { // default somewhat centered on player for cheat messages
	const MapWindow *map_window = game->get_map_window();
	if (!map_window || map_window->Status() != WIDGET_VISIBLE) // scripted sequence like intro and intro menu
		return;
	MapCoord loc = game->get_player()->get_actor()->get_location();
	loc.x = (loc.x - map_window->get_cur_x() - 2) * 16;
	loc.y = (loc.y - map_window->get_cur_y() - 1) * 16;

	add_anim(new TextAnim(text, loc, 1500));
}

/*** TextEffect ***/
/* Print Text to MapWindow for duration
 */
TextEffect::TextEffect(Std::string text, const MapCoord &location) {
	add_anim(new TextAnim(text, location, 1500));
}

/* On ANIM_DONE: end
 */
uint16 TextEffect::callback(uint16 msg, CallBack *caller, void *msg_data) {
	if (msg == MESG_ANIM_DONE) {
		delete_self();
	}
	return 0;
}

/*** ExplosiveEffect ***/
ExplosiveEffect::ExplosiveEffect(uint16 x, uint16 y, uint32 size, uint16 dmg)
	: start_at(), anim(nullptr) {
	start_at.x = x;
	start_at.y = y;
	radius = size;
	hit_damage = dmg;

	start_anim();
}


/* Pause world & start animation.
 */
void ExplosiveEffect::start_anim() {
	game->pause_world();
	game->pause_user();
	add_anim(new ExplosiveAnim(start_at, radius));
}


/* Handle messages from animation. Hit actors & objects.
 */
uint16 ExplosiveEffect::callback(uint16 msg, CallBack *caller, void *msg_data) {
	bool stop_effect = false;
	switch (msg) {
	case MESG_ANIM_HIT: {
		MapEntity *hit_ent = static_cast<MapEntity *>(msg_data);
		if (hit_ent->entity_type == ENT_ACTOR) {
			if (hit_damage != 0) // hit actor if effect causes damage
				hit_ent->actor->hit(hit_damage);
		} else if (hit_ent->entity_type == ENT_OBJ) {
			DEBUG(0, LEVEL_DEBUGGING, "Explosion hit object %d (%x,%x)\n", hit_ent->obj->obj_n, hit_ent->obj->x, hit_ent->obj->y);
			stop_effect = hit_object(hit_ent->obj);
		}
		break;
	}
	case MESG_ANIM_DONE:
		stop_effect = true;
		break;
	}

	if (stop_effect) {
		if (msg != MESG_ANIM_DONE)
			anim->stop();
		game->unpause_world();
		game->unpause_user();
		delete_self();
	}
	return 0;
}


/* UseCodeExplosiveEffect: before deleting send message to source object
 */
void UseCodeExplosiveEffect::delete_self() {
	if (obj)
		game->get_usecode()->message_obj(obj, MESG_EFFECT_COMPLETE, this);
	Effect::delete_self();
}


/* The explosion hit an object.
 * Returns true if the effect should end, false to continue.
 */
bool UseCodeExplosiveEffect::hit_object(Obj *hit_obj) {
	// ignite & destroy powder kegs (U6)
	if (hit_obj->obj_n == 223 && hit_obj != original_obj) {
		// FIXME: this doesn't belong here (U6/obj specific)
		uint16 x = hit_obj->x, y = hit_obj->y;
		game->get_obj_manager()->remove_obj_from_map(hit_obj);
		delete_obj(hit_obj);
		if (obj) // pass our source obj on to next effect as original_obj
			new UseCodeExplosiveEffect(nullptr, x, y, 2, hit_damage, obj);
		else // pass original_obj on to next effect
			new UseCodeExplosiveEffect(nullptr, x, y, 2, hit_damage, original_obj);
	}
	return false;
}


/*** ThrowObjectEffect ***/
ThrowObjectEffect::ThrowObjectEffect() {
	obj_manager = game->get_obj_manager();

	anim = nullptr;
	throw_obj = nullptr;
	throw_tile = nullptr;
	throw_speed = 0;
	degrees = 0;
	stop_flags = 0;
}


void ThrowObjectEffect::start_anim() {
	game->pause_anims();
	game->pause_world();
	game->pause_user();

	assert(throw_tile || throw_obj); // make sure it was properly initialized
	assert(throw_speed != 0);

	if (throw_obj)
		anim = new TossAnim(throw_obj, degrees, start_at, stop_at, throw_speed, stop_flags);
	else
		anim = new TossAnim(throw_tile, start_at, stop_at, throw_speed, stop_flags);
	add_anim(anim);
}


/* Object has stopped. */
void ThrowObjectEffect::hit_target() {
	if (anim)
		anim->stop();
	game->unpause_all();
	delete_self();
}


/* The animation will travel from original object location to drop location if
 * nullptr actor is specified.
 */
DropEffect::DropEffect(Obj *obj, uint16 qty, Actor *actor, MapCoord *drop_loc) {
	drop_from_actor = actor;
	start_at = drop_from_actor ? drop_from_actor->get_location() : MapCoord(obj->x, obj->y, obj->z);
	stop_at = *drop_loc;
	degrees = 90;

	get_obj(obj, qty); // remove from actor, set throw_obj

	if (start_at != stop_at) {
		throw_speed = 192; // animation speed
		start_anim();
	} else
		hit_target(); // done already? why bother calling DropEffect? :p
}


/* Take `qty' objects of a stack if necessary, and remove from the actor's
 * inventory. Set `throw_obj'.
 */
void DropEffect::get_obj(Obj *obj, uint16 qty) {
	throw_obj = obj_manager->get_obj_from_stack(obj, qty);
	if (drop_from_actor)
		drop_from_actor->inventory_remove_obj(throw_obj);
}


/* On ANIM_HIT_WORLD: end at hit location
 * On ANIM_DONE: end
 */
uint16 DropEffect::callback(uint16 msg, CallBack *caller, void *msg_data) {
	// if throw_obj is nullptr, object already hit target
	if (!throw_obj || (msg != MESG_ANIM_DONE && msg != MESG_ANIM_HIT_WORLD))
		return 0;

	if (msg == MESG_ANIM_HIT_WORLD && stop_at == *(MapCoord *)msg_data
	        && anim)
		anim->stop();

	hit_target();
	return 0;
}


/* Add object to map. (call before completing effect) */
void DropEffect::hit_target() {
	throw_obj->x = stop_at.x;
	throw_obj->y = stop_at.y;
	throw_obj->z = stop_at.z;

	//FIXME drop logic should probably be in lua script.
	if (drop_from_actor && obj_manager->is_breakable(throw_obj) && start_at.distance(stop_at) > 1) {
		nuvie_game_t game_type = game->get_game_type();
		if (game_type == NUVIE_GAME_U6 && throw_obj->obj_n == OBJ_U6_DRAGON_EGG) {
			throw_obj->frame_n = 1; //brake egg.
			obj_manager->add_obj(throw_obj, OBJ_ADD_TOP);
		} else if (game_type == NUVIE_GAME_U6 && throw_obj->obj_n == OBJ_U6_MIRROR) {
			throw_obj->frame_n = 2; //break mirror.
			obj_manager->add_obj(throw_obj, OBJ_ADD_TOP);
		} else {
			// remove items from container if there is one
			if (game->get_usecode()->is_container(throw_obj)) {
				U6Link *link = throw_obj->container->start();
				for (; link != nullptr; link = throw_obj->container->start()) {
					Obj *obj = (Obj *)link->data;
					obj_manager->moveto_map(obj, stop_at);
				}
			}
			obj_manager->unlink_from_engine(throw_obj);
			delete_obj(throw_obj);
		}

		Game::get_game()->get_scroll()->display_string("\nIt broke!\n");
		Game::get_game()->get_sound_manager()->playSfx(NUVIE_SFX_BROKEN_GLASS);
	} else {
		Obj *dest_obj = obj_manager->get_obj(stop_at.x, stop_at.y, stop_at.z);
		if (obj_manager->can_store_obj(dest_obj, throw_obj))
			obj_manager->moveto_container(throw_obj, dest_obj);
		else
			obj_manager->add_obj(throw_obj, OBJ_ADD_TOP);
	}
	throw_obj = nullptr; // set as dropped

	// not appropriate to do "Events::endAction(true)" from here to display
	// prompt, as we MUST unpause_user() in ThrowObjectEffect::hit_target, and
	// that would be redundant and may not unpause everything if wait mode was
	// already cancelled... so just prompt
	game->get_scroll()->display_string("\n");
	game->get_scroll()->display_prompt();

	game->get_map_window()->updateBlacking();
	ThrowObjectEffect::hit_target(); // calls delete_self()
}


/*** MissileEffect ***/
MissileEffect::MissileEffect(uint16 tile_num, uint16 obj_n, const MapCoord &source,
							 const MapCoord &target, uint8 dmg,
							 uint8 intercept, uint16 speed) {
	actor_manager = game->get_actor_manager();
	hit_actor = 0;
	hit_obj = 0;

	init(tile_num, obj_n, source, target, dmg, intercept, speed);
}

/* Start effect. If target is unset then the actor is the target. */
void MissileEffect::init(uint16 tile_num, uint16 obj_n,
						 const MapCoord &source, const MapCoord &target,
						 uint32 dmg, uint8 intercept, uint32 speed) {
	assert(tile_num || obj_n); // at least obj_n must be set
	// (although it might work if throw_obj is already set)
	assert(speed != 0);
	assert(intercept != 0); // must hit target

	if (obj_n != 0)
		throw_obj = new_obj(obj_n, 0, 0, 0, 0);
	if (tile_num != 0)
		throw_tile = game->get_tile_manager()->get_tile(tile_num);
	else if (throw_obj != 0)
		throw_tile = obj_manager->get_obj_tile(throw_obj->obj_n, 0);
	throw_speed = speed;
	hit_damage = dmg;

	start_at = source;
	stop_at = target;
	stop_flags = intercept;
	assert(stop_at != start_at); // Hmm, can't attack self with boomerang then
//    if (stop_at != start_at) {
//      start_at.x=WRAPPED_COORD(start_at.x+1,start_at.z);
//      start_at.y=WRAPPED_COORD(start_at.y-1,start_at.z);
//    }

	// set tile rotation here based on obj_num
	if (throw_obj != 0) {
		if (throw_obj->obj_n == OBJ_U6_SPEAR)
			degrees = 315;
		if (throw_obj->obj_n == OBJ_U6_THROWING_AXE)
			degrees = 0;
		if (throw_obj->obj_n == OBJ_U6_DAGGER)
			degrees = 315;
		if (throw_obj->obj_n == OBJ_U6_ARROW)
			degrees = 270;
		if (throw_obj->obj_n == OBJ_U6_BOLT)
			degrees = 270;
	}
	start_anim();
}

/* On HIT: hit Actor or Obj and end
 * On HIT_WORLD: end at hit location, hit Actor or Obj, else place obj
 * On DONE: end
 */
uint16 MissileEffect::callback(uint16 msg, CallBack *caller, void *msg_data) {
	if (msg != MESG_ANIM_DONE && msg != MESG_ANIM_HIT_WORLD && msg != MESG_ANIM_HIT)
		return 0;

	if (msg == MESG_ANIM_DONE) {
		// will always hit anything at the target
		// FIXME: only hit breakable objects like doors
//        hit_obj = obj_manager->get_obj(stop_at.x,stop_at.y,stop_at.z);
		hit_actor = actor_manager->get_actor(stop_at.x, stop_at.y, stop_at.z);
		hit_target();
	} else if (msg == MESG_ANIM_HIT && ((MapEntity *)msg_data)->entity_type == ENT_ACTOR) {
		if (hit_damage != 0)
			hit_actor = ((MapEntity *)msg_data)->actor;
		hit_target();
	} else if (msg == MESG_ANIM_HIT && ((MapEntity *)msg_data)->entity_type == ENT_OBJ) {
		// FIXME: only hit breakable objects like doors
		/*        if(hit_damage != 0)
		            hit_obj = ((MapEntity*)msg_data)->obj;
		        hit_target();*/
	}
	// MESG_ANIM_HIT_WORLD
	hit_blocking();
	return 0;
}

/* Hit target or add object to map. (call before completing effect) */
void MissileEffect::hit_target() {
	if (hit_actor) {
		hit_actor->hit(hit_damage, ACTOR_FORCE_HIT);
		delete_obj(throw_obj);
		throw_obj = 0; // don't drop
	} else if (hit_obj) {
		if (hit_obj->qty < hit_damage)
			hit_obj->qty = 0;
		else hit_obj->qty -= hit_damage;
		delete_obj(throw_obj);
		throw_obj = 0; // don't drop
	}
	if (throw_obj != 0) {
		throw_obj->x = stop_at.x;
		throw_obj->y = stop_at.y;
		throw_obj->z = stop_at.z;
		throw_obj->status |= OBJ_STATUS_OK_TO_TAKE | OBJ_STATUS_TEMPORARY;
		if (obj_manager->is_stackable(throw_obj))
			throw_obj->qty = 1; // stackable objects must have a quantity
		obj_manager->add_obj(throw_obj, OBJ_ADD_TOP);
		throw_obj = 0;
	}

	ThrowObjectEffect::hit_target(); // calls delete_self()
}

void MissileEffect::hit_blocking() {
	delete_obj(throw_obj);
	ThrowObjectEffect::hit_target();
}


/*** SleepEffect ***/
/* The TimedAdvance is started after the fade-out completes. */
SleepEffect::SleepEffect(Std::string until)
	: timer(nullptr),
	  stop_hour(0),
	  stop_minute(0),
	  stop_time("") {
	stop_time = until;
	game->pause_user();
	effect_manager->watch_effect(this, new FadeEffect(FADE_PIXELATED, FADE_OUT));
}


SleepEffect::SleepEffect(uint8 to_hour)
	: timer(nullptr),
	  stop_hour(to_hour),
	  stop_minute(0),
	  stop_time("") {
	game->pause_user();
	effect_manager->watch_effect(this, new FadeEffect(FADE_PIXELATED, FADE_OUT));
}


SleepEffect::~SleepEffect() {
	//if(timer) // make sure it doesn't try to call us again
	//    timer->clear_target();
}


/* As with TimedEffect, make sure the timer doesn't try to use callback again.
 */
void SleepEffect::delete_self() {
	//timer->clear_target(); // this will also stop/delete the TimedAdvance
	//timer = nullptr;
	Effect::delete_self();
}


/* Resume normal play when requested time has been reached.
 */
//FIXME: need to handle TimedAdvance() errors and fade-in
uint16 SleepEffect::callback(uint16 msg, CallBack *caller, void *data) {
	uint8 hour = Game::get_game()->get_clock()->get_hour();
	uint8 minute = Game::get_game()->get_clock()->get_minute();

	// waited for FadeEffect
	if (msg == MESG_EFFECT_COMPLETE) {
		if (timer == nullptr) { // starting
			if (stop_time != "") { // advance to start time
				timer = new TimedAdvance(stop_time, 360); // 6 hours per second FIXME: it isn't going anywhere near that fast
				timer->set_target(this);
				timer->get_time_from_string(stop_hour, stop_minute, stop_time);
				// stop_hour & stop_minute are checked each hour
			} else { // advance a number of hours
				uint16 advance_h = (hour == stop_hour) ? 24
				                   : (hour < stop_hour) ? (stop_hour - hour)
				                   : (24 - (hour - stop_hour));
				timer = new TimedAdvance(advance_h, 360);
				timer->set_target(this);
				stop_minute = minute;
			}
		} else { // stopping
			Party *party = game->get_party();
			for (int s = 0; s < party->get_party_size(); s++) {
				Actor *actor = party->get_actor(s);

				//heal actors.
				uint8 hp_diff = actor->get_maxhp() - actor->get_hp();
				if (hp_diff > 0) {
					if (hp_diff == 1)
						hp_diff = 2;
					actor->set_hp(actor->get_hp() + NUVIE_RAND() % (hp_diff / 2) + hp_diff / 2);
				}
			}
			game->unpause_user();
			delete_self();
		}
		return 0;
	}
	// assume msg == MESG_TIMED; will stop after effect completes
	if (hour == stop_hour && minute >= stop_minute)
		effect_manager->watch_effect(this, new FadeEffect(FADE_PIXELATED, FADE_IN));

	return 0;
}


/*** FadeEffect ***/
static const int FADE_EFFECT_MAX_ITERATIONS = 20;

FadeEffect::FadeEffect(FadeType fade, FadeDirection dir, uint32 color, uint32 speed) {
	speed = speed ? speed : game->get_map_window()->get_win_area() * 2116; // was 256000
	init(fade, dir, color, nullptr, 0, 0, speed);
}

/* Takes an image to fade from/to. */
FadeEffect::FadeEffect(FadeType fade, FadeDirection dir, Graphics::ManagedSurface *capture, uint32 speed) {
	speed = speed ? speed : game->get_map_window()->get_win_area() * 1620; // was 196000
	init(fade, dir, 0, capture, 0, 0, speed); // color=black
}

/* Localizes effect to specific coordinates. The size of the effect is determined
 * by the size of the image. */
FadeEffect::FadeEffect(FadeType fade, FadeDirection dir, Graphics::ManagedSurface *capture, uint16 x, uint16 y, uint32 speed) {
	speed = speed ? speed : 1024;
	init(fade, dir, 0, capture, x, y, speed); // color=black
}


void FadeEffect::init(FadeType fade, FadeDirection dir, uint32 color, Graphics::ManagedSurface *capture, uint16 x, uint16 y, uint32 speed) {
	if (current_fade) {
		delete_self();
		return;
	}
	current_fade = this; // cleared in dtor

	screen = game->get_screen();
	map_window = game->get_map_window();
	viewport = new Common::Rect(map_window->GetRect());

	fade_type = fade;
	fade_dir = dir;
	fade_speed = speed; // pixels-per-second (to check, not draw)

	evtime = prev_evtime = 0;
	fade_x = x;
	fade_y = y;
	fade_from = nullptr;
	fade_iterations = 0;
	if (capture) {
		fade_from = new Graphics::ManagedSurface(capture->w, capture->h, capture->format);
		fade_from->blitFrom(*capture);
	}

	if (fade_type == FADE_PIXELATED || fade_type == FADE_PIXELATED_ONTOP) {
		pixelated_color = color;
		init_pixelated_fade();
	} else
		init_circle_fade();
}


FadeEffect::~FadeEffect() {
	//moved to delete_self
}

void FadeEffect::delete_self() {
	if (current_fade == this) { // these weren't init. if FadeEffect didn't start
		delete viewport;
		if (fade_dir == FADE_IN) // overlay should be empty now, so just delete it
			map_window->set_overlay(nullptr);
		if (fade_from) {
			delete fade_from;
			fade_from = nullptr;
		}

		current_fade = nullptr;
	}

	TimedEffect::delete_self();
}

/* Start effect. */
void FadeEffect::init_pixelated_fade() {
	int fillret = -1; // check error
	overlay = map_window->get_overlay();
	if (overlay != nullptr) {
		pixel_count = fade_from ? (fade_from->w) * (fade_from->h)
		              : (overlay->w - fade_x) * (overlay->h - fade_y);
		// clear overlay to fill color or transparent
		if (fade_dir == FADE_OUT) {
			if (fade_from) { // fade from captured surface to transparent
				// put surface on transparent background (not checked)
				fillret = SDL_FillRect(overlay, nullptr, uint32(TRANSPARENT_COLOR));
				Common::Rect overlay_rect(fade_x, fade_y, fade_x, fade_y);
				fillret = SDL_BlitSurface(fade_from, nullptr, overlay, &overlay_rect);
			} else // fade from transparent to color
				fillret = SDL_FillRect(overlay, nullptr, uint32(TRANSPARENT_COLOR));
		} else {
			if (fade_from) // fade from transparent to captured surface
				fillret = SDL_FillRect(overlay, nullptr, uint32(TRANSPARENT_COLOR));
			else // fade from color to transparent
				fillret = SDL_FillRect(overlay, nullptr, uint32(pixelated_color));
		}
	}
	if (fillret == -1) {
		DEBUG(0, LEVEL_DEBUGGING, "FadeEffect: error creating overlay surface\n");
		delete_self();
		return;
	}

	// if FADE_PIXELATED_ONTOP is set, place the effect layer above the map border
	map_window->set_overlay_level((fade_type == FADE_PIXELATED)
	                              ? MAP_OVERLAY_DEFAULT : MAP_OVERLAY_ONTOP);
	colored_total = 0;
	start_timer(1); // fire timer continuously
}


/* Start effect.
 */
void FadeEffect::init_circle_fade() {
	delete_self(); // FIXME
}


/* Called by the timer as frequently as possible. Do the appropriate
 * fade method and stop when the effect is complete.
 */
uint16 FadeEffect::callback(uint16 msg, CallBack *caller, void *data) {
	bool fade_complete = false;

	// warning: msg is assumed to be CB_TIMED and data is set
	evtime = *(uint32 *)(data);

	// do effect
	if (fade_type == FADE_PIXELATED || fade_type == FADE_PIXELATED_ONTOP)
		fade_complete = (fade_dir == FADE_OUT) ? pixelated_fade_out() : pixelated_fade_in();
	else /* CIRCLE */
		fade_complete = (fade_dir == FADE_OUT) ? circle_fade_out() : circle_fade_in();

	// done
	if (fade_complete == true) {
		delete_self();
		return 1;
	}
	return 0;
}


/* Scan the overlay, starting at pixel rnum, for a transparent pixel if fading
 * out, and a colored pixel if fading in.
 * Returns true if a free pixel was found and set as rnum.
 */
// FIXME: this probably doesn't work because it only handles 8bpp
inline bool FadeEffect::find_free_pixel(uint32 &rnum, uint32 pixelCount) {
	uint8 scan_color = (fade_dir == FADE_OUT) ? TRANSPARENT_COLOR
	                   : pixelated_color;
	const uint8 *pixels = (const uint8 *)(overlay->getPixels());

	for (uint32 p = rnum; p < pixelCount; p++) // check all pixels after rnum
		if (pixels[p] == scan_color) {
			rnum = p;
			return true;
		}
	for (uint32 q = 0; q < rnum; q++) // check all pixels before rnum
		if (pixels[q] == scan_color) {
			rnum = q;
			return true;
		}
	return false;
}


/* Returns the next pixel to check/colorize. */
#if 0
#warning this crashes if x,y is near boundary
#warning make sure center_thresh doesn't go over boundary
inline uint32 FadeEffect::get_random_pixel(uint16 center_thresh) {
	if (center_x == -1 || center_y == -1)
		return (NUVIE_RAND() % pixel_count);

	uint16 x = center_x, y = center_y;
	if (center_thresh == 0)
		center_thresh = overlay->w / 2;
	x += (NUVIE_RAND() % (center_thresh * 2)) - center_thresh,
	     y += (NUVIE_RAND() % (center_thresh * 2)) - center_thresh;
	return ((y * overlay->w) + x);
}
#endif

/* Randomly add pixels of the appropriate color to the overlay. If the color
 * is -1, it will be taken from the "fade_from" surface.
 * Returns true when the overlay is completely colored.
 */
bool FadeEffect::pixelated_fade_core(uint32 pixels_to_check, sint16 fade_to) {
	Graphics::Surface s = overlay->getSubArea(Common::Rect(0, 0, overlay->w, overlay->h));
	uint8 *pixels = (uint8 *)s.getPixels();
	const uint8 *from_pixels = fade_from ? (const uint8 *)(fade_from->getPixels()) : nullptr;
	uint32 p = 0; // scan counter
	uint32 rnum = 0; // pixel index
	uint32 colored = 0; // number of pixels that get colored
	uint16 fade_width = fade_from ? fade_from->w : overlay->w - fade_x;
	uint16 fade_height = fade_from ? fade_from->h : overlay->h - fade_y;
	uint8 color = fade_to;

	if (fade_to == -1 && fade_from == nullptr) {
		return false;
	}

	while (p < pixels_to_check) {
		uint16 x = uint16(float(NUVIE_RAND()) * (fade_width - 1) / NUVIE_RAND_MAX) + fade_x,
		       y = uint16(float(NUVIE_RAND()) * (fade_height - 1) / NUVIE_RAND_MAX) + fade_y;
		if (x >= overlay->w) x = overlay->w - 1; // prevent overflow if fade_from is too big
		if (y >= overlay->h) y = overlay->h - 1;
		rnum = y * overlay->w + x;
//ERIC        rnum = y*overlay->pitch + x;

		if (fade_to == -1) { // get color from "fade_from"
			x -= fade_x;
			y -= fade_y;
			color = from_pixels[y * fade_from->w + x];
		}
		if (pixels[rnum] != color) {
			pixels[rnum] = color;
			++colored;
			++colored_total; // another pixel was set
		}
		++p;
	}
	(void)colored; // Fix warning about unused variable
	// all but two lines colored
	if (colored_total >= (pixel_count - fade_width * 2) || fade_iterations > FADE_EFFECT_MAX_ITERATIONS) { // fill the rest
		if (fade_to >= 0)
			SDL_FillRect(overlay, nullptr, (uint32)fade_to);
		else { // Note: assert(fade_from) if(fade_to < 0)
			Common::Rect fade_from_rect(fade_from->w, (int16)fade_from->h);
			Common::Rect overlay_rect(fade_x, fade_y, fade_x + fade_from->w, fade_y + fade_from->h);
			SDL_BlitSurface(fade_from, &fade_from_rect, overlay, &overlay_rect);
		}
		return true;
	} else return false;
}


/* Color some of the mapwindow.
 * Returns true when all pixels have been filled, and nothing is visible.
 */
bool FadeEffect::pixelated_fade_out() {
	if (fade_from)
		return (pixelated_fade_core(pixels_to_check(), TRANSPARENT_COLOR));
	return (pixelated_fade_core(pixels_to_check(), pixelated_color));
}


/* Clear some of the mapwindow.
 * Returns true when all colored pixels have been removed, and the MapWindow
 * is visible.
 */
bool FadeEffect::pixelated_fade_in() {
	if (fade_from)
		return (pixelated_fade_core(pixels_to_check(), -1));
	return (pixelated_fade_core(pixels_to_check(), TRANSPARENT_COLOR));
}


/* Returns the number of pixels that should be checked/colored (based on speed)
 * since the previous call.
 */
uint32 FadeEffect::pixels_to_check() {
	uint32 time_passed = (prev_evtime == 0) ? 0 : evtime - prev_evtime;
	uint32 fraction = 1000 / (time_passed > 0 ? time_passed : 1); // % of second passed, in milliseconds
	uint32 pixels_per_fraction = fade_speed / (fraction > 0 ? fraction : 1);
	prev_evtime = evtime;
	fade_iterations++;
	return pixels_per_fraction;
}


/* Reduce the MapWindow's ambient light level, according to the set speed.
 * Returns true when nothing is visible.
 */
bool FadeEffect::circle_fade_out() {
// FIXME
	return false;
}


/* Add to the MapWindow's ambient light level, according to the set speed.
 * Returns true when the light level has returned to normal.
 */
bool FadeEffect::circle_fade_in() {
// FIXME
	return false;
}


/* Pause game and do FadeEffect.
 */
GameFadeInEffect::GameFadeInEffect(uint32 color)
	: FadeEffect(FADE_PIXELATED_ONTOP, FADE_IN, color) {
	game->pause_user();
}


GameFadeInEffect::~GameFadeInEffect() {
}


/* Identical to FadeEffect, but unpause game when finished.
 */
uint16 GameFadeInEffect::callback(uint16 msg, CallBack *caller, void *data) {
	// done
	if (FadeEffect::callback(msg, caller, data) != 0)
		game->unpause_user();
	return 0;
}


FadeObjectEffect::FadeObjectEffect(Obj *obj, FadeDirection dir) {
	obj_manager = game->get_obj_manager();
	fade_obj = obj;
	fade_dir = dir;

	Graphics::ManagedSurface *capture = game->get_map_window()->get_sdl_surface();
	if (fade_dir == FADE_IN) { // fading IN to object, so fade OUT from capture
		effect_manager->watch_effect(this, /* call me */
		                             new FadeEffect(FADE_PIXELATED, FADE_OUT, capture));
		obj_manager->add_obj(fade_obj, OBJ_ADD_TOP);
		game->get_map_window()->updateBlacking(); // object is likely a moongate
	} else if (fade_dir == FADE_OUT) {
		effect_manager->watch_effect(this, /* call me */
		                             new FadeEffect(FADE_PIXELATED, FADE_OUT, capture, 0, 0, game->get_map_window()->get_win_area() * 1058)); //was 128000
//        obj_manager->remove_obj(fade_obj);
		game->get_map_window()->updateBlacking();
	}
	delete capture;

	game->pause_user();
}

FadeObjectEffect::~FadeObjectEffect() {
	game->unpause_user();
}

/* Assume FadeEffect is complete. */
uint16 FadeObjectEffect::callback(uint16 msg, CallBack *caller, void *data) {
	delete_self();
	return 0;
}


/* These types of local/vanish effects are slightly longer than a normal Fade.
 * FIXME: FadeEffect should take local effect area, or change speed to time.
 */
VanishEffect::VanishEffect(bool pause_user)
	: input_blocked(pause_user) {
	Graphics::ManagedSurface *capture = game->get_map_window()->get_sdl_surface();
//    effect_manager->watch_effect(this, /* call me */
//                                 new FadeEffect(FADE_PIXELATED, FADE_OUT, capture, 0, 0, 128000));
	effect_manager->watch_effect(this, /* call me */
	                             new FadeEffect(FADE_PIXELATED, FADE_OUT, capture));
	delete capture;

	if (input_blocked == VANISH_WAIT)
		game->pause_user();
	game->pause_anims();
}

VanishEffect::~VanishEffect() {
	game->unpause_anims();
	if (input_blocked == VANISH_WAIT)
		game->unpause_user();
}

/* Assume FadeEffect is complete. */
uint16 VanishEffect::callback(uint16 msg, CallBack *caller, void *data) {
	delete_self();
	return 0;
}


/* TileFadeEffect */
TileFadeEffect::TileFadeEffect(const MapCoord &loc, Tile *from, Tile *to, FadeType type, uint16 speed)
		: actor(nullptr), inc_reverse(false), spd(0) {
	add_anim(new TileFadeAnim(loc, from, to, speed));
	num_anim_running = 1;
}

//Fade out actor.
TileFadeEffect::TileFadeEffect(Actor *a, uint16 speed)
		: actor(a), inc_reverse(false), spd(speed), num_anim_running(0) {
	add_actor_anim();
	actor->hide();
}

TileFadeEffect::~TileFadeEffect() {

}

void TileFadeEffect::add_actor_anim() {
	MapCoord loc = actor->get_location();
	Tile *from = actor->get_tile();
	add_tile_anim(loc, from);

	const Std::list<Obj *> &surrounding_objs = actor->get_surrounding_obj_list();
	for (Obj *obj : surrounding_objs)
		add_obj_anim(obj);
}

void TileFadeEffect::add_obj_anim(Obj *obj) {
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();
	MapCoord loc(obj->x, obj->y, obj->z);
	add_tile_anim(loc, obj_manager->get_obj_tile(obj->obj_n, obj->frame_n));
}

void TileFadeEffect::add_fade_anim(const MapCoord &loc, Tile *tile) {
	add_anim(new TileFadeAnim(loc, tile, nullptr, spd));
	num_anim_running++;
}

void TileFadeEffect::add_tile_anim(const MapCoord &loc_, Tile *tile) {
	MapCoord loc = loc_;
	TileManager *tile_manager = Game::get_game()->get_tile_manager();
	uint16 tile_num = tile->tile_num;

	add_fade_anim(loc, tile);

	if (tile->dbl_width) {
		tile_num--;
		loc.x -= 1;
		add_fade_anim(loc, tile_manager->get_tile(tile_num));
		loc.x += 1;
	}

	if (tile->dbl_height) {
		tile_num--;
		loc.y -= 1;
		add_fade_anim(loc, tile_manager->get_tile(tile_num));
		loc.y += 1;
	}

	if (tile->dbl_width && tile->dbl_height) {
		tile_num--;
		loc.x -= 1;
		loc.y -= 1;
		add_fade_anim(loc, tile_manager->get_tile(tile_num));
		loc.x += 1;
		loc.y += 1;
	}
}

uint16 TileFadeEffect::callback(uint16 msg, CallBack *caller, void *data) {
	if (msg == MESG_ANIM_DONE) {
		num_anim_running--;
	}

	if (num_anim_running == 0) {
		if (inc_reverse) {
			inc_reverse = false;
			add_actor_anim();
			return 0;
		}

		if (actor)
			actor->show();

		delete_self();
	}

	return 0;
}


TileBlackFadeEffect::TileBlackFadeEffect(Actor *a, uint8 fade_color, uint16 speed) {
	init(fade_color, speed);
	actor = a;
	actor->hide();
	add_actor_anim();
}

TileBlackFadeEffect::TileBlackFadeEffect(Obj *o, uint8 fade_color, uint16 speed) {
	init(fade_color, speed);
	obj = o;
	obj->set_invisible(true);
	add_obj_anim(obj);
}

void TileBlackFadeEffect::init(uint8 fade_color, uint16 speed) {
	fade_speed = speed;
	color = fade_color;
	actor = nullptr;
	obj = nullptr;
	reverse = false;

	num_anim_running = 0;
}
TileBlackFadeEffect::~TileBlackFadeEffect() {

}

void TileBlackFadeEffect::add_actor_anim() {
	MapCoord loc = actor->get_location();
	Tile *from = actor->get_tile();
	add_tile_anim(loc, from);

	const Std::list<Obj *> &surrounding_objs = actor->get_surrounding_obj_list();
	for (Obj *o : surrounding_objs)
		add_obj_anim(o);
}

void TileBlackFadeEffect::add_obj_anim(Obj *o) {
	MapCoord loc(o);
	Tile *from = Game::get_game()->get_obj_manager()->get_obj_tile(o->obj_n, o->frame_n);
	add_tile_anim(loc, from);
}

void TileBlackFadeEffect::add_tile_anim(const MapCoord &loc_, Tile *tile) {
	TileManager *tile_manager = Game::get_game()->get_tile_manager();
	uint16 tile_num = tile->tile_num;

	add_anim(new TileFadeAnim(loc_, tile, 0, color, reverse, fade_speed));
	num_anim_running++;

	MapCoord loc = loc_;
	if (tile->dbl_width) {
		tile_num--;
		loc.x -= 1;
		add_anim(new TileFadeAnim(loc, tile_manager->get_tile(tile_num), 0, color, reverse, fade_speed));
		num_anim_running++;
		loc.x += 1;
	}

	if (tile->dbl_height) {
		tile_num--;
		loc.y -= 1;
		add_anim(new TileFadeAnim(loc, tile_manager->get_tile(tile_num), 0, color, reverse, fade_speed));
		num_anim_running++;
		loc.y += 1;
	}

	if (tile->dbl_width && tile->dbl_height) {
		tile_num--;
		loc.x -= 1;
		loc.y -= 1;
		add_anim(new TileFadeAnim(loc, tile_manager->get_tile(tile_num), 0, color, reverse, fade_speed));
		num_anim_running++;
		loc.x += 1;
		loc.y += 1;
	}
}

uint16 TileBlackFadeEffect::callback(uint16 msg, CallBack *caller, void *data) {
	if (msg == MESG_ANIM_DONE) {
		num_anim_running--;
	}

	if (num_anim_running == 0) {
		if (reverse == false) {
			reverse = true;
			if (actor)
				add_actor_anim();
			else
				add_obj_anim(obj);

			return 0;
		}

		if (actor)
			actor->show();
		else
			obj->set_invisible(false);

		delete_self();
	}

	return 0;
}

XorEffect::XorEffect(uint32 eff_ms)
	: map_window(game->get_map_window()),
	  length(eff_ms) {
	game->pause_user();
	game->pause_anims();

	init_effect();
}

void XorEffect::init_effect() {
	capture = map_window->get_sdl_surface();
	map_window->set_overlay_level(MAP_OVERLAY_DEFAULT);
	map_window->set_overlay(capture);

	xor_capture(0xd); // changes black to pink
	start_timer(length);
}

/* Timer finished. Cleanup. */
uint16 XorEffect::callback(uint16 msg, CallBack *caller, void *data) {
	if (msg == MESG_TIMED) {
		stop_timer();
		game->unpause_anims();
		game->unpause_user();
		map_window->set_overlay(nullptr);
		delete_self();
	}
	return 0;
}

/* Do binary-xor on each pixel of the mapwindow image.*/
void XorEffect::xor_capture(uint8 mod) {
	Graphics::Surface s = capture->getSubArea(Common::Rect(0, 0, capture->w, capture->h));

	uint8 *pixels = (uint8 *)s.getPixels();
	for (int p = 0; p < (capture->w * capture->h); p++)
		pixels[p] ^= mod;
}


U6WhitePotionEffect::U6WhitePotionEffect(uint32 eff_ms, uint32 delay_ms, Obj *callback_obj)
	: map_window(game->get_map_window()),
	  state(0), start_length(1000),
	  eff1_length(eff_ms), eff2_length(800),
	  xray_length(delay_ms), capture(0),
	  potion(callback_obj) {
	game->pause_user();
	game->pause_anims();

	init_effect();
}

void U6WhitePotionEffect::init_effect() {
	// FIXME: play sound, and change state to 1 when sound is complete
	capture = map_window->get_sdl_surface();
	map_window->set_overlay_level(MAP_OVERLAY_DEFAULT);
	map_window->set_overlay(capture);
	start_timer(start_length);
}

/* The state is changed from current to next when this is called. */
uint16 U6WhitePotionEffect::callback(uint16 msg, CallBack *caller, void *data) {
	if (msg == MESG_TIMED) {
		stop_timer();

		if (state == 0) { // start/sound effect
			// FIXME: make start_length a timeout, force sound to stop
			xor_capture(0xd); // changes black to pink
			start_timer(eff1_length);
			state = 1;
		} else if (state == 1) { // xor-effect
			map_window->set_overlay(nullptr);
			start_timer(eff2_length);
			state = 2;
		} else if (state == 2) { // character outline
			game->unpause_anims();
			map_window->set_x_ray_view(X_RAY_ON);
			map_window->updateBlacking();
			start_timer(xray_length);
			state = 3;
		} else if (state == 3) { // x-ray
			map_window->set_x_ray_view(X_RAY_OFF);
			map_window->updateBlacking();
			game->unpause_user();
			if (potion)
				game->get_usecode()->message_obj(potion, MESG_EFFECT_COMPLETE, this);
			state = 4; // finished
			delete_self();
		}
	}
	return 0;
}

/* Do binary-xor on each pixel of the mapwindow image.*/
void U6WhitePotionEffect::xor_capture(uint8 mod) {
	Graphics::Surface s = capture->getSubArea(Common::Rect(0, 0, capture->w, capture->h));
	uint8 *pixels = (uint8 *)s.getPixels();
	for (int p = 0; p < (capture->w * capture->h); p++)
		pixels[p] ^= mod;
}


XRayEffect::XRayEffect(uint32 eff_ms) {
	xray_length = eff_ms;
	init_effect();
}

void XRayEffect::init_effect() {
	Game::get_game()->get_map_window()->set_x_ray_view(X_RAY_ON);
	start_timer(xray_length);
}

uint16 XRayEffect::callback(uint16 msg, CallBack *caller, void *data) {
	if (msg == MESG_TIMED) {
		stop_timer();
		Game::get_game()->get_map_window()->set_x_ray_view(X_RAY_OFF);
		delete_self();
	}

	return 0;
}

PauseEffect::PauseEffect() {
	game->pause_world();
	// FIXME: need a way to detect any keyboard/mouse input
	game->get_scroll()->set_input_mode(true, "\n", true);
	game->get_scroll()->request_input(this, 0);
}

/* The effect ends when this is called. (if input is correct) */
uint16 PauseEffect::callback(uint16 msg, CallBack *caller, void *data) {
	if (msg == MESG_INPUT_READY) {
		game->unpause_world();
		delete_self();
	}
	return 0;
}

WizardEyeEffect::WizardEyeEffect(const MapCoord &location, uint16 duration) {
	// Disable keymapper so Wizard Eye can receive keyboard input.
	// FIXME: Remove this once the effect can use keymapper-bound keys.
	g_system->getEventManager()->getKeymapper()->setEnabled(false);
	game->get_map_window()->wizard_eye_start(location, duration, this);
}

uint16 WizardEyeEffect::callback(uint16 msg, CallBack *caller, void *data) {
	if (msg == MESG_EFFECT_COMPLETE) {
		delete_self();
		// FIXME: Remove this once the effect can use keymapper-bound keys.
		g_system->getEventManager()->getKeymapper()->setEnabled(true);
	}

	return 0;
}

TextInputEffect::TextInputEffect(const char *allowed_chars, bool can_escape) {
	game->pause_world();
	// FIXME: need a way to detect any keyboard/mouse input
	game->get_gui()->unblock();
	game->get_scroll()->set_input_mode(true, allowed_chars, can_escape);
	game->get_scroll()->request_input(this, 0);
}

/* The effect ends when this is called. (if input is correct) */
uint16 TextInputEffect::callback(uint16 msg, CallBack *caller, void *data) {
	if (msg == MESG_INPUT_READY) {
		input = *(Std::string *)data;
		game->unpause_world();
		delete_self();
	}
	return 0;
}


PeerEffect::PeerEffect(uint16 x, uint16 y, uint8 z, Obj *callback_obj)
	: map_window(game->get_map_window()), overlay(0),
	  gem(callback_obj), area(x, y, z), tile_trans(0),
	  map_pitch(0) {
	uint8 lvl = 0;
	map_window->get_level(&lvl);
	map_pitch = (lvl == 0) ? 1024 : 256;

	init_effect();
}

void PeerEffect::init_effect() {
	overlay = map_window->get_sdl_surface();
	map_window->set_overlay_level(MAP_OVERLAY_DEFAULT);
	map_window->set_overlay(overlay);
	assert(overlay->w % PEER_TILEW == 0); // overlay must be a multiple of tile size
	SDL_FillRect(overlay, nullptr, 0);

	peer();
}

void PeerEffect::delete_self() {
	map_window->set_overlay(nullptr);
	if (gem)
		game->get_usecode()->message_obj(gem, MESG_EFFECT_COMPLETE, this);
	else // FIXME: I don't want prompt display here, so it's also in UseCode,
		// but it has to be here if no object was set. (until we have another
		// way to tell caller effect is complete, and return to player)
	{
		game->get_scroll()->display_string("\n");
		game->get_scroll()->display_prompt();
	}
	Effect::delete_self();
}

void PeerEffect::peer() {
	uint16 w = overlay->w, h = overlay->h;
	// effect is limited to 48x48 area
	if (overlay->w > 48 * PEER_TILEW) w = 48 * PEER_TILEW;
	if (overlay->h > 48 * PEER_TILEW) h = 48 * PEER_TILEW;

	MapCoord player_loc = game->get_player()->get_actor()->get_location();
	uint16 cx = player_loc.x - area.x; // rough center of area
	uint16 cy = player_loc.y - area.y;
	area.x %= map_pitch; // we have to wrap here because we use a map buffer
	area.y %= map_pitch;
	uint8 *mapbuffer = new uint8[48 * 48]; // array of tile types/colors
	memset(mapbuffer, 0x00, sizeof(uint8) * 48 * 48); // fill with black
	fill_buffer(mapbuffer, cx, cy);

	for (int x = 0; x < w; x += PEER_TILEW)
		for (int y = 0; y < h; y += PEER_TILEW) {
			uint16 wx = area.x + x / PEER_TILEW, wy = area.y + y / PEER_TILEW;
			uint8 tile_type = mapbuffer[(wy - area.y) * 48 + (wx - area.x)];
			blit_tile(x, y, tile_type);
			if (tile_type != 0x00) {
				Actor *actor = game->get_actor_manager()->get_actor(wx, wy, area.z);
				if (actor)
					blit_actor(actor);
			}
		}

	delete [] mapbuffer;
}

void PeerEffect::fill_buffer(uint8 *mapbuffer, uint16 x, uint16 y) {
	uint16 wx = area.x + x, wy = area.y + y;
	uint8 *tile = &mapbuffer[y * 48 + x];

	if (*tile != 0x00)
		return; // already filled

	wx %= map_pitch; // we have to wrap here because we use a map buffer
	wy %= map_pitch;
	*tile = get_tilemap_type(wx, wy, area.z);

	// stop at unpassable tiles
	// FIXME: stop at Nothing/black tiles
	if (*tile != peer_tilemap[2]
	        || game->get_game_map()->get_tile(wx, wy, area.z, true)->passable) {
		if (y > 0) {
			if (x > 0) fill_buffer(mapbuffer, x - 1, y - 1); // +-+-+
			if (y > 0) fill_buffer(mapbuffer, x,  y - 1); // |\|/|
			if (x + 1 < 48) fill_buffer(mapbuffer, x + 1, y - 1);
		}

		if (x > 0)     fill_buffer(mapbuffer, x - 1, y); // +-+-+
		if (x + 1 < 48)  fill_buffer(mapbuffer, x + 1, y);

		if (y + 1 < 48) {
			if (x > 0) fill_buffer(mapbuffer, x - 1, y + 1); // |/|\|
			fill_buffer(mapbuffer, x,  y + 1); // +-+-+
			if (x + 1 < 48) fill_buffer(mapbuffer, x + 1, y + 1);
		}
	}
}

inline void PeerEffect::blit_tile(uint16 x, uint16 y, uint8 c) {
	Graphics::Surface s = overlay->getSubArea(Common::Rect(0, 0, overlay->w, overlay->h));
	uint8 *pixels = (uint8 *)s.getPixels();
	for (int j = 0; j < PEER_TILEW && j < overlay->h; j++)
		for (int i = 0; i < PEER_TILEW && i < overlay->w; i++) {
			if (peer_tile[i * PEER_TILEW + j] != tile_trans)
				pixels[overlay->w * (y + j) + (x + i)] = c;
		}
}

inline void PeerEffect::blit_actor(Actor *actor) {
	tile_trans = 1;
	blit_tile((actor->get_location().x - area.x)*PEER_TILEW,
	          (actor->get_location().y - area.y)*PEER_TILEW, 0x0F);
	tile_trans = 0;
	if (game->get_player()->get_actor() == actor)
		blit_tile((actor->get_location().x - area.x)*PEER_TILEW,
		          (actor->get_location().y - area.y)*PEER_TILEW, 0x0F);
}

inline uint8 PeerEffect::get_tilemap_type(uint16 wx, uint16 wy, uint8 wz) {
	Map *map = game->get_game_map();
	// ignore objects (bridges and docks), and show coasts as land
	if (map->is_water(wx, wy, wz, true) && !map->get_tile(wx, wy, wz, true)->passable)
		return peer_tilemap[1];
	if (!map->is_passable(wx, wy, wz))
		return peer_tilemap[2];
	if (map->is_damaging(wx, wy, wz))
		return peer_tilemap[3];
	return peer_tilemap[0]; // ground/passable
}

WingStrikeEffect::WingStrikeEffect(Actor *target_actor) {
	actor = target_actor;

	add_anim(new WingAnim(actor->get_location()));
}

uint16 WingStrikeEffect::callback(uint16 msg, CallBack *caller, void *data) {
	switch (msg) {
	case MESG_ANIM_HIT :
		DEBUG(0, LEVEL_DEBUGGING, "hit target!\n");
		Game::get_game()->get_script()->call_actor_hit(actor, (NUVIE_RAND() % 20) + 1);
		break;
	case MESG_ANIM_DONE :
		delete_self();
		break;
	}
	return 0;
}

HailStormEffect::HailStormEffect(const MapCoord &target) {
	add_anim(new HailstormAnim(target));
}

uint16 HailStormEffect::callback(uint16 msg, CallBack *caller, void *data) {
	switch (msg) {
	case MESG_ANIM_HIT :
		DEBUG(0, LEVEL_DEBUGGING, "hit target!\n");
		Game::get_game()->get_script()->call_actor_hit((Actor *)data, 1);
		break;
	case MESG_ANIM_DONE :
		delete_self();
		break;
	}
	return 0;
}

/*** AsyncEffect ***/
AsyncEffect::AsyncEffect(Effect *e) {
	effect_complete = false;
	effect = e;
	effect->retain();
	effect_manager->watch_effect(this, effect);
}

AsyncEffect::~AsyncEffect() {
	effect->release();
}

/* The effect is marked as defunct after run finishes and will be removed from the system.*/
void AsyncEffect::run(bool process_gui_input) {
	if (!process_gui_input)
		Game::get_game()->pause_user();
	for (; effect_complete == false;) {
		//spin world
		Game::get_game()->update_once(process_gui_input);
		if (!effect_complete)
			Game::get_game()->update_once_display();
	}
	if (!process_gui_input)
		Game::get_game()->unpause_user();
	delete_self();
}

uint16 AsyncEffect::callback(uint16 msg, CallBack *caller, void *data) {

	// effect complete
	if (msg == MESG_EFFECT_COMPLETE) {
		effect_complete = true;
	}

	return 0;
}

} // End of namespace Nuvie
} // End of namespace Ultima
