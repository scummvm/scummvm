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

//logic management

#ifndef	_LOGIC
#define	_LOGIC

//#include "src\driver96.h"
#include "defs.h"
#include "header.h"

#define	TREE_SIZE	3



class	logic
{
	public:

	int		Process_session(void);	//do one cycle of the current session
	void	Express_change_session(uint32	sesh_id);	//cause the logic loop to terminate and drop out
	void	Natural_change_session(uint32	sesh_id);	//new logic begins next cycle
	uint32	Return_run_list(void);
	void	Logic_up(uint32	new_script);	//setup script_id and script_pc in cur_object_hub - called by FN_gosub()
	void	Logic_replace(uint32	new_script);
	void	Logic_one(uint32	new_script);
	void	Total_restart(void);
	uint32	Examine_run_list(void);
	void	Reset_kill_list(void);	//James 25mar97


	private:

  	uint32	current_run_list;	//denotes the res id of the game-object-list in current use
	void	Process_kill_list(void);
	uint32	pc;	//pc during logic loop
	_object_hub	*cur_object_hub;	//each object has one of these tacked onto the beginning

};

extern	logic	LLogic;

int32 FN_add_to_kill_list(int32 *params);	//James9jan97

#endif
