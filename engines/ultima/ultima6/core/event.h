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

#ifndef ULTIMA6_CORE_EVENT_H
#define ULTIMA6_CORE_EVENT_H

#include <list>
#include "ultima/shared/std/string.h"

#include <SDL.h>
#include <script/Script.h>

#include "ObjManager.h"
//#include "GUI_CallBack.h"
#include "CallBack.h"
#include "KeysEnum.h"

namespace Ultima {
namespace Ultima6 {

class Actor;
class CallbackTarget;
class Configuration;
class Converse;
class Book;
class Game;
class TimeQueue;
class MapWindow;
class MsgScroll;
class GameClock;
class Player;
class ViewManager;
class UseCode;
class GUI;
class GUI_Dialog;
class Magic;
class KeyBinder;
class FpsCounter;
class ScriptThread;

#define NUVIE_INTERVAL    50
#define PUSH_FROM_PLAYER false
#define PUSH_FROM_OBJECT true

typedef enum {
	LOOK_MODE = 0,
	USE_MODE,
	GET_MODE,
	MOVE_MODE,
	DROP_MODE,
	TALK_MODE, /* finding an actor to talk to */
	ATTACK_MODE,
	PUSH_MODE,
	REST_MODE, /* modes before this need targets if using the command bar selected action */
	CAST_MODE,
	COMBAT_MODE, /* only used to cancel previous actions */
	SPELL_MODE, //direct spell casting without spell select etc.
	EQUIP_MODE,
	WAIT_MODE, /* waiting for something, optionally display prompt when finished */
	INPUT_MODE,
	MULTIUSE_MODE,
	KEYINPUT_MODE,
	SCRIPT_MODE
} EventMode;

extern uint32 nuvieGameCounter;

// type of input that event may collect and send somewhere
#define EVENTINPUT_MAPCOORD 0
#define EVENTINPUT_KEY      1
#define EVENTINPUT_STRING   2
#define EVENTINPUT_OBJECT   3
#define EVENTINPUT_MAPCOORD_DIR 4
#define EVENTINPUT_SPELL_NUM 5

struct EventInput_s {
	uint8 type; // 0=loc,1=key,2=str,3=obj,4=actor
//    union
//    {
	SDL_Keycode key; // last key entered, if capturing input
	ActionKeyType action_key_type; // last ActionKeyType entered if capturing input
	MapCoord *loc; // target location, or direction if relative ???
	std::string *str; // ???
//    };
	void set_loc(MapCoord c);
	EventInput_s() : loc(0), str(0), obj(0), actor(0), get_direction(false), get_text(false), target_init(0), select_from_inventory(false), select_range(0) {
		spell_num = 0;
		type = 0;
	}
	~EventInput_s();

	Obj *obj; // top object at loc (or object from inventory)
	Actor *actor; // actor at loc
	bool get_direction; // if true, entering directions selects a target
	bool get_text; // if true, the MsgScroll is polled for text input
	MapCoord *target_init; // where MapWindow cursor is centered when targeting
	bool select_from_inventory; // if true, objects from inventory will be selected (and not from the map)
	uint8 select_range; // limits movement of MapWindow cursor from center
	sint16 spell_num;
};
typedef struct EventInput_s EventInput;

class Event : public CallBack {
	friend class Magic; // FIXME
	Configuration *config;
	GUI *gui;
	Game *game;
	ObjManager *obj_manager;
	MapWindow *map_window;
	MsgScroll *scroll;
	GameClock *clock;
	Player *player;
	Converse *converse;
	ViewManager *view_manager;
	UseCode *usecode;
	Magic *magic;
	KeyBinder *keybinder;
	GUI_Dialog *gamemenu_dialog;

	SDL_Event event;
	EventMode mode, last_mode;
	EventInput input; // collected/received input (of any type)
// std::vector<EventMode> mode_stack; // current mode is at the end of the list
	int ts; //timestamp for TimeLeft() method.
	char alt_code_str[4]; // string representation of alt-code input
	uint8 alt_code_len; // how many characters have been input for alt-code
	uint16 active_alt_code; // alt-code that needs more input
	uint8 alt_code_input_num; // alt-code can get multiple inputs

	TimeQueue *time_queue, *game_time_queue;
	Obj *drop_obj;
	uint16 drop_qty;
	sint32 drop_x, drop_y; // only to allow pre-targeting from MapWindow, feel free to ignore this
	uint8 rest_time; // How many hours?
	uint8 rest_guard; // Who will guard?
	Obj *push_obj;
	Actor *push_actor;

	bool drop_from_key;
	bool showingDialog;
	bool showingQuitDialog;
	bool ignore_timeleft; // do not wait for NUVIE_INTERVAL
	bool move_in_inventory;
	bool in_control_cheat;
	bool looking_at_spellbook;
	bool direction_selects_target;

	uint32 fps_timestamp;
	uint16 fps_counter;
	FpsCounter *fps_counter_widget;
	ScriptThread *scriptThread;

public:
	Event(Configuration *cfg);
	virtual ~Event();

	bool init(ObjManager *om, MapWindow *mw, MsgScroll *ms, Player *p, Magic *mg,
	          GameClock *gc, ViewManager *vm, UseCode *uc, GUI *g, KeyBinder *kb);
	GUI_Dialog *get_gamemenu_dialog() {
		return gamemenu_dialog;
	}
	TimeQueue *get_time_queue() {
		return (time_queue);
	}
	TimeQueue *get_game_time_queue() {
		return (game_time_queue);
	}
	EventMode get_mode() {
		return (mode);
	}
	EventMode get_last_mode() {
		return (last_mode);
	}
	void set_mode(EventMode new_mode);

	bool is_direction_selecting_targets() {
		return direction_selects_target;
	}
	void set_direction_selects_target(bool val) {
		direction_selects_target = val;
	}
	bool using_pickpocket_cheat;
	bool cursor_mode;
	void update_timers();
	bool update();
	bool handleEvent(const SDL_Event *event);
	void request_input(CallBack *caller, void *user_data = NULL);
	void target_spell();
	void close_spellbook();
// Prompt for input.
// obsolete:
// void useselect_mode(Obj *src, const char *prompt = NULL); // deprecated
// void freeselect_mode(Obj *src, const char *prompt = NULL); // deprecated
	void get_scroll_input(const char *allowed = NULL, bool can_escape = true, bool using_target_cursor = false, bool set_numbers_only_to_true = true);
	void get_inventory_obj(Actor *actor, bool getting_target = true);
	void get_spell_num(Actor *caster, Obj *spell_container);
// void get_amount();
	void get_direction(const char *prompt);
	void get_direction(const MapCoord &from, const char *prompt);
	void get_target(const char *prompt);
	void get_target(const MapCoord &init, const char *prompt);
// void get_obj_from_inventory(Actor *actor, const char *prompt);
	void display_portrait(Actor *actor, const char *name = NULL);
// Start a new action, setting a new mode and prompting for input.
	bool newAction(EventMode new_mode);
// void doAction(sint16 rel_x = 0, sint16 rel_y = 0);
// void doAction(Obj *obj);
	void doAction();
	void cancelAction();
	void endAction(bool prompt = false);
// Send input back to Event, performing an action for the current mode.
	bool select_obj(Obj *obj, Actor *actor = NULL);
	bool select_view_obj(Obj *obj, Actor *actor);
	bool select_actor(Actor *actor);
	bool select_direction(sint16 rel_x, sint16 rel_y);
	bool select_target(uint16 x, uint16 y, uint8 z = 0);
	bool select_party_member(uint8 num);
	bool select_spell_num(sint16 spell_num);
// bool select_obj(Obj *obj = NULL, Actor *actor = NULL);
// bool select_obj(sint16 rel_x, sint16 rel_y);
// There is no "select_text", as Event polls MsgScroll for new input.
// Similiarly, a "select_key" is unnecessary. The following method
// starts sending all keyboard input to 'caller'. (with the CB_DATA_READY message)
	void key_redirect(CallBack *caller, void *user_data);
	void cancel_key_redirect();

	/* These will be replaced in the future with an InputAction class. */
	bool move(sint16 rel_x, sint16 rel_y);

	bool use_start();
	bool use(sint16 rel_x, sint16 rel_y);
	bool use(MapCoord coord);
	bool use(Obj *obj);
	bool use(Actor *actor, uint16 x, uint16 y);

	bool get_start();
	bool get(MapCoord coord);
	bool get(sint16 rel_x, sint16 rel_y);
	bool perform_get(Obj *obj, Obj *container_obj = NULL, Actor *actor = NULL);

	bool look_start();
	bool lookAtCursor(bool delayed = false, uint16 x = 0, uint16 y  = 0,  uint8 z = 0, Obj *obj = NULL, Actor *actor = NULL);
	bool look(Obj *obj);
	bool look(Actor *actor);
	bool search(Obj *obj);

	bool talk_start();
	bool talk_cursor();
	bool talk(Actor *actor);
	bool talk(Obj *obj);
	bool perform_talk(Actor *actor);

	bool attack();

	bool push_start();
	bool pushFrom(Obj *obj);
	bool pushFrom(sint16 rel_x, sint16 rel_y);
	bool pushFrom(MapCoord target);
	bool pushTo(Obj *obj, Actor *actor);
	bool pushTo(sint16 rel_x, sint16 rel_y, bool push_from = PUSH_FROM_PLAYER);

	void solo_mode(uint32 actor_num);
	bool party_mode();
	bool toggle_combat();

	bool ready(Obj *obj, Actor *actor = NULL);
	bool unready(Obj *obj);

	bool drop_start();
	bool drop_select(Obj *obj, uint16 qty = 0);
	bool drop_count(uint16 qty);
	bool perform_drop();
	void set_drop_from_key(bool closing_gumps) {
		drop_from_key = closing_gumps;
	}
	bool drop(Obj *obj, uint16 qty, uint16 x, uint16 y);
	bool drop(uint16 x, uint16 y) {
		return (drop(drop_obj, drop_qty, x, y));
	}
	void set_drop_target(uint16 x, uint16 y) {
		drop_x = sint32(x);
		drop_y = sint32(y);
	}
	bool can_move_obj_between_actors(Obj *obj, Actor *src_actor, Actor *target_actor, bool display_name = false);
	void display_not_aboard_vehicle(bool show_prompt = true);
	void display_move_text(Actor *target_actor, Obj *obj);
	bool can_get_to_actor(Actor *actor, uint16 x, uint16 y);
	bool using_control_cheat() {
		return in_control_cheat;
	}
	void set_control_cheat(bool control_cheat) {
		in_control_cheat = control_cheat;
	}
	bool is_looking_at_spellbook() {
		return looking_at_spellbook;
	}
	void set_looking_at_spellbook(bool looking) {
		looking_at_spellbook = looking;
	}

	bool rest();
	bool rest_input(uint16 input);

	void cast_spell_directly(uint8 spell_num);
	bool can_target_icon(); // Target the actor or container tile in inventory and party view

// these are both for mouse-using convenience
	void walk_to_mouse_cursor(uint32 mx, uint32 my);
	void multiuse(uint16 wx, uint16 wy);

	void alt_code(const char *cs);
	void alt_code_input(const char *in);
	void clear_alt_code() {
		alt_code_str[0] = '\0';
		alt_code_len = 0;
	}
	bool alt_code_teleport(const char *location_string);
	void alt_code_infostring();
	void alt_code_teleport_menu(uint32 selection);
	bool alt_code_teleport_to_person(uint32 npc);

	void wait();
	void set_ignore_timeleft(bool newsetting) {
		ignore_timeleft = newsetting;
	}
	EventInput *get_input() {
		return &input;
	}
// These cursor methods are use to make sure Event knows where the cursor is
// when objects are selected with ENTER. (since MapWindow and InventoryView
// may each independantly show/hide their own cursors)
	void moveCursorToMapWindow(bool ToggleCursor = false);
	void moveCursorToInventory();

	void toggleFpsDisplay();
	void close_gumps();
	bool do_not_show_target_cursor;
	bool dont_show_target_cursor();
	bool input_really_needs_directon();
	void quitDialog();
	void saveDialog();
	void gameMenuDialog();
	bool actor_exists(Actor *a);

	/* FIXME: Some of the above (action) functions can be removed from public, so
	   that we don't need to check for WAIT mode in all of them. */
protected:

	inline uint32 TimeLeft();

	uint16 callback(uint16 msg, CallBack *caller, void *data);
	bool handleSDL_KEYDOWN(const SDL_Event *event);
	const char *print_mode(EventMode mode);
	void try_next_attack();

};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
