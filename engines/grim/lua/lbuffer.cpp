/*
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lstate.h"

namespace Grim {

/*-------------------------------------------------------
**  Auxiliary buffer
-------------------------------------------------------*/

#define BUFF_STEP	32

#define openspace(size)  if (Mbuffnext + (size) > Mbuffsize) Openspace(size)

static void Openspace(int32 size) {
	int32 base = Mbuffbase - Mbuffer;
	Mbuffsize *= 2;
	if (Mbuffnext + size > Mbuffsize)  // still not big enough?
		Mbuffsize = Mbuffnext + size;
	Mbuffer = (char *)luaM_realloc(Mbuffer, Mbuffsize);
	Mbuffbase = Mbuffer + base;
}

char *luaL_openspace(int32 size) {
	openspace(size);
	return Mbuffer + Mbuffnext;
}

void luaL_addchar(int32 c) {
	openspace(BUFF_STEP);
	Mbuffer[Mbuffnext++] = c;
}

void luaL_resetbuffer() {
	Mbuffnext = Mbuffbase - Mbuffer;
}

void luaL_addsize(int32 n) {
	Mbuffnext += n;
}

int32 luaL_newbuffer(int32 size) {
	int32 old = Mbuffbase - Mbuffer;
	openspace(size);
	Mbuffbase = Mbuffer + Mbuffnext;
	return old;
}

void luaL_oldbuffer(int32 old) {
	Mbuffnext = Mbuffbase - Mbuffer;
	Mbuffbase = Mbuffer + old;
}

char *luaL_buffer() {
	return Mbuffbase;
}

} // end of namespace Grim
