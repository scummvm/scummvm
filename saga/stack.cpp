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
 
	Simple integer stack module

 Notes: 
*/

#include "reinherit.h"

#include "stack.h"
#include "stack_mod.h"

namespace Saga {

int ISTACK_Create(R_ISTACK * stack, int stack_len, int flags)
{

	R_ISTACK new_stack;
	int *new_stack_data;

	*stack = NULL;

	new_stack = (R_ISTACK_tag *)malloc(sizeof(struct R_ISTACK_tag));

	if (new_stack == NULL) {
		return STACK_MEM;
	}

	new_stack_data = (int *)calloc(stack_len, sizeof(int));

	if (new_stack_data == NULL) {

		free(new_stack);
		return STACK_MEM;
	}

	new_stack->flags = flags;
	new_stack->len = stack_len;
	new_stack->top = -1;

	return STACK_SUCCESS;
}

int ISTACK_Destroy(R_ISTACK * stack)
{

	if (stack != NULL) {
		free((*stack)->data);
	}

	free(stack);

	*stack = NULL;

	return STACK_SUCCESS;
}

int ISTACK_Clear(R_ISTACK stack)
{

	stack->top = -1;
	return STACK_SUCCESS;
}

int ISTACK_PushNull(R_ISTACK stack)
{

	if (stack->top >= (stack->len - 1)) {

		if (stack->flags & STACK_FIXED) {

			return STACK_OVERFLOW;
		} else if (ISTACK_Grow(stack) != STACK_SUCCESS) {

			return STACK_MEM;
		}
	}

	stack->top++;

}

int ISTACK_Push(R_ISTACK stack, int value)
{

	if (stack->top >= (stack->len - 1)) {

		if (stack->flags & STACK_FIXED) {

			return STACK_OVERFLOW;
		} else if (ISTACK_Grow(stack) != STACK_SUCCESS) {

			return STACK_MEM;
		}
	}

	stack->top++;
	stack->data[stack->top] = value;

	return STACK_SUCCESS;
}

int ISTACK_Pop(R_ISTACK stack, int *value)
{

	if (stack->top <= -1) {

		return STACK_UNDERFLOW;
	}

	if (value == NULL) {

		stack->top--;
		return STACK_SUCCESS;
	}

	*value = stack->data[stack->top];
	stack->top--;

	return STACK_SUCCESS;
}

int ISTACK_Top(R_ISTACK stack, int *value)
{
	*value = 0;

	if (stack->top <= -1) {

		return STACK_UNDERFLOW;
	}

	*value = stack->data[stack->top];

	return STACK_SUCCESS;

}

int ISTACK_Grow(R_ISTACK stack)
{

	int *new_data;

	new_data = (int *)realloc(stack->data, (stack->len * 2) * sizeof(int));

	if (new_data == NULL) {

		return STACK_MEM;
	};

	stack->data = new_data;
	stack->len *= 2;

	return STACK_SUCCESS;
}

} // End of namespace Saga

