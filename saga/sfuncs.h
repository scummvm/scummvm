/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Scripting module script function component header file

#ifndef SAGA_SFUNCS_H
#define SAGA_SFUNCS_H

#include "sthread.h"

namespace Saga {

#define R_SFUNC_NUM 78

#define R_SCRIPTFUNC_PARAMS R_SCRIPT_THREAD *thread

typedef int (*SFunc_T) (R_SCRIPTFUNC_PARAMS);

struct R_SFUNC_ENTRY {
	int sfunc_num;
	int sfunc_argc;
	SFunc_T sfunc_fp;
};

extern R_SFUNC_ENTRY SFuncList[];

// SF 1
int SF_Sleep(R_SCRIPTFUNC_PARAMS);
// SF 3
int SF_3(R_SCRIPTFUNC_PARAMS);
// SF 4
int SF_SetCommandText(R_SCRIPTFUNC_PARAMS);
// SF 6
int SF_ActorWalkTo(R_SCRIPTFUNC_PARAMS);
// SF 8
int SF_SetActorOrient(R_SCRIPTFUNC_PARAMS);
// SF 11
int SF_FreezeInterface(R_SCRIPTFUNC_PARAMS);
// SF 26
int SF_StartAnim(R_SCRIPTFUNC_PARAMS);
// SF 27
int SF_ActorWalkToAsync(R_SCRIPTFUNC_PARAMS);
// SF 30
int SF_PlaceActor(R_SCRIPTFUNC_PARAMS);
// SF 36
int SF_ActorWalkTo2(R_SCRIPTFUNC_PARAMS);
// SF 37
int SF_SetActorAct(R_SCRIPTFUNC_PARAMS);
// SF 38
int SF_SetActorAct2(R_SCRIPTFUNC_PARAMS);
// SF 41
int SF_LinkAnim(R_SCRIPTFUNC_PARAMS);
// SF 43
int SF_PlaceActorEx(R_SCRIPTFUNC_PARAMS);
// SF 44
int SF_CheckUserInterrupt(R_SCRIPTFUNC_PARAMS);

} // End of namespace Saga

#endif
