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

//------------------------------------------------------------------------------------
#include "stdafx.h"
#include "driver/driver96.h"
#include "driver/palette.h"
#include "common/gameDetector.h"
#include "common/config-file.h"
#include "common/timer.h"
#include "build_display.h"
#include "console.h"
#include "controls.h"
#include "debug.h"
#include "events.h"
#include "header.h"
#include "interpreter.h"
#include "layers.h"
#include "logic.h"
#include "maketext.h"
#include "memory.h"
#include "mouse.h"
#include "protocol.h"
#include "resman.h"
#include "scroll.h"
#include "sound.h"
#include "speech.h"
#include "startup.h"
#include "sword2.h"
#include "sync.h"

#define MAX_PATH 260

int  RunningFromCd();

uint8 quitGame = 0;

//------------------------------------------------------------------------------------
// version & owner details

//So version string is 18 bytes long :
//Version String =  <8 byte header,5 character version, \0, INT32 time>

uint8 version_string[HEAD_LEN+10]	=	{1, 255, 37, 22, 45, 128, 34, 67};

uint8 unencoded_name[HEAD_LEN+48]	=	{76, 185, 205, 23, 44, 34, 24, 34,
										'R','e','v','o','l','u','t','i','o','n',' ',
										'S','o','f','t','w','a','r','e',' ','L','t','d',
										0};

uint8 encoded_name[HEAD_LEN+48]		=	{44, 32, 190, 222, 123, 65, 233, 99,
										179, 209, 225, 157, 222, 238, 219, 209, 143, 224, 133, 190,
										232, 209, 162, 177, 198, 228, 202, 146, 180, 232, 214, 65,
										65, 65, 116, 104, 116, 114, 107, 104, 32, 49, 64, 35, 123,
										125, 61, 45, 41, 40, 163, 36, 49, 123, 125, 10};

//------------------------------------------------------------------------------------

uint8 gamePaused=0;			// James17jun97
//uint32 pause_text_bloc_no=0;	// James17jun97
uint8 graphics_level_fudged=0;	// James10july97
uint8 stepOneCycle=0;	// for use while game paused

//------------------------------------------------------------------------------------

static const VersionSettings sword2_settings[] = {
	/* Broken Sword 2 */
	{"sword2", "Broken Sword II", GID_SWORD2, 99, MDT_ADLIB | MDT_NATIVE, GF_DEFAULT_TO_1X_SCALER, "players.clu" },
	{"sword2demo", "Broken Sword II (Demo)", GID_SWORD2_DEMO, 99, MDT_ADLIB | MDT_NATIVE, GF_DEFAULT_TO_1X_SCALER, "players.clu" },
	{NULL, NULL, 0, 0, MDT_NONE, 0, NULL}
};

Sword2State *g_sword2 = NULL;

const VersionSettings *Engine_SWORD2_targetList() {
	return sword2_settings;
}

Engine *Engine_SWORD2_create(GameDetector *detector, OSystem *syst) {
	return new Sword2State(detector, syst);
}

Sword2State::Sword2State(GameDetector *detector, OSystem *syst)
	: Engine(detector, syst) {
	
	_detector = detector;
	g_sword2 = this;
	_features = detector->_game.features;
	_gameId = detector->_game.id;
	_game_name = strdup(detector->_gameFileName.c_str());
	_bootParam = detector->_bootParam;
	_saveSlot = detector->_save_slot;

	// Setup mixer
	if (!_mixer->bindToSystem(syst))
		warning("Sound initialization failed");

	_mixer->setVolume(kDefaultSFXVolume * kDefaultMasterVolume / 255);

	_sound = new Sword2Sound(_mixer);
}


void Sword2State::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int32 Sword2State::InitialiseGame(void)
{
//init engine drivers

	uint8	*file;

	Zdebug("CALLING: Init_memory_manager");
	Init_memory_manager();		// get some falling RAM and put it in your pocket, never let it slip away
	Zdebug("RETURNED.");

	Zdebug("CALLING: res_man.InitResMan");
	res_man.InitResMan();		// initialise the resource manager
	Zdebug("RETURNED from res_man.InitResMan");

								// initialise global script variables
	file=res_man.Res_open(1);	// res 1 is the globals list
	Zdebug("CALLING: SetGlobalInterpreterVariables");
	SetGlobalInterpreterVariables((int32*)(file+sizeof(_standardHeader)));
	Zdebug("RETURNED.");
//	res_man.Res_close(1);		// DON'T CLOSE VARIABLES RESOURCE - KEEP IT OPEN AT VERY START OF MEMORY SO IT CAN'T MOVE!

	file=res_man.Res_open(8);	// DON'T CLOSE PLAYER OBJECT RESOURCE - KEEP IT OPEN IN MEMORY SO IT CAN'T MOVE!

	//----------------------------------------
	Zdebug("CALLING: InitialiseFontResourceFlags");
	InitialiseFontResourceFlags();	// Set up font resource variables for this language version (James31july97)
									// Also set the windows application name to the proper game name
	Zdebug("RETURNED.");
	//----------------------------------------

	Zdebug("CALLING: Init_console");
	Init_console();				// set up the console system
	Zdebug("RETURNED.");

	#ifdef _SWORD2_DEBUG
		Zdebug("CALLING: Init_start_menu");
		Init_start_menu();			// read in all the startup information
		Zdebug("RETURNED from Init_start_menu");
	#endif	// _SWORD2_DEBUG



	Zdebug("CALLING: Init_text_bloc_system");
	Init_text_bloc_system();	// no blocs live
	Zdebug("RETURNED.");

	Zdebug("CALLING: Init_sync_system");
	Init_sync_system();
	Zdebug("RETURNED.");

	Zdebug("CALLING: Init_event_system");
	Init_event_system();
	Zdebug("RETURNED.");
	
	Zdebug("CALLING: Init_fx_queue");
	Init_fx_queue();			// initialise the sound fx queue
	Zdebug("RETURNED.");

	// all demos (not just web)
	if (_gameId == GID_SWORD2_DEMO)
		DEMO=1;		// set script variable

	return(0);
}
//------------------------------------------------------------------------------------
void	Close_game()	//Tony11Oct96
{
	Zdebug("Close_game() STARTING:");
	EraseBackBuffer();

	Kill_music();			// Stop music instantly! (James22aug97)
	Close_memory_manager();	// free the memory again
	res_man.Close_ResMan();

	Zdebug("Close_game() DONE.");
}
//------------------------------------------------------------------------------------
int32 GameCycle(void)
{
//do one game cycle


	{
		if	(LLogic.Return_run_list())	//got a screen to run?
		{
			do	//run the logic session UNTIL a full loop has been performed
			{
				Reset_render_lists();	// reset the graphic 'buildit' list before a new logic list (see FN_register_frame)
				Reset_mouse_list();		// reset the mouse hot-spot list (see FN_register_mouse & FN_register_frame)
			}
			while(LLogic.Process_session());	//keep going as long as new lists keep getting put in - i.e. screen changes
		}
		else	//start the console and print the start options perhaps?
		{
			StartConsole();
			Print_to_console("AWAITING START COMMAND: (Enter 's 1' then 'q' to start from beginning)");
		}
	}

	if (this_screen.scroll_flag)	// if this screen is wide
		Set_scrolling();			// recompute the scroll offsets every game-cycle

	Mouse_engine();	//check the mouse

	Process_fx_queue();

	res_man.Res_next_cycle();	// update age and calculate previous cycle memory usage

	if (quitGame)
		return(1);
	else
		return(0);
}
//------------------------------------------------------------------------------------
// int main(int argc, char *argv[])
// int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
void Sword2State::go()
{
	OSystem::Property prop;
	uint32 rv;
	uint8  breakOut = 0;
	char	c;

//	Zdebug("[%s]", lpCmdLine);


	#ifndef _SWORD2_DEBUG
	DisableQuitKey();	// so cannot use Ctrl-Q from the release versions (full game or demo)
	#endif


	if	(RunningFromCd())	//stop this game being run from CD
		exit(-1);



	// Call the application "Revolution" until the resource manager is ready to dig the name out of a text file
	// See InitialiseGame() which calls InitialiseFontResourceFlags() in maketext.cpp
	// Have to do it like this since we cannot really fire up the resource manager until a window
	// has been created as any errors are displayed via a window, thus time becomes a loop.

	Zdebug("CALLING: InitialiseWindow");
	// rv = InitialiseWindow(hInstance, hPrevInstance, lpCmdLine, nCmdShow, "Revolution");
	rv = RD_OK;
 	Zdebug("RETURNED with rv = %.8x", rv);
	if (rv != RD_OK)
	{
		// ReportDriverError(rv);
		return;
	}

	// Override global scaler with any game-specific define
	if (g_config->get("gfx_mode")) {
		prop.gfx_mode = _detector->parseGraphicsMode(g_config->get("gfx_mode"));
		_system->property(OSystem::PROP_SET_GFX_MODE, &prop);
	}

	Zdebug("CALLING: InitialiseDisplay");
	_system->init_size(640, 480);
	rv = InitialiseDisplay(640, 480, 8, RD_FULLSCREEN);

	// Override global fullscreen setting with any game-specific define
	if (g_config->getBool("fullscreen", false)) {
		if (!_system->property(OSystem::PROP_GET_FULLSCREEN, 0))
			_system->property(OSystem::PROP_TOGGLE_FULLSCREEN, 0);
	}
		
	Zdebug("RETURNED with rv = %.8x", rv);
	if (rv != RD_OK)
	{
		// ReportDriverError(rv);
		CloseAppWindow();
		return;
	}

	Zdebug("CALLING: ReadOptionSettings");
	ReadOptionSettings();	//restore the menu settings
  	Zdebug("RETURNED.");

	Zdebug("CALLING: InitialiseSound");
	rv = _sound->InitialiseSound(22050, 2, 16);
 	Zdebug("RETURNED with rv = %.8x", rv);
	// don't care if this fails, because it should still work without sound cards
	// but it should set a global system flag so that we can avoid loading sound fx & streaming music
	// because they won't be heard anyway
/*
	if (rv != RD_OK)
	{
		ReportDriverError(rv);
		CloseAppWindow();
		return(0);
	}
*/

	g_engine->_timer->installProcedure(sword2_sound_handler, 1000000);
	Zdebug("CALLING: InitialiseGame");
	if (InitialiseGame())
	{
	  	Zdebug("RETURNED from InitialiseGame - closing game");
		CloseAppWindow();
		return;
	}
   	Zdebug("RETURNED from InitialiseGame - ok");



//check for restore game on startup - at the mo any passed argument is good enough to trigger this
	// if	(lpCmdLine[0])	//non zero
	if (_saveSlot != -1)
	{
		// RestoreGame(_saveSlot); use traditional way for now...
		
		Set_mouse(NORMAL_MOUSE_ID);

		if	(!Restore_control())	// restore a game
			Start_game();
	}
	//-------------------------------------------------------------
	// release versions only (full-game and demo)
//	#if NDEBUG	// comment this out for debug versions to start game automatically!
	else
		Start_game();
//	#endif		// comment this out for debug versions to start game automatically!
	//-------------------------------------------------------------


	Zdebug("CALLING: InitialiseRenderCycle");
	InitialiseRenderCycle();
  	Zdebug("RETURNED.");

	while (TRUE)
	{
		if (ServiceWindows() == RDERR_APPCLOSED)
		{
			break;		// break out of main game loop
		}

		
		// check for events
		parseEvents();
#ifdef _SWORD2_DEBUG
		if (grabbingSequences && (!console_status))
			GrabScreenShot();
#endif

		while (!gotTheFocus)
		{
			if (ServiceWindows() == RDERR_APPCLOSED)
			{
				breakOut = 1;
				break;	// break out of this while-loop
			}
		}

		if (breakOut)	// if we are closing down the game
			break;		// break out of main game loop

//-----

#ifdef _SWORD2_DEBUG
		if	(console_status)
		{
			if	(One_console())
			{
				EndConsole();
				UnpauseAllSound();	// see sound.cpp
			}
		}
#endif

		if	(!console_status)	//not in console mode - if the console is quit we want to get a logic cycle in before
		{							//the screen is build. Mostly because of first scroll cycle stuff
#ifdef _SWORD2_DEBUG
			if (stepOneCycle)	// if we've just stepped forward one cycle while the game was paused
			{
				PauseGame();
				stepOneCycle=0;
			}
#endif
			if (KeyWaiting())
			{
				ReadKey(&c);
#ifdef _SWORD2_DEBUG
				if (c==27)					// ESC whether paused or not
				{
					PauseAllSound();					// see sound.cpp
					StartConsole();						// start the console
				}
				else
#endif
					if (gamePaused)					// if currently paused
					{
						if (toupper(c)=='P')				// 'P' while paused = unpause!
						{
							UnpauseGame();
						}
#ifdef _SWORD2_DEBUG
						else if (toupper(c)==' ')			// SPACE bar while paused = step one frame!
						{
							stepOneCycle=1;					// step through one game cycle
							UnpauseGame();
						}
#endif	// _SWORD2_DEBUG
					}
				else if (toupper(c)=='P')	// 'P' while not paused = pause!
				{
					PauseGame();
				}
#ifdef _SWORD2_DEBUG	// frame-skipping only allowed on debug version
				else if (toupper(c)=='S')	// 'S' toggles speed up (by skipping display rendering)
				{
					renderSkip = 1 - renderSkip;
				}
#endif	// _SWORD2_DEBUG
			}

			if (gamePaused==0)	// skip GameCycle if we're paused
			{
#ifdef _SWORD2_DEBUG
				gameCycle += 1;
#endif

				if (GameCycle())
					break;		// break out of main game loop
			}

#ifdef _SWORD2_DEBUG
			Build_debug_text();			// creates the debug text blocks
#endif	// _SWORD2_DEBUG
		}
//-----

		// James (24mar97)

#ifdef _SWORD2_DEBUG
		if ((console_status)||(renderSkip==0)||(gameCycle%4 == 0))	// if not in console & 'renderSkip' is set, only render display once every 4 game-cycles
			Build_display();	// create and flip the screen
#else
		Build_display();	// create and flip the screen
#endif	// _SWORD2_DEBUG

	}

	Close_game();	//close engine systems down
	CloseAppWindow();

	return;	//quit the game
}
//------------------------------------------------------------------------------------
int RunningFromCd()
{
/*
   char  sCDName[MAX_PATH];
   char  sRoot[MAX_PATH];
   DWORD dwMaxCompLength, dwFSFlags;
   GetModuleFileName(NULL , sRoot, _MAX_PATH);
   *(strchr(sRoot,'\\')+1) = '\0';

   if (!GetVolumeInformation(sRoot, sCDName,_MAX_PATH, NULL, &dwMaxCompLength, &dwFSFlags, NULL, 0))
      return -1;
   if (!scumm_strnicmp(sCDName,CD1_LABEL,6))
      return 1;
   if (!scumm_strnicmp(sCDName,CD2_LABEL,6))
      return 2;
*/
   return 0;
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void	Sword2State::Start_game(void)	//Tony29May97
{
//boot the game straight into a start script
	int screen_manager_id;

	Zdebug("Start_game() STARTING:");

	// all demos not just web
	if (_gameId == GID_SWORD2_DEMO)
		screen_manager_id = 19;		// DOCKS SECTION START
	else
		screen_manager_id = 949;	// INTRO & PARIS START

	// FIXME this could be validated against startup.inf for valid numbers
	// to stop people shooting themselves in the foot
	if (_bootParam != 0)
		screen_manager_id = _bootParam;
	
	char	*raw_script;
	char	*raw_data_ad;
	uint32	null_pc=1;	// the required start-scripts are both script #1 in the respective ScreenManager objects

	raw_data_ad	= (char*) (res_man.Res_open(8));					// open george object, ready for start script to reference
	raw_script	= (char*) (res_man.Res_open(screen_manager_id));	// open the ScreenManager object

	RunScript ( raw_script, raw_data_ad, &null_pc );			// run the start script now (because no console)

	res_man.Res_close(screen_manager_id);						// close the ScreenManager object
	res_man.Res_close(8);										// close george

	Zdebug("Start_game() DONE.");
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void PauseGame(void)		// James17jun97
{
//	uint8	*text;

//	text = FetchTextLine( res_man.Res_open(3258), 449 );	// open text file & get the line "PAUSED"
//	pause_text_bloc_no = Build_new_block(text+2, 320, 210, 640, 184, RDSPR_TRANS+RDSPR_DISPLAYALIGN, SPEECH_FONT_ID, POSITION_AT_CENTRE_OF_BASE);
//	res_man.Res_close(3258);	// now ok to close the text file

	//---------------------------
	// don't allow Pause while screen fading or while black (James 03sep97)
	if(GetFadeStatus()!=RDFADE_NONE)
		return;
	//---------------------------
	
  	PauseAllSound();

//make a normal mouse
	ClearPointerText();
//	mouse_mode=MOUSE_normal;
	SetLuggageAnim(NULL, 0);	//this is the only place allowed to do it this way
	Set_mouse(0);			// blank cursor
	mouse_touching=1;	//forces engine to choose a cursor

	if (current_graphics_level==3)	// if level at max
	{
		UpdateGraphicsLevel(2);	// turn down because palette-matching won't work when dimmed
		graphics_level_fudged=1;
	}

	if (stepOneCycle==0)	// don't dim it if we're single-stepping through frames
	{
  		DimPalette();				// dim the palette during the pause (James26jun97)
	}

	gamePaused=1;
}
//------------------------------------------------------------------------------------
void UnpauseGame(void)		// James17jun97
{
//	Kill_text_bloc(pause_text_bloc_no);	// removed "PAUSED" from screen

	if	((OBJECT_HELD)&&(real_luggage_item))
		Set_luggage(real_luggage_item);

	UnpauseAllSound();

	SetFullPalette(0xffffffff);	// put back game screen palette; see Build_display.cpp (James26jun97)

 	if (graphics_level_fudged)	// if level at max
	{
		UpdateGraphicsLevel(3);	// turn up again
		graphics_level_fudged=0;
	}

	gamePaused=0;
	unpause_zone=2;

	if	((!mouse_status)||(choosing))	//if mouse is about or we're in a chooser menu
		Set_mouse(NORMAL_MOUSE_ID);
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

