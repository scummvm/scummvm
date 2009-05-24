/*
** $Id$
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#include "engine/lua/lua.h"
#include "engine/lua/lobject.h"

TObject *luaA_Address(lua_Object o);
void luaA_pushobject(TObject *o);
void luaA_packresults();
int32 luaA_passresults();

#endif
