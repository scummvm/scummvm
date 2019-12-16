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

namespace Glk {
namespace Adrift {

/* Assorted definitions and constants. */
static const sc_uint DEBUG_MAGIC = 0xc4584d2e;
enum { DEBUG_BUFFER_SIZE = 256 };

/* Debugging command and command argument type. */
enum sc_command_t {
	DEBUG_NONE = 0, DEBUG_CONTINUE, DEBUG_STEP, DEBUG_BUFFER, DEBUG_RESOURCES,
	DEBUG_HELP, DEBUG_GAME,
	DEBUG_PLAYER, DEBUG_ROOMS, DEBUG_OBJECTS, DEBUG_NPCS, DEBUG_EVENTS,
	DEBUG_TASKS, DEBUG_VARIABLES,
	DEBUG_OLDPLAYER, DEBUG_OLDROOMS, DEBUG_OLDOBJECTS, DEBUG_OLDNPCS,
	DEBUG_OLDEVENTS, DEBUG_OLDTASKS, DEBUG_OLDVARIABLES,
	DEBUG_WATCHPLAYER, DEBUG_WATCHOBJECTS, DEBUG_WATCHNPCS, DEBUG_WATCHEVENTS,
	DEBUG_WATCHTASKS, DEBUG_WATCHVARIABLES,
	DEBUG_CLEARPLAYER, DEBUG_CLEAROBJECTS, DEBUG_CLEARNPCS, DEBUG_CLEAREVENTS,
	DEBUG_CLEARTASKS, DEBUG_CLEARVARIABLES,
	DEBUG_WATCHALL, DEBUG_CLEARALL, DEBUG_RANDOM,
	DEBUG_QUIT
};

enum sc_command_type_t { COMMAND_QUERY = 0, COMMAND_RANGE, COMMAND_ONE, COMMAND_ALL };

/* Table connecting debugging command strings to commands. */
struct sc_strings_t {
	const sc_char *const command_string;
	const sc_command_t command;
};
static const sc_strings_t DEBUG_COMMANDS[] = {
	{"continue", DEBUG_CONTINUE}, {"step", DEBUG_STEP}, {"buffer", DEBUG_BUFFER},
	{"resources", DEBUG_RESOURCES}, {"help", DEBUG_HELP}, {"game", DEBUG_GAME},
	{"player", DEBUG_PLAYER}, {"rooms", DEBUG_ROOMS}, {"objects", DEBUG_OBJECTS},
	{"npcs", DEBUG_NPCS}, {"events", DEBUG_EVENTS}, {"tasks", DEBUG_TASKS},
	{"variables", DEBUG_VARIABLES},
	{"oldplayer", DEBUG_OLDPLAYER}, {"oldrooms", DEBUG_OLDROOMS},
	{"oldobjects", DEBUG_OLDOBJECTS}, {"oldnpcs", DEBUG_OLDNPCS},
	{"oldevents", DEBUG_OLDEVENTS}, {"oldtasks", DEBUG_OLDTASKS},
	{"oldvariables", DEBUG_OLDVARIABLES},
	{"watchplayer", DEBUG_WATCHPLAYER}, {"clearplayer", DEBUG_CLEARPLAYER},
	{"watchobjects", DEBUG_WATCHOBJECTS}, {"watchnpcs", DEBUG_WATCHNPCS},
	{"watchevents", DEBUG_WATCHEVENTS}, {"watchtasks", DEBUG_WATCHTASKS},
	{"watchvariables", DEBUG_WATCHVARIABLES},
	{"clearobjects", DEBUG_CLEAROBJECTS}, {"clearnpcs", DEBUG_CLEARNPCS},
	{"clearevents", DEBUG_CLEAREVENTS}, {"cleartasks", DEBUG_CLEARTASKS},
	{"clearvariables", DEBUG_CLEARVARIABLES}, {"watchall", DEBUG_WATCHALL},
	{"clearall", DEBUG_CLEARALL}, {"random", DEBUG_RANDOM}, {"quit", DEBUG_QUIT},
	{NULL, DEBUG_NONE}
};

/*
 * Debugging control information structure.  The structure is created and
 * added to the game on enabling debug, and removed and destroyed on
 * disabling debugging.
 */
struct sc_debugger_s {
	sc_uint magic;
	sc_bool *watch_objects;
	sc_bool *watch_npcs;
	sc_bool *watch_events;
	sc_bool *watch_tasks;
	sc_bool *watch_variables;
	sc_bool watch_player;
	sc_bool single_step;
	sc_bool quit_pending;
	sc_uint elapsed_seconds;
};
typedef sc_debugger_s sc_debugger_t;


/*
 * debug_is_valid()
 *
 * Return TRUE if pointer is a valid debugger, FALSE otherwise.
 */
static sc_bool debug_is_valid(sc_debuggerref_t debug) {
	return debug && debug->magic == DEBUG_MAGIC;
}


/*
 * debug_get_debugger()
 *
 * Return the debugger reference from a game, or NULL if none.
 */
static sc_debuggerref_t debug_get_debugger(sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	return game->debugger;
}


/*
 * debug_variable_count()
 *
 * Common helper to return the count of variables defined in a game.
 */
static sc_int debug_variable_count(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key;
	sc_int variable_count;

	/* Find and return the variables count. */
	vt_key.string = "Variables";
	variable_count = prop_get_child_count(bundle, "I<-s", &vt_key);

	return variable_count;
}


/*
 * debug_initialize()
 *
 * Create a new set of debug control information, and append it to the
 * game passed in.
 */
static void debug_initialize(sc_gameref_t game) {
	sc_debuggerref_t debug;

	/* Create the easy bits of the new debugging set. */
	debug = (sc_debuggerref_t)sc_malloc(sizeof(*debug));
	debug->magic = DEBUG_MAGIC;
	debug->watch_player = FALSE;
	debug->single_step = FALSE;
	debug->quit_pending = FALSE;
	debug->elapsed_seconds = 0;

	/* Allocate watchpoints for everything we can watch. */
	debug->watch_objects = (sc_bool *)sc_malloc(gs_object_count(game)
	                       * sizeof(*debug->watch_objects));
	debug->watch_npcs = (sc_bool *)sc_malloc(gs_npc_count(game)
	                    * sizeof(*debug->watch_npcs));
	debug->watch_events = (sc_bool *)sc_malloc(gs_event_count(game)
	                      * sizeof(*debug->watch_events));
	debug->watch_tasks = (sc_bool *)sc_malloc(gs_task_count(game)
	                     * sizeof(*debug->watch_tasks));
	debug->watch_variables = (sc_bool *)sc_malloc(debug_variable_count(game)
	                         * sizeof(*debug->watch_variables));

	/* Clear all watchpoint arrays. */
	memset(debug->watch_objects, FALSE,
	       gs_object_count(game) * sizeof(*debug->watch_objects));
	memset(debug->watch_npcs, FALSE,
	       gs_npc_count(game) * sizeof(*debug->watch_npcs));
	memset(debug->watch_events, FALSE,
	       gs_event_count(game) * sizeof(*debug->watch_events));
	memset(debug->watch_tasks, FALSE,
	       gs_task_count(game) * sizeof(*debug->watch_tasks));
	memset(debug->watch_variables, FALSE,
	       debug_variable_count(game) * sizeof(*debug->watch_variables));

	/* Append the new debugger set to the game. */
	assert(!game->debugger);
	game->debugger = debug;
}


/*
 * debug_finalize()
 *
 * Destroy a debug data set, free its heap memory, and remove its reference
 * from the game.
 */
static void debug_finalize(sc_gameref_t game) {
	sc_debuggerref_t debug = debug_get_debugger(game);
	assert(debug_is_valid(debug));

	/* Free all allocated watchpoint arrays. */
	sc_free(debug->watch_objects);
	sc_free(debug->watch_npcs);
	sc_free(debug->watch_events);
	sc_free(debug->watch_tasks);
	sc_free(debug->watch_variables);

	/* Poison and free the debugger itself. */
	memset(debug, 0xaa, sizeof(*debug));
	sc_free(debug);

	/* Remove the debug reference from the game. */
	game->debugger = NULL;
}


/*
 * debug_help()
 *
 * Print debugging help.
 */
static void debug_help(sc_command_t topic) {
	/* Is help general, or specific? */
	if (topic == DEBUG_NONE) {
		if_print_debug(
		    "The following debugging commands examine game state:\n\n");
		if_print_debug(
		    " game               -- Print general game information,"
		    " and class counts\n"
		    " player             -- Show the player location and position\n"
		    " rooms [Range]      -- Print information on game rooms\n"
		    " objects [Range]    -- Print information on objects in the game\n"
		    " npcs [Range]       -- Print information on game NPCs\n"
		    " events [Range]     -- Print information on the game's events\n"
		    " tasks [Range]      -- Print information on the game's tasks\n"
		    " variables [Range]  -- Show variables defined by the game\n\n");
		if_print_debug(
		    "Most commands take range inputs.  This can be a single number, to"
		    " apply the command to just that item, a range such as '0 to 10' (or"
		    " '0 - 10', '0 .. 10', or simply '0 10') to apply to that range of"
		    " items, or '*' to apply the command to all items of the class.  If"
		    " omitted, the command is applied only to the items of the class"
		    " 'relevant' to the current game state; see the help for specific"
		    " commands for more on what is 'relevant'.\n\n");
		if_print_debug(
		    "The 'player', 'objects', 'npcs', 'events', 'tasks', and 'variables'"
		    " commands may be prefixed with 'old', in which case the values"
		    " printed will be those for the previous game turn, rather than the"
		    " current values.\n\n");
		if_print_debug(
		    "These debugging commands manage watchpoints:\n\n");
		if_print_debug(
		    "The 'player', 'objects', 'npcs', 'events', 'tasks', and 'variables'"
		    " commands may be prefixed with 'watch', to set watchpoints."
		    "  Watchpoints automatically enter the debugger when the item changes"
		    " state during a game turn.  For example 'watchobject 10' monitors"
		    " object 10 for changes, and 'watchnpc *' monitors all NPCs.  A"
		    " 'watch' command with no range prints out all watchpoints set for"
		    " that class.\n\n");
		if_print_debug(
		    "Prefix commands with 'clear' to clear watchpoints, for example"
		    " 'clearnpcs *'.  Use 'watchall' to obtain a complete list of every"
		    " watchpoint set, and 'clearall' to clear all watchpoints in one go."
		    "  A 'clear' command with no range behaves the same as a 'watch'"
		    " command with no range.\n\n");
		if_print_debug(
		    "These debugging commands print details of game output and control the"
		    " debugger and interpreter:\n\n");
		if_print_debug(
		    " buffer          -- Show the current buffered game text\n"
		    " resources       -- Show current and requested game resources\n"
		    " random [Seed]   -- Control the random number generator\n"
		    " step            -- Run one game turn, then re-enter the debugger\n"
		    " continue        -- Leave the debugger and resume the game\n"
		    " quit            -- Exit the interpreter main loop\n"
		    " help [Command]  -- Print help specific to Command\n\n");
		if_print_debug(
		    "Debugging commands may be abbreviated to their shortest unambiguous"
		    " form.\n\n");
		if_print_debug(
		    "Use the 'debug' or '#debug' command in a game, typed at the usual"
		    " game prompt, to return to the debugger.\n");
		return;
	}

	/* Command-specific help. */
	switch (topic) {
	case DEBUG_HELP:
		if_print_debug(
		    "Give the name of the command you want help on, for example 'help"
		    " continue'.\n");
		break;

	case DEBUG_CONTINUE:
		if_print_debug(
		    "Leave the debugger and resume the game.  Use the 'debug' or '#debug'"
		    " command in a game, typed at the usual game prompt, to return to the"
		    " debugger.\n");
		break;

	case DEBUG_STEP:
		if_print_debug(
		    "Run one game turn, then re-enter the debugger.  Useful for games that"
		    " intercept empty input lines, which otherwise catch the 'debug'"
		    " command before SCARE can get to it.\n");
		break;

	case DEBUG_QUIT:
		if_print_debug(
		    "Exit the interpreter main loop.  Equivalent to a confirmed 'quit'"
		    " from within the game itself, this ends the interpreter session.\n");
		break;

	case DEBUG_BUFFER:
		if_print_debug(
		    "Print the current text that the game has buffered for output.  The"
		    " debugger catches games before they have printed their turn output"
		    " -- this is the text that will be filtered and printed on exiting the"
		    " debugger.\n");
		break;

	case DEBUG_RESOURCES:
		if_print_debug(
		    "Print any resources currently active, and any requested by the game"
		    " on the current turn.  The requested resources will become the active"
		    " ones on exiting the debugger.\n");
		break;

	case DEBUG_RANDOM:
		if_print_debug(
		    "If no seed is given, report the current random number generator"
		    " setting.  Otherwise, seed the random number generator with the value"
		    " given.  This is useful for persuading games with random sections to"
		    " behave predictably.  A new seed value of zero is invalid.\n");
		break;

	case DEBUG_GAME:
		if_print_debug(
		    "Print general game information, including the number of rooms,"
		    " objects, events, tasks, and variables that the game defines\n");
		break;

	case DEBUG_PLAYER:
		if_print_debug(
		    "Print out the current player room and position, and any parent object"
		    " of the player character.\n");
		break;

	case DEBUG_OLDPLAYER:
		if_print_debug(
		    "Print out the player room and position from the previous turn, and"
		    " any parent object of the player character.\n");
		break;

	case DEBUG_ROOMS:
		if_print_debug(
		    "Print out the name and contents of rooms in the range.  If no range,"
		    " print details of the room containing the player.\n");
		break;

	case DEBUG_OLDROOMS:
		if_print_debug(
		    "Print out the name and contents of rooms in the range for the"
		    " previous turn.  If no range, print details of the room that"
		    " contained the player on the previous turn.\n");
		break;

	case DEBUG_OBJECTS:
		if_print_debug(
		    "Print out details of all objects in the range.  If no range, print"
		    " details of objects in the room containing the player, and visible to"
		    " the player.\n");
		break;

	case DEBUG_OLDOBJECTS:
		if_print_debug(
		    "Print out details of all objects in the range for the previous turn."
		    "  If no range, print details of objects in the room that contained"
		    " the player, and were visible to the player.\n");
		break;

	case DEBUG_NPCS:
		if_print_debug(
		    "Print out details of all NPCs in the range.  If no range, print"
		    " details of only NPCs in the room containing the player.\n");
		break;

	case DEBUG_OLDNPCS:
		if_print_debug(
		    "Print out details of all NPCs in the range for the previous turn."
		    "  If no range, print details of only NPCs in the room that contained"
		    " the player.\n");
		break;

	case DEBUG_EVENTS:
		if_print_debug(
		    "Print out details of all events in the range.  If no range, print"
		    " details of only events currently running.\n");
		break;

	case DEBUG_OLDEVENTS:
		if_print_debug(
		    "Print out details of all events in the range for the previous turn."
		    "  If no range, print details of only events running on the previous"
		    " turn.\n");
		break;

	case DEBUG_TASKS:
		if_print_debug(
		    "Print out details of all tasks in the range.  If no range, print"
		    " details of only tasks that are runnable, for the current state of"
		    " the game.\n");
		break;

	case DEBUG_OLDTASKS:
		if_print_debug(
		    "Print out details of all tasks in the range for the previous turn."
		    "  If no range, print details of only tasks that were runnable, for"
		    " the previous state of the game.\n");
		break;

	case DEBUG_VARIABLES:
		if_print_debug(
		    "Print out the names, types, and values of all game variables in the"
		    " range.  If no range, print details of all variables (equivalent to"
		    " 'variables *').\n");
		break;

	case DEBUG_OLDVARIABLES:
		if_print_debug(
		    "Print out the names, types, and values at the previous turn of all"
		    " game variables in the range.  If no range, print details of all"
		    " variables (equivalent to 'variables *').\n");
		break;

	case DEBUG_WATCHPLAYER:
		if_print_debug(
		    "If no range is given, list any watchpoint on player movement.  If"
		    " range '0' is given, set a watchpoint on player movement.  Other"
		    " usages of 'watchplayer' behave as if no range is given.\n");
		break;

	case DEBUG_WATCHOBJECTS:
		if_print_debug(
		    "Set watchpoints on all objects in the range.  If no range, list out"
		    " object watchpoints currently set.\n");
		break;

	case DEBUG_WATCHNPCS:
		if_print_debug(
		    "Set watchpoints on all NPCs in the range.  If no range, list out NPC"
		    " watchpoints currently set.\n");
		break;

	case DEBUG_WATCHEVENTS:
		if_print_debug(
		    "Set watchpoints on all events in the range.  If no range, list out"
		    " event watchpoints currently set.\n");
		break;

	case DEBUG_WATCHTASKS:
		if_print_debug(
		    "Set watchpoints on all tasks in the range.  If no range, list out"
		    " task watchpoints currently set.\n");
		break;

	case DEBUG_WATCHVARIABLES:
		if_print_debug(
		    "Set watchpoints on all game variables in the range.  If no range,"
		    " list variable watchpoints currently set.\n");
		break;

	case DEBUG_CLEARPLAYER:
		if_print_debug(
		    "Clear any watchpoint set on player movements.\n");
		break;

	case DEBUG_CLEAROBJECTS:
		if_print_debug(
		    "Clear watchpoints on all objects in the range.  If no range, list"
		    " out object watchpoints currently set.\n");
		break;

	case DEBUG_CLEARNPCS:
		if_print_debug(
		    "Clear watchpoints on all NPCs in the range.  If no range, list out"
		    " NPC watchpoints currently set.\n");
		break;

	case DEBUG_CLEAREVENTS:
		if_print_debug(
		    "Clear watchpoints on all events in the range.  If no range, list out"
		    " event watchpoints currently set.\n");
		break;

	case DEBUG_CLEARTASKS:
		if_print_debug(
		    "Clear watchpoints on all tasks in the range.  If no range, list out"
		    " task watchpoints currently set.\n");
		break;

	case DEBUG_CLEARVARIABLES:
		if_print_debug(
		    "Clear watchpoints on all game variables in the range.  If no range,"
		    " list variable watchpoints currently set.\n");
		break;

	case DEBUG_WATCHALL:
		if_print_debug(
		    "Print out a list of all all watchpoints set for all the classes of"
		    " item on which watchpoints can be used.\n");
		break;

	case DEBUG_CLEARALL:
		if_print_debug(
		    "Clear all watchpoints set, on all classes of item on which"
		    " watchpoints can be used.\n");
		break;

	default:
		if_print_debug(
		    "Sorry, there is no help available on that at the moment.\n");
		break;
	}
}


/*
 * debug_print_quoted()
 * debug_print_player()
 * debug_print_room()
 * debug_print_object()
 * debug_print_npc()
 * debug_print_event()
 * debug_print_task()
 * debug_print_variable()
 *
 * Low level output helpers.
 */
static void debug_print_quoted(const sc_char *string) {
	if_print_debug_character('"');
	if_print_debug(string);
	if_print_debug_character('"');
}

static void debug_print_player(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	const sc_char *playername;

	vt_key[0].string = "Globals";
	vt_key[1].string = "PlayerName";
	playername = prop_get_string(bundle, "S<-ss", vt_key);
	if_print_debug("Player ");
	debug_print_quoted(playername);
}

static void debug_print_room(sc_gameref_t game, sc_int room) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_char buffer[32];
	const sc_char *name;

	if_print_debug("Room ");
	if (room < 0 || room >= gs_room_count(game)) {
		sprintf(buffer, "%ld ", room);
		if_print_debug(buffer);
		if_print_debug("[Out of range]");
		return;
	}

	vt_key[0].string = "Rooms";
	vt_key[1].integer = room;
	vt_key[2].string = "Short";
	name = prop_get_string(bundle, "S<-sis", vt_key);
	sprintf(buffer, "%ld ", room);
	if_print_debug(buffer);
	debug_print_quoted(name);
}

static void debug_print_object(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_bool bstatic;
	sc_char buffer[32];
	const sc_char *prefix, *name;

	if (object < 0 || object >= gs_object_count(game)) {
		if_print_debug("Object ");
		sprintf(buffer, "%ld ", object);
		if_print_debug(buffer);
		if_print_debug("[Out of range]");
		return;
	}

	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Static";
	bstatic = prop_get_boolean(bundle, "B<-sis", vt_key);
	vt_key[2].string = "Prefix";
	prefix = prop_get_string(bundle, "S<-sis", vt_key);
	vt_key[2].string = "Short";
	name = prop_get_string(bundle, "S<-sis", vt_key);
	if (bstatic)
		if_print_debug("Static ");
	else
		if_print_debug("Dynamic ");
	sprintf(buffer, "%ld ", object);
	if_print_debug(buffer);
	debug_print_quoted(prefix);
	if_print_debug_character(' ');
	debug_print_quoted(name);
}

static void debug_print_npc(sc_gameref_t game, sc_int npc) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_char buffer[32];
	const sc_char *prefix, *name;

	if_print_debug("NPC ");
	if (npc < 0 || npc >= gs_npc_count(game)) {
		sprintf(buffer, "%ld ", npc);
		if_print_debug(buffer);
		if_print_debug("[Out of range]");
		return;
	}

	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Prefix";
	prefix = prop_get_string(bundle, "S<-sis", vt_key);
	vt_key[2].string = "Name";
	name = prop_get_string(bundle, "S<-sis", vt_key);
	sprintf(buffer, "%ld ", npc);
	if_print_debug(buffer);
	debug_print_quoted(prefix);
	if_print_debug_character(' ');
	debug_print_quoted(name);
}

static void debug_print_event(sc_gameref_t game, sc_int event) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_char buffer[32];
	const sc_char *name;

	if_print_debug("Event ");
	if (event < 0 || event >= gs_event_count(game)) {
		sprintf(buffer, "%ld ", event);
		if_print_debug(buffer);
		if_print_debug("[Out of range]");
		return;
	}

	vt_key[0].string = "Events";
	vt_key[1].integer = event;
	vt_key[2].string = "Short";
	name = prop_get_string(bundle, "S<-sis", vt_key);
	sprintf(buffer, "%ld ", event);
	if_print_debug(buffer);
	debug_print_quoted(name);
}

static void debug_print_task(sc_gameref_t game, sc_int task) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_char buffer[32];
	const sc_char *command;

	if_print_debug("Task ");
	if (task < 0 || task >= gs_task_count(game)) {
		sprintf(buffer, "%ld ", task);
		if_print_debug(buffer);
		if_print_debug("[Out of range]");
		return;
	}

	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = "Command";
	vt_key[3].integer = 0;
	command = prop_get_string(bundle, "S<-sisi", vt_key);
	sprintf(buffer, "%ld ", task);
	if_print_debug(buffer);
	debug_print_quoted(command);
}

static void debug_print_variable(sc_gameref_t game, sc_int variable) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_vartype_t vt_key[3], vt_rvalue;
	sc_char buffer[32];
	sc_int var_type;
	const sc_char *name;

	if (variable < 0 || variable >= debug_variable_count(game)) {
		if_print_debug("Variable ");
		sprintf(buffer, "%ld ", variable);
		if_print_debug(buffer);
		if_print_debug("[Out of range]");
		return;
	}

	vt_key[0].string = "Variables";
	vt_key[1].integer = variable;
	vt_key[2].string = "Name";
	name = prop_get_string(bundle, "S<-sis", vt_key);

	if (var_get(vars, name, &var_type, &vt_rvalue)) {
		switch (var_type) {
		case VAR_INTEGER:
			if_print_debug("Integer ");
			break;
		case VAR_STRING:
			if_print_debug("String ");
			break;
		default:
			if_print_debug("[Invalid type] ");
			break;
		}
	} else
		if_print_debug("[Invalid variable] ");
	sprintf(buffer, "%ld ", variable);
	if_print_debug(buffer);
	debug_print_quoted(name);
}


/*
 * debug_game()
 *
 * Display overall game details.
 */
static void debug_game(sc_gameref_t game, sc_command_type_t type) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_debuggerref_t debug = debug_get_debugger(game);
	sc_vartype_t vt_key[2];
	const sc_char *version, *gamename, *compiledate, *gameauthor;
	sc_int perspective, waitturns;
	sc_bool has_sound, has_graphics, has_battle;
	sc_char buffer[32];
	assert(debug_is_valid(debug));

	if (type != COMMAND_QUERY) {
		if_print_debug("The Game command takes no arguments.\n");
		return;
	}

	if_print_debug("Game ");
	vt_key[0].string = "Globals";
	vt_key[1].string = "GameName";
	gamename = prop_get_string(bundle, "S<-ss", vt_key);
	debug_print_quoted(gamename);
	if_print_debug_character('\n');

	if_print_debug("    Compiled ");
	vt_key[0].string = "CompileDate";
	compiledate = prop_get_string(bundle, "S<-s", vt_key);
	debug_print_quoted(compiledate);

	if_print_debug(", Author ");
	vt_key[0].string = "Globals";
	vt_key[1].string = "GameAuthor";
	gameauthor = prop_get_string(bundle, "S<-ss", vt_key);
	debug_print_quoted(gameauthor);
	if_print_debug_character('\n');

	vt_key[0].string = "VersionString";
	version = prop_get_string(bundle, "S<-s", vt_key);
	if_print_debug("    Version ");
	if_print_debug(version);

	vt_key[0].string = "Globals";
	vt_key[1].string = "Perspective";
	perspective = prop_get_integer(bundle, "I<-ss", vt_key);
	switch (perspective) {
	case 0:
		if_print_debug(", First person");
		break;
	case 1:
		if_print_debug(", Second person");
		break;
	case 2:
		if_print_debug(", Third person");
		break;
	default:
		if_print_debug(", [Unknown perspective]");
		break;
	}

	vt_key[0].string = "Globals";
	vt_key[1].string = "WaitTurns";
	waitturns = prop_get_integer(bundle, "I<-ss", vt_key);
	if_print_debug(", Waitturns ");
	sprintf(buffer, "%ld", waitturns);
	if_print_debug(buffer);

	vt_key[0].string = "Globals";
	vt_key[1].string = "Sound";
	has_sound = prop_get_boolean(bundle, "B<-ss", vt_key);
	vt_key[1].string = "Graphics";
	has_graphics = prop_get_boolean(bundle, "B<-ss", vt_key);
	if (has_sound)
		if_print_debug(", Sound");
	if (has_graphics)
		if_print_debug(", Graphics");
	if_print_debug_character('\n');

	vt_key[0].string = "Globals";
	vt_key[1].string = "BattleSystem";
	has_battle = prop_get_boolean(bundle, "B<-ss", vt_key);
	if (has_battle)
		if_print_debug("    Battle system\n");

	if_print_debug("    Room count ");
	sprintf(buffer, "%ld", gs_room_count(game));
	if_print_debug(buffer);

	if_print_debug(", Object count ");
	sprintf(buffer, "%ld", gs_object_count(game));
	if_print_debug(buffer);

	if_print_debug(", NPC count ");
	sprintf(buffer, "%ld", gs_npc_count(game));
	if_print_debug(buffer);
	if_print_debug_character('\n');

	if_print_debug("    Event count ");
	sprintf(buffer, "%ld", gs_event_count(game));
	if_print_debug(buffer);

	if_print_debug(", Task count ");
	sprintf(buffer, "%ld", gs_task_count(game));
	if_print_debug(buffer);

	if_print_debug(", Variable count ");
	sprintf(buffer, "%ld", debug_variable_count(game));
	if_print_debug(buffer);
	if_print_debug_character('\n');

	if (game->is_running)
		if_print_debug("    Running");
	else
		if_print_debug("    Not running");
	if (game->has_completed)
		if_print_debug(", Completed");
	else
		if_print_debug(", Not completed");
	if (game->verbose)
		if_print_debug(", Verbose");
	else
		if_print_debug(", Not verbose");
	if (game->bold_room_names)
		if_print_debug(", Bold");
	else
		if_print_debug(", Not bold");
	if (game->undo_available)
		if_print_debug(", Undo");
	else
		if_print_debug(", No undo");
	if_print_debug_character('\n');

	if_print_debug("    Score ");
	sprintf(buffer, "%ld", game->score);
	if_print_debug(buffer);
	if_print_debug(", Turns ");
	sprintf(buffer, "%ld", game->turns);
	if_print_debug(buffer);
	if_print_debug(", Seconds ");
	sprintf(buffer, "%lu", debug->elapsed_seconds);
	if_print_debug(buffer);
	if_print_debug_character('\n');
}


/*
 * debug_player()
 *
 * Print a few brief details about the player status.
 */
static void debug_player(sc_gameref_t game, sc_command_t command, sc_command_type_t type) {
	if (type != COMMAND_QUERY) {
		if_print_debug("The Player command takes no arguments.\n");
		return;
	}

	if (command == DEBUG_OLDPLAYER) {
		if (!game->undo_available) {
			if_print_debug("There is no previous game state to examine.\n");
			return;
		}

		game = game->undo;
		assert(gs_is_game_valid(game));
	}

	debug_print_player(game);
	if_print_debug_character('\n');

	if (gs_playerroom(game) == -1)
		if_print_debug("    Hidden!\n");
	else {
		if_print_debug("    In ");
		debug_print_room(game, gs_playerroom(game));
		if_print_debug_character('\n');
	}

	switch (gs_playerposition(game)) {
	case 0:
		if_print_debug("    Standing\n");
		break;
	case 1:
		if_print_debug("    Sitting\n");
		break;
	case 2:
		if_print_debug("    Lying\n");
		break;
	default:
		if_print_debug("    [Invalid position]\n");
		break;
	}

	if (gs_playerparent(game) != -1) {
		if_print_debug("    Parent is ");
		debug_print_object(game, gs_playerparent(game));
		if_print_debug_character('\n');
	}
}


/*
 * debug_normalize_arguments()
 *
 * Normalize a set of arguments parsed from a debugger command line, for
 * debug commands that take ranges.
 */
static sc_bool debug_normalize_arguments(sc_command_type_t type, sc_int *arg1, sc_int *arg2, sc_int limit) {
	sc_int low = 0, high = 0;

	/* Set range low and high depending on the command type. */
	switch (type) {
	case COMMAND_QUERY:
	case COMMAND_ALL:
		low = 0;
		high = limit - 1;
		break;
	case COMMAND_ONE:
		low = *arg1;
		high = *arg1;
		break;
	case COMMAND_RANGE:
		low = *arg1;
		high = *arg2;
		break;
	default:
		sc_fatal("debug_normalize_arguments: bad command type\n");
	}

	/* If range is valid, copy out and return TRUE. */
	if (low >= 0 && low < limit && high >= 0 && high < limit && high >= low) {
		*arg1 = low;
		*arg2 = high;
		return TRUE;
	}

	/* Input range is invalid. */
	return FALSE;
}


/*
 * debug_filter_room()
 * debug_dump_room()
 *
 * Print details of rooms and their direct contents.
 */
static sc_bool debug_filter_room(sc_gameref_t game, sc_int room) {
	return room == gs_playerroom(game);
}

static void debug_dump_room(sc_gameref_t game, sc_int room) {
	sc_int object, npc;

	debug_print_room(game, room);
	if_print_debug_character('\n');

	if (gs_room_seen(game, room))
		if_print_debug("    Visited\n");
	else
		if_print_debug("    Not visited\n");

	if (gs_playerroom(game) == room) {
		if_print_debug("    ");
		debug_print_player(game);
		if_print_debug_character('\n');
	}

	for (object = 0; object < gs_object_count(game); object++) {
		if (obj_indirectly_in_room(game, object, room)) {
			if_print_debug("    ");
			debug_print_object(game, object);
			if_print_debug_character('\n');
		}
	}

	for (npc = 0; npc < gs_npc_count(game); npc++) {
		if (npc_in_room(game, npc, room)) {
			if_print_debug("    ");
			debug_print_npc(game, npc);
			if_print_debug_character('\n');
		}
	}
}


/*
 * debug_filter_object()
 * debug_dump_object()
 *
 * Print the changeable details of game objects.
 */
static sc_bool debug_filter_object(sc_gameref_t game, sc_int object) {
	return obj_indirectly_in_room(game, object, gs_playerroom(game));
}

static void debug_dump_object(sc_gameref_t game, sc_int object) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int openness;
	sc_vartype_t vt_key[3];
	sc_bool bstatic, is_statussed;
	sc_int position, parent;

	debug_print_object(game, object);
	if_print_debug_character('\n');

	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Static";
	bstatic = prop_get_boolean(bundle, "B<-sis", vt_key);

	if (gs_object_seen(game, object))
		if_print_debug("    Seen");
	else
		if_print_debug("    Not seen");
	if (bstatic) {
		if (gs_object_static_unmoved(game, object))
			if_print_debug(", Not relocated");
		else
			if_print_debug(", Relocated");
	} else {
		vt_key[2].string = "OnlyWhenNotMoved";
		if (prop_get_integer(bundle, "I<-sis", vt_key) == 1) {
			if (gs_object_unmoved(game, object))
				if_print_debug(", Not moved");
			else
				if_print_debug(", Moved");
		}
	}
	openness = gs_object_openness(game, object);
	switch (openness) {
	case OBJ_OPEN:
		if_print_debug(", Open");
		break;
	case OBJ_CLOSED:
		if_print_debug(", Closed");
		break;
	case OBJ_LOCKED:
		if_print_debug(", Locked");
		break;
	default:
		if_print_debug(", Unknown openness");
		break;
	}
	if_print_debug_character('\n');

	position = gs_object_position(game, object);
	parent = gs_object_parent(game, object);
	switch (position) {
	case OBJ_HIDDEN:
		if (bstatic)
			if_print_debug("    Static default\n");
		else
			if_print_debug("    Hidden\n");
		break;
	case OBJ_HELD_PLAYER:
		if_print_debug("    Held by ");
		debug_print_player(game);
		if_print_debug_character('\n');
		break;
	case OBJ_HELD_NPC:
		if_print_debug("    Held by ");
		debug_print_npc(game, parent);
		if_print_debug_character('\n');
		break;
	case OBJ_WORN_PLAYER:
		if_print_debug("    Worn by ");
		debug_print_player(game);
		if_print_debug_character('\n');
		break;
	case OBJ_WORN_NPC:
		if_print_debug("    Worn by ");
		debug_print_npc(game, parent);
		if_print_debug_character('\n');
		break;
	case OBJ_PART_NPC:
		if_print_debug("    Part of ");
		if (parent == -1)
			debug_print_player(game);
		else
			debug_print_npc(game, parent);
		if_print_debug_character('\n');
		break;
	case OBJ_ON_OBJECT:
		if_print_debug("    On ");
		debug_print_object(game, parent);
		if_print_debug_character('\n');
		break;
	case OBJ_IN_OBJECT:
		if_print_debug("    Inside ");
		debug_print_object(game, parent);
		if_print_debug_character('\n');
		break;
	default:
		if_print_debug("    In ");
		debug_print_room(game, position - 1);
		if_print_debug_character('\n');
		break;
	}

	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "CurrentState";
	is_statussed = prop_get_integer(bundle, "I<-sis", vt_key) != 0;
	if (is_statussed) {
		sc_char buffer[32];
		const sc_char *states;

		if_print_debug("    State ");
		sprintf(buffer, "%ld", gs_object_state(game, object));
		if_print_debug(buffer);

		vt_key[2].string = "States";
		states = prop_get_string(bundle, "S<-sis", vt_key);
		if_print_debug(" of ");
		debug_print_quoted(states);
		if_print_debug_character('\n');
	}
}


/*
 * debug_filter_npc()
 * debug_dump_npc()
 *
 * Print stuff about NPCs.
 */
static sc_bool debug_filter_npc(sc_gameref_t game, sc_int npc) {
	return npc_in_room(game, npc, gs_playerroom(game));
}

static void debug_dump_npc(sc_gameref_t game, sc_int npc) {
	debug_print_npc(game, npc);
	if_print_debug_character('\n');

	if (gs_npc_seen(game, npc))
		if_print_debug("    Seen\n");
	else
		if_print_debug("    Not seen\n");

	if (gs_npc_location(game, npc) - 1 == -1)
		if_print_debug("    Hidden\n");
	else {
		if_print_debug("    In ");
		debug_print_room(game, gs_npc_location(game, npc) - 1);
		if_print_debug_character('\n');
	}

	switch (gs_npc_position(game, npc)) {
	case 0:
		if_print_debug("    Standing\n");
		break;
	case 1:
		if_print_debug("    Sitting\n");
		break;
	case 2:
		if_print_debug("    Lying\n");
		break;
	default:
		if_print_debug("    [Invalid position]\n");
		break;
	}

	if (gs_npc_parent(game, npc) != -1) {
		if_print_debug("    Parent is ");
		debug_print_object(game, gs_npc_parent(game, npc));
		if_print_debug_character('\n');
	}

	if (gs_npc_walkstep_count(game, npc) > 0) {
		sc_char buffer[32];
		sc_int walk;

		if_print_debug("    Walkstep count ");
		sprintf(buffer, "%ld", gs_npc_walkstep_count(game, npc));
		if_print_debug(buffer);
		if_print_debug(", Walks { ");
		for (walk = 0; walk < gs_npc_walkstep_count(game, npc); walk++) {
			sprintf(buffer, "%ld", gs_npc_walkstep(game, npc, walk));
			if_print_debug(buffer);
			if_print_debug_character(' ');
		}
		if_print_debug("}.\n");
	}
}


/*
 * debug_filter_event()
 * debug_dump_event()
 *
 * Print stuff about events.
 */
static sc_bool debug_filter_event(sc_gameref_t game, sc_int event) {
	return gs_event_state(game, event) == ES_RUNNING;
}

static void debug_dump_event(sc_gameref_t game, sc_int event) {
	sc_char buffer[32];

	debug_print_event(game, event);
	if_print_debug_character('\n');

	switch (gs_event_state(game, event)) {
	case ES_WAITING:
		if_print_debug("    Waiting\n");
		break;
	case ES_RUNNING:
		if_print_debug("    Running\n");
		break;
	case ES_AWAITING:
		if_print_debug("    Awaiting\n");
		break;
	case ES_FINISHED:
		if_print_debug("    Finished\n");
		break;
	case ES_PAUSED:
		if_print_debug("    Paused\n");
		break;
	default:
		if_print_debug("    [Invalid state]\n");
		break;
	}

	if_print_debug("    Time ");
	sprintf(buffer, "%ld\n", gs_event_time(game, event));
	if_print_debug(buffer);
}


/*
 * debug_filter_task()
 * debug_dump_task()
 *
 * Print stuff about tasks.
 */
static sc_bool debug_filter_task(sc_gameref_t game, sc_int task) {
	return task_can_run_task(game, task);
}

static void debug_dump_task(sc_gameref_t game, sc_int task) {
	debug_print_task(game, task);
	if_print_debug_character('\n');

	if (task_can_run_task(game, task))
		if_print_debug("    Runnable");
	else
		if_print_debug("    Not runnable");
	if (gs_task_done(game, task))
		if_print_debug(", Done");
	else
		if_print_debug(", Not done");
	if (gs_task_scored(game, task))
		if_print_debug(", Scored\n");
	else
		if_print_debug(", Not scored\n");
}


/*
 * debug_dump_variable()
 *
 * Print stuff about variables.
 */
static void debug_dump_variable(sc_gameref_t game, sc_int variable) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_vartype_t vt_key[3], vt_rvalue;
	const sc_char *name;
	sc_int var_type;

	debug_print_variable(game, variable);
	if_print_debug_character('\n');

	vt_key[0].string = "Variables";
	vt_key[1].integer = variable;
	vt_key[2].string = "Name";
	name = prop_get_string(bundle, "S<-sis", vt_key);

	if_print_debug("    Value = ");
	if (var_get(vars, name, &var_type, &vt_rvalue)) {
		switch (var_type) {
		case VAR_INTEGER: {
			sc_char buffer[32];

			sprintf(buffer, "%ld", vt_rvalue.integer);
			if_print_debug(buffer);
			break;
		}
		case VAR_STRING:
			debug_print_quoted(vt_rvalue.string);
			break;
		default:
			if_print_debug("[Unknown]");
			break;
		}
	} else
		if_print_debug("[Unknown]");
	if_print_debug_character('\n');
}


/*
 * debug_dump_common()
 *
 * Common handler for iterating dumps of classes.
 */
static void debug_dump_common(sc_gameref_t game, sc_command_t command,
		sc_command_type_t type, sc_int arg1, sc_int arg2) {
	sc_int low = arg1, high = arg2;
	sc_int limit, index_;
	const sc_char *class_;
	sc_bool(*filter_function)(sc_gameref_t, sc_int);
	void (*dumper_function)(sc_gameref_t, sc_int);
	sc_bool printed = FALSE;

	/* Initialize variables to avoid gcc warnings. */
	limit = 0;
	class_ = NULL;
	filter_function = NULL;
	dumper_function = NULL;

	/* Switch to undo game on relevant commands. */
	switch (command) {
	case DEBUG_OLDROOMS:
	case DEBUG_OLDOBJECTS:
	case DEBUG_OLDNPCS:
	case DEBUG_OLDEVENTS:
	case DEBUG_OLDTASKS:
	case DEBUG_OLDVARIABLES:
		if (!game->undo_available) {
			if_print_debug("There is no previous game state to examine.\n");
			return;
		}

		game = game->undo;
		assert(gs_is_game_valid(game));
		break;

	default:
		break;
	}

	/* Demultiplex dump command. */
	switch (command) {
	case DEBUG_ROOMS:
	case DEBUG_OLDROOMS:
		class_ = "Room";
		filter_function = debug_filter_room;
		dumper_function = debug_dump_room;
		limit = gs_room_count(game);
		break;
	case DEBUG_OBJECTS:
	case DEBUG_OLDOBJECTS:
		class_ = "Object";
		filter_function = debug_filter_object;
		dumper_function = debug_dump_object;
		limit = gs_object_count(game);
		break;
	case DEBUG_NPCS:
	case DEBUG_OLDNPCS:
		class_ = "NPC";
		filter_function = debug_filter_npc;
		dumper_function = debug_dump_npc;
		limit = gs_npc_count(game);
		break;
	case DEBUG_EVENTS:
	case DEBUG_OLDEVENTS:
		class_ = "Event";
		filter_function = debug_filter_event;
		dumper_function = debug_dump_event;
		limit = gs_event_count(game);
		break;
	case DEBUG_TASKS:
	case DEBUG_OLDTASKS:
		class_ = "Task";
		filter_function = debug_filter_task;
		dumper_function = debug_dump_task;
		limit = gs_task_count(game);
		break;
	case DEBUG_VARIABLES:
	case DEBUG_OLDVARIABLES:
		class_ = "Variable";
		filter_function = NULL;
		dumper_function = debug_dump_variable;
		limit = debug_variable_count(game);
		break;
	default:
		sc_fatal("debug_dump_common: invalid command\n");
	}

	/* Normalize to this limit. */
	if (!debug_normalize_arguments(type, &low, &high, limit)) {
		if (limit == 0) {
			if_print_debug("There is nothing of type ");
			debug_print_quoted(class_);
			if_print_debug(" to print.\n");
		} else {
			if_print_debug("Invalid item or range for ");
			debug_print_quoted(class_);
			if (limit == 1)
				if_print_debug("; only 0 is valid.\n");
			else {
				sc_char buffer[32];

				if_print_debug("; valid values are 0 to ");
				sprintf(buffer, "%ld", limit - 1);
				if_print_debug(buffer);
				if_print_debug(".\n");
			}
		}
		return;
	}

	/* Print each item of the class, filtering on query commands. */
	for (index_ = low; index_ <= high; index_++) {
		if (type == COMMAND_QUERY
		        && filter_function && !filter_function(game, index_))
			continue;

		if (printed)
			if_print_debug_character('\n');
		dumper_function(game, index_);
		printed = TRUE;
	}
	if (!printed) {
		if_print_debug("Nothing of type ");
		debug_print_quoted(class_);
		if_print_debug(" is relevant.\nTry \"");
		if_print_debug(class_);
		if_print_debug(" *\" to show all items of this type.\n");
	}
}


/*
 * debug_buffer()
 *
 * Print the current raw printfilter contents.
 */
static void debug_buffer(sc_gameref_t game, sc_command_type_t type) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_char *buffer;

	if (type != COMMAND_QUERY) {
		if_print_debug("The Buffer command takes no arguments.\n");
		return;
	}

	buffer = pf_get_buffer(filter);
	if (buffer)
		if_print_debug(buffer);
	else
		if_print_debug("There is no game text buffered.\n");
}


/*
 * debug_print_resource()
 *
 * Helper for debug_resources().
 */
static void debug_print_resource(const sc_resource_t *resource) {
	sc_char buffer[32];

	debug_print_quoted(resource->name);
	if_print_debug(", offset ");
	sprintf(buffer, "%ld", resource->offset);
	if_print_debug(buffer);
	if_print_debug(", length ");
	sprintf(buffer, "%ld", resource->length);
	if_print_debug(buffer);
}


/*
 * debug_resources()
 *
 * Print any active and requested resources.
 */
static void debug_resources(sc_gameref_t game, sc_command_type_t type) {
	sc_bool printed = FALSE;

	if (type != COMMAND_QUERY) {
		if_print_debug("The Resources command takes no arguments.\n");
		return;
	}

	if (game->stop_sound) {
		if_print_debug("Sound stop");
		if (strlen(game->requested_sound.name) > 0)
			if_print_debug(" before new sound");
		if_print_debug(" requested");
		if (game->sound_active)
			if_print_debug(", sound active");
		if_print_debug(".\n");
		printed = TRUE;
	}
	if (!res_compare_resource(&game->requested_sound,
	                          &game->playing_sound)) {
		if_print_debug("Requested Sound ");
		debug_print_resource(&game->requested_sound);
		if_print_debug(".\n");
		printed = TRUE;
	}
	if (!res_compare_resource(&game->requested_graphic,
	                          &game->displayed_graphic)) {
		if_print_debug("Requested Graphic ");
		debug_print_resource(&game->requested_graphic);
		if_print_debug(".\n");
		printed = TRUE;
	}

	if (strlen(game->playing_sound.name) > 0) {
		if_print_debug("Playing Sound ");
		debug_print_resource(&game->playing_sound);
		if_print_debug(".\n");
		printed = TRUE;
	}
	if (strlen(game->displayed_graphic.name) > 0) {
		if_print_debug("Displaying Graphic ");
		debug_print_resource(&game->displayed_graphic);
		if_print_debug(".\n");
		printed = TRUE;
	}

	if (!printed)
		if_print_debug("There is no game resource activity.\n");
}


/*
 * debug_random()
 *
 * Report the PRNG in use, and seed the random number generator to the
 * given value.
 */
static void debug_random(sc_command_type_t type, sc_int new_seed) {
	const sc_char *random_type;
	sc_char buffer[32];

	if (type != COMMAND_ONE && type != COMMAND_QUERY) {
		if_print_debug("The Random command takes either one argument or"
		               " no arguments.\n");
		return;
	}

	random_type = sc_is_congruential_random() ? "congruential" : "platform";

	if (type == COMMAND_QUERY) {
		if_print_debug("The ");
		if_print_debug(random_type);
		if_print_debug(" random number generator is selected.\n");
		return;
	}

	if (new_seed == 0) {
		if_print_debug("The seed value may not be zero.\n");
		return;
	}

	sc_seed_random(new_seed);

	if_print_debug("Set seed ");
	sprintf(buffer, "%ld", new_seed);
	if_print_debug(buffer);
	if_print_debug(" for the ");
	if_print_debug(random_type);
	if_print_debug(" random number generator.\n");
}


/*
 * debug_watchpoint_common()
 *
 * Common handler for setting and clearing watchpoints.
 */
static void debug_watchpoint_common(sc_gameref_t game, sc_command_t command,
		sc_command_type_t type, sc_int arg1, sc_int arg2) {
	const sc_debuggerref_t debug = debug_get_debugger(game);
	sc_int low = arg1, high = arg2;
	sc_int limit, index_;
	const sc_char *class_;
	sc_bool *watchpoints, action;
	sc_char buffer[32];
	assert(debug_is_valid(debug));

	/* Initialize variables to avoid gcc warnings. */
	limit = 0;
	class_ = NULL;
	watchpoints = NULL;
	action = FALSE;

	/* Set action to TRUE or FALSE, for setting/clearing watchpoints. */
	switch (command) {
	case DEBUG_WATCHPLAYER:
	case DEBUG_WATCHOBJECTS:
	case DEBUG_WATCHNPCS:
	case DEBUG_WATCHEVENTS:
	case DEBUG_WATCHTASKS:
	case DEBUG_WATCHVARIABLES:
		action = TRUE;
		break;
	case DEBUG_CLEARPLAYER:
	case DEBUG_CLEAROBJECTS:
	case DEBUG_CLEARNPCS:
	case DEBUG_CLEAREVENTS:
	case DEBUG_CLEARTASKS:
	case DEBUG_CLEARVARIABLES:
		action = FALSE;
		break;
	default:
		sc_fatal("debug_watchpoint_common: invalid command\n");
	}

	/* Handle player watchpoint setting. */
	if (command == DEBUG_WATCHPLAYER || command == DEBUG_CLEARPLAYER) {
		if (command == DEBUG_CLEARPLAYER) {
			debug->watch_player = action;
			if_print_debug("Cleared Player watchpoint.\n");
		} else if (type == COMMAND_ONE && arg1 == 0) {
			debug->watch_player = action;
			if_print_debug("Set Player watchpoint.\n");
		} else {
			if (debug->watch_player)
				if_print_debug("Player watchpoint is set.\n");
			else
				if_print_debug("No Player watchpoint is set; to set one, use"
				               " \"Watchplayer 0\".\n");
		}
		return;
	}

	/* Demultiplex watchpoint command. */
	switch (command) {
	case DEBUG_WATCHOBJECTS:
	case DEBUG_CLEAROBJECTS:
		class_ = "Object";
		watchpoints = debug->watch_objects;
		limit = gs_object_count(game);
		break;
	case DEBUG_WATCHNPCS:
	case DEBUG_CLEARNPCS:
		class_ = "NPC";
		watchpoints = debug->watch_npcs;
		limit = gs_npc_count(game);
		break;
	case DEBUG_WATCHEVENTS:
	case DEBUG_CLEAREVENTS:
		class_ = "Event";
		watchpoints = debug->watch_events;
		limit = gs_event_count(game);
		break;
	case DEBUG_WATCHTASKS:
	case DEBUG_CLEARTASKS:
		class_ = "Task";
		watchpoints = debug->watch_tasks;
		limit = gs_task_count(game);
		break;
	case DEBUG_WATCHVARIABLES:
	case DEBUG_CLEARVARIABLES:
		class_ = "Variable";
		watchpoints = debug->watch_variables;
		limit = debug_variable_count(game);
		break;
	default:
		sc_fatal("debug_watchpoint_common: invalid command\n");
	}

	/* Normalize to this limit. */
	if (!debug_normalize_arguments(type, &low, &high, limit)) {
		if (limit == 0) {
			if_print_debug("There is nothing of type ");
			debug_print_quoted(class_);
			if_print_debug(" to watch.\n");
		} else {
			if_print_debug("Invalid item or range for ");
			debug_print_quoted(class_);
			if (limit == 1)
				if_print_debug("; only 0 is valid.\n");
			else {
				if_print_debug("; valid values are 0 to ");
				sprintf(buffer, "%ld", limit - 1);
				if_print_debug(buffer);
				if_print_debug(".\n");
			}
		}
		return;
	}

	/* On query, search the array for set flags, and print out. */
	if (type == COMMAND_QUERY) {
		sc_bool printed = FALSE;

		/* Scan for set watchpoints, and list each found. */
		for (index_ = low; index_ <= high; index_++) {
			if (watchpoints[index_]) {
				if (!printed) {
					if_print_debug("Watchpoints are set for ");
					if_print_debug(class_);
					if_print_debug(" { ");
				}
				sprintf(buffer, "%ld", index_);
				if_print_debug(buffer);
				if_print_debug_character(' ');
				printed = TRUE;
			}
		}
		if (printed)
			if_print_debug("}.\n");
		else {
			if_print_debug("No ");
			if_print_debug(class_);
			if_print_debug(" watchpoints are set.\n");
		}
		return;
	}

	/*
	 * For non-queries, set watchpoint flags as defined in action for
	 * the range determined, and print confirmation.
	 */
	for (index_ = low; index_ <= high; index_++)
		watchpoints[index_] = action;

	if (action)
		if_print_debug("Set ");
	else
		if_print_debug("Cleared ");
	sprintf(buffer, "%ld ", high - low + 1);
	if_print_debug(buffer);
	if_print_debug(class_);
	if (high == low)
		if_print_debug(" watchpoint.\n");
	else
		if_print_debug(" watchpoints.\n");
}


/*
 * debug_watchall_common()
 *
 * Common handler to list out and clear all set watchpoints at a stroke.
 */
static void debug_watchall_common(sc_gameref_t game, sc_command_t command, sc_command_type_t type) {
	const sc_debuggerref_t debug = debug_get_debugger(game);
	assert(debug_is_valid(debug));

	if (type != COMMAND_QUERY) {
		if (command == DEBUG_WATCHALL)
			if_print_debug("The Watchall command takes no arguments.\n");
		else
			if_print_debug("The Clearall command takes no arguments.\n");
		return;
	}

	/* Query all set watchpoints using common watchpoint handler... */
	if (command == DEBUG_WATCHALL) {
		debug_watchpoint_common(game,
		                        DEBUG_WATCHPLAYER, COMMAND_QUERY, 0, 0);
		debug_watchpoint_common(game,
		                        DEBUG_WATCHOBJECTS, COMMAND_QUERY, 0, 0);
		debug_watchpoint_common(game,
		                        DEBUG_WATCHNPCS, COMMAND_QUERY, 0, 0);
		debug_watchpoint_common(game,
		                        DEBUG_WATCHEVENTS, COMMAND_QUERY, 0, 0);
		debug_watchpoint_common(game,
		                        DEBUG_WATCHTASKS, COMMAND_QUERY, 0, 0);
		debug_watchpoint_common(game,
		                        DEBUG_WATCHVARIABLES, COMMAND_QUERY, 0, 0);
		return;
	}

	/* ...but reset all the fast way, with memset(). */
	assert(command == DEBUG_CLEARALL);
	debug->watch_player = FALSE;
	memset(debug->watch_objects, FALSE,
	       gs_object_count(game) * sizeof(*debug->watch_objects));
	memset(debug->watch_npcs, FALSE,
	       gs_npc_count(game) * sizeof(*debug->watch_npcs));
	memset(debug->watch_events, FALSE,
	       gs_event_count(game) * sizeof(*debug->watch_events));
	memset(debug->watch_tasks, FALSE,
	       gs_task_count(game) * sizeof(*debug->watch_tasks));
	memset(debug->watch_variables, FALSE,
	       debug_variable_count(game) * sizeof(*debug->watch_variables));
	if_print_debug("Cleared all watchpoints.\n");
}


/*
 * debug_compare_object()
 *
 * Compare two objects, and return TRUE if the same.
 */
static sc_bool debug_compare_object(sc_gameref_t from, sc_gameref_t with, sc_int object) {
	const sc_objectstate_t *from_object = from->objects + object;
	const sc_objectstate_t *with_object = with->objects + object;

	return from_object->unmoved == with_object->unmoved
	       && from_object->static_unmoved == with_object->static_unmoved
	       && from_object->position == with_object->position
	       && from_object->parent == with_object->parent
	       && from_object->openness == with_object->openness
	       && from_object->state == with_object->state
	       && from_object->seen == with_object->seen;
}


/*
 * debug_compare_npc()
 *
 * Compare two NPCs, and return TRUE if the same.
 */
static sc_bool debug_compare_npc(sc_gameref_t from, sc_gameref_t with, sc_int npc) {
	const sc_npcstate_t *from_npc = from->npcs + npc;
	const sc_npcstate_t *with_npc = with->npcs + npc;

	if (from_npc->walkstep_count != with_npc->walkstep_count)
		sc_fatal("debug_compare_npc: walkstep count error\n");

	return from_npc->location == with_npc->location
	       && from_npc->position == with_npc->position
	       && from_npc->parent == with_npc->parent
	       && from_npc->seen == with_npc->seen
	       && memcmp(from_npc->walksteps, with_npc->walksteps,
	                 from_npc->walkstep_count
	                 * sizeof(*from_npc->walksteps)) == 0;
}


/*
 * debug_compare_event()
 *
 * Compare two events, and return TRUE if the same.
 */
static sc_bool debug_compare_event(sc_gameref_t from, sc_gameref_t with, sc_int event) {
	const sc_eventstate_t *from_event = from->events + event;
	const sc_eventstate_t *with_event = with->events + event;

	return from_event->state == with_event->state
	       && from_event->time == with_event->time;
}


/*
 * debug_compare_task()
 *
 * Compare two tasks, and return TRUE if the same.
 */
static sc_bool debug_compare_task(sc_gameref_t from, sc_gameref_t with, sc_int task) {
	const sc_taskstate_t *from_task = from->tasks + task;
	const sc_taskstate_t *with_task = with->tasks + task;

	return from_task->done == with_task->done
	       && from_task->scored == with_task->scored;
}


/*
 * debug_compare_variable()
 *
 * Compare two variables, and return TRUE if the same.
 */
static sc_bool debug_compare_variable(sc_gameref_t from, sc_gameref_t with, sc_int variable) {
	const sc_prop_setref_t bundle = from->bundle;
	const sc_var_setref_t from_var = from->vars;
	const sc_var_setref_t with_var = with->vars;
	sc_vartype_t vt_key[3], vt_rvalue, vt_rvalue2;
	const sc_char *name;
	sc_int var_type, var_type2;
	sc_bool equal = FALSE;
	vt_rvalue.voidp = vt_rvalue2.voidp = nullptr;

	if (from->bundle != with->bundle)
		sc_fatal("debug_compare_variable: property sharing malfunction\n");

	vt_key[0].string = "Variables";
	vt_key[1].integer = variable;
	vt_key[2].string = "Name";
	name = prop_get_string(bundle, "S<-sis", vt_key);

	if (!var_get(from_var, name, &var_type, &vt_rvalue)
	        || !var_get(with_var, name, &var_type2, &vt_rvalue2))
		sc_fatal("debug_compare_variable: can't find variable %s\n", name);
	else if (var_type != var_type2)
		sc_fatal("debug_compare_variable: variable type mismatch %s\n", name);

	switch (var_type) {
	case VAR_INTEGER:
		equal = (vt_rvalue.integer == vt_rvalue2.integer);
		break;
	case VAR_STRING:
		equal = !strcmp(vt_rvalue.string, vt_rvalue2.string);
		break;
	default:
		sc_fatal("debug_compare_variable:"
		         " invalid variable type, %ld\n", var_type);
	}

	return equal;
}


/*
 * debug_check_class()
 *
 * Central handler for checking watchpoints.  Compares a number of items
 * of a class using the comparison function given, where indicated by a
 * watchpoints flags array.  Prints entries that differ, and returns TRUE
 * if any differed.
 */
static sc_bool debug_check_class(sc_gameref_t from, sc_gameref_t with, const sc_char *class_,
		sc_int class_count, const sc_bool *watchpoints,
		sc_bool(*const compare_function) (sc_gameref_t, sc_gameref_t, sc_int)) {
	sc_int index_;
	sc_bool triggered = FALSE;

	/*
	 * Scan the watchpoints array for set watchpoints, comparing classes
	 * where the watchpoint flag is set.
	 */
	for (index_ = 0; index_ < class_count; index_++) {
		if (!watchpoints[index_])
			continue;

		if (!compare_function(from, with, index_)) {
			sc_char buffer[32];

			if (!triggered) {
				if_print_debug("--- ");
				if_print_debug(class_);
				if_print_debug(" watchpoint triggered { ");
			}
			sprintf(buffer, "%ld ", index_);
			if_print_debug(buffer);
			triggered = TRUE;
		}
	}
	if (triggered)
		if_print_debug("}.\n");

	/* Return TRUE if anything differed. */
	return triggered;
}


/*
 * debug_check_watchpoints()
 *
 * Checks the game against the undo game for all set watchpoints.  Returns
 * TRUE if any triggered, FALSE if none (or if the undo game isn't available,
 * in which case no check is possible).
 */
static sc_bool debug_check_watchpoints(sc_gameref_t game) {
	const sc_debuggerref_t debug = debug_get_debugger(game);
	const sc_gameref_t undo = game->undo;
	sc_bool triggered;
	assert(debug_is_valid(debug) && gs_is_game_valid(undo));

	/* If no undo is present, no check is possible. */
	if (!game->undo_available)
		return FALSE;

	/* Check first for player watchpoint. */
	triggered = FALSE;
	if (debug->watch_player) {
		if (gs_playerroom(game) != gs_playerroom(undo)
		        || gs_playerposition(game) != gs_playerposition(undo)
		        || gs_playerparent(game) != gs_playerparent(undo)) {
			if_print_debug("--- Player watchpoint triggered.\n");
			triggered |= TRUE;
		}
	}

	/* Now check other classes of watchpoint. */
	triggered |= debug_check_class(game, undo,
	                               "Object", gs_object_count(game),
	                               debug->watch_objects, debug_compare_object);
	triggered |= debug_check_class(game, undo,
	                               "NPC", gs_npc_count(game),
	                               debug->watch_npcs, debug_compare_npc);
	triggered |= debug_check_class(game, undo,
	                               "Event", gs_event_count(game),
	                               debug->watch_events, debug_compare_event);
	triggered |= debug_check_class(game, undo,
	                               "Task", gs_task_count(game),
	                               debug->watch_tasks, debug_compare_task);
	triggered |= debug_check_class(game, undo,
	                               "Variable", debug_variable_count(game),
	                               debug->watch_variables,
	                               debug_compare_variable);

	return triggered;
}


/*
 * debug_parse_command()
 *
 * Given a debugging command string, try to parse it and return the
 * appropriate command and its arguments.  Returns DEBUG_NONE if the parse
 * fails.
 */
static sc_command_t debug_parse_command(const sc_char *command_string,
		sc_command_type_t *type, sc_int *arg1, sc_int *arg2, sc_command_t *help_topic) {
	sc_command_t return_command;
	sc_command_type_t return_type;
	sc_int val1, val2, converted, matches;
	sc_char *help, *string, junk, wildcard;
	sc_bool is_help, is_parsed, is_wildcard;
	const sc_strings_t *entry;

	/* Allocate temporary strings long enough to take a copy of the input. */
	string = (sc_char *)sc_malloc(strlen(command_string) + 1);
	help = (sc_char *)sc_malloc(strlen(command_string) + 1);

	/*
	 * Parse the input line, in a very simplistic fashion.  The argument count
	 * is one less than sscanf converts.
	 */
	is_parsed = is_wildcard = is_help = FALSE;
	val1 = val2 = 0;
	converted = sscanf(command_string, " %s %s %c", help, string, &junk);
	if (converted == 2 && sc_strcasecmp(help, "help") == 0) {
		is_help = TRUE;
		is_parsed = TRUE;
	}
	sc_free(help);
	if (!is_parsed) {
		converted = sscanf(command_string,
		                   " %s %ld to %ld %c", string, &val1, &val2, &junk);
		if (converted != 3)
			converted = sscanf(command_string,
			                   " %s %ld - %ld %c", string, &val1, &val2, &junk);
		if (converted != 3)
			converted = sscanf(command_string,
			                   " %s %ld .. %ld %c", string, &val1, &val2, &junk);
		if (converted != 3)
			converted = sscanf(command_string,
			                   " %s %ld %ld %c", string, &val1, &val2, &junk);
		is_parsed |= converted == 3;
	}
	if (!is_parsed) {
		converted = sscanf(command_string,
		                   " %s %ld %c", string, &val1, &junk);
		is_parsed |= converted == 2;
	}
	if (!is_parsed) {
		converted = sscanf(command_string,
		                   " %s %c %c", string, &wildcard, &junk);
		if (converted == 2 && wildcard == '*') {
			is_wildcard = TRUE;
			is_parsed = TRUE;
		} else
			is_parsed |= converted == 1;
	}
	if (!is_parsed) {
		if_print_debug("Invalid debug command.");
		if_print_debug("  Type 'help' for a list of valid commands.\n");
		sc_free(string);
		return DEBUG_NONE;
	}

	/* Decide on a command type based on the parse. */
	if (is_wildcard)
		return_type = COMMAND_ALL;
	else if (converted == 3)
		return_type = COMMAND_RANGE;
	else if (converted == 2)
		return_type = COMMAND_ONE;
	else
		return_type = COMMAND_QUERY;

	/*
	 * Find the first unambiguous command matching the string.  If none,
	 * return DEBUG_NONE.
	 */
	matches = 0;
	return_command = DEBUG_NONE;
	for (entry = DEBUG_COMMANDS; entry->command_string; entry++) {
		if (sc_strncasecmp(string, entry->command_string, strlen(string)) == 0) {
			matches++;
			return_command = entry->command;
		}
	}
	if (matches != 1) {
		if (matches > 1)
			if_print_debug("Ambiguous debug command.");
		else
			if_print_debug("Unrecognized debug command.");
		if_print_debug("  Type 'help' for a list of valid commands.\n");
		sc_free(string);
		return DEBUG_NONE;
	}

	/* Done with temporary command parse area. */
	sc_free(string);

	/*
	 * Return the command type, arguments, and the debugging command.  For help
	 * <topic>, the command is help, with the command on which help requested
	 * in *help_topic.  All clear, then?
	 */
	*type = return_type;
	*arg1 = val1;
	*arg2 = val2;
	*help_topic = is_help ? return_command : DEBUG_NONE;
	return is_help ? DEBUG_HELP : return_command;
}


/*
 * debug_dispatch()
 *
 * Dispatch a debugging command to the appropriate handler.
 */
static void debug_dispatch(sc_gameref_t game, sc_command_t command, sc_command_type_t type,
		sc_int arg1, sc_int arg2, sc_command_t help_topic) {
	/* Demultiplex debugging command, and call handlers. */
	switch (command) {
	case DEBUG_HELP:
		debug_help(help_topic);
		break;
	case DEBUG_BUFFER:
		debug_buffer(game, type);
		break;
	case DEBUG_RESOURCES:
		debug_resources(game, type);
		break;
	case DEBUG_RANDOM:
		debug_random(type, arg1);
		break;
	case DEBUG_GAME:
		debug_game(game, type);
		break;
	case DEBUG_PLAYER:
	case DEBUG_OLDPLAYER:
		debug_player(game, command, type);
		break;
	case DEBUG_ROOMS:
	case DEBUG_OBJECTS:
	case DEBUG_NPCS:
	case DEBUG_EVENTS:
	case DEBUG_TASKS:
	case DEBUG_VARIABLES:
	case DEBUG_OLDROOMS:
	case DEBUG_OLDOBJECTS:
	case DEBUG_OLDNPCS:
	case DEBUG_OLDEVENTS:
	case DEBUG_OLDTASKS:
	case DEBUG_OLDVARIABLES:
		debug_dump_common(game, command, type, arg1, arg2);
		break;
	case DEBUG_WATCHPLAYER:
	case DEBUG_WATCHOBJECTS:
	case DEBUG_WATCHNPCS:
	case DEBUG_WATCHEVENTS:
	case DEBUG_WATCHTASKS:
	case DEBUG_WATCHVARIABLES:
	case DEBUG_CLEARPLAYER:
	case DEBUG_CLEAROBJECTS:
	case DEBUG_CLEARNPCS:
	case DEBUG_CLEAREVENTS:
	case DEBUG_CLEARTASKS:
	case DEBUG_CLEARVARIABLES:
		debug_watchpoint_common(game, command, type, arg1, arg2);
		break;
	case DEBUG_WATCHALL:
	case DEBUG_CLEARALL:
		debug_watchall_common(game, command, type);
		break;
	case DEBUG_NONE:
		break;
	default:
		sc_fatal("debug_dispatch: invalid debug command\n");
	}
}


/*
 * debug_dialog()
 *
 * Create a small debugging dialog with the user.
 */
static void debug_dialog(CONTEXT, sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_debuggerref_t debug = debug_get_debugger(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	assert(debug_is_valid(debug));

	/*
	 * Note elapsed seconds, so time stands still while debugging, and clear
	 * any pending game quit left over from prior dialogs (just in case).
	 */
	debug->elapsed_seconds = var_get_elapsed_seconds(vars);
	debug->quit_pending = FALSE;

	/* Handle debug commands until debugger quit or game quit. */
	while (TRUE) {
		sc_char buffer[DEBUG_BUFFER_SIZE];
		sc_command_t command, help_topic;
		sc_command_type_t type;
		sc_int arg1, arg2;

		/* Get a debugging command string from the user. */
		do {
			if_read_debug(buffer, sizeof(buffer));

			if (g_vm->shouldQuit())
				return;
		} while (sc_strempty(buffer));

		/* Parse the command read, and handle dialog exit commands. */
		command = debug_parse_command(buffer,
		                              &type, &arg1, &arg2, &help_topic);
		if (command == DEBUG_CONTINUE || command == DEBUG_STEP) {
			if (!game->is_running) {
				if_print_debug("The game is no longer running.\n");
				continue;
			}

			debug->single_step = (command == DEBUG_STEP);
			break;
		} else if (command == DEBUG_QUIT) {
			/*
			 * If the game is not running, we can't halt it, and we don't need
			 * to confirm the quit (either the player "quit" or the game
			 * completed), so leave the dialog loop.
			 */
			if (!game->is_running)
				break;

			/*
			 * The game is still running, so confirm quit by requiring a repeat,
			 * or if this is the confirmation, force the game to a halt.
			 */
			if (!debug->quit_pending) {
				if_print_debug("Use 'quit' again to confirm, or another"
				               " debugger command to cancel.\n");
				debug->quit_pending = TRUE;
				continue;
			}

			/* Drop printfilter contents and quit the game. */
			pf_empty(filter);
			CALL1(run_quit, game);

			/* Just in case... */
			if_print_debug("Unable to quit from the game.  Sorry.\n");
			continue;
		}

		/* Dispatch the remaining debugging commands, and clear quit flag. */
		debug_dispatch(game, command, type, arg1, arg2, help_topic);
		debug->quit_pending = FALSE;
	}

	/* Restart time, and clear any pending game quit. */
	var_set_elapsed_seconds(vars, debug->elapsed_seconds);
	debug->quit_pending = FALSE;
}


/*
 * debug_run_command()
 *
 * Handle a single debugging command line from the outside world.  Returns
 * TRUE if valid, FALSE if invalid (parse failed, not understood).
 */
sc_bool debug_run_command(sc_gameref_t game, const sc_char *debug_command) {
	const sc_debuggerref_t debug = debug_get_debugger(game);
	sc_command_t command, help_topic;
	sc_command_type_t type;
	sc_int arg1, arg2;

	/* If debugging disallowed (not initialized), refuse the call. */
	if (debug) {
		/*
		 * Parse the command string passed in, and return FALSE if the parse
		 * fails, or if it returns DEBUG_CONTINUE, DEBUG_STEP, or DEBUG_QUIT,
		 * none of which make any sense in this context.
		 */
		command = debug_parse_command(debug_command,
		                              &type, &arg1, &arg2, &help_topic);
		if (command == DEBUG_NONE
		        || command == DEBUG_CONTINUE || command == DEBUG_STEP
		        || command == DEBUG_QUIT)
			return FALSE;

		/* Dispatch the remaining debugging commands, return successfully. */
		debug_dispatch(game, command, type, arg1, arg2, help_topic);
		return TRUE;
	}

	return FALSE;
}


/*
 * debug_cmd_debugger()
 *
 * Called by the run main loop on user "debug" command request.  Prints
 * a polite refusal if debugging is not enabled, otherwise runs a debugging
 * dialog.  Uses if_print_string() as this isn't debug output.
 */
sc_bool debug_cmd_debugger(sc_gameref_t game) {
	const sc_debuggerref_t debug = debug_get_debugger(game);
	Context context;

	/* If debugging disallowed (not initialized), ignore the call. */
	if (debug) {
		debug_dialog(context, game);
	} else {
		if_print_string("SCARE's game debugger is not enabled.  Sorry.\n");
	}
	/*
	 * Set as administrative command, so as not to consume a game turn, and
	 * return successfully.
	 */
	game->is_admin = TRUE;
	return TRUE;
}


/*
 * debug_game_started()
 * debug_game_ended()
 *
 * The first is called on entry to the game main loop, and gives us a chance
 * to look at things before any turns are run, and to set watchpoints to
 * catch things in games that use catch-all command tasks on startup (The PK
 * Girl, for example).
 *
 * The second is called on exit from the game, and may make a final sweep for
 * watchpoints and offer the debug dialog one last time.
 */
void debug_game_started(CONTEXT, sc_gameref_t game) {
	const sc_debuggerref_t debug = debug_get_debugger(game);

	/* If debugging disallowed (not initialized), ignore the call. */
	if (debug) {
		/* Starting a new game, or a restore or undo of an old one? */
		if (!gs_room_seen(game, gs_playerroom(game))) {
			/*
			 * It's a new game starting or restarting.  Print a banner, and
			 * run the debugger dialog.
			 */
			if_print_debug("\n--- SCARE " SCARE_VERSION SCARE_PATCH_LEVEL
			               " Game Debugger\n"
			               "--- Type 'help' for a list of commands.\n");
			CALL1(debug_dialog, game);
		} else {
			/*
			 * It's a restore or undo through memos, so run the dialog only if
			 * single-stepping; no need to check watchpoints for this case as
			 * none can be set -- no undo.
			 */
			if (debug->single_step) {
				CALL1(debug_dialog, game);
			}
		}
	}
}

void debug_game_ended(CONTEXT, sc_gameref_t game) {
	const sc_debuggerref_t debug = debug_get_debugger(game);

	/* If debugging disallowed (not initialized), ignore the call. */
	if (debug) {
		/*
		 * Using our carnal knowledge of the run main loop, we know here that
		 * if the loop exited with do_restart or do_restore, we'll get a call to
		 * debug_game_start() when the loop restarts.  So in this case, ignore
		 * the call (even if single stepping).
		 */
		if (game->do_restart || game->do_restore)
			return;

		/*
		 * Check for any final watchpoints, and print a message describing why
		 * we're here.  Suppress the check for watchpoints if the user exited
		 * the game, as it'll only be a repeat of any found last turn update.
		 */
		if (!game->is_running) {
			if (game->has_completed) {
				debug_check_watchpoints(game);
				if_print_debug("\n--- The game has completed.\n");
			} else
				if_print_debug("\n--- The game has exited.\n");
		} else {
			debug_check_watchpoints(game);
			if_print_debug("\n--- The game is still running!\n");
		}

		/* Run a final dialog. */
		CALL1(debug_dialog, game);
	}
}


/*
 * debug_turn_update()
 *
 * Called after each turn by the main game loop.  Checks for any set
 * watchpoints, and triggers a debug dialog when any fire.
 */
void debug_turn_update(CONTEXT, sc_gameref_t game) {
	const sc_debuggerref_t debug = debug_get_debugger(game);

	/* If debugging disallowed (not initialized), ignore the call. */
	if (debug) {
		/*
		 * Again using carnal knowledge of the run main loop, if we're in
		 * mid-wait, ignore the call.  Also, ignore the call if the game is
		 * no longer running, as we'll see a debug_game_ended() call come
		 * along to handle that.
		 */
		if (game->waitcounter > 0 || !game->is_running)
			return;

		/*
		 * Run debugger dialog if any watchpoints triggered, or if single
		 * stepping (even if none triggered).
		 */
		if (debug_check_watchpoints(game) || debug->single_step) {
			CALL1(debug_dialog, game);
		}
	}
}


/*
 * debug_set_enabled()
 * debug_get_enabled()
 *
 * Enable/disable debugging, and return debugging status.  Debugging is
 * enabled when there is a debugger reference in the game, and disabled
 * when it's NULL -- that's the flag.  To avoid lugging about all the
 * watchpoint memory with a game, debugger data is allocated on enabling,
 * and free'd on disabling; as a result, any set watchpoints are lost on
 * disabling.
 */
void debug_set_enabled(sc_gameref_t game, sc_bool enable) {
	const sc_debuggerref_t debug = debug_get_debugger(game);

	/*
	 * If enabling and not already enabled, or disabling and not already
	 * disabled, either initialize or finalize..
	 */
	if ((enable && !debug) || (!enable && debug)) {
		/* Initialize or finalize debugging, as appropriate. */
		if (enable)
			debug_initialize(game);
		else
			debug_finalize(game);
	}
}

sc_bool debug_get_enabled(sc_gameref_t game) {
	const sc_debuggerref_t debug = debug_get_debugger(game);

	return debug != NULL;
}

} // End of namespace Adrift
} // End of namespace Glk
