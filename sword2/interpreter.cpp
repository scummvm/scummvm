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

#include "console.h"
#include "driver/driver96.h"
#include "interpreter.h"

// This file serves two purposes. It is compiled as part of the test functions
// of Linc, and also as part of the game

// I assume Linc was the name of some sort of development tool. Anyway, I've
// removed the pieces of code that were labelled as INSIDE_LINC, because we
// don't have it, and probably wouldn't have much use for it if we did.

// The machine code table

int32 FN_test_function(int32 *params);
int32 FN_test_flags(int32 *params);
int32 FN_register_start_point(int32 *params);
int32 FN_init_background(int32 *params);
int32 FN_set_session(int32 *params);
int32 FN_back_sprite(int32 *params);	
int32 FN_sort_sprite(int32 *params);	
int32 FN_fore_sprite(int32 *params);	
int32 FN_register_mouse(int32 *params);	
int32 FN_anim(int32 *);
int32 FN_random(int32 *);
int32 FN_preload(int32 *);
int32 FN_add_subject(int32 *);
int32 FN_interact(int32 *);
int32 FN_choose(int32 *);
int32 FN_walk(int32 *);
int32 FN_walk_to_anim(int32 *);		// walk to start position of anim
int32 FN_turn(int32 *);			// turn to (dir)
int32 FN_stand_at(int32 *);		// stand at (x,y,dir)
int32 FN_stand(int32 *);		// stand facing (dir)
int32 FN_stand_after_anim(int32 *);	// stand at end position of anim
int32 FN_pause(int32 *);
int32 FN_mega_table_anim(int32 *);
int32 FN_add_menu_object(int32 *);
int32 FN_start_conversation(int32 *);
int32 FN_end_conversation(int32 *);
int32 FN_set_frame(int32 *);
int32 FN_random_pause(int32 *);
int32 FN_register_frame(int32 *);
int32 FN_no_sprite(int32 *);
int32 FN_send_sync(int32 *);
int32 FN_update_player_stats(int32 *);
int32 FN_pass_graph(int32 *);
int32 FN_init_floor_mouse(int32 *);
int32 FN_pass_mega(int32 *);
int32 FN_face_xy(int32 *);
int32 FN_end_session(int32 *);
int32 FN_no_human(int32 *);
int32 FN_add_human(int32 *);
int32 FN_we_wait(int32 *);
int32 FN_they_do_we_wait(int32 *);
int32 FN_they_do(int32 *);
int32 FN_walk_to_talk_to_mega(int32 *);
int32 FN_fade_down(int32 *);
int32 FN_i_speak(int32 *);
int32 FN_total_restart(int32 *);
int32 FN_set_walkgrid(int32 *);
int32 FN_speech_process(int32 *);
int32 FN_set_scaling(int32 *);
int32 FN_start_event(int32 *);
int32 FN_check_event_waiting(int32 *);
int32 FN_request_speech(int32 *);
int32 FN_gosub(int32 *);
int32 FN_timed_wait(int32 *);
int32 FN_play_fx(int32 *);
int32 FN_stop_fx(int32 *);
int32 FN_play_music(int32 *);
int32 FN_stop_music(int32 *);
int32 FN_set_value(int32 *);
int32 FN_new_script(int32 *);
int32 FN_get_sync(int32 *);
int32 FN_wait_sync(int32 *);
int32 FN_register_walkgrid(int32 *);
int32 FN_reverse_mega_table_anim(int32 *);
int32 FN_reverse_anim(int32 *);
int32 FN_add_to_kill_list(int32 *);
int32 FN_set_standby_coords(int32 *);
int32 FN_back_par0_sprite(int32 *params);
int32 FN_back_par1_sprite(int32 *params);
int32 FN_fore_par0_sprite(int32 *params);
int32 FN_fore_par1_sprite(int32 *params);
int32 FN_set_player_action_event(int32 *params);
int32 FN_set_scroll_coordinate(int32 *params);
int32 FN_stand_at_anim(int32 *params);
int32 FN_set_scroll_left_mouse(int32 *params);
int32 FN_set_scroll_right_mouse(int32 *params);
int32 FN_colour(int32 *params);
int32 FN_flash(int32 *params);
int32 FN_prefetch(int32 *params);
int32 FN_get_player_savedata(int32 *params);
int32 FN_pass_player_savedata(int32 *params);
int32 FN_send_event(int32 *params);
int32 FN_add_walkgrid(int32 *params);
int32 FN_remove_walkgrid(int32 *params);
int32 FN_check_for_event(int32 *params);
int32 FN_pause_for_event(int32 *params);
int32 FN_clear_event(int32 *params);
int32 FN_face_mega(int32 *params);
int32 FN_play_sequence(int32 *params);
int32 FN_shaded_sprite(int32 *params);
int32 FN_unshaded_sprite(int32 *params);
int32 FN_fade_up(int32 *params);
int32 FN_display_msg(int32 *params);
int32 FN_set_object_held(int32 *params);
int32 FN_add_sequence_text(int32 *params);
int32 FN_reset_globals(int32 *params);
int32 FN_set_palette(int32 *params);
int32 FN_register_pointer_text(int32 *params);
int32 FN_fetch_wait(int32 *params);
int32 FN_release(int32 *params);
int32 FN_sound_fetch(int32 *params);
int32 FN_prepare_music(int32 *params);
int32 FN_smacker_lead_in(int32 *params);
int32 FN_smacker_lead_out(int32 *params);
int32 FN_stop_all_fx(int32 *params);
int32 FN_check_player_activity(int32 *params);
int32 FN_reset_player_activity_delay(int32 *params);
int32 FN_check_music_playing(int32 *params);
int32 FN_play_credits(int32 *params);
int32 FN_set_scroll_speed_normal(int32 *params);
int32 FN_set_scroll_speed_slow(int32 *params);
int32 FN_remove_chooser(int32 *params);
int32 FN_set_fx_vol_and_pan(int32 *params);
int32 FN_set_fx_vol(int32 *params);
int32 FN_restore_game(int32 *params);
int32 FN_refresh_inventory(int32 *params);
int32 FN_change_shadows(int32 *params);

#define MAX_FN_NUMBER	117

extern int32 (*McodeTable[]) (int32 *);

// Point to the global variable data
int32 *globalInterpreterVariables2 = NULL;

int g_debugFlag = 0;	// Set this to turn debugging on

int32 (*McodeTable[MAX_FN_NUMBER + 1]) (int32 *) = {
	FN_test_function,
	FN_test_flags,
	FN_register_start_point,
	FN_init_background,
	FN_set_session,
	FN_back_sprite,
	FN_sort_sprite,
	FN_fore_sprite,
	FN_register_mouse,
	FN_anim,
	FN_random,
	FN_preload,
	FN_add_subject,
	FN_interact,
	FN_choose,
	FN_walk,
	FN_walk_to_anim,
	FN_turn,
	FN_stand_at,
	FN_stand,
	FN_stand_after_anim,
	FN_pause,
	FN_mega_table_anim,
	FN_add_menu_object,
	FN_start_conversation,
	FN_end_conversation,
	FN_set_frame,
	FN_random_pause,
	FN_register_frame,
	FN_no_sprite,
	FN_send_sync,
	FN_update_player_stats,
	FN_pass_graph,
	FN_init_floor_mouse,
	FN_pass_mega,
	FN_face_xy,
	FN_end_session,
	FN_no_human,
	FN_add_human,
	FN_we_wait,
	FN_they_do_we_wait,
	FN_they_do,
	FN_walk_to_talk_to_mega,
	FN_fade_down,
	FN_i_speak,
	FN_total_restart,
	FN_set_walkgrid,
	FN_speech_process,
	FN_set_scaling,
	FN_start_event,
	FN_check_event_waiting,
	FN_request_speech,
	FN_gosub,
	FN_timed_wait,
	FN_play_fx,
	FN_stop_fx,
	FN_play_music,
	FN_stop_music,
	FN_set_value,
	FN_new_script,
	FN_get_sync,
	FN_wait_sync,
	FN_register_walkgrid,
	FN_reverse_mega_table_anim,
	FN_reverse_anim,
	FN_add_to_kill_list,
	FN_set_standby_coords,
	FN_back_par0_sprite,
	FN_back_par1_sprite,
	FN_fore_par0_sprite,
	FN_fore_par1_sprite,
	FN_set_player_action_event,
	FN_set_scroll_coordinate,
	FN_stand_at_anim,
	FN_set_scroll_left_mouse,
	FN_set_scroll_right_mouse,
	FN_colour,
	FN_flash,
	FN_prefetch,
	FN_get_player_savedata,
	FN_pass_player_savedata,
	FN_send_event,
	FN_add_walkgrid,
	FN_remove_walkgrid,
	FN_check_for_event,
	FN_pause_for_event,
	FN_clear_event,
	FN_face_mega,
	FN_play_sequence,
	FN_shaded_sprite,
	FN_unshaded_sprite,
	FN_fade_up,
	FN_display_msg,
	FN_set_object_held,
	FN_add_sequence_text,
	FN_reset_globals,
	FN_set_palette,
	FN_register_pointer_text,
	FN_fetch_wait,
	FN_release,
	FN_prepare_music,
	FN_sound_fetch,
	FN_prepare_music,
	FN_smacker_lead_in,
	FN_smacker_lead_out,
	FN_stop_all_fx,
	FN_check_player_activity,
	FN_reset_player_activity_delay,
	FN_check_music_playing,
	FN_play_credits,
	FN_set_scroll_speed_normal,
	FN_set_scroll_speed_slow,
	FN_remove_chooser,
	FN_set_fx_vol_and_pan,
	FN_set_fx_vol,
	FN_restore_game,
	FN_refresh_inventory,
	FN_change_shadows,
};

#define CHECKSTACKPOINTER2 ASSERT(stackPointer2 >= 0 && stackPointer2 < STACK_SIZE);
#define	PUSHONSTACK(x) { stack2[stackPointer2] = (x); stackPointer2++; CHECKSTACKPOINTER2 }
#define POPOFFSTACK(x) { x = stack2[stackPointer2 - 1]; stackPointer2--; CHECKSTACKPOINTER2 }
#define DOOPERATION(x) { stack2[stackPointer2 - 2] = (x); stackPointer2--; CHECKSTACKPOINTER2 }

void SetGlobalInterpreterVariables(int32 *vars) {
	globalInterpreterVariables2 = vars;
}

int RunScript(char *scriptData, char *objectData, uint32 *offset) {
	#define STACK_SIZE		10

	_standardHeader *header = (_standardHeader *) scriptData;
	scriptData += sizeof(_standardHeader) + sizeof(_object_hub);

	// The script data format:
	//	int32_TYPE	1		Size of variable space in bytes
	//	...				The variable space
	//	int32_TYPE	1		numberOfScripts
	//	int32_TYPE	numberOfScripts	The offsets for each script

	// Initialise some stuff

	int ip = 0;			 // Code pointer
	int curCommand,parameter, value; // Command and parameter variables
	int32 stack2[STACK_SIZE];	 // The current stack
	int32 stackPointer2 = 0;	// Position within stack
	int parameterReturnedFromMcodeFunction = 0;	// Allow scripts to return things
	int savedStartOfMcode = 0;	// For saving start of mcode commands

	int count;
	int retVal;
	int caseCount, foundCase;
	int scriptNumber, foundScript;
	const char *tempScrPtr;

	// Get the start of variables and start of code
	DEBUG("Enter interpreter data %x, object %x, offset %d", scriptData, objectData, *offset);

	// FIXME: 'scriptData' and 'variables' used to be const. However,
	// this code writes into 'variables' so it can not be const.

	char *variables = scriptData + sizeof(int);
	const char *code = scriptData + (int32) READ_LE_UINT32(scriptData) + sizeof(int);
	uint32 noScripts = (int32) READ_LE_UINT32(code);

	if (*offset < noScripts) {
		ip = READ_LE_UINT32((const int *) code + *offset + 1);
		DEBUG("Start script %d with offset %d",*offset,ip);
	} else {
		ip = *offset;
		DEBUG("Start script with offset %d",ip);
	}

	code += noScripts * sizeof(int) + sizeof(int);

#ifdef DONTPROCESSSCRIPTCHECKSUM
	code += sizeof(int) * 3;
#else
	// Code should nop be pointing at an identifier and a checksum
	const int *checksumBlock = (const int *) code;
	code += sizeof(int) * 3;

	if (READ_LE_UINT32(checksumBlock) != 12345678) {
		Con_fatal_error("Invalid script in object %s", header->name);
		return 0;
	}

	int codeLen = READ_LE_UINT32(checksumBlock + 1);
	int checksum = 0;

	for (count = 0; count < codeLen; count++)
		checksum += (unsigned char) code[count];

	if (checksum != (int32) READ_LE_UINT32(checksumBlock + 2)) {
		Con_fatal_error("Checksum error in object %s", header->name);
		return 0;
	}
#endif

	int runningScript = 1;

	while (runningScript) {
		curCommand = code[ip++];

		switch(curCommand) {
			case CP_END_SCRIPT:
				// End the script
				DEBUG("End script",0);
				runningScript = 0;
				break;

			case CP_PUSH_LOCAL_VAR32:
				// Push the contents of a local variable
				Read16ip(parameter);
				DEBUG("Push local var %d (%d)", parameter, *(int32 *) (variables + parameter));
				PUSHONSTACK(*(int32 *) (variables + parameter));
				break;

			case CP_PUSH_GLOBAL_VAR32:
				// Push a global variable
				Read16ip(parameter);
				DEBUG("Push global var %d (%d)", parameter, globalInterpreterVariables2[parameter]);
				ASSERT(globalInterpreterVariables2);
				PUSHONSTACK(globalInterpreterVariables2[parameter]);
				break;

			case CP_POP_LOCAL_VAR32:
				// Pop a value into a local word variable
				Read16ip(parameter);
				POPOFFSTACK(value);
				DEBUG("Pop %d into var %d", value, parameter);
				*((int32 *) (variables + parameter)) = value;
				break;

			case CP_CALL_MCODE:
				// Call an mcode routine
				Read16ip(parameter);
				ASSERT(parameter <= MAX_FN_NUMBER);
				// amount to adjust stack by (no of parameters)
				Read8ip(value);
				DEBUG("Call mcode %d with stack = %x", parameter, stack2 + stackPointer2 - value);
				retVal = McodeTable[parameter](stack2 + stackPointer2 - value);
				stackPointer2 -= value;
				CHECKSTACKPOINTER2

				switch (retVal & 7) {
					case IR_STOP:
						// Quit out for a cycle
						*offset = ip;
						return 0;

					case IR_CONT:
						// Continue as normal
						break;

					case IR_TERMINATE:
						// Return without updating the
						// offset
						return 2;

					case IR_REPEAT:
						// Return setting offset to
						// start of this function call
						*offset = savedStartOfMcode;
						return 0;

					case IR_GOSUB:
						// that's really neat
						*offset = ip;
						return 2;

					default:
						ASSERT(FALSE);
				}
				parameterReturnedFromMcodeFunction = retVal >> 3;
				break;

			case CP_PUSH_LOCAL_ADDR:
				// push the address of a local variable
				Read16ip(parameter);
				DEBUG("Push address of local variable %d (%x)", parameter, (int32) (variables + parameter));
				PUSHONSTACK((int32) (variables + parameter));
				break;

			case CP_PUSH_INT32:
				// Push a long word value on to the stack
				Read32ip(parameter);
				DEBUG("Push int32 %d (%x)", parameter, parameter);
				PUSHONSTACK(parameter);
				break;

			case CP_SKIPONFALSE:
				// Skip if the value on the stack is false
				Read32ipLeaveip(parameter);
				POPOFFSTACK(value);
				DEBUG("Skip %d if %d is false", parameter, value);
				if (value)
					ip += sizeof(int32);
				else
					ip += parameter;
				break;

			case CP_SKIPALWAYS:
				// skip a block
				Read32ipLeaveip(parameter);
				DEBUG("Skip %d", parameter);
				ip += parameter;
				break;

			case CP_SWITCH:
				// 9 switch
				POPOFFSTACK(value);
				Read32ip(caseCount);

				// Search the cases
				foundCase = 0;
				for (count = 0; count < caseCount && !foundCase; count++) {
					if (value == (int32) READ_LE_UINT32(code + ip)) {
						// We have found the case, so
						// lets jump to it
						foundCase = 1;
						ip += READ_LE_UINT32(code + ip + sizeof(int32));
					} else
						ip += sizeof(int32) * 2;
				}

				// If we found no matching case then use the
				// default

				if (!foundCase)
					ip += READ_LE_UINT32(code + ip);

				break;

			case CP_ADDNPOP_LOCAL_VAR32:
				Read16ip(parameter);
				POPOFFSTACK(value);
				*((int32 *) (variables + parameter)) += value;
				DEBUG("+= %d into var %d->%d", value, parameter, *(int32 *) (variables + parameter));
				break;

			case CP_SUBNPOP_LOCAL_VAR32:
				Read16ip(parameter);
				POPOFFSTACK(value);
				*((int32 *) (variables + parameter)) -= value;
				DEBUG("-= %d into var %d->%d", value, parameter, *(int32 *) (variables + parameter));
				break;

			case CP_SKIPONTRUE:
				// Skip if the value on the stack is TRUE
				Read32ipLeaveip(parameter);
				POPOFFSTACK(value);
				DEBUG("Skip %d if %d is false", parameter, value);
				if (!value)
					ip += sizeof(int32);
				else
					ip += parameter;
				break;

			case CP_POP_GLOBAL_VAR32:
				// Pop a global variable
				Read16ip(parameter);
				POPOFFSTACK(value);
				DEBUG("Pop %d into global var %d", value, parameter);

#ifdef TRACEGLOBALVARIABLESET
				TRACEGLOBALVARIABLESET(parameter, value);
#endif

				globalInterpreterVariables2[parameter] = value;
				break;

			case CP_ADDNPOP_GLOBAL_VAR32:
				// Add and pop a global variable
				Read16ip(parameter);
				// parameter = *((int16_TYPE *) (code + ip));
				// ip += 2;
				POPOFFSTACK(value);
				globalInterpreterVariables2[parameter] += value;
				DEBUG("+= %d into global var %d->%d", value, parameter, *(int32 *) (variables + parameter));
				break;

			case CP_SUBNPOP_GLOBAL_VAR32:
				// Sub and pop a global variable
				Read16ip(parameter);
				POPOFFSTACK(value);
				globalInterpreterVariables2[parameter] -= value;
				DEBUG("-= %d into global var %d->%d", value, parameter, *(int32 *) (variables + parameter));
				break;

			case CP_DEBUGON:
				// Turn debugging on
				g_debugFlag = 1;
				break;

			case CP_DEBUGOFF:
				// Turn debugging on
				g_debugFlag = 0;
				break;

			case CP_QUIT:
				// Quit out for a cycle
				*offset = ip;
				return 0;

			case CP_TERMINATE:
				// Quit out immediately without affecting the
				// offset pointer
				return 3;

			// Operators

			case OP_ISEQUAL:
				// '=='
				DEBUG("%d == %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] == stack2[stackPointer2 - 1]);
				DOOPERATION (stack2[stackPointer2 - 2] == stack2[stackPointer2 - 1]);
				break;

			case OP_PLUS:
				// '+'
				DEBUG("%d + %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] + stack2[stackPointer2 - 1]);
				DOOPERATION(stack2[stackPointer2 - 2] + stack2[stackPointer2 - 1]);
				break;

			case OP_MINUS:
				// '-'
				DEBUG("%d - %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] - stack2[stackPointer2 - 1]);
				DOOPERATION(stack2[stackPointer2 - 2] - stack2[stackPointer2 - 1]);
				break;

			case OP_TIMES:
				// '*'
				DEBUG("%d * %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] * stack2[stackPointer2 - 1]);
				DOOPERATION(stack2[stackPointer2 - 2] * stack2[stackPointer2 - 1]);
				break;

			case OP_DIVIDE:
				// '/'
				DEBUG("%d / %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] / stack2[stackPointer2 - 1]);
				DOOPERATION(stack2[stackPointer2 - 2] / stack2[stackPointer2 - 1]);
				break;

			case OP_NOTEQUAL:
				// '!='
				DEBUG("%d != %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] != stack2[stackPointer2 - 1]);
				DOOPERATION(stack2[stackPointer2 - 2] != stack2[stackPointer2 - 1]);
				break;

			case OP_ANDAND:
				// '&&'
				DEBUG("%d != %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] && stack2[stackPointer2 - 1]);
				DOOPERATION(stack2[stackPointer2 - 2] && stack2[stackPointer2 - 1]);
				break;

			case OP_GTTHAN:
				// '>'
				DEBUG("%d > %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] > stack2[stackPointer2 - 1]);
				DOOPERATION(stack2[stackPointer2 - 2] > stack2[stackPointer2 - 1]);
				break;

			case OP_LSTHAN:
				// '<'
				DEBUG("%d < %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] < stack2[stackPointer2 - 1]);
				DOOPERATION(stack2[stackPointer2 - 2] < stack2[stackPointer2 - 1]);
				break;

			case CP_JUMP_ON_RETURNED:
				// Jump to a part of the script depending on
				// the return value from an mcode routine

				// Get the maximum value
				Read8ip(parameter);

				ip += READ_LE_UINT32(code + ip + parameterReturnedFromMcodeFunction * 4);
				break;

			case CP_TEMP_TEXT_PROCESS:
				// Process a text line
				// This was apparently used in Linc
				Read32ip(parameter);
				DEBUG("Process text id %d", parameter);
				break;

			case CP_SAVE_MCODE_START:
				// Save the start position on an mcode
				// instruction in case we need to restart it
				// again
				savedStartOfMcode = ip - 1;
				break;

			case CP_RESTART_SCRIPT:
				// Start the script again
				// Do a ip search to find the script we are
				// running

				tempScrPtr = scriptData + READ_LE_UINT32(scriptData) + sizeof(int);
				scriptNumber = 0;
				foundScript = 0;

				for (count = 1; count < (int) noScripts && !foundScript; count++) {
					if (ip < ((const int *) tempScrPtr)[count + 1]) {
						scriptNumber = count - 1;
						foundScript = 1;
					}
				}

				if (!foundScript)
					scriptNumber = count - 1;

				// So we know what script we are running,
				// lets restart it

				ip = ((const int *) tempScrPtr)[scriptNumber + 1];
				break;

			case CP_PUSH_STRING:
				// Push the address of a string on to the stack
				// Get the string size
				Read8ip(parameter);
				// ip points to the string
				PUSHONSTACK((int) (code + ip));
				ip += (parameter + 1);
				break;

			case CP_PUSH_DEREFERENCED_STRUCTURE:
				// Push the address of a dereferenced structure
				Read32ip(parameter);
				DEBUG("Push address of far variable (%x)", (int32) (variables + parameter));
				PUSHONSTACK((int) (objectData + sizeof(int) + sizeof(_standardHeader) + sizeof(_object_hub) + parameter));
				break;

			case OP_GTTHANE:
				// '>='
				DEBUG("%d > %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] >= stack2[stackPointer2 - 1]);
				DOOPERATION(stack2[stackPointer2 - 2] >= stack2[stackPointer2 - 1]);
				break;

			case OP_LSTHANE:
				// '<='
				DEBUG("%d < %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] <= stack2[stackPointer2 - 1]);
				DOOPERATION(stack2[stackPointer2 - 2] <= stack2[stackPointer2 - 1]);
				break;

			case OP_OROR:
				// '||'
				DEBUG("%d || %d -> %d",
					stack2[stackPointer2 - 2],
					stack2[stackPointer2 - 1],
					stack2[stackPointer2 - 2] || stack2[stackPointer2 - 1]);
				DOOPERATION (stack2[stackPointer2 - 2] || stack2[stackPointer2 - 1]);
				break;

			default:
				Con_fatal_error("Interpreter error: Invalid token %d", curCommand);
				return 3;
		}
	}

	return 1;
}
