/*
** Tag methods
** See Copyright Notice in lua.h
*/

#ifndef GRIM_LTM_H
#define GRIM_LTM_H


#include "engines/grim/lua/lobject.h"
#include "engines/grim/lua/lstate.h"

namespace Grim {

/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER IM"
*/
typedef enum {
	IM_GETTABLE = 0,
	IM_SETTABLE,
	IM_INDEX,
	IM_GETGLOBAL,
	IM_SETGLOBAL,
	IM_ADD,
	IM_SUB,
	IM_MUL,
	IM_DIV,
	IM_POW,
	IM_UNM,
	IM_LT,
	IM_LE,
	IM_GT,
	IM_GE,
	IM_CONCAT,
	IM_GC,
	IM_FUNCTION
} eIMS;
typedef int32 IMS;

#define IM_N 18

struct IM {
	TObject int_method[IM_N];
};


#define luaT_getim(tag, event)	(&IMtable[-(tag)].int_method[event])
#define luaT_getimbyObj(o, e)	(luaT_getim(luaT_efectivetag(o), (e)))

extern const char *luaT_eventname[];

void luaT_init();
void luaT_realtag(int32 tag);
int32 luaT_efectivetag(TObject *o);
void luaT_settagmethod(int32 t, const char *event, TObject *func);
TObject *luaT_gettagmethod(int32 t, const char *event);
const char *luaT_travtagmethods(int32 (*fn)(TObject *));

void luaT_setfallback();  // only if LUA_COMPAT2_5

} // end of namespace Grim

#endif
