/*
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef GRIM_LDO_H
#define GRIM_LDO_H


#include "engines/grim/lua/lobject.h"
#include "engines/grim/lua/lstate.h"

namespace Grim {

#define MULT_RET        255


/*
** macro to increment stack top.
** There must be always an empty slot at the lua_state->stack.top
*/
#define incr_top { if (lua_state->stack.top >= lua_state->stack.last) luaD_checkstack(1); lua_state->stack.top++; }

// macros to convert from lua_Object to (TObject *) and back

#define Address(lo)     ((lo) + lua_state->stack.stack - 1)
#define Ref(st)         ((st) - lua_state->stack.stack + 1)

void luaD_init();
void luaD_initthr();
void luaD_adjusttop(StkId newtop);
void luaD_openstack(int32 nelems);
void luaD_lineHook(int32 line);
void luaD_callHook(StkId base, TProtoFunc *tf, int32 isreturn);
void luaD_postret(StkId firstResult);
int32 luaD_call(StkId base, int32 nResults);
void luaD_callTM(TObject *f, int32 nParams, int32 nResults);
int32 luaD_protectedrun(int32 nResults);
void luaD_gcIM(TObject *o);
void luaD_travstack(int32 (*fn)(TObject *));
void luaD_checkstack(int32 n);

} // end of namespace Grim

#endif
