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

// Scripting engine stack component

#include "saga.h"

#include "gfx_mod.h"
#include "console_mod.h"
#include "text_mod.h"

#include "script.h"
#include "sstack.h"

namespace Saga {

int SSTACK_Create(SSTACK *stack, int stack_len, int flags) {
	SSTACK new_stack;
	SDataWord_T *new_stack_data;

	*stack = NULL;

	new_stack = (SSTACK_tag *)malloc(sizeof(struct SSTACK_tag));
	if (new_stack == NULL) {
		return STACK_MEM;
	}

	new_stack_data = (SDataWord_T *)calloc(stack_len, sizeof *new_stack_data);
	if (new_stack_data == NULL) {
		free(new_stack);
		return STACK_MEM;
	}

	new_stack->data = new_stack_data;
	new_stack->flags = flags;
	new_stack->len = stack_len;
	new_stack->top = -1;
	*stack = new_stack;

	return STACK_SUCCESS;
}

int SSTACK_Destroy(SSTACK stack) {
	if (stack != NULL) {
		free(stack->data);
	}

	free(stack);

	return STACK_SUCCESS;
}

int SSTACK_Clear(SSTACK stack) {
	stack->top = -1;

	return STACK_SUCCESS;
}

int SSTACK_PushNull(SSTACK stack) {
	if (stack->top >= (stack->len - 1)) {
		if (stack->flags & STACK_FIXED) {
			return STACK_OVERFLOW;
		} else if (SSTACK_Grow(stack) != STACK_SUCCESS) {
			return STACK_MEM;
		}
	}

	stack->top++;

	return STACK_SUCCESS;
}

int SSTACK_Push(SSTACK stack, SDataWord_T value) {
	if (stack->top >= (stack->len - 1)) {
		if (stack->flags & STACK_FIXED) {
			return STACK_OVERFLOW;
		} else if (SSTACK_Grow(stack) != STACK_SUCCESS) {
			return STACK_MEM;
		}
	}

	stack->top++;
	stack->data[stack->top] = value;

	return STACK_SUCCESS;
}

int SSTACK_Pop(SSTACK stack, SDataWord_T *value) {
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

int SSTACK_Top(SSTACK stack, SDataWord_T *value) {
	*value = 0;

	if (stack->top <= -1) {
		return STACK_UNDERFLOW;
	}

	*value = stack->data[stack->top];

	return STACK_SUCCESS;
}

int SSTACK_Grow(SSTACK stack) {
	SDataWord_T *new_data;

	if ((stack->len * 2) > R_STACK_SIZE_LIMIT) {
		CON_Print(S_ERROR_PREFIX "Stack fault: growing beyond limit.");
		return STACK_OVERFLOW;
	}

	new_data = (SDataWord_T *)realloc(stack->data, (stack->len * 2) * sizeof *new_data);
	if (new_data == NULL) {
		return STACK_MEM;
	}

	stack->data = new_data;
	stack->len *= 2;

	return STACK_SUCCESS;
}

} // End of namespace Saga

