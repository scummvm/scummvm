/*
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#ifndef GRIM_LTABLE_H
#define GRIM_LTABLE_H

#include "engines/grim/lua/lobject.h"

namespace Grim {

#define node(t, i)	(&(t)->node[i])
#define ref(n)		(&(n)->ref)
#define val(n)		(&(n)->val)
#define nhash(t)	((t)->nhash)

Hash *luaH_new(int32 nhash);
void luaH_free(Hash *frees);
TObject *luaH_get(Hash *t, TObject *r);
TObject *luaH_set(Hash *t, TObject *r);
Node *luaH_next(TObject *o, TObject *r);
Node *hashnodecreate(int32 nhash);
int32 present(Hash *t, TObject *key);

} // end of namespace Grim

#endif
