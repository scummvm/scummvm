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

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "stdafx.h"
#include "driver/driver96.h"
#include "build_display.h"
#include "credits.h"
#include "debug.h"
#include "defs.h"
#include "function.h"
#include "interpreter.h"
#include "layers.h"	// for 'this_screen' structure
#include "logic.h"
#include "memory.h"
#include "object.h"
#include "protocol.h"
#include "resman.h"
#include "sword2.h"	// for CloseGame()
//------------------------------------------------------------------------------------
typedef	struct
{
	uint32	a;
	uint32	b;
} test_struct;

//------------------------------------------------------------------------------------

Object_graphic	engine_graph;	// global for engine
Object_mega		engine_mega;	// global for engine

//------------------------------------------------------------------------------------
int32 FN_test_function(int32 *params)
{
//param	0 address of a flag
	Zdebug(" TEST %d %d", *params, RESULT);


	return(IR_CONT);
}
//------------------------------------------------------------------------------------
int32 FN_test_flags(int32 *params)
{
//param	0 value of flag

	test_struct	*tony;


	tony = (test_struct*) *params;	//address of structure



//	Zdebug("\nFN_test_flags %d, %d\n", tony->a, tony->b );

	return(IR_CONT);
}
//------------------------------------------------------------------------------------
int32 FN_gosub(int32 *params)	//Tony23Sept96
{
//hurray, script subroutines
//param	0 id of script


	LLogic.Logic_up(*params);

	return(4);	//logic goes up - pc is saved for current level
}
//------------------------------------------------------------------------------------
int32 FN_new_script(int32 *params)	//Tony13Nov96
{
//change current script - must be followed by a TERMINATE script directive
//param	0 id of script
	Zdebug("FN_new_script %d", *params);

	PLAYER_ACTION=0;	//must clear this

	LLogic.Logic_replace( *params );

	return(IR_TERMINATE);	//drop out no pc save - and around again
}
//------------------------------------------------------------------------------------
int32 FN_interact(int32 *params)	//Tony13Nov96
{
//run targets action on a subroutine
//called by player on his base level 0 idle, for example


//param	0 id of target from which we derive action script reference

	Zdebug("FN_interact %d", *params);
	PLAYER_ACTION=0;	//must clear this

	LLogic.Logic_up( (*params*65536)+2);	//3rd script of clicked on id

	return(IR_GOSUB);	//out, up and around again - pc is saved for current level to be returned to
}
//------------------------------------------------------------------------------------

// Open & close a resource.
// Forces a resource into memory before it's "officially" opened for use.
// eg. if an anim needs to run on smoothly from another, "preloading" gets it into memory in advance
//     to avoid the cacheing delay that normally occurs before the first frame.

int32 FN_preload(int32 *params)	// (1Nov96 JEL)
{
	res_man.Res_open(params[0]);	// open resource
	res_man.Res_close(params[0]);	// close resource

	return(IR_CONT);	// continue script
}


// Go fetch resource in the background.
int32 FN_prefetch(int32 *params)
{
	return(IR_CONT);
}


// Fetches a resource in the background but prevents the script from continuing until the resource is in memory.
int32 FN_fetch_wait(int32 *params)
{
	return (IR_CONT);
}


// Releases a resource from memory. Used for freeing memory for sprites that have just been used
// and will not be used again.
// Sometimes it is better to kick out a sprite straight away so that the memory can be used for
// more frequent animations.
int32 FN_release(int32 *params)
{
	return (IR_CONT);
}


//------------------------------------------------------------------------------------
// Generates a random number between 'min' & 'max' inclusive, and sticks it in the script flag 'result'

int32 FN_random(int32 *params)	// (1nov96 JEL)
{
	uint32 min = params[0];
	uint32 max = params[1];

	RESULT = (rand() % (max-min+1)) + min;	// return_value = random integer between min and max, inclusive
	
	return(IR_CONT);	// continue script
}
//------------------------------------------------------------------------------------
int32 FN_pause(int32 *params)	// (19nov96 JEL)
{
	// params:	0 pointer to object's logic structure
	//			1 number of game-cycles to pause

	//NB. Pause-value of 0 causes script to continue, 1 causes a 1-cycle quit, 2 gives 2 cycles, etc.

	Object_logic *ob_logic = (Object_logic *)params[0];

	if (ob_logic->looping==0)	// start the pause
	{
		ob_logic->looping = 1;
		ob_logic->pause   = params[1];	// no. of game cycles
	}

	if (ob_logic->pause)	// if non-zero
	{
		ob_logic->pause--;	// decrement the pause count
		return(IR_REPEAT);	// drop out of script, but call this again next cycle
	}
	else					// pause count is zerp
	{
		ob_logic->looping = 0;
		return(IR_CONT);	// continue script
	}
}
//------------------------------------------------------------------------------------
int32 FN_random_pause(int32 *params)	// (26nov96 JEL)
{
	// params:	0 pointer to object's logic structure
	//			1 minimum number of game-cycles to pause
	//			2 maximum number of game-cycles to pause

	Object_logic *ob_logic = (Object_logic *)params[0];
	int32 pars[2];


	if (ob_logic->looping==0)
	{
		pars[0] = params[1];	// min
		pars[1] = params[2];	// max

		FN_random(pars);

		pars[1] = RESULT;		// random value between 'min' & 'max' inclusive
	}

	pars[0] = params[0];		// &logic

	return FN_pause(pars);
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
int32 FN_pass_graph(int32 *params)	//Tony28Nov96
{
//makes an engine local copy of passed graphic_structure and mega_structure - run script 4 of an object to request this
//used by FN_turn_to(id) etc
//remember, we cannot simply read a compact any longer but instead must request it from the object itself

//params	0 pointer to a graphic structure	*might not need this?


	memcpy( &engine_graph, (uint8*)params[0], sizeof(Object_graphic));

	return(IR_CONT);	//makes no odds
}
//------------------------------------------------------------------------------------
int32 FN_pass_mega(int32 *params)	//Tony28Nov96
{
//makes an engine local copy of passed graphic_structure and mega_structure - run script 4 of an object to request this
//used by FN_turn_to(id) etc
//remember, we cannot simply read a compact any longer but instead must request it from the object itself

//params 0 pointer to a mega structure


	memcpy( &engine_mega, (uint8*)params[0], sizeof(Object_mega));

	return(IR_CONT);	//makes no odds
}
//------------------------------------------------------------------------------------
// temp. function!
// used for setting far-referenced megaset resource field in mega object, from start script

int32 FN_set_value(int32 *params)	// (02jan97 JEL)
{
	// params:	0 pointer to object's mega structure
	//			1 value to set it to

	Object_mega *ob_mega = (Object_mega *)params[0];


	ob_mega->megaset_res = params[1];

	return(IR_CONT);	// continue script
}
//------------------------------------------------------------------------------------
#define BLACK	0
#define WHITE	1
#define RED		2
#define GREEN	3
#define BLUE	4
//------------------------------------------------------------------------------------
uint8 black[4]	= {0,0,0,0};
uint8 white[4]	= {255,255,255,0};
uint8 red[4]	= {255,0,0,0};
uint8 green[4]	= {0,255,0,0};
uint8 blue[4]	= {0,0,255,0};
//------------------------------------------------------------------------------------
// flash colour 0 (ie. border) - useful during script development
// eg. FN_flash(BLUE) where a text line is missed; RED when some code missing, etc

int32 FN_flash(int32 *params)	// (James14feb97)
{
	// params	0: colour to flash

#ifdef _BS2_DEBUG

	uint32 count;

	switch (params[0])	// what colour?
	{
		case WHITE:
			SetPalette(0, 1, white, RDPAL_INSTANT);
			break;

		case RED:
			SetPalette(0, 1, red, RDPAL_INSTANT);
			break;

		case GREEN:
			SetPalette(0, 1, green, RDPAL_INSTANT);
			break;

		case BLUE:
			SetPalette(0, 1, blue, RDPAL_INSTANT);
			break;
	}

 	for	(count=0; count<0x80000; count++)
	{
		count++;
		count--;
	}

	SetPalette(0, 1, black, RDPAL_INSTANT);

#endif	// _BS2_DEBUG

	return(IR_CONT);
}
//------------------------------------------------------------------------------------
// set border colour - useful during script development
// eg. set to colour during a timer situation, then black when timed out

int32 FN_colour(int32 *params)	// (James14feb97)
{
	// params	0: colour (see defines above)

#ifdef _BS2_DEBUG

	switch (params[0])	// what colour?
	{
		case BLACK:
			SetPalette(0, 1, black,	RDPAL_INSTANT);
			break;

		case WHITE:
			SetPalette(0, 1, white,	RDPAL_INSTANT);
			break;

		case RED:
			SetPalette(0, 1, red,	RDPAL_INSTANT);
			break;

		case GREEN:
			SetPalette(0, 1, green,	RDPAL_INSTANT);
			break;

		case BLUE:
			SetPalette(0, 1, blue,	RDPAL_INSTANT);
			break;
	}

#endif	// _BS2_DEBUG

	return(IR_CONT);
}
//------------------------------------------------------------------------------------
// Display a message to the user on the screen.
//

int32 FN_display_msg(int32 *params)	// (Chris 15/5/97)
{
	// params	0: Text number of message to be displayed.
	uint32	local_text = params[0]&0xffff;
	uint32	text_res   = params[0]/SIZE;

	// Display message for three seconds.
	DisplayMsg(FetchTextLine( res_man.Res_open(text_res), local_text )+2, 3);	// +2 to skip the encoded text number in the first 2 chars; 3 is duration in seconds
	res_man.Res_close(text_res);
	RemoveMsg();

	return(IR_CONT);
}
//------------------------------------------------------------------------------------
// FN_reset_globals is used by the demo - so it can loop back & restart itself
int32	FN_reset_globals(int32	*params)	//Tony29May97
{
	int32	size;
	uint32	*globals;
	int	j;

	size = res_man.Res_fetch_len(1);

	size-=sizeof(_standardHeader);

	Zdebug("\nglobals size %d", size/4);

	globals = (uint32*) ((uint8 *) res_man.Res_open(1)+sizeof(_standardHeader));

	for	(j=0;j<size/4;j++)
		globals[j]=0;	//blank each global variable

	res_man.Res_close(1);

	res_man.Kill_all_objects(0);	//all objects but george

//	SetGlobalInterpreterVariables((int32*)(res_man.Res_open(1)+sizeof(_standardHeader)));	//reopen global variables resource & send address to interpreter - it won't be moving
//	res_man.Res_close(1);

	//---------------------------------------------------------------
	// FOR THE DEMO - FORCE THE SCROLLING TO BE RESET! (James29may97)
	// - this is taken from FN_init_background
	this_screen.scroll_flag = 2;	// switch on scrolling (2 means first time on screen)
	//---------------------------------------------------------------

	return(IR_CONT);
}
//------------------------------------------------------------------------------------
// FN_play_credits - Plays the credits?
// This function just quits the game if this is the playable demo, ie. credits are NOT played in the demo any more!

extern uint8 quitGame;			// From sword2.cpp

int32 FN_play_credits(int32 *params)
{

/*	uint32	rv;	// for Credits() return value

	if (!DEMO)	// this ju
	{
		_drvDrawStatus		ds;
		_drvSoundStatus		ss;
		_drvKeyStatus		ks;

		ClearAllFx();	// Must stop all fx
		CloseFx(-2);	// including leadins
		CloseFx(-1);	// including leadouts
		StopMusic();	// Stop any streaming music
		
		for (int i = 0; i<16; i++)
			g_bs2->_sound->UpdateCompSampleStreaming();	// And wait for it to die

		GetDrawStatus (&ds);
		GetSoundStatus(&ss);
		GetKeyStatus  (&ks);

		rv = Credits(&ds, &ss, res_man.GetCdPath(), GetRenderType()==3, &gotTheFocus, &ks);
		SetDrawStatus (&ds);	// (James14aug97) Because game crashing when trying to close down after credits
		SetSoundStatus(&ss);	// -"-
	}

	// returns non-zero if Ctrl-Q was pressed to quit the game during the credits

	if (rv || DEMO)		// if Ctrl-Q pressed during credits, or if this is the playable demo
	{
		Close_game();	//close engine systems down
		RestoreDisplay();
		CloseAppWindow();
		exit(0);		// quit the game
	}

*/
	return (IR_CONT);
}
//------------------------------------------------------------------------------------

