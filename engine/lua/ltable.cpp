/*
** $Id$
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#include "engine/lua/lauxlib.h"
#include "engine/lua/lmem.h"
#include "engine/lua/lobject.h"
#include "engine/lua/lstate.h"
#include "engine/lua/ltable.h"
#include "engine/lua/lua.h"

#define gcsize(n)		(1 + (n / 16))
#define nuse(t)			((t)->nuse)
#define nodevector(t)	((t)->node)
#define REHASH_LIMIT	0.70    // avoid more than this % full
#define TagDefault		LUA_T_ARRAY;

static long int hashindex(TObject *ref) {
	long int h;
	switch (ttype(ref)) {
	case LUA_T_NUMBER:
		h = (long int)nvalue(ref);
		break;
	case LUA_T_STRING:
	case LUA_T_USERDATA:
		h = (long int)tsvalue(ref);
		break;
	case LUA_T_ARRAY:
		h = (long int)avalue(ref);
		break;
	case LUA_T_PROTO:
		h = (long int)tfvalue(ref);
		break;
	case LUA_T_CPROTO:
		h = (long int)fvalue(ref);
		break;
	case LUA_T_CLOSURE:
		h = (long int)clvalue(ref);
		break;
	case LUA_T_TASK:
		h = (long int)nvalue(ref);
		break;
	default:
		lua_error("unexpected type to index table");
		h = 0;  // to avoid warnings
	}
	return (h >= 0 ? h : -(h + 1));
}

int32 present(Hash *t, TObject *key) {
	int32 tsize = nhash(t);
	long int h = hashindex(key);
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
		L->nblocks -= gcsize(frees->nhash);
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
	luaO_insertlist(&(L->roottable), (GCnode *)t);
	L->nblocks += gcsize(nhash);
	return t;
}

static int32 newsize(Hash *t) {
	Node *v = t->node;
	int32 size = nhash(t);
	int32 realuse = 0;
	int32 i;
	for (i = 0; i < size; i++) {
		if (ttype(ref(v + i)) != LUA_T_NIL && ttype(val(v + i)) != LUA_T_NIL)
			realuse++;
	}
	if (2 * (realuse + 1) <= size)  // +1 is the new element
		return size;  // don't need to grow, just rehash
	else
		return luaO_redimension(size);
}

static void rehash(Hash *t) {
	int32 nold = nhash(t);
	Node *vold = nodevector(t);
	int32 nnew = newsize(t);
	int32 i;
	nodevector(t) = hashnodecreate(nnew);
	nhash(t) = nnew;
	for (i = 0; i < nold; i++) {
		Node *n = vold + i;
		if (ttype(ref(n)) != LUA_T_NIL && ttype(val(n)) != LUA_T_NIL)
			*node(t, present(t, ref(n))) = *n;  // copy old node to luaM_new hash
	}
	L->nblocks += gcsize(nnew) - gcsize(nold);
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
