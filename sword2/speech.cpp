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
#include "sword2/anims.h"
#include "sword2/console.h"
#include "sword2/controls.h"	// for 'subtitles' & 'speechSelected'
#include "sword2/debug.h"
#include "sword2/defs.h"
#include "sword2/events.h"
#include "sword2/function.h"
#include "sword2/interpreter.h"
#include "sword2/layers.h"		// for 'this_screen'
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/mouse.h"
#include "sword2/object.h"
#include "sword2/protocol.h"
#include "sword2/resman.h"
#include "sword2/sound.h"
#include "sword2/speech.h"
#include "sword2/sword2.h"
#include "sword2/walker.h"

namespace Sword2 {

#define	INS_talk		1
#define	INS_anim		2
#define	INS_reverse_anim	3
#define	INS_walk		4
#define	INS_turn		5
#define	INS_face		6
#define	INS_trace		7
#define	INS_no_sprite		8
#define	INS_sort		9
#define	INS_foreground		10
#define	INS_background		11
#define	INS_table_anim		12
#define	INS_reverse_table_anim	13
#define	INS_walk_to_anim	14
#define	INS_set_frame		15
#define	INS_stand_after_anim	16

#define	INS_quit		42

// when not playing a wav we calculate the speech time based upon length of
// ascii

uint32 speech_time = 0;

uint32 speech_text_bloc_no = 0;
uint32 anim_id = 0;

// 0 lip synced and repeating - 1 normal once through
uint32 speech_anim_type;

uint32 left_click_delay = 0;	// click-delay for LEFT mouse button
uint32 right_click_delay = 0;	// click-delay for RIGHT mouse button

// ref number for default response when luggage icon is used on a person
// & it doesn't match any of the icons which would have been in the chooser
uint32 default_response_id = 0;

// "TEXT" - current official text line number - will match the wav filenames
int16 officialTextNumber = 0;

// usually 0; if non-zero then it's the id of whoever we're waiting for in a
// speech script see fnTheyDo, fnTheyDoWeWait and fnWeWait
int32 speechScriptWaiting = 0;

// calculated by LocateTalker() for use in speech-panning & text-sprite
// positioning
int16 text_x, text_y;

_subject_unit subject_list[MAX_SUBJECT_LIST];

void LocateTalker(int32	*params);
void Form_text(int32 *params);
uint8 WantSpeechForLine(uint32 wavId);

#ifdef _SWORD2_DEBUG
void GetCorrectCdForSpeech(int32 wavId);	// for testing speech & text
#endif

int32 Logic::fnAddSubject(int32 *params) {
	// params:	0 id
	//		1 daves reference number

	if (IN_SUBJECT == 0) {
		// This is the start of the new subject list
		// Set the default repsonse id to zero in case we're never
		// passed one
		default_response_id = 0;
	}

	// - this just means we'd get the response for the 1st icon in the
	// chooser which is better than crashing

	if (params[0] == -1)
	{
		// this isn't an icon at all, it's telling us the id of the
		// default response

		// and here it is - this is the ref number we will return if
		default_response_id = params[1];

		// a luggage icon is clicked on someone when it wouldn't have
		// been in the chooser list (see fnChoose below)
	} else {
		subject_list[IN_SUBJECT].res = params[0];
		subject_list[IN_SUBJECT].ref = params[1];

		debug(5, "fnAddSubject res %d, uid %d", params[0], params[1]);

		IN_SUBJECT++;
	}

	return IR_CONT;
}

// could alternately use logic->looping of course
int choosing = 0;

int32 Logic::fnChoose(int32 *params) {
	// params:	none

	// the human is switched off so there will be no normal mouse engine

	_mouseEvent *me;
	uint32 j, hit;
	uint8 *icon;
	uint32 pos = 0;

	AUTO_SELECTED = 0;	// see below

	// new thing to intercept objects held at time of clicking on a person

	if (OBJECT_HELD) {
		// If we are using a luggage icon on the person, scan the
		// subject list to see if this icon would have been available
		// at this time.
		//
 		// If it is there, return the relevant 'ref' number (as if it
		// had been selected from within the conversation). If not,
		// just return a special code to get the default text line(s)
		// for unsupported objects.
		//
		// Note that we won't display the subject icons in this case!

		// scan the subject list for a match with our 'object_held'
		while (pos < IN_SUBJECT) {
			if (subject_list[pos].res == OBJECT_HELD) {
				// if we've found a match, clear it so it
				// doesn't keep happening!
				OBJECT_HELD = 0;

				// clear the subject list
				IN_SUBJECT = 0;

				// return special subject chosen code (same
				// as in normal chooser routine below)
				return IR_CONT + (subject_list[pos].ref << 3);
			}
			pos++;
		}

		OBJECT_HELD = 0; // clear it so it doesn't keep happening!
		IN_SUBJECT = 0;	 // clear the subject list

		// so that the speech script uses the default text for
		// objects that are not accounted for
		return IR_CONT + (default_response_id << 3);
	}

	// new thing for skipping chooser with "nothing else to say" text

	// If this is the 1st time the chooser is coming up in this
	// conversation, AND there's only 1 subject, AND it's the EXIT icon

	if (CHOOSER_COUNT_FLAG == 0 && IN_SUBJECT == 1 && subject_list[0].res == EXIT_ICON) {
		AUTO_SELECTED = 1;	// for speech script
		IN_SUBJECT = 0;		// clear the subject list

		// return special subject chosen code (same as in normal
		// chooser routine below)
		return IR_CONT + (subject_list[0].ref << 3);
	}

	if (!choosing) {
		// new choose session
		// build menus from subject_list

		if (!IN_SUBJECT)
			error("fnChoose with no subjects :-O");

		// init top menu from master list
		// all icons are highlighted / full colour

		for (j = 0; j < 15; j++) {
			if (j < IN_SUBJECT) {
				debug(5, " ICON res %d for %d", subject_list[j].res, j);
				icon = res_man.open(subject_list[j].res) + sizeof(_standardHeader) + RDMENU_ICONWIDE * RDMENU_ICONDEEP;
				g_display->setMenuIcon(RDMENU_BOTTOM, (uint8) j, icon);
				res_man.close(subject_list[j].res);
			} else {
				//no icon here
				debug(5, " NULL for %d", j);
				g_display->setMenuIcon(RDMENU_BOTTOM, (uint8) j, NULL);
			}
		}

		// start menus appearing
		g_display->showMenu(RDMENU_BOTTOM);

		// lets have the mouse pointer back
		Set_mouse(NORMAL_MOUSE_ID);

		choosing = 1;

		// again next cycle
		return IR_REPEAT;
	} else {
		// menu is there - we're just waiting for a click
		debug(5, "choosing");

		me = MouseEvent();

		// we only care about left clicks
		// we ignore mouse releases

		if (me && (me->buttons & RD_LEFTBUTTONDOWN)) {
			// check for click on a menu
			// if so then end the choose, highlight only the
			// chosen, blank the mouse and return the ref code * 8

			if (g_display->_mouseY > 399 && g_display->_mouseX >= 24 && g_display->_mouseX < 640 - 24) {
				//which are we over?
				hit = (g_display->_mouseX - 24) / 40;

				//clicked on something - what button?
				if (hit < IN_SUBJECT) {
					debug(5, "Icons available:");

					// change icons
					for (j = 0; j < IN_SUBJECT; j++) {
						debug(5, "%s", FetchObjectName(subject_list[j].res));

						// change all others to grey
						if (j != hit) {
							icon = res_man.open( subject_list[j].res ) + sizeof(_standardHeader);
							g_display->setMenuIcon(RDMENU_BOTTOM, (uint8) j, icon);
							res_man.close(subject_list[j].res);
						}
					}


					debug(5, "Selected: %s", FetchObjectName(subject_list[hit].res));

					// this is our looping flag
					choosing = 0;

					IN_SUBJECT = 0;

					// blank mouse again
					Set_mouse(0);

					debug(5, "hit %d - ref %d  ref*8 %d", hit, subject_list[hit].ref, subject_list[hit].ref * 8);

					// for non-speech scripts that manually
					// call the chooser
					RESULT = subject_list[hit].res;

					// return special subject chosen code
					return IR_CONT + (subject_list[hit].ref << 3);
				}
			}
		}

		debug(5, "end choose");

		// again next cycle
		return IR_REPEAT;
	}
}

int32 Logic::fnStartConversation(int32 *params) {
	// Start conversation

	// reset 'chooser_count_flag' at the start of each conversation:

	// Note that fnStartConversation might accidently be called every time
	// the script loops back for another chooser but we only want to reset
	// the chooser count flag the first time this function is called ie.
	// when talk flag is zero

	// params:	none

	if (TALK_FLAG == 0)
		CHOOSER_COUNT_FLAG = 0;	// see fnChooser & speech scripts

	fnNoHuman(params);
	return IR_CONT;
}

int32 Logic::fnEndConversation(int32 *params) {
	// end conversation

	// params:	none

	g_display->hideMenu(RDMENU_BOTTOM);

	if (g_display->_mouseY > 399) {
		// will wait for cursor to move off the bottom menu
		mouse_mode = MOUSE_holding;
		debug(5, "   holding");
	}

	TALK_FLAG = 0;	// in-case DC forgets

	// restart george's base script
	// LLogic.totalRestart();

	//drop out without saving pc and go around again
	return IR_CONT;
}

int32 Logic::fnTheyDo(int32 *params) {
	// doesn't send the command until target is waiting - once sent we
	// carry on

	// params:	0 target
	//		1 command
	//		2 ins1
	//		3 ins2
	//		4 ins3
	//		5 ins4
	//		6 ins5

	uint32 null_pc = 5;		// 4th script - get-speech-state
	char *raw_script_ad;
	_standardHeader	*head;
	int32 target = params[0];

	// request status of target
	head = (_standardHeader*) res_man.open(target);
	if (head->fileType != GAME_OBJECT)
		error("fnTheyDo %d not an object", target);

	raw_script_ad = (char *) head;

	// call the base script - this is the graphic/mouse service call
	runScript(raw_script_ad, raw_script_ad, &null_pc);

	res_man.close(target);

	// result is 1 for waiting, 0 for busy

	if (RESULT == 1 && !INS_COMMAND) {
		// its waiting and no other command is queueing
		// reset debug flag now that we're no longer waiting - see
		// debug.cpp

		speechScriptWaiting = 0;

		SPEECH_ID = params[0];
		INS_COMMAND = params[1];
		INS1 = params[2];
		INS2 = params[3];
		INS3 = params[4];
		INS4 = params[5];
		INS5 = params[6];

		return IR_CONT;
	}

	// debug flag to indicate who we're waiting for - see debug.cpp
	speechScriptWaiting = target;

	// target is busy so come back again next cycle
	return IR_REPEAT;
}

int32 Logic::fnTheyDoWeWait(int32 *params) {
	// give target a command and wait for it to register as finished

	// params:	0 pointer to ob_logic
	//		1 target
	//		2 command
	//		3 ins1
	//		4 ins2
	//		5 ins3
	//		6 ins4
	//		7 ins5

	// 'looping' flag is used as a sent command yes/no

	Object_logic *ob_logic;

	uint32 null_pc = 5;		// 4th script - get-speech-state
	char *raw_script_ad;
	_standardHeader	*head;
	int32 target = params[1];

	// ok, see if the target is busy - we must request this info from the
	// target object

	head = (_standardHeader*) res_man.open(target);
	if (head->fileType != GAME_OBJECT)
		error("fnTheyDoWeWait %d not an object", target);

	raw_script_ad = (char *) head;

	// call the base script - this is the graphic/mouse service call
	runScript(raw_script_ad, raw_script_ad, &null_pc);

	res_man.close(target);

	ob_logic = (Object_logic *) params[0];

	if (!INS_COMMAND && RESULT == 1 && ob_logic->looping == 0) {
		// first time so set up targets command if target is waiting

		debug(5, "FNtdww sending command to %d", target);

		SPEECH_ID = params[1];
		INS_COMMAND = params[2];
		INS1 = params[3];
		INS2 = params[4];
		INS3 = params[5];
		INS4 = params[6];
		INS5 = params[7];

		ob_logic->looping = 1;

		// debug flag to indicate who we're waiting for - see debug.cpp
		speechScriptWaiting = target;

		// finish this cycle - but come back again to check for it
		// being finished
		return IR_REPEAT;
	} else if (ob_logic->looping == 0) {
		// did not send the command
		// debug flag to indicate who we're waiting for - see debug.cpp
		speechScriptWaiting = target;

		// come back next go and try again to send the instruction
		return IR_REPEAT;
	}

	// ok, the command has been sent - has the target actually done it yet?

	// result is 1 for waiting, 0 for busy

	if (RESULT == 1) {
		// its waiting now so we can be finished with all this
		debug(5, "FNtdww finished");

		// not looping anymore
		ob_logic->looping = 0;

		// reset debug flag now that we're no longer waiting - see
		// debug.cpp
		speechScriptWaiting = 0;
		return IR_CONT;
	}

	debug(5, "FNtdww just waiting");

	// debug flag to indicate who we're waiting for - see debug.cpp
	speechScriptWaiting = target;

	// see ya next cycle
	return IR_REPEAT;
}

int32 Logic::fnWeWait(int32 *params) {
	// loop until the target is free

	// params:	0 target

	uint32 null_pc = 5;		// 4th script - get-speech-state
	char *raw_script_ad;
	_standardHeader	*head;
	int32 target = params[0];

	// request status of target
	head = (_standardHeader*) res_man.open(target);
	if (head->fileType != GAME_OBJECT)
		error("fnWeWait: %d not an object", target);

	raw_script_ad = (char *) head;

	// call the base script - this is the graphic/mouse service call
	runScript(raw_script_ad, raw_script_ad, &null_pc);

	res_man.close(target);

	// result is 1 for waiting, 0 for busy

	if (RESULT == 1) {
		// reset debug flag now that we're no longer waiting - see
		// debug.cpp
		speechScriptWaiting = 0;
		return IR_CONT;
	}

	// debug flag to indicate who we're waiting for - see debug.cpp
	speechScriptWaiting = target;

	// target is busy so come back again next cycle
	return IR_REPEAT;
}

int32 Logic::fnTimedWait(int32 *params) {
	// loop until the target is free but only while the timer is high
	// useful when clicking on a target to talk to them - if they never
	// reply then this'll fall out avoiding a lock up

	// params:	0 ob_logic
	//		1 target
	//		2 number of cycles before give up

	uint32 null_pc = 5;		// 4th script - get-speech-state
	char *raw_script_ad;
	Object_logic *ob_logic;
	_standardHeader	*head;
	int32 target = params[1];

	ob_logic = (Object_logic *) params[0];

	if (!ob_logic->looping)
		ob_logic->looping = params[2];	//first time in

	// request status of target
	head = (_standardHeader*) res_man.open(target);
	if (head->fileType != GAME_OBJECT)
		error("fnTimedWait %d not an object", target);

	raw_script_ad = (char *) head;

	// call the base script - this is the graphic/mouse service call
	runScript(raw_script_ad, raw_script_ad, &null_pc);

	res_man.close(target);

	// result is 1 for waiting, 0 for busy

	if (RESULT == 1) {
		// reset because counter is likely to be still high
		ob_logic->looping = 0;

		//means ok
		RESULT = 0;

		// reset debug flag now that we're no longer waiting - see
		// debug.cpp
		speechScriptWaiting = 0;

		return IR_CONT;
	}

	ob_logic->looping--;

	if (!ob_logic->looping) {	// time up - caller must check RESULT
		// not ok
		RESULT = 1;

		//clear the event that hasn't been picked up - in theory,
		// none of this should ever happen
		Kill_all_ids_events(target);

		debug(5, "EVENT timed out");

		// reset debug flag now that we're no longer waiting - see
		// debug.cpp
		speechScriptWaiting = 0;

		return IR_CONT;
	}

	// debug flag to indicate who we're waiting for - see debug.cpp
	speechScriptWaiting = target;

	// target is busy so come back again next cycle
	return IR_REPEAT;
}

int32 Logic::fnSpeechProcess(int32 *params) {
	// Recieve and sequence the commands sent from the conversation
	// script.

	// We have to do this in a slightly tweeky manner as we can no longer
	// have generic scripts.

	// This function comes in with all the structures that will be
	// required.

	// params:	0 pointer to ob_graphic
	//		1 pointer to ob_speech
	//		2 pointer to ob_logic
	//		3 pointer to ob_mega
	//		4 pointer to ob_walkdata

	// note - we could save a var and ditch wait_state and check
	// 'command' for non zero means busy

	Object_speech *ob_speech;
	int32 pars[9];
	int32 ret;

	ob_speech = (Object_speech *) params[1];

	debug(5, "  SP");

	while(1) {
		//we are currently running a command
		switch (ob_speech->command) {
		case 0:
			// Do nothing
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

			debug(5, "speech-process talk");

			// run the function - (it thinks it's been called from
			// script - bloody fool)

			if (fnISpeak(pars) != IR_REPEAT) {
				debug(5, "speech-process talk finished");

				// command finished
				ob_speech->command = 0;

				// waiting for command
				ob_speech->wait_state = 1;
			}

			// come back again next cycle
			return IR_REPEAT;
		case INS_turn:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = ob_speech->ins1;	// direction to turn to

			if (fnTurn(pars) != IR_REPEAT) {
				// command finished
				ob_speech->command = 0;

				// waiting for command
				ob_speech->wait_state = 1;
			}

			// come back again next cycle
			return IR_REPEAT;
		case INS_face:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = ob_speech->ins1;	// target

			if (fnFaceMega(pars) != IR_REPEAT) {
				// command finished
				ob_speech->command = 0;

				// waiting for command
				ob_speech->wait_state = 1;
			}

			// come back again next cycle
			return IR_REPEAT;
		case INS_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = ob_speech->ins1;	// anim res

			if (fnAnim(pars) != IR_REPEAT) {
				// command finished
				ob_speech->command = 0;

				// waiting for command
				ob_speech->wait_state = 1;
			}

			// come back again next cycle
			return IR_REPEAT;
		case INS_reverse_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = ob_speech->ins1;	// anim res

			if (fnReverseAnim(pars) != IR_REPEAT) {
				// command finished
				ob_speech->command = 0;

				// waiting for command
				ob_speech->wait_state = 1;
			}

			// come back again next cycle
			return IR_REPEAT;
		case INS_table_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = ob_speech->ins1;	// pointer to anim table

			if (fnMegaTableAnim(pars) != IR_REPEAT) {
				// command finished
				ob_speech->command = 0;

				// waiting for command
				ob_speech->wait_state = 1;
			}

			// come back again next cycle
			return IR_REPEAT;
		case INS_reverse_table_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = ob_speech->ins1;	// pointer to anim table

			if (fnReverseMegaTableAnim(pars) != IR_REPEAT) {
				// command finished
				ob_speech->command = 0;

				// waiting for command
				ob_speech->wait_state = 1;
			}

			// come back again next cycle
			return IR_REPEAT;
		case INS_no_sprite:
			fnNoSprite(params);		// ob_graphic
			ob_speech->command = 0;		// command finished
			ob_speech->wait_state = 1;	// waiting for command
			return IR_REPEAT ;
		case INS_sort:
			fnSortSprite(params);		// ob_graphic
			ob_speech->command = 0;		// command finished
			ob_speech->wait_state = 1;	// waiting for command
			return IR_REPEAT;
		case INS_foreground:
			fnForeSprite(params);		// ob_graphic
			ob_speech->command = 0;		// command finished
			ob_speech->wait_state = 1;	// waiting for command
			return IR_REPEAT;
		case INS_background:
			fnBackSprite(params);		// ob_graphic
			ob_speech->command = 0;		// command finished
			ob_speech->wait_state = 1;	// waiting for command
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
				debug(5, "speech-process walk finished");

				// command finished
				ob_speech->command = 0;

				//waiting for command
				ob_speech->wait_state = 1;
			}

			// come back again next cycle
			return IR_REPEAT;
		case INS_walk_to_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = ob_speech->ins1;	// anim resource

			if (fnWalkToAnim(pars) != IR_REPEAT) {
				debug(5, "speech-process walk finished");

				// command finished
				ob_speech->command = 0;

				// waiting for command
				ob_speech->wait_state = 1;
			}

			// come back again next cycle
			return IR_REPEAT;
		case INS_stand_after_anim:
			pars[0] = params[0];		// ob_graphic
			pars[1] = params[3];		// ob_mega
			pars[2] = ob_speech->ins1;	// anim resource
			fnStandAfterAnim(pars);
			ob_speech->command = 0;		// command finished
			ob_speech->wait_state = 1;	// waiting for command
			return IR_REPEAT;		// come back again next cycle
		case INS_set_frame:
			pars[0] = params[0];		// ob_graphic
			pars[1] = ob_speech->ins1;	// anim_resource
			pars[2] = ob_speech->ins2;	// FIRST_FRAME or LAST_FRAME
			ret = fnSetFrame(pars);
			ob_speech->command = 0;		// command finished
			ob_speech->wait_state = 1;	// waiting for command
			return IR_REPEAT;		// come back again next cycle
		case INS_quit:
			debug(5, "speech-process - quit");

			ob_speech->command = 0;		// finish with all this
			// ob_speech->wait_state = 0;	// start with waiting for command next conversation
			return IR_CONT;			// thats it, we're finished with this
		default:
			ob_speech->command = 0;		// not yet implemented - just cancel
			ob_speech->wait_state = 1;	// waiting for command
			break;
		}

		if (SPEECH_ID == ID) {
			// new command for us!
			// clear this or it could trigger next go
			SPEECH_ID = 0;

			// grab the command - potentially, we only have this
			// cycle to do this

			ob_speech->command = INS_COMMAND;
			ob_speech->ins1 = INS1;
			ob_speech->ins2 = INS2;
			ob_speech->ins3 = INS3;
			ob_speech->ins4 = INS4;
			ob_speech->ins5 = INS5;

			// the current send has been recieved - i.e. seperate
			// multiple they-do's

			INS_COMMAND = 0;

			// now busy
			ob_speech->wait_state = 0;

			debug(5, "received new command %d", INS_COMMAND);

			// we'll drop off and be caught by the while(1), so
			// kicking in the new command straight away
		} else {
			// no new command
			// we could run a blink anim (or something) here

			// now free
			ob_speech->wait_state = 1;

			// come back again next cycle
			return IR_REPEAT;
		}
	}
}

#define S_OB_GRAPHIC	0
#define S_OB_SPEECH	1
#define S_OB_LOGIC	2
#define S_OB_MEGA	3

#define S_TEXT		4
#define S_WAV		5
#define S_ANIM		6
#define S_DIR_TABLE	7
#define S_ANIM_MODE	8

uint32 unpause_zone = 0;

int32 Logic::fnISpeak(int32 *params) {
	// its the super versatile fnSpeak
	// text and wavs can be selected in any combination

	// we can assume no human - there should be no human at least!

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

	_mouseEvent *me;
	_animHeader *anim_head;
	Object_logic *ob_logic;
	Object_graphic *ob_graphic;
	Object_mega *ob_mega;
	uint8 *anim_file;
	uint32 local_text;
	uint32 text_res;
	uint8 *text;
	static uint8 textRunning, speechRunning;
	int32 *anim_table;
	uint8 speechFinished = 0;
	int8 speech_pan;
	char speechFile[256];
	static uint8 cycle_skip = 0;
  	uint32	rv;

	// for text/speech testing & checking for correct file type
	_standardHeader	*head;
	// for text/speech testing - keeping track of text resource currently being tested
	static uint32 currentTextResource = 0;

	// set up the pointers which we know we'll always need

	ob_logic = (Object_logic *) params[S_OB_LOGIC];
	ob_graphic = (Object_graphic *) params[S_OB_GRAPHIC];

	// FIRST TIME ONLY: create the text, load the wav, set up the anim,
	// etc.

	if (!ob_logic->looping) {
		// New fudge to wait for smacker samples to finish
		// since they can over-run into the game

		if (g_sound->getSpeechStatus() != RDSE_SAMPLEFINISHED)
			return IR_REPEAT;
		
		// New fudge for 'fx' subtitles
		// If subtitles switched off, and we don't want to use a wav
		// for this line either, then just quit back to script right
		// now!

		if (gui->_subtitles == 0 && WantSpeechForLine(params[S_WAV]) == 0)
			return IR_CONT;

		if (cycle_skip == 0) {
			// drop out for 1st cycle to allow walks/anims to end
			// & display last frame/ before system locks while
			// speech loaded

			cycle_skip = 1;
			return IR_REPEAT;
		} else
			cycle_skip = 0;

 		textNumber = params[S_TEXT];	// for debug info

		// For testing all text & speech!
		// A script loop can send any text number to fnISpeak and it
		// will only run the valid ones or return with 'result' equal
		// to '1' or '2' to mean 'invalid text resource' and 'text
		// number out of range' respectively
		//
		// See 'testing_routines' object in George's Player Character
		// section of linc

		if (SYSTEM_TESTING_TEXT) {
			RESULT = 0;

			text_res = params[S_TEXT] / SIZE;
			local_text = params[S_TEXT] & 0xffff;

			// if the resource number is within range & it's not
			// a null resource

			if (res_man.checkValid(text_res)) {
				// open the resource
				head = (_standardHeader*) res_man.open(text_res);

				if (head->fileType == TEXT_FILE) {
					// if it's not an animation file
					// if line number is out of range
					if (CheckTextLine((uint8*) head, local_text) == 0) {
						// line number out of range
						RESULT = 2;
					}
				} else {
					// invalid (not a text resource)
					RESULT = 1;
				}

				// close the resource
				res_man.close(text_res);

				if (RESULT)
					return IR_CONT;
			} else {
				// not a valid resource number - invalid (null
				// resource)
				RESULT = 1;
				return IR_CONT;
			}
		}

		// Pull out the text line to get the official text number
		// (for wav id). Once the wav id's go into all script text
		// commands, we'll only need this for _SWORD2_DEBUG

		text_res = params[S_TEXT] / SIZE;
		local_text = params[S_TEXT] & 0xffff;

		// open text file & get the line
		text = FetchTextLine(res_man.open(text_res), local_text);
		officialTextNumber = READ_LE_UINT16(text);

		// now ok to close the text file
		res_man.close(text_res);

		// prevent dud lines from appearing while testing text & speech
		// since these will not occur in the game anyway

		if (SYSTEM_TESTING_TEXT) {	// if testing text & speech
			// if actor number is 0 and text line is just a 'dash'
			// character
			if (officialTextNumber == 0 && text[2] == '-' && text[3] == 0) {
				// dud line - return & continue script
				RESULT = 3;
				return IR_CONT;
			}
		}

		// set the 'looping_flag' & the text-click-delay
 
		ob_logic->looping = 1;

		// can't left-click past the text for the first half second
		left_click_delay = 6;

		// can't right-click past the text for the first quarter second
		right_click_delay = 3;

		// Write to walkthrough file (zebug0.txt)
		// if (player_id != george), then player is controlling Nico

		if (PLAYER_ID != CUR_PLAYER_ID)
			debug(5, "(%d) Nico: %s", officialTextNumber, text + 2);
		else
			debug(5, "(%d) %s: %s", officialTextNumber, FetchObjectName(ID), text + 2);

		// Set up the speech animation

		if (params[S_ANIM]) {
			// just a straight anim
			anim_id = params[S_ANIM];

			// anim type
			speech_anim_type = SPEECHANIMFLAG;

			// set the talker's graphic to this speech anim now
			ob_graphic->anim_resource = anim_id;

			// set to first frame
			ob_graphic->anim_pc = 0;
		} else if (params[S_DIR_TABLE]) {
			// use this direction table to derive the anim
			// NB. ASSUMES WE HAVE A MEGA OBJECT!!

			ob_mega = (Object_mega*) params[S_OB_MEGA];

			// pointer to anim table
			anim_table = (int32 *) params[S_DIR_TABLE];

			// appropriate anim resource is in 'table[direction]'
			anim_id = anim_table[ob_mega->current_dir];

			// anim type
			speech_anim_type = SPEECHANIMFLAG;

			// set the talker's graphic to this speech anim now
			ob_graphic->anim_resource = anim_id;

			// set to first frame
			ob_graphic->anim_pc = 0;
		} else {
			// no animation choosen
			anim_id = 0;
		}

		// Default back to looped lip synced anims.
		SPEECHANIMFLAG = 0;

		// set up 'text_x' & 'text_y' for speech-pan and/or
		// text-sprite position

		LocateTalker(params);

		// is it to be speech or subtitles or both?

		// assume not running until know otherwise
		speechRunning = 0;

		// New fudge for 'fx' subtitles
		// if speech is selected, and this line is allowed speech
		// (not if it's an fx subtitle!)

		if (!g_sound->isSpeechMute() && WantSpeechForLine(officialTextNumber)) {
			// if the wavId paramter is zero because not yet
			// compiled into speech command, we can still get it
			// from the 1st 2 chars of the text line

			if (!params[S_WAV])
				params[S_WAV] = (int32) officialTextNumber;

#define SPEECH_VOLUME	16	// 0..16
#define SPEECH_PAN	0	// -16..16

			speech_pan = ((text_x - 320) * 16) / 320;

			// 'text_x'	'speech_pan'
			//  0		-16
			//  320		0
			//  640		16

			// keep within limits of -16..16, just in case
			if (speech_pan < -16)
				speech_pan = -16;
			else if (speech_pan > 16)
				speech_pan = 16;

			// if we're testing text & speech
			if (SYSTEM_TESTING_TEXT) {
				// if we've moved onto a new text resource,
				// we will want to check if the CD needs
				// changing again - can only know which CD to
				// get if the wavID is non-zero

				if (text_res != currentTextResource && params[S_WAV]) {
					// ensure correct CD is in for this
					// wavId
					// GetCorrectCdForSpeech(params[S_WAV]);
					currentTextResource = text_res;
				}
			}

			// set up path to speech cluster
			// first checking if we have speech1.clu or
			// speech2.clu in current directory (for translators
			// to test)

			File fp;

			sprintf(speechFile, "speech%d.clu", res_man.whichCd());

			if (fp.open(speechFile))
				fp.close();
			else
				strcpy(speechFile, "speech.clu");

			// Load speech but don't start playing yet
			rv = g_sound->playCompSpeech(speechFile, params[S_WAV], SPEECH_VOLUME, speech_pan);
			if (rv == RD_OK) {
				// ok, we've got something to play
				// (2 means not playing yet - see below)
				speechRunning = 1;

				// set it playing now (we might want to do
				// this next cycle, don't know yet)
				g_sound->unpauseSpeech();
			} else {
				debug(5, "ERROR: PlayCompSpeech(speechFile=\"%s\", wav=%d (res=%d pos=%d)) returned %.8x", speechFile, params[S_WAV], text_res, local_text, rv);
			}
		}

		// if we want subtitles, or speech failed to load
		if (gui->_subtitles || speechRunning == 0) {
			// then we're going to show the text
			textRunning = 1;

			// so create the text sprite
			Form_text(params);
		} else {
			// otherwise don't want text
			textRunning = 0;
		}
	}

	// EVERY TIME: run a cycle of animation, if there is one

	if (anim_id) {
		// there is an animation
		// increment the anim frame number
		ob_graphic->anim_pc++;

		// open the anim file
		anim_file = res_man.open(ob_graphic->anim_resource);
		anim_head = FetchAnimHeader(anim_file);

		if (!speech_anim_type) {
			// ANIM IS TO BE LIP-SYNC'ED & REPEATING
			// if finished the anim
			if (ob_graphic->anim_pc == (int32) (anim_head->noAnimFrames)) {
				// restart from frame 0
				ob_graphic->anim_pc = 0;
			} else if (speechRunning) {
				// if playing a sample
				if (!unpause_zone) {
					// if we're at a quiet bit
					if (g_sound->amISpeaking() == RDSE_QUIET) {
						// restart from frame 0
						// ('closed mouth' frame)
						ob_graphic->anim_pc = 0;
					}
				}
			}
		} else {
			// ANIM IS TO PLAY ONCE ONLY
			if (ob_graphic->anim_pc == (int32) (anim_head->noAnimFrames) - 1) {
				// reached the last frame of the anim
				// hold anim on this last frame
				anim_id = 0;
			}
		}

		// close the anim file
		res_man.close(ob_graphic->anim_resource);
	} else if (speech_anim_type) {
		// Placed here so we actually display the last frame of the
		// anim.
		speech_anim_type = 0;
	}

	// EVERY TIME: FIND OUT IF WE NEED TO STOP THE SPEECH NOW...

	// if there is a wav then we're using that to end the speech naturally

	// if playing a sample (note that value of '2' means about to play!)

	if (speechRunning == 1) {
		if (!unpause_zone) {
			// has it finished?
			if (g_sound->getSpeechStatus() == RDSE_SAMPLEFINISHED)
				speechFinished = 1;
		} else
			unpause_zone--;
	} else if (speechRunning == 0 && speech_time) {
		// counting down text time because there is no sample - this
		// ends the speech

		// if no sample then we're using speech_time to end speech
		// naturally

		speech_time--;
		if (!speech_time)
			speechFinished = 1;
	}

	// ok, all is running along smoothly - but a click means stop
	// unnaturally

	// so that we can go to the options panel while text & speech is
	// being tested
	if (SYSTEM_TESTING_TEXT == 0 || g_display->_mouseY > 0) {
		me = MouseEvent();

		// Note that we now have TWO click-delays - one for LEFT
		// button, one for RIGHT BUTTON

		if ((!left_click_delay && me && (me->buttons & RD_LEFTBUTTONDOWN)) ||
		    (!right_click_delay && me && (me->buttons&RD_RIGHTBUTTONDOWN))) {
			// mouse click, after click_delay has expired -> end
			// the speech we ignore mouse releases

			// if testing text & speech
			if (SYSTEM_TESTING_TEXT) {
				// and RB used to click past text
				if (me->buttons & RD_RIGHTBUTTONDOWN) {
					// then we want the previous line again
					SYSTEM_WANT_PREVIOUS_LINE = 1;
				} else {
					// LB just want next line again
					SYSTEM_WANT_PREVIOUS_LINE = 0;
				}
			}

			do {
				// trash anything thats buffered
				me = MouseEvent();
			} while (me);

			speechFinished = 1;

			// if speech sample playing
			if (speechRunning) {
				// halt the sample prematurely
				g_sound->stopSpeech();
			}
		}
	}

	// if we are finishing the speech this cycle, do the business

	// !speech_anim_type, as we want an anim which is playing once to
	// have finished.

	if (speechFinished && !speech_anim_type) {
		// if there is text
		if (speech_text_bloc_no) {
			// kill the text block
			fontRenderer.killTextBloc(speech_text_bloc_no);
			speech_text_bloc_no = 0;
		}

		// if there is a speech anim
		if (anim_id) {
			// end it on 1st frame (closed mouth)
			anim_id = 0;
			ob_graphic->anim_pc = 0;
		}

		textRunning = 0;
		speechRunning = 0;

		// no longer in a script function loop
		ob_logic->looping = 0;

		// reset for debug info
		textNumber = 0;

		// reset to zero, in case text line not even extracted (since
		// this number comes from the text line)
		officialTextNumber = 0;

		RESULT = 0;	// ok
		return IR_CONT;
	}

	// speech still going, so decrement the click_delay if it's still
	// active

	// count down to clickability

	if (left_click_delay)
		left_click_delay--;

 	if (right_click_delay)
		right_click_delay--;

	// back again next cycle
	return IR_REPEAT;
}

#define GAP_ABOVE_HEAD	20	// distance kept above talking sprite

void LocateTalker(int32	*params) {
	// sets 'text_x' & 'text_y' for position of text sprite
	// but 'text_x' also used to calculate speech-pan

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

	Object_mega *ob_mega;

	uint8 *file;
	_frameHeader *frame_head;
	_animHeader *anim_head;
	_cdtEntry *cdt_entry;
	uint16 scale;

	// if there's no anim
	if (anim_id == 0) {
		// assume it's Voice-Over text, so it goes at bottom of screen
		text_x = 320;
		text_y = 400;
	} else {
		// Note: this code has been adapted from Register_frame() in
		// build_display.cpp

		// open animation file & set up the necessary pointers
		file = res_man.open(anim_id);

		anim_head = FetchAnimHeader(file);

		// '0' means 1st frame
		cdt_entry = FetchCdtEntry(file, 0);

		// '0' means 1st frame
		frame_head = FetchFrameHeader(file, 0);

		// check if this frame has offsets ie. this is a scalable
		// mega frame

		if (cdt_entry->frameType & FRAME_OFFSET) {
			// this may be NULL
			ob_mega = (Object_mega*) params[S_OB_MEGA];

			// calc scale at which to print the sprite, based on
			// feet y-coord & scaling constants (NB. 'scale' is
			// actually 256 * true_scale, to maintain accuracy)

			// Ay+B gives 256 * scale ie. 256 * 256 * true_scale
			// for even better accuracy, ie. scale = (Ay + B) / 256
			scale = (uint16) ((ob_mega->scale_a * ob_mega->feet_y + ob_mega->scale_b) / 256);

			// calc suitable centre point above the head, based on
			// scaled height

			// just use 'feet_x' as centre
			text_x = (int16) (ob_mega->feet_x);

			// add scaled y-offset to feet_y coord to get top of
			// sprite
			text_y = (int16) (ob_mega->feet_y + (cdt_entry->y * scale) / 256);
		} else {
			// it's a non-scaling anim - calc suitable centre
			// point above the head, based on scaled width

			// x-coord + half of width
			text_x = cdt_entry->x + (frame_head->width) / 2;
			text_y = cdt_entry->y;
		}

		// leave space above their head
		text_y -= GAP_ABOVE_HEAD;
			
		// adjust the text coords for RDSPR_DISPLAYALIGN

		text_x -= this_screen.scroll_offset_x;
		text_y -= this_screen.scroll_offset_y;

		// release the anim resource
		res_man.close(anim_id);
	}
}

void Form_text(int32 *params) {
	// its the first time in so we build the text block if we need one
	// we also bring in the wav if there is one
	// also setup the animation if there is one

	// anim is optional - anim can be a repeating lip-sync or a run-once
	// anim

	// if there is no wav then the text comes up instead
	// there can be any combination of text/wav playing

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

	uint32 local_text;
	uint32 text_res;
	uint8 *text;
	uint32 textWidth;
	Object_speech *ob_speech;

	// should always be a text line, as all text is derived from line of
	// text

	if (params[S_TEXT]) {
	 	ob_speech = (Object_speech *) params[S_OB_SPEECH];

		// establish the max width allowed for this text sprite

		// if a specific width has been set up for this character,
		// then override the default

 		if (ob_speech->width)
			textWidth = ob_speech->width;
		else
			textWidth = 400;

		// pull out the text line & make the sprite & text block

		text_res = params[S_TEXT] / SIZE;
		local_text = params[S_TEXT] & 0xffff;

		// open text file & get the line
		text = FetchTextLine(res_man.open(text_res), local_text);

		// 'text + 2' to skip the first 2 bytes which form the line
		// reference number

		speech_text_bloc_no = fontRenderer.buildNewBloc(
			text + 2, text_x, text_y,
			textWidth, ob_speech->pen,
			RDSPR_TRANS | RDSPR_DISPLAYALIGN,
			g_sword2->_speechFontId, POSITION_AT_CENTRE_OF_BASE);

		// now ok to close the text file
		res_man.close(text_res);

		// set speech duration, in case not using wav
		// no. of cycles = (no. of chars) + 30

		speech_time = strlen((char *) text) + 30;
	} else {
		// no text line passed? - this is bad
		debug(5, "no text line for speech wav %d", params[S_WAV]);
	}
}

#ifdef _SWORD2_DEBUG
void GetCorrectCdForSpeech(int32 wavId) {
	File fp;

	// 1, 2 or 0 (if speech on both cd's, ie. no need to change)
	uint8 cd;

	if (!fp.open("cd.bin"))
		error("Need cd.bin file for testing speech!");

	fp.seek(wavId, SEEK_SET);
	fp.read(&cd, 1);

	fp.close();

	// if we specifically need CD1 or CD2 (ie. it's not on both)
	// then check it's there (& ask for it if it's not there)
	if (cd == 1 || cd == 2)
		res_man.getCd(cd);
}
#endif

// For preventing sfx subtitles from trying to load speech samples
// - since the sfx are implemented as normal sfx, so we don't want them as
// speech samples too
// - and we only want the subtitles if selected, not if samples can't be found!

uint8 WantSpeechForLine(uint32 wavId) {
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
	case 920:	// location 62
	case 923:	// location 62
	case 926:	// location 62
		// don't want speech for these lines!
		return 0;
	default:
		// ok for all other lines
		return 1;
	}
}

} // End of namespace Sword2
