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
 
    Scripting engine stack component header file

 Notes: 
*/

#ifndef SAGA_SSTACK_H
#define SAGA_SSTACK_H

#include "script_mod.h"

namespace Saga {

#define R_STACK_SIZE_LIMIT 16384

struct SSTACK_tag {
	int flags;
	int len;
	int top;
	SDataWord_T *data;
};

typedef struct SSTACK_tag *SSTACK;

typedef enum SSTACK_ERR_enum {
	STACK_SUCCESS = 0,
	STACK_ERROR,
	STACK_MEM,
	STACK_UNDERFLOW,
	STACK_OVERFLOW
} SSTACK_ERR_CODE;

typedef enum SSTACK_FLAGS_enum {
	STACK_FIXED = 0x00,
	STACK_GROW = 0x01
} SSTACK_FLAGS;

int SSTACK_Create(SSTACK * stack, int stack_len, int flags);

int SSTACK_Destroy(SSTACK stack);

int SSTACK_Clear(SSTACK stack);

int SSTACK_Push(SSTACK stack, SDataWord_T value);

int SSTACK_PushNull(SSTACK stack);

int SSTACK_Pop(SSTACK stack, SDataWord_T * value);

int SSTACK_Top(SSTACK stack, SDataWord_T * value);

int SSTACK_Grow(SSTACK stack);

} // End of namespace Saga

#endif				/* SAGA_SSTACK_H */
