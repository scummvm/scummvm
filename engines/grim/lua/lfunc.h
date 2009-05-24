/*
** $Id$
** Lua Function structures
** See Copyright Notice in lua.h
*/

#ifndef lfunc_h
#define lfunc_h


#include "engine/lua/lobject.h"


TProtoFunc *luaF_newproto();
Closure *luaF_newclosure(int32 nelems);
void luaF_freeproto(TProtoFunc *l);
void luaF_freeclosure(Closure *l);

char *luaF_getlocalname (TProtoFunc *func, int32 local_number, int32 line);


#endif
