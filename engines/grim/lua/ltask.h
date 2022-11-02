#ifndef GRIM_LTASK_H
#define GRIM_LTASK_H

#include "engines/grim/lua/lua.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/lbuiltin.h"

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
	bool executed;
	StkId initBase;
	int32 initResults;
};

void lua_taskinit(lua_Task *task, lua_Task *next, StkId tbase, int results);
void lua_taskresume(lua_Task *task, Closure *closure, TProtoFunc *protofunc, StkId tbase);
StkId luaV_execute(lua_Task *task);

void pause_script();
void unpause_script();

void runtasks(LState *const rootState);

} // end of namespace Grim

#endif
