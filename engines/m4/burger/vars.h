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
	kFIRST_GLOBAL_TRIGGER = 10000,

	// System triggers 
	kRESUME_CONVERSATION = 10001,	// Yep, that's what it does!
	kSERIES_STREAM_BREAK = 10002,	// Used only by set_next_stream_break () in global.cpp
	kSERIES_PLAY_BREAK_0 = 10003,
	kSERIES_PLAY_BREAK_1 = 10004,
	kSERIES_PLAY_BREAK_2 = 10005,
	kRELEASE_TRIGGER_DIGI_CHECK = 10006,	// Used only by release_trigger_digi_check() in global.cpp
	kAdvanceTime = 10007,

	kNPC_SPEECH_STARTED = 10010,	// NPC talk started
	kNPC_SPEECH_FINISHED = 10011,	// NPC talk finished
	kWILBUR_SPEECH_STARTED = 10013,
	kWILBURS_SPEECH_FINISHED = 10014,
	kWILBURS_SPEECH_START = 10015,
	kCHANGE_WILBUR_ANIMATION = 10016,
	kAdvanceHour = 10017,
	kSET_FACING = 10018,
	kPOOF = 10019,
	kSET_COMMANDS_ALLOWED = 10020,
	kUNPOOF = 10021,
	kAbduction = 10022,
	kTestTimeout = 10023,
	kTELEPORTED1 = 10024,
	kTELEPORTED2 = 10025,
	k10027 = 10027,
	kBurlEntersTown = 10028,
	kBurlGetsFed = 10029,
	kBurlStopsEating = 10030,
	kBurlLeavesTown = 10031,
	kBandEntersTown = 10032,

	kCALLED_EACH_LOOP = 32764,

	kLAST_GLOBAL_TRIGGER
};

class Vars : public M4::Vars {
private:
	static void custom_ascii_converter_proc(char *string);

protected:
	void main_cold_data_init() override;

public:
	// These are defined as statics because they're referenced in data arrays
	static int32 _wilbur_should;
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
	int _flumix_should = 0;
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
