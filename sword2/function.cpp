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
#include "build_display.h"
#include "credits.h"
#include "debug.h"
#include "defs.h"
#include "function.h"
#include "interpreter.h"
#include "layers.h"	// for 'this_screen' structure
#include "logic.h"
#include "protocol.h"
#include "resman.h"
#include "sword2.h"	// for CloseGame()

Object_graphic engine_graph;	// global for engine
Object_mega engine_mega;	// global for engine

int32 FN_test_function(int32 *params) {
	// param	0 address of a flag

	Zdebug(" TEST %d %d", params[0], RESULT);
	return IR_CONT;
}

int32 FN_test_flags(int32 *params) {
	// param	0 value of flag

	// Zdebug("\nFN_test_flags %d, %d\n", params[0], params[1]);
	return IR_CONT;
}

int32 FN_gosub(int32 *params) {		// Tony23Sept96
	// hurray, script subroutines
	// param	0 id of script

	LLogic.Logic_up(params[0]);

	// logic goes up - pc is saved for current level
	return IR_GOSUB;
}

int32 FN_new_script(int32 *params) {	// Tony13Nov96
	// change current script - must be followed by a TERMINATE script
	// directive
	// param	0 id of script

	Zdebug("FN_new_script %d", params[0]);

	// must clear this
	PLAYER_ACTION = 0;

	LLogic.Logic_replace(params[0]);

	//drop out no pc save - and around again
	return IR_TERMINATE;
}

int32 FN_interact(int32 *params) {	// Tony13Nov96
	// run targets action on a subroutine
	// called by player on his base level 0 idle, for example
	// param	0 id of target from which we derive action script
	//		  reference

	Zdebug("FN_interact %d", params[0]);

	// must clear this
	PLAYER_ACTION = 0;

	// 3rd script of clicked on id
	LLogic.Logic_up((params[0] * 65536) + 2);

	// out, up and around again - pc is saved for current level to be
	// returned to
	return IR_GOSUB;
}

int32 FN_preload(int32 *params)	{	// (1Nov96 JEL)
	// Open & close a resource.
	// Forces a resource into memory before it's "officially" opened for
	// use. eg. if an anim needs to run on smoothly from another,
	// "preloading" gets it into memory in advance to avoid the cacheing
	// delay that normally occurs before the first frame.

	res_man.Res_open(params[0]);	// open resource
	res_man.Res_close(params[0]);	// close resource
	return IR_CONT;			// continue script
}

int32 FN_prefetch(int32 *params) {
	// Go fetch resource in the background.

	return IR_CONT;
}

int32 FN_fetch_wait(int32 *params) {
	// Fetches a resource in the background but prevents the script from
	// continuing until the resource is in memory.

	return IR_CONT;
}

int32 FN_release(int32 *params) {
	// Releases a resource from memory. Used for freeing memory for
	// sprites that have just been used and will not be used again.
	// Sometimes it is better to kick out a sprite straight away so that
	// the memory can be used for more frequent animations.

	return IR_CONT;
}

int32 FN_random(int32 *params) {	// (1nov96 JEL)
	// Generates a random number between 'min' & 'max' inclusive, and
	// sticks it in the script flag 'result'

	uint32 min = params[0];
	uint32 max = params[1];

	// return_value = random integer between min and max, inclusive
	RESULT = (rand() % (max-min + 1)) + min;
	
	// continue script
	return IR_CONT;
}

int32 FN_pause(int32 *params) {		// (19nov96 JEL)
	// params:	0 pointer to object's logic structure
	//		1 number of game-cycles to pause

	// NB. Pause-value of 0 causes script to continue, 1 causes a 1-cycle
	// quit, 2 gives 2 cycles, etc.

	Object_logic *ob_logic = (Object_logic *) params[0];

	if (ob_logic->looping == 0) {
		// start the pause
		ob_logic->looping = 1;

		// no. of game cycles
		ob_logic->pause = params[1];
	}

	if (ob_logic->pause) {
		// decrement the pause count
		ob_logic->pause--;

		// drop out of script, but call this again next cycle
		return IR_REPEAT;
	} else {
		ob_logic->looping = 0;

		// continue script
		return IR_CONT;
	}
}

int32 FN_random_pause(int32 *params) {	// (26nov96 JEL)
	// params:	0 pointer to object's logic structure
	//		1 minimum number of game-cycles to pause
	//		2 maximum number of game-cycles to pause

	Object_logic *ob_logic = (Object_logic *) params[0];
	int32 pars[2];

	if (ob_logic->looping == 0) {
		pars[0] = params[1];	// min
		pars[1] = params[2];	// max

		FN_random(pars);

		// random value between 'min' & 'max' inclusive
		pars[1] = RESULT;
	}

	pars[0] = params[0];		// &logic
	return FN_pause(pars);
}

int32 FN_pass_graph(int32 *params) {	// Tony28Nov96
	// makes an engine local copy of passed graphic_structure and
	// mega_structure - run script 4 of an object to request this
	// used by FN_turn_to(id) etc
	//
	// remember, we cannot simply read a compact any longer but instead
	// must request it from the object itself

	//params	0 pointer to a graphic structure (might not need this?)

	memcpy(&engine_graph, (uint8 *) params[0], sizeof(Object_graphic));

	// makes no odds
	return IR_CONT;
}

int32 FN_pass_mega(int32 *params) {	// Tony28Nov96
	// makes an engine local copy of passed graphic_structure and
	// mega_structure - run script 4 of an object to request this
	// used by FN_turn_to(id) etc
	//
	// remember, we cannot simply read a compact any longer but instead
	// must request it from the object itself

	// params 	0 pointer to a mega structure

	memcpy(&engine_mega, (uint8*) params[0], sizeof(Object_mega));

	//makes no odds
	return IR_CONT;
}

int32 FN_set_value(int32 *params) {	// (02jan97 JEL)
	// temp. function!
	// used for setting far-referenced megaset resource field in mega
	// object, from start script

	// params:	0 pointer to object's mega structure
	//		1 value to set it to

	Object_mega *ob_mega = (Object_mega *)params[0];

	ob_mega->megaset_res = params[1];

	// continue script
	return IR_CONT;
}

#define BLACK	0
#define WHITE	1
#define RED	2
#define GREEN	3
#define BLUE	4

uint8 black[4]	= {  0,    0,   0,   0 };
uint8 white[4]	= { 255, 255, 255,   0 };
uint8 red[4]	= { 255,   0,   0,   0 };
uint8 green[4]	= {   0, 255,   0,   0 };
uint8 blue[4]	= {   0,   0, 255,   0 };

int32 FN_flash(int32 *params) {		// (James14feb97)
	// flash colour 0 (ie. border) - useful during script development
	// eg. FN_flash(BLUE) where a text line is missed; RED when some code
	// missing, etc

	// params	0: colour to flash

#ifdef _SWORD2_DEBUG
	// what colour?
	switch (params[0]) {
	case WHITE:
		BS2_SetPalette(0, 1, white, RDPAL_INSTANT);
		break;
	case RED:
		BS2_SetPalette(0, 1, red, RDPAL_INSTANT);
		break;
	case GREEN:
		BS2_SetPalette(0, 1, green, RDPAL_INSTANT);
		break;
	case BLUE:
		BS2_SetPalette(0, 1, blue, RDPAL_INSTANT);
		break;
	}

	// There used to be a busy-wait loop here, so I don't know how long
	// the delay was meant to be. Probably doesn't matter much.

	ServiceWindows();
	g_system->delay_msecs(250);
	BS2_SetPalette(0, 1, black, RDPAL_INSTANT);
#endif

	return IR_CONT;
}


int32 FN_colour(int32 *params) {	// (James14feb97)
	// set border colour - useful during script development
	// eg. set to colour during a timer situation, then black when timed
	// out

	// params	0: colour (see defines above)

#ifdef _SWORD2_DEBUG
	// what colour?
	switch (params[0]) {
	case BLACK:
		BS2_SetPalette(0, 1, black, RDPAL_INSTANT);
		break;
	case WHITE:
		BS2_SetPalette(0, 1, white, RDPAL_INSTANT);
		break;
	case RED:
		BS2_SetPalette(0, 1, red, RDPAL_INSTANT);
		break;
	case GREEN:
		BS2_SetPalette(0, 1, green, RDPAL_INSTANT);
		break;
	case BLUE:
		BS2_SetPalette(0, 1, blue, RDPAL_INSTANT);
		break;
	}
#endif

	return IR_CONT;
}

int32 FN_display_msg(int32 *params) {	// (Chris 15/5/97)
	// Display a message to the user on the screen.
	//
	// params	0: Text number of message to be displayed.

	uint32 local_text = params[0] & 0xffff;
	uint32 text_res = params[0] / SIZE;

	// Display message for three seconds.

	// +2 to skip the encoded text number in the first 2 chars; 3 is
	// duration in seconds

	DisplayMsg(FetchTextLine(res_man.Res_open(text_res), local_text) + 2, 3);
	res_man.Res_close(text_res);
	RemoveMsg();

	return IR_CONT;
}

int32 FN_reset_globals(int32 *params) {	// Tony29May97
	// FN_reset_globals is used by the demo - so it can loop back &
	// restart itself

	int32 size;
	uint32 *globals;
	int j;

	size = res_man.Res_fetch_len(1);
	size -= sizeof(_standardHeader);

	Zdebug("\nglobals size %d", size / 4);

	globals = (uint32*) ((uint8 *) res_man.Res_open(1) + sizeof(_standardHeader));

	// blank each global variable
	for (j = 0; j < size / 4; j++)
		globals[j] = 0;

	res_man.Res_close(1);

	// all objects but george
	res_man.Kill_all_objects(0);

	// reopen global variables resource & send address to interpreter - it
	// won't be moving
	// SetGlobalInterpreterVariables((int32 *) (res_man.Res_open(1) + sizeof(_standardHeader)));
	// res_man.Res_close(1);

	// FOR THE DEMO - FORCE THE SCROLLING TO BE RESET! (James29may97)
	// - this is taken from FN_init_background

	// switch on scrolling (2 means first time on screen)
	this_screen.scroll_flag = 2;

	return IR_CONT;
}

int32 FN_play_credits(int32 *params) {
	// FN_play_credits - Plays the credits?
	// This function just quits the game if this is the playable demo, ie.
	// credits are NOT played in the demo any more!

	/*	uint32	rv;	// for Credits() return value

	if (!DEMO) {
		_drvDrawStatus ds;
		_drvSoundStatus ss;
		_drvKeyStatus ks;

		ClearAllFx();	// Must stop all fx
		CloseFx(-2);	// including leadins
		CloseFx(-1);	// including leadouts
		StopMusic();	// Stop any streaming music

#if 0
		// FIXME: I don't think this is needed
		
		// And wait for it to die
		for (int i = 0; i<16; i++) {
			g_sound->UpdateCompSampleStreaming();
		}
#endif

		GetDrawStatus(&ds);
		GetSoundStatus(&ss);
		GetKeyStatus(&ks);

		rv = Credits(&ds, &ss, res_man.GetCdPath(), GetRenderType()==3, &gotTheFocus, &ks);

		// (James14aug97) Because game crashing when trying to close
		// down after credits
		SetDrawStatus(&ds);
		SetSoundStatus(&ss);
	}
*/

	// FIXME: This probably isn't the correct way of shutting down ScummVM

	if (g_sword2->_gameId == GID_SWORD2_DEMO) {
		Close_game();		// close engine systems down
		CloseAppWindow();
		exit(0);		// quit the game
	}

	return IR_CONT;
}
