/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
#include "common/file.h"
#include "common/system.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/build_display.h"
#include "sword2/console.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/sound.h"
#include "sword2/driver/animation.h"

namespace Sword2 {

int32 Logic::fnTestFunction(int32 *params) {
	// params:	0 address of a flag
	return IR_CONT;
}

int32 Logic::fnTestFlags(int32 *params) {
	// params:	0 value of flag
	return IR_CONT;
}

int32 Logic::fnRegisterStartPoint(int32 *params) {
	// params:	0 id of startup script to call - key
	// 		1 pointer to ascii message

	int32 key = params[0];
	char *name = (char *) _vm->_memory->decodePtr(params[1]);

	_vm->registerStartPoint(key, name);
	return IR_CONT;
}

int32 Logic::fnInitBackground(int32 *params) {
	// this screen defines the size of the back buffer

	// params:	0 res id of normal background layer - cannot be 0
	//		1 1 yes 0 no for a new palette

	_vm->_screen->initBackground(params[0], params[1]);
	return IR_CONT;
}

/**
 * This function is used by start scripts.
 */

int32 Logic::fnSetSession(int32 *params) {
	// params:	0 id of new run list

	expressChangeSession(params[0]);
	return IR_CONT;
}

int32 Logic::fnBackSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	setSpriteStatus(params[0], BACK_SPRITE);
	return IR_CONT;
}

int32 Logic::fnSortSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	setSpriteStatus(params[0], SORT_SPRITE);
	return IR_CONT;
}

int32 Logic::fnForeSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	setSpriteStatus(params[0], FORE_SPRITE);
	return IR_CONT;
}

int32 Logic::fnRegisterMouse(int32 *params) {
	// this call would be made from an objects service script 0
	// the object would be one with no graphic but with a mouse - i.e. a
	// floor or one whose mouse area is manually defined rather than
	// intended to fit sprite shape

	// params:	0 pointer to ObjectMouse or 0 for no write to mouse
	//		  list

	ObjectMouse *ob_mouse = (ObjectMouse *) _vm->_memory->decodePtr(params[0]);

	_vm->_mouse->registerMouse(ob_mouse, NULL);
	return IR_CONT;
}

int32 Logic::fnAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 resource id of animation file

	// 0 means normal forward anim
	return animate(params, false);
}

int32 Logic::fnRandom(int32 *params) {
	// params:	0 min
	//		1 max

	_scriptVars[RESULT] = _vm->_rnd.getRandomNumberRng(params[0], params[1]);
	return IR_CONT;
}

int32 Logic::fnPreLoad(int32 *params) {
	// Forces a resource into memory before it's "officially" opened for
	// use. eg. if an anim needs to run on smoothly from another,
	// "preloading" gets it into memory in advance to avoid the cacheing
	// delay that normally occurs before the first frame.

	// params:	0 resource to preload

	_vm->_resman->openResource(params[0]);
	_vm->_resman->closeResource(params[0]);
	return IR_CONT;
}

int32 Logic::fnAddSubject(int32 *params) {
	// params:	0 id
	//		1 daves reference number

	if (_scriptVars[IN_SUBJECT] == 0) {
		// This is the start of the new subject list. Set the default
		// repsonse id to zero in case we're never passed one.
		_defaultResponseId = 0;
	}

	if (params[0] == -1) {
		// Id -1 is used for setting the default response, i.e. the
		// response when someone uses an object on a person and he
		// doesn't know anything about it. See fnChoose() below.

		_defaultResponseId = params[1];
	} else {
		debug(5, "fnAddSubject res %d, uid %d", params[0], params[1]);
		_subjectList[_scriptVars[IN_SUBJECT]].res = params[0];
		_subjectList[_scriptVars[IN_SUBJECT]].ref = params[1];
		_scriptVars[IN_SUBJECT]++;
	}

	return IR_CONT;
}

int32 Logic::fnInteract(int32 *params) {
	// Run targets action on a subroutine. Called by player on his base
	// level 0 idle, for example.

	// params:	0 id of target from which we derive action script
	//		  reference

	_scriptVars[PLAYER_ACTION] = 0;		// must clear this
	logicUp((params[0] << 16) | 2);		// 3rd script of clicked on id

	// Out, up and around again - pc is saved for current level to be
	// returned to.
	return IR_GOSUB;
}

int32 Logic::fnChoose(int32 *params) {
	// params:	none

	// This opcode is used to open the conversation menu. The human is
	// switched off so there will be no normal mouse engine.

	// The player's choice is piggy-backed on the standard opcode return
	// values, to be used with the CP_JUMP_ON_RETURNED opcode. As far as I
	// can tell, this is the only function that uses that feature.

	uint i;

	_scriptVars[AUTO_SELECTED] = 0;

	if (_scriptVars[OBJECT_HELD]) {
		// The player used an object on a person. In this case it
		// triggered a conversation menu. Act as if the user tried to
		// talk to the person about that object. If the person doesn't
		// know anything about it, use the default response.

		uint32 response = _defaultResponseId;

		for (i = 0; i < _scriptVars[IN_SUBJECT]; i++) {
			if (_subjectList[i].res == _scriptVars[OBJECT_HELD]) {
				response = _subjectList[i].ref;
				break;
			}
		}

		// The user won't be holding the object any more, and the
		// conversation menu will be closed.

		_scriptVars[OBJECT_HELD] = 0;
		_scriptVars[IN_SUBJECT] = 0;
		return IR_CONT | (response << 3);
	}

	if (_scriptVars[CHOOSER_COUNT_FLAG] == 0 && _scriptVars[IN_SUBJECT] == 1 && _subjectList[0].res == EXIT_ICON) {
		// This is the first time the chooser is coming up in this
		// conversation, there is only one subject and that's the
		// EXIT icon.
		//
		// In other words, the player doesn't have anything to talk
		// about. Skip it.

		// The conversation menu will be closed. We set AUTO_SELECTED
		// because the speech script depends on it.

		_scriptVars[AUTO_SELECTED] = 1;
		_scriptVars[IN_SUBJECT] = 0;
		return IR_CONT | (_subjectList[0].ref << 3);
	}

	byte *icon;

	if (!_choosing) {
		// This is a new conversation menu.

		if (!_scriptVars[IN_SUBJECT])
			error("fnChoose with no subjects");

		for (i = 0; i < _scriptVars[IN_SUBJECT]; i++) {
			icon = _vm->_resman->openResource(_subjectList[i].res) + sizeof(StandardHeader) + RDMENU_ICONWIDE * RDMENU_ICONDEEP;
			_vm->_mouse->setMenuIcon(RDMENU_BOTTOM, i, icon);
			_vm->_resman->closeResource(_subjectList[i].res);
		}

		for (; i < 15; i++)
			_vm->_mouse->setMenuIcon(RDMENU_BOTTOM, (uint8) i, NULL);

		_vm->_mouse->showMenu(RDMENU_BOTTOM);
		_vm->_mouse->setMouse(NORMAL_MOUSE_ID);
		_choosing = true;
		return IR_REPEAT;
	}

	// The menu is there - we're just waiting for a click. We only care
	// about left clicks.

	MouseEvent *me = _vm->mouseEvent();
	int mouseX, mouseY;

	_vm->_mouse->getPos(mouseX, mouseY);

	if (!me || !(me->buttons & RD_LEFTBUTTONDOWN) || mouseY < 400)
		return IR_REPEAT;

	// Check for click on a menu.

	int hit = _vm->_mouse->menuClick(_scriptVars[IN_SUBJECT]);
	if (hit < 0)
		return IR_REPEAT;

	// Hilight the clicked icon by greying the others.

	for (i = 0; i < _scriptVars[IN_SUBJECT]; i++) {
		if ((int) i != hit) {
			icon = _vm->_resman->openResource(_subjectList[i].res) + sizeof(StandardHeader);
			_vm->_mouse->setMenuIcon(RDMENU_BOTTOM, i, icon);
			_vm->_resman->closeResource(_subjectList[i].res);
		}
	}

	// For non-speech scripts that manually call the chooser
	_scriptVars[RESULT] = _subjectList[hit].res;

	// The conversation menu will be closed

	_choosing = false;
	_scriptVars[IN_SUBJECT] = 0;
	_vm->_mouse->setMouse(0);

	return IR_CONT | (_subjectList[hit].ref << 3);
}

/**
 * Walk mega to (x,y,dir). Set RESULT to 0 if it succeeded. Otherwise, set
 * RESULT to 1.
 */

int32 Logic::fnWalk(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target x-coord
	//		5 target y-coord
	//		6 target direction (8 means end walk on ANY direction)

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);
	ObjectGraphic *ob_graph = (ObjectGraphic *) _vm->_memory->decodePtr(params[1]);
	ObjectMega *ob_mega  = (ObjectMega *) _vm->_memory->decodePtr(params[2]);

	int16 target_x = (int16) params[4];
	int16 target_y = (int16) params[5];
	uint8 target_dir = (uint8) params[6];

	ObjectWalkdata *ob_walkdata;

	// If this is the start of the walk, calculate the route.

	if (!ob_logic->looping) {
		// If we're already there, don't even bother allocating
		// memory and calling the router, just quit back & continue
		// the script! This avoids an embarassing mega stand frame
		// appearing for one cycle when we're already in position for
		// an anim eg. repeatedly clicking on same object to repeat
		// an anim - no mega frame will appear in between runs of the
		// anim.
		
		if (ob_mega->feet_x == target_x && ob_mega->feet_y == target_y && ob_mega->current_dir == target_dir) {
			_scriptVars[RESULT] = 0;
			return IR_CONT;
		}

		assert(params[6] >= 0 && params[6] <= 8);

		ob_walkdata = (ObjectWalkdata *) _vm->_memory->decodePtr(params[3]);

		ob_mega->walk_pc = 0;

		// Set up mem for _walkData in route_slots[] & set mega's
		// 'route_slot_id' accordingly

		_router->allocateRouteMem();

		int32 route = _router->routeFinder(ob_mega, ob_walkdata, target_x, target_y, target_dir);

		// 0 = can't make route to target
		// 1 = created route
		// 2 = zero route but may need to turn

		if (route == 1 || route == 2) {
			// so script fnWalk loop continues until end of
			// walk-anim

			ob_logic->looping = 1;

			// need to animate the route now, so don't set result
			// or return yet!

			// started walk
			ob_mega->currently_walking = 1;

			// (see fnGetPlayerSaveData() in save_rest.cpp
		} else {
			_router->freeRouteMem();
			_scriptVars[RESULT] = 1;
			return IR_CONT;
		}

		// Walk is about to start, so set the mega's graphic resource
		ob_graph->anim_resource = ob_mega->megaset_res;
	} else if (_scriptVars[EXIT_FADING] && _vm->_screen->getFadeStatus() == RDFADE_BLACK) {
		// Double clicked an exit so quit the walk when screen is black
		// ok, thats it - back to script and change screen

		ob_logic->looping = 0;
		_router->freeRouteMem();

		// Must clear in-case on the new screen there's a walk
		// instruction (which would get cut short)
		_scriptVars[EXIT_CLICK_ID] = 0;

		// finished walk
		ob_mega->currently_walking = 0;

		// see fnGetPlayerSaveData() in save_rest.cpp

		_scriptVars[RESULT] = 0;

		// continue the script so that RESULT can be checked!
		return IR_CONT;
	}

	// get pointer to walkanim & current frame position

	WalkData *walkAnim = _router->getRouteMem();
	int32 walk_pc = ob_mega->walk_pc;

	// If stopping the walk early, overwrite the next step with a
	// slow-out, then finish

	if (checkEventWaiting()) {
		if (walkAnim[walk_pc].step == 0 && walkAnim[walk_pc + 1].step == 1) {
			// At the beginning of a step
			ob_walkdata = (ObjectWalkdata *) _vm->_memory->decodePtr(params[3]);
			_router->earlySlowOut(ob_mega, ob_walkdata);
		}
	}

	// Get new frame of walk

	ob_graph->anim_pc = walkAnim[walk_pc].frame;
	ob_mega->current_dir = walkAnim[walk_pc].dir;
	ob_mega->feet_x = walkAnim[walk_pc].x;
	ob_mega->feet_y = walkAnim[walk_pc].y;

	// Check if NEXT frame is in fact the end-marker of the walk sequence
	// so we can return to script just as the final (stand) frame of the
	// walk is set - so that if followed by an anim, the anim's first
	// frame replaces the final stand-frame of the walk (see below)

	// '512' is end-marker
	if (walkAnim[walk_pc + 1].frame == 512) {
		ob_logic->looping = 0;
		_router->freeRouteMem();

		// finished walk
		ob_mega->currently_walking = 0;

		// (see fnGetPlayerSaveData() in save_rest.cpp

		// if George's walk has been interrupted to run a new action
		// script for instance or Nico's walk has been interrupted by
		// player clicking on her to talk

		// There used to be code here for checking if two megas were
		// colliding, but that code had been commented out, and it
		// was only run if a function that always returned zero
		// returned non-zero.

		if (checkEventWaiting()) {
			startEvent();
			_scriptVars[RESULT] = 1;
			return IR_TERMINATE;
		} else {
			_scriptVars[RESULT] = 0;

			// CONTINUE the script so that RESULT can be checked!
			// Also, if an anim command follows the fnWalk command,
			// the 1st frame of the anim (which is always a stand
			// frame itself) can replace the final stand frame of
			// the walk, to hide the slight difference between the
			// shrinking on the mega frames and the pre-shrunk anim
			// start-frame.

			return IR_CONT;
		}
	}

	// Increment the walkanim frame number and come back next cycle

	ob_mega->walk_pc++;
	return IR_REPEAT;
}

/**
 * Walk mega to start position of anim
 */

int32 Logic::fnWalkToAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 anim resource id

	int32 pars[7];

	// Walkdata is needed for earlySlowOut if player clicks elsewhere
	// during the walk.

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	// If this is the start of the walk, read anim file to get start coords

	if (!ob_logic->looping) {
		byte *anim_file = _vm->_resman->openResource(params[4]);
		AnimHeader *anim_head = _vm->fetchAnimHeader( anim_file );

		pars[4] = anim_head->feetStartX;
		pars[5] = anim_head->feetStartY;
		pars[6] = anim_head->feetStartDir;

		_vm->_resman->closeResource(params[4]);

		// If start coords not yet set in anim header, use the standby
		// coords (which should be set beforehand in the script).

		if (pars[4] == 0 && pars[5] == 0) {
			byte buf[NAME_LEN];

			pars[4] = _standbyX;
			pars[5] = _standbyY;
			pars[6] = _standbyDir;

			debug(3, "WARNING: fnWalkToAnim(%s) used standby coords", _vm->fetchObjectName(params[4], buf));
		}

		assert(pars[6] >= 0 && pars[6] <= 7);
	}

	return fnWalk(pars);
}

/**
 * Turn mega to the specified direction. Just needs to call fnWalk() with
 * current feet coords, so router can produce anim of turn frames.
 */

int32 Logic::fnTurn(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target direction

	int32 pars[7];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	// If this is the start of the turn, get the mega's current feet
	// coords + the required direction

	if (!ob_logic->looping) {
		assert(params[4] >= 0 && params[4] <= 7);

		ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[2]);

		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = params[4];
	}

	return fnWalk(pars);
}

/**
 * Stand mega at (x,y,dir)
 * Sets up the graphic object, but also needs to set the new 'current_dir' in
 * the mega object, so the router knows in future
 */

int32 Logic::fnStandAt(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 target x-coord
	//		3 target y-coord
	//		4 target direction

	assert(params[4] >= 0 && params[4] <= 7);

	ObjectGraphic *ob_graph = (ObjectGraphic *) _vm->_memory->decodePtr(params[0]);
	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[1]);

	// set up the stand frame & set the mega's new direction

	ob_mega->feet_x = params[2];
	ob_mega->feet_y = params[3];
	ob_mega->current_dir = params[4];

	// mega-set animation file
	ob_graph->anim_resource	= ob_mega->megaset_res;

	// dir + first stand frame (always frame 96)
	ob_graph->anim_pc = params[4] + 96;

	return IR_CONT;
}

/**
 * Stand mega into the specified direction at current feet coords.
 * Just needs to call fnStandAt() with current feet coords.
 */

int32 Logic::fnStand(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 target direction

	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[1]);

	int32 pars[5];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = ob_mega->feet_x;
	pars[3] = ob_mega->feet_y;
	pars[4] = params[2];

	return fnStandAt(pars);
}

/**
 * stand mega at end position of anim
 */

int32 Logic::fnStandAfterAnim(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 anim resource id

	byte *anim_file = _vm->_resman->openResource(params[2]);
	AnimHeader *anim_head = _vm->fetchAnimHeader(anim_file);

	int32 pars[5];

	pars[0] = params[0];
	pars[1] = params[1];

	pars[2] = anim_head->feetEndX;
	pars[3] = anim_head->feetEndY;
	pars[4] = anim_head->feetEndDir;

	// If start coords not available either use the standby coords (which
	// should be set beforehand in the script)

	if (pars[2] == 0 && pars[3] == 0) {
		byte buf[NAME_LEN];

		pars[2] = _standbyX;
		pars[3] = _standbyY;
		pars[4] = _standbyDir;

		debug(3, "WARNING: fnStandAfterAnim(%s) used standby coords", _vm->fetchObjectName(params[2], buf));
	}

	assert(pars[4] >= 0 && pars[4] <= 7);

	_vm->_resman->closeResource(params[2]);
	return fnStandAt(pars);
}

int32 Logic::fnPause(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 number of game-cycles to pause

	// NB. Pause-value of 0 causes script to continue, 1 causes a 1-cycle
	// quit, 2 gives 2 cycles, etc.

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	if (ob_logic->looping == 0) {
		ob_logic->looping = 1;
		ob_logic->pause = params[1];
	}

	if (ob_logic->pause) {
		ob_logic->pause--;
		return IR_REPEAT;
	}

	ob_logic->looping = 0;
	return IR_CONT;
}

int32 Logic::fnMegaTableAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to animation table

	// 0 means normal forward anim
	return megaTableAnimate(params, false);
}

int32 Logic::fnAddMenuObject(int32 *params) {
	// params:	0 pointer to a MenuObject structure to copy down
	MenuObject *menuObject = (MenuObject *) _vm->_memory->decodePtr(params[0]);

	_vm->_mouse->addMenuObject(menuObject);
	return IR_CONT;
}

/**
 * Start a conversation.
 *
 * Note that fnStartConversation() might accidentally be called every time the
 * script loops back for another chooser, but we only want to reset the chooser
 * count flag the first time this function is called, i.e. when the talk flag
 * is zero.
 */

int32 Logic::fnStartConversation(int32 *params) {
	// params:	none

	_vm->_mouse->startConversation();
	return IR_CONT;
}

/**
 * End a conversation.
 */

int32 Logic::fnEndConversation(int32 *params) {
	// params:	none

	_vm->_mouse->endConversation();
	return IR_CONT;
}

int32 Logic::fnSetFrame(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 resource id of animation file
	//		2 frame flag (0=first 1=last)

	int32 res = params[1];
	assert(res);

	// open the resource (& check it's valid)
	byte *anim_file = _vm->_resman->openResource(res);

	StandardHeader *head = (StandardHeader *) anim_file;
	assert(head->fileType == ANIMATION_FILE);

	// set up pointer to the animation header
	AnimHeader *anim_head = _vm->fetchAnimHeader(anim_file);

	// set up anim resource in graphic object
	ObjectGraphic *ob_graphic = (ObjectGraphic *) _vm->_memory->decodePtr(params[0]);

	ob_graphic->anim_resource = res;
	ob_graphic->anim_pc = params[2] ? anim_head->noAnimFrames - 1 : 0;

	// Close the anim file and drop out of script
	_vm->_resman->closeResource(ob_graphic->anim_resource);
	return IR_CONT;
}

int32 Logic::fnRandomPause(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 minimum number of game-cycles to pause
	//		2 maximum number of game-cycles to pause

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);
	int32 pars[2];

	if (ob_logic->looping == 0) {
		pars[0] = params[1];
		pars[1] = params[2];
		fnRandom(pars);
		pars[1] = _scriptVars[RESULT];
	}

	pars[0] = params[0];
	return fnPause(pars);
}

int32 Logic::fnRegisterFrame(int32 *params) {
	// this call would be made from an objects service script 0

	// params:	0 pointer to mouse structure or NULL for no write to
	//		  mouse list (non-zero means write sprite-shape to
	//		  mouse list)
	//		1 pointer to graphic structure
	//		2 pointer to mega structure or NULL if not a mega

	ObjectMouse *ob_mouse = (ObjectMouse *) _vm->_memory->decodePtr(params[0]);
	ObjectGraphic *ob_graph = (ObjectGraphic *) _vm->_memory->decodePtr(params[1]);
	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[2]);

	_vm->_screen->registerFrame(ob_mouse, ob_graph, ob_mega);
	return IR_CONT;
}

int32 Logic::fnNoSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	setSpriteStatus(params[0], NO_SPRITE);
	return IR_CONT;
}

int32 Logic::fnSendSync(int32 *params) {
	// params:	0 sync's recipient
	//		1 sync value

	for (int i = 0; i < MAX_syncs; i++) {
		if (_syncList[i].id == 0) {
			debug(5, "%d sends sync %d to %d", _scriptVars[ID], params[1], params[0]);
			_syncList[i].id = params[0];
			_syncList[i].sync = params[1];
			return IR_CONT;
		}
	}

	// The original code didn't even check for this condition, so maybe
	// it should be a fatal error?

	warning("No free sync slot");
	return IR_CONT;
}

int32 Logic::fnUpdatePlayerStats(int32 *params) {
	// engine needs to know certain info about the player

	// params:	0 pointer to mega structure

	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[0]);
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	screenInfo->player_feet_x = ob_mega->feet_x;
	screenInfo->player_feet_y = ob_mega->feet_y;

	// for the script
	_scriptVars[PLAYER_FEET_X] = ob_mega->feet_x;
	_scriptVars[PLAYER_FEET_Y] = ob_mega->feet_y;
	_scriptVars[PLAYER_CUR_DIR] = ob_mega->current_dir;
	_scriptVars[SCROLL_OFFSET_X] = screenInfo->scroll_offset_x;

	debug(5, "fnUpdatePlayerStats: %d %d", ob_mega->feet_x, ob_mega->feet_y);

	return IR_CONT;
}

int32 Logic::fnPassGraph(int32 *params) {
	// makes an engine local copy of passed ObjectGraphic - run script 4
	// of an object to request this used by fnTurnTo(id) etc
	//
	// remember, we cannot simply read a compact any longer but instead
	// must request it from the object itself

	// params:	0 pointer to an ObjectGraphic structure

	warning("fnPassGraph() is a no-op now");
	return IR_CONT;
}

int32 Logic::fnInitFloorMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

 	ObjectMouse *ob_mouse = (ObjectMouse *) _vm->_memory->decodePtr(params[0]);
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	// floor is always lowest priority

	ob_mouse->x1 = 0;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = screenInfo->screen_wide - 1;
	ob_mouse->y2 = screenInfo->screen_deep - 1;
	ob_mouse->priority = 9;
	ob_mouse->pointer = NORMAL_MOUSE_ID;
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

	memcpy(&_engineMega, _vm->_memory->decodePtr(params[0]), sizeof(ObjectMega));
	return IR_CONT;
}

/**
 * Turn mega to face point (x,y) on the floor
 * Just needs to call fnWalk() with current feet coords & direction computed
 * by whatTarget()
 */

int32 Logic::fnFaceXY(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target x-coord
	//		5 target y-coord

	int32 pars[7];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	// If this is the start of the turn, get the mega's current feet
	// coords + the required direction

	if (!ob_logic->looping) {
		ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[2]);
	
		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = whatTarget(ob_mega->feet_x, ob_mega->feet_y, params[4], params[5]);
	}

	return fnWalk(pars);
}

/**
 * Causes no more objects in this logic loop to be processed. The logic engine
 * will restart at the beginning of the new list. The current screen will not
 * be drawn!
 */

int32 Logic::fnEndSession(int32 *params) {
	// params:	0 id of new run-list

	// terminate current and change to next run-list
	expressChangeSession(params[0]);

	// stop the script - logic engine will now go around and the new
	// screen will begin
	return IR_STOP;
}

int32 Logic::fnNoHuman(int32 *params) {
	// params:	none

	_vm->_mouse->noHuman();
	return IR_CONT;
}

int32 Logic::fnAddHuman(int32 *params) {
	// params:	none

	_vm->_mouse->addHuman();
	return IR_CONT;
}

/**
 * Wait for a target to become waiting, i.e. not busy.
 */

int32 Logic::fnWeWait(int32 *params) {
	// params:	0 target

	StandardHeader *head = (StandardHeader *) _vm->_resman->openResource(params[0]);
	assert(head->fileType == GAME_OBJECT);

	// Run the target's get-speech-state script

	int32 target = params[0];
	char *raw_script_ad = (char *) head;
	uint32 null_pc = 5;

	runScript(raw_script_ad, raw_script_ad, &null_pc);

	_vm->_resman->closeResource(target);

	if (_scriptVars[RESULT] == 0) {
		// The target is busy. Try again.
		_vm->_debugger->_speechScriptWaiting = target;
		return IR_REPEAT;
	}

	// The target is waiting, i.e. not busy.

	_vm->_debugger->_speechScriptWaiting = 0;
	return IR_CONT;
}

/**
 * Wait for a target to become waiting, i.e. not busy, send a command to it,
 * then wait for it to finish.
 */

int32 Logic::fnTheyDoWeWait(int32 *params) {
	// params:	0 pointer to ob_logic
	//		1 target
	//		2 command
	//		3 ins1
	//		4 ins2
	//		5 ins3
	//		6 ins4
	//		7 ins5

	StandardHeader *head = (StandardHeader *) _vm->_resman->openResource(params[1]);
	assert(head->fileType == GAME_OBJECT);

	// Run the target's get-speech-state script

	int32 target = params[1];
	char *raw_script_ad = (char *) head;
	uint32 null_pc = 5;

	runScript(raw_script_ad, raw_script_ad, &null_pc);

	_vm->_resman->closeResource(target);

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	if (_scriptVars[RESULT] == 1 && !_scriptVars[INS_COMMAND] && ob_logic->looping == 0) {
		// The target is waiting, i.e. not busy, and there is no other
		// command queued. We haven't sent the command yet, so do it.

		debug(5, "fnTheyDoWeWait: sending command to %d", target);

		_vm->_debugger->_speechScriptWaiting = target;
		ob_logic->looping = 1;

		_scriptVars[SPEECH_ID] = params[1];
		_scriptVars[INS_COMMAND] = params[2];
		_scriptVars[INS1] = params[3];
		_scriptVars[INS2] = params[4];
		_scriptVars[INS3] = params[5];
		_scriptVars[INS4] = params[6];
		_scriptVars[INS5] = params[7];

		return IR_REPEAT;
	}

	if (ob_logic->looping == 0) {
		// The command has not been sent yet. Keep waiting.
		_vm->_debugger->_speechScriptWaiting = target;
		return IR_REPEAT;
	}

	if (_scriptVars[RESULT] == 0) {
		// The command has been sent, and the target is busy doing it.
		// Wait for it to finish.

		debug(5, "fnTheyDoWeWait: Waiting for %d to finish", target);

		_vm->_debugger->_speechScriptWaiting = target;
		return IR_REPEAT;
	}

	debug(5, "fnTheyDoWeWait: %d finished", target);

	ob_logic->looping = 0;
	_vm->_debugger->_speechScriptWaiting = 0;
	return IR_CONT;
}

/**
 * Wait for a target to become waiting, i.e. not busy, then send a command to
 * it.
 */

int32 Logic::fnTheyDo(int32 *params) {
	// params:	0 target
	//		1 command
	//		2 ins1
	//		3 ins2
	//		4 ins3
	//		5 ins4
	//		6 ins5

	StandardHeader *head = (StandardHeader *) _vm->_resman->openResource(params[0]);
	assert (head->fileType == GAME_OBJECT);

	// Run the target's get-speech-state script

	int32 target = params[0];
	char *raw_script_ad = (char *) head;
	uint32 null_pc = 5;

	runScript(raw_script_ad, raw_script_ad, &null_pc);

	_vm->_resman->closeResource(target);

	if (_scriptVars[RESULT] == 1 && !_scriptVars[INS_COMMAND]) {
		// The target is waiting, i.e. not busy, and there is no other
		// command queued. Send the command.

		debug(5, "fnTheyDo: sending command to %d", target);

		_vm->_debugger->_speechScriptWaiting = 0;

		_scriptVars[SPEECH_ID] = params[0];
		_scriptVars[INS_COMMAND] = params[1];
		_scriptVars[INS1] = params[2];
		_scriptVars[INS2] = params[3];
		_scriptVars[INS3] = params[4];
		_scriptVars[INS4] = params[5];
		_scriptVars[INS5] = params[6];

		return IR_CONT;
	}

	// The target is busy. Come back again next cycle.

	_vm->_debugger->_speechScriptWaiting = target;
	return IR_REPEAT;
}

/**
 * Route to the left or right hand side of target id, if possible.
 */

int32 Logic::fnWalkToTalkToMega(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 id of target mega to face
	//		5 distance

	int32 pars[7];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	// If this is the start of the walk, calculate the route.

	if (!ob_logic->looping)	{
		StandardHeader *head = (StandardHeader *) _vm->_resman->openResource(params[4]);

		assert(head->fileType == GAME_OBJECT);

		// Call the base script. This is the graphic/mouse service
		// call, and will set _engineMega to the ObjectMega of mega we
		// want to route to.

		char *raw_script_ad = (char *) head;
		uint32 null_pc = 3;

		runScript(raw_script_ad, raw_script_ad, &null_pc);

		_vm->_resman->closeResource(params[4]);

		// Stand exactly beside the mega, ie. at same y-coord
		pars[5] = _engineMega.feet_y;

		ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[2]);

		// Apply scale factor to walk distance. Ay+B gives 256 * scale
		// ie. 256 * 256 * true_scale for even better accuracy, ie.
		// scale = (Ay + B) / 256

		int scale = (ob_mega->scale_a * ob_mega->feet_y + ob_mega->scale_b) / 256;
		int mega_separation = (params[5] * scale) / 256;

		debug(4, "Target is at (%d, %d), separation %d", _engineMega.feet_x, _engineMega.feet_y, mega_separation);

		if (_engineMega.feet_x < ob_mega->feet_x) {
			// Target is left of us, so aim to stand to their
			// right. Face down_left

			pars[4] = _engineMega.feet_x + mega_separation;
			pars[6] = 5;
		} else {
			// Ok, must be right of us so aim to stand to their
			// left. Face down_right.

			pars[4] = _engineMega.feet_x - mega_separation;
			pars[6] = 3;
		}
	}

	return fnWalk(pars);
}

int32 Logic::fnFadeDown(int32 *params) {
	// NONE means up! can only be called when screen is fully faded up -
	// multiple calls wont have strange effects

	// params:	none

	if (_vm->_screen->getFadeStatus() == RDFADE_NONE)
		_vm->_screen->fadeDown();

	return IR_CONT;
}

enum {
	S_OB_GRAPHIC	= 0,
	S_OB_SPEECH	= 1,
	S_OB_LOGIC	= 2,
	S_OB_MEGA	= 3,

	S_TEXT		= 4,
	S_WAV		= 5,
	S_ANIM		= 6,
	S_DIR_TABLE	= 7,
	S_ANIM_MODE	= 8
};

/**
 * It's the super versatile fnSpeak. Text and wavs can be selected in any
 * combination.
 *
 * @note We can assume no human - there should be no human, at least!
 */

int32 Logic::fnISpeak(int32 *params) {
	// params:	0 pointer to ob_graphic
	//		1 pointer to ob_speech
	//		2 pointer to ob_logic
	//		3 pointer to ob_mega
	//		4 encoded text number
	//		5 wav res id
	//		6 anim res id
	//		7 anim table res id
	//		8 animation mode	0 lip synced,
	//					1 just straight animation

	static bool cycle_skip = false;
	static bool speechRunning;

	// Set up the pointers which we know we'll always need

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[S_OB_LOGIC]);
	ObjectGraphic *ob_graphic = (ObjectGraphic *) _vm->_memory->decodePtr(params[S_OB_GRAPHIC]);

	// FIRST TIME ONLY: create the text, load the wav, set up the anim,
	// etc.

	if (!ob_logic->looping) {
		// New fudge to wait for smacker samples to finish
		// since they can over-run into the game

		if (_vm->_sound->getSpeechStatus() != RDSE_SAMPLEFINISHED)
			return IR_REPEAT;
		
		// New fudge for 'fx' subtitles: If subtitles switched off, and
		// we don't want to use a wav for this line either, then just
		// quit back to script right now!

		if (!_vm->getSubtitles() && !wantSpeechForLine(params[S_WAV]))
			return IR_CONT;

		// Drop out for 1st cycle to allow walks/anims to end and
		// display last frame before system locks while speech loaded

		if (!cycle_skip) {
			cycle_skip = true;
			return IR_REPEAT;
		}

		cycle_skip = false;

		_vm->_debugger->_textNumber = params[S_TEXT];

		// Pull out the text line to get the official text number
		// (for wav id). Once the wav id's go into all script text
		// commands, we'll only need this for debugging.

		uint32 text_res = params[S_TEXT] / SIZE;
		uint32 local_text = params[S_TEXT] & 0xffff;

		// For testing all text & speech!
		//
		// A script loop can send any text number to fnISpeak and it
		// will only run the valid ones or return with 'result' equal
		// to '1' or '2' to mean 'invalid text resource' and 'text
		// number out of range' respectively
		//
		// See 'testing_routines' object in George's Player Character
		// section of linc

		if (_scriptVars[SYSTEM_TESTING_TEXT]) {
			if (!_vm->_resman->checkValid(text_res)) {
				// Not a valid resource number - invalid (null
				// resource)
				_scriptVars[RESULT] = 1;
				return IR_CONT;
			}

			StandardHeader *head = (StandardHeader *) _vm->_resman->openResource(text_res);

			if (head->fileType != TEXT_FILE) {
				// Invalid - not a text resource
				_vm->_resman->closeResource(text_res);
				_scriptVars[RESULT] = 1;
				return IR_CONT;
			}

			if (!_vm->checkTextLine((byte *) head, local_text)) {
				// Line number out of range
				_vm->_resman->closeResource(text_res);
				_scriptVars[RESULT] = 2;
				return IR_CONT;
			}

			_vm->_resman->closeResource(text_res);
			_scriptVars[RESULT] = 0;
		}

		byte *text = _vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text);
		_officialTextNumber = READ_LE_UINT16(text);
		_vm->_resman->closeResource(text_res);

		// Prevent dud lines from appearing while testing text & speech
		// since these will not occur in the game anyway

		if (_scriptVars[SYSTEM_TESTING_TEXT]) {
			// If actor number is 0 and text line is just a 'dash'
			// character
			if (_officialTextNumber == 0 && text[2] == '-' && text[3] == 0) {
				_scriptVars[RESULT] = 3;
				return IR_CONT;
			}
		}

		// Set the 'looping_flag' and the text-click-delays. We can
		// left-click past the text after half a second, and
		// right-click past it after a quarter of a second.
 
		ob_logic->looping = 1;
		_leftClickDelay = 6;
		_rightClickDelay = 3;

		if (_scriptVars[PLAYER_ID] != CUR_PLAYER_ID)
			debug(5, "(%d) Nico: %s", _officialTextNumber, text + 2);
		else {
			byte buf[NAME_LEN];

			debug(5, "(%d) %s: %s", _officialTextNumber, _vm->fetchObjectName(_scriptVars[ID], buf), text + 2);
		}

		// Set up the speech animation

		if (params[S_ANIM]) {
			// Just a straight anim.
			_animId = params[6];
		} else if (params[S_DIR_TABLE]) {
			// Use this direction table to derive the anim
			// NB. ASSUMES WE HAVE A MEGA OBJECT!!

			ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[S_OB_MEGA]);
			int32 *anim_table = (int32 *) _vm->_memory->decodePtr(params[S_DIR_TABLE]);

			_animId = anim_table[ob_mega->current_dir];
		} else {
			// No animation choosen
			_animId = 0;
		}

		if (_animId) {
			// Set the talker's graphic to the first frame of this
			// speech anim for now.

			_speechAnimType = _scriptVars[SPEECHANIMFLAG];
			ob_graphic->anim_resource = _animId;
			ob_graphic->anim_pc = 0;
		}

		// Default back to looped lip synced anims.
		_scriptVars[SPEECHANIMFLAG] = 0;

		// Set up _textX and _textY for speech panning and/or text
		// sprite position.

		locateTalker(params);

		// Is it to be speech or subtitles or both?

		// Assume not running until know otherwise
		speechRunning = false;

		// New fudge for 'fx' subtitles: If speech is selected, and
		// this line is allowed speech (not if it's an fx subtitle!)

		if (!_vm->_sound->isSpeechMute() && wantSpeechForLine(_officialTextNumber)) {
			// If the wavId parameter is zero because not yet
			// compiled into speech command, we can still get it
			// from the 1st 2 chars of the text line.

			if (!params[S_WAV])
				params[S_WAV] = (int32) _officialTextNumber;

			// Panning goes from -16 (left) to 16 (right)
			int8 speech_pan = ((_textX - 320) * 16) / 320;

			if (speech_pan < -16)
				speech_pan = -16;
			else if (speech_pan > 16)
				speech_pan = 16;

			uint32 rv = _vm->_sound->playCompSpeech(params[S_WAV], 16, speech_pan);

			if (rv == RD_OK) {
				// Ok, we've got something to play. Set it
				// playing now. (We might want to do this the
				// next cycle, don't know yet.)

				speechRunning = true;
				_vm->_sound->unpauseSpeech();
			} else {
				debug(5, "ERROR: PlayCompSpeech(wav=%d (res=%d pos=%d)) returned %.8x", params[S_WAV], text_res, local_text, rv);
			}
		}

		if (_vm->getSubtitles() || !speechRunning) {
			// We want subtitles, or the speech failed to load.
			// Either way, we're going to show the text so create
			// the text sprite.

			formText(params);
		}
	}

	// EVERY TIME: run a cycle of animation, if there is one

	if (_animId) {
		// There is an animation - Increment the anim frame number.
		ob_graphic->anim_pc++;

		byte *anim_file = _vm->_resman->openResource(ob_graphic->anim_resource);
		AnimHeader *anim_head = _vm->fetchAnimHeader(anim_file);

		if (!_speechAnimType) {
			// ANIM IS TO BE LIP-SYNC'ED & REPEATING

			if (ob_graphic->anim_pc == (int32) (anim_head->noAnimFrames)) {
				// End of animation - restart from frame 0
				ob_graphic->anim_pc = 0;
			} else if (speechRunning && _vm->_sound->amISpeaking() == RDSE_QUIET) {
				// The speech is running, but we're at a quiet
				// bit. Restart from frame 0 (closed mouth).
				ob_graphic->anim_pc = 0;
			}
		} else {
			// ANIM IS TO PLAY ONCE ONLY
			if (ob_graphic->anim_pc == (int32) (anim_head->noAnimFrames) - 1) {
				// Reached the last frame of the anim. Hold
				// anim on this last frame
				_animId = 0;
			}
		}

		_vm->_resman->closeResource(ob_graphic->anim_resource);
	} else if (_speechAnimType) {
		// Placed here so we actually display the last frame of the
		// anim.
		_speechAnimType = 0;
	}

	// EVERY TIME: FIND OUT IF WE NEED TO STOP THE SPEECH NOW...

	// If there is a wav then we're using that to end the speech naturally

	bool speechFinished = false;

	// If playing a sample

	if (speechRunning) {
		// Has it finished?
		if (_vm->_sound->getSpeechStatus() == RDSE_SAMPLEFINISHED)
			speechFinished = true;
	} else if (!speechRunning && _speechTime) {
		// Counting down text time because there is no sample - this
		// ends the speech

		// if no sample then we're using _speechTime to end speech
		// naturally

		_speechTime--;
		if (!_speechTime)
			speechFinished = true;
	}

	// Ok, all is running along smoothly - but a click means stop
	// unnaturally

	int mouseX, mouseY;

	_vm->_mouse->getPos(mouseX, mouseY);

	// So that we can go to the options panel while text & speech is
	// being tested
	if (_scriptVars[SYSTEM_TESTING_TEXT] == 0 || mouseY > 0) {
		MouseEvent *me = _vm->mouseEvent();

		// Note that we now have TWO click-delays - one for LEFT
		// button, one for RIGHT BUTTON

		if ((!_leftClickDelay && me && (me->buttons & RD_LEFTBUTTONDOWN)) ||
		    (!_rightClickDelay && me && (me->buttons & RD_RIGHTBUTTONDOWN))) {
			// Mouse click, after click_delay has expired -> end
			// the speech.

			// if testing text & speech
			if (_scriptVars[SYSTEM_TESTING_TEXT]) {
				// and RB used to click past text
				if (me->buttons & RD_RIGHTBUTTONDOWN) {
					// then we want the previous line again
					_scriptVars[SYSTEM_WANT_PREVIOUS_LINE] = 1;
				} else {
					// LB just want next line again
					_scriptVars[SYSTEM_WANT_PREVIOUS_LINE] = 0;
				}
			}

			speechFinished = true;

			// if speech sample playing, halt it prematurely
			if (speechRunning)
				_vm->_sound->stopSpeech();
		}
	}

	// If we are finishing the speech this cycle, do the business

	// !speechAnimType, as we want an anim which is playing once to have
	// finished.

	if (speechFinished && !_speechAnimType) {
		// If there is text, kill it
		if (_speechTextBlocNo) {
			_vm->_fontRenderer->killTextBloc(_speechTextBlocNo);
			_speechTextBlocNo = 0;
		}

		// if there is a speech anim, end it on closed mouth frame
		if (_animId) {
			_animId = 0;
			ob_graphic->anim_pc = 0;
		}

		speechRunning = false;

		// no longer in a script function loop
		ob_logic->looping = 0;

		_vm->_debugger->_textNumber = 0;

		// reset to zero, in case text line not even extracted (since
		// this number comes from the text line)
		_officialTextNumber = 0;

		_scriptVars[RESULT] = 0;
		return IR_CONT;
	}

	// Speech still going, so decrement the click_delay if it's still
	// active

	if (_leftClickDelay)
		_leftClickDelay--;

 	if (_rightClickDelay)
		_rightClickDelay--;

	return IR_REPEAT;
}

/**
 * Reset the object and restart script 1 on level 0
 */

#define LEVEL (_curObjectHub->logic_level)

int32 Logic::fnTotalRestart(int32 *params) {
	// mega runs this to restart its base logic again - like being cached
	// in again

	// params:	none

	LEVEL = 0;
	_curObjectHub->script_pc[0] = 1;
	return IR_TERMINATE;
}

int32 Logic::fnSetWalkGrid(int32 *params) {
	// params:	none

	warning("fnSetWalkGrid() is no longer a valid opcode");
	return IR_CONT;
}

/**
 * Receive and sequence the commands sent from the conversation script. We have
 * to do this in a slightly tweeky manner as we can no longer have generic
 * scripts.
 */

enum {
	INS_talk		= 1,
	INS_anim		= 2,
	INS_reverse_anim	= 3,
	INS_walk		= 4,
	INS_turn		= 5,
	INS_face		= 6,
	INS_trace		= 7,
	INS_no_sprite		= 8,
	INS_sort		= 9,
	INS_foreground		= 10,
	INS_background		= 11,
	INS_table_anim		= 12,
	INS_reverse_table_anim	= 13,
	INS_walk_to_anim	= 14,
	INS_set_frame		= 15,
	INS_stand_after_anim	= 16,
	INS_quit		= 42
};

int32 Logic::fnSpeechProcess(int32 *params) {
	// params:	0 pointer to ob_graphic
	//		1 pointer to ob_speech
	//		2 pointer to ob_logic
	//		3 pointer to ob_mega
	//		4 pointer to ob_walkdata

	ObjectSpeech *ob_speech = (ObjectSpeech *) _vm->_memory->decodePtr(params[1]);

	while (1) {
		int32 pars[9];

		// Check which command we're waiting for, and call the
		// appropriate function. Once we're done, clear the command
		// and set wait_state to 1.
		//
		// Note: we could save a var and ditch wait_state and check
		// 'command' for non zero means busy
		//
		// Note: I can't see that we ever check the value of wait_state
		// but perhaps it accesses that memory location directly?

		switch (ob_speech->command) {
		case 0:
			break;
		case INS_talk:
			pars[0] = params[0];		// ob_graphic
			pars[1] = params[1];		// ob_speech
			pars[2] = params[2];		// ob_logic
			pars[3] = params[3];		// ob_mega
			pars[4] = ob_speech->ins1;	// encoded text number
			pars[5] = ob_speech->ins2;	// wav res id
			pars[6] = ob_speech->ins3;	// anim res id
			pars[7] = ob_speech->ins4;	// anim table res id
			pars[8] = ob_speech->ins5;	// animation mode - 0 lip synced, 1 just straight animation

			if (fnISpeak(pars) != IR_REPEAT) {
				ob_speech->command = 0;
				ob_speech->wait_state = 1;
			}

			return IR_REPEAT;
		case INS_turn:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = ob_speech->ins1;	// direction to turn to

			if (fnTurn(pars) != IR_REPEAT) {
				ob_speech->command = 0;
				ob_speech->wait_state = 1;
			}

			return IR_REPEAT;
		case INS_face:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = ob_speech->ins1;	// target

			if (fnFaceMega(pars) != IR_REPEAT) {
				ob_speech->command = 0;
				ob_speech->wait_state = 1;
			}

			return IR_REPEAT;
		case INS_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = ob_speech->ins1;	// anim res

			if (fnAnim(pars) != IR_REPEAT) {
				ob_speech->command = 0;
				ob_speech->wait_state = 1;
			}

			return IR_REPEAT;
		case INS_reverse_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = ob_speech->ins1;	// anim res

			if (fnReverseAnim(pars) != IR_REPEAT) {
				ob_speech->command = 0;
				ob_speech->wait_state = 1;
			}

			return IR_REPEAT;
		case INS_table_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = ob_speech->ins1;	// pointer to anim table

			if (fnMegaTableAnim(pars) != IR_REPEAT) {
				ob_speech->command = 0;
				ob_speech->wait_state = 1;
			}

			return IR_REPEAT;
		case INS_reverse_table_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = ob_speech->ins1;	// pointer to anim table

			if (fnReverseMegaTableAnim(pars) != IR_REPEAT) {
				ob_speech->command = 0;
				ob_speech->wait_state = 1;
			}

			return IR_REPEAT;
		case INS_no_sprite:
			fnNoSprite(params);		// ob_graphic

			ob_speech->command = 0;
			ob_speech->wait_state = 1;
			return IR_REPEAT ;
		case INS_sort:
			fnSortSprite(params);		// ob_graphic

			ob_speech->command = 0;
			ob_speech->wait_state = 1;
			return IR_REPEAT;
		case INS_foreground:
			fnForeSprite(params);		// ob_graphic

			ob_speech->command = 0;
			ob_speech->wait_state = 1;
			return IR_REPEAT;
		case INS_background:
			fnBackSprite(params);		// ob_graphic

			ob_speech->command = 0;
			ob_speech->wait_state = 1;
			return IR_REPEAT;
		case INS_walk:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = ob_speech->ins1;	// target x
			pars[5] = ob_speech->ins2;	// target y
			pars[6] = ob_speech->ins3;	// target direction

			if (fnWalk(pars) != IR_REPEAT) {
				ob_speech->command = 0;
				ob_speech->wait_state = 1;
			}

			return IR_REPEAT;
		case INS_walk_to_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = ob_speech->ins1;	// anim resource

			if (fnWalkToAnim(pars) != IR_REPEAT) {
				ob_speech->command = 0;
				ob_speech->wait_state = 1;
			}

			return IR_REPEAT;
		case INS_stand_after_anim:
			pars[0] = params[0];		// ob_graphic
			pars[1] = params[3];		// ob_mega
			pars[2] = ob_speech->ins1;	// anim resource

			fnStandAfterAnim(pars);

			ob_speech->command = 0;
			ob_speech->wait_state = 1;
			return IR_REPEAT;
		case INS_set_frame:
			pars[0] = params[0];		// ob_graphic
			pars[1] = ob_speech->ins1;	// anim_resource
			pars[2] = ob_speech->ins2;	// FIRST_FRAME or LAST_FRAME
			fnSetFrame(pars);

			ob_speech->command = 0;
			ob_speech->wait_state = 1;
			return IR_REPEAT;
		case INS_quit:
			// That's it - we're finished with this
			ob_speech->command = 0;
			// ob_speech->wait_state = 0;
			return IR_CONT;
		default:
			// Unimplemented command - just cancel
			ob_speech->command = 0;
			ob_speech->wait_state = 1;
			break;
		}

		if (_scriptVars[SPEECH_ID] == _scriptVars[ID]) {
			// There's a new command for us! Grab the command -
			// potentially we only have this cycle to do this - and
			// set things up so that the command will be picked up
			// on the next iteration of the while loop.

			debug(5, "fnSpeechProcess: Received new command %d", _scriptVars[INS_COMMAND]);

			_scriptVars[SPEECH_ID] = 0;

			ob_speech->command = _scriptVars[INS_COMMAND];
			ob_speech->ins1 = _scriptVars[INS1];
			ob_speech->ins2 = _scriptVars[INS2];
			ob_speech->ins3 = _scriptVars[INS3];
			ob_speech->ins4 = _scriptVars[INS4];
			ob_speech->ins5 = _scriptVars[INS5];
			ob_speech->wait_state = 0;

			_scriptVars[INS_COMMAND] = 0;
		} else {
			// No new command. We could run a blink anim (or
			// something) here.

			ob_speech->wait_state = 1;
			return IR_REPEAT;
		}
	}
}

int32 Logic::fnSetScaling(int32 *params) {
	// params:	0 pointer to object's mega structure
	//		1 scale constant A
	//		2 scale constant B

	// 256 * s = A * y + B

	// Where s is system scale, which itself is (256 * actual_scale) ie.
	// s == 128 is half size

 	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[0]);

	ob_mega->scale_a = params[1];
	ob_mega->scale_b = params[2];

	return IR_CONT;
}

int32 Logic::fnStartEvent(int32 *params) {
	// params:	none

	startEvent();
	return IR_TERMINATE;
}

int32 Logic::fnCheckEventWaiting(int32 *params) {
	// params:	none

	_scriptVars[RESULT] = checkEventWaiting();
	return IR_CONT;
}

int32 Logic::fnRequestSpeech(int32 *params) {
	// change current script - must be followed by a TERMINATE script
	// directive

	// params:	0 id of target to catch the event and startup speech
	//		  servicing

	// Full script id to interact with - megas run their own 7th script
	sendEvent(params[0], (params[0] << 16) | 6);
	return IR_CONT;
}

int32 Logic::fnGosub(int32 *params) {
	// params:	0 id of script

	// Hurray, script subroutines. Logic goes up - pc is saved for current
	// level.
	logicUp(params[0]);
	return IR_GOSUB;
}

/**
 * Wait for a target to become waiting, i.e. not busy, or until we time out.
 * This is useful when clicking on a target to talk to it, and it doesn't
 * reply. This way, we won't lock up.
 *
 * If the target becomes waiting, RESULT is set to 0. If we time out, RESULT is
 * set to 1.
 */

int32 Logic::fnTimedWait(int32 *params) {
	// params:	0 ob_logic
	//		1 target
	//		2 number of cycles before give up

	StandardHeader *head = (StandardHeader *) _vm->_resman->openResource(params[1]);
	assert(head->fileType == GAME_OBJECT);

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	if (!ob_logic->looping) {
		// This is the first time, so set up the time-out.
		ob_logic->looping = params[2];
	}

	// Run the target's get-speech-state script

	int32 target = params[1];
	char *raw_script_ad = (char *) head;
	uint32 null_pc = 5;

	runScript(raw_script_ad, raw_script_ad, &null_pc);

	_vm->_resman->closeResource(target);

	if (_scriptVars[RESULT] == 1) {
		// The target is waiting, i.e. not busy

		_vm->_debugger->_speechScriptWaiting = 0;

		ob_logic->looping = 0;
		_scriptVars[RESULT] = 0;
		return IR_CONT;
	}

	ob_logic->looping--;

	if (!ob_logic->looping) {
		// Time's up.

		debug(5, "fnTimedWait: Timed out waiting for %d", target);
		_vm->_debugger->_speechScriptWaiting = 0;

		// Clear the event that hasn't been picked up - in theory,
		// none of this should ever happen.

		killAllIdsEvents(target);
		_scriptVars[RESULT] = 1;
		return IR_CONT;
	}

	// Target is busy. Keep trying.

	_vm->_debugger->_speechScriptWaiting = target;
	return IR_REPEAT;
}

int32 Logic::fnPlayFx(int32 *params) {
	// params:	0 sample resource id
	//		1 type		(FX_SPOT, FX_RANDOM, FX_LOOP)
	//		2 delay		(0..65535)
	//		3 volume	(0..16)
	//		4 pan		(-16..16)

	// example script:
	//		fnPlayFx (FXWATER, FX_LOOP, 0, 10, 15);
	//		// fx_water is just a local script flag
	//		fx_water = result;
	//		.
	//		.
	//		.
	//		fnStopFx (fx_water);

	int32 res = params[0];
	int32 type = params[1];
	int32 delay = params[2];
	int32 volume = params[3];
	int32 pan = params[4];

	_vm->_sound->queueFx(res, type, delay, volume, pan);
	return IR_CONT;
}

int32 Logic::fnStopFx(int32 *params) {
	// params:	0 position in queue
	if (_vm->_sound->stopFx(params[0]) != RD_OK)
		debug(5, "SFX ERROR: Trying to stop an inactive sound slot");

	return IR_CONT;
}

/**
 * Start a tune playing, to play once or to loop until stopped or next one
 * played.
 */

int32 Logic::fnPlayMusic(int32 *params) {
	// params:	0 tune id
	//		1 loop flag (0 or 1)

	char filename[128];
	bool loopFlag;
	uint32 rv;

	loopFlag = (params[1] == FX_LOOP);

	rv = _vm->_sound->streamCompMusic(params[0], loopFlag);

	if (rv)
		debug(5, "ERROR: streamCompMusic(%s, %d, %d) returned error 0x%.8x", filename, params[0], loopFlag, rv);

	return IR_CONT;
}

int32 Logic::fnStopMusic(int32 *params) {
	// params:	none

	_vm->_sound->stopMusic(false);
	return IR_CONT;
}

int32 Logic::fnSetValue(int32 *params) {
	// temp. function!

	// used for setting far-referenced megaset resource field in mega
	// object, from start script

	// params:	0 pointer to object's mega structure
	//		1 value to set it to

	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[0]);

	ob_mega->megaset_res = params[1];
	return IR_CONT;
}

int32 Logic::fnNewScript(int32 *params) {
	// change current script - must be followed by a TERMINATE script
	// directive

	// params:	0 id of script

	_scriptVars[PLAYER_ACTION] = 0;		// must clear this
	logicReplace(params[0]);
	return IR_TERMINATE;
}

/**
 * Like getSync(), but called from scripts. Sets the RESULT variable to
 * the sync value, or 0 if none is found.
 */

int32 Logic::fnGetSync(int32 *params) {
	// params:	none

	int slot = getSync();

	_scriptVars[RESULT] = (slot != -1) ? _syncList[slot].sync : 0;
	return IR_CONT;
}

/**
 * Wait for sync to happen. Sets the RESULT variable to the sync value, once
 * it has been found.
 */

int32 Logic::fnWaitSync(int32 *params) {
	// params:	none

	debug(6, "fnWaitSync: %d waits", _scriptVars[ID]);

	int slot = getSync();

	if (slot == -1)
		return IR_REPEAT;

	debug(5, "fnWaitSync: %d got sync %d", _scriptVars[ID], _syncList[slot].sync);
	_scriptVars[RESULT] = _syncList[slot].sync;
	return IR_CONT;
}

int32 Logic::fnRegisterWalkGrid(int32 *params) {
	// params:	none

	warning("fnRegisterWalkGrid() is no longer a valid opcode");
	return IR_CONT;
}

int32 Logic::fnReverseMegaTableAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to animation table

	// 1 means reverse anim
	return megaTableAnimate(params, true);
}

int32 Logic::fnReverseAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 resource id of animation file

	// 1 means reverse anim
	return animate(params, true);
}

/**
 * Mark this object for killing - to be killed when player leaves this screen.
 * Object reloads and script restarts upon re-entry to screen, which causes
 * this object's startup logic to be re-run every time we enter the screen.
 * "Which is nice."
 *
 * @note Call ONCE from object's logic script, i.e. in startup code, so not
 * re-called every time script frops off and restarts!
 */

int32 Logic::fnAddToKillList(int32 *params) {
	// params:	none

	// DON'T EVER KILL GEORGE!
	if (_scriptVars[ID] == CUR_PLAYER_ID)
		return IR_CONT;

	// Scan the list to see if it's already included

	for (uint32 i = 0; i < _kills; i++) {
		if (_objectKillList[i] == _scriptVars[ID])
			return IR_CONT;
	}

	assert(_kills < OBJECT_KILL_LIST_SIZE);	// no room at the inn

	_objectKillList[_kills++] = _scriptVars[ID];

	// "another one bites the dust"

	// When we leave the screen, all these object resources are to be
	// cleaned out of memory and the kill list emptied by doing
	// '_kills = 0', ensuring that all resources are in fact still in
	// memory and, more importantly, closed before killing!

	return IR_CONT;
}

/**
 * Set the standby walk coords to be used by fnWalkToAnim() and
 * fnStandAfterAnim() when the anim header's start/end coords are zero.
 * Useful during development; can stay in final game anyway.
 */

int32 Logic::fnSetStandbyCoords(int32 *params) {
	// params:	0 x-coord
	//		1 y-coord
	//		2 direction (0..7)

	assert(params[2] >= 0 && params[2] <= 7);

	_standbyX = (int16) params[0];
	_standbyY = (int16) params[1];
	_standbyDir = (uint8) params[2];

	return IR_CONT;
}

int32 Logic::fnBackPar0Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	setSpriteStatus(params[0], BGP0_SPRITE);
	return IR_CONT;
}

int32 Logic::fnBackPar1Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	setSpriteStatus(params[0], BGP1_SPRITE);
	return IR_CONT;
}

int32 Logic::fnForePar0Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	setSpriteStatus(params[0], FGP0_SPRITE);
	return IR_CONT;
}

int32 Logic::fnForePar1Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	setSpriteStatus(params[0], FGP1_SPRITE);
	return IR_CONT;
}

int32 Logic::fnSetPlayerActionEvent(int32 *params) {
	// we want to intercept the player character and have him interact
	// with an object - from script this code is the same as the mouse
	// engine calls when you click on an object - here, a third party
	// does the clicking IYSWIM

	// note - this routine used CUR_PLAYER_ID as the target

	// params:	0 id to interact with

	setPlayerActionEvent(CUR_PLAYER_ID, params[0]);
	return IR_CONT;
}

/**
 * Set the special scroll offset variables
 *
 * Call when starting screens and to change the camera within screens
 *
 * call AFTER fnInitBackground() to override the defaults
 */

int32 Logic::fnSetScrollCoordinate(int32 *params) {
	// params:	0 feet_x value
	// 		1 feet_y value

  	// Called feet_x and feet_y to retain intellectual compatibility with
	// Sword1!
	//
	// feet_x & feet_y refer to the physical screen coords where the
	// system will try to maintain George's feet

	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	screenInfo->feet_x = params[0];
	screenInfo->feet_y = params[1];
	return IR_CONT;
}

/**
 * Stand mega at start position of anim
 */

int32 Logic::fnStandAtAnim(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 anim resource id

	byte *anim_file = _vm->_resman->openResource(params[2]);
	AnimHeader *anim_head = _vm->fetchAnimHeader(anim_file);

	int32 pars[5];

	pars[0] = params[0];
	pars[1] = params[1];

	pars[2] = anim_head->feetStartX;
	pars[3] = anim_head->feetStartY;
	pars[4] = anim_head->feetStartDir;

	// If start coords not available use the standby coords (which should
	// be set beforehand in the script)

	if (pars[2] == 0 && pars[3] == 0) {
		byte buf[NAME_LEN];

		pars[2] = _standbyX;
		pars[3] = _standbyY;
		pars[4] = _standbyDir;

		debug(3, "WARNING: fnStandAtAnim(%s) used standby coords", _vm->fetchObjectName(params[2], buf));
	}

	assert(pars[4] >= 0 && pars[4] <= 7);

	_vm->_resman->closeResource(params[2]);
	return fnStandAt(pars);
}

#define SCROLL_MOUSE_WIDTH 20

int32 Logic::fnSetScrollLeftMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

 	ObjectMouse *ob_mouse = (ObjectMouse *) _vm->_memory->decodePtr(params[0]);
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	// Highest priority

	ob_mouse->x1 = 0;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = screenInfo->scroll_offset_x + SCROLL_MOUSE_WIDTH;
	ob_mouse->y2 = screenInfo->screen_deep - 1;
	ob_mouse->priority = 0;

	if (screenInfo->scroll_offset_x > 0) {
		// not fully scrolled to the left
		ob_mouse->pointer = SCROLL_LEFT_MOUSE_ID;
	} else {
		// so the mouse area doesn't get registered
		ob_mouse->pointer = 0;
	}

	return IR_CONT;
}

int32 Logic::fnSetScrollRightMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

	ObjectMouse *ob_mouse = (ObjectMouse *) _vm->_memory->decodePtr(params[0]);
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	// Highest priority

	ob_mouse->x1 = screenInfo->scroll_offset_x + _vm->_screen->getScreenWide() - SCROLL_MOUSE_WIDTH;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = screenInfo->screen_wide - 1;
	ob_mouse->y2 = screenInfo->screen_deep - 1;
	ob_mouse->priority = 0;

	if (screenInfo->scroll_offset_x < screenInfo->max_scroll_offset_x) {
		// not fully scrolled to the right
		ob_mouse->pointer = SCROLL_RIGHT_MOUSE_ID;
	} else {
		// so the mouse area doesn't get registered
		ob_mouse->pointer = 0;
	}

	return IR_CONT;
}

int32 Logic::fnColour(int32 *params) {
	// set border colour - useful during script development
	// eg. set to colour during a timer situation, then black when timed
	// out

	// params	0: colour (see defines above)

#ifdef SWORD2_DEBUG
	// what colour?
	switch (params[0]) {
	case BLACK:
		_vm->_screen->setPalette(0, 1, black, RDPAL_INSTANT);
		break;
	case WHITE:
		_vm->_screen->setPalette(0, 1, white, RDPAL_INSTANT);
		break;
	case RED:
		_vm->_screen->setPalette(0, 1, red, RDPAL_INSTANT);
		break;
	case GREEN:
		_vm->_screen->setPalette(0, 1, green, RDPAL_INSTANT);
		break;
	case BLUE:
		_vm->_screen->setPalette(0, 1, blue, RDPAL_INSTANT);
		break;
	}
#endif

	return IR_CONT;
}

#ifdef SWORD2_DEBUG
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

#ifdef SWORD2_DEBUG
	// what colour?
	switch (params[0]) {
	case WHITE:
		_vm->_screen->setPalette(0, 1, white, RDPAL_INSTANT);
		break;
	case RED:
		_vm->_screen->setPalette(0, 1, red, RDPAL_INSTANT);
		break;
	case GREEN:
		_vm->_screen->setPalette(0, 1, green, RDPAL_INSTANT);
		break;
	case BLUE:
		_vm->_screen->setPalette(0, 1, blue, RDPAL_INSTANT);
		break;
	}

	// There used to be a busy-wait loop here, so I don't know how long
	// the delay was meant to be. Probably doesn't matter much.

	_vm->_screen->updateDisplay();
	_vm->_system->delayMillis(250);
	_vm->_screen->setPalette(0, 1, black, RDPAL_INSTANT);
#endif

	return IR_CONT;
}

int32 Logic::fnPreFetch(int32 *params) {
	// Go fetch resource in the background.

	// params:	0 resource to fetch [guess]

	return IR_CONT;
}

/**
 * Reverse of fnPassPlayerSaveData() - run script 8 of player object.
 */

int32 Logic::fnGetPlayerSaveData(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure

	byte *logic_ptr = _vm->_memory->decodePtr(params[0]);
	byte *graphic_ptr = _vm->_memory->decodePtr(params[1]);
	byte *mega_ptr = _vm->_memory->decodePtr(params[2]);

	// Copy from savegame header to player object

	memcpy(logic_ptr, &_vm->_saveGameHeader.logic, sizeof(ObjectLogic));
	memcpy(graphic_ptr, &_vm->_saveGameHeader.graphic, sizeof(ObjectGraphic));
	memcpy(mega_ptr, &_vm->_saveGameHeader.mega, sizeof(ObjectMega));

 	// Any walk-data must be cleared - the player will be set to stand if
	// he was walking when saved.

	ObjectMega *ob_mega = (ObjectMega *) mega_ptr;

	if (ob_mega->currently_walking) {
		ob_mega->currently_walking = 0;

		int32 pars[3];

		pars[0] = params[1];			// ob_graphic;
		pars[1] = params[2];			// ob_mega
		pars[2] = ob_mega->current_dir;

		fnStand(pars);

		// Reset looping flag (which would have been 1 during fnWalk)
		ObjectLogic *ob_logic = (ObjectLogic *) logic_ptr;

		ob_logic->looping = 0;
	}

	return IR_CONT;
}

/**
 * Copies the 4 essential player structures into the savegame header - run
 * script 7 of player object to request this.
 *
 * Remember, we cannot simply read a compact any longer but instead must
 * request it from the object itself.
 */

int32 Logic::fnPassPlayerSaveData(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure

	// Copy from player object to savegame header

	memcpy(&_vm->_saveGameHeader.logic, _vm->_memory->decodePtr(params[0]), sizeof(ObjectLogic));
	memcpy(&_vm->_saveGameHeader.graphic, _vm->_memory->decodePtr(params[1]), sizeof(ObjectGraphic));
	memcpy(&_vm->_saveGameHeader.mega, _vm->_memory->decodePtr(params[2]), sizeof(ObjectMega));

	return IR_CONT;
}

int32 Logic::fnSendEvent(int32 *params) {
	// we want to intercept the player character and have him interact
	// with an object - from script

	// params:	0 id to receive event
	//		1 script to run

	sendEvent(params[0], params[1]);
	return IR_CONT;
}

/**
 * Add this walkgrid resource to the list of those used for routing in this
 * location. Note that this is ignored if the resource is already in the list.
 */

int32 Logic::fnAddWalkGrid(int32 *params) {
	// params:	0 id of walkgrid resource

	// All objects that add walkgrids must be restarted whenever we
	// re-enter a location.

	// DON'T EVER KILL GEORGE!
	if (_scriptVars[ID] != 8) {
		// Need to call this in case it wasn't called in script!
		fnAddToKillList(NULL);
	}

	_router->addWalkGrid(params[0]);
	fnPreLoad(params);
	return IR_CONT;
}

/**
 * Remove this walkgrid resource from the list of those used for routing in
 * this location. Note that this is ignored if the resource isn't actually
 * in the list.
 */

int32 Logic::fnRemoveWalkGrid(int32 *params) {
	// params:	0 id of walkgrid resource

	_router->removeWalkGrid(params[0]);
	return IR_CONT;
}

// like fnCheckEventWaiting, but starts the event rather than setting RESULT
// to 1

int32 Logic::fnCheckForEvent(int32 *params) {
	// params:	none

	if (checkEventWaiting()) {
		startEvent();
		return IR_TERMINATE;
	}

	return IR_CONT;
}

// combination of fnPause and fnCheckForEvent
// - ie. does a pause, but also checks for event each cycle

int32 Logic::fnPauseForEvent(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 number of game-cycles to pause

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	if (checkEventWaiting()) {
		ob_logic->looping = 0;
		startEvent();
		return IR_TERMINATE;
	}

	return fnPause(params);
}

int32 Logic::fnClearEvent(int32 *params) {
	// params:	none

	clearEvent(_scriptVars[ID]);
	return IR_CONT;
}

int32 Logic::fnFaceMega(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 id of target mega to face

	int32 pars[7];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	// If this is the start of the walk, decide where to walk to.

	if (!ob_logic->looping) {
		StandardHeader *head = (StandardHeader *) _vm->_resman->openResource(params[4]);

		assert(head->fileType == GAME_OBJECT);

		// Call the base script. This is the graphic/mouse service
		// call, and will set _engineMega to the ObjectMega of mega we
		// want to turn to face.

		char *raw_script_ad = (char *) head;
		uint32 null_pc = 3;

		runScript(raw_script_ad, raw_script_ad, &null_pc);

		_vm->_resman->closeResource(params[4]);

		ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[2]);

		pars[3] = params[3];
		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = whatTarget(ob_mega->feet_x, ob_mega->feet_y, _engineMega.feet_x, _engineMega.feet_y);
	}

	return fnWalk(pars);
}

int32 Logic::fnPlaySequence(int32 *params) {
	// params:	0 pointer to null-terminated ascii filename
	// 		1 number of frames in the sequence, used for PSX.

	char filename[30];
	MovieTextObject *sequenceSpeechArray[MAX_SEQUENCE_TEXT_LINES + 1];

	// The original code had some #ifdef blocks for skipping or muting the
	// cutscenes - fondly described as "the biggest fudge in the history
	// of computer games" - but at the very least we want to show the
	// cutscene subtitles, so I removed them.

	debug(5, "fnPlaySequence(\"%s\");", (const char *) _vm->_memory->decodePtr(params[0]));

	// add the appropriate file extension & play it

	strcpy(filename, (const char *) _vm->_memory->decodePtr(params[0]));

	// Write to walkthrough file (zebug0.txt)
 	debug(5, "PLAYING SEQUENCE \"%s\"", filename);

	// now create the text sprites, if any

	if (_sequenceTextLines)
		createSequenceSpeech(sequenceSpeechArray);

	// don't want to carry on streaming game music when smacker starts!
	fnStopMusic(NULL);

	// pause sfx during sequence
	_vm->_sound->pauseFx();

	MoviePlayer player(_vm);
 	uint32 rv;

	if (_sequenceTextLines && !_scriptVars[DEMO])
		rv = player.play(filename, sequenceSpeechArray, _smackerLeadIn, _smackerLeadOut);
	else
		rv = player.play(filename, NULL, _smackerLeadIn, _smackerLeadOut);

	// check the error return-value
	if (rv)
		debug(5, "MoviePlayer.play(\"%s\") returned 0x%.8x", filename, rv);

	// unpause sound fx again, in case we're staying in same location
	_vm->_sound->unpauseFx();

	_smackerLeadIn = 0;
	_smackerLeadOut = 0;

	// now clear the text sprites, if any

	if (_sequenceTextLines)
		clearSequenceSpeech(sequenceSpeechArray);

	// now clear the screen in case the Sequence was quitted (using ESC)
	// rather than fading down to black

	_vm->_screen->clearScene();

	// zero the entire palette in case we're about to fade up!

	byte pal[4 * 256];

	memset(pal, 0, sizeof(pal));
	_vm->_screen->setPalette(0, 256, pal, RDPAL_INSTANT);

	debug(5, "fnPlaySequence FINISHED");
	return IR_CONT;
}

int32 Logic::fnShadedSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	setSpriteShading(params[0], SHADED_SPRITE);
	return IR_CONT;
}

int32 Logic::fnUnshadedSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	setSpriteShading(params[0], UNSHADED_SPRITE);
	return IR_CONT;
}

int32 Logic::fnFadeUp(int32 *params) {
	// params:	none

	_vm->_screen->waitForFade();

	if (_vm->_screen->getFadeStatus() == RDFADE_BLACK)
		_vm->_screen->fadeUp();

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

	_vm->_screen->displayMsg(_vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text) + 2, 3);
	_vm->_resman->closeResource(text_res);

	return IR_CONT;
}

int32 Logic::fnSetObjectHeld(int32 *params) {
	// params:	0 luggage icon to set
	uint32 res = (uint32) params[0];

	_vm->_mouse->setObjectHeld(res);
	return IR_CONT;
}

int32 Logic::fnAddSequenceText(int32 *params) {
	// params:	0 text number
	//		1 frame number to start the text displaying
	//		2 frame number to stop the text dispalying

	assert(_sequenceTextLines < MAX_SEQUENCE_TEXT_LINES);

	_sequenceTextList[_sequenceTextLines].textNumber = params[0];
	_sequenceTextList[_sequenceTextLines].startFrame = params[1];
	_sequenceTextList[_sequenceTextLines].endFrame = params[2];
	_sequenceTextLines++;
	return IR_CONT;
}

int32 Logic::fnResetGlobals(int32 *params) {
	// fnResetGlobals is used by the demo - so it can loop back & restart
	// itself

	// params:	none

	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	int32 size;
	uint32 *globals;

	size = _vm->_resman->fetchLen(1);
	size -= sizeof(StandardHeader);

	debug(5, "globals size: %d", size);

	globals = (uint32 *) ((byte *) _vm->_resman->openResource(1) + sizeof(StandardHeader));

	// blank each global variable
	memset(globals, 0, size);

	_vm->_resman->closeResource(1);

	// all objects but george
	_vm->_resman->killAllObjects(false);

	// FOR THE DEMO - FORCE THE SCROLLING TO BE RESET!
	// - this is taken from fnInitBackground

	// switch on scrolling (2 means first time on screen)
	screenInfo->scroll_flag = 2;

	return IR_CONT;
}

int32 Logic::fnSetPalette(int32 *params) {
	// params:	0 resource number of palette file, or 0 if it's to be
	//		  the palette from the current screen

	_vm->_screen->setFullPalette(params[0]);
	return IR_CONT;
}

// use this in the object's service script prior to registering the mouse area
// ie. before fnRegisterMouse or fnRegisterFrame
// - best if kept at very top of service script

int32 Logic::fnRegisterPointerText(int32 *params) {
	// params:	0 local id of text line to use as pointer text

	_vm->_mouse->registerPointerText(params[0]);
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

int32 Logic::fnPrepareMusic(int32 *params) {
	// params:	1 id of music to prepare [guess]
	return IR_CONT;
}

int32 Logic::fnSoundFetch(int32 *params) {
	// params:	0 id of sound to fetch [guess]
	return IR_CONT;
}

int32 Logic::fnSmackerLeadIn(int32 *params) {
	// params:	0 id of lead-in music

	// ready for use in fnPlaySequence
	_smackerLeadIn = params[0];
	return IR_CONT;
}

int32 Logic::fnSmackerLeadOut(int32 *params) {
	// params:	0 id of lead-out music

	// ready for use in fnPlaySequence
	_smackerLeadOut = params[0];
	return IR_CONT;
}

/**
 * Stops all FX and clears the entire FX queue.
 */

int32 Logic::fnStopAllFx(int32 *params) {
	// params:	none

	_vm->_sound->clearFxQueue();
	return IR_CONT;
}

int32 Logic::fnCheckPlayerActivity(int32 *params) {
	// Used to decide when to trigger music cues described as "no player
	// activity for a while"

	// params:	0 threshold delay in seconds, ie. what we want to
	//		  check the actual delay against

	uint32 seconds = (uint32) params[0];

	_vm->_mouse->checkPlayerActivity(seconds);
	return IR_CONT;
}

int32 Logic::fnResetPlayerActivityDelay(int32 *params) {
	// Use if you want to deliberately reset the "no player activity"
	// counter for any reason

	// params:	none

	_vm->_mouse->resetPlayerActivityDelay();
	return IR_CONT;
}

int32 Logic::fnCheckMusicPlaying(int32 *params) {
	// params:	none

	// sets result to no. of seconds of current tune remaining
	// or 0 if no music playing

	// in seconds, rounded up to the nearest second
	_scriptVars[RESULT] = _vm->_sound->musicTimeRemaining();

	return IR_CONT;
}

// FIXME:
//
// The original credits used a different font. I think it's stored in the
// font.clu file, but I don't know how to interpret it.
//
// The original used the entire screen. This version cuts off the top and
// bottom of the screen, because that's where the menus would usually be.
//
// The original had some sort of smoke effect at the bottom of the screen.

enum {
	LINE_LEFT,
	LINE_CENTER,
	LINE_RIGHT
};

struct CreditsLine {
	char *str;
	byte type;
	int top;
	int height;
	byte *sprite;
};

#define CREDITS_FONT_HEIGHT 25
#define CREDITS_LINE_SPACING 20

int32 Logic::fnPlayCredits(int32 *params) {
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();
	uint32 loopingMusicId = _vm->_sound->getLoopingMusicId();

	// This function just quits the game if this is the playable demo, ie.
	// credits are NOT played in the demo any more!

	// params:	none

	if (_scriptVars[DEMO]) {
		_vm->closeGame();
		return IR_STOP;
	}

	// Prepare for the credits by fading down, stoping the music, etc.

	_vm->_mouse->setMouse(0);

	_vm->_sound->muteFx(true);
	_vm->_sound->muteSpeech(true);

	_vm->_screen->waitForFade();
	_vm->_screen->fadeDown();
	_vm->_screen->waitForFade();

	_vm->_mouse->closeMenuImmediately();

	// There are three files which I believe are involved in showing the
	// credits:
	//
	// credits.bmp  - The "Smacker" logo, stored as follows:
	//
	//     width     2 bytes, little endian
	//     height    2 bytes, little endian
	//     palette   3 * 256 bytes
	//     data      width * height bytes
	//
	//     Note that the maximum colour component in the palette is 0x3F.
	//     This is the same resolution as the _paletteMatch table. I doubt
	//     that this is a coincidence, but let's use the image palette
	//     directly anyway, just to be safe.
	//
	// credits.clu  - The credits text
	//
	//     This is simply a text file with CRLF line endings.
	//     '^' is not shown, but used to mark the center of the line.
	//     '@' is used as a placeholder for the "Smacker" logo. At least
	//     when it appears alone.
	//     Remaining lines are centered.
	//
	// fonts.clu    - The credits font?
	//
	//     FIXME: At this time I don't know how to interpret fonts.clu. For
	//     now, let's just the standard speech font instead.

	SpriteInfo spriteInfo;
	File f;
	int i;

	// Read the "Smacker" logo

	uint16 logoWidth = 0;
	uint16 logoHeight = 0;
	byte *logoData = NULL;
	byte palette[256 * 4];

	if (f.open("credits.bmp")) {
		logoWidth = f.readUint16LE();
		logoHeight = f.readUint16LE();

		for (i = 0; i < 256; i++) {
			palette[i * 4 + 0] = f.readByte() << 2;
			palette[i * 4 + 1] = f.readByte() << 2;
			palette[i * 4 + 2] = f.readByte() << 2;
			palette[i * 4 + 3] = 0;
		}

		logoData = (byte *) malloc(logoWidth * logoHeight);

		f.read(logoData, logoWidth * logoHeight);
		f.close();
	} else {
		warning("Can't find credits.bmp");
		memset(palette, 0, sizeof(palette));
		palette[14 * 4 + 0] = 252;
		palette[14 * 4 + 1] = 252;
		palette[14 * 4 + 2] = 252;
		palette[14 * 4 + 3] = 0;
	}

	_vm->_screen->setPalette(0, 256, palette, RDPAL_INSTANT);

	// Read the credits text

	// This should be plenty
	CreditsLine creditsLines[350];

	for (i = 0; i < ARRAYSIZE(creditsLines); i++) {
		creditsLines[i].str = NULL;
		creditsLines[i].sprite = NULL;
	}

	if (!f.open("credits.clu")) {
		warning("Can't find credits.clu");
		return IR_CONT;
	}

	int lineTop = 400;
	int lineCount = 0;
	int paragraphStart = 0;
	bool hasCenterMark = false;

	while (1) {
		if (lineCount >= ARRAYSIZE(creditsLines)) {
			warning("Too many credits lines");
			break;
		}

		char buffer[80];
		char *line = f.gets(buffer, sizeof(buffer));

		if (!line || *line == 0) {
			if (!hasCenterMark) {
				for (i = paragraphStart; i < lineCount; i++)
					creditsLines[i].type = LINE_CENTER;
			}
			paragraphStart = lineCount;
			hasCenterMark = false;
			if (paragraphStart == lineCount)
				lineTop += CREDITS_LINE_SPACING;

			if (!line)
				break;

			continue;
		}

		char *center_mark = strchr(line, '^');

		if (center_mark) {
			// The current paragraph has at least one center mark.
			hasCenterMark = true;

			if (center_mark != line) {
				// The center mark is somewhere inside the
				// line. Split it into left and right side.
				*center_mark = 0;

				creditsLines[lineCount].top = lineTop;
				creditsLines[lineCount].height = CREDITS_FONT_HEIGHT;
				creditsLines[lineCount].type = LINE_LEFT;
				creditsLines[lineCount].str = strdup(line);

				lineCount++;

				if (lineCount >= ARRAYSIZE(creditsLines)) {
					warning("Too many credits lines");
					break;
				}

				*center_mark = '^';
			}

			line = center_mark;
		}

		creditsLines[lineCount].top = lineTop;

		if (*line == '^') {
			creditsLines[lineCount].type = LINE_RIGHT;
			line++;
		} else
			creditsLines[lineCount].type = LINE_LEFT;

		if (strcmp(line, "@") == 0) {
			creditsLines[lineCount].height = logoHeight;
			lineTop += logoHeight;
		} else {
			creditsLines[lineCount].height = CREDITS_FONT_HEIGHT;
			lineTop += CREDITS_LINE_SPACING;
		}

		creditsLines[lineCount].str = strdup(line);
		lineCount++;
	}

	f.close();

	// We could easily add some ScummVM stuff to the credits, if we wanted
	// to. On the other hand, anyone with the attention span to actually
	// read all the credits probably already knows. :-)

	// Start the music and roll the credits

	// The credits music (which can also be heard briefly in the "carib"
	// cutscene) is played once.

	int32 pars[2];

	pars[0] = 309;
	pars[1] = FX_SPOT;
	fnPlayMusic(pars);

	_vm->_screen->clearScene();
	_vm->_screen->fadeUp(0);

	spriteInfo.scale = 0;
	spriteInfo.scaledWidth = 0;
	spriteInfo.scaledHeight = 0;
	spriteInfo.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION | RDSPR_TRANS;
	spriteInfo.blend = 0;

	int startLine = 0;
	int scrollPos = 0;

	bool abortCredits = false;

	int scrollSteps = lineTop + CREDITS_FONT_HEIGHT;
	uint32 musicStart = _vm->getMillis();

	// Ideally the music should last just a tiny bit longer than the
	// credits. Note that musicTimeRemaining() will return 0 if the music
	// is muted, so we need a sensible fallback for that case.

	uint32 musicLength = MAX((int32) (1000 * (_vm->_sound->musicTimeRemaining() - 3)), 25 * (int32) scrollSteps);

	while (scrollPos < scrollSteps && !_vm->_quit) {
		bool foundStartLine = false;

		_vm->_screen->clearScene();

		for (i = startLine; i < lineCount; i++) {
			// Free any sprites that have scrolled off the screen

			if (creditsLines[i].top + creditsLines[i].height < scrollPos) {
				if (creditsLines[i].sprite) {
					free(creditsLines[i].sprite);
					creditsLines[i].sprite = NULL;
					debug(2, "Freeing sprite '%s'", creditsLines[i].str);
				}
				if (creditsLines[i].str) {
					free(creditsLines[i].str);
					creditsLines[i].str = NULL;
				}
			} else if (creditsLines[i].top < scrollPos + 400) {
				if (!foundStartLine) {
					startLine = i;
					foundStartLine = true;
				}

				if (!creditsLines[i].sprite) {
					debug(2, "Creating sprite '%s'", creditsLines[i].str);
					creditsLines[i].sprite = _vm->_fontRenderer->makeTextSprite((byte *) creditsLines[i].str, 600, 14, _vm->_speechFontId, 0);
				}

				FrameHeader *frame = (FrameHeader *) creditsLines[i].sprite;

				spriteInfo.y = creditsLines[i].top - scrollPos;
				spriteInfo.w = frame->width;
				spriteInfo.h = frame->height;
				spriteInfo.data = creditsLines[i].sprite + sizeof(FrameHeader);

				switch (creditsLines[i].type) {
				case LINE_LEFT:
					spriteInfo.x = RENDERWIDE / 2 - 5 - frame->width;
					break;
				case LINE_RIGHT:
					spriteInfo.x = RENDERWIDE / 2 + 5;
					break;
				case LINE_CENTER:
					if (strcmp(creditsLines[i].str, "@") == 0) {
						spriteInfo.data = logoData;
						spriteInfo.x = (RENDERWIDE - logoWidth) / 2;
						spriteInfo.w = logoWidth;
						spriteInfo.h = logoHeight;
					} else
						spriteInfo.x = (RENDERWIDE - frame->width) / 2;
					break;
				}

				if (spriteInfo.data)
					_vm->_screen->drawSprite(&spriteInfo);
			} else
				break;
		}

		_vm->_screen->updateDisplay();

		KeyboardEvent *ke = _vm->keyboardEvent();

		if (ke && ke->keycode == 27) {
			if (!abortCredits) {
				abortCredits = true;
				_vm->_screen->fadeDown();
			}
		}

		if (abortCredits && _vm->_screen->getFadeStatus() == RDFADE_BLACK)
			break;

		_vm->sleepUntil(musicStart + (musicLength * scrollPos) / scrollSteps);
		scrollPos++;
	}

	// We're done. Clean up and try to put everything back where it was
	// before the credits.

	for (i = 0; i < lineCount; i++) {
		if (creditsLines[i].str)
			free(creditsLines[i].str);
		if (creditsLines[i].sprite)
			free(creditsLines[i].sprite);
	}

	if (logoData)
		free(logoData);

	if (!abortCredits) {
		// The music should either have stopped or be about to stop, so
		// wait for it to really happen.

		while (_vm->_sound->musicTimeRemaining() && !_vm->_quit) {
			_vm->_screen->updateDisplay(false);
			_vm->_system->delayMillis(100);
		}
	}

	if (_vm->_quit)
		return IR_CONT;

	_vm->_sound->muteFx(false);
	_vm->_sound->muteSpeech(false);

	if (loopingMusicId) {
		pars[0] = loopingMusicId;
		pars[1] = FX_LOOP;
		fnPlayMusic(pars);
	} else
		fnStopMusic(NULL);

	screenInfo->new_palette = 99;

	if (!_vm->_mouse->getMouseStatus() || _choosing)
		_vm->_mouse->setMouse(NORMAL_MOUSE_ID);

	if (_scriptVars[DEAD])
		_vm->_mouse->buildSystemMenu();

	return IR_CONT;
}

int32 Logic::fnSetScrollSpeedNormal(int32 *params) {
	// params:	none

	_vm->_screen->setScrollFraction(16);
	return IR_CONT;
}

int32 Logic::fnSetScrollSpeedSlow(int32 *params) {
	// params:	none

	_vm->_screen->setScrollFraction(32);
	return IR_CONT;
}

// Called from speech scripts to remove the chooser bar when it's not
// appropriate to keep it displayed

int32 Logic::fnRemoveChooser(int32 *params) {
	// params:	none

	_vm->_mouse->hideMenu(RDMENU_BOTTOM);
	return IR_CONT;
}

/**
 * Alter the volume and pan of a currently playing FX
 */

int32 Logic::fnSetFxVolAndPan(int32 *params) {
	// params:	0 id of fx (ie. the id returned in 'result' from
	//		  fnPlayFx
	//		1 new volume (0..16)
	//		2 new pan (-16..16)

	debug(5, "fnSetFxVolAndPan(%d, %d, %d)", params[0], params[1], params[2]);

	_vm->_sound->setFxIdVolumePan(params[0], params[1], params[2]);
	return IR_CONT;
}

/**
 * Alter the volume of a currently playing FX
 */

int32 Logic::fnSetFxVol(int32 *params) {
	// params:	0 id of fx (ie. the id returned in 'result' from
	//		  fnPlayFx
	//		1 new volume (0..16)

	_vm->_sound->setFxIdVolumePan(params[0], params[1]);
	return IR_CONT;
}

int32 Logic::fnRestoreGame(int32 *params) {
	// params:	none
	return IR_CONT;
}

int32 Logic::fnRefreshInventory(int32 *params) {
	// Called from 'menu_look_or_combine' script in 'menu_master' object
	// to update the menu to display a combined object while George runs
	// voice-over. Note that 'object_held' must be set to the graphic of
	// the combined object

	// params:	none

	_vm->_mouse->refreshInventory();
	return IR_CONT;
}

int32 Logic::fnChangeShadows(int32 *params) {
	// params:	none
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	// if last screen was using a shading mask (see below)
	if (screenInfo->mask_flag) {
		uint32 rv = _vm->_screen->closeLightMask();
		if (rv)
			error("Driver Error %.8x", rv);
		screenInfo->mask_flag = false;
	}

	return IR_CONT;
}

} // End of namespace Sword2
