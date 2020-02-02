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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/core/egg_manager.h"
#include "ultima/nuvie/core/tile_manager.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"

namespace Ultima {
namespace Nuvie {

static const int obj_egg_table[5] = {0,   // NUVIE_GAME_NONE
                                     335, // NUVIE_GAME_U6
                                     466,   // NUVIE_GAME_MD
                                     0,
                                     230
                                    };  // NUVIE_GAME_SE

static iAVLKey get_iAVLKey(const void *item) {
	return ((const ObjTreeNode *)item)->key;
}

ObjManager::ObjManager(Configuration *cfg, TileManager *tm, EggManager *em) {
	uint8 i;
	Std::string show_eggs_key;

	config = cfg;
	tile_manager = tm;
	egg_manager = em;
	usecode = NULL;
	obj_save_count = 0;

	load_basetile();
	load_weight_table();

	memset(actor_inventories, 0, sizeof(actor_inventories));

	for (i = 0; i < 64; i++) {
		surface[i] = iAVLAllocTree(get_iAVLKey);
	}

	for (i = 0; i < 5; i++) {
		dungeon[i] = iAVLAllocTree(get_iAVLKey);
	}

	last_obj_blk_x = 0;
	last_obj_blk_y = 0;
	last_obj_blk_z = OBJ_TEMP_INIT;

	config->value("config/GameType", game_type);

//save the egg tile_num incase we want to switch egg display on again.
	egg_tile_num = get_obj_tile_num(obj_egg_table[game_type]);

	show_eggs_key = config_get_game_key(config);
	show_eggs_key.append("/show_eggs");

	config->value(show_eggs_key, show_eggs);
//if(!show_eggs)
//  show_egg_objs(false);

	Std::string custom_tile_str;
	config->value(config_get_game_key(config) + "/custom_actor_tiles", custom_tile_str, "default");
	if (custom_tile_str == "default") {
		if (Game::get_game()->is_new_style())
			custom_actor_tiles = true;
		else
			custom_actor_tiles = false;
	} else if (custom_tile_str == "yes")
		custom_actor_tiles = true;
	else
		custom_actor_tiles = false;
}

ObjManager::~ObjManager() {
	clean();

	unsigned int i;
	for (i = 0; i < 64; i++)
		iAVLFreeTree(surface[i], clean_obj_tree_node);

	for (i = 0; i < 5; i++)
		iAVLFreeTree(dungeon[i], clean_obj_tree_node);

	for (i = 0; i < 256; i++) {
		if (actor_inventories[i]) {
			delete actor_inventories[i];
		}
	}
}

bool ObjManager::load_basetile() {
	Std::string filename;
	NuvieIOFileRead basetile;
	uint16 i;

	config_get_path(config, "basetile", filename);

	if (basetile.open(filename) == false)
		return false;

	for (i = 0; i < 1024; i++) {
		obj_to_tile[i] = basetile.read2();
		obj_stackable[i] = (uint8)tile_manager->tile_is_stackable(obj_to_tile[i]);
	} // FIXME: tile_manager's tile_is_stackable is incorrect for (at least) Zu Ylem, silver snake venom.

	return true;
}

bool ObjManager::load_weight_table() {
	Std::string filename;
	NuvieIOFileRead tileflag;

	config_get_path(config, "tileflag", filename);

	if (tileflag.open(filename) == false)
		return false;

	tileflag.seek(0x1000);

	tileflag.readToBuf(obj_weight, 1024);

	return true;
}


bool ObjManager::load_super_chunk(NuvieIO *chunk_buf, uint8 level, uint8 chunk_offset) {
	NuvieIOFileRead file;
	U6LList *list;
	uint16 num_objs;
	Obj *obj;
	uint16 i;
	U6LList *inventory_list;
	iAVLTree *obj_tree;

	if (level == 0)
		obj_tree = surface[chunk_offset];
	else
		obj_tree = dungeon[level - 1];

	list = new U6LList();

	num_objs = chunk_buf->read2();
//DEBUG(0,LEVEL_DEBUGGING,"chunk %02d number of objects: %d\n", chunk_offset, num_objs);

	for (i = 0; i < num_objs; i++) {
		obj = loadObj(chunk_buf);

		list->add(obj);

		if (obj->obj_n == obj_egg_table[game_type]) {
			egg_manager->add_egg(obj);
			// set egg visibility
			obj->set_invisible(Game::get_game()->are_cheats_enabled() ? !show_eggs : true);
		}

		if (usecode->is_container(obj)) { //object type is container, but may be empty
			obj->make_container();
		}

		if (obj->get_engine_loc() == OBJ_LOC_INV || obj->get_engine_loc() == OBJ_LOC_READIED) { //triggered when object in actor's inventory OR equipped
			//FIXME need to add to inventory properly!! eg set engine loc.
			inventory_list = get_actor_inventory(obj->x);
			inventory_list->add(obj);
		} else {
			if (obj->is_in_container()) { //object in container
				addObjToContainer(list, obj);
			} else {
				add_obj(obj); // show remaining objects
				/*         if(show_eggs || obj->obj_n != obj_egg_table[game_type]) // show remaining objects, hiding eggs if neccecary.
				            {
				             add_obj(obj);
				            // print_obj(obj,false);
				            }*/
			}

		}
		//print_obj(obj,false);
	}

	// Unused variable
	(void)obj_tree;

	delete list;

	return true;
}

bool ObjManager::save_super_chunk(NuvieIO *save_buf, uint8 level, uint8 chunk_offset) {
	iAVLTree *obj_tree;
	ObjTreeNode *item;
	U6Link *link;
	iAVLCursor node;
	uint32 start_pos;
	uint32 finish_pos;
	uint16 egg_type = obj_egg_table[game_type];

	if (level == 0)
		obj_tree = surface[chunk_offset];
	else
		obj_tree = dungeon[level - 1];

	item = (ObjTreeNode *)iAVLFirst(&node, obj_tree);

	start_pos = save_buf->position();

//skip the 2 bytes for number of objects.
	save_buf->write2(0); // we'll fill this in later on.

	obj_save_count = 0;

	for (; item;) {
		for (link = item->obj_list->end(); link != NULL; link = link->prev) {
			if (((Obj *)link->data)->obj_n != egg_type) // we don't save eggs here. They are saved in save_eggs()
				save_obj(save_buf, (Obj *)link->data, obj_save_count);
		}

		item = (ObjTreeNode *)iAVLNext(&node);
	}

	finish_pos = save_buf->position();
	save_buf->seek(start_pos);

	save_buf->write2(obj_save_count);
	save_buf->seek(finish_pos);

	return true;
}

bool ObjManager::save_eggs(NuvieIO *save_buf) {
	uint32 start_pos;
	uint32 finish_pos;
	Std::list<Egg *> *egg_list;
	Std::list<Egg *>::iterator egg;

	start_pos = save_buf->position();

//skip number of objects we will fill that in at the end.
	save_buf->write2(0);

	egg_list = egg_manager->get_egg_list();

	obj_save_count = 0;

	for (egg = egg_list->begin(); egg != egg_list->end(); egg++)
		save_obj(save_buf, (*egg)->obj, obj_save_count);

	finish_pos = save_buf->position();
	save_buf->seek(start_pos);

	save_buf->write2(obj_save_count);
	save_buf->seek(finish_pos);

	DEBUG(0, LEVEL_DEBUGGING, "Eggs: %d\n", obj_save_count);

	return true;
}

bool ObjManager::save_inventories(NuvieIO *save_buf) {
	uint32 start_pos;
	uint32 finish_pos;
	U6Link *link;
	uint16 i;

	start_pos = save_buf->position();

	save_buf->write2(0);

	obj_save_count = 0;

	for (i = 0; i < 256; i++) {
		if (actor_inventories[i] != NULL) {
			for (link = actor_inventories[i]->start(); link != NULL; link = link->next) {
				save_obj(save_buf, (Obj *)link->data, obj_save_count);
			}
		}
	}

	DEBUG(0, LEVEL_DEBUGGING, "Actor Inventories: %d\n", obj_save_count);

	finish_pos = save_buf->position();
	save_buf->seek(start_pos);

	save_buf->write2(obj_save_count);
	save_buf->seek(finish_pos);

	return true;
}

bool ObjManager::save_obj(NuvieIO *save_buf, Obj *obj, uint16 parent_objblk_n) {
	uint8 b;
	U6Link *link;
	uint16 objblk_n;

	if (obj->is_in_container()) { //obj is in a container
		//obj->in_container(); // in container
		obj->x = parent_objblk_n & 0x3ff; //save 10bits in x
		obj->y &= 0xffc0; //clear lower 6 bits
		obj->y |= (parent_objblk_n >> 10); //save top 6bits
	} else {
		if (!obj->is_readied()) {
			obj->status &= (0xff ^ OBJ_STATUS_IN_CONTAINER);
		}
	}

	if (obj->is_in_inventory(OBJ_DONT_CHECK_PARENT))
		obj->x = obj->get_actor_holding_obj()->get_actor_num();

//set original status location bits.
	obj->status &= OBJ_STATUS_MASK_SET;
	switch (obj->get_engine_loc()) {
	case OBJ_LOC_MAP :
		obj->status |= OBJ_STATUS_ON_MAP;
		break;
	case OBJ_LOC_CONT :
		obj->status |= OBJ_STATUS_IN_CONTAINER;
		break;
	case OBJ_LOC_INV :
		obj->status |= OBJ_STATUS_IN_INVENTORY;
		break;
	case OBJ_LOC_READIED :
		obj->status |= OBJ_STATUS_READIED;
		break;
	}

	save_buf->write1(obj->status);
	save_buf->write1(obj->x & 0xff);
	b = obj->x >> 8;
	b += obj->y << 2;
	save_buf->write1(b);

	b = obj->y >> 6;
	b += obj->z << 4;

	save_buf->write1(b);

	save_buf->write1(obj->obj_n & 0xff);

	b = obj->obj_n >> 8;
	b += obj->frame_n << 2;

	save_buf->write1(b);

	save_buf->write1((uint8)(obj->qty & 0xff)); //only save the lower byte to disk.

	if (is_stackable(obj))
		save_buf->write1(obj->qty >> 8);
	else
		save_buf->write1(obj->quality);

	objblk_n = obj_save_count;

	obj_save_count += 1;

	if (obj->container) {
		for (link = obj->container->end(); link != NULL; link = link->prev)
			save_obj(save_buf, (Obj *)link->data, objblk_n);
	}

	return true;
}

void ObjManager::clean() {
	uint8 i;

	egg_manager->clean(Game::get_game()->are_cheats_enabled() ? show_eggs : false); //show_eggs determines wether we delete the actual Objs from egg manager.

	for (i = 0; i < 64; i++)
		iAVLCleanTree(surface[i], clean_obj_tree_node);

	for (i = 0; i < 5; i++)
		iAVLCleanTree(dungeon[i], clean_obj_tree_node);

	clean_actor_inventories();

// remove the temporary object list. The objects were deleted from the surface and dungeon trees.
	temp_obj_list.clear();

	for (Std::list<Obj *>::iterator it = tile_obj_list.begin(); it != tile_obj_list.end(); ++it) {
		delete *it;
	}
	tile_obj_list.clear();

	return;
}

void ObjManager::clean_actor_inventories() {
	U6Link *link;
	uint16 i;

	for (i = 0; i < 256; i++) {
		if (actor_inventories[i]) {
			for (link = actor_inventories[i]->start(); link != NULL;) {
				Obj *obj = (Obj *)link->data;
				link = link->next;
				delete_obj(obj);
			}
			actor_inventories[i]->removeAll();
		}
	}

	return;
}

/*
U6LList *ObjManager::get_obj_superchunk(uint16 x, uint16 y, uint8 level)
{
 uint16 i;

 if(level == 0)
   {
    i = y * 8 + x;
    return surface[i];
   }

 return dungeon[level-1];
}
*/

bool ObjManager::is_boundary(uint16 x, uint16 y, uint8 level, uint8 boundary_type, Obj *excluded_obj) {
	U6Link *link;
	U6LList *obj_list;
	Obj *obj;
	Tile *tile, *tile1;
	uint16 tile_num;
	bool check_tile;
	uint16 i, j;
	uint16 next_x, next_y;

	next_x = WRAPPED_COORD(x + 1, level);
	next_y = WRAPPED_COORD(y + 1, level);

	for (j = y; j <= y + 1; j++) {
		for (i = x; i <= x + 1; i++) {
			obj_list = get_obj_list(WRAPPED_COORD(i, level), WRAPPED_COORD(j, level), level);

			if (obj_list != NULL) {
				link = obj_list->end();

				for (check_tile = false; link != NULL; link = link->prev) {
					obj = (Obj *)link->data;
					if (obj == excluded_obj)
						continue;
					tile_num = get_obj_tile_num(obj->obj_n) + obj->frame_n;
					tile = tile_manager->get_original_tile(tile_num);

					if (obj->x == x && obj->y == y) {
						check_tile = true;
					}
					if (tile->dbl_width && obj->x == next_x && obj->y == y) {
						tile_num--;
						check_tile = true;
					}
					if (tile->dbl_height && obj->x == x && obj->y == next_y) {
						tile_num--;
						check_tile = true;
					}
					if (obj->x == next_x && obj->y == next_y && tile->dbl_width && tile->dbl_height) {
						tile_num -= 2;
						check_tile = true;
					}
					if (check_tile) {
						tile1 = tile_manager->get_tile(tile_num);
						if (tile1->flags2 & boundary_type) //either TILEFLAG_BOUNDARY or TILEFLAG_MISSILE_BOUNDARY
							return true;

						check_tile = false;
					}
				}
			}
		}
	}

	return false;
}

/* no longer needed.
//FIX this needs to be moved magicnumbers :(
bool ObjManager::is_door(Obj * obj)
{
 //for U6
 if((obj->obj_n >= 297 && obj->obj_n <= 300) || obj->obj_n == 334 || obj->obj_n == 213) //OBJ_U6_MOUSEHOLE)
   return true;

 return false;
}
*/

uint8 ObjManager::is_passable(uint16 x, uint16 y, uint8 level) {
	U6Link *link;
	U6LList *obj_list;
	Obj *obj;
	Tile *tile, *tile1;
	uint16 tile_num;
	bool check_tile;
	bool object_at_location = false;
	uint16 i, j;

	uint16 x2 = WRAPPED_COORD((x + 1), level); // wrap on map edge
	uint16 y2 = WRAPPED_COORD((y + 1), level);

	for (i = x;; i = x2) { // only checks x and x2
		for (j = y;; j = y2) { // only checks y and y2
			obj_list = get_obj_list(i, j, level);
			if (i == x && j == y && obj_list) {
				if (obj_list->end() != NULL)
					object_at_location = true;
			}
			if (obj_list != NULL) {
				link = obj_list->end();

				for (check_tile = false; link != NULL; link = link->prev) {
					obj = (Obj *)link->data;
					tile_num = get_obj_tile_num(obj->obj_n) + obj->frame_n;
					tile = tile_manager->get_original_tile(tile_num);

					if (obj->x == x && obj->y == y) {
						check_tile = true;
					}
					if (tile->dbl_width && obj->x == x2 && obj->y == y) {
						tile_num--;
						check_tile = true;
					}
					if (tile->dbl_height && obj->x == x && obj->y == y2) {
						tile_num--;
						check_tile = true;
					}
					if (obj->x == x2 && obj->y == y2 && tile->dbl_width && tile->dbl_height) {
						tile_num -= 3;
						check_tile = true;
					}
					if (check_tile) {
						tile1 = tile_manager->get_original_tile(tile_num);
						if (tile1->passable == false)
							return OBJ_NOT_PASSABLE;
						check_tile = false;
					}
				}
			}
			if (j == y) j = y2;
			else break;
		}
		if (i == x) i = x2;
		else break;
	}

	if (object_at_location)
		return OBJ_PASSABLE;

	return OBJ_NO_OBJ;
}

bool ObjManager::is_forced_passable(uint16 x, uint16 y, uint8 level) {
	U6LList *obj_list;
	U6Link *link;
	Obj *obj;
	Tile *tile;

	obj_list = get_obj_list(x, y, level);

	if (obj_list) {
		for (link = obj_list->start(); link != NULL; link = link->next) {
			obj = (Obj *)link->data;
			tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n);
			if (tile->flags3 & TILEFLAG_FORCED_PASSABLE)
				return true;
		}
	}

	return false;
}

bool ObjManager::is_door(uint16 x, uint16 y, uint8 level) {
	U6LList *obj_list = get_obj_list(x, y, level);
	U6Link *link;
	Obj *obj;

	if (obj_list) {
		for (link = obj_list->start(); link != NULL; link = link->next) {
			obj = (Obj *)link->data;
			if (usecode->is_door(obj))
				return true;
		}
	}
	return false;
}

bool ObjManager::is_damaging(uint16 x, uint16 y, uint8 level) {
	U6LList *obj_list;
	U6Link *link;
	Obj *obj;
	Tile *tile;

	obj_list = get_obj_list(x, y, level);

	if (obj_list) {
		for (link = obj_list->start(); link != NULL; link = link->next) {
			obj = (Obj *)link->data;
			tile = tile_manager->get_original_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n); //get_tile(get_obj_tile_num(obj->obj_n)+obj->frame_n);
			if (tile->flags1 & TILEFLAG_DAMAGING)
				return true;
		}
	}

	return false;
}

bool ObjManager::is_stackable(Obj *obj) {
// Tile *tile;

	if (obj == NULL)
		return false;
	if (obj->is_readied()) // readied objects cannot be stacked --SB-X
		return false;
	/*
	 tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n)+obj->frame_n);

	 if(tile_manager->tile_is_stackable(tile->tile_num))
	   return true;

	 return false;
	*/
	if (game_type == NUVIE_GAME_U6) {
		switch (obj->obj_n) {
		case OBJ_U6_TORCH: // 0x5A, // torch
			if (obj->frame_n == 1) {
				return false;
			} else {
				return true;
			}
		case OBJ_U6_LOCK_PICK: // 0x3F, // lock pick
		case OBJ_U6_GEM: // 0x4D, // gem
		case OBJ_U6_ARROW: // 0x37, // arrow
		case OBJ_U6_BOLT: // 0x38, // bolt
		case OBJ_U6_BLACK_PEARL: // 0x41, // black pearl
		case OBJ_U6_BLOOD_MOSS: // 0x42, // bit of blood moss
		case OBJ_U6_GARLIC: // 0x43, // bulb of garlic
		case OBJ_U6_GINSENG: // 0x44, // ginseng root
		case OBJ_U6_MANDRAKE_ROOT: // 0x45, // mandrake root
		case OBJ_U6_NIGHTSHADE: // 0x46, // nightshade mushroom
		case OBJ_U6_SPIDER_SILK: // 0x47, // strand of spidersilk
		case OBJ_U6_SULFUROUS_ASH: // 0x48, // bit of sulfurous ash
		case OBJ_U6_EFFECT: // 0x151, // effect
		case OBJ_U6_BREAD: // 0x80, // loaf of bread
		case OBJ_U6_MEAT_PORTION: // 0x81, // portion of meat
		case OBJ_U6_FLASK_OF_OIL: // 0x53, // flask of oil
		case OBJ_U6_EGG: // 0x14F, // egg
		case OBJ_U6_GOLD_NUGGET: // 0x59, // gold nugget
		case OBJ_U6_ZU_YLEM: // 0x5B, // Zu Ylem
		case OBJ_U6_SNAKE_VENOM: // 0x5C, // silver snake venom
		case OBJ_U6_GOLD: // 0x58  // Gold coin
			return true;
		default:
			return false;
		}
	}

	if (game_type == NUVIE_GAME_SE) {
		switch (obj->obj_n) {
		case OBJ_SE_MAGNESIUM_RIBBON:
		case OBJ_SE_SPEAR:
		case OBJ_SE_THROWING_AXE:
		case OBJ_SE_POISONED_DART:
		case OBJ_SE_RIFLE_BULLET:
		case OBJ_SE_KNIFE:
		case OBJ_SE_ARROW:
		case OBJ_SE_TURTLE_BAIT:
		case OBJ_SE_FEATHER:
		case OBJ_SE_CHOCOLATL:
		case OBJ_SE_PINDE:
		case OBJ_SE_YOPO:
		case OBJ_SE_GOLD:
		case OBJ_SE_GOLD_NUGGET:
		case OBJ_SE_DIAMOND:
		case OBJ_SE_EMERALD:
		case OBJ_SE_RUBY:
		case OBJ_SE_CORN_MEAL:
		case OBJ_SE_TORTILLA:
		case OBJ_SE_MEAT_103:
		case OBJ_SE_BERRY:
		case OBJ_SE_CAKE:
		case OBJ_SE_CORN:
		case OBJ_SE_BEAN:
		case OBJ_SE_MEAT_110:
		case OBJ_SE_ORCHID:
		case OBJ_SE_PEPPER:
		case OBJ_SE_SULFUR:
		case OBJ_SE_CHARCOAL:
		case OBJ_SE_POTASSIUM_NITRATE:
		case OBJ_SE_SOFT_CLAY_POT:
		case OBJ_SE_FIRED_CLAY_POT:
		case OBJ_SE_CLOTH_STRIP:
		case OBJ_SE_GRENADE:
		case OBJ_SE_TAR:
		case OBJ_SE_WATER:
		case OBJ_SE_CLOTH:
		case OBJ_SE_TARRED_CLOTH_STRIP:
		case OBJ_SE_CLAY:
		case OBJ_SE_GUNPOWDER:
		case OBJ_SE_BRANCH:
		case OBJ_SE_TORCH:
		case OBJ_SE_FLAX:
		case OBJ_SE_RIB_BONE:
		case OBJ_SE_CHOP:
		case OBJ_SE_DEVICE:
			return true;
		default:
			return false;
		}
	}

	if (game_type == NUVIE_GAME_MD) {
		switch (obj->obj_n) {
		case OBJ_MD_PISTOL_ROUND:
		case OBJ_MD_SHOTGUN_SHELL:
		case OBJ_MD_RIFLE_ROUND:
		case OBJ_MD_ELEPHANT_GUN_ROUND:
		case OBJ_MD_SLING_STONE:
		case OBJ_MD_ARROW:
		case OBJ_MD_CAN_OF_LAMP_OIL:
		case OBJ_MD_MATCH:
		case OBJ_MD_TORCH:
		case OBJ_MD_BLOB_OF_OXIUM:
		case OBJ_MD_BERRY:
		case OBJ_MD_BERRY1:
		case OBJ_MD_BERRY2:
		case OBJ_MD_BERRY4:
		case OBJ_MD_CHIP_OF_RADIUM:
		case OBJ_MD_DOLLAR:
		case OBJ_MD_RUBLE:
		case OBJ_MD_WORMSBANE_SEED:
		case OBJ_MD_PAGE:
		case OBJ_MD_BERRY3:
		case OBJ_MD_OXYGENATED_AIR_BOTTLE:
			return true;
		default:
			return false;
		}
	}

	return (bool)obj_stackable[obj->obj_n];
}

bool ObjManager::is_breakable(Obj *obj) {
	if (game_type == NUVIE_GAME_U6) {
		switch (obj->obj_n) {
		case OBJ_U6_FLASK_OF_OIL:
		case OBJ_U6_SNAKE_VENOM:
		case OBJ_U6_CRYSTAL_BALL:
		case OBJ_U6_MIRROR:
		case OBJ_U6_WINE:
		case OBJ_U6_MEAD:
		case OBJ_U6_ALE:
		case OBJ_U6_WINE_GLASS:
		case OBJ_U6_PLATE:
		case OBJ_U6_MUG:
		case OBJ_U6_HONEY_JAR:
		case OBJ_U6_JAR_OF_HONEY:
		case OBJ_U6_POTION:
		case OBJ_U6_WATER_VASE:
		case OBJ_U6_DRAGON_EGG:
			return true;
		default:
			break;
		}
	} else if (game_type == NUVIE_GAME_SE) {
		switch (obj->obj_n) {
		case OBJ_SE_MORTAR:
		case OBJ_SE_GRINDING_STONE:
		case OBJ_SE_JUG_OF_PLACHTA:
		case OBJ_SE_BOTTLE_OF_LIQUOR:
		case OBJ_SE_JAR:
		case OBJ_SE_FIRED_CLAY_POT:
		case OBJ_SE_GRENADE:
		case OBJ_SE_JUG:
		case OBJ_SE_POT:
			return true;
		default:
			break;
		}
	}

	return false;
}

bool ObjManager::can_store_obj(Obj *target, Obj *src) {
	if (target == src || !can_get_obj(src) || target == NULL)
		return false;

	if (game_type == NUVIE_GAME_U6) {
		if (src->obj_n == OBJ_U6_TRAP)
			return false;

		if (target->obj_n == OBJ_U6_BAG
		        || target->obj_n == OBJ_U6_BACKPACK
		        || target->obj_n == OBJ_U6_BASKET
		        || (target->obj_n == OBJ_U6_CRATE && target->frame_n == 0)
		        || (target->obj_n == OBJ_U6_BARREL && target->frame_n == 0)
		        || (target->obj_n == OBJ_U6_CHEST && target->frame_n == 0)
		        || (target->obj_n == OBJ_U6_SPELLBOOK && src->obj_n == OBJ_U6_SPELL
		            && !target->find_in_container(OBJ_U6_SPELL, src->quality)
		            && !target->find_in_container(OBJ_U6_SPELL, 255)) // this quality contains all spells
		        || (target->obj_n == OBJ_U6_VORTEX_CUBE && src->obj_n == OBJ_U6_MOONSTONE))
			return true;

		if ((target->is_in_inventory() || Game::get_game()->doubleclick_opens_containers())
		        && ((target->obj_n == OBJ_U6_CHEST && target->frame_n == 1)
		            || target->obj_n == OBJ_U6_DEAD_BODY
		            || target->obj_n == OBJ_U6_MOUSE
		            || target->obj_n == OBJ_U6_REMAINS
		            || target->obj_n == OBJ_U6_DRAKE
		            || target->obj_n == OBJ_U6_MONGBAT))
			return true;

		if (Game::get_game()->doubleclick_opens_containers()
		        && (target->obj_n == OBJ_U6_DESK
		            || target->obj_n == OBJ_U6_DRAWER
		            || target->obj_n == OBJ_U6_GRAVE
		            || target->obj_n == OBJ_U6_REAPER
		            || target->obj_n == OBJ_U6_DEAD_GARGOYLE
		            || target->obj_n == OBJ_U6_DEAD_CYCLOPS))
			return true;
	} else if (game_type == NUVIE_GAME_SE) {
		if (src->has_container() || usecode->is_container(src))
			return false;
		if (target->obj_n == OBJ_SE_JUG || target->obj_n == OBJ_SE_POUCH
		        || target->obj_n == OBJ_SE_BASKET || target->obj_n == OBJ_SE_POT)
			return true;
		if (target->obj_n == OBJ_SE_MORTAR || target->obj_n == OBJ_SE_GRINDING_STONE
		        || target->obj_n == OBJ_SE_JAR) {
			switch (src->obj_n) {
			case OBJ_SE_MAGNESIUM_RIBBON:
			case OBJ_SE_CHOCOLATL:
			case OBJ_SE_PINDE:
			case OBJ_SE_YOPO:
			case OBJ_SE_CORN_MEAL:
			case OBJ_SE_CORN:
			case OBJ_SE_SULFUR:
			case OBJ_SE_CHARCOAL:
			case OBJ_SE_POTASSIUM_NITRATE:
			case OBJ_SE_GUNPOWDER:
				if (target->obj_n == OBJ_SE_JAR) {
					if (target->container_count_objects() == 0 || // only allow one object
					        target->find_in_container(src->obj_n, src->quality))
						return true;
					else
						return false;
				}
				return true;
			default:
				return false;
			}
		}
	} else { // MD
		if (src->has_container() || usecode->is_container(src))
			return false;
		switch (target->obj_n) {
		case OBJ_MD_BRASS_CHEST:
		case OBJ_MD_OBSIDIAN_BOX:
		case OBJ_MD_WOODEN_CRATE:
		case OBJ_MD_STEAMER_TRUNK:
		case OBJ_MD_BARREL:
		case OBJ_MD_CRATE:
		case OBJ_MD_BRASS_TRUNK:
			if (target->frame_n == 0) {
				return true;
			} else
				return false;
		case OBJ_MD_BACKPACK:
		case OBJ_MD_LARGE_SACK:
		case OBJ_MD_SMALL_POUCH:
		case OBJ_MD_CARPET_BAG:
		case OBJ_MD_BAG:
		case OBJ_MD_LEAD_BOX:
			return true;
		default:
			return false;
		}
	}
	return false;
}

bool ObjManager::can_get_obj(Obj *obj) {
	// objects with 0 weight aren't gettable.
	//255 is the max weight and means an object is movable but not getable.
	//we can't get object that contains toptiles either. This makes dragon bits ungettable etc.
	// excluding container items here, we just want the object itself to
	// check if it weighs 0 or 255. no need to scale as we don't compare
	// with other weights
	if (obj == NULL)
		return false;
	if (Game::get_game()->get_script()->call_can_get_obj_override(obj))
		return true;

	float weight = get_obj_weight(obj, OBJ_WEIGHT_EXCLUDE_CONTAINER_ITEMS, OBJ_WEIGHT_DONT_SCALE, OBJ_WEIGHT_EXCLUDE_QTY);
	if ((weight != 0 && weight != 255 && has_toptile(obj) == false
	        && (!obj->is_on_map() || !Game::get_game()->get_map_window()->tile_is_black(obj->x, obj->y, obj)))
	        || Game::get_game()->using_hackmove())
		return true;

	return false;
}

bool ObjManager::has_reduced_weight(uint16 obj_n) {
	// FIXME: HERE BE HARDCODED VALUES! FIXME: not sure if this list is complete!
	if (game_type == NUVIE_GAME_U6) { // luteijn: I only know about U6...
		if ((obj_n == OBJ_U6_GOLD)
		        || (obj_n == OBJ_U6_BLACK_PEARL) // not using range because don't want to depend on underlying magic numbers relations
		        || (obj_n == OBJ_U6_BLOOD_MOSS)
		        || (obj_n == OBJ_U6_GARLIC)
		        || (obj_n == OBJ_U6_GINSENG)
		        || (obj_n == OBJ_U6_MANDRAKE_ROOT)
		        || (obj_n == OBJ_U6_NIGHTSHADE)
		        || (obj_n == OBJ_U6_SPIDER_SILK)
		        || (obj_n == OBJ_U6_SULFUROUS_ASH)
		   ) {
			return true;
		}
	} else if (game_type == NUVIE_GAME_SE) {
		switch (obj_n) {
		case OBJ_SE_RIFLE_BULLET:
		case OBJ_SE_FEATHER:
		case OBJ_SE_CHOCOLATL:
		case OBJ_SE_PINDE:
		case OBJ_SE_YOPO:
		case OBJ_SE_GOLD:
		case OBJ_SE_DIAMOND:
		case OBJ_SE_EMERALD:
		case OBJ_SE_RUBY:
		case OBJ_SE_PEPPER:
		case OBJ_SE_SULFUR:
		case OBJ_SE_CHARCOAL:
		case OBJ_SE_POTASSIUM_NITRATE:
		case OBJ_SE_CLOTH_STRIP:
			return true;
		default:
			return false;
		}
	} else if (game_type == NUVIE_GAME_MD) {
		switch (obj_n) {
		case OBJ_MD_PISTOL_ROUND:
		case OBJ_MD_SHOTGUN_SHELL:
		case OBJ_MD_RIFLE_ROUND:
		case OBJ_MD_ELEPHANT_GUN_ROUND:
		case OBJ_MD_SLING_STONE:
		case OBJ_MD_ARROW:
		case OBJ_MD_POCKETWATCH:
		case OBJ_MD_SPECTACLES:
		case OBJ_MD_MASONIC_SYMBOL:
		case OBJ_MD_MATCH:
		case OBJ_MD_BLOB_OF_OXIUM:
		case OBJ_MD_BERRY:
		case OBJ_MD_BERRY1:
		case OBJ_MD_BERRY2:
		case OBJ_MD_BERRY4:
		case OBJ_MD_DREAMSTUFF:
		case OBJ_MD_DOLLAR:
		case OBJ_MD_RUBLE:
			return true;
		default:
			return false;
		}
	}

	return false;
}

bool ObjManager::has_toptile(Obj *obj) {
	Tile *tile;
	uint8 i = 1;

	tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n);

	if (tile->dbl_width)
		i++;

	if (tile->dbl_height)
		i++;

	if (tile->dbl_width && tile->dbl_height)
		i++;

	for (; i > 0; i--) {
		if (tile->toptile)
			return true;

		if (i != 1)
			tile = tile_manager->get_tile(tile->tile_num - 1);
	}

	return false;
}

//gets the linked list of objects at a particular location.

U6LList *ObjManager::get_obj_list(uint16 x, uint16 y, uint8 level) {
	iAVLTree *obj_tree;
	iAVLKey key;
	ObjTreeNode *item;

	WRAP_COORD(x, level); // wrap on map edge
	WRAP_COORD(y, level);

	obj_tree = get_obj_tree(x, y, level);
	key = get_obj_tree_key(x, y, level);

	item = (ObjTreeNode *)iAVLSearch(obj_tree, key);
	if (item)
		return item->obj_list;

	return NULL;
}

Tile *ObjManager::get_obj_tile(uint16 obj_n, uint8 frame_n) {
	return  tile_manager->get_tile(get_obj_tile_num(obj_n) + frame_n);
}

Tile *ObjManager::get_obj_tile(uint16 x, uint16 y, uint8 level, bool top_obj) {
	Obj *obj;
	Tile *tile;
	uint16 tile_num;

	obj = get_obj(x, y, level, top_obj);
	if (obj == NULL)
		return NULL;

	tile_num = get_obj_tile_num(obj->obj_n) + obj->frame_n;
	tile = tile_manager->get_tile(tile_num);

	if (tile->dbl_width && obj->x == x + 1 && obj->y == y)
		tile_num--;
	if (tile->dbl_height && obj->x == x && obj->y == y + 1)
		tile_num--;
	if (obj->x == x + 1 && obj->y == y + 1 && tile->dbl_width && tile->dbl_height)
		tile_num -= 2;

	return tile_manager->get_original_tile(tile_num);
}

Tile *ObjManager::get_obj_dmg_tile(uint16 x, uint16 y, uint8 level) {
	Tile *tile;
	U6LList *obj_list;
	U6Link *link;
	Obj *obj = NULL;

	obj_list = get_obj_list(x, y, level);

	if (obj_list != NULL) {
		for (link = obj_list->end(); link != NULL; link = link->prev) {
			obj = (Obj *)link->data;
			tile = tile_manager->get_original_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n);

			if (tile->damages == true)
				return tile;
		}
	}

	return NULL;
}

bool ObjManager::obj_is_damaging(Obj *obj, Actor *actor) {
	if (!obj)
		return false;

	Tile *tile = tile_manager->get_original_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n);

	if (tile && tile->damages == true) {
		if (actor) {
			MsgScroll *scroll = Game::get_game()->get_scroll();
			scroll->display_string("\n\nNot possible\n");
			Game::get_game()->get_script()->call_actor_tile_dmg(actor, tile->tile_num);
			actor->display_condition(); // indicate that object hurt the player
			scroll->display_string("\n");
			scroll->display_prompt();
		}
		return true;
	} else
		return false;
}

Obj *ObjManager::get_obj(uint16 x, uint16 y, uint8 level, bool top_obj, bool include_ignored_objects, Obj *excluded_obj) {
	Obj *obj;
	Tile *tile;

	obj = get_objBasedAt(x, y, level, top_obj, include_ignored_objects, excluded_obj);
	if (obj != NULL)
		return obj;

	obj = get_objBasedAt(x + 1, y + 1, level, top_obj, include_ignored_objects, excluded_obj);
	if (obj != NULL) {
		tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n);
		if (tile->dbl_width && tile->dbl_height)
			return obj;
	}

	obj = get_objBasedAt(x, y + 1, level, top_obj, include_ignored_objects, excluded_obj);
	if (obj != NULL) {
		tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n);
		if (tile->dbl_height)
			return obj;
	}

	obj = get_objBasedAt(x + 1, y, level, top_obj, include_ignored_objects, excluded_obj);
	if (obj != NULL) {
		tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n);
		if (tile->dbl_width)
			return obj;
	}


	return NULL;
}

Obj *ObjManager::get_obj_of_type_from_location_inc_multi_tile(uint16 obj_n, uint16 x, uint16 y, uint8 z) {
	return get_obj_of_type_from_location_inc_multi_tile(obj_n, -1, -1, x, y, z);
}

Obj *ObjManager::get_obj_of_type_from_location_inc_multi_tile(uint16 obj_n, sint16 quality, sint32 qty, uint16 x, uint16 y, uint8 z) {
	Obj *obj;
	Tile *tile;

	obj = get_obj_of_type_from_location(obj_n, quality, qty, x, y, z);
	if (obj != NULL)
		return obj;

	obj = get_obj_of_type_from_location(obj_n, quality, qty, x + 1, y + 1, z);
	if (obj != NULL) {
		tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n);
		if (tile->dbl_width && tile->dbl_height)
			return obj;
	}

	obj = get_obj_of_type_from_location(obj_n, quality, qty, x, y + 1, z);
	if (obj != NULL) {
		tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n);
		if (tile->dbl_height)
			return obj;
	}

	obj = get_obj_of_type_from_location(obj_n, quality, qty, x + 1, y, z);
	if (obj != NULL) {
		tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n) + obj->frame_n);
		if (tile->dbl_width)
			return obj;
	}

	return NULL;
}


Obj *ObjManager::get_obj_of_type_from_location(uint16 obj_n, uint16 x, uint16 y, uint8 z) {
	return get_obj_of_type_from_location(obj_n, -1, -1, x, y, z);
}

Obj *ObjManager::get_obj_of_type_from_location(uint16 obj_n, sint16 quality, sint32 qty, uint16 x, uint16 y, uint8 z) {
	U6LList *obj_list;
	U6Link *link;
	Obj *obj;

	obj_list = get_obj_list(x, y, z);

	if (obj_list == NULL)
		return NULL;
// start from the top of the stack
	for (link = obj_list->end(); link != NULL; link = link->prev) {
		obj = (Obj *)link->data;
		if (obj->obj_n == obj_n) {
			if (quality != -1 && obj->quality != (uint8)quality)
				continue;

			if (qty != -1 && obj->qty != (uint16)qty)
				continue;

			return obj;
		}
	}

	return NULL;
}

// x, y in world coords
Obj *ObjManager::get_objBasedAt(uint16 x, uint16 y, uint8 level, bool top_obj, bool include_ignored_objects, Obj *excluded_obj) {
	U6Link *link;
	U6LList *obj_list;
	Obj *obj;

	obj_list = get_obj_list(x, y, level);

	if (obj_list != NULL) {
		if (top_obj)
			link = obj_list->end();
		else
			link = obj_list->start();

		while (link != NULL) {
			obj = (Obj *)link->data;

			if (obj != excluded_obj) {
				if (include_ignored_objects)
					return obj;

				Tile *tile = get_obj_tile(obj->obj_n, obj->frame_n);
				if ((tile->flags3 & TILEFLAG_IGNORE) != TILEFLAG_IGNORE)
					return obj;
			}

			if (top_obj)
				link = link->prev;
			else
				link = link->next;
		}
	}

	return NULL;
}

// ObjManager keeps one instance of tile_obj per object.
// SE has 3 tile objects (Trees, Yucca Plants, and Oven Fires)
Obj *ObjManager::get_tile_obj(uint16 obj_n) {
	for (Std::list<Obj *>::iterator it = tile_obj_list.begin(); it != tile_obj_list.end(); ++it) {
		if ((*it)->obj_n == obj_n) {
			return *it;
		}
	}
	Obj *obj = new Obj();
	obj->obj_n = obj_n;
	obj->set_on_map(NULL);
	tile_obj_list.push_back(obj);
	return obj;
}

/*
bool ObjManager::add_obj(Obj *obj, bool addOnTop)
{
 return add_obj(get_obj_tree(obj->x,obj->y,obj->z), obj, addOnTop);
}
*/

bool ObjManager::remove_obj_from_map(Obj *obj) {
	U6LList *obj_list;

	if (obj->get_engine_loc() != OBJ_LOC_MAP)
		return false;

	obj_list = (U6LList *)obj->parent;

	if (obj_list == NULL)
		return false;

	obj_list->remove(obj);
	remove_obj(obj);

	return true;
}

void ObjManager::remove_obj(Obj *obj) {
	if (obj->status & OBJ_STATUS_TEMPORARY)
		temp_obj_list_remove(obj);

	if (obj->obj_n == obj_egg_table[game_type]) {
		egg_manager->remove_egg(obj);
	}

	obj->set_noloc();

	return;
}

// remove all objects of type obj_n from location (x,y,z)

bool ObjManager::remove_obj_type_from_location(uint16 obj_n, uint16 x, uint16 y, uint8 z) {
	U6LList *obj_list;
	U6Link *link;
	Obj *obj;
	bool objects_deleted = false;

	obj_list = get_obj_list(x, y, z);

	if (obj_list != NULL) {
		for (link = obj_list->start(); link != NULL;) {
			obj = (Obj *)link->data;
			link = link->next;

			if (obj->obj_n == obj_n) {
				remove_obj_from_map(obj);
				delete_obj(obj);
				objects_deleted = true;
			}
		}
	}

	return objects_deleted;
}

Obj *ObjManager::copy_obj(Obj *obj) {
	Obj *new_obj;

	if (obj == NULL)
		return NULL;

	new_obj = new Obj(*obj);
	/* changed to direct copy in case we add new members to Obj --SB-X
	 new_obj->obj_n = obj->obj_n;
	 new_obj->frame_n = obj->frame_n;

	 new_obj->status = obj->status;
	 new_obj->qty = obj->qty;
	 new_obj->quality = obj->quality;

	 new_obj->x = obj->x;
	 new_obj->y = obj->y;
	 new_obj->z = obj->z;*/

// should we copy container???
	new_obj->container = 0;

	return new_obj;
}

bool ObjManager::move(Obj *obj, uint16 x, uint16 y, uint8 level) {
	if (remove_obj_from_map(obj) == false)
		return false;

	obj->x = x;
	obj->y = y;
	obj->z = level;

	add_obj(obj, true); // add the object on top of the stack

	return true;
}

/* Returns an objects look-string, its general description.
 */
const char *ObjManager::look_obj(Obj *obj, bool show_prefix) {
	const char *desc;
	if (obj == NULL)
		return NULL;

	desc = tile_manager->lookAtTile(get_obj_tile_num(obj) + obj->frame_n, obj->qty, show_prefix);

	return desc;
}

const char *ObjManager::get_obj_name(Obj *obj) {
	return tile_manager->lookAtTile(get_obj_tile_num(obj->obj_n), 0, false);
}

const char *ObjManager::get_obj_name(uint16 obj_n) {
	return tile_manager->lookAtTile(get_obj_tile_num(obj_n), 0, false);
}

const char *ObjManager::get_obj_name(uint16 obj_n, uint8 frame_n) {
	return tile_manager->lookAtTile(get_obj_tile_num(obj_n) + frame_n, 0, false);
}

float ObjManager::get_obj_weight(uint16 obj_n) {
	float weight = (float)get_obj_weight_unscaled(obj_n);
	if (has_reduced_weight(obj_n)) {
		weight /= 10;
	}

	return weight / 10;
}

float ObjManager::get_obj_weight(Obj *obj, bool include_container_items, bool scale, bool include_qty) {
	float weight;
	U6Link *link;

	weight = obj_weight[obj->obj_n];

	if (is_stackable(obj)) {
		if (include_qty) {
			if (obj->qty == 0)
				obj->qty = 1;
			weight *= obj->qty;
		}
		/* luteijn: only some need to be divided by an extra 10 for a total of 100.
		 * unfortunately can't seem to find a tileflag that controls this so would have to be hardcoded!
		 */
		if (has_reduced_weight(obj)) {
			weight /= 10; // luteijn: regardless of the scaling flag!
		}
	}

	if (obj->container != NULL && include_container_items == OBJ_WEIGHT_INCLUDE_CONTAINER_ITEMS) {
		for (link = obj->container->start(); link != NULL; link = link->next)
			/* weight += get_obj_weight(reinterpret_cast<Obj*>(link->data), false);*/ //don't scale container objects yet.
			weight += get_obj_weight(reinterpret_cast<Obj *>(link->data), OBJ_WEIGHT_INCLUDE_CONTAINER_ITEMS, OBJ_WEIGHT_DONT_SCALE); //don't scale container objects yet. luteijn: and use the right flag to do so!
	}

	if (scale == OBJ_WEIGHT_DO_SCALE) {
		weight /= 10;
	}

	return weight;
}

uint16 ObjManager::get_obj_tile_num(uint16 obj_num) { //assume obj_num is < 1024 :)
	return obj_to_tile[obj_num];
}

inline bool ObjManager::is_corpse(Obj *obj) {
	if (game_type == NUVIE_GAME_U6) {
		switch (obj->obj_n) {
		case OBJ_U6_DEAD_BODY:
		case OBJ_U6_DEAD_CYCLOPS:
		case OBJ_U6_DEAD_GARGOYLE:
		case OBJ_U6_DOG: // Kador id_n 135
		case OBJ_U6_MOUSE: // Sherry id_n 9
		case OBJ_U6_HORSE_CARCASS: // Pushme Pullyu id 130, Smith id 132
			return true;
		default:
			break;
		}
	} else if (game_type == NUVIE_GAME_SE) {
		/* TODO - add SE body obj numbers
		switch (obj->obj_n) {
		default:
			break;
		}
		*/
	} else { // MD
		/* TODO - add MD body obj numbers
		switch (obj->obj_n) {
		default:
			break;
		}
		*/
	}
	return false;
}

uint16 ObjManager::get_obj_tile_num(Obj *obj) { //assume obj_num is < 1024 :)
	if (custom_actor_tiles && is_corpse(obj)) {
		return Game::get_game()->get_actor_manager()->get_actor(obj->quality)->get_custom_tile_num(obj->obj_n);
	}

	uint16 obj_num = obj->obj_n;
	// Savage Empire Tile Object (Get Tile from Map Location)
	if (game_type == NUVIE_GAME_SE &&
	        Game::get_game()->get_script()->call_is_tile_object(obj_num)) {
		return (Game::get_game()->get_game_map()->get_tile(obj->x, obj->y, obj->z)->tile_num);
	}
	return get_obj_tile_num(obj_num);
}

void ObjManager::set_obj_tile_num(uint16 obj_num, uint16 tile_num) {
	obj_to_tile[obj_num] = tile_num;
	return;
}

/* Animate all visible tiles of an object `loop_count' times. */
void ObjManager::animate_forwards(Obj *obj, uint32 loop_count) {
	// In U6 there is no place where one object must animate and nearby objects
	// of the same type don't also animate, so just forward to TileManager.
	tile_manager->set_anim_loop(get_obj_tile_num(obj->obj_n), loop_count, 0);
}

/* Animate in reverse all visible tiles of an object `loop_count' times. */
void ObjManager::animate_backwards(Obj *obj, uint32 loop_count) {
	tile_manager->set_anim_loop(get_obj_tile_num(obj->obj_n), loop_count, 1);
}


U6LList *ObjManager::get_actor_inventory(uint16 actor_num) {
	if (actor_num >= 256)
		return NULL;

	if (actor_inventories[actor_num] == NULL) {
		actor_inventories[actor_num] = new U6LList();
	}

	return actor_inventories[actor_num];
}

bool ObjManager::actor_has_inventory(uint16 actor_num) {
	if (actor_inventories[actor_num] != NULL) {
		if (actor_inventories[actor_num]->start() != NULL)
			return true;
	}

	return false;
}

Obj *ObjManager::find_next_obj(uint8 level, Obj *prev_obj, bool match_frame_n, bool match_quality) {
	if (prev_obj == NULL)
		return NULL;

	Obj **p = &prev_obj;

	return find_obj(level, prev_obj->obj_n, prev_obj->quality, match_quality, prev_obj->frame_n, match_frame_n, p);
}

Obj *ObjManager::find_obj(uint8 level, uint16 obj_n, uint8 quality, bool match_quality, uint16 frame_n, bool match_frame_n, Obj **prev_obj) {
	uint8 i;
	Obj *new_obj;

	if (level == 0) {
		for (i = 0; i < 64; i++) {
			new_obj = find_obj_in_tree(obj_n, quality, match_quality, frame_n, match_frame_n, prev_obj, surface[i]);
			if (new_obj != NULL)
				return new_obj;
		}
	} else {
		new_obj = find_obj_in_tree(obj_n, quality, match_quality, frame_n, match_frame_n, prev_obj, dungeon[level - 1]);
		if (new_obj != NULL)
			return new_obj;
	}

	return NULL;
}

inline Obj *ObjManager::find_obj_in_tree(uint16 obj_n, uint8 quality, bool match_quality, uint8 frame_n, bool match_frame_n, Obj **prev_obj, iAVLTree *obj_tree) {
	iAVLCursor cursor;
	ObjTreeNode *node;
	U6Link *link;
	Obj *new_obj;

	node = (ObjTreeNode *)iAVLFirst(&cursor, obj_tree);

	for (; node != NULL;) {
		link = ((U6LList *)(node->obj_list))->start();
		for (; link != NULL; link = link->next) {
			new_obj = (Obj *)link->data;
			if (new_obj->obj_n == obj_n && (match_quality == false || new_obj->quality == quality) && (match_frame_n == false || new_obj->frame_n == frame_n)) {
				if (prev_obj != NULL && new_obj == *prev_obj)
					*prev_obj = NULL;
				else {
					if (prev_obj == NULL || *prev_obj == NULL)
						return new_obj;
				}
			}
			/* Don't search containers.
			     if(prev_obj == NULL)
			     {
			         new_obj = new_obj->find_in_container(obj_n, quality, match_quality, frame_n, match_frame_n, prev_obj);
			         if(new_obj)
			           return new_obj;
			     }
			*/
		}

		node = (ObjTreeNode *)iAVLNext(&cursor);
	}

	return NULL;
}

bool ObjManager::add_obj(Obj *obj, bool addOnTop) {
	iAVLTree *obj_tree;
	ObjTreeNode *node;
	U6LList *obj_list;
	iAVLKey key;

	obj_tree = get_obj_tree(obj->x, obj->y, obj->z);
	key = get_obj_tree_key(obj);

	node = (ObjTreeNode *)iAVLSearch(obj_tree, key);

	if (node == NULL) {
		obj_list = new U6LList();

		node = new ObjTreeNode;
		node->key = key;
		node->obj_list = obj_list;

		iAVLInsert(obj_tree, node);
	} else {
		obj_list = node->obj_list;
	}

	if (addOnTop)
		obj_list->add(obj);
	else
		obj_list->addAtPos(0, obj);

	if (obj->status & OBJ_STATUS_TEMPORARY)
		temp_obj_list_add(obj);

	obj->set_on_map(obj_list); //mark object as on map.

	return true;
}
bool ObjManager::addObjToContainer(U6LList *llist, Obj *obj) {
	U6Link *link;
	Obj *c_obj = NULL; //container object
	uint16 index;

	index = ((obj->y & 0x3f) << 10) + obj->x; //10 bits from x and 6 bits from y

	link = llist->gotoPos(index);
	if (link != NULL)
		c_obj = (Obj *)link->data;

	if (c_obj) { // we've found our container.
		c_obj->add(obj);

		//DEBUG(0,LEVEL_DEBUGGING,"Cont: %s\n", tile_manager->lookAtTile(get_obj_tile_num(c_obj->obj_n)+c_obj->frame_n,0,false));
		//DEBUG(0,LEVEL_DEBUGGING,"Add to container %s", tile_manager->lookAtTile(get_obj_tile_num(obj->obj_n)+obj->frame_n,0,false));
		//DEBUG(1,LEVEL_DEBUGGING," -> %s (%x,%x,%x)\n", tile_manager->lookAtTile(get_obj_tile_num(c_obj->obj_n)+c_obj->frame_n,0,false),c_obj->x,c_obj->y,c_obj->z);
		return true;
	}

	return false;
}

Obj *ObjManager::loadObj(NuvieIO *buf) {
	uint8 b1, b2;
	Obj *obj;

	obj = new Obj();
//obj->objblk_n = objblk_n;

	obj->status = buf->read1();

//set new nuvie location bits.
	switch (obj->status & OBJ_STATUS_MASK_GET) {
	case OBJ_STATUS_ON_MAP :
		obj->set_on_map(NULL);
		break;//obj->nuvie_status |= OBJ_LOC_MAP; break;
	case OBJ_STATUS_IN_CONTAINER :
		obj->set_in_container(NULL);
		break;//obj->nuvie_status |= OBJ_LOC_CONT; break;
	case OBJ_STATUS_IN_INVENTORY :
		obj->set_in_inventory();
		break;//obj->nuvie_status |= OBJ_LOC_INV; break;
	case OBJ_STATUS_READIED :
		obj->readied();
		break;//obj->nuvie_status |= OBJ_LOC_READIED; break;
	}

	obj->x = buf->read1(); // h
	b1 = buf->read1();
	obj->x += (b1 & 0x3) << 8;

	obj->y = (b1 & 0xfc) >> 2;
	b2 = buf->read1();
	obj->y += (b2 & 0xf) << 6;

	obj->z = (b2 & 0xf0) >> 4;

	b1 = buf->read1();
	b2 = buf->read1();
	obj->obj_n = b1;
	obj->obj_n += (b2 & 0x3) << 8;

	obj->frame_n = (b2 & 0xfc) >> 2;

	obj->qty = buf->read1();
	obj->quality = buf->read1();
	if (is_stackable(obj))
		obj->qty = (uint16)(obj->quality << 8) + obj->qty;

//if(obj->qty == 0)
//  obj->qty = 1;

	return obj;
}


iAVLTree *ObjManager::get_obj_tree(uint16 x, uint16 y, uint8 level) {
	if (level == 0) {
		x >>= 7; // x = floor(x / 128)   128 = superchunk width
		y >>= 7; // y = floor(y / 128)   128 = superchunk height

		return surface[x + y * 8];
	}

	if (level > 5)
		return NULL;

	return dungeon[level - 1];
}

inline iAVLKey ObjManager::get_obj_tree_key(Obj *obj) {
	return get_obj_tree_key(obj->x, obj->y, obj->z);
}

iAVLKey ObjManager::get_obj_tree_key(uint16 x, uint16 y, uint8 level) {
	iAVLKey key;
	if (level == 0)
		key._int = y * 1024 + x;
	else
		key._int = y * 256 + x;

	return key;
}

void ObjManager::update(uint16 x, uint16 y, uint8 z, bool teleport) {
	uint16 cur_blk_x, cur_blk_y;

	cur_blk_x = x >> 3; // x / 8;
	cur_blk_y = y >> 3; // y / 8;

// We're changing levels so clean out all temp objects on the current level.
	if (last_obj_blk_z != z) {
		if (last_obj_blk_z != OBJ_TEMP_INIT)
			temp_obj_list_clean_level(last_obj_blk_z);

		egg_manager->spawn_eggs(x, y, z, teleport);

		last_obj_blk_x = cur_blk_x;
		last_obj_blk_y = cur_blk_y;
		last_obj_blk_z = z;

		return;
	}

//FIX for level change. we want to remove all temps on level change.
	if (cur_blk_x != last_obj_blk_x || cur_blk_y != last_obj_blk_y) {
		last_obj_blk_x = cur_blk_x;
		last_obj_blk_y = cur_blk_y;

		temp_obj_list_clean_area(x, y);
		egg_manager->spawn_eggs(x, y, z, teleport);

	}

	return;
}

bool ObjManager::temp_obj_list_add(Obj *obj) {
	if (obj == NULL)
		return false;

	temp_obj_list.push_back(obj);

	return true;
}

bool ObjManager::temp_obj_list_remove(Obj *obj) {
	temp_obj_list.remove(obj);
	return true;
}

void ObjManager::remove_temp_obj(Obj *tmp_obj) {
	//FIXME MD has special temp object flag override logic. This should be implemented in lua script.
	if (game_type != NUVIE_GAME_MD || (tmp_obj->obj_n != OBJ_MD_DREAM_TELEPORTER && tmp_obj->frame_n != 0)) {
		DEBUG(0,
		      LEVEL_DEBUGGING,
		      "Removing obj %s.\n",
		      tile_manager->lookAtTile(get_obj_tile_num((tmp_obj)->obj_n) + (tmp_obj)->frame_n, 0, false));
		remove_obj_from_map(tmp_obj);
		delete_obj(tmp_obj);
	}
}

// clean objects from a whole level.
void ObjManager::temp_obj_list_clean_level(uint8 z) {
	Std::list<Obj *>::iterator obj;
	Obj *tmp_obj;

	for (obj = temp_obj_list.begin(); obj != temp_obj_list.end();) {
		if ((*obj)->z == z) {
			tmp_obj = *obj++;
			remove_temp_obj(tmp_obj);
		} else
			obj++;
	}

	return;
}


// Clean objects more than 19 tiles from position
void ObjManager::temp_obj_list_clean_area(uint16 x, uint16 y) {
	Std::list<Obj *>::iterator obj;
	Obj *tmp_obj;
	sint16 dist_x, dist_y;

	for (obj = temp_obj_list.begin(); obj != temp_obj_list.end();) {
		dist_x = abs((sint16)(*obj)->x - x);
		dist_y = abs((sint16)(*obj)->y - y);

		if (dist_x > 19 || dist_y > 19) {
			tmp_obj = *obj++;
			remove_temp_obj(tmp_obj);
		} else
			obj++;
	}

	return;
}

/*
inline U6LList *ObjManager::get_schunk_list(uint16 x, uint16 y, uint8 level)
{
 uint16 sx, sy;

 if(level == 0)
   {
    sx = x / 128;
    sy = y / 128;
    return surface[sy * 8 + sx];
   }

 return dungeon[level-1];
}
*/
//prints a human readable list of object number / names.

void ObjManager::print_object_list() {
	uint16 i;

	DEBUG(0, LEVEL_INFORMATIONAL, "print_object_list:\n");
	for (i = 0; i < 1024; i++) {
		DEBUG(1, LEVEL_INFORMATIONAL, "%04d: %s\n", i, tile_manager->lookAtTile(get_obj_tile_num(i), 0, false));
	}

	return;
}

void ObjManager::print_egg_list() {
	uint8 i;

	for (i = 0; i < 64; i++)
		print_egg_tree(surface[i]);

	for (i = 0; i < 5; i++)
		print_egg_tree(dungeon[i]);

	return;
}

inline void ObjManager::print_egg_tree(iAVLTree *obj_tree) {
	ObjTreeNode *tree_node;
	iAVLCursor cursor;
	U6LList *obj_list;
	U6Link *link;
	Obj *obj;

	tree_node = (ObjTreeNode *)iAVLFirst(&cursor, obj_tree);

	for (; tree_node != NULL; tree_node = (ObjTreeNode *)iAVLNext(&cursor)) {
		obj_list = (U6LList *)tree_node->obj_list;
		for (link = obj_list->start(); link != NULL; link = link->next) {
			obj = (Obj *)link->data;
			if (obj->obj_n == 335) {
				print_obj(obj, false);
			}
		}
	}

	return;
}

void ObjManager::print_obj(Obj *obj, bool in_container, uint8 indent) {
	U6Link *link;
	Obj *container_obj;
	const CombatType *c_type = NULL;
	Actor *a = Game::get_game()->get_player()->get_actor();

	if (a != NULL)
		c_type = a->get_object_combat_type(obj->obj_n);

	DEBUG(1, LEVEL_INFORMATIONAL, "\n");
	print_indent(LEVEL_INFORMATIONAL, indent);
	DEBUG(1, LEVEL_INFORMATIONAL, "%s ", tile_manager->lookAtTile(get_obj_tile_num(obj->obj_n) + obj->frame_n, 0, false));

	if (in_container == false)
		DEBUG(1, LEVEL_INFORMATIONAL, "at %x, %x, %x (%d,%d,%d)", obj->x, obj->y, obj->z, obj->x, obj->y, obj->z);
	DEBUG(1, LEVEL_INFORMATIONAL, "\n");

	print_indent(LEVEL_INFORMATIONAL, indent);
	DEBUG(1, LEVEL_INFORMATIONAL, "object (Obj *) %p\n", obj);
	print_indent(LEVEL_INFORMATIONAL, indent);

	DEBUG(1, LEVEL_INFORMATIONAL, "engine loc: ");
	switch (obj->get_engine_loc()) {
	case OBJ_LOC_MAP :
		DEBUG(1, LEVEL_INFORMATIONAL, "MAP");
		break;
	case OBJ_LOC_CONT :
		DEBUG(1, LEVEL_INFORMATIONAL, "CONTAINER");
		break;
	case OBJ_LOC_INV :
		DEBUG(1, LEVEL_INFORMATIONAL, "INVENTORY");
		break;
	case OBJ_LOC_READIED :
		DEBUG(1, LEVEL_INFORMATIONAL, "INVENTORY READIED");
		break;
	case OBJ_LOC_NONE :
		DEBUG(1, LEVEL_INFORMATIONAL, "NONE");
		break;
	default :
		DEBUG(1, LEVEL_INFORMATIONAL, "**UNKNOWN**");
		break;
	}
	if (obj->is_actor_obj())
		DEBUG(1, LEVEL_INFORMATIONAL, " (ACTOR_OBJ)");
	DEBUG(1, LEVEL_INFORMATIONAL, "\n");

	DEBUG(1, LEVEL_INFORMATIONAL, "parent (");
	switch (obj->get_engine_loc()) {
	case OBJ_LOC_MAP :
		DEBUG(1, LEVEL_INFORMATIONAL, "U6LList");
		break;
	case OBJ_LOC_CONT :
		DEBUG(1, LEVEL_INFORMATIONAL, "Obj");
		break;
	case OBJ_LOC_INV :
	case OBJ_LOC_READIED :
		DEBUG(1, LEVEL_INFORMATIONAL, "Actor");
		break;
	default :
		DEBUG(1, LEVEL_INFORMATIONAL, "void");
		break;
	}
	DEBUG(1, LEVEL_INFORMATIONAL, " *) %p\n", obj->parent);

	print_indent(LEVEL_INFORMATIONAL, indent);
// DEBUG(1,LEVEL_DEBUGGING,"objblk_n: %d\n", obj->objblk_n);

	print_indent(LEVEL_INFORMATIONAL, indent);
	DEBUG(1, LEVEL_INFORMATIONAL, "obj_n: %d\n", obj->obj_n);

	print_indent(LEVEL_INFORMATIONAL, indent);
	DEBUG(1, LEVEL_INFORMATIONAL, "frame_n: %d\n", obj->frame_n);

	print_indent(LEVEL_INFORMATIONAL, indent);
	DEBUG(1, LEVEL_INFORMATIONAL, "Tile: %d\n", get_obj_tile_num(obj->obj_n));

	print_indent(LEVEL_INFORMATIONAL, indent);
	DEBUG(1, LEVEL_INFORMATIONAL, "Status: ");
	print_b(LEVEL_INFORMATIONAL, obj->status);
	if (obj->status != 0) {
		DEBUG(1, LEVEL_INFORMATIONAL, " ( ");
		if (obj->is_readied())
			DEBUG(1, LEVEL_INFORMATIONAL, "POS:Ready ");
		else if (obj->is_in_container())
			DEBUG(1, LEVEL_INFORMATIONAL, "POS:Cont ");
		else if (obj->is_in_inventory())
			DEBUG(1, LEVEL_INFORMATIONAL, "POS:Inv ");
		if (obj->is_ok_to_take())
			DEBUG(1, LEVEL_INFORMATIONAL, "OK ");
		if (obj->is_temporary())
			DEBUG(1, LEVEL_INFORMATIONAL, "TEMP ");
		if (obj->is_invisible())
			DEBUG(1, LEVEL_INFORMATIONAL, "INVIS ");
		if (obj->is_egg_active()) {
			if (obj->obj_n < 256)
				DEBUG(1, LEVEL_INFORMATIONAL, "MUTANT ");
			else
				DEBUG(1, LEVEL_INFORMATIONAL, "BROKEN ");
		}

		DEBUG(1, LEVEL_INFORMATIONAL, ")");
	}

	DEBUG(1, LEVEL_INFORMATIONAL, "\n");

	if (in_container) {
		print_indent(LEVEL_INFORMATIONAL, indent);
		DEBUG(1, LEVEL_INFORMATIONAL, "parent_id = %d, y = %d, z = %d\n", obj->x, obj->y, obj->z);
	}

	print_indent(LEVEL_INFORMATIONAL, indent);
	DEBUG(1, LEVEL_INFORMATIONAL, "Quantity: %d\n", obj->qty);
	print_indent(LEVEL_INFORMATIONAL, indent);
	DEBUG(1, LEVEL_INFORMATIONAL, "Quality: %d\n", obj->quality);
	if (c_type != NULL) {
		DEBUG(1, LEVEL_INFORMATIONAL, "attack/damage = %d, defence/defense = %d\n", c_type->damage, c_type->defense); // FIXME add the rest of the combat values
	}

	if (obj->container) {
		print_indent(LEVEL_INFORMATIONAL, indent);
		DEBUG(1, LEVEL_INFORMATIONAL, "Container\n");
		print_indent(LEVEL_INFORMATIONAL, indent);
		DEBUG(1, LEVEL_INFORMATIONAL, "---------");

		for (link = obj->container->start(); link != NULL; link = link->next) {
			container_obj = (Obj *)link->data;
			print_obj(container_obj, true, indent + 2);
		}

		print_indent(LEVEL_INFORMATIONAL, indent);
		DEBUG(1, LEVEL_INFORMATIONAL, "---------\n");
	}

	if (in_container == false)
		DEBUG(1, LEVEL_INFORMATIONAL, "\n");

	return;
}

Obj *new_obj(uint16 obj_n, uint8 frame_n, uint16 x, uint16 y, uint16 z) {
	Obj *obj;

	obj = new Obj();

	obj->obj_n = obj_n;
	obj->frame_n = frame_n;

	obj->x = x;
	obj->y = y;
	obj->z = z;

	return obj;
}

void delete_obj(Obj *obj) {
	U6Link *link;

	if (obj->is_script_obj() == false) {
		if (obj->container) {
			for (link = obj->container->start(); link != NULL;) {
				Obj *cont_obj = (Obj *)link->data;
				link = link->next;

				delete_obj(cont_obj);
			}
		}

		if (obj->container)
			delete obj->container;
		delete obj;
	}

	return;
}

// add object to list, stacking with existing objects if possible
// This is used for adding objects to inventory OR a container.
// *It will stack onto the new object and delete the existing object!*

//FIXME!!!!! We need to set on_map() etc if going to the map.

bool ObjManager::list_add_obj(U6LList *llist, Obj *obj, bool stack_objects, uint32 pos) {
	Obj *stack_with;
	uint16 new_qty;
	U6Link *link;

	if (!llist || !obj)
		return false;

	assert(pos == 0 || pos < llist->count());

	if (stack_objects && is_stackable(obj)) {
		for (link = llist->start(); link != NULL;) {
			stack_with = (Obj *)link->data;
			link = link->next;

			if (stack_with->obj_n == obj->obj_n && stack_with->frame_n == obj->frame_n
			        && stack_with->quality == obj->quality && is_stackable(stack_with)) {
				new_qty = obj->qty + stack_with->qty;
				obj->qty = new_qty;
				llist->addAtPos(llist->findPos(stack_with), obj);

				llist->remove(stack_with);
				delete_obj(stack_with);

				return true;
			}
		}
	}

	llist->addAtPos(pos, obj);

	return true;
}

/* Call load usecode for all objects (after loading them). This should be in
 * loadObj() but that was crashing when usecode tried to use timers.
 */
void ObjManager::startObjs() {
	uint8 i;

//iterate through surface chunks.
	for (i = 0; i < 64; i++)
		start_obj_usecode(surface[i]);

//iterate through dungeon chunks.
	for (i = 0; i < 5; i++)
		start_obj_usecode(dungeon[i]);

}

inline void ObjManager::start_obj_usecode(iAVLTree *obj_tree) {
	ObjTreeNode *tree_node;
	iAVLCursor cursor;
	U6LList *obj_list;
	U6Link *link;
	Obj *obj;

	tree_node = (ObjTreeNode *)iAVLFirst(&cursor, obj_tree);
	for (; tree_node != NULL; tree_node = (ObjTreeNode *)iAVLNext(&cursor)) {
		obj_list = (U6LList *)tree_node->obj_list;
		for (link = obj_list->start(); link != NULL; link = link->next) {
			obj = (Obj *)link->data;
			if (usecode->has_loadcode(obj))
				usecode->load_obj(obj);
		}
	}
}


/* Subtract an object stack with quantity set to `count' from original object
 * stack `obj'.
 * Returns a new object if a stack could be subtracted from the original,
 * leaving the original intact.
 * Returns the original if its quantity was smaller than the requested count or
 * it is not stackable.
 */
Obj *ObjManager::get_obj_from_stack(Obj *obj, uint32 count) {
	if (count == 0 || obj->qty <= count || !is_stackable(obj))
		return (obj);
	// requested is over 0, original quantity is greater than requested, object
	//  is stackable
	Obj *new_obj = copy_obj(obj);
	new_obj->qty = count;
	obj->qty -= count; // remove requested from original
	return (new_obj);
}

void clean_obj_tree_node(void *node) {
	U6Link *link;
	ObjTreeNode *obj_node = (ObjTreeNode *)node;

	for (link = obj_node->obj_list->start(); link != NULL;) {
		Obj *obj = (Obj *)link->data;
		link = link->next;

		delete_obj(obj);
	}

	delete obj_node->obj_list;
	delete obj_node;

	return;
}

bool ObjManager::unlink_from_engine(Obj *obj, bool run_usecode) {
	Actor *a;
	Obj *cont_obj;

	switch (obj->get_engine_loc()) {
	case OBJ_LOC_NONE :
		break;
	case OBJ_LOC_MAP :
		remove_obj_from_map(obj);
		break;
// inventory_remove_obj unreadies
	case OBJ_LOC_READIED :/* a = (Actor *)obj->parent;
                           a->remove_readied_object(obj, run_usecode);
                           a->inventory_remove_obj(obj, run_usecode);
                           break;
*/
	case OBJ_LOC_INV :
		a = (Actor *)obj->parent;
		a->inventory_remove_obj(obj, run_usecode);
		break;

	case OBJ_LOC_CONT :
		cont_obj = obj->get_container_obj();
		if (cont_obj)
			cont_obj->remove(obj); //remove from parent container.
		break;
		break;
	}

	return true;
}

bool ObjManager::moveto_map(Obj *obj, MapCoord location) {
	unlink_from_engine(obj);

	obj->x = location.x;
	obj->y = location.y;
	obj->z = location.z;

	add_obj(obj, OBJ_ADD_TOP);

	return true;
}

bool ObjManager::moveto_inventory(Obj *obj, uint16 actor_num) {
	ActorManager *am = Game::get_game()->get_actor_manager();
	if (!am)
		return false;

	return moveto_inventory(obj, am->get_actor(actor_num));
}

bool ObjManager::moveto_inventory(Obj *obj, Actor *actor) {
	unlink_from_engine(obj);
	actor->inventory_add_object(obj);

	return true;
}

bool ObjManager::moveto_container(Obj *obj, Obj *container_obj, bool stack) {
	if (obj == container_obj)
		return false;

	unlink_from_engine(obj);
	container_obj->add(obj, stack);
	if (game_type == NUVIE_GAME_SE) {
		if (container_obj->obj_n == OBJ_SE_JAR) { // frame changes depending on contents
			switch (obj->obj_n) {
			case OBJ_SE_CORN_MEAL:
			case OBJ_SE_CORN:
			case OBJ_SE_SULFUR:
				container_obj->frame_n = 1; // yellow jar
				break;
			case OBJ_SE_MAGNESIUM_RIBBON:
			case OBJ_SE_POTASSIUM_NITRATE:
				container_obj->frame_n = 2; // white jar
				break;
			default:
				container_obj->frame_n = 3; // black jar
				break;
			}
		}
	}
	return true;
}

} // End of namespace Nuvie
} // End of namespace Ultima
