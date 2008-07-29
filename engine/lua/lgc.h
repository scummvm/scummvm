/*
** $Id$
** Garbage Collector
** See Copyright Notice in lua.h
*/

#ifndef lgc_h
#define lgc_h


#include "lobject.h"


void luaC_checkGC();
TObject* luaC_getref(int32 ref);
int32 luaC_ref(TObject *o, int32 lock);
void luaC_hashcallIM(Hash *l);
void luaC_strcallIM(TaggedString *l);


#endif
