#include <stdlib.h>
#include <stdio.h>

#include "ltask.h"
#include "lauxlib.h"
#include "lmem.h"
#include "ldo.h"
#include "lvm.h"

void pause_scripts (void) {
	struct lua_Task *t;

	for (t = L->root_task->next; t != NULL; t = t->next) {
		if (L->curr_task != t)
			t->Tstate = PAUSE;
	}
}

void unpause_scripts (void) {
	struct lua_Task *t;

	for (t = L->root_task->next; t != NULL; t = t->next) {
		if (L->curr_task != t)
			t->Tstate = YIELD;
	}
}

void start_script (void) {
	struct lua_Task *old_task = L->curr_task, *new_task;
	TObject *f = L->stack.stack + L->Cstack.lua2C;

	f = L->stack.stack + L->Cstack.lua2C;
	if (ttype(f) == LUA_T_CLOSURE)
		f = &clvalue(f)->consts[0];
	// Start nothing? start_script gets called in this fashion
	// by the scene in the scrimshaw parlor, if we just return
	// immediately the game proceeds ok.
	if (ttype(f) == LUA_T_NIL)
		return;
	if (ttype(f) != LUA_T_PROTO)
		lua_error("can only start_script with a Lua function");

	/* Create a new task with an empty stack */
	new_task = luaI_newtask();

	/* Put the function and arguments onto the new task's stack */
	for (int i = 0; i < old_task->Cstack.num; i++) {
		*(L->stack.top) = *(old_task->stack.stack + old_task->Cstack.lua2C + i);
		incr_top;
	}

	/* Create a CallInfo frame */
	luaD_precall(L->stack.stack, 1, MULT_RET);
	ttype(L->stack.stack) = (ttype(L->stack.stack) == LUA_T_CLOSURE) ? LUA_T_CLMARK : LUA_T_PMARK;

	/* Switch back to the old task */
	L->Tstate = YIELD;
	luaI_switchtask(old_task);
	L->curr_task = old_task;

	/* Insert new task at end of list */
	L->last_task->next = new_task;
	L->last_task = new_task;

	/* Return task handle */
	ttype(L->stack.top) = LUA_T_TASK;
	nvalue(L->stack.top) = (real)new_task->id;
	incr_top;
}

void stop_script (void) {
	struct lua_Task *prev, *t;
	TObject *f = L->stack.stack + L->Cstack.lua2C;
	int match;

	if ((f == LUA_NOOBJECT) || (ttype(f) != LUA_T_CLOSURE && ttype(f) != LUA_T_PROTO && ttype(f) != LUA_T_TASK))
		lua_error("Bad argument to stop_script");

	prev = L->root_task;
	while ((t = prev->next) != NULL) {
		switch (ttype(f)) {
		case LUA_T_CLOSURE:
			match = (ttype(t->stack.stack) == LUA_T_CLMARK && clvalue(t->stack.stack) == clvalue(f));
			break;
		case LUA_T_PROTO:
			match = (ttype(t->stack.stack) == LUA_T_PMARK && tfvalue(t->stack.stack) == tfvalue(f));
			break;
		case LUA_T_TASK:
			match = (t->id == (int)nvalue(f));
			break;
		default:  /* Shut up gcc */
			break;
		}

		if (match) {
			prev->next = t->next;  /* Remove from list of active tasks */
			t->next = NULL;
			if (prev->next == NULL) {
				L->last_task = prev;
				if (t == L->curr_task) {
					L->Tstate = DONE;
				}
			} else {
				t->Tstate = DONE;
			}
		} else {
			prev = t;
		}
	}
}

void next_script (void) {
	struct lua_Task *t, *prev;
	TObject *f = L->stack.stack + L->Cstack.lua2C;

	if (f == LUA_NOOBJECT)
		lua_error("Bad argument to next_script: no obeject");

	prev = L->root_task;
	if (ttype(f) == LUA_T_NIL) {
		t = L->root_task;
	} else if (ttype(f) == LUA_T_TASK) {
		int taskId = (int)nvalue(f);
		for (t = L->root_task->next; t != NULL; t = t->next) {
			if (t->id == taskId)
				break;
		}
	} else {
		lua_error("Bad argument to next_script.");
	}
	if (t == NULL) {
		lua_pushnil();
	} else {
		t = t->next;
		if (t == NULL) {
			lua_pushnil();
		} else {
			ttype(L->stack.top) = LUA_T_TASK;
			nvalue(L->stack.top) = (real)t->id;
			incr_top;
		}
	}
}

void identify_script (void) {
	struct lua_Task *t;
	TObject *f = L->stack.stack + L->Cstack.lua2C;

	if ((f == LUA_NOOBJECT) || ttype(f) != LUA_T_TASK) {
		lua_error("Bad argument to identify_script");
	}

	int taskId = (int)nvalue(f);
	for (t = L->root_task->next; t != NULL; t = t->next) {
		if (t->id == taskId)
			break;
	}

	if ((t == NULL) || (t->Tstate == DONE)) {
			ttype(L->stack.top) = LUA_T_NIL;
	} else {
		*L->stack.top = *t->stack.stack;
	}
	incr_top;
}

void find_script (void) {
	struct lua_Task *t, *foundTask = NULL;
	TObject *f = L->stack.stack + L->Cstack.lua2C;
	int countTasks = 0, taskId;

	switch (ttype(f)) {
	case LUA_T_CLOSURE:
		for (t = L->root_task->next; t != NULL; t = t->next) {
			if ((ttype(t->stack.stack) == LUA_T_CLOSURE || ttype(t->stack.stack) == LUA_T_CMARK) && clvalue(t->stack.stack) == clvalue(f)) {
				foundTask = t;
				countTasks++;
			}
		}
		t = foundTask;
		break;
	case LUA_T_PROTO:
		for (t = L->root_task->next; t != NULL; t = t->next) {
			if ((ttype(t->stack.stack) == LUA_T_PROTO || ttype(t->stack.stack) == LUA_T_PMARK) && tfvalue(t->stack.stack) == tfvalue(f)) {
				foundTask = t;
				countTasks++;
			}
		}
		t = foundTask;
		break;
	case LUA_T_TASK:
		taskId = (int)nvalue(f);
		for (t = L->root_task->next; t != NULL; t = t->next) {
			if ((t->id == taskId) && (t->Tstate != DONE)) {
				ttype(L->stack.top) = LUA_T_TASK;
				nvalue(L->stack.top) = nvalue(f);
				incr_top;
				lua_pushnumber(1.0f);
				return;
			}
		}
		break;
	default:
		lua_error("Bad argument to find_script");
	}

	if (t != NULL) {
		ttype(L->stack.top) = LUA_T_TASK;
		nvalue(L->stack.top) = (real)t->id;
		incr_top;
		lua_pushnumber(countTasks);
	} else {
		lua_pushnil();
		lua_pushnumber(0.0f);
	}
}

void break_here (void) {
	struct CallInfo *ci;

	if (L->curr_task == L->root_task) {
		lua_error("Cannot break in root thread");
	}
	/* Check for any C functions in the call stack */
	for (ci = L->ci-1; ci > L->base_ci; ci--)
		if (ci->tf == NULL)
			lua_error("Cannot yield through C function");

	L->Tstate = YIELD;
}

void current_script (void) {
	if (L->curr_task == L->root_task) {
		lua_pushnil();
	} else {
		ttype(L->stack.top) = LUA_T_TASK;
		nvalue(L->stack.top) = (real)L->curr_task->id;
		incr_top;
	}
}

void lua_runtasks (void) {
	struct lua_Task *t, *prev;
	struct lua_Task *old_task = L->curr_task;
	jmp_buf myErrorJmp;

	prev = L->root_task;
	while ((t = prev->next) != NULL) {
		luaI_switchtask(t);
		// Tstate is not available until after switching tasks
		if (t->Tstate == PAUSE)
			continue;
		L->errorJmp = &myErrorJmp;
		L->Tstate = RUN;
		if (setjmp(myErrorJmp) == 0) {
			luaV_execute(L->base_ci + 1);
			if (L->Tstate == RUN) { /* Must have run to completion */
				L->Tstate = DONE;
			}
		} else { /* an error occurred */
			L->Tstate = DONE;
		}
		L->errorJmp = NULL;
		prev = t;
	}
	// Free the completed tasks
	// This MUST occur after all the tasks have been run (not during)
	// or else when one task is freed right after another the task
	// execution gets hosed.  Test Case: Switching between tw.set and
	// tb.set in Rubacava causes a crash without this.
	prev = L->root_task;
	while ((t = prev->next) != NULL) {
		luaI_switchtask(t);
		if (L->Tstate == DONE) { // Remove from list of active tasks
			luaI_switchtask(old_task);
			prev->next = t->next;
			t->next = NULL;
			if (prev->next == NULL) {
				L->last_task = prev;
			}
			luaM_free(t);
		} else {
			prev = t;
		}
	}
	if (L->curr_task != old_task) {
		luaI_switchtask(old_task);
	}
}
