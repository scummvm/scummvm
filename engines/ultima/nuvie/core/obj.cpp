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
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/u6_objects.h"

namespace Ultima {
namespace Nuvie {

Obj::Obj() {
	obj_n = 0;
	status = 0;
	nuvie_status = 0;
	frame_n = 0;
	qty = 0;
	quality = 0;
	parent = NULL;
	container = NULL;
	x = 0;
	y = 0;
	z = 0;
}

Obj::Obj(Obj *sobj) {
	memcpy(this, sobj, sizeof(Obj));

	parent = NULL;
	container = NULL;
}

void Obj::make_container() {
	if (container == NULL)
		container = new U6LList();

	return;
}

Obj *Obj::get_container_obj(bool recursive) {
	Obj *obj = (is_in_container() ? (Obj *)parent : NULL);

	if (recursive) {
		while (obj && obj->is_in_container())
			obj = (Obj *)obj->parent;
	}

	return obj;
}

void Obj::set_on_map(U6LList *map_list) {
	parent = map_list;
	nuvie_status &= NUVIE_OBJ_STATUS_LOC_MASK_SET;
	nuvie_status |= OBJ_LOC_MAP;

	return;
}

void Obj::set_in_container(Obj *container_obj) {
	parent = (void *)container_obj;
	nuvie_status &= NUVIE_OBJ_STATUS_LOC_MASK_SET;
	nuvie_status |= OBJ_LOC_CONT;

	return;
}

void Obj::set_invisible(bool flag) {
	if (flag)
		status |= OBJ_STATUS_INVISIBLE;
	else if (is_invisible())
		status ^= OBJ_STATUS_INVISIBLE;

	return;
}

void Obj::set_temporary(bool flag) {
	if (flag)
		status |= OBJ_STATUS_TEMPORARY;
	else if (is_temporary())
		status ^= OBJ_STATUS_TEMPORARY;

	return;
}

void Obj::set_ok_to_take(bool flag, bool recursive) {
	if (flag)
		status |= OBJ_STATUS_OK_TO_TAKE;
	else if (is_ok_to_take())
		status ^= OBJ_STATUS_OK_TO_TAKE;

	if (recursive && container) {
		for (U6Link *link = container->start(); link != NULL; link = link->next) {
			Obj *obj = (Obj *)link->data;
			obj->set_ok_to_take(flag, recursive);
		}
	}
}

void Obj::set_in_inventory() {
	nuvie_status &= NUVIE_OBJ_STATUS_LOC_MASK_SET;
	nuvie_status |= OBJ_LOC_INV;

	return;
}

void Obj::readied() { //set_readied() ??
	nuvie_status &= NUVIE_OBJ_STATUS_LOC_MASK_SET;
	nuvie_status |= OBJ_LOC_READIED;

	return;
}

void Obj::set_noloc() {
	parent = NULL;
	nuvie_status &= NUVIE_OBJ_STATUS_LOC_MASK_SET; //clear location bits 0 = no loc

	return;
}

void Obj::set_in_script(bool flag) {
	if (flag)
		nuvie_status |= NUVIE_OBJ_STATUS_SCRIPTING;
	else if (is_script_obj())
		nuvie_status ^= NUVIE_OBJ_STATUS_SCRIPTING;

	return;
}

void Obj::set_actor_obj(bool flag) {
	if (flag)
		nuvie_status |= NUVIE_OBJ_STATUS_ACTOR_OBJ;
	else if (is_actor_obj())
		nuvie_status ^= NUVIE_OBJ_STATUS_ACTOR_OBJ;

	return;
}

/* Returns true if an object is in an actor inventory, including containers and readied items. */

bool Obj::is_in_inventory(bool check_parent) {
	switch (get_engine_loc()) {
	case OBJ_LOC_INV :
	case OBJ_LOC_READIED :
		return true;
	case OBJ_LOC_CONT :
		if (check_parent)
			return ((Obj *)parent)->is_in_inventory(check_parent);
		break;
	default :
		break;
	}

	return false;
}

uint8 Obj::get_engine_loc() {
	return (nuvie_status & NUVIE_OBJ_STATUS_LOC_MASK_GET);
}

Actor *Obj::get_actor_holding_obj() {
	switch (get_engine_loc()) {
	case OBJ_LOC_INV :
	case OBJ_LOC_READIED :
		return (Actor *)this->parent;

	case OBJ_LOC_CONT :
		return ((Obj *)parent)->get_actor_holding_obj();

	default :
		break;
	}

	return NULL;
}

//Add child object into container, stacking if required
void Obj::add(Obj *obj, bool stack) {
	if (container == NULL)
		make_container();

	if (stack && Game::get_game()->get_obj_manager()->is_stackable(obj))
		add_and_stack(obj);
	else
		container->addAtPos(0, obj);

	obj->set_in_container(this);

	return;
}

void Obj::add_and_stack(Obj *obj) {
	U6Link *link;
	Obj *cont_obj;

	//should we recurse through nested containers?
	for (link = container->start(); link != NULL;) {
		cont_obj = (Obj *)link->data;
		link = link->next;
		//match on obj_n, frame_n and quality.
		if (obj->obj_n == cont_obj->obj_n && obj->frame_n == cont_obj->frame_n && obj->quality == cont_obj->quality) {
			obj->qty += cont_obj->qty;
			container->replace(cont_obj, obj); //replace cont_obj with obj in container list. should we do this to link->data directly?
			delete_obj(cont_obj);

			return;
		}
	}

	container->addAtPos(0, obj); // add the object as we couldn't find another object to stack with.

	return;
}

//Remove child object from container.
bool Obj::remove(Obj *obj) {
	if (container == NULL)
		return false;

	if (container->remove(obj) == false)
		return false;
	if (Game::get_game()->get_game_type() == NUVIE_GAME_SE) {
		if (obj_n == OBJ_SE_JAR)
			frame_n = 0; // empty jar frame
	}
	obj->x = 0;
	obj->y = 0;
	obj->z = 0;

	obj->set_noloc();

	return true;
}

Obj *Obj::find_in_container(uint16 objN, uint8 quality_, bool match_quality, uint8 frameN, bool match_frame_n, Obj **prev_obj) {
	U6Link *link;
	Obj *obj;

	if (container == NULL)
		return NULL;

	for (link = container->start(); link != NULL; link = link->next) {
		obj = (Obj *)link->data;
		if (obj) {
			if (obj->obj_n == objN && (match_quality == false || obj->quality == quality_) && (match_frame_n == false || obj->frame_n == frameN)) {
				if (prev_obj != NULL && obj == *prev_obj)
					prev_obj = NULL;
				else {
					if (prev_obj == NULL || *prev_obj == NULL)
						return obj;
				}
			}

			if (obj->container) {
				obj = obj->find_in_container(objN, quality_, match_quality, frameN, match_frame_n, prev_obj);
				if (obj)
					return obj;
			}
		}
	}

	return NULL;
}

uint32 Obj::get_total_qty(uint16 match_obj_n) {
	U6Link *link;
	Obj *obj;
	uint16 total_qty = 0;

	if (obj_n == match_obj_n) {
		if (qty == 0)
			total_qty += 1;
		else
			total_qty += qty;
	}

	if (container != NULL) {
		for (link = container->start(); link != NULL; link = link->next) {
			obj = (Obj *)link->data;
			if (obj) {
				if (obj->container)
					total_qty += obj->get_total_qty(match_obj_n);
				else if (obj->obj_n == match_obj_n) {
					if (obj->qty == 0)
						total_qty += 1;
					else
						total_qty += obj->qty;
				}
			}
		}
	}

	return total_qty;
}

uint32 Obj::container_count_objects() {
	uint32 count = 0;
	U6Link *link;

	if (container != NULL) {
		for (link = container->start(); link != NULL; link = link->next) {
			++count;
		}
	}
	return count;
}

bool Obj::is_ok_to_take() {
	return ((status & OBJ_STATUS_OK_TO_TAKE) || Game::get_game()->using_hackmove());
}

} // End of namespace Nuvie
} // End of namespace Ultima
