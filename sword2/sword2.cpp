/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "common/config-manager.h"
#include "bs2/build_display.h"
#include "bs2/console.h"
#include "bs2/controls.h"
#include "bs2/credits.h"
#include "bs2/debug.h"
#include "bs2/events.h"
#include "bs2/header.h"
#include "bs2/interpreter.h"
#include "bs2/layers.h"
#include "bs2/logic.h"
#include "bs2/maketext.h"
#include "bs2/memory.h"
#include "bs2/mouse.h"
#include "bs2/protocol.h"
#include "bs2/resman.h"
#include "bs2/save_rest.h"
#include "bs2/scroll.h"
#include "bs2/sound.h"
#include "bs2/speech.h"
#include "bs2/startup.h"
#include "bs2/sword2.h"
#include "bs2/sync.h"
#include "bs2/driver/driver96.h"
#include "bs2/driver/palette.h"

extern uint16 _debugLevel;

static const TargetSettings sword2_settings[] = {
	/* Broken Sword 2 */
	{"sword2", "Broken Sword II", GID_SWORD2, 99, MDT_ADLIB | MDT_NATIVE, GF_DEFAULT_TO_1X_SCALER, "players.clu" },
	{"sword2alt", "Broken Sword II (alt)", GID_SWORD2, 99, MDT_ADLIB | MDT_NATIVE, GF_DEFAULT_TO_1X_SCALER, "r2ctlns.ocx" },
	{"sword2demo", "Broken Sword II (Demo)", GID_SWORD2_DEMO, 99, MDT_ADLIB | MDT_NATIVE, GF_DEFAULT_TO_1X_SCALER, "players.clu" },
	{NULL, NULL, 0, 0, MDT_NONE, 0, NULL}
};

const TargetSettings *Engine_SWORD2_targetList() {
	return sword2_settings;
}

Engine *Engine_SWORD2_create(GameDetector *detector, OSystem *syst) {
	return new Sword2::Sword2Engine(detector, syst);
}

REGISTER_PLUGIN("Broken Sword II", Engine_SWORD2_targetList, Engine_SWORD2_create);

namespace Sword2 {

uint8 quitGame = 0;

// version & owner details

// So version string is 18 bytes long :
// Version String =  <8 byte header,5 character version, \0, INT32 time>

uint8 version_string[HEAD_LEN + 10] = { 1, 255, 37, 22, 45, 128, 34, 67 };

uint8 unencoded_name[HEAD_LEN + 48] = {
	76, 185, 205, 23, 44, 34, 24, 34,
	'R','e','v','o','l','u','t','i','o','n',' ',
	'S','o','f','t','w','a','r','e',' ','L','t','d',
	0 };

uint8 encoded_name[HEAD_LEN + 48] = {
	44, 32, 190, 222, 123, 65, 233, 99,
	179, 209, 225, 157, 222, 238, 219, 209, 143, 224, 133, 190,
	232, 209, 162, 177, 198, 228, 202, 146, 180, 232, 214, 65,
	65, 65, 116, 104, 116, 114, 107, 104, 32, 49, 64, 35, 123,
	125, 61, 45, 41, 40, 163, 36, 49, 123, 125, 10 };

uint8 gamePaused = 0;
uint8 graphics_level_fudged = 0;
uint8 stepOneCycle = 0;			// for use while game paused

Sword2Engine *g_sword2 = NULL;
Sound *g_sound = NULL;

Sword2Engine::Sword2Engine(GameDetector *detector, OSystem *syst)
	: Engine(detector, syst) {
	
	_detector = detector;
	g_sword2 = this;
	_features = detector->_game.features;
	_gameId = detector->_game.id;
	_game_name = strdup(detector->_gameFileName.c_str());
	_bootParam = ConfMan.getInt("boot_param");
	_saveSlot = ConfMan.getInt("save_slot");
	_debugLevel = ConfMan.getInt("debuglevel");

	// Setup mixer
	if (!_mixer->bindToSystem(syst))
		warning("Sound initialization failed");

	// We have our own volume settings panel, so don't let ScummVM's mixer
	// soften the sound in any way.

	_mixer->setVolume(256);
	_mixer->setMusicVolume(256);

	g_sound = _sound = new Sound(_mixer);
	
	File::setDefaultDirectory(_gameDataPath);
}

void Sword2Engine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int32 Sword2Engine::InitialiseGame(void) {
	// init engine drivers

	uint8 *file;

	// get some falling RAM and put it in your pocket, never let it slip
	// away

	debug(5, "CALLING: memory.init");
	memory.init();

	// initialise the resource manager
	debug(5, "CALLING: res_man.init");
	res_man.init();

	// initialise global script variables
	// res 1 is the globals list
	file = res_man.open(1);
	debug(5, "CALLING: SetGlobalInterpreterVariables");
	SetGlobalInterpreterVariables((int32 * ) (file + sizeof(_standardHeader)));

	// DON'T CLOSE VARIABLES RESOURCE - KEEP IT OPEN AT VERY START OF
	// MEMORY SO IT CAN'T MOVE!

	// DON'T CLOSE PLAYER OBJECT RESOURCE - KEEP IT OPEN IN MEMORY SO IT
	// CAN'T MOVE!

	file = res_man.open(8);

	// Set up font resource variables for this language version

	debug(5, "CALLING: InitialiseFontResourceFlags");
	InitialiseFontResourceFlags();

	// set up the console system

	debug(5, "CALLING: Init_console");
	Init_console();

#ifdef _SWORD2_DEBUG
	// read in all the startup information

	debug(5, "CALLING: Init_start_menu");
	Init_start_menu();
#endif

	// no blocs live

	debug(5, "CALLING: Init_text_bloc_system");
	Init_text_bloc_system();

	debug(5, "CALLING: Init_sync_system");
	Init_sync_system();

	debug(5, "CALLING: Init_event_system");
	Init_event_system();
	
	// initialise the sound fx queue

	debug(5, "CALLING: Init_fx_queue");
	Init_fx_queue();

	// all demos (not just web)
	if (_gameId == GID_SWORD2_DEMO) {
		// set script variable
		DEMO = 1;
	}

	return 0;
}

void Close_game() {
	debug(5, "Close_game() STARTING:");
	EraseBackBuffer();

	// Stop music instantly!
	Kill_music();

	// free the memory again
	memory.exit();
	res_man.exit();
}

int32 GameCycle(void) {
	// do one game cycle

	//got a screen to run?
	if (LLogic.getRunList()) {
		//run the logic session UNTIL a full loop has been performed
		do {
			// reset the graphic 'buildit' list before a new
			// logic list (see FN_register_frame)
			Reset_render_lists();

			// reset the mouse hot-spot list (see FN_register_mouse
			// & FN_register_frame)
			Reset_mouse_list();

			// keep going as long as new lists keep getting put in
			// - i.e. screen changes
		} while (LLogic.processSession());
	} else {
		// start the console and print the start options perhaps?
		StartConsole();
		Print_to_console("AWAITING START COMMAND: (Enter 's 1' then 'q' to start from beginning)");
	}

	// if this screen is wide, recompute the scroll offsets every cycle
	if (this_screen.scroll_flag)
		Set_scrolling();

	Mouse_engine();
	Process_fx_queue();

	// update age and calculate previous cycle memory usage
	res_man.nextCycle();

	if (quitGame)
		return 1;

	return 0;
}

void Sword2Engine::go() {
	uint32 rv;
	uint8 breakOut = 0;
	_keyboardEvent ke;

	// Call the application "Revolution" until the resource manager is
	// ready to dig the name out of a text file. See InitialiseGame()
	// which calls InitialiseFontResourceFlags() in maketext.cpp
	//
	// Have to do it like this since we cannot really fire up the resource
	// manager until a window has been created as any errors are displayed
	// via a window, thus time becomes a loop.

	debug(5, "CALLING: InitialiseDisplay");
	rv = InitialiseDisplay(640, 480);

	if (rv != RD_OK) {
		// ReportDriverError(rv);
		CloseAppWindow();
		return;
	}

	debug(5, "CALLING: readOptionSettings");
	gui.readOptionSettings();

	debug(5, "CALLING: InitialiseGame");
	if (InitialiseGame()) {
		CloseAppWindow();
		return;
	}

	if (_saveSlot != -1) {
		if (SaveExists(_saveSlot))
			RestoreGame(_saveSlot);
		else { // show restore menu
			Set_mouse(NORMAL_MOUSE_ID);
			if (!gui.restoreControl())
				Start_game();
		}
	} else
		Start_game();

	debug(5, "CALLING: InitialiseRenderCycle");
	InitialiseRenderCycle();

	while (1) {
		ServiceWindows();

#ifdef _SWORD2_DEBUG
// FIXME: If we want this, we should re-work it to use the backend's
// screenshot functionality.
//		if (grabbingSequences && !console_status)
//			GrabScreenShot();
#endif

		// if we are closing down the game, break out of main game loop
		if (breakOut)
			break;

#ifdef _SWORD2_DEBUG
		if (console_status) {
			if (One_console()) {
				EndConsole();
				UnpauseAllSound();	// see sound.cpp
			}
		}
#endif

		// not in console mode - if the console is quit we want to get
		// a logic cycle in before

		if (!console_status) {
			// the screen is build. Mostly because of first scroll
			// cycle stuff

#ifdef _SWORD2_DEBUG
			// if we've just stepped forward one cycle while the
			// game was paused

			if (stepOneCycle) {
				PauseGame();
				stepOneCycle = 0;
			}
#endif

			if (KeyWaiting()) {
				ReadKey(&ke);

				char c = toupper(ke.ascii);

#ifdef _SWORD2_DEBUG
				// ESC whether paused or not
				if (ke.keycode == 27) {
					PauseAllSound(); // see sound.cpp
					StartConsole();	 // start the console
				} else
#endif
				if (gamePaused) {	// if currently paused
					if (c == 'P') {
						// 'P' while paused = unpause!
						UnpauseGame();
					}
#ifdef _SWORD2_DEBUG
					// frame-skipping only allowed on
					// debug version

					else if (c == ' ') {
						// SPACE bar while paused =
						// step one frame!
						stepOneCycle = 1;
						UnpauseGame();
					}
#endif
				} else if (c == 'P') {
					// 'P' while not paused = pause!
					PauseGame();
				} else if (c == 'C' && _gameId == GID_SWORD2) {
					FN_play_credits(NULL);
				}
#ifdef _SWORD2_DEBUG
				else if (c == 'S') {
					// 'S' toggles speed up (by skipping
					// display rendering)
					renderSkip = 1 - renderSkip;
				}
#endif
			}

			// skip GameCycle if we're paused
			if (gamePaused == 0) {
#ifdef _SWORD2_DEBUG
				gameCycle++;
#endif

				if (GameCycle()) {
					// break out of main game loop
					break;
				}
			}

#ifdef _SWORD2_DEBUG
			// creates the debug text blocks
			Build_debug_text();
#endif
		}

		// James (24mar97)

#ifdef _SWORD2_DEBUG
		// if not in console & 'renderSkip' is set, only render
		// display once every 4 game-cycles

		if (console_status || renderSkip == 0 || (gameCycle % 4) == 0)
			Build_display();	// create and flip the screen
#else
		// create and flip the screen
		Build_display();
#endif
	}

	Close_game();		//close engine systems down
	CloseAppWindow();

	return;			//quit the game
}

void Sword2Engine::Start_game(void) {
	// boot the game straight into a start script

	int screen_manager_id;

	debug(5, "Start_game() STARTING:");

	// all demos not just web
	if (_gameId == GID_SWORD2_DEMO)
		screen_manager_id = 19;		// DOCKS SECTION START
	else
		screen_manager_id = 949;	// INTRO & PARIS START

	// FIXME this could be validated against startup.inf for valid numbers
	// to stop people shooting themselves in the foot

	if (_bootParam != 0)
		screen_manager_id = _bootParam;
	
	char *raw_script;
	char *raw_data_ad;

	// the required start-scripts are both script #1 in the respective
	// ScreenManager objects

	uint32 null_pc = 1;

	// open george object, ready for start script to reference
	raw_data_ad = (char *) res_man.open(8);

	// open the ScreenManager object
	raw_script = (char *) res_man.open(screen_manager_id);

	// run the start script now (because no console)
	RunScript(raw_script, raw_data_ad, &null_pc);

	// close the ScreenManager object
	res_man.close(screen_manager_id);

	// close george
	res_man.close(8);

	debug(5, "Start_game() DONE.");
}

// FIXME: Move this to some better place?

void sleepUntil(int32 time) {
	while ((int32) SVM_timeGetTime() < time) {
		g_sword2->parseEvents();

		// Make sure menu animations and fades don't suffer
		ProcessMenu();
		ServiceWindows();

		g_system->delay_msecs(10);
	}
}

void PauseGame(void) {
	// uint8 *text;

	// open text file & get the line "PAUSED"
	// text = FetchTextLine(res_man.open(3258), 449);
	// pause_text_bloc_no = Build_new_block(text + 2, 320, 210, 640, 184, RDSPR_TRANS | RDSPR_DISPLAYALIGN, SPEECH_FONT_ID, POSITION_AT_CENTRE_OF_BASE);
	// now ok to close the text file
	// res_man.close(3258);

	// don't allow Pause while screen fading or while black (James 03sep97)
	if (GetFadeStatus() != RDFADE_NONE)
		return;
	
  	PauseAllSound();

	//make a normal mouse
	ClearPointerText();

	// mouse_mode=MOUSE_normal;

	//this is the only place allowed to do it this way
	SetLuggageAnim(NULL, 0);

	// blank cursor
	Set_mouse(0);

	// forces engine to choose a cursor
	mouse_touching = 1;

	// if level at max, turn down because palette-matching won't work
	// when dimmed

	if (gui._currentGraphicsLevel == 3) {
		gui.updateGraphicsLevel(2);
		graphics_level_fudged = 1;
	}

	// don't dim it if we're single-stepping through frames
	// dim the palette during the pause (James26jun97)

	if (stepOneCycle == 0)
  		DimPalette();

	gamePaused = 1;
}

void UnpauseGame(void) {
	// removed "PAUSED" from screen
	// Kill_text_bloc(pause_text_bloc_no);

	if (OBJECT_HELD && real_luggage_item)
		Set_luggage(real_luggage_item);

	UnpauseAllSound();

	// put back game screen palette; see Build_display.cpp
	SetFullPalette(0xffffffff);

	// If graphics level at max, turn up again
 	if (graphics_level_fudged) {
		gui.updateGraphicsLevel(3);
		graphics_level_fudged = 0;
	}

	gamePaused = 0;
	unpause_zone = 2;

	// if mouse is about or we're in a chooser menu
	if (!mouse_status || choosing)
		Set_mouse(NORMAL_MOUSE_ID);
}

} // End of namespace Sword2
