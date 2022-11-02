/*
** Garbage Collector
** See Copyright Notice in lua.h
*/

#ifndef GRIM_LGC_H
#define GRIM_LGC_H


#include "lobject.h"

namespace Grim {

void luaC_checkGC();
TObject* luaC_getref(int32 r);
int32 luaC_ref(TObject *o, int32 lock);
void luaC_hashcallIM(Hash *l);
void luaC_strcallIM(TaggedString *l);

} // end of namespace Grim

#endif
