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

// Actor management module public header file

#ifndef SAGA_ACTOR_MOD_H
#define SAGA_ACTOR_MOD_H

namespace Saga {

enum R_ACTOR_WALKFLAGS {
	WALK_NONE = 0x00,
	WALK_NOREORIENT = 0x01
};

enum R_ACTOR_ORIENTATIONS {
	ORIENT_N = 0,
	ORIENT_NE = 1,
	ORIENT_E = 2,
	ORIENT_SE = 3,
	ORIENT_S = 4,
	ORIENT_SW = 5,
	ORIENT_W = 6,
	ORIENT_NW = 7
};

enum R_ACTOR_ACTIONS {
	ACTION_IDLE = 0,
	ACTION_WALK = 1,
	ACTION_SPEAK = 2,
	ACTION_COUNT
};

enum R_ACTOR_ACTIONFLAGS {
	ACTION_NONE = 0x00,
	ACTION_LOOP = 0x01
};

int ACTOR_Register();
int ACTOR_Init();
int ACTOR_Shutdown();

int ACTOR_Direct(int msec);

int ACTOR_Create(int actor_id, int x, int y);
int ACTOR_ActorExists(uint16 actor_id);

int ACTOR_DrawList();
int ACTOR_AtoS(R_POINT *logical, const R_POINT *actor);
int ACTOR_StoA(R_POINT *actor, const R_POINT *screen);

int ACTOR_Move(int index, R_POINT *move_pt);
int ACTOR_MoveRelative(int index, R_POINT *move_pt);

int ACTOR_WalkTo(int index, R_POINT *walk_pt, uint16 flags, R_SEMAPHORE *sem);

int ACTOR_GetActorIndex(uint16 actor_id);

int ACTOR_Speak(int index, const char *d_string, uint16 d_voice_rn, R_SEMAPHORE *sem);

int ACTOR_SkipDialogue();

int ACTOR_GetSpeechTime(const char *d_string, uint16 d_voice_rn);
int ACTOR_SetOrientation(int index, int orient);
int ACTOR_SetAction(int index, int action_n, uint16 action_flags);
int ACTOR_SetDefaultAction(int index, int action_n, uint16 action_flags);

} // End of namespace Saga

#endif
