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

#include "bs2/object.h"

namespace Sword2 {

struct _event_unit {
	uint32 id;
	uint32 interact_id;
};

#define	MAX_events 10

extern _event_unit event_list[MAX_events];

void Init_event_system(void);
int32 FN_set_event(int32 *params);
void Set_player_action_event(uint32 id, uint32 interact_id);
int32 FN_check_event_waiting(void);
void Start_event(void);
int32 FN_start_event(void);
uint32 Check_event_waiting(void);
void Kill_all_ids_events(uint32 id);

#ifdef _SWORD2_DEBUG 
uint32 CountEvents(void);
#endif 

} // End of namespace Sword2

#endif
