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

#ifndef M4_BURGER_VARS_H
#define M4_BURGER_VARS_H

#include "common/textconsole.h"
#include "m4/vars.h"
#include "m4/burger/core/release_trigger.h"
#include "m4/burger/core/stream_break.h"
#include "m4/burger/gui/gui.h"
#include "m4/burger/gui/gui_gizmo.h"
#include "m4/burger/gui/game_menu.h"
#include "m4/burger/flags.h"
#include "m4/burger/hotkeys.h"
#include "m4/burger/inventory.h"
#include "m4/burger/series_player.h"
#include "m4/burger/walker.h"

namespace M4 {
namespace Burger {

enum GameMode {
	JUST_OVERVIEW = 0, INTERACTIVE_DEMO = 1, MAGAZINE_DEMO = 2, WHOLE_GAME = 3
};

enum global_triggers {
	gFIRST_GLOBAL_TRIGGER = 10000,

	// System triggers 
	gRESUME_CONVERSATION = 10001,	// Yep, that's what it does!
	gSERIES_STREAM_BREAK = 10002,	// Used only by set_next_stream_break () in global.cpp
	gSERIES_PLAY_BREAK_0 = 10003,
	gSERIES_PLAY_BREAK_1 = 10004,
	gSERIES_PLAY_BREAK_2 = 10005,
	gRELEASE_TRIGGER_DIGI_CHECK = 10006,	// Used only by release_trigger_digi_check() in global.cpp

	gNPC_SPEECH_STARTED = 10010,	// NPC talk started
	gNPC_SPEECH_FINISHED = 10011,	// NPC talk finished
	g10013 = 10013,
	gWILBURS_SPEECH_FINISHED = 10014,
	gWILBURS_SPEECH_START = 10015,
	gCHANGE_WILBUR_ANIMATION = 10016,
	gSET_FACING = 10018,
	kSET_DEST = 10019,
	gSET_COMMANDS_ALLOWED = 10020,
	gUNPOOF = 10021,
	gTELEPROTED1 = 10024,
	gTELEPROTED2 = 10025,
	g10027 = 10027,
	g10028 = 10028,

	kCALLED_EACH_LOOP = 32764,

	gLAST_GLOBAL_TRIGGER
#if UNUSED
	gREACT_TO_WHISTLE,				// For anything that reacts to the wistle being blown
	gCHANGE_WILBUR_ANIMATION,
#endif
};

class Vars : public M4::Vars {
private:
	static void custom_ascii_converter_proc(char *string);
	static void escape_key_pressed(void *, void *);
	static void cb_F2(void *, void *);
	static void cb_F3(void *, void *);

protected:
	void main_cold_data_init() override;

public:
	// These are defined as statics because they're referenced in data arrays
	static long _wilbur_should;
public:
	Burger::Flags _flags;
	GameMode _executing = WHOLE_GAME;
	GUI::GUI_Globals _gui;
	GUI::Gizmo_Globals _gizmo;
	GUI::Interface _interface;
	Burger::Hotkeys _hotkeys;
	GUI::MenuGlobals _menu;
	SeriesPlayers _seriesPlayers;
	ReleaseTrigger_Globals _releaseTrigger;
	StreamBreak_Globals _streamBreak;
	Burger::Walker _walker;

	int _wilburTerm = 2;
	bool _menuSystemInitialized = false;
	bool _gameMenuFromMain = false;
	int _room902Flag = 0;
	int _roomVal3 = 0;
	int _roomVal4 = 0;
	int _roomVal7 = 3;
	machine *_npcSpeech1 = nullptr;
	machine *_npcSpeech2 = nullptr;
	int _npcTrigger = -1;
	int _player_facing_x = 0, _player_facing_y = 0;
	int _player_dest_x = 0, _player_dest_y = 0;
	int _player_trigger = 0;

public:
	Vars();
	virtual ~Vars();

	M4::Interface *getInterface() override {
		return &_interface;
	}
	Hotkeys *getHotkeys() override {
		return &_hotkeys;
	}
	M4::Walker *getWalker() override {
		return &_walker;
	}
	void initialize_game() override;

	void global_menu_system_init() override;
};

extern Vars *g_vars;

#undef _G
#undef _GI
#define _G(X) (::M4::Burger::g_vars->_##X)
#define _GI(X) _G(interface)._##X
#define _GINT() _G(interface)

} // namespace Burger
} // namespace M4

#endif
