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

#include "stdafx.h"
//#include "src\driver96.h"
#include "anims.h"
#include "console.h"
#include "controls.h"	// for 'subtitles' & 'speechSelected'
#include "debug.h"
#include "defs.h"
#include "events.h"
#include "function.h"
#include "interpreter.h"
#include "layers.h"	// for 'this_screen'
#include "logic.h"
#include "maketext.h"
#include "memory.h"
#include "mouse.h"
#include "object.h"
#include "protocol.h"
#include "resman.h"
#include "sound.h"
#include "speech.h"
#include "walker.h"
//------------------------------------------------------------------------------------

#define	INS_talk				1
#define	INS_anim				2
#define	INS_reverse_anim		3
#define	INS_walk				4
#define	INS_turn				5
#define	INS_face				6
#define	INS_trace				7
#define	INS_no_sprite			8
#define	INS_sort				9
#define	INS_foreground			10
#define	INS_background			11
#define	INS_table_anim			12
#define	INS_reverse_table_anim	13
#define	INS_walk_to_anim		14
#define	INS_set_frame			15
#define	INS_stand_after_anim	16

#define	INS_quit				42

//------------------------------------------------------------------------------------

uint32	speech_time=0;	//when not playing a wav we calculate the speech time based upon length of ascii

uint32	speech_text_bloc_no=0;
uint32	anim_id=0;
uint32	speech_anim_type;	//0 lip synced and repeating - 1 normal once through
uint32	left_click_delay=0;		// click-delay for LEFT mouse button
uint32	right_click_delay=0;	// click-delay for RIGHT mouse button

uint32	default_response_id=0;	// ref number for default response when luggage icon is used on a person
								// & it doesn't match any of the icons which would have been in the chooser
int16 officialTextNumber=0;		// "TEXT" - current official text line number - will match the wav filenames

int32 speechScriptWaiting = 0;		// usually 0; if non-zero then it's the id of whoever we're waiting for in a speech script
									// see FN_they_do, FN_they_do_we_wait & FN_we_wait

int16 text_x, text_y;	// calculated by LocateTalker() for use in speech-panning & text-sprite positioning

_subject_unit	subject_list[MAX_SUBJECT_LIST];


//------------------------------------------------------------------------------------
// local function prototypes

int32	FN_i_speak(int32 *params);
void	LocateTalker(int32	*params);	// (James 01july97)
void	Form_text(int32	*params);		//Tony18Oct96
BOOL	Is_anim_boxed(uint32 res);		//Tony20Oct96
uint8 WantSpeechForLine(uint32 wavId);	// James (29july97)

#ifdef _DEBUG
void GetCorrectCdForSpeech(int32 wavId);	// for testing speech & text
#endif

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
int32 FN_add_subject(int32 *params)	// James12nov96 / Tony18Nov96
{
//	param[0] id
// param[1] daves reference number

	if (IN_SUBJECT==0)			// if this is the start of the new subject list	(James 07may97)
		default_response_id=0;	// set the default repsonse id to zero in case we're never passed one
								// - this just means we'd get the response for the 1st icon in the chooser
								//   which is better than crashing

	if (params[0] == -1)		// this isn't an icon at all, it's telling us the id of the default response
	{
		default_response_id = params[1];	// and here it is - this is the ref number we will return if
											// a luggage icon is clicked on someone when it wouldn't have
											// been in the chooser list (see FN_choose below)
	}
	else
	{
		subject_list[IN_SUBJECT].res=params[0];
		subject_list[IN_SUBJECT].ref=params[1];

//		Zdebug("FN_add_subject res %d, uid %d", params[0], params[1]);

		IN_SUBJECT+=1;
	}


	return(IR_CONT);			// continue script
}
//------------------------------------------------------------------------------------
int	choosing=0;	//could alternately use logic->looping of course

int32 FN_choose(int32 *params)	//Tony19Nov96
{
//no params

//the human is switched off so there will be no normal mouse engine


	_mouseEvent	*me;
	uint32	j,hit;
	uint8	*icon;
	uint32	pos=0;

//	Zdebug("into choose");

	
	AUTO_SELECTED=0;	// see below (James23may97)


	//-------------------------------------------
	// new thing to intercept objects held at time of clicking on a person
	// (James 06may97)

	if (OBJECT_HELD)	// if we are using a luggage icon on the person
	{
		// scan the subject list to see if this icon would have been available at this time
 		// if it is there, return the relevant 'ref' number (as if it had been selected from within the conversation)
		// if not, just return a special code to get the default text line(s) for unsupported objects
		// Note that we won't display the subject icons in this case!

		while (pos < IN_SUBJECT)	// scan the subject list for a match with our 'object_held'
		{
			if (subject_list[pos].res == OBJECT_HELD)		// if we've found a match
			{
				OBJECT_HELD=0;								// clear it so it doesn't keep happening!
				IN_SUBJECT=0;								// clear the subject list
				return(IR_CONT+(subject_list[pos].ref<<3));	// return special subject chosen code (same as in normal chooser routine below)
			}
			pos++;				// next position
		}

		OBJECT_HELD=0;		// clear it so it doesn't keep happening!
		IN_SUBJECT=0;		// clear the subject list
		return(IR_CONT+(default_response_id<<3));	// so that the speech script uses the default text for objects that are not accounted for
	}
	//-------------------------------------------
	// new thing for skipping chooser with "nothing else to say" text
	// (James 23may97)

	// If this is the 1st time the chooser is coming up in this conversation
	// AND there's only 1 subject
	// AND it's the EXIT icon
	if ((CHOOSER_COUNT_FLAG==0) && (IN_SUBJECT==1) && (subject_list[0].res == EXIT_ICON))
	{
		AUTO_SELECTED=1;	// for speech script

		IN_SUBJECT=0;								// clear the subject list
		return(IR_CONT+(subject_list[0].ref<<3));	// return special subject chosen code (same as in normal chooser routine below)
	}
	//-------------------------------------------


	if	(!choosing)	//new choose session
	{


//		build menus from subject_list

		if	(!IN_SUBJECT)
			Con_fatal_error("FN_choose with no subjects :-O");

//init top menu from master list
		for	(j=0;j<15;j++)	//all icons are highlighted / full colour
		{
			if	(j<IN_SUBJECT)
			{
//				Zdebug(" ICON res %d for %d", subject_list[j].res, j);
				icon = res_man.Res_open( subject_list[j].res ) + sizeof(_standardHeader) + RDMENU_ICONWIDE*RDMENU_ICONDEEP;
				SetMenuIcon(RDMENU_BOTTOM, j, icon);
				res_man.Res_close( subject_list[j].res );
			}
			else
			{	SetMenuIcon(RDMENU_BOTTOM, j, NULL);	//no icon here
				//Zdebug(" NULL for %d", j);
			}
		}

//		start menus appearing
		ShowMenu(RDMENU_BOTTOM);


//		lets have the mouse pointer back
		Set_mouse(NORMAL_MOUSE_ID);

		choosing=1;

		return(IR_REPEAT);		//again next cycle
	}

	else	//menu is there - we're just waiting for a click
	{
//		Zdebug("choosing");
		me = MouseEvent();	//get mouse event

//		we only care about left clicks
//		we ignore mouse releases

		if	((me!=NULL)&&(me->buttons&RD_LEFTBUTTONDOWN))
		{
//			check for click on a menu
//			if so then end the choose, highlight only the chosen, blank the mouse and return the ref code *8

			if	((mousey>399)&&(mousex>=24)&&(mousex<640-24))
			{
				hit=(mousex-24)/40;	//which are we over?

				if	(hit<IN_SUBJECT)	//clicked on something - what button?
				{
		   			//--------------------------------------
					// Write to walkthrough file (zebug0.txt)
					#ifdef _DEBUG
 					Zdebug(0,"----------------------");
					Zdebug(0,"Icons available:");
					#endif
 		   			//--------------------------------------

					for	(j=0;j<IN_SUBJECT;j++)	//change icons
					{
				   		//--------------------------------------
						// Write to walkthrough file (zebug0.txt)
						#ifdef _DEBUG
						Zdebug(0,"%s", FetchObjectName(subject_list[j].res));
						#endif
	 		   			//--------------------------------------

						if	(j!=hit)	//change all others to grey
						{
							icon = res_man.Res_open( subject_list[j].res ) + sizeof(_standardHeader);	//now grey
							SetMenuIcon(RDMENU_BOTTOM, j, icon);
							res_man.Res_close( subject_list[j].res );
						}
					}

		   			//--------------------------------------
					// Write to walkthrough file (zebug0.txt)
					#ifdef _DEBUG
					Zdebug(0,"Selected: %s", FetchObjectName(subject_list[hit].res));
					Zdebug(0,"----------------------");
   					#endif
 		   			//--------------------------------------

					choosing=0;	//this is our looping flag

					IN_SUBJECT=0;

					Set_mouse(NULL);	//blank mouse again

//					Zdebug("hit %d - ref %d  ref*8 %d", hit, subject_list[hit].ref, subject_list[hit].ref*8);

					RESULT=subject_list[hit].res;	//for non-speech scripts that manually call the chooser

					return(IR_CONT+(subject_list[hit].ref<<3));		//return special subject chosen code
				}
			}
		}
//		Zdebug("end choose");
		return(IR_REPEAT);		//again next cycle
	}
}
//------------------------------------------------------------------------------------
int32 FN_start_conversation(int32 *params)	//Tony27Nov96
{
//Start conversation

//FN_no_human();		// an FN_no_human
//FN_change_speech_text (PLAYER,  GEORGE_WIDTH,  GEORGE_PEN );

//params:	0 <empty>

//	Zdebug("FN_start_conversation %d", ID);

	//--------------------------------------------------------------
	// reset 'chooser_count_flag' at the start of each conversation:

	// Note that FN_start_conversation might accidently be called
	// every time the script loops back for another chooser
	// but we only want to reset the chooser count flag the first time this function is called
	// ie. when talk flag is zero
	if (TALK_FLAG==0)
		CHOOSER_COUNT_FLAG=0;	// see FN_chooser & speech scripts
 	//--------------------------------------------------------------

	FN_no_human(params);

	return(IR_CONT);
}
//------------------------------------------------------------------------------------
int32 FN_end_conversation(int32 *params)	//Tony27Nov96
{
//end conversation
//talk_flag=0;
//FN_end_chooser();
//FN_add_human();
//FN_change_speech_text (PLAYER,  VOICE_OVER_WIDTH,  VOICE_OVER_PEN );
//FN_idle();
//params:	0 <empty>


//	Zdebug("FN_end_conversation");

	HideMenu(RDMENU_BOTTOM);

	if	(mousey>399)
	{	mouse_mode=MOUSE_holding;	//will wait for cursor to move off the bottom menu
		Zdebug("   holding");
	}

	TALK_FLAG=0;	//in-case DC forgets


// restart george's base script
//	LLogic.Total_restart();

	if (params);

	return(IR_CONT);	//drop out without saving pc and go around again
}
//------------------------------------------------------------------------------------
int32 FN_they_do(int32	*params)	//S2.1(18Jan95tw)	Tony3Dec96
{
//doesn't send the command until target is waiting - once sent we carry on

//params	0 target
//			1 command
//			2 ins1
//			3 ins2
//			4 ins3
//			5 ins4
//			6 ins5

	uint32	null_pc=5;	//4th script - get-speech-state
	char	*raw_script_ad;
	_standardHeader	*head;
	int32	target=params[0];


//request status of target
	head = (_standardHeader*) res_man.Res_open(target);
	if	(head->fileType!=GAME_OBJECT)
		Con_fatal_error("FN_they_do %d not an object", target);

	raw_script_ad = (char *)head; // (head+1) + sizeof(_object_hub);	//get to raw script data

	RunScript( raw_script_ad, raw_script_ad, &null_pc );	//call the base script - this is the graphic/mouse service call
	res_man.Res_close(target);

//result is 1 for waiting, 0 for busy

	if	((RESULT==1)&&(!INS_COMMAND))	//its waiting and no other command is queueing
	{
		speechScriptWaiting = 0;	// reset debug flag now that we're no longer waiting - see debug.cpp

		SPEECH_ID	= params[0];
		INS_COMMAND	= params[1];
		INS1		= params[2];
		INS2		= params[3];
		INS3		= params[4];
		INS4		= params[5];
		INS5		= params[6];

		return(IR_CONT);	//script cont
	}

	speechScriptWaiting = target;	// debug flag to indicate who we're waiting for - see debug.cpp
	return(IR_REPEAT);					// target is busy so come back again next cycle
}
//------------------------------------------------------------------------------------
int32 FN_they_do_we_wait(int32	*params)	//Tony3Dec96
{
//give target a command and wait for it to register as finished

//params	0 pointer to ob_logic
//			1 target
//			2 command
//			3 ins1
//			4 ins2
//			5 ins3
//			6 ins4
//			7 ins5

//'looping' flag is used as a sent command yes/no

	Object_logic	*ob_logic;

	uint32	null_pc=5;	//4th script - get-speech-state
	char	*raw_script_ad;
	_standardHeader	*head;
	int32	target=params[1];


//	Zdebug("FN_they_do_we_wait id %d, command %d", params[1], params[2]);


//	ok, see if the target is busy - we must request this info from the target object
	head = (_standardHeader*) res_man.Res_open(target);
	if	(head->fileType!=GAME_OBJECT)
		Con_fatal_error("FN_they_do_we_wait %d not an object", target);

	raw_script_ad = (char *)head; // (head+1) + sizeof(_object_hub);	//get to raw script data

	RunScript( raw_script_ad, raw_script_ad, &null_pc );	//call the base script - this is the graphic/mouse service call
	res_man.Res_close(target);





	ob_logic = (Object_logic *)params[0];

	if ((!INS_COMMAND)&&(RESULT==1)&&(ob_logic->looping==0))	//first time so set up targets command if target is waiting
	{

//		Zdebug("FNtdww sending command to %d", target);

		SPEECH_ID	= params[1];
		INS_COMMAND	= params[2];
		INS1		= params[3];
		INS2		= params[4];
		INS3		= params[5];
		INS4		= params[6];
		INS5		= params[7];

		ob_logic->looping=1;

		speechScriptWaiting = target;	// debug flag to indicate who we're waiting for - see debug.cpp
		return(IR_REPEAT);						// finish this cycle - but come back again to check for it being finished
	}
	else	if	(ob_logic->looping==0)	//did not send the command
	{
		speechScriptWaiting = target;	// debug flag to indicate who we're waiting for - see debug.cpp
		return(IR_REPEAT);				// come back next go and try again to send the instruction
	}


//ok, the command has been sent - has the target actually done it yet?

//result is 1 for waiting, 0 for busy

	if	(RESULT==1)							// its waiting now so we can be finished with all this
	{
//		Zdebug("FNtdww finished");
		ob_logic->looping=0;				// not looping anymore

		speechScriptWaiting = 0;			// reset debug flag now that we're no longer waiting - see debug.cpp
		return(IR_CONT);					// script cont
	}

//	Zdebug("FNtdww just waiting");

	speechScriptWaiting = target;	// debug flag to indicate who we're waiting for - see debug.cpp
	return(IR_REPEAT);						// see ya next cycle
}
//------------------------------------------------------------------------------------
int32 FN_we_wait(int32	*params)	//Tony3Dec96
{
//loop until the target is free

//params	0 target

	uint32	null_pc=5;	//4th script - get-speech-state
	char	*raw_script_ad;
	_standardHeader	*head;
	int32	target=params[0];

//request status of target
	head = (_standardHeader*) res_man.Res_open(target);
	if	(head->fileType!=GAME_OBJECT)
		Con_fatal_error("FN_we_wait %d not an object", target);

	raw_script_ad = (char *)head; // (head+1) + sizeof(_object_hub);	//get to raw script data

	RunScript( raw_script_ad, raw_script_ad, &null_pc );	//call the base script - this is the graphic/mouse service call
	res_man.Res_close(target);

//result is 1 for waiting, 0 for busy

	if	(RESULT==1)
	{
		speechScriptWaiting = 0;			// reset debug flag now that we're no longer waiting - see debug.cpp
		return(IR_CONT);					// script cont
	}

	speechScriptWaiting = target;	// debug flag to indicate who we're waiting for - see debug.cpp
	return(IR_REPEAT);						// target is busy so come back again next cycle
}
//------------------------------------------------------------------------------------
int32 FN_timed_wait(int32	*params)	//Tony12Dec96
{
//loop until the target is free but only while the timer is high
//useful when clicking on a target to talk to them - if they never reply then this'll fall out avoiding a lock up

//params	0 ob_logic
//			1 target
//			2 number of cycles before give up

	uint32	null_pc=5;	//4th script - get-speech-state
	char	*raw_script_ad;
	Object_logic	*ob_logic;
	_standardHeader	*head;
	int32	target=params[1];


	ob_logic = (Object_logic *)params[0];


	if	(!ob_logic->looping)
		ob_logic->looping=params[2];	//first time in


//request status of target
	head = (_standardHeader*) res_man.Res_open(target);
	if	(head->fileType!=GAME_OBJECT)
		Con_fatal_error("FN_timed_wait %d not an object", target);

	raw_script_ad = (char *)head; // (head+1) + sizeof(_object_hub);	//get to raw script data

	RunScript( raw_script_ad, raw_script_ad, &null_pc );	//call the base script - this is the graphic/mouse service call
	res_man.Res_close(target);

//result is 1 for waiting, 0 for busy

	ob_logic->looping--;

	if	(RESULT==1)	//its waiting
	{
		ob_logic->looping=0;	//reset because counter is likely to be still high
		RESULT=0;	//means ok

		speechScriptWaiting = 0;	// reset debug flag now that we're no longer waiting - see debug.cpp
		return(IR_CONT);			// script cont
	}


	if	(!ob_logic->looping)	//time up - caller must check RESULT
	{

		RESULT=1;	//not ok

//		kill the event

		Kill_all_ids_events(target);	//clear the event that hasn't been picked up - in theory, none of this should ever happen

		Zdebug("EVENT timed out");

		speechScriptWaiting = 0;	// reset debug flag now that we're no longer waiting - see debug.cpp
		return(IR_CONT);	//script cont
	}

	speechScriptWaiting = target;	// debug flag to indicate who we're waiting for - see debug.cpp
	return(IR_REPEAT);				// target is busy so come back again next cycle
}
//------------------------------------------------------------------------------------
int32	FN_speech_process(int32	*params)	//Tony5Dec96
{
//recieve and sequence the commands sent from the conversation script
//we have to do this in a slightly tweeky manner as we can no longer have generic scripts
//this function comes in with all the structures that will be required

//param	0 pointer to ob_graphic
//param	1 pointer to ob_speech
//param	2 pointer to ob_logic
//param	3 pointer to ob_mega
//param	4 pointer to ob_walkdata

//note - we could save a var and ditch wait_state and check 'command' for non zero means busy

	Object_speech	*ob_speech;

	int32	pars[9];
	int32	ret;


	ob_speech = (Object_speech*) params[1];


//	Zdebug("  SP");

	while(1)
	{
		if	(ob_speech->command)	//we are currently running a command
		{
			switch(ob_speech->command)
			{
				//----------------------------------------------
				case	INS_talk:

					pars[0]=params[0];	//ob_graphic
					pars[1]=params[1];	//ob_speech
					pars[2]=params[2];	//ob_logic
					pars[3]=params[3];	//ob_mega

					pars[4]=ob_speech->ins1;	//param	4 encoded text number
					pars[5]=ob_speech->ins2;	//param	5 wav res id
					pars[6]=ob_speech->ins3;	//param	6 anim res id
					pars[7]=ob_speech->ins4;	//param	7 anim table res id
					pars[8]=ob_speech->ins5;	//param	8 animation mode	0 lip synced, 1 just straight animation

				//	Zdebug("speech-process talk");

					ret = FN_i_speak(pars);	//run the function - (it thinks its been called from script - bloody fool)

					if	(ret!=IR_REPEAT)
					{	ob_speech->command=0;	//command finished
						ob_speech->wait_state=1;	//waiting for command
//						Zdebug("speech-process talk finished");
					}
					return(IR_REPEAT);	//come back again next cycle

				//----------------------------------------------
				case	INS_turn:

					pars[0]=params[2];			// ob_logic
					pars[1]=params[0];			// ob_graphic
					pars[2]=params[3];			// ob_mega
					pars[3]=params[4];			// ob_walkdata
					pars[4]=ob_speech->ins1;	// direction to turn to

					ret = FN_turn(pars);

					if	(ret!=IR_REPEAT)
					{	ob_speech->command=0;	//command finished
						ob_speech->wait_state=1;	//waiting for command
					}
					return(IR_REPEAT);	//come back again next cycle

				//----------------------------------------------
				case	INS_face:

					pars[0]=params[2];			// ob_logic
					pars[1]=params[0];			// ob_graphic
					pars[2]=params[3];			// ob_mega
					pars[3]=params[4];			// ob_walkdata
					pars[4]=ob_speech->ins1;	// target

					ret = FN_face_mega(pars);

					if	(ret!=IR_REPEAT)
					{	ob_speech->command=0;	//command finished
						ob_speech->wait_state=1;	//waiting for command
					}
					return(IR_REPEAT);	//come back again next cycle

				//----------------------------------------------
				case	INS_anim:

					pars[0]=params[2];	//ob_logic
					pars[1]=params[0];	//ob_graphic
					pars[2]=ob_speech->ins1;	//anim res

					ret= FN_anim(pars);

					if	(ret!=IR_REPEAT)
					{	ob_speech->command=0;	//command finished
						ob_speech->wait_state=1;	//waiting for command
					}
					return(IR_REPEAT);	//come back again next cycle

				//----------------------------------------------
				case	INS_reverse_anim:

					pars[0]=params[2];	//ob_logic
					pars[1]=params[0];	//ob_graphic
					pars[2]=ob_speech->ins1;	//anim res

					ret= FN_reverse_anim(pars);

					if	(ret!=IR_REPEAT)
					{	ob_speech->command=0;	//command finished
						ob_speech->wait_state=1;	//waiting for command
					}
					return(IR_REPEAT);	//come back again next cycle

				//----------------------------------------------
				case	INS_table_anim:

					pars[0]=params[2];	//ob_logic
					pars[1]=params[0];	//ob_graphic
					pars[2]=params[3];	//ob_mega
					pars[3]=ob_speech->ins1;	//pointer to anim table

					ret= FN_mega_table_anim(pars);

					if	(ret!=IR_REPEAT)
					{	ob_speech->command=0;		//command finished
						ob_speech->wait_state=1;	//waiting for command
					}
					return(IR_REPEAT);	//come back again next cycle

				//----------------------------------------------
				case	INS_reverse_table_anim:

					pars[0]=params[2];	//ob_logic
					pars[1]=params[0];	//ob_graphic
					pars[2]=params[3];	//ob_mega
					pars[3]=ob_speech->ins1;	//pointer to anim table

					ret= FN_reverse_mega_table_anim(pars);

					if	(ret!=IR_REPEAT)
					{	ob_speech->command=0;		//command finished
						ob_speech->wait_state=1;	//waiting for command
					}
					return(IR_REPEAT);	//come back again next cycle

				//----------------------------------------------
				case	INS_no_sprite:

					FN_no_sprite(params);		// ob_graphic;

					ob_speech->command=0;		//command finished
					ob_speech->wait_state=1;	//waiting for command

					return(IR_REPEAT);

				//----------------------------------------------
				case	INS_sort:

					FN_sort_sprite(params);		// ob_graphic;

					ob_speech->command=0;		//command finished
					ob_speech->wait_state=1;	//waiting for command

					return(IR_REPEAT);

				//----------------------------------------------
				case	INS_foreground:

					FN_fore_sprite(params);		// ob_graphic;

					ob_speech->command=0;		//command finished
					ob_speech->wait_state=1;	//waiting for command

					return(IR_REPEAT);

				//----------------------------------------------
				case	INS_background:

					FN_back_sprite(params);		// ob_graphic;

					ob_speech->command=0;		//command finished
					ob_speech->wait_state=1;	//waiting for command

					return(IR_REPEAT);

				//----------------------------------------------
				case	INS_walk:

					pars[0]=params[2];	//ob_logic
					pars[1]=params[0];	//ob_graphic
					pars[2]=params[3];	//ob_mega
					pars[3]=params[4];	//ob_walkdata

					pars[4]=ob_speech->ins1;	//target x
					pars[5]=ob_speech->ins2;	//target y
					pars[6]=ob_speech->ins3;	//target direction

					ret= FN_walk(pars);

					if	(ret!=IR_REPEAT)
					{	ob_speech->command=0;	//command finished
						ob_speech->wait_state=1;	//waiting for command
//						Zdebug("speech-process walk finished");
					}
					return(IR_REPEAT);	//come back again next cycle

				//----------------------------------------------
				case	INS_walk_to_anim:

					pars[0]=params[2];	//ob_logic
					pars[1]=params[0];	//ob_graphic
					pars[2]=params[3];	//ob_mega
					pars[3]=params[4];	//ob_walkdata

					pars[4]=ob_speech->ins1;	// anim resource

					ret= FN_walk_to_anim(pars);

					if	(ret!=IR_REPEAT)
					{	ob_speech->command=0;	//command finished
						ob_speech->wait_state=1;	//waiting for command
//						Zdebug("speech-process walk finished");
					}
					return(IR_REPEAT);	//come back again next cycle

				//----------------------------------------------
				case	INS_stand_after_anim:

					pars[0]=params[0];			// ob_graphic
					pars[1]=params[3];			// ob_mega
					pars[2]=ob_speech->ins1;	// anim resource

					ret= FN_stand_after_anim(pars);

					ob_speech->command=0;		// command finished
					ob_speech->wait_state=1;	// waiting for command

					return(IR_REPEAT);	// come back again next cycle

				//----------------------------------------------
				case	INS_set_frame:

					pars[0]=params[0];			// ob_graphic
					pars[1]=ob_speech->ins1;	// anim_resource
					pars[2]=ob_speech->ins2;	// FIRST_FRAME or LAST_FRAME

					ret= FN_set_frame(pars);

					ob_speech->command=0;	//command finished
					ob_speech->wait_state=1;	//waiting for command

					return(IR_REPEAT);	//come back again next cycle

				//----------------------------------------------
				case	INS_quit:

//					Zdebug("speech-process - quit");
					ob_speech->command=0;	//finish with all this
				//	ob_speech->wait_state=0;	//start with waiting for command next conversation
					return(IR_CONT);	//thats it, we're finished with this

				//----------------------------------------------
				default:

					ob_speech->command=0;	//not yet implemented - just cancel
					ob_speech->wait_state=1;	//waiting for command
					break;

				//----------------------------------------------
			}
		}



		if	(SPEECH_ID==ID)	//new command for us!
		{

			SPEECH_ID=0;	//clear this or it could trigger next go

//			grab the command - potentially, we only have this cycle to do this
			ob_speech->command	= INS_COMMAND;
			ob_speech->ins1		= INS1;
			ob_speech->ins2		= INS2;
			ob_speech->ins3		= INS3;
			ob_speech->ins4		= INS4;
			ob_speech->ins5		= INS5;

			INS_COMMAND=0;	//the current send has been recieved - i.e. seperate multiple they-do's

			ob_speech->wait_state=0;	//now busy

//			Zdebug("received new command %d", INS_COMMAND);

//			we'll drop off and be caught by the while(1), so kicking in the new command straight away

		}
		else	//no new command
		{
//			we could run a blink anim (or something) here

			ob_speech->wait_state=1;	//now free
			return(IR_REPEAT);	//come back again next cycle
		}
	}


}
//------------------------------------------------------------------------------------




















//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
#if	1==2


 ooo  l     dddd         sss  ttttt  u   u fffff fffff
o   o l     d   d       s       t    u   u f     f
o   o l     d   d        sss    t    u   u ffff  ffff
o   o l     d   d           s   t    u   u f     f
o   o l     d   d       s   s   t    u   u f     f
o   o l     d   d       s   s   t    u   u f     f
 ooo   llll dddd         sss    t     uuu  f     f




// ScriptReference 0,19
mega_interact			//standard interact protocol for megas who have been clicked on
{									//replaces S2's mega_sss...		(Jan95tw)

	o_down_flag=0;	//reset on entry - we dont know what this flag will be
									//we will not be wiping a command because the player will
									//not start until we signal - at the end of this loop...
									//"smart" - readers voice
	do
	{
		if (o_down_flag!=0)			//recieved any instructions?

			switch(o_down_flag)
			{
				//--------------------------------------------
				case	INS_talk:		// (o_ins3=cdt or anim table, o_ins2=text id, o_ins1=graphic or 0)
				{
					FN_I_speak(o_ins3, o_ins2, o_ins1);
					o_down_flag=0;
				}
				break;

				//--------------------------------------------
				case	INS_anim:	//run coord-less anim		(o_ins1=cdt or anim table, o_ins2=graphic or 0, o_ins3=0)
				{
					FN_anim( o_ins1, o_ins2 );
					o_down_flag=0;
				}
				break;

				//--------------------------------------------

				case	INS_full_anim:	//run anim with coords	(o_ins1=cdt o_ins2=graphic o_ins3=0)
				{
					FN_full_anim( o_ins1, o_ins2 );
					o_down_flag=0;
				}
				break;

				//--------------------------------------------
				case	INS_walk:							// walk to (o_ins1,o_ins2) & stand in direction 'o_ins3'
				{
					FN_walk(o_ins1,o_ins2,o_ins3,STAND);
					o_down_flag=0;
				}
				break;

				//--------------------------------------------
				case	INS_turn:				//turn/stand as specified
				{
					FN_turn(o_ins1,STAND);	// turn to direction 'o_ins1'
					o_down_flag=0;
				}
				break;

				//--------------------------------------------
				case	INS_face:				//turn to face specified mega character
				{
					FN_face(o_ins1);
					o_down_flag=0;
				}
				break;

				//--------------------------------------------
				case	INS_trace:				// face compact id 'o_ins1' - continues until next command (because 'o_down_flag' not reset to zero)
				{
					FN_face(o_ins1);
					o_down_flag=INS_trace;	// do this again & again until a new command is received
				}
				break;

				//--------------------------------------------
				case	INS_no_sprite:
				{
					FN_no_sprite();
					o_down_flag=0;
				}
				break;

				//--------------------------------------------
				case	INS_sort:
				{
					FN_sort();
					o_down_flag=0;
				}
				break;

				//--------------------------------------------
				case	INS_foreground:
				{
					FN_foreground();
					o_down_flag=0;
				}
				break;

				//--------------------------------------------
				case	INS_background:
				{
					FN_background();
					o_down_flag=0;
				}
				break;

				//--------------------------------------------
				case	INS_quit:
				{
				}
				break;						//fall out of outer loop

				//--------------------------------------------
				default:
				{
					FN_talk_error();	//shut the system down - I can't be bothered to recover from this
				}
				break;

				//--------------------------------------------
			}


		FN_add_talk_wait_status_bit();			// add waiting bit to status for player to pickup
		FN_quit();													// drop out of script until next cycle to see if new instructions have arrived
		FN_remove_talk_wait_status_bit();		// off again, in case command received

	}
	while(o_down_flag!=INS_quit);	//until we're told to terminate


	FN_goto_bookmark();						//get the mega going again
}
//-----------------------------------------------------------------------------------------
// ScriptReference 0,20

mega_approach_script	// George runs this script to get over to 'target_mega', who's already running 'mega_interact'
{

	//FN_they_do_we_wait(target_mega,INS_trace,GEORGE,0,0);	// 'target_mega' repeatedly faces george while he walks over

	FN_they_do_we_wait(target_mega,INS_face,GEORGE,0,0);	// Mega turns to face George

	FN_get_pos(target_mega);
	target_x = return_value;
	target_y = return_value_2;
	distance_apart = return_value_4;

	walk_attempt = 0;	// keep count of attempts

	if ((o_xcoord) < target_x)	// If George is currently left of Duane
		walk_flag = 0;	// try LHS first
	else
		walk_flag = 1;	// try RHS first


	//--------------------------------------------------------
	do
	{
		if (walk_flag==0)		// (0) try LHS - & face DOWN_RIGHT
		{
			FN_walk(target_x-distance_apart,target_y,DOWN_RIGHT,STAND);

			if (o_down_flag)
				walk_flag=2;	// made it
			else
				walk_flag=1;	// try RHS next, or fail (to anywhere)
		}

		else								// (1) try RHS & face DOWN_LEFT
		{
			FN_walk(target_x+distance_apart,target_y,DOWN_LEFT,STAND);

			if (o_down_flag)
				walk_flag=2;	// made it
			else
				walk_flag=0;	// try LHS next, or fail (to anywhere)
		}

		walk_attempt+=1;
	}
	while ((walk_flag<2)&&(walk_attempt<2));	// try again until we get there or we've tried both sides
	//--------------------------------------------------------


	if (walk_flag!=2)	// if LHS & RHS both failed - try anywhere
	{
		FN_walk(target_x,target_y,ANY,STAND);	// walk George to (near) Mega's coords
	}


	FN_we_wait(target_mega);	// wait for Duane to be ready to receive a command

	if ((o_xcoord) < target_x)	// If George is now left of Duane
	{
		FN_they_do_we_wait(target_mega,INS_turn,DOWN_LEFT,0,0);		// Mega faces DOWN_LEFT
		FN_turn(DOWN_RIGHT, STAND);											// George faces DOWN_RIGHT
	}
	else	// George is now right of Duane
	{
		FN_they_do_we_wait(target_mega,INS_turn,DOWN_RIGHT,0,0);	// Mega faces DOWN_LEFT
		FN_turn(DOWN_LEFT, STAND);											// George faces DOWN_RIGHT
	}
	//-----------------------------------------------------------------------------------
}
//------------------------------------------------------------------------------------
int32 FN_they_do(object *compact, int32 id, int32 tar, int32 a, int32 b, int32 c, int32 d, int32 x)	//S2.1(18Jan95tw)
{
	// NB. a, b & c could be resID's!! (25jul95 JEL)

	object	*target;

	compact;id;x;

	//Tdebug("FN_they_do %d %d %d %d %d", tar,a,b,c,d);

	target = (object *) Lock_object(tar);

	target->o_down_flag = a;	// the actual instruction; INS_talk, INS_quit, etc.
	target->o_ins1			= b;	// and now the 3 parameters...	(updated 24apr95JEL)
	target->o_ins2			= c;
	target->o_ins3			= d;

	Unlock_object(tar);

	return(1);			//script cont
}

//--------------------------------------------------------------------------------------

//send an instruction to mega we're talking to and wait until it has finished before
//returning to script

int32 FN_they_do_we_wait(object *compact, int32 id, int32 tar, int32 a, int32 b, int32 c, int32 d, int32 x)
{								//S2.1(18Jan95tw)

	// NB. a, b & c could be resID's!! (25jul95 JEL)

	object	*target;

	compact;id;x;

	//Tdebug("FN_they_do_we_wait %d %d %d %d %d", tar,a,b,c,d);

	target = (object *) Lock_object(tar);

	target->o_down_flag = a;	// the actual instruction; INS_talk, INS_quit, etc.
	target->o_ins1			= b;	// and now the 3 parameters...	(updated 24apr95JEL)
	target->o_ins2			= c;
	target->o_ins3			= d;

	compact->o_logic = LOGIC_wait_for_talk;	// we wait until they've finished
	compact->o_down_flag=tar;								// we wait for this person

	target->o_status &= (0xffffffff-STAT_TALK_WAIT);		// remove for this cycle - remember, this damn system is totally seamless...

	Unlock_object(tar);
	return(0);			//script stop
}

//--------------------------------------------------------------------------------------
// wait until last instruction it has finished before returning to script

int32 FN_we_wait(object *compact, int32 id, int32 tar, int32 a, int32 b, int32 c, int32 d, int32 e)	// (7JULY95 JEL)
{
	object	*target;

	compact;id;a;b;c;d;e;

	target = (object *) Lock_object(tar);

	//Tdebug("FN_we_wait %d", tar);

	compact->o_logic = LOGIC_wait_for_talk;	// we wait until they've finished
	compact->o_down_flag=tar;								// we wait for this person

	target->o_status &= (0xffffffff-STAT_TALK_WAIT);		// remove for this cycle - remember, this damn system is totally seamless...

	Unlock_object(tar);
	return(0);			//script stop
}

//--------------------------------------------------------------------------------------
int32 FN_add_talk_wait_status_bit(object *compact, int32 id, int32 tar, int32 a, int32 b, int32 c, int32 d, int32 e)	// (21may96 JEL)
{
	id;tar;a;b;c;d;e;

	compact->o_status |= STAT_TALK_WAIT;

	return(1);			//script continue
}
//--------------------------------------------------------------------------------------
int32 FN_remove_talk_wait_status_bit(object *compact, int32 id, int32 tar, int32 a, int32 b, int32 c, int32 d, int32 e)	// (21may96 JEL)
{
	id;tar;a;b;c;d;e;

	compact->o_status &= (0xffffffff-STAT_TALK_WAIT);

	return(1);			//script continue
}
//--------------------------------------------------------------------------------------

//mega_interact has recieved an instruction it does not understand - I have chosen to
//halt the game to ensure the error is noticed...

int32 FN_talk_error(object *compact, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x)	//S2.1(18Jan95tw)
{
	compact;id;a;b;c;d;z;x;

	Tdebug("FN_talk_error for %d - instruct = %d",id, compact->o_down_flag);
	Go_dos("FN_talk_error for %d - instruct = %d",id, compact->o_down_flag);

	return(0);
}

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

//wait for target to switch into STAT_TALK_WAIT then go back into script mode
//used during speech they_do commands - we must look to a specific target in-case
//of multiple particpents in the conversation - a sync from the target is not enough...

Logic_wait_talk(object *compact, int id)			//S2.1(25Jan95tw)
{
	object	*target;

	id;

	target = (object *) Lock_object(compact->o_down_flag);	//holds id of person we're waiting for


	if	(!(target->o_status&STAT_TALK_WAIT))
	{
		Unlock_object(compact->o_down_flag);	//holds id of person we're waiting for
		return(0);								//0 which means drop out of logic
	}
	else
	{
		compact->o_logic=LOGIC_script;	//back to script again next cycle
		Unlock_object(compact->o_down_flag);	//holds id of person we're waiting for
		return(1);	// go straight back into script
		//return(0);	// drop out for one cycle (allows speech text sprite to disappear before continuing - 13Mar95JEL)
	}
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
#endif



//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
uint32	unpause_zone=0;
//------------------------------------------------------------------------------------
int32 FN_i_speak(int32 *params)	//Tony18Oct96 (revamped by James01july97)
{
//its the super versatile FN_speak
//text and wavs can be selected in any combination

//we can assume no human - there should be no human at least!

//param	0 pointer to ob_graphic
//param	1 pointer to ob_speech
//param	2 pointer to ob_logic
//param	3 pointer to ob_mega

//param	4 encoded text number
//param	5 wav res id
//param	6 anim res id
//param	7 anim table res id
//param	8 animation mode	0 lip synced, 1 just straight animation

#define	S_OB_GRAPHIC	0
#define	S_OB_SPEECH		1
#define	S_OB_LOGIC		2
#define	S_OB_MEGA		3

#define	S_TEXT			4
#define	S_WAV			5
#define	S_ANIM			6
#define	S_DIR_TABLE		7
#define	S_ANIM_MODE		8

	_mouseEvent		*me;
	_animHeader		*anim_head;
	Object_logic	*ob_logic;
	Object_graphic	*ob_graphic;
	Object_mega		*ob_mega;
	uint8			*anim_file;
	uint32			local_text;
	uint32			text_res;
	uint8			*text;
	static uint8	textRunning, speechRunning;
	int32			*anim_table;
	uint8			speechFinished=0;	// James25feb97
	int8			speech_pan;
	char			speechFile[256];
	static uint8	cycle_skip=0;

	#ifdef _DEBUG			// (James26jun97)
	_standardHeader	*head;	// for text/speech testing & checking for correct file type
	static	uint32	currentTextResource=0;	// for text/speech testing - keeping track of text resource currently being tested
	#endif

  	uint32	rv;	// drivers return value


	//-----------------------------------------------
	// set up the pointers which we know we'll always need

	ob_logic	= (Object_logic *) params[S_OB_LOGIC];
	ob_graphic	= (Object_graphic *) params[S_OB_GRAPHIC];

	//-----------------------------------------------
	// FIRST TIME ONLY: create the text, load the wav, set up the anim, etc.

	if	(!ob_logic->looping)
	{
	 	//-------------------------
		// New fudge to wait for smacker samples to finish (James31july97)
		// since they can over-run into the game

		if (GetSpeechStatus()!=RDSE_SAMPLEFINISHED)	// has it finished?
			return (IR_REPEAT);
		
	 	//-------------------------
		// New fudge for 'fx' subtitles (James 29july97)
		// If subtitles switched off, and we don't want to use a wav for this line either,
		// then just quit back to script right now!
		if ((subtitles==0) && (WantSpeechForLine(params[S_WAV])==0))
			return (IR_CONT);

	 	//-------------------------
		if (cycle_skip==0)	// (James 17july97)
		{
			// drop out for 1st cycle to allow walks/anims to end & display last frame
			// before system locks while speech loaded
			cycle_skip=1;
			return (IR_REPEAT);
		}
		else
			cycle_skip=0;
	 	//-------------------------

		//-----------------------------------------------------------
		#ifdef _DEBUG			// (James26jun97)

 		textNumber	= params[S_TEXT];	// for debug info

		// For testing all text & speech!	(James26jun97)
		// A script loop can send any text number to FN_I_speak & it will only run the valid ones
		// or return with 'result' equal to '1' or '2' to mean 'invalid text resource'
		// and 'text number out of range' respectively
		// See 'testing_routines' object in George's Player Character section of linc

		if (SYSTEM_TESTING_TEXT)
		{
			RESULT=0;

			text_res	= params[S_TEXT]/SIZE;
			local_text	= params[S_TEXT]&0xffff;

			if (res_man.Res_check_valid(text_res))	// if the resource number is within range & it's not a null resource
			{
				head = (_standardHeader*) res_man.Res_open(text_res);	// open the resource

				if (head->fileType==TEXT_FILE)							// if it's not an animation file
				{
					if (CheckTextLine((uint8*)head,local_text)==0)		// if line number is out of range
						RESULT=2;										// line number out of range
				}
				else
					RESULT=1;											// invalid (not a text resource)

				res_man.Res_close(text_res);							// close the resource

				if (RESULT)
					return(IR_CONT);
			}
			else
			{															// not a valid resource number
				RESULT=1;												// invalid (null resource)
				return(IR_CONT);
			}
		}

		#endif	// _DEBUG
 		//-----------------------------------------------------------
		// pull out the text line to get the official text number (for wav id)
		// Once the wav id's go into all script text commands, we'll only need this for _DEBUG

		text_res	= params[S_TEXT]/SIZE;
		local_text	= params[S_TEXT]&0xffff;

		text = FetchTextLine( res_man.Res_open(text_res), local_text );	// open text file & get the line
		//officialTextNumber = *(uint16*)text;	// 1st word of text line is the official line number (this doesn't work on PSX)
		memcpy(&officialTextNumber, text, 2);	// this works on PSX & PC

		res_man.Res_close(text_res);	// now ok to close the text file

		//--------------------------------------
		#ifdef _DEBUG			// (James09jul97)

		// prevent dud lines from appearing while testing text & speech
		// since these will not occur in the game anyway

		if (SYSTEM_TESTING_TEXT)	// if testing text & speech
		{	// if actor number is 0 and text line is just a 'dash' character
			if ((officialTextNumber==0) && (text[2]=='-') && (text[3]==NULL))
			{
				RESULT=3;			// dud line
				return(IR_CONT);	// return & continue script
			}
		}

		#endif	// _DEBUG
		//--------------------------------------
		// set the 'looping_flag' & the text-click-delay
 
		ob_logic->looping=1;
		left_click_delay=6;		// can't left-click past the text for the first half second
		right_click_delay=3;	// can't right-click past the text for the first quarter second

		//----------------------------------------------------------
		// Write to walkthrough file (zebug0.txt)

		#ifdef _DEBUG
		if (PLAYER_ID!=CUR_PLAYER_ID)	// if (player_id != george), then player is controlling Nico
			Zdebug(0,"(%d) Nico: %s", officialTextNumber, text+2);	// so write 'Nico' instead of George
		else	// ok, it's George anyway
			Zdebug(0,"(%d) %s: %s", officialTextNumber, FetchObjectName(ID), text+2);
		#endif

		//--------------------------------------
		// Set up the speech animation

		if	(params[S_ANIM])	//just a straight anim
		{
			anim_id=params[S_ANIM];
			speech_anim_type=SPEECHANIMFLAG;		//params[S_ANIM_MODE];	//anim type

			ob_graphic->anim_resource = anim_id;	// set the talker's graphic to this speech anim now
			ob_graphic->anim_pc = 0;				// set to first frame
		}
		else if (params[S_DIR_TABLE])	//use this direction table to derive the anim	NB. ASSUMES WE HAVE A MEGA OBJECT!!
		{
			ob_mega = (Object_mega*) params[S_OB_MEGA];

			anim_table = (int32 *)params[S_DIR_TABLE];	// pointer to anim table
			anim_id = anim_table[ob_mega->current_dir];	// appropriate anim resource is in 'table[direction]'

			speech_anim_type=SPEECHANIMFLAG;		//params[S_ANIM_MODE];	//anim type

			ob_graphic->anim_resource = anim_id;	// set the talker's graphic to this speech anim now
			ob_graphic->anim_pc = 0;				// set to first frame
		}
		else	//no animation choosen
		{
			anim_id=0;	//no animation
		}

		SPEECHANIMFLAG = 0;							// Default back to looped lip synced anims.

		//--------------------------------------
		// set up 'text_x' & 'text_y' for speech-pan and/or text-sprite position

		LocateTalker(params);

		//--------------------------------------
		// is it to be speech or subtitles or both?

		speechRunning=0;	// assume not running until know otherwise

		// New fudge for 'fx' subtitles (James 29july97)
		// if speech is selected, and this line is allowed speech (not if it's an fx subtitle!)
		if (speechSelected && WantSpeechForLine(officialTextNumber))
		{
			// if the wavId paramter is zero because not yet compiled into speech command,
			// we can still get it from the 1st 2 chars of the text line
			if (!params[S_WAV])
				params[S_WAV] = (int32)officialTextNumber;

			#define SPEECH_VOLUME	16	// 0..16
			#define SPEECH_PAN		0	// -16..16

			speech_pan = ((text_x-320)*16)/320;
			// 'text_x'		'speech_pan'
			//		0			-16
			//		320			0
			//		640			16

			if (speech_pan<-16)		// keep within limits of -16..16, just in case
				speech_pan=-16;
			else if (speech_pan>16)
				speech_pan=16;

			#ifdef _DEBUG
			if (SYSTEM_TESTING_TEXT)	// if we're testing text & speech
			{
				// if we've moved onto a new text resource, we will want to check
				// if the CD needs changing again
				// - can only know which CD to get if the wavID is non-zero
				if ((text_res != currentTextResource) && (params[S_WAV]))
				{
					GetCorrectCdForSpeech(params[S_WAV]);	// ensure correct CD is in for this wavId
					currentTextResource = text_res;
				}
			}
			#endif

			//------------------------------
			// set up path to speech cluster
			// first checking if we have speech1.clu or speech2.clu in current directory (for translators to test)

#ifdef _WEBDEMO		// (James 01oct97)
			strcpy(speechFile,"SPEECH.CLU");
#else


#ifdef _DEBUG
			if ((res_man.WhichCd()==1) && (!access("speech1.clu",0)))	// if 0 ie. if it's there
			{
				strcpy(speechFile,"speech1.clu");
			}
			else if ((res_man.WhichCd()==2) && (!access("speech2.clu",0)))	// if 0 ie. if it's there
			{
				strcpy(speechFile,"speech2.clu");
			}
			else
#endif	// _DEBUG
			{
				strcpy(speechFile,res_man.GetCdPath());
				strcat(speechFile,"CLUSTERS\\SPEECH.CLU");
			}
#endif	// _WEBDEMO
 			//------------------------------


			rv = PlayCompSpeech(speechFile, params[S_WAV], SPEECH_VOLUME, speech_pan);	// Load speech but don't start playing yet
			if (rv == RD_OK)
			{
				speechRunning=1;	// ok, we've got something to play	(2 means not playing yet - see below)
				UnpauseSpeech();	// set it playing now (we might want to do this next cycle, don't know yet)
			}
			#ifdef _DEBUG
			else
			{
				Zdebug("ERROR: PlayCompSpeech(speechFile=\"%s\", wav=%d (res=%d pos=%d)) returned %.8x", speechFile, params[S_WAV], text_res, local_text, rv);
			}
			#endif


		}

		if (subtitles || (speechRunning==0))	// if we want subtitles, or speech failed to load
		{
			textRunning=1;						// then we're going to show the text
			Form_text(params);				// so create the text sprite
		}
		else
			textRunning=0;						// otherwise don't want text

 		//--------------------------------------

	}
  	//-----------------------------------------------
	// EVERY TIME: run a cycle of animation, if there is one

	if	(anim_id)	// there is an animation
	{
		ob_graphic->anim_pc++;	// increment the anim frame number

		anim_file	= res_man.Res_open(ob_graphic->anim_resource);			// open the anim file
		anim_head	= FetchAnimHeader( anim_file );

		if	(!speech_anim_type)												// ANIM IS TO BE LIP-SYNC'ED & REPEATING
		{
			if (ob_graphic->anim_pc == (int32)(anim_head->noAnimFrames))	// if finished the anim
				ob_graphic->anim_pc=0;										// restart from frame 0
			else if (speechRunning)											// if playing a sample
			{
				if	(!unpause_zone)
				{	if (AmISpeaking()==RDSE_QUIET)								// if we're at a quiet bit
						ob_graphic->anim_pc=0;									// restart from frame 0 ('closed mouth' frame)
				}
			}
		}
		else																// ANIM IS TO PLAY ONCE ONLY
		{
			if (ob_graphic->anim_pc == (int32)(anim_head->noAnimFrames)-1)	// reached the last frame of the anim
			{
				anim_id=0;													// hold anim on this last frame
			}
		}

		res_man.Res_close(ob_graphic->anim_resource);						// close the anim file
	}
	else if (speech_anim_type)
		speech_anim_type = 0;												// Placed here so we actually display the last frame of the anim.

	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	// EVERY TIME: FIND OUT IF WE NEED TO STOP THE SPEECH NOW...
	//-----------------------------------------------------------------------
	// if there is a wav then we're using that to end the speech naturally

	if (speechRunning==1)	// if playing a sample (note that value of '2' means about to play!)
	{
		if	(!unpause_zone)
		{	if (GetSpeechStatus()==RDSE_SAMPLEFINISHED)	// has it finished?
				speechFinished=1;						// James25feb97
		}
		else	unpause_zone--;
	}
   	//-----------------------------------------------------------------------
	// if no sample then we're using speech_time to end speech naturally

	else if ((speechRunning==0)&&(speech_time))		// counting down text time because there is no sample - this ends the speech
	{
		speech_time--;
		if (!speech_time)
			speechFinished=1;						// James25feb97
	}
  	//-----------------------------------------------------------------------
	// ok, all is running along smoothly - but a click means stop unnaturally

	#ifdef _DEBUG
	if ((SYSTEM_TESTING_TEXT==0)||(mousey>0))	// so that we can go to the options panel while text & speech is being tested
	#endif
	{
		me = MouseEvent();								// get mouse event

		// Note that we now have TWO click-delays - one for LEFT button, one for RIGHT BUTTON
		if ( ((!left_click_delay)&&(me!=NULL)&&(me->buttons&RD_LEFTBUTTONDOWN)) || ((!right_click_delay)&&(me!=NULL)&&(me->buttons&RD_RIGHTBUTTONDOWN)) )
		{
			// mouse click, after click_delay has expired -> end the speech
	 		// we ignore mouse releases

			//-----------------------------------------------------------
			#ifdef _DEBUG	// (James26jun97)
			if (SYSTEM_TESTING_TEXT)	// if testing text & speech
			{
				if (me->buttons&RD_RIGHTBUTTONDOWN)	// and RB used to click past text
					SYSTEM_WANT_PREVIOUS_LINE=1;	// then we want the previous line again
				else
					SYSTEM_WANT_PREVIOUS_LINE=0;	// LB just want next line again
			}
			#endif
			//-----------------------------------------------------------

			do
				me = MouseEvent();	//trash anything thats buffered
			while(me!=NULL);

			speechFinished=1;							// James25feb97

			if (speechRunning)							// if speech sample playing
			{
				StopSpeech();							// halt the sample prematurely
			}
		}
	}
   	//-----------------------------------------------------------------------
	// if we are finishing the speech this cycle, do the business

	if (speechFinished && !speech_anim_type)		// !speech_anim_type, as we want an anim which is playing once to have finished.
	{
		if (speech_text_bloc_no)					// if there is text
		{
			Kill_text_bloc(speech_text_bloc_no);	// kill the text block
			speech_text_bloc_no=0;
		}

		if (anim_id)								// if there is a speech anim
		{
			anim_id=0;
			ob_graphic->anim_pc=0;					// end it on 1st frame (closed mouth)
		}

		textRunning=0;
		speechRunning=0;

		ob_logic->looping=0;						// no longer in a script function loop

#ifdef _DEBUG
		textNumber			= 0;					// reset for debug info
#endif // _DEBUG
		officialTextNumber	= 0;					// reset to zero, in case text line not even extracted (since this number comes from the text line)

		RESULT=0;	// ok (James09july97)
		return(IR_CONT);							// continue with the script
	}
   	//-----------------------------------------------------------------------
	// speech still going, so decrement the click_delay if it's still active

	if (left_click_delay)
		left_click_delay--;	// count down to clickability

 	if (right_click_delay)
		right_click_delay--;	// count down to clickability

  	//-----------------------------------------------------------------------

	return(IR_REPEAT);	// back again next cycle

}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void LocateTalker(int32	*params)	// (James 01july97)
{
	// sets 'text_x' & 'text_y' for position of text sprite
	// but 'text_x' also used to calculate speech-pan

	//param	0 pointer to ob_graphic
	//param	1 pointer to ob_speech
	//param	2 pointer to ob_logic
	//param	3 pointer to ob_mega

	//param	4 encoded text number
	//param	5 wav res id
	//param	6 anim res id
	//param	7 pointer to anim table
	//param	8 animation mode	0 lip synced, 1 just straight animation

	Object_mega *ob_mega;

	uint8			*file;
	_frameHeader	*frame_head;
	_animHeader		*anim_head;
	_cdtEntry		*cdt_entry;
	uint16			scale;




	if (anim_id==0)				// if there's no anim
	{							// assume it's Voice-Over text
		text_x	= 320;			// so it goes at bottom of screen
		text_y	= 400;
	}
	else
	{
	   	#define GAP_ABOVE_HEAD	20	// distance kept above talking sprite

		//-------------------------------------------
		// Note: this code has been adapted from Register_frame() in build_display.cpp
		//-------------------------------------------
		// open animation file & set up the necessary pointers

		file		= res_man.Res_open(anim_id);

		anim_head	= FetchAnimHeader( file );
		cdt_entry	= FetchCdtEntry( file, 0 );		// '0' means 1st frame
		frame_head	= FetchFrameHeader( file, 0 );	// '0' means 1st frame

		//-------------------------------------------	
		// check if this frame has offsets ie. this is a scalable mega frame

		if ((cdt_entry->frameType) & FRAME_OFFSET)
		{
			ob_mega = (Object_mega*) params[S_OB_MEGA];		// this may be NULL

			// calc scale at which to print the sprite, based on feet y-coord & scaling constants (NB. 'scale' is actually 256*true_scale, to maintain accuracy)
			scale = (ob_mega->scale_a * ob_mega->feet_y + ob_mega->scale_b)/256;	// Ay+B gives 256*scale ie. 256*256*true_scale for even better accuracy, ie. scale = (Ay+B)/256

			// calc suitable centre point above the head, based on scaled height
			text_x = ob_mega->feet_x;								// just use 'feet_x' as centre
			text_y = ob_mega->feet_y + (cdt_entry->y * scale)/256;	// add scaled y-offset to feet_y coord to get top of sprite
		}
		else	// it's a non-scaling anim
		{
			// calc suitable centre point above the head, based on scaled width
			text_x = cdt_entry->x + (frame_head->width)/2;	// x-coord + half of width
			text_y = cdt_entry->y;							// y-coord
		}

		text_y -= GAP_ABOVE_HEAD;	// leave space above their head
			
		//-------------------------------------------	
		// adjust the text coords for RDSPR_DISPLAYALIGN

		text_x -= this_screen.scroll_offset_x;
		text_y -= this_screen.scroll_offset_y;

		//-------------------------------------------	
		// release the anim resource

		res_man.Res_close(anim_id);

		//-------------------------------------------	
	}
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void	Form_text(int32	*params)	//Tony18Oct96
{
//its the first time in so we build the text block if we need one
//we also bring in the wav if there is one
//also setup the animation if there is one

//anim is optional - anim can be a repeating lip-sync or a run-once anim
//if there is no wav then the text comes up instead
//there can be any combination of text/wav playing

//param	0 pointer to ob_graphic
//param	1 pointer to ob_speech
//param	2 pointer to ob_logic
//param	3 pointer to ob_mega

//param	4 encoded text number
//param	5 wav res id
//param	6 anim res id
//param	7 pointer to anim table
//param	8 animation mode	0 lip synced, 1 just straight animation


	uint32	local_text;
	uint32	text_res;
	uint8	*text;
	uint32	textWidth;
	Object_speech	*ob_speech;



	if	(params[S_TEXT])	//should always be a text line, as all text is derived from line of text
	{
	 	ob_speech = (Object_speech*) params[S_OB_SPEECH];

		//----------------------------------------------------------
		// establish the max width allowed for this text sprite

 		if (ob_speech->width)				// if a specific width has been set up for this character
			textWidth = ob_speech->width;	// override the default
		else
			textWidth = 400;				// otherwise use the default

		//----------------------------------------------------------
		// pull out the text line & make the sprite & text block

		text_res	= params[S_TEXT]/SIZE;
		local_text	= params[S_TEXT]&0xffff;

		text = FetchTextLine( res_man.Res_open(text_res), local_text );	// open text file & get the line

		// 'text+2' to skip the first 2 bytes which form the line reference number
		speech_text_bloc_no = Build_new_block(text+2, text_x, text_y, textWidth, ob_speech->pen, RDSPR_TRANS+RDSPR_DISPLAYALIGN, speech_font_id, POSITION_AT_CENTRE_OF_BASE);

		res_man.Res_close(text_res);	// now ok to close the text file

 		//----------------------------------------------------------
		// set speech duration, in case not using wav

		speech_time = strlen((char *)text) + 30;	// no. of cycles = (no. of chars) + 10

		//----------------------------------------------------------
	}
	else	// no text line passed? - this is bad
	{
		// Zdebug(9,"no text line for speech wav %d", params[S_WAV]);	//stream 9 for missing text & wavs
		Zdebug("no text line for speech wav %d", params[S_WAV]);	//stream 9 for missing text & wavs
	}
}
//------------------------------------------------------------------------------------
#ifdef _DEBUG

void GetCorrectCdForSpeech(int32 wavId)
{
	FILE	*fp;
	uint8	cd;	// 1, 2 or 0 (if speech on both cd's, ie. no need to change)

	fp = fopen("cd.bin","rb");

	if (fp==NULL)
		Con_fatal_error("Need cd.bin file for testing speech!");

	fseek(fp, wavId, SEEK_SET);
	fread(&cd, 1, 1, fp);

	fclose(fp);

	if ((cd==1)||(cd==2))	// if we specifically need CD1 or CD2 (ie. it's not on both)
		res_man.GetCd(cd);	// then check it's there (& ask for it if it's not there)
}

#endif
//------------------------------------------------------------------------------------
// For preventing sfx subtitles from trying to load speech samples
// - since the sfx are implemented as normal sfx, so we don't want them as speech samples too
// - and we only want the subtitles if selected, not if samples can't be found!

uint8 WantSpeechForLine(uint32 wavId)	// James (29july97)
{
	switch (wavId)
	{
		case 1328:	// AttendantSpeech
					//		SFX(Phone71);
					//		FX <Telephone rings>

		case 2059:	// PabloSpeech
					//		SFX (2059);
					//		FX <Sound of sporadic gunfire from below>

		case 4082:	// DuaneSpeech
					//		SFX (4082);
					//		FX <Pffffffffffft! Frp. (Unimpressive, flatulent noise.)>

		case 4214:	// cat_52
					//		SFX (4214);
					//		4214FXMeow!

		case 4568:	// trapdoor_13
	 				//		SFX (4568);
					//		4568fx<door slamming>

		case 4913:	// LobineauSpeech
					//		SFX (tone2);
					//		FX <Lobineau hangs up>

		case 5120:	// bush_66
					//		SFX (5120);
					//		5120FX<loud buzzing>

		case 528:	// PresidentaSpeech
					//		SFX (528);
					//		FX <Nearby Crash of Collapsing Masonry>

		case 920:	// location 62

		case 923:	// location 62

		case 926:	// location 62

		{
			return 0;	// don't want speech for these lines!
			break;
		}

		default:
			return 1;	// ok for all other lines
	}
}
//------------------------------------------------------------------------------------
