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

//--------------------------------------------------------------------------------------
// WALKER.CPP by James (14nov96)

// script functions for moving megas about the place & also for keeping tabs on them

// FN_walk()				// walk to (x,y,dir)
// FN_walk_to_anim()		// walk to start position of anim
// FN_turn()				// turn to (dir)
// FN_stand_at()			// stand at (x,y,dir)
// FN_stand()				// stand facing (dir)
// FN_stand_after_anim()	// stand at end position of anim
// FN_face_id()				// turn to face object (id)
// FN_face_xy()				// turn to face point (x,y)
// FN_is_facing()			// is mega (id) facing us?
// FN_get_pos()				// get details of another mega's position

//--------------------------------------------------------------------------------------

//#include "src\driver96.h"
#include "console.h"
#include "defs.h"
#include "events.h"
#include "function.h"
#include "interpreter.h"
#include "logic.h"	// for FN_add_to_kill_list
#include "object.h"
#include "protocol.h"
#include "router.h"
#include "sync.h"

//--------------------------------------------------------------------------------------

int16	standby_x;			// see FN_set_standby_coords
int16	standby_y;
uint8	standby_dir;

//--------------------------------------------------------------------------------------
/*
uint8 Check_walk_anim_ok( Object_mega *ob_mega, Object_walkdata *ob_walkdata );
//--------------------------------------------------------------------------------------
// NEW CODE TO VERIFY THAT THE WALK-ANIM CONTAINS NO INVALID FRAMES!
// (James 15sep97)
uint8 Check_walk_anim_ok( Object_mega *ob_mega, Object_walkdata *ob_walkdata )
{
	int32		walk_pc=0;
	_walkData	*walkAnim;
	uint8		*anim_file;
	_animHeader	*anim_head;
	uint32		lastValidFrameNo;
	uint8		ok=1;


	anim_file = res_man.Res_open(ob_mega->megaset_res);	// open mega-set file
	anim_head = FetchAnimHeader( anim_file );			// set up pointer to the animation header
 	lastValidFrameNo = anim_head->noAnimFrames-1;		// get last valid frame number
	res_man.Res_close(ob_mega->megaset_res);			// close file

   	walkAnim = LockRouteMem();	// lock the _walkData array

	while (ok && (walkAnim[walk_pc].frame != 512))	// '512' id end-marker
	{
		if (walkAnim[walk_pc].frame > lastValidFrameNo)	// if frame exceeds the allowed range
			ok=0;

		walk_pc++;
	}

	FloatRouteMem();	// allow _walkData array to float about memory again

	return(ok);
}
*/
//--------------------------------------------------------------------------------------
// walk mega to (x,y,dir)

int32 FN_walk(int32 *params)	// James (14nov96)
{
	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 pointer to object's mega structure
	//			3 pointer to object's walkdata structure
	//			4 target x-coord
	//			5 target y-coord
	//			6 target direction

	Object_logic	*ob_logic;
	Object_graphic	*ob_graph;
	Object_mega		*ob_mega;
	Object_walkdata	*ob_walkdata;
	int16			target_x;
	int16			target_y;
	uint8			target_dir;
	int8			route;
	int32			walk_pc;
	_walkData		*walkAnim;

	//----------------------------------------------------------------------------------------
	// get the parameters

	ob_logic	= (Object_logic *)params[0];
	ob_graph	= (Object_graphic *)params[1];
	ob_mega		= (Object_mega *)params[2];

	target_x	= params[4];
	target_y	= params[5];
	target_dir	= params[6];

	//----------------------------------------------------------------------------------------
	// if this is the start of the walk, calculate route

	if (ob_logic->looping==0)
	{
		//---------------------------
		// If we're already there, don't even bother allocating memory and calling the router,
		// just quit back & continue the script!
		// This avoids an embarassing mega stand frame appearing for one cycle when we're already
		// in position for an anim eg. repeatedly clicking on same object to repeat an anim
		// - no mega frame will appear in between runs of the anim.
		
		if ((ob_mega->feet_x == target_x) && (ob_mega->feet_y == target_y)
			&& (ob_mega->current_dir == target_dir))
		{
			RESULT = 0;				// 0 means ok - finished walk
			return(IR_CONT);			// may as well continue the script
		}

		//---------------------------
		if ((params[6] < 0) || (params[6] > 8))	// invalid direction (NB. '8' means end walk on ANY direction)
			Con_fatal_error("Invalid direction (%d) in FN_walk (%s line %u)",params[6],__FILE__,__LINE__);
		//---------------------------

		ob_walkdata	= (Object_walkdata *)params[3];

		ob_mega->walk_pc=0;			//always

		AllocateRouteMem();	// set up mem for _walkData in route_slots[] & set mega's 'route_slot_id' accordingly
		route = RouteFinder(ob_mega, ob_walkdata, target_x, target_y, target_dir);

		/*
		if (id == PLAYER)
		{
			nExtraBars = 0;
			nExtraNodes = 0;
			if	((route == 1) || (route == 2))
			{
				megaOnGrid = 0;	// if we have just checked a grid with the mega on the grid take the mega off
				reRouteGeorge = 0;
			}
		}
		*/

		if	((route == 1) || (route == 2))	// 1=created route	2=zero route but may need to turn
		{
			//-------------------------------------------
			ob_logic->looping	= 1;		// so script FN_walk loop continues until end of walk-anim
											// need to animate the route now, so don't set result or return yet!

			ob_mega->currently_walking=1;	// started walk(James23jun97)
											// (see FN_get_player_savedata() in save_rest.cpp
			//-------------------------------------------
		}
		else							// 0=can't make route to target
		{
			FreeRouteMem();		// free up the walkdata mem block
			RESULT = 1;					// 1 means error, no walk created
			return(IR_CONT);			// may as well continue the script
		}

		// ok, walk is about to start, so set the mega's graphic resource
		ob_graph->anim_resource = ob_mega->megaset_res;
	}
	//----------------------------------------------------------------------------------------
	// double clicked an exit so quit the walk when screen is black

 	else if ((EXIT_FADING) && (GetFadeStatus()==RDFADE_BLACK))
	{
//		ok, thats it - back to script and change screen

		ob_logic->looping=0;	// so script loop stops
		FreeRouteMem();			// free up the walkdata mem block

		EXIT_CLICK_ID=0;		// must clear in-case on the new screen there's a walk instruction (which would get cut short)
//		EXIT_FADING=0;	// this will be reset when we change screens, so we can use it in script to check if a 2nd-click came along


		ob_mega->currently_walking=0;	// finished walk (James23jun97)
										// (see FN_get_player_savedata() in save_rest.cpp

		ob_mega->colliding=0;

		RESULT = 0;				// 0 means ok
		return(IR_CONT);		// continue the script so that RESULT can be checked!
	}
	//----------------------------------------------------------------------------------------
	// get pointer to walkanim & current frame position

	walkAnim	= LockRouteMem();	// lock the _walkData array
	walk_pc		= ob_mega->walk_pc;

	//----------------------------------------------------------------------------------------
	// if stopping the walk early, overwrite the next step with a slow-out, then finish

	if (Check_event_waiting())
	{
		if ((walkAnim[walk_pc].step == 0) && (walkAnim[walk_pc+1].step == 1))	// at the beginning of a step
		{
			ob_walkdata	= (Object_walkdata *)params[3];
			EarlySlowOut(ob_mega,ob_walkdata);
		}
	}
/*
	else if (CheckForCollision())
	{
		if ((walkAnim[walk_pc].step == 0) && (walkAnim[walk_pc+1].step == 1))	// at the beginning of a step
		{
			ob_walkdata	= (Object_walkdata *)params[3];
			EarlySlowOut(ob_mega,ob_walkdata);

			ob_mega->colliding=1;
		}
	}
*/
	//------------------------------------------------------------------
	// get new frame of walk

	ob_graph->anim_pc		= walkAnim[walk_pc].frame;
	ob_mega->current_dir	= walkAnim[walk_pc].dir;
	ob_mega->feet_x			= walkAnim[walk_pc].x;
	ob_mega->feet_y			= walkAnim[walk_pc].y;

	//------------------------------------------------------------------
	// check if NEXT frame is in fact the end-marker of the walk sequence
	// so we can return to script just as the final (stand) frame of the walk is set
	// - so that if followed by an anim, the anim's first frame replaces the final stand-frame
	// of the walk (see below)

	if (walkAnim[walk_pc+1].frame==512)	// '512' is end-marker
	{
		ob_logic->looping=0;	// so script loop stops
		FreeRouteMem();	// free up the walkdata mem block

		ob_mega->currently_walking=0;	// finished walk(James23jun97)
										// (see FN_get_player_savedata() in save_rest.cpp
/*
		if (ID==CUR_PLAYER_ID)
		{
			george_walking = 0;

			if (megaOnGrid == 2)
				megaOnGrid = 0;
		}
*/

		if (Check_event_waiting())	// if George's walk has been interrupted to run a new action script for instance
		{							// or Nico's walk has been interrupted by player clicking on her to talk
			ob_mega->colliding=0;	// Don't care about collision now we've got an event
			Start_event();
			RESULT = 1;				// 1 means didn't finish walk
			return(IR_TERMINATE);
		}
		else if (ob_mega->colliding)	// If we almost collided with another mega,
		{								// then we want to re-route from scratch.
			ob_mega->colliding=0;		// reset the flag now we've acknowledged the collision
			return(IR_REPEAT);			// Stop the script, but repeat this call next cycle
		}
		else
		{
			RESULT = 0;				// 0 means ok - finished walk
			return(IR_CONT);		// CONTINUE the script so that RESULT can be checked!
									// Also, if an anim command follows the FN_walk command,
									// the 1st frame of the anim (which is always a stand frame itself)
									// can replace the final stand frame of the walk, to hide the
									// slight difference between the shrinking on the mega frames
									// and the pre-shrunk anim start-frame.
		}
	}
	//----------------------------------------------------------------------------------------
	// increment the walkanim frame number, float the walkanim & come back next cycle

	ob_mega->walk_pc++;

	FloatRouteMem();	// allow _walkData array to float about memory again
	return(IR_REPEAT);	// stop the script, but repeat this call next cycle

	//------------------------------------------------------------------
}
//--------------------------------------------------------------------------------------
// walk mega to start position of anim

int32 FN_walk_to_anim(int32 *params)	// James (14nov96)
{
	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 pointer to object's mega structure
	//			3 pointer to object's walkdata structure
	//			4 anim resource id

	Object_logic	*ob_logic;
	uint8			*anim_file;
	_animHeader		*anim_head;
	int32			pars[7];

	//----------------------------------------------------------------------------------------
	// if this is the start of the walk, read anim file to get start coords

	ob_logic = (Object_logic *)params[0];

	if (ob_logic->looping==0)
	{
		anim_file = res_man.Res_open(params[4]);		// open anim file
		anim_head = FetchAnimHeader( anim_file );		// point to animation header

		pars[4] = anim_head->feetStartX;				// target_x
		pars[5] = anim_head->feetStartY;				// target_y
		pars[6] = anim_head->feetStartDir;				// target_dir

		res_man.Res_close(params[4]);					// close anim file

		//------------------------------------------
		if ((pars[4]==0)&&(pars[5]==0))					// if start coords not yet set in anim header
		{
			pars[4] = standby_x;						// use the standby coords
			pars[5] = standby_y;						// (which should be set beforehand in the script)
			pars[6] = standby_dir;

			Zdebug("WARNING: FN_walk_to_anim(%s) used standby coords", FetchObjectName(params[4]));
		}

		if ((pars[6] < 0) || (pars[6] > 7))				// check for invalid direction
			Con_fatal_error("Invalid direction (%d) in FN_walk_to_anim (%s line %u)", pars[6],__FILE__,__LINE__);

		//------------------------------------------
	}
	//----------------------------------------------------------------------------------------
	// set up the rest of the parameters for FN_walk()

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];	// walkdata - needed for EarlySlowOut if player clicks elsewhere during the walk

	//-------------------------------------------------------------------------------------------------------

	return FN_walk(pars);	// call FN_walk() with target coords set to anim start position
}

//--------------------------------------------------------------------------------------
// turn mega to <direction>
// just needs to call FN_walk() with current feet coords, so router can produce anim of turn frames

int32 FN_turn(int32 *params)	// James (15nov96)
{
	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 pointer to object's mega structure
	//			3 pointer to object's walkdata structure
	//			4 target direction

	Object_logic	*ob_logic;
	Object_mega		*ob_mega;
	int32			pars[7];

	// if this is the start of the turn, get the mega's current feet coords + the required direction

	ob_logic = (Object_logic *)params[0];

	if (ob_logic->looping==0)
	{
	 	//--------------------------------------------
		if ((params[4] < 0) || (params[4] > 7))	// invalid direction
			Con_fatal_error("Invalid direction (%d) in FN_turn (%s line %u)",params[4],__FILE__,__LINE__);
		//--------------------------------------------

	 	ob_mega = (Object_mega *)params[2];
	
		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = params[4];	// DIRECTION to turn to
	}

	//----------------------------------------------------------------------------------------
	// set up the rest of the parameters for FN_walk()

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	//----------------------------------------------------------------------------------------

	return FN_walk(pars);	// call FN_walk() with target coords set to feet coords
}
//--------------------------------------------------------------------------------------
// stand mega at (x,y,dir)
// sets up the graphic object, but also needs to set the new 'current_dir' in the mega object, so the router knows in future

int32 FN_stand_at(int32 *params)	// James
{
	// params:	0 pointer to object's graphic structure
	//			1 pointer to object's mega structure
	//			2 target x-coord
	//			3 target y-coord
	//			4 target direction

	Object_mega		*ob_mega;
	Object_graphic	*ob_graph;

	//----------------------------------------------------------------------------------------
	// check for invalid direction

	if ((params[4] < 0) || (params[4] > 7))	// invalid direction
		Con_fatal_error("Invalid direction (%d) in FN_stand_at (%s line %u)",params[4],__FILE__,__LINE__);

	//----------------------------------------------------------------------------------------
	// set up pointers to the graphic & mega structure

	ob_graph	= (Object_graphic *)params[0];
	ob_mega		= (Object_mega *)params[1];

	//----------------------------------------------------------------------------------------
	// set up the stand frame & set the mega's new direction

	ob_graph->anim_resource	= ob_mega->megaset_res;	// mega-set animation file
	ob_mega->feet_x			= params[2];			// x
	ob_mega->feet_y			= params[3];			// y
	ob_graph->anim_pc		= params[4]+96;			// dir + first stand frame (always frame 96)
	ob_mega->current_dir	= params[4];			// dir

 	//----------------------------------------------------------------------------------------

	return(IR_CONT);	// continue the script
}

//--------------------------------------------------------------------------------------
// stand mega in <direction> at current feet coords
// just needs to call FN_stand_at() with current feet coords

int32 FN_stand(int32 *params)	// James (15nov96)
{
	// params:	0 pointer to object's graphic structure
	//			1 pointer to object's mega structure
	//			2 target direction

	Object_mega *ob_mega = (Object_mega *)params[1];
	int32 pars[5];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = ob_mega->feet_x;
	pars[3] = ob_mega->feet_y;
	pars[4] = params[2];	// DIRECTION to stand in

	return FN_stand_at(pars);	// call FN_stand_at() with target coords set to feet coords
}
//--------------------------------------------------------------------------------------
// stand mega at end position of anim

int32 FN_stand_after_anim(int32 *params)	// James (14nov96)
{
	// params:	0 pointer to object's graphic structure
	//			1 pointer to object's mega structure
	//			2 anim resource id

	uint8 *anim_file;
	_animHeader *anim_head;
	int32 pars[5];

	//----------------------------------------------------------------------------------------
	// open the anim file & set up a pointer to the animation header

	anim_file = res_man.Res_open(params[2]);	// open anim file
	anim_head = FetchAnimHeader( anim_file );

	//----------------------------------------------------------------------------------------
	// set up the parameter list for FN_walk_to()

	pars[0] = params[0];
	pars[1] = params[1];

	pars[2] = anim_head->feetEndX;					// x
	pars[3] = anim_head->feetEndY;					// y
	pars[4] = anim_head->feetEndDir;				// dir

	//----------------------------------------------------------------------------------------

	if ((pars[2]==0)&&(pars[3]==0))					// if start coords not available either
	{
		pars[2] = standby_x;						// use the standby coords
		pars[3] = standby_y;						// (which should be set beforehand in the script)
		pars[4] = standby_dir;

		Zdebug("WARNING: FN_stand_after_anim(%s) used standby coords", FetchObjectName(params[2]));
	}

	if ((pars[4] < 0) || (pars[4] > 7))				// check for invalid direction
		Con_fatal_error("Invalid direction (%d) in FN_stand_after_anim (%s line %u)", pars[4],__FILE__,__LINE__);

	//----------------------------------------------------------------------------------------
	// close the anim file

	res_man.Res_close(params[2]);		// close anim file

	//----------------------------------------------------------------------------------------

	return FN_stand_at(pars);			// call FN_stand_at() with target coords set to anim end position
}

//--------------------------------------------------------------------------------------
// stand mega at start position of anim

int32 FN_stand_at_anim(int32 *params)	// James (07feb97)
{
	// params:	0 pointer to object's graphic structure
	//			1 pointer to object's mega structure
	//			2 anim resource id

	uint8 *anim_file;
	_animHeader *anim_head;
	int32 pars[5];

	//----------------------------------------------------------------------------------------
	// open the anim file & set up a pointer to the animation header

	anim_file = res_man.Res_open(params[2]);	// open anim file
	anim_head = FetchAnimHeader( anim_file );

	//----------------------------------------------------------------------------------------
	// set up the parameter list for FN_walk_to()

	pars[0] = params[0];
	pars[1] = params[1];

	pars[2] = anim_head->feetStartX;				// x
	pars[3] = anim_head->feetStartY;				// y
	pars[4] = anim_head->feetStartDir;				// dir

	if ((pars[2]==0)&&(pars[3]==0))					// if start coords not available
	{
		pars[2] = standby_x;						// use the standby coords
		pars[3] = standby_y;						// (which should be set beforehand in the script)
		pars[4] = standby_dir;

		Zdebug("WARNING: FN_stand_at_anim(%s) used standby coords", FetchObjectName(params[2]));
	}

	if ((pars[4] < 0) || (pars[4] > 7))				// check for invalid direction
		Con_fatal_error("Invalid direction (%d) in FN_stand_after_anim (%s line %u)", pars[4],__FILE__,__LINE__);

	//-------------------------------------------------------------------------------------------------------
	// close the anim file

	res_man.Res_close(params[2]);		// close anim file

	//-------------------------------------------------------------------------------------------------------

	return FN_stand_at(pars);			// call FN_stand_at() with target coords set to anim end position
}

//--------------------------------------------------------------------------------------
// Code to workout direction from start to dest

#define	diagonalx 36	// used in what_target not valid for all megas	jps 17mar95
#define	diagonaly 8


int What_target(int startX,	int	startY, int	destX, int	destY)		//S2.1(20Jul95JPS)
{
	int tar_dir;
//setting up
	int deltaX = destX-startX;
	int deltaY = destY-startY;
	int signX = (deltaX > 0);
	int signY = (deltaY > 0);
	int	slope;

	if ( (abs(deltaY) * diagonalx ) < (abs(deltaX) * diagonaly / 2))
	{
		slope = 0;// its flat
	}
	else if ( (abs(deltaY) * diagonalx / 2) > (abs(deltaX) * diagonaly ) )
	{
		slope = 2;// its vertical
	}
	else
	{
		slope = 1;// its diagonal
	}

	if (slope == 0) //flat
	{
		if (signX == 1)	// going right
		{
			tar_dir = 2;
		}
		else
		{
			tar_dir = 6;
		}
	}
	else if (slope == 2) //vertical
	{
		if (signY == 1)	// going down
		{
			tar_dir = 4;
		}
		else
		{
			tar_dir = 0;
		}
	}
	else if (signX == 1) //right diagonal
	{
		if (signY == 1)	// going down
		{
			tar_dir = 3;
		}
		else
		{
			tar_dir = 1;
		}
	}
 	else //left diagonal
	{
		if (signY == 1)	// going down
		{
			tar_dir = 5;
		}
		else
		{
			tar_dir = 7;
		}
	}
	return tar_dir;
}

//--------------------------------------------------------------------------------------
// turn mega to face point (x,y) on the floor
// just needs to call FN_walk() with current feet coords & direction computed by What_target()

int32 FN_face_xy(int32 *params)	// James (29nov96)
{
	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 pointer to object's mega structure
	//			3 pointer to object's walkdata structure
	//			4 target x-coord
	//			5 target y-coord

	Object_logic	*ob_logic;
	Object_mega		*ob_mega;
 	int32			pars[7];

	//----------------------------------------------------------------------------------------
	// if this is the start of the turn, get the mega's current feet coords + the required direction

	ob_logic = (Object_logic *)params[0];

	if (ob_logic->looping==0)
	{
	 	ob_mega = (Object_mega *)params[2];
	
		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = What_target( ob_mega->feet_x, ob_mega->feet_y, params[4], params[5] );	// set target direction
	}

	//----------------------------------------------------------------------------------------
	// set up the rest of the parameters for FN_walk()

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	//----------------------------------------------------------------------------------------

	return FN_walk(pars);	// call FN_walk() with target coords set to feet coords
}
//--------------------------------------------------------------------------------------
int32	FN_face_mega(int32	*params) 	//S2.1(3mar95jps)	Tony29Nov96
{
//params	0 pointer to object's logic structure
//			1 pointer to object's graphic structure
//			2 pointer to object's mega structure
//			3 pointer to object's walkdata structure

//			4 id of target mega to face

	uint32	null_pc=3;	//get ob_mega
	char	*raw_script_ad;
	int32	pars[7];
	Object_logic	*ob_logic;
	Object_mega		*ob_mega;
	_standardHeader	*head;


	ob_mega = (Object_mega *)params[2];
	ob_logic = (Object_logic *)params[0];


	if (ob_logic->looping==0)
	{

//		get targets info
		head = (_standardHeader*) res_man.Res_open(params[4]);
		if	(head->fileType!=GAME_OBJECT)
			Con_fatal_error("FN_face_mega %d not an object", params[4]);

		raw_script_ad = (char *)head;	// (head+1) + sizeof(_object_hub);	//get to raw script data

		RunScript( raw_script_ad, raw_script_ad, &null_pc );	//call the base script - this is the graphic/mouse service call
		res_man.Res_close(params[4]);

//		engine_mega is now the Object_mega of mega we want to turn to face

		pars[3] = params[3];
		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = What_target( ob_mega->feet_x, ob_mega->feet_y, engine_mega.feet_x, engine_mega.feet_y );
	}



	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	return FN_walk(pars);	// call FN_walk() with target coords set to feet coords

}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//	FN_walk		(here for reference instead of splitting a window)

	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 pointer to object's mega structure
	//			3 pointer to object's walkdata structure
	//			4 target x-coord
	//			5 target y-coord
	//			6 target direction
//------------------------------------------------------------------------------------
int32	FN_walk_to_talk_to_mega(int32	*params)	//Tony2Dec96
{
//we route to left or right hand side of target id if possible
//target is a shrinking mega

	Object_mega		*ob_mega;
	Object_logic	*ob_logic;

	uint32	null_pc=3;	//4th script - get mega
	char	*raw_script_ad;
	int32	pars[7];
	int	scale;
	int	mega_seperation=params[5];
	_standardHeader	*head;


//params	0 pointer to object's logic structure
//			1 pointer to object's graphic structure
//			2 pointer to object's mega structure
//			3 pointer to object's walkdata structure

//			4 id of target mega to face
//			5 distance


	ob_logic = (Object_logic*) params[0];
	ob_mega = (Object_mega*) params[2];

	pars[0] = params[0];	// standard stuff
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];			// walkdata



	if	(!ob_logic->looping)	//not been here before so decide where to walk-to
	{
//		first request the targets info
		head = (_standardHeader*) res_man.Res_open(params[4]);
		if	(head->fileType!=GAME_OBJECT)
			Con_fatal_error("FN_walk_to_talk_to_mega %d not an object", params[4]);

		raw_script_ad = (char *)head;	// (head+1) + sizeof(_object_hub);	//get to raw script data
		RunScript( raw_script_ad, raw_script_ad, &null_pc );	//call the base script - this is the graphic/mouse service call
		res_man.Res_close(params[4]);

//		engine_mega is now the Object_mega of mega we want to route to


		pars[5] = engine_mega.feet_y;	// stand exactly beside the mega, ie. at same y-coord


//		apply scale factor to walk distance
		scale = (ob_mega->scale_a * ob_mega->feet_y + ob_mega->scale_b)/256;	// Ay+B gives 256*scale ie. 256*256*true_scale for even better accuracy, ie. scale = (Ay+B)/256

		mega_seperation= (mega_seperation*scale)/256;

//		Zdebug("seperation %d", mega_seperation);
//		Zdebug(" target x %d, y %d", engine_mega.feet_x, engine_mega.feet_y);

		if	(engine_mega.feet_x < ob_mega->feet_x)	// target is left of us
		{
			pars[4] = engine_mega.feet_x+mega_seperation;	// so aim to stand to their right
			pars[6] = 5;	// face down_left
		}
		else										// ok, must be right of us
		{
			pars[4] = engine_mega.feet_x-mega_seperation;	// so aim to stand to their left
			pars[6] = 3;	// face down_right
		}
	}

  	//first cycle builds the route - thereafter merely follows it

	return FN_walk(pars);	//call FN_walk() with target coords set to feet coords
							//RESULT will be 1 when it finishes or 0 if it failed to build route
}
//------------------------------------------------------------------------------------
int32 FN_set_walkgrid(int32 *params)	// (6dec96 JEL)
{
	Con_fatal_error("FN_set_walkgrid no longer valid");
	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
// add this walkgrid resource to the list of those used for routing in this location
// - note this is ignored in the resource is already in the list

int32 FN_add_walkgrid(int32 *params)	// (03mar97 JEL)
{
	// params	0 id of walkgrid resource

	// all objects that add walkgrids must be restarted whenever we reneter a location

	if (ID != 8)	// DON'T EVER KILL GEORGE!
		FN_add_to_kill_list(params);// need to call this in case it wasn't called in script! ('params' just used as dummy param)

	AddWalkGrid(params[0]);

	res_man.Res_open(params[0]);	// Touch the grid, getting it into memory.
	res_man.Res_close(params[0]);

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
// remove this walkgrid resource from the list of those used for routing in this location
// - note that this is ignored if the resource isn't actually in the list

int32 FN_remove_walkgrid(int32 *params)	// (03mar97 JEL)
{
	// params	0 id of walkgrid resource

	RemoveWalkGrid(params[0]);

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_register_walkgrid(int32 *params)
{
	Con_fatal_error("FN_register_walkgrid no longer valid");
	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_set_scaling(int32 *params)	// (6dec96 JEL)
{
	// params	0 pointer to object's mega structure
	//				1 scale constant A
	//				2 scale constant B

	// 256*s = A*y + B

	// where s is system scale, which itself is (256 * actual_scale) ie. s==128 is half size

 	Object_mega *ob_mega = (Object_mega *) params[0];

	ob_mega->scale_a = params[1];
	ob_mega->scale_b = params[2];

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_set_standby_coords(int32 *params)	// (10dec97 JEL)
{
	// set the standby walk coords to be used by FN_walk_to_anim & FN_stand_after_anim
	// when the anim header's start/end coords are zero

	// useful during development; can stay in final game anyway

	// params	0 x-coord
	//			1 y-coord
	//			2 direction (0..7)

	//----------------------------------------------------------------------------------------
	// check for invalid direction

	if ((params[2] < 0) || (params[2] > 7))	// invalid direction
		Con_fatal_error("Invalid direction (%d) in FN_set_standby_coords (%s line %u)",params[2],__FILE__,__LINE__);

	//----------------------------------------------------------------------------------------

	standby_x	= params[0];
	standby_y	= params[1];
	standby_dir	= params[2];

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
