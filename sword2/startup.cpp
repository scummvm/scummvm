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
#include <stdio.h>

#include "stdafx.h"
//#include "src\driver96.h"
#include "build_display.h"
#include "console.h"
#include "debug.h"
#include "defs.h"
#include "header.h"
#include "interpreter.h"
#include "maketext.h"	// for Kill_text_bloc()
#include "memory.h"
#include "mouse.h"		// for FN_add_human()
#include "object.h"
#include "resman.h"
#include "router.h"
#include "sound.h"
#include "speech.h"		// for 'speech_text_bloc_no' - so that speech text can be cleared when running a new start-script
#include "startup.h"
#include "sword2.h"	// (James11aug97) for CloseGame()
#include "sync.h"
#include "tony_gsdk.h"

//------------------------------------------------------------------------------------
uint32	total_startups=0;
uint32	total_screen_managers=0;
uint32	res;

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
_startup	start_list[MAX_starts];
//------------------------------------------------------------------------------------
uint32	Init_start_menu(void)	//Tony13Aug96
{
//print out a list of all the start points available
//there should be a linc produced file called startup.txt
//this file should contain ascii numbers of all the resource game objects that are screen managers
//we query each in turn and setup an array of start structures
//if the file doesn't exist then we say so and return a 0


	uint32	end;
	mem	*temp;
	uint32	pos=0;
	uint32	j=0;
	char	*raw_script;
	uint32	null_pc;


	char	ascii_start_ids[MAX_starts][7];


//ok, load in the master screen manager file




	total_startups=0;	//no starts



	Zdebug("initialising start menu");


	if	(!(end=Read_file("STARTUP.INF", &temp, UID_temp)))
	{
		Zdebug("Init_start_menu cannot open startup.inf");
		return(0);	//meaning no start menu available
	}



//ok, we've loaded in the resource.inf file which contains a list of all the files
//now extract the filenames
	do
	{
		while(((char)*(temp->ad+j))!=13)	//item must have an #0d0a
		{
			ascii_start_ids[total_screen_managers][pos]=*(temp->ad+j);
			j++;
			pos++;

		};

		ascii_start_ids[total_screen_managers][pos]=0;	//NULL terminate our extracted string

		pos=0;	//reset position in current slot between entries
		j+=2;	//past the 0a
		total_screen_managers++;	//done another

		if	(total_screen_managers==MAX_starts)
		{	Zdebug("WARNING MAX_starts exceeded!");
			break;
		}
	}
	while(j<end);	//using this method the Gode generated resource.inf must have #0d0a on the last entry



	Zdebug("%d screen manager objects", total_screen_managers);

//open each object and make a query call. the object must fill in a startup structure
//it may fill in several if it wishes - for instance a startup could be set for later in the game where specific vars are set
	for	(j=0;j<total_screen_managers;j++)
	{
		res=atoi(ascii_start_ids[j]);

		Zdebug("+querying screen manager %d", res);


//		resopen each one and run through the interpretter
//		script 0 is the query request script


		if (res_man.Res_check_valid(res))	// if the resource number is within range & it's not a null resource (James 12mar97)
		{									// - need to check in case un-built sections included in start list
			Zdebug("- resource %d ok",res);
			raw_script= (char*) (res_man.Res_open(res));	//+sizeof(_standardHeader)+sizeof(_object_hub));
			null_pc=0;	//
			RunScript ( raw_script, raw_script, &null_pc );
			res_man.Res_close(res);
		}
		else
		{
			Zdebug("- resource %d invalid",res);
		}
	}


	Zdebug("");	//line feed


	Free_mem(temp);	//release the Talloc

	return(1);
}
//------------------------------------------------------------------------------------
int32 FN_register_start_point(int32 *params)	//Tony14Oct96
{
	// param	0 id of startup script to call - key
	// param	1 pointer to ascii message


//	Zdebug(" FN_register_start_point %d %s", params[0], params[1]);

	#ifdef _SWORD2_DEBUG
	if (total_startups==MAX_starts)
		Con_fatal_error("ERROR: start_list full [%s line %u]",__FILE__,__LINE__);

	if (strlen((char*)params[1])+1 > MAX_description)	// +1 to allow for NULL terminator
		Con_fatal_error("ERROR: startup description too long [%s line %u]",__FILE__,__LINE__);
	#endif

	start_list[total_startups].start_res_id	= res;			// this objects id
	start_list[total_startups].key			= params[0];	// a key code to be passed to a script via a script var to SWITCH in the correct start
	strcpy(start_list[total_startups].description, (char*)params[1]);

	total_startups++;	//point to next

	return(1);
}
//------------------------------------------------------------------------------------
uint32	Con_print_start_menu(void)	//Tony14Oct96
{
	//the console 'starts' (or 's') command which lists out all the registered start points in the game
	uint32	j;
	int		scrolls=0;
	char	c;


	if	(!total_startups)
	{	Print_to_console("Sorry - no startup positions registered?");

		if	(!total_screen_managers)
			Print_to_console("There is a problem with startup.inf");
		else
			Print_to_console(" (%d screen managers found in startup.inf)", total_screen_managers);

	}
	else
	{
		for(j=0;j<total_startups;j++)
		{
			Print_to_console("%d  (%s)", j, start_list[j].description);
			Build_display();
			scrolls++;


			if	(scrolls==18)
			{
				Temp_print_to_console("- Press ESC to stop or any other key to continue");
				Build_display();

				do
				{
			 		//--------------------------------------------------
					// Service windows
					while (!gotTheFocus)
						if (ServiceWindows() == RDERR_APPCLOSED)
							break;

					if (ServiceWindows() == RDERR_APPCLOSED)	// if we pressed Ctrl-Q
					{
						Close_game();	//close engine systems down
						RestoreDisplay();
						CloseAppWindow();
						exit(0);	//quit the game
					}
 					//--------------------------------------------------
				}
				while(!KeyWaiting());

				ReadKey(&c);	//kill the key we just pressed
				if	(c==27)	//ESC
					break;

				Clear_console_line();	//clear the Press Esc message ready for the new line
				scrolls=0;
			}	


		}
	}
	return(1);
}
//------------------------------------------------------------------------------------
uint32	Con_start(uint8 *input)	//Tony15Oct96
{
//if the second word id is a numeric that can be applied to a genuine startup then do it
	uint32	j=0;
	uint32	start;
	char	*raw_script;
	char	*raw_data_ad;
	uint32	null_pc;



	if (*input == 0)		// so that typing 'S' then <enter> works on NT (James26feb97)
	{
		Con_print_start_menu();
		return(1);
	}


	while(*(input+j))
	{
		if ( (*(input+j)>='0') && (*(input+j)<='9') )
			j++;
		else
			break;
	}


	if	(!*(input+j))	//didn't quit out of loop on a non numeric chr$
	{
		start = atoi((char*)input);

		if	(!total_startups)
			Print_to_console("Sorry - there are no startups!");

		else	if	(start<total_startups)	//a legal start
		{
//			do the startup as we've specified a legal start

			//--------------------------------------------------------------
			// restarting - stop sfx, music & speech!

			Clear_fx_queue();
			//---------------------------------------------
			FN_stop_music(NULL);	// fade out any music that is currently playing
			//---------------------------------------------

			g_sword2->_sound->UnpauseSpeech();
			g_sword2->_sound->StopSpeechSword2();							// halt the sample prematurely

			//--------------------------------------------------------------
			// clean out all resources & flags, ready for a total restart (James24mar97)

			res_man.Remove_all_res();	// remove all resources from memory, including player object & global variables
			SetGlobalInterpreterVariables((int32*)(res_man.Res_open(1)+sizeof(_standardHeader)));	// reopen global variables resource & send address to interpreter - it won't be moving
			res_man.Res_close(1);

			FreeAllRouteMem();	// free all the route memory blocks from previous game

			if (speech_text_bloc_no)					// if there was speech text
			{
				Kill_text_bloc(speech_text_bloc_no);	// kill the text block
				speech_text_bloc_no=0;
			}

			//--------------------------------------------------------------

//			set the key
			raw_data_ad= (char*) (res_man.Res_open(8));	//+sizeof(_standardHeader)+sizeof(_object_hub));	//open george
			raw_script= (char*) (res_man.Res_open(start_list[start].start_res_id));	//+sizeof(_standardHeader)+sizeof(_object_hub));
			null_pc=start_list[start].key&0xffff;	//denotes script to run
			Print_to_console("running start %d", start);
			RunScript ( raw_script, raw_data_ad, &null_pc );

			res_man.Res_close(start_list[start].start_res_id);
			res_man.Res_close(8);	//close george

			FN_add_human(NULL);	// make sure thre's a mouse, in case restarting while mouse not available
		}
		else
			Print_to_console("not a legal start position");
	}
	else
	{
		Con_print_start_menu();		// so that typing 'S' then <enter> works under Win95
	}
	return(1);
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------


