/*
** $Id$
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef auxlib_h
#define auxlib_h


#include "engine/lua/lua.h"


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
#define luaL_check_string(n)	(luaL_check_lstr((n), NULL))
#define luaL_opt_string(n, d) (luaL_opt_lstr((n), (d), NULL))

void luaL_openlib(luaL_reg *l, int32 n);
void luaL_addlibtolist(luaL_reg *l, int32 n);
void luaL_argerror(int32 numarg, const char *extramsg);
const char *luaL_check_lstr(int32 numArg, int32 *len);
const char *luaL_opt_lstr(int32 numArg, const char *def, int32 *len);
double luaL_check_number(int32 numArg);
double luaL_opt_number(int32 numArg, double def);
lua_Object luaL_functionarg(int32 arg);
lua_Object luaL_tablearg(int32 arg);
lua_Object luaL_nonnullarg(int32 numArg);
void luaL_verror(const char *fmt, ...);
char *luaL_openspace(int32 size);
void luaL_resetbuffer();
void luaL_addchar(int32 c);
int32 luaL_getsize();
void luaL_addsize(int32 n);
int32 luaL_newbuffer(int32 size);
void luaL_oldbuffer(int32 old);
char *luaL_buffer();
int32 luaL_findstring(const char *name, const char *list[]);


#endif
