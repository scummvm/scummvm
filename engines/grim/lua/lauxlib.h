/*
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef GRIM_AUXLIB_H
#define GRIM_AUXLIB_H


#include "engines/grim/lua/lua.h"

namespace Grim {

struct luaL_reg {
	const char *name;
	lua_CFunction func;
};

struct luaL_libList {
	luaL_reg *list;
	int32 number;
	luaL_libList *next;
};

extern luaL_libList *list_of_libs;

#define luaL_arg_check(cond, numarg, extramsg) if (!(cond)) \
	luaL_argerror(numarg,extramsg)

void luaL_openlib(luaL_reg *l, int32 n);
void luaL_addlibtolist(luaL_reg *l, int32 n);
void luaL_argerror(int32 numarg, const char *extramsg);
const char *luaL_check_string(int32 numArg);
const char *luaL_opt_string(int32 numArg, const char *def);
float luaL_check_number(int32 numArg);
float luaL_opt_number(int32 numArg, float def);
lua_Object luaL_functionarg(int32 arg);
lua_Object luaL_tablearg(int32 arg);
lua_Object luaL_nonnullarg(int32 numArg);
void luaL_verror(const char *fmt, ...);
char *luaL_openspace(int32 size);
void luaL_resetbuffer();
void luaL_addchar(int32 c);
void luaL_addsize(int32 n);
int32 luaL_newbuffer(int32 size);
void luaL_oldbuffer(int32 old);
char *luaL_buffer();

} // end of namespace Grim

#endif
