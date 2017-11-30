#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp

#ifdef _MSC_VER
#pragma warning(disable:4611)
#endif

#include "engines/grim/lua/ltask.h"
#include "engines/grim/lua/lapi.h"
#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/ldo.h"
#include "engines/grim/lua/lvm.h"
#include "engines/grim/grim.h"

#include "common/textconsole.h"

namespace Grim {

void lua_taskinit(lua_Task *task, lua_Task *next, StkId tbase, int results) {
	task->some_flag = 0;
	task->next = next;
	task->some_base = tbase;
	task->some_results = results;
}

void lua_taskresume(lua_Task *task, Closure *closure, TProtoFunc *protofunc, StkId tbase) {
	task->cl = closure;
	task->tf = protofunc;
	task->base = tbase;
	task->pc = task->tf->code;
	task->consts = task->tf->consts;
	task->S = &lua_state->stack;
}

void start_script() {
	lua_Object paramObj = lua_getparam(1);
	lua_Type type = paramObj == LUA_NOOBJECT ? LUA_T_NIL : ttype(Address(paramObj));

	if (paramObj == LUA_NOOBJECT || (type != LUA_T_CPROTO && type != LUA_T_PROTO)) {
		if (g_grim->getGameType() == GType_MONKEY4) {
			/* In the discussion with meathook (dlg_meathook2.lua),
			 * start_script is called as start_script(meathook:shake_head(...)).
			 * But start_script expects start_script(meathook.shake_head, shake_head, ...). */
			warning("Bad argument to start_script, ignoring");
			lua_pushnil();
			return;
		} else {
			lua_error("Bad argument to start_script");
			return;
		}
	}

	LState *state = luaM_new(LState);
	lua_stateinit(state);

	state->next = lua_state->next;
	state->prev = lua_state;
	if (state->next)
		state->next->prev = state;
	lua_state->next = state;

	state->taskFunc.ttype = type;
	state->taskFunc.value = Address(paramObj)->value;

	int l = 2;
	for (lua_Object object = lua_getparam(l++); object != LUA_NOOBJECT; object = lua_getparam(l++)) {
		TObject ptr;
		ptr.ttype = Address(object)->ttype;
		ptr.value = Address(object)->value;
		LState *tmpState = lua_state;
		lua_state = state;
		luaA_pushobject(&ptr);
		lua_state = tmpState;
	}

	ttype(lua_state->stack.top) = LUA_T_TASK;
	nvalue(lua_state->stack.top) = (float)state->id;
	incr_top;
}

void next_script() {
	lua_Object paramObj = lua_getparam(1);
	lua_Type type = paramObj == LUA_NOOBJECT ? LUA_T_NIL : ttype(Address(paramObj));

	if (paramObj == LUA_NOOBJECT || (type != LUA_T_TASK && type != LUA_T_NIL))
		lua_error("Bad argument to next_script");

	if (type == LUA_T_TASK) {
		uint32 task = (uint32)nvalue(Address(paramObj));
		LState *state;
		for (state = lua_rootState->next; state != nullptr; state = state->next) {
			if (state->id == task) {
				if (state->next) {
					ttype(lua_state->stack.top) = LUA_T_TASK;
					nvalue(lua_state->stack.top) = (float)state->next->id;
					incr_top;
				} else
					lua_pushnil();
				return;
			}
		}
	}

	if (lua_rootState->next) {
		ttype(lua_state->stack.top) = LUA_T_TASK;
		nvalue(lua_state->stack.top) = (float)lua_rootState->next->id;
		incr_top;
	} else
		lua_pushnil();
}

void stop_script() {
	lua_Object paramObj = lua_getparam(1);
	lua_Type type = paramObj == LUA_NOOBJECT ? LUA_T_NIL : ttype(Address(paramObj));
	LState *state;

	if (paramObj == LUA_NOOBJECT || (type != LUA_T_CPROTO && type != LUA_T_PROTO && type != LUA_T_TASK))
		lua_error("Bad argument to stop_script");

	if (type == LUA_T_TASK) {
		uint32 task = (uint32)nvalue(Address(paramObj));
		for (state = lua_rootState->next; state != nullptr; state = state->next) {
			if (state->id == task)
				break;
		}
		if (state) {
			if (state != lua_state) {
				lua_statedeinit(state);
				luaM_free(state);
			}
		}
	} else if (type == LUA_T_PROTO || type == LUA_T_CPROTO) {
		for (state = lua_rootState->next; state != nullptr;) {
			bool match;
			if (type == LUA_T_PROTO) {
				match = (state->taskFunc.ttype == type && tfvalue(&state->taskFunc) == tfvalue(Address(paramObj)));
			} else {
				match = (state->taskFunc.ttype == type && fvalue(&state->taskFunc) == fvalue(Address(paramObj)));
			}
			if (match && state != lua_state) {
				LState *tmp = state->next;
				lua_statedeinit(state);
				luaM_free(state);
				state = tmp;
			} else {
				state = state->next;
			}
		}
	}
}

void identify_script() {
	lua_Object paramObj = lua_getparam(1);
	lua_Type type = paramObj == LUA_NOOBJECT ? LUA_T_NIL : ttype(Address(paramObj));

	if (paramObj == LUA_NOOBJECT || type != LUA_T_TASK)
		lua_error("Bad argument to identify_script");

	uint32 task = (uint32)nvalue(Address(paramObj));
	LState *state;
	for (state = lua_rootState->next; state != nullptr; state = state->next) {
		if (state->id == task) {
			luaA_pushobject(&state->taskFunc);
			return;
		}
	}

	lua_pushnil();
}

void find_script() {
	lua_Object paramObj = lua_getparam(1);
	lua_Type type = paramObj == LUA_NOOBJECT ? LUA_T_NIL : ttype(Address(paramObj));

	if (paramObj == LUA_NOOBJECT || (type != LUA_T_CPROTO && type != LUA_T_PROTO && type != LUA_T_TASK)) {
		if (g_grim->getGameType() == GType_GRIM) {
			lua_error("Bad argument to find_script");
		} else {
			ttype(lua_state->stack.top) = LUA_T_TASK;
			nvalue(lua_state->stack.top) = lua_state->id;
			incr_top;
			lua_pushnumber(1.0f);
			return;
		}
	}

	if (type == LUA_T_TASK) {
		uint32 task = (uint32)nvalue(Address(paramObj));
		LState *state;
		for (state = lua_rootState->next; state != nullptr; state = state->next) {
			if (state->id == task) {
				lua_pushobject(paramObj);
				lua_pushnumber(1.0f);
				return;
			}
		}
	} else if (type == LUA_T_PROTO || type == LUA_T_CPROTO) {
		int task = -1, countTasks = 0;
		bool match;
		LState *state;
		for (state = lua_rootState->next; state != nullptr; state = state->next) {
			if (type == LUA_T_PROTO) {
				match = (state->taskFunc.ttype == type && tfvalue(&state->taskFunc) == tfvalue(Address(paramObj)));
			} else {
				match = (state->taskFunc.ttype == type && fvalue(&state->taskFunc) == fvalue(Address(paramObj)));
			}
			if (match) {
				task = state->id;
				countTasks++;
			}
		}
		if (countTasks) {
			assert(task != -1);
			ttype(lua_state->stack.top) = LUA_T_TASK;
			nvalue(lua_state->stack.top) = (float)task;
			incr_top;
			lua_pushnumber((float)countTasks);
			return;
		}
	}

	lua_pushnil();
	lua_pushnumber(0.0f);
}

void pause_script() {
	lua_Object taskObj = lua_getparam(1);
	lua_Type type = taskObj == LUA_NOOBJECT ? LUA_T_NIL : ttype(Address(taskObj));
	if (type != LUA_T_TASK) {
		lua_error("Bad argument to pause_script");
		return;
	}

	uint32 task = (uint32)nvalue(Address(taskObj));
	LState *state;
	for (state = lua_rootState->next; state != nullptr; state = state->next) {
		if (state->id == task) {
			state->paused = true;
			return;
		}
	}
}

void pause_scripts() {
	LState *t;
	lua_Object boolObj = lua_getparam(1);

	bool p = false;
	if (!lua_isnil(boolObj))
		p = true;


	for (t = lua_rootState->next; t != nullptr; t = t->next) {
		if (lua_state != t) {
			if (p) {
				t->all_paused++;
			} else {
				t->all_paused = 1;
			}
		}

	}
}

void unpause_script() {
	lua_Object taskObj = lua_getparam(1);
	lua_Type type = taskObj == LUA_NOOBJECT ? LUA_T_NIL : ttype(Address(taskObj));
	if (type != LUA_T_TASK) {
		lua_error("Bad argument to unpause_script");
		return;
	}

	uint32 task = (uint32)nvalue(Address(taskObj));
	LState *state;
	for (state = lua_rootState->next; state != nullptr; state = state->next) {
		if (state->id == task) {
			state->paused = false;
			return;
		}
	}
}

void unpause_scripts() {
	LState *t;
	lua_Object boolObj = lua_getparam(1);

	bool p = false;
	if (!lua_isnil(boolObj))
		p = true;

	for (t = lua_rootState->next; t != nullptr; t = t->next) {
		if (lua_state != t) {
			if (p) {
				if (t->all_paused > 0)
					t->all_paused--;
			} else {
				t->all_paused = 0;
			}
		}
	}
}

void current_script() {
	ttype(lua_state->stack.top) = LUA_T_TASK;
	nvalue(lua_state->stack.top) = (float)lua_state->id;
	incr_top;
}

void break_here() {}

void sleep_for() {
	lua_Object msObj = lua_getparam(1);

	if (lua_isnumber(msObj)) {
		int ms = (int)lua_getnumber(msObj);
		lua_state->sleepFor = ms;
	}
}

void lua_runtasks() {
	if (!lua_state || !lua_state->next) {
		return;
	}

	// Mark all the states to be updated
	LState *state = lua_state->next;
	do {
		if (state->sleepFor > 0) {
			state->sleepFor -= g_grim->getFrameTime();
		} else {
			state->updated = false;
		}
		state = state->next;
	} while	(state);

	// And run them
	runtasks(lua_state);
}

void runtasks(LState *const rootState) {
	lua_state = lua_state->next;
	while (lua_state) {
		LState *nextState = nullptr;
		bool stillRunning;
		if (!lua_state->all_paused && !lua_state->updated && !lua_state->paused) {
			jmp_buf	errorJmp;
			lua_state->errorJmp = &errorJmp;
			if (setjmp(errorJmp)) {
				lua_Task *t, *m;
				for (t = lua_state->task; t != nullptr;) {
					m = t->next;
					luaM_free(t);
					t = m;
				}
				stillRunning = false;
				lua_state->task = nullptr;
			} else {
				if (lua_state->task) {
					stillRunning = luaD_call(lua_state->task->some_base, lua_state->task->some_results);
				} else {
					StkId base = lua_state->Cstack.base;
					luaD_openstack((lua_state->stack.top - lua_state->stack.stack) - base);
					set_normalized(lua_state->stack.stack + lua_state->Cstack.base, &lua_state->taskFunc);
					stillRunning = luaD_call(base + 1, 255);
				}
			}
			nextState = lua_state->next;
			// The state returned. Delete it
			if (!stillRunning) {
				lua_statedeinit(lua_state);
				luaM_free(lua_state);
			} else {
				lua_state->updated = true;
			}
		} else {
			nextState = lua_state->next;
		}
		lua_state = nextState;
	}

	// Restore the value of lua_state to the main script
	lua_state = rootState;
	// Check for states that may have been created in this run.
	LState *state = lua_state->next;
	while (state) {
		if (!state->all_paused && !state->paused && !state->updated) {
			// New state! Run a new pass.
			runtasks(rootState);
			return;
		}
		state = state->next;
	}
}

} // end of namespace Grim
