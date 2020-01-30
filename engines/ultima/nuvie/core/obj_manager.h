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

#ifndef NUVIE_CORE_OBJ_MANAGER_H
#define NUVIE_CORE_OBJ_MANAGER_H

#include "ultima/shared/std/containers.h"
#include "ultima/nuvie/misc/iavl_tree.h"
#include "ultima/nuvie/core/tile_manager.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/core/obj.h"

namespace Ultima {
namespace Nuvie {

//class U6LList;
class Configuration;
class EggManager;
class UseCode;
class NuvieIO;
class MapCoord;
class Actor;

//is_passable return codes
#define OBJ_NO_OBJ       0
#define OBJ_NOT_PASSABLE 1
#define OBJ_PASSABLE     2

#define OBJ_WEIGHT_INCLUDE_CONTAINER_ITEMS true
#define OBJ_WEIGHT_EXCLUDE_CONTAINER_ITEMS false

#define OBJ_WEIGHT_DO_SCALE true
#define OBJ_WEIGHT_DONT_SCALE false

#define OBJ_WEIGHT_EXCLUDE_QTY false

#define OBJ_ADD_TOP true

#define OBJ_SHOW_PREFIX true

#define OBJ_TEMP_INIT 255 // this is used to stop temporary objects from being cleaned upon startup.

#define OBJ_SEARCH_TOP true
#define OBJ_INCLUDE_IGNORED true
#define OBJ_EXCLUDE_IGNORED false

struct ObjTreeNode {
	iAVLKey key;
	U6LList *obj_list;
};

Obj *new_obj(uint16 obj_n, uint8 frame_n, uint16 x, uint16 y, uint16 z);
void delete_obj(Obj *obj);

void clean_obj_tree_node(void *node);

class ObjManager {
	Configuration *config;
	int game_type;
	EggManager *egg_manager;
	TileManager *tile_manager;
//chunk object trees.
	iAVLTree *surface[64];
	iAVLTree *dungeon[5];

	uint16 obj_to_tile[1024]; //maps object number (index) to tile number.
	uint8 obj_weight[1024];
	uint8 obj_stackable[1024];
	U6LList *actor_inventories[256];

	bool show_eggs;
	uint16 egg_tile_num;

	UseCode *usecode;

	Std::list<Obj *> temp_obj_list;
	Std::list<Obj *> tile_obj_list; // SE single instance 'map tile' objects
	uint16 last_obj_blk_x, last_obj_blk_y;
	uint8 last_obj_blk_z;

	uint16 obj_save_count;

	bool custom_actor_tiles;

public:

	ObjManager(Configuration *cfg, TileManager *tm, EggManager *em);
	~ObjManager();

	bool use_custom_actor_tiles() {
		return custom_actor_tiles;
	}
	bool is_showing_eggs() {
		return show_eggs;
	}
	void set_show_eggs(bool value) {
		show_eggs = value;
	}

	bool loadObjs();
	bool load_super_chunk(NuvieIO *chunk_buf, uint8 level, uint8 chunk_offset);
	void startObjs();
	void clean();
	void clean_actor_inventories();

	bool save_super_chunk(NuvieIO *save_buf, uint8 level, uint8 chunk_offset);
	bool save_eggs(NuvieIO *save_buf);
	bool save_inventories(NuvieIO *save_buf);
	bool save_obj(NuvieIO *save_buf, Obj *obj, uint16 parent_objblk_n);

	void set_usecode(UseCode *uc) {
		usecode = uc;
	}
	UseCode *get_usecode()        {
		return (usecode);
	}
	EggManager *get_egg_manager() {
		return (egg_manager);
	}

//U6LList *get_obj_superchunk(uint16 x, uint16 y, uint8 level);
	bool is_boundary(uint16 x, uint16 y, uint8 level, uint8 boundary_type = TILEFLAG_BOUNDARY, Obj *excluded_obj = NULL);
//bool is_door(Obj * obj);
	bool is_damaging(uint16 x, uint16 y, uint8 level);
	uint8 is_passable(uint16 x, uint16 y, uint8 level);
	bool is_forced_passable(uint16 x, uint16 y, uint8 level);
	bool is_stackable(Obj *obj);
	bool is_breakable(Obj *obj);
	bool can_store_obj(Obj *target, Obj *src); // Bag, open chest, spellbook.
	bool can_get_obj(Obj *obj);
	bool has_reduced_weight(uint16 obj_n);
	bool has_reduced_weight(Obj *obj) {
		return has_reduced_weight(obj->obj_n);
	}
	bool has_toptile(Obj *obj);
	bool obj_is_damaging(Obj *obj, Actor *actor = NULL); // if actor, it will damage and display text
	bool is_door(uint16 x, uint16 y, uint8 level);

	U6LList *get_obj_list(uint16 x, uint16 y, uint8 level);

	Tile *get_obj_tile(uint16 obj_n, uint8 frame_n);
	Tile *get_obj_tile(uint16 x, uint16 y, uint8 level, bool top_obj = true);
	Tile *get_obj_dmg_tile(uint16 x, uint16 y, uint8 level);
	Obj *get_obj(uint16 x, uint16 y, uint8 level, bool top_obj = OBJ_SEARCH_TOP, bool include_ignored_objects = OBJ_EXCLUDE_IGNORED, Obj *excluded_obj = NULL);
	Obj *get_obj_of_type_from_location_inc_multi_tile(uint16 obj_n, uint16 x, uint16 y, uint8 z);
	Obj *get_obj_of_type_from_location_inc_multi_tile(uint16 obj_n, sint16 quality, sint32 qty, uint16 x, uint16 y, uint8 z);
	Obj *get_obj_of_type_from_location(uint16 obj_n, uint16 x, uint16 y, uint8 z);
	Obj *get_obj_of_type_from_location(uint16 obj_n, sint16 quality, sint32 qty, uint16 x, uint16 y, uint8 z);
	Obj *get_objBasedAt(uint16 x, uint16 y, uint8 level, bool top_obj, bool include_ignored_objects = true, Obj *excluded_obj = NULL);
	Obj *get_tile_obj(uint16 obj_n);

	uint16 get_obj_tile_num(uint16 obj_num);
	inline bool is_corpse(Obj *obj);
	uint16 get_obj_tile_num(Obj *obj);
	void set_obj_tile_num(uint16 obj_num, uint16 tile_num);

	U6LList *get_actor_inventory(uint16 actor_num);
	bool actor_has_inventory(uint16 actor_num);

	Obj *find_next_obj(uint8 level, Obj *prev_obj, bool match_frame_n = OBJ_NOMATCH_FRAME_N, bool match_quality = OBJ_MATCH_QUALITY);
	Obj *find_obj(uint8 level, uint16 obj_n, uint8 quality, bool match_quality = OBJ_MATCH_QUALITY, uint16 frame_n = 0, bool match_frame_n = OBJ_NOMATCH_FRAME_N,  Obj **prev_obj = NULL);

	bool move(Obj *obj, uint16 x, uint16 y, uint8 level);
	bool add_obj(Obj *obj, bool addOnTop = false);
	bool remove_obj_from_map(Obj *obj);
	bool remove_obj_type_from_location(uint16 obj_n, uint16 x, uint16 y, uint8 z);


	Obj *copy_obj(Obj *obj);
	const char *look_obj(Obj *obj, bool show_prefix = false);
	Obj *get_obj_from_stack(Obj *obj, uint32 count);

	bool list_add_obj(U6LList *list, Obj *obj, bool stack_objects = true, uint32 pos = 0);

	const char *get_obj_name(Obj *obj);
	const char *get_obj_name(uint16 obj_n);
	const char *get_obj_name(uint16 obj_n, uint8 frame_n);

	float get_obj_weight(Obj *obj, bool include_container_items = OBJ_WEIGHT_INCLUDE_CONTAINER_ITEMS, bool scale = true, bool include_qty = true);
	uint8 get_obj_weight_unscaled(uint16 obj_n) {
		return (obj_weight[obj_n]);
	}
	float get_obj_weight(uint16 obj_n);

	void animate_forwards(Obj *obj, uint32 loop_count = 1);
	void animate_backwards(Obj *obj, uint32 loop_count = 1);

	void update(uint16 x, uint16 y, uint8 z, bool teleport = false);

	bool unlink_from_engine(Obj *obj, bool run_usecode = true);

	bool moveto_map(Obj *obj, MapCoord location);
	bool moveto_inventory(Obj *obj, uint16 actor_num);
	bool moveto_inventory(Obj *obj, Actor *actor);
	bool moveto_container(Obj *obj, Obj *container_obj, bool stack = true);

protected:

	void remove_obj(Obj *obj);

	bool load_basetile();
	bool load_weight_table();


	bool addObjToContainer(U6LList *list, Obj *obj);
	Obj *loadObj(NuvieIO *buf);
	iAVLTree *get_obj_tree(uint16 x, uint16 y, uint8 level);

	iAVLKey get_obj_tree_key(Obj *obj);
	iAVLKey get_obj_tree_key(uint16 x, uint16 y, uint8 level);
//inline U6LList *ObjManager::get_schunk_list(uint16 x, uint16 y, uint8 level);

	bool temp_obj_list_add(Obj *obj);
	bool temp_obj_list_remove(Obj *obj);
	void temp_obj_list_clean_level(uint8 z);
	void temp_obj_list_clean_area(uint16 x, uint16 y);

	void remove_temp_obj(Obj *tmp_obj);

	inline Obj *find_obj_in_tree(uint16 obj_n, uint8 quality, bool match_quality, uint8 frame_n, bool match_frame_n, Obj **prev_obj, iAVLTree *obj_tree);
	inline void start_obj_usecode(iAVLTree *obj_tree);
	inline void print_egg_tree(iAVLTree *obj_tree);

public:
	void print_object_list();
	void print_egg_list();
	void print_obj(Obj *obj, bool in_container, uint8 indent = 0);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
