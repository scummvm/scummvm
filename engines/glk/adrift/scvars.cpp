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
#include "glk/glk.h"
#include "glk/events.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o Gender enumerations are 0/1/2, but 1/2/3 in jAsea.  The 0/1/2 values
 *   seem to be right.  Is jAsea off by one?
 *
 * o jAsea tries to read Globals.CompileDate.  It's just CompileDate.
 *
 * o State_ and obstate are implemented, but not fully tested due to a lack
 *   of games that use them.
 */

/* Assorted definitions and constants. */
static const sc_uint VARS_MAGIC = 0xabcc7a71;
static const sc_char NUL = '\0';

/* Variables trace flag. */
static sc_bool var_trace = FALSE;

/* Table of numbers zero to twenty spelled out. */
enum { VAR_NUMBERS_SIZE = 21 };
static const sc_char *const VAR_NUMBERS[VAR_NUMBERS_SIZE] = {
	"zero", "one", "two", "three", "four", "five", "six", "seven", "eight",
	"nine", "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen",
	"sixteen", "seventeen", "eighteen", "nineteen", "twenty"
};

/* Variable entry, held on a list hashed by variable name. */
struct sc_var_s {
	struct sc_var_s *next;

	const sc_char *name;
	sc_int type;
	sc_vartype_t value;
};
typedef sc_var_s sc_var_t;
typedef sc_var_t *sc_varref_t;

/*
 * Variables set structure.  A self-contained set of variables on which
 * variables functions operate.  211 is prime, making it a reasonable hash
 * divisor.  There's no rehashing here; few games, if any, are likely to
 * exceed a fill factor of two (~422 variables).
 */
enum { VAR_HASH_TABLE_SIZE = 211 };
struct sc_var_set_s {
	sc_uint magic;
	sc_prop_setref_t bundle;
	sc_int referenced_character;
	sc_int referenced_object;
	sc_int referenced_number;
	sc_bool is_number_referenced;
	sc_char *referenced_text;
	sc_char *temporary;
	uint32 timestamp;
	sc_uint time_offset;
	sc_gameref_t game;
	sc_varref_t variable[VAR_HASH_TABLE_SIZE];
};
typedef sc_var_set_s sc_var_set_t;


/*
 * var_is_valid()
 *
 * Return TRUE if pointer is a valid variables set, FALSE otherwise.
 */
static sc_bool var_is_valid(sc_var_setref_t vars) {
	return vars && vars->magic == VARS_MAGIC;
}


/*
 * var_hash_name()
 *
 * Hash a variable name, modulo'ed to the number of buckets.
 */
static sc_uint var_hash_name(const sc_char *name) {
	return sc_hash(name) % VAR_HASH_TABLE_SIZE;
}


/*
 * var_create_empty()
 *
 * Create and return a new empty set of variables.
 */
static sc_var_setref_t var_create_empty(void) {
	sc_var_setref_t vars;
	sc_int index_;

	/* Create a clean set of variables. */
	vars = (sc_var_setref_t)sc_malloc(sizeof(*vars));
	vars->magic = VARS_MAGIC;
	vars->bundle = nullptr;
	vars->referenced_character = -1;
	vars->referenced_object = -1;
	vars->referenced_number = 0;
	vars->is_number_referenced = FALSE;
	vars->referenced_text = nullptr;
	vars->temporary = nullptr;
	vars->timestamp = g_vm->_events->getTotalPlayTicks() / 1000;
	vars->time_offset = 0;
	vars->game = nullptr;

	/* Clear all variable hash lists. */
	for (index_ = 0; index_ < VAR_HASH_TABLE_SIZE; index_++)
		vars->variable[index_] = nullptr;

	return vars;
}


/*
 * var_destroy()
 *
 * Destroy a variable set, and free its heap memory.
 */
void var_destroy(sc_var_setref_t vars) {
	sc_int index_;
	assert(var_is_valid(vars));

	/*
	 * Free the content of each string variable, and variable entry.  String
	 * variable content needs to use mutable string instead of const string.
	 */
	for (index_ = 0; index_ < VAR_HASH_TABLE_SIZE; index_++) {
		sc_varref_t var, next;

		for (var = vars->variable[index_]; var; var = next) {
			next = var->next;
			if (var->type == VAR_STRING)
				sc_free(var->value.mutable_string);
			sc_free(var);
		}
	}

	/* Free any temporary and reference text storage area. */
	sc_free(vars->temporary);
	sc_free(vars->referenced_text);

	/* Poison and free the variable set itself. */
	memset(vars, 0xaa, sizeof(*vars));
	sc_free(vars);
}


/*
 * var_find()
 * var_add()
 *
 * Find and return a pointer to a named variable structure, or nullptr if no such
 * variable exists, and add a new variable structure to the lists.
 */
static sc_varref_t var_find(sc_var_setref_t vars, const sc_char *name) {
	sc_uint hash;
	sc_varref_t var;

	/* Hash name, search list and return if name match found. */
	hash = var_hash_name(name);
	for (var = vars->variable[hash]; var; var = var->next) {
		if (strcmp(name, var->name) == 0)
			break;
	}

	/* Return variable, or nullptr if no such variable. */
	return var;
}

static sc_varref_t var_add(sc_var_setref_t vars, const sc_char *name, sc_int type) {
	sc_varref_t var;
	sc_uint hash;

	/* Create a new variable entry. */
	var = (sc_varref_t)sc_malloc(sizeof(*var));
	var->name = name;
	var->type = type;
	var->value.voidp = nullptr;

	/* Hash its name, and insert it at start of the relevant list. */
	hash = var_hash_name(name);
	var->next = vars->variable[hash];
	vars->variable[hash] = var;

	return var;
}


/*
 * var_get_scare_version()
 *
 * Return the value of %scare_version%.  Used to generate the system version
 * of this variable, and to re-initialize user versions initialized to zero.
 */
static sc_int var_get_scare_version(void) {
	sc_int major, minor, point, version;

	if (sscanf(SCARE_VERSION, "%ld.%ld.%ld", &major, &minor, &point) != 3) {
		sc_error("var_get_scare_version: unable to generate scare_version\n");
		return 0;
	}

	version = major * 10000 + minor * 100 + point;
	return version;
}


/*
 * var_put()
 *
 * Store a variable type in a named variable.  If not present, the variable
 * is created.  Type is one of 'I' or 'S' for integer or string.
 */
void var_put(sc_var_setref_t vars, const sc_char *name, sc_int type, sc_vartype_t vt_value) {
	sc_varref_t var;
	sc_bool is_modification;
	assert(var_is_valid(vars));
	assert(name);

	/* Check type is either integer or string. */
	switch (type) {
	case VAR_INTEGER:
	case VAR_STRING:
		break;

	default:
		sc_fatal("var_put: invalid variable type, %ld\n", type);
	}

	/* See if the user variable already exists. */
	var = var_find(vars, name);
	if (var) {
		/* Verify that nothing is trying to change the variable's type. */
		if (var->type != type)
			sc_fatal("var_put: variable type changed, %s\n", name);

		/*
		 * Special case %scare_version%.  If a game changes its value, it may
		 * compromise version checking, so warn here, but continue.
		 */
		if (strcmp(name, "scare_version") == 0) {
			if (var->value.integer != vt_value.integer)
				sc_error("var_put: warning: %%%s%% value changed\n", name);
		}

		is_modification = TRUE;
	} else {
		/*
		 * Special case %scare_version%.  If a game defines this and initializes
		 * it to zero, re-initialize it to SCARE's version number.  Games that
		 * define %scare_version%, initially zero, can use this to test if
		 * running under SCARE or Runner.
		 */
		if (strcmp(name, "scare_version") == 0 && vt_value.integer == 0) {
			vt_value.integer = var_get_scare_version();

			if (var_trace)
				sc_trace("Variable: %%%s%% [new] caught and mapped\n", name);
		}

		/*
		 * Create a new and empty variable entry.  The mutable string needs to
		 * be set to nullptr here so that realloc works correctly on assigning
		 * the value below.
		 */
		var = var_add(vars, name, type);
		var->value.mutable_string = nullptr;

		is_modification = FALSE;
	}

	/* Update the existing variable, or populate the new one fully. */
	switch (var->type) {
	case VAR_INTEGER:
		var->value.integer = vt_value.integer;
		break;

	case VAR_STRING:
		/* Use mutable string instead of const string. */
		var->value.mutable_string = (sc_char *)sc_realloc(var->value.mutable_string,
		                            strlen(vt_value.string) + 1);
		strcpy(var->value.mutable_string, vt_value.string);
		break;

	default:
		sc_fatal("var_put: invalid variable type, %ld\n", var->type);
	}

	if (var_trace) {
		sc_trace("Variable: %%%s%%%s = ",
		         name, is_modification ? "" : " [new]");
		switch (var->type) {
		case VAR_INTEGER:
			sc_trace("%ld", var->value.integer);
			break;
		case VAR_STRING:
			sc_trace("\"%s\"", var->value.string);
			break;

		default:
			sc_trace("[invalid variable type, %ld]", var->type);
			break;
		}
		sc_trace("\n");
	}
}


/*
 * var_append_temp()
 *
 * Helper for object listers.  Extends temporary, and appends the given text
 * to the string.
 */
static void var_append_temp(sc_var_setref_t vars, const sc_char *string) {
	sc_bool new_sentence;
	sc_int noted;

	if (!vars->temporary) {
		/* Create a new temporary area and copy string. */
		new_sentence = TRUE;
		noted = 0;
		vars->temporary = (sc_char *)sc_malloc(strlen(string) + 1);
		strcpy(vars->temporary, string);
	} else {
		/* Append string to existing temporary. */
		new_sentence = (vars->temporary[0] == NUL);
		noted = strlen(vars->temporary);
		vars->temporary = (sc_char *)sc_realloc(vars->temporary,
		                                        strlen(vars->temporary) +
		                                        strlen(string) + 1);
		strcat(vars->temporary, string);
	}

	if (new_sentence)
		vars->temporary[noted] = sc_toupper(vars->temporary[noted]);
}


/*
 * var_print_object_np
 * var_print_object
 *
 * Convenience functions to append an object's name, with and without any
 * prefix, to variables temporary.
 */
static void var_print_object_np(sc_gameref_t game, sc_int object) {
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	const sc_char *prefix, *normalized, *name;

	/* Get the object's prefix. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Prefix";
	prefix = prop_get_string(bundle, "S<-sis", vt_key);

	/*
	 * Try the same shenanigans as done by the equivalent function in the
	 * library.
	 */
	normalized = prefix;
	if (sc_compare_word(prefix, "a", 1)) {
		normalized = prefix + 1;
		var_append_temp(vars, "the");
	} else if (sc_compare_word(prefix, "an", 2)) {
		normalized = prefix + 2;
		var_append_temp(vars, "the");
	} else if (sc_compare_word(prefix, "the", 3)) {
		normalized = prefix + 3;
		var_append_temp(vars, "the");
	} else if (sc_compare_word(prefix, "some", 4)) {
		normalized = prefix + 4;
		var_append_temp(vars, "the");
	} else if (sc_strempty(prefix))
		var_append_temp(vars, "the ");

	/* As with the library, handle the remaining prefix. */
	if (!sc_strempty(normalized)) {
		var_append_temp(vars, normalized);
		var_append_temp(vars, " ");
	} else if (normalized > prefix)
		var_append_temp(vars, " ");

	/*
	 * Print the object's name, again, as with the library, stripping any
	 * leading article
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
	var_append_temp(vars, name);
}

static void var_print_object(sc_gameref_t game, sc_int object) {
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	const sc_char *prefix, *name;

	/*
	 * Get the object's prefix.  As with the library, if the prefix is empty,
	 * put in an "a ".
	 */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Prefix";
	prefix = prop_get_string(bundle, "S<-sis", vt_key);
	if (!sc_strempty(prefix)) {
		var_append_temp(vars, prefix);
		var_append_temp(vars, " ");
	} else
		var_append_temp(vars, "a ");

	/* Print the object's name. */
	vt_key[2].string = "Short";
	name = prop_get_string(bundle, "S<-sis", vt_key);
	var_append_temp(vars, name);
}


/*
 * var_select_plurality()
 *
 * Convenience function for listers.  Selects one of two responses depending
 * on whether an object appears singular or plural.
 */
static const sc_char *var_select_plurality(sc_gameref_t game, sc_int object,
		const sc_char *singular, const sc_char *plural) {
	return obj_appears_plural(game, object) ? plural : singular;
}


/*
 * var_list_in_object()
 *
 * List the objects in a given container object.
 */
static void var_list_in_object(sc_gameref_t game, sc_int container) {
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int object, count, trail;

	/* List out the objects contained in this object. */
	count = 0;
	trail = -1;
	for (object = 0; object < gs_object_count(game); object++) {
		/* Contained? */
		if (gs_object_position(game, object) == OBJ_IN_OBJECT
		        && gs_object_parent(game, object) == container) {
			if (count > 0) {
				if (count > 1)
					var_append_temp(vars, ", ");

				/* Print out the current list object. */
				var_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			var_print_object(game, trail);
			var_append_temp(vars,
			                var_select_plurality(game, trail,
			                                     " is inside ",
			                                     " are inside "));
		} else {
			var_append_temp(vars, " and ");
			var_print_object(game, trail);
			var_append_temp(vars, " are inside ");
		}

		/* Print out the container. */
		var_print_object_np(game, container);
		var_append_temp(vars, ".");
	}
}


/*
 * var_list_on_object()
 *
 * List the objects on a given surface object.
 */
static void var_list_on_object(sc_gameref_t game, sc_int supporter) {
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int object, count, trail;

	/* List out the objects standing on this object. */
	count = 0;
	trail = -1;
	for (object = 0; object < gs_object_count(game); object++) {
		/* Standing on? */
		if (gs_object_position(game, object) == OBJ_ON_OBJECT
		        && gs_object_parent(game, object) == supporter) {
			if (count > 0) {
				if (count > 1)
					var_append_temp(vars, ", ");

				/* Print out the current list object. */
				var_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			var_print_object(game, trail);
			var_append_temp(vars,
			                var_select_plurality(game, trail,
			                                     " is on ", " are on "));
		} else {
			var_append_temp(vars, " and ");
			var_print_object(game, trail);
			var_append_temp(vars, " are on ");
		}

		/* Print out the surface. */
		var_print_object_np(game, supporter);
		var_append_temp(vars, ".");
	}
}


/*
 * var_list_onin_object()
 *
 * List the objects on and in a given associate object.
 */
static void var_list_onin_object(sc_gameref_t game, sc_int associate) {
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int object, count, trail;
	sc_bool supporting;

	/* List out the objects standing on this object. */
	count = 0;
	trail = -1;
	supporting = FALSE;
	for (object = 0; object < gs_object_count(game); object++) {
		/* Standing on? */
		if (gs_object_position(game, object) == OBJ_ON_OBJECT
		        && gs_object_parent(game, object) == associate) {
			if (count > 0) {
				if (count > 1)
					var_append_temp(vars, ", ");

				/* Print out the current list object. */
				var_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			var_print_object(game, trail);
			var_append_temp(vars,
			                var_select_plurality(game, trail,
			                                     " is on ", " are on "));
		} else {
			var_append_temp(vars, " and ");
			var_print_object(game, trail);
			var_append_temp(vars, " are on ");
		}

		/* Print out the surface. */
		var_print_object_np(game, associate);
		supporting = TRUE;
	}

	/* List out the objects contained in this object. */
	count = 0;
	trail = -1;
	for (object = 0; object < gs_object_count(game); object++) {
		/* Contained? */
		if (gs_object_position(game, object) == OBJ_IN_OBJECT
		        && gs_object_parent(game, object) == associate) {
			if (count > 0) {
				if (count == 1) {
					if (supporting)
						var_append_temp(vars, ", and ");
				} else
					var_append_temp(vars, ", ");

				/* Print out the current list object. */
				var_print_object(game, trail);
			}
			trail = object;
			count++;
		}
	}
	if (count >= 1) {
		/* Print out final listed object. */
		if (count == 1) {
			if (supporting)
				var_append_temp(vars, ", and ");
			var_print_object(game, trail);
			var_append_temp(vars,
			                var_select_plurality(game, trail,
			                                     " is inside ",
			                                     " are inside "));
		} else {
			var_append_temp(vars, " and ");
			var_print_object(game, trail);
			var_append_temp(vars, " are inside");
		}

		/* Print out the container. */
		if (!supporting) {
			var_append_temp(vars, " ");
			var_print_object_np(game, associate);
		}
		var_append_temp(vars, ".");
	} else {
		if (supporting)
			var_append_temp(vars, ".");
	}
}


/*
 * var_return_integer()
 * var_return_string()
 *
 * Convenience helpers for var_get_system().  Provide convenience and some
 * mild syntactic sugar for making returning a value as a system variable
 * a bit easier.  Set appropriate values for return type and the relevant
 * return value field, and always return TRUE.  A macro was tempting here...
 */
static sc_bool var_return_integer(sc_int value, sc_int *type, sc_vartype_t *vt_rvalue) {
	*type = VAR_INTEGER;
	vt_rvalue->integer = value;
	return TRUE;
}

static sc_bool var_return_string(const sc_char *value, sc_int *type, sc_vartype_t *vt_rvalue) {
	*type = VAR_STRING;
	vt_rvalue->string = value;
	return TRUE;
}


/*
 * var_get_system()
 *
 * Construct a system variable, and return its type and value, or FALSE
 * if invalid name passed in.  Uses var_return_*() to reduce code untidiness.
 */
static sc_bool var_get_system(sc_var_setref_t vars, const sc_char *name,
		sc_int *type, sc_vartype_t *vt_rvalue) {
	const sc_prop_setref_t bundle = vars->bundle;
	const sc_gameref_t game = vars->game;

	/* Check name for known system variables. */
	if (strcmp(name, "author") == 0) {
		sc_vartype_t vt_key[2];
		const sc_char *author;

		/* Get and return the global gameauthor string. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "GameAuthor";
		author = prop_get_string(bundle, "S<-ss", vt_key);
		if (sc_strempty(author))
			author = "[Author unknown]";

		return var_return_string(author, type, vt_rvalue);
	}

	else if (strcmp(name, "character") == 0) {
		/* See if there is a referenced character. */
		if (vars->referenced_character != -1) {
			sc_vartype_t vt_key[3];
			const sc_char *npc_name;

			/* Return the character name string. */
			vt_key[0].string = "NPCs";
			vt_key[1].integer = vars->referenced_character;
			vt_key[2].string = "Name";
			npc_name = prop_get_string(bundle, "S<-sis", vt_key);
			if (sc_strempty(npc_name))
				npc_name = "[Character unknown]";

			return var_return_string(npc_name, type, vt_rvalue);
		} else {
			sc_error("var_get_system: no referenced character yet\n");
			return var_return_string("[Character unknown]", type, vt_rvalue);
		}
	}

	else if (strcmp(name, "heshe") == 0 || strcmp(name, "himher") == 0) {
		/* See if there is a referenced character. */
		if (vars->referenced_character != -1) {
			sc_vartype_t vt_key[3];
			sc_int gender;
			const sc_char *retval;

			/* Return the appropriate character gender string. */
			vt_key[0].string = "NPCs";
			vt_key[1].integer = vars->referenced_character;
			vt_key[2].string = "Gender";
			gender = prop_get_integer(bundle, "I<-sis", vt_key);
			switch (gender) {
			case NPC_MALE:
				retval = (strcmp(name, "heshe") == 0) ? "he" : "him";
				break;
			case NPC_FEMALE:
				retval = (strcmp(name, "heshe") == 0) ? "she" : "her";
				break;
			case NPC_NEUTER:
				retval = "it";
				break;

			default:
				sc_error("var_get_system: unknown gender, %ld\n", gender);
				retval = "[Gender unknown]";
				break;
			}
			return var_return_string(retval, type, vt_rvalue);
		} else {
			sc_error("var_get_system: no referenced character yet\n");
			return var_return_string("[Gender unknown]", type, vt_rvalue);
		}
	}

	else if (strncmp(name, "in_", 3) == 0) {
		sc_int saved_ref_object = vars->referenced_object;

		/* Check there's enough information to return a value. */
		if (!game) {
			sc_error("var_get_system: no game for in_\n");
			return var_return_string("[In_ unavailable]", type, vt_rvalue);
		}
		if (!uip_match("%object%", name + 3, game)) {
			sc_error("var_get_system: invalid object for in_\n");
			return var_return_string("[In_ unavailable]", type, vt_rvalue);
		}

		/* Clear any current temporary for appends. */
		vars->temporary = (sc_char *)sc_realloc(vars->temporary, 1);
		strcpy(vars->temporary, "");

		/* Write what's in the object into temporary. */
		var_list_in_object(game, vars->referenced_object);

		/* Restore saved referenced object and return. */
		vars->referenced_object = saved_ref_object;
		return var_return_string(vars->temporary, type, vt_rvalue);
	}

	else if (strcmp(name, "maxscore") == 0) {
		sc_vartype_t vt_key[2];
		sc_int maxscore;

		/* Return the maximum score. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "MaxScore";
		maxscore = prop_get_integer(bundle, "I<-ss", vt_key);

		return var_return_integer(maxscore, type, vt_rvalue);
	}

	else if (strcmp(name, "modified") == 0) {
		sc_vartype_t vt_key;
		const sc_char *compiledate;

		/* Return the game compilation date. */
		vt_key.string = "CompileDate";
		compiledate = prop_get_string(bundle, "S<-s", &vt_key);
		if (sc_strempty(compiledate))
			compiledate = "[Modified unknown]";

		return var_return_string(compiledate, type, vt_rvalue);
	}

	else if (strcmp(name, "number") == 0) {
		/* Return the referenced number, or 0 if none yet. */
		if (!vars->is_number_referenced)
			sc_error("var_get_system: no referenced number yet\n");

		return var_return_integer(vars->referenced_number, type, vt_rvalue);
	}

	else if (strcmp(name, "object") == 0) {
		/* See if we have a referenced object yet. */
		if (vars->referenced_object != -1) {
			/* Return object name with its prefix. */
			sc_vartype_t vt_key[3];
			const sc_char *prefix, *objname;

			vt_key[0].string = "Objects";
			vt_key[1].integer = vars->referenced_object;
			vt_key[2].string = "Prefix";
			prefix = prop_get_string(bundle, "S<-sis", vt_key);

			vars->temporary = (sc_char *)sc_realloc(vars->temporary, strlen(prefix) + 1);
			strcpy(vars->temporary, prefix);

			vt_key[2].string = "Short";
			objname = prop_get_string(bundle, "S<-sis", vt_key);

			vars->temporary = (sc_char *)sc_realloc(vars->temporary,
			                                        strlen(vars->temporary)
			                                        + strlen(objname) + 2);
			strcat(vars->temporary, " ");
			strcat(vars->temporary, objname);

			return var_return_string(vars->temporary, type, vt_rvalue);
		} else {
			sc_error("var_get_system: no referenced object yet\n");
			return var_return_string("[Object unknown]", type, vt_rvalue);
		}
	}

	else if (strcmp(name, "obstate") == 0) {
		sc_vartype_t vt_key[3];
		sc_bool is_statussed;
		sc_char *state;

		/* Check there's enough information to return a value. */
		if (!game) {
			sc_error("var_get_system: no game for obstate\n");
			return var_return_string("[Obstate unavailable]", type, vt_rvalue);
		}
		if (vars->referenced_object == -1) {
			sc_error("var_get_system: no object for obstate\n");
			return var_return_string("[Obstate unavailable]", type, vt_rvalue);
		}

		/*
		 * If not a stateful object, Runner 4.0.45 crashes; we'll do something
		 * different here.
		 */
		vt_key[0].string = "Objects";
		vt_key[1].integer = vars->referenced_object;
		vt_key[2].string = "CurrentState";
		is_statussed = prop_get_integer(bundle, "I<-sis", vt_key) != 0;
		if (!is_statussed)
			return var_return_string("stateless", type, vt_rvalue);

		/* Get state, and copy to temporary. */
		state = obj_state_name(game, vars->referenced_object);
		if (!state) {
			sc_error("var_get_system: invalid state for obstate\n");
			return var_return_string("[Obstate unknown]", type, vt_rvalue);
		}
		vars->temporary = (sc_char *)sc_realloc(vars->temporary, strlen(state) + 1);
		strcpy(vars->temporary, state);
		sc_free(state);

		/* Return temporary. */
		return var_return_string(vars->temporary, type, vt_rvalue);
	}

	else if (strcmp(name, "obstatus") == 0) {
		sc_vartype_t vt_key[3];
		sc_bool is_openable;
		sc_int openness;
		const sc_char *retval;

		/* Check there's enough information to return a value. */
		if (!game) {
			sc_error("var_get_system: no game for obstatus\n");
			return var_return_string("[Obstatus unavailable]", type, vt_rvalue);
		}
		if (vars->referenced_object == -1) {
			sc_error("var_get_system: no object for obstatus\n");
			return var_return_string("[Obstatus unavailable]", type, vt_rvalue);
		}

		/* If not an openable object, return unopenable to match Adrift. */
		vt_key[0].string = "Objects";
		vt_key[1].integer = vars->referenced_object;
		vt_key[2].string = "Openable";
		is_openable = prop_get_integer(bundle, "I<-sis", vt_key) != 0;
		if (!is_openable)
			return var_return_string("unopenable", type, vt_rvalue);

		/* Return one of open, closed, or locked. */
		openness = gs_object_openness(game, vars->referenced_object);
		switch (openness) {
		case OBJ_OPEN:
			retval = "open";
			break;
		case OBJ_CLOSED:
			retval = "closed";
			break;
		case OBJ_LOCKED:
			retval = "locked";
			break;
		default:
			retval = "[Obstatus unknown]";
			break;
		}
		return var_return_string(retval, type, vt_rvalue);
	}

	else if (strncmp(name, "on_", 3) == 0) {
		sc_int saved_ref_object = vars->referenced_object;

		/* Check there's enough information to return a value. */
		if (!game) {
			sc_error("var_get_system: no game for on_\n");
			return var_return_string("[On_ unavailable]", type, vt_rvalue);
		}
		if (!uip_match("%object%", name + 3, game)) {
			sc_error("var_get_system: invalid object for on_\n");
			return var_return_string("[On_ unavailable]", type, vt_rvalue);
		}

		/* Clear any current temporary for appends. */
		vars->temporary = (sc_char *)sc_realloc(vars->temporary, 1);
		strcpy(vars->temporary, "");

		/* Write what's on the object into temporary. */
		var_list_on_object(game, vars->referenced_object);

		/* Restore saved referenced object and return. */
		vars->referenced_object = saved_ref_object;
		return var_return_string(vars->temporary, type, vt_rvalue);
	}

	else if (strncmp(name, "onin_", 5) == 0) {
		sc_int saved_ref_object = vars->referenced_object;

		/* Check there's enough information to return a value. */
		if (!game) {
			sc_error("var_get_system: no game for onin_\n");
			return var_return_string("[Onin_ unavailable]", type, vt_rvalue);
		}
		if (!uip_match("%object%", name + 5, game)) {
			sc_error("var_get_system: invalid object for onin_\n");
			return var_return_string("[Onin_ unavailable]", type, vt_rvalue);
		}

		/* Clear any current temporary for appends. */
		vars->temporary = (sc_char *)sc_realloc(vars->temporary, 1);
		strcpy(vars->temporary, "");

		/* Write what's on/in the object into temporary. */
		var_list_onin_object(game, vars->referenced_object);

		/* Restore saved referenced object and return. */
		vars->referenced_object = saved_ref_object;
		return var_return_string(vars->temporary, type, vt_rvalue);
	}

	else if (strcmp(name, "player") == 0) {
		sc_vartype_t vt_key[2];
		const sc_char *playername;

		/*
		 * Return player's name from properties, or just "Player" if not set
		 * in the properties.
		 */
		vt_key[0].string = "Globals";
		vt_key[1].string = "PlayerName";
		playername = prop_get_string(bundle, "S<-ss", vt_key);
		if (sc_strempty(playername))
			playername = "Player";

		return var_return_string(playername, type, vt_rvalue);
	}

	else if (strcmp(name, "room") == 0) {
		const sc_char *roomname;

		/* Check there's enough information to return a value. */
		if (!game) {
			sc_error("var_get_system: no game for room\n");
			return var_return_string("[Room unavailable]", type, vt_rvalue);
		}

		/* Return the current player room. */
		roomname = lib_get_room_name(game, gs_playerroom(game));
		return var_return_string(roomname, type, vt_rvalue);
	}

	else if (strcmp(name, "score") == 0) {
		/* Check there's enough information to return a value. */
		if (!game) {
			sc_error("var_get_system: no game for score\n");
			return var_return_integer(0, type, vt_rvalue);
		}

		/* Return the current game score. */
		return var_return_integer(game->score, type, vt_rvalue);
	}

	else if (strncmp(name, "state_", 6) == 0) {
		sc_int saved_ref_object = vars->referenced_object;
		sc_vartype_t vt_key[3];
		sc_bool is_statussed;
		sc_char *state;

		/* Check there's enough information to return a value. */
		if (!game) {
			sc_error("var_get_system: no game for state_\n");
			return var_return_string("[State_ unavailable]", type, vt_rvalue);
		}
		if (!uip_match("%object%", name + 6, game)) {
			sc_error("var_get_system: invalid object for state_\n");
			return var_return_string("[State_ unavailable]", type, vt_rvalue);
		}

		/* Verify this is a stateful object. */
		vt_key[0].string = "Objects";
		vt_key[1].integer = vars->referenced_object;
		vt_key[2].string = "CurrentState";
		is_statussed = prop_get_integer(bundle, "I<-sis", vt_key) != 0;
		if (!is_statussed) {
			vars->referenced_object = saved_ref_object;
			sc_error("var_get_system: stateless object for state_\n");
			return var_return_string("[State_ unavailable]", type, vt_rvalue);
		}

		/* Get state, and copy to temporary. */
		state = obj_state_name(game, vars->referenced_object);
		if (!state) {
			vars->referenced_object = saved_ref_object;
			sc_error("var_get_system: invalid state for state_\n");
			return var_return_string("[State_ unknown]", type, vt_rvalue);
		}
		vars->temporary = (sc_char *)sc_realloc(vars->temporary, strlen(state) + 1);
		strcpy(vars->temporary, state);
		sc_free(state);

		/* Restore saved referenced object and return. */
		vars->referenced_object = saved_ref_object;
		return var_return_string(vars->temporary, type, vt_rvalue);
	}

	else if (strncmp(name, "status_", 7) == 0) {
		sc_int saved_ref_object = vars->referenced_object;
		sc_vartype_t vt_key[3];
		sc_bool is_openable;
		sc_int openness;
		const sc_char *retval;

		/* Check there's enough information to return a value. */
		if (!game) {
			sc_error("var_get_system: no game for status_\n");
			return var_return_string("[Status_ unavailable]", type, vt_rvalue);
		}
		if (!uip_match("%object%", name + 7, game)) {
			sc_error("var_get_system: invalid object for status_\n");
			return var_return_string("[Status_ unavailable]", type, vt_rvalue);
		}

		/* Verify this is an openable object. */
		vt_key[0].string = "Objects";
		vt_key[1].integer = vars->referenced_object;
		vt_key[2].string = "Openable";
		is_openable = prop_get_integer(bundle, "I<-sis", vt_key) != 0;
		if (!is_openable) {
			vars->referenced_object = saved_ref_object;
			sc_error("var_get_system: stateless object for status_\n");
			return var_return_string("[Status_ unavailable]", type, vt_rvalue);
		}

		/* Return one of open, closed, or locked. */
		openness = gs_object_openness(game, vars->referenced_object);
		switch (openness) {
		case OBJ_OPEN:
			retval = "open";
			break;
		case OBJ_CLOSED:
			retval = "closed";
			break;
		case OBJ_LOCKED:
			retval = "locked";
			break;
		default:
			retval = "[Status_ unknown]";
			break;
		}

		/* Restore saved referenced object and return. */
		vars->referenced_object = saved_ref_object;
		return var_return_string(retval, type, vt_rvalue);
	}

	else if (strcmp(name, "t_number") == 0) {
		/* See if we have a referenced number yet. */
		if (vars->is_number_referenced) {
			sc_int number;
			const sc_char *retval;

			/* Return the referenced number as a string. */
			number = vars->referenced_number;
			if (number >= 0 && number < VAR_NUMBERS_SIZE)
				retval = VAR_NUMBERS[number];
			else {
				vars->temporary = (sc_char *)sc_realloc(vars->temporary, 32);
				sprintf(vars->temporary, "%ld", number);
				retval = vars->temporary;
			}

			return var_return_string(retval, type, vt_rvalue);
		} else {
			sc_error("var_get_system: no referenced number yet\n");
			return var_return_string("[Number unknown]", type, vt_rvalue);
		}
	}

	else if (strncmp(name, "t_", 2) == 0) {
		sc_varref_t var;

		/* Find the variable; must be a user, not a system, one. */
		var = var_find(vars, name + 2);
		if (!var) {
			sc_error("var_get_system:"
			         " no such variable, %s\n", name + 2);
			return var_return_string("[Unknown variable]", type, vt_rvalue);
		} else if (var->type != VAR_INTEGER) {
			sc_error("var_get_system:"
			         " not an integer variable, %s\n", name + 2);
			return var_return_string(var->value.string, type, vt_rvalue);
		} else {
			sc_int number;
			const sc_char *retval;

			/* Return the variable value as a string. */
			number = var->value.integer;
			if (number >= 0 && number < VAR_NUMBERS_SIZE)
				retval = VAR_NUMBERS[number];
			else {
				vars->temporary = (sc_char *)sc_realloc(vars->temporary, 32);
				sprintf(vars->temporary, "%ld", number);
				retval = vars->temporary;
			}

			return var_return_string(retval, type, vt_rvalue);
		}
	}

	else if (strcmp(name, "text") == 0) {
		const sc_char *retval;

		/* Return any referenced text, otherwise a neutral string. */
		if (vars->referenced_text)
			retval = vars->referenced_text;
		else {
			sc_error("var_get_system: no text yet to reference\n");
			retval = "[Text unknown]";
		}

		return var_return_string(retval, type, vt_rvalue);
	}

	else if (strcmp(name, "theobject") == 0) {
		/* See if we have a referenced object yet. */
		if (vars->referenced_object != -1) {
			/* Return object name prefixed with "the"... */
			sc_vartype_t vt_key[3];
			const sc_char *prefix, *normalized, *objname;

			vt_key[0].string = "Objects";
			vt_key[1].integer = vars->referenced_object;
			vt_key[2].string = "Prefix";
			prefix = prop_get_string(bundle, "S<-sis", vt_key);

			vars->temporary = (sc_char *)sc_realloc(vars->temporary, strlen(prefix) + 5);
			strcpy(vars->temporary, "");

			normalized = prefix;
			if (sc_compare_word(prefix, "a", 1)) {
				strcat(vars->temporary, "the");
				normalized = prefix + 1;
			} else if (sc_compare_word(prefix, "an", 2)) {
				strcat(vars->temporary, "the");
				normalized = prefix + 2;
			} else if (sc_compare_word(prefix, "the", 3)) {
				strcat(vars->temporary, "the");
				normalized = prefix + 3;
			} else if (sc_compare_word(prefix, "some", 4)) {
				strcat(vars->temporary, "the");
				normalized = prefix + 4;
			} else if (sc_strempty(prefix))
				strcat(vars->temporary, "the ");

			if (!sc_strempty(normalized)) {
				strcat(vars->temporary, normalized);
				strcat(vars->temporary, " ");
			} else if (normalized > prefix)
				strcat(vars->temporary, " ");

			vt_key[2].string = "Short";
			objname = prop_get_string(bundle, "S<-sis", vt_key);
			if (sc_compare_word(objname, "a", 1))
				objname += 1;
			else if (sc_compare_word(objname, "an", 2))
				objname += 2;
			else if (sc_compare_word(objname, "the", 3))
				objname += 3;
			else if (sc_compare_word(objname, "some", 4))
				objname += 4;

			vars->temporary = (sc_char *)sc_realloc(vars->temporary,
			                                        strlen(vars->temporary)
			                                        + strlen(objname) + 1);
			strcat(vars->temporary, objname);

			return var_return_string(vars->temporary, type, vt_rvalue);
		} else {
			sc_error("var_get_system: no referenced object yet\n");
			return var_return_string("[Object unknown]", type, vt_rvalue);
		}
	}

	else if (strcmp(name, "time") == 0) {
		double delta;
		sc_int retval;

		/* Return the elapsed game time in seconds. */
		delta = vars->timestamp - (g_vm->_events->getTotalPlayTicks() / 1000);
		retval = (sc_int) delta + vars->time_offset;

		return var_return_integer(retval, type, vt_rvalue);
	}

	else if (strcmp(name, "title") == 0) {
		sc_vartype_t vt_key[2];
		const sc_char *gamename;

		/* Return the game's title. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "GameName";
		gamename = prop_get_string(bundle, "S<-ss", vt_key);
		if (sc_strempty(gamename))
			gamename = "[Title unknown]";

		return var_return_string(gamename, type, vt_rvalue);
	}

	else if (strcmp(name, "turns") == 0) {
		/* Check there's enough information to return a value. */
		if (!game) {
			sc_error("var_get_system: no game for turns\n");
			return var_return_integer(0, type, vt_rvalue);
		}

		/* Return the count of game turns. */
		return var_return_integer(game->turns, type, vt_rvalue);
	}

	else if (strcmp(name, "version") == 0) {
		/* Return the Adrift emulation level of SCARE. */
		return var_return_integer(SCARE_EMULATION, type, vt_rvalue);
	}

	else if (strcmp(name, "scare_version") == 0) {
		/* Private system variable, return SCARE's version number. */
		return var_return_integer(var_get_scare_version(), type, vt_rvalue);
	}

	return FALSE;
}


/*
 * var_get_user()
 *
 * Retrieve a user variable, and return its type and value, or FALSE if the
 * name passed in is not a defined user variable.
 */
static sc_bool var_get_user(sc_var_setref_t vars, const sc_char *name,
		sc_int *type, sc_vartype_t *vt_rvalue) {
	sc_varref_t var;

	/* Check user variables for a reference to the named variable. */
	var = var_find(vars, name);
	if (var) {
		/* Copy out variable details. */
		*type = var->type;
		switch (var->type) {
		case VAR_INTEGER:
			vt_rvalue->integer = var->value.integer;
			break;
		case VAR_STRING:
			vt_rvalue->string = var->value.string;
			break;

		default:
			sc_fatal("var_get_user: invalid variable type, %ld\n", var->type);
		}

		/* Return success. */
		return TRUE;
	}

	return FALSE;
}


/*
 * var_get()
 *
 * Retrieve a variable, and return its value and type.  Returns FALSE if the
 * named variable does not exist.
 */
sc_bool var_get(sc_var_setref_t vars, const sc_char *name, sc_int *type, sc_vartype_t *vt_rvalue) {
	sc_bool status;
	assert(var_is_valid(vars));
	assert(name && type && vt_rvalue);

	/*
	 * Check user and system variables for a reference to the name.  User
	 * variables take precedence over system ones; that is, they may override
	 * them in a game.
	 */
	status = var_get_user(vars, name, type, vt_rvalue);
	if (!status)
		status = var_get_system(vars, name, type, vt_rvalue);

	if (var_trace) {
		if (status) {
			sc_trace("Variable: %%%s%% retrieved, ", name);
			switch (*type) {
			case VAR_INTEGER:
				sc_trace("%ld", vt_rvalue->integer);
				break;
			case VAR_STRING:
				sc_trace("\"%s\"", vt_rvalue->string);
				break;

			default:
				sc_trace("Variable: invalid variable type, %ld\n", *type);
				break;
			}
			sc_trace("\n");
		} else
			sc_trace("Variable: \"%s\", no such variable\n", name);
	}

	return status;
}


/*
 * var_put_integer()
 * var_get_integer()
 *
 * Convenience functions to store and retrieve an integer variable.  It is
 * an error for the variable not to exist or to have the wrong type.
 */
void var_put_integer(sc_var_setref_t vars, const sc_char *name, sc_int value) {
	sc_vartype_t vt_value;
	assert(var_is_valid(vars));

	vt_value.integer = value;
	var_put(vars, name, VAR_INTEGER, vt_value);
}

sc_int var_get_integer(sc_var_setref_t vars, const sc_char *name) {
	sc_vartype_t vt_rvalue;
	sc_int type;
	assert(var_is_valid(vars));

	if (!var_get(vars, name, &type, &vt_rvalue))
		sc_fatal("var_get_integer: no such variable, %s\n", name);
	else if (type != VAR_INTEGER)
		sc_fatal("var_get_integer: not an integer, %s\n", name);

	return vt_rvalue.integer;
}


/*
 * var_put_string()
 * var_get_string()
 *
 * Convenience functions to store and retrieve a string variable.  It is
 * an error for the variable not to exist or to have the wrong type.
 */
void var_put_string(sc_var_setref_t vars, const sc_char *name, const sc_char *string) {
	sc_vartype_t vt_value;
	assert(var_is_valid(vars));

	vt_value.string = string;
	var_put(vars, name, VAR_STRING, vt_value);
}

const sc_char *var_get_string(sc_var_setref_t vars, const sc_char *name) {
	sc_vartype_t vt_rvalue;
	sc_int type;
	assert(var_is_valid(vars));

	if (!var_get(vars, name, &type, &vt_rvalue))
		sc_fatal("var_get_string: no such variable, %s\n", name);
	else if (type != VAR_STRING)
		sc_fatal("var_get_string: not a string, %s\n", name);

	return vt_rvalue.string;
}


/*
 * var_create()
 *
 * Create and return a new set of variables.  Variables are created from the
 * properties bundle passed in.
 */
sc_var_setref_t var_create(sc_prop_setref_t bundle) {
	sc_var_setref_t vars;
	sc_int var_count, index_;
	sc_vartype_t vt_key[3];
	assert(bundle);

	/* Create a clean set of variables to fill from the bundle. */
	vars = var_create_empty();
	vars->bundle = bundle;

	/* Retrieve the count of variables. */
	vt_key[0].string = "Variables";
	var_count = prop_get_child_count(bundle, "I<-s", vt_key);

	/* Create a variable for each variable property held. */
	for (index_ = 0; index_ < var_count; index_++) {
		const sc_char *name;
		sc_int var_type;
		const sc_char *value;

		/* Retrieve variable name, type, and string initial value. */
		vt_key[1].integer = index_;
		vt_key[2].string = "Name";
		name = prop_get_string(bundle, "S<-sis", vt_key);

		vt_key[2].string = "Type";
		var_type = prop_get_integer(bundle, "I<-sis", vt_key);

		vt_key[2].string = "Value";
		value = prop_get_string(bundle, "S<-sis", vt_key);

		/* Handle numerics and strings differently. */
		switch (var_type) {
		case TAFVAR_NUMERIC: {
			sc_int integer_value;
			if (sscanf(value, "%ld", &integer_value) != 1) {
				sc_error("var_create:"
				         " invalid numeric variable %s, %s\n", name, value);
				integer_value = 0;
			}
			var_put_integer(vars, name, integer_value);
			break;
		}

		case TAFVAR_STRING:
			var_put_string(vars, name, value);
			break;

		default:
			sc_fatal("var_create: invalid variable type, %ld\n", var_type);
		}
	}

	return vars;
}


/*
 * var_register_game()
 *
 * Register the game, used by variables to satisfy requests for selected
 * system variables.  To ensure integrity, the game being registered must
 * reference this variable set.
 */
void var_register_game(sc_var_setref_t vars, sc_gameref_t game) {
	assert(var_is_valid(vars));
	assert(gs_is_game_valid(game));

	if (vars != gs_get_vars(game))
		sc_fatal("var_register_game: game binding error\n");

	vars->game = game;
}


/*
 * var_set_ref_character()
 * var_set_ref_object()
 * var_set_ref_number()
 * var_set_ref_text()
 *
 * Set the "referenced" character, object, number, and text.
 */
void var_set_ref_character(sc_var_setref_t vars, sc_int character) {
	assert(var_is_valid(vars));
	vars->referenced_character = character;
}

void var_set_ref_object(sc_var_setref_t vars, sc_int object) {
	assert(var_is_valid(vars));
	vars->referenced_object = object;
}

void var_set_ref_number(sc_var_setref_t vars, sc_int number) {
	assert(var_is_valid(vars));
	vars->referenced_number = number;
	vars->is_number_referenced = TRUE;
}

void var_set_ref_text(sc_var_setref_t vars, const sc_char *text) {
	assert(var_is_valid(vars));

	/* Take a copy of the string, and retain it. */
	vars->referenced_text = (sc_char *)sc_realloc(vars->referenced_text, strlen(text) + 1);
	strcpy(vars->referenced_text, text);
}


/*
 * var_get_ref_character()
 * var_get_ref_object()
 * var_get_ref_number()
 * var_get_ref_text()
 *
 * Get the "referenced" character, object, number, and text.
 */
sc_int var_get_ref_character(sc_var_setref_t vars) {
	assert(var_is_valid(vars));
	return vars->referenced_character;
}

sc_int var_get_ref_object(sc_var_setref_t vars) {
	assert(var_is_valid(vars));
	return vars->referenced_object;
}

sc_int var_get_ref_number(sc_var_setref_t vars) {
	assert(var_is_valid(vars));
	return vars->referenced_number;
}

const sc_char *var_get_ref_text(sc_var_setref_t vars) {
	assert(var_is_valid(vars));

	/*
	 * If currently nullptr, return "".  A game may check restrictions involving
	 * referenced text before any value has been set; returning "" here for
	 * this case prevents problems later (strcmp (nullptr, ...), for example).
	 */
	return vars->referenced_text ? vars->referenced_text : "";
}


/*
 * var_get_elapsed_seconds()
 * var_set_elapsed_seconds()
 *
 * Get a count of seconds elapsed since the variables were created (start
 * of game), and set the count to a given value (game restore).
 */
sc_uint var_get_elapsed_seconds(sc_var_setref_t vars) {
	double delta;
	assert(var_is_valid(vars));

	delta = vars->timestamp - g_vm->_events->getTotalPlayTicks();
	return (sc_uint) delta + vars->time_offset;
}

void var_set_elapsed_seconds(sc_var_setref_t vars, sc_uint seconds) {
	assert(var_is_valid(vars));

	/*
	 * Reset the timestamp to now, and store seconds in offset.  This is sort-of
	 * forced by the fact that ANSI offers difftime but no 'settime' -- here,
	 * we'd really want to set the timestamp to now less seconds.
	 */
	vars->timestamp = g_vm->_events->getTotalPlayTicks() / 1000;
	vars->time_offset = seconds;
}


/*
 * var_debug_trace()
 *
 * Set variable tracing on/off.
 */
void var_debug_trace(sc_bool flag) {
	var_trace = flag;
}


/*
 * var_debug_dump()
 *
 * Print out a complete variables set.
 */
void var_debug_dump(sc_var_setref_t vars) {
	sc_int index_;
	sc_varref_t var;
	assert(var_is_valid(vars));

	/* Dump complete structure. */
	sc_trace("Variable: debug dump follows...\n");
	sc_trace("vars->bundle = %p\n", (void *) vars->bundle);
	sc_trace("vars->referenced_character = %ld\n", vars->referenced_character);
	sc_trace("vars->referenced_object = %ld\n", vars->referenced_object);
	sc_trace("vars->referenced_number = %ld\n", vars->referenced_number);
	sc_trace("vars->is_number_referenced = %s\n",
	         vars->is_number_referenced ? "true" : "false");

	sc_trace("vars->referenced_text = ");
	if (vars->referenced_text)
		sc_trace("\"%s\"\n", vars->referenced_text);
	else
		sc_trace("(nil)\n");

	sc_trace("vars->temporary = %p\n", (void *) vars->temporary);
	sc_trace("vars->timestamp = %lu\n", (sc_uint) vars->timestamp);
	sc_trace("vars->game = %p\n", (void *) vars->game);

	sc_trace("vars->variables =\n");
	for (index_ = 0; index_ < VAR_HASH_TABLE_SIZE; index_++) {
		for (var = vars->variable[index_]; var; var = var->next) {
			if (var == vars->variable[index_])
				sc_trace("%3ld : ", index_);
			else
				sc_trace("    : ");
			switch (var->type) {
			case VAR_STRING:
				sc_trace("[String ] %s = \"%s\"", var->name, var->value.string);
				break;
			case VAR_INTEGER:
				sc_trace("[Integer] %s = %ld", var->name, var->value.integer);
				break;

			default:
				sc_trace("[Invalid] %s = %p", var->name, var->value.voidp);
				break;
			}
			sc_trace("\n");
		}
	}
}

} // End of namespace Adrift
} // End of namespace Glk
