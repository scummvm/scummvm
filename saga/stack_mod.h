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
 
	Simple integer stack module public header

 Notes: 
*/

#ifndef SAGA_STACK_MOD_H
#define SAGA_STACK_MOD_H

#include "stack.h"

namespace Saga {

typedef enum STACK_ERR_enum {
	STACK_SUCCESS = 0,
	STACK_ERROR,
	STACK_MEM,
	STACK_UNDERFLOW,
	STACK_OVERFLOW
} STACK_ERR_CODE;

typedef enum STACK_FLAGS_enum {
	STACK_FIXED = 0x00,
	STACK_GROW = 0x01
} STACK_FLAGS;

typedef struct R_ISTACK_tag *R_ISTACK;

int ISTACK_Create(R_ISTACK * stack, int stack_len, int flags);

int ISTACK_Destroy(R_ISTACK * stack);

int ISTACK_Clear(R_ISTACK stack);

int ISTACK_Push(R_ISTACK stack, int value);

int ISTACK_PushNull(R_ISTACK stack);

int ISTACK_Pop(R_ISTACK stack, int *value);

int ISTACK_Grow(R_ISTACK stack);

} // End of namespace Saga

#endif				/* SAGA_STACK_MOD_H */
