/* Copyright (C) 1994-2004 Revolution Software Ltd
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
#include "sword2/sword2.h"
#include "sword2/console.h"
#include "sword2/controls.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"
#include "sword2/driver/d_draw.h"
#include "sword2/driver/d_sound.h"

namespace Sword2 {

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
			_vm->_graphics->setMenuIcon(RDMENU_BOTTOM, i, icon);
			_vm->_resman->closeResource(_subjectList[i].res);
		}

		for (; i < 15; i++)
			_vm->_graphics->setMenuIcon(RDMENU_BOTTOM, (uint8) i, NULL);

		_vm->_graphics->showMenu(RDMENU_BOTTOM);
		_vm->setMouse(NORMAL_MOUSE_ID);
		_choosing = true;
		return IR_REPEAT;
	}

	// The menu is there - we're just waiting for a click. We only care
	// about left clicks.

	MouseEvent *me = _vm->mouseEvent();

	if (!me || !(me->buttons & RD_LEFTBUTTONDOWN) || _vm->_mouseY < 400)
		return IR_REPEAT;

	// Check for click on a menu.

	int hit = _vm->menuClick(_scriptVars[IN_SUBJECT]);
	if (hit < 0)
		return IR_REPEAT;

	// Hilight the clicked icon by greying the others.

	for (i = 0; i < _scriptVars[IN_SUBJECT]; i++) {
		if ((int) i != hit) {
			icon = _vm->_resman->openResource(_subjectList[i].res) + sizeof(StandardHeader);
			_vm->_graphics->setMenuIcon(RDMENU_BOTTOM, i, icon);
			_vm->_resman->closeResource(_subjectList[i].res);
		}
	}

	// For non-speech scripts that manually call the chooser
	_scriptVars[RESULT] = _subjectList[hit].res;

	// The conversation menu will be closed

	_choosing = false;
	_scriptVars[IN_SUBJECT] = 0;
	_vm->setMouse(0);

	return IR_CONT | (_subjectList[hit].ref << 3);
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

	if (_scriptVars[TALK_FLAG] == 0) {
		// See fnChooser & speech scripts
		_scriptVars[CHOOSER_COUNT_FLAG] = 0;
	}

	fnNoHuman(params);
	return IR_CONT;
}

/**
 * End a conversation.
 */

int32 Logic::fnEndConversation(int32 *params) {
	// params:	none

	_vm->_graphics->hideMenu(RDMENU_BOTTOM);

	if (_vm->_mouseY > 399) {
		// Will wait for cursor to move off the bottom menu
		_vm->_mouseMode = MOUSE_holding;
	}

	// In case DC forgets
	_scriptVars[TALK_FLAG] = 0;

	return IR_CONT;
}

// To request the status of a target, we run its 4th script, get-speech-state.
// This will cause RESULT to be set to either 1 (target is waiting) or 0
// (target is busy).

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

/**
 * Receive and sequence the commands sent from the conversation script. We have
 * to do this in a slightly tweeky manner as we can no longer have generic
 * scripts.
 */

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
			fnBackSprite(pars);		// ob_graphic

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

		if (!_vm->_gui->_subtitles && !wantSpeechForLine(params[S_WAV]))
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

			char speechFile[20];

			sprintf(speechFile, "speech%d.clu", _vm->_resman->whichCd());

			File fp;

			if (fp.open(speechFile))
				fp.close();
			else
				strcpy(speechFile, "speech.clu");

			// Load speech but don't start playing yet
			uint32 rv = _vm->_sound->playCompSpeech(speechFile, params[S_WAV], 16, speech_pan);

			if (rv == RD_OK) {
				// Ok, we've got something to play. Set it
				// playing now. (We might want to do this the
				// next cycle, don't know yet.)

				speechRunning = true;
				_vm->_sound->unpauseSpeech();
			} else {
				debug(5, "ERROR: PlayCompSpeech(speechFile=\"%s\", wav=%d (res=%d pos=%d)) returned %.8x", speechFile, params[S_WAV], text_res, local_text, rv);
			}
		}

		if (_vm->_gui->_subtitles || !speechRunning) {
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

	// So that we can go to the options panel while text & speech is
	// being tested
	if (_scriptVars[SYSTEM_TESTING_TEXT] == 0 || _vm->_mouseY > 0) {
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

// Distance kept above talking sprite
#define GAP_ABOVE_HEAD 20

/**
 * Sets _textX and _textY for position of text sprite. Note that _textX is
 * also used to calculate speech pan.
 */

void Logic::locateTalker(int32 *params) {
	// params:	0 pointer to ob_graphic
	//		1 pointer to ob_speech
	//		2 pointer to ob_logic
	//		3 pointer to ob_mega
	//		4 encoded text number
	//		5 wav res id
	//		6 anim res id
	//		7 pointer to anim table
	//		8 animation mode	0 lip synced,
	//					1 just straight animation

	if (!_animId) {
		// There is no animation. Assume it's voice-over text, and put
		// it at the bottom of the screen.

		_textX = 320;
		_textY = 400;
		return;
	}

	byte *file = _vm->_resman->openResource(_animId);

	// '0' means 1st frame

	CdtEntry *cdt_entry = _vm->fetchCdtEntry(file, 0);
	FrameHeader *frame_head = _vm->fetchFrameHeader(file, 0);

	// Note: This part of the code is quite similar to registerFrame().

	if (cdt_entry->frameType & FRAME_OFFSET) {
		// The frame has offsets, i.e. it's a scalable mega frame
		ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[S_OB_MEGA]);

		// Calculate scale at which to print the sprite, based on feet
		// y-coord and scaling constants (NB. 'scale' is actually
		// 256 * true_scale, to maintain accuracy)

		// Ay+B gives 256 * scale ie. 256 * 256 * true_scale for even
		// better accuracy, ie. scale = (Ay + B) / 256

		uint16 scale = (uint16) ((ob_mega->scale_a * ob_mega->feet_y + ob_mega->scale_b) / 256);

		// Calc suitable centre point above the head, based on scaled
		// height

		// just use 'feet_x' as centre
		_textX = (int16) ob_mega->feet_x;

		// Add scaled y-offset to feet_y coord to get top of sprite
		_textY = (int16) (ob_mega->feet_y + (cdt_entry->y * scale) / 256);
	} else {
		// It's a non-scaling anim - calc suitable centre point above
		// the head, based on scaled width

		// x-coord + half of width
		_textX = cdt_entry->x + (frame_head->width) / 2;
		_textY = cdt_entry->y;
	}

	_vm->_resman->closeResource(_animId);

	// Leave space above their head
	_textY -= GAP_ABOVE_HEAD;
			
	// Adjust the text coords for RDSPR_DISPLAYALIGN

	_textX -= _vm->_thisScreen.scroll_offset_x;
	_textY -= _vm->_thisScreen.scroll_offset_y;
}

/**
 * This function is called the first time to build the text, if we need one. If
 * If necessary it also brings in the wav and sets up the animation.
 *
 * If there is an animation it can be repeating lip-sync or run-once.
 *
 * If there is no wav, then the text comes up instead. There can be any
 * combination of text/wav playing.
 */

void Logic::formText(int32 *params) {
	// params	0 pointer to ob_graphic
	// 		1 pointer to ob_speech
	//		2 pointer to ob_logic
	//		3 pointer to ob_mega
	//		4 encoded text number
	//		5 wav res id
	//		6 anim res id
	//		7 pointer to anim table
	//		8 animation mode	0 lip synced,
	//					1 just straight animation

	// There should always be a text line, as all text is derived from it.
	// If there is none, that's bad...

	if (!params[S_TEXT]) {
		warning("No text line for speech wav %d", params[S_WAV]);
		return;
	}

	ObjectSpeech *ob_speech = (ObjectSpeech *) _vm->_memory->decodePtr(params[S_OB_SPEECH]);

	// Establish the max width allowed for this text sprite.
	uint32 textWidth = ob_speech->width ? ob_speech->width : 400;

	// Pull out the text line, and make the sprite and text block

	uint32 text_res = params[S_TEXT] / SIZE;
	uint32 local_text = params[S_TEXT] & 0xffff;
	byte *text = _vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text);

	// 'text + 2' to skip the first 2 bytes which form the line reference
	// number

	_speechTextBlocNo = _vm->_fontRenderer->buildNewBloc(
		text + 2, _textX, _textY,
		textWidth, ob_speech->pen,
		RDSPR_TRANS | RDSPR_DISPLAYALIGN,
		_vm->_speechFontId, POSITION_AT_CENTRE_OF_BASE);

	_vm->_resman->closeResource(text_res);

	// Set speech duration, in case not using a wav.
	_speechTime = strlen((char *) text) + 30;
}

/**
 * There are some hard-coded cases where speech is used to illustrate a sound
 * effect. In this case there is no sound associated with the speech itself.
 */

bool Logic::wantSpeechForLine(uint32 wavId) {
	switch (wavId) {
	case 1328:	// AttendantSpeech
			//	SFX(Phone71);
			//	FX <Telephone rings>
	case 2059:	// PabloSpeech
			//	SFX (2059);
			//	FX <Sound of sporadic gunfire from below>
	case 4082:	// DuaneSpeech
			//	SFX (4082);
			//	FX <Pffffffffffft! Frp. (Unimpressive, flatulent noise.)>
	case 4214:	// cat_52
			//	SFX (4214);
			//	4214FXMeow!
	case 4568:	// trapdoor_13
 			//	SFX (4568);
			//	4568fx<door slamming>
	case 4913:	// LobineauSpeech
			//	SFX (tone2);
			//	FX <Lobineau hangs up>
	case 5120:	// bush_66
			//	SFX (5120);
			//	5120FX<loud buzzing>
	case 528:	// PresidentaSpeech
			//	SFX (528);
			//	FX <Nearby Crash of Collapsing Masonry>
	case 920:	// Zombie Island forest maze (bird)
	case 923:	// Zombie Island forest maze (monkey)
	case 926:	// Zombie Island forest maze (zombie)
		// Don't want speech for these lines!
		return false;
	default:
		// Ok for all other lines
		return true;
	}
}

} // End of namespace Sword2
