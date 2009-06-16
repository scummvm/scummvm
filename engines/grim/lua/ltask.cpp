
#include "engines/grim/lua/ltask.h"
#include "engines/grim/lua/lapi.h"
#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/ldo.h"
#include "engines/grim/lua/lvm.h"

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
	lua_Type type = ttype(Address(paramObj));

	if (paramObj == LUA_NOOBJECT || type != LUA_T_CPROTO && type != LUA_T_PROTO)
		lua_error("Bad argument to start_script");

	LState *state = luaM_new(LState);
	lua_stateinit(state);

	if (type == LUA_T_CPROTO)
		assert(0); // TODO verify if used

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
	lua_Type type = ttype(Address(paramObj));

	if (paramObj == LUA_NOOBJECT || type != LUA_T_TASK && type != LUA_T_NIL)
		lua_error("Bad argument to next_script");

	if (type == LUA_T_TASK) {
		int task = (int)nvalue(Address(paramObj));
		LState *state;
		for (state = lua_rootState->next; state != NULL; state = state->next) {
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
	lua_Type type = ttype(Address(paramObj));
	LState *state;

	if (paramObj == LUA_NOOBJECT || type != LUA_T_CPROTO && type != LUA_T_PROTO && type != LUA_T_TASK)
		lua_error("Bad argument to stop_script");

	if (type == LUA_T_TASK) {
		int task = (int)nvalue(Address(paramObj));
		for (state = lua_rootState->next; state != NULL; state = state->next) {
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
		for (state = lua_rootState->next; state != NULL;) {
			bool match;
			if (type == LUA_T_PROTO) {
				match = (state->taskFunc.ttype == type && tfvalue(&state->taskFunc) == tfvalue(Address(paramObj)));
			} else {
				// TODO - verify if used
				assert(0);
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
	lua_Type type = ttype(Address(paramObj));

	if (paramObj == LUA_NOOBJECT || type != LUA_T_TASK)
		lua_error("Bad argument to identify_script");

	int task = (int)nvalue(Address(paramObj));
	LState *state;
	for (state = lua_rootState->next; state != NULL; state = state->next) {
		if (state->id == task) {
			luaA_pushobject(&state->taskFunc);
			return;
		}
	}

	lua_pushnil();
}

void find_script() {
	lua_Object paramObj = lua_getparam(1);
	lua_Type type = ttype(Address(paramObj));

	if (paramObj == LUA_NOOBJECT || type != LUA_T_CPROTO && type != LUA_T_PROTO && type != LUA_T_TASK)
		lua_error("Bad argument to find_script");

	if (type == LUA_T_TASK) {
		int task = (int)nvalue(Address(paramObj));
		LState *state;
		for (state = lua_rootState->next; state != NULL; state = state->next) {
			if (state->id == task) {
				lua_pushobject(paramObj);
				lua_pushnumber(1.0f);
				return;
			}
		}
	} else if (type == LUA_T_PROTO || type == LUA_T_CPROTO) {
		int task, countTasks = 0;
		bool match;
		LState *state;
		for (state = lua_rootState->next; state != NULL; state = state->next) {
			if (type == LUA_T_PROTO) {
				match = (state->taskFunc.ttype == type && tfvalue(&state->taskFunc) == tfvalue(Address(paramObj)));
			} else {
				// TODO - verify if used
				assert(0);
				match = (state->taskFunc.ttype == type && fvalue(&state->taskFunc) == fvalue(Address(paramObj)));
			}
			if (match) {
				task = state->id;
				countTasks++;
			}
		}
		if (countTasks) {
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

void pause_scripts() {
	LState *t;

	for (t = lua_rootState->next; t != NULL; t = t->next) {
		if (lua_state != t)
			t->paused = 0;
	}
}

void unpause_scripts() {
	LState *t;

	for (t = lua_rootState->next; t != NULL; t = t->next) {
		if (lua_state != t)
			t->paused = 0;
	}
}

void current_script() {
	ttype(lua_state->stack.top) = LUA_T_TASK;
	nvalue(lua_state->stack.top) = (float)lua_state->id;
	incr_top;
}

void break_here() {}

void lua_runtasks() {
	int32 flag;
	LState *tmpState = lua_state;
	LState *state = lua_state->next;
	if (state) {
		do {
			state->flag2 = 0;
			state = state->next;
		} while	(state);

loop:
		lua_state = lua_state->next;
		if (lua_state) {
			while (1) {
				if (!lua_state->flag2 && !lua_state->paused) {
					jmp_buf	errorJmp;
					lua_state->errorJmp = &errorJmp;
					if (setjmp(errorJmp)) {
						lua_Task *t, *m;
						for (t = lua_state->task; t != NULL;) {
							m = t->next;
							delete t;
							t = m;
						}
						flag = 0;
						lua_state->task = NULL;
					} else {
						if (lua_state->task) {
							flag = luaD_call(lua_state->task->some_base, lua_state->task->some_results);
						} else {
							StkId base = lua_state->Cstack.base;
							luaD_openstack((lua_state->stack.top - lua_state->stack.stack) - base);
							set_normalized(lua_state->stack.stack + lua_state->Cstack.base, &lua_state->taskFunc);
							flag = luaD_call(base + 1, 255);
						}
					}
					if (!flag) {
						state = lua_state->next;
						lua_statedeinit(lua_state);
						luaM_free(lua_state);
						goto label2;
					}
					lua_state->flag2 = 1;
				}
				state = lua_state->next;
label2:
				lua_state = state;
				if (!state) {
					break;
				}
			}
		}
		lua_state = tmpState;
		state = lua_state->next;
		do {
			if (!state)
				break;
			if (!state->paused && !state->flag2)
				goto loop;
			state = state->next;
		} while (state);
	}
}

} // end of namespace Grim
