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

#undef longjmp
#undef setjmp
#include <setjmp.h>

namespace Glk {
namespace Adrift {

/* Assorted definitions and constants. */
enum { MAX_NESTING_DEPTH = 32 };
static const sc_char NUL = '\0';

/* Trace flag, set before running. */
static sc_bool restr_trace = FALSE;


/*
 * restr_integer_variable()
 *
 * Return the index of the n'th integer found.
 */
static sc_int restr_integer_variable(sc_gameref_t game, sc_int n) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int var_count, var, count;

	/* Get the count of variables. */
	vt_key[0].string = "Variables";
	var_count = prop_get_child_count(bundle, "I<-s", vt_key);

	/* Progress through variables until n integers found. */
	count = n;
	for (var = 0; var < var_count && count >= 0; var++) {
		sc_int type;

		vt_key[1].integer = var;
		vt_key[2].string = "Type";
		type = prop_get_integer(bundle, "I<-sis", vt_key);
		if (type == TAFVAR_NUMERIC)
			count--;
	}
	return var - 1;
}


/*
 * restr_object_in_place()
 *
 * Is object in a certain place, state, or condition.
 */
static sc_bool restr_object_in_place(sc_gameref_t game, sc_int object, sc_int var2, sc_int var3) {
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int npc;

	if (restr_trace) {
		sc_trace("Restr: checking"
		         " object in place, %ld, %ld, %ld\n", object, var2, var3);
	}

	/* Var2 controls what we do. */
	switch (var2) {
	case 0:
	case 6:                    /* In room */
		if (var3 == 0)
			return gs_object_position(game, object) == OBJ_HIDDEN;
		else
			return gs_object_position(game, object) == var3;

	case 1:
	case 7:                    /* Held by */
		if (var3 == 0)            /* Player */
			return gs_object_position(game, object) == OBJ_HELD_PLAYER;
		else if (var3 == 1)       /* Ref character */
			npc = var_get_ref_character(vars);
		else
			npc = var3 - 2;

		return gs_object_position(game, object) == OBJ_HELD_NPC
		       && gs_object_parent(game, object) == npc;

	case 2:
	case 8:                    /* Worn by */
		if (var3 == 0)            /* Player */
			return gs_object_position(game, object) == OBJ_WORN_PLAYER;
		else if (var3 == 1)       /* Ref character */
			npc = var_get_ref_character(vars);
		else
			npc = var3 - 2;

		return gs_object_position(game, object) == OBJ_WORN_NPC
		       && gs_object_parent(game, object) == npc;

	case 3:
	case 9:                    /* Visible to */
		if (var3 == 0)            /* Player */
			return obj_indirectly_in_room(game,
			                              object, gs_playerroom(game));
		else if (var3 == 1)       /* Ref character */
			npc = var_get_ref_character(vars);
		else
			npc = var3 - 2;

		return obj_indirectly_in_room(game, object,
		                              gs_npc_location(game, npc) - 1);

	case 4:
	case 10:                   /* Inside */
		if (var3 == 0)            /* Nothing? */
			return gs_object_position(game, object) != OBJ_IN_OBJECT;

		return gs_object_position(game, object) == OBJ_IN_OBJECT
		       && gs_object_parent(game, object) == obj_container_object(game,
		               var3 - 1);

	case 5:
	case 11:                   /* On top of */
		if (var3 == 0)            /* Nothing? */
			return gs_object_position(game, object) != OBJ_ON_OBJECT;

		return gs_object_position(game, object) == OBJ_ON_OBJECT
		       && gs_object_parent(game, object) == obj_surface_object(game,
		               var3 - 1);

	default:
		sc_fatal("restr_object_in_place: bad var2, %ld\n", var2);
		return FALSE;
	}
}


/*
 * restr_pass_task_object_location()
 *
 * Evaluate restrictions relating to object location.
 */
static sc_bool restr_pass_task_object_location(sc_gameref_t game,
		sc_int var1, sc_int var2, sc_int var3) {
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_bool should_be;
	sc_int object;

	if (restr_trace) {
		sc_trace("Restr: running object"
		         " location restriction, %ld, %ld, %ld\n", var1, var2, var3);
	}

	/* Initialize variables to avoid gcc warnings. */
	should_be = FALSE;
	object = -1;

	/* See how things should look. */
	if (var2 >= 0 && var2 < 6)
		should_be = TRUE;
	else if (var2 >= 6 && var2 < 12)
		should_be = FALSE;
	else
		sc_fatal("restr_pass_task_object_location: bad var2, %ld\n", var2);

	/* Now find the addressed object. */
	if (var1 == 0) {
		object = -1;              /* No object */
		should_be = !should_be;
	} else if (var1 == 1)
		object = -1;                /* Any object */
	else if (var1 == 2)
		object = var_get_ref_object(vars);
	else if (var1 >= 3)
		object = obj_dynamic_object(game, var1 - 3);
	else
		sc_fatal("restr_pass_task_object_location: bad var1, %ld\n", var1);

	/*
	 * Here it seems that we have to special case static objects that may have
	 * crept in through the referenced object.  The object in place function
	 * isn't built to handle these.
	 *
	 * TODO What is the meaning of applying object restrictions to static
	 * objects?
	 */
	if (var1 == 2 && object != -1 && obj_is_static(game, object)) {
		if (restr_trace) {
			sc_trace("Restr:"
			         " restriction object %ld is static, rejecting\n", object);
		}

		return FALSE;
	}

	/* Try to put it all together. */
	if (object == -1) {
		sc_int target;

		for (target = 0; target < gs_object_count(game); target++) {
			if (restr_object_in_place(game, target, var2, var3))
				return should_be;
		}
		return !should_be;
	}
	return should_be == restr_object_in_place(game, object, var2, var3);
}


/*
 * restr_pass_task_object_state()
 *
 * Evaluate restrictions relating to object states.  This function is called
 * from the library by lib_pass_alt_room(), so cannot be static.
 */
sc_bool restr_pass_task_object_state(sc_gameref_t game, sc_int var1, sc_int var2) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_vartype_t vt_key[3];
	sc_int object, openable, key;

	if (restr_trace) {
		sc_trace("Restr:"
		         " running object state restriction, %ld, %ld\n", var1, var2);
	}

	/* Find the object being addressed. */
	if (var1 == 0)
		object = var_get_ref_object(vars);
	else
		object = obj_stateful_object(game, var1 - 1);

	/* We're interested only in openable objects. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Openable";
	openable = prop_get_integer(bundle, "I<-sis", vt_key);
	if (openable > 0) {
		/* Is this object lockable? */
		vt_key[2].string = "Key";
		key = prop_get_integer(bundle, "I<-sis", vt_key);
		if (key >= 0) {
			if (var2 <= 2)
				return gs_object_openness(game, object) == var2 + 5;
			else
				return gs_object_state(game, object) == var2 - 2;
		} else {
			if (var2 <= 1)
				return gs_object_openness(game, object) == var2 + 5;
			else
				return gs_object_state(game, object) == var2 - 1;
		}
	} else
		return gs_object_state(game, object) == var2 + 1;
}


/*
 * restr_pass_task_task_state()
 *
 * Evaluate restrictions relating to task states.
 */
static sc_bool restr_pass_task_task_state(sc_gameref_t game, sc_int var1, sc_int var2) {
	sc_bool should_be;

	if (restr_trace)
		sc_trace("Restr: running task restriction, %ld, %ld\n", var1, var2);

	/* Initialize variables to avoid gcc warnings. */
	should_be = FALSE;

	/* See if the task should be done or not done. */
	if (var2 == 0)
		should_be = TRUE;
	else if (var2 == 1)
		should_be = FALSE;
	else
		sc_fatal("restr_pass_task_task_state: bad var2, %ld\n", var2);

	/* Check all tasks? */
	if (var1 == 0) {
		sc_int task;

		for (task = 0; task < gs_task_count(game); task++) {
			if (gs_task_done(game, task) == should_be)
				return FALSE;
		}
		return TRUE;
	}

	/* Check just the given task. */
	return gs_task_done(game, var1 - 1) == should_be;
}


/*
 * restr_pass_task_char()
 *
 * Evaluate restrictions relating to player and NPCs.
 */
static sc_bool restr_pass_task_char(sc_gameref_t game, sc_int var1, sc_int var2, sc_int var3) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int npc1, npc2;

	if (restr_trace) {
		sc_trace("Restr:"
		         " running char restriction, %ld, %ld, %ld\n", var1, var2, var3);
	}

	/* Handle var2 types 1 and 2. */
	if (var2 == 1)                /* Not in same room as */
		return !restr_pass_task_char(game, var1, 0, var3);
	else if (var2 == 2)           /* Alone */
		return !restr_pass_task_char(game, var1, 3, var3);

	/* Decode NPC number, -1 if none. */
	npc1 = npc2 = -1;
	if (var1 == 1)
		npc1 = var_get_ref_character(vars);
	else if (var1 > 1)
		npc1 = var1 - 2;

	/* Player or NPC? */
	if (var1 == 0) {
		sc_vartype_t vt_key[2];
		sc_int gender;

		/* Player -- decode based on var2. */
		switch (var2) {
		case 0:                /* In same room as */
			if (var3 == 1)
				npc2 = var_get_ref_character(vars);
			else if (var3 > 1)
				npc2 = var3 - 2;
			if (var3 == 0)       /* Player */
				return TRUE;
			else
				return npc_in_room(game, npc2, gs_playerroom(game));

		case 3:                /* Not alone */
			return npc_count_in_room(game, gs_playerroom(game)) > 1;

		case 4:                /* Standing on */
			return gs_playerposition(game) == 0
			       && gs_playerparent(game) == obj_standable_object(game,
			               var3 - 1);

		case 5:                /* Sitting on */
			return gs_playerposition(game) == 1
			       && gs_playerparent(game) == obj_standable_object(game,
			               var3 - 1);

		case 6:                /* Lying on */
			return gs_playerposition(game) == 2
			       && gs_playerparent(game) == obj_lieable_object(game,
			               var3 - 1);

		case 7:                /* Player gender */
			vt_key[0].string = "Globals";
			vt_key[1].string = "PlayerGender";
			gender = prop_get_integer(bundle, "I<-ss", vt_key);
			return gender == var3;

		default:
			sc_fatal("restr_pass_task_char: invalid type, %ld\n", var2);
			return FALSE;
		}
	} else {
		sc_vartype_t vt_key[3];
		sc_int gender;

		/* NPC -- decode based on var2. */
		switch (var2) {
		case 0:                /* In same room as */
			if (var3 == 0)
				return npc_in_room(game, npc1, gs_playerroom(game));
			if (var3 == 1)
				npc2 = var_get_ref_character(vars);
			else if (var3 > 1)
				npc2 = var3 - 2;
			return npc_in_room(game, npc1, gs_npc_location(game, npc2) - 1);

		case 3:                /* Not alone */
			return npc_count_in_room(game, gs_npc_location(game, npc1) - 1) > 1;

		case 4:                /* Standing on */
			return gs_npc_position(game, npc1) == 0
			       && gs_playerparent(game) == obj_standable_object(game, var3);

		case 5:                /* Sitting on */
			return gs_npc_position(game, npc1) == 1
			       && gs_playerparent(game) == obj_standable_object(game, var3);

		case 6:                /* Lying on */
			return gs_npc_position(game, npc1) == 2
			       && gs_playerparent(game) == obj_lieable_object(game, var3);

		case 7:                /* NPC gender */
			vt_key[0].string = "NPCs";
			vt_key[1].integer = npc1;
			vt_key[2].string = "Gender";
			gender = prop_get_integer(bundle, "I<-sis", vt_key);
			return gender == var3;

		default:
			sc_fatal("restr_pass_task_char: invalid type, %ld\n", var2);
			return FALSE;
		}
	}
}


/*
 * restr_pass_task_int_var()
 *
 * Helper for restr_pass_task_var(), handles integer variable restrictions.
 */
static sc_bool restr_pass_task_int_var(sc_gameref_t game, sc_int var2, sc_int var3, sc_int value) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_vartype_t vt_key[3];
	sc_int value2;

	if (restr_trace) {
		sc_trace("Restr: running"
		         " integer var restriction, %ld, %ld, %ld\n", var2, var3, value);
	}

	/* Compare against var3 if that's what var2 says. */
	switch (var2) {
	case 0:
		return value < var3;
	case 1:
		return value <= var3;
	case 2:
		return value == var3;
	case 3:
		return value >= var3;
	case 4:
		return value > var3;
	case 5:
		return value != var3;

	default:
		/*
		 * Compare against the integer var numbered in var3 - 1, or the
		 * referenced number if var3 is zero.  Make sure that we're comparing
		 * integer variables.
		 */
		if (var3 == 0)
			value2 = var_get_ref_number(vars);
		else {
			const sc_char *name;
			sc_int ivar, type;

			ivar = restr_integer_variable(game, var3 - 1);
			vt_key[0].string = "Variables";
			vt_key[1].integer = ivar;
			vt_key[2].string = "Name";
			name = prop_get_string(bundle, "S<-sis", vt_key);
			vt_key[2].string = "Type";
			type = prop_get_integer(bundle, "I<-sis", vt_key);

			if (type != TAFVAR_NUMERIC) {
				sc_fatal("restr_pass_task_int_var:"
				         " non-integer in comparison, %s\n", name);
			}

			/* Get the value in variable numbered in var3 - 1. */
			value2 = var_get_integer(vars, name);
		}

		switch (var2) {
		case 10:
			return value < value2;
		case 11:
			return value <= value2;
		case 12:
			return value == value2;
		case 13:
			return value >= value2;
		case 14:
			return value > value2;
		case 15:
			return value != value2;

		default:
			sc_fatal("restr_pass_task_int_var:"
			         " unknown int comparison, %ld\n", var2);
			return FALSE;
		}
	}
}


/*
 * restr_pass_task_string_var()
 *
 * Helper for restr_pass_task_var(), handles string variable restrictions.
 */
static sc_bool restr_pass_task_string_var(sc_int var2, const sc_char *var4, const sc_char *value) {
	if (restr_trace) {
		sc_trace("Restr: running string"
		         " var restriction, %ld, \"%s\", \"%s\"\n", var2, var4, value);
	}

	/* Make comparison against var4 based on var2 value. */
	switch (var2) {
	case 0:
		return strcmp(value, var4) == 0;   /* == */
	case 1:
		return strcmp(value, var4) != 0;   /* != */

	default:
		sc_fatal("restr_pass_task_string_var:"
		         " unknown string comparison, %ld\n", var2);
		return FALSE;
	}
}


/*
 * restr_pass_task_var()
 *
 * Evaluate restrictions relating to variables.
 */
static sc_bool restr_pass_task_var(sc_gameref_t game, sc_int var1, sc_int var2, sc_int var3,
		const sc_char *var4) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_vartype_t vt_key[3];
	sc_int type, value;
	const sc_char *name, *string;

	if (restr_trace) {
		sc_trace("Restr: running var restriction,"
		         " %ld, %ld, %ld, \"%s\"\n", var1, var2, var3, var4);
	}

	/*
	 * For var1=0, compare against referenced number.  For var1=1, compare
	 * against referenced text.
	 */
	if (var1 == 0) {
		value = var_get_ref_number(vars);
		return restr_pass_task_int_var(game, var2, var3, value);
	} else if (var1 == 1) {
		string = var_get_ref_text(vars);
		return restr_pass_task_string_var(var2, var4, string);
	}

	/* Get the name and type of the variable being addressed. */
	vt_key[0].string = "Variables";
	vt_key[1].integer = var1 - 2;
	vt_key[2].string = "Name";
	name = prop_get_string(bundle, "S<-sis", vt_key);
	vt_key[2].string = "Type";
	type = prop_get_integer(bundle, "I<-sis", vt_key);

	/* Select first based on variable type. */
	switch (type) {
	case TAFVAR_NUMERIC:
		value = var_get_integer(vars, name);
		return restr_pass_task_int_var(game, var2, var3, value);

	case TAFVAR_STRING:
		string = var_get_string(vars, name);
		return restr_pass_task_string_var(var2, var4, string);

	default:
		sc_fatal("restr_pass_task_var: invalid variable type, %ld\n", type);
		return FALSE;
	}
}


/*
 * restr_pass_task_restriction()
 *
 * Demultiplexer for task restrictions.
 */
static sc_bool restr_pass_task_restriction(sc_gameref_t game, sc_int task, sc_int restriction) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int type, var1, var2, var3;
	const sc_char *var4;
	sc_bool result = FALSE;

	if (restr_trace) {
		sc_trace("Restr:"
		         " evaluating task %ld restriction %ld\n", task, restriction);
	}

	/* Get the task restriction type. */
	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = "Restrictions";
	vt_key[3].integer = restriction;
	vt_key[4].string = "Type";
	type = prop_get_integer(bundle, "I<-sisis", vt_key);

	/* Demultiplex depending on type. */
	switch (type) {
	case 0:                    /* Object location. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var3";
		var3 = prop_get_integer(bundle, "I<-sisis", vt_key);
		result = restr_pass_task_object_location(game, var1, var2, var3);
		break;

	case 1:                    /* Object state. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		result = restr_pass_task_object_state(game, var1, var2);
		break;

	case 2:                    /* Task state. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		result = restr_pass_task_task_state(game, var1, var2);
		break;

	case 3:                    /* Player and NPCs. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var3";
		var3 = prop_get_integer(bundle, "I<-sisis", vt_key);
		result = restr_pass_task_char(game, var1, var2, var3);
		break;

	case 4:                    /* Variable. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var3";
		var3 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var4";
		var4 = prop_get_string(bundle, "S<-sisis", vt_key);
		result = restr_pass_task_var(game, var1, var2, var3, var4);
		break;

	default:
		sc_fatal("restr_pass_task_restriction:"
		         " unknown restriction type %ld\n", type);
	}

	if (restr_trace) {
		sc_trace("Restr: task %ld restriction"
		         " %ld is %s\n", task, restriction, result ? "PASS" : "FAIL");
	}

	return result;
}


/* Enumeration of restrictions combination string tokens. */
enum {
	TOK_RESTRICTION = '#',
	TOK_AND = 'A',
	TOK_OR = 'O',
	TOK_LPAREN = '(',
	TOK_RPAREN = ')',
	TOK_EOS = '\0'
};

/* #O#A(#O#)-style expression, for tokenizing. */
static const sc_char *restr_expression = NULL;
static sc_int restr_index = 0;

/*
 * restr_tokenize_start()
 * restr_tokenize_end()
 *
 * Start and wrap up restrictions combinations string tokenization.
 */
static void restr_tokenize_start(const sc_char *expression) {
	/* Save expression, and restart index. */
	restr_expression = expression;
	restr_index = 0;
}

static void restr_tokenize_end(void) {
	restr_expression = NULL;
	restr_index = 0;
}


/*
 * restr_next_token()
 *
 * Simple tokenizer for restrictions combination expressions.
 */
static sc_char restr_next_token(void) {
	assert(restr_expression);

	/* Find the next non-space, and return it. */
	while (TRUE) {
		/* Return NUL if at string end. */
		if (restr_expression[restr_index] == NUL)
			return restr_expression[restr_index];

		/* Spin on whitespace. */
		restr_index++;
		if (sc_isspace(restr_expression[restr_index - 1]))
			continue;

		/* Return the character just passed. */
		return restr_expression[restr_index - 1];
	}
}


/* Evaluation values stack. */
static sc_bool restr_eval_values[MAX_NESTING_DEPTH];
static sc_int restr_eval_stack = 0;

/*
 * The restriction number to evaluate.  This advances with each call to
 * evaluate and stack a restriction result.
 */
static sc_int restr_eval_restriction = 0;

/* The current game used to evaluate restrictions, and the task in question. */
static sc_gameref_t restr_eval_game = NULL;
static sc_int restr_eval_task = 0;

/* The id of the lowest-indexed failing restriction. */
static sc_int restr_lowest_fail = -1;

/*
 * restr_eval_start()
 *
 * Reset the evaluation stack to an empty state, and note the things we have
 * to note for when we need to evaluate a restriction.
 */
static void restr_eval_start(sc_gameref_t game, sc_int task) {
	/* Clear stack. */
	restr_eval_stack = 0;
	restr_eval_restriction = 0;

	/* Note evaluation details. */
	restr_eval_game = game;
	restr_eval_task = task;

	/* Clear lowest indexed failing restriction. */
	restr_lowest_fail = -1;
}


/*
 * restr_eval_push()
 *
 * Push a value onto the values stack.
 */
static void restr_eval_push(sc_bool value) {
	if (restr_eval_stack >= MAX_NESTING_DEPTH)
		sc_fatal("restr_eval_push: stack overflow\n");

	restr_eval_values[restr_eval_stack++] = value;
}


/*
 * expr_restr_action()
 *
 * Evaluate the effect of an and/or into the values stack.
 */
static void restr_eval_action(sc_char token) {
	/* Select action based on parsed token. */
	switch (token) {
	/* Handle evaluating and pushing a restriction result. */
	case TOK_RESTRICTION: {
		sc_bool result;

		/* Evaluate and push the next restriction. */
		result = restr_pass_task_restriction(restr_eval_game,
		                                     restr_eval_task,
		                                     restr_eval_restriction);
		restr_eval_push(result);

		/*
		 * If the restriction failed, and there isn't yet a first failing one
		 * set, note this one as the first to fail.
		 */
		if (restr_lowest_fail == -1 && !result)
			restr_lowest_fail = restr_eval_restriction;

		/* Increment restriction sequence identifier. */
		restr_eval_restriction++;
		break;
	}

	/* Handle cases of or-ing/and-ing restrictions. */
	case TOK_OR:
	case TOK_AND: {
		sc_bool val1, val2, result = FALSE;
		assert(restr_eval_stack >= 2);

		/* Get the top two stack values. */
		val1 = restr_eval_values[restr_eval_stack - 2];
		val2 = restr_eval_values[restr_eval_stack - 1];

		/* Or, or and, into result. */
		switch (token) {
		case TOK_OR:
			result = val1 || val2;
			break;
		case TOK_AND:
			result = val1 && val2;
			break;

		default:
			sc_fatal("restr_eval_action: bad token, '%c'\n", token);
		}

		/* Put result back at top of stack. */
		restr_eval_stack--;
		restr_eval_values[restr_eval_stack - 1] = result;
		break;
	}

	default:
		sc_fatal("restr_eval_action: bad token, '%c'\n", token);
	}
}


/*
 * restr_eval_result()
 *
 * Return the top of the values stack as the evaluation result.
 */
static sc_int restr_eval_result(sc_int *lowest_fail) {
	if (restr_eval_stack != 1)
		sc_fatal("restr_eval_result: values stack not completed\n");

	*lowest_fail = restr_lowest_fail;
	return restr_eval_values[0];
}


/* Single lookahead token for parser. */
static sc_char restr_lookahead = '\0';

/*
 * restr_match()
 *
 * Match a token with an expectation.
 */
static void restr_match(CONTEXT, sc_char c) {
	if (restr_lookahead == c)
		restr_lookahead = restr_next_token();
	else {
		sc_error("restr_match: syntax error, expected %d, got %d\n", c, restr_lookahead);
		LONG_JUMP;
	}
}


/* Forward declaration for recursion. */
static void restr_bexpr(CONTEXT);

/*
 * restr_andexpr()
 * restr_orexpr()
 * restr_bexpr()
 *
 * Expression parsers.  Here we go again...
 */
static void restr_andexpr(CONTEXT) {
	CALL0(restr_bexpr);
	while (restr_lookahead == TOK_AND) {
		CALL1(restr_match, TOK_AND);
		CALL0(restr_bexpr);
		restr_eval_action(TOK_AND);
	}
}

static void restr_orexpr(CONTEXT) {
	CALL0(restr_andexpr);
	while (restr_lookahead == TOK_OR) {
		CALL1(restr_match, TOK_OR);
		CALL0(restr_andexpr);
		restr_eval_action(TOK_OR);
	}
}

static void restr_bexpr(CONTEXT) {
	switch (restr_lookahead) {
	case TOK_RESTRICTION:
		CALL1(restr_match, TOK_RESTRICTION);
		restr_eval_action(TOK_RESTRICTION);
		break;

	case TOK_LPAREN:
		CALL1(restr_match, TOK_LPAREN);
		CALL0(restr_orexpr);
		CALL1(restr_match, TOK_RPAREN);
		break;

	default:
		sc_error("restr_bexpr: syntax error, unexpected %d\n", restr_lookahead);
		LONG_JUMP;
	}
}


/*
 * restr_get_fail_message()
 *
 * Get the FailMessage for the given task restriction; NULL if none.
 */
static const sc_char *restr_get_fail_message(sc_gameref_t game, sc_int task, sc_int restriction) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	const sc_char *message;

	/* Get the restriction message. */
	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = "Restrictions";
	vt_key[3].integer = restriction;
	vt_key[4].string = "FailMessage";
	message = prop_get_string(bundle, "S<-sisis", vt_key);

	/* Return it, or NULL if empty. */
	return !sc_strempty(message) ? message : NULL;
}


/*
 * restr_debug_trace()
 *
 * Set restrictions tracing on/off.
 */
void restr_debug_trace(sc_bool flag) {
	restr_trace = flag;
}


/*
 * restr_eval_task_restrictions()
 *
 * Main handler for a given set of task restrictions.  Returns TRUE in pass
 * if the restrictions pass, FALSE if not.  On FALSE pass returns, it also
 * returns a fail message string from the restriction deemed to have caused
 * the failure (that is, the first one with a FailMessage property), or NULL
 * if no failing restriction has a FailMessage.  The function's main return
 * value is TRUE if restrictions parsed successfully, FALSE otherwise.
 */
sc_bool restr_eval_task_restrictions(sc_gameref_t game, sc_int task, sc_bool *pass,
		const sc_char **fail_message) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int restr_count, lowest_fail;
	const sc_char *pattern;
	sc_bool result;
	Context context;
	assert(pass && fail_message);

	/* Get the count of restrictions on the task. */
	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = "Restrictions";
	restr_count = prop_get_child_count(bundle, "I<-sis", vt_key);

	/* If none, stop now, acting as if all passed. */
	if (restr_count == 0) {
		if (restr_trace)
			sc_trace("Restr: task %ld has no restrictions\n", task);

		*pass = TRUE;
		*fail_message = NULL;
		return TRUE;
	}

	/* Get the task's restriction combination pattern. */
	vt_key[2].string = "RestrMask";
	pattern = prop_get_string(bundle, "S<-sis", vt_key);

	if (restr_trace) {
		sc_trace("Restr: task %ld has %ld restrictions, %s\n", task, restr_count, pattern);
	}

	/* Set up the evaluation stack and tokenizer. */
	restr_eval_start(game, task);
	restr_tokenize_start(pattern);

	// Parse the pattern, and ensure it ends at string end
	restr_lookahead = restr_next_token();
	restr_orexpr(context);
	if (!context._break)
		restr_match(context, TOK_EOS);

	if (context._break) {
		// Parse error -- clean up tokenizer and return fail
		restr_tokenize_end();
		return FALSE;
	}

	/* Clean up tokenizer and get the evaluation result. */
	restr_tokenize_end();
	result = restr_eval_result(&lowest_fail);

	if (restr_trace) {
		sc_trace("Restr: task %ld restrictions %s\n", task, result ? "PASS" : "FAIL");
	}

	/*
	 * Return the result, and if a restriction fails, then return the
	 * FailMessage of the lowest indexed failing restriction (or NULL if this
	 * restriction has no FailMessage).
	 *
	 * Then return TRUE since parsing and running the restrictions succeeded
	 * (even if the restrictions themselves didn't).
	 */
	*pass = result;
	if (result)
		*fail_message = NULL;
	else
		*fail_message = restr_get_fail_message(game, task, lowest_fail);
	return TRUE;
}

} // End of namespace Adrift
} // End of namespace Glk
