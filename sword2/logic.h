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

// logic management

#ifndef _LOGIC
#define _LOGIC

#include "defs.h"
#include "header.h"

namespace Sword2 {

#define	TREE_SIZE 3

class logic {
public:
	//do one cycle of the current session
	int processSession(void);

	// cause the logic loop to terminate and drop out
	void expressChangeSession(uint32 sesh_id);

	// new logic begins next cycle
	void naturalChangeSession(uint32 sesh_id);

	uint32 getRunList(void);

	// setup script_id and script_pc in _curObjectHub - called by
	// FN_gosub()
	void logicUp(uint32 new_script);

	void logicReplace(uint32 new_script);
	void logicOne(uint32 new_script);
	void totalRestart(void);
	uint32 examineRunList(void);
	void resetKillList(void);

private:
	// denotes the res id of the game-object-list in current use
	uint32 _currentRunList;

	void processKillList(void);

	//pc during logic loop
	uint32 _pc;

	// each object has one of these tacked onto the beginning
	_object_hub *_curObjectHub;
};

extern logic LLogic;

int32 FN_add_to_kill_list(int32 *params);

} // End of namespace Sword2

#endif
