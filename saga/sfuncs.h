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

int SF_sleep(R_SCRIPTFUNC_PARAMS);
int SF_3(R_SCRIPTFUNC_PARAMS);
int SF_setCommandText(R_SCRIPTFUNC_PARAMS);
int SF_actorWalkTo(R_SCRIPTFUNC_PARAMS);
int SF_setFacing(R_SCRIPTFUNC_PARAMS);
int SF_freezeInterface(R_SCRIPTFUNC_PARAMS);
int SF_startAnim(R_SCRIPTFUNC_PARAMS);
int SF_actorWalkToAsync(R_SCRIPTFUNC_PARAMS);
int SF_moveTo(R_SCRIPTFUNC_PARAMS);
int SF_actorWalk(R_SCRIPTFUNC_PARAMS);
int SF_cycleActorFrames(R_SCRIPTFUNC_PARAMS);
int SF_setFrame(R_SCRIPTFUNC_PARAMS);
int SF_linkAnim(R_SCRIPTFUNC_PARAMS);
int SF_placeActor(R_SCRIPTFUNC_PARAMS);
int SF_checkUserInterrupt(R_SCRIPTFUNC_PARAMS);
int SF_moveRelative(R_SCRIPTFUNC_PARAMS);
int SF_doAction(R_SCRIPTFUNC_PARAMS);
int SF_faceTowards(R_SCRIPTFUNC_PARAMS);
int SF_setFollower(R_SCRIPTFUNC_PARAMS);
int SF_centerActor(R_SCRIPTFUNC_PARAMS);
int SF_setActorState(R_SCRIPTFUNC_PARAMS);
int SF_swapActors(R_SCRIPTFUNC_PARAMS);
int SF_scriptSpecialWalk(R_SCRIPTFUNC_PARAMS);
int SF_walkRelative(R_SCRIPTFUNC_PARAMS);
int SF_throwActor(R_SCRIPTFUNC_PARAMS);
int SF_waitWalk(R_SCRIPTFUNC_PARAMS);
int SF_changeActorScene(R_SCRIPTFUNC_PARAMS);
int SF_climb(R_SCRIPTFUNC_PARAMS);
int SF_setActorZ(R_SCRIPTFUNC_PARAMS);
int SF_getActorX(R_SCRIPTFUNC_PARAMS);
int SF_getActorY(R_SCRIPTFUNC_PARAMS);

}

#endif
