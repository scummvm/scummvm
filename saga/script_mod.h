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
 
    Scripting module public header

 Notes: 
*/

#ifndef SAGA_SCRIPT_MOD_H
#define SAGA_SCRIPT_MOD_H

namespace Saga {

typedef unsigned int SDataWord_T;

typedef struct R_SCRIPT_THREAD_tag R_SCRIPT_THREAD;

enum R_SCRIPT_VERBS {

	S_VERB_WALKTO = 0,
	S_VERB_LOOKAT = 2,
	S_VERB_PICKUP = 1,
	S_VERB_TALKTO,
	S_VERB_OPEN = 5,
	S_VERB_CLOSE = 6,
	S_VERB_USE = 8,
	S_VERB_GIVE
};

int SCRIPT_Register(void);

int SCRIPT_Init(void);

int SCRIPT_Shutdown(void);

int SCRIPT_Load(int script_num);

int SCRIPT_Free(void);

int SDATA_GetWord(int n_buf, int n_word, SDataWord_T * data);

int SDATA_PutWord(int n_buf, int n_word, SDataWord_T data);

int SDATA_SetBit(int n_buf, SDataWord_T n_bit, int bitstate);

int SDATA_GetBit(int n_buf, SDataWord_T n_bit, int *bitstate);

int SDATA_ReadWordS(SDataWord_T word);

uint16 SDATA_ReadWordU(SDataWord_T word);

R_SCRIPT_THREAD *STHREAD_Create(void);

int STHREAD_Execute(R_SCRIPT_THREAD * thread, int ep_num);

int STHREAD_ExecThreads(int msec);

int STHREAD_HoldSem(R_SEMAPHORE * sem);

int STHREAD_ReleaseSem(R_SEMAPHORE * sem);

int STHREAD_DebugStep(void);

} // End of namespace Saga

#endif				/* SAGA_SCRIPT_MOD_H */
