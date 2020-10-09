#ifndef GRIM_LTASK_H
#define GRIM_LTASK_H

#include "engines/grim/lua/lua.h"
#include "engines/grim/lua/lstate.h"

namespace Grim {

struct lua_Task {
	lua_Task *next;
	struct Stack *S;
	Closure *cl;
	TProtoFunc *tf;
	StkId base;
	byte *pc;
	TObject *consts;
	int32 aux;
	bool some_flag;
	StkId some_base;
	int32 some_results;
};

void lua_taskinit(lua_Task *task, lua_Task *next, StkId tbase, int results);
void lua_taskresume(lua_Task *task, Closure *closure, TProtoFunc *protofunc, StkId tbase);
StkId luaV_execute(lua_Task *task);

void start_script();
void stop_script();
void next_script();
void identify_script();
void pause_script();
void pause_scripts();
void unpause_script();
void unpause_scripts();
void find_script();
void break_here();
void sleep_for();

void runtasks(LState *const rootState);

} // end of namespace Grim

#endif
