/*
** Debugging API
** See Copyright Notice in lua.h
*/


#ifndef GRIM_LUADEBUG_H
#define GRIM_LUADEBUG_H


#include "engines/grim/lua/lua.h"

namespace Grim {

typedef lua_Object lua_Function;

typedef void (*lua_LHFunction)(int32 line);
typedef void (*lua_CHFunction)(lua_Function func, const char *file, int32 line);

lua_Function lua_stackedfunction(int32 level);
void lua_funcinfo(lua_Object func, const char **filename, int32 *linedefined);
int32 lua_currentline(lua_Function func);
const char *lua_getobjname(lua_Object o, const char **name);

lua_Object lua_getlocal(lua_Function func, int32 local_number, char **name);
int32 lua_setlocal(lua_Function func, int32 local_number);

extern lua_LHFunction lua_linehook;
extern lua_CHFunction lua_callhook;
extern int32 lua_debug;

} // end of namespace Grim


#endif
