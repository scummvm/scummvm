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
#include "console.h"
#include "debug.h"
#include "defs.h"
#include "interpreter.h"
#include "memory.h"
#include "object.h"
#include "sync.h"
//------------------------------------------------------------------------------------
typedef	struct
{
	uint32	id;
	uint32	sync;
}	_sync_unit;	//haaaaaaaa

#define	MAX_syncs	10	//there wont be many will there. probably 2 at most i reckon


_sync_unit	sync_list[MAX_syncs];

//------------------------------------------------------------------------------------
void	Init_sync_system(void)	//Tony27Nov96
{
//set list to 0's

	uint32	j;



	for	(j=0;j<MAX_syncs;j++)
		sync_list[j].id=0;


}
//------------------------------------------------------------------------------------
int32	FN_send_sync(int32 *params)	//Tony27Nov96
{
//param	0 sync's recipient
//param	1 sync value


	uint32	current_sync=0;


	if	(sync_list[current_sync].id)
	{
		do
			current_sync++;
		while(sync_list[current_sync].id);	//zip along until we find a free slot
		
	}

//	Zdebug(" %d sending sync %d to %d", ID, params[1], params[0]);


	sync_list[current_sync].id=params[0];
	sync_list[current_sync].sync=params[1];



	return(IR_CONT);
}
//------------------------------------------------------------------------------------
void	Clear_syncs(uint32	id)	//Tony27Nov96
{
//clear any syncs registered for this id
//call this just after the id has been processed

	uint32	j;


//there could in theory be more than one sync waiting for us so clear the lot

	for	(j=0;j<MAX_syncs;j++)
		if	(sync_list[j].id==id)
		{	//Zdebug("removing sync %d for %d", j, id);
			sync_list[j].id=0;
		}


}
//------------------------------------------------------------------------------------
uint32	Get_sync(void)	//Tony27Nov96
{
	// check for a sync waiting for this character
	// - called from system code eg. from inside FN_anim(), to see if animation to be quit

	uint32	j;


	for	(j=0;j<MAX_syncs;j++)
		if	(sync_list[j].id == ID)
			return(1);	//means sync found	Tony12July97

//			return(sync_list[j].sync);	//return sync value waiting



	return(0);	//no sync found

}
//------------------------------------------------------------------------------------
int32	FN_get_sync(int32	*params)	//Tony27Nov96
{
// check for a sync waiting for this character
// - called from script

//params	none


	uint32	j;


	for	(j=0;j<MAX_syncs;j++)
		if	(sync_list[j].id == ID) {
			RESULT=sync_list[j].sync;
			return(IR_CONT);	//return sync value waiting
		}

	RESULT=0;

	// if (params); what is the point of this... khalek

	return(IR_CONT);	//no sync found

}
//------------------------------------------------------------------------------------
int32	FN_wait_sync(int32	*params)	//Tony27Nov96
{
//keep calling until a sync recieved

//params	none


	uint32	j;

	j=ID;


//	Zdebug("%d waits", ID);

	
	for (j=0;j<MAX_syncs;j++) {
		if (sync_list[j].id == ID) {
			RESULT=sync_list[j].sync;
			//Zdebug(" go");
			return(IR_CONT);	//return sync value waiting
		}
	}

	// if (params); // what is the point of this... - khalek

	return(IR_REPEAT);	//back again next cycle

}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

