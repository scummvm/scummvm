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

// Scripting module thread management component header file

#ifndef SAGA_STHREAD_H__
#define SAGA_STHREAD_H__

#include "sstack.h"

namespace Saga {

#define STHREAD_DEF_INSTR_COUNT 7

struct R_SEMAPHORE_tag {
	int hold_count;
};

struct R_SCRIPT_THREAD_tag {
	int executing;

	int sleep_time;
	int ep_num; // Entrypoint number
	unsigned long ep_offset; // Entrypoint offset
	unsigned long i_offset; // Instruction offset

	R_SEMAPHORE sem;
	SSTACK stack;
};

R_SCRIPT_THREAD *STHREAD_Create();
int STHREAD_Destroy(R_SCRIPT_THREAD *thread);
int STHREAD_SetEntrypoint(R_SCRIPT_THREAD *thread, int ep_num);
int STHREAD_Execute(R_SCRIPT_THREAD *thread, int ep_num);
int STHREAD_Run(R_SCRIPT_THREAD *thread, int instr_limit, int msec);
unsigned long GetReadOffset(const byte *read_p);
unsigned char *GetReadPtr(R_SCRIPT_THREAD *thread);
size_t GetReadLen(R_SCRIPT_THREAD *thread);
int SDEBUG_PrintInstr(R_SCRIPT_THREAD *thread);

}				// End of namespace Saga

#endif
