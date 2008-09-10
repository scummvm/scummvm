/*
** $Id$
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#include "common/util.h"

#include "engine/lua/lmem.h"
#include "engine/lua/lobject.h"
#include "engine/lua/lstate.h"
#include "engine/lua/lstring.h"
#include "engine/lua/lua.h"


#define gcsizestring(l)	(1 + (l / 64))  // "weight" for a string with length 'l'

TaggedString EMPTY = {{NULL, 2}, 0L, 0, {{{LUA_T_NIL, {NULL}}, 0L}}, {0}};

void luaS_init() {
	int32 i;
	L->string_root = luaM_newvector(NUM_HASHS, stringtable);
	for (i = 0; i < NUM_HASHS; i++) {
		L->string_root[i].size = 0;
		L->string_root[i].nuse = 0;
		L->string_root[i].hash = NULL;
	}
}

static uint32 hash_s(const char *s, int32 l) {
	uint32 h = 0;
	while (l--)
		h = ((h << 5) - h) ^ (byte)*(s++);
	return h;
}

static int32 newsize(stringtable *tb) {
	int32 size = tb->size;
	int32 realuse = 0;
	int32 i;

	// count how many entries are really in use
	for (i = 0; i < size; i++)
		if (tb->hash[i] && tb->hash[i] != &EMPTY)
			realuse++;
	if (2 * (realuse + 1) <= size)  // +1 is the new element
		return size;  // don't need to grow, just rehash to clear EMPTYs
	else
		return luaO_redimension(size);
}

static void grow(stringtable *tb) {
	int32 ns = newsize(tb);
	TaggedString **newhash = luaM_newvector(ns, TaggedString *);
	int32 i;

	for (i = 0; i < ns; i++)
		newhash[i] = NULL;
	// rehash
	tb->nuse = 0;
	for (i = 0; i < tb->size; i++) {
		if (tb->hash[i] && tb->hash[i] != &EMPTY) {
			int32 h = tb->hash[i]->hash % ns;
			while (newhash[h])
				h = (h + 1) % ns;
			newhash[h] = tb->hash[i];
			tb->nuse++;
		}
	}
	luaM_free(tb->hash);
	tb->size = ns;
	tb->hash = newhash;
}

static TaggedString *newone_s(const char *str, int32 l, uint32 h) {
	TaggedString *ts = (TaggedString *)luaM_malloc(sizeof(TaggedString) + l);
	memcpy(ts->str, str, l);
	ts->str[l] = 0;  // ending 0
	ts->u.s.globalval.ttype = LUA_T_NIL;  // initialize global value
	ts->u.s.len = l;
	ts->constindex = 0;
	L->nblocks += gcsizestring(l);
	ts->head.marked = 0;
	ts->head.next = (GCnode *)ts;  // signal it is in no list
	ts->hash = h;
	return ts;
}

static TaggedString *newone_u(char *buff, int32 tag, uint32 h) {
	TaggedString *ts = luaM_new(TaggedString);
	ts->u.d.v = buff;
	ts->u.d.tag = (tag == LUA_ANYTAG) ? 0 : tag;
	ts->constindex = -1;  // tag -> this is a userdata
	L->nblocks++;
	ts->head.marked = 0;
	ts->head.next = (GCnode *)ts;  // signal it is in no list
	ts->hash = h;
	return ts;
}

static TaggedString *insert_s (const char *str, int32 l, stringtable *tb) {
	TaggedString *ts;
	uint32 h = hash_s(str, l);
	int32 size = tb->size;
	int32 i;
	int32 j = -1;
	if (tb->nuse * 3 >= size * 2) {
		grow(tb);
		size = tb->size;
	}
	for (i = h % size; (ts = tb->hash[i]); ) {
		if (ts == &EMPTY)
			j = i;
		else if (ts->constindex >= 0 && ts->u.s.len == l && (memcmp(str, ts->str, MAX(l, ts->u.s.len)) == 0))
			return ts;
		if (++i == size)
			i = 0;
	}
	// not found
	if (j != -1)  // is there an EMPTY space?
		i = j;
	else
		tb->nuse++;
	ts = tb->hash[i] = newone_s(str, l, h);
	return ts;
}

static TaggedString *insert_u(void *buff, int32 tag, stringtable *tb) {
	TaggedString *ts;
#ifdef TARGET_64BITS
	uint32 h = (uint64)buff;
#else
	uint32 h = (uint32)buff;
#endif
	int32 size = tb->size;
	int32 i;
	int32 j = -1;
	if (tb->nuse * 3 >= size * 2) {
		grow(tb);
		size = tb->size;
	}
	for (i = h % size; (ts = tb->hash[i]) != NULL; ) {
		if (ts == &EMPTY)
			j = i;
		else if (ts->constindex < 0 && (tag == ts->u.d.tag || tag == LUA_ANYTAG) && buff == ts->u.d.v)
			return ts;
		if (++i == size)
			i = 0;
	}
	// not found
	if (j != -1)  // is there an EMPTY space?
		i = j;
	else
		tb->nuse++;
	ts = tb->hash[i] = newone_u((char *)buff, tag, h);
	return ts;
}

TaggedString *luaS_createudata(void *udata, int32 tag) {
#ifdef TARGET_64BITS
	return insert_u(udata, tag, &L->string_root[(uint64)udata % NUM_HASHS]);
#else
	return insert_u(udata, tag, &L->string_root[(uint32)udata % NUM_HASHS]);
#endif
}

TaggedString *luaS_newlstr(const char *str, int32 l) {
	int32 i = (l == 0) ? 0 : (byte)str[0];
	return insert_s(str, l, &L->string_root[i % NUM_HASHS]);
}

TaggedString *luaS_new(const char *str) {
	return luaS_newlstr(str, strlen(str));
}

TaggedString *luaS_newfixedstring(const char *str) {
	TaggedString *ts = luaS_new(str);
	if (ts->head.marked == 0)
		ts->head.marked = 2;  // avoid GC
	return ts;
}

void luaS_free(TaggedString *l) {
	while (l) {
		TaggedString *next = (TaggedString *)l->head.next;
		L->nblocks -= (l->constindex == -1) ? 1 : gcsizestring(l->u.s.len);
		luaM_free(l);
		l = next;
	}
}

/*
** Garbage collection functions.
*/

static void remove_from_list(GCnode *l) {
	while (l) {
		GCnode *next = l->next;
		while (next && !next->marked)
			next = l->next = next->next;
		l = next;
	}
}

TaggedString *luaS_collector() {
	TaggedString *frees = NULL;
	int32 i;
	remove_from_list(&(L->rootglobal));
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tb = &L->string_root[i];
		int32 j;
		for (j = 0; j < tb->size; j++) {
			TaggedString *t = tb->hash[j];
			if (!t)
				continue;
			if (t->head.marked == 1)
				t->head.marked = 0;
			else if (!t->head.marked) {
				t->head.next = (GCnode *)frees;
				frees = t;
				tb->hash[j] = &EMPTY;
			}
		}
	}
	return frees;
}

TaggedString *luaS_collectudata() {
	TaggedString *frees = NULL;
	int32 i;
	L->rootglobal.next = NULL;  // empty list of globals
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tb = &L->string_root[i];
		int32 j;
		for (j = 0; j < tb->size; j++) {
			TaggedString *t = tb->hash[j];
			if (!t || t == &EMPTY || t->constindex != -1)
				continue;  // get only user data
			t->head.next = (GCnode *)frees;
			frees = t;
			tb->hash[j] = &EMPTY;
		}
	}
	return frees;
}

void luaS_freeall() {
	int32 i;
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tb = &L->string_root[i];
		int32 j;
		for (j = 0; j < tb->size; j++) {
			TaggedString *t = tb->hash[j];
			if (t == &EMPTY)
				continue;
			luaM_free(t);
		}
		luaM_free(tb->hash);
	}
	luaM_free(L->string_root);
}

void luaS_rawsetglobal(TaggedString *ts, TObject *newval) {
	ts->u.s.globalval = *newval;
	if (ts->head.next == (GCnode *)ts) {  // is not in list?
		ts->head.next = L->rootglobal.next;
		L->rootglobal.next = (GCnode *)ts;
	}
}

char *luaS_travsymbol (int32 (*fn)(TObject *)) {
	TaggedString *g;
	for (g = (TaggedString *)L->rootglobal.next; g; g = (TaggedString *)g->head.next)
		if (fn(&g->u.s.globalval))
			return g->str;
	return NULL;
}

int32 luaS_globaldefined(const char *name) {
	TaggedString *ts = luaS_new(name);
	return ts->u.s.globalval.ttype != LUA_T_NIL;
}

