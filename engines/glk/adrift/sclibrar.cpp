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

#include "glk/adrift/adrift.h"
#include "glk/adrift/scprotos.h"
#include "glk/adrift/scgamest.h"
#include "glk/adrift/serialization.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o Ensure module messages precisely match the real Runner ones.  This
 *   matters for ALRs.
 *
 * o Capacity checks on the player and on containers are implemented, but
 *   may not be right.
 */

/* Assorted definitions and constants. */
static const sc_char NUL = '\0';
static const sc_char COMMA = ',';
enum {
	SECS_PER_MINUTE = 60,
	MINS_PER_HOUR = 60,
	SECS_PER_HOUR = 3600
};
enum { LIB_ALLOCATION_AVOIDANCE_SIZE = 128 };

/* Trace flag, set before running. */
static sc_bool lib_trace = FALSE;


/*
 * lib_warn_battle_system()
 *
 * Display a warning when the battle system is detected in a game.  Print
 * directly rather than using the printfilter to avoid possible clashes
 * with ALRs.
 */
void lib_warn_battle_system(void) {
	if_print_tag(SC_TAG_FONT, "size=16");
	if_print_string("SCARE WARNING");
	if_print_tag(SC_TAG_ENDFONT, "");

	if_print_string(
	    "\n\nThe game uses Adrift's Battle System, something not fully supported"
	    " by this release of SCARE.\n\n");

	if_print_string(
	    "SCARE will still run the game, but it will not create character"
	    " battles where they would normally occur.  For some games, this may"
	    " be perfectly okay, as the Battle System is sometimes turned on"
	    " by accident in a game, but never actually used.  For others, though,"
	    " the omission of this feature may be more serious.\n\n");

	if_print_string("Please press a key to continue...\n\n");
	if_print_tag(SC_TAG_WAITKEY, "");
}


/*
 * lib_random_roomgroup_member()
 *
 * Return a random member of a roomgroup.
 */
sc_int lib_random_roomgroup_member(sc_gameref_t game, sc_int roomgroup) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_int count, room;

	/* Get the count of rooms in the group. */
	vt_key[0].string = "RoomGroups";
	vt_key[1].integer = roomgroup;
	vt_key[2].string = "List2";
	count = prop_get_child_count(bundle, "I<-sis", vt_key);
	if (count == 0) {
		sc_fatal("lib_random_roomgroup_member:"
		         " no rooms in group %ld\n", roomgroup);
	}

	/* Pick a room at random and return it. */
	vt_key[3].integer = sc_randomint(0, count - 1);
	room = prop_get_integer(bundle, "I<-sisi", vt_key);

	if (lib_trace) {
		sc_trace("Library: random room for group %ld is %ld\n",
		         roomgroup, room);
	}

	return room;
}


/*
 * lib_use_room_alt()
 *
 * Return TRUE if a particular alternate room description should be used.
 */
static sc_bool lib_use_room_alt(sc_gameref_t game, sc_int room, sc_int alt) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int type;
	sc_bool retval;

	/* Get alternate type. */
	vt_key[0].string = "Rooms";
	vt_key[1].integer = room;
	vt_key[2].string = "Alts";
	vt_key[3].integer = alt;
	vt_key[4].string = "Type";
	type = prop_get_integer(bundle, "I<-sisis", vt_key);

	/* Select based on type. */
	retval = FALSE;
	switch (type) {
	case 0: {                  /* Task. */
		sc_int var2, var3;

		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		if (var2 == 0)          /* No task. */
			retval = TRUE;
		else {
			vt_key[4].string = "Var3";
			var3 = prop_get_integer(bundle, "I<-sisis", vt_key);

			retval = gs_task_done(game, var2 - 1) == !(var3 != 0);
		}
		break;
	}

	case 1: {                  /* Stateful object. */
		sc_int var2, var3, object;

		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		if (var2 == 0)          /* No object. */
			retval = TRUE;
		else {
			vt_key[4].string = "Var3";
			var3 = prop_get_integer(bundle, "I<-sisis", vt_key);

			object = obj_stateful_index(game, var2 - 1);
			retval = restr_pass_task_object_state(game, object + 1, var3 - 1);
		}
		break;
	}

	case 2: {                  /* Player condition. */
		sc_int var2, var3, object;

		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var3";
		var3 = prop_get_integer(bundle, "I<-sisis", vt_key);

		if (var3 == 0) {
			switch (var2) {
			case 0:
			case 2:
			case 5:
				retval = TRUE;
				break;
			case 1:
			case 3:
			case 4:
				retval = FALSE;
				break;
			default:
				sc_fatal("lib_use_room_alt:"
				         " invalid player condition, %ld\n", var2);
			}
			break;
		}

		if (var2 == 2 || var2 == 3)
			object = obj_wearable_object(game, var3 - 1);
		else
			object = obj_dynamic_object(game, var3 - 1);

		switch (var2) {
		case 0:              /* Isn't holding (or wearing). */
			retval = gs_object_position(game, object) != OBJ_HELD_PLAYER
			         && gs_object_position(game, object) != OBJ_WORN_PLAYER;
			break;
		case 1:              /* Is holding (or wearing). */
			retval = gs_object_position(game, object) == OBJ_HELD_PLAYER
			         || gs_object_position(game, object) == OBJ_WORN_PLAYER;
			break;
		case 2:              /* Isn't wearing. */
			retval = gs_object_position(game, object) != OBJ_WORN_PLAYER;
			break;
		case 3:              /* Is wearing. */
			retval = gs_object_position(game, object) == OBJ_WORN_PLAYER;
			break;
		case 4:              /* Isn't in the same room as. */
			retval = !obj_indirectly_in_room(game,
			                                 object, gs_playerroom(game));
			break;
		case 5:              /* Is in the same room as. */
			retval = obj_indirectly_in_room(game,
			                                object, gs_playerroom(game));
			break;
		default:
			sc_fatal("lib_use_room_alt:"
			         " invalid player condition, %ld\n", var2);
		}
		break;
	}

	default:
		sc_fatal("lib_use_room_alt: invalid type, %ld\n", type);
	}

	return retval;
}


/*
 * lib_find_starting_alt()
 *
 * Return the alt index for the alt at which we need to start running down
 * the alts list when generating room names or descriptions.  Returns -1 if
 * no alt overrides the default room long description.
 */
static sc_int lib_find_starting_alt(sc_gameref_t game, sc_int room) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int alt_count, alt, retval;

	/* Get count of room alternates. */
	vt_key[0].string = "Rooms";
	vt_key[1].integer = room;
	vt_key[2].string = "Alts";
	alt_count = prop_get_child_count(bundle, "I<-sis", vt_key);

	/* Search backwards for a method-0 or method-1 overriding description. */
	retval = -1;
	for (alt = alt_count - 1; alt >= 0; alt--) {
		sc_int method;

		vt_key[3].integer = alt;
		vt_key[4].string = "DisplayRoom";
		method = prop_get_integer(bundle, "I<-sisis", vt_key);

		if (!(method == 0 || method == 1))
			continue;

		if (lib_use_room_alt(game, room, alt)) {
			const sc_char *m1;

			vt_key[3].integer = alt;
			vt_key[4].string = "M1";
			m1 = prop_get_string(bundle, "S<-sisis", vt_key);
			if (!sc_strempty(m1)) {
				retval = alt;
				break;
			}
		} else {
			const sc_char *m2;

			vt_key[3].integer = alt;
			vt_key[4].string = "M2";
			m2 = prop_get_string(bundle, "S<-sisis", vt_key);
			if (!sc_strempty(m2)) {
				retval = alt;
				break;
			}
		}
	}

	/* Return the index of the base alt, or -1 if none found. */
	return retval;
}


/*
 * lib_get_room_name()
 * lib_print_room_name()
 *
 * Get/print out the name for a given room.
 */
const sc_char *lib_get_room_name(sc_gameref_t game, sc_int room) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int alt_count, alt, start;
	const sc_char *name;

	/* Get the basic room name, and the count of room alternates. */
	vt_key[0].string = "Rooms";
	vt_key[1].integer = room;
	vt_key[2].string = "Short";
	name = prop_get_string(bundle, "S<-sis", vt_key);

	vt_key[2].string = "Alts";
	alt_count = prop_get_child_count(bundle, "I<-sis", vt_key);

	/* Get our starting point in the alts list. */
	start = lib_find_starting_alt(game, room);

	/*
	 * Run forwards through all alts lower than our starting point, or all alts
	 * if no starting point found.
	 */
	for (alt = (start != -1) ? start : 0; alt < alt_count; alt++) {
		/* Ignore all non-method-2 alts except for the starter. */
		if (alt != start) {
			sc_int method;

			vt_key[3].integer = alt;
			vt_key[4].string = "DisplayRoom";
			method = prop_get_integer(bundle, "I<-sisis", vt_key);

			if (method != 2)
				continue;
		}

		/* If this alt offers a name change, note it and continue. */
		if (lib_use_room_alt(game, room, alt)) {
			const sc_char *changed;

			vt_key[3].integer = alt;
			vt_key[4].string = "Changed";
			changed = prop_get_string(bundle, "S<-sisis", vt_key);
			if (!sc_strempty(changed))
				name = changed;
		}
	}

	/* Return the final selected name. */
	return name;
}

void lib_print_room_name(sc_gameref_t game, sc_int room) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_char *name;

	/* Print the room name, possibly in bold. */
	name = lib_get_room_name(game, room);
	if (game->bold_room_names) {
		pf_buffer_tag(filter, SC_TAG_BOLD);
		pf_buffer_string(filter, name);
		pf_buffer_tag(filter, SC_TAG_ENDBOLD);
	} else
		pf_buffer_string(filter, name);
	pf_buffer_character(filter, '\n');
}


/*
 * lib_print_object_np
 * lib_print_object
 *
 * Convenience functions to print out an object's name, with a "normalized"
 * prefix -- any "a"/"an"/"some" is replaced by "the" -- and with the full
 * prefix.
 */
static void lib_print_object_np(sc_gameref_t game, sc_int object) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	const sc_char *prefix, *normalized, *name;

	/* Get the object's prefix. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Prefix";
	prefix = prop_get_string(bundle, "S<-sis", vt_key);

	/*
	 * Normalize by skipping any leading "a"/"an"/"some", replacing it instead
	 * with "the", and skipping any odd "the" already present.  If no prefix at
	 * all, add a "the " anyway.
	 *
	 * TODO This is empirical, based on observed Adrift Runner behavior, and
	 * what it's _really_ supposed to do is a mystery.  This routine has been a
	 * real PITA.
	 */
	normalized = prefix;
	if (sc_compare_word(prefix, "a", 1)) {
		normalized = prefix + 1;
		pf_buffer_string(filter, "the");
	} else if (sc_compare_word(prefix, "an", 2)) {
		normalized = prefix + 2;
		pf_buffer_string(filter, "the");
	} else if (sc_compare_word(prefix, "the", 3)) {
		normalized = prefix + 3;
		pf_buffer_string(filter, "the");
	} else if (sc_compare_word(prefix, "some", 4)) {
		normalized = prefix + 4;
		pf_buffer_string(filter, "the");
	} else if (sc_strempty(prefix))
		pf_buffer_string(filter, "the ");

	/*
	 * If the remaining normalized prefix isn't empty, print it, and a space.
	 * If it is, then consider adding a space to any "the" printed above, except
	 * for the one done for empty prefixes, that is.
	 */
	if (!sc_strempty(normalized)) {
		pf_buffer_string(filter, normalized);
		pf_buffer_character(filter, ' ');
	} else if (normalized > prefix)
		pf_buffer_character(filter, ' ');

	/*
	 * Print the object's name; here we also look for a leading article and
	 * strip if found -- some games may avoid prefix and do this instead.
	 */
	vt_key[2].string = "Short";
	name = prop_get_string(bundle, "S<-sis", vt_key);
	if (sc_compare_word(name, "a", 1))
		name += 1;
	else if (sc_compare_word(name, "an", 2))
		name += 2;
	else if (sc_compare_word(name, "the", 3))
		name += 3;
	else if (sc_compare_word(name, "some", 4))
		name += 4;
	pf_buffer_string(filter, name);
}

static void lib_print_object(sc_gameref_t game, sc_int object) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	const sc_char *prefix, *name;

	/*
	 * Get the object's prefix, and print if not empty, otherwise default to an
	 * "a " prefix, as that's what Adrift seems to do.
	 */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Prefix";
	prefix = prop_get_string(bundle, "S<-sis", vt_key);
	if (!sc_strempty(prefix)) {
		pf_buffer_string(filter, prefix);
		pf_buffer_character(filter, ' ');
	} else
		pf_buffer_string(filter, "a ");

	/* Print object name. */
	vt_key[2].string = "Short";
	name = prop_get_string(bundle, "S<-sis", vt_key);
	pf_buffer_string(filter, name);
}


/*
 * lib_print_npc_np
 * lib_print_npc
 *
 * Convenience functions to print out an NPC's name, with and without
 * any prefix.
 */
static void lib_print_npc_np(sc_gameref_t game, sc_int npc) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	const sc_char *name;

	/* Get the NPC's short description, and print it. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Name";
	name = prop_get_string(bundle, "S<-sis", vt_key);

	pf_buffer_string(filter, name);
}

#if 0
static void lib_print_npc(sc_gameref_t game, sc_int npc) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	const sc_char *prefix;

	/* Get the NPC's prefix. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Prefix";
	prefix = prop_get_string(bundle, "S<-sis", vt_key);

	/* If the prefix isn't empty, print it, then print NPC name. */
	if (!sc_strempty(prefix)) {
		pf_buffer_string(filter, prefix);
		pf_buffer_character(filter, ' ');
	}
	lib_print_npc_np(game, npc);
}
#endif


/*
 * lib_select_response()
 * lib_select_plurality()
 *
 * Convenience functions for multiple handlers.  Returns the appropriate
 * response string for a game, based on perspective or object plurality.
 */
static const sc_char *lib_select_response(sc_gameref_t game,
		const sc_char *second_person, const sc_char *first_person, const sc_char *third_person) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	sc_int perspective;
	const sc_char *response;

	/* Return the response appropriate for Perspective. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "Perspective";
	perspective = prop_get_integer(bundle, "I<-ss", vt_key);
	switch (perspective) {
	case LIB_FIRST_PERSON:
		response = first_person;
		break;
	case LIB_SECOND_PERSON:
		response = second_person;
		break;
	case LIB_THIRD_PERSON:
		response = third_person;
		break;
	default:
		sc_error("lib_select_response:"
		         " unknown perspective, %ld\n", perspective);
		response = second_person;
		break;
	}

	return response;
}

static const sc_char *lib_select_plurality(sc_gameref_t game, sc_int object,
		const sc_char *singular, const sc_char *plural) {
	return obj_appears_plural(game, object) ? plural : singular;
}


/*
 * lib_get_npc_inroom_text()
 *
 * Returns the inroom description to be use for an NPC; if the NPC has
 * gone walkabout and offers a changed description, return that; otherwise
 * return the standard inroom text.
 */
static const sc_char *lib_get_npc_inroom_text(sc_gameref_t game, sc_int npc) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int walk_count, walk;
	const sc_char *inroomtext;

	/* Get the count of NPC walks. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Walks";
	walk_count = prop_get_child_count(bundle, "I<-sis", vt_key);

	/* Check for any active walk with a description, return if found. */
	for (walk = walk_count - 1; walk >= 0; walk--) {
		if (gs_npc_walkstep(game, npc, walk) > 0) {
			const sc_char *changeddesc;

			/* Get and check any walk active description. */
			vt_key[3].integer = walk;
			vt_key[4].string = "ChangedDesc";
			changeddesc = prop_get_string(bundle, "S<-sisis", vt_key);
			if (!sc_strempty(changeddesc))
				return changeddesc;
		}
	}

	/* Return the standard inroom text. */
	vt_key[2].string = "InRoomText";
	inroomtext = prop_get_string(bundle, "S<-sis", vt_key);
	return inroomtext;
}


/*
 * lib_print_room_contents()
 *
 * Print a list of the contents of a room.
 */
static void lib_print_room_contents(sc_gameref_t game, sc_int room) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_int object, npc, count, trail;

	/* List all objects that show their initial description. */
	count = 0;
	for (object = 0; object < gs_object_count(game); object++) {
		if (obj_directly_in_room(game, object, room)
		        && obj_shows_initial_description(game, object)) {
			const sc_char *inroomdesc;

			/* Find and print in room description. */
			vt_key[0].string = "Objects";
			vt_key[1].integer = object;
			vt_key[2].string = "InRoomDesc";
			inroomdesc = prop_get_string(bundle, "S<-sis", vt_key);
			if (!sc_strempty(inroomdesc)) {
				if (count == 0)
					pf_buffer_character(filter, '\n');
				else
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter, inroomdesc);
				count++;
			}
		}
	}
	if (count > 0)
		pf_buffer_character(filter, '\n');

	/*
	 * List dynamic objects directly located in the room, and not already listed
	 * above since they lack, or suppress, an in room description.
	 *
	 * If an object sets ListFlag, then if dynamic it's suppressed from the list
	 * where it would normally be included, but if static it's included where it
	 * would normally be excluded.
	 */
	count = 0;
	trail = -1;
	for (object = 0; object < gs_object_count(game); object++) {
		if (obj_directly_in_room(game, object, room)) {
			const sc_char *inroomdesc;

			vt_key[0].string = "Objects";
			vt_key[1].integer = object;
			vt_key[2].string = "InRoomDesc";
			inroomdesc = prop_get_string(bundle, "S<-sis", vt_key);

			if (!obj_shows_initial_description(game, object)
			        || sc_strempty(inroomdesc)) {
				sc_bool listflag;

				vt_key[2].string = "ListFlag";
				listflag = prop_get_boolean(bundle, "B<-sis", vt_key);

				if (listflag == obj_is_static(game, object)) {
					if (count > 0) {
						if (count == 1)
							pf_buffer_string(filter,
							                 lib_select_plurality(game, trail,
							                                      "\nAlso here is ",
							                                      "\nAlso here are "));
						else
							pf_buffer_string(filter, ", ");
						lib_print_object(game, trail);
					}
					trail = object;
					count++;
				}
			}
		}
	}
	if (count >= 1) {
		if (count == 1)
			pf_buffer_string(filter,
			                 lib_select_plurality(game, trail,
			                                      "\nAlso here is ",
			                                      "\nAlso here are "));
		else
			pf_buffer_string(filter, " and ");
		lib_print_object(game, trail);
		pf_buffer_string(filter, ".\n");
	}

	/* List NPCs directly in the room that have an in room description. */
	count = 0;
	for (npc = 0; npc < gs_npc_count(game); npc++) {
		if (npc_in_room(game, npc, room)) {
			const sc_char *description;

			/* Print any non='#' in-room description. */
			description = lib_get_npc_inroom_text(game, npc);
			if (!sc_strempty(description) && sc_strcasecmp(description, "#")) {
				if (count == 0)
					pf_buffer_character(filter, '\n');
				else
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter, description);
				count++;
			}
		}
	}
	if (count > 0)
		pf_buffer_character(filter, '\n');

	/*
	 * List NPCs in the room that don't have an in room description and that
	 * request a default "...is here" with "#".
	 *
	 * TODO Is this right?
	 */
	count = 0;
	trail = -1;
	for (npc = 0; npc < gs_npc_count(game); npc++) {
		if (npc_in_room(game, npc, room)) {
			const sc_char *description;

			/* Print name for descriptions marked '#'. */
			description = lib_get_npc_inroom_text(game, npc);
			if (!sc_strempty(description) && !sc_strcasecmp(description, "#")) {
				if (count > 0) {
					if (count > 1)
						pf_buffer_string(filter, ", ");
					else {
						pf_buffer_character(filter, '\n');
						pf_new_sentence(filter);
					}
					lib_print_npc_np(game, trail);
				}
				trail = npc;
				count++;
			}
		}
	}
	if (count >= 1) {
		if (count == 1) {
			pf_buffer_character(filter, '\n');
			pf_new_sentence(filter);
			lib_print_npc_np(game, trail);
			pf_buffer_string(filter, " is here");
		} else {
			pf_buffer_string(filter, " and ");
			lib_print_npc_np(game, trail);
			pf_buffer_string(filter, " are here");
		}
		pf_buffer_string(filter, ".\n");
	}
}


/*
 * lib_print_room_description()
 *
 * Print out the long description for a given room.
 */
void lib_print_room_description(sc_gameref_t game, sc_int room) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_bool showobjects, is_described, is_suppressed;
	sc_int alt_count, alt, start, event;

	/* Get count of room alternates. */
	vt_key[0].string = "Rooms";
	vt_key[1].integer = room;
	vt_key[2].string = "Alts";
	alt_count = prop_get_child_count(bundle, "I<-sis", vt_key);

	/* Start with no description, and get our starting point in the alts list. */
	is_described = FALSE;
	start = lib_find_starting_alt(game, room);

	/* Print the standard description unless a start alt indicates not. */
	if (start == -1)
		is_suppressed = FALSE;
	else {
		sc_int method;

		vt_key[3].integer = start;
		vt_key[4].string = "DisplayRoom";
		method = prop_get_integer(bundle, "I<-sisis", vt_key);

		is_suppressed = (method == 0);
	}
	if (!is_suppressed) {
		const sc_char *description;

		vt_key[0].string = "Rooms";
		vt_key[1].integer = room;
		vt_key[2].string = "Long";
		description = prop_get_string(bundle, "S<-sis", vt_key);
		if (!sc_strempty(description)) {
			pf_buffer_string(filter, description);
			is_described = TRUE;
		}

		vt_key[2].string = "Res";
		res_handle_resource(game, "sis", vt_key);
	}

	/* Ensure that we're back to handling room alts. */
	vt_key[0].string = "Rooms";
	vt_key[1].integer = room;
	vt_key[2].string = "Alts";

	/*
	 * Run forwards through all alts lower than our starting point, or all alts
	 * if no starting point overrider found.
	 */
	showobjects = TRUE;
	for (alt = (start != -1) ? start : 0; alt < alt_count; alt++) {
		/* Ignore all non-method-2 alts except for the starter. */
		if (alt != start) {
			sc_int method;

			vt_key[3].integer = alt;
			vt_key[4].string = "DisplayRoom";
			method = prop_get_integer(bundle, "I<-sisis", vt_key);

			if (method != 2)
				continue;
		}

		if (lib_use_room_alt(game, room, alt)) {
			const sc_char *m1;
			sc_int hideobjects;

			vt_key[3].integer = alt;
			vt_key[4].string = "M1";
			m1 = prop_get_string(bundle, "S<-sisis", vt_key);
			if (!sc_strempty(m1)) {
				if (is_described)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter, m1);
				is_described = TRUE;
			}

			vt_key[4].string = "Res1";
			res_handle_resource(game, "sisis", vt_key);

			vt_key[4].string = "HideObjects";
			hideobjects = prop_get_integer(bundle, "I<-sisis", vt_key);
			if (hideobjects == 1)
				showobjects = FALSE;
		} else {
			const sc_char *m2;

			vt_key[3].integer = alt;
			vt_key[4].string = "M2";
			m2 = prop_get_string(bundle, "S<-sisis", vt_key);
			if (!sc_strempty(m2)) {
				if (is_described)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter, m2);
				is_described = TRUE;
			}

			vt_key[4].string = "Res2";
			res_handle_resource(game, "sisis", vt_key);
		}
	}

	/* Print out any relevant event look text. */
	for (event = 0; event < gs_event_count(game); event++) {
		if (gs_event_state(game, event) == ES_RUNNING
		        && evt_can_see_event(game, event)) {
			const sc_char *looktext;

			vt_key[0].string = "Events";
			vt_key[1].integer = event;
			vt_key[2].string = "LookText";
			looktext = prop_get_string(bundle, "S<-sis", vt_key);
			if (is_described)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter, looktext);
			is_described = TRUE;

			vt_key[2].string = "Res";
			vt_key[3].integer = 1;
			res_handle_resource(game, "sisi", vt_key);
		}
	}
	if (is_described)
		pf_buffer_character(filter, '\n');

	/* Finally, print room contents. */
	if (showobjects)
		lib_print_room_contents(game, room);
}


/*
 * lib_can_go()
 *
 * Return TRUE if the player can move in the given direction.
 */
static sc_bool lib_can_go(sc_gameref_t game, sc_int room, sc_int direction) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int restriction;
	sc_bool is_restricted = FALSE;

	/* Set up invariant parts of key. */
	vt_key[0].string = "Rooms";
	vt_key[1].integer = room;
	vt_key[2].string = "Exits";
	vt_key[3].integer = direction;

	/* Check for any movement restrictions. */
	vt_key[4].string = "Var1";
	restriction = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;
	if (restriction >= 0) {
		sc_int type;

		if (lib_trace)
			sc_trace("Library: hit move restriction\n");

		/* Get restriction type. */
		vt_key[4].string = "Var3";
		type = prop_get_integer(bundle, "I<-sisis", vt_key);
		switch (type) {
		case 0: {              /* Task type restriction */
			sc_int check;

			/* Get the expected completion state. */
			vt_key[4].string = "Var2";
			check = prop_get_integer(bundle, "I<-sisis", vt_key);

			if (lib_trace) {
				sc_trace("Library: task %ld, check %ld\n",
				         restriction, check);
			}

			/* Restrict if task isn't done/not done as expected. */
			if ((check != 0) == gs_task_done(game, restriction))
				is_restricted = TRUE;
			break;
		}

		case 1: {              /* Object state restriction */
			sc_int object, check, openable;

			/* Get the target object. */
			object = obj_stateful_object(game, restriction);

			/* Get the expected object state. */
			vt_key[4].string = "Var2";
			check = prop_get_integer(bundle, "I<-sisis", vt_key);

			if (lib_trace)
				sc_trace("Library: object %ld, check %ld\n", object, check);

			/* Check openable and lockable objects. */
			vt_key[0].string = "Objects";
			vt_key[1].integer = object;
			vt_key[2].string = "Openable";
			openable = prop_get_integer(bundle, "I<-sis", vt_key);
			if (openable > 0) {
				sc_int lockable;

				/* See if lockable. */
				vt_key[2].string = "Key";
				lockable = prop_get_integer(bundle, "I<-sis", vt_key);
				if (lockable >= 0) {
					/* Lockable. */
					if (check <= 2) {
						if (gs_object_openness(game, object) != check + 5)
							is_restricted = TRUE;
					} else {
						if (gs_object_state(game, object) != check - 2)
							is_restricted = TRUE;
					}
				} else {
					/* Not lockable, though openable. */
					if (check <= 1) {
						if (gs_object_openness(game, object) != check + 5)
							is_restricted = TRUE;
					} else {
						if (gs_object_state(game, object) != check - 1)
							is_restricted = TRUE;
					}
				}
			} else {
				/* Not openable. */
				if (gs_object_state(game, object) != check + 1)
					is_restricted = TRUE;
			}
			break;
		}

		default:
			break;
		}
	}

	/* Return TRUE if not restricted. */
	return !is_restricted;
}


/* List of direction names, for printing and counting exits. */
static const sc_char *const DIRNAMES_4[] = {
	"north", "east", "south", "west", "up", "down", "in", "out",
	NULL
};
static const sc_char *const DIRNAMES_8[] = {
	"north", "east", "south", "west", "up", "down", "in", "out",
	"northeast", "southeast", "southwest", "northwest",
	NULL
};


/*
 * lib_cmd_print_room_exits()
 *
 * Print a list of exits from the player room.
 */
sc_bool lib_cmd_print_room_exits(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_bool eightpointcompass;
	const sc_char *const *dirnames;
	sc_int count, index_, trail;

	/* Decide on four or eight point compass names list. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "EightPointCompass";
	eightpointcompass = prop_get_boolean(bundle, "B<-ss", vt_key);
	dirnames = eightpointcompass ? DIRNAMES_8 : DIRNAMES_4;

	/* Poll for an exit for each valid direction name. */
	count = 0;
	trail = -1;
	for (index_ = 0; dirnames[index_]; index_++) {
		sc_vartype_t vt_rvalue;

		vt_key[0].string = "Rooms";
		vt_key[1].integer = gs_playerroom(game);
		vt_key[2].string = "Exits";
		vt_key[3].integer = index_;
		if (prop_get(bundle, "I<-sisi", &vt_rvalue, vt_key)
		        && lib_can_go(game, gs_playerroom(game), index_)) {
			if (count > 0) {
				if (count == 1) {
					/* Vary text slightly for DispFirstRoom. */
					if (game->turns == 0)
						pf_buffer_string(filter, "There are exits ");
					else
						pf_buffer_string(filter,
						                 lib_select_response(game,
						                                     "You can move ",
						                                     "I can move ",
						                                     "%player% can move "));
				} else
					pf_buffer_string(filter, ", ");
				pf_buffer_string(filter, dirnames[trail]);
			}
			trail = index_;
			count++;
		}
	}
	if (count >= 1) {
		if (count == 1) {
			/* Vary text slightly for DispFirstRoom. */
			if (game->turns == 0)
				pf_buffer_string(filter, "There is an exit ");
			else
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You can only move ",
				                                     "I can only move ",
				                                     "%player% can only move "));
		} else
			pf_buffer_string(filter, " and ");
		pf_buffer_string(filter, dirnames[trail]);
		pf_buffer_string(filter, ".\n");
	} else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't go in any direction!\n",
		                                     "I can't go in any direction!\n",
		                                     "%player% can't go in any direction!\n"));
	}

	return TRUE;
}


/*
 * lib_describe_player_room()
 *
 * Print out details of the player room, in brief if verbose not set and the
 * room has already been visited.
 */
static void lib_describe_player_room(sc_gameref_t game, sc_bool force_verbose) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];

	/* Print the room name. */
	lib_print_room_name(game, gs_playerroom(game));

	/* Print other room details if applicable. */
	if (force_verbose
	        || game->verbose || !gs_room_seen(game, gs_playerroom(game))) {
		sc_bool showexits;

		/* Print room description, and objects and NPCs. */
		lib_print_room_description(game, gs_playerroom(game));

		/* Print exits if the ShowExits global requests it. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "ShowExits";
		showexits = prop_get_boolean(bundle, "B<-ss", vt_key);
		if (showexits) {
			pf_buffer_character(filter, '\n');
			lib_cmd_print_room_exits(game);
		}
	}
}


/*
 * lib_cmd_look()
 *
 * Command handler for "look" command.
 */
sc_bool lib_cmd_look(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_character(filter, '\n');
	lib_describe_player_room(game, TRUE);
	return TRUE;
}


/*
 * lib_cmd_quit()
 *
 * Called on "quit".  Exits from the game main loop.
 */
sc_bool lib_cmd_quit(sc_gameref_t game) {
	if (if_confirm(SC_CONF_QUIT))
		game->is_running = FALSE;

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_restart()
 *
 * Called on "restart".  Exits from the game main loop with restart
 * request set.
 */
sc_bool lib_cmd_restart(sc_gameref_t game) {
	if (if_confirm(SC_CONF_RESTART)) {
		game->is_running = FALSE;
		game->do_restart = TRUE;
	}

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_undo()
 *
 * Called on "undo".  Restores any undo game or memo to the main game.
 */
sc_bool lib_cmd_undo(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_memo_setref_t memento = gs_get_memento(game);

	/* If an undo buffer is available, restore it. */
	if (game->undo_available) {
		gs_copy(game, game->undo);
		game->undo_available = FALSE;

		lib_print_room_name(game, gs_playerroom(game));
		pf_buffer_string(filter, "[The previous turn has been undone.]\n");

		/* Undo can't properly unravel layered sounds... */
		game->stop_sound = TRUE;
	}

	/*
	 * If there is no undo buffer, try to restore one saved previously in a
	 * memo.  If that works, treat as for restore from file, since that's
	 * effectively what it is.
	 */
	else if (memo_load_game(memento, game)) {
		lib_print_room_name(game, gs_playerroom(game));
		pf_buffer_string(filter, "[The previous turn has been undone.]\n");

		game->is_running = FALSE;
		game->do_restore = TRUE;
	}

	/* If no undo buffer and memo restore failed, there's no undo available. */
	else if (game->turns == 0)
		pf_buffer_string(filter, "You can't undo what hasn't been done.\n");
	else
		pf_buffer_string(filter, "Sorry, no more undo is available.\n");

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_history_common()
 * lib_cmd_history_number()
 * lib_cmd_history()
 *
 * Prints a history of saved commands for the game.  Print directly rather
 * than using the printfilter to avoid possible clashes with ALRs.
 */
static sc_bool lib_cmd_history_common(sc_gameref_t game, sc_int limit) {
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_memo_setref_t memento = gs_get_memento(game);
	sc_int first, count, timestamp;

	/*
	 * The runner main loop will add an entry for the "history" command that
	 * got us here, but it hasn't done so yet.  To keep the history list
	 * accurate for recalling commands, we add a surrogate "history" command
	 * to the history here, and remove it when we've done listing.  This matches
	 * the c-shell, which always shows 'history' listed last.
	 */
	timestamp = var_get_elapsed_seconds(vars);
	memo_save_command(memento, "[history]", timestamp, game->turns);

	/* Decide on the first history to display; all if limit is 0 or less. */
	if (limit > 0) {
		/*
		 * Get a count of the history length recorded.  Because of the surrogate
		 * "history" above, this is always at least one.  From this, choose a
		 * start point for the display; all if not enough history.
		 */
		count = memo_get_command_count(memento);
		first = (count > limit) ? count - limit : 0;
	} else
		first = 0;

	if_print_string("These are your most recent game commands:\n\n");

	/* Display history starting at the first entry determined above. */
	memo_first_command(memento);
	for (count = 0; memo_more_commands(memento); count++) {
		const sc_char *command;
		sc_int sequence, turns;

		/* Obtain the history entry, and write if included. */
		memo_next_command(memento, &command, &sequence, &timestamp, &turns);
		if (count >= first) {
			sc_int hr, min, sec;
			sc_char buffer[64];

			/* Write the history entry sequence. */
			sprintf(buffer, "%4ld -- Time ", sequence);
			if_print_string(buffer);

			/* Separate the timestamp out into components. */
			hr = timestamp / SECS_PER_HOUR;
			min = (timestamp % SECS_PER_HOUR) / MINS_PER_HOUR;
			sec = timestamp % SECS_PER_MINUTE;

			/* Print playing time as "[HHh ][M]Mm SSs". */
			if (hr > 0)
				sprintf(buffer, "%ldh %02ldm %02lds", hr, min, sec);
			else
				sprintf(buffer, "%ldm %02lds", min, sec);
			if_print_string(buffer);

			/* Follow up with the turns count, and the command string itself. */
			sprintf(buffer, ", turn %ld : ", turns);
			if_print_string(buffer);
			if_print_string(command);
			if_print_character('\n');
		}
	}

	/* Remove the surrogate "history"; the main loop will add the real one. */
	memo_unsave_command(memento);

	game->is_admin = TRUE;
	return TRUE;
}

sc_bool lib_cmd_history_number(sc_gameref_t game) {
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int limit;

	/* Get requested length of history list, and complain if not valid. */
	limit = var_get_ref_number(vars);
	if (limit < 1) {
		if_print_string("That's not a valid history length.\n");

		game->is_admin = TRUE;
		return TRUE;
	}

	return lib_cmd_history_common(game, limit);
}

sc_bool lib_cmd_history(sc_gameref_t game) {
	return lib_cmd_history_common(game, 0);
}


/*
 * lib_cmd_again()
 * lib_cmd_redo_number()
 * lib_cmd_redo_text_last_common()
 * lib_cmd_redo_text()
 * lib_cmd_redo_last()
 *
 * The first function is called on "again", and simply sets the game do_again
 * flag.  The others allow the user to select a command from the history list
 * to re-run.
 */
sc_bool lib_cmd_again(sc_gameref_t game) {
	game->do_again = TRUE;
	game->redo_sequence = 0;

	game->is_admin = TRUE;
	return TRUE;
}

sc_bool lib_cmd_redo_number(sc_gameref_t game) {
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_memo_setref_t memento = gs_get_memento(game);
	sc_int sequence;

	/*
	 * Get the history sequence entry requested and validate it.  The sequence
	 * may be positive (absolute) or negative (relative to history end), but
	 * not zero.
	 */
	sequence = var_get_ref_number(vars);
	if (sequence != 0 && memo_find_command(memento, sequence)) {
		game->do_again = TRUE;
		game->redo_sequence = sequence;
	} else {
		if_print_string("No matching entry found in the command history.\n");

		/*
		 * This is a failed redo, but returning FALSE will cause the game's
		 * unknown command message to come up.  However, returning TRUE will
		 * cause the runner main loop to add this to its history, and at some
		 * point a "redo 7" could cause problems (say, when it's at sequence 7,
		 * where it'll cause an infinite loop).  To work round this, here we'll
		 * return a redo_sequence _without_ do_again, and have the runner catch
		 * that as an indication not to save the command in its history.  Sorry
		 * for the ugliness.
		 */
		game->do_again = FALSE;
		game->redo_sequence = INTEGER_MAX;
	}

	game->is_admin = TRUE;
	return TRUE;
}

static sc_bool lib_cmd_redo_text_last_common(sc_gameref_t game, const sc_char *target) {
	const sc_memo_setref_t memento = gs_get_memento(game);
	sc_bool is_do_last, is_contains;
	sc_int length, matched_sequence;

	/* Make a special case of "!!", rerun the final command in the history. */
	is_do_last = (strcmp(target, "!") == 0);

	/*
	 * Differentiate starts-with and contains searches, setting is_contains and
	 * advancing by one if the target begins '?' (word search).  Note target
	 * string length.
	 */
	is_contains = (target[0] == '?');
	target += is_contains ? 1 : 0;
	length = strlen(target);

	/* If there's no text left to search for, reject this call now. */
	if (length == 0) {
		if_print_string("No matching entry found in the command history.\n");

		/* As with failed numeric redo above, special-case this return. */
		game->do_again = FALSE;
		game->redo_sequence = INTEGER_MAX;

		game->is_admin = TRUE;
		return TRUE;
	}

	/*
	 * Search saved commands for one that matches the target string in the
	 * required way.  We want to return the most recently saved match, so ideally
	 * we'd search backwards, but the iterator is only forwards, so we do it the
	 * hard way.
	 */
	matched_sequence = 0;
	memo_first_command(memento);
	while (memo_more_commands(memento)) {
		const sc_char *command;
		sc_int sequence, timestamp, turns;
		sc_bool is_matched;

		/* Get the command; only command and sequence are relevant. */
		memo_next_command(memento, &command, &sequence, &timestamp, &turns);

		/*
		 * If this is the "!!" special case, match everything.  Otherwise,
		 * either search the command for the target, or match if the command
		 * begins with the target.
		 */
		if (is_do_last)
			is_matched = TRUE;
		else if (is_contains) {
			sc_int index_;

			/* Search this command for an occurrence of target anywhere. */
			is_matched = FALSE;
			for (index_ = strlen(command) - length; index_ >= 0; index_--) {
				if (sc_strncasecmp(command + index_, target, length) == 0) {
					is_matched = TRUE;
					break;
				}
			}
		} else
			is_matched = (sc_strncasecmp(command, target, length) == 0);

		/* If the command matched the target criteria, note it and continue. */
		if (is_matched)
			matched_sequence = sequence;
	}

	/* If we found a match, set the redo values accordingly. */
	if (matched_sequence > 0) {
		game->do_again = TRUE;
		game->redo_sequence = matched_sequence;
	} else {
		if_print_string("No matching entry found in the command history.\n");

		/* As with failed numeric redo above, special-case this return. */
		game->do_again = FALSE;
		game->redo_sequence = INTEGER_MAX;
	}

	game->is_admin = TRUE;
	return TRUE;
}

sc_bool lib_cmd_redo_text(sc_gameref_t game) {
	const sc_var_setref_t vars = gs_get_vars(game);

	/* Call the common redo with the referenced text from %text%. */
	return lib_cmd_redo_text_last_common(game, var_get_ref_text(vars));
}

sc_bool lib_cmd_redo_last(sc_gameref_t game) {
	/* Call the common redo with, literally, "!", forming "!!" . */
	return lib_cmd_redo_text_last_common(game, "!");
}


/*
 * lib_cmd_hints()
 *
 * Called on "hints".  Requests the interface to display any available hints.
 */
sc_bool lib_cmd_hints(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int task;
	sc_bool game_has_hints;

	/*
	 * Check for the presence of any game hints at all, no matter whether the
	 * task is runnable or not.
	 */
	game_has_hints = FALSE;
	for (task = 0; task < gs_task_count(game); task++) {
		if (task_has_hints(game, task)) {
			game_has_hints = TRUE;
			break;
		}
	}

	/* If the game has hints, display any relevant ones. */
	if (game_has_hints) {
		if (run_hint_iterate(game, NULL)) {
			if (if_confirm(SC_CONF_VIEW_HINTS))
				if_display_hints(game);
		} else
			pf_buffer_string(filter, "There are currently no hints available.\n");
	} else {
		pf_buffer_string(filter,
		                 "There are no hints available for this adventure.\n");
		pf_buffer_string(filter,
		                 "You're just going to have to work it out for"
		                 " yourself...\n");
	}

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_print_string_bold()
 * lib_print_string_italics()
 *
 * Convenience helpers for printing licensing and game information.
 */
static void lib_print_string_bold(const sc_char *string) {
	if_print_tag(SC_TAG_BOLD, "");
	if_print_string(string);
	if_print_tag(SC_TAG_ENDBOLD, "");
}

static void lib_print_string_italics(const sc_char *string) {
	if_print_tag(SC_TAG_ITALICS, "");
	if_print_string(string);
	if_print_tag(SC_TAG_ENDITALICS, "");
}


/*
 * lib_cmd_help()
 * lib_cmd_license()
 *
 * A form of standard help output for games that don't define it themselves,
 * and the GPL licensing.  Print directly rather than using the printfilter
 * to avoid possible clashes with ALRs.
 */
sc_bool lib_cmd_help(sc_gameref_t game) {
	if_print_string(
	    "These are some of the typical commands used in this adventure:\n\n");

	if_print_string(
	    "  [N]orth, [E]ast, [S]outh, [W]est, [U]p, [D]own, [In], [O]ut,"
	    " [L]ook, [Exits]\n  E[x]amine <object>, [Get <object>],"
	    " [Drop <object>], [...it], [...all]\n  [Where is <object>]\n"
	    "  [Give <object> to  <character>], [Open...], [Close...],"
	    " [Ask <character> about <subject>]\n"
	    "  [Wear <object>], [Remove <object>], [I]nventory\n"
	    "  [Put <object> into <object>], [Put <object> onto <object>]\n");

	if_print_string("\nUse the ");
	lib_print_string_italics("Save");
	if_print_string(", ");
	lib_print_string_italics("Restore");
	if_print_string(", ");
	lib_print_string_italics("Undo");
	if_print_string(", and ");
	lib_print_string_italics("Quit");
	if_print_string(
	    " commands to save and restore games, undo a move, and leave the "
	    " game.  Use ");
	lib_print_string_italics("History");
	if_print_string(" and ");
	lib_print_string_italics("Redo");
	if_print_string(
	    " to view and repeat recent game commands.\n");

	if_print_string("\nThe ");
	lib_print_string_italics("Hint");
	if_print_string(" command displays any game hints, ");
	lib_print_string_italics("Notify");
	if_print_string(" provides score change notification, and ");
	lib_print_string_italics("Verbose");
	if_print_string(" and ");
	lib_print_string_italics("Brief");
	if_print_string(" control room descriptions.\n");

	if_print_string("\nUse ");
	lib_print_string_italics("License");
	if_print_string(
	    " to view SCARE's licensing terms and conditions, and ");
	lib_print_string_italics("Version");
	if_print_string(
	    " to print both SCARE's and the game's version number.\n");

	game->is_admin = TRUE;
	return TRUE;
}

sc_bool lib_cmd_license(sc_gameref_t game) {
	lib_print_string_bold("SCARE");
	if_print_string(" is ");
	lib_print_string_italics(
	    "Copyright (C) 2003-2008  Simon Baldwin and Mark J. Tilford");
	if_print_string(".\n\n");

	if_print_string(
	    "This program is free software; you can redistribute it and/or modify"
	    " it under the terms of version 2 of the GNU General Public License"
	    " as published by the Free Software Foundation.\n\n");

	if_print_string(
	    "This program is distributed in the hope that it will be useful, but ");
	lib_print_string_bold("WITHOUT ANY WARRANTY");
	if_print_string("; without even the implied warranty of ");
	lib_print_string_bold("MERCHANTABILITY");
	if_print_string(" or ");
	lib_print_string_bold("FITNESS FOR A PARTICULAR PURPOSE");
	if_print_string(
	    ".  See the GNU General Public License for more details.\n\n");

	if_print_string(
	    "You should have received a copy of the GNU General Public License"
	    " along with this program; if not, write to the Free Software"
	    " Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301"
	    " USA\n\n");

	if_print_string("Please report any bugs, omissions, or misfeatures to ");
	lib_print_string_italics("simon_baldwin@yahoo.com");
	if_print_string(".\n");

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_information()
 *
 * Display a few small pieces of game information, done by a dialog GUI
 * in real Adrift.  Prints directly rather than using the printfilter to
 * avoid possible clashes with ALRs.
 */
sc_bool lib_cmd_information(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_vartype_t vt_key[2];
	const sc_char *gamename, *compile_date, *gameauthor;
	sc_char *filtered;

	vt_key[0].string = "Globals";
	vt_key[1].string = "GameName";
	gamename = prop_get_string(bundle, "S<-ss", vt_key);
	filtered = pf_filter_for_info(gamename, vars);
	pf_strip_tags(filtered);

	if_print_string("\"");
	if_print_string(!sc_strempty(filtered) ? filtered : "Untitled");
	if_print_string("\"");
	sc_free(filtered);

	vt_key[0].string = "CompileDate";
	compile_date = prop_get_string(bundle, "S<-s", vt_key);
	if (!sc_strempty(compile_date)) {
		if_print_string(", ");
		if_print_string(compile_date);
	}

	vt_key[0].string = "Globals";
	vt_key[1].string = "GameAuthor";
	gameauthor = prop_get_string(bundle, "S<-ss", vt_key);
	filtered = pf_filter_for_info(gameauthor, vars);
	pf_strip_tags(filtered);

	if_print_string(", ");
	if_print_string(!sc_strempty(filtered) ? filtered : "Anonymous");
	if_print_string(".\n");
	sc_free(filtered);

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_clear()
 *
 * Clear the main game window (almost).
 */
sc_bool lib_cmd_clear(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_tag(filter, SC_TAG_CLS);
	pf_buffer_string(filter, "Screen cleared.\n");
	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_statusline()
 *
 * Display the status line as would be shown by the Runner.  Useful for
 * interpreter builds that can't offer a true status line.  Prints directly
 * rather than using the printfilter to avoid possible clashes with ALRs.
 */
sc_bool lib_cmd_statusline(sc_gameref_t game) {
	const sc_char *name, *author, *room, *status;
	sc_int score;

	/*
	 * Retrieve the game's name and author, the description of the current
	 * game room, and any formatted game status line.
	 */
	run_get_attributes(game, &name, &author, NULL, NULL,
	                   &score, NULL, &room, &status, NULL, NULL, NULL, NULL);

	/* If nothing is yet determined, print the game name and author. */
	if (!room || sc_strempty(room)) {
		if_print_string(name);
		if_print_string(" | ");
		if_print_string(author);
	} else {
		/* Print the player location, and a separator. */
		if_print_string(room);
		if_print_string(" | ");

		/* If the game offers a status line, print it, otherwise the score. */
		if (status && !sc_strempty(status))
			if_print_string(status);
		else {
			sc_char buffer[32];

			if_print_string("Score: ");
			sprintf(buffer, "%ld", score);
			if_print_string(buffer);
		}
	}
	if_print_character('\n');

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_version()
 *
 * Display the "Runner version".  Prints directly rather than using the
 * printfilter to avoid possible clashes with ALRs.
 */
sc_bool lib_cmd_version(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key;
	sc_char buffer[64];
	sc_int major, minor, point;
	const sc_char *version;

	if_print_string("SCARE version ");
	if_print_string(SCARE_VERSION SCARE_PATCH_LEVEL);
	if_print_string(" [Adrift ");
	major = SCARE_EMULATION / 1000;
	minor = (SCARE_EMULATION % 1000) / 100;
	point = SCARE_EMULATION % 100;
	sprintf(buffer, "%ld.%02ld.%02ld", major, minor, point);
	if_print_string(buffer);
	if_print_string(" compatible], ");

	vt_key.string = "VersionString";
	version = prop_get_string(bundle, "S<-s", &vt_key);
	if_print_string("Generator version ");
	if_print_string(version);
	if_print_string(".\n");

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_wait()
 * lib_cmd_wait_number()
 *
 * Set game waitcounter to a count of turns for which the main loop will run
 * without taking input.  Many Adrift Runners ignore any WaitTurns setting in
 * the game, and use always use one; this might make a game misbehave, so to
 * try to cover this case we supply 'wait N' as a player control to override
 * the game's setting.  The latter prints directly rather than using the
 * printfilter to avoid possible clashes with ALRs.
 */
sc_bool lib_cmd_wait(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	sc_int waitturns;

	/* Note if wait turns is different from the game's setting. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "WaitTurns";
	waitturns = prop_get_integer(bundle, "I<-ss", vt_key);
	if (waitturns != game->waitturns) {
		sc_char buffer[32];

		pf_buffer_string(filter, "(");
		sprintf(buffer, "%ld", game->waitturns);
		pf_buffer_string(filter, buffer);
		pf_buffer_string(filter,
		                 game->waitturns == 1 ? " turn)\n" : " turns)\n");
	}

	/* Reset the wait counter to the current waitturns setting. */
	game->waitcounter = game->waitturns;

	pf_buffer_string(filter, "Time passes...\n");
	return TRUE;
}

sc_bool lib_cmd_wait_number(sc_gameref_t game) {
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int waitturns;
	sc_char buffer[32];

	/* Get and validate the waitturns setting. */
	waitturns = var_get_ref_number(vars);
	if (waitturns < 1 || waitturns > 20) {
		if_print_string("You can only wait between 1 and 20 turns.\n");
		game->is_admin = TRUE;
		return TRUE;
	}

	/* Update the game setting, and confirm for the player. */
	game->waitturns = waitturns;

	if_print_string("The game will now wait ");
	sprintf(buffer, "%ld", waitturns);
	if_print_string(buffer);
	if_print_string(waitturns == 1 ? " turn" : " turns");
	if_print_string(" for each 'wait' command you enter.\n");

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_verbose()
 * lib_cmd_brief()
 *
 * Set/clear game verbose flag.  Print directly rather than using the
 * printfilter to avoid possible clashes with ALRs.
 */
sc_bool lib_cmd_verbose(sc_gameref_t game) {
	/* Set game verbose flag and return. */
	game->verbose = TRUE;
	if_print_string("The game is now in its ");
	if_print_tag(SC_TAG_ITALICS, "");
	if_print_string("verbose");
	if_print_tag(SC_TAG_ENDITALICS, "");
	if_print_string(" mode, which always gives long descriptions of locations"
	                " (even if you've been there before).\n");

	game->is_admin = TRUE;
	return TRUE;
}

sc_bool lib_cmd_brief(sc_gameref_t game) {
	/* Clear game verbose flag and return. */
	game->verbose = FALSE;
	if_print_string("The game is now in its ");
	if_print_tag(SC_TAG_ITALICS, "");
	if_print_string("brief");
	if_print_tag(SC_TAG_ENDITALICS, "");
	if_print_string(" mode, which gives long descriptions of places never"
	                " before visited and short descriptions otherwise.\n");

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_notify_on_off()
 * lib_cmd_notify()
 *
 * Set/clear/query game score change notification flag.  Print directly
 * rather than using the printfilter to avoid possible clashes with ALRs.
 */
sc_bool lib_cmd_notify_on_off(sc_gameref_t game) {
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_char *control;

	/* Get the text following the notify command, and check for "on"/"off". */
	control = var_get_ref_text(vars);
	if (sc_strcasecmp(control, "on") == 0) {
		/* Set score change notification. */
		game->notify_score_change = TRUE;
		if_print_string("Game score change notification is now ");
		if_print_tag(SC_TAG_ITALICS, "");
		if_print_string("on");
		if_print_tag(SC_TAG_ENDITALICS, "");
		if_print_string(", and the game will tell you of any changes in the"
		                " score.\n");
	} else if (sc_strcasecmp(control, "off") == 0) {
		/* Clear score change notification. */
		game->notify_score_change = FALSE;
		if_print_string("Game score change notification is now ");
		if_print_tag(SC_TAG_ITALICS, "");
		if_print_string("off");
		if_print_tag(SC_TAG_ENDITALICS, "");
		if_print_string(", and the game will be silent on changes in the"
		                " score.\n");
	} else {
		if_print_string("Use 'notify on' or 'notify off' to control game"
		                " score notification.\n");
	}

	game->is_admin = TRUE;
	return TRUE;
}

sc_bool lib_cmd_notify(sc_gameref_t game) {
	/* Report the current state of notification. */
	if_print_string("Game score change notification is ");
	if_print_tag(SC_TAG_ITALICS, "");
	if_print_string(game->notify_score_change ? "on" : "off");
	if_print_tag(SC_TAG_ENDITALICS, "");

	if (game->notify_score_change) {
		if_print_string(", and the game will tell you of any changes in the"
		                " score.\n");
	} else {
		if_print_string(", and the game will be silent on changes in the"
		                " score.\n");
	}

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_time()
 * lib_cmd_date()
 *
 * Print elapsed game time, and smart-alec "date" response.  The Adrift
 * Runner responds here with the system time and date, but we'll do something
 * different.
 */
sc_bool lib_cmd_time(sc_gameref_t game) {
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_uint timestamp;
	sc_int hr, min, sec;
	sc_char buffer[64];

	/* Get elapsed game time and convert to hour, minutes, and seconds. */
	timestamp = var_get_elapsed_seconds(vars);
	hr = timestamp / SECS_PER_HOUR;
	min = (timestamp % SECS_PER_HOUR) / MINS_PER_HOUR;
	sec = timestamp % SECS_PER_MINUTE;
	if (hr > 0)
		sprintf(buffer, "%ldh %02ldm %02lds", hr, min, sec);
	else
		sprintf(buffer, "%ldm %02lds", min, sec);

	/* Print the game's elapsed time. */
	if_print_string("You have been running the game for ");
	if_print_string(buffer);
	if_print_string(".\n");

	game->is_admin = TRUE;
	return TRUE;
}

sc_bool lib_cmd_date(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "Maybe we should just be good friends.\n");
	return TRUE;
}


/*
 * Direction enumeration.  Used by movement commands, to multiplex them all
 * into a single function.  The values are explicit to ensure they match
 * enumerations in the game data.
 */
enum {
	DIR_NORTH = 0, DIR_EAST = 1, DIR_SOUTH = 2, DIR_WEST = 3,
	DIR_UP = 4, DIR_DOWN = 5, DIR_IN = 6, DIR_OUT = 7,
	DIR_NORTHEAST = 8, DIR_SOUTHEAST = 9, DIR_SOUTHWEST = 10, DIR_NORTHWEST = 11
};


/*
 * lib_go()
 *
 * Central movement command, called by all movement handlers.
 */
static sc_bool lib_go(sc_gameref_t game, sc_int direction) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5], vt_rvalue;
	sc_bool eightpointcompass, is_trapped, is_exitable[12];
	sc_int destination, index_;
	const sc_char *const *dirnames;

	/* Decide on four or eight point compass names list. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "EightPointCompass";
	eightpointcompass = prop_get_boolean(bundle, "B<-ss", vt_key);
	dirnames = eightpointcompass ? DIRNAMES_8 : DIRNAMES_4;

	/* Start by seeing if there are any exits at all available. */
	is_trapped = TRUE;
	for (index_ = 0; dirnames[index_]; index_++) {
		vt_key[0].string = "Rooms";
		vt_key[1].integer = gs_playerroom(game);
		vt_key[2].string = "Exits";
		vt_key[3].integer = index_;
		if (prop_get(bundle, "I<-sisi", &vt_rvalue, vt_key)
		        && lib_can_go(game, gs_playerroom(game), index_)) {
			is_exitable[index_] = TRUE;
			is_trapped = FALSE;
		} else
			is_exitable[index_] = FALSE;
	}
	if (is_trapped) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't go in any direction!\n",
		                                     "I can't go in any direction!\n",
		                                     "%player% can't go in any direction!\n"));
		return TRUE;
	}

	/*
	 * Check for the exit, and if it doesn't exist, refuse, and list the possible
	 * options.
	 */
	vt_key[0].string = "Rooms";
	vt_key[1].integer = gs_playerroom(game);
	vt_key[2].string = "Exits";
	vt_key[3].integer = direction;
	vt_key[4].string = "Dest";
	if (prop_get(bundle, "I<-sisis", &vt_rvalue, vt_key))
		destination = vt_rvalue.integer - 1;
	else {
		sc_int count, trail;

		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't go in that direction, but you can move ",
		                                     "I can't go in that direction, but I can move ",
		                                     "%player% can't go in that direction, but can move "));

		/* List available exits, found in exit test loop earlier. */
		count = 0;
		trail = -1;
		for (index_ = 0; dirnames[index_]; index_++) {
			if (is_exitable[index_]) {
				if (count > 0) {
					if (count > 1)
						pf_buffer_string(filter, ", ");
					pf_buffer_string(filter, dirnames[trail]);
				}
				trail = index_;
				count++;
			}
		}
		if (count >= 1) {
			if (count > 1)
				pf_buffer_string(filter, " and ");
			pf_buffer_string(filter, dirnames[trail]);
		}
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	/* Check for any movement restrictions. */
	if (!lib_can_go(game, gs_playerroom(game), direction)) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't go in that direction (at present).\n",
		                                     "I can't go in that direction (at present).\n",
		                                     "%player% can't go in that direction (at present).\n"));
		return TRUE;
	}

	if (lib_trace) {
		sc_trace("Library: moving player from %ld to %ld\n",
		         gs_playerroom(game), destination);
	}

	/* Indicate if getting off something or standing up first. */
	if (gs_playerparent(game) != -1) {
		pf_buffer_string(filter, "(Getting off ");
		lib_print_object_np(game, gs_playerparent(game));
		pf_buffer_string(filter, " first)\n");
	} else if (gs_playerposition(game) != 0)
		pf_buffer_string(filter, "(Standing up first)\n");

	/* Confirm and then make move. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You move ",
	                                     "I move ",
	                                     "%player% moves "));
	pf_buffer_string(filter, dirnames[direction]);
	pf_buffer_string(filter, ".\n");

	gs_move_player_to_room(game, destination);

	/* Describe the new room and return. */
	lib_describe_player_room(game, FALSE);
	return TRUE;
}


/*
 * lib_cmd_go_*()
 *
 * Direction-specific movement commands.
 */
sc_bool lib_cmd_go_north(sc_gameref_t game) {
	return lib_go(game, DIR_NORTH);
}

sc_bool lib_cmd_go_east(sc_gameref_t game) {
	return lib_go(game, DIR_EAST);
}

sc_bool lib_cmd_go_south(sc_gameref_t game) {
	return lib_go(game, DIR_SOUTH);
}

sc_bool lib_cmd_go_west(sc_gameref_t game) {
	return lib_go(game, DIR_WEST);
}

sc_bool lib_cmd_go_up(sc_gameref_t game) {
	return lib_go(game, DIR_UP);
}

sc_bool lib_cmd_go_down(sc_gameref_t game) {
	return lib_go(game, DIR_DOWN);
}

sc_bool lib_cmd_go_in(sc_gameref_t game) {
	return lib_go(game, DIR_IN);
}

sc_bool lib_cmd_go_out(sc_gameref_t game) {
	return lib_go(game, DIR_OUT);
}

sc_bool lib_cmd_go_northeast(sc_gameref_t game) {
	return lib_go(game, DIR_NORTHEAST);
}

sc_bool lib_cmd_go_southeast(sc_gameref_t game) {
	return lib_go(game, DIR_SOUTHEAST);
}

sc_bool lib_cmd_go_northwest(sc_gameref_t game) {
	return lib_go(game, DIR_NORTHWEST);
}

sc_bool lib_cmd_go_southwest(sc_gameref_t game) {
	return lib_go(game, DIR_SOUTHWEST);
}


/*
 * lib_compare_rooms()
 *
 * Helper for lib_cmd_go_room().  Compare the name of the passed in room
 * with the string passed in, and return TRUE if they match.  The routine
 * requires that string is filtered, stripped, trimmed and normalized.
 */
static sc_bool lib_compare_rooms(sc_gameref_t game, sc_int room, const sc_char *string) {
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_char *name, *compare_name;
	sc_bool status;

	/* Get the name of the room, and filter it down to a plain string. */
	name = pf_filter(lib_get_room_name(game, room), vars, bundle);
	pf_strip_tags(name);
	sc_normalize_string(sc_trim_string(name));

	/* Bypass any prefix on the room name. */
	if (sc_compare_word(name, "a", 1))
		compare_name = name + 1;
	else if (sc_compare_word(name, "an", 2))
		compare_name = name + 2;
	else if (sc_compare_word(name, "the", 3))
		compare_name = name + 3;
	else
		compare_name = name;
	sc_trim_string(compare_name);

	/* Compare strings, then free the allocated name. */
	status = sc_strcasecmp(compare_name, string) == 0;
	sc_free(name);

	return status;
}


/*
 * lib_cmd_go_room()
 *
 * A weak replica of the Runner's claimed ability to go to a named room via
 * rooms that have already been visited using a shortest-path search.  This
 * version scans adjacent rooms for accessibility, and then generates the
 * required directional move for any unique match.
 *
 * Note that rooms can have the same name after they've been cleaned up for
 * text comparisons, for example, two "Manor Grounds" at the start of Humbug,
 * differentiated within the game with trailing "<some_tag>" components.
 */
sc_bool lib_cmd_go_room(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5], vt_rvalue;
	sc_bool eightpointcompass, is_trapped, is_ambiguous;
	sc_int direction, destination, index_;
	const sc_char *const *dirnames;
	sc_char *name, *compare_name;

	/* Determine the requested room, and filter it down to a plain string. */
	name = pf_filter(var_get_ref_text(vars), vars, bundle);
	pf_strip_tags(name);
	sc_normalize_string(sc_trim_string(name));

	/* Bypass any prefix on the request room name. */
	if (sc_compare_word(name, "a", 1))
		compare_name = name + 1;
	else if (sc_compare_word(name, "an", 2))
		compare_name = name + 2;
	else if (sc_compare_word(name, "the", 3))
		compare_name = name + 3;
	else
		compare_name = name;
	sc_trim_string(compare_name);

	/* See if the named room is the current player room. */
	if (lib_compare_rooms(game, gs_playerroom(game), compare_name)) {
		pf_buffer_string(filter, "You are already there!\n");
		sc_free(name);
		return TRUE;
	}

	/* Decide on four or eight point compass names list. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "EightPointCompass";
	eightpointcompass = prop_get_boolean(bundle, "B<-ss", vt_key);
	dirnames = eightpointcompass ? DIRNAMES_8 : DIRNAMES_4;

	/* Search adjacent and available rooms for a name match. */
	is_trapped = TRUE;
	is_ambiguous = FALSE;
	direction = -1;
	destination = -1;
	for (index_ = 0; dirnames[index_]; index_++) {
		vt_key[0].string = "Rooms";
		vt_key[1].integer = gs_playerroom(game);
		vt_key[2].string = "Exits";
		vt_key[3].integer = index_;
		if (prop_get(bundle, "I<-sisi", &vt_rvalue, vt_key)
		        && lib_can_go(game, gs_playerroom(game), index_)) {
			is_trapped = FALSE;

			/*
			 * Room is available.  Compare its name with that requested provided
			 * that it's a location we've not already accepted (that is, some
			 * rooms are reachable by multiple directions, such as both "south"
			 * and "out").
			 */
			vt_key[4].string = "Dest";
			if (prop_get(bundle, "I<-sisis", &vt_rvalue, vt_key)) {
				sc_int location;

				location = vt_rvalue.integer - 1;
				if (location != destination
				        && lib_compare_rooms(game, location, compare_name)) {
					if (direction != -1)
						is_ambiguous = TRUE;
					direction = index_;
					destination = location;
				}
			}
		}
	}
	sc_free(name);

	/* If trapped or it's unclear where to go, handle these cases. */
	if (is_trapped) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't go in any direction!\n",
		                                     "I can't go in any direction!\n",
		                                     "%player% can't go in any direction!\n"));
		return TRUE;
	} else if (is_ambiguous) {
		pf_buffer_string(filter,
		                 "I'm not clear about where you want to go."
		                 "  Please try using just a direction.\n");
		pf_buffer_character(filter, '\n');
		lib_cmd_print_room_exits(game);
		return TRUE;
	}

	/* If no match, note it, otherwise handle as standard directional move. */
	if (direction == -1) {
		pf_buffer_string(filter, "I don't know how to get there from here.\n");
		pf_buffer_character(filter, '\n');
		lib_cmd_print_room_exits(game);
		return TRUE;
	}

	return lib_go(game, direction);
}


/*
 * lib_cmd_examine_self()
 *
 * Show the long description of a player.
 */
sc_bool lib_cmd_examine_self(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	sc_int task, object, count, trail;
	const sc_char *description, *position = NULL;

	/* Get selection task. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "Task";
	task = prop_get_integer(bundle, "I<-ss", vt_key) - 1;

	/* Select either the main or the alternate description. */
	if (task >= 0 && gs_task_done(game, task))
		vt_key[1].string = "AltDesc";
	else
		vt_key[1].string = "PlayerDesc";

	/* Print the description, or default response. */
	description = prop_get_string(bundle, "S<-ss", vt_key);
	if (!sc_strempty(description))
		pf_buffer_string(filter, description);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are as well as can be expected,"
		                                     " considering the circumstances.",
		                                     "I am as well as can be expected,"
		                                     " considering the circumstances.",
		                                     "%player% is as well as can be expected,"
		                                     " considering the circumstances."));
	}

	/* If not just standing on the floor, say more. */
	switch (gs_playerposition(game)) {
	case 0:
		position = lib_select_response(game,
		                               "You are standing",
		                               "I am standing",
		                               "%player% is standing");
		break;
	case 1:
		position = lib_select_response(game,
		                               "You are sitting down",
		                               "I am sitting down",
		                               "%player% is sitting down");
		break;
	case 2:
		position = lib_select_response(game,
		                               "You are lying down",
		                               "I am lying down",
		                               "%player% is lying down");
		break;
	default:
		break;
	}

	if (position
	        && !(gs_playerposition(game) == 0 && gs_playerparent(game) == -1)) {
		pf_buffer_string(filter, "  ");
		pf_buffer_string(filter, position);
		if (gs_playerparent(game) != -1) {
			pf_buffer_string(filter, " on ");
			lib_print_object_np(game, gs_playerparent(game));
		}
		pf_buffer_character(filter, '.');
	}

	/* Find and list each object worn by the player. */
	count = 0;
	trail = -1;
	for (object = 0; object < gs_object_count(game); object++) {
		if (gs_object_position(game, object) == OBJ_WORN_PLAYER) {
			if (count > 0) {
				if (count == 1) {
					pf_buffer_string(filter,
					                 lib_select_response(game,
					                                     "  You are wearing ",
					                                     "  I am wearing ",
					                                     "  %player% is wearing "));
				} else
					pf_buffer_string(filter, ", ");
				lib_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "  You are wearing ",
			                                     "  I am wearing ",
			                                     "  %player% is wearing "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object(game, trail);
		pf_buffer_character(filter, '.');
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_disambiguate_npc()
 *
 * Filter, then search the set of NPC matches.  If only one matched, note
 * and return it.  If multiple matched, print a disambiguation message and
 * the list, and return -1 with *is_ambiguous TRUE.  If none matched, return
 * -1 with *is_ambiguous FALSE if requested, otherwise print a message then
 * return -1.
 */
static sc_int lib_disambiguate_npc(sc_gameref_t game, const sc_char *verb, sc_bool *is_ambiguous) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int count, index_, npc, listed;

	/*
	 * Filter out all referenced NPCs not actually visible or seen.  Count the
	 * number of NPCs remaining as referenced by the last command, and note the
	 * last referenced NPC, for where count is 1.
	 */
	count = 0;
	npc = -1;
	for (index_ = 0; index_ < gs_npc_count(game); index_++) {
		if (game->npc_references[index_]
		        && gs_npc_seen(game, index_)
		        && npc_in_room(game, index_, gs_playerroom(game))) {
			count++;
			npc = index_;
		} else
			game->npc_references[index_] = FALSE;
	}

	/* If the reference is unambiguous, set in variables and return it. */
	if (count == 1) {
		/* Set this NPC as the referenced character. */
		var_set_ref_character(vars, npc);

		/* Return, setting no ambiguity. */
		if (is_ambiguous)
			*is_ambiguous = FALSE;
		return npc;
	}

	/* If nothing referenced, return no NPC. */
	if (count == 0) {
		if (is_ambiguous)
			*is_ambiguous = FALSE;
		else {
			pf_buffer_string(filter,
			                 "Please be more clear, who do you want to ");
			pf_buffer_string(filter, verb);
			pf_buffer_string(filter, "?\n");
		}
		return -1;
	}

	/* The NPC reference is ambiguous, so list the choices. */
	pf_buffer_string(filter, "Please be more clear, who do you want to ");
	pf_buffer_string(filter, verb);
	pf_buffer_string(filter, "?  ");

	pf_new_sentence(filter);
	listed = 0;
	for (index_ = 0; index_ < gs_npc_count(game); index_++) {
		if (game->npc_references[index_]) {
			lib_print_npc_np(game, index_);
			listed++;
			if (listed < count)
				pf_buffer_string(filter, (listed < count - 1) ? ", " : " or ");
		}
	}
	pf_buffer_string(filter, "?\n");

	/* Return no NPC for an ambiguous reference. */
	if (is_ambiguous)
		*is_ambiguous = TRUE;
	return -1;
}


/*
 * lib_disambiguate_object_common()
 * lib_disambiguate_object()
 * lib_disambiguate_object_extended()
 *
 * Filter, then search the set of object matches.  If only one matched, note
 * and return it.  If multiple matched, print a disambiguation message and
 * the list, and return -1 with *is_ambiguous TRUE.  If none matched, return
 * -1 with *is_ambiguous FALSE if requested, otherwise print a message then
 * return -1.
 *
 * Extended disambiguation operates as normal disambiguation, except that if
 * normal disambiguation returns more than one object, the resolver function,
 * if supplied, is used to see if the multiple objects can be resolved into
 * just one object.  The resolver function can normally be the same as the
 * function used to filter objects for multiple references.
 */
static sc_int lib_disambiguate_object_common(sc_gameref_t game, const sc_char *verb,
		sc_bool(*resolver)(sc_gameref_t, sc_int, sc_int),
		sc_int resolver_arg, sc_bool *is_ambiguous) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int count, index_, object, listed;

	/*
	 * Filter out all referenced objects not actually visible or seen.  Count
	 * the number of objects remaining as referenced by the last command, and
	 * note the last referenced object, for where count is 1.
	 */
	count = 0;
	object = -1;
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		if (game->object_references[index_]
		        && gs_object_seen(game, index_)
		        && obj_indirectly_in_room(game, index_, gs_playerroom(game))) {
			count++;
			object = index_;
		} else
			game->object_references[index_] = FALSE;
	}

	/*
	 * If this reference is ambiguous and a resolver was supplied, try to
	 * resolve it unambiguously by calling the resolver filter on the remaining
	 * set references.
	 */
	if (resolver && count > 1) {
		sc_int retry_count;

		/*
		 * Search for objects accepted by the resolver filter, but don't filter
		 * references just yet.  Again, note the last referenced.
		 */
		retry_count = 0;
		object = -1;
		for (index_ = 0; index_ < gs_object_count(game); index_++) {
			if (game->object_references[index_]
			        && resolver(game, index_, resolver_arg)) {
				retry_count++;
				object = index_;
			}
		}

		/* See if we narrowed the field without eliminating every object. */
		if (retry_count > 0 && retry_count < count) {
			/*
			 * If we got down to a single object, the ambiguity is resolved.
			 * In this case, set count to 1 so that 'object' is returned.
			 */
			if (retry_count == 1)
				count = retry_count;
			else {
				/*
				 * We got down to fewer objects; reduce references so that the
				 * disambiguation message is clearer.  Note that here we still
				 * leave with count greater than 1.
				 */
				count = 0;
				for (index_ = 0; index_ < gs_object_count(game); index_++) {
					if (game->object_references[index_]
					        && resolver(game, index_, resolver_arg))
						count++;
					else
						game->object_references[index_] = FALSE;
				}
			}
		}
	}

	/* If the reference is unambiguous, set in variables and return it. */
	if (count == 1) {
		/* Set this object as referenced. */
		var_set_ref_object(vars, object);

		/* Return, setting no ambiguity. */
		if (is_ambiguous)
			*is_ambiguous = FALSE;
		return object;
	}

	/* If nothing referenced, return no object. */
	if (count == 0) {
		if (is_ambiguous)
			*is_ambiguous = FALSE;
		else {
			pf_buffer_string(filter,
			                 "Please be more clear, what do you want to ");
			pf_buffer_string(filter, verb);
			pf_buffer_string(filter, "?\n");
		}
		return -1;
	}

	/* The object reference is ambiguous, so list the choices. */
	pf_buffer_string(filter, "Please be more clear, what do you want to ");
	pf_buffer_string(filter, verb);
	pf_buffer_string(filter, "?  ");

	pf_new_sentence(filter);
	listed = 0;
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		if (game->object_references[index_]) {
			lib_print_object_np(game, index_);
			listed++;
			if (listed < count)
				pf_buffer_string(filter, (listed < count - 1) ? ", " : " or ");
		}
	}
	pf_buffer_string(filter, "?\n");

	/* Return no object for an ambiguous reference. */
	if (is_ambiguous)
		*is_ambiguous = TRUE;
	return -1;
}

static sc_int lib_disambiguate_object(sc_gameref_t game, const sc_char *verb, sc_bool *is_ambiguous) {
	return lib_disambiguate_object_common(game, verb, NULL, -1, is_ambiguous);
}

static sc_int lib_disambiguate_object_extended(sc_gameref_t game, const sc_char *verb,
		sc_bool(*resolver)(sc_gameref_t, sc_int, sc_int), sc_int resolver_arg, sc_bool *is_ambiguous) {
	return lib_disambiguate_object_common(game, verb,
	                                      resolver, resolver_arg, is_ambiguous);
}


/*
 * lib_list_npc_inventory()
 *
 * List objects carried and worn by an NPC.
 */
static sc_bool lib_list_npc_inventory(sc_gameref_t game, sc_int npc, sc_bool is_described) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object, count, trail;
	sc_bool wearing;

	/* Find and list each object worn by the NPC. */
	count = 0;
	trail = -1;
	wearing = FALSE;
	for (object = 0; object < gs_object_count(game); object++) {
		if (gs_object_position(game, object) == OBJ_WORN_NPC
		        && gs_object_parent(game, object) == npc) {
			if (count > 0) {
				if (count == 1) {
					if (is_described)
						pf_buffer_string(filter, "  ");
					pf_new_sentence(filter);
					lib_print_npc_np(game, npc);
					pf_buffer_string(filter, " is wearing ");
				} else
					pf_buffer_string(filter, ", ");
				lib_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			if (is_described)
				pf_buffer_string(filter, "  ");
			pf_new_sentence(filter);
			lib_print_npc_np(game, npc);
			pf_buffer_string(filter, " is wearing ");
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object(game, trail);
		wearing = TRUE;
	}

	/* Find and list each object owned by the NPC. */
	count = 0;
	trail = -1;
	for (object = 0; object < gs_object_count(game); object++) {
		if (gs_object_position(game, object) == OBJ_HELD_NPC
		        && gs_object_parent(game, object) == npc) {
			if (count > 0) {
				if (count == 1) {
					if (!wearing) {
						if (is_described)
							pf_buffer_string(filter, "  ");
						pf_new_sentence(filter);
						lib_print_npc_np(game, npc);
					} else
						pf_buffer_string(filter, ", and");
					pf_buffer_string(filter, " is carrying ");
				} else
					pf_buffer_string(filter, ", ");
				lib_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			if (!wearing) {
				if (is_described)
					pf_buffer_string(filter, "  ");
				pf_new_sentence(filter);
				lib_print_npc_np(game, npc);
			} else
				pf_buffer_string(filter, ", and");
			pf_buffer_string(filter, " is carrying ");
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object(game, trail);
		pf_buffer_character(filter, '.');
	} else {
		if (wearing)
			pf_buffer_character(filter, '.');
	}

	/* Return TRUE if anything worn or carried. */
	return wearing || count > 0;
}


/*
 * lib_cmd_examine_npc()
 *
 * Show the long description of the most recently referenced NPC, and a
 * list of what they're wearing and carrying.
 */
sc_bool lib_cmd_examine_npc(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_int npc, task, resource;
	sc_bool is_ambiguous;
	const sc_char *description;

	/* Get the referenced npc, and if none, consider complete. */
	npc = lib_disambiguate_npc(game, "examine", &is_ambiguous);
	if (npc == -1)
		return is_ambiguous;

	/* Get selection task. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Task";
	task = prop_get_integer(bundle, "I<-sis", vt_key) - 1;

	/* Select either the main or the alternate description. */
	if (task >= 0 && gs_task_done(game, task)) {
		vt_key[2].string = "AltText";
		resource = 1;
	} else {
		vt_key[2].string = "Descr";
		resource = 0;
	}

	/* Print the description, or a default message if none. */
	description = prop_get_string(bundle, "S<-sis", vt_key);
	if (!sc_strempty(description))
		pf_buffer_string(filter, description);
	else {
		pf_buffer_string(filter, "There's nothing special about ");
		lib_print_npc_np(game, npc);
		pf_buffer_character(filter, '.');
	}

	/* Handle any associated resource. */
	vt_key[2].string = "Res";
	vt_key[3].integer = resource;
	res_handle_resource(game, "sisi", vt_key);

	/* Print what the NPC is wearing and carrying. */
	lib_list_npc_inventory(game, npc, TRUE);

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_list_in_object_normal()
 *
 * List the objects in a given container object, normal format listing.
 */
static sc_bool lib_list_in_object_normal(sc_gameref_t game, sc_int container, sc_bool is_described) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object, count, trail;

	/* List out the containers contained in this container. */
	count = 0;
	trail = -1;
	for (object = 0; object < gs_object_count(game); object++) {
		/* Contained? */
		if (gs_object_position(game, object) == OBJ_IN_OBJECT
		        && gs_object_parent(game, object) == container) {
			if (count > 0) {
				if (count == 1) {
					if (is_described)
						pf_buffer_string(filter, "  ");
					pf_buffer_string(filter, "Inside ");
					lib_print_object_np(game, container);
					pf_buffer_string(filter,
					                 lib_select_plurality(game, trail,
					                                      " is ", " are "));
				} else
					pf_buffer_string(filter, ", ");

				/* Print out the current list object. */
				lib_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			if (is_described)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter, "Inside ");
			lib_print_object_np(game, container);
			pf_buffer_string(filter,
			                 lib_select_plurality(game, trail,
			                                      " is ", " are "));
		} else
			pf_buffer_string(filter, " and ");

		/* Print out the final object. */
		lib_print_object(game, trail);
		pf_buffer_character(filter, '.');
	}

	/* Return TRUE if anything listed. */
	return count > 0;
}


/*
 * lib_list_in_object_alternate()
 *
 * List the objects in a given container object, alternate format listing.
 */
static sc_bool lib_list_in_object_alternate(sc_gameref_t game,sc_int container, sc_bool is_described) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object, count, trail;

	/* List out the objects contained in this object. */
	count = 0;
	trail = -1;
	for (object = 0; object < gs_object_count(game); object++) {
		/* Contained? */
		if (gs_object_position(game, object) == OBJ_IN_OBJECT
		        && gs_object_parent(game, object) == container) {
			if (count > 0) {
				if (count == 1) {
					if (is_described)
						pf_buffer_string(filter, "  ");
					pf_new_sentence(filter);
				} else
					pf_buffer_string(filter, ", ");

				/* Print out the current list object. */
				lib_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			if (is_described)
				pf_buffer_string(filter, "  ");
			pf_new_sentence(filter);
			lib_print_object(game, trail);
			pf_buffer_string(filter,
			                 lib_select_plurality(game, trail,
			                                      " is inside ",
			                                      " are inside "));
		} else {
			pf_buffer_string(filter, " and ");
			lib_print_object(game, trail);
			pf_buffer_string(filter, " are inside ");
		}

		/* Print out the container. */
		lib_print_object_np(game, container);
		pf_buffer_character(filter, '.');
	}

	/* Return TRUE if anything listed. */
	return count > 0;
}


/*
 * lib_list_in_object()
 *
 * List the objects in a given container object.
 *
 * TODO The Adrift Runner has two distinct styles it uses for listing objects
 * within a container, but which it picks at any one point is, frankly, a
 * mystery.  The selection below seems to work with the few games checked for
 * this, and in particular works with the ALR magic in "To Hell in a Hamper",
 * but it's almost certainly wrong.  Or, at minimum, incomplete.
 */
static sc_bool lib_list_in_object(sc_gameref_t game, sc_int container, sc_bool is_described) {
	sc_bool use_alternate_format = FALSE;

	/*
	 * Switch if the object is static and part of an NPC or the player, or if
	 * the count of contained objects in a dynamic container is exactly one.
	 */
	if (obj_is_static(game, container)) {
		sc_int object_position;

		object_position = gs_object_position(game, container);

		if (object_position == OBJ_PART_NPC || object_position == OBJ_PART_PLAYER)
			use_alternate_format = TRUE;
	} else {
		sc_int object, count;

		count = 0;
		for (object = 0; object < gs_object_count(game); object++) {
			if (gs_object_position(game, object) == OBJ_IN_OBJECT
			        && gs_object_parent(game, object) == container)
				count++;
			if (count > 1)
				break;
		}

		if (count == 1)
			use_alternate_format = TRUE;
	}

	/* List contained objects using the selected handler. */
	return use_alternate_format
	       ? lib_list_in_object_alternate(game, container, is_described)
	       : lib_list_in_object_normal(game, container, is_described);
}


/*
 * lib_list_on_object()
 *
 * List the objects on a given surface object.
 */
static sc_bool lib_list_on_object(sc_gameref_t game, sc_int supporter, sc_bool is_described) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object, count, trail;

	/* List out the objects standing on this object. */
	count = 0;
	trail = -1;
	for (object = 0; object < gs_object_count(game); object++) {
		/* Standing on? */
		if (gs_object_position(game, object) == OBJ_ON_OBJECT
		        && gs_object_parent(game, object) == supporter) {
			if (count > 0) {
				if (count == 1) {
					if (is_described)
						pf_buffer_string(filter, "  ");
					pf_new_sentence(filter);
				} else
					pf_buffer_string(filter, ", ");

				/* Print out the current list object. */
				lib_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			if (is_described)
				pf_buffer_string(filter, "  ");
			pf_new_sentence(filter);
			lib_print_object(game, trail);
			pf_buffer_string(filter,
			                 lib_select_plurality(game, trail,
			                                      " is on ",
			                                      " are on "));
		} else {
			pf_buffer_string(filter, " and ");
			lib_print_object(game, trail);
			pf_buffer_string(filter, " are on ");
		}

		/* Print out the surface. */
		lib_print_object_np(game, supporter);
		pf_buffer_character(filter, '.');
	}

	/* Return TRUE if anything listed. */
	return count > 0;
}


/*
 * lib_list_object_state()
 *
 * Describe the state of a stateful object.
 */
static sc_bool lib_list_object_state(sc_gameref_t game, sc_int object, sc_bool is_described) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_bool is_statussed;
	sc_char *state;

	/* Get object statefulness. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "CurrentState";
	is_statussed = prop_get_integer(bundle, "I<-sis", vt_key) != 0;

	/* Ensure this is a stateful object. */
	if (is_statussed) {
		if (is_described)
			pf_buffer_string(filter, "  ");
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object, " is ", " are "));

		/* Add object state string. */
		state = obj_state_name(game, object);
		if (state) {
			pf_buffer_string(filter, state);
			sc_free(state);
			pf_buffer_string(filter, ".");
		} else {
			sc_error("lib_list_object_state: invalid object state\n");
			pf_buffer_string(filter, "[invalid state].");
		}
	}

	/* Return TRUE if a state was printed. */
	return is_statussed;
}


/*
 * lib_cmd_examine_object()
 *
 * Show the long description of the most recently referenced object.
 */
sc_bool lib_cmd_examine_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int object, task, openness;
	sc_bool is_described, is_statussed, is_mentioned, is_ambiguous, should_be;
	const sc_char *description, *resource;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "examine", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Begin assuming no description printed. */
	is_described = FALSE;

	/*
	 * Get selection task and expected state; for the expected task state, FALSE
	 * indicates task completed, TRUE not completed.
	 */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Task";
	task = prop_get_integer(bundle, "I<-sis", vt_key) - 1;
	vt_key[2].string = "TaskNotDone";
	should_be = !prop_get_boolean(bundle, "B<-sis", vt_key);

	/* Select either the main or the alternate description. */
	if (task >= 0 && gs_task_done(game, task) == should_be) {
		vt_key[2].string = "AltDesc";
		resource = "Res2";
	} else {
		vt_key[2].string = "Description";
		resource = "Res1";
	}

	/* Print the description, or a default response. */
	description = prop_get_string(bundle, "S<-sis", vt_key);
	if (!sc_strempty(description)) {
		pf_buffer_string(filter, description);
		is_described |= TRUE;
	}

	/* Handle any associated resource. */
	vt_key[2].string = resource;
	res_handle_resource(game, "sis", vt_key);

	/* If the object is openable, print its openness state. */
	openness = gs_object_openness(game, object);
	switch (openness) {
	case OBJ_OPEN:
		if (is_described)
			pf_buffer_string(filter, "  ");
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object,
		                                      " is open.", " are open."));
		is_described |= TRUE;
		break;

	case OBJ_CLOSED:
		if (is_described)
			pf_buffer_string(filter, "  ");
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object,
		                                      " is closed.", " are closed."));
		is_described |= TRUE;
		break;

	case OBJ_LOCKED:
		if (is_described)
			pf_buffer_string(filter, "  ");
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object,
		                                      " is locked.", " are locked."));
		is_described |= TRUE;
		break;

	default:
		break;
	}

	/* Add any extra details for stateful objects. */
	vt_key[1].integer = object;
	vt_key[2].string = "CurrentState";
	is_statussed = prop_get_integer(bundle, "I<-sis", vt_key) != 0;
	if (is_statussed) {
		vt_key[2].string = "StateListed";
		is_mentioned = prop_get_boolean(bundle, "B<-sis", vt_key);
		if (is_mentioned)
			is_described |= lib_list_object_state(game, object, is_described);
	}

	/* For open container objects, list out what's in them. */
	if (obj_is_container(game, object) && openness <= OBJ_OPEN)
		is_described |= lib_list_in_object(game, object, is_described);

	/* For surface objects, list out what's on them. */
	if (obj_is_surface(game, object))
		is_described |= lib_list_on_object(game, object, is_described);

	/* If nothing yet said, print a default response. */
	if (!is_described) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You see nothing special about ",
		                                     "I see nothing special about ",
		                                     "%player% sees nothing special about "));
		lib_print_object_np(game, object);
		pf_buffer_character(filter, '.');
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_save_game_references()
 * lib_restore_game_references()
 *
 * Helpers for trying game commands.  Save and restore game references
 * so that parsing game commands doesn't interfere with backend loops that
 * are working through game references set by prior commands.  Saving
 * references uses the buffer passed in if possible, otherwise allocates
 * its own buffer; testing the return value shows which happened.
 */
static sc_bool *lib_save_object_references(sc_gameref_t game, sc_bool buffer[], sc_int length) {
	sc_int required, available;
	sc_bool *references;

	/*
	 * Calculate the required bytes for references, and then either allocate or
	 * use the buffer supplied.
	 */
	required = gs_object_count(game) * sizeof(*references);
	available = length * sizeof(buffer[0]);
	references = required > available ? (sc_bool *)sc_malloc(required) : buffer;

	/* Copy over references from the game, and return the saved copy. */
	memcpy(references, game->object_references, required);
	return references;
}

static void lib_restore_object_references(sc_gameref_t game, const sc_bool references[]) {
	sc_int bytes;

	/* Calculate the bytes in the references array, and copy back to the game. */
	bytes = gs_object_count(game) * sizeof(references[0]);
	memcpy(game->object_references, references, bytes);
}


/*
 * lib_try_game_command_common()
 * lib_try_game_command_short()
 * lib_try_game_command_with_object()
 * lib_try_game_command_with_npc()
 *
 * Try a game command with a standard verb.  Used by get and drop handlers
 * to retry game commands using standard "get " and "drop " commands.  This
 * makes "take/pick up/put down" work with a game's overridden get/drop.
 */
static sc_bool lib_try_game_command_common(sc_gameref_t game, const sc_char *verb, sc_int object,
		const sc_char *preposition, sc_int associate, sc_bool is_associate_object,
		sc_bool is_associate_npc) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_char buffer[LIB_ALLOCATION_AVOIDANCE_SIZE];
	sc_bool references_buffer[LIB_ALLOCATION_AVOIDANCE_SIZE];
	const sc_char *prefix, *name;
	sc_char *command;
	sc_bool *references, status;
	assert(!is_associate_object || !is_associate_npc);

	/* Save the game's references, for restore later on. */
	references = lib_save_object_references(game, references_buffer,
	                                        LIB_ALLOCATION_AVOIDANCE_SIZE);

	/* Get the addressed object's prefix and main name. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Prefix";
	prefix = prop_get_string(bundle, "S<-sis", vt_key);
	vt_key[2].string = "Short";
	name = prop_get_string(bundle, "S<-sis", vt_key);

	/* Construct and try for game commands with a standard verb. */
	if (is_associate_object || is_associate_npc) {
		const sc_char *associate_prefix, *associate_name;
		sc_int required;

		/* Get the associate's prefix and main name. */
		if (is_associate_object) {
			vt_key[0].string = "Objects";
			vt_key[1].integer = associate;
			vt_key[2].string = "Prefix";
			associate_prefix = prop_get_string(bundle, "S<-sis", vt_key);
			vt_key[2].string = "Short";
			associate_name = prop_get_string(bundle, "S<-sis", vt_key);
		} else {
			assert(is_associate_npc);
			vt_key[0].string = "NPCs";
			vt_key[1].integer = associate;
			vt_key[2].string = "Prefix";
			associate_prefix = prop_get_string(bundle, "S<-sis", vt_key);
			vt_key[2].string = "Name";
			associate_name = prop_get_string(bundle, "S<-sis", vt_key);
		}

		assert(preposition);
		required = strlen(verb) + strlen(prefix) + strlen(name)
		           + strlen(preposition) + strlen(associate_prefix)
		           + strlen(associate_name) + 6;
		command = required > (sc_int) sizeof(buffer)
		          ? (sc_char *)sc_malloc(required) : buffer;

		/*
		 * Try the command with and without prefixes on both the target object
		 * and the associate.
		 */
		sprintf(command, "%s %s %s %s %s %s", verb,
		        prefix, name, preposition, associate_prefix, associate_name);
		status = run_game_task_commands(game, command);
		if (!status) {
			sprintf(command, "%s %s %s %s %s",
			        verb, prefix, name, preposition, associate_name);
			status = run_game_task_commands(game, command);
		}
		if (!status) {
			sprintf(command, "%s %s %s %s %s",
			        verb, name, preposition, associate_prefix, associate_name);
			status = run_game_task_commands(game, command);
		}
		if (!status) {
			sprintf(command, "%s %s %s %s",
			        verb, name, preposition, associate_name);
			status = run_game_task_commands(game, command);
		}
	} else {
		sc_int required;

		required = strlen(verb) + strlen(prefix) + strlen(name) + 3;
		command = required > (sc_int) sizeof(buffer)
		          ? (sc_char *)sc_malloc(required) : buffer;

		/* Try the command with and without prefixes on the addressed object. */
		sprintf(command, "%s %s %s", verb, prefix, name);
		status = run_game_task_commands(game, command);
		if (!status) {
			sprintf(command, "%s %s", verb, name);
			status = run_game_task_commands(game, command);
		}
	}

	/* Restore the game object references back to their state on entry. */
	lib_restore_object_references(game, references);

	/* Free any allocations, and return the game command status. */
	if (command != buffer)
		sc_free(command);
	if (references != references_buffer)
		sc_free(references);
	return status;
}

static sc_bool lib_try_game_command_short(sc_gameref_t game, const sc_char *verb, sc_int object) {
	return lib_try_game_command_common(game, verb, object, NULL, -1, FALSE, FALSE);
}

static sc_bool lib_try_game_command_with_object(sc_gameref_t game, const sc_char *verb,
		sc_int object, const sc_char *preposition, sc_int other_object) {
	return lib_try_game_command_common(game, verb, object,
	                                   preposition, other_object, TRUE, FALSE);
}

static sc_bool lib_try_game_command_with_npc(sc_gameref_t game, const sc_char *verb,
		sc_int object, const sc_char *preposition, sc_int npc) {
	return lib_try_game_command_common(game, verb, object,
	                                   preposition, npc, FALSE, TRUE);
}


/*
 * lib_parse_next_object()
 *
 * Helper for lib_parse_multiple_objects().  Extracts the next object, if any,
 * from referenced text, and returns it.  Disambiguates any ambiguous objects
 * using the verb supplied, and sets are_more_objects if we found an object
 * but there appear to be more following it.
 */
static sc_bool lib_parse_next_object(sc_gameref_t game, const sc_char *verb,
		sc_bool(*resolver)(sc_gameref_t, sc_int, sc_int), sc_int resolver_arg,
		sc_int *object, sc_bool *are_more_objects, sc_bool *is_ambiguous) {
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_char *list;
	sc_bool is_matched;

	/* Look for "object" or "object and ...", and set match and more flags. */
	list = var_get_ref_text(vars);
	if (uip_match("%object%", list, game)) {
		*are_more_objects = FALSE;
		is_matched = TRUE;
	} else if (uip_match("%object% and %text%", list, game)) {
		*are_more_objects = TRUE;
		is_matched = TRUE;
	} else
		is_matched = FALSE;

	/* If we extracted an object from referenced text, disambiguate. */
	if (is_matched)
		*object = lib_disambiguate_object_extended(game, verb,
		          resolver, resolver_arg,
		          is_ambiguous);
	else
		*is_ambiguous = FALSE;

	/* Return TRUE if we matched anything. */
	return is_matched;
}


/*
 * lib_parse_multiple_objects()
 *
 * Parser for commands that take multiple object targets from a %text% match.
 * Parses object lists such as "object" and "object and object" and returns
 * the multiple objects in the game's multiple_references.
 */
static sc_bool lib_parse_multiple_objects(sc_gameref_t game, const sc_char *verb,
		sc_bool(*resolver)(sc_gameref_t, sc_int, sc_int),
		sc_int resolver_arg, sc_int *count) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int count_, object;
	sc_bool are_more_objects, is_ambiguous;

	/* Initialize variables to avoid gcc warnings. */
	object = -1;
	are_more_objects = FALSE;

	/* Clear all current multiple object references, and the count. */
	gs_clear_multiple_references(game);
	count_ = 0;

	/*
	 * Parse the first object from the list.  If we get nothing here, return
	 * FALSE if it didn't look like a multiple object list, TRUE if ambiguous.
	 * Beyond here, we always return TRUE, since after this point _something_
	 * looked believable...
	 */
	if (!lib_parse_next_object(game, verb,
	                           resolver, resolver_arg,
	                           &object, &are_more_objects, &is_ambiguous))
		return FALSE;
	else if (object == -1) {
		if (is_ambiguous) {
			/*
			 * Return TRUE, with zero count, to cause caller to return.  We get
			 * here if the first parsed object was ambiguous.  In this case,
			 * the disambiguation has printed a message, so we want our caller
			 * to simply return TRUE to indicate that the command was handled,
			 * albeit not fully successfully.
			 */
			*count = count_;
			return TRUE;
		} else {
			/*
			 * No object matched after disambiguation, so return FALSE to have
			 * our caller ignore the command.
			 */
			return FALSE;
		}
	}

	/* Mark this first object as referenced in the return array. */
	game->multiple_references[object] = TRUE;
	count_++;

	/* Now parse each additional object from the list. */
	while (are_more_objects) {
		sc_int last_object;

		/*
		 * If no next object, leave the loop.  If no disambiguation message
		 * then it was probably garble, so print a message for that case.  We
		 * also catch repeated objects here.
		 */
		last_object = object;
		if (!lib_parse_next_object(game, verb,
		                           resolver, resolver_arg,
		                           &object, &are_more_objects, &is_ambiguous)
		        || object == -1
		        || game->multiple_references[object]) {
			if (!is_ambiguous) {
				pf_buffer_string(filter,
				                 "I only understood you as far as wanting to ");
				pf_buffer_string(filter, verb);
				pf_buffer_character(filter, ' ');
				lib_print_object_np(game, last_object);
				pf_buffer_string(filter, ".\n");
			}

			/* Zero count to indicate an error somewhere in the list. */
			count_ = 0;
			break;
		}

		/* Mark the object as referenced in the return array. */
		game->multiple_references[object] = TRUE;
		count_++;
	}

	/* We found at least enough of an object list to say we matched. */
	*count = count_;
	return TRUE;
}


/*
 * lib_apply_multiple_filter()
 * lib_apply_except_filter()
 *
 * Apply filters for multiple object frontends.  Transfer multiple object
 * references into standard object references, using the supplied filter.
 * The first is inclusive, the second exclusive.
 */
static sc_int lib_apply_multiple_filter(sc_gameref_t game,
		sc_bool(*filter)(sc_gameref_t, sc_int, sc_int),
		sc_int filter_arg, sc_int *references) {
	sc_int count, object, references_;

	/* Clear all object references initially. */
	gs_clear_object_references(game);

	/*
	 * Find objects included by the filter, and transfer the reference of each
	 * from the multiple references into standard references.
	 */
	count = 0;
	references_ = references ? *references : 0;
	for (object = 0; object < gs_object_count(game); object++) {
		if (filter(game, object, filter_arg)) {
			/* Transfer the reference. */
			if (game->multiple_references[object]) {
				game->object_references[object] = TRUE;
				count++;
				game->multiple_references[object] = FALSE;
				references_--;
			}
		}
	}

	/* Copy back the updated reference count, return count. */
	if (references)
		*references = references_;
	return count;
}

static sc_int lib_apply_except_filter(sc_gameref_t game,
		sc_bool(*filter)(sc_gameref_t, sc_int, sc_int),
		sc_int filter_arg, sc_int *references) {
	sc_int count, object, references_;

	/* Clear all object references initially. */
	gs_clear_object_references(game);

	/*
	 * Find objects included by the filter, and transfer the reference of each
	 * from the multiple references into standard references.
	 */
	count = 0;
	references_ = references ? *references : 0;
	for (object = 0; object < gs_object_count(game); object++) {
		if (filter(game, object, filter_arg)) {
			/* If excepted, remove from exceptions, else add to references. */
			if (game->multiple_references[object]) {
				game->multiple_references[object] = FALSE;
				references_--;
			} else {
				game->object_references[object] = TRUE;
				count++;
			}
		}
	}

	/* Copy back the updated reference count, return count. */
	if (references)
		*references = references_;
	return count;
}


/*
 * lib_cmd_count()
 *
 * Display player weight and size limits and amounts currently carried.
 */
sc_bool lib_cmd_count(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int index_, size, weight;
	sc_char buffer[32];

	/* Sum sizes for objects currently held or worn by player. */
	size = 0;
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		if (gs_object_position(game, index_) == OBJ_HELD_PLAYER
		        || gs_object_position(game, index_) == OBJ_WORN_PLAYER)
			size += obj_get_size(game, index_);
	}

	/* Sum weights for objects currently held or worn by player. */
	weight = 0;
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		if (gs_object_position(game, index_) == OBJ_HELD_PLAYER
		        || gs_object_position(game, index_) == OBJ_WORN_PLAYER)
			weight += obj_get_weight(game, index_);
	}

	/* Print the player limits and amounts used. */
	pf_buffer_string(filter, "Size:    You have ");
	sprintf(buffer, "%ld", size);
	pf_buffer_string(filter, buffer);
	pf_buffer_string(filter, ".  The most you can hold is ");
	sprintf(buffer, "%ld", obj_get_player_size_limit(game));
	pf_buffer_string(filter, buffer);
	pf_buffer_string(filter, ".\n");

	pf_buffer_string(filter, "Weight:  You have ");
	sprintf(buffer, "%ld", weight);
	pf_buffer_string(filter, buffer);
	pf_buffer_string(filter, ".  The most you can hold is ");
	sprintf(buffer, "%ld", obj_get_player_weight_limit(game));
	pf_buffer_string(filter, buffer);
	pf_buffer_string(filter, ".\n");

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_object_too_heavy()
 *
 * Return TRUE if the given object is too heavy for the player to carry.
 */
static sc_bool lib_object_too_heavy(sc_gameref_t game, sc_int object, sc_bool *is_portable) {
	sc_int player_limit, index_, weight, object_weight;

	/* Get the player limit and the given object weight. */
	player_limit = obj_get_player_weight_limit(game);
	object_weight = obj_get_weight(game, object);

	/* Sum weights for objects currently held or worn by player. */
	weight = 0;
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		if (gs_object_position(game, index_) == OBJ_HELD_PLAYER
		        || gs_object_position(game, index_) == OBJ_WORN_PLAYER)
			weight += obj_get_weight(game, index_);
	}

	/* If requested, return object portability. */
	if (is_portable)
		*is_portable = !(object_weight > player_limit);

	/* Return TRUE if the new object exceeds limit. */
	return weight + object_weight > player_limit;
}


/*
 * lib_object_too_large()
 *
 * Return TRUE if the given object is too large for the player to carry.
 */
static sc_bool lib_object_too_large(sc_gameref_t game, sc_int object, sc_bool *is_portable) {
	sc_int player_limit, index_, size, object_size;

	/* Get the player limit and the given object size. */
	player_limit = obj_get_player_size_limit(game);
	object_size = obj_get_size(game, object);

	/* Sum sizes for objects currently held or worn by player. */
	size = 0;
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		if (gs_object_position(game, index_) == OBJ_HELD_PLAYER
		        || gs_object_position(game, index_) == OBJ_WORN_PLAYER)
			size += obj_get_size(game, index_);
	}

	/* If requested, return object portability. */
	if (is_portable)
		*is_portable = !(object_size > player_limit);

	/* Return TRUE if the new object exceeds limit. */
	return size + object_size > player_limit;
}


/*
 * lib_cmd_take_npc()
 *
 * Reject attempts to take an npc.
 */
sc_bool lib_cmd_take_npc(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int npc;
	sc_bool is_ambiguous;

	/* Get the referenced npc, and if none, consider complete. */
	npc = lib_disambiguate_npc(game, "take", &is_ambiguous);
	if (npc == -1)
		return is_ambiguous;

	/* Reject this attempt. */
	pf_buffer_string(filter, "I don't think ");
	lib_print_npc_np(game, npc);
	pf_buffer_string(filter, " would appreciate being handled.\n");
	return TRUE;
}


/*
 * lib_take_backend_common()
 *
 * Common backend handler for taking objects.  Takes all objects currently
 * referenced in the game, trying game commands first, and then moving other
 * unhandled objects to the player inventory.
 *
 * Objects to action are flagged in object_references; objects requested but
 * deemed not actionable are flagged in multiple_references.
 */
static void lib_take_backend_common(sc_gameref_t game, sc_int associate,
		sc_bool is_associate_object, sc_bool is_associate_npc) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object_count, object, count, trail, total, npc;
	sc_int too_heavy, too_large;
	sc_bool too_heavy_portable, too_large_portable, has_printed;
	assert(!is_associate_object || !is_associate_npc);

	/* Initialize our notions of anything exceeding player capacity. */
	too_heavy_portable = too_large_portable = FALSE;
	too_large = too_heavy = -1;

	/*
	 * Try game commands for all referenced objects first.  If any succeed,
	 * remove that reference from the list.  At the same time, filter out and
	 * flag any object that takes us over the player's capacity.  We report
	 * only the first.
	 */
	has_printed = FALSE;
	object_count = gs_object_count(game);
	for (object = 0; object < object_count; object++) {
		sc_bool status;

		if (!game->object_references[object])
			continue;

		/*
		 * If the object is inside or on something already held by the player,
		 * capacity checks are meaningless.
		 */
		if (!((gs_object_position(game, object) == OBJ_IN_OBJECT
		        || gs_object_position(game, object) == OBJ_ON_OBJECT)
		        && obj_indirectly_held_by_player(game,
		                gs_object_parent(game, object)))) {
			sc_bool is_portable;

			/*
			 * See if the object takes us beyond capacity.  If it does and it's
			 * the first of its kind, note it and continue.
			 */
			if (lib_object_too_heavy(game, object, &is_portable)) {
				if (too_heavy == -1) {
					too_heavy = object;
					too_heavy_portable = is_portable;
				}
				game->object_references[object] = FALSE;
				continue;
			}
			if (lib_object_too_large(game, object, &is_portable)) {
				if (too_large == -1) {
					too_large = object;
					too_large_portable = is_portable;
				}
				game->object_references[object] = FALSE;
				continue;
			}
		}

		/* Now try for a game command, using the associate if supplied. */
		if (is_associate_object)
			status = lib_try_game_command_with_object(game, "get",
			         object, "from", associate);
		else if (is_associate_npc)
			status = lib_try_game_command_with_npc(game, "get",
			                                       object, "from", associate);
		else
			status = lib_try_game_command_short(game, "get", object);
		if (status) {
			game->object_references[object] = FALSE;
			has_printed = TRUE;
		}
	}

	/*
	 * We attempt acquisition of get-able objects here only for cases where
	 * there is either no associate, or where the associate is an object.  If
	 * the associate is an NPC, we're going to refuse all acquisitions later
	 * on, by forcing object references.
	 */
	total = 0;
	if (!is_associate_npc) {
		sc_int parent, start, limit;

		/*
		 * Attempt to acquire each remaining get-able object in turn, looping
		 * on each possible parent object in turn, with an initial parent of
		 * -1 for objects not contained or supported.
		 *
		 * If we're dealing with only objects from a known container or
		 * supporter, eliminate all but one iteration of the parent search.
		 */
		start = is_associate_object ? associate : -1;
		limit = is_associate_object ? associate : object_count - 1;

		for (parent = start; parent <= limit; parent++) {
			count = 0;
			trail = -1;
			for (object = 0; object < object_count; object++) {
				sc_bool is_portable;

				if (!game->object_references[object])
					continue;

				/*
				 * If parent is -1, ignore contained objects, otherwise ignore
				 * objects not contained, or if contained, not contained by the
				 * current parent.
				 */
				if (parent == -1) {
					if (gs_object_position(game, object) == OBJ_IN_OBJECT
					        || gs_object_position(game, object) == OBJ_ON_OBJECT)
						continue;
				} else {
					if (!(gs_object_position(game, object) == OBJ_IN_OBJECT
					        || gs_object_position(game, object) == OBJ_ON_OBJECT))
						continue;
					if (gs_object_parent(game, object) != parent)
						continue;
				}

				/*
				 * Here we have to repeat capacity checks.  As objects are
				 * acquired more and more of the player's capacity gets used up.
				 * This means a check directly before each acquisition.
				 */
				if (parent == -1
				        || !obj_indirectly_held_by_player(game, parent)) {
					if (lib_object_too_heavy(game, object, &is_portable)) {
						if (too_heavy == -1) {
							too_heavy = object;
							too_heavy_portable = is_portable;
						}
						continue;
					}
					if (lib_object_too_large(game, object, &is_portable)) {
						if (too_large == -1) {
							too_large = object;
							too_large_portable = is_portable;
						}
						continue;
					}
				}

				if (count > 0) {
					if (count == 1) {
						if (has_printed)
							pf_buffer_string(filter, total == 0 ? "\n" : "  ");
						if (parent == -1)
							pf_buffer_string(filter,
							                 lib_select_response(game,
							                                     "You pick up ",
							                                     "I pick up ",
							                                     "%player% picks up "));
						else
							pf_buffer_string(filter,
							                 lib_select_response(game,
							                                     "You take ",
							                                     "I take ",
							                                     "%player% takes "));
					} else
						pf_buffer_string(filter, ", ");
					lib_print_object_np(game, trail);
				}
				trail = object;
				count++;

				gs_object_player_get(game, object);
			}

			if (count >= 1) {
				if (count == 1) {
					if (has_printed)
						pf_buffer_string(filter, total == 0 ? "\n" : "  ");
					if (parent == -1)
						pf_buffer_string(filter,
						                 lib_select_response(game,
						                                     "You pick up ",
						                                     "I pick up ",
						                                     "%player% picks up "));
					else
						pf_buffer_string(filter,
						                 lib_select_response(game,
						                                     "You take ",
						                                     "I take ",
						                                     "%player% takes "));
				} else
					pf_buffer_string(filter, " and ");
				lib_print_object_np(game, trail);
				if (parent != -1) {
					pf_buffer_string(filter, " from ");
					lib_print_object_np(game, parent);
				}
				pf_buffer_character(filter, '.');
			}
			total += count;
			has_printed |= count > 0;
		}
	}

	/*
	 * If we ran out of capacity, either in weight or in size, print the
	 * details.  Note that we currently only report the first object of any
	 * type to go over capacity.
	 */
	if (too_heavy != -1) {
		if (has_printed)
			pf_buffer_string(filter, "  ");
		pf_new_sentence(filter);
		lib_print_object_np(game, too_heavy);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, too_heavy, " is", " are"));
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     " too heavy for you to carry",
		                                     " too heavy for me to carry",
		                                     " too heavy for %player% to carry"));
		if (too_heavy_portable)
			pf_buffer_string(filter, " at the moment");
		pf_buffer_character(filter, '.');
		has_printed |= TRUE;
	} else if (too_large != -1) {
		if (has_printed)
			pf_buffer_string(filter, "  ");
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "Your hands are full",
		                                     "My hands are full",
		                                     "%player%'s hands are full"));
		if (too_large_portable)
			pf_buffer_string(filter, " at the moment");
		pf_buffer_character(filter, '.');
		has_printed |= TRUE;
	}

	/*
	 * Note any remaining multiple references left out of the take operation.
	 * This is some workload...
	 *
	 * First, deal with the case where we have an associated object.
	 */
	if (is_associate_object) {
		count = 0;
		trail = -1;
		for (object = 0; object < object_count; object++) {
			if (!game->multiple_references[object])
				continue;

			if (gs_object_position(game, object) == OBJ_HELD_PLAYER
			        || gs_object_position(game, object) == OBJ_WORN_PLAYER)
				continue;

			if (count > 0) {
				if (count == 1) {
					if (has_printed)
						pf_buffer_string(filter, "  ");
					pf_new_sentence(filter);
					lib_print_object_np(game, trail);
				} else
					pf_buffer_string(filter, ", ");
			}
			trail = object;
			count++;

			game->multiple_references[object] = FALSE;
		}

		if (count >= 1) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_new_sentence(filter);
				lib_print_object_np(game, trail);
				pf_buffer_string(filter,
				                 lib_select_plurality(game, trail,
				                                      " is not ",
				                                      " are not "));
			} else {
				pf_buffer_string(filter, " and ");
				lib_print_object_np(game, trail);
				pf_buffer_string(filter, " are not ");
			}
			if (obj_is_container(game, associate)) {
				pf_buffer_string(filter, "in ");
				if (obj_is_surface(game, associate))
					pf_buffer_string(filter, "or on ");
			} else
				pf_buffer_string(filter, "on ");
			lib_print_object_np(game, associate);
			pf_buffer_character(filter, '.');
		}
		has_printed |= count > 0;
	}

	/*
	 * Now, deal with the case where we have an associated NPC.  Once this
	 * case is handled, we can force the object references so that the code
	 * that follows on from here will report errors taking all objects.
	 *
	 * Note that this means that we can never successfully take an object
	 * from an NPC; that'll have to happen via a game's own commands.
	 */
	if (is_associate_npc) {
		count = 0;
		trail = -1;
		for (object = 0; object < object_count; object++) {
			if (!game->multiple_references[object])
				continue;

			if (gs_object_position(game, object) == OBJ_PART_NPC)
				continue;

			if (count > 0) {
				if (count == 1) {
					if (has_printed)
						pf_buffer_string(filter, "  ");
					pf_new_sentence(filter);
					lib_print_npc_np(game, associate);
					pf_buffer_string(filter, " is not carrying ");
				} else
					pf_buffer_string(filter, ", ");
				lib_print_object_np(game, trail);
			}
			trail = object;
			count++;

			game->multiple_references[object] = FALSE;
		}

		if (count >= 1) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_new_sentence(filter);
				lib_print_npc_np(game, associate);
				pf_buffer_string(filter, " is not carrying ");
				lib_print_object_np(game, trail);
			} else {
				pf_buffer_string(filter, " or ");
				lib_print_object_np(game, trail);
			}
			pf_buffer_character(filter, '!');
		}
		has_printed |= count > 0;

		/*
		 * Merge any remaining object references into multiple references,
		 * so that succeeding code complains about the inability to acquire
		 * these objects.
		 */
		for (object = 0; object < object_count; object++) {
			game->multiple_references[object] |= game->object_references[object];
			game->object_references[object] = FALSE;
		}
	}

	/*
	 * The remainder of this routine is common error reporting for both object
	 * and NPC associates (and also for no associates).
	 */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->multiple_references[object])
			continue;

		if (gs_object_position(game, object) != OBJ_HELD_PLAYER)
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You've already got ",
				                                     "I've already got ",
				                                     "%player% already has "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->multiple_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You've already got ",
			                                     "I've already got ",
			                                     "%player% already has "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '!');
	}
	has_printed |= count > 0;

	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->multiple_references[object])
			continue;

		if (gs_object_position(game, object) != OBJ_WORN_PLAYER)
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You're already wearing ",
				                                     "I'm already wearing ",
				                                     "%player% is already wearing "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->multiple_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You're already wearing ",
			                                     "I'm already wearing ",
			                                     "%player% is already wearing "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '!');
	}
	has_printed |= count > 0;

	for (npc = 0; npc < gs_npc_count(game); npc++) {
		count = 0;
		trail = -1;
		for (object = 0; object < object_count; object++) {
			if (!game->multiple_references[object])
				continue;

			if (gs_object_position(game, object) != OBJ_HELD_NPC
			        && gs_object_position(game, object) != OBJ_WORN_NPC)
				continue;
			if (gs_object_parent(game, object) != npc)
				continue;

			if (count > 0) {
				if (count == 1) {
					if (has_printed)
						pf_buffer_string(filter, "  ");
					pf_new_sentence(filter);
					lib_print_npc_np(game, gs_object_parent(game, trail));
					pf_buffer_string(filter,
					                 lib_select_response(game,
					                                     " refuses to give you ",
					                                     " refuses to give me ",
					                                     " refuses to give %player% "));
				} else
					pf_buffer_string(filter, ", ");
				lib_print_object_np(game, trail);
			}
			trail = object;
			count++;

			game->multiple_references[object] = FALSE;
		}

		if (count >= 1) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_new_sentence(filter);
				lib_print_npc_np(game, gs_object_parent(game, trail));
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     " refuses to give you ",
				                                     " refuses to give me ",
				                                     " refuses to give %player% "));
			} else
				pf_buffer_string(filter, " and ");
			lib_print_object_np(game, trail);
			pf_buffer_character(filter, '!');
		}
		has_printed |= count > 0;
	}

	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->multiple_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You can't take ",
				                                     "I can't take ",
				                                     "%player% can't take "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->multiple_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You can't take ",
			                                     "I can't take ",
			                                     "%player% can't take "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '!');
	}
}


/*
 * lib_take_backend()
 * lib_take_from_object_backend()
 * lib_take_from_npc_backend()
 *
 * Facets of lib_take_backend_common().  Provide backend handling for either
 * the plain "take" handlers, or the "take from <something>" handlers.
 */
static void lib_take_backend(sc_gameref_t game) {
	lib_take_backend_common(game, -1, FALSE, FALSE);
}

static void lib_take_from_object_backend(sc_gameref_t game, sc_int associate) {
	lib_take_backend_common(game, associate, TRUE, FALSE);
}

static void lib_take_from_npc_backend(sc_gameref_t game, sc_int associate) {
	lib_take_backend_common(game, associate, FALSE, TRUE);
}


/*
 * lib_take_filter()
 * lib_take_not_associated_filter()
 *
 * Helper functions for deciding if an object may be acquired in this context.
 * Returns TRUE if an object may be acquired, FALSE otherwise.
 */
static sc_bool lib_take_filter(sc_gameref_t game, sc_int object, sc_int unused) {
	assert(unused == -1);

	/*
	 * To be take-able, an object must be visible in the room, not static,
	 * and not already held or worn by the player or an NPC.
	 */
	return obj_indirectly_in_room(game, object, gs_playerroom(game))
	       && !obj_is_static(game, object)
	       && !(gs_object_position(game, object) == OBJ_HELD_PLAYER
	            || gs_object_position(game, object) == OBJ_WORN_PLAYER)
	       && !(gs_object_position(game, object) == OBJ_HELD_NPC
	            || gs_object_position(game, object) == OBJ_WORN_NPC);
}

static sc_bool lib_take_not_associated_filter(sc_gameref_t game, sc_int object, sc_int unused) {
	assert(unused == -1);

	/* In addition to other checks, the object may not be in or on an object. */
	return lib_take_filter(game, object, -1)
	       && !(gs_object_position(game, object) == OBJ_ON_OBJECT
	            || gs_object_position(game, object) == OBJ_IN_OBJECT);
}


/*
 * lib_cmd_take_all()
 *
 * Attempt to take all objects currently visible to the player.
 */
sc_bool lib_cmd_take_all(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects;

	/* Filter objects into references, then handle with the backend. */
	gs_set_multiple_references(game);
	objects = lib_apply_multiple_filter(game,
	                                    lib_take_not_associated_filter, -1,
	                                    NULL);
	gs_clear_multiple_references(game);
	if (objects > 0)
		lib_take_backend(game);
	else
		pf_buffer_string(filter, "There is nothing to pick up here.");

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_take_except_multiple()
 *
 * Take all objects currently available to the player, excepting those listed
 * in %text%.
 */
sc_bool lib_cmd_take_except_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects, references;

	/* Parse the multiple objects list to find leave target objects. */
	if (!lib_parse_multiple_objects(game, "leave",
	                                lib_take_not_associated_filter, -1,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_except_filter(game,
	                                  lib_take_not_associated_filter, -1,
	                                  &references);
	if (objects > 0 || references > 0)
		lib_take_backend(game);
	else {
		if (objects == 0)
			pf_buffer_string(filter, "There is nothing else to pick up here.");
		else
			pf_buffer_string(filter, "There is nothing to pick up here.");
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_take_multiple()
 *
 * Take all objects currently available to the player and listed in %text%.
 */
sc_bool lib_cmd_take_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects, references;

	/* Parse the multiple objects list to find take target objects. */
	if (!lib_parse_multiple_objects(game, "take",
	                                lib_take_filter, -1,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_multiple_filter(game,
	                                    lib_take_filter, -1,
	                                    &references);
	if (objects > 0 || references > 0)
		lib_take_backend(game);
	else
		pf_buffer_string(filter, "There is nothing to pick up here.");

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_take_from_filter()
 *
 * Helper function for deciding if an object may be acquired in this context.
 * Returns TRUE if an object may be acquired, FALSE otherwise.
 */
static sc_bool lib_take_from_filter(sc_gameref_t game, sc_int object, sc_int associate) {
	/*
	 * To be take-able, an object must be either inside or on the specified
	 * object.
	 */
	return (gs_object_position(game, object) == OBJ_IN_OBJECT
	        || gs_object_position(game, object) == OBJ_ON_OBJECT)
	       && !obj_is_static(game, object)
	       && gs_object_parent(game, object) == associate;
}


/*
 * lib_take_from_empty()
 *
 * Common error handling for when nothing is taken from a container or
 * supporter object.
 */
static void lib_take_from_empty(sc_gameref_t game, sc_int associate, sc_bool is_except) {
	const sc_filterref_t filter = gs_get_filter(game);

	if (obj_is_container(game, associate) && obj_is_surface(game, associate)) {
		if (gs_object_openness(game, associate) <= OBJ_OPEN) {
			if (is_except)
				pf_buffer_string(filter, "There is nothing else in or on ");
			else
				pf_buffer_string(filter, "There is nothing in or on ");
			lib_print_object_np(game, associate);
			pf_buffer_character(filter, '.');
		} else {
			if (is_except)
				pf_buffer_string(filter, "There is nothing else on ");
			else
				pf_buffer_string(filter, "There is nothing on ");
			lib_print_object_np(game, associate);
			if (gs_object_openness(game, associate) == OBJ_LOCKED)
				pf_buffer_string(filter, " and it is locked.");
			else
				pf_buffer_string(filter, " and it is closed.");
		}
	} else {
		if (obj_is_container(game, associate)) {
			if (gs_object_openness(game, associate) <= OBJ_OPEN) {
				if (is_except)
					pf_buffer_string(filter, "There is nothing else inside ");
				else
					pf_buffer_string(filter, "There is nothing inside ");
				lib_print_object_np(game, associate);
				pf_buffer_character(filter, '.');
			} else {
				pf_new_sentence(filter);
				lib_print_object_np(game, associate);
				pf_buffer_string(filter,
				                 lib_select_plurality(game, associate,
				                                      " is ", " are "));
				if (gs_object_openness(game, associate) == OBJ_LOCKED)
					pf_buffer_string(filter, "locked.");
				else
					pf_buffer_string(filter, "closed.");
			}
		} else {
			if (is_except)
				pf_buffer_string(filter, "There is nothing else on ");
			else
				pf_buffer_string(filter, "There is nothing on ");
			lib_print_object_np(game, associate);
			pf_buffer_character(filter, '.');
		}
	}
}


/*
 * lib_take_from_is_valid()
 *
 * Validate the supporter requested in "take from" commands.
 */
static sc_bool lib_take_from_is_valid(sc_gameref_t game, sc_int associate) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Disallow emptying non-container/non-surface objects. */
	if (!(obj_is_container(game, associate)
	        || obj_is_surface(game, associate))) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't take anything from ",
		                                     "I can't take anything from ",
		                                     "%player% can't take anything from "));
		lib_print_object_np(game, associate);
		pf_buffer_string(filter, ".\n");
		return FALSE;
	}

	/* If object is a container, and is closed, reject now. */
	if (obj_is_container(game, associate)
	        && gs_object_openness(game, associate) > OBJ_OPEN) {
		pf_new_sentence(filter);
		lib_print_object_np(game, associate);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, associate,
		                                      " is closed.\n",
		                                      " are closed.\n"));
		return FALSE;
	}

	/* Associate is a valid target for "take from". */
	return TRUE;
}


/*
 * lib_cmd_take_all_from()
 *
 * Attempt to take all objects contained in or supported by a given object.
 */
sc_bool lib_cmd_take_all_from(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int associate, objects;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	associate = lib_disambiguate_object(game, "take from", &is_ambiguous);
	if (associate == -1)
		return is_ambiguous;

	/* Validate the associate object to take from. */
	if (!lib_take_from_is_valid(game, associate))
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	gs_set_multiple_references(game);
	objects = lib_apply_multiple_filter(game,
	                                    lib_take_from_filter, associate,
	                                    NULL);
	gs_clear_multiple_references(game);
	if (objects > 0)
		lib_take_from_object_backend(game, associate);
	else
		lib_take_from_empty(game, associate, FALSE);

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_take_from_except_multiple()
 *
 * Take all objects contained in or supported by a given object, excepting
 * those listed in %text%.
 */
sc_bool lib_cmd_take_from_except_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int associate, objects, references;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	associate = lib_disambiguate_object(game, "take from", &is_ambiguous);
	if (associate == -1)
		return is_ambiguous;

	/* Parse the multiple objects list to find leave target objects. */
	if (!lib_parse_multiple_objects(game, "leave",
	                                lib_take_from_filter, associate,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Validate the associate object to take from. */
	if (!lib_take_from_is_valid(game, associate))
		return TRUE;

	/* As a special case, complain about requests to retain the associate. */
	if (game->multiple_references[associate]) {
		pf_buffer_string(filter,
		                 "I only understood you as far as wanting to leave ");
		lib_print_object_np(game, associate);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_except_filter(game,
	                                  lib_take_from_filter, associate,
	                                  &references);
	if (objects > 0 || references > 0)
		lib_take_from_object_backend(game, associate);
	else
		lib_take_from_empty(game, associate, TRUE);

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_take_from_multiple()
 *
 * Take the objects currently inside an object and listed in %text%.  This
 * function isn't mandatory -- plain "take <object>" works fine with contain-
 * ers and surfaces, but it's a standard in Adrift so here it is.
 */
sc_bool lib_cmd_take_from_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int associate, objects, references;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	associate = lib_disambiguate_object(game, "take from", &is_ambiguous);
	if (associate == -1)
		return is_ambiguous;

	/* Parse the multiple objects list to find take target objects. */
	if (!lib_parse_multiple_objects(game, "take",
	                                lib_take_from_filter, associate,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Validate the associate object to take from. */
	if (!lib_take_from_is_valid(game, associate))
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_multiple_filter(game,
	                                    lib_take_from_filter, associate,
	                                    &references);
	if (objects > 0 || references > 0)
		lib_take_from_object_backend(game, associate);
	else
		lib_take_from_empty(game, associate, FALSE);

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_take_from_npc_filter()
 *
 * Helper function for deciding if an object may be acquired in this context.
 * Returns TRUE if an object may be acquired, FALSE otherwise.
 */
static sc_bool lib_take_from_npc_filter(sc_gameref_t game, sc_int object, sc_int associate) {
	/*
	 * To be take-able, an object must be either held or worn by the specified
	 * NPC.
	 */
	return (gs_object_position(game, object) == OBJ_HELD_NPC
	        || gs_object_position(game, object) == OBJ_WORN_NPC)
	       && !obj_is_static(game, object)
	       && gs_object_parent(game, object) == associate;
}


/*
 * lib_cmd_take_all_from_npc()
 *
 * Attempt to take all objects held or worn by a given NPC.
 */
sc_bool lib_cmd_take_all_from_npc(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int associate, objects;
	sc_bool is_ambiguous;

	/* Get the referenced NPC, and if none, consider complete. */
	associate = lib_disambiguate_npc(game, "take from", &is_ambiguous);
	if (associate == -1)
		return is_ambiguous;

	/* Filter objects into references, then handle with the backend. */
	gs_set_multiple_references(game);
	objects = lib_apply_multiple_filter(game,
	                                    lib_take_from_npc_filter, associate,
	                                    NULL);
	gs_clear_multiple_references(game);
	if (objects > 0)
		lib_take_from_npc_backend(game, associate);
	else {
		pf_new_sentence(filter);
		lib_print_npc_np(game, associate);
		pf_buffer_string(filter, " is not carrying anything!");
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_take_from_npc_except_multiple()
 *
 * Attempt to take all objects held or worn by a given NPC, excepting those
 * listed in %text%.
 */
sc_bool lib_cmd_take_from_npc_except_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int associate, objects, references;
	sc_bool is_ambiguous;

	/* Get the referenced NPC, and if none, consider complete. */
	associate = lib_disambiguate_npc(game, "take from", &is_ambiguous);
	if (associate == -1)
		return is_ambiguous;

	/* Parse the multiple objects list to find leave target objects. */
	if (!lib_parse_multiple_objects(game, "leave",
	                                lib_take_from_npc_filter, associate,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_except_filter(game,
	                                  lib_take_from_npc_filter, associate,
	                                  &references);
	if (objects > 0 || references > 0)
		lib_take_from_npc_backend(game, associate);
	else {
		pf_new_sentence(filter);
		lib_print_npc_np(game, associate);
		pf_buffer_string(filter, " is not carrying anything else!");
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_take_from_npc_multiple()
 *
 * Attempt to take the objects currently held or worn by an NPC and listed
 * in %text%.
 */
sc_bool lib_cmd_take_from_npc_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int associate, objects, references;
	sc_bool is_ambiguous;

	/* Get the referenced NPC, and if none, consider complete. */
	associate = lib_disambiguate_npc(game, "take from", &is_ambiguous);
	if (associate == -1)
		return is_ambiguous;

	/* Parse the multiple objects list to find take target objects. */
	if (!lib_parse_multiple_objects(game, "take",
	                                lib_take_from_npc_filter, associate,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_multiple_filter(game,
	                                    lib_take_from_npc_filter, associate,
	                                    &references);
	if (objects > 0 || references > 0)
		lib_take_from_npc_backend(game, associate);
	else {
		pf_new_sentence(filter);
		lib_print_npc_np(game, associate);
		pf_buffer_string(filter, " is not carrying anything!");
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_drop_backend()
 *
 * Common backend handler for dropping objects.  Drops all objects currently
 * referenced in the game, trying game commands first, and then moving other
 * unhandled objects to the player room floor.
 *
 * Objects to action are flagged in object_references; objects requested but
 * deemed not actionable are flagged in multiple_references.
 */
static void lib_drop_backend(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object_count, object, count, trail;
	sc_bool has_printed;

	/*
	 * Try game commands for all referenced objects first.  If any succeed,
	 * remove that reference from the list.
	 */
	has_printed = FALSE;
	object_count = gs_object_count(game);
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		if (lib_try_game_command_short(game, "drop", object)) {
			game->object_references[object] = FALSE;
			has_printed = TRUE;
		}
	}

	/* Drop every object that remains referenced. */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You drop ",
				                                     "I drop ",
				                                     "%player% drops "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		gs_object_to_room(game, object, gs_playerroom(game));
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You drop ",
			                                     "I drop ",
			                                     "%player% drops "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '.');
	}
	has_printed |= count > 0;

	/* Note any remaining multiple references left out of the drop operation. */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->multiple_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You are not holding ",
				                                     "I am not holding ",
				                                     "%player% is not holding "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->multiple_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You are not holding ",
			                                     "I am not holding ",
			                                     "%player% is not holding "));
		} else
			pf_buffer_string(filter, " or ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '.');
	}
}


/*
 * lib_drop_filter()
 *
 * Helper function for deciding if an object may be dropped in this context.
 * Returns TRUE if an object may be dropped, FALSE otherwise.
 */
static sc_bool lib_drop_filter(sc_gameref_t game, sc_int object, sc_int unused) {
	assert(unused == -1);

	return !obj_is_static(game, object)
	       && gs_object_position(game, object) == OBJ_HELD_PLAYER;
}


/*
 * lib_cmd_drop_all()
 *
 * Drop all objects currently held by the player.
 */
sc_bool lib_cmd_drop_all(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects;

	/* Filter objects into references, then handle with the backend. */
	gs_set_multiple_references(game);
	objects = lib_apply_multiple_filter(game,
	                                    lib_drop_filter, -1,
	                                    NULL);
	gs_clear_multiple_references(game);
	if (objects > 0)
		lib_drop_backend(game);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You're not carrying anything.",
		                                     "I'm not carrying anything.",
		                                     "%player%'s not carrying anything."));
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_drop_except_multiple()
 *
 * Drop all objects currently held by the player, excepting those listed in
 * %text%.
 */
sc_bool lib_cmd_drop_except_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects, references;

	/* Parse the multiple objects list to find retain target objects. */
	if (!lib_parse_multiple_objects(game, "retain",
	                                lib_drop_filter, -1,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_except_filter(game,
	                                  lib_drop_filter, -1,
	                                  &references);
	if (objects > 0 || references > 0)
		lib_drop_backend(game);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding anything",
		                                     "I am not holding anything",
		                                     "%player% is not holding anything"));
		if (objects == 0)
			pf_buffer_string(filter, " else");
		pf_buffer_character(filter, '.');
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_drop_multiple()
 *
 * Drop all objects currently held by the player and listed in %text%.
 */
sc_bool lib_cmd_drop_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects, references;

	/* Parse the multiple objects list to find drop target objects. */
	if (!lib_parse_multiple_objects(game, "drop",
	                                lib_drop_filter, -1,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_multiple_filter(game,
	                                    lib_drop_filter, -1,
	                                    &references);
	if (objects > 0 || references > 0)
		lib_drop_backend(game);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding anything.",
		                                     "I am not holding anything.",
		                                     "%player% is not holding anything."));
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_give_object_npc()
 * lib_cmd_give_object()
 *
 * Attempt to give an object to an NPC.
 */
sc_bool lib_cmd_give_object_npc(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object, npc;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "give", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Get the referenced npc, and if none, consider complete. */
	npc = lib_disambiguate_npc(game, "give to", NULL);
	if (npc == -1)
		return TRUE;

	/* Reject if not holding the object offered. */
	if (gs_object_position(game, object) != OBJ_HELD_PLAYER) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You don't have ",
		                                     "I don't have ",
		                                     "%player% doesn't have "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, "!\n");
		return TRUE;
	}

	/* After all that, the npc is disinterested. */
	pf_new_sentence(filter);
	lib_print_npc_np(game, npc);
	pf_buffer_string(filter, " doesn't seem interested in ");
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}

sc_bool lib_cmd_give_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "give", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Reject if not holding the object offered. */
	if (gs_object_position(game, object) != OBJ_HELD_PLAYER) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You don't have ",
		                                     "I don't have ",
		                                     "%player% doesn't have "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, "!\n");
		return TRUE;
	}

	/* After all that, we have to ask (and shouldn't this be "to whom?"). */
	pf_buffer_string(filter, "Give ");
	lib_print_object_np(game, object);
	pf_buffer_string(filter, " to who?\n");
	return TRUE;
}


/*
 * lib_wear_backend()
 *
 * Common backend handler for wearing objects.  Puts on all objects currently
 * referenced in the game, moving objects to worn by player.
 *
 * Objects to action are flagged in object_references; objects requested but
 * deemed not actionable are flagged in multiple_references.
 */
static void lib_wear_backend(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object_count, object, count, trail;
	sc_bool has_printed;

	/*
	 * Try game commands for all referenced objects first.  If any succeed,
	 * remove that reference from the list.
	 */
	has_printed = FALSE;
	object_count = gs_object_count(game);
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		if (lib_try_game_command_short(game, "wear", object)) {
			game->object_references[object] = FALSE;
			has_printed = TRUE;
		}
	}

	/* Wear every object referenced. */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You put on ",
				                                     "I put on ",
				                                     "%player% puts on "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		gs_object_player_wear(game, object);
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You put on ",
			                                     "I put on ",
			                                     "%player% puts on "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '.');
	}
	has_printed |= count > 0;

	/* Note any remaining multiple references left out of the wear operation. */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->multiple_references[object])
			continue;

		if (gs_object_position(game, object) != OBJ_WORN_PLAYER)
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You are already wearing ",
				                                     "I am already wearing ",
				                                     "%player% is already wearing "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->multiple_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You are already wearing ",
			                                     "I am already wearing ",
			                                     "%player% is already wearing "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '.');
	}
	has_printed |= count > 0;

	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->multiple_references[object])
			continue;

		if (gs_object_position(game, object) == OBJ_HELD_PLAYER)
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You are not holding ",
				                                     "I am not holding ",
				                                     "%player% is not holding "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->multiple_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You are not holding ",
			                                     "I am not holding ",
			                                     "%player% is not holding "));
		} else
			pf_buffer_string(filter, " or ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '.');
	}
	has_printed |= count > 0;

	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->multiple_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You can't wear ",
				                                     "I can't wear ",
				                                     "%player% can't wear "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->multiple_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You can't wear ",
			                                     "I can't wear ",
			                                     "%player% can't wear "));
		} else
			pf_buffer_string(filter, " or ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '.');
	}
}


/*
 * lib_wear_filter()
 *
 * Helper function for deciding if an object may be worn in this context.
 * Returns TRUE if an object may be worn, FALSE otherwise.
 */
static sc_bool lib_wear_filter(sc_gameref_t game, sc_int object, sc_int unused) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	assert(unused == -1);

	/*
	 * The object is wearable if the player is holding it, and it's not static
	 * (static moved to player inventory by event), and if it's marked wearable
	 * in properties.
	 */
	if (gs_object_position(game, object) == OBJ_HELD_PLAYER
	        && !obj_is_static(game, object)) {
		sc_vartype_t vt_key[3];

		/* Return wearability from the object properties. */
		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Wearable";
		return prop_get_boolean(bundle, "B<-sis", vt_key);
	}

	return FALSE;
}


/*
 * lib_cmd_wear_all()
 *
 * Wear all wearable objects currently held by the player.
 */
sc_bool lib_cmd_wear_all(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects;

	/* Filter objects into references, then handle with the backend. */
	gs_set_multiple_references(game);
	objects = lib_apply_multiple_filter(game,
	                                    lib_wear_filter, -1,
	                                    NULL);
	gs_clear_multiple_references(game);
	if (objects > 0)
		lib_wear_backend(game);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You're not carrying anything",
		                                     "I'm not carrying anything",
		                                     "%player%'s not carrying anything"));
		pf_buffer_string(filter, " that can be worn.");
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_wear_except_multiple()
 *
 * Wear all wearable objects currently held by the player, excepting those
 * listed in %text%.
 */
sc_bool lib_cmd_wear_except_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects, references;

	/* Parse the multiple objects list to find retain target objects. */
	if (!lib_parse_multiple_objects(game, "retain",
	                                lib_wear_filter, -1,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_except_filter(game,
	                                  lib_wear_filter, -1,
	                                  &references);
	if (objects > 0 || references > 0)
		lib_wear_backend(game);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding anything",
		                                     "I am not holding anything",
		                                     "%player% is not holding anything"));
		if (objects == 0)
			pf_buffer_string(filter, " else");
		pf_buffer_string(filter, " that can be worn.");
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_wear_multiple()
 *
 * Wear all objects currently held by the player, wearable, and listed
 * in %text%.
 */
sc_bool lib_cmd_wear_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects, references;

	/* Parse the multiple objects list to find wear target objects. */
	if (!lib_parse_multiple_objects(game, "wear",
	                                lib_wear_filter, -1,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_multiple_filter(game,
	                                    lib_wear_filter, -1,
	                                    &references);
	if (objects > 0 || references > 0)
		lib_wear_backend(game);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding anything",
		                                     "I am not holding anything",
		                                     "%player% is not holding anything"));
		pf_buffer_string(filter, " that can be worn.");
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_remove_backend()
 *
 * Common backend handler for removing objects.  Takes off on all objects
 * currently referenced in the game, moving objects to held by player.
 *
 * Objects to action are flagged in object_references; objects requested but
 * deemed not actionable are flagged in multiple_references.
 */
static void lib_remove_backend(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object_count, object, count, trail;
	sc_bool has_printed;

	/*
	 * Try game commands for all referenced objects first.  If any succeed,
	 * remove that reference from the list.
	 */
	has_printed = FALSE;
	object_count = gs_object_count(game);
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		if (lib_try_game_command_short(game, "remove", object)) {
			game->object_references[object] = FALSE;
			has_printed = TRUE;
		}
	}

	/* Remove every object referenced. */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You remove ",
				                                     "I remove ",
				                                     "%player% removes "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		gs_object_player_get(game, object);
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You remove ",
			                                     "I remove ",
			                                     "%player% removes "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '.');
	}
	has_printed |= count > 0;

	/* Note any remaining multiple references left out of the remove operation. */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->multiple_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You are not wearing ",
				                                     "I am not wearing ",
				                                     "%player% is not wearing "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->multiple_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You are not wearing ",
			                                     "I am not wearing ",
			                                     "%player% is not wearing "));
		} else
			pf_buffer_string(filter, " or ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '!');
	}
}


/*
 * lib_remove_filter()
 *
 * Helper function for deciding if an object may be removed in this context.
 * Returns TRUE if an object is currently being worn, FALSE otherwise.
 */
static sc_bool lib_remove_filter(sc_gameref_t game, sc_int object, sc_int unused) {
	assert(unused == -1);

	return !obj_is_static(game, object)
	       && gs_object_position(game, object) == OBJ_WORN_PLAYER;
}


/*
 * lib_cmd_remove_all()
 *
 * Remove all objects currently held by the player.
 */
sc_bool lib_cmd_remove_all(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects;

	/* Filter objects into references, then handle with the backend. */
	gs_set_multiple_references(game);
	objects = lib_apply_multiple_filter(game,
	                                    lib_remove_filter, -1,
	                                    NULL);
	gs_clear_multiple_references(game);
	if (objects > 0)
		lib_remove_backend(game);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You're not wearing anything",
		                                     "I'm not wearing anything",
		                                     "%player%'s not wearing anything"));
		pf_buffer_string(filter, " that can be removed.");
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_remove_except_multiple()
 *
 * Remove all objects currently worn by the player, excepting those listed
 * in %text%.
 */
sc_bool lib_cmd_remove_except_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects, references;

	/* Parse the multiple objects list to find retain target objects. */
	if (!lib_parse_multiple_objects(game, "retain",
	                                lib_remove_filter, -1,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_except_filter(game,
	                                  lib_remove_filter, -1,
	                                  &references);
	if (objects > 0 || references > 0)
		lib_remove_backend(game);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not wearing anything",
		                                     "I am not wearing anything",
		                                     "%player% is not wearing anything"));
		if (objects == 0)
			pf_buffer_string(filter, " else");
		pf_buffer_string(filter, " that can be removed.");
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_remove_multiple()
 *
 * Remove all objects currently worn by the player, and listed in %text%.
 */
sc_bool lib_cmd_remove_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int objects, references;

	/* Parse the multiple objects list to find remove target objects. */
	if (!lib_parse_multiple_objects(game, "remove",
	                                lib_remove_filter, -1,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_multiple_filter(game,
	                                    lib_remove_filter, -1,
	                                    &references);
	if (objects > 0 || references > 0)
		lib_remove_backend(game);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding anything",
		                                     "I am not holding anything",
		                                     "%player% is not holding anything"));
		pf_buffer_string(filter, " that can be removed.");
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_inventory()
 *
 * List objects carried and worn by the player.
 */
sc_bool lib_cmd_inventory(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object, count, trail;
	sc_bool wearing;

	/* Find and list each object worn by the player. */
	count = 0;
	trail = -1;
	wearing = FALSE;
	for (object = 0; object < gs_object_count(game); object++) {
		if (gs_object_position(game, object) == OBJ_WORN_PLAYER) {
			if (count > 0) {
				if (count == 1) {
					pf_buffer_string(filter,
					                 lib_select_response(game,
					                                     "You are wearing ",
					                                     "I am wearing ",
					                                     "%player% is wearing "));
				} else
					pf_buffer_string(filter, ", ");
				lib_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You are wearing ",
			                                     "I am wearing ",
			                                     "%player% is wearing "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object(game, trail);
		wearing = TRUE;
	}

	/* Find and list each object owned by the player. */
	count = 0;
	for (object = 0; object < gs_object_count(game); object++) {
		if (gs_object_position(game, object) == OBJ_HELD_PLAYER) {
			if (count > 0) {
				if (count == 1) {
					if (wearing) {
						pf_buffer_string(filter,
						                 lib_select_response(game,
						                                     ", and you are carrying ",
						                                     ", and I am carrying ",
						                                     ", and %player% is carrying "));
					} else {
						pf_buffer_string(filter,
						                 lib_select_response(game,
						                                     "You are carrying ",
						                                     "I am carrying ",
						                                     "%player% is carrying "));
					}
				} else
					pf_buffer_string(filter, ", ");
				lib_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			if (wearing) {
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     ", and you are carrying ",
				                                     ", and I am carrying ",
				                                     ", and %player% is carrying "));
			} else {
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You are carrying ",
				                                     "I am carrying ",
				                                     "%player% is carrying "));
			}
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object(game, trail);
		pf_buffer_character(filter, '.');

		/* Print contents of every container and surface carried. */
		for (object = 0; object < gs_object_count(game); object++) {
			if (gs_object_position(game, object) == OBJ_HELD_PLAYER) {
				if (obj_is_container(game, object)
				        && gs_object_openness(game, object) <= OBJ_OPEN)
					lib_list_in_object(game, object, TRUE);

				if (obj_is_surface(game, object))
					lib_list_on_object(game, object, TRUE);
			}
		}
		pf_buffer_character(filter, '\n');
	} else {
		if (wearing) {
			pf_buffer_string(filter, ", and ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "you are carrying nothing.\n",
			                                     "I am carrying nothing.\n",
			                                     "%player% is carrying nothing.\n"));
		} else {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You are carrying nothing.\n",
			                                     "I am carrying nothing.\n",
			                                     "%player% is carrying nothing.\n"));
		}
	}

	/* Successful command. */
	return TRUE;
}


/*
 * lib_cmd_open_object()
 *
 * Attempt to open the referenced object.
 */
sc_bool lib_cmd_open_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object, openness;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "open", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Get the current object openness. */
	openness = gs_object_openness(game, object);

	/* React to the request based on openness state. */
	switch (openness) {
	case OBJ_OPEN:
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object,
		                                      " is already open!\n",
		                                      " are already open!\n"));
		return TRUE;

	case OBJ_CLOSED:
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You open ",
		                                     "I open ",
		                                     "%player% opens "));
		lib_print_object_np(game, object);
		pf_buffer_character(filter, '.');

		/* Set open state, and list contents. */
		gs_set_object_openness(game, object, OBJ_OPEN);
		lib_list_in_object(game, object, TRUE);
		pf_buffer_character(filter, '\n');
		return TRUE;

	case OBJ_LOCKED:
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't open ",
		                                     "I can't open ",
		                                     "%player% can't open "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, " as it is locked!\n");
		return TRUE;

	default:
		break;
	}

	/* The object isn't openable. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You can't open ",
	                                     "I can't open ",
	                                     "%player% can't open "));
	lib_print_object_np(game, object);
	pf_buffer_string(filter, "!\n");
	return TRUE;
}


/*
 * lib_cmd_close_object()
 *
 * Attempt to close the referenced object.
 */
sc_bool lib_cmd_close_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object, openness;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "close", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Get the current object openness. */
	openness = gs_object_openness(game, object);

	/* React to the request based on openness state. */
	switch (openness) {
	case OBJ_OPEN:
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You close ",
		                                     "I close ",
		                                     "%player% closes "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, ".\n");

		/* Set closed state. */
		gs_set_object_openness(game, object, OBJ_CLOSED);
		return TRUE;

	case OBJ_CLOSED:
	case OBJ_LOCKED:
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object,
		                                      " is already closed!\n",
		                                      " are already closed!\n"));
		return TRUE;

	default:
		break;
	}

	/* The object isn't closeable. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You can't close ",
	                                     "I can't close ",
	                                     "%player% can't close "));
	lib_print_object_np(game, object);
	pf_buffer_string(filter, "!\n");
	return TRUE;
}


/*
 * lib_attempt_key_acquisition()
 *
 * Automatically get an object being used as a key, if possible.
 */
static void lib_attempt_key_acquisition(sc_gameref_t game, sc_int object) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Disallow getting static objects. */
	if (obj_is_static(game, object))
		return;

	/* If the object is not seen or available, reject the attempt. */
	if (!(gs_object_seen(game, object)
	        && obj_indirectly_in_room(game, object, gs_playerroom(game))))
		return;

	/*
	 * Check if we already have it, or are wearing it, or if a NPC has or is
	 * wearing it.
	 */
	if (gs_object_position(game, object) == OBJ_HELD_PLAYER
	        || gs_object_position(game, object) == OBJ_WORN_PLAYER
	        || gs_object_position(game, object) == OBJ_HELD_NPC
	        || gs_object_position(game, object) == OBJ_WORN_NPC)
		return;

	/*
	 * If the object is contained in or on something we're already holding,
	 * capacity checks are meaningless.
	 */
	if (!obj_indirectly_held_by_player(game, object)) {
		if (lib_object_too_heavy(game, object, NULL)
		        || lib_object_too_large(game, object, NULL))
			return;
	}

	/* Retry game commands for the object with a standard "get". */
	if (lib_try_game_command_short(game, "get", object))
		return;

	/* Note what we're doing. */
	if (gs_object_position(game, object) == OBJ_IN_OBJECT
	        || gs_object_position(game, object) == OBJ_ON_OBJECT) {
		pf_buffer_string(filter, "(Taking ");
		lib_print_object_np(game, object);

		pf_buffer_string(filter, " from ");
		lib_print_object_np(game, gs_object_parent(game, object));
		pf_buffer_string(filter, " first)\n");
	} else {
		pf_buffer_string(filter, "(Picking up ");
		lib_print_object_np(game, object);
		pf_buffer_string(filter, " first)\n");
	}

	/* Take possession of the object. */
	gs_object_player_get(game, object);
}


/*
 * lib_cmd_unlock_object_with()
 *
 * Attempt to unlock the referenced object.
 */
sc_bool lib_cmd_unlock_object_with(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, key, openness;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "unlock", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/*
	 * Now try to get the key from referenced text, and disambiguate as usual.
	 */
	if (!uip_match("%object%", var_get_ref_text(vars), game)) {
		pf_buffer_string(filter, "What do you want to unlock that with?\n");
		return TRUE;
	}
	key = lib_disambiguate_object(game, "unlock that with", NULL);
	if (key == -1)
		return TRUE;

	/* React to the request based on openness state. */
	openness = gs_object_openness(game, object);
	switch (openness) {
	case OBJ_OPEN:
	case OBJ_CLOSED:
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object,
		                                      " is not locked!\n",
		                                      " are not locked!\n"));
		return TRUE;

	case OBJ_LOCKED: {
		sc_vartype_t vt_key[3];
		sc_int key_index, the_key;

		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Key";
		key_index = prop_get_integer(bundle, "I<-sis", vt_key);
		if (key_index == -1)
			break;

		the_key = obj_dynamic_object(game, key_index);
		if (the_key != key) {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You can't unlock ",
			                                     "I can't unlock ",
			                                     "%player% can't unlock "));
			lib_print_object_np(game, object);
			pf_buffer_string(filter, " with ");
			lib_print_object_np(game, key);
			pf_buffer_string(filter, ".\n");
			return TRUE;
		}

		if (gs_object_position(game, key) != OBJ_HELD_PLAYER) {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You are not holding ",
			                                     "I am not holding ",
			                                     "%player% is not holding "));
			lib_print_object_np(game, key);
			pf_buffer_string(filter, ".\n");
			return TRUE;
		}

		gs_set_object_openness(game, object, OBJ_CLOSED);
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You unlock ",
		                                     "I unlock ",
		                                     "%player% unlocks "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, " with ");
		lib_print_object_np(game, key);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	default:
		break;
	}

	/* The object isn't lockable. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You can't unlock ",
	                                     "I can't unlock ",
	                                     "%player% can't unlock "));
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}


/*
 * lib_cmd_unlock_object()
 *
 * Attempt to unlock the referenced object, automatically selecting key.
 */
sc_bool lib_cmd_unlock_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, openness;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "unlock", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* React to the request based on openness state. */
	openness = gs_object_openness(game, object);
	switch (openness) {
	case OBJ_OPEN:
	case OBJ_CLOSED:
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object,
		                                      " is not locked!\n",
		                                      " are not locked!\n"));
		return TRUE;

	case OBJ_LOCKED: {
		sc_vartype_t vt_key[3];
		sc_int key_index, key;

		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Key";
		key_index = prop_get_integer(bundle, "I<-sis", vt_key);
		if (key_index == -1)
			break;

		key = obj_dynamic_object(game, key_index);
		lib_attempt_key_acquisition(game, key);
		if (gs_object_position(game, key) != OBJ_HELD_PLAYER) {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You don't have",
			                                     "I don't have",
			                                     "%player% doesn't have"));
			pf_buffer_string(filter, " anything to unlock ");
			lib_print_object_np(game, object);
			pf_buffer_string(filter, " with!\n");
			return TRUE;
		}

		gs_set_object_openness(game, object, OBJ_CLOSED);
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You unlock ",
		                                     "I unlock ",
		                                     "%player% unlocks "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, " with ");
		lib_print_object_np(game, key);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	default:
		break;
	}

	/* The object isn't lockable. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You can't unlock ",
	                                     "I can't unlock ",
	                                     "%player% can't unlock "));
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}


/*
 * lib_cmd_lock_object_with()
 *
 * Attempt to lock the referenced object.
 */
sc_bool lib_cmd_lock_object_with(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, key, openness;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "lock", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/*
	 * Now try to get the key from referenced text, and disambiguate as usual.
	 */
	if (!uip_match("%object%", var_get_ref_text(vars), game)) {
		pf_buffer_string(filter, "What do you want to lock that with?\n");
		return TRUE;
	}
	key = lib_disambiguate_object(game, "lock that with", NULL);
	if (key == -1)
		return TRUE;

	/* React to the request based on openness state. */
	openness = gs_object_openness(game, object);
	switch (openness) {
	case OBJ_OPEN:
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't lock ",
		                                     "I can't lock ",
		                                     "%player% can't lock "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, " as it is open.\n");
		return TRUE;

	case OBJ_CLOSED: {
		sc_vartype_t vt_key[3];
		sc_int key_index, the_key;

		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Key";
		key_index = prop_get_integer(bundle, "I<-sis", vt_key);
		if (key_index == -1)
			break;

		the_key = obj_dynamic_object(game, key_index);
		if (the_key != key) {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You can't lock ",
			                                     "I can't lock ",
			                                     "%player% can't lock "));
			lib_print_object_np(game, object);
			pf_buffer_string(filter, " with ");
			lib_print_object_np(game, key);
			pf_buffer_string(filter, ".\n");
			return TRUE;
		}

		if (gs_object_position(game, key) != OBJ_HELD_PLAYER) {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You are not holding ",
			                                     "I am not holding ",
			                                     "%player% is not holding "));
			lib_print_object_np(game, key);
			pf_buffer_string(filter, ".\n");
			return TRUE;
		}

		gs_set_object_openness(game, object, OBJ_LOCKED);
		pf_buffer_string(filter, lib_select_response(game,
		                 "You lock ",
		                 "I lock ",
		                 "%player% locks "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, " with ");
		lib_print_object_np(game, key);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	case OBJ_LOCKED:
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object,
		                                      " is already locked!\n",
		                                      " are already locked!\n"));
		return TRUE;

	default:
		break;
	}

	/* The object isn't lockable. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You can't lock ",
	                                     "I can't lock ",
	                                     "%player% can't lock "));
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}


/*
 * lib_cmd_lock_object()
 *
 * Attempt to lock the referenced object, automatically selecting key.
 */
sc_bool lib_cmd_lock_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, openness;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "lock", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* React to the request based on openness state. */
	openness = gs_object_openness(game, object);
	switch (openness) {
	case OBJ_OPEN:
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't lock ",
		                                     "I can't lock ",
		                                     "%player% can't lock "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, " as it is open.\n");
		return TRUE;

	case OBJ_CLOSED: {
		sc_vartype_t vt_key[3];
		sc_int key_index, key;

		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Key";
		key_index = prop_get_integer(bundle, "I<-sis", vt_key);
		if (key_index == -1)
			break;

		key = obj_dynamic_object(game, key_index);
		lib_attempt_key_acquisition(game, key);
		if (gs_object_position(game, key) != OBJ_HELD_PLAYER) {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You don't have",
			                                     "I don't have",
			                                     "%player% doesn't have"));
			pf_buffer_string(filter, " anything to lock ");
			lib_print_object_np(game, object);
			pf_buffer_string(filter, " with!\n");
			return TRUE;
		}

		gs_set_object_openness(game, object, OBJ_LOCKED);
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You lock ",
		                                     "I lock ",
		                                     "%player% locks "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, " with ");
		lib_print_object_np(game, key);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	case OBJ_LOCKED:
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object,
		                                      " is already locked!\n",
		                                      " are already locked!\n"));
		return TRUE;

	default:
		break;
	}

	/* The object isn't lockable. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You can't lock ",
	                                     "I can't lock ",
	                                     "%player% can't lock "));
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}


/*
 * lib_compare_subject()
 *
 * Compare a subject, comma or NUL terminated.  Helper for ask.
 */
static sc_bool lib_compare_subject(const sc_char *subject, sc_int posn, const sc_char *string) {
	sc_int word_posn, string_posn;

	/* Skip any leading subject spaces. */
	for (word_posn = posn;
	        subject[word_posn] != NUL && sc_isspace(subject[word_posn]);)
		word_posn++;
	for (string_posn = 0;
	        string[string_posn] != NUL && sc_isspace(string[string_posn]);)
		string_posn++;

	/* Match characters from words with the string at position. */
	while (TRUE) {
		/* Any character mismatch means no match. */
		if (sc_tolower(subject[word_posn]) != sc_tolower(string[string_posn]))
			return FALSE;

		/* Move to next character in each. */
		word_posn++;
		string_posn++;

		/*
		 * If at space, advance over whitespace in subjects list.  Stop when we
		 * hit the end of the element or list.
		 */
		while (sc_isspace(subject[word_posn])
		        && subject[word_posn] != COMMA && subject[word_posn] != NUL)
			subject++;

		/* Advance over whitespace in the current string too. */
		while (sc_isspace(string[string_posn]) && string[string_posn] != NUL)
			string_posn++;

		/*
		 * If we found the end of the subject, and the end of the current string,
		 * we've matched.  If not at the end of the current string, though, only
		 * a partial match.
		 */
		if (subject[word_posn] == NUL || subject[word_posn] == COMMA) {
			if (string[string_posn] == NUL)
				break;
			else
				return FALSE;
		}
	}

	/* Matched in the loop; return TRUE. */
	return TRUE;
}


/*
 * lib_npc_reply_to()
 *
 * Reply for an NPC on a given topic.  Helper for ask.
 */
static sc_bool lib_npc_reply_to(sc_gameref_t game, sc_int npc, sc_int topic) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int task;
	const sc_char *response;

	/* Find any associated task to control response. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Topics";
	vt_key[3].integer = topic;
	vt_key[4].string = "Task";
	task = prop_get_integer(bundle, "I<-sisis", vt_key);

	/* Get the response, and print if anything there. */
	if (task > 0 && gs_task_done(game, task - 1))
		vt_key[4].string = "AltReply";
	else
		vt_key[4].string = "Reply";
	response = prop_get_string(bundle, "S<-sisis", vt_key);
	if (!sc_strempty(response)) {
		pf_buffer_string(filter, response);
		pf_buffer_character(filter, '\n');
		return TRUE;
	}

	/* No response to this combination. */
	return FALSE;
}


/*
 * lib_cmd_ask_npc_about()
 *
 * Converse with NPC.
 */
sc_bool lib_cmd_ask_npc_about(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int npc, topic_count, topic, topic_match, default_topic;
	sc_bool found, default_found, is_ambiguous;

	/* Get the referenced npc, and if none, consider complete. */
	npc = lib_disambiguate_npc(game, "ask", &is_ambiguous);
	if (npc == -1)
		return is_ambiguous;

	if (lib_trace)
		sc_trace("Library: asking NPC %ld\n", npc);

	/* Get the topics the NPC converses about. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Topics";
	topic_count = prop_get_child_count(bundle, "I<-sis", vt_key);
	topic_match = default_topic = -1;
	found = default_found = FALSE;
	for (topic = 0; topic < topic_count; topic++) {
		const sc_char *subjects;
		sc_int posn;

		/* Get subject list for this topic. */
		vt_key[3].integer = topic;
		vt_key[4].string = "Subject";
		subjects = prop_get_string(bundle, "S<-sisis", vt_key);

		/* If this is the special "*" topic, note and continue. */
		if (!sc_strcasecmp(subjects, "*")) {
			if (lib_trace)
				sc_trace("Library: \"*\" is %ld\n", topic);

			default_topic = topic;
			default_found = TRUE;
			continue;
		}

		/* Split into subjects by comma delimiter. */
		for (posn = 0; subjects[posn] != NUL;) {
			if (lib_trace)
				sc_trace("Library: subject %s[%ld]\n", subjects, posn);

			/* See if this subject matches. */
			if (lib_compare_subject(subjects, posn, var_get_ref_text(vars))) {
				if (lib_trace)
					sc_trace("Library: matched\n");

				topic_match = topic;
				found = TRUE;
				break;
			}

			/* Move to next subject, or end of list. */
			while (subjects[posn] != COMMA && subjects[posn] != NUL)
				posn++;
			if (subjects[posn] == COMMA)
				posn++;
		}
	}

	/* Handle any matched subject first, and "*" second. */
	if (found && lib_npc_reply_to(game, npc, topic_match))
		return TRUE;
	else if (default_found && lib_npc_reply_to(game, npc, default_topic))
		return TRUE;

	/* NPC has no response. */
	pf_new_sentence(filter);
	lib_print_npc_np(game, npc);
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     " does not respond to your question.\n",
	                                     " does not respond to my question.\n",
	                                     " does not respond to %player%'s question.\n"));
	return TRUE;
}


/*
 * lib_check_put_in_recursion()
 *
 * Checks for infinite recursion when placing an object in an object.  Returns
 * TRUE if no recursion detected.
 */
static sc_bool lib_check_put_in_recursion(sc_gameref_t game, sc_int object,
		sc_int container, sc_bool report) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int check;

	/* Avoid the obvious possibility of infinite recursion. */
	if (container == object) {
		if (report) {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You can't put an object inside itself!",
			                                     "I can't put an object inside itself!",
			                                     "%player% can't put an object inside itself!"));
		}
		return FALSE;
	}

	/* Avoid the subtle possibility of infinite recursion. */
	check = container;
	while (gs_object_position(game, check) == OBJ_ON_OBJECT
	        || gs_object_position(game, check) == OBJ_IN_OBJECT) {
		check = gs_object_parent(game, check);
		if (check == object) {
			if (report) {
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You can't put an object inside one",
				                                     "I can't put an object inside one",
				                                     "%player% can't put an object inside one"));
				pf_buffer_string(filter, " it's on or in!");
			}
			return FALSE;
		}
	}

	/* No infinite recursion detected. */
	return TRUE;
}


/*
 * lib_put_in_backend()
 *
 * Common backend handler for placing objects in containers.  Places all
 * objects currently referenced in the game into a container, trying game
 * commands first, and then moving other unhandled objects into the container.
 *
 * Objects to action are flagged in object_references; objects requested but
 * deemed not actionable are flagged in multiple_references.
 */
static void lib_put_in_backend(sc_gameref_t game, sc_int container) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object_count, object, count, trail, capacity, maxsize;
	sc_bool has_printed;

	/*
	 * Try game commands for all referenced objects first.  If any succeed,
	 * remove that reference from the list.  At the same time, check for and
	 * weed out any moves that result in infinite recursion.
	 */
	has_printed = FALSE;
	object_count = gs_object_count(game);
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		/* Reject and remove attempts to place objects in themselves. */
		if (!lib_check_put_in_recursion(game, object, container, !has_printed)) {
			game->object_references[object] = FALSE;
			has_printed = TRUE;
			continue;
		}

		if (lib_try_game_command_with_object(game,
		                                     "put", object, "in", container)) {
			game->object_references[object] = FALSE;
			has_printed = TRUE;
		}
	}

	/* Retrieve the container's limits. */
	maxsize = obj_get_container_maxsize(game, container);
	capacity = obj_get_container_capacity(game, container);

	/* Put in every object that remains referenced. */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		/* If too big, or exceeds container limits, ignore for now. */
		if (obj_get_size(game, object) > maxsize)
			continue;
		else {
			sc_int other, contains;

			contains = 0;
			for (other = 0; other < gs_object_count(game); other++) {
				if (gs_object_position(game, other) == OBJ_IN_OBJECT
				        && gs_object_parent(game, other) == container)
					contains++;
			}
			if (contains >= capacity)
				continue;
		}

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You put ",
				                                     "I put ",
				                                     "%player% puts "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		gs_object_move_into(game, object, container);
		game->object_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You put ",
			                                     "I put ",
			                                     "%player% puts "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object_np(game, trail);
		pf_buffer_string(filter, " inside ");
		lib_print_object_np(game, container);
		pf_buffer_character(filter, '.');
	}
	has_printed |= count > 0;

	/*
	 * Report objects not put in because of their size.  These objects remain in
	 * standard references, as do objects rejected because of capacity limits.
	 * By removing too large objects in this loop, we're left later on with just
	 * the objects rejected by capacity limits.
	 */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		if (!(obj_get_size(game, object) > maxsize))
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_new_sentence(filter);
				lib_print_object_np(game, trail);
			} else
				pf_buffer_string(filter, ", ");
		}
		trail = object;
		count++;

		game->object_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_new_sentence(filter);
			lib_print_object_np(game, trail);
			pf_buffer_string(filter,
			                 lib_select_plurality(game, trail,
			                                      " is too big",
			                                      " are too big"));
		} else {
			pf_buffer_string(filter, " and ");
			lib_print_object_np(game, trail);
			pf_buffer_string(filter, " are too big");
		}
		pf_buffer_string(filter, " to fit inside ");
		lib_print_object_np(game, container);
		pf_buffer_character(filter, '.');
	}
	has_printed |= count > 0;

	/*
	 * Report objects not put in because the container is too full.  This should
	 * be all remaining objects in standard references.
	 */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_new_sentence(filter);
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->object_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_new_sentence(filter);
			lib_print_object_np(game, trail);
		} else {
			pf_buffer_string(filter, " and ");
			lib_print_object_np(game, trail);
		}
		pf_buffer_string(filter, " can't fit inside ");
		lib_print_object_np(game, container);
		pf_buffer_string(filter, " at the moment.");
	}
	has_printed |= count > 0;

	/* Note any remaining multiple references left out of the operation. */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->multiple_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You are not holding ",
				                                     "I am not holding ",
				                                     "%player% is not holding "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->multiple_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You are not holding ",
			                                     "I am not holding ",
			                                     "%player% is not holding "));
		} else
			pf_buffer_string(filter, " or ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '.');
	}
}


/*
 * lib_put_in_filter()
 * lib_put_in_not_container_filter()
 *
 * Helper functions for deciding if an object may be put in another this
 * context.  Returns TRUE if an object may be manipulated, FALSE otherwise.
 */
static sc_bool lib_put_in_filter(sc_gameref_t game, sc_int object, sc_int unused) {
	assert(unused == -1);

	return !obj_is_static(game, object)
	       && gs_object_position(game, object) == OBJ_HELD_PLAYER;
}

static sc_bool lib_put_in_not_container_filter(sc_gameref_t game, sc_int object, sc_int container) {
	return lib_put_in_filter(game, object, -1) && object != container;
}


/*
 * lib_put_in_is_valid()
 *
 * Validate the container requested in "put in" commands.
 */
static sc_bool lib_put_in_is_valid(sc_gameref_t game, sc_int container) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Verify that the container object is a container. */
	if (!obj_is_container(game, container)) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't put anything inside ",
		                                     "I can't put anything inside ",
		                                     "%player% can't put anything inside "));
		lib_print_object_np(game, container);
		pf_buffer_string(filter, "!\n");
		return FALSE;
	}

	/* If the container is closed, reject now. */
	if (gs_object_openness(game, container) > OBJ_OPEN) {
		pf_new_sentence(filter);
		lib_print_object_np(game, container);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, container, " is", " are"));
		if (gs_object_openness(game, container) == OBJ_LOCKED)
			pf_buffer_string(filter, " locked!\n");
		else
			pf_buffer_string(filter, " closed!\n");
		return FALSE;
	}

	/* Container is a valid target for "put in". */
	return TRUE;
}


/*
 * lib_cmd_put_all_in()
 *
 * Put all objects currently held by the player into a container.
 */
sc_bool lib_cmd_put_all_in(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int container, objects;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	container = lib_disambiguate_object(game, "put that into", &is_ambiguous);
	if (container == -1)
		return is_ambiguous;

	/* Validate the container object to take from. */
	if (!lib_put_in_is_valid(game, container))
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	gs_set_multiple_references(game);
	objects = lib_apply_multiple_filter(game,
	                                    lib_put_in_not_container_filter,
	                                    container, NULL);
	gs_clear_multiple_references(game);
	if (objects > 0)
		lib_put_in_backend(game, container);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You're not carrying anything",
		                                     "I'm not carrying anything",
		                                     "%player%'s not carrying anything"));
		if (obj_indirectly_held_by_player(game, container))
			pf_buffer_string(filter, " else");
		pf_buffer_character(filter, '.');
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_put_in_except_multiple()
 *
 * Put all objects currently held by the player into an object, excepting
 * those listed in %text%.
 */
sc_bool lib_cmd_put_in_except_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int container, objects, references;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	container = lib_disambiguate_object(game, "put that into", &is_ambiguous);
	if (container == -1)
		return is_ambiguous;

	/* Parse the multiple objects list to find retain target objects. */
	if (!lib_parse_multiple_objects(game, "retain",
	                                lib_put_in_not_container_filter,
	                                container, &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Validate the container object to put into. */
	if (!lib_put_in_is_valid(game, container))
		return TRUE;

	/* As a special case, complain about requests to retain the container. */
	if (game->multiple_references[container]) {
		pf_buffer_string(filter,
		                 "I only understood you as far as wanting to retain ");
		lib_print_object_np(game, container);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_except_filter(game,
	                                  lib_put_in_not_container_filter,
	                                  container, &references);
	if (objects > 0 || references > 0)
		lib_put_in_backend(game, container);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding anything",
		                                     "I am not holding anything",
		                                     "%player% is not holding anything"));
		if (objects == 0)
			pf_buffer_string(filter, " else");
		pf_buffer_character(filter, '.');
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_put_in_multiple()
 *
 * Put all objects currently held by the player and listed in %text% into an
 * object.
 */
sc_bool lib_cmd_put_in_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int container, objects, references;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	container = lib_disambiguate_object(game, "put that into", &is_ambiguous);
	if (container == -1)
		return is_ambiguous;

	/* Parse the multiple objects list to find retain target objects. */
	if (!lib_parse_multiple_objects(game, "move",
	                                lib_put_in_filter, -1,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Validate the container object to put into. */
	if (!lib_put_in_is_valid(game, container))
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_multiple_filter(game,
	                                    lib_put_in_filter, -1,
	                                    &references);
	if (objects > 0 || references > 0)
		lib_put_in_backend(game, container);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding anything.",
		                                     "I am not holding anything.",
		                                     "%player% is not holding anything."));
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_check_put_on_recursion()
 *
 * Checks for infinite recursion when placing an object on an object.  Returns
 * TRUE if no recursion detected.
 */
static sc_bool lib_check_put_on_recursion(sc_gameref_t game, sc_int object,
		sc_int supporter, sc_bool report) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int check;

	/* Avoid the obvious possibility of infinite recursion. */
	if (supporter == object) {
		if (report) {
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You can't put an object onto itself!",
			                                     "I can't put an object onto itself!",
			                                     "%player% can't put an object onto itself!"));
		}
		return FALSE;
	}

	/* Avoid the subtle possibility of infinite recursion. */
	check = supporter;
	while (gs_object_position(game, check) == OBJ_ON_OBJECT
	        || gs_object_position(game, check) == OBJ_IN_OBJECT) {
		check = gs_object_parent(game, check);
		if (check == object) {
			if (report) {
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You can't put an object onto one",
				                                     "I can't put an object onto one",
				                                     "%player% can't put an object onto one"));
				pf_buffer_string(filter, " it's on or in!");
			}
			return FALSE;
		}
	}

	/* No infinite recursion detected. */
	return TRUE;
}


/*
 * lib_put_on_backend()
 *
 * Common backend handler for placing objects on supporters.  Places all
 * objects currently referenced in the game onto a supporter, trying game
 * commands first, and then moving other unhandled objects onto the supporter.
 *
 * Objects to action are flagged in object_references; objects requested but
 * deemed not actionable are flagged in multiple_references.
 */
static void lib_put_on_backend(sc_gameref_t game, sc_int supporter) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object_count, object, count, trail;
	sc_bool has_printed;

	/*
	 * Try game commands for all referenced objects first.  If any succeed,
	 * remove that reference from the list.  At the same time, check for and
	 * weed out any moves that result in infinite recursion.
	 */
	has_printed = FALSE;
	object_count = gs_object_count(game);
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		/* Reject and remove attempts to place objects on themselves. */
		if (!lib_check_put_on_recursion(game, object, supporter, !has_printed)) {
			game->object_references[object] = FALSE;
			has_printed = TRUE;
			continue;
		}

		if (lib_try_game_command_with_object(game,
		                                     "put", object, "on", supporter)) {
			game->object_references[object] = FALSE;
			has_printed = TRUE;
		}
	}

	/* Put on every object that remains referenced. */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->object_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You put ",
				                                     "I put ",
				                                     "%player% puts "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		gs_object_move_onto(game, object, supporter);
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You put ",
			                                     "I put ",
			                                     "%player% puts "));
		} else
			pf_buffer_string(filter, " and ");
		lib_print_object_np(game, trail);
		pf_buffer_string(filter, " onto ");
		lib_print_object_np(game, supporter);
		pf_buffer_character(filter, '.');
	}
	has_printed |= count > 0;

	/* Note any remaining multiple references left out of the operation. */
	count = 0;
	trail = -1;
	for (object = 0; object < object_count; object++) {
		if (!game->multiple_references[object])
			continue;

		if (count > 0) {
			if (count == 1) {
				if (has_printed)
					pf_buffer_string(filter, "  ");
				pf_buffer_string(filter,
				                 lib_select_response(game,
				                                     "You are not holding ",
				                                     "I am not holding ",
				                                     "%player% is not holding "));
			} else
				pf_buffer_string(filter, ", ");
			lib_print_object_np(game, trail);
		}
		trail = object;
		count++;

		game->multiple_references[object] = FALSE;
	}

	if (count >= 1) {
		if (count == 1) {
			if (has_printed)
				pf_buffer_string(filter, "  ");
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     "You are not holding ",
			                                     "I am not holding ",
			                                     "%player% is not holding "));
		} else
			pf_buffer_string(filter, " or ");
		lib_print_object_np(game, trail);
		pf_buffer_character(filter, '.');
	}
}


/*
 * lib_put_on_filter()
 * lib_put_on_not_supporter_filter()
 *
 * Helper functions for deciding if an object may be put on another this
 * context.  Returns TRUE if an object may be manipulated, FALSE otherwise.
 */
static sc_bool lib_put_on_filter(sc_gameref_t game, sc_int object, sc_int unused) {
	assert(unused == -1);

	return !obj_is_static(game, object)
	       && gs_object_position(game, object) == OBJ_HELD_PLAYER;
}

static sc_bool
lib_put_on_not_supporter_filter(sc_gameref_t game,
                                sc_int object, sc_int supporter) {
	return lib_put_on_filter(game, object, -1) && object != supporter;
}


/*
 * lib_put_on_is_valid()
 *
 * Validate the supporter requested in "put on" commands.
 */
static sc_bool lib_put_on_is_valid(sc_gameref_t game, sc_int supporter) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Verify that the supporter object is a supporter. */
	if (!obj_is_surface(game, supporter)) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't put anything on ",
		                                     "I can't put anything on ",
		                                     "%player% can't put anything on "));
		lib_print_object_np(game, supporter);
		pf_buffer_string(filter, "!\n");
		return FALSE;
	}

	/* Surface is a valid target for "put on". */
	return TRUE;
}


/*
 * lib_cmd_put_all_on()
 *
 * Put all objects currently held by the player onto a supporter.
 */
sc_bool lib_cmd_put_all_on(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int supporter, objects;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	supporter = lib_disambiguate_object(game, "put that onto", &is_ambiguous);
	if (supporter == -1)
		return is_ambiguous;

	/* Validate the supporter object to take from. */
	if (!lib_put_on_is_valid(game, supporter))
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	gs_set_multiple_references(game);
	objects = lib_apply_multiple_filter(game,
	                                    lib_put_on_not_supporter_filter,
	                                    supporter, NULL);
	gs_clear_multiple_references(game);
	if (objects > 0)
		lib_put_on_backend(game, supporter);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You're not carrying anything",
		                                     "I'm not carrying anything",
		                                     "%player%'s not carrying anything"));
		if (obj_indirectly_held_by_player(game, supporter))
			pf_buffer_string(filter, " else");
		pf_buffer_character(filter, '.');
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_put_on_except_multiple()
 *
 * Put all objects currently held by the player onto an object, excepting
 * those listed in %text%.
 */
sc_bool lib_cmd_put_on_except_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int supporter, objects, references;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	supporter = lib_disambiguate_object(game, "put that onto", &is_ambiguous);
	if (supporter == -1)
		return is_ambiguous;

	/* Parse the multiple objects list to find retain target objects. */
	if (!lib_parse_multiple_objects(game, "retain",
	                                lib_put_on_not_supporter_filter,
	                                supporter, &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Validate the supporter object to put into. */
	if (!lib_put_on_is_valid(game, supporter))
		return TRUE;

	/* As a special case, complain about requests to retain the supporter. */
	if (game->multiple_references[supporter]) {
		pf_buffer_string(filter,
		                 "I only understood you as far as wanting to retain ");
		lib_print_object_np(game, supporter);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_except_filter(game,
	                                  lib_put_on_not_supporter_filter,
	                                  supporter, &references);
	if (objects > 0 || references > 0)
		lib_put_on_backend(game, supporter);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding anything",
		                                     "I am not holding anything",
		                                     "%player% is not holding anything"));
		if (objects == 0)
			pf_buffer_string(filter, " else");
		pf_buffer_character(filter, '.');
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_put_on_multiple()
 *
 * Put all objects currently held by the player and listed in %text% onto an
 * object.
 */
sc_bool lib_cmd_put_on_multiple(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int supporter, objects, references;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	supporter = lib_disambiguate_object(game, "put that onto", &is_ambiguous);
	if (supporter == -1)
		return is_ambiguous;

	/* Parse the multiple objects list to find retain target objects. */
	if (!lib_parse_multiple_objects(game, "move",
	                                lib_put_on_filter, -1,
	                                &references))
		return FALSE;
	else if (references == 0)
		return TRUE;

	/* Validate the supporter object to put into. */
	if (!lib_put_on_is_valid(game, supporter))
		return TRUE;

	/* Filter objects into references, then handle with the backend. */
	objects = lib_apply_multiple_filter(game,
	                                    lib_put_on_filter, -1,
	                                    &references);
	if (objects > 0 || references > 0)
		lib_put_on_backend(game, supporter);
	else {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding anything.",
		                                     "I am not holding anything.",
		                                     "%player% is not holding anything."));
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}


/*
 * lib_cmd_read_object()
 * lib_cmd_read_other()
 *
 * Attempt to read the referenced object, or something else.
 */
sc_bool lib_cmd_read_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int object, task;
	sc_bool is_readable, is_ambiguous;
	const sc_char *readtext, *description;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "read", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Verify that the object is readable. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Readable";
	is_readable = prop_get_boolean(bundle, "B<-sis", vt_key);
	if (!is_readable) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't read ",
		                                     "I can't read ",
		                                     "%player% can't read "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, "!\n");
		return TRUE;
	}

	/* Get and print the object's read text, if any. */
	vt_key[2].string = "ReadText";
	readtext = prop_get_string(bundle, "S<-sis", vt_key);
	if (!sc_strempty(readtext)) {
		pf_buffer_string(filter, readtext);
		pf_buffer_character(filter, '\n');
		return TRUE;
	}

	/* Degrade to a shortened object examine. */
	vt_key[2].string = "Task";
	task = prop_get_integer(bundle, "I<-sis", vt_key) - 1;

	/* Select either the main or the alternate description. */
	if (task >= 0 && gs_task_done(game, task))
		vt_key[2].string = "AltDesc";
	else
		vt_key[2].string = "Description";

	/* Print the description, or a "nothing special" default. */
	description = prop_get_string(bundle, "S<-sis", vt_key);
	if (!sc_strempty(description))
		pf_buffer_string(filter, description);
	else {
		pf_buffer_string(filter, "There is nothing special about ");
		lib_print_object_np(game, object);
		pf_buffer_character(filter, '.');
	}

	pf_buffer_character(filter, '\n');
	return TRUE;
}

sc_bool lib_cmd_read_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Reject the attempt. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You see no such thing.\n",
	                                     "I see no such thing.\n",
	                                     "%player% sees no such thing.\n"));
	return TRUE;
}


/*
 * lib_cmd_attack_npc()
 * lib_cmd_attack_npc_with()
 *
 * Attempt to attack an NPC, with and without weaponry.
 */
sc_bool lib_cmd_attack_npc(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int npc;
	sc_bool is_ambiguous;

	/* Get the referenced npc, and if none, consider complete. */
	npc = lib_disambiguate_npc(game, "attack", &is_ambiguous);
	if (npc == -1)
		return is_ambiguous;

	/* Print a standard response. */
	pf_new_sentence(filter);
	lib_print_npc_np(game, npc);
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     " avoids your feeble attempts.\n",
	                                     " avoids my feeble attempts.\n",
	                                     " avoids %player%'s feeble attempts.\n"));
	return TRUE;
}

sc_bool lib_cmd_attack_npc_with(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, npc;
	sc_vartype_t vt_key[3];
	sc_bool weapon, is_ambiguous;

	/* Get the referenced npc, and if none, consider complete. */
	npc = lib_disambiguate_npc(game, "attack", &is_ambiguous);
	if (npc == -1)
		return is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "attack with", NULL);
	if (object == -1)
		return TRUE;

	/* Ensure the referenced object is held. */
	if (gs_object_position(game, object) != OBJ_HELD_PLAYER) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding ",
		                                     "I am not holding ",
		                                     "%player% is not holding "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	/* Check for static object moved to player by event. */
	if (obj_is_static(game, object)) {
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object, " is", " are"));
		pf_buffer_string(filter, " not a weapon.\n");
		return TRUE;
	}

	/* Print standard response depending on if the object is a weapon. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Weapon";
	weapon = prop_get_boolean(bundle, "B<-sis", vt_key);
	if (weapon) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You swing at ",
		                                     "I swing at ",
		                                     "%player% swings at "));
		lib_print_npc_np(game, npc);
		pf_buffer_string(filter, " with ");
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     " but you miss.\n",
		                                     " but I miss.\n",
		                                     " but misses.\n"));
	} else {
		/*
		 * TODO Adrift uses "affective" [sic] here.  Should SCARE be right, or
		 * bug-compatible?
		 */
		pf_buffer_string(filter, "I don't think ");
		lib_print_object_np(game, object);
		pf_buffer_string(filter, " would be a very effective weapon.\n");
	}
	return TRUE;
}


/*
 * lib_cmd_kiss_npc()
 * lib_cmd_kiss_object()
 * lib_cmd_kiss_other()
 *
 * Reject romantic advances in all cases.
 */
sc_bool lib_cmd_kiss_npc(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int npc, gender;
	sc_bool is_ambiguous;

	/* Get the referenced npc, and if none, consider complete. */
	npc = lib_disambiguate_npc(game, "kiss", &is_ambiguous);
	if (npc == -1)
		return is_ambiguous;

	/* Reject this attempt. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Gender";
	gender = prop_get_integer(bundle, "I<-sis", vt_key);

	switch (gender) {
	case NPC_MALE:
		pf_buffer_string(filter, "I'm not sure he would appreciate that!\n");
		break;

	case NPC_FEMALE:
		pf_buffer_string(filter, "I'm not sure she would appreciate that!\n");
		break;

	case NPC_NEUTER:
		pf_buffer_string(filter, "I'm not sure it would appreciate that!\n");
		break;

	default:
		sc_error("lib_cmd_kiss_npc: unknown gender, %ld\n", gender);
	}
	return TRUE;
}

sc_bool lib_cmd_kiss_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "kiss", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Reject this attempt. */
	pf_buffer_string(filter, "I'm not sure ");
	lib_print_object_np(game, object);
	pf_buffer_string(filter, " would appreciate that.\n");
	return TRUE;
}

sc_bool lib_cmd_kiss_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Reject this attempt. */
	pf_buffer_string(filter, "I'm not sure it would appreciate that.\n");
	return TRUE;
}


/*
 * lib_cmd_buy_object()
 * lib_cmd_buy_other()
 *
 * Standard responses to attempts to buy something.
 */
sc_bool lib_cmd_buy_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "buy", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Reject this attempt. */
	pf_buffer_string(filter, "I don't think ");
	lib_print_object_np(game, object);
	pf_buffer_string(filter,
	                 lib_select_plurality(game, object, " is", " are"));
	pf_buffer_string(filter, " for sale.\n");
	return TRUE;
}

sc_bool lib_cmd_buy_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Reject this attempt. */
	pf_buffer_string(filter, "I don't think that is for sale.\n");
	return TRUE;
}


/*
 * lib_cmd_break_object()
 * lib_cmd_break_other()
 *
 * Standard responses to attempts to break something.
 */
sc_bool lib_cmd_break_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "break", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Reject this attempt. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You might need ",
	                                     "I might need ",
	                                     "%player% might need "));
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}

sc_bool lib_cmd_break_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Reject this attempt. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You might need that.\n",
	                                     "I might need that.\n",
	                                     "%player% might need that.\n"));
	return TRUE;
}


/*
 * lib_cmd_smell_object()
 * lib_cmd_smell_other()
 *
 * Standard responses to attempts to smell something.
 */
sc_bool lib_cmd_smell_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "smell", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Reject this attempt. */
	pf_new_sentence(filter);
	lib_print_object_np(game, object);
	pf_buffer_string(filter, " smells normal.\n");
	return TRUE;
}

sc_bool lib_cmd_smell_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Reject this attempt. */
	pf_buffer_string(filter, "That smells normal.\n");
	return TRUE;
}


/*
 * lib_cmd_sell_object()
 * lib_cmd_sell_other()
 *
 * Standard responses to attempts to sell something.
 */
sc_bool lib_cmd_sell_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "sell", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Reject this attempt. */
	pf_buffer_string(filter, "No-one is interested in buying ");
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}

sc_bool lib_cmd_sell_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "No-one is interested in buying that.\n");
	return TRUE;
}


/*
 * lib_cmd_eat_object()
 *
 * Consume edible objects.
 */
sc_bool lib_cmd_eat_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int object;
	sc_bool edible, is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "eat", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Check that we have the object to eat. */
	if (gs_object_position(game, object) != OBJ_HELD_PLAYER) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not holding ",
		                                     "I am not holding ",
		                                     "%player% is not holding "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	/* Check for static object moved to player by event. */
	if (obj_is_static(game, object)) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't eat ",
		                                     "I can't eat ",
		                                     "%player% can't eat "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	/* Is this object inedible? */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Edible";
	edible = prop_get_boolean(bundle, "B<-sis", vt_key);
	if (!edible) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't eat ",
		                                     "I can't eat ",
		                                     "%player% can't eat "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, ".\n");
		return TRUE;
	}

	/* Confirm, and hide the object. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You eat ",
	                                     "I eat ", "%player% eats "));
	lib_print_object_np(game, object);
	pf_buffer_string(filter,
	                 ".  Not bad, but it could do with a pinch of salt!\n");
	gs_object_make_hidden(game, object);
	return TRUE;
}


/* Enumerated sit/stand/lie types. */
enum {
	OBJ_STANDABLE_MASK = 1 << 0,
	OBJ_LIEABLE_MASK = 1 << 1
};
enum {
	MOVE_SIT, MOVE_SIT_FLOOR,
	MOVE_STAND, MOVE_STAND_FLOOR, MOVE_LIE, MOVE_LIE_FLOOR
};

/*
 * lib_stand_sit_lie()
 *
 * Central handler for stand, sit, and lie commands.
 */
static sc_bool lib_stand_sit_lie(sc_gameref_t game, sc_int movement) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int object, position;
	const sc_char *already_doing_that, *success_message;

	/* Initialize variables to avoid gcc warnings. */
	object = -1;
	already_doing_that = FALSE;
	success_message = FALSE;
	position = 0;

	/* Get a target object for movement, -1 if floor. */
	switch (movement) {
	case MOVE_STAND:
	case MOVE_SIT:
	case MOVE_LIE: {
		const sc_char *disambiguate, *cant_do_that;
		sc_int sit_lie_flags, movement_mask;
		sc_vartype_t vt_key[3];
		sc_bool is_ambiguous;

		/* Initialize variables to avoid gcc warnings. */
		disambiguate = NULL;
		cant_do_that = NULL;
		movement_mask = 0;

		/* Set disambiguation and not amenable messages. */
		switch (movement) {
		case MOVE_STAND:
			disambiguate = "stand on";
			cant_do_that = lib_select_response(game,
			                                   "You can't stand on ",
			                                   "I can't stand on ",
			                                   "%player% can't stand on ");
			movement_mask = OBJ_STANDABLE_MASK;
			break;
		case MOVE_SIT:
			disambiguate = "sit on";
			cant_do_that = lib_select_response(game,
			                                   "You can't sit on ",
			                                   "I can't sit on ",
			                                   "%player% can't sit on ");
			movement_mask = OBJ_STANDABLE_MASK;
			break;
		case MOVE_LIE:
			disambiguate = "lie on";
			cant_do_that = lib_select_response(game,
			                                   "You can't lie on ",
			                                   "I can't lie on ",
			                                   "%player% can't lie on ");
			movement_mask = OBJ_LIEABLE_MASK;
			break;
		default:
			sc_fatal("lib_sit_stand_lie: movement error, %ld\n", movement);
		}

		/* Get the referenced object; if none, consider complete. */
		object = lib_disambiguate_object(game, disambiguate, &is_ambiguous);
		if (object == -1)
			return is_ambiguous;

		/* Verify the referenced object is amenable. */
		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "SitLie";
		sit_lie_flags = prop_get_integer(bundle, "I<-sis", vt_key);
		if (!(sit_lie_flags & movement_mask)) {
			pf_buffer_string(filter, cant_do_that);
			lib_print_object_np(game, object);
			pf_buffer_string(filter, ".\n");
			return TRUE;
		}
		break;
	}

	case MOVE_STAND_FLOOR:
	case MOVE_SIT_FLOOR:
	case MOVE_LIE_FLOOR:
		object = -1;
		break;

	default:
		sc_fatal("lib_sit_stand_lie: movement error, %ld\n", movement);
	}

	/* Set up confirmation messages and position. */
	switch (movement) {
	case MOVE_STAND:
		already_doing_that = lib_select_response(game,
		                     "You are already standing on ",
		                     "I am already standing on ",
		                     "%player% is already standing on ");
		success_message = lib_select_response(game,
		                                      "You stand on ",
		                                      "I stand on ",
		                                      "%player% stands on ");
		position = 0;
		break;

	case MOVE_STAND_FLOOR:
		already_doing_that = lib_select_response(game,
		                     "You are already standing!\n",
		                     "I am already standing!\n",
		                     "%player% is already standing!\n");
		success_message = lib_select_response(game,
		                                      "You stand up",
		                                      "I stand up",
		                                      "%player% stands up");
		position = 0;
		break;

	case MOVE_SIT:
		already_doing_that = lib_select_response(game,
		                     "You are already sitting on ",
		                     "I am already sitting on ",
		                     "%player% is already sitting on ");
		if (gs_playerposition(game) == 2)
			success_message = lib_select_response(game,
			                                      "You sit up on ",
			                                      "I sit up on ",
			                                      "%player% sits up on ");
		else
			success_message = lib_select_response(game,
			                                      "You sit down on ",
			                                      "I sit down on ",
			                                      "%player% sits down on ");
		position = 1;
		break;

	case MOVE_SIT_FLOOR:
		already_doing_that = lib_select_response(game,
		                     "You are already sitting down.\n",
		                     "I am already sitting down.\n",
		                     "%player% is already sitting down.\n");
		if (gs_playerposition(game) == 2)
			success_message = lib_select_response(game,
			                                      "You sit up on the ground.\n",
			                                      "I sit up on the ground.\n",
			                                      "%player% sits up on the ground.\n");
		else
			success_message = lib_select_response(game,
			                                      "You sit down on the ground.\n",
			                                      "I sit down on the ground.\n",
			                                      "%player% sits down on the ground.\n");
		position = 1;
		break;

	case MOVE_LIE:
		already_doing_that = lib_select_response(game,
		                     "You are already lying on ",
		                     "I am already lying on ",
		                     "%player% is already lying on ");
		success_message = lib_select_response(game,
		                                      "You lie down on ",
		                                      "I lie down on ",
		                                      "%player% lies down on ");
		position = 2;
		break;

	case MOVE_LIE_FLOOR:
		already_doing_that = lib_select_response(game,
		                     "You are already lying down.\n",
		                     "I am already lying down.\n",
		                     "%player% is already lying down.\n");
		success_message = lib_select_response(game,
		                                      "You lie down on the ground.\n",
		                                      "I lie down on the ground.\n",
		                                      "%player% lies down on the ground.\n");
		position = 2;
		break;

	default:
		sc_fatal("lib_sit_stand_lie: movement error, %ld\n", movement);
	}

	/* See if already doing this. */
	if (gs_playerposition(game) == position && gs_playerparent(game) == object) {
		pf_buffer_string(filter, already_doing_that);
		if (object != -1) {
			lib_print_object_np(game, object);
			pf_buffer_string(filter, ".\n");
		}
		return TRUE;
	}

	/* Confirm movement, with special case for getting off an object. */
	pf_buffer_string(filter, success_message);
	if (movement == MOVE_STAND_FLOOR) {
		if (gs_playerparent(game) != -1) {
			pf_buffer_string(filter, " from ");
			lib_print_object_np(game, gs_playerparent(game));
		}
		pf_buffer_string(filter, ".\n");
	} else if (object != -1) {
		lib_print_object_np(game, object);
		pf_buffer_string(filter, ".\n");
	}

	/* Adjust player position and parent. */
	gs_set_playerposition(game, position);
	gs_set_playerparent(game, object);
	return TRUE;
}


/*
 * lib_cmd_stand_*
 * lib_cmd_sit_*
 * lib_cmd_lie_*
 *
 * Stand, sit, or lie on an object, or on the floor.
 */
sc_bool lib_cmd_stand_on_object(sc_gameref_t game) {
	return lib_stand_sit_lie(game, MOVE_STAND);
}

sc_bool lib_cmd_stand_on_floor(sc_gameref_t game) {
	return lib_stand_sit_lie(game, MOVE_STAND_FLOOR);
}

sc_bool lib_cmd_sit_on_object(sc_gameref_t game) {
	return lib_stand_sit_lie(game, MOVE_SIT);
}

sc_bool lib_cmd_sit_on_floor(sc_gameref_t game) {
	return lib_stand_sit_lie(game, MOVE_SIT_FLOOR);
}

sc_bool lib_cmd_lie_on_object(sc_gameref_t game) {
	return lib_stand_sit_lie(game, MOVE_LIE);
}

sc_bool lib_cmd_lie_on_floor(sc_gameref_t game) {
	return lib_stand_sit_lie(game, MOVE_LIE_FLOOR);
}


/*
 * lib_cmd_get_off_object()
 * lib_cmd_get_off()
 *
 * Get off whatever supporter the player rests on.
 */
sc_bool lib_cmd_get_off_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object;
	sc_bool is_ambiguous;

	/* Get the referenced object; if none, consider complete. */
	object = lib_disambiguate_object(game, "get off", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Reject the attempt if the player is not on the given object. */
	if (gs_playerparent(game) != object) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not on ",
		                                     "I am not on ",
		                                     "%player% is not on "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, "!\n");
		return TRUE;
	}

	/* Confirm movement. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You get off ", "I get off ",
	                                     "%player% gets off "));
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");

	/* Adjust player position and parent. */
	gs_set_playerposition(game, 0);
	gs_set_playerparent(game, -1);
	return TRUE;
}

sc_bool lib_cmd_get_off(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Reject the attempt if the player is not on anything. */
	if (gs_playerparent(game) == -1) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are not on anything!\n",
		                                     "I am not on anything!\n",
		                                     "%player% is not on anything!\n"));
		return TRUE;
	}

	/* Confirm movement. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You get off ", "I get off ",
	                                     "%player% gets off "));
	lib_print_object_np(game, gs_playerparent(game));
	pf_buffer_string(filter, ".\n");

	/* Adjust player position and parent. */
	gs_set_playerposition(game, 0);
	gs_set_playerparent(game, -1);
	return TRUE;
}


/*
 * lib_cmd_save()
 * lib_cmd_restore()
 *
 * Save/restore a game.
 */
sc_bool lib_cmd_save(sc_gameref_t game) {
	if (if_confirm(SC_CONF_SAVE)) {
		if (g_vm->saveGame().getCode() == Common::kNoError)
			if_print_string("Ok.\n");
		else
			if_print_string("Save failed.\n");
	}

	game->is_admin = TRUE;
	return TRUE;
}

sc_bool lib_cmd_restore(sc_gameref_t game) {
	if (if_confirm(SC_CONF_RESTORE)) {
		if (g_vm->loadGame().getCode() == Common::kNoError) {
			if_print_string("Ok.\n");
			game->is_running = FALSE;
			game->do_restore = TRUE;
		} else {
			if_print_string("Restore failed.\n");
		}
	}

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_locate_object()
 * lib_cmd_locate_npc()
 *
 * Display the location of a selected object, and selected NPC.
 */
sc_bool lib_cmd_locate_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int index_, count, object, room, position, parent;

	game->is_admin = TRUE;

	/*
	 * Filter to remove unseen object references.  Note that this is different
	 * from NPCs, who we acknowledge even when unseen.
	 */
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		if (!gs_object_seen(game, index_))
			game->object_references[index_] = FALSE;
	}

	/* Count the number of objects referenced by the last command. */
	count = 0;
	object = -1;
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		if (game->object_references[index_]) {
			count++;
			object = index_;
		}
	}

	/*
	 * If no objects identified, be coy about revealing anything; if more than
	 * one, be vague.
	 */
	if (count == 0) {
		pf_buffer_string(filter, "I don't know where that is.\n");
		return TRUE;
	} else if (count > 1) {
		pf_buffer_string(filter,
		                 "Please be more clear about what you want to"
		                 " locate.\n");
		return TRUE;
	}

	/*
	 * The reference is unambiguous, so we're responsible for noting it in
	 * variables.  Disambiguation would normally do this for us, but we just
	 * bypassed it.
	 */
	var_set_ref_object(vars, object);

	/* See if we can print a message based on position and parent. */
	position = gs_object_position(game, object);
	parent = gs_object_parent(game, object);
	switch (position) {
	case OBJ_HIDDEN:
		if (!obj_is_static(game, object)) {
			pf_buffer_string(filter, "I don't know where that is.\n");
			return TRUE;
		}
		break;

	case OBJ_HELD_PLAYER:
		pf_new_sentence(filter);
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are carrying ",
		                                     "I am carrying ",
		                                     "%player% is carrying "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, "!\n");
		return TRUE;

	case OBJ_WORN_PLAYER:
		pf_new_sentence(filter);
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You are wearing ",
		                                     "I am wearing ",
		                                     "%player% is wearing "));
		lib_print_object_np(game, object);
		pf_buffer_string(filter, "!\n");
		return TRUE;

	case OBJ_HELD_NPC:
	case OBJ_WORN_NPC:
		if (gs_npc_seen(game, parent)) {
			pf_new_sentence(filter);
			lib_print_npc_np(game, parent);
			pf_buffer_string(filter,
			                 (position == OBJ_HELD_NPC)
			                 ? " is holding " : " is wearing ");
			lib_print_object_np(game, object);
			pf_buffer_string(filter, ".\n");
		} else
			pf_buffer_string(filter, "I don't know where that is.\n");
		return TRUE;

	case OBJ_PART_NPC:
		if (parent == -1) {
			pf_new_sentence(filter);
			lib_print_object_np(game, object);
			pf_buffer_string(filter,
			                 lib_select_plurality(game, object, " is", " are"));
			pf_buffer_string(filter,
			                 lib_select_response(game,
			                                     " a part of you!\n",
			                                     " a part of me!\n",
			                                     " a part of %player%!\n"));
		} else {
			if (gs_npc_seen(game, parent)) {
				pf_new_sentence(filter);
				lib_print_object_np(game, object);
				pf_buffer_string(filter,
				                 lib_select_plurality(game, object,
				                                      " is", " are"));
				pf_buffer_string(filter, " a part of ");
				lib_print_npc_np(game, parent);
				pf_buffer_string(filter, ".\n");
			} else
				pf_buffer_string(filter, "I don't know where that is.\n");
		}
		return TRUE;

	case OBJ_ON_OBJECT:
	case OBJ_IN_OBJECT:
		if (gs_object_seen(game, parent)) {
			pf_new_sentence(filter);
			lib_print_object_np(game, object);
			pf_buffer_string(filter,
			                 lib_select_plurality(game, object, " is", " are"));
			pf_buffer_string(filter,
			                 (position == OBJ_ON_OBJECT) ? " on " : " inside ");
			lib_print_object_np(game, parent);
			pf_buffer_string(filter, ".\n");
		} else
			pf_buffer_string(filter, "I don't know where that is.\n");
		return TRUE;

	default:
		break;
	}

	/*
	 * Object is either static unmoved, or dynamic and on the floor of a room.
	 * Check each room for the object, stopping on first found.
	 */
	for (room = 0; room < gs_room_count(game); room++) {
		if (obj_indirectly_in_room(game, object, room))
			break;
	}
	if (room == gs_room_count(game)) {
		pf_buffer_string(filter, "I don't know where that is.\n");
		return TRUE;
	}

	/* Check that this room's been visited by the player. */
	if (!gs_room_seen(game, room)) {
		pf_new_sentence(filter);
		lib_print_object_np(game, object);
		pf_buffer_string(filter,
		                 lib_select_plurality(game, object, " is", " are"));
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     " somewhere that you haven't been yet.\n",
		                                     " somewhere that I haven't been yet.\n",
		                                     " somewhere that %player% hasn't been yet.\n"));
		return TRUE;
	}

	/* Print the details of the object's room. */
	pf_new_sentence(filter);
	lib_print_object_np(game, object);
	pf_buffer_string(filter, " -- ");
	pf_buffer_string(filter, lib_get_room_name(game, room));
	pf_buffer_string(filter, ".\n");
	return TRUE;
}

sc_bool lib_cmd_locate_npc(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int index_, count, npc, room;

	game->is_admin = TRUE;

	/* Count the number of NPCs referenced by the last command. */
	count = 0;
	npc = -1;
	for (index_ = 0; index_ < gs_npc_count(game); index_++) {
		if (game->npc_references[index_]) {
			count++;
			npc = index_;
		}
	}

	/*
	 * If no NPCs identified, be coy about revealing anything; if more than one,
	 * be vague.  The "... where that is..." is the correct message even for
	 * NPCs -- it's the same response as for lib_locate_other().
	 */
	if (count == 0) {
		pf_buffer_string(filter, "I don't know where that is.\n");
		return TRUE;
	} else if (count > 1) {
		pf_buffer_string(filter,
		                 "Please be more clear about who you want to locate.\n");
		return TRUE;
	}

	/*
	 * The reference is unambiguous, so we're responsible for noting it in
	 * variables.  Disambiguation would normally do this for us, but we just
	 * bypassed it.
	 */
	var_set_ref_character(vars, npc);

	/* See if this NPC has been seen yet. */
	if (!gs_npc_seen(game, npc)) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You haven't seen ",
		                                     "I haven't seen ",
		                                     "%player% hasn't seen "));
		lib_print_npc_np(game, npc);
		pf_buffer_string(filter, " yet!\n");
		return TRUE;
	}

	/* Check each room for the NPC, stopping on first found. */
	for (room = 0; room < gs_room_count(game); room++) {
		if (npc_in_room(game, npc, room))
			break;
	}
	if (room == gs_room_count(game)) {
		pf_buffer_string(filter, "I don't know where ");
		lib_print_npc_np(game, npc);
		pf_buffer_string(filter, " is.\n");
		return TRUE;
	}

	/* Check that this room's been visited by the player. */
	if (!gs_room_seen(game, room)) {
		lib_print_npc_np(game, npc);
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     " is somewhere that you haven't been yet.\n",
		                                     " is somewhere that I haven't been yet.\n",
		                                     " is somewhere that %player% hasn't been yet.\n"));
		return TRUE;
	}

	/* Print the location, and smart-alec response. */
	pf_new_sentence(filter);
	lib_print_npc_np(game, npc);
	pf_buffer_string(filter, " -- ");
	pf_buffer_string(filter, lib_get_room_name(game, room));
#if 0
	if (room == gs_playerroom(game)) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "  (Right next to you, silly!)",
		                                     "  (Right next to me, silly!)",
		                                     "  (Right next to %player%, silly!)"));
	}
#endif
	pf_buffer_string(filter, ".\n");
	return TRUE;
}


/*
 * lib_cmd_turns()
 * lib_cmd_score()
 *
 * Display turns taken and score so far.
 */
sc_bool lib_cmd_turns(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_char buffer[32];

	pf_buffer_string(filter, "You have taken ");
	sprintf(buffer, "%ld", game->turns);
	pf_buffer_string(filter, buffer);
	if (game->turns == 1)
		pf_buffer_string(filter, " turn so far.\n");
	else
		pf_buffer_string(filter, " turns so far.\n");

	game->is_admin = TRUE;
	return TRUE;
}

sc_bool lib_cmd_score(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	sc_int max_score, percent;
	sc_char buffer[32];

	/* Get max score, and calculate score as a percentage. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "MaxScore";
	max_score = prop_get_integer(bundle, "I<-ss", vt_key);
	if (game->score > 0 && max_score > 0)
		percent = (game->score * 100) / max_score;
	else
		percent = 0;

	/* Output carefully formatted response. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "Your score is ",
	                                     "My score is ",
	                                     "%player%'s score is "));
	sprintf(buffer, "%ld", game->score);
	pf_buffer_string(filter, buffer);
	pf_buffer_string(filter, " out of a maximum of ");
	sprintf(buffer, "%ld", max_score);
	pf_buffer_string(filter, buffer);
	pf_buffer_string(filter, ".  (");
	sprintf(buffer, "%ld", percent);
	pf_buffer_string(filter, buffer);
	pf_buffer_string(filter, "%)\n");

	game->is_admin = TRUE;
	return TRUE;
}


/*
 * lib_cmd_*()
 *
 * Standard response commands.  These are uninteresting catch-all cases,
 * but it's good to make then right as game ALRs may look for them.
 */
sc_bool lib_cmd_profanity(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 "I really don't think there's any need for language like"
	                 " that!\n");
	return TRUE;
}

sc_bool lib_cmd_examine_all(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "Please examine one object at a time.\n");
	return TRUE;
}

sc_bool lib_cmd_examine_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You see no such thing.\n",
	                                     "I see no such thing.\n",
	                                     "%player% sees no such thing.\n"));
	return TRUE;
}

sc_bool lib_cmd_locate_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "I don't know where that is!\n");
	game->is_admin = TRUE;
	return TRUE;
}

sc_bool lib_cmd_unix_like(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "This isn't Unix you know!\n");
	return TRUE;
}

sc_bool lib_cmd_dos_like(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "This isn't Dos you know!\n");
	return TRUE;
}

sc_bool lib_cmd_cry(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "There's no need for that!\n");
	return TRUE;
}

sc_bool lib_cmd_dance(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You do a little dance.\n",
	                                     "I do a little dance.\n",
	                                     "%player% does a little dance.\n"));
	return TRUE;
}

sc_bool lib_cmd_eat_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "I don't understand what you are trying to eat.\n");
	return TRUE;
}

sc_bool lib_cmd_fight(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "There is nothing worth fighting here.\n");
	return TRUE;
}

sc_bool lib_cmd_feed(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "There is nothing worth feeding here.\n");
	return TRUE;
}

sc_bool lib_cmd_feel(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You feel nothing out of the ordinary.\n",
	                                     "I feel nothing out of the ordinary.\n",
	                                     "%player% feels nothing out of the ordinary.\n"));
	return TRUE;
}

sc_bool lib_cmd_fly(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You can't fly.\n",
	                                     "I can't fly.\n",
	                                     "%player% can't fly.\n"));
	return TRUE;
}

sc_bool lib_cmd_hint(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 "You're just going to have to work it out for"
	                 " yourself...\n");
	return TRUE;
}

sc_bool lib_cmd_hum(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You hum a little tune.\n",
	                                     "I hum a little tune.\n",
	                                     "%player% hums a little tune.\n"));
	return TRUE;
}

sc_bool lib_cmd_jump(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "Wheee-boinng.\n");
	return TRUE;
}

sc_bool lib_cmd_listen(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You hear nothing out of the ordinary.\n",
	                                     "I hear nothing out of the ordinary.\n",
	                                     "%player% hears nothing out of the ordinary.\n"));
	return TRUE;
}

sc_bool lib_cmd_please(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "Your kindness gets you nowhere.\n",
	                                     "My kindness gets me nowhere.\n",
	                                     "%player%'s kindness gets nowhere.\n"));
	return TRUE;
}

sc_bool lib_cmd_punch(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "Who do you think you are, Mike Tyson?\n");
	return TRUE;
}

sc_bool lib_cmd_run(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "Why would you want to run?\n",
	                                     "Why would I want to run?\n",
	                                     "Why would %player% want to run?\n"));
	return TRUE;
}

sc_bool lib_cmd_shout(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "Aaarrrrgggghhhhhh!\n");
	return TRUE;
}

sc_bool lib_cmd_say(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_char *string = NULL;

	switch (sc_randomint(1, 5)) {
	case 1:
		string = "Gosh, that was very impressive.\n";
		break;
	case 2:
		string = lib_select_response(game,
		                             "Not surprisingly, no-one takes any notice"
		                             " of you.\n",
		                             "Not surprisingly, no-one takes any notice"
		                             " of me.\n",
		                             "Not surprisingly, no-one takes any notice"
		                             " of %player%.\n");
		break;
	case 3:
		string = "Wow!  That achieved a lot.\n";
		break;
	case 4:
		string = "Uh huh, yes, very interesting.\n";
		break;
	default:
		string = "That's the most interesting thing I've ever heard!\n";
		break;
	}

	pf_buffer_string(filter, string);
	return TRUE;
}

sc_bool lib_cmd_sing(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You sing a little song.\n",
	                                     "I sing a little song.\n",
	                                     "%player% sings a little song.\n"));
	return TRUE;
}

sc_bool lib_cmd_sleep(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "Zzzzz.  Bored are you?\n");
	return TRUE;
}

sc_bool lib_cmd_talk(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "No-one listens to your rabblings.\n",
	                                     "No-one listens to my rabblings.\n",
	                                     "No-one listens to %player%'s rabblings.\n"));
	return TRUE;
}

sc_bool lib_cmd_thank(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "You're welcome.\n");
	return TRUE;
}

sc_bool lib_cmd_whistle(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You whistle a little tune.\n",
	                                     "I whistle a little tune.\n",
	                                     "%player% whistles a little tune.\n"));
	return TRUE;
}

sc_bool lib_cmd_interrogation(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_char *string = NULL;

	switch (sc_randomint(1, 17)) {
	case 1:
		string = "Why do you want to know?\n";
		break;
	case 2:
		string = "Interesting question.\n";
		break;
	case 3:
		string = "Let me think about that one...\n";
		break;
	case 4:
		string = "I haven't a clue!\n";
		break;
	case 5:
		string = "All these questions are hurting my head.\n";
		break;
	case 6:
		string = "I'm not going to tell you.\n";
		break;
	case 7:
		string = "Someday I'll know the answer to that one.\n";
		break;
	case 8:
		string = "I could tell you, but then I'd have to kill you.\n";
		break;
	case 9:
		string = "Ha, as if I'd tell you!\n";
		break;
	case 10:
		string = "Ask me again later.\n";
		break;
	case 11:
		string = "I don't know - could you ask anyone else?\n";
		break;
	case 12:
		string = "Err, yes?!?\n";
		break;
	case 13:
		string = "Let me just check my memory banks...\n";
		break;
	case 14:
		string = "Because that's just the way it is.\n";
		break;
	case 15:
		string = "Do I ask you all sorts of awkward questions?\n";
		break;
	case 16:
		string = "Questions, questions...\n";
		break;
	default:
		string = "Who cares.\n";
		break;
	}

	pf_buffer_string(filter, string);
	return TRUE;
}

sc_bool lib_cmd_xyzzy(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 "I'm sorry, but XYZZY doesn't do anything special in"
	                 " this game!\n");
	return TRUE;
}

sc_bool lib_cmd_egotistic(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

#if 0
	pf_buffer_string(filter,
	                 "Campbell wrote this Adrift Runner.  It's pretty"
	                 " good huh!\n");
#else
	pf_buffer_string(filter, "No comment.\n");
#endif

	return TRUE;
}

sc_bool lib_cmd_yes_or_no(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter,
	                 "That's interesting, but it doesn't mean much.\n");
	return TRUE;
}


/*
 * lib_cmd_ask_npc()
 * lib_cmd_ask_object()
 * lib_cmd_ask_other()
 *
 * Malformed and rhetorical question responses.
 */
sc_bool lib_cmd_ask_npc(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int npc;
	sc_bool is_ambiguous;

	/* Get the referenced npc, and if none, consider complete. */
	npc = lib_disambiguate_npc(game, "ask", &is_ambiguous);
	if (npc == -1)
		return is_ambiguous;

	/* Incomplete ask command, so offer help and return. */
	pf_buffer_string(filter, "Use the format \"ask ");
	lib_print_npc_np(game, npc);
	pf_buffer_string(filter, " about [subject]\".\n");
	return TRUE;
}

sc_bool lib_cmd_ask_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object;
	sc_bool is_ambiguous;

	/* Get the referenced object, and if none, consider complete. */
	object = lib_disambiguate_object(game, "ask", &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* No reply. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You get no reply from ",
	                                     "I get no reply from ",
	                                     "%player% gets no reply from "));
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}

sc_bool lib_cmd_ask_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	/* Incomplete ask command, so offer help and return. */
	pf_buffer_string(filter,
	                 "Use the format \"ask [character] about [subject]\".\n");
	return TRUE;
}


/*
 * lib_cmd_kill_other()
 *
 * Uninteresting kill message when no weaponry is involved.
 */
sc_bool lib_cmd_kill_other(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, "Now that isn't very nice.\n");
	return TRUE;
}


/*
 * lib_nothing_happens_common()
 * lib_nothing_happens_object()
 * lib_nothing_happens_other()
 *
 * Central handler for a range of nothing-happens messages.  More
 * uninteresting responses.
 */
static sc_bool lib_nothing_happens_common(sc_gameref_t game, const sc_char *verb_general,
		const sc_char *verb_third_person, sc_bool is_object) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	sc_int perspective, object;
	const sc_char *person, *verb;
	sc_bool is_ambiguous;

	/* Use person and verb tense according to perspective. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "Perspective";
	perspective = prop_get_integer(bundle, "I<-ss", vt_key);
	switch (perspective) {
	case LIB_FIRST_PERSON:
		person = "I ";
		verb = verb_general;
		break;
	case LIB_SECOND_PERSON:
		person = "You ";
		verb = verb_general;
		break;
	case LIB_THIRD_PERSON:
		person = "%player% ";
		verb = verb_third_person;
		break;
	default:
		sc_error("lib_nothing_happens: unknown perspective, %ld\n", perspective);
		person = "You ";
		verb = verb_general;
		break;
	}

	/* If the command target was not an object, end it here. */
	if (!is_object) {
		pf_buffer_string(filter, person);
		pf_buffer_string(filter, verb);
		pf_buffer_string(filter, ", but nothing happens.\n");
		return TRUE;
	}

	/* Get the referenced object.  If none, return immediately. */
	object = lib_disambiguate_object(game, verb_general, &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Nothing happens. */
	pf_buffer_string(filter, person);
	pf_buffer_string(filter, verb);
	pf_buffer_character(filter, ' ');
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ", but nothing happens.\n");
	return TRUE;
}

static sc_bool lib_nothing_happens_object(sc_gameref_t game,
		const sc_char *verb_general, const sc_char *verb_third_person) {
	return lib_nothing_happens_common(game,
	                                  verb_general, verb_third_person, TRUE);
}

static sc_bool lib_nothing_happens_other(sc_gameref_t game,
		const sc_char *verb_general, const sc_char *verb_third_person) {
	return lib_nothing_happens_common(game,
	                                  verb_general, verb_third_person, FALSE);
}


/*
 * lib_cmd_*()
 *
 * Shake, rattle and roll, and assorted nothing-happens handlers.
 */
sc_bool lib_cmd_hit_object(sc_gameref_t game) {
	return lib_nothing_happens_object(game, "hit", "hits");
}

sc_bool lib_cmd_kick_object(sc_gameref_t game) {
	return lib_nothing_happens_object(game, "kick", "kicks");
}

sc_bool lib_cmd_press_object(sc_gameref_t game) {
	return lib_nothing_happens_object(game, "press", "presses");
}

sc_bool lib_cmd_push_object(sc_gameref_t game) {
	return lib_nothing_happens_object(game, "push", "pushes");
}

sc_bool lib_cmd_pull_object(sc_gameref_t game) {
	return lib_nothing_happens_object(game, "pull", "pulls");
}

sc_bool lib_cmd_shake_object(sc_gameref_t game) {
	return lib_nothing_happens_object(game, "shake", "shakes");
}

sc_bool lib_cmd_hit_other(sc_gameref_t game) {
	return lib_nothing_happens_other(game, "hit", "hits");
}

sc_bool lib_cmd_kick_other(sc_gameref_t game) {
	return lib_nothing_happens_other(game, "kick", "kicks");
}

sc_bool lib_cmd_press_other(sc_gameref_t game) {
	return lib_nothing_happens_other(game, "press", "presses");
}

sc_bool lib_cmd_push_other(sc_gameref_t game) {
	return lib_nothing_happens_other(game, "push", "pushes");
}

sc_bool lib_cmd_pull_other(sc_gameref_t game) {
	return lib_nothing_happens_other(game, "pull", "pulls");
}

sc_bool lib_cmd_shake_other(sc_gameref_t game) {
	return lib_nothing_happens_other(game, "shake", "shakes");
}


/*
 * lib_cant_do_common()
 * lib_cant_do_object()
 * lib_cant_do_other()
 *
 * Central handler for a range of can't-do messages.  Yet more uninterest-
 * ing responses.
 */
static sc_bool lib_cant_do_common(sc_gameref_t game, const sc_char *verb, sc_bool is_object) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object;
	sc_bool is_ambiguous;

	/* If the target is not an object, end it here. */
	if (!is_object) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "You can't ",
		                                     "I can't ", "%player% can't "));
		pf_buffer_string(filter, verb);
		pf_buffer_string(filter, " that.\n");
		return TRUE;
	}

	/* Get the referenced object.  If none, return immediately. */
	object = lib_disambiguate_object(game, verb, &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Whatever it is, don't do it. */
	pf_buffer_string(filter,
	                 lib_select_response(game,
	                                     "You can't ",
	                                     "I can't ", "%player% can't "));
	pf_buffer_string(filter, verb);
	pf_buffer_character(filter, ' ');
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}

static sc_bool lib_cant_do_object(sc_gameref_t game, const sc_char *verb) {
	return lib_cant_do_common(game, verb, TRUE);
}

static sc_bool lib_cant_do_other(sc_gameref_t game, const sc_char *verb) {
	return lib_cant_do_common(game, verb, FALSE);
}


/*
 * lib_cmd_*()
 *
 * Assorted can't-do messages.
 */
sc_bool lib_cmd_block_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "block");
}

sc_bool lib_cmd_climb_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "climb");
}

sc_bool lib_cmd_clean_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "clean");
}

sc_bool lib_cmd_cut_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "cut");
}

sc_bool lib_cmd_drink_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "drink");
}

sc_bool lib_cmd_light_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "light");
}

sc_bool lib_cmd_lift_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "lift");
}

sc_bool lib_cmd_move_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "move");
}

sc_bool lib_cmd_rub_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "rub");
}

sc_bool lib_cmd_stop_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "stop");
}

sc_bool lib_cmd_suck_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "suck");
}

sc_bool lib_cmd_touch_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "touch");
}

sc_bool lib_cmd_turn_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "turn");
}

sc_bool lib_cmd_unblock_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "unblock");
}

sc_bool lib_cmd_wash_object(sc_gameref_t game) {
	return lib_cant_do_object(game, "wash");
}

sc_bool lib_cmd_block_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "block");
}

sc_bool lib_cmd_climb_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "climb");
}

sc_bool lib_cmd_clean_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "clean");
}

sc_bool lib_cmd_close_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "close");
}

sc_bool lib_cmd_lock_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "lock");
}

sc_bool lib_cmd_unlock_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "unlock");
}

sc_bool lib_cmd_stand_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "stand on");
}

sc_bool lib_cmd_sit_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "sit on");
}

sc_bool lib_cmd_lie_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "lie on");
}

sc_bool lib_cmd_cut_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "cut");
}

sc_bool lib_cmd_drink_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "drink");
}

sc_bool lib_cmd_lift_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "lift");
}

sc_bool lib_cmd_light_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "light");
}

sc_bool lib_cmd_move_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "move");
}

sc_bool lib_cmd_stop_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "stop");
}

sc_bool lib_cmd_rub_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "rub");
}

sc_bool lib_cmd_suck_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "suck");
}

sc_bool lib_cmd_turn_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "turn");
}

sc_bool lib_cmd_touch_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "touch");
}

sc_bool lib_cmd_unblock_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "unblock");
}

sc_bool lib_cmd_wash_other(sc_gameref_t game) {
	return lib_cant_do_other(game, "wash");
}


/*
 * lib_dont_think_common()
 * lib_dont_think_object()
 * lib_dont_think_other()
 *
 * Central handler for a range of don't_think messages.  Still more
 * uninteresting responses.
 */
static sc_bool lib_dont_think_common(sc_gameref_t game,
                      const sc_char *verb, sc_bool is_object) {
	const sc_filterref_t filter = gs_get_filter(game);
	sc_int object;
	sc_bool is_ambiguous;

	/* If the target is not an object, end it here. */
	if (!is_object) {
		pf_buffer_string(filter,
		                 lib_select_response(game,
		                                     "I don't think you can ",
		                                     "I don't think I can ",
		                                     "I don't think %player% can "));
		pf_buffer_string(filter, verb);
		pf_buffer_string(filter, " that.\n");
		return TRUE;
	}

	/* Get the referenced object.  If none, return immediately. */
	object = lib_disambiguate_object(game, verb, &is_ambiguous);
	if (object == -1)
		return is_ambiguous;

	/* Whatever it is, don't do it. */
	pf_buffer_string(filter, "I don't think you can ");
	pf_buffer_string(filter, verb);
	pf_buffer_character(filter, ' ');
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}

static sc_bool lib_dont_think_object(sc_gameref_t game, const sc_char *verb) {
	return lib_dont_think_common(game, verb, TRUE);
}

static sc_bool lib_dont_think_other(sc_gameref_t game, const sc_char *verb) {
	return lib_dont_think_common(game, verb, FALSE);
}


/*
 * lib_cmd_*()
 *
 * Assorted don't-think messages.
 */
sc_bool lib_cmd_fix_object(sc_gameref_t game) {
	return lib_dont_think_object(game, "fix");
}

sc_bool lib_cmd_mend_object(sc_gameref_t game) {
	return lib_dont_think_object(game, "mend");
}

sc_bool lib_cmd_repair_object(sc_gameref_t game) {
	return lib_dont_think_object(game, "repair");
}

sc_bool lib_cmd_fix_other(sc_gameref_t game) {
	return lib_dont_think_other(game, "fix");
}

sc_bool lib_cmd_mend_other(sc_gameref_t game) {
	return lib_dont_think_other(game, "mend");
}

sc_bool lib_cmd_repair_other(sc_gameref_t game) {
	return lib_dont_think_other(game, "repair");
}


/*
 * lib_what()
 *
 * Central handler for doing something, but unsure to what.
 */
static sc_bool lib_what(sc_gameref_t game, const sc_char *verb) {
	const sc_filterref_t filter = gs_get_filter(game);

	pf_buffer_string(filter, verb);
	pf_buffer_string(filter, " what?\n");
	return TRUE;
}


/*
 * lib_cmd_*()
 *
 * Assorted "what?" messages.
 */
sc_bool lib_cmd_block_what(sc_gameref_t game) {
	return lib_what(game, "Block");
}

sc_bool lib_cmd_break_what(sc_gameref_t game) {
	return lib_what(game, "Break");
}

sc_bool lib_cmd_destroy_what(sc_gameref_t game) {
	return lib_what(game, "Destroy");
}

sc_bool lib_cmd_smash_what(sc_gameref_t game) {
	return lib_what(game, "Smash");
}

sc_bool lib_cmd_buy_what(sc_gameref_t game) {
	return lib_what(game, "Buy");
}

sc_bool lib_cmd_clean_what(sc_gameref_t game) {
	return lib_what(game, "Clean");
}

sc_bool lib_cmd_climb_what(sc_gameref_t game) {
	return lib_what(game, "Climb");
}

sc_bool lib_cmd_cut_what(sc_gameref_t game) {
	return lib_what(game, "Cut");
}

sc_bool lib_cmd_drink_what(sc_gameref_t game) {
	return lib_what(game, "Drink");
}

sc_bool lib_cmd_fix_what(sc_gameref_t game) {
	return lib_what(game, "Fix");
}

sc_bool lib_cmd_hit_what(sc_gameref_t game) {
	return lib_what(game, "Hit");
}

sc_bool lib_cmd_kick_what(sc_gameref_t game) {
	return lib_what(game, "Kick");
}

sc_bool lib_cmd_light_what(sc_gameref_t game) {
	return lib_what(game, "Light");
}

sc_bool lib_cmd_lift_what(sc_gameref_t game) {
	return lib_what(game, "Lift");
}

sc_bool lib_cmd_mend_what(sc_gameref_t game) {
	return lib_what(game, "Mend");
}

sc_bool lib_cmd_move_what(sc_gameref_t game) {
	return lib_what(game, "Move");
}

sc_bool lib_cmd_press_what(sc_gameref_t game) {
	return lib_what(game, "Press");
}

sc_bool lib_cmd_pull_what(sc_gameref_t game) {
	return lib_what(game, "Pull");
}

sc_bool lib_cmd_push_what(sc_gameref_t game) {
	return lib_what(game, "Push");
}

sc_bool lib_cmd_repair_what(sc_gameref_t game) {
	return lib_what(game, "Repair");
}

sc_bool lib_cmd_sell_what(sc_gameref_t game) {
	return lib_what(game, "Sell");
}

sc_bool lib_cmd_shake_what(sc_gameref_t game) {
	return lib_what(game, "Shake");
}

sc_bool lib_cmd_rub_what(sc_gameref_t game) {
	return lib_what(game, "Rub");
}

sc_bool lib_cmd_stop_what(sc_gameref_t game) {
	return lib_what(game, "Stop");
}

sc_bool lib_cmd_suck_what(sc_gameref_t game) {
	return lib_what(game, "Suck");
}

sc_bool lib_cmd_touch_what(sc_gameref_t game) {
	return lib_what(game, "Touch");
}

sc_bool lib_cmd_turn_what(sc_gameref_t game) {
	return lib_what(game, "Turn");
}

sc_bool lib_cmd_unblock_what(sc_gameref_t game) {
	return lib_what(game, "Unblock");
}

sc_bool lib_cmd_wash_what(sc_gameref_t game) {
	return lib_what(game, "Wash");
}

sc_bool lib_cmd_drop_what(sc_gameref_t game) {
	return lib_what(game, "Drop");
}

sc_bool lib_cmd_get_what(sc_gameref_t game) {
	return lib_what(game, "Take");
}

sc_bool lib_cmd_give_what(sc_gameref_t game) {
	return lib_what(game, "Give");
}

sc_bool lib_cmd_open_what(sc_gameref_t game) {
	return lib_what(game, "Open");
}

sc_bool lib_cmd_remove_what(sc_gameref_t game) {
	return lib_what(game, "Remove");
}

sc_bool lib_cmd_wear_what(sc_gameref_t game) {
	return lib_what(game, "Wear");
}

sc_bool lib_cmd_lock_what(sc_gameref_t game) {
	return lib_what(game, "Lock");
}

sc_bool lib_cmd_unlock_what(sc_gameref_t game) {
	return lib_what(game, "Unlock");
}


/*
 * lib_cmd_verb_object()
 * lib_cmd_verb_character()
 *
 * Handlers for unrecognized verbs with known object/NPC.
 */
sc_bool lib_cmd_verb_object(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int count, object, index_;

	/* Ensure the reference is unambiguous. */
	count = 0;
	object = -1;
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		if (game->object_references[index_]
		        && gs_object_seen(game, index_)
		        && obj_indirectly_in_room(game, index_, gs_playerroom(game))) {
			count++;
			object = index_;
		}
	}
	if (count != 1)
		return FALSE;

	/* Save in variables. */
	var_set_ref_object(vars, object);

	/* Print don't understand message. */
	pf_buffer_string(filter, "I don't understand what you want me to do with ");
	lib_print_object_np(game, object);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}

sc_bool lib_cmd_verb_npc(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int count, npc, index_;

	/* Ensure the reference is unambiguous. */
	count = 0;
	npc = -1;
	for (index_ = 0; index_ < gs_npc_count(game); index_++) {
		if (game->npc_references[index_]
		        && gs_npc_seen(game, index_)
		        && npc_in_room(game, index_, gs_playerroom(game))) {
			count++;
			npc = index_;
		}
	}
	if (count != 1)
		return FALSE;

	/* Save in variables. */
	var_set_ref_character(vars, npc);

	/* Print don't understand message; unlike objects, there's no "me" here. */
	pf_buffer_string(filter, "I don't understand what you want to do with ");
	lib_print_npc_np(game, npc);
	pf_buffer_string(filter, ".\n");
	return TRUE;
}


/*
 * lib_debug_trace()
 *
 * Set library tracing on/off.
 */
void lib_debug_trace(sc_bool flag) {
	lib_trace = flag;
}

} // End of namespace Adrift
} // End of namespace Glk
