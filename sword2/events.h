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

#ifndef	_EVENTS
#define	_EVENTS

//#include "src\driver96.h"
#include "object.h"


typedef	struct
{
	uint32	id;
	uint32	interact_id;
}	_event_unit;

#define	MAX_events	10

extern _event_unit	event_list[MAX_events];

void	Init_event_system(void);	//Tony4Dec96
int32	FN_set_event(int32 *params);	//Tony13Nov96
void	Set_player_action_event(uint32	id, uint32	interact_id);	//Tony4Dec96
int32	FN_check_event_waiting(void);	//Tony4Dec96
void	Start_event(void);	//Tony4Dec96
int32	FN_start_event(void);	//Tony4Dec96
uint32	Check_event_waiting(void);	//Tony4Dec96
void	Kill_all_ids_events(uint32	id);	//Tony18Dec96
uint32	CountEvents(void);	// James11july97


#endif
