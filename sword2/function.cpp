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

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"

namespace Sword2 {

int32 Logic::fnTestFunction(int32 *params) {
	// params:	0 address of a flag
	return IR_CONT;
}

int32 Logic::fnTestFlags(int32 *params) {
	// params:	0 value of flag
	return IR_CONT;
}

int32 Logic::fnGosub(int32 *params) {
	// hurray, script subroutines

	// params:	0 id of script

	logicUp(params[0]);

	// logic goes up - pc is saved for current level
	return IR_GOSUB;
}

int32 Logic::fnNewScript(int32 *params) {
	// change current script - must be followed by a TERMINATE script
	// directive

	// params:	0 id of script

	// must clear this
	PLAYER_ACTION = 0;

	logicReplace(params[0]);

	// drop out no pc save - and around again
	return IR_TERMINATE;
}

int32 Logic::fnInteract(int32 *params) {
	// run targets action on a subroutine
	// called by player on his base level 0 idle, for example

	// params:	0 id of target from which we derive action script
	//		  reference

	// must clear this
	PLAYER_ACTION = 0;

	// 3rd script of clicked on id
	logicUp((params[0] < 16) + 2);

	// out, up and around again - pc is saved for current level to be
	// returned to
	return IR_GOSUB;
}

int32 Logic::fnPreLoad(int32 *params) {
	// Open & close a resource.

	// Forces a resource into memory before it's "officially" opened for
	// use. eg. if an anim needs to run on smoothly from another,
	// "preloading" gets it into memory in advance to avoid the cacheing
	// delay that normally occurs before the first frame.

	// params:	0 resource to preload

	_vm->_resman->openResource(params[0]);
	_vm->_resman->closeResource(params[0]);
	return IR_CONT;
}

int32 Logic::fnPreFetch(int32 *params) {
	// Go fetch resource in the background.

	// params:	0 resource to fetch [guess]

	return IR_CONT;
}

int32 Logic::fnFetchWait(int32 *params) {
	// Fetches a resource in the background but prevents the script from
	// continuing until the resource is in memory.

	// params:	0 resource to fetch [guess]

	return IR_CONT;
}

int32 Logic::fnRelease(int32 *params) {
	// Releases a resource from memory. Used for freeing memory for
	// sprites that have just been used and will not be used again.
	// Sometimes it is better to kick out a sprite straight away so that
	// the memory can be used for more frequent animations.

	// params:	0 resource to release [guess]

	return IR_CONT;
}

int32 Logic::fnRandom(int32 *params) {
	// Generates a random number between 'min' & 'max' inclusive, and
	// sticks it in the script flag 'result'

	// params:	0 min
	//		1 max

	RESULT = _vm->_rnd.getRandomNumberRng(params[0], params[1]);
	return IR_CONT;
}

int32 Logic::fnPause(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 number of game-cycles to pause

	// NB. Pause-value of 0 causes script to continue, 1 causes a 1-cycle
	// quit, 2 gives 2 cycles, etc.

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->intToPtr(params[0]);

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

int32 Logic::fnRandomPause(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 minimum number of game-cycles to pause
	//		2 maximum number of game-cycles to pause

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->intToPtr(params[0]);
	int32 pars[2];

	if (ob_logic->looping == 0) {
		pars[0] = params[1];
		pars[1] = params[2];

		fnRandom(pars);
		pars[1] = RESULT;
	}

	pars[0] = params[0];
	return fnPause(pars);
}

int32 Logic::fnPassGraph(int32 *params) {
	// makes an engine local copy of passed ObjectGraphic - run script 4
	// of an object to request this used by fnTurnTo(id) etc
	//
	// remember, we cannot simply read a compact any longer but instead
	// must request it from the object itself

	// params:	0 pointer to an ObjectGraphic structure

	warning("fnPassGraph() is a no-op now");

	// makes no odds
	return IR_CONT;
}

int32 Logic::fnPassMega(int32 *params) {
	// makes an engine local copy of passed graphic_structure and
	// mega_structure - run script 4 of an object to request this
	// used by fnTurnTo(id) etc
	//
	// remember, we cannot simply read a compact any longer but instead
	// must request it from the object itself

	// params: 	0 pointer to a mega structure

	memcpy(&_vm->_engineMega, _vm->_memory->intToPtr(params[0]), sizeof(ObjectMega));

	// makes no odds
	return IR_CONT;
}

int32 Logic::fnSetValue(int32 *params) {
	// temp. function!

	// used for setting far-referenced megaset resource field in mega
	// object, from start script

	// params:	0 pointer to object's mega structure
	//		1 value to set it to

	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->intToPtr(params[0]);

	ob_mega->megaset_res = params[1];

	// continue script
	return IR_CONT;
}

#ifdef _SWORD2_DEBUG
#define BLACK	0
#define WHITE	1
#define RED	2
#define GREEN	3
#define BLUE	4

static uint8 black[4]	= {  0,    0,   0,   0 };
static uint8 white[4]	= { 255, 255, 255,   0 };
static uint8 red[4]	= { 255,   0,   0,   0 };
static uint8 green[4]	= {   0, 255,   0,   0 };
static uint8 blue[4]	= {   0,   0, 255,   0 };
#endif

int32 Logic::fnFlash(int32 *params) {
	// flash colour 0 (ie. border) - useful during script development
	// eg. fnFlash(BLUE) where a text line is missed; RED when some code
	// missing, etc

	// params:	0 colour to flash

#ifdef _SWORD2_DEBUG
	// what colour?
	switch (params[0]) {
	case WHITE:
		_vm->_graphics->setPalette(0, 1, white, RDPAL_INSTANT);
		break;
	case RED:
		_vm->_graphics->setPalette(0, 1, red, RDPAL_INSTANT);
		break;
	case GREEN:
		_vm->_graphics->setPalette(0, 1, green, RDPAL_INSTANT);
		break;
	case BLUE:
		_vm->_graphics->setPalette(0, 1, blue, RDPAL_INSTANT);
		break;
	}

	// There used to be a busy-wait loop here, so I don't know how long
	// the delay was meant to be. Probably doesn't matter much.

	_vm->_graphics->updateDisplay();
	_vm->_system->delay_msecs(250);
	_vm->_graphics->setPalette(0, 1, black, RDPAL_INSTANT);
#endif

	return IR_CONT;
}


int32 Logic::fnColour(int32 *params) {
	// set border colour - useful during script development
	// eg. set to colour during a timer situation, then black when timed
	// out

	// params	0: colour (see defines above)

#ifdef _SWORD2_DEBUG
	// what colour?
	switch (params[0]) {
	case BLACK:
		_vm->_graphics->setPalette(0, 1, black, RDPAL_INSTANT);
		break;
	case WHITE:
		_vm->_graphics->setPalette(0, 1, white, RDPAL_INSTANT);
		break;
	case RED:
		_vm->_graphics->setPalette(0, 1, red, RDPAL_INSTANT);
		break;
	case GREEN:
		_vm->_graphics->setPalette(0, 1, green, RDPAL_INSTANT);
		break;
	case BLUE:
		_vm->_graphics->setPalette(0, 1, blue, RDPAL_INSTANT);
		break;
	}
#endif

	return IR_CONT;
}

int32 Logic::fnDisplayMsg(int32 *params) {
	// Display a message to the user on the screen.

	// params:	0 Text number of message to be displayed.

	uint32 local_text = params[0] & 0xffff;
	uint32 text_res = params[0] / SIZE;

	// Display message for three seconds.

	// +2 to skip the encoded text number in the first 2 chars; 3 is
	// duration in seconds

	_vm->displayMsg(_vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text) + 2, 3);
	_vm->_resman->closeResource(text_res);
	_vm->removeMsg();

	return IR_CONT;
}

int32 Logic::fnResetGlobals(int32 *params) {
	// fnResetGlobals is used by the demo - so it can loop back & restart
	// itself

	// params:	none

	int32 size;
	uint32 *globals;

	size = _vm->_resman->fetchLen(1);
	size -= sizeof(StandardHeader);

	debug(5, "globals size: %d", size);

	globals = (uint32 *) ((uint8 *) _vm->_resman->openResource(1) + sizeof(StandardHeader));

	// blank each global variable
	memset(globals, 0, size);

	_vm->_resman->closeResource(1);

	// all objects but george
	_vm->_resman->killAllObjects(false);

	// FOR THE DEMO - FORCE THE SCROLLING TO BE RESET!
	// - this is taken from fnInitBackground

	// switch on scrolling (2 means first time on screen)
	_vm->_thisScreen.scroll_flag = 2;

	return IR_CONT;
}

int32 Logic::fnPlayCredits(int32 *params) {
	// This function just quits the game if this is the playable demo, ie.
	// credits are NOT played in the demo any more!

	// params:	none

	if (!DEMO) {
		uint8 oldPal[1024];
		uint8 tmpPal[1024];
		int32 music_length;
		int32 pars[2];

		_vm->_sound->saveMusicState();

		_vm->_sound->muteFx(true);
		_vm->_sound->muteSpeech(true);
		_vm->_sound->stopMusic();

		memcpy(oldPal, _vm->_graphics->_palCopy, 1024);
		memset(tmpPal, 0, 1024);

		_vm->_graphics->waitForFade();
		_vm->_graphics->fadeDown();
		_vm->_graphics->waitForFade();

		tmpPal[4] = 255;
		tmpPal[5] = 255;
		tmpPal[6] = 255;
		_vm->_graphics->setPalette(0, 256, tmpPal, RDPAL_INSTANT);

		// Play the credits music. Is it enough with just one
		// repetition of it?

		pars[0] = 309;
		pars[1] = FX_SPOT;
		fnPlayMusic(pars);

		music_length = 1000 * _vm->_sound->musicTimeRemaining();

		debug(0, "Credits music length: ~%d ms", music_length);

		_vm->_graphics->closeMenuImmediately();

		while (_vm->_sound->musicTimeRemaining()) {
			_vm->_graphics->clearScene();
			_vm->_graphics->setNeedFullRedraw();

			// FIXME: Draw the credits text. The actual text
			// messages are stored in credits.clu, and I'm guessing
			// that credits.bmp or font.clu may be the font.

			_vm->_graphics->updateDisplay();

			KeyboardEvent ke;

			if (_vm->_input->readKey(&ke) == RD_OK && ke.keycode == 27)
				break;

			_vm->_system->delay_msecs(30);
		}

		fnStopMusic(NULL);
		_vm->_sound->restoreMusicState();

		_vm->_graphics->setPalette(0, 256, oldPal, RDPAL_FADE);
		_vm->_graphics->fadeUp();
		_vm->_graphics->updateDisplay();
		_vm->buildDisplay();
		_vm->_graphics->waitForFade();

		_vm->_sound->muteFx(false);
		_vm->_sound->muteSpeech(false);
	}

	if (_vm->_features & GF_DEMO) {
		_vm->closeGame();
	}

	return IR_CONT;
}

} // End of namespace Sword2
