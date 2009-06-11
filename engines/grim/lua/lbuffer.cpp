/*
** $Id$
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lstate.h"

namespace Grim {

/*-------------------------------------------------------
**  Auxiliary buffer
-------------------------------------------------------*/

#define BUFF_STEP	32

#define openspace(size)  if (lua_state->Mbuffnext + (size) > lua_state->Mbuffsize) Openspace(size)

static void Openspace(int32 size) {
	LState *l = lua_state;  // to optimize
	int32 base = l->Mbuffbase-l->Mbuffer;
	l->Mbuffsize *= 2;
	if (l->Mbuffnext + size > l->Mbuffsize)  // still not big enough?
		l->Mbuffsize = l->Mbuffnext+size;
	l->Mbuffer = (char *)luaM_realloc(l->Mbuffer, l->Mbuffsize);
	l->Mbuffbase = l->Mbuffer+base;
}

char *luaL_openspace(int32 size) {
	openspace(size);
	return lua_state->Mbuffer + lua_state->Mbuffnext;
}

void luaL_addchar(int32 c) {
	openspace(BUFF_STEP);
	lua_state->Mbuffer[lua_state->Mbuffnext++] = c;
}

void luaL_resetbuffer() {
	lua_state->Mbuffnext = lua_state->Mbuffbase - lua_state->Mbuffer;
}

void luaL_addsize(int32 n) {
	lua_state->Mbuffnext += n;
}

int32 luaL_newbuffer(int32 size) {
	int32 old = lua_state->Mbuffbase - lua_state->Mbuffer;
	openspace(size);
	lua_state->Mbuffbase = lua_state->Mbuffer + lua_state->Mbuffnext;
	return old;
}

void luaL_oldbuffer(int32 old) {
	lua_state->Mbuffnext = lua_state->Mbuffbase - lua_state->Mbuffer;
	lua_state->Mbuffbase = lua_state->Mbuffer + old;
}

char *luaL_buffer() {
	return lua_state->Mbuffbase;
}

} // end of namespace Grim
