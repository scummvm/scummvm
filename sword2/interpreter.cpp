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

#ifndef INSIDE_LINC			// Are we running in linc?
#include "console.h"
#endif

#include "driver/driver96.h"
#include "interpreter.h"



//		This file serves two purposes. It is compiled as part of the test functions
//		of Linc, and also as part of the game


//		The machine code table

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
int32 FN_walk_to_anim(int32 *);					// walk to start position of anim
int32 FN_turn(int32 *);							// turn to (dir)
int32 FN_stand_at(int32 *);						// stand at (x,y,dir)
int32 FN_stand(int32 *);						// stand facing (dir)
int32 FN_stand_after_anim(int32 *);				// stand at end position of anim
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

extern int32 (*McodeTable[])(int32 *);

#ifndef INSIDE_LINC
int32 * globalInterpreterVariables2 = NULL;		// Point to the global varibale data
int		g_debugFlag = 0;						// Set this to turn debugging on
#endif


int32 (*McodeTable[MAX_FN_NUMBER+1])(int32 *) =
{	FN_test_function,
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

#define CHECKSTACKPOINTER2 ASSERT((stackPointer2>=0)&&(stackPointer2<STACK_SIZE));
#define	PUSHONSTACK(x) {stack2[stackPointer2] = (x);stackPointer2++;CHECKSTACKPOINTER2;}
#define POPOFFSTACK(x) {x=stack2[stackPointer2-1];stackPointer2--;CHECKSTACKPOINTER2;}
#define DOOPERATION(x) {stack2[stackPointer2-2] = (x);stackPointer2--;CHECKSTACKPOINTER2;}

#ifndef INSIDE_LINC
void SetGlobalInterpreterVariables(int32 *vars)
{
	globalInterpreterVariables2 = vars;
}
#endif

#ifdef INSIDE_LINC			// Are we running in linc?
int RunScript ( MCBOVirtualSword &engine , const char * scriptData , char * objectData , uint32 *offset )
#else
int RunScript ( char * scriptData , char * objectData , uint32 *offset )
#endif
{
	#define STACK_SIZE		10

	_standardHeader *header = (_standardHeader *)scriptData;
	scriptData += sizeof(_standardHeader) + sizeof(_object_hub);

	// The script data format:


	//		int32_TYPE	1									Size of variable space in bytes
	//		...											The variable space
	//		int32_TYPE	1									numberOfScripts
	//		int32_TYPE	numberOfScripts						The offsets for each script


	// Initialise some stuff

	int ip = 0;									// Code pointer
	int curCommand,parameter,value;				// Command and parameter variables
	int32 stack2[STACK_SIZE];						// The current stack
	int32 stackPointer2 = 0;						// Position within stack
	int parameterReturnedFromMcodeFunction=0;	// Allow scripts to return things
	int savedStartOfMcode=0;			// For saving start of mcode commands

	// Get the start of variables and start of code
	DEBUG3("Enter interpreter data %x, object %x, offset %d",scriptData,objectData,*offset);

	// FIXME: 'scriptData' and 'variables' used to be const. However,
	// this code writes into 'variables' so it can not be const.
	char *variables = scriptData + sizeof(int);
	const char *code = scriptData + *((int *)scriptData) + sizeof(int);
	uint32 noScripts = *((const int32 *)code);
	if ( (*offset) < noScripts)
	{	ip = ((const int *)code)[(*offset)+1];
		DEBUG2("Start script %d with offset %d",*offset,ip);
	}
	else
	{	ip = (*offset);
		DEBUG1("Start script with offset %d",ip);
	}

	code += noScripts * sizeof(int) + sizeof(int);

/************************************************************************************************/
#ifdef DONTPROCESSSCRIPTCHECKSUM

	code += sizeof(int) * 3;

#else

	// Code should nopw be pointing at an identifier and a checksum
	const int *checksumBlock = (const int *)code;
	code += sizeof(int) * 3;

	if (checksumBlock[0] != 12345678)
	{
#ifdef INSIDE_LINC
		AfxMessageBox(CVString("Invalid script in object %s",header->name));
#else
		Con_fatal_error("Invalid script in object %s",header->name);
#endif
		return(0);
	}
	int codeLen = checksumBlock[1];
	int checksum = 0;
	for (int count = 0 ; count < codeLen ; count++)
		checksum += (unsigned char)code[count];
	if ( checksum != checksumBlock[2] )
	{
#ifdef INSIDE_LINC
		AfxMessageBox(CVString("Checksum error in script %s",header->name));
#else
		Con_fatal_error("Checksum error in object %s",header->name);
#endif
		return(0);
	}

#endif //DONTPROCESSSCRIPTCHECKSUM

/************************************************************************************************/

	int runningScript = 1;
	while ( runningScript )
	{	curCommand = code[ip++];
		switch(curCommand)
		{	case CP_END_SCRIPT:			// 0	End the script
				DEBUG1("End script",0);
				runningScript = 0;
#ifdef INSIDE_LINC
				engine.AddTextLine( "End script" , VS_COL_GREY );
				engine.AddTextLine(	"" , VS_COL_GREY );
#endif
				break;

			case CP_PUSH_LOCAL_VAR32:		// 1	Push the contents of a local variable
				Read16ip(parameter)
				DEBUG2("Push local var %d (%d)",parameter,*((int *)(variables+parameter)));
				PUSHONSTACK ( *((int *)(variables+parameter)) );
				break;


			case CP_PUSH_GLOBAL_VAR32:			// 2	Push a global variable
				Read16ip(parameter)
#ifdef INSIDE_LINC
				DEBUG2("Push global var %d (%d)",parameter,g_GlobalVariables.GetLocalByIndex(parameter).GetValue());
				PUSHONSTACK ( g_GlobalVariables.GetLocalByIndex(parameter).GetValue() );
#else
				DEBUG2("Push global var %d (%d)",parameter,globalInterpreterVariables2[parameter]);
				ASSERT(globalInterpreterVariables2);
				PUSHONSTACK ( globalInterpreterVariables2[parameter] );
#endif
				break;

			case CP_POP_LOCAL_VAR32:		// 3	Pop a value into a local word variable
				Read16ip(parameter)
				POPOFFSTACK ( value );
				DEBUG2("Pop %d into var %d",value,parameter);
				*((int *)(variables+parameter)) = value;
				break;

			case CP_CALL_MCODE:			// 4	Call an mcode routine
			{
				Read16ip(parameter)
				ASSERT(parameter <= MAX_FN_NUMBER);
				value = *((const int8 *)(code+ip));			// amount to adjust stack by (no of parameters)
				ip ++;
				DEBUG2("Call mcode %d with stack = %x",parameter,stack2+(stackPointer2-value));
#ifdef INSIDE_LINC
				int retVal = engine.McodeTable(parameter , stack2+(stackPointer2-value));
#else
				int retVal = McodeTable[parameter](stack2+(stackPointer2-value));
#endif
				stackPointer2 -= value;
				CHECKSTACKPOINTER2
				switch ( retVal & 7 )
				{	case IR_STOP:	// 0: Quit out for a cycle
						*offset = ip;
						return(0);

					case IR_CONT:	//	1:	// Continue as normal
						break;

					case IR_TERMINATE:	//	2:
						// Return without updating the offset
						return(2);

					case IR_REPEAT:	//	3:
						// Return setting offset to start of this function call
						*offset = savedStartOfMcode;
						return(0);

					case IR_GOSUB:	//	4:	//that's really neat
						*offset = ip;
						return(2);

					default:
						ASSERT(FALSE);
				}
				parameterReturnedFromMcodeFunction = retVal >> 3;
			}
				break;

			case CP_PUSH_LOCAL_ADDR:	// 5	push the address of a local variable
				Read16ip(parameter)
				DEBUG2("Push address of local variable %d (%x)",parameter,(int32)(variables + parameter));
				PUSHONSTACK ( (int32)(variables + parameter) );
				break;

			case CP_PUSH_INT32:			// 6	Push a long word value on to the stack
				Read32ip(parameter)
				DEBUG2("Push int32 %d (%x)",parameter,parameter);
				PUSHONSTACK ( parameter );
				break;


			case CP_SKIPONFALSE:		//	7	Skip if the value on the stack is false
				Read32ipLeaveip(parameter)
				POPOFFSTACK ( value );
				DEBUG2("Skip %d if %d is false",parameter,value);
				if (value)
					ip += sizeof(int32);
				else
					ip += parameter;
				break;

			case CP_SKIPALLWAYS:		// 8 skip a block
				Read32ipLeaveip(parameter)
				DEBUG1("Skip %d",parameter);
				ip += parameter;
				break;

			case CP_SWITCH:				// 9 switch
			{	POPOFFSTACK ( value );
				int caseCount;
				Read32ip(caseCount)
				// Search the cases
				int foundCase = 0;
				for (int count = 0 ; (count < caseCount) && (!foundCase) ; count++)
				{
					if (value == *((const int32 *)(code+ip)))
					{	// We have found the case, so lets jump to it
						foundCase = 1;
						ip +=  *((const int32 *)(code+ip+sizeof(int32)));
					}
					else
						ip += sizeof(int32) * 2;
				}
				// If we found no matching case then use the default
				if (!foundCase)
				{
					ip += *((const int32 *)(code+ip));
				}
			}
				break;

			case CP_ADDNPOP_LOCAL_VAR32:						// 10
				Read16ip(parameter)
				POPOFFSTACK ( value );
				*((int *)(variables+parameter)) += value;
				DEBUG3("+= %d into var %d->%d",value,parameter,*((int *)(variables+parameter)));
				break;

			case CP_SUBNPOP_LOCAL_VAR32:						// 11
				Read16ip(parameter)
				POPOFFSTACK ( value );
				*((int *)(variables+parameter)) -= value;
				DEBUG3("-= %d into var %d->%d",value,parameter,*((int *)(variables+parameter)));
				break;

			case CP_SKIPONTRUE:									//	12	Skip if the value on the stack is TRUE
				Read32ipLeaveip(parameter)
				POPOFFSTACK ( value );
				DEBUG2("Skip %d if %d is false",parameter,value);
				if (!value)
					ip += sizeof(int32);
				else
					ip += parameter;
				break;

			case CP_POP_GLOBAL_VAR32:							// 13				// Pop a global variable
				Read16ip(parameter)
				POPOFFSTACK ( value );
				DEBUG2("Pop %d into global var %d",value,parameter);
#ifdef INSIDE_LINC
				g_GlobalVariables.lclSet(parameter,value);
				engine.AddTextLine(CVString(	"Set variable %s to %d",
													g_GlobalVariables.GetLocalByIndex(parameter).GetName(),
													g_GlobalVariables.GetLocalByIndex(parameter).GetValue() ),
									VS_COL_GREY);
#else //INSIDE_LINC

#ifdef TRACEGLOBALVARIABLESET
				TRACEGLOBALVARIABLESET(parameter,value);
#endif

				globalInterpreterVariables2[parameter] = value;

#endif
				break;

			case CP_ADDNPOP_GLOBAL_VAR32:						// 14			Add and pop a global variable
			{	Read16ip(parameter)
//				parameter = *((int16_TYPE *)(code+ip));
//				ip += 2;
				POPOFFSTACK ( value );
#ifdef INSIDE_LINC
				int newVal = g_GlobalVariables.GetLocalByIndex(parameter).GetValue() + value ;
				g_GlobalVariables.lclSet(parameter, newVal );
				engine.AddTextLine(	CVString(	"Set variable %s to %d",
													g_GlobalVariables.GetLocalByIndex(parameter).GetName(),
													g_GlobalVariables.GetLocalByIndex(parameter).GetValue() ),
										VS_COL_GREY);
#else
				globalInterpreterVariables2[parameter] += value;
				DEBUG3("+= %d into global var %d->%d",value,parameter,*((int *)(variables+parameter)));
#endif
				break;
			}

			case CP_SUBNPOP_GLOBAL_VAR32:						// 15			Sub and pop a global variable
			{	Read16ip(parameter)
//				parameter = *((int16_TYPE *)(code+ip));
//				ip += 2;
				POPOFFSTACK ( value );
#ifdef INSIDE_LINC
				int newVal = g_GlobalVariables.GetLocalByIndex(parameter).GetValue() - value ;
				g_GlobalVariables.lclSet(parameter, newVal );
				engine.AddTextLine(	CVString(	"Set variable %s to %d",
													g_GlobalVariables.GetLocalByIndex(parameter).GetName(),
													g_GlobalVariables.GetLocalByIndex(parameter).GetValue() ),
										VS_COL_GREY);
#else
				globalInterpreterVariables2[parameter] -= value;
				DEBUG3("-= %d into global var %d->%d",value,parameter,*((int *)(variables+parameter)));
#endif
				break;
			}

			case CP_DEBUGON:
				// Turn debugging on
				g_debugFlag = 1;
				break;

			case CP_DEBUGOFF:
				// Turn debugging on
				g_debugFlag = 0;
				break;

			case CP_QUIT:
#ifdef INSIDE_LINC
				break;
#else
				// Quit out for a cycle
				*offset = ip;
				return(0);
#endif

			case CP_TERMINATE:
				// Quit out immediately without affecting the offset pointer
				return(3);

/******************************************************************************************************************
******************************************************************************************************************/

			// Operators

			case OP_ISEQUAL:	//	20				// '=='
				DEBUG3("%d == %d -> %d",	stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] == stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] == stack2[stackPointer2-1]) );
				break;

			case OP_PLUS:		//	21				// '+'
				DEBUG3("%d + %d -> %d",		stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] + stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] + stack2[stackPointer2-1]) );
				break;

			case OP_MINUS:		//	22				// '+'
				DEBUG3("%d - %d -> %d",		stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] - stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] - stack2[stackPointer2-1]) );
				break;

			case OP_TIMES:		//	23				// '+'
				DEBUG3("%d * %d -> %d",		stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] * stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] * stack2[stackPointer2-1]) );
				break;

			case OP_DEVIDE:		//	24				// '+'
				DEBUG3("%d / %d -> %d",		stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] / stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] / stack2[stackPointer2-1]) );
				break;

			case OP_NOTEQUAL:	//	25				// '!='
				DEBUG3("%d != %d -> %d",	stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] != stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] != stack2[stackPointer2-1]) );
				break;

			case OP_ANDAND:		// 26
				DEBUG3("%d != %d -> %d",	stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] && stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] && stack2[stackPointer2-1]) );
				break;

			case OP_GTTHAN:		// 27 >
				DEBUG3("%d > %d -> %d",		stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] > stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] > stack2[stackPointer2-1]) );
				break;

			case OP_LSTHAN:		// 28 <
				DEBUG3("%d < %d -> %d",		stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] < stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] < stack2[stackPointer2-1]) );
				break;

			case CP_JUMP_ON_RETURNED:	// 29
			{	// Jump to a part of the script depending on the return value from an mcode routine
				parameter = *((const int8 *)(code+ip));		// Get the maximum value
				ip++;
#ifdef INSIDE_LINC
				TRACE("ip %d: Parameter %d skip %d\r\n",	ip,
															parameterReturnedFromMcodeFunction,
															((const int32 *)(code+ip))[parameterReturnedFromMcodeFunction] );
#endif

				ip += ((const int32 *)(code+ip))[parameterReturnedFromMcodeFunction];
			}
				break;

			case CP_TEMP_TEXT_PROCESS:	// 30
				// Process a text line
				Read32ip(parameter)
//				parameter = *((int32_TYPE *)(code+ip));
//				ip += sizeof(int32_TYPE);;
				DEBUG1("Process text id %d",parameter);
#ifdef INSIDE_LINC
				// Linc only for the moment
				engine.ProcessTextLine(parameter);
#endif //INSIDE_LINC
				break;

			case CP_SAVE_MCODE_START:	// 31
				// Save the start position on an mcode instruction in case we need to restart it again
				savedStartOfMcode = ip-1;
				break;

			case CP_RESTART_SCRIPT:	// 32
			{	// Start the script again
				// Do a ip search to find the script we are running
				const char *tempScrPtr = scriptData + *((int *)scriptData) + sizeof(int);
				int scriptNumber = 0;
				int foundScript = 0;
				uint32 count = 0;
				for (count = 1 ; (count < noScripts) && (!foundScript) ; count++)
				{	if (ip < ((const int *)tempScrPtr)[count+1])
					{	scriptNumber = count - 1 ;
						foundScript = 1;
					}
				}
				if (!foundScript)
					scriptNumber = count - 1 ;
				// So we know what script we are running, lets restart it
				ip = ((const int *)tempScrPtr)[scriptNumber+1];
				break;
			}

			case CP_PUSH_STRING:	// 33
			{	// Push the address of a string on to the stack
				parameter = *((const int8 *)(code+ip));		// Get the string size
				ip += 1;
				// ip points to the string
				PUSHONSTACK( (int)(code+ip) );
				ip += (parameter+1);
				break;
			}

			case CP_PUSH_DEREFERENCED_STRUCTURE:	// 34
			{	// Push the address of a dereferenced structure
				Read32ip(parameter)
				DEBUG1("Push address of far variable (%x)",(int32)(variables + parameter));
				PUSHONSTACK( (int)(objectData + sizeof(int) + sizeof(_standardHeader) + sizeof(_object_hub) + parameter));
				break;
			}

			case OP_GTTHANE:		// 35 >=
				DEBUG3("%d > %d -> %d",		stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] >= stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] >= stack2[stackPointer2-1]) );
				break;

			case OP_LSTHANE:		// 36 <=
				DEBUG3("%d < %d -> %d",		stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] <= stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] <= stack2[stackPointer2-1]) );
				break;

			case OP_OROR:			// 37
				DEBUG3("%d || %d -> %d",	stack2[stackPointer2-2],
											stack2[stackPointer2-1],
											stack2[stackPointer2-2] || stack2[stackPointer2-1]);
				DOOPERATION ( (stack2[stackPointer2-2] || stack2[stackPointer2-1]) );
				break;


			default:
#ifdef INSIDE_LINC
				AfxMessageBox(CVString("Invalid interpreter token %d",curCommand));
#else
				Con_fatal_error("Interpreter error: Invalid token %d", curCommand);
#endif
				return(3);
		}

	}

	return(1);
}
