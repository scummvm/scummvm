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
// #include <libsn.h> PSX?
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//#include "src\driver96.h"
#include "build_display.h"
#include "console.h"
#include "debug.h"
#include "header.h"
#include "interpreter.h"
#include "logic.h"
#include "memory.h"
#include "resman.h"
#include "router.h"	// for ClearWalkGridList()
#include "sound.h"
#include "sword2.h"	// (James19aug97) for CloseGame()
#include "sync.h"

//------------------------------------------------------------------------------------
logic	LLogic;	//declare the object

#define	LEVEL cur_object_hub->logic_level

#define	OBJECT_KILL_LIST_SIZE	50	// this must allow for the largest number of objects in a screen

uint32	object_kill_list[OBJECT_KILL_LIST_SIZE];
uint32	kills=0;	// keeps note of no. of objects in the kill list

//------------------------------------------------------------------------------------
int	logic::Process_session(void)	//Tony6June96 (first run 21Oct96)
{
//do one cycle of the current session



	uint32	run_list;
	uint32	ret,script;
	uint32	*game_object_list;
	char	*raw_script_ad;
	char	*raw_data_ad;
	uint32	null_pc;
	_standardHeader	*head;
	_standardHeader	*far_head;
	uint32	id;

	run_list=current_run_list;	//might change during the session, so take a copy here
	pc=0;	//point to first object in list

	static uint32 cycle=0;


	cycle++;
//	Zdebug("\n CYCLE %d", cycle);

	while(pc!=0xffffffff)	//by minusing the pc we can cause an immediate cessation of logic processing on the current list
	{
		head = (_standardHeader*) res_man.Res_open(run_list);
		if	(head->fileType!=RUN_LIST)
			Con_fatal_error("Logic_engine %d not a run_list", run_list);

		game_object_list = (uint32	*) (head+1);
		ID = game_object_list[pc++];	//read the next id
		id=ID;
		res_man.Res_close(run_list);	//release the list again so it can float in memory - at this point not one thing should be locked


//		Zdebug("%d", ID);

		if	(!ID)	//null terminated
			return(0);	//end the session naturally

		head = (_standardHeader*) res_man.Res_open(ID);
		if	(head->fileType!=GAME_OBJECT)
			Con_fatal_error("Logic_engine %d not an object", ID);

		cur_object_hub = (_object_hub	*) (head+1);

//		Zdebug(" %d id(%d) pc(%d)", cur_object_hub->logic_level, cur_object_hub->script_id[cur_object_hub->logic_level], cur_object_hub->script_pc[cur_object_hub->logic_level]);



//		do the logic for this object
//		we keep going until a function says to stop - remember, system operations are run via function calls to drivers now
		do
		{
			script = cur_object_hub->script_id[LEVEL];	//get the script id as we may be running a script from another object...


//			there is a distinction between running one of our own scripts and that of another object
			if	((script/SIZE)==ID)	//its our script
			{
//				Zdebug("run script %d pc%d", script/SIZE, cur_object_hub->script_pc[LEVEL]);

//				raw_script_ad = (char *) (cur_object_hub+1);	//this is the script data

				raw_script_ad = (char*) head;

				ret=RunScript( raw_script_ad, raw_script_ad, &cur_object_hub->script_pc[LEVEL] );	//script and data object are us/same

			}
			else	//we're running the script of another game object - get our data object address
			{
//				get the foreign objects script data address

				raw_data_ad=(char*)head;

				far_head = (_standardHeader*) res_man.Res_open(script/SIZE);
				if	((far_head->fileType!=GAME_OBJECT)&&((far_head->fileType!=SCREEN_MANAGER)))
					Con_fatal_error("Logic_engine %d not a far object (its a %d)", script/SIZE, far_head->fileType);

//				raw_script_ad = (char*) (head+1) + sizeof(_standardHeader);

//				get our objects data address
//				raw_data_ad = (char*) (cur_object_hub+1);

				raw_script_ad=(char*)far_head;

				ret=RunScript( raw_script_ad, raw_data_ad, &cur_object_hub->script_pc[LEVEL] );

				res_man.Res_close(script/SIZE);	//close foreign object again

				raw_script_ad=raw_data_ad;	//reset to us for service script
			}

			if	(ret==1)	//this script has finished - drop down a level
			{
				if (cur_object_hub->logic_level)	//check that it's not already on level 0 !
					cur_object_hub->logic_level--;
				else	//Hmmm, level 0 terminated :-| Let's be different this time and simply let it restart next go :-)
				{
					cur_object_hub->script_pc[LEVEL]=(cur_object_hub->script_id[LEVEL]&0xffff);	//reset to rerun
//					Zdebug("**WARNING object %d script 0 terminated!", id);
					ret=0;	//cause us to drop out for a cycle
				}
			}
			else	if	(ret>2)
			{
				Con_fatal_error("Process_session: illegal script return type %d (%s line %u)",ret,__FILE__,__LINE__);
			}

//			if ret==2 then we simply go around again - a new script or subroutine will kick in and run

		}
		while(ret);	//keep processing scripts until 0 for quit is returned


//		any post logic system requests to go here

		Clear_syncs(ID);	//clear any syncs that were waiting for this character - it has used them or now looses them

		if	(pc!=0xffffffff)	//the session is still valid so run the service script
		{	null_pc=0;
			RunScript( raw_script_ad, raw_script_ad, &null_pc );	//call the base script - this is the graphic/mouse service call
		}
																				//made for all live objects

//		and that's it so close the object resource
		res_man.Res_close(ID);
	};


	Process_kill_list();	//leaving a room so remove all ids that must reboot correctly

	Zdebug("RESTART the loop");
	

	return(1);	//means restart the loop
}
//------------------------------------------------------------------------------------
void	logic::Express_change_session(uint32	sesh_id)	//Tony6June96
{
//a game-object can bring an immediate halt to the session and cause a new one to start without a screen update

	current_run_list=sesh_id;	//set to new
	pc=0xffffffff;	//causes session to quit

	EXIT_FADING=0;	// reset now in case we double-clicked an exit prior to changing screen

	Init_sync_system();		// we're trashing the list - presumably to change room
							// in theory sync waiting in the list could be left behind and never removed - so we trash the lot

	ClearWalkGridList();	// reset walkgrid list (see FN_register_walkgrid)
	Clear_fx_queue();		// stops all fx & clears the queue
	FreeAllRouteMem();		// free all the route memory blocks from previous game
}
//------------------------------------------------------------------------------------
void	logic::Natural_change_session(uint32	sesh_id)	//Tony7June96
{
//a new session will begin next game cycle.
//the current cycle will conclude and build the screen and flip into view as normal

	current_run_list=sesh_id;	//set to new
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
uint32	logic::Return_run_list(void)	//Tony18Sept96
{
//pass back the private cur_object_list variable - not sure we need this

	return(current_run_list);	//return the id
}
//------------------------------------------------------------------------------------
int32 FN_set_session(int32 *params)	//Tony29Oct96
{
//used by player invoked start scripts

//param	0 id of new run list
	LLogic.Express_change_session(*params);	//now!

	return(IR_CONT);	//cont
}
//------------------------------------------------------------------------------------
int32 FN_end_session(int32 *params)	//Tony21Sept96
{
//causes no more objects in this logic loop to be processed
//the logic engine will restart at the beginning of the new list
// !!the current screen will not be drawn!!

//param	0	id of new run-list

	LLogic.Express_change_session(*params);	//terminate current and change to next run-list

	return(0);	//stop the script - logic engine will now go around and the new screen will begin
}
//------------------------------------------------------------------------------------
void	logic::Logic_up(uint32	new_script)	//Tony23Sept96
{
//move the current object up a level
//called by FN_gosub command - remember, only the logic object has access to cur_object_hub


	cur_object_hub->logic_level++;	//going up a level - and we'll keeping going this cycle

	if	(cur_object_hub->logic_level==3)	//can be 0,1,2
		Con_fatal_error("Logic_up id %d has run off script tree! :-O", ID);

	cur_object_hub->script_id[cur_object_hub->logic_level]=new_script;	//setup new script on next level (not the current level)
	cur_object_hub->script_pc[cur_object_hub->logic_level]=new_script&0xffff;

	//Zdebug("new pc = %d", new_script&0xffff);

}
//------------------------------------------------------------------------------------
void	logic::Logic_one(uint32	new_script)	//Tony4Dec96
{
//force to level one

	cur_object_hub->logic_level=1;

	cur_object_hub->script_id[1]=new_script;	//setup new script on level 1
	cur_object_hub->script_pc[1]=new_script&0xffff;

}
//------------------------------------------------------------------------------------

void	logic::Logic_replace(uint32	new_script)	//Tony13Nov96
{
//change current logic - script must quit with a TERMINATE directive - which does not write to &pc

	cur_object_hub->script_id[cur_object_hub->logic_level]=new_script;	//setup new script on this level
	cur_object_hub->script_pc[cur_object_hub->logic_level]=new_script&0xffff;

}
//------------------------------------------------------------------------------------
uint32	logic::Examine_run_list(void)	//Tony25Oct96
{
	uint32	*game_object_list;
	_standardHeader	*file_header;
	int	scrolls=0;
	char	c;


	if	(current_run_list)
	{
		game_object_list = (uint32	*) (res_man.Res_open(current_run_list)+sizeof(_standardHeader));	//open and lock in place

		Print_to_console("runlist number %d", current_run_list);

		while(*(game_object_list))
		{
			file_header = (_standardHeader*) res_man.Res_open(*(game_object_list));
			Print_to_console(" %d %s",*(game_object_list), file_header->name);
			res_man.Res_close(*(game_object_list++));

			scrolls++;
			Build_display();

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




		res_man.Res_close(current_run_list);
	}
	else	Print_to_console("no run list set");


	Scroll_console();
	return(1);
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void	logic::Total_restart(void)	//Tony18Sept96
{
//reset the object restart script 1 on level 0

	cur_object_hub->logic_level=0;

	//cur_object_hub->script_id[0]=1;
	cur_object_hub->script_pc[0]=1;	//reset to rerun

}
//------------------------------------------------------------------------------------
int32 FN_total_restart(int32 *params)	//Tony5Dec96
{
//mega runs this to restart its base logic again - like being cached in again

	LLogic.Total_restart();

	if (params);

	return(IR_TERMINATE);	//drop out without saving pc and go around again
}
//------------------------------------------------------------------------------------
int32 FN_add_to_kill_list(int32 *params)	//James9jan97
{
	// call *once* from object's logic script - ie. in startup code
	// - so not re-called every time script drops off & restarts!

	// mark this object for killing - to be killed when player leaves this screen
	// - so object reloads & script restarts upon re-entry to screen
	// - causes this object's startup logic to be re-run every time we enter the screen
	// - "which is nice"

	// params: none

	uint32 entry;


	if (ID != 8)	// DON'T EVER KILL GEORGE!
	{
		// first, scan list to see if this object is already included (05mar97 James)
		entry=0;
		while ((entry < kills) && (object_kill_list[entry] != ID))
			entry++;

		if (entry == kills)	// if this ID isn't already in the list, then add it, (otherwise finish) (05mar97 James)
		{
			#ifdef _DEBUG
			if (kills == OBJECT_KILL_LIST_SIZE)	// no room at the inn
				Con_fatal_error("List full in FN_add_to_kill_list(%u) (%s line %u)",ID,__FILE__,__LINE__);
			#endif

			object_kill_list[kills] = ID;	// add this 'ID' to the kill list
			kills++;						// "another one bites the dust"

			// when we leave the screen, all these object resources are to be cleaned out of memory
			// and the kill list emptied by doing 'kills=0'
			// - ensuring that all resources are in fact still in memory & more importantly closed
			// before killing!
		}
	}

	return(IR_CONT);	// continue script
}
//------------------------------------------------------------------------------------
void	logic::Process_kill_list(void)	//Tony10Jan97
{

	uint32	j;


	if	(kills)
		for	(j=0;j<kills;j++)
			res_man.Remove_res(object_kill_list[j]);


	kills=0;

}
//------------------------------------------------------------------------------------
void	logic::Reset_kill_list(void)	//James 25mar97
{
	kills=0;
}
//------------------------------------------------------------------------------------

