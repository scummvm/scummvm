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
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/core/converse.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/book.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/views/portrait_view.h"
#include "ultima/nuvie/core/timed_event.h"
#include "ultima/nuvie/views/inventory_view.h"
#include "ultima/nuvie/views/party_view.h"
#include "ultima/nuvie/views/actor_view.h"
#include "ultima/nuvie/gui/widgets/command_bar.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/core/effect_manager.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/core/magic.h"
#include "ultima/nuvie/gui/gui_yes_no_dialog.h"
#include "ultima/nuvie/menus/game_menu_dialog.h"
#include "ultima/nuvie/views/inventory_widget.h"
#include "ultima/nuvie/keybinding/keys.h"
#include "ultima/nuvie/views/spell_view.h"
#include "ultima/nuvie/gui/widgets/fps_counter.h"
#include "ultima/nuvie/script/script.h"

#include "common/system.h"

namespace Ultima {
namespace Nuvie {

Events *Events::g_events;

using Std::string;

EventInput_s::~EventInput_s() {
	if (target_init) delete target_init;
	if (str) delete str;
	if (loc) delete loc;
}

void EventInput_s::set_loc(MapCoord c) {
	if ((type == EVENTINPUT_MAPCOORD || type == EVENTINPUT_MAPCOORD_DIR) && loc != 0) delete loc;
	loc = new MapCoord(c);
}

Events::Events(Shared::EventsCallback *callback, Configuration *cfg) : Shared::EventsManager(callback), config(cfg) {
	g_events = this;
	clear();
}

Events::~Events() {
	g_events = nullptr;

	delete time_queue;
	delete game_time_queue;
}

void Events::clear() {
	clear_alt_code();
	active_alt_code = 0;
	alt_code_input_num = 0;

	game = Game::get_game();
	gui = NULL;
	obj_manager = NULL;
	map_window = NULL;
	scroll = NULL;
	clock = NULL;
	player = NULL;
	view_manager = NULL;
	usecode = NULL;
	magic = NULL;
	drop_obj = NULL;
	ts = 0;
	drop_qty = 0;
	drop_x = drop_y = -1;
	rest_time = 0;
	rest_guard = 0;
	push_obj = NULL;
	push_actor = NULL;
	drop_from_key = false;
	move_in_inventory = false;
	time_queue = game_time_queue = NULL;
	showingDialog = false;
	gamemenu_dialog = NULL;
	ignore_timeleft = false;
	in_control_cheat = false;
	looking_at_spellbook = false;
	using_pickpocket_cheat = false;
	do_not_show_target_cursor = false;
	config->value("config/input/direction_selects_target", direction_selects_target, true);

	mode = MOVE_MODE;
	last_mode = MOVE_MODE;

	fps_timestamp = 0;
	fps_counter = 0;

	scriptThread = NULL;
}

bool Events::init(ObjManager *om, MapWindow *mw, MsgScroll *ms, Player *p, Magic *mg,
                 GameClock *gc, ViewManager *vm, UseCode *uc, GUI *g, KeyBinder *kb) {
	clear();

	gui = g;
	obj_manager = om;
	map_window = mw;
	scroll = ms;
	clock = gc;
	player = p;
	view_manager = vm;
	usecode = uc;

	mode = MOVE_MODE;
	last_mode = MOVE_MODE;
	input.get_direction = false;
	input.get_text = false;
	cursor_mode = false;
	input.target_init = NULL;

	time_queue = new TimeQueue;
	game_time_queue = new TimeQueue;
	magic = mg;
	keybinder = kb;

	fps_timestamp = clock->get_ticks();

	fps_counter_widget = new FpsCounter(game);
	gui->AddWidget(fps_counter_widget);
	fps_counter_widget->Hide();
	scriptThread = NULL;

	return true;
}

void Events::update_timers() {
	time_queue->call_timers(clock->get_ticks());
	game_time_queue->call_timers(clock->get_game_ticks());
}

bool Events::update() {
	bool idle = true;
	// timed
	time_queue->call_timers(clock->get_ticks());
	game_time_queue->call_timers(clock->get_game_ticks());

	// polled
	Common::Event evt;
	while (pollEvent(evt)) {
		idle = false;
		switch (gui->HandleEvent(&evt)) {
		case GUI_PASS :
			if (handleEvent(&evt) == false) {
				game->quit();
				return false;
			}
			break;

		case GUI_QUIT :
			game->quit();
			return false;

		default :
			break;
		}
	}

	if (idle)
		gui->Idle(); // run Idle() for all widgets

	if (showingDialog) // temp. fix to show normal cursor over quit dialog
		game->set_mouse_pointer(0);

	return true;
}

bool Events::handleSDL_KEYDOWN(const Common::Event *event_) {
	// when casting the magic class will handle keyboard events
	if (mode == KEYINPUT_MODE) {
		Common::KeyCode sym = event_->kbd.keycode;
		ActionKeyType action_key_type = OTHER_KEY;

		if (!((magic->is_selecting_spell() && ((sym >= Common::KEYCODE_a && sym <= Common::KEYCODE_z) || sym == Common::KEYCODE_BACKSPACE)) ||
		        ((magic->is_waiting_for_location() || last_mode == USE_MODE) && sym >= Common::KEYCODE_1 && sym <= Common::KEYCODE_9))) {
			ActionType a = keybinder->get_ActionType(event_->kbd);
			action_key_type = keybinder->GetActionKeyType(a);
			//switch (action_key_type) {
			//default:
				if (keybinder->handle_always_available_keys(a)) return true;
			//	break;
			//}
		}
		input.type = EVENTINPUT_KEY;
		input.key = sym;
		input.action_key_type = action_key_type;
		// callback should return a true value if it handled the event_
		if (action_key_type != CANCEL_ACTION_KEY && message(CB_DATA_READY, (char *) &input))
			return true;
		callback_target = 0;
		endAction(); // no more keys for you! (end KEYINPUT_MODE)
		keybinder->HandleEvent(event_);
		return true;
	}

	byte mods = event_->kbd.flags;
	// alt-code input
	if (mods & Common::KBD_ALT) {
		if (mode == MOVE_MODE)
			switch (event_->kbd.keycode) {
			case Common::KEYCODE_KP0:
			case Common::KEYCODE_0:
				alt_code_str[alt_code_len++] = '0';
				break;

			case Common::KEYCODE_KP1:
			case Common::KEYCODE_1:
				alt_code_str[alt_code_len++] = '1';
				break;

			case Common::KEYCODE_KP2:
			case Common::KEYCODE_2:
				alt_code_str[alt_code_len++] = '2';
				break;

			case Common::KEYCODE_KP3:
			case Common::KEYCODE_3:
				alt_code_str[alt_code_len++] = '3';
				break;

			case Common::KEYCODE_KP4:
			case Common::KEYCODE_4:
				alt_code_str[alt_code_len++] = '4';
				break;

			case Common::KEYCODE_KP5:
			case Common::KEYCODE_5:
				alt_code_str[alt_code_len++] = '5';
				break;

			case Common::KEYCODE_KP6:
			case Common::KEYCODE_6:
				alt_code_str[alt_code_len++] = '6';
				break;

			case Common::KEYCODE_KP7:
			case Common::KEYCODE_7:
				alt_code_str[alt_code_len++] = '7';
				break;

			case Common::KEYCODE_KP8:
			case Common::KEYCODE_8:
				alt_code_str[alt_code_len++] = '8';
				break;

			case Common::KEYCODE_KP9:
			case Common::KEYCODE_9:
				alt_code_str[alt_code_len++] = '9';
				break;
			default:
				keybinder->HandleEvent(event_);
				return true;
			}
		if (alt_code_len != 0) {
			alt_code_str[alt_code_len] = '\0';
			if (alt_code_len == 3) {
				alt_code(alt_code_str);
				clear_alt_code();
			}
		}
		return true;
	}

	keybinder->HandleEvent(event_);

	return true;
}

void Events::target_spell() {
	input.type = EVENTINPUT_KEY;
	input.key = Common::KEYCODE_RETURN; // only needed to overwrite old value so it isn't a number or backspace
	input.action_key_type = DO_ACTION_KEY;
	message(CB_DATA_READY, (char *) &input);
	callback_target = 0;
	endAction();
	doAction();
}

void Events::close_spellbook() {
	if (callback_target) {
		callback_target = 0;
		endAction();
	}
	cancelAction();
}

bool Events::handleEvent(const Common::Event *event_) {
	if (game->user_paused())
		return true;

	switch (event_->type) {
	case Common::EVENT_MOUSEMOVE:
		break;
	case Common::EVENT_KEYUP:
		if (event_->kbd.flags & Common::KBD_ALT) {
			clear_alt_code();
		}
		break;

	case Common::EVENT_KEYDOWN:
		handleSDL_KEYDOWN(event_);
		break;

	case Common::EVENT_QUIT:
		return false;

	default:
		break;
	}

	if (input.get_text && scroll->has_input()) {
		if (active_alt_code) {
			endAction(); // exit INPUT_MODE
			alt_code_input(scroll->get_input().c_str());
		} else {
			doAction();
		}
	}
	return true;
}

void Events::get_direction(const char *prompt) {
//    use_obj = src;
	assert(mode != INPUT_MODE);
	set_mode(INPUT_MODE); // saves previous mode
	if (prompt)
		scroll->display_string(prompt);
	input.get_direction = true;

	moveCursorToMapWindow();
	map_window->centerCursor();
	map_window->set_show_cursor(false);
//    map_window->set_show_use_cursor(true); // set in moveCursorToMapWindow()
	if (do_not_show_target_cursor && direction_selects_target)
		map_window->set_show_use_cursor(false);
	input.target_init = new MapCoord(map_window->get_cursorCoord()); // depends on MapWindow size
}
/* This version of get_direction() doesn't show the cursor. */
void Events::get_direction(const MapCoord &from, const char *prompt) {
	get_direction(prompt);
	map_window->moveCursor(from.x - map_window->get_cur_x(), from.y - map_window->get_cur_y());
	input.target_init->x = from.x;
	input.target_init->y = from.y;
	if (input_really_needs_directon()) { // actually getting a direction
		if (!direction_selects_target)
			map_window->set_show_cursor(true);
		map_window->set_show_use_cursor(false);
		map_window->set_mousecenter(from.x - map_window->get_cur_x(), from.y - map_window->get_cur_y());
	}
}

void Events::get_target(const char *prompt) {
//    use_obj = src;
	assert(mode != INPUT_MODE);
	set_mode(INPUT_MODE); // saves previous mode
	if (prompt)
		scroll->display_string(prompt);
	input.get_direction = false;

	map_window->centerCursor();
	moveCursorToMapWindow();
}

void Events::get_target(const MapCoord &init, const char *prompt) {
	get_target(prompt);
	map_window->moveCursor(init.x, init.y);
}

/* Switch focus to MsgScroll and start getting user input. */
void Events::get_scroll_input(const char *allowed,
                             bool can_escape,
                             bool using_target_cursor,
                             bool set_numbers_only_to_true) {
	assert(scroll);
	if (!using_target_cursor) {
		assert(mode != INPUT_MODE);
		set_mode(INPUT_MODE); // saves previous mode
	}
	input.get_text = true;
	scroll->set_input_mode(true, allowed, can_escape, using_target_cursor, set_numbers_only_to_true);
//no need to grab focus because any input will eventually reach MsgScroll,
//    scroll->grab_focus();
}

void Events::get_inventory_obj(Actor *actor, bool getting_target) {
	if (getting_target) {
		get_target("");
		moveCursorToInventory();
	} else if (!game->is_new_style())
		view_manager->set_inventory_mode();
	if (game->is_new_style()) {
		//view_manager->set_inventory_mode();
		view_manager->open_container_view(actor); //FIXME need to open container gump in pickpocket mode.
		view_manager->open_doll_view(actor);
	} else {
		view_manager->get_inventory_view()->set_actor(actor, true);
	}
}

void Events::get_spell_num(Actor *caster, Obj *spell_container) {
	//get_target("");
	view_manager->set_spell_mode(caster, spell_container, true);
	view_manager->get_current_view()->grab_focus();
}

/* Send all keyboard input to caller, with user_data.
   ESC always cancels sending any further input. */
void Events::key_redirect(CallBack *caller, void *user_data) {
	assert(mode != INPUT_MODE && mode != KEYINPUT_MODE);
	request_input(caller, user_data);
	set_mode(KEYINPUT_MODE); // saves previous mode
}

void Events::cancel_key_redirect() {
	assert(mode == KEYINPUT_MODE);
	endAction();
}

/* Switch focus to PortraitView, display a portrait, and wait for user input. */
void Events::display_portrait(Actor *actor, const char *name) {
	view_manager->set_portrait_mode(actor, name);
	view_manager->get_portrait_view()->set_waiting(true);
}

/* Set callback & callback_user_data so that a message will be sent to the
 * caller when input has been gathered. */
void Events::request_input(CallBack *caller, void *user_data) {
	callback_target = caller;
	callback_user_data = (char *) user_data;
}

// typically this will be coming from inventory
bool Events::select_obj(Obj *obj, Actor *actor) {
	if (looking_at_spellbook && view_manager->get_spell_view() != NULL) {
		view_manager->get_spell_view()->close_look();
		return false;
	}
	assert(mode == INPUT_MODE);
	//assert(input.select_from_inventory == true);

	input.type = EVENTINPUT_OBJECT;
	input.obj = obj;
	input.actor = actor;
	endAction(); // mode = prev_mode
	doAction();
	return true;
}

bool Events::select_actor(Actor *actor) {
	assert(mode == INPUT_MODE);

	input.type = EVENTINPUT_MAPCOORD;
	input.actor = actor;
	input.set_loc(actor->get_location());
	endAction(); // mode = prev_mode
	doAction();
	return true;
}

bool Events::select_direction(sint16 rel_x, sint16 rel_y) {
	assert(mode == INPUT_MODE);
	assert(input.get_direction == true);

	input.type = EVENTINPUT_MAPCOORD_DIR;
	input.set_loc(MapCoord(rel_x, rel_y));
	// assumes mapwindow cursor is at the location
	input.actor = map_window->get_actorAtCursor();
	input.obj = map_window->get_objAtCursor();
	endAction(); // mode = prev_mode
	doAction();
	return true;
}

// automatically converted to direction if requested
bool Events::select_target(uint16 x, uint16 y, uint8 z) {
	// FIXME: is this even correct behavior?! if an arrow key is used, a direction
	// should be returned, but you can still select any target with the mouse
	// (which works, but then what's the point of using directions?)
	if (input.get_direction)
		return select_direction(x - input.target_init->x,
		                        y - input.target_init->y);
	if (mode != ATTACK_MODE) { // FIXME: make ATTACK_MODE use INPUT_MODE
		// need to handle weapon range
		assert(mode == INPUT_MODE);

		input.type = EVENTINPUT_MAPCOORD;
		input.set_loc(MapCoord(x, y, z));
		// assumes mapwindow cursor is at the location
		input.actor = map_window->get_actorAtCursor();
		input.obj = map_window->get_objAtCursor();
		endAction(); // mode = prev_mode
	}
	doAction();
	return true;
}

// called when selecting an actor by number
bool Events::select_party_member(uint8 num) {
	Party *party = player->get_party();
	if (num < party->get_party_size()) {
		select_actor(party->get_actor(num));
		return true;
	}
	return false;
}

bool Events::select_spell_num(sint16 spell_num) {
	//assert(mode == INPUT_MODE);
	//assert(input.select_from_inventory == true);



	input.type = EVENTINPUT_SPELL_NUM;
	input.spell_num = spell_num;
	//endAction(); // mode = prev_mode
	game->get_view_manager()->close_spell_mode();
	doAction();
	return true;
}

// move the cursor or walk around; do action for direction-targeted modes
bool Events::move(sint16 rel_x, sint16 rel_y) {
	MapCoord cursor_coord;

	if (game->user_paused())
		return false;
	EventMode current_mode;
	if (last_mode == MULTIUSE_MODE && game->get_party()->is_in_combat_mode())
		current_mode = ATTACK_MODE;
	else
		current_mode = mode;

	switch (current_mode) {
	case ATTACK_MODE     :
		cursor_coord = map_window->get_cursorCoord();
		cursor_coord.x = WRAPPED_COORD(cursor_coord.x + rel_x, cursor_coord.z);
		cursor_coord.y = WRAPPED_COORD(cursor_coord.y + rel_y, cursor_coord.z);
		if (!player->weapon_can_hit(cursor_coord.x, cursor_coord.y))
			break;
		DEBUG(0, LEVEL_DEBUGGING, "attack select(%d,%d)\n", cursor_coord.x, cursor_coord.y);
		map_window->moveCursorRelative(rel_x, rel_y);
		break;

	case EQUIP_MODE      :
		map_window->moveCursorRelative(rel_x, rel_y);
		break;

	case INPUT_MODE      : {
		bool needs_dir = input_really_needs_directon();
		if (!direction_selects_target && needs_dir) {
			cursor_coord = map_window->get_cursorCoord();
			cursor_coord.x = WRAPPED_COORD(cursor_coord.x + rel_x, cursor_coord.z);
			cursor_coord.y = WRAPPED_COORD(cursor_coord.y + rel_y, cursor_coord.z);
			if (input.target_init->distance(cursor_coord) > 1)
				break;
		} else if (last_mode == CAST_MODE) {
			cursor_coord = map_window->get_cursorCoord();
			cursor_coord.x = WRAPPED_COORD(cursor_coord.x + rel_x, cursor_coord.z);
			cursor_coord.y = WRAPPED_COORD(cursor_coord.y + rel_y, cursor_coord.z);
			if (player->get_actor()->get_range(cursor_coord.x, cursor_coord.y) > 7)
				break;
		}
		map_window->moveCursorRelative(rel_x, rel_y);
		if (direction_selects_target && needs_dir)
			select_direction(rel_x, rel_y);
		break;
	}
	default              :
		if (player->check_walk_delay() && !view_manager->gumps_are_active()) {
			player->moveRelative(rel_x, rel_y);
			game->time_changed();
		}
		break;
	}

	return true;
}

/* Begin a conversation with an actor if him/her/it is willing to talk.
 * Returns true if conversation starts.
 */
bool Events::perform_talk(Actor *actor) {
	ActorManager *actor_manager = game->get_actor_manager();
	Actor *pc = player->get_actor();
	uint8 id = actor->get_actor_num();

	if (game->get_game_type() != NUVIE_GAME_U6) {
		return game->get_script()->call_talk_to_actor(actor);
	}

	if (actor->is_in_vehicle()) {
		scroll->display_string("Not in vehicle.\n");
		return false;
	}
	if (id == pc->get_actor_num()) {  // actor is controlled by player
		// Note: being the player, this should ALWAYS use the real name
		scroll->display_string(actor->get_name());
		scroll->display_string("\n");
		scroll->display_string("Talking to yourself?\n");
		return false;
	}
	if (actor->is_in_party() && !actor->is_onscreen()) {
		scroll->display_string(actor->get_name());
		scroll->display_string("\n");
		scroll->display_string("Not on screen.\n");
		return false;
	}
	// FIXME: this check and the "no response" messages should be in Converse
	if (!player->in_party_mode() && !pc->is_avatar()) { //only the avatar can talk in solo mode
		// always display look-string on failure
		scroll->display_string(actor->get_name());
		scroll->display_string("\n");
		scroll->display_string("Not in solo mode.\n");
	} else if (actor->is_sleeping() || actor->is_paralyzed() || actor->get_corpser_flag()
	           || actor->get_alignment() == ACTOR_ALIGNMENT_EVIL
	           || actor->get_alignment() == ACTOR_ALIGNMENT_CHAOTIC
	           || (actor->get_alignment() == ACTOR_ALIGNMENT_NEUTRAL && actor->will_not_talk())) {
		// always display name or look-string on failure
		scroll->display_string(actor->get_name());
		scroll->display_string("\n\nNo response\n");
	} else if (game->get_converse()->start(actor)) {  // load and begin npc script
		// try to use real name
		scroll->display_string(actor->get_name());
		scroll->display_string("\n");
		// turn towards eachother
		pc->face_actor(actor);
		if (!actor->is_immobile())
			actor->face_actor(pc);
		return (true);
	} else {  // some actor that has no script
		// always display look-string on failure
		scroll->display_string(actor_manager->look_actor(actor));
		scroll->display_string("\n");
		scroll->display_string("Funny, no response.\n");
	}
	return (false);
}

/* Talk to `actor'. Return to the prompt if no conversation starts.
 * Returns the result of the talk function.
 */
bool Events::talk(Actor *actor) {
	bool talking = true;
	if (game->user_paused())
		return (false);

	endAction();

	if (!actor) {
		scroll->display_string("nothing!\n");
		talking = false;
	} else if (!perform_talk(actor))
		talking = false;

	if (!talking) {
//        scroll->display_string("\n");
//        scroll->display_prompt();
		endAction(true);
	}
	return (talking);
}

bool Events::talk_cursor() {
	Actor *actor = map_window->get_actorAtCursor();
	if (actor && input.actor->is_visible())
		return talk(actor);
	return talk(map_window->get_objAtCursor());
}

bool Events::talk_start() {
	if (game->user_paused())
		return (false);
	close_gumps();
	get_target("Talk-");
	return true;
}

/* You can talk to some objects using their quality as actor number. */
bool Events::talk(Obj *obj) {
	ActorManager *actor_manager = game->get_actor_manager();
	if (obj) {
		if (game->get_game_type() == NUVIE_GAME_U6) {
			if (obj->obj_n == OBJ_U6_SHRINE
			        || obj->obj_n == OBJ_U6_STATUE_OF_MONDAIN
			        || obj->obj_n == OBJ_U6_STATUE_OF_MINAX
			        || obj->obj_n == OBJ_U6_STATUE_OF_EXODUS)
				return (talk(actor_manager->get_actor(obj->quality)));
		} else {
			endAction();
			bool status = game->get_script()->call_talk_to_obj(obj);
			if (status == false) {
				scroll->display_string("\n");
				scroll->display_prompt();
			}
			return status;
		}
	}
	scroll->display_string("nothing!\n");
	endAction();
	scroll->display_string("\n");
	scroll->display_prompt();
	return (false);
}

void Events::try_next_attack() {
	if (Game::get_game()->get_actor_manager()->get_avatar()->get_hp() == 0) { // need to end turn if Avatar died
		endAction();
		Game::get_game()->get_actor_manager()->startActors();
		return;
	} else if (player->attack_select_next_weapon(true) == false) {
		player->subtract_movement_points(10);
		game->get_actor_manager()->startActors(); // end player turn
		endAction();
	} else {
		map_window->set_show_cursor(true);
		mode = ATTACK_MODE; // FIXME: need to return after WAIT_MODE
		//endAction(false);
		//newAction(ATTACK_MODE);
	}
}

bool Events::attack() {
	MapCoord target = map_window->get_cursorCoord();
	Actor *actor = map_window->get_actorAtCursor();
	Actor *p = player->get_actor();
	bool tile_is_black = map_window->tile_is_black(target.x, target.y);

	if (game->get_script()->call_out_of_ammo(p, p->get_weapon_obj(player->get_current_weapon()), true)) {
		// the function prints out the message
		try_next_attack(); // SE and MD have weapons that need ammo and only take up 1 slot
		return true;
	} else if (tile_is_black)
		scroll->display_string("nothing!\n");
	else if (actor) {
		if (actor->get_actor_num() == player->get_actor()->get_actor_num() //don't attack yourself.
		        || (actor->is_in_party() && actor->get_alignment() == ACTOR_ALIGNMENT_GOOD)) {
			ActorManager *actor_manager = game->get_actor_manager();
			Actor *a = actor_manager->get_actor(actor->get_x(), actor->get_y(), actor->get_z(), true, actor);
			if (a) // exclude previous target if we find another actor
				actor = a;
			else if (actor->get_actor_num() == player->get_actor()->get_actor_num()) {
				scroll->display_string("pass.\n");
				player->subtract_movement_points(10);
				endAction(true);
				return true;
			}
		}
		if (actor->is_visible()) {
			scroll->display_string(actor->get_name());
			scroll->display_string(".\n");
		}
	}
	if ((!actor || !actor->is_visible()) && !tile_is_black) {
		Obj *obj = map_window->get_objAtCursor();
		if (obj && (!obj->is_on_map() || !map_window->tile_is_black(obj->x, obj->y, obj))) {
			scroll->display_string(obj_manager->get_obj_name(obj->obj_n, obj->frame_n));
			scroll->display_string(".\n");
		} else {
			scroll->display_string(game->get_game_map()->look(target.x, target.y, target.z));
			scroll->display_string(".\n");
		}
	}

	map_window->set_show_cursor(false);
	player->attack(target, actor);

	try_next_attack();

	return true;
}

bool Events::get_start() {
	if (game->user_paused())
		return false;
	if (game->get_script()->call_is_ranged_select(GET))
		get_target("Get-");
	else
		get_direction("Get-");
	return true;
}

bool Events::push_start() {
	if (game->user_paused())
		return false;
	push_obj = NULL;
	push_actor = NULL;
	if (game->get_script()->call_is_ranged_select(MOVE))
		get_target("Move-");
	else
		get_direction("Move-");
	return true;
}

/* Get object into an actor. (no mode change) */
bool Events::perform_get(Obj *obj, Obj *container_obj, Actor *actor) {
	bool got_object = false;
	bool can_perform_get = false;
	//float weight;
	if (game->user_paused())
		return (false);

	if (obj) {
		if (!actor)
			actor = player->get_actor();

		if (obj->is_on_map() && map_window->tile_is_black(obj->x, obj->y, obj)) {
			scroll->display_string("nothing");
		} else {
			scroll->display_string(obj_manager->look_obj(obj));

			if (game->using_hackmove())
				can_perform_get = true;
			else if (!map_window->can_get_obj(actor, obj)) {
				scroll->display_string("\n\nCan't reach it.");
			} else if (obj->is_on_map()) {
				MapCoord target(obj->x, obj->y, obj->z);
				if (!game->get_script()->call_is_ranged_select(GET)
				        && player->get_actor()->get_location().distance(target) > 1
				        && map_window->get_interface() == INTERFACE_NORMAL) {
					scroll->display_string("\n\nOut of range!");
				} else if (obj_manager->obj_is_damaging(obj, actor)) {
					return false;
				} else {
					can_perform_get = true;
				}
			} else {
				can_perform_get = true;
			}
		}
	} else
		scroll->display_string("nothing");

	if (can_perform_get) {
		// perform GET usecode (can't add to container)
		if (usecode->has_getcode(obj) && (usecode->get_obj(obj, actor) == false)) {
			game->get_script()->call_actor_subtract_movement_points(actor, 3);
			scroll->display_string("\n");
			scroll->display_prompt();
			map_window->updateBlacking();
			return (false); // ???
		}

		got_object = game->get_script()->call_actor_get_obj(actor, obj, container_obj);
	}

	scroll->display_string("\n\n");
	scroll->display_prompt();
	map_window->updateBlacking();
	return (got_object);
}

/* Get object at selected position, and end action. */
bool Events::get(sint16 rel_x, sint16 rel_y) {
	uint16 x, y;
	uint8 level;

	player->get_location(&x, &y, &level);
	return get(MapCoord((uint16)(x + rel_x), (uint16)(y + rel_y), level));
}

bool Events::get(MapCoord coord) {
	Obj *obj = obj_manager->get_obj(coord.x, coord.y, coord.z, OBJ_SEARCH_TOP, OBJ_EXCLUDE_IGNORED);
	bool got_object;
	if (!game->is_new_style())
		got_object = perform_get(obj, view_manager->get_inventory_view()->get_inventory_widget()->get_container(),
		                         player->get_actor());
	else
		got_object = perform_get(obj, NULL, player->get_actor());
	view_manager->update(); //redraw views to show new item.
	endAction();

	return got_object;
}

bool Events::use_start() {
	if (game->user_paused())
		return false;
	if (game->get_script()->call_is_ranged_select(USE))
		get_target("Use-");
	else
		get_direction("Use-");

	return true;
}

bool Events::use(Obj *obj) {
	if (game->user_paused())
		return false;
	if (obj && obj->is_on_map() && map_window->tile_is_black(obj->x, obj->y, obj)) {
		Obj *bottom_obj = obj_manager->get_obj(obj->x, obj->y, obj->z, false);
		if (game->get_game_type() == NUVIE_GAME_U6 && bottom_obj->obj_n == OBJ_U6_SECRET_DOOR // hack for frame 2
		        && !map_window->tile_is_black(obj->x, obj->y, bottom_obj))
			obj = bottom_obj;
		else
			obj = NULL;
	}
	if (!obj) {
		scroll->display_string("nothing\n");
		endAction(true);
		return true;
	}
	MapCoord target(obj->x, obj->y, obj->z);
	MapCoord player_loc = player->get_actor()->get_location();
	bool display_prompt = true;

	scroll->display_string(obj_manager->look_obj(obj));
	scroll->display_string("\n");

	if (!usecode->has_usecode(obj)) {
		scroll->display_string("\nNot usable\n");
		DEBUG(0, LEVEL_DEBUGGING, "Object %d:%d\n", obj->obj_n, obj->frame_n);
	} else if (!obj->is_in_inventory() && map_window->get_interface() == INTERFACE_NORMAL
	           && !game->get_script()->call_is_ranged_select(USE) && player->get_actor()->get_location().distance(target) > 1) {
		scroll->display_string("\nOut of range!\n");
		DEBUG(0, LEVEL_DEBUGGING, "distance to object: %d\n", player->get_actor()->get_location().distance(target));
	} else if (!player->in_party_mode() && obj->is_in_inventory() && !obj->get_actor_holding_obj()->is_onscreen()) {
		scroll->display_string("\nNot on screen.\n");
	} else if (!obj->is_in_inventory() && !game->get_script()->call_is_ranged_select(USE)
	           && !map_window->can_get_obj(player->get_actor(), obj) && player_loc != target) {
		scroll->display_string("\nCan't reach it\n");
	} else { // Usable
		display_prompt = usecode->use_obj(obj, player->get_actor());
		player->subtract_movement_points(MOVE_COST_USE);
	}

	if (mode == USE_MODE && usecode->get_running_script() == NULL) // check mode because UseCode may have changed it
		endAction(display_prompt);
	return true;
}

bool Events::use(Actor *actor, uint16 x, uint16 y) {
	if (game->user_paused())
		return false;
	bool display_prompt = true;
	Obj *obj = actor->make_obj();

	if (!map_window->tile_is_black(x, y) && usecode->has_usecode(actor)) {
		if (game->get_game_type() == NUVIE_GAME_U6 && obj->obj_n == OBJ_U6_HORSE_WITH_RIDER)
			scroll->display_string("horse");
		else
			scroll->display_string(obj_manager->look_obj(obj));
		scroll->display_string("\n");

		MapCoord player_loc = player->get_actor()->get_location();
		MapCoord target = MapCoord(x, y, player_loc.z);

		if (player_loc.distance(target) > 1
		        && map_window->get_interface() == INTERFACE_NORMAL) {
			scroll->display_string("\nOut of range!\n");
			DEBUG(0, LEVEL_DEBUGGING, "distance to object: %d\n", player_loc.distance(target));
		} else if (!can_get_to_actor(actor, x, y))
			scroll->display_string("\nBlocked.\n");
		else {
			display_prompt = usecode->use_obj(obj, player->get_actor());
			player->subtract_movement_points(5);
		}
	} else {
		scroll->display_string("nothing\n");
		DEBUG(0, LEVEL_DEBUGGING, "Object %d:%d\n", obj->obj_n, obj->frame_n);
	}
// FIXME: usecode might request input, causing the obj to be accessed again,
// so we can't delete it in that case
	assert(mode == USE_MODE || game->user_paused());
	delete_obj(obj); // we were using an actor so free the temp Obj
	if (mode == USE_MODE) // check mode because UseCode may have changed it
		endAction(display_prompt);
	return (true);
}

bool Events::use(sint16 rel_x, sint16 rel_y) {
	map_window->centerCursor();
	map_window->moveCursorRelative(rel_x, rel_y);
	return use(map_window->get_cursorCoord());
}

bool Events::use(MapCoord coord) {
	if (game->user_paused())
		return false;

	if (!map_window->tile_is_black(coord.x, coord.y)) {
		Actor *actor = game->get_actor_manager()->get_actor(coord.x, coord.y, coord.z);
		Obj *obj = map_window->get_objAtCoord(coord, OBJ_SEARCH_TOP, OBJ_EXCLUDE_IGNORED, true);

		if (obj && obj->is_on_map() && map_window->tile_is_black(obj->x, obj->y, obj)) {
			Obj *bottom_obj = obj_manager->get_obj(obj->x, obj->y, obj->z, false);
			if (game->get_game_type() == NUVIE_GAME_U6 && bottom_obj->obj_n == OBJ_U6_SECRET_DOOR // hack for frame 2
			        && !map_window->tile_is_black(obj->x, obj->y, bottom_obj))
				obj = bottom_obj;
			else
				obj = NULL;
		}
		bool visible_actor = actor && actor->is_visible();

		if (obj && (!visible_actor || !usecode->has_usecode(actor)))
			return (use(obj));
		if (visible_actor) {
			return (use(actor, coord.x, coord.y));
		}
	}

	scroll->display_string("nothing\n");
	endAction(true);
	return true;
}

bool Events::look_start() {
	if (game->user_paused())
		return (false);
	get_target("Look-");
	return true;
}

/* Returns true if object can be searched. (false if prompt shouldn't be shown)
 */
bool Events::look(Obj *obj) {
	if (game->user_paused())
		return (false);

	if (obj) {
		if (game->get_game_type() == NUVIE_GAME_U6) {
			if (obj->obj_n == OBJ_U6_STATUE_OF_MONDAIN
			        || obj->obj_n == OBJ_U6_STATUE_OF_MINAX
			        || obj->obj_n == OBJ_U6_STATUE_OF_EXODUS) {
				Actor *actor = game->get_actor_manager()->get_actor(obj->quality);
				look(actor);
				return false;
			} else if (obj->obj_n == OBJ_U6_SPELLBOOK) {
				looking_at_spellbook = true;
				game->get_script()->call_look_obj(obj);
				Actor *reader = obj->get_actor_holding_obj();
				if (!reader)
					reader = player->get_actor();
				view_manager->close_all_gumps();
				view_manager->set_spell_mode(reader, obj, false);
				view_manager->get_current_view()->grab_focus();
				return false;
			}
		}
		obj_manager->print_obj(obj, false); // DEBUG
		/*      if(game->is_new_style())
		      {
		          new TextEffect(obj_manager->look_obj(obj, true), MapCoord((obj->x - map_window->get_cur_x())*16,(obj->y-map_window->get_cur_y())*16,obj->z));
		      }*/
		if (game->get_script()->call_look_obj(obj) == false) {
			scroll->display_prompt();
			return false;
		}
	}

	return true;
}

/* Returns true if there was a portrait for actor. */
bool Events::look(Actor *actor) {
	ActorManager *actor_manager = game->get_actor_manager();
	sint16 p_id = -1; // party member number of actor
	bool had_portrait = false;

	if (game->user_paused())
		return (false);

	if (actor->get_actor_num() != 0) {
		display_portrait(actor);
		had_portrait = view_manager->get_portrait_view()->get_waiting();
	}

	actor_manager->print_actor(actor); // DEBUG
	scroll->display_string("Thou dost see ");
	// show real actor name and portrait if in avatar's party
	if ((p_id = player->get_party()->get_member_num(actor)) >= 0)
		scroll->display_string(player->get_party()->get_actor_name(p_id));
	else
		scroll->display_string(actor_manager->look_actor(actor, true));
	scroll->display_string("\n");
	return (had_portrait);
}

bool Events::search(Obj *obj) {
	MapCoord player_loc = player->get_actor()->get_location(),
	         target_loc = map_window->get_cursorCoord();

	if (game->user_paused())
		return (false);

	if (obj->get_engine_loc() == OBJ_LOC_MAP && player_loc.distance(target_loc) <= 1) {
		scroll->display_string("\nSearching here, you find ");
		if (!usecode->search_obj(obj, player->get_actor()))
			scroll->display_string("nothing.\n");
		else {
			scroll->display_string(".\n");
			map_window->updateBlacking(); // secret doors
		}
		return (true);
	}
	return (false);
}

// looks at the whatever is at MapWindow cursor location
bool Events::lookAtCursor(bool delayed, uint16 x, uint16 y, uint8 z, Obj *obj, Actor *actor) {
	bool display_prompt = true;

	if (!delayed) {
		x = map_window->get_cursorCoord().x;
		y = map_window->get_cursorCoord().y;
		z = map_window->get_cursorCoord().z;
		obj = map_window->get_objAtCursor();
		actor = map_window->get_actorAtCursor();
	}

	if (obj && obj->is_on_map() && ((obj->status & OBJ_STATUS_INVISIBLE) || map_window->tile_is_black(x, y, obj))) {
		Obj *bottom_obj = obj_manager->get_obj(x, y, z, false);
		if (game->get_game_type() == NUVIE_GAME_U6 && bottom_obj->obj_n == OBJ_U6_SECRET_DOOR // hack for frame 2
		        && !map_window->tile_is_black(x, y, bottom_obj))
			obj = bottom_obj;
		else
			obj = NULL;
	}
	if (game->user_paused())
		return false;

	if (map_window->tile_is_black(x, y))
		scroll->display_string("Thou dost see darkness.\n");
	else if (actor && actor->is_visible())
		display_prompt = !look(actor);
	else if (obj) {
		if (look(obj))
			search(obj);
		else
			display_prompt = false;
	} else { // ground
		scroll->display_string("Thou dost see ");
		/*   if(game->is_new_style())
		     new TextEffect(game->get_game_map()->look(x, y, z), MapCoord((x - map_window->get_cur_x())*16,(y-map_window->get_cur_y())*16,z));*/
		scroll->display_string(game->get_game_map()->look(x, y, z));
		scroll->display_string("\n");
	}

	endAction(display_prompt);
	return true;
}

bool Events::pushTo(Obj *obj, Actor *actor) {
	bool ok = false;

	if (obj) {
		if (game->get_game_type() == NUVIE_GAME_SE || push_obj != obj)
			scroll->display_string(obj_manager->look_obj(obj));
		scroll->display_string("\n");

		if (obj_manager->can_store_obj(obj, push_obj)) {
			if (obj->is_in_inventory()) {
				Actor *src_actor = game->get_player()->get_actor();
				Actor *target_actor = obj->get_actor_holding_obj();
				if (can_move_obj_between_actors(push_obj, src_actor, target_actor, false))
					obj_manager->moveto_container(push_obj, obj);
				scroll->message("\n\n");
				endAction();
				return (true);
			}
			ok = obj_manager->moveto_container(push_obj, obj);
		}
	} else {
		if (actor) {
			Actor *src_actor;
			if (push_obj->is_in_inventory())
				src_actor = push_obj->get_actor_holding_obj();
			else
				src_actor = game->get_player()->get_actor();

			if (can_move_obj_between_actors(push_obj, src_actor, actor, true))
				obj_manager->moveto_inventory(push_obj, actor);
			scroll->message("\n\n");
			endAction();
			return (true);
		} else {
			scroll->message("nobody.\n\n");
			endAction();
			return false;
		}
	}

	if (!ok) {
		if (obj == push_obj) {
			if (game->get_game_type() == NUVIE_GAME_MD)
				scroll->display_string("\nAn item can't be placed inside itself!\n\n");
			else if (game->get_game_type() == NUVIE_GAME_SE)
				scroll->display_string("\nYou can't do that!\n\n");
			else if (obj->container)
				scroll->display_string("\nHow can a container go into itself!\n\n");
			else
				scroll->display_string("\nnot a container\n\n");
		} else if (game->get_game_type() == NUVIE_GAME_U6 && obj->obj_n == OBJ_U6_VORTEX_CUBE)
			scroll->display_string("\nOnly moonstones can go into the vortex cube.\n\n");
		else if (game->get_game_type() == NUVIE_GAME_U6 && obj->obj_n == OBJ_U6_SPELLBOOK) {
			if (push_obj->obj_n == OBJ_U6_SPELL)
				scroll->display_string("\nThe spellbook already has this spell.\n\n");
			else
				scroll->display_string("\nOnly spells can go into the spellbook.\n\n");
		} else if (game->get_game_type() == NUVIE_GAME_U6 && !obj->container)
			scroll->display_string("\nnot a container\n\n");
		else if (game->get_game_type() == NUVIE_GAME_U6)
			scroll->display_string("\nNot possible!\n\n");
		else
			scroll->display_string("\nYou can't do that!\n\n");
	}

	scroll->display_prompt();
	endAction();
	return (true);
}

/* Move selected object in direction relative to object.
 * (coordinates can be relative to player or object)
 */
bool Events::pushTo(sint16 rel_x, sint16 rel_y, bool push_from) {
	Tile *obj_tile;
	bool can_move = false; // some checks must determine if object can_move
	Map *map = game->get_game_map();
	MapCoord pusher = player->get_actor()->get_location();
	MapCoord from, to; // absolute locations: object, target
	sint16 pushrel_x, pushrel_y; // direction relative to object
	LineTestResult lt;
	Script *script = game->get_script();

	if (game->user_paused())
		return (false);

	if (!push_actor && !push_obj) {
		scroll->display_string("what?\n\n");
		scroll->display_prompt();
		endAction();
		return (false);
	}

	if (push_actor) {
		if (!push_actor->can_be_moved() || push_actor->get_tile_type() != ACTOR_ST) {
			scroll->display_string("Not possible\n\n");
			scroll->display_prompt();
			endAction();
			return false;
		}
		from = push_actor->get_location();
	} else {
		if (push_obj->is_on_map()) {
			from = MapCoord(push_obj->x, push_obj->y, push_obj->z);
		} else {
			// exchange inventory.
			Actor *src_actor = push_obj->get_actor_holding_obj();
			if (!src_actor) // container on the map (container gump)
				src_actor = player->get_actor();
//            if(src_actor)
			{
				Actor *target_actor = map->get_actor(rel_x, rel_y, src_actor->get_z());
				if (can_move_obj_between_actors(push_obj, src_actor, target_actor, true)) {
					obj_manager->moveto_inventory(push_obj, target_actor);
					script->call_actor_subtract_movement_points(src_actor, 5);
				}
			}
			scroll->message("\n\n");
			endAction();
			return (true);
		}
	}

	if (push_from == PUSH_FROM_PLAYER) { // coordinates must be converted
		to.x = pusher.x + rel_x;
		to.y = pusher.y + rel_y;
	} else {
		to.x = from.x + rel_x;
		to.y = from.y + rel_y;
	}
	pushrel_x = to.x - from.x;
	pushrel_y = to.y - from.y;

	if (map_window->get_interface() == INTERFACE_NORMAL || push_actor) {
		// you can only push one space at a time
		pushrel_x = (pushrel_x == 0) ? 0 : (pushrel_x < 0) ? -1 : 1;
		pushrel_y = (pushrel_y == 0) ? 0 : (pushrel_y < 0) ? -1 : 1;
	}
	to.x = from.x + pushrel_x;
	to.y = from.y + pushrel_y;
	to.z = from.z;

	scroll->display_string(get_direction_name(pushrel_x, pushrel_y));
	scroll->display_string(".\n\n");

	if (pushrel_x == 0 && pushrel_y == 0) {
		scroll->display_prompt();
		endAction();
		return true;
	}
	CanDropOrMoveMsg can_move_check;
	if (push_obj && (can_move_check = map_window->can_drop_or_move_obj(to.x, to.y, player->get_actor(), push_obj))
	        != MSG_SUCCESS) {
//        scroll->display_string("Blocked.\n");  // using text from can_drop_or_move_obj
		map_window->display_can_drop_or_move_msg(can_move_check, "");
		endAction(true);
		return true;
	}
	DEBUG(0, LEVEL_WARNING, "deduct moves from player\n");
	// FIXME: the random chance here is just made up, I don't know what
	//        kind of check U6 did ("Failed.\n\n")
	if (push_actor) {
		// if actor can take a step, do so; else 50% chance of pushing them
		if (push_actor == player->get_actor()) {
			if (player->check_walk_delay() && !view_manager->gumps_are_active()) {
				player->moveRelative(pushrel_x, pushrel_y);
				game->time_changed();
			}
		} else if (map->lineTest(to.x, to.y, to.x, to.y, to.z, LT_HitActors | LT_HitUnpassable, lt))
			scroll->display_string("Blocked.\n\n");
		else if (!push_actor->moveRelative(pushrel_x, pushrel_y)) {
			if (NUVIE_RAND() % 2) { // already checked if target is passable
				push_actor->move(to.x, to.y, from.z, ACTOR_FORCE_MOVE | ACTOR_IGNORE_DANGER);
				player->subtract_movement_points(5);
			} else
				scroll->display_string("Failed.\n\n");
		}
	} else {
		if (map_window->get_interface() != INTERFACE_IGNORE_BLOCK
		        && map_window->blocked_by_wall(player->get_actor(), push_obj)) {
			scroll->display_string("Blocked.\n\n");
		} else if (!usecode->has_movecode(push_obj) || usecode->move_obj(push_obj, pushrel_x, pushrel_y)) {
			if (game->get_game_type() == NUVIE_GAME_U6 && (push_obj->obj_n == OBJ_U6_SKIFF
			        || push_obj->obj_n == OBJ_U6_RAFT)) {
				Obj *to_obj = obj_manager->get_obj(to.x, to.y, to.z, true);
				if (to_obj) {
					if (obj_manager->can_store_obj(to_obj, push_obj))
						can_move = obj_manager->moveto_container(push_obj, to_obj);
				} else if (map->lineTest(to.x, to.y, to.x, to.y, to.z, LT_HitActors | LT_HitUnpassable, lt)) {
					if (!lt.hitActor && map->is_water(to.x, to.y, to.z))
						can_move = obj_manager->move(push_obj, to.x, to.y, to.z);
				} else
					can_move = obj_manager->move(push_obj, to.x, to.y, to.z);
			} else if (map_window->get_interface() != INTERFACE_IGNORE_BLOCK &&
			           map->lineTest(to.x,
			                         to.y,
			                         to.x,
			                         to.y,
			                         to.z,
			                         LT_HitActors | LT_HitUnpassable,
			                         lt,
			                         0,
			                         game->get_game_type() == NUVIE_GAME_U6 ? NULL
			                         : push_obj)) { //FIXME should we exclude push_obj for U6 too?
				if (lt.hitObj) {
					if (obj_manager->can_store_obj(lt.hitObj, push_obj)) { //if we are moving onto a container.
						can_move = obj_manager->moveto_container(push_obj, lt.hitObj);
					} else {
						// We can place an object on a bench or table. Or on any other object if
						// the object is passable and not on a boundary.

						obj_tile = obj_manager->get_obj_tile(lt.hitObj->obj_n, lt.hitObj->frame_n);
						if ((obj_tile->flags3 & TILEFLAG_CAN_PLACE_ONTOP) ||
						        (obj_tile->passable && !map->is_boundary(lt.hit_x, lt.hit_y, lt.hit_level))) {
							/* do normal move if no usecode or return from usecode was true */
							//if(!usecode->has_movecode(push_obj) || usecode->move_obj(push_obj,pushrel_x,pushrel_y))
							can_move = obj_manager->move(push_obj, to.x, to.y, from.z);
						}
					}
				}
			} else {
				Obj *obj = obj_manager->get_obj(to.x, to.y, to.z);
				if (map_window->get_interface() == INTERFACE_IGNORE_BLOCK
				        && map->get_actor(to.x, to.y, to.z)) {} // don't allow moving under actor
				else if (obj && obj_manager->can_store_obj(obj, push_obj)) { //if we are moving onto a container.
					can_move = obj_manager->moveto_container(push_obj, obj);
				} else {
					/* do normal move if no usecode or return from usecode was true */
					//if(!usecode->has_movecode(push_obj) || usecode->move_obj(push_obj,pushrel_x,pushrel_y))
					can_move = obj_manager->move(push_obj, to.x, to.y, from.z);
				}
			}
			if (!can_move)
				scroll->display_string("Blocked.\n\n");
		}
		if (can_move)
			player->subtract_movement_points(5);
	}
	scroll->display_prompt();
	endAction();
	return (true);
}

bool Events::pushFrom(Obj *obj) {
	scroll->display_string(obj_manager->look_obj(obj));
	push_obj = obj;
	if (game->get_game_type() == NUVIE_GAME_MD)
		get_target("\nWhere? ");
	else
		get_target("\nTo ");
	return true;
}

/* Select object to move. */
bool Events::pushFrom(sint16 rel_x, sint16 rel_y) {
	MapCoord from = player->get_actor()->get_location();
	MapCoord target = MapCoord(from.x + rel_x, from.y + rel_y, from.z);
	return pushFrom(target);
}

/* Select object to move. */
bool Events::pushFrom(MapCoord target) {
	ActorManager *actor_manager = game->get_actor_manager();
	Script *script = game->get_script();
	MapCoord from = player->get_actor()->get_location();

	if (game->user_paused())
		return (false);

	map_window->set_show_use_cursor(false);
	if (from.x != target.x || from.y != target.y) {
		push_obj = obj_manager->get_obj(target.x, target.y, from.z);
	}
	push_actor = actor_manager->get_actor(target.x, target.y, from.z);
	if (map_window->tile_is_black(target.x, target.y, push_obj)) {
		scroll->display_string("nothing.\n");
		endAction(true);
		return false;
	}
	if (push_obj
	        && (obj_manager->get_obj_weight(push_obj, OBJ_WEIGHT_EXCLUDE_CONTAINER_ITEMS) == 0))
		push_obj = NULL;

	if (push_actor && push_actor->is_visible()) {
		scroll->display_string(push_actor->get_name());
		push_obj = NULL;
	} else if (push_obj) {
		scroll->display_string(obj_manager->look_obj(push_obj));
		push_actor = NULL;
	} else {
		scroll->display_string("nothing.\n");
		endAction(true);
		return false;
	}

	if (from.distance(target) > 1 && !script->call_is_ranged_select(MOVE)
	        && map_window->get_interface() == INTERFACE_NORMAL) {
		scroll->display_string("\n\nOut of range!\n");
		endAction(true);
	} else if (map_window->get_interface() != INTERFACE_NORMAL
	           && ((push_obj && !map_window->can_get_obj(player->get_actor(), push_obj))
	               || (push_actor && !can_get_to_actor(push_actor, target.x, target.y)))) {
		scroll->display_string("\n\nCan't reach it\n");
		endAction(true);
	} else {
		get_direction(MapCoord(target.x, target.y), "\nTo ");
	}
	return true;
}

bool Events::actor_exists(Actor *a) {
	if (a->get_z() > 5 || a->get_actor_num() == 0
	        || ((a->is_temp() || a->get_strength() == 0) && a->get_x() == 0 && a->get_y() == 0
	            && a->get_z() == 0) // temp actor that has been cleaned up or invalid normal npc
	        /*|| strcmp(a->get_name(true), "Nothing") == 0*/) { // This last one probably isn't needed anymore
		scroll->display_string("\nnpc is invalid or at invalid location");
		return false;
	}
	return true;
}

/* Send input to active alt-code. */
void Events::alt_code_input(const char *in) {
	ActorManager *am = game->get_actor_manager();
	Actor *a = am->get_actor((uint8) strtol(in, NULL, 10));
	static string teleport_string = "";
	static Obj obj;
	uint8 a_num = 0;
	switch (active_alt_code) {
	case 300: // show NPC portrait (FIXME: should be show portrait number)
		if (a) {
			am->print_actor(a); //print actor debug info
			display_portrait(a);
		}
		scroll->display_string("\n");
		active_alt_code = 0;
		break;

	case 301: // Show Midgame graphics
		game->get_script()->call_play_midgame_sequence((uint16) strtol(in, NULL, 10));
		scroll->display_string("\n");
		active_alt_code = 0;
		break;

	case 400: // talk to NPC (FIXME: get portrait and inventory too)
		a_num = (uint8) strtol(in, NULL, 10);
		if (a_num == 0 || !game->get_converse()->start(a_num)) {
			scroll->display_string("\n");
			scroll->display_prompt();
		}
		active_alt_code = 0;
		break;

	/*        case 214:
	            alt_code_teleport(in); //teleport player & party to location string
	            scroll->display_string("\n");
	            scroll->display_prompt();
	            active_alt_code = 0;
	            break;
	*/

	case 214: // teleport player & party to location string
		teleport_string += " ";
		teleport_string += in;
		++alt_code_input_num;
		if (alt_code_input_num == 1) {
			if (game->get_game_type() == NUVIE_GAME_U6)
				scroll->display_string("\n<uai>: ");
			else
				scroll->display_string("\ny: ");
			get_scroll_input(NULL, true, false, false);
		} else if (alt_code_input_num == 2) {
			if (game->get_game_type() == NUVIE_GAME_U6)
				scroll->display_string("\n<zi>: ");
			else
				scroll->display_string("\nz: ");
			get_scroll_input(NULL, true, false, false);
		} else {
			alt_code_teleport(teleport_string.c_str());
			scroll->display_string("\n");
			scroll->display_prompt();
			teleport_string = "";
			alt_code_input_num = 0;
			active_alt_code = 0;
		}
		break;

	case 314: // teleport player & party to selected location
		if (strtol(in, NULL, 10) != 0)
			alt_code_teleport_menu((uint32) strtol(in, NULL, 10));
		if (strtol(in, NULL, 10) == 0 || alt_code_input_num > 2) {
			scroll->display_string("\n");
			scroll->display_prompt();
			alt_code_input_num = 0;
			active_alt_code = 0;
		}
		break;

	case 414: // teleport player & party to NPC location
		if (actor_exists(a))
			alt_code_teleport_to_person((uint32) strtol(in, NULL, 10));
		scroll->display_string("\n\n");
		scroll->display_prompt();
		active_alt_code = 0;
		break;

	case 500: // control/watch anyone
		if (!actor_exists(a)) {
			scroll->display_string("\n\n");
		} else if (!a->is_alive()) {
			scroll->display_string("\n");
			scroll->display_string(a->get_name(true));
			scroll->display_string(" is dead\n\n");
		} else {
			player->set_actor(a);
			player->set_mapwindow_centered(true);
			if (!game->is_new_style())
				view_manager->set_inventory_mode(); // reset inventoryview
			if (game->get_party()->contains_actor(player->get_actor())) {
				in_control_cheat = false;
				uint8 member_num = game->get_party()->get_member_num(player->get_actor());
				if (!game->is_new_style())
					view_manager->get_inventory_view()->set_party_member(member_num);
			} else {
				in_control_cheat = true;
				if (!game->is_new_style())
					view_manager->get_inventory_view()->set_actor(player->get_actor());
			}
			game->get_party()->update_light_sources();
			scroll->display_string("\n\n");
		}
		scroll->display_prompt();
		active_alt_code = 0;
		break;

	case 501: { // resurrect npc
		if (!actor_exists(a)) {
			// Do nothing. It already prints a message
		} else if (a->is_alive()) {
			scroll->display_string("\n");
			scroll->display_string(a->get_name(true));
			scroll->display_string(" is not dead.");
		} else {
			bool failed = true;
			for (int i = 1; i < 8; i++) {
				uint16 newx = NUVIE_RAND() % 10 + player->get_actor()->get_x() - 5;
				uint16 newy = NUVIE_RAND() % 10 + player->get_actor()->get_y() - 5;
				if (a->move(newx, newy, player->get_actor()->get_z())) {
					failed = false;
					MapCoord res_loc(newx, newy, player->get_actor()->get_z());
					a->resurrect(res_loc);
				}
			}
			if (failed) // No location found. Resurrect anyway.
				a->resurrect(player->get_actor()->get_location());
		}
		scroll->display_string("\n\n");
		scroll->display_prompt();
		active_alt_code = 0;
		break;
	}
	case 456: // polymorph
		if (alt_code_input_num == 0) {
			obj.obj_n = strtol(in, NULL, 10);
			scroll->display_string("\nNpc number? ");
			get_scroll_input();
			++alt_code_input_num;
		} else {
			a->morph(obj.obj_n);
			scroll->display_string("\nMorphed!\n\n");
			scroll->display_prompt();
			alt_code_input_num = 0;
			active_alt_code = 0;
		}
		break;
	}
}

/* Get an alt-code from `cs' and use it.
 */
void Events::alt_code(const char *cs) {
	uint16 c = (uint16) strtol(cs, NULL, 10);
	switch (c) {
	case 300: // display portrait by number
		scroll->display_string("Portrait? ");
		get_scroll_input();
		active_alt_code = c;
		break;

	case 301: // display midgame sequence
		scroll->display_string("Midgame? ");
		get_scroll_input();
		active_alt_code = c;
		break;

	case 400: // talk to anyone (FIXME: get portrait and inventory too)
		scroll->display_string("Npc number? ");
		get_scroll_input();
		active_alt_code = c;
		break;

	case 500: // control/watch anyone
		if (player->is_in_vehicle()
		        || game->get_party()->is_in_combat_mode()) {
			if (player->is_in_vehicle())
				display_not_aboard_vehicle(false);
			else
				scroll->display_string("\nNot while in combat mode!\n\n");
			scroll->display_prompt();
			active_alt_code = 0;
			break;
		}
		scroll->display_string("Npc number? ");
		get_scroll_input();
		active_alt_code = c;
		break;

	case 501: // resurrect npc
		if (player->is_in_vehicle()) {
			display_not_aboard_vehicle(false);
			scroll->display_prompt();
			active_alt_code = 0;
			break;
		}
		scroll->display_string("Npc number? ");
		get_scroll_input();
		active_alt_code = c;
		break;

	case 456: // polymorph
		scroll->display_string("Object number? ");
		get_scroll_input();
		active_alt_code = c;
		break;

	case 213:
		alt_code_infostring();
		active_alt_code = 0;
		break;

	/*        case 214:
	            scroll->display_string("Location: \n",2);
	            scroll->display_string(" ",0);
	            get_scroll_input();
	            active_alt_code = c;
	            break;
	*/
	case 214:
		if (player->is_in_vehicle()) {
			if (game->get_game_type() == NUVIE_GAME_U6)
				scroll->display_string("\n<nat uail abord wip!>\n");
			else
				display_not_aboard_vehicle();
			scroll->display_prompt();
			active_alt_code = 0;
		} else {
			if (game->get_game_type() == NUVIE_GAME_U6)
				scroll->display_string("\n<gotu eks>: ");
			else
				scroll->display_string("\ngoto x: ");
			get_scroll_input(NULL, true, false, false);
			active_alt_code = c;
		}
		break;

	case 215:
		//clock->advance_to_next_hour();
		game->get_script()->call_advance_time(60);
		scroll->display_string(clock->get_time_string());
		scroll->display_string("\n");
		scroll->display_prompt();
		game->time_changed();
		active_alt_code = 0;
		break;

	case 216:
		scroll->display_string(clock->get_time_string());
		scroll->display_string("\n");
		scroll->display_prompt();
		active_alt_code = 0;
		break;
	case 222: {
		bool ethereal = !game->is_ethereal();
		game->set_ethereal(ethereal);
		game->get_party()->set_ethereal(ethereal);
		const char *message = ethereal ? "Party desolidifies!\n\n" : "Party solidifies!\n\n";
		scroll->message(message);
		break;
	}
	case 314: // teleport player & party to selected location
		if (player->is_in_vehicle()) {
			display_not_aboard_vehicle();
			active_alt_code = 0;
		} else {
			alt_code_teleport_menu(0);
			active_alt_code = c;
		}
		break;

	case 414: // teleport player & party to NPC location
		if (player->is_in_vehicle()) {
			display_not_aboard_vehicle();
			active_alt_code = 0;
			break;
		}
		scroll->display_string("Npc number? ");
		get_scroll_input();
		active_alt_code = c;
		break;

	case 600: // map editor
		view_manager->open_mapeditor_view();
		active_alt_code = 0;
		break;

	default: // attempt to handle the altcode with lua script.
		Game::get_game()->get_script()->call_handle_alt_code(c);
		scroll->display_prompt();
		break;
	}
}

bool Events::alt_code_teleport(const char *location_string) {
	char *next_num;
	uint16 x, y, z;

	if (!location_string || !strlen(location_string))
		return false;

	x = strtol(location_string, &next_num, 16);
	y = strtol(next_num, &next_num, 16);
	z = strtol(next_num, &next_num, 16);

	if ((x == 0 && y == 0) || z > 5)
		return false;
	player->move(x, y, z, true);

	// This is a bit of a hack but we would like to update the music when teleporting.
	game->get_party()->update_music();

	return true;
}

// changed to show time instead of date (SB-X)
void Events::alt_code_infostring() {
	char buf[14]; // kkhhmmxxxyyyz
	uint8 karma;
	uint8 hour;
	uint8 minute;
	uint16 x, y;
	uint8 z;

	karma = player->get_karma();
	player->get_location(&x, &y, &z);

	hour = clock->get_hour();
	minute = clock->get_minute();

	sprintf(buf, "%02d%02d%02d%03X%03X%x", karma, hour, minute, x, y, z);

	scroll->display_string(buf);
	scroll->display_string("\n");
	new PeerEffect((x - x % 8) - 18, (y - y % 8) - 18, z); // wrap to chunk boundary, and center
	// in 11x11 MapWindow
}

/* Move player to NPC location. */
bool Events::alt_code_teleport_to_person(uint32 npc) {
	ActorManager *actor_manager = game->get_actor_manager();
	MapCoord actor_location = actor_manager->get_actor(npc)->get_location();
	player->move(actor_location.x, actor_location.y, actor_location.z, true);
	if (!actor_manager->toss_actor(player->get_actor(), 2, 2))
		actor_manager->toss_actor(player->get_actor(), 4, 4);
	return (true);
}

/* Display teleport destinations, get input. */
void Events::alt_code_teleport_menu(uint32 selection) {
	static uint8 category = 0;
	const char *teleport_dest = "";
	if (alt_code_input_num == 0) { // select category
		if (game->get_game_type() == NUVIE_GAME_U6) {
			scroll->display_string("\nLazy Teleporters' Menu!\n");
			scroll->display_string(" 1) Cities\n");
			scroll->display_string(" 2) Major Areas\n");
			scroll->display_string(" 3) Shrines\n");
			scroll->display_string(" 4) Gargoyles\n");
			scroll->display_string(" 5) Dungeons\n");
			scroll->display_string(" 6) More Dungeons\n");
			scroll->display_string(" 7) Other\n");
			scroll->display_string("Category? ");
			get_scroll_input("01234567");
		} else if (game->get_game_type() == NUVIE_GAME_SE) {
			scroll->display_string("\nLazy Teleporters' Menu!\n");
			scroll->display_string(" 1) Villages\n");
			scroll->display_string(" 2) More Villages\n");
			scroll->display_string(" 3) S. Places\n");
			scroll->display_string(" 4) Resources\n");
			scroll->display_string(" 5) Teleport Pads\n");
			scroll->display_string(" 6) Caves\n");
			scroll->display_string(" 7) Myrm. Holes\n");
			scroll->display_string("Category? ");
			get_scroll_input("01234567");
		}
	} else if (alt_code_input_num == 1) { // selected category, select location
		category = selection;
		scroll->display_string("\n");
		if (game->get_game_type() == NUVIE_GAME_U6) {
			switch (selection) {
			case 1:
				scroll->display_string("Cities\n");
				scroll->display_string(" 1) Britain\n");
				scroll->display_string(" 2) Trinsic\n");
				scroll->display_string(" 3) Yew\n");
				scroll->display_string(" 4) Minoc\n");
				scroll->display_string(" 5) Moonglow\n");
				scroll->display_string(" 6) Jhelom\n");
				scroll->display_string(" 7) Skara Brae\n");
				scroll->display_string(" 8) New Magincia\n");
				if (!game->is_new_style())
					scroll->display_string(" 9) Buc's Den\n");
				else
					scroll->display_string(" 9) Buccaneer's Den\n");
				scroll->display_string("Location? ");
				get_scroll_input("0123456789");
				break;
			case 2:
				scroll->display_string("Major Areas\n");
				scroll->display_string(" 1) Cove\n");
				scroll->display_string(" 2) Paws\n");
				scroll->display_string(" 3) Serpent's Hold\n");
				scroll->display_string(" 4) Empath Abbey\n");
				scroll->display_string(" 5) Lycaeum\n");
				scroll->display_string(" 6) Library\n");
				scroll->display_string(" 7) Sutek's Island\n");
				scroll->display_string(" 8) Stonegate\n");
				scroll->display_string(" 9) The Codex\n");
				scroll->display_string("Location? ");
				get_scroll_input("0123456789");
				break;
			case 3:
				scroll->display_string("Shrines\n");
				scroll->display_string(" 1) Honesty\n");
				scroll->display_string(" 2) Compassion\n");
				scroll->display_string(" 3) Valor\n");
				scroll->display_string(" 4) Justice\n");
				scroll->display_string(" 5) Sacrifice\n");
				scroll->display_string(" 6) Honor\n");
				scroll->display_string(" 7) Humility\n");
				scroll->display_string(" 8) Spirituality\n");
				scroll->display_string("Location? ");
				get_scroll_input("012345678");
				break;
			case 4:
				scroll->display_string("Gargoyles\n");
				if (!game->is_new_style())
					scroll->display_string(" 1) Hall\n");
				else
					scroll->display_string(" 1) Hall of Knowledge\n");
				scroll->display_string(" 2) Singularity\n");
				scroll->display_string(" 3) King's Temple\n");
				scroll->display_string(" 4) Tomb of Kings\n");
				scroll->display_string(" 5) Hythloth\n");
				scroll->display_string(" 6) Control\n");
				scroll->display_string(" 7) Passion\n");
				scroll->display_string(" 8) Diligence\n");
				scroll->display_string("Location? ");
				get_scroll_input("012345678");
				break;
			case 5:
				scroll->display_string("Dungeons\n");
				scroll->display_string(" 1) Ant Mound\n");
				if (!game->is_new_style())
					scroll->display_string(" 2) Buc's Cave\n");
				else
					scroll->display_string(" 2) Buccaneer's Cave\n");
				scroll->display_string(" 3) Covetous\n");
				scroll->display_string(" 4) Crypts\n");
				scroll->display_string(" 5) Cyclops Cave\n");
				scroll->display_string(" 6) Deceit\n");
				scroll->display_string(" 7) Despise\n");
				scroll->display_string(" 8) Destard\n");
				if (!game->is_new_style())
					scroll->display_string(" 9) Heftimus's\n");
				else
					scroll->display_string(" 9) Heftimus's Cave\n");
				scroll->display_string("Location? ");
				get_scroll_input("0123456789");
				break;
			case 6:
				scroll->display_string("More Dungeons\n");
				scroll->display_string(" 1) Hero's Hole\n");
				scroll->display_string(" 2) Hythloth\n");
				scroll->display_string(" 3) Pirate Cave\n");
				scroll->display_string(" 4) Sewers\n");
				scroll->display_string(" 5) Shame\n");
				scroll->display_string(" 6) Spider Cave\n");
				scroll->display_string(" 7) Sutek's Island\n");
				scroll->display_string(" 8) Swamp Cave\n");
				scroll->display_string(" 9) Wrong\n");
				scroll->display_string("Location? ");
				get_scroll_input("0123456789");
				break;
			case 7:
				scroll->display_string("Other\n");
				scroll->display_string(" 1) Iolo's Hut\n");
				scroll->display_string(" 2) Lumberjack\n");
				scroll->display_string(" 3) Saw Mill\n");
				scroll->display_string(" 4) Thieves Guild\n");
				scroll->display_string(" 5) Wisps\n");
				scroll->display_string(" 6) Dagger Isle\n");
				scroll->display_string(" 7) Shipwreck\n");
				scroll->display_string(" 8) Phoenix\n");
				scroll->display_string("Location? ");
				get_scroll_input("012345678");
				break;
			}
		} else if (game->get_game_type() == NUVIE_GAME_SE) {
			switch (selection) {
			case 1:
				scroll->display_string("Villages\n");
				scroll->display_string(" 1) Barako\n");
				scroll->display_string(" 2) Kurak\n");
				scroll->display_string(" 3) Pindiro\n");
				scroll->display_string(" 4) Yolaru\n");
				scroll->display_string(" 5) Tichticatl\n");
				scroll->display_string(" 6) Jukari\n");
				scroll->display_string(" 7) Disquiqui\n");
				scroll->display_string(" 8) Barrab\n");
				scroll->display_string(" 9) Urali\n");
				scroll->display_string("Location? ");
				get_scroll_input("0123456789");
				break;
			case 2:
				scroll->display_string("More Villages\n");
				scroll->display_string(" 1) Haakur\n");
				scroll->display_string(" 2) Sakkhra\n");
				scroll->display_string(" 3) Old Pindiro\n");
				scroll->display_string("Location? ");
				get_scroll_input("0123");
				break;
			case 3:
				scroll->display_string("Special Places\n");
				scroll->display_string(" 1) Laboratory\n");
				scroll->display_string(" 2) Drum Hill\n");
				scroll->display_string(" 3) Topuru's Isle\n");
				scroll->display_string(" 4) Gem Stand\n");
				if (!game->is_new_style())
					scroll->display_string(" 5) Thunderer\n");
				else
					scroll->display_string(" 5) Thunderer Peak\n");
				scroll->display_string(" 6) Great Mesa\n");
				scroll->display_string(" 7) Kotl City\n");
				scroll->display_string(" 8) Disq. Tyran.\n");
				scroll->display_string(" 9) Silverback\n");
				scroll->display_string("Location? ");
				get_scroll_input("0123456789");
				break;
			case 4:
				scroll->display_string("Resources\n");
				scroll->display_string(" 1) Sulphur Pits\n");
				scroll->display_string(" 2) Tar Pits\n");
				scroll->display_string(" 3) Pot.Nitrate\n");
				scroll->display_string(" 4) Yucca Plants\n");
				scroll->display_string(" 5) Bamboo\n");
				scroll->display_string(" 6) River Banks\n");
				scroll->display_string(" 7) Corn Stalks\n");
				scroll->display_string(" 8) Blue Stone\n");
				scroll->display_string("Location? ");
				get_scroll_input("012345678");
				break;
			case 5:
				scroll->display_string("Teleport Pads\n");
				scroll->display_string(" 1) Barako\n");
				scroll->display_string(" 2) Dead Pad\n");
				if (!game->is_new_style())
					scroll->display_string(" 3) K./Y.\n");
				else
					scroll->display_string(" 3) Kurak/Yolaru\n");
				scroll->display_string(" 4) Nahuatla\n");
				scroll->display_string(" 5) Jukari\n");
				scroll->display_string(" 6) Disquiqui\n");
				scroll->display_string(" 7) Barrab\n");
				scroll->display_string(" 8) Sakkhra\n");
				scroll->display_string(" 9) Hub\n");
				scroll->display_string("Location? ");
				get_scroll_input("0123456789");
				break;
			case 6:
				scroll->display_string("Caves\n");
				scroll->display_string(" 1) Spider\n");
				scroll->display_string(" 2) Jukari Ritual\n");
				scroll->display_string(" 3) Silverback\n");
				scroll->display_string(" 4) Fritz\n");
				scroll->display_string(" 5) Urali Spirit\n");
				scroll->display_string(" 6) Urali Chief\n");
				scroll->display_string(" 7) To Urali\n");
				scroll->display_string(" 8) From Urali\n");
				scroll->display_string(" 9) Denys\n");
				scroll->display_string("Location? ");
				get_scroll_input("0123456789");
				break;
			case 7:
				scroll->display_string("Myrmidex Holes\n");
				if (!game->is_new_style())
					scroll->display_string(" 1) S of Disq.\n");
				else
					scroll->display_string(" 1) S of Disquiqui\n");
				if (!game->is_new_style())
					scroll->display_string(" 2) W of G. Mesa\n");
				else
					scroll->display_string(" 2) W of Great Mesa\n");
				scroll->display_string(" 3) W of Hub\n");
				if (!game->is_new_style())
					scroll->display_string(" 4) E of Drum H.\n");
				else
					scroll->display_string(" 4) E of Drum Hill\n");
				scroll->display_string(" 5) SW of Kurak\n");
				scroll->display_string(" 6) Old Pindiro\n");
				scroll->display_string(" 7) S of Pindiro\n");
				scroll->display_string("Location? ");
				get_scroll_input("01234567");
				break;
			}
		}
	} else if (alt_code_input_num == 2) { // selected location, teleport
		if (game->get_game_type() == NUVIE_GAME_U6) {
			switch (category) {
			case 1:
				if (selection == 1) // Britain
					teleport_dest = "133 1a3 0";
				else if (selection == 2) // Trinsic
					teleport_dest = "19b 2e2 0";
				else if (selection == 3) // Yew
					teleport_dest = "ec a7 0";
				else if (selection == 4) // Minoc
					teleport_dest = "254 63 0";
				else if (selection == 5) // Moonglow
					teleport_dest = "38a 203 0";
				else if (selection == 6) // Jhelom
					teleport_dest = "a0 36b 0";
				else if (selection == 7) // Skara Brae
					teleport_dest = "54 203 0";
				else if (selection == 8) // New Magincia
					teleport_dest = "2e3 2ab 0";
				else if (selection == 9) // Buc's Den
					teleport_dest = "246 274 0";
				break;
			case 2:
				if (selection == 1) // Cove
					teleport_dest = "223 163 0";
				else if (selection == 2) // Paws
					teleport_dest = "198 264 0";
				else if (selection == 3) // Serpent's Hold
					teleport_dest = "22e 3bc 0";
				else if (selection == 4) // Empath Abbey
					teleport_dest = "83 db 0";
				else if (selection == 5) // Lycaeum
					teleport_dest = "37b 1a4 0";
				else if (selection == 6) // Library
					teleport_dest = "37b 1b4 0";
				else if (selection == 7) // Sutek's Island
					teleport_dest = "316 3d4 0";
				else if (selection == 8) // Stonegate
					teleport_dest = "25f 11d 0";
				else if (selection == 9) // The Codex
					teleport_dest = "39b 354 0";
				break;
			case 3:
				if (selection == 1) // Honesty
					teleport_dest = "3a7 109 0";
				else if (selection == 2) // Compassion
					teleport_dest = "1f7 168 0";
				else if (selection == 3) // Valor
					teleport_dest = "9f 3b1 0";
				else if (selection == 4) // Justice
					teleport_dest = "127 28 0";
				else if (selection == 5) // Sacrifice
					teleport_dest = "33e a6 0";
				else if (selection == 6) // Honor
					teleport_dest = "147 339 0";
				else if (selection == 7) // Humility
					teleport_dest = "397 3a8 0";
				else if (selection == 8) // Spirituality
					teleport_dest = "18 16 1";
				break;
			case 4:
				if (selection == 1) // Hall of Knowledge
					teleport_dest = "7f af 5";
				else if (selection == 2) // Temple of Singularity
					teleport_dest = "7f 37 5";
				else if (selection == 3) // Temple of Kings
					teleport_dest = "7f 50 5";
				else if (selection == 4) // Tomb of Kings
					teleport_dest = "7f 9 4";
				else if (selection == 5) // Hythloth exit
					teleport_dest = "dc db 5";
				else if (selection == 6) // Shrine of Control
					teleport_dest = "43 2c 5";
				else if (selection == 7) // Shrine of Passion
					teleport_dest = "bc 2c 5";
				else if (selection == 8) // Shrine of Diligence
					teleport_dest = "6c dc 5";
				break;
			case 5:
				if (selection == 1) // Ant Mound
					teleport_dest = "365 bb 0";
				else if (selection == 2) // Buc's Cave
					teleport_dest = "234 253 0";
				else if (selection == 3) // Covetous
					teleport_dest = "273 73 0";
				else if (selection == 4) // Crypts
					teleport_dest = "364 15a 0";
				else if (selection == 5) // Cyclops Cave
					teleport_dest = "b9 1b5 0";
				else if (selection == 6) // Deceit
					teleport_dest = "3c4 136 0";
				else if (selection == 7) // Despise
					teleport_dest = "16D 10a 0";
				else if (selection == 8) // Destard
					teleport_dest = "11c 292 0";
				else if (selection == 9) // Heftimus's
					teleport_dest = "84 35b 0";
				break;
			case 6:
				if (selection == 1) // Hero's Hole
					teleport_dest = "15c 32a 0";
				else if (selection == 2) // Hythloth
					teleport_dest = "3b4 3a4 0";
				else if (selection == 3) // Pirate Cave
					teleport_dest = "2c3 342 0";
				else if (selection == 4) // Sewers
					teleport_dest = "123 17a 0";
				else if (selection == 5) // Shame
					teleport_dest = "eb 19b 0";
				else if (selection == 6) // Spider Cave
					teleport_dest = "5c fb 0";
				else if (selection == 7) // Sutek's Island
					teleport_dest = "316 3d4 0";
				else if (selection == 8) // Swamp Cave
					teleport_dest = "263 16c 0";
				else if (selection == 9) // Wrong
					teleport_dest = "1f4 53 0";
				break;
			case 7:
				if (selection == 1) // Iolo's Hut
					teleport_dest = "c3 e8 0";
				else if (selection == 2) // Lumberjack (Yew)
					teleport_dest = "b2 94 0";
				else if (selection == 3) // Saw Mill (Minoc)
					teleport_dest = "2a4 65 0";
				else if (selection == 4) // Thieves Guild
					teleport_dest = "233 25e 0";
				else if (selection == 5) // Wisps
					teleport_dest = "a5 115 0";
				else if (selection == 6) // Dagger Island
					teleport_dest = "3a9 d3 0";
				else if (selection == 7) // Shipwreck
					teleport_dest = "1aa 3a6 0";
				else if (selection == 8) // Phoenix
					teleport_dest = "76 46 3";
				break;
			}
		} else if (game->get_game_type() == NUVIE_GAME_SE) {
			// Modifications needed when collision working
			// Currently NPC's end in 'bad spots' on some locations
			switch (category) {
			case 1:
				if (selection == 1) // Barako
					teleport_dest = "153 d1 0";
				else if (selection == 2) // Kurak
					teleport_dest = "19c 11a 0";
				else if (selection == 3) // Pindiro
					teleport_dest = "244 7f 0";
				else if (selection == 4) // Yolaru
					teleport_dest = "24b 142 0";
				else if (selection == 5) // Tichticatl
					teleport_dest = "242 22f 0";
				else if (selection == 6) // Jukari
					teleport_dest = "2ad 331 0";
				else if (selection == 7) // Disquiqui
					teleport_dest = "17d 228 0";
				else if (selection == 8) // Barrab
					teleport_dest = "f3 27a 0";
				else if (selection == 9) // Urali
					teleport_dest = "3e5 157 0";
				break;
			case 2:
				if (selection == 1) // Haakur
					teleport_dest = "34d 28e 0";
				else if (selection == 2) // Sakkhra
					teleport_dest = "6c 25e 0";
				else if (selection == 3) // Old Pindiro
					teleport_dest = "18e 2f 0";
				break;
			case 3:
				if (selection == 1) // Laboratory
					teleport_dest = "1db 18a 0";
				else if (selection == 2) // Drum Hill
					teleport_dest = "216 1c8 0";
				else if (selection == 3) // Topuru's Isle
					teleport_dest = "10e b9 0";
				else if (selection == 4) // Gem Stand
					teleport_dest = "a9 1e4 0";
				else if (selection == 5) // Thunderer Peak
					teleport_dest = "d8 192 0";
				else if (selection == 6) // Great Mesa
					teleport_dest = "c2 210 0";
				else if (selection == 7) // Kotl City ??? Approx
					teleport_dest = "bd 1c9 0";
				else if (selection == 8) // Disq. Tyran.
					teleport_dest = "1a6 249 0";
				else if (selection == 9) // Silverback
					teleport_dest = "110 49 0";
				break;
			case 4:
				if (selection == 1) // Sulphur Pits
					teleport_dest = "2da 2bb 0";
				else if (selection == 2) // Tar Pits
					teleport_dest = "1c3 150 0";
				else if (selection == 3) // Pot.Nitrate
					teleport_dest = "2dd 19e 0";
				else if (selection == 4) // Yucca Plants
					teleport_dest = "19f 115 0";
				else if (selection == 5) // Bamboo
					teleport_dest = "2fe 23e 0";
				else if (selection == 6) // River Banks
					teleport_dest = "253 5e 0";
				else if (selection == 7) // Corn Stalks
					teleport_dest = "18b 221 0";
				else if (selection == 8) // Blue Stone
					teleport_dest = "a8 259 0";
				break;
			case 5:
				if (selection == 1) // Barako
					teleport_dest = "178 ac 0";
				else if (selection == 2) // Dead Pad
					teleport_dest = "198 1e 0";
				else if (selection == 3) // Kurak/Yolaru
					teleport_dest = "216 11d 0";
				else if (selection == 4) // Nahuatla
					teleport_dest = "26b 259 0";
				else if (selection == 5) // Jukari
					teleport_dest = "2ba 306 0";
				else if (selection == 6) // Disquiqui
					teleport_dest = "171 25b 0";
				else if (selection == 7) // Barrab
					teleport_dest = "ce 26a 0";
				else if (selection == 8) // Sakkhra
					teleport_dest = "67 266 0";
				else if (selection == 9) // Hub
					teleport_dest = "b8 1c6 0";
				break;
			case 6:
				if (selection == 1) // Spider
					teleport_dest = "389 2ed 0";
				else if (selection == 2) // Jukari Ritual
					teleport_dest = "3a1 34d 0";
				else if (selection == 3) // Silverback
					teleport_dest = "123 45 0";
				else if (selection == 4) // Fritz
					teleport_dest = "1f1 4d 0";
				else if (selection == 5) // Urali Spirit
					teleport_dest = "3bc ec 0";
				else if (selection == 6) // Urali Chief
					teleport_dest = "3db 19e 0";
				else if (selection == 7) // To Urali
					teleport_dest = "2ad 176 0";
				else if (selection == 8) // From Urali
					teleport_dest = "335 15e 0";
				else if (selection == 9) // Denys
					teleport_dest = "2d5 19e 0";
				break;
			case 7:
				if (selection == 1) // S of Disquiqui
					teleport_dest = "15e 277 0";
				else if (selection == 2) // W of Great Mesa
					teleport_dest = "8d 1fc 0";
				else if (selection == 3) // W of Hub
					teleport_dest = "8d 1ca 0";
				else if (selection == 4) // E of Drum Hill
					teleport_dest = "27b 1d3 0";
				else if (selection == 5) // SW of Kurak
					teleport_dest = "173 14f 0";
				else if (selection == 6) // Old Pindiro
					teleport_dest = "189 45 0";
				else if (selection == 7) // S of Pindiro
					teleport_dest = "257 dc 0";
				break;
			}
		}
		if (strlen(teleport_dest)) {
			scroll->display_string("\n(");
			scroll->display_string(teleport_dest);
			scroll->display_string(")\n");
			alt_code_teleport(teleport_dest);
		}
	}
	++alt_code_input_num;
}

void Events::wait() {
	if (!ignore_timeleft)
		g_system->delayMillis(TimeLeft());
}

//Protected

inline uint32 Events::TimeLeft() {
	static uint32 next_time = 0;
	uint32 now;

	now = clock->get_ticks();
	if (fps_counter == 60) {
		fps_counter = 0;
		float fps = 1000 / ((float)(now - fps_timestamp) / 60);
		//printf("FPS: %f %d\n", fps, (uint32)(now - fps_timestamp));
		fps_counter_widget->setFps(fps);
		fps_timestamp = now;
	} else
		fps_counter++;

	if (next_time <= now) {
		next_time = now + NUVIE_INTERVAL;
		return (0);
	}
	uint32 delay = next_time - now;
	next_time += NUVIE_INTERVAL;
	return (delay);
}

void Events::toggleFpsDisplay() {
	if (fps_counter_widget->Status() == WIDGET_VISIBLE)
		fps_counter_widget->Hide();
	else
		fps_counter_widget->Show();
	if (!game->is_new_style())
		game->get_gui()->force_full_redraw();
}

void Events::quitDialog() {
	GUI_Widget *quit_dialog;
	if (mode == MOVE_MODE || mode == EQUIP_MODE) {
		map_window->set_looking(false);
		map_window->set_walking(false);
		showingDialog = true;

		close_gumps();
		uint16 x_off = game->get_game_x_offset();
		uint16 y_off = game->get_game_y_offset();

		x_off += (game->get_game_width() - 170) / 2;
		y_off += (game->get_game_height() - 80) / 2;
		quit_dialog = (GUI_Widget *) new GUI_YesNoDialog(gui,
		              x_off,
		              y_off,
		              170,
		              80,
		              "Do you want to Quit",
		              this,
		              this);

		gui->AddWidget(quit_dialog);
		gui->lock_input(quit_dialog);
	}

	return;
}

void Events::gameMenuDialog() {
	if (mode == MOVE_MODE && !view_manager->gumps_are_active()) {
		showingDialog = true;
		map_window->set_looking(false);
		map_window->set_walking(false);
		gamemenu_dialog = new GameMenuDialog(this);
		gui->AddWidget(gamemenu_dialog);
		gui->lock_input(gamemenu_dialog);
		keybinder->set_enable_joy_repeat(false);
	} else
		cancelAction();
}

uint16 Events::callback(uint16 msg, CallBack *caller, void *data) {
	GUI_Widget *widget;

	switch (msg) { // Handle callback from quit dialog.
	case YESNODIALOG_CB_YES :
		showingDialog = false;
		game->get_gui()->unlock_input();
		return GUI_QUIT;
	case YESNODIALOG_CB_NO :
		widget = (GUI_Widget *)data;
		widget->Delete();

		showingDialog = false;
		if (gamemenu_dialog != NULL)
			gui->lock_input(gamemenu_dialog);
		else
			game->get_gui()->unlock_input();
		return GUI_YUM;
	case GAMEMENUDIALOG_CB_DELETE :
		showingDialog = false;
		gamemenu_dialog = NULL;
		keybinder->set_enable_joy_repeat(true);
		return GUI_YUM;
	}

	return GUI_PASS;
}

/* Switch to solo mode.
 */
void Events::solo_mode(uint32 party_member) {
	Actor *actor = player->get_party()->get_actor(party_member);

	if (game->user_paused())
		return;

	if (!actor || player->is_in_vehicle())
		return;

	if (player->get_party()->is_in_combat_mode())
		scroll->display_string("Not in combat mode!\n\n");
	else if (player->set_solo_mode(actor)) {
		scroll->display_string("Solo mode\n\n");
		player->set_mapwindow_centered(true);
		actor->set_worktype(0x02); // Player
		if (in_control_cheat)
			game->get_party()->update_light_sources();
		in_control_cheat = false;

		if (game->is_new_style()) {// do nothing for now
		} else if (view_manager->get_current_view() == view_manager->get_inventory_view())
			view_manager->get_inventory_view()->set_party_member(party_member);
		else if (view_manager->get_current_view() == view_manager->get_actor_view())
			view_manager->get_actor_view()->set_party_member(party_member);
	}
	scroll->display_prompt();
}

/* Switch to party mode. */
bool Events::party_mode() {
	bool was_in_control_cheat; // go in party mode no matter what (we know are we not in combat or vehicle)
	MapCoord leader_loc;
	if (in_control_cheat) {
		in_control_cheat = false;
		was_in_control_cheat = true;
		view_manager->set_party_mode();
		game->get_party()->update_light_sources();
	} else
		was_in_control_cheat = false;
	Actor *actor = player->get_party()->get_actor(0);
	assert(actor); // there must be a leader

	if (game->user_paused() && !was_in_control_cheat) // don't return if died in control cheat
		return false;

	if (player->is_in_vehicle())
		return false;

	bool success = false;
	leader_loc = actor->get_location();

	if (player->get_party()->is_in_combat_mode())
		scroll->display_string("Not in combat mode!\n");
	else if (player->get_party()->is_at(leader_loc, 6) || was_in_control_cheat) {
		if (player->set_party_mode(player->get_party()->get_actor(0))) {
			success = true;
			scroll->display_string("Party mode\n");
			player->set_mapwindow_centered(true);
		}
	} else
		scroll->display_string("Not everyone is here.\n");
	scroll->display_string("\n");
	scroll->display_prompt();
	return success;
}

/* Switch to or from combat mode. */
bool Events::toggle_combat() {
	Party *party = player->get_party();
	bool combat_mode = !party->is_in_combat_mode();

	if (!player->in_party_mode()) {
		scroll->display_string("Not in solo mode.\n\n");
		scroll->display_prompt();
	} else if (party->is_in_vehicle()) {
		display_not_aboard_vehicle();
	} else if (in_control_cheat) {
		scroll->display_string("\nNot while using control cheat!\n\n");
		scroll->display_prompt();
	} else
		party->set_in_combat_mode(combat_mode);

	if (party->is_in_combat_mode() == combat_mode) {
		if (combat_mode)
			scroll->display_string("Begin combat!\n\n");
		else {
			scroll->display_string("Break off combat!\n\n");
			player->set_actor(party->get_leader_actor()); // return control to leader
			player->set_mapwindow_centered(true); // center mapwindow
		}
		scroll->display_prompt();

		return true;
	}

	return false;
}

/* Make actor wear an object they are holding. */
bool Events::ready(Obj *obj, Actor *actor) {
	if (!actor)
		actor = game->get_actor_manager()->get_actor(obj->x);
	bool readied = false;

	if (game->user_paused())
		return (false);

	scroll->display_fmt_string("Ready-%s\n", obj_manager->look_obj(obj, false));
	float obj_weight = obj_manager->get_obj_weight(obj, OBJ_WEIGHT_INCLUDE_CONTAINER_ITEMS,
	                   OBJ_WEIGHT_DO_SCALE, OBJ_WEIGHT_EXCLUDE_QTY);
	float equip_weight = actor->get_inventory_equip_weight() + obj_weight;
	float total_weight = actor->get_inventory_weight();

	if (obj->get_actor_holding_obj() != actor)
		total_weight += obj_weight;

	if ((actor->get_strength() < equip_weight
	        || actor->get_strength() * 2 < total_weight) && !game->using_hackmove())
		scroll->display_string("\nToo heavy!\n");
	// perform READY usecode
	else if (actor->can_ready_obj(obj) && usecode->has_readycode(obj) && (usecode->ready_obj(obj, actor) == false)) {
		scroll->display_string("\n");
		scroll->display_prompt();
		return (obj->is_readied()); // handled by usecode
	} else if (obj->is_in_container() && obj->get_actor_holding_obj() != actor
	           && !Game::get_game()->get_map_window()->can_get_obj(actor, obj->get_container_obj()))
		scroll->display_string("\nCan't reach it\n");
	else if (!(readied = actor->add_readied_object(obj))) {
		if (actor->get_object_readiable_location(obj) == ACTOR_NOT_READIABLE)
			scroll->display_string("\nCan't be readied!\n");
		else
			scroll->display_string("\nNo place to put!\n");
	}
	scroll->display_string("\n");
	scroll->display_prompt();
	return (readied);
}

/* Make actor hold an object they are wearing. */
bool Events::unready(Obj *obj) {
	Actor *actor = game->get_actor_manager()->get_actor(obj->x);

	if (game->user_paused())
		return (false);

	scroll->display_fmt_string("Unready-%s\n", obj_manager->look_obj(obj, false));

	// perform unREADY usecode
	if (usecode->has_readycode(obj) && (usecode->ready_obj(obj, actor) == false)) {
		scroll->display_string("\n");
		scroll->display_prompt();
		return (!obj->is_readied()); // handled by usecode
	}

	actor->remove_readied_object(obj, false); // already ran usecode so don't run when unequipping

	scroll->display_string("\n");
	scroll->display_prompt();
	return (true);
}

bool Events::drop_start() {
	if (game->user_paused())
		return false;
	drop_obj = NULL;
	drop_qty = 0;
	drop_x = drop_y = -1;

//    get_obj_from_inventory(some actor, "Drop-");
//    get_obj_from_inventory("Drop-");
	get_target("Drop-");
//    moveCursorToInventory(); done in newAction()
	return true;
}

/* Print object name and select it as object to be dropped. If qty is 0, the
 * amount to drop may be requested.
 */
bool Events::drop_select(Obj *obj, uint16 qty) {
	if (game->user_paused())
		return false;

	drop_obj = obj;
	scroll->display_string(drop_obj ? obj_manager->look_obj(drop_obj) : "nothing");
	scroll->display_string("\n");
	if (drop_from_key)
		close_gumps();
	if (drop_obj) {
		if (qty == 0 && obj_manager->is_stackable(drop_obj) && drop_obj->qty > 1) {
			scroll->display_string("How many? ");
//            newAction(DROPCOUNT_MODE);
			get_scroll_input(); // "How many?"
			return true;
		}
		drop_count(1);
	} else endAction(true);

	return true;
}

/* Select quantity of `drop_obj' to be dropped. (qty 0 = drop nothing) */
bool Events::drop_count(uint16 qty) {
	if (game->user_paused())
		return (false);

	drop_qty = qty;
	scroll->display_string("\n");

	if (drop_qty != 0) {
		if (drop_x == -1)
			get_target("Location:");
		else { // h4x0r3d by SB-X... eventually integrate MapWindow dragndrop better with this drop-action
			scroll->display_string("Location:");
			perform_drop(); // use already selected target: drop_x,drop_y
		}
	} else
		endAction(true); // cancelled

	return true;
}

/* Make actor holding selected object drop it at cursor coordinates. Wait for
 * drop effect to complete before ending the action.
 */
bool Events::perform_drop() {
	if (game->user_paused())
		return false;
	if (drop_x == -1 || drop_y == -1) {
		if (input.loc == NULL) {
			scroll->display_string("Not possible\n");
			endAction(true);
			return false;
		}

		if (drop_x == -1) drop_x = input.loc->x;
		if (drop_y == -1) drop_y = input.loc->y;
	}

	return (drop(drop_obj, drop_qty, uint16(drop_x), uint16(drop_y)));
}

/* Make actor holding object drop it at x,y. */
bool Events::drop(Obj *obj, uint16 qty, uint16 x, uint16 y) {
	if (game->user_paused())
		return false;

	bool drop_from_map = obj->get_engine_loc() == OBJ_LOC_MAP;

	Actor *actor = (obj->is_in_inventory()) // includes held containers
	               ? obj->get_actor_holding_obj()
	               : player->get_actor();
	MapCoord actor_loc = actor->get_location();
	MapCoord drop_loc(x, y, actor_loc.z);
	/* not used in the original game engine
	    sint16 rel_x = x - actor_loc.x;
	    sint16 rel_y = y - actor_loc.y;
	    if(rel_x != 0 || rel_y != 0)
	    {
	        scroll->display_string(get_direction_name(rel_x, rel_y));
	        scroll->display_string(".");
	    }*/
	CanDropOrMoveMsg can_drop;
	if (!drop_from_map // already checked in map window
	        && (can_drop = map_window->can_drop_or_move_obj(drop_loc.x, drop_loc.y, actor, obj)) != MSG_SUCCESS) {
//        scroll->display_string("\n\nNot possible\n"); // using text from can_drop_or_move_obj
		map_window->display_can_drop_or_move_msg(can_drop, "\n\n");
		endAction(true); // because the DropEffect is never called to do this
		return false;
	}

	// all object management is contained in the effect (use requested quantity)
	if (!usecode->has_dropcode(obj)
	        || usecode->drop_obj(obj, actor, drop_loc.x, drop_loc.y, qty ? qty : obj->qty)) {
		bool interface_fullscreen = map_window->get_interface() != INTERFACE_NORMAL;
		if (interface_fullscreen) {
			if (qty < obj->qty && obj_manager->is_stackable(obj))
				obj = obj_manager->get_obj_from_stack(obj, qty);
			Obj *dest_obj = obj_manager->get_obj(drop_loc.x, drop_loc.y, drop_loc.z);
			if (obj_manager->can_store_obj(dest_obj, obj))
				obj_manager->moveto_container(obj, dest_obj);
			else
				obj_manager->moveto_map(obj, drop_loc);
		} else if (drop_from_map) {
			if (qty >= obj->qty || !obj_manager->is_stackable(obj))
				obj_manager->remove_obj_from_map(obj); // stop ghosting from drop effect
		}

		if (!drop_from_map) // preserve ok to take if it was never in inventory
			obj->status |= OBJ_STATUS_OK_TO_TAKE;

		if (!interface_fullscreen)
			new DropEffect(obj, qty ? qty : obj->qty, actor, &drop_loc);
		if (drop_from_map && map_window->original_obj_loc.distance(drop_loc) > 1) // get plus drop
			player->subtract_movement_points(6); // get plus drop
		else if (drop_from_map) // move
			player->subtract_movement_points(5);
		else
			game->get_script()->call_actor_subtract_movement_points(actor, 3);
		scroll->message("\n\n");
		endAction(false);
		set_mode(MOVE_MODE);
		return true;
	}
	// handled by usecode
	endAction(true); // because the DropEffect is never called to do this
	return false;
}

bool Events::rest() {
	if (rest_time != 0) { // already got time & started the campfire; time to Rest
		assert(last_mode == REST_MODE); // we'll need to clear Rest mode after
		// exiting Wait mode
		player->get_party()->rest_sleep(rest_time, rest_guard - 1);
		return true;
	}
	scroll->display_string("Rest");

	string err_str;
	if (!player->get_party()->can_rest(err_str)) {
		scroll->display_string(err_str);
		scroll->display_string("\n");
		endAction(true);
		return false;
	}

	if (player->get_actor()->get_obj_n() == OBJ_U6_SHIP) {
		scroll->display_string("\n");
		player->repairShip();
		endAction(true);
	} else {
		scroll->display_string("\nHow many hours? ");
		get_scroll_input("0123456789");
	}
	return true;
}

/* Get hours to Rest, or number of party member who will guard. These must be
   entered in order. */
bool Events::rest_input(uint16 input_) {
	Party *party = player->get_party();
	scroll->set_input_mode(false);
	scroll->display_string("\n");
	if (rest_time == 0) {
		rest_time = input_;
		if (rest_time == 0) {
			endAction(true);
			return false;
		}
		if (party->get_party_size() > 1) {
			scroll->display_string("Who will guard? ");
			get_target("");
			get_scroll_input("0123456789", true, true);
		} else {
			party->rest_gather(); // nobody can guard; start now
		}
	} else {
		rest_guard = input_;
		if (rest_guard > party->get_party_size())
			rest_guard = 0;
		if (rest_guard == 0)
			scroll->display_string("none\n");
		else {
			scroll->display_string(party->get_actor(rest_guard - 1)->get_name());
			scroll->display_string("\n");
		}
		scroll->display_string("\n");
		party->rest_gather();
	}
	return true;
}

void Events::cast_spell_directly(uint8 spell_num) {
	endAction(false);
	newAction(SPELL_MODE);
	input.type = EVENTINPUT_KEY;
	input.spell_num = spell_num;
	doAction();
}

/* Walk the player towards the mouse cursor. (just 1 space for now) */
void Events::walk_to_mouse_cursor(uint32 mx, uint32 my) {
// FIXME: might add generic walk_to() action to Player
// player->walk_to(uint16 x, uint16 y, uint16 move_max, uint16 timeout_seconds);
//    int wx, wy;
	sint16 rx, ry;

	if (game->user_paused() || !player->check_walk_delay())
		return;

	// Mouse->World->RelativeDirection
//    map_window->mouseToWorldCoords((int)mx, (int)my, wx, wy);
	map_window->get_movement_direction((uint16) mx, (uint16) my, rx, ry);
	player->moveRelative(rx, ry, true);
	game->time_changed();
}

/* Talk to NPC, read a sign, or use an object at map coordinates.
 * FIXME: should be able to handle objects from inventory
 */
void Events::multiuse(uint16 wx, uint16 wy) {
	ActorManager *actor_manager = game->get_actor_manager();
	Obj *obj = NULL;
	Actor *actor = NULL, *player_actor = player->get_actor();
	bool using_actor = false; //, talking = false;
	MapCoord player_location(player_actor->get_location());
	MapCoord target(player_actor->get_location()); // changes to target location
	bool in_combat = player->get_party()->is_in_combat_mode();

	if (game->user_paused() || map_window->tile_is_black(wx, wy))
		return;

	obj = obj_manager->get_obj(wx, wy, target.z);
	actor = actor_manager->get_actor(wx, wy, target.z);

	// use object or actor?
	if (actor) {
		if ((!actor->is_visible() && !in_combat) || (in_combat
		        && (actor->get_actor_num() == player->get_actor()->get_actor_num() //don't attack yourself.
		            || actor->get_alignment() == ACTOR_ALIGNMENT_GOOD))) {
			Actor *a = actor_manager->get_actor(actor->get_x(), actor->get_y(), actor->get_z(), true, actor);
			if (a || (!in_combat && (!actor->is_visible() // null invisible actors if not in combat and no one is found
			                         || (actor == player_actor && !game->is_new_style()
			                             && actor->get_actor_num() != 0)))) // pass if in combat if player and not showing inventory
				actor = a;
		}

		if (actor) {
			using_actor = true;
			target.x = actor->get_location().x;
			target.y = actor->get_location().y;
			DEBUG(0, LEVEL_DEBUGGING, "Use actor at %d,%d\n", target.x, target.y);
		}
	}
	if (obj && !using_actor) {
		target.x = obj->x;
		target.y = obj->y;
		DEBUG(0, LEVEL_DEBUGGING, "Use object at %d,%d\n", obj->x, obj->y);
	}

	if (in_combat && (obj || using_actor)) {
		if (!using_actor || actor->get_alignment() != ACTOR_ALIGNMENT_GOOD) {
			newAction(ATTACK_MODE);
			if (get_mode() == ATTACK_MODE) {
				map_window->moveCursor(wx - map_window->get_cur_x(), wy - map_window->get_cur_y());
				select_target(uint16(wx), uint16(wy), target.z);
			}
			return;
		}
	}

	if (using_actor) { // use or talk to an actor
		if (using_pickpocket_cheat && game->are_cheats_enabled()) {
			get_inventory_obj(actor, false);
			return;
		}
		bool can_use;
		if (game->get_game_type() == NUVIE_GAME_U6 && (actor->get_actor_num() == 132 // Smith
		        || actor->get_actor_num() == 130)) // Pushme Pullyu
			can_use = false;
		else
			can_use = usecode->has_usecode(actor);
		if (can_use) {
			scroll->display_string("Use-", MSGSCROLL_NO_MAP_DISPLAY);
			set_mode(USE_MODE);
			use(actor, wx, wy);
		} else {
			if (game->is_new_style() && actor == actor_manager->get_player()) {
				//open inventory here.
				view_manager->open_doll_view(in_control_cheat ? actor : NULL);
			} else if (target == player_location)
				using_actor = false;
			else {
				newAction(TALK_MODE);
				talk(actor);
			}
		}
		if (using_actor)
			return;
	}
	if (!obj)
		return;
	else if (usecode->is_readable(obj)) {
		scroll->display_string("Look-", MSGSCROLL_NO_MAP_DISPLAY);
		set_mode(LOOK_MODE);
		look(obj);
		endAction(false); // FIXME: should be in look()
	} else if (game->get_game_type() == NUVIE_GAME_U6
	           && (obj->obj_n == OBJ_U6_SHRINE
	               || obj->obj_n == OBJ_U6_STATUE_OF_MONDAIN
	               || obj->obj_n == OBJ_U6_STATUE_OF_MINAX
	               || obj->obj_n == OBJ_U6_STATUE_OF_EXODUS)) {
		scroll->display_string("Talk-", MSGSCROLL_NO_MAP_DISPLAY);
		set_mode(TALK_MODE);
		talk(obj);
	} else { // use a real object
		if (newAction(USE_MODE))
			select_obj(obj);
	}
}

/* Do the final action for the current mode, with a selected target. */
void Events::doAction() {
	if (game->user_paused())
		return;

	if (mode == MOVE_MODE) {
		scroll->display_string("what?\n", MSGSCROLL_NO_MAP_DISPLAY);
		endAction(true);
		return;
	}
	if (mode == INPUT_MODE) { // set input to current cursor coord
		if (input.get_text) {
			if (last_mode == REST_MODE && rest_time != 0 && !scroll->has_input()) {
				select_target(map_window->get_cursorCoord().x,
				              map_window->get_cursorCoord().y,
				              map_window->get_cursorCoord().z);
				return;
			}
			assert(scroll->has_input()); // doAction should only be called when input is ready
			assert(input.str == 0);
			input.str = new string(scroll->get_input());
			endAction();
			doAction();
		} else if (input.select_from_inventory) // some redirection here...
			view_manager->get_inventory_view()->select_objAtCursor();
		else
			select_target(map_window->get_cursorCoord().x, map_window->get_cursorCoord().y, map_window->get_cursorCoord().z);
		// the above function will switch back to the previous mode that
		// started getting input, and call doAction() again, which should
		// eventually result in an endAction()
		return;
	} else if (callback_target) { // send input elsewhere
		message(CB_DATA_READY, (char *) &input);
		callback_target = 0;
		endAction(true);
		return;
	}

	if (mode == LOOK_MODE) {
		if (looking_at_spellbook && view_manager->get_spell_view() != NULL) {
			view_manager->get_spell_view()->close_look();
			return;
		}
		if (input.type == EVENTINPUT_OBJECT && input.obj && (!input.obj->is_on_map()
		        || (!(input.obj->status & OBJ_STATUS_INVISIBLE)
		            && !map_window->tile_is_black(input.obj->x,
		                    input.obj->y,
		                    input.obj)))) {   // look() returns false if prompt was already printed
			bool prompt_in_endAction = look(input.obj);
			endAction(prompt_in_endAction);
		} else if (input.type == EVENTINPUT_MAPCOORD && input.actor && input.actor->is_visible()) {
			bool prompt = !look(input.actor);
			endAction(prompt);
		} else {
			lookAtCursor();
		}
	} else if (mode == TALK_MODE) {
		if (input.type == EVENTINPUT_OBJECT)
			talk(input.obj);
		else if (input.type == EVENTINPUT_MAPCOORD && input.actor && input.actor->is_visible())
			talk(input.actor);
		else
			talk_cursor();
		endAction();
	} else if (mode == USE_MODE) {

		if (usecode) {
			ScriptThread *usecode_script = usecode->get_running_script();
			if (usecode_script != NULL) {
				uint8 script_state = usecode_script->get_state();
				switch (script_state) {
				case NUVIE_SCRIPT_GET_DIRECTION :
					if (input.type == EVENTINPUT_MAPCOORD_DIR) {
						usecode_script->resume_with_direction(get_direction_code(input.loc->sx, input.loc->sy));
					}
					break;
				case NUVIE_SCRIPT_GET_OBJ :
					usecode_script->resume_with_obj(input.obj);
					if (!game->is_new_style()) {
						view_manager->get_inventory_view()->release_focus();
						sint8 leader = game->get_party()->get_leader();
						if (leader >= 0) {
							view_manager->get_inventory_view()->set_party_member(leader);
						}
					} else
						view_manager->close_all_gumps();
					break;
				}
			} else {
//          if(game->is_new_style()) // don't do this it wll crash when using containers inside a gump
//            view_manager->close_all_gumps();

				if (input.type == EVENTINPUT_OBJECT)
					use(input.obj);
				else if (input.type == EVENTINPUT_MAPCOORD_DIR) {
					if (input.actor && input.actor->is_visible() && usecode->has_usecode(input.actor)) {
						MapCoord loc = game->get_player()->get_actor()->get_location();
						use(input.actor, loc.x + input.loc->sx, loc.y + input.loc->sy);
					} else
						use(input.loc->sx, input.loc->sy);
				} else if (input.type == EVENTINPUT_MAPCOORD) {
					use(*input.loc);
				} else {
					scroll->display_string("what?\n");
					endAction(true);
				}
			}

			usecode_script = usecode->get_running_script();
			if (usecode_script != NULL) {
				uint8 script_state = usecode_script->get_state();
				switch (script_state) {
				case NUVIE_SCRIPT_GET_DIRECTION :
					get_direction("");
					break;
				case NUVIE_SCRIPT_GET_OBJ :
					get_target("");
					break;
				}
			}

			if (mode == USE_MODE && (usecode_script == NULL || usecode_script->is_running() == false)) {
				endAction(true);
			}
		}

		// assert(mode != USE_MODE);
	} else if (mode == GET_MODE) {
		if (input.type == EVENTINPUT_OBJECT)
			perform_get(input.obj);
		else if (input.type == EVENTINPUT_MAPCOORD_DIR)
			get(input.loc->sx, input.loc->sy);
		else if (input.type == EVENTINPUT_MAPCOORD)
			get(*input.loc);
		else {
			scroll->display_string("what?\n");
			endAction(true);
		}
		endAction();
	} else if (mode == ATTACK_MODE) {
		attack();
	} else if (mode == PUSH_MODE) {
		assert(
		    input.type == EVENTINPUT_MAPCOORD_DIR || input.type == EVENTINPUT_OBJECT || input.type == EVENTINPUT_MAPCOORD);
		if (input.type == EVENTINPUT_MAPCOORD_DIR) {
			if (!push_obj && !push_actor)
				pushFrom(input.loc->sx, input.loc->sy);
			else
				pushTo(input.loc->sx, input.loc->sy, PUSH_FROM_OBJECT);
		} else if (input.type == EVENTINPUT_MAPCOORD && !move_in_inventory) {
			if (!push_obj && !push_actor)
				pushFrom(*input.loc);
			else
				pushTo(input.loc->x, input.loc->y);
		} else {
			if (!push_obj) {
				move_in_inventory = true;
				pushFrom(input.obj);
			} else {
				pushTo(input.obj, input.actor);
			}
		}
	} else if (mode == DROP_MODE) { // called repeatedly
		if (!drop_obj) {
			if (input.select_from_inventory == false)
				return endAction(true);

			if (input.type == EVENTINPUT_MAPCOORD) {
				scroll->display_string("nothing\n");
				return endAction(true);
			}

			assert(input.type == EVENTINPUT_OBJECT);
			drop_select(input.obj);
		} else if (!drop_qty) {
			assert(input.str);
			if (strncmp(input.str->c_str(), "", input.str->length()) == 0) {
				char buf[6];
				snprintf(buf, sizeof(buf), "%u", drop_obj->qty);
				scroll->display_string(buf);
				drop_count(drop_obj->qty);
			} else
				drop_count(strtol(input.str->c_str(), NULL, 10));
		} else
			perform_drop();
	} else if (mode == REST_MODE) {
		if (rest_time != 0 && !input.str) {
			sint8 party_num;
			if (input.actor)
				party_num = game->get_party()->get_member_num(input.actor) + 1;
			else
				party_num = 0;
			rest_input(party_num > 0 ? party_num : 0);
			return;
		}
		assert(input.str);
		if (strncmp(input.str->c_str(), "", input.str->length()) == 0) {
			if (rest_time == 0)
				scroll->display_string("0");
			rest_input(0);
		} else
			rest_input(strtol(input.str->c_str(), NULL, 10));
	} else if (mode == CAST_MODE || mode == SPELL_MODE) {
		if (input.type == EVENTINPUT_MAPCOORD) {
			if (magic->is_waiting_for_location())
				magic->resume(MapCoord(input.loc->x, input.loc->y, input.loc->z));
			else if (magic->is_waiting_for_obj())
				magic->resume(input.obj);
			else {
				magic->resume();
				if (!game->is_new_style() && game->get_party()->get_leader() != -1)
					view_manager->get_inventory_view()->set_party_member(game->get_party()->get_leader());
			}
		} else if (input.type == EVENTINPUT_MAPCOORD_DIR) {
			magic->resume(get_direction_code(input.loc->sx, input.loc->sy));
		} else if (input.type == EVENTINPUT_OBJECT) {
			magic->resume(input.obj);
			if (!game->is_new_style() && game->get_party()->get_leader() != -1) {
				view_manager->get_inventory_view()->release_focus();
				view_manager->get_inventory_view()->set_party_member(game->get_party()->get_leader());
			} else
				view_manager->get_inventory_view()->Hide();
		} else if (input.type == EVENTINPUT_SPELL_NUM) {
			if (input.spell_num != -1)
				magic->resume_with_spell_num(input.spell_num);
			else
				magic->resume();
		} else {
			if (mode == CAST_MODE)
				magic->cast();
			else
				magic->cast_spell_directly(input.spell_num);
		}

		for (; magic->is_waiting_to_talk();) {
			talk(magic->get_actor_from_script());
			magic->resume();
		}

		if (magic->is_waiting_for_location() || magic->is_waiting_for_obj())
			get_target("");
		else if (magic->is_waiting_for_direction())
			get_direction("");
		else if (magic->is_waiting_for_inventory_obj()) {
			get_inventory_obj(magic->get_actor_from_script());
		} else if (magic->is_waiting_for_spell()) {
			get_spell_num(player->get_actor(), magic->get_spellbook_obj());
		} else {
			endAction(true);
		}
	} else if (mode == MULTIUSE_MODE) {
		if (input.loc) { // on map
			set_mode(MOVE_MODE);
			multiuse(input.loc->sx, input.loc->sy);
		} else { // tryed on views/gumps
			Obj *obj = input.obj; // newAction(USE_MODE) will NULL input.obj
			if (!obj) { // not sure if this is needed
				set_mode(MOVE_MODE);
				return;
			}

			if (usecode->is_readable(obj)) { // look at a scroll or book
				set_mode(LOOK_MODE);
				look(obj);
				endAction(false); // FIXME: should be in look()
				return;
			}
			set_mode(USE_MODE);
			use(obj);
		}
	} else if (cursor_mode) {
		MapCoord loc = map_window->get_cursorCoord(); // need to preserve locations if a target is needed
		uint16 cursor_x = loc.x - map_window->get_cur_x();
		uint16 cursor_y = loc.y - map_window->get_cur_y();

		if (!game->get_command_bar()->try_selected_action(-1)) { // no input needed
			map_window->set_show_cursor(false);
			return;
		}
		map_window->moveCursor(cursor_x, cursor_y);
		select_target(loc.x, loc.y, loc.z); // the returned location
	} else if (mode == SCRIPT_MODE) {
		if (scriptThread != NULL) {
			uint8 script_state = scriptThread->get_state();
			switch (script_state) {
			case NUVIE_SCRIPT_GET_DIRECTION :
				if (input.type == EVENTINPUT_MAPCOORD_DIR) {
					scriptThread->resume_with_direction(get_direction_code(input.loc->sx, input.loc->sy));
				}
				break;
			case NUVIE_SCRIPT_GET_TARGET :
			case NUVIE_SCRIPT_GET_OBJ :
				if (input.type == EVENTINPUT_MAPCOORD) {
					scriptThread->resume_with_location(MapCoord(input.loc->x, input.loc->y, input.loc->z));
				}
				break;
			default:
				break;
			}

			script_state = scriptThread->get_state();
			switch (script_state) {
			case NUVIE_SCRIPT_GET_DIRECTION :
				get_direction("");
				break;
			case NUVIE_SCRIPT_GET_TARGET :
				get_target("");
				break;

			case NUVIE_SCRIPT_FINISHED:
				delete scriptThread;
				scriptThread = NULL;
				endAction(true);
				return;

			default:
				break;
			}
		}
	} else
		cancelAction();
}

/* Cancel the action for the current mode, switch back to MOVE_MODE if possible. */
void Events::cancelAction() {
	if (game->user_paused())
		return;
	if (view_manager->gumps_are_active() && (magic == NULL || !magic->is_waiting_for_inventory_obj()))
		return close_gumps();
	if (mode == INPUT_MODE) { // cancel action of previous mode
		if (magic != NULL && magic->is_waiting_for_inventory_obj()) {
			if (!game->is_new_style() && game->get_party()->get_leader() != -1) {
				view_manager->get_inventory_view()->release_focus();
				view_manager->get_inventory_view()->set_party_member(game->get_party()->get_leader());
			} else
				view_manager->get_inventory_view()->Hide();
		} else {
			if (usecode) {
				if (usecode->is_script_running()) {
					if (!game->is_new_style()
					        && game->get_party()->get_leader() != -1) { //FIXME consolidate this logic with magic script logic above
						view_manager->get_inventory_view()->release_focus();
						view_manager->get_inventory_view()->set_party_member(game->get_party()->get_leader());
					}
//              else
//                  view_manager->close_all_gumps();
				}
			}
			if (last_mode == PUSH_MODE) {
				if (push_obj || push_actor) {
					if (move_in_inventory)
						scroll->display_string("nobody.\n");
					else
						scroll->display_string("nowhere.\n");
					endAction();
					endAction(true);
					return;
				}
			}
		}
		endAction();
		cancelAction();
		return;
	}

	if (mode == MOVE_MODE) {
		player->pass();
	} else if (mode == CAST_MODE) {
		if (magic->is_waiting_to_resume())
			magic->resume();
		else {

			scroll->display_string("nothing\n");
			view_manager->close_spell_mode();
		}
	} else if (mode == USE_MODE) {
		if (usecode->is_script_running()) {
			usecode->get_running_script()->resume_with_nil();
		}

		if (callback_target) {
			message(CB_INPUT_CANCELED, (char *) &input);
			callback_target = NULL;
			callback_user_data = NULL;
		}
	} else if (mode == EQUIP_MODE) {
		endAction();
		return;
	} else if (looking_at_spellbook && view_manager->get_spell_view() != NULL) {
		view_manager->get_spell_view()->close_look();
		return;
	} else {
		scroll->display_string("what?\n");
		if (mode == ATTACK_MODE) {
			player->subtract_movement_points(10);
			game->get_actor_manager()->startActors(); // end player turn
			endAction();
			return;
		}
	}

	endAction(true);
}

/* Request new EventMode, for selecting a target.
 * Returns true the mode is changed. (basically if a new "select an
 * object/direction for this action" prompt is displayed)
 */
bool Events::newAction(EventMode new_mode) {
	map_window->set_looking(false);
	map_window->set_walking(false);

	if (game->user_paused())
		return (false);
	cursor_mode = false;
// FIXME: make ATTACK_MODE use INPUT_MODE
	if (mode == ATTACK_MODE && new_mode == ATTACK_MODE) {
		close_gumps();
		doAction();
		return (mode == ATTACK_MODE);
	}
	if (looking_at_spellbook && view_manager->get_spell_view() != NULL) { // pushed L while looking at spell book
		view_manager->get_spell_view()->close_look();
		return false;
	}
	// since INPUT_MODE must be set to get input, it wouldn't make sense that
	// a mode would be requested again to complete the action
	assert(mode != new_mode);

	// called again (same key pressed twice); equivalent of pressing ENTER so call doAction() to set input
	if (mode == INPUT_MODE && new_mode == last_mode) {
		doAction();
		return (!(mode == MOVE_MODE));
	} else if (mode != MOVE_MODE && mode != EQUIP_MODE) { // already in another mode; exit
		cancelAction();
		return (false);
	}
	move_in_inventory = false;

	set_mode(new_mode);
	if (new_mode != COMBAT_MODE)
		game->set_mouse_pointer(1);
	switch (new_mode) {
	case CAST_MODE:
		/* TODO check if spellbook ready before changing mode */
		scroll->display_string("Cast-");
		if (!magic->start_new_spell()) {
			mode = MOVE_MODE;
			scroll->display_prompt();
		} else
			key_redirect((CallBack *) magic, NULL);
		break;
	case SPELL_MODE:
		break;
	case LOOK_MODE:
		look_start();
		break;
	case TALK_MODE:
		talk_start();
		break;
	case USE_MODE:
		use_start();
		break;
	case GET_MODE:
		get_start();
		break;
	case MULTIUSE_MODE:
		get_target("");
		if (game->get_party()->is_in_combat_mode())
			player->attack_select_init(false);
		break;
	case ATTACK_MODE:
		close_gumps();
		if (game->get_game_type() == NUVIE_GAME_U6
		        && player->is_in_vehicle()
		        && player->get_actor()->get_obj_n() != OBJ_U6_SHIP) {
			scroll->display_string("Attack-");
			display_not_aboard_vehicle(false);
			endAction(true);
			return false;
		}
		if (game->get_game_type() != NUVIE_GAME_U6) {
			scriptThread = game->get_script()->call_function_in_thread("player_attack");
			mode = SCRIPT_MODE;
			scriptThread->start();
			switch (scriptThread->get_state()) {
			case NUVIE_SCRIPT_GET_TARGET:
				get_target("");
				break;
			}
			break;
		}
		player->attack_select_init();
		map_window->set_show_cursor(true);
		break;
	case PUSH_MODE:
		push_start();
		break;
	case DROP_MODE:
		drop_start();
		// fall through
	case EQUIP_MODE: // if this was called from moveCursorToInventory, the
		// mode has now changed, so it wont be called again
		moveCursorToInventory();
		break;
//		case DROPCOUNT_MODE:
//			get_scroll_input(); /* "How many?" */
//			break;
	case REST_MODE:
		rest_time = rest_guard = 0;
		rest();
		break;
	case COMBAT_MODE:
		toggle_combat();
		mode = MOVE_MODE;
		break;
	default:
		cancelAction(); // "what?"
		return (false);
	}
	return (true); // ready for object/direction
}

/* Revert to default MOVE_MODE. (walking)
 * This clears visible cursors, and resets all variables used by actions.
 */
void Events::endAction(bool prompt) {
	if (prompt) {
		scroll->display_string("\n");
		scroll->display_prompt();
	}

	if (mode == PUSH_MODE) {
		push_obj = NULL;
		push_actor = NULL;
		map_window->reset_mousecenter();
	} else if (mode == DROP_MODE) {
		drop_obj = NULL;
		drop_qty = 0;
		drop_from_key = false;
	} else if (mode == REST_MODE) {
		rest_time = rest_guard = 0;
		scroll->set_using_target_cursor(false);
	}
	if (cursor_mode || mode == EQUIP_MODE) {
		cursor_mode = false;
		map_window->set_show_cursor(false);
	}
	if (mode == ATTACK_MODE) { // FIXME: make ATTACK_MODE use INPUT_MODE
		map_window->set_show_cursor(false);
	}

	// Revert to the previous mode, instead of MOVE_MODE.
	/* Switching from INPUT_MODE, clear state indicating the type of input
	     to return, but leave returned input. Clear returned input only when
	     entering INPUT_MODE, or deleting Events. */
	if (/*game->user_paused() ||*/ mode == INPUT_MODE || mode == KEYINPUT_MODE) {
		mode = last_mode;
//        callback_target = 0;
		input.get_text = false;
//        input.select_from_inventory = false; // indicates cursor location
		input.get_direction = false;
		do_not_show_target_cursor = false;
		map_window->set_show_use_cursor(false);
		map_window->set_show_cursor(false);
		if (!game->is_new_style())
			view_manager->get_inventory_view()->set_show_cursor(false);
//    game->set_mouse_pointer(0);
		return;
	} else if (!looking_at_spellbook)
		set_mode(MOVE_MODE);

	map_window->updateBlacking();
}
// save current mode if switching to WAIT_MODE or INPUT_MODE
void Events::set_mode(EventMode new_mode) {
	DEBUG(0,
	      LEVEL_DEBUGGING,
	      "new mode = %s,  mode = %s, last mode = %s\n",
	      print_mode(new_mode),
	      print_mode(mode),
	      print_mode(last_mode));
	if (new_mode == WAIT_MODE && (last_mode == EQUIP_MODE || last_mode == REST_MODE))
		last_mode = mode;
	else if ((new_mode == INPUT_MODE || new_mode == KEYINPUT_MODE))
		last_mode = mode;
	else
		last_mode = MOVE_MODE;
	mode = new_mode;

	// re-init input state
	if (mode == INPUT_MODE || mode == KEYINPUT_MODE) {
		if (input.target_init) delete input.target_init;
		if (input.str) delete input.str;
		if (input.loc) delete input.loc;
		input.target_init = 0;
		input.str = 0;
		input.loc = 0;
		input.actor = 0;
		input.obj = 0;
	}
}

void Events::moveCursorToInventory() {
	if (push_actor)
		return;
	cursor_mode = false;
	if (mode == MOVE_MODE)
		newAction(EQUIP_MODE);
	else {
		map_window->set_show_cursor(false); // hide both MapWindow cursors
		map_window->set_show_use_cursor(false);
		if (!game->is_new_style()) {
			view_manager->get_inventory_view()->set_show_cursor(true);
			view_manager->get_inventory_view()->grab_focus(); // Inventory wants keyboard input
		} else {
			//view_manager->open_container_view(player->get_actor());
		}
	}
	input.select_from_inventory = true;
}

// Note that the cursor is not recentered here.
void Events::moveCursorToMapWindow(bool ToggleCursor) {
	input.select_from_inventory = false;
	if (!game->is_new_style()) {
		view_manager->get_inventory_view()->set_show_cursor(false);
		view_manager->get_inventory_view()->release_focus();
	} else {
		//Removed due to delete issues while dragging. view_manager->close_container_view(player->get_actor());
	}
	if (input.get_direction) // show the correct MapWindow cursor
		map_window->set_show_use_cursor(true);
	else if (ToggleCursor && mode == EQUIP_MODE) {
		if (game->get_command_bar()->get_selected_action() == -1)
			mode = MOVE_MODE;
		else {
			cursor_mode = true;
			map_window->centerCursor();
			map_window->set_show_cursor(true);
		}
	} else
		map_window->set_show_cursor(true);

//    map_window->grab_focus(); FIXME add move() and keyhandler to MapWindow, and uncomment this
}

static const char eventModeStrings[][17] = {
	"LOOK_MODE",
	"USE_MODE",
	"GET_MODE",
	"MOVE_MODE",
	"DROP_MODE",
	"TALK_MODE", /* finding an actor to talk to */
	"ATTACK_MODE",
	"PUSH_MODE",
	"REST_MODE",
	"CAST_MODE",
	"COMBAT_MODE", /* only used to cancel previous actions */
	"SPELL_MODE", //direct spell casting without spell select etc.
	"EQUIP_MODE",
	"WAIT_MODE", /* waiting for something, optionally display prompt when finished */
	"INPUT_MODE",
	"MULTIUSE_MODE",
	"KEYINPUT_MODE",
	"SCRIPT_MODE"
};

const char *Events::print_mode(EventMode mode_) {
	return eventModeStrings[mode_];
}

bool Events::can_target_icon() {
	if (mode == INPUT_MODE && (last_mode == TALK_MODE
	                           || last_mode == CAST_MODE || last_mode == SPELL_MODE
	                           || last_mode == LOOK_MODE || move_in_inventory
	                           || last_mode == USE_MODE || last_mode == REST_MODE))
		return true;
	else
		return false;
}

void Events::display_not_aboard_vehicle(bool show_prompt) {
	if (player->get_actor()->get_obj_n() == OBJ_U6_INFLATED_BALLOON)
		scroll->display_string("Not while aboard balloon!\n\n");
	else
		scroll->display_string("Not while aboard ship!\n\n");
	if (show_prompt)
		scroll->display_prompt();
}

bool Events::can_move_obj_between_actors(Obj *obj,
                                        Actor *src_actor,
                                        Actor *target_actor,
                                        bool display_name) { // exchange inventory
	MapCoord from = src_actor->get_location();

	if (target_actor) {
		if (display_name) {
			scroll->display_string(target_actor == src_actor ? "yourself" : target_actor->get_name());
			scroll->display_string(".");
		}

		if (!target_actor->is_in_party() && target_actor != player->get_actor()) {
			scroll->display_string("\n\nOnly within the party!");
			return false;
		}

		if (game->using_hackmove())
			return true;
		if (player->is_in_vehicle()) {
			display_not_aboard_vehicle();
			return false;
		}

		if (target_actor == src_actor && obj->is_in_inventory())
			return true;

		MapCoord to = target_actor->get_location();

		if (!map_window->tile_is_black(from.x, from.y)
		        && !map_window->tile_is_black(to.x, to.y)) {
			if (from.distance(to) < 5 || (map_window->get_interface() != INTERFACE_NORMAL
			                              && target_actor->is_onscreen() && src_actor->is_onscreen())) {
				if (game->get_script()->call_actor_get_obj(target_actor, obj))
					return true;
			} else
				scroll->display_string("\n\nOut of range!");
		} else
			scroll->display_string("\n\nBlocked!"); // original said Out of Range!
	} else
		scroll->display_string("\n\nnobody.");

	return false;
}

void Events::display_move_text(Actor *target_actor, Obj *obj) {
	scroll->display_string("Move-");
	scroll->display_string(obj_manager->look_obj(obj, OBJ_SHOW_PREFIX));
	if (game->get_game_type() == NUVIE_GAME_MD)
		scroll->display_string("\nWhere? ");
	else
		scroll->display_string(" To ");
	scroll->display_string(target_actor->get_name());
	scroll->display_string(".");
}

bool Events::can_get_to_actor(Actor *actor, uint16 x, uint16 y) { // need the exact tile
	if (map_window->get_interface() == INTERFACE_IGNORE_BLOCK
	        || player->get_actor() == actor)
		return true;

	LineTestResult lt;
	Map *map = game->get_game_map();
	MapCoord player_loc = player->get_actor()->get_location();

// FIXME false obj matches can occur (should be extremly rare)
	if (map->lineTest(player_loc.x, player_loc.y, x, y, player_loc.z, LT_HitUnpassable, lt)
	        && (!lt.hitObj || lt.hitObj->quality != actor->get_actor_num())) // actor part
		return false;

	return true;
}

bool Events::select_view_obj(Obj *obj, Actor *actor) {
	if ((last_mode == CAST_MODE || last_mode == SPELL_MODE)
	        && !magic->is_waiting_for_obj() && !magic->is_waiting_for_inventory_obj())
		cancelAction();
	else {
		if (!obj || push_actor != NULL)
			return false;
		if (usecode->cannot_unready(obj) && ((last_mode == DROP_MODE && drop_obj == NULL)
		                                     || (last_mode == PUSH_MODE && push_obj == NULL))) {
			scroll->display_string(obj_manager->look_obj(obj, false));
			scroll->display_string("\n");
			usecode->ready_obj(obj, obj->get_actor_holding_obj());
			endAction(true);
			set_mode(MOVE_MODE);
		} else
			select_obj(obj, actor);
	}
	return true;
}

void Events::close_gumps() {
//	if(game->is_new_style())
	{
		view_manager->close_all_gumps();
	}
}

bool Events::dont_show_target_cursor() {
	if (do_not_show_target_cursor || push_actor)
		return true;
	else
		return false;
}

bool Events::input_really_needs_directon() {
	if ((input.get_direction && (map_window->get_interface() == INTERFACE_NORMAL || last_mode == CAST_MODE)) ||
	        dont_show_target_cursor())
		return true;
	else
		return false;
}

bool shouldQuit() {
	return g_engine->shouldQuit();
}

} // End of namespace Nuvie
} // End of namespace Ultima
