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
/*

 Description:   
 
    Scripting module script function component

 Notes: 
*/

#include "reinherit.h"

#include "yslib.h"

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "actor_mod.h"
#include "animation_mod.h"
#include "console_mod.h"
#include "interface_mod.h"
#include "text_mod.h"

/*
\*--------------------------------------------------------------------------*/
#include "script.h"
#include "sfuncs.h"

namespace Saga {

R_SFUNC_ENTRY SFuncList[R_SFUNC_NUM] = {

	{0, 0, NULL}
	,
	{1, 1, SF_Sleep}
	,
	{2, 0, NULL}
	,
	{3, 1, SF_3}
	,
	{4, 1, SF_SetCommandText}
	,
	{5, 0, NULL}
	,
	{6, 3, SF_ActorWalkTo}
	,
	{7, 0, NULL}
	,
	{8, 2, SF_SetActorOrient}
	,
	{9, 0, NULL}
	,
	{10, 0, NULL}
	,
	{11, 1, SF_FreezeInterface}
	,
	{12, 0, NULL}
	,
	{13, 0, NULL}
	,
	{14, 0, NULL}
	,
	{15, 0, NULL}
	,
	{16, 0, NULL}
	,
	{17, 0, NULL}
	,
	{18, 0, NULL}
	,
	{19, 0, NULL}
	,
	{20, 0, NULL}
	,
	{21, 0, NULL}
	,
	{22, 0, NULL}
	,
	{23, 0, NULL}
	,
	{24, 0, NULL}
	,
	{25, 0, NULL}
	,
	{26, 3, SF_StartAnim}
	,
	{27, 3, SF_ActorWalkToAsync}
	,
	{28, 0, NULL}
	,
	{29, 0, NULL}
	,
	{30, 3, SF_PlaceActor}
	,
	{31, 0, NULL}
	,
	{32, 0, NULL}
	,
	{33, 0, NULL}
	,
	{34, 0, NULL}
	,
	{35, 0, NULL}
	,
	{36, 4, SF_ActorWalkTo2}
	,
	{37, 4, SF_SetActorAct}
	,
	{38, 3, SF_SetActorAct2}
	,
	{39, 0, NULL}
	,
	{40, 0, NULL}
	,
	{41, 4, SF_LinkAnim}
	,
	{42, 0, NULL}
	,
	{43, 6, SF_PlaceActorEx}
	,
	{44, 0, SF_CheckUserInterrupt}
	,
	{45, 0, NULL}
	,
	{46, 0, NULL}
	,
	{47, 0, NULL}
	,
	{48, 0, NULL}
	,
	{49, 0, NULL}
	,
	{50, 0, NULL}
	,
	{51, 0, NULL}
	,
	{52, 0, NULL}
	,
	{53, 0, NULL}
	,
	{54, 0, NULL}
	,
	{55, 0, NULL}
	,
	{56, 0, NULL}
	,
	{57, 0, NULL}
	,
	{58, 0, NULL}
	,
	{59, 0, NULL}
	,
	{60, 0, NULL}
	,
	{61, 0, NULL}
	,
	{62, 0, NULL}
	,
	{63, 0, NULL}
	,
	{64, 0, NULL}
	,
	{65, 0, NULL}
	,
	{66, 0, NULL}
	,
	{67, 0, NULL}
	,
	{68, 0, NULL}
	,
	{69, 0, NULL}
	,
	{70, 0, NULL}
	,
	{71, 0, NULL}
	,
	{72, 0, NULL}
	,
	{73, 0, NULL}
	,
	{74, 0, NULL}
	,
	{75, 0, NULL}
	,
	{76, 0, NULL}
	,
	{77, 0, NULL}
};

int SF_Sleep(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #1 (0x01) blocking
 *
 * Suspends thread execution for the specified time period
 *
 * Param1: time to suspend ( units? )
 *
\*--------------------------------------------------------------------------*/
{
	SDataWord_T time_param;

	int time;

	SSTACK_Pop(thread->stack, &time_param);

	time = SDATA_ReadWordU(time_param);

	thread->sleep_time = time * 10;

	return R_SUCCESS;
}

int SF_3(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #3 (0x03)
 *
 * Unknown function; pops a parameter and pushes a return value
 *
 * Param1: unknown
 *
\*--------------------------------------------------------------------------*/
{
	/* INCOMPLETE */

	SDataWord_T param1;

	SSTACK_Pop(thread->stack, &param1);

	SSTACK_Push(thread->stack, 0);	/* push for now to allow intro faire 
					 * setup to run completely */

	return R_SUCCESS;
}

int SF_SetCommandText(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #4 (0x04) nonblocking
 *
 * Set the command display to the specified text string
 *
 * Param1: dialogue index of string
 *
\*--------------------------------------------------------------------------*/
{
	SDataWord_T s_idx_parm;

	SSTACK_Pop(thread->stack, &s_idx_parm);

	/* INCOMPLETE */

	return R_SUCCESS;
}

int SF_ActorWalkTo(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #6 (0x06) blocking
 *
 * Commands the specified actor to walk to the given position
 *
 * Param1: actor id
 * Param2: actor destination x
 * Param3: actor destination y
 *
\*--------------------------------------------------------------------------*/
{
	SDataWord_T actor_parm;

	SDataWord_T x_parm;
	SDataWord_T y_parm;

	int actor_id;
	int actor_idx;

	R_POINT pt;

	SSTACK_Pop(thread->stack, &actor_parm);

	SSTACK_Pop(thread->stack, &x_parm);
	SSTACK_Pop(thread->stack, &y_parm);

	actor_id = SDATA_ReadWordS(actor_parm);

	actor_idx = ACTOR_GetActorIndex(actor_id);
	if (actor_idx < 0) {
		CON_Print(S_WARN_PREFIX "SF.08: Actor id 0x%X not found.",
		    actor_id);
		return R_FAILURE;
	}

	pt.x = SDATA_ReadWordS(x_parm);
	pt.y = SDATA_ReadWordS(y_parm);

	ACTOR_WalkTo(actor_idx, &pt, 0, &thread->sem);

	return R_SUCCESS;
}

int SF_SetActorOrient(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #8 (0x08) nonblocking
 *
 * Sets the orientation of the specified actor.
 *
 * Param1: actor id
 * Param2: actor orientation
 *
\*--------------------------------------------------------------------------*/
{
	SDataWord_T actor_parm;
	SDataWord_T orient_parm;

	int actor_id;
	int actor_idx;
	int orientation;

	SSTACK_Pop(thread->stack, &actor_parm);
	SSTACK_Pop(thread->stack, &orient_parm);

	actor_id = SDATA_ReadWordS(actor_parm);
	orientation = SDATA_ReadWordS(orient_parm);

	actor_idx = ACTOR_GetActorIndex(actor_id);
	if (actor_idx < 0) {
		CON_Print(S_WARN_PREFIX "SF.08: Actor id 0x%X not found.",
		    actor_id);

		return R_FAILURE;
	}

	ACTOR_SetOrientation(actor_idx, orientation);
	return R_SUCCESS;
}

int SF_FreezeInterface(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #11 (0x0B) nonblocking
 *
 * If the parameter is true, the user interface is disabled while script 
 * continues to run. If the parameter is false, the user interface is 
 * reenabled.
 *
 * Param1: boolean
 *
\*--------------------------------------------------------------------------*/
{
	SDataWord_T b_param;

	SSTACK_Pop(thread->stack, &b_param);

	if (b_param) {
		INTERFACE_Deactivate();
	} else {
		INTERFACE_Activate();
	}

	return R_SUCCESS;
}

int SF_StartAnim(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #26 (0x1A) nonblocking
 *
 * Starts the specified animation 
 *
 * Param1: ?
 * Param2: frames of animation to play or -1 to loop
 * Param3: animation id
 *
\*--------------------------------------------------------------------------*/
{
	SDataWord_T unk_parm;
	SDataWord_T frame_parm;
	SDataWord_T anim_id_parm;

	int frame_count;
	int anim_id;

	SSTACK_Pop(thread->stack, &anim_id_parm);
	SSTACK_Pop(thread->stack, &frame_parm);
	SSTACK_Pop(thread->stack, &unk_parm);

	frame_count = SDATA_ReadWordS(frame_parm);
	anim_id = SDATA_ReadWordS(anim_id_parm);

	if (ANIM_Play(anim_id, 0) != R_SUCCESS) {

		CON_Print(S_WARN_PREFIX
		    "SF.26: ANIM_Play() failed. Anim id: %u\n", anim_id);

		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SF_ActorWalkToAsync(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #27 (0x1B) nonblocking
 *
 * Commands the specified actor to walk to the given position
 *
 * Param1: actor id
 * Param2: actor destination x
 * Param3: actor destination y
 *
\*--------------------------------------------------------------------------*/
{
	SDataWord_T actor_parm;

	SDataWord_T x_parm;
	SDataWord_T y_parm;

	int actor_id;
	int actor_idx;

	R_POINT pt;

	SSTACK_Pop(thread->stack, &actor_parm);

	SSTACK_Pop(thread->stack, &x_parm);
	SSTACK_Pop(thread->stack, &y_parm);

	actor_id = SDATA_ReadWordS(actor_parm);

	actor_idx = ACTOR_GetActorIndex(actor_id);
	if (actor_idx < 0) {
		CON_Print(S_WARN_PREFIX "SF.08: Actor id 0x%X not found.",
		    actor_id);
		return R_FAILURE;
	}

	pt.x = SDATA_ReadWordS(x_parm);
	pt.y = SDATA_ReadWordS(y_parm);

	ACTOR_WalkTo(actor_idx, &pt, 0, NULL);

	return R_SUCCESS;
}

int SF_PlaceActor(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #30 (0x1E) nonblocking
 *
 * Positions an actor at the specified location; actor is created if the 
 *  actor does not already exist.
 *
 * Param1: actor id
 * Param2: actor pos x
 * Param3: actor pos y
 *
\*--------------------------------------------------------------------------*/
{
	SDataWord_T actor_parm;

	SDataWord_T x_parm;
	SDataWord_T y_parm;

	int actor_id;
	int actor_idx;
	int result;

	R_POINT pt;

	SSTACK_Pop(thread->stack, &actor_parm);

	SSTACK_Pop(thread->stack, &x_parm);
	SSTACK_Pop(thread->stack, &y_parm);

	actor_id = SDATA_ReadWordS(actor_parm);

	pt.x = SDATA_ReadWordS(x_parm);
	pt.y = SDATA_ReadWordS(y_parm);

	if (!ACTOR_ActorExists(actor_id)) {

		result = ACTOR_Create(actor_id, pt.x, pt.y);

		if (result != R_SUCCESS) {

			CON_Print(S_WARN_PREFIX
			    "SF.30: Couldn't create actor 0x%X.", actor_id);
			return R_FAILURE;
		}
	} else {

		actor_idx = ACTOR_GetActorIndex(actor_id);

		ACTOR_Move(actor_idx, &pt);
	}

	return R_SUCCESS;
}

int SF_ActorWalkTo2(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #36 (0x24) ?
 *
 * Commands the specified actor to walk to the given position
 *
 * Param1: actor id
 * Param2: actor destination x
 * Param3: actor destination y
 * Param4: unknown
 *
\*--------------------------------------------------------------------------*/
{
	/* INCOMPLETE */

	SDataWord_T actor_parm;

	SDataWord_T x_parm;
	SDataWord_T y_parm;
	SDataWord_T unk_parm;

	int actor_idx;

	R_POINT pt;

	SSTACK_Pop(thread->stack, &actor_parm);

	SSTACK_Pop(thread->stack, &x_parm);
	SSTACK_Pop(thread->stack, &y_parm);
	SSTACK_Pop(thread->stack, &unk_parm);

	actor_idx = ACTOR_GetActorIndex(SDATA_ReadWordS(actor_parm));
	if (actor_idx < 0) {
		CON_Print(S_WARN_PREFIX "SF.36: Actor id 0x%X not found.",
		    (int)actor_parm);
		return R_FAILURE;
	}

	pt.x = SDATA_ReadWordS(x_parm);
	pt.y = SDATA_ReadWordS(y_parm);

#if 1
	ACTOR_WalkTo(actor_idx, &pt, 0, NULL);
#else
	ACTOR_WalkTo(actor_idx, &pt, 0, &thread->sem);
#endif

	return R_SUCCESS;
}

int SF_SetActorAct(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #37 (0x25) nonblocking
 *
 * Sets an actor to the specified action state
 *
 * Param1: actor id
 * Param2: unknown
 * Param3: actor action state
 * Param4: unknown
 *
\*--------------------------------------------------------------------------*/
{
	/* INCOMPLETE */

	SDataWord_T actor_parm;

	SDataWord_T unk1_parm;
	SDataWord_T unk2_parm;
	SDataWord_T action_parm;

	int actor_id;
	int actor_idx;

	int action;
	/*uint flags; */

	SSTACK_Pop(thread->stack, &actor_parm);

	SSTACK_Pop(thread->stack, &unk1_parm);
	SSTACK_Pop(thread->stack, &action_parm);
	SSTACK_Pop(thread->stack, &unk2_parm);

	actor_id = SDATA_ReadWordS(actor_parm);
	action = SDATA_ReadWordS(action_parm);

	actor_idx = ACTOR_GetActorIndex(actor_id);

	if (ACTOR_SetAction(actor_idx, action, ACTION_NONE) != R_SUCCESS) {
		CON_Print(S_WARN_PREFIX "SF.37: ACTOR_SetAction() failed.");

		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SF_SetActorAct2(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #38 (0x26) nonblocking
 *
 * Sets an actor to the specified action state
 *
 * Param1: actor id
 * Param2: actor action state
 * Param3: unknown
 *
\*--------------------------------------------------------------------------*/
{
	/* INCOMPLETE */

	SDataWord_T actor_parm;

	SDataWord_T unk1_parm;
	SDataWord_T action_parm;

	int actor_id;
	int actor_idx;

	int action;
	/*uint flags; */

	SSTACK_Pop(thread->stack, &actor_parm);

	SSTACK_Pop(thread->stack, &action_parm);
	SSTACK_Pop(thread->stack, &unk1_parm);

	actor_id = SDATA_ReadWordS(actor_parm);
	action = SDATA_ReadWordS(action_parm);

	actor_idx = ACTOR_GetActorIndex(actor_id);

	if (ACTOR_SetAction(actor_idx, action, ACTION_NONE) != R_SUCCESS) {
		CON_Print(S_WARN_PREFIX "SF.38: ACTOR_SetAction() failed.");

		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SF_LinkAnim(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #41 (0x29) nonblocking
 *
 * Links the specified animations for playback
 *
 * Param1: ?
 * Param2: total linked frame count
 * Param3: animation id link target
 * Param4: animation id link source
 *
\*--------------------------------------------------------------------------*/
{
	SDataWord_T unk_parm;

	SDataWord_T tframes_parm;
	SDataWord_T anim1_parm;
	SDataWord_T anim2_parm;

	int tframes;

	uint anim_id1;
	uint anim_id2;

	SSTACK_Pop(thread->stack, &anim1_parm);
	SSTACK_Pop(thread->stack, &anim2_parm);

	SSTACK_Pop(thread->stack, &tframes_parm);
	SSTACK_Pop(thread->stack, &unk_parm);

	tframes = SDATA_ReadWordS(tframes_parm);

	anim_id1 = SDATA_ReadWordU(anim1_parm);
	anim_id2 = SDATA_ReadWordU(anim2_parm);

	if (ANIM_Link(anim_id1, anim_id2) != R_SUCCESS) {

		CON_Print(S_WARN_PREFIX
		    "SF.41: ANIM_Link() failed. (%u->%u)\n", anim_id1,
		    anim_id2);

		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SF_PlaceActorEx(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #43 (0x2B) nonblocking
 *
 * Positions an actor at the specified location; actor is created if the 
 *  actor does not already exist.
 *
 * Param1: actor id
 * Param2: actor pos x
 * Param3: actor pos y
 * Param4: ?
 * Param5: actor action
 * Param6: ?
 *
\*--------------------------------------------------------------------------*/
{
	/* INCOMPLETE */

	SDataWord_T actor_parm;

	SDataWord_T x_parm;
	SDataWord_T y_parm;

	SDataWord_T action_parm;

	SDataWord_T unknown_parm;

	int actor_id;
	int actor_idx;
	int action_state;
	int result;

	R_POINT pt;

	SSTACK_Pop(thread->stack, &actor_parm);

	SSTACK_Pop(thread->stack, &x_parm);
	SSTACK_Pop(thread->stack, &y_parm);

	SSTACK_Pop(thread->stack, &unknown_parm);
	SSTACK_Pop(thread->stack, &action_parm);
	SSTACK_Pop(thread->stack, &unknown_parm);

	actor_id = SDATA_ReadWordS(actor_parm);

	pt.x = SDATA_ReadWordS(x_parm);
	pt.y = SDATA_ReadWordS(y_parm);

	action_state = SDATA_ReadWordU(action_parm);

	if (!ACTOR_ActorExists(actor_id)) {

		result = ACTOR_Create(actor_id, pt.x, pt.y);

		if (result != R_SUCCESS) {

			CON_Print(S_WARN_PREFIX
			    "SF.43: Couldn't create actor 0x%X.", actor_id);

			return R_FAILURE;
		}
	} else {

		actor_idx = ACTOR_GetActorIndex(actor_id);

		ACTOR_Move(actor_idx, &pt);
	}

	actor_idx = ACTOR_GetActorIndex(actor_id);

	ACTOR_SetDefaultAction(actor_idx, action_state, ACTION_NONE);
	ACTOR_SetAction(actor_idx, action_state, ACTION_NONE);

	return R_SUCCESS;
}

int SF_CheckUserInterrupt(R_SCRIPTFUNC_PARAMS)
/*--------------------------------------------------------------------------*\
 * 
 * Script function #44 (0x2C) nonblocking
 *
 * Checks to see if the user has interrupted a currently playing 
 * game cinematic. Pushes a zero or positive value if the game 
 * has not been interrupted.
 *
\*--------------------------------------------------------------------------*/
{
	SSTACK_Push(thread->stack, 0);

	/* INCOMPLETE */

	return R_SUCCESS;
}

} // End of namespace Saga
