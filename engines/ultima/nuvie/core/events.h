/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NUVIE_CORE_EVENT_H
#define NUVIE_CORE_EVENT_H

#include "ultima/shared/engine/events.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"
#include "ultima/nuvie/misc/call_back.h"
#include "ultima/nuvie/keybinding/keys_enum.h"
#include "ultima/nuvie/core/obj_manager.h"

namespace Ultima {
namespace Nuvie {

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

enum EventMode {
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
};

extern uint32 nuvieGameCounter;

// type of input that event may collect and send somewhere
#define EVENTINPUT_MAPCOORD 0
#define EVENTINPUT_KEY      1
#define EVENTINPUT_STRING   2
#define EVENTINPUT_OBJECT   3
#define EVENTINPUT_MAPCOORD_DIR 4
#define EVENTINPUT_SPELL_NUM 5

/**
 * Joystick actions mapped to dummy unused keycode values
 */
const Common::KeyCode FIRST_JOY = (Common::KeyCode)400;
const Common::KeyCode JOY_UP = FIRST_JOY;               // PS d-pad when analog is disabled. left stick when enabled
const Common::KeyCode JOY_DOWN = (Common::KeyCode)(FIRST_JOY + 1);
const Common::KeyCode JOY_LEFT = (Common::KeyCode)(FIRST_JOY + 2);
const Common::KeyCode JOY_RIGHT = (Common::KeyCode)(FIRST_JOY + 3);
const Common::KeyCode JOY_RIGHTUP = (Common::KeyCode)(FIRST_JOY + 4);
const Common::KeyCode JOY_RIGHTDOWN = (Common::KeyCode)(FIRST_JOY + 5);
const Common::KeyCode JOY_LEFTUP = (Common::KeyCode)(FIRST_JOY + 6);
const Common::KeyCode JOY_LEFTDOWN = (Common::KeyCode)(FIRST_JOY + 7);
const Common::KeyCode JOY_UP2 = (Common::KeyCode)(FIRST_JOY + 8); // PS right stick when analog is enabled
const Common::KeyCode JOY_DOWN2 = (Common::KeyCode)(FIRST_JOY + 9);
const Common::KeyCode JOY_LEFT2 = (Common::KeyCode)(FIRST_JOY + 10);
const Common::KeyCode JOY_RIGHT2 = (Common::KeyCode)(FIRST_JOY + 11);
const Common::KeyCode JOY_RIGHTUP2 = (Common::KeyCode)(FIRST_JOY + 12);
const Common::KeyCode JOY_RIGHTDOWN2 = (Common::KeyCode)(FIRST_JOY + 13);
const Common::KeyCode JOY_LEFTUP2 = (Common::KeyCode)(FIRST_JOY + 14);
const Common::KeyCode JOY_LEFTDOWN2 = (Common::KeyCode)(FIRST_JOY + 15);
const Common::KeyCode JOY_UP3 = (Common::KeyCode)(FIRST_JOY + 16);
const Common::KeyCode JOY_DOWN3 = (Common::KeyCode)(FIRST_JOY + 17);
const Common::KeyCode JOY_LEFT3 = (Common::KeyCode)(FIRST_JOY + 18);
const Common::KeyCode JOY_RIGHT3 = (Common::KeyCode)(FIRST_JOY + 19);
const Common::KeyCode JOY_RIGHTUP3 = (Common::KeyCode)(FIRST_JOY + 20);
const Common::KeyCode JOY_RIGHTDOWN3 = (Common::KeyCode)(FIRST_JOY + 21);
const Common::KeyCode JOY_LEFTUP3 = (Common::KeyCode)(FIRST_JOY + 22);
const Common::KeyCode JOY_LEFTDOWN3 = (Common::KeyCode)(FIRST_JOY + 23);
const Common::KeyCode JOY_UP4 = (Common::KeyCode)(FIRST_JOY + 24);
const Common::KeyCode JOY_DOWN4 = (Common::KeyCode)(FIRST_JOY + 25);
const Common::KeyCode JOY_LEFT4 = (Common::KeyCode)(FIRST_JOY + 26);
const Common::KeyCode JOY_RIGHT4 = (Common::KeyCode)(FIRST_JOY + 27);
const Common::KeyCode JOY_RIGHTUP4 = (Common::KeyCode)(FIRST_JOY + 28);
const Common::KeyCode JOY_RIGHTDOWN4 = (Common::KeyCode)(FIRST_JOY + 29);
const Common::KeyCode JOY_LEFTUP4 = (Common::KeyCode)(FIRST_JOY + 30);
const Common::KeyCode JOY_LEFTDOWN4 = (Common::KeyCode)(FIRST_JOY + 31);
const Common::KeyCode JOY_HAT_UP = (Common::KeyCode)(FIRST_JOY + 32); // PS d-pad when analog is enabled
const Common::KeyCode JOY_HAT_DOWN = (Common::KeyCode)(FIRST_JOY + 33);
const Common::KeyCode JOY_HAT_LEFT = (Common::KeyCode)(FIRST_JOY + 34);
const Common::KeyCode JOY_HAT_RIGHT = (Common::KeyCode)(FIRST_JOY + 35);
const Common::KeyCode JOY_HAT_RIGHTUP = (Common::KeyCode)(FIRST_JOY + 36);
const Common::KeyCode JOY_HAT_RIGHTDOWN = (Common::KeyCode)(FIRST_JOY + 37);
const Common::KeyCode JOY_HAT_LEFTUP = (Common::KeyCode)(FIRST_JOY + 38);
const Common::KeyCode JOY_HAT_LEFTDOWN = (Common::KeyCode)(FIRST_JOY + 39);
const Common::KeyCode JOY0 = (Common::KeyCode)(FIRST_JOY + 40); // PS triangle
const Common::KeyCode JOY1 = (Common::KeyCode)(FIRST_JOY + 41); // PS circle
const Common::KeyCode JOY2 = (Common::KeyCode)(FIRST_JOY + 42); // PS x
const Common::KeyCode JOY3 = (Common::KeyCode)(FIRST_JOY + 43); // PS square
const Common::KeyCode JOY4 = (Common::KeyCode)(FIRST_JOY + 44); // PS L2
const Common::KeyCode JOY5 = (Common::KeyCode)(FIRST_JOY + 45); // PS R2
const Common::KeyCode JOY6 = (Common::KeyCode)(FIRST_JOY + 46); // PS L1
const Common::KeyCode JOY7 = (Common::KeyCode)(FIRST_JOY + 47); // PS R1
const Common::KeyCode JOY8 = (Common::KeyCode)(FIRST_JOY + 48); // PS select
const Common::KeyCode JOY9 = (Common::KeyCode)(FIRST_JOY + 49); // PS start
const Common::KeyCode JOY10 = (Common::KeyCode)(FIRST_JOY + 50); // PS L3 (analog must be enabled)
const Common::KeyCode JOY11 = (Common::KeyCode)(FIRST_JOY + 51); // PS R3 (analog must be enabled)
const Common::KeyCode JOY12 = (Common::KeyCode)(FIRST_JOY + 52);
const Common::KeyCode JOY13 = (Common::KeyCode)(FIRST_JOY + 53);
const Common::KeyCode JOY14 = (Common::KeyCode)(FIRST_JOY + 54);
const Common::KeyCode JOY15 = (Common::KeyCode)(FIRST_JOY + 55);
const Common::KeyCode JOY16 = (Common::KeyCode)(FIRST_JOY + 56);
const Common::KeyCode JOY17 = (Common::KeyCode)(FIRST_JOY + 57);
const Common::KeyCode JOY18 = (Common::KeyCode)(FIRST_JOY + 58);
const Common::KeyCode JOY19 = (Common::KeyCode)(FIRST_JOY + 59);


struct EventInput_s {
	uint8 type; // 0=loc,1=key,2=str,3=obj,4=actor
//    union
//    {
	Common::KeyCode key; // last key entered, if capturing input
	ActionKeyType action_key_type; // last ActionKeyType entered if capturing input
	MapCoord *loc; // target location, or direction if relative ???
	Std::string *str; // ???
//    };
	void set_loc(const MapCoord &c);
	EventInput_s() : loc(0), str(0), obj(0), actor(0), get_direction(false), get_text(false),
		target_init(0), select_from_inventory(false), select_range(0), key(Common::KEYCODE_INVALID),
		action_key_type(ActionKeyType::CANCEL_ACTION_KEY), spell_num(0), type(0) {
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

class Events : public Ultima::Shared::EventsManager, public CallBack {
	friend class Magic; // FIXME
private:
	const Configuration *config;
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
	GUI_Dialog *assetviewer_dialog;

	Common::Event event;
	EventMode mode, last_mode;
	EventInput input; // collected/received input (of any type)
// Std::vector<EventMode> mode_stack; // current mode is at the end of the list
	int ts; //timestamp for TimeLeft() method.
	int altCodeVal;
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
	bool _keymapperStateBeforeKEYINPUT;

	uint32 fps_timestamp;
	uint16 fps_counter;
	FpsCounter *fps_counter_widget;
	ScriptThread *scriptThread;

	static Events *g_events;
protected:
	inline uint32 TimeLeft();

	uint16 callback(uint16 msg, CallBack *caller, void *data = nullptr) override;
	bool handleSDL_KEYDOWN(const Common::Event *event);
	const char *print_mode(EventMode mode);
	void try_next_attack();

public:
	Events(Shared::EventsCallback *callback, const Configuration *cfg);
	~Events() override;

	void clear();

	bool init(ObjManager *om, MapWindow *mw, MsgScroll *ms, Player *p, Magic *mg,
	          GameClock *gc, ViewManager *vm, UseCode *uc, GUI *g, KeyBinder *kb);
	GUI_Dialog *get_gamemenu_dialog() {
		return gamemenu_dialog;
	}
	TimeQueue *get_time_queue() {
		return time_queue;
	}
	TimeQueue *get_game_time_queue() {
		return game_time_queue;
	}
	EventMode get_mode() const {
		return mode;
	}
	EventMode get_last_mode() const {
		return last_mode;
	}
	void set_mode(EventMode new_mode);

	bool is_direction_selecting_targets() const {
		return direction_selects_target;
	}
	void set_direction_selects_target(bool val) {
		direction_selects_target = val;
	}
	bool using_pickpocket_cheat;
	bool cursor_mode;
	void update_timers();
	bool update();
	bool handleEvent(const Common::Event *event);
	void request_input(CallBack *caller, void *user_data = nullptr);
	void target_spell();
	void close_spellbook();
// Prompt for input.
// obsolete:
// void useselect_mode(Obj *src, const char *prompt = nullptr); // deprecated
// void freeselect_mode(Obj *src, const char *prompt = nullptr); // deprecated
	void get_scroll_input(const char *allowed = nullptr, bool can_escape = true, bool using_target_cursor = false, bool set_numbers_only_to_true = true);
	void get_inventory_obj(Actor *actor, bool getting_target = true);
	void get_spell_num(Actor *caster, Obj *spell_container);
// void get_amount();
	void get_direction(const char *prompt);
	void get_direction(const MapCoord &from, const char *prompt);
	void get_target(const char *prompt);
	void get_target(const MapCoord &init, const char *prompt);
// void get_obj_from_inventory(Actor *actor, const char *prompt);
	void display_portrait(Actor *actor, const char *name = nullptr);
// Start a new action, setting a new mode and prompting for input.
	bool newAction(EventMode new_mode);
// void doAction(sint16 rel_x = 0, sint16 rel_y = 0);
// void doAction(Obj *obj);
	void doAction();
	void cancelAction();
	void endAction(bool prompt = false);
// Send input back to Events, performing an action for the current mode.
	bool select_obj(Obj *obj, Actor *actor = nullptr);
	bool select_view_obj(Obj *obj, Actor *actor);
	bool select_actor(Actor *actor);
	bool select_direction(sint16 rel_x, sint16 rel_y);
	bool select_target(uint16 x, uint16 y, uint8 z = 0);
	bool select_party_member(uint8 num);
	bool select_spell_num(sint16 spell_num);
// bool select_obj(Obj *obj = nullptr, Actor *actor = nullptr);
// bool select_obj(sint16 rel_x, sint16 rel_y);
// There is no "select_text", as Events polls MsgScroll for new input.
// Similarly, a "select_key" is unnecessary. The following method
// starts sending all keyboard input to 'caller'. (with the CB_DATA_READY message)
	void key_redirect(CallBack *caller, void *user_data);
	void cancel_key_redirect();

	/* These will be replaced in the future with an InputAction class. */
	bool move(sint16 rel_x, sint16 rel_y);

	bool use_start();
	bool use(sint16 rel_x, sint16 rel_y);
	bool use(const MapCoord &coord);
	bool use(Obj *obj);
	bool use(Actor *actor, uint16 x, uint16 y);

	bool get_start();
	bool get(const MapCoord &coord);
	bool get(sint16 rel_x, sint16 rel_y);
	bool perform_get(Obj *obj, Obj *container_obj = nullptr, Actor *actor = nullptr);

	bool look_start();
	bool lookAtCursor(bool delayed = false, uint16 x = 0, uint16 y  = 0,  uint8 z = 0, Obj *obj = nullptr, Actor *actor = nullptr);
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
	bool pushFrom(const MapCoord &target);
	bool pushTo(Obj *obj, Actor *actor);
	bool pushTo(sint16 rel_x, sint16 rel_y, bool push_from = PUSH_FROM_PLAYER);

	void solo_mode(uint32 actor_num);
	bool party_mode();
	bool toggle_combat();

	bool ready(Obj *obj, Actor *actor = nullptr);
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
	bool can_get_to_actor(const Actor *actor, uint16 x, uint16 y);
	bool using_control_cheat() const {
		return in_control_cheat;
	}
	void set_control_cheat(bool control_cheat) {
		in_control_cheat = control_cheat;
	}
	bool is_looking_at_spellbook() const {
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

	void alt_code(int c);
	void alt_code_input(const char *in);
	void clear_alt_code() { altCodeVal = 0; }

	void toggleAltCodeMode(bool enable);
	void appendAltCode(int code);

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
// These cursor methods are use to make sure Events knows where the cursor is
// when objects are selected with ENTER. (since MapWindow and InventoryView
// may each independently show/hide their own cursors)
	void moveCursorToMapWindow(bool ToggleCursor = false);
	void moveCursorToInventory();

	void toggleFpsDisplay();
	void close_gumps();
	bool do_not_show_target_cursor;
	bool dont_show_target_cursor() const;
	bool input_really_needs_directon() const;
	void quitDialog();
	void gameMenuDialog();
	void assetViewer();
	bool actor_exists(const Actor *a) const;

	/* FIXME: Some of the above (action) functions can be removed from public, so
	   that we don't need to check for WAIT mode in all of them. */

	   /**
		* Gets a reference to the events manager
		*/
	static Events *get() { return g_events; }
};

extern bool shouldQuit();

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
