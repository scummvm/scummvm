/*
** $Id$
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lobject.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/ltable.h"
#include "engines/grim/lua/lua.h"

namespace Grim {

#define gcsize(n)		(1 + (n / 16))
#define nuse(t)			((t)->nuse)
#define nodevector(t)	((t)->node)
#define REHASH_LIMIT	0.70    // avoid more than this % full
#define TagDefault		LUA_T_ARRAY;

#ifdef TARGET_64BITS
static int64 hashindex(TObject *ref) {
	int64 h;

	switch (ttype(ref)) {
	case LUA_T_NUMBER:
		h = (int64)nvalue(ref);
		break;
	case LUA_T_STRING:
	case LUA_T_USERDATA:
		h = (int64)tsvalue(ref);
		break;
	case LUA_T_ARRAY:
		h = (int64)avalue(ref);
		break;
	case LUA_T_PROTO:
		h = (int64)tfvalue(ref);
		break;
	case LUA_T_CPROTO:
		h = (int64)fvalue(ref);
		break;
	case LUA_T_CLOSURE:
		h = (int64)clvalue(ref);
		break;
	case LUA_T_TASK:
		h = (int64)nvalue(ref);
		break;
	default:
		lua_error("unexpected type to index table");
		h = 0;  // to avoid warnings
	}
	return (h >= 0 ? h : -(h + 1));
}

#else

static int32 hashindex(TObject *ref) {
	int32 h;

	switch (ttype(ref)) {
	case LUA_T_NUMBER:
		h = (int32)nvalue(ref);
		break;
	case LUA_T_STRING:
	case LUA_T_USERDATA:
		h = (int32)tsvalue(ref);
		break;
	case LUA_T_ARRAY:
		h = (int32)avalue(ref);
		break;
	case LUA_T_PROTO:
		h = (int32)tfvalue(ref);
		break;
	case LUA_T_CPROTO:
		h = (int32)fvalue(ref);
		break;
	case LUA_T_CLOSURE:
		h = (int32)clvalue(ref);
		break;
	case LUA_T_TASK:
		h = (int32)nvalue(ref);
		break;
	default:
		lua_error("unexpected type to index table");
		h = 0;  // to avoid warnings
	}
	return (h >= 0 ? h : -(h + 1));
}

#endif

int32 present(Hash *t, TObject *key) {
	int32 tsize = nhash(t);
	int32 h = (int32)hashindex(key);
	int32 h1 = h % tsize;
	TObject *rf = ref(node(t, h1));
	if (ttype(rf) != LUA_T_NIL && !luaO_equalObj(key, rf)) {
		int32 h2 = h % (tsize - 2) + 1;
		do {
			h1 += h2;
			if (h1 >= tsize)
				h1 -= tsize;
			rf = ref(node(t, h1));
		} while (ttype(rf) != LUA_T_NIL && !luaO_equalObj(key, rf));
	}
	return h1;
}

/*
** Alloc a vector node
*/
Node *hashnodecreate(int32 nhash) {
	Node *v = luaM_newvector(nhash, Node);
	int32 i;
	for (i = 0; i < nhash; i++)
		ttype(ref(&v[i])) = LUA_T_NIL;
	return v;
}

/*
** Delete a hash
*/
static void hashdelete(Hash *t) {
	luaM_free(nodevector(t));
	luaM_free(t);
}

void luaH_free(Hash *frees) {
	while (frees) {
		Hash *next = (Hash *)frees->head.next;
		lua_state->nblocks -= gcsize(frees->nhash);
		hashdelete(frees);
		frees = next;
	}
}

Hash *luaH_new(int32 nhash) {
	Hash *t = luaM_new(Hash);
	nhash = luaO_redimension((int32)((float)nhash / REHASH_LIMIT));
	nodevector(t) = hashnodecreate(nhash);
	nhash(t) = nhash;
	nuse(t) = 0;
	t->htag = TagDefault;
	luaO_insertlist(&(lua_state->roottable), (GCnode *)t);
	lua_state->nblocks += gcsize(nhash);
	return t;
}

/*
** Rehash:
** Check if table has deleted slots. It it has, it does not need to
** grow, since rehash will reuse them.
*/
static int emptyslots(Hash *t) {
	int i;

	for (i = nhash(t) - 1; i >= 0; i--) {
		Node *n = node(t, i);
		if (ttype(ref(n)) != LUA_T_NIL && ttype(val(n)) == LUA_T_NIL)
			return 1;
	}
	return 0;
}

static void rehash(Hash *t) {
	int32 nold = nhash(t);
	Node *vold = nodevector(t);
	int32 i;
	if (!emptyslots(t))
		nhash(t) = luaO_redimension(nhash(t));
	nodevector(t) = hashnodecreate(nhash(t));
	for (i = 0; i < nold; i++) {
		Node *n = vold + i;
		if (ttype(ref(n)) != LUA_T_NIL && ttype(val(n)) != LUA_T_NIL)
			*node(t, present(t, ref(n))) = *n;  // copy old node to luaM_new hash
	}
	lua_state->nblocks += gcsize(t->nhash) - gcsize(nold);
	luaM_free(vold);
}

/*
** If the hash node is present, return its pointer, otherwise return
** null.
*/
TObject *luaH_get(Hash *t, TObject *ref) {
	int32 h = present(t, ref);
	if (ttype(ref(node(t, h))) != LUA_T_NIL)
		return val(node(t, h));
	else
		return NULL;
}

/*
** If the hash node is present, return its pointer, otherwise create a luaM_new
** node for the given reference and also return its pointer.
*/
TObject *luaH_set(Hash *t, TObject *ref) {
	Node *n = node(t, present(t, ref));
	if (ttype(ref(n)) == LUA_T_NIL) {
		nuse(t)++;
		if ((float)nuse(t) > (float)nhash(t) * REHASH_LIMIT) {
			rehash(t);
			n = node(t, present(t, ref));
		}
		*ref(n) = *ref;
		ttype(val(n)) = LUA_T_NIL;
	}
	return (val(n));
}

static Node *hashnext(Hash *t, int32 i) {
	Node *n;
	int32 tsize = nhash(t);
	if (i >= tsize)
		return NULL;
	n = node(t, i);
	while (ttype(ref(n)) == LUA_T_NIL || ttype(val(n)) == LUA_T_NIL) {
		if (++i >= tsize)
			return NULL;
		n = node(t, i);
	}
	return node(t, i);
}

Node *luaH_next(TObject *o, TObject *r) {
	Hash *t = avalue(o);
	if (ttype(r) == LUA_T_NIL)
		return hashnext(t, 0);
	else {
		int32 i = present(t, r);
		Node *n = node(t, i);
		luaL_arg_check(ttype(ref(n)) != LUA_T_NIL && ttype(val(n)) != LUA_T_NIL, 2, "key not found");
		return hashnext(t, i + 1);
	}
}

} // end of namespace Grim
