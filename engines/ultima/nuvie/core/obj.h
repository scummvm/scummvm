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

#ifndef NUVIE_CORE_OBJ_H
#define NUVIE_CORE_OBJ_H

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_llist.h"

namespace Ultima {
namespace Nuvie {

class Actor;

#define NO_OBJ_STATUS 0

// obj status bit flags
#define OBJ_STATUS_OK_TO_TAKE    0x1
//#define OBJ_STATUS_SEEN_EGG      0x2  // something to do with eggs <- not sure about this one.
#define OBJ_STATUS_INVISIBLE     0x2  // I think this is correct
#define OBJ_STATUS_CHARMED       0x4 // objlist.txt says 'charmed'


// position: A 2 bit field, so can't use plain | to check / |= to set these.
// FIXME: check to make sure we don't do this anywhere anymore
#define OBJ_STATUS_ON_MAP        0x0
#define OBJ_STATUS_IN_CONTAINER  0x8
#define OBJ_STATUS_IN_INVENTORY 0x10
#define OBJ_STATUS_READIED      0x18
#define OBJ_STATUS_MASK_GET     0x18
#define OBJ_STATUS_MASK_SET     0xE7

#define OBJ_STATUS_TEMPORARY    0x20
#define OBJ_STATUS_EGG_ACTIVE   0x40  // something to do with eggs
#define OBJ_STATUS_BROKEN       0x40
#define OBJ_STATUS_MUTANT       0x40
#define OBJ_STATUS_CURSED       0x40
#define OBJ_STATUS_LIT          0x80


//first 3 bits of nuvie_status code object location
//in the nuvie engine.

//Nuvie engine obj locations
#define OBJ_LOC_NONE    0
#define OBJ_LOC_INV     1
#define OBJ_LOC_MAP     2
#define OBJ_LOC_READIED 3
#define OBJ_LOC_CONT    4

#define NUVIE_OBJ_STATUS_LOC_MASK_GET 0x7
#define NUVIE_OBJ_STATUS_LOC_MASK_SET 0xf8

#define NUVIE_OBJ_STATUS_SCRIPTING 0x8
#define NUVIE_OBJ_STATUS_ACTOR_OBJ 0x10

#define OBJ_MATCH_QUALITY true
#define OBJ_NOMATCH_QUALITY false

#define OBJ_MATCH_FRAME_N true
#define OBJ_NOMATCH_FRAME_N false

//We use this in Obj::is_in_inventory()
#define OBJ_DONT_CHECK_PARENT false

class Obj {
	uint8 nuvie_status;

public:
	//uint16 objblk_n;

	uint16 obj_n;
	uint8 frame_n;
	uint8 status;
	uint16 x;
	uint16 y;
	uint8 z;

	uint16 qty;
	uint8 quality;
	void *parent;  //either an Obj pointer or an Actor pointer depending on engine_loc.
	U6LList *container;

public:
	Obj();
	Obj(Obj *sobj);

	bool is_script_obj()    {
		return (nuvie_status & NUVIE_OBJ_STATUS_SCRIPTING);
	}
	bool is_actor_obj()     {
		return (nuvie_status & NUVIE_OBJ_STATUS_ACTOR_OBJ);
	}

	bool is_ok_to_take();
	bool is_invisible()     {
		return (status & OBJ_STATUS_INVISIBLE);
	}
	bool is_charmed()     {
		return (status & OBJ_STATUS_CHARMED);
	}
	bool is_temporary()    {
		return (status & OBJ_STATUS_TEMPORARY);
	}
	bool is_egg_active()   {
		return (status & OBJ_STATUS_EGG_ACTIVE);
	}
	bool is_broken()       {
		return (status & OBJ_STATUS_BROKEN);
	}
	bool is_mutant()       {
		return (status & OBJ_STATUS_MUTANT);
	}
	bool is_cursed()       {
		return (status & OBJ_STATUS_CURSED);
	}
	bool is_lit()          {
		return (status & OBJ_STATUS_LIT);
	}

	bool is_on_map()       {
		return ((nuvie_status & NUVIE_OBJ_STATUS_LOC_MASK_GET) == OBJ_LOC_MAP);
	}
	bool is_in_container() {
		return ((nuvie_status & NUVIE_OBJ_STATUS_LOC_MASK_GET) == OBJ_LOC_CONT);
	}
	bool is_in_inventory(bool check_parent = true);

	bool is_readied()      {
		return ((nuvie_status & NUVIE_OBJ_STATUS_LOC_MASK_GET) == OBJ_LOC_READIED);
	}

	bool has_container() {
		return (container != NULL);
	}
	void make_container();
	Obj *get_container_obj(bool recursive = false);
	uint32 container_count_objects();

	uint8 get_engine_loc();
	Actor *get_actor_holding_obj();

	void set_on_map(U6LList *map_list);
	void set_in_container(Obj *container_obj);
	void set_in_inventory();

	void set_invisible(bool flag);

	void set_temporary(bool flag = true);

	void set_ok_to_take(bool flag, bool recursive = false);

	void readied();
	void set_noloc();
	void set_in_script(bool flag);
	void set_actor_obj(bool flag);

	void add(Obj *obj, bool stack = false);

	bool remove(Obj *obj);

	Obj *find_in_container(uint16 obj_n, uint8 quality, bool match_quality = OBJ_MATCH_QUALITY, uint8 frame_n = 0, bool match_frame_n = OBJ_NOMATCH_FRAME_N, Obj **prev_obj = NULL);

	uint32 get_total_qty(uint16 match_obj_n);

protected:

	void add_and_stack(Obj *obj);

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
