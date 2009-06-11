/*
** $Id$
** Garbage Collector
** See Copyright Notice in lua.h
*/


#include "engines/grim/lua/ldo.h"
#include "engines/grim/lua/lfunc.h"
#include "engines/grim/lua/lgc.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lobject.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/lstring.h"
#include "engines/grim/lua/ltable.h"
#include "engines/grim/lua/ltm.h"
#include "engines/grim/lua/lua.h"

namespace Grim {

static int32 markobject (TObject *o);

/*
** =======================================================
** REF mechanism
** =======================================================
*/

int32 luaC_ref(TObject *o, int32 lock) {
	int32 ref;
	if (ttype(o) == LUA_T_NIL)
		ref = -1;   // special ref for nil
	else {
		for (ref = 0; ref < lua_state->refSize; ref++)
			if (lua_state->refArray[ref].status == FREE)
				goto found;
		// no more empty spaces */
		{
			int32 oldSize = lua_state->refSize;
			lua_state->refSize = luaM_growvector(&lua_state->refArray, lua_state->refSize, struct ref, refEM, MAX_WORD);
			for (ref = oldSize; ref < lua_state->refSize; ref++)
				lua_state->refArray[ref].status = FREE;
			ref = oldSize;
		}
found:
		lua_state->refArray[ref].o = *o;
		lua_state->refArray[ref].status = lock ? LOCK : HOLD;
	}
	return ref;
}

void lua_unref(int32 ref) {
	if (ref >= 0 && ref < lua_state->refSize)
		lua_state->refArray[ref].status = FREE;
}

TObject* luaC_getref(int32 ref) {
	if (ref == -1)
		return &luaO_nilobject;
	if (ref >= 0 && ref < lua_state->refSize && (lua_state->refArray[ref].status == LOCK || lua_state->refArray[ref].status == HOLD))
		return &lua_state->refArray[ref].o;
	else
		return NULL;
}

static void travlock() {
	int32 i;
	for (i = 0; i < lua_state->refSize; i++) {
		if (lua_state->refArray[i].status == LOCK) {
			markobject(&lua_state->refArray[i].o);
		}
	}
}

static int32 ismarked(TObject *o) {
	// valid only for locked objects
	switch (o->ttype) {
	case LUA_T_STRING:
	case LUA_T_USERDATA:
		return o->value.ts->head.marked;
	case LUA_T_ARRAY:
		return o->value.a->head.marked;
	case LUA_T_CLOSURE:
		return o->value.cl->head.marked;
	case LUA_T_PROTO:
		return o->value.tf->head.marked;
#ifdef LUA_DEBUG
	case LUA_T_LINE:
	case LUA_T_CLMARK:
	case LUA_T_CMARK:
	case LUA_T_PMARK:
		LUA_INTERNALERROR("internal error");
#endif
	default:  // nil, number or cproto
		return 1;
	}
}

static void invalidaterefs() {
	int32 i;
	for (i = 0; i < lua_state->refSize; i++)
		if (lua_state->refArray[i].status == HOLD && !ismarked(&lua_state->refArray[i].o))
			lua_state->refArray[i].status = COLLECTED;
}

void luaC_hashcallIM(Hash *l) {
	TObject t;
	ttype(&t) = LUA_T_ARRAY;
	for (; l; l = (Hash *)l->head.next) {
		avalue(&t) = l;
		luaD_gcIM(&t);
	}
}

void luaC_strcallIM(TaggedString *l) {
	TObject o;
	ttype(&o) = LUA_T_USERDATA;
	for (; l; l=(TaggedString *)l->head.next) {
		if (l->constindex == -1) {  // is userdata?
			tsvalue(&o) = l;
			luaD_gcIM(&o);
		}
	}
}

static GCnode *listcollect(GCnode *l) {
	GCnode *frees = NULL;
	while (l) {
		GCnode *next = l->next;
		l->marked = 0;
		while (next && !next->marked) {
			l->next = next->next;
			next->next = frees;
			frees = next;
			next = l->next;
		}
		l = next;
	}
	return frees;
}

static void strmark(TaggedString *s) {
	if (!s->head.marked)
		s->head.marked = 1;
}

static void protomark(TProtoFunc *f) {
	if (!f->head.marked) {
		LocVar *v = f->locvars;
		int32 i;
		f->head.marked = 1;
		if (f->fileName)
			strmark(f->fileName);
		for (i = 0; i < f->nconsts; i++)
			markobject(&f->consts[i]);
		if (v) {
			for (; v->line != -1; v++) {
				if (v->varname)
					strmark(v->varname);
			}
		}
	}
}

static void closuremark(Closure *f) {
	if (!f->head.marked) {
		int32 i;
		f->head.marked = 1;
		for (i = f->nelems; i >= 0; i--)
			markobject(&f->consts[i]);
	}
}

static void hashmark(Hash *h) {
	if (!h->head.marked) {
		int32 i;
		h->head.marked = 1;
		for (i = 0; i < nhash(h); i++) {
			Node *n = node(h, i);
			if (ttype(ref(n)) != LUA_T_NIL) {
				markobject(&n->ref);
				markobject(&n->val);
			}
		}
	}
}

static void globalmark() {
	TaggedString *g;
	for (g = (TaggedString *)lua_state->rootglobal.next; g; g = (TaggedString *)g->head.next){
		if (g->globalval.ttype != LUA_T_NIL) {
			markobject(&g->globalval);
			strmark(g);  // cannot collect non nil global variables
		}
	}
}

static int32 markobject(TObject *o) {
	switch (ttype(o)) {
	case LUA_T_USERDATA:
	case LUA_T_STRING:
		strmark(tsvalue(o));
		break;
	case LUA_T_ARRAY:
		hashmark(avalue(o));
		break;
	case LUA_T_CLOSURE:
	case LUA_T_CLMARK:
		closuremark(o->value.cl);
		break;
	case LUA_T_PROTO:
	case LUA_T_PMARK:
		protomark(o->value.tf);
		break;
	default:
		break;  // numbers, cprotos, etc
	}
	return 0;
}

static void markall() {
	luaD_travstack(markobject); // mark stack objects
	globalmark();  // mark global variable values and names
	travlock(); // mark locked objects
	luaT_travtagmethods(markobject);  // mark fallbacks
}

int32 lua_collectgarbage(int32 limit) {
	int32 recovered = lua_state->nblocks;  // to subtract nblocks after gc
	Hash *freetable;
	TaggedString *freestr;
	TProtoFunc *freefunc;
	Closure *freeclos;
	markall();
	invalidaterefs();
	freestr = luaS_collector();
	freetable = (Hash *)listcollect(&(lua_state->roottable));
	freefunc = (TProtoFunc *)listcollect(&(lua_state->rootproto));
	freeclos = (Closure *)listcollect(&(lua_state->rootcl));
	lua_state->GCthreshold *= 4;  // to avoid GC during GC
	luaC_hashcallIM(freetable);  // GC tag methods for tables
	luaC_strcallIM(freestr);  // GC tag methods for userdata
	luaD_gcIM(&luaO_nilobject);  // GC tag method for nil (signal end of GC)
	luaH_free(freetable);
	luaS_free(freestr);
	luaF_freeproto(freefunc);
	luaF_freeclosure(freeclos);
	recovered = recovered-lua_state->nblocks;
	lua_state->GCthreshold = (limit == 0) ? 2 * lua_state->nblocks : lua_state->nblocks + limit;
	return recovered;
}

void luaC_checkGC() {
	if (lua_state->nblocks >= lua_state->GCthreshold)
		lua_collectgarbage(0);
}

} // end of namespace Grim
