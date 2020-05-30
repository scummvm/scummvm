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

#include "glk/adrift/scare.h"
#include "glk/adrift/scprotos.h"
#include "glk/adrift/scgamest.h"

namespace Glk {
namespace Adrift {

/* Assorted definitions and constants. */
static const sc_char NUL = '\0';

/* Trace flag, set before running. */
static sc_bool obj_trace = FALSE;


/*
 * obj_is_static()
 * obj_is_surface()
 * obj_is_container()
 *
 * Convenience functions to return TRUE for given object attributes.
 */
sc_bool obj_is_static(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_bool bstatic;

	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Static";
	bstatic = prop_get_boolean(bundle, "B<-sis", vt_key);
	return bstatic;
}

sc_bool obj_is_container(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_bool is_container;

	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Container";
	is_container = prop_get_boolean(bundle, "B<-sis", vt_key);
	return is_container;
}

sc_bool obj_is_surface(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_bool is_surface;

	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Surface";
	is_surface = prop_get_boolean(bundle, "B<-sis", vt_key);
	return is_surface;
}


/*
 * obj_container_object()
 *
 * Return the index of the n'th container object found.
 */
sc_int obj_container_object(sc_gameref_t game, sc_int n) {
	sc_int object, count;

	/* Progress through objects until n containers found. */
	count = n;
	for (object = 0; object < gs_object_count(game) && count >= 0; object++) {
		if (obj_is_container(game, object))
			count--;
	}
	return object - 1;
}


/*
 * obj_container_index()
 *
 * Return index such that obj_container_object(index) == objnum.
 */
sc_int obj_container_index(sc_gameref_t game, sc_int objnum) {
	sc_int object, count;

	/* Progress through objects up to objnum. */
	count = 0;
	for (object = 0; object < objnum; object++) {
		if (obj_is_container(game, object))
			count++;
	}
	return count;
}


/*
 * obj_surface_object()
 *
 * Return the index of the n'th surface object found.
 */
sc_int obj_surface_object(sc_gameref_t game, sc_int n) {
	sc_int object, count;

	/* Progress through objects until n surfaces found. */
	count = n;
	for (object = 0; object < gs_object_count(game) && count >= 0; object++) {
		if (obj_is_surface(game, object))
			count--;
	}
	return object - 1;
}


/*
 * obj_surface_index()
 *
 * Return index such that obj_surface_object(index) == objnum.
 */
sc_int obj_surface_index(sc_gameref_t game, sc_int objnum) {
	sc_int object, count;

	/* Progress through objects up to objnum. */
	count = 0;
	for (object = 0; object < objnum; object++) {
		if (obj_is_surface(game, object))
			count++;
	}
	return count;
}


/*
 * obj_stateful_object()
 *
 * Return the index of the n'th openable or statussed object found.
 */
sc_int obj_stateful_object(sc_gameref_t game, sc_int n) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, count;

	/* Progress through objects until n matches found. */
	count = n;
	for (object = 0; object < gs_object_count(game) && count >= 0; object++) {
		sc_vartype_t vt_key[3];
		sc_bool is_openable, is_statussed;

		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Openable";
		is_openable = prop_get_integer(bundle, "I<-sis", vt_key) != 0;
		vt_key[2].string = "CurrentState";
		is_statussed = prop_get_integer(bundle, "I<-sis", vt_key) != 0;
		if (is_openable || is_statussed)
			count--;
	}
	return object - 1;
}


/*
 * obj_stateful_index()
 *
 * Return index such that obj_stateful_object(index) == objnum.
 */
sc_int obj_stateful_index(sc_gameref_t game, sc_int objnum) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, count;

	/* Progress through objects up to objnum. */
	count = 0;
	for (object = 0; object < objnum; object++) {
		sc_vartype_t vt_key[3];
		sc_bool is_openable, is_statussed;

		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Openable";
		is_openable = prop_get_integer(bundle, "I<-sis", vt_key) != 0;
		vt_key[2].string = "CurrentState";
		is_statussed = prop_get_integer(bundle, "I<-sis", vt_key) != 0;
		if (is_openable || is_statussed)
			count++;
	}
	return count;
}


/*
 * obj_state_name()
 *
 * Return the string name of the state of a given stateful object.  The
 * string is malloc'ed, and needs to be freed by the caller.  Returns NULL
 * if no valid state string found.
 */
sc_char *obj_state_name(sc_gameref_t game, sc_int objnum) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	const sc_char *states;
	sc_int length, state, count, first, last;
	sc_char *string;

	/* Get the list of state strings for the object. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = objnum;
	vt_key[2].string = "States";
	states = prop_get_string(bundle, "S<-sis", vt_key);

	/* Find the start of the element for the current state. */
	state = gs_object_state(game, objnum);
	length = strlen(states);
	for (first = 0, count = state; first < length && count > 1; first++) {
		if (states[first] == '|')
			count--;
	}
	if (count != 1)
		return NULL;

	/* Find the end of the state string. */
	for (last = first; last < length; last++) {
		if (states[last] == '|')
			break;
	}

	/* Allocate and take a copy of the state string. */
	string = (sc_char *)sc_malloc(last - first + 1);
	memcpy(string, states + first, last - first);
	string[last - first] = NUL;

	return string;
}


/*
 * obj_dynamic_object()
 *
 * Return the index of the n'th non-static object found.
 */
sc_int obj_dynamic_object(sc_gameref_t game, sc_int n) {
	sc_int object, count;

	/* Progress through objects until n matches found. */
	count = n;
	for (object = 0; object < gs_object_count(game) && count >= 0; object++) {
		if (!obj_is_static(game, object))
			count--;
	}
	return object - 1;
}


/*
 * obj_wearable_object()
 *
 * Return the index of the n'th wearable object found.
 */
sc_int obj_wearable_object(sc_gameref_t game, sc_int n) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, count;

	/* Progress through objects until n matches found. */
	count = n;
	for (object = 0; object < gs_object_count(game) && count >= 0; object++) {
		if (!obj_is_static(game, object)) {
			sc_vartype_t vt_key[3];

			vt_key[0].string = "Objects";
			vt_key[1].integer = object;
			vt_key[2].string = "Wearable";
			if (prop_get_boolean(bundle, "B<-sis", vt_key))
				count--;
		}
	}
	return object - 1;
}


/*
 * Size is held in the ten's digit of SizeWeight, and weight in the units.
 * Size and weight are multipliers -- the relative size and weight of objects
 * rises by a factor of three for each incremental multiplier.  These factors
 * are also used for the maximum size of object that can fit in a container,
 * and the number of these that fit.
 */
enum {
	OBJ_DIMENSION_DIVISOR = 10,
	OBJ_DIMENSION_MULTIPLE = 3
};

/*
 * obj_get_size()
 * obj_get_weight()
 *
 * Return the relative size and weight of an object.  For containers, the
 * weight includes the weight of each contained object.
 *
 * TODO It's possible to have static objects in the player inventory, moved
 * by events -- how should these be handled, as they have no SizeWeight?
 */
sc_int obj_get_size(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int size, count;

	/* TODO For now, give static objects no size. */
	if (obj_is_static(game, object))
		return 0;

	/* Size is the 'tens' component of SizeWeight. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "SizeWeight";
	count = prop_get_integer(bundle, "I<-sis", vt_key) / OBJ_DIMENSION_DIVISOR;

	/*
	 * Calculate base object size.  Unlike weights below, we take this as simply
	 * being the maximum size; that is, when a container carries other objects
	 * its weight increases by the sum of objects carried, but its size remains
	 * constant.
	 */
	size = 1;
	for (; count > 0; count--)
		size *= OBJ_DIMENSION_MULTIPLE;

	if (obj_trace)
		sc_trace("Object: object %ld is size %ld\n", object, size);

	/* Return total size. */
	return size;
}

sc_int obj_get_weight(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int weight, count;

	/* TODO For now, give static objects no weight. */
	if (obj_is_static(game, object))
		return 0;

	/* Weight is the 'units' component of SizeWeight. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "SizeWeight";
	count = prop_get_integer(bundle, "I<-sis", vt_key) % OBJ_DIMENSION_DIVISOR;

	/* Calculate base object weight. */
	weight = 1;
	for (; count > 0; count--)
		weight *= OBJ_DIMENSION_MULTIPLE;

	/* If this is a container or a surface, add weights of parented objects. */
	if (obj_is_container(game, object) || obj_is_surface(game, object)) {
		sc_int other;

		/* Find and add contained or surface objects. */
		for (other = 0; other < gs_object_count(game); other++) {
			if ((gs_object_position(game, other) == OBJ_IN_OBJECT
			        || gs_object_position(game, other) == OBJ_ON_OBJECT)
			        && gs_object_parent(game, other) == object) {
				weight += obj_get_weight(game, other);
			}
		}
	}

	if (obj_trace)
		sc_trace("Object: object %ld is weight %ld\n", object, weight);

	/* Return total weight. */
	return weight;
}


/*
 * obj_convert_player_limit()
 * obj_get_player_size_limit()
 * obj_get_player_weight_limit()
 *
 * Return the limits set on the sizes and weights a player can handle.  Not
 * really object-related except that they deal with sizing multiples.
 */
static sc_int obj_convert_player_limit(sc_int value) {
	sc_int retval, index_;

	/* 'Tens' of value multiplied by 3 to the power 'units' of value. */
	retval = value / OBJ_DIMENSION_DIVISOR;
	for (index_ = 0; index_ < value % OBJ_DIMENSION_DIVISOR; index_++)
		retval *= OBJ_DIMENSION_MULTIPLE;

	return retval;
}

sc_int obj_get_player_size_limit(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	sc_int max_size;

	vt_key[0].string = "Globals";
	vt_key[1].string = "MaxSize";
	max_size = prop_get_integer(bundle, "I<-ss", vt_key);

	return obj_convert_player_limit(max_size);
}

sc_int obj_get_player_weight_limit(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	sc_int max_weight;

	vt_key[0].string = "Globals";
	vt_key[1].string = "MaxWt";
	max_weight = prop_get_integer(bundle, "I<-ss", vt_key);

	return obj_convert_player_limit(max_weight);
}


/*
 * obj_get_container_maxsize()
 * obj_get_container_capacity()
 *
 * Return the maximum size of an object that can be placed in a container,
 * and the number that will fit.
 */
sc_int obj_get_container_maxsize(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int maxsize, count;

	/* Maxsize is found from the 'units' component of Capacity. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Capacity";
	count = prop_get_integer(bundle, "I<-sis", vt_key) % OBJ_DIMENSION_DIVISOR;

	/* Calculate and return maximum size. */
	maxsize = 1;
	for (; count > 0; count--)
		maxsize *= OBJ_DIMENSION_MULTIPLE;

	if (obj_trace)
		sc_trace("Object: object %ld has max size %ld\n", object, maxsize);

	return maxsize;
}

sc_int obj_get_container_capacity(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int capacity;

	/* The count of objects is in the 'tens' component of Capacity. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Capacity";
	capacity = prop_get_integer(bundle, "I<-sis", vt_key)
	           / OBJ_DIMENSION_DIVISOR;

	if (obj_trace)
		sc_trace("Object: object %ld has capacity %ld\n", object, capacity);

	return capacity;
}


/* Sit/lie bit mask enumerations. */
enum {
	OBJ_STANDABLE_MASK = 1 << 0,
	OBJ_LIEABLE_MASK = 1 << 1
};

/*
 * obj_standable_object()
 *
 * Return the index of the n'th standable object found.
 */
sc_int obj_standable_object(sc_gameref_t game, sc_int n) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, count;

	/* Progress through objects until n standable found. */
	count = n;
	for (object = 0; object < gs_object_count(game) && count >= 0; object++) {
		sc_vartype_t vt_key[3];
		sc_int sit_lie_flags;

		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "SitLie";
		sit_lie_flags = prop_get_integer(bundle, "I<-sis", vt_key);
		if (sit_lie_flags & OBJ_STANDABLE_MASK)
			count--;
	}
	return object - 1;
}


/*
 * obj_lieable_object()
 *
 * Return the index of the n'th lieable object found.
 */
sc_int obj_lieable_object(sc_gameref_t game, sc_int n) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, count;

	/* Progress through objects until n lieable found. */
	count = n;
	for (object = 0; object < gs_object_count(game) && count >= 0; object++) {
		sc_vartype_t vt_key[3];
		sc_int sit_lie_flags;

		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "SitLie";
		sit_lie_flags = prop_get_integer(bundle, "I<-sis", vt_key);
		if (sit_lie_flags & OBJ_LIEABLE_MASK)
			count--;
	}
	return object - 1;
}


/*
 * obj_appears_plural()
 *
 * Return TRUE if the object appears to be plural.  Adrift makes a guess at
 * this to produce "... is on.." or "... are on...".  It's not clear how it
 * does it, but it looks something like: singular if prefix is "a" or "an"
 * or ""; plural if prefix is "the" or "some" and short name ends with 's'
 * that is not preceded by 'u'.
 */
sc_bool obj_appears_plural(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	const sc_char *prefix, *name;

	/* Check prefix for "a", "an", or empty. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Prefix";
	prefix = prop_get_string(bundle, "S<-sis", vt_key);

	if (!(sc_strempty(prefix)
	        || sc_compare_word(prefix, "a", 1)
	        || sc_compare_word(prefix, "an", 2))) {
		sc_int length;

		/* Check name for ending in 's', but not 'us'. */
		vt_key[2].string = "Short";
		name = prop_get_string(bundle, "S<-sis", vt_key);
		length = strlen(name);

		if (!sc_strempty(name)
		        && sc_tolower(name[length - 1]) == 's'
		        && (length < 2 || sc_tolower(name[length - 2]) != 'u'))
			return TRUE;
	}

	/* Doesn't look plural. */
	return FALSE;
}


/*
 * obj_directly_in_room_internal()
 * obj_directly_in_room()
 *
 * Return TRUE if a given object is currently on the floor of a given room.
 */
static sc_bool obj_directly_in_room_internal(sc_gameref_t game, sc_int object, sc_int room) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);

	/* See if the object is static or dynamic. */
	if (obj_is_static(game, object)) {
		sc_vartype_t vt_key[5];
		sc_int type;

		/* Static object moved to player or room by event? */
		if (!gs_object_static_unmoved(game, object)) {
			if (gs_object_position(game, object) == OBJ_HELD_PLAYER)
				return FALSE;
			else
				return gs_object_position(game, object) - 1 == room;
		}

		/* Check and return the room list for the object. */
		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Where";
		vt_key[3].string = "Type";
		type = prop_get_integer(bundle, "I<-siss", vt_key);
		switch (type) {
		case ROOMLIST_ALL_ROOMS:
			return TRUE;
		case ROOMLIST_NO_ROOMS:
		case ROOMLIST_NPC_PART:
			return FALSE;

		case ROOMLIST_ONE_ROOM:
			vt_key[3].string = "Room";
			return prop_get_integer(bundle, "I<-siss", vt_key) == room + 1;

		case ROOMLIST_SOME_ROOMS:
			vt_key[3].string = "Rooms";
			vt_key[4].integer = room + 1;
			return prop_get_boolean(bundle, "B<-sissi", vt_key);

		default:
			sc_fatal("obj_directly_in_room_internal:"
			         " invalid type, %ld\n", type);
			return FALSE;
		}
	} else
		return gs_object_position(game, object) == room + 1;
}

sc_bool obj_directly_in_room(sc_gameref_t game, sc_int object, sc_int room) {
	sc_bool result;

	/* Check, trace result, and return. */
	result = obj_directly_in_room_internal(game, object, room);

	if (obj_trace) {
		sc_trace("Object: checking for object %ld directly in room %ld, %s\n",
		         object, room, result ? "true" : "false");
	}

	return result;
}


/*
 * obj_indirectly_in_room_internal()
 * obj_indirectly_in_room()
 *
 * Return TRUE if a given object is currently in a given room, either
 * directly, on an object indirectly, in an open object indirectly, or
 * carried by an NPC in the room.
 */
static sc_bool obj_indirectly_in_room_internal(sc_gameref_t game, sc_int object, sc_int room) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);

	/* See if the object is static or dynamic. */
	if (obj_is_static(game, object)) {
		sc_vartype_t vt_key[5];
		sc_int type;

		/* Static object moved to player or room by event? */
		if (!gs_object_static_unmoved(game, object)) {
			if (gs_object_position(game, object) == OBJ_HELD_PLAYER)
				return gs_player_in_room(game, room);
			else
				return gs_object_position(game, object) - 1 == room;
		}

		/* Check and return the room list for the object. */
		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Where";
		vt_key[3].string = "Type";
		type = prop_get_integer(bundle, "I<-siss", vt_key);
		switch (type) {
		case ROOMLIST_ALL_ROOMS:
			return TRUE;
		case ROOMLIST_NO_ROOMS:
			return FALSE;

		case ROOMLIST_ONE_ROOM:
			vt_key[3].string = "Room";
			return prop_get_integer(bundle, "I<-siss", vt_key) == room + 1;

		case ROOMLIST_SOME_ROOMS:
			vt_key[3].string = "Rooms";
			vt_key[4].integer = room + 1;
			return prop_get_boolean(bundle, "B<-sissi", vt_key);

		case ROOMLIST_NPC_PART: {
			sc_int npc;

			vt_key[2].string = "Parent";
			npc = prop_get_integer(bundle, "I<-sis", vt_key);
			if (npc == 0)
				return gs_player_in_room(game, room);
			else
				return npc_in_room(game, npc - 1, room);
		}

		default:
			sc_fatal("obj_indirectly_in_room_internal:"
			         " invalid type, %ld\n", type);
			return FALSE;
		}
	} else {
		sc_int parent, position;

		/* Get dynamic object's parent and position. */
		parent = gs_object_parent(game, object);
		position = gs_object_position(game, object);

		/* Decide depending on positioning. */
		switch (position) {
		case OBJ_HIDDEN:       /* Hidden. */
			return FALSE;

		case OBJ_HELD_PLAYER:  /* Held by player. */
		case OBJ_WORN_PLAYER:  /* Worn by player. */
			return gs_player_in_room(game, room);

		case OBJ_HELD_NPC:     /* Held by NPC. */
		case OBJ_WORN_NPC:     /* Worn by NPC. */
			return npc_in_room(game, parent, room);

		case OBJ_IN_OBJECT: {  /* In another object. */
			sc_int openness;

			openness = gs_object_openness(game, parent);
			switch (openness) {
			case OBJ_WONTCLOSE:
			case OBJ_OPEN:
				return obj_indirectly_in_room(game, parent, room);
			default:
				return FALSE;
			}
		}

		case OBJ_ON_OBJECT:    /* On another object. */
			return obj_indirectly_in_room(game, parent, room);

		default:               /* Within a room. */
			if (position > gs_room_count(game) + 1) {
				sc_error("sc_object_indirectly_in_room:"
				         " position out of bounds, %ld\n", position);
			}
			return position - 1 == room;
		}
	}
}

sc_bool obj_indirectly_in_room(sc_gameref_t game, sc_int object, sc_int Room) {
	sc_bool result;

	/* Check, trace result, and return. */
	result = obj_indirectly_in_room_internal(game, object, Room);

	if (obj_trace) {
		sc_trace("Object: checking for object %ld indirectly in room %ld, %s\n",
		         object, Room, result ? "true" : "false");
	}

	return result;
}


/*
 * obj_indirectly_held_by_player_internal()
 * obj_indirectly_held_by_player()
 *
 * Return TRUE if a given object is currently held by the player, either
 * directly, on an object indirectly, or in an open object indirectly.
 */
static sc_bool obj_indirectly_held_by_player_internal(sc_gameref_t game, sc_int object) {
	/* See if the object is static or dynamic. */
	if (obj_is_static(game, object)) {
		/* Static object moved to player or room by event? */
		if (!gs_object_static_unmoved(game, object)) {
			if (gs_object_position(game, object) == OBJ_HELD_PLAYER)
				return TRUE;
			else
				return FALSE;
		}

		/* An unmoved static object is not held by the player. */
		return FALSE;
	} else {
		sc_int parent, position;

		/* Get dynamic object's parent and position. */
		parent = gs_object_parent(game, object);
		position = gs_object_position(game, object);

		/* Decide depending on positioning. */
		switch (position) {
		case OBJ_HIDDEN:       /* Hidden. */
			return FALSE;

		case OBJ_HELD_PLAYER:  /* Held by player. */
		case OBJ_WORN_PLAYER:  /* Worn by player. */
			return TRUE;

		case OBJ_HELD_NPC:     /* Held by NPC. */
		case OBJ_WORN_NPC:     /* Worn by NPC. */
			return FALSE;

		case OBJ_IN_OBJECT: {  /* In another object. */
			sc_int openness;

			openness = gs_object_openness(game, parent);
			switch (openness) {
			case OBJ_WONTCLOSE:
			case OBJ_OPEN:
				return obj_indirectly_held_by_player(game, parent);
			default:
				return FALSE;
			}
		}

		case OBJ_ON_OBJECT:    /* On another object. */
			return obj_indirectly_held_by_player(game, parent);

		default:               /* Within a room. */
			return FALSE;
		}
	}
}

sc_bool obj_indirectly_held_by_player(sc_gameref_t game, sc_int object) {
	sc_bool result;

	/* Check, trace result, and return. */
	result = obj_indirectly_held_by_player_internal(game, object);

	if (obj_trace) {
		sc_trace("Object: checking for object %ld indirectly"
		         " held by player, %s\n", object, result ? "true" : "false");
	}

	return result;
}


/*
 * sc_obj_shows_initial_description()
 *
 * Return TRUE if this object should be listed as room content.
 */
sc_bool obj_shows_initial_description(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int onlywhennotmoved;

	/* Get only when moved property. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "OnlyWhenNotMoved";
	onlywhennotmoved = prop_get_integer(bundle, "I<-sis", vt_key);

	/* Combine this with game in mysterious ways. */
	switch (onlywhennotmoved) {
	case 0:
		return TRUE;

	case 1:
		return gs_object_unmoved(game, object);

	case 2: {
		sc_int initialposition;

		if (gs_object_unmoved(game, object))
			return TRUE;

		vt_key[2].string = "InitialPosition";
		initialposition = prop_get_integer(bundle, "I<-sis", vt_key) - 3;
		return gs_object_position(game, object) == initialposition;
	}

	default:
		break;
	}

	/* What you talkin' 'bout, Willis? */
	return FALSE;
}


/*
 * obj_turn_update()
 * obj_setup_initial()
 *
 * Set initial values for object states, and update after a turn.
 */
void obj_turn_update(sc_gameref_t game) {
	sc_int index_;

	/* Update object seen flag to current state. */
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		if (!gs_object_seen(game, index_)
		        && obj_indirectly_in_room(game, index_, gs_playerroom(game)))
			gs_set_object_seen(game, index_, TRUE);
	}
}

void obj_setup_initial(sc_gameref_t game) {
	/* Set initial seen states for objects. */
	obj_turn_update(game);
}


/*
 * obj_debug_trace()
 *
 * Set object tracing on/off.
 */
void obj_debug_trace(sc_bool flag) {
	obj_trace = flag;
}

} // End of namespace Adrift
} // End of namespace Glk
