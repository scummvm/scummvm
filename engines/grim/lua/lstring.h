/*
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ifndef LSTRING_H
#define LSTRING_H


#include "engines/grim/lua/lobject.h"

namespace Grim {

void luaS_init();
TaggedString *luaS_createudata(void *udata, int32 tag);
TaggedString *luaS_collector();
void luaS_free (TaggedString *l);
TaggedString *luaS_new(const char *str);
TaggedString *luaS_newfixedstring (const char *str);
void luaS_rawsetglobal(TaggedString *ts, TObject *newval);
char *luaS_travsymbol(int32 (*fn)(TObject *));
int32 luaS_globaldefined(const char *name);
TaggedString *luaS_collectudata();
void luaS_freeall();

extern TaggedString EMPTY;
#define NUM_HASHS  61

} // end of namespace Grim

#endif
