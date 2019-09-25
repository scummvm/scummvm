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

/* Assorted definitions and constants. */
enum { LINE_BUFFER_SIZE = 256 };
static const sc_char NUL = '\0';
static const sc_char SPECIAL_PATTERN = '#';
static const sc_char WILDCARD_PATTERN = '*';
static const sc_char *const WHITESPACE = "\t\n\v\f\r ";
static const sc_char *const SEPARATORS = ".,";


/*
 * run_is_task_function()
 *
 * Check for the presence of a command function in the first task command,
 * and action it if found.  This is a 4.0.42 compatibility hack -- at
 * present, only getdynfromroom() exists.  Returns TRUE if function found
 * and handled.
 */
static sc_bool run_is_task_function(const sc_char *pattern, sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_vartype_t vt_key[3];
	sc_int room, object;
	sc_char *argument;

	/* Simple comparison against the one known task expression. */
	argument = (sc_char *)sc_malloc(strlen(pattern) + 1);
	if (sscanf(pattern, " # %%object%% = getdynfromroom (%[^)])", argument) == 0) {
		sc_free(argument);
		return FALSE;
	}

	/*
	 * Compare the argument read in against known room names.
	 *
	 * TODO Is this simple room name comparison good enough?
	 */
	vt_key[0].string = "Rooms";
	for (room = 0; room < gs_room_count(game); room++) {
		const sc_char *name;

		vt_key[1].integer = room;
		vt_key[2].string = "Short";
		name = prop_get_string(bundle, "S<-sis", vt_key);
		if (sc_strcasecmp(name, argument) == 0)
			break;
	}
	sc_free(argument);
	if (room == gs_room_count(game))
		return FALSE;

	/*
	 * Select a dynamic object from the room.
	 *
	 * TODO What are the selection criteria supposed to be?  Here we use "on
	 * the floor".
	 */
	vt_key[0].string = "Objects";
	for (object = 0; object < gs_object_count(game); object++) {
		sc_bool bstatic;

		vt_key[1].integer = object;
		vt_key[2].string = "Static";
		bstatic = prop_get_boolean(bundle, "B<-sis", vt_key);
		if (!bstatic && obj_directly_in_room(game, object, room))
			break;
	}
	if (object == gs_object_count(game))
		return FALSE;

	/* Set this object reference, unambiguously, as if %object% match. */
	gs_clear_object_references(game);
	game->object_references[object] = TRUE;
	var_set_ref_object(vars, object);

	return TRUE;
}


/* Structure used to associate a pattern with a handler function. */
struct sc_commands_s {
	const sc_char *const command;
	sc_bool(*const handler)(sc_gameref_t game);
};
typedef sc_commands_s sc_commands_t;
typedef sc_commands_t *sc_commandsref_t;

/* Movement commands for the four point compass. */
static sc_commands_t MOVE_COMMANDS_4[] = {
	{"{go {to {the}}} [north/n]", lib_cmd_go_north},
	{"{go {to {the}}} [east/e]", lib_cmd_go_east},
	{"{go {to {the}}} [south/s]", lib_cmd_go_south},
	{"{go {to {the}}} [west/w]", lib_cmd_go_west},
	{"{go {to {the}}} [up/u]", lib_cmd_go_up},
	{"{go {to {the}}} [down/d]", lib_cmd_go_down},
	{"{go {to {the}}} [in]", lib_cmd_go_in},
	{"{go {to {the}}} [out/o]", lib_cmd_go_out},
	{NULL, NULL}
};

/* Movement commands for the eight point compass. */
static sc_commands_t MOVE_COMMANDS_8[] = {
	{"{go {to {the}}} [north/n]", lib_cmd_go_north},
	{"{go {to {the}}} [east/e]", lib_cmd_go_east},
	{"{go {to {the}}} [south/s]", lib_cmd_go_south},
	{"{go {to {the}}} [west/w]", lib_cmd_go_west},
	{"{go {to {the}}} [up/u]", lib_cmd_go_up},
	{"{go {to {the}}} [down/d]", lib_cmd_go_down},
	{"{go {to {the}}} [in]", lib_cmd_go_in},
	{"{go {to {the}}} [out/o]", lib_cmd_go_out},
	{"{go {to {the}}} [northeast/north-east/ne]", lib_cmd_go_northeast},
	{"{go {to {the}}} [southeast/south-east/se]", lib_cmd_go_southeast},
	{"{go {to {the}}} [northwest/north-west/nw]", lib_cmd_go_northwest},
	{"{go {to {the}}} [southwest/south-west/sw]", lib_cmd_go_southwest},
	{NULL, NULL}
};

/* "Priority" library commands, may take precedence over the game. */
static sc_commands_t PRIORITY_COMMANDS[] = {

	/* Acquisition of and disposal of inventory. */
	{
		"[[get/take/remove/extract] [all/everything] from/empty] %object%",
		lib_cmd_take_all_from
	},
	{
		"[[get/take/remove/extract] [all/everything] from/empty] %object%"
		" [[except/but] {for}/apart from] %text%",
		lib_cmd_take_from_except_multiple
	},
	{
		"[get/take/remove/extract] [all/everything]"
		" [[except/but] {for}/apart from] %text% from %object%",
		lib_cmd_take_from_except_multiple
	},
	{
		"[get/take/remove/extract] %text% from %object%",
		lib_cmd_take_from_multiple
	},
	{"[get/take] [all/everything] from %character%", lib_cmd_take_all_from_npc},
	{
		"[get/take] [all/everything] from %character%"
		" [[except/but] {for}/apart from] %text%",
		lib_cmd_take_from_npc_except_multiple
	},
	{
		"[get/take] [all/everything]"
		" [[except/but] {for}/apart from] %text% from %character%",
		lib_cmd_take_from_npc_except_multiple
	},
	{"[get/take] %text% from %character%", lib_cmd_take_from_npc_multiple},
	{
		"[[get/take/pick up] [all/everything]/pick [all/everything] up]",
		lib_cmd_take_all
	},
	{
		"[get/take/pick up] [all/everything] [[except/but] {for}/apart from] %text%",
		lib_cmd_take_except_multiple
	},
	{"[get/take/pick up] %text%", lib_cmd_take_multiple},
	{"pick %text% up", lib_cmd_take_multiple},
	{
		"[[drop/put down] [all/everything]/put [all/everything] down]",
		lib_cmd_drop_all
	},
	{
		"[drop/put down] [all/everything] [[except/but] {for}/apart from] %text%",
		lib_cmd_drop_except_multiple
	},
	{"[drop/put down] %text%", lib_cmd_drop_multiple},
	{"put %text% down", lib_cmd_drop_multiple},
	{NULL, NULL}
};

/* Standard library commands, other than movement and priority above. */
static sc_commands_t STANDARD_COMMANDS[] = {

	/* Inventory, and general investigation of surroundings. */
	{"[inventory/inv/i]", lib_cmd_inventory},
	{"[x/ex/exam/examine/l/look {at}] {{the} [room/location]}", lib_cmd_look},
	{"[x/ex/exam/examine/look {at/in}] %object%", lib_cmd_examine_object},
	{"[x/ex/exam/examine/look {at}] %character%", lib_cmd_examine_npc},
	{"[x/ex/exam/examine/look {at}] [me/self/myself]", lib_cmd_examine_self},
	{"[x/ex/exam/examine/look {at}] all", lib_cmd_examine_all},

	/* Attempted acquisition of and disposal of NPCs. */
	{"[get/take/pick up] %character%", lib_cmd_take_npc},
	{"pick %character% up", lib_cmd_take_npc},

	/* Manipulating selected objects. */
	{"put [all/everything] [in/into/inside {of}] %object%", lib_cmd_put_all_in},
	{
		"put [all/everything] [[except/but] {for}/apart from] %text%"
		" [in/into/inside {of}] %object%", lib_cmd_put_in_except_multiple
	},
	{"put %text% [in/into/inside {of}] %object%", lib_cmd_put_in_multiple},
	{"put [all/everything] [on/onto/on top of] %object%", lib_cmd_put_all_on},
	{
		"put [all/everything] [[except/but] {for}/apart from] %text%"
		" [on/onto/on top of] %object%", lib_cmd_put_on_except_multiple
	},
	{"put %text% [on/onto/on top of] %object%", lib_cmd_put_on_multiple},
	{"open %object%", lib_cmd_open_object},
	{"close %object%", lib_cmd_close_object},
	{"unlock %object% with %text%", lib_cmd_unlock_object_with},
	{"lock %object% with %text%", lib_cmd_lock_object_with},
	{"unlock %object%", lib_cmd_unlock_object},
	{"lock %object%", lib_cmd_lock_object},
	{"read %object%", lib_cmd_read_object},
	{"read *", lib_cmd_read_other},
	{"give %object% to %character%", lib_cmd_give_object_npc},
	{"sit {down/up} [on/in] %object%", lib_cmd_sit_on_object},
	{"stand {up/down} [on/in] %object%", lib_cmd_stand_on_object},
	{"[lie/lay] on %object%", lib_cmd_lie_on_object},
	{"get {down/up} off %object%", lib_cmd_get_off_object},
	{"get off", lib_cmd_get_off},
	{"sit {down/up} {[on/in] {the} [ground/floor]}", lib_cmd_sit_on_floor},
	{"stand {up/down} {[on/in] {the} [ground/floor]}", lib_cmd_stand_on_floor},
	{"[lie/lay] {down/up} {[on/in] {the} [ground/floor]}", lib_cmd_lie_on_floor},
	{"eat %object%", lib_cmd_eat_object},

	/* Dressing up, and dressing down. */
	{
		"[[wear/put on/don] [all/everything]/put [all/everything] on]",
		lib_cmd_wear_all
	},
	{
		"[wear/put on/don] [all/everything] [[except/but] {for}/apart from] %text%",
		lib_cmd_wear_except_multiple
	},
	{"[wear/put on/don] %text%", lib_cmd_wear_multiple},
	{"put %text% on", lib_cmd_wear_multiple},
	{
		"[[remove/take off/doff] [all/everything]/take [all/everything] off/strip]",
		lib_cmd_remove_all
	},
	{
		"[remove/take off/doff] [all/everything]"
		" [[except/but] {for}/apart from] %text%",
		lib_cmd_remove_except_multiple
	},
	{"[remove/take off/doff] %text%", lib_cmd_remove_multiple},
	{"take %text% off", lib_cmd_remove_multiple},

	/* Selected NPC interactions and conversation. */
	{"ask %character% about %text%", lib_cmd_ask_npc_about},
	{
		"[attack/hit/kick/slap/shoot/stab] %character% with %object%",
		lib_cmd_attack_npc_with
	},
	{"[attack/shoot] %character%", lib_cmd_attack_npc},

	/* More movement, waiting, and miscellaneous administrative commands. */
	{"[goto/go {to}] %text%", lib_cmd_go_room},
	{"[goto/go {to}] *", lib_cmd_print_room_exits},
	{"[exit/exits/directions/where]", lib_cmd_print_room_exits},
	{"[wait/z] %number%", lib_cmd_wait_number},
	{"[wait/z]", lib_cmd_wait},
	{"save", lib_cmd_save},
	{"[restore/load]", lib_cmd_restore},
	{"restart", lib_cmd_restart},
	{"[again/g]", lib_cmd_again},
	{"[redo /!]%number%", lib_cmd_redo_number},
	{"[redo /!]%text%", lib_cmd_redo_text},
	{"[redo/!]", lib_cmd_redo_last},
	{"[quit/q]", lib_cmd_quit},
	{"turns", lib_cmd_turns},
	{"score", lib_cmd_score},
	{"undo", lib_cmd_undo},
	{"[hist/history] %number%", lib_cmd_history_number},
	{"[hist/history]", lib_cmd_history},
	{"[hint/hints]", lib_cmd_hints},
	{"verbose", lib_cmd_verbose},
	{"brief", lib_cmd_brief},
	{"[notify/notification] %text%", lib_cmd_notify_on_off},
	{"[notify/notification]", lib_cmd_notify},
	{"time", lib_cmd_time},
	{"date", lib_cmd_date},
	{"[help/commands]", lib_cmd_help},
	{"[gpl/license]", lib_cmd_license},
	{"[about/info/information/author]", lib_cmd_information},
	{"[clear/cls/clr]", lib_cmd_clear},
	{"status{line}", lib_cmd_statusline},
	{"version", lib_cmd_version},

	{"[locate/where {is/are}/find] %object%", lib_cmd_locate_object},
	{"[locate/where {is}/find] %character%", lib_cmd_locate_npc},

	{"[count/num]", lib_cmd_count},

	/* Standard response commands; no real action, just output. */
	{"[get/take/pick up] *", lib_cmd_get_what},
	{"open *", lib_cmd_open_what},
	{"close *", lib_cmd_close_other},
	{"give %object% *", lib_cmd_give_object},
	{"give *", lib_cmd_give_what},
	{"lock %text%", lib_cmd_lock_other},
	{"lock", lib_cmd_lock_what},
	{"unlock %text%", lib_cmd_unlock_other},
	{"unlock", lib_cmd_unlock_what},
	{"sit {down/up} [on/in] *", lib_cmd_sit_other},
	{"stand {up/down} [on/in] *", lib_cmd_stand_other},
	{"[lie/lay] {down/up} [on/in] *", lib_cmd_lie_other},
	{"[remove/take off/doff] *", lib_cmd_remove_what},
	{"[drop/put down] *", lib_cmd_drop_what},
	{"[wear/put on/don] *", lib_cmd_wear_what},
	{
		"[shit/fuck/bastard/cunt/crap/hell/shag/bollocks/bollox/bugger] *",
		lib_cmd_profanity
	},
	{"[x/examine/look {at}] *", lib_cmd_examine_other},
	{"[locate/where {is/are}/find] *", lib_cmd_locate_other},
	{"[cp/mv/ln/ls] *", lib_cmd_unix_like},
	{"dir *", lib_cmd_dos_like},
	{"ask %character% *", lib_cmd_ask_npc},
	{"ask %object% *", lib_cmd_ask_object},
	{"ask *", lib_cmd_ask_other},
	{"block %object% *", lib_cmd_block_object},
	{"block %text%", lib_cmd_block_other},
	{"block", lib_cmd_block_what},
	{"[break/destroy/smash] %object% *", lib_cmd_break_object},
	{"[break/destroy/smash] %text%", lib_cmd_break_other},
	{"break", lib_cmd_break_what},
	{"destroy", lib_cmd_destroy_what},
	{"smash", lib_cmd_smash_what},
	{"buy %object% *", lib_cmd_buy_object},
	{"buy %text%", lib_cmd_buy_other},
	{"buy", lib_cmd_buy_what},
	{"clean %object% *", lib_cmd_clean_object},
	{"clean %text%", lib_cmd_clean_other},
	{"clean", lib_cmd_clean_what},
	{"climb %object% *", lib_cmd_climb_object},
	{"climb %text%", lib_cmd_climb_other},
	{"climb", lib_cmd_climb_what},
	{"cry *", lib_cmd_cry},
	{"cut %object% *", lib_cmd_cut_object},
	{"cut %text%", lib_cmd_cut_other},
	{"cut", lib_cmd_cut_what},
	{"dance *", lib_cmd_dance},
	{"drink %object% *", lib_cmd_drink_object},
	{"drink %text%", lib_cmd_drink_other},
	{"drink", lib_cmd_drink_what},
	{"eat *", lib_cmd_eat_other},
	{"feed *", lib_cmd_feed},
	{"feel *", lib_cmd_feel},
	{"fight *", lib_cmd_fight},
	{"fix %object% *", lib_cmd_fix_object},
	{"fix %text%", lib_cmd_fix_other},
	{"fix", lib_cmd_fix_what},
	{"fly *", lib_cmd_fly},
	{"hint *", lib_cmd_hint},
	{"hit %character%", lib_cmd_attack_npc},
	{"hit %object% *", lib_cmd_hit_object},
	{"hit %text%", lib_cmd_hit_other},
	{"hit", lib_cmd_hit_what},
	{"hum *", lib_cmd_hum},
	{"jump *", lib_cmd_jump},
	{"kick %character%", lib_cmd_attack_npc},
	{"kick %object% *", lib_cmd_kick_object},
	{"kick %text%", lib_cmd_kick_other},
	{"kick", lib_cmd_kick_what},
	{"kiss %character% *", lib_cmd_kiss_npc},
	{"kiss %object% *", lib_cmd_kiss_object},
	{"kiss *", lib_cmd_kiss_other},
	{"kill *", lib_cmd_kill_other},
	{"lift %object% *", lib_cmd_lift_object},
	{"lift %text%", lib_cmd_lift_other},
	{"lift", lib_cmd_lift_what},
	{"light %object% *", lib_cmd_light_object},
	{"light %text%", lib_cmd_light_other},
	{"light", lib_cmd_light_what},
	{"listen *", lib_cmd_listen},
	{"mend %object% *", lib_cmd_mend_object},
	{"mend %text%", lib_cmd_mend_other},
	{"mend", lib_cmd_mend_what},
	{"move %object% *", lib_cmd_move_object},
	{"move %text%", lib_cmd_move_other},
	{"move", lib_cmd_move_what},
	{"please *", lib_cmd_please},
	{"press %object% *", lib_cmd_press_object},
	{"press %text%", lib_cmd_press_other},
	{"press", lib_cmd_press_what},
	{"pull %object% *", lib_cmd_pull_object},
	{"pull %text%", lib_cmd_pull_other},
	{"pull", lib_cmd_pull_what},
	{"punch *", lib_cmd_punch},
	{"push %object% *", lib_cmd_push_object},
	{"push %text%", lib_cmd_push_other},
	{"push", lib_cmd_push_what},
	{"repair %object% *", lib_cmd_repair_object},
	{"repair %text%", lib_cmd_repair_other},
	{"repair", lib_cmd_repair_what},
	{"rub %object% *", lib_cmd_rub_object},
	{"rub %text%", lib_cmd_rub_other},
	{"rub", lib_cmd_rub_what},
	{"run *", lib_cmd_run},
	{"say *", lib_cmd_say},
	{"sell %object% *", lib_cmd_sell_object},
	{"sell %text%", lib_cmd_sell_other},
	{"sell", lib_cmd_sell_what},
	{"shake %object% *", lib_cmd_shake_object},
	{"shake %text%", lib_cmd_shake_other},
	{"shake", lib_cmd_shake_what},
	{"shout *", lib_cmd_shout},
	{"sing *", lib_cmd_sing},
	{"sleep *", lib_cmd_sleep},
	{"smell %object% *", lib_cmd_smell_object},
	{"smell *", lib_cmd_smell_other},
	{"stop %object% *", lib_cmd_stop_object},
	{"stop %text%", lib_cmd_stop_other},
	{"stop", lib_cmd_stop_what},
	{"suck %object% *", lib_cmd_suck_object},
	{"suck %text%", lib_cmd_suck_other},
	{"suck", lib_cmd_suck_what},
	{"talk *", lib_cmd_talk},
	{"thank *", lib_cmd_thank},
	{"turn %object% *", lib_cmd_turn_object},
	{"turn %text%", lib_cmd_turn_other},
	{"turn", lib_cmd_turn_what},
	{"touch %object% *", lib_cmd_touch_object},
	{"touch %text%", lib_cmd_touch_other},
	{"touch", lib_cmd_touch_what},
	{"unblock %object% *", lib_cmd_unblock_object},
	{"unblock %text%", lib_cmd_unblock_other},
	{"unblock", lib_cmd_unblock_what},
	{"wash %object% *", lib_cmd_wash_object},
	{"wash %text%", lib_cmd_wash_other},
	{"wash", lib_cmd_wash_what},
	{"whistle *", lib_cmd_whistle},
	{"[why/when/what/can/how] *", lib_cmd_interrogation},
	{"xyzzy *", lib_cmd_xyzzy},
	{"campbell", lib_cmd_egotistic},
	{"[yes/no] *", lib_cmd_yes_or_no},
	{"* %object% *", lib_cmd_verb_object},
	{"* %character% *", lib_cmd_verb_npc},

	/* SCARE debugger hook command, placed last just in case... */
	{"{#}debug{ger}", debug_cmd_debugger},

	{NULL, NULL}
};


/*
 * run_priority_commands()
 * run_standard_commands()
 *
 * Compare a user input string against commands recognized by the library,
 * and action any command.  Returns TRUE if the string matched a command
 * that then ran successfully, FALSE otherwise.
 *
 * "Priority" commands are ones that Adrift seems to action no matter what
 * the game tries to override.  For example, a simple game with one "ball"
 * object and a task "* ball *" should, if the task is restricted, override
 * "take ball" such that the ball can never be acquired.  Adrift lets the
 * "take" succeed, though (and more curiously, may respond "I don't
 * understand..." to "drop ball").  This could be an Adrift bug.  Shrug.
 *
 * For now, I can't find any better way to try to handle it than to make
 * object acquisition take precedence over game commands.
 */
static sc_bool run_priority_commands(sc_gameref_t game, const sc_char *string) {
	sc_commandsref_t command;

	for (command = PRIORITY_COMMANDS; command->command; command++) {
		if (uip_match(command->command, string, game)) {
			if (command->handler(game))
				return TRUE;
		}
	}

	/* Nothing matched match the string.  Or if it did, its handler failed. */
	return FALSE;
}

static sc_bool run_standard_commands(sc_gameref_t game, const sc_char *string) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[2];
	sc_bool eightpointcompass;
	sc_commandsref_t command;

	/* Select the appropriate movement commands. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "EightPointCompass";
	eightpointcompass = prop_get_boolean(bundle, "B<-ss", vt_key);
	command = eightpointcompass ? MOVE_COMMANDS_8 : MOVE_COMMANDS_4;

	/*
	 * Search movement commands first, returning TRUE if any matching command
	 * handler succeeded.  Then repeat for standard library commands.
	 */
	for (; command->command; command++) {
		if (uip_match(command->command, string, game)) {
			if (command->handler(game))
				return TRUE;
		}
	}

	for (command = STANDARD_COMMANDS; command->command; command++) {
		if (uip_match(command->command, string, game)) {
			if (command->handler(game))
				return TRUE;
		}
	}

	/* Nothing matched match the string.  Or if it did, its handler failed. */
	return FALSE;
}


/*
 * run_update_status()
 *
 * Update the game's current room and status line strings.
 */
static void run_update_status(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_vartype_t vt_key[2];
	const sc_char *name, *status;
	sc_char *filtered;
	sc_bool statusbox;

	/* Get the current room name, and filter and untag it. */
	name = lib_get_room_name(game, gs_playerroom(game));
	filtered = pf_filter(name, vars, bundle);
	pf_strip_tags(filtered);

	/* Free any existing room name, then save this room name. */
	sc_free(game->current_room_name);
	game->current_room_name = filtered;

	/* See if the game does a status box. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "StatusBox";
	statusbox = prop_get_boolean(bundle, "B<-ss", vt_key);
	if (statusbox) {
		/* Get the status line, and filter and untag it. */
		vt_key[1].string = "StatusBoxText";
		status = prop_get_string(bundle, "S<-ss", vt_key);
		filtered = pf_filter(status, vars, bundle);
		pf_strip_tags(filtered);
	} else
		/* No status line, so use NULL. */
		filtered = NULL;

	/* Free any existing status line, then save this status text. */
	sc_free(game->status_line);
	game->status_line = filtered;
}


/*
 * run_notify_score_change()
 *
 * Print an indication of any score change, if appropriate.  The change is
 * detected by comparing against the undo game.  Uses if_print_string()
 * directly for printing, rather than the filter, so that it can place its
 * output ahead of buffered printfilter text.
 */
static void run_notify_score_change(sc_gameref_t game) {
	const sc_gameref_t undo = game->undo;
	sc_char buffer[32];
	assert(gs_is_game_valid(undo));

	/*
	 * Do nothing if no undo available, or if notification is off, or if we've
	 * already done this once this turn.
	 */
	if (!game->undo_available
	        || !game->notify_score_change || game->has_notified)
		return;

	/* Note any change in the score. */
	if (game->score > undo->score) {
		if_print_string("(Your score has increased by ");
		sprintf(buffer, "%ld", game->score - undo->score);
		if_print_string(buffer);
		if_print_string(")\n");
	} else if (game->score < undo->score) {
		if_print_string("(Your score has decreased by ");
		sprintf(buffer, "%ld", undo->score - game->score);
		if_print_string(buffer);
		if_print_string(")\n");
	}
	game->has_notified = TRUE;
}


/*
 * run_match_task_common()
 * run_match_task_commands()
 * run_match_task_functions()
 *
 * Helpers for run_game_commands_common().
 *
 * Search task command for a match to the string passed in, returning TRUE
 * if a task command matches, FALSE otherwise.  Ordinary or reverse commands
 * are selected by 'forwards'.
 */
static sc_bool run_match_task_common(sc_gameref_t game, sc_int task, const sc_char *string,
		sc_bool forwards, sc_bool is_library, sc_bool is_normal) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_int command_count, command;
	sc_bool is_matched;

	/* Get the count of task commands. */
	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = forwards ? "Command" : "ReverseCommand";
	command_count = prop_get_child_count(bundle, "I<-sis", vt_key);

	/* Iterate over commands, looking for patterns that match string. */
	is_matched = FALSE;
	for (command = 0; command < command_count; command++) {
		const sc_char *pattern;
		sc_int first;

		/* Retrieve the pattern for this command, find its first character. */
		vt_key[3].integer = command;
		pattern = prop_get_string(bundle, "S<-sisi", vt_key);
		first = strspn(pattern, WHITESPACE);

		/* Match using either the parser, or the special function matcher. */
		if (is_normal) {
			if (pattern[first] != SPECIAL_PATTERN) {
				/*
				 * Make a special case of library calls and commands that begin
				 * with a wildcard; these we ignore for this match attempt.
				 */
				if (is_library && pattern[first] == WILDCARD_PATTERN)
					is_matched = FALSE;
				else
					is_matched = uip_match(pattern, string, game);
			}
		} else {
			if (pattern[first] == SPECIAL_PATTERN)
				is_matched = run_is_task_function(pattern, game);
		}

		/* Stop searching if we find a match. */
		if (is_matched)
			break;
	}

	/* Return TRUE if we found a pattern match. */
	return is_matched;
}

static sc_bool run_match_task_commands(sc_gameref_t game, sc_int task,
		const sc_char *string, sc_bool forwards, sc_bool is_library) {
	/*
	 * Match tasks using the normal pattern matcher, with or without any note
	 * about whether the call is from the library.
	 */
	return run_match_task_common(game, task, string, forwards, is_library, TRUE);
}

static sc_bool run_match_task_functions(sc_gameref_t game, sc_int task,
		const sc_char *string, sc_bool forwards) {
	/* Match tasks against "task command functions". */
	return run_match_task_common(game, task, string, forwards, FALSE, FALSE);
}


/*
 * run_task_is_unrestricted()
 * run_task_is_loudly_restricted()
 *
 * Helpers for run_game_commands_common().
 *
 * Adapters for uncovering task restriction state.  The first returns TRUE
 * if the task is unrestricted, and can therefore run unimpeded.  The second
 * returns TRUE iff the task is restricted and has a fail message that
 * indicates why it fails; such tasks, if run, produce their failure message
 * and don't change state.
 */
static sc_bool run_task_is_unrestricted(sc_gameref_t game, sc_int task) {
	sc_bool restrictions_passed;
	const sc_char *fail_message;

	/*
	 * Evaluate task restrictions, and if they fail to parse for some reason,
	 * return as if restrictions did not pass.
	 */
	if (!restr_eval_task_restrictions(game, task,
	                                  &restrictions_passed, &fail_message)) {
		sc_error("run_task_is_unrestricted: restrictions error, %ld\n", task);
		return FALSE;
	}

	/* Return TRUE if the task is unrestricted. */
	return restrictions_passed;
}

static sc_bool run_task_is_loudly_restricted(sc_gameref_t game, sc_int task) {
	sc_bool restrictions_passed;
	const sc_char *fail_message;

	/*
	 * Evaluate task restrictions, and if they fail to parse for some reason,
	 * return as if restrictions did not pass.
	 */
	if (!restr_eval_task_restrictions(game, task,
	                                  &restrictions_passed, &fail_message)) {
		sc_error("run_task_is_loudly_restricted:"
		         " restrictions error, %ld\n", task);
		return TRUE;
	}

	/* Return TRUE if the task is restricted and indicates why. */
	return !restrictions_passed && (fail_message != NULL);
}


/*
 * run_game_commands_common()
 * run_game_commands_in_parser_context()
 * run_game_commands_in_library_context()
 *
 * The central handler for running, or at least trying to run, game-defined
 * tasks that have commands that match the input string.  Here's the algorithm
 * as currently understood (and it may not be right, so be warned):
 *
 *  for each task executable in the current room
 *    for direction in forwards, backwards
 *      for each command string defined by the task for this direction
 *        match against player input
 *      if any command string matched player input
 *        if task restrictions pass
 *          run the task actions in the current direction
 *          if the task actions produced output
 *            return
 *          is_matched := true
 *          break out of all loops
 *
 *  if not is_matched and we're allowing restrictions to fail tasks
 *    for each task executable in the current room
 *      for direction in forwards, backwards
 *        for each command string defined by the task for this direction
 *          match against player input
 *        if any command string matched player input
 *          if task restrictions fail with an error message
 *            run the task, to persuade it to print this error message
 *            return
 *
 * Part of the fun and games is that run_game_task_commands() is called by the
 * library to try to run "get " and "drop " game commands for standard get/drop
 * handlers and get_all/drop_all handlers.  No pressure, then.
 */
static sc_bool run_game_commands_common(sc_gameref_t game, const sc_char *string,
		sc_bool include_restrictions, sc_bool is_library) {
	sc_bool is_matched = FALSE, is_handled = FALSE;
	sc_bool *is_matching;
	sc_int task_count, task, direction;

	/*
	 * Matching is expensive, so it helps to use a cache of results from the
	 * first loop in the second.  If we're using the second, that is.
	 */
	task_count = gs_task_count(game);
	if (include_restrictions) {
		is_matching = (sc_bool *)sc_malloc(task_count * sizeof(*is_matching));
		memset(is_matching, FALSE, task_count * sizeof(*is_matching));
	} else
		is_matching = NULL;

	/*
	 * Iterate over every task, ignoring those not runnable.  For each runnable
	 * task, try matching task commands, and on matches, check restrictions and
	 * if they pass, try running the task.
	 */
	for (task = 0; task < task_count; task++) {
		if (!task_can_run_task(game, task))
			continue;

		/*
		 * Try matching forwards and reverse commands.  If there's a match for
		 * unrestricted tasks, run the task, and if it runs (defined as printing
		 * some game output), we're done; otherwise, note the command match but
		 * keep searching for other possible matches.
		 */
		for (direction = 0; direction < 2; direction++) {
			const sc_bool is_forwards = !direction;

			if (task_can_run_task_directional(game, task, is_forwards)
			        && run_match_task_commands(game, task, string,
			                                   is_forwards, is_library)) {
				if (run_task_is_unrestricted(game, task)) {
					if (task_run_task(game, task, is_forwards))
						is_handled = TRUE;
					is_matched = TRUE;
					break;
				}

				if (is_matching)
					is_matching[task] = TRUE;
			}
		}
		if (is_matched)
			break;
	}

	/*
	 * If no match, and we've been asked to consider failing restrictions, look
	 * through all of the runnable tasks again, this time searching for
	 * restricted ones with a fail message.  Use the cache built above to weed
	 * out matches that are certain to fail.
	 */
	if (!is_handled && !is_matched && include_restrictions) {
		for (task = 0; task < task_count; task++) {
			if (!is_matching[task] || !task_can_run_task(game, task))
				continue;

			/*
			 * Check matches of forwards and reverse commands.  If there's a
			 * match for restricted tasks (ones that have and will print a fail
			 * message if we try to run them), run the task to get the print of
			 * the fail message, and we're done.
			 */
			for (direction = 0; direction < 2; direction++) {
				const sc_bool is_forwards = !direction;

				if (task_can_run_task_directional(game, task, is_forwards)
				        && run_match_task_commands(game, task, string,
				                                   is_forwards, is_library)) {
					if (run_task_is_loudly_restricted(game, task)) {
						if (task_run_task(game, task, is_forwards)) {
							is_handled = TRUE;
							break;
						}
					}
				}
			}
			if (is_handled)
				break;
		}
	}

	/* Return TRUE if any game task handled the command in some way. */
	sc_free(is_matching);
	return is_handled;
}

static sc_bool run_game_commands_in_parser_context(sc_gameref_t game,
		const sc_char *string, sc_bool include_restrictions) {
	/*
	 * Try game commands, either with or without restrictions, and all full and
	 * complete parse matching (no special case for game commands that begin
	 * with a '*' wildcard).
	 */
	return run_game_commands_common(game, string, include_restrictions, FALSE);
}

static sc_bool run_game_commands_in_library_context(sc_gameref_t game, const sc_char *string) {
	/*
	 * Try game commands, including restrictions, and noting that this is a
	 * library call so that the parse matcher can exclude game commands that
	 * begin with a '*' wildcard.
	 */
	return run_game_commands_common(game, string, TRUE, TRUE);
}


/*
 * run_game_functions()
 *
 * Iterate over every task, ignoring those not runnable, searching just for
 * "task command functions".  These seem to happen in addition to any regular
 * command matches, so we try them as a separate action.
 */
static void run_game_functions(sc_gameref_t game, const sc_char *string) {
	sc_int task_count, task, direction;

	/* Iterate over every task, ignoring those not runnable. */
	task_count = gs_task_count(game);
	for (task = 0; task < task_count; task++) {
		if (!task_can_run_task(game, task))
			continue;

		/*
		 * Try matching forwards and reverse commands.  I don't know if it's
		 * valid to put a function in a reverse command, but nevertheless...
		 */
		for (direction = 0; direction < 2; direction++) {
			const sc_bool is_forwards = !direction;

			if (task_can_run_task_directional(game, task, is_forwards)
			        && run_match_task_functions(game, task, string, is_forwards)) {
				if (run_task_is_unrestricted(game, task))
					task_run_task(game, task, is_forwards);
			}
		}
	}
}


/*
 * run_all_commands()
 * run_game_task_commands()
 *
 * Alternative facets of run_commands_common().  The first is used by the
 * main user input handling loop; the latter by the library when looking for
 * game commands that override standard actions.
 */
static sc_bool run_all_commands(sc_gameref_t game, const sc_char *string) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_bool status;

	/*
	 * Adrift command matching is just weird, perhaps broken.  In theory, a
	 * game can override system commands with a properly constructed task and
	 * set of command matchers.  However, the Runner isn't terribly consistent
	 * in when this will work and when not, and some games rely on that in-
	 * consistency.  In particular, a game with a "* object" task that has
	 * failing restrictions will not be able to override the system's "take
	 * object", whereas a game's "take object", under the same circumstances,
	 * will.  Yet if the restrictions pass, a game's "* object" overrides the
	 * system's "take object" with no apparent difficulty.
	 *
	 * For example, "The Woods Are Dark" has a "* ball *" task with the
	 * restriction "must be holding ball".  Without special casing it, there's
	 * no way to get the ball in the first place.
	 *
	 * Trying to find the right way to do things here, then, has been tricky.
	 * Here's the current process:  First, run game commands, ignoring any
	 * cases where restrictions fail to let the task run.  Next, try "priority"
	 * system commands; ones that move objects to inventory.  These system
	 * commands will call back into trying game commands for objects taken or
	 * dropped, and in those tries, allow overrides only if the game task is
	 * explicit about what it's doing (that is, doesn't start with "*"), and
	 * handle restrictions in those tries.  After that, retry all game commands
	 * again with restrictions enabled.  And finally, try all other standard
	 * library commands.
	 *
	 * TODO This is the fourth or fifth attempt at getting this to match the
	 * Runner, which is surprisingly inconsistent in this area.  What on earth
	 * is the real behavior supposed to be?
	 */
	status = run_game_commands_in_parser_context(game, string, FALSE);
	if (!status)
		status = run_priority_commands(game, string);
	if (!status)
		status = run_game_commands_in_parser_context(game, string, TRUE);
	if (!status)
		status = run_standard_commands(game, string);

	/*
	 * For version 4.0 games, it seems that if any command succeeded, we need
	 * need to scan for and run any matching "task command functions", in
	 * addition to anything done above.
	 */
	if (status && !game->is_admin) {
		sc_vartype_t vt_key;
		sc_int version;

		/* Check "task command functions" for version 4.0 only. */
		vt_key.string = "Version";
		version = prop_get_integer(bundle, "I<-s", &vt_key);
		if (version == TAF_VERSION_400)
			run_game_functions(game, string);
	}

	return status;
}

sc_bool run_game_task_commands(sc_gameref_t game, const sc_char *string) {
	return run_game_commands_in_library_context(game, string);
}


/*
 * run_player_input()
 *
 * Take a line of player input and buffer it.  Split the line into elements
 * separated by periods.  For the first element, try to match it to either a
 * task or a standard command, and return TRUE if it matched, FALSE otherwise.
 *
 * On subsequent calls, successively work with the next line element until
 * none remain.  In this case, prompt for more player input and continue as
 * above.
 *
 * For the case of "again" or "g", rerun the last successful command element.
 *
 * One extra special special case; if called with a game that is not running,
 * this is a signal to reset all noted line input to initial conditions, and
 * just return.  Sorry about the ugliness.
 */
static sc_bool run_player_input(sc_gameref_t game) {
	static sc_char line_buffer[LINE_BUFFER_SIZE];
	static sc_char prior_element[LINE_BUFFER_SIZE];
	static sc_char line_element[LINE_BUFFER_SIZE];

	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_memo_setref_t memento = gs_get_memento(game);
	sc_bool is_rerunning, was_undo_available, status;
	sc_char *filtered, *replaced;
	const sc_char *command;

	/* Special case; reset statics if the game isn't running. */
	if (!game->is_running) {
		memset(line_buffer, NUL, sizeof(line_buffer));
		memset(prior_element, NUL, sizeof(prior_element));
		memset(line_element, NUL, sizeof(line_element));
		return TRUE;
	}

	/*
	 * Save the settings of the game's do_again and undo_available flags for
	 * later checks.
	 */
	is_rerunning = game->do_again;
	was_undo_available = game->undo_available;

	/* See if the player asked to rerun a command element. */
	if (game->do_again) {
		game->do_again = FALSE;

		/* Check there is a last element to repeat. */
		if (prior_element[0] == NUL) {
			pf_buffer_string(filter, "You can hardly repeat that.\n");
			return FALSE;
		}

		/* Make the last element the current input element. */
		strcpy(line_element, prior_element);
	} else {
		sc_int length, extent;

		/*
		 * If there's none buffered, read a new line of player input.  Other-
		 * wise, separate output so far with a newline.
		 */
		if (line_buffer[0] == NUL)
			if_read_line(line_buffer, sizeof(line_buffer));
		else
			if_print_character('\n');

		/*
		 * Find the length of the next input line element.  Unless the line
		 * buffer is empty, we always take the first character, even if it's a
		 * separator.  This catches odd input like "." and turns it into a
		 * parser complaint, rather than treating it as two empty commands with
		 * a separator between them; this makes it close to what Inform does
		 * with similar inputs.
		 */
		length = (line_buffer[0] == NUL) ? 0 : 1;
		while (line_buffer[length] != NUL
		        && strchr(SEPARATORS, line_buffer[length]) == NULL)
			length++;

		/*
		 * Make this the current input element, and remove it, the separator,
		 * and any trailing whitespace, from the front of the line buffer.
		 * Removing whitespace prevents "i. ." looking like "i" and ""; it
		 * instead looks like "i" and ".", and results in a parser complaint.
		 */
		memcpy(line_element, line_buffer, length);
		line_element[length] = NUL;

		extent = length;
		extent += (line_buffer[length] == NUL
		           || strchr(SEPARATORS, line_buffer[length]) == NULL) ? 0 : 1;
		extent += strspn(line_buffer + extent, WHITESPACE);
		memmove(line_buffer,
		        line_buffer + extent, strlen(line_buffer) - extent + 1);
	}

	/* Copy the current game to the temporary undo buffer. */
	gs_copy(game->temporary, game);

	/* Filter the input element for synonyms, then for pronouns. */
	filtered = pf_filter_input(line_element, bundle);
	replaced = uip_replace_pronouns(game, filtered ? filtered : line_element);

	/*
	 * If filtering didn't replace synonyms, or no pronouns were replaced, use
	 * the original line element.
	 */
	command = replaced ? sc_normalize_string(replaced)
	          : (filtered ? sc_normalize_string(filtered) : line_element);
	if (command != line_element) {
		if_print_tag(SC_TAG_ITALICS, "");
		if_print_character('[');
		if_print_string(command);
		if_print_character(']');
		if_print_tag(SC_TAG_ENDITALICS, "");
		if_print_character('\n');
	}

	/* Try the command line element against command matchers. */
	status = run_all_commands(game, command);
	if (!status) {
		/* Only complain on non-empty command input line elements. */
		if (!sc_strempty(command)) {
			sc_vartype_t vt_key[2];
			sc_char *escaped;
			const sc_char *message;

			/* Command line element not understood. */
			escaped = pf_escape(sc_normalize_string(line_element));
			var_set_ref_text(vars, escaped);
			sc_free(escaped);
			vt_key[0].string = "Globals";
			vt_key[1].string = "DontUnderstand";
			message = prop_get_string(bundle, "S<-ss", vt_key);
			pf_buffer_string(filter, message);
			pf_buffer_character(filter, '\n');

			/*
			 * On a line element that's not understood, throw out any remaining
			 * input line elements.
			 */
			line_buffer[0] = NUL;
			sc_free(filtered);
			sc_free(replaced);
			return status;
		}
	} else {
		/*
		 * Unless administrative, back up any valid undo, copy the temporary
		 * game into the undo buffer, flag the undo buffer as available, and
		 * assign any pronouns used in the command ready for the next iteration.
		 */
		if (!game->is_admin) {
			if (game->undo_available)
				memo_save_game(memento, game->undo);

			gs_copy(game->undo, game->temporary);
			game->undo_available = TRUE;

			uip_assign_pronouns(game, command);
		}
	}
	sc_free(filtered);
	sc_free(replaced);

	/*
	 * If do_again is set, we'll come round with the prior command in line
	 * element in a moment, so save nothing for that case.  Otherwise save the
	 * command in the history.
	 */
	if (!sc_strempty(line_element) && !game->do_again) {
		/*
		 * If this is a failed redo, redo_sequence will be set but do_again will
		 * be clear.  Suppress the save for this special case; otherwise, failed
		 * redo commands get into the history, where they can cause problems
		 * later on.
		 */
		if (game->redo_sequence == 0) {
			sc_int timestamp;

			timestamp = var_get_elapsed_seconds(vars);
			memo_save_command(memento, line_element, timestamp, game->turns);
		} else
			game->redo_sequence = 0;
	}

	/*
	 * Special case restart and restore commands; throw out any remaining input
	 * and return straight away.  Do the same if this was an undo, detected by
	 * noting that undo is no longer available, where it was on entry.
	 */
	if (game->do_restart || game->do_restore
	        || (was_undo_available && !game->undo_available)) {
		line_buffer[0] = NUL;
		return status;
	}

	/* If not empty, consider as saving for "again" calls and in the history. */
	if (!sc_strempty(line_element)) {
		/*
		 * Unless "again", note this line element as prior input.  "Again" shows
		 * up as do_again set in the game, where it wasn't when we entered here.
		 */
		if (!game->do_again && !is_rerunning)
			strcpy(prior_element, line_element);

		/*
		 * If this was a request to run a command from the history, copy that
		 * command into the prior_element for the next iteration.  The library
		 * should have verified the value in redo_sequence, so fetching the
		 * command string should not fail.
		 */
		if (game->do_again && game->redo_sequence != 0) {
			const sc_char *redo_command;

			redo_command = memo_find_command(memento, game->redo_sequence);
			if (redo_command)
				strcpy(prior_element, redo_command);
			else {
				sc_error("run_player_input: invalid redo sequence request\n");
				game->do_again = FALSE;
			}
			game->redo_sequence = 0;
		}
	}

	return status;
}


/*
 * run_main_loop()
 *
 * Main interpreter loop.
 */
static void run_main_loop(CONTEXT, sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);

	/*
	 * This may not be the very first time this game has been used, for example
	 * saving a game right at the start, or undo-ing back to the start through
	 * memos.  Caught by looking to see if the player room is marked as seen.
	 */
	if (!gs_room_seen(game, gs_playerroom(game))) {
		sc_vartype_t vt_key[2];
		const sc_char *gamename, *startuptext;
		sc_bool disp_first_room, battle_system;

		/* If battle system and no debugger display a warning. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "BattleSystem";
		battle_system = prop_get_boolean(bundle, "B<-ss", vt_key);
		if (battle_system && !debug_get_enabled(game)) {
			if_print_tag(SC_TAG_CLS, "");
			lib_warn_battle_system();
		}

		/* Initial clear screen. */
		pf_buffer_tag(filter, SC_TAG_CLS);

		/* Print the game name. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "GameName";
		gamename = prop_get_string(bundle, "S<-ss", vt_key);
		pf_buffer_string(filter, gamename);
		pf_buffer_character(filter, '\n');

		/* Print the game header. */
		vt_key[0].string = "Header";
		vt_key[1].string = "StartupText";
		startuptext = prop_get_string(bundle, "S<-ss", vt_key);
		pf_buffer_string(filter, startuptext);
		pf_buffer_character(filter, '\n');

		/* If flagged, describe the initial room. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "DispFirstRoom";
		disp_first_room = prop_get_boolean(bundle, "B<-ss", vt_key);
		if (disp_first_room)
			lib_cmd_look(game);

		/* Handle any introductory resources. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "IntroRes";
		res_handle_resource(game, "ss", vt_key);

		/* Set initial values for NPC and object states. */
		npc_setup_initial(game);
		obj_setup_initial(game);

		/* Nudge events and NPCs. */
		evt_tick_events(game);
		npc_tick_npcs(game);

		/*
		 * Notify the debugger that the game has started.  This is a chance to
		 * set watchpoints to catch game startup actions.  Done before setting
		 * the initial room visited as this is how the debugger differentiates
		 * restarts from restore or undo back to game start.
		 */
		CALL1(debug_game_started, game);

		/* Note the initial room as visited. */
		gs_set_room_seen(game, gs_playerroom(game), TRUE);
	} else {
		/* Notify the debugger that the game has restarted. */
		CALL1(debug_game_started, game);
	}

	/*
	 * Game loop, exits either when a command parser handler sets the game
	 * running flag to FALSE, or by call to run_quit().
	 */
	game->is_running &= !g_vm->shouldQuit();
	while (game->is_running) {
		sc_bool status;

		/*
		 * Synchronize any resources in use; do this before flushing so that any
		 * appropriate graphics/sound appear before waits or waitkey tag delays
		 * invoked by flushing the printfilter.  Also, print any score change
		 * notifications.
		 */
		res_sync_resources(game);
		run_notify_score_change(game);

		/*
		 * Flush printfilter of any accumulated output, and clear any prior
		 * notion of administrative commands from input.
		 */
		pf_flush(filter, vars, bundle);
		game->is_admin = FALSE;

		/* If waitcounter is zero, accept and try a command. */
		if (game->waitcounter == 0) {
			/* Not waiting, so handle a player input line. */
			run_update_status(game);
			status = run_player_input(game);

			/*
			 * If waitcounter is now set, decrement it, as this turn counts as
			 * one of them.
			 */
			if (game->waitcounter > 0)
				game->waitcounter--;
		} else {
			/*
			 * Currently "waiting"; decrement wait turns, then run a turn having
			 * taken no input.
			 */
			game->waitcounter--;
			status = TRUE;
		}

		/*
		 * Do usual turn stuff unless either something stopped the game, or the
		 * last command didn't match, or the last command did match but was
		 * administrative.
		 */
		if (status && !game->is_admin) {
			/* Increment turn counter, and clear notifications done flag. */
			game->turns++;
			game->has_notified = FALSE;

			if (game->is_running) {
				/* Nudge events and NPCs. */
				evt_tick_events(game);
				npc_tick_npcs(game);

				/* Update NPC and object states. */
				npc_turn_update(game);
				obj_turn_update(game);

				/* Note the current room as visited. */
				gs_set_room_seen(game, gs_playerroom(game), TRUE);

				/* Give the debugger a chance to catch watchpoints. */
				CALL1(debug_turn_update, game);
			}
		}

		game->is_running &= !g_vm->shouldQuit();
	}

	/*
	 * Final status update, for games that vary it on completion, then notify
	 * the debugger that the game has ended, to let it make a last watchpoint
	 * scan and offer the dialog if appropriate.
	 */
	run_update_status(game);
	CALL1(debug_game_ended, game);

	/*
	 * Final resource sync, score change notification and printfilter flush
	 * on game-instigated loop exit.
	 */
	res_sync_resources(game);
	run_notify_score_change(game);
	pf_flush(filter, vars, bundle);

	/*
	 * Reset static variables inside run_player_input() with a call to it with
	 * is_running false; this is a special case.
	 */
	assert(!game->is_running);
	run_player_input(game);
}


/*
 * run_create()
 *
 * Create a game context from a callback.
 */
sc_gameref_t run_create(sc_read_callbackref_t callback, void *opaque) {
	sc_tafref_t taf;
	sc_prop_setref_t bundle;
	sc_var_setref_t vars, temporary_vars, undo_vars;
	sc_filterref_t filter;
	sc_gameref_t game, temporary_game, undo_game;
	assert(callback);

	/* Create a new TAF using the callback; return NULL if this fails. */
	taf = taf_create(callback, opaque);
	if (!taf)
		return NULL;
	else if (if_get_trace_flag(SC_DUMP_TAF))
		taf_debug_dump(taf);

	/* Create a properties bundle, and parse the TAF data into it. */
	bundle = prop_create(taf);
	if (!bundle) {
		sc_error("run_create: error parsing game data\n");
		taf_destroy(taf);
		return NULL;
	} else if (if_get_trace_flag(SC_DUMP_PROPERTIES))
		prop_debug_dump(bundle);

	/* Try to set an interpreter locale from the properties bundle. */
	loc_detect_game_locale(bundle);
	if (if_get_trace_flag(SC_DUMP_LOCALE_TABLES))
		loc_debug_dump();

	/* Create a set of variables from the bundle. */
	vars = var_create(bundle);
	if (if_get_trace_flag(SC_DUMP_VARIABLES))
		var_debug_dump(vars);

	/* Create a printfilter for the game. */
	filter = pf_create();

	/*
	 * Create an initial game state, and register it with variables.  Also,
	 * create undo buffers, and initialize them in the same way.
	 */
	game = gs_create(vars, bundle, filter);
	var_register_game(vars, game);

	temporary_vars = var_create(bundle);
	temporary_game = gs_create(temporary_vars, bundle, filter);
	var_register_game(temporary_vars, temporary_game);

	undo_vars = var_create(bundle);
	undo_game = gs_create(undo_vars, bundle, filter);
	var_register_game(undo_vars, undo_game);

	/* Add the undo buffers and memos to the game, and return it. */
	game->temporary = temporary_game;
	game->undo = undo_game;
	game->memento = memo_create();
	return game;
}


/*
 * run_restart_handler()
 *
 * Return a game context to initial states to restart a game.
 */
static void run_restart_handler(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_gameref_t new_game;
	sc_var_setref_t new_vars;

	/*
	 * Create a fresh set of variables from the current game properties,
	 * then a new game using these variables and existing properties and
	 * printfilter.
	 */
	new_vars = var_create(bundle);
	new_game = gs_create(new_vars, bundle, filter);
	var_register_game(new_vars, new_game);

	/*
	 * Overwrite the dynamic parts of the current game with the new one.
	 */
	new_game->temporary = game->temporary;
	new_game->undo = game->undo;
	gs_copy(game, new_game);

	/* Destroy invalid game status strings. */
	sc_free(game->current_room_name);
	game->current_room_name = NULL;
	sc_free(game->status_line);
	game->status_line = NULL;

	/*
	 * Now it's safely copied, destroy the temporary new game, and its
	 * associated variable set.
	 */
	gs_destroy(new_game);
	var_destroy(new_vars);

	/* Reset resources handling. */
	res_cancel_resources(game);
}


/*
 * run_restore_handler()
 *
 * Adjust a game context for continuation after restoring a game.
 */
static void run_restore_handler(sc_gameref_t game) {
	/* Invalidate the undo buffer. */
	game->undo_available = FALSE;

	/*
	 * Resources handling?  Arguably we should re-offer resources active when
	 * the game was saved, but I can't see how this can be achieved with Adrift
	 * the way it is.  Canceling is too broad, so I'll go here with just
	 * stopping sounds (in case looping).
	 *
	 * TODO Rationalize what happens here.
	 */
	game->stop_sound = TRUE;
}


/*
 * run_quit_handler()
 *
 * Tidy up printfilter and input statics on game quit.
 */
static void run_quit_handler(sc_gameref_t game) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);

	/* Flush printfilter and notifications of any dangling output. */
	run_notify_score_change(game);
	pf_flush(filter, vars, bundle);

	/* Cancel any active resources. */
	res_cancel_resources(game);

	/*
	 * Make the special call to reset all of the static variables inside
	 * run_player_input().
	 */
	assert(!game->is_running);
	run_player_input(game);
}


/*
 * run_interpret()
 *
 * Intepret the game in a game context.
 */
void run_interpret(CONTEXT, sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	/* Verify the game is not already running, and is runnable. */
	if (game->is_running) {
		sc_error("run_interpret: game is already running\n");
		return;
	}
	if (game->has_completed) {
		sc_error("run_interpret: game has already completed\n");
		return;
	}

	/* Refuse to run a game with no rooms. */
	if (gs_room_count(game) == 0) {
		sc_error("run_interpret: game contains no rooms\n");
		return;
	}

	/* Run the main interpreter loop until no more restarts. */
	game->is_running = TRUE;
	do {
		// Run the game until some form of halt is requested
		CALL1(run_main_loop, game);

		/*
		 * If the halt was a restart or restore, cancel the request, handle
		 * restart or restore game adjustments, and set the game running
		 * again.
		 */
		if (game->do_restart) {
			game->do_restart = FALSE;
			run_restart_handler(game);
			game->is_running = TRUE;
		}

		if (game->do_restore) {
			game->do_restore = FALSE;
			run_restore_handler(game);
			game->is_running = TRUE;
		}
	} while (game->is_running);

	/* Tidy up the printfilter and input statics. */
	run_quit_handler(game);
}


/*
 * run_destroy()
 *
 * Destroy a game context, and free all resources.
 */
void run_destroy(sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	/* Can't destroy the context of a running game. */
	if (game->is_running) {
		sc_error("run_destroy: game is running, stop it first\n");
		return;
	}

	/*
	 * Cancel any game state debugger -- this frees its resources.  Only the
	 * primary game may have acquired a debugger.
	 */
	debug_set_enabled(game, FALSE);
	assert(!debug_get_enabled(game->temporary));
	assert(!debug_get_enabled(game->undo));

	/*
	 * Destroy the game state, variables, properties bundle, memos, undo
	 * buffers and their variables, and filter.  The bundle and printfilter
	 * are shared by the main game, the undo game, and the temporary game, so
	 * destroy these only once!  The main game has a memento, but it is not
	 * visible to these other two games, neither of which have one.
	 */
	assert(gs_get_bundle(game->temporary) == gs_get_bundle(game));
	assert(gs_get_filter(game->temporary) == gs_get_filter(game));
	assert(gs_get_vars(game->temporary) != gs_get_vars(game));
	assert(!gs_get_memento(game->temporary));
	var_destroy(gs_get_vars(game->temporary));
	gs_destroy(game->temporary);

	assert(gs_get_bundle(game->undo) == gs_get_bundle(game));
	assert(gs_get_filter(game->undo) == gs_get_filter(game));
	assert(gs_get_vars(game->undo) != gs_get_vars(game));
	assert(!gs_get_memento(game->undo));
	var_destroy(gs_get_vars(game->undo));
	gs_destroy(game->undo);

	prop_destroy(gs_get_bundle(game));
	pf_destroy(gs_get_filter(game));
	var_destroy(gs_get_vars(game));
	memo_destroy(gs_get_memento(game));

	gs_destroy(game);
}


/*
 * run_quit()
 *
 * Quits a running game.  This function calls a longjump to act as if
 * run_main_loop() returned, and so never returns to its caller.
 */
void run_quit(CONTEXT, sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	// Disallow quitting a non-running game
	if (!game->is_running) {
		sc_error("run_quit: game is not running\n");
		return;
	}

	// Exit the main loop
	game->is_running = FALSE;
	LONG_JUMP;
}


/*
 * run_restart()
 *
 * Restarts either a running or a stopped game.  For running games, this
 * function calls a longjump to act as if run_main_loop() returned, and so
 * never returns to its caller.  For stopped games, it returns.
 */
void run_restart(CONTEXT, sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	/*
	 * If the game is running, stop it, request a restart, and exit the main
	 * loop with a longjump.
	 */
	if (game->is_running) {
		game->is_running = FALSE;
		game->do_restart = TRUE;
		LONG_JUMP;
	}

	// Restart locally, and ensure that the game remains stopped
	run_restart_handler(game);
	game->is_running = FALSE;
}


/*
 * run_save()
 * run_save_prompted()
 *
 * Saves either a running or a stopped game.
 */
void run_save(sc_gameref_t game, sc_write_callbackref_t callback, void *opaque) {
	assert(gs_is_game_valid(game));
	assert(callback);

	SaveSerializer ser(game, callback, opaque);
	ser.save();
}

sc_bool run_save_prompted(sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	return g_vm->saveGame().getCode() == Common::kNoError;
}

/*
 * run_restore_common()
 * run_restore()
 * run_restore_prompted()
 *
 * Restores either a running or a stopped game.  For running games, on
 * successful restore, these functions call a longjump to act as if
 * run_main_loop() returned, and so never return to their caller.  On failed
 * restore, and for stopped games, they will return, with TRUE if successful,
 * FALSE if restore failed.
 */
static sc_bool run_restore_common(CONTEXT, sc_gameref_t game, sc_read_callbackref_t callback, void *opaque) {
	sc_bool is_running, status;

	/*
	 * Save the game running flag, and call the restore appropriate for the
	 * caller.  The indication of a call from run_restore_prompted() is a
	 * callback of NULL; callback cannot be NULL for run_restore() calls.
	 */
	is_running = game->is_running;
	LoadSerializer ser(game, callback, opaque);
	status = ser.load();
	if (status) {
		/* Loading a game clears is_running -- restore it here. */
		game->is_running = is_running;

		/*
		 * If the game is (was) running, set flags so that the interpreter
		 * loop cycles, and exit the main loop with a longjump.
		 */
		if (game->is_running) {
			game->is_running = FALSE;
			game->do_restore = TRUE;
			LONG_JUMP0;
		}
	}

	/* Return TRUE on successful restore of a stopped game, FALSE on error. */
	return status;
}

sc_bool run_restore(CONTEXT, sc_gameref_t game, sc_read_callbackref_t callback, void *opaque) {
	assert(gs_is_game_valid(game));
	assert(callback);

	return run_restore_common(context, game, callback, opaque);
}

sc_bool run_restore_prompted(CONTEXT, sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	return run_restore_common(context, game, NULL, NULL);
}


/*
 * run_undo()
 *
 * Undo a turn in either a running or a stopped game.  Returns TRUE on
 * successful undo, FALSE if no undo buffer is available.
 */
sc_bool run_undo(CONTEXT, sc_gameref_t game) {
	const sc_memo_setref_t memento = gs_get_memento(game);
	sc_bool is_running;
	assert(gs_is_game_valid(game));

	/* Save the game's running state, so we can restore it later. */
	is_running = game->is_running;

	/* If there's an undo buffer available, restore it. */
	if (game->undo_available) {
		/* Restore the undo buffer, and then restore running flag. */
		gs_copy(game, game->undo);
		game->undo_available = FALSE;
		game->is_running = is_running;

		/* Location may have changed; update status. */
		run_update_status(game);

		/* Bring resources into line with the revised game. */
		res_sync_resources(game);
		return TRUE;
	}

	/*
	 * If there is no undo buffer, try to restore one saved previously in a
	 * memo.  Handle as if restoring from a file.
	 */
	if (memo_load_game(memento, game)) {
		/* Loading a game clears is_running -- restore it here. */
		game->is_running = is_running;

		/*
		 * If the game is (was) running, set flags so that the interpreter
		 * loop cycles, and exit the main loop with a longjump.
		 */
		if (game->is_running) {
			game->is_running = FALSE;
			game->do_restore = TRUE;
			LONG_JUMP0;
		}

		/* Game undo on non-running game accomplished with memos. */
		return TRUE;
	}

	/* No undo buffer and no memos available. */
	return FALSE;
}


/*
 * run_is_running()
 *
 * Query the game running state.
 */
sc_bool run_is_running(sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	return game->is_running;
}


/*
 * run_has_completed()
 *
 * Query the game completion state.  Completed games cannot be resumed,
 * since they've run the exit task and thus have nowhere to go.
 */
sc_bool run_has_completed(sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	return game->has_completed;
}


/*
 * run_is_undo_available()
 *
 * Query the game turn undo buffer and memo availability.
 */
sc_bool run_is_undo_available(sc_gameref_t game) {
	const sc_memo_setref_t memento = gs_get_memento(game);
	assert(gs_is_game_valid(game));

	return game->undo_available || memo_is_load_available(memento);
}


/*
 * run_get_attributes()
 * run_set_attributes()
 *
 * Get and set selected game attributes.
 */
void run_get_attributes(sc_gameref_t game, const sc_char **game_name, const sc_char **game_author,
		const sc_char **game_compile_date, sc_int *turns, sc_int *score, sc_int *max_score,
		const sc_char **current_room_name, const sc_char **status_line, const sc_char **preferred_font,
		sc_bool *bold_room_names, sc_bool *verbose, sc_bool *notify_score_change) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_vartype_t vt_key[2];
	assert(gs_is_game_valid(game));

	/* Return the game name, author, and compile date if requested. */
	if (game_name) {
		if (!game->title) {
			const sc_char *gamename;
			sc_char *filtered;

			vt_key[0].string = "Globals";
			vt_key[1].string = "GameName";
			gamename = prop_get_string(bundle, "S<-ss", vt_key);

			filtered = pf_filter_for_info(gamename, vars);
			pf_strip_tags(filtered);
			game->title = filtered;
		}
		*game_name = game->title;
	}
	if (game_author) {
		if (!game->author) {
			const sc_char *gameauthor;
			sc_char *filtered;

			vt_key[0].string = "Globals";
			vt_key[1].string = "GameAuthor";
			gameauthor = prop_get_string(bundle, "S<-ss", vt_key);

			filtered = pf_filter_for_info(gameauthor, vars);
			pf_strip_tags(filtered);
			game->author = filtered;
		}
		*game_author = game->author;
	}
	if (game_compile_date) {
		vt_key[0].string = "CompileDate";
		*game_compile_date = prop_get_string(bundle, "S<-s", vt_key);
	}

	/* Return the current room name and status line if requested. */
	if (current_room_name)
		*current_room_name = game->current_room_name;
	if (status_line)
		*status_line = game->status_line;

	/* Return any game preferred font, or NULL if none. */
	if (preferred_font) {
		vt_key[0].string = "CustomFont";
		if (prop_get_boolean(bundle, "B<-s", vt_key)) {
			vt_key[0].string = "FontNameSize";
			*preferred_font = prop_get_string(bundle, "S<-s", vt_key);
		} else
			*preferred_font = NULL;
	}

	/* Return any other selected game attributes. */
	if (turns)
		*turns = game->turns;
	if (score)
		*score = game->score;
	if (max_score) {
		vt_key[0].string = "Globals";
		vt_key[1].string = "MaxScore";
		*max_score = prop_get_integer(bundle, "I<-ss", vt_key);
	}
	if (bold_room_names)
		*bold_room_names = game->bold_room_names;
	if (verbose)
		*verbose = game->verbose;
	if (notify_score_change)
		*notify_score_change = game->notify_score_change;
}

void run_set_attributes(sc_gameref_t game, sc_bool bold_room_names, sc_bool verbose,
		sc_bool notify_score_change) {
	assert(gs_is_game_valid(game));

	/* Set game options. */
	game->bold_room_names = bold_room_names;
	game->verbose = verbose;
	game->notify_score_change = notify_score_change;
}


/*
 * run_hint_iterate()
 *
 * Return the next hint appropriate to the game state, or the first if
 * hint is NULL.  Returns NULL if none, or no more hints.  This function
 * works with pointers to a task state rather than task indexes so that
 * the token passed in and out is a pointer, and readily made opaque to
 * the client as a void*.
 */
sc_hintref_t run_hint_iterate(sc_gameref_t game, sc_hintref_t hint) {
	sc_int task;
	assert(gs_is_game_valid(game));

	/*
	 * Hint is a pointer to a task state; convert to a task index, adding one
	 * to move on to the next task, or start at the first task if null.
	 */
	if (!hint)
		task = 0;
	else {
		/* Convert into pointer, and range check. */
		task = hint - game->tasks;
		if (task < 0 || task >= gs_task_count(game)) {
			sc_error("run_hint_iterate: invalid iteration hint\n");
			return NULL;
		}

		/* Advance beyond current task. */
		task++;
	}

	/* Scan for the next runnable task that offers a hint. */
	for (; task < gs_task_count(game); task++) {
		if (task_can_run_task(game, task) && task_has_hints(game, task))
			break;
	}

	/* Return a pointer to the state of the task identified, or NULL. */
	return task < gs_task_count(game) ? game->tasks + task : NULL;
}


/*
 * run_get_hint_common()
 * run_get_hint_question()
 * run_get_subtle_hint()
 * run_get_unsubtle_hint()
 *
 * Return the strings for a hint.  Front-ends to task functions.  Each
 * converts the hint "address" to a task index through pointer arithmetic,
 * then filters it and returns a temporary, valid only until the next hint
 * call.
 *
 * Hint strings are NULL if empty (not defined by the game).
 */
static const sc_char *run_get_hint_common(sc_gameref_t game, sc_hintref_t hint,
		const sc_char * (*handler)(sc_gameref_t, sc_int)) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int task;
	const sc_char *string;
	assert(gs_is_game_valid(game));

	/* Verify the caller passed in a valid hint. */
	task = hint - game->tasks;
	if (task < 0 || task >= gs_task_count(game)) {
		sc_error("run_get_hint_common: invalid iteration hint\n");
		return NULL;
	} else if (!task_has_hints(game, task)) {
		sc_error("run_get_hint_common: task has no hint\n");
		return NULL;
	}

	/* Get the required game text by calling the given handler function. */
	string = handler(game, task);
	if (!sc_strempty(string)) {
		sc_char *filtered;

		/* Filter and strip tags, note in game. */
		filtered = pf_filter(string, vars, bundle);
		pf_strip_tags_for_hints(filtered);
		sc_free(game->hint_text);
		game->hint_text = filtered;
	} else {
		/* Hint text is empty; drop any text noted in game. */
		sc_free(game->hint_text);
		game->hint_text = NULL;
	}

	return game->hint_text;
}

const sc_char *run_get_hint_question(sc_gameref_t game, sc_hintref_t hint) {
	return run_get_hint_common(game, hint, task_get_hint_question);
}

const sc_char *run_get_subtle_hint(sc_gameref_t game, sc_hintref_t hint) {
	return run_get_hint_common(game, hint, task_get_hint_subtle);
}

const sc_char *run_get_unsubtle_hint(sc_gameref_t game, sc_hintref_t hint) {
	return run_get_hint_common(game, hint, task_get_hint_unsubtle);
}

} // End of namespace Adrift
} // End of namespace Glk
