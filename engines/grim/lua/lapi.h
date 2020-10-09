/*
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef GRIM_LAPI_H
#define GRIM_LAPI_H


#include "engines/grim/lua/lua.h"
#include "engines/grim/lua/lobject.h"

namespace Grim {

TObject *luaA_Address(lua_Object o);
void luaA_pushobject(TObject *o);
void luaA_packresults();
int32 luaA_passresults();
void set_normalized(TObject *d, TObject *s);

} // end of namespace Grim

#endif
