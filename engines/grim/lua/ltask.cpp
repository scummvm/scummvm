
#include "ltask.h"
#include "lauxlib.h"
#include "lmem.h"
#include "ldo.h"
#include "lvm.h"

namespace Grim {

void pause_scripts() {
	lua_Task *t;

	for (t = lua_state->root_task->next; t != NULL; t = t->next) {
		if (lua_state->curr_task != t && t->Tstate != DONE)
			t->Tstate = PAUSE;
	}
}

void unpause_scripts() {
	lua_Task *t;

	for (t = lua_state->root_task->next; t != NULL; t = t->next) {
		if (lua_state->curr_task != t && t->Tstate != DONE)
			t->Tstate = YIELD;
	}
}

void start_script() {
	struct lua_Task *old_task = lua_state->curr_task, *new_task;
	TObject *f = lua_state->stack.stack + lua_state->Cstack.lua2C;

	f = lua_state->stack.stack + lua_state->Cstack.lua2C;
	if (ttype(f) == LUA_T_CLOSURE)
		f = &clvalue(f)->consts[0];
	// Start nothing? start_script gets called in this fashion
	// by the scene in the scrimshaw parlor, if we just return
	// immediately the game proceeds ok.
	if (ttype(f) == LUA_T_NIL)
		return;
	if (ttype(f) != LUA_T_PROTO)
		lua_error("can only start_script with a Lua function");

	// Create a new task with an empty stack
	new_task = luaI_newtask();

	// Put the function and arguments onto the new task's stack
	for (int32 i = 0; i < old_task->Cstack.num; i++) {
		*(lua_state->stack.top) = *(old_task->stack.stack + old_task->Cstack.lua2C + i);
		incr_top;
	}

	// Create a CallInfo frame
	luaD_precall(lua_state->stack.stack, 1, MULT_RET);
	ttype(lua_state->stack.stack) = (ttype(lua_state->stack.stack) == LUA_T_CLOSURE) ? LUA_T_CLMARK : LUA_T_PMARK;

	// Switch back to the old task
	lua_state->Tstate = YIELD;
	luaI_switchtask(old_task);
	lua_state->curr_task = old_task;

	// Insert new task at end of list
	lua_state->last_task->next = new_task;
	lua_state->last_task = new_task;

	// Return task handle
	ttype(lua_state->stack.top) = LUA_T_TASK;
	nvalue(lua_state->stack.top) = (float)new_task->id;
	incr_top;
}

void stop_script() {
	lua_Task *prev, *t;
	TObject *f = lua_state->stack.stack + lua_state->Cstack.lua2C;
	int32 match = 0;

	if (f == LUA_NOOBJECT || (ttype(f) != LUA_T_CLOSURE && ttype(f) != LUA_T_PROTO && ttype(f) != LUA_T_TASK))
		lua_error("Bad argument to stop_script");

	prev = lua_state->root_task;
	while ((t = prev->next)) {
		switch (ttype(f)) {
		case LUA_T_CLOSURE:
			match = (ttype(t->stack.stack) == LUA_T_CLMARK && clvalue(t->stack.stack) == clvalue(f));
			break;
		case LUA_T_PROTO:
			match = (ttype(t->stack.stack) == LUA_T_PMARK && tfvalue(t->stack.stack) == tfvalue(f));
			break;
		case LUA_T_TASK:
			match = (t->id == (int32)nvalue(f));
			break;
		default:  // Shut up gcc
			break;
		}

		if (match) {
			prev->next = t->next;  // Remove from list of active tasks
			t->next = NULL;
			if (!prev->next) {
				lua_state->last_task = prev;
				if (t == lua_state->curr_task) {
					lua_state->Tstate = DONE;
				}
			} else {
				t->Tstate = DONE;
			}
		} else {
			prev = t;
		}
	}
}

void next_script() {
	lua_Task *t = NULL, *prev;
	TObject *f = lua_state->stack.stack + lua_state->Cstack.lua2C;

	if (f == LUA_NOOBJECT)
		lua_error("Bad argument to next_script: no obeject");

	prev = lua_state->root_task;
	if (ttype(f) == LUA_T_NIL) {
		t = lua_state->root_task;
	} else if (ttype(f) == LUA_T_TASK) {
		int32 taskId = (int32)nvalue(f);
		for (t = lua_state->root_task->next; t != NULL; t = t->next) {
			if (t->id == taskId)
				break;
		}
	} else {
		lua_error("Bad argument to next_script.");
	}
	if (!t) {
		lua_pushnil();
	} else {
		t = t->next;
		if (!t) {
			lua_pushnil();
		} else {
			ttype(lua_state->stack.top) = LUA_T_TASK;
			nvalue(lua_state->stack.top) = (float)t->id;
			incr_top;
		}
	}
}

void identify_script() {
	lua_Task *t;
	TObject *f = lua_state->stack.stack + lua_state->Cstack.lua2C;

	if (f == LUA_NOOBJECT || ttype(f) != LUA_T_TASK) {
		lua_error("Bad argument to identify_script");
	}

	int32 taskId = (int32)nvalue(f);
	for (t = lua_state->root_task->next; t != NULL; t = t->next) {
		if (t->id == taskId)
			break;
	}

	if (!t || t->Tstate == DONE) {
		ttype(lua_state->stack.top) = LUA_T_NIL;
	} else {
		*lua_state->stack.top = *t->stack.stack;
	}
	incr_top;
}

void find_script() {
	lua_Task *t = NULL, *foundTask = NULL;
	TObject *f = lua_state->stack.stack + lua_state->Cstack.lua2C;
	int32 countTasks = 0, taskId;

	switch (ttype(f)) {
	case LUA_T_CLOSURE:
		for (t = lua_state->root_task->next; t != NULL; t = t->next) {
			if ((ttype(t->stack.stack) == LUA_T_CLOSURE || ttype(t->stack.stack) == LUA_T_CMARK) && clvalue(t->stack.stack) == clvalue(f)) {
				foundTask = t;
				countTasks++;
			}
		}
		t = foundTask;
		break;
	case LUA_T_PROTO:
		for (t = lua_state->root_task->next; t != NULL; t = t->next) {
			if ((ttype(t->stack.stack) == LUA_T_PROTO || ttype(t->stack.stack) == LUA_T_PMARK) && tfvalue(t->stack.stack) == tfvalue(f)) {
				foundTask = t;
				countTasks++;
			}
		}
		t = foundTask;
		break;
	case LUA_T_TASK:
		taskId = (int32)nvalue(f);
		for (t = lua_state->root_task->next; t != NULL; t = t->next) {
			if ((t->id == taskId) && (t->Tstate != DONE)) {
				ttype(lua_state->stack.top) = LUA_T_TASK;
				nvalue(lua_state->stack.top) = nvalue(f);
				incr_top;
				lua_pushnumber(1.0f);
				return;
			}
		}
		break;
	default:
		lua_error("Bad argument to find_script");
	}

	if (t) {
		ttype(lua_state->stack.top) = LUA_T_TASK;
		nvalue(lua_state->stack.top) = (float)t->id;
		incr_top;
		lua_pushnumber(countTasks);
	} else {
		lua_pushnil();
		lua_pushnumber(0.0f);
	}
}

void break_here() {
	CallInfo *ci;

	if (lua_state->curr_task == lua_state->root_task) {
		lua_error("Cannot break in root thread");
	}
	// Check for any C functions in the call stack
	for (ci = lua_state->ci-1; ci > lua_state->base_ci; ci--)
		if (ci->tf == NULL)
			lua_error("Cannot yield through C function");

	lua_state->Tstate = YIELD;
}

void current_script() {
	if (lua_state->curr_task == lua_state->root_task) {
		lua_pushnil();
	} else {
		ttype(lua_state->stack.top) = LUA_T_TASK;
		nvalue(lua_state->stack.top) = (float)lua_state->curr_task->id;
		incr_top;
	}
}

void lua_runtasks() {
	lua_Task *t, *prev;
	lua_Task *old_task = lua_state->curr_task;
	jmp_buf myErrorJmp;

	prev = lua_state->root_task;
	while ((t = prev->next)) {
		luaI_switchtask(t);
		// Tstate is not available until after switching tasks
		if (t->Tstate == PAUSE) {
			prev = t;
			continue;
		}
		lua_state->errorJmp = &myErrorJmp;
		lua_state->Tstate = RUN;
		if (setjmp(myErrorJmp) == 0) {
			luaV_execute(lua_state->base_ci + 1);
			if (lua_state->Tstate == RUN) { // Must have run to completion
				lua_state->Tstate = DONE;
			}
		} else { // an error occurred
			lua_state->Tstate = DONE;
		}
		lua_state->errorJmp = NULL;
		prev = t;
	}
	// Free the completed tasks
	// This MUST occur after all the tasks have been run (not during)
	// or else when one task is freed right after another the task
	// execution gets hosed.  Test Case: Switching between tw.set and
	// tb.set in Rubacava causes a crash without this.
	prev = lua_state->root_task;
	while ((t = prev->next)) {
		luaI_switchtask(t);
		if (lua_state->Tstate == DONE) { // Remove from list of active tasks
			luaI_switchtask(old_task);
			prev->next = t->next;
			t->next = NULL;
			if (!prev->next) {
				lua_state->last_task = prev;
			}
			luaM_free(t);
		} else {
			prev = t;
		}
	}
	if (lua_state->curr_task != old_task) {
		luaI_switchtask(old_task);
	}
}

} // end of namespace Grim
