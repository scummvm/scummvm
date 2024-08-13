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

#ifndef NUVIE_ACTORS_ACTOR_MANAGER_H
#define NUVIE_ACTORS_ACTOR_MANAGER_H

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/misc/actor_list.h"
#include "ultima/nuvie/actors/actor.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class Map;
class TileManager;
class GameClock;
class MapCoord;


#define ACTORMANAGER_MAX_ACTORS 256

class ActorManager {
	const Configuration *config;
	TileManager *tile_manager;
	ObjManager *obj_manager;

	bool update; // ActorManager is not paused
	bool wait_for_player; // Player's turn; wait until updateActors() is called
	bool combat_movement; // Defines actor movement type (individual/party)
	bool should_clean_temp_actors; // If set, temp actors are cleaned when > 19 tiles from player.

	Map *map;
	Actor *actors[ACTORMANAGER_MAX_ACTORS];
	uint8 player_actor;
	uint8 temp_actor_offset;
	GameClock *_clock;

	uint16 last_obj_blk_x, last_obj_blk_y;
	uint8 last_obj_blk_z;
	uint16 cur_x, cur_y;
	uint8 cur_z;
	MapCoord *cmp_actor_loc; // data for sort_distance() & cmp_distance_to_loc()

public:

	ActorManager(const Configuration *cfg, Map *m, TileManager *tm, ObjManager *om, GameClock *c);
	~ActorManager();

	void init();
	void clean();

	bool load(NuvieIO *objlist);
	bool save(NuvieIO *objlist);
// ActorList
	ActorList *get_actor_list(); // *returns a NEW list*
	ActorList *sort_nearest(ActorList *list, uint16 x, uint16 y, uint8 z); // ascending distance
	ActorList *filter_distance(ActorList *list, uint16 x, uint16 y, uint8 z, uint16 dist);
	ActorList *filter_alignment(ActorList *list, ActorAlignment align);
	ActorList *filter_party(ActorList *list);

	Actor *get_actor(uint8 actor_num) const;
	Actor *get_actor(uint16 x, uint16 y, uint8 z,  bool inc_surrounding_objs = true, Actor *excluded_actor = nullptr);
	Actor *get_actor_holding_obj(Obj *obj);

	private:
	Actor *findActorAtImpl(uint16 x, uint16 y, uint8 z, bool(*predicateWrapper)(void *, const Actor *), bool incDoubleTile, bool incSurroundingObjs, void *predicate) const;

	public:
	/**
	 * @brief Find first actor at location for which predicate function returns true
	 * @param x world coordinate
	 * @param y world coordinate
	 * @param z level
	 * @param predicate predicate function/lambda of the form: bool f(const Actor*)
	 * @param incDoubleTile include all tiles of double width/height actors
	 * @param incSurroundingObjs include surrounding actor objects
	 * @return pointer to actor or nullptr
	 */
	template<typename F>
	Actor *findActorAt(uint16 x, uint16 y, uint8 z, F predicate, bool incDoubleTile = true, bool incSurroundingObjs = true) {
		// This is a template so it can take lambdas.
		// To keep the implementation out of the header, the type of the passed lambda/function
		// is hidden inside a wrapping lambda, which is then passed as a function pointer.
		// TODO: Use a class for this.
		auto predicateWrapper = +[](void *wrappedPredicate, const Actor *a) -> bool {
			return (*(F*)wrappedPredicate)(a);
		};
		return findActorAtImpl(x, y, z, predicateWrapper, incDoubleTile, incSurroundingObjs, &predicate);
	}

	Actor *get_avatar();

	Actor *get_player();
	void set_player(Actor *a);

	const char *look_actor(const Actor *a, bool show_prefix = true);

	void set_update(bool u) {
		update = u;
	}
	bool get_update() const {
		return update;
	}
	void set_combat_movement(bool c);

	void updateActors(uint16 x, uint16 y, uint8 z);
	void twitchActors();
	void moveActors();
	void startActors();
	void updateSchedules(bool teleport = false);

	void clear_actor(Actor *actor);
	bool resurrect_actor(Obj *actor_obj, MapCoord new_position);

	bool is_temp_actor(Actor *actor);
	bool is_temp_actor(uint8 id_n);
	bool create_temp_actor(uint16 obj_n, uint8 obj_status, uint16 x, uint16 y, uint8 z, ActorAlignment alignment, uint8 worktype, Actor **new_actor = nullptr);
	bool clone_actor(Actor *actor, Actor **new_actor, MapCoord new_location);
	bool toss_actor(Actor *actor, uint16 xrange, uint16 yrange);
	bool toss_actor_get_location(uint16 start_x, uint16 start_y, uint8 start_z, uint16 xrange, uint16 yrange, MapCoord *location);
	void print_actor(Actor *actor);
	bool can_put_actor(const MapCoord &location);
	void enable_temp_actor_cleaning(bool value) {
		should_clean_temp_actors = value;
	}

protected:

	Actor *get_multi_tile_actor(uint16 x, uint16 y, uint8 z);

	bool loadActorSchedules();
	inline Actor *find_free_temp_actor();
	inline ActorList *filter_active_actors(ActorList *list, uint16 x, uint16 y, uint8 z);

	void update_temp_actors(uint16 x, uint16 y, uint8 z);
	void clean_temp_actors_from_level(uint8 level);
	void clean_temp_actors_from_area(uint16 x, uint16 y);

	inline void clean_temp_actor(Actor *actor);

private:

	bool loadCustomTiles(nuvie_game_t game_type);
	void loadNPCTiles(const Common::Path &datadir);
	void loadAvatarTiles(const Common::Path &datadir);
	void loadCustomBaseTiles();
	Std::set<Std::string> getCustomTileFilenames(const Common::Path &datadir, const Std::string &filenamePrefix);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
