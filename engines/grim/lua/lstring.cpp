/*
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp

#include "common/util.h"

#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lobject.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/lstring.h"
#include "engines/grim/lua/lua.h"

namespace Grim {

#define gcsizestring(l)	(1 + (l / 64))  // "weight" for a string with length 'l'

TaggedString EMPTY = {{nullptr, 2}, 0, 0L, {LUA_T_NIL, {nullptr}}, {0}};

void luaS_init() {
	int32 i;
	string_root = luaM_newvector(NUM_HASHS, stringtable);
	for (i = 0; i < NUM_HASHS; i++) {
		string_root[i].size = 0;
		string_root[i].nuse = 0;
		string_root[i].hash = nullptr;
	}
}

static uint32 hash(const char *s, int32 tag) {
	uint32 h;
	if (tag != LUA_T_STRING) {
		h = (uintptr)s;
	} else {
		h = 0;
		while (*s)
			h = ((h << 5) - h) ^ (byte)*(s++);
	}
	return h;
}

static void grow(stringtable *tb) {
	int newsize = luaO_redimension(tb->size);
	TaggedString **newhash = luaM_newvector(newsize, TaggedString *);
	int32 i;

	for (i = 0; i < newsize; i++)
		newhash[i] = nullptr;
	// rehash
	tb->nuse = 0;
	for (i = 0; i < tb->size; i++) {
		if (tb->hash[i] && tb->hash[i] != &EMPTY) {
			int32 h = tb->hash[i]->hash % newsize;
			while (newhash[h])
				h = (h + 1) % newsize;
			newhash[h] = tb->hash[i];
			tb->nuse++;
		}
	}
	luaM_free(tb->hash);
	tb->size = newsize;
	tb->hash = newhash;
}

static TaggedString *newone(const char *buff, int32 tag, uint32 h) {
	TaggedString *ts;
	if (tag == LUA_T_STRING) {
		int l = strlen(buff);
		ts = (TaggedString *)luaM_malloc(sizeof(TaggedString) + l);
		strcpy(ts->str, buff);
		ts->globalval.ttype = LUA_T_NIL;  /* initialize global value */
		ts->constindex = 0;
		nblocks += gcsizestring(l);
	} else {
		ts = (TaggedString *)luaM_malloc(sizeof(TaggedString));
		ts->globalval.value.ts = (TaggedString *)const_cast<char *>(buff);
		ts->globalval.ttype = (lua_Type)(tag == LUA_ANYTAG ? 0 : tag);
		ts->constindex = -1;  /* tag -> this is a userdata */
		nblocks++;
	}
	ts->head.marked = 0;
	ts->head.next = (GCnode *)ts;  // signal it is in no list
	ts->hash = h;
	return ts;
}

static TaggedString *insert(const char *buff, int32 tag, stringtable *tb) {
	TaggedString *ts;
	uint32 h = hash(buff, tag);
	int32 size = tb->size;
	int32 i;
	int32 j = -1;
	if (tb->nuse * 3 >= size * 2) {
		grow(tb);
		size = tb->size;
	}
	for (i = h % size; (ts = tb->hash[i]) != nullptr; ) {
		if (ts == &EMPTY)
			j = i;
		else if ((ts->constindex >= 0) ? // is a string?
				(tag == LUA_T_STRING && (strcmp(buff, ts->str) == 0)) :
				((tag == ts->globalval.ttype || tag == LUA_ANYTAG) && buff == (const char *)ts->globalval.value.ts))
			return ts;
		if (++i == size)
			i = 0;
	}
	// not found
	if (j != -1)  // is there an EMPTY space?
		i = j;
	else
		tb->nuse++;
	ts = tb->hash[i] = newone(buff, tag, h);
	return ts;
}

TaggedString *luaS_createudata(void *udata, int32 tag) {
	return insert((char *)udata, tag, &string_root[(uintptr)udata % NUM_HASHS]);
}

TaggedString *luaS_new(const char *str) {
	return insert(str, LUA_T_STRING, &string_root[(uintptr)str[0] % NUM_HASHS]);
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
		nblocks -= (l->constindex == -1) ? 1 : gcsizestring(strlen(l->str));
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
	TaggedString *frees = nullptr;
	int32 i;
	remove_from_list(&rootglobal);
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tb = &string_root[i];
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
	TaggedString *frees = nullptr;
	int32 i;
	rootglobal.next = nullptr;  // empty list of globals
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tb = &string_root[i];
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
		stringtable *tb = &string_root[i];
		int32 j;
		for (j = 0; j < tb->size; j++) {
			TaggedString *t = tb->hash[j];
			if (t == &EMPTY)
				continue;
			luaM_free(t);
		}
		luaM_free(tb->hash);
	}
	luaM_free(string_root);
}

void luaS_rawsetglobal(TaggedString *ts, TObject *newval) {
	ts->globalval = *newval;
	if (ts->head.next == (GCnode *)ts) {  // is not in list?
		ts->head.next = rootglobal.next;
		rootglobal.next = (GCnode *)ts;
	}
}

char *luaS_travsymbol (int32 (*fn)(TObject *)) {
	TaggedString *g;
	for (g = (TaggedString *)rootglobal.next; g; g = (TaggedString *)g->head.next)
		if (fn(&g->globalval))
			return g->str;
	return nullptr;
}

int32 luaS_globaldefined(const char *name) {
	TaggedString *ts = luaS_new(name);
	return ts->globalval.ttype != LUA_T_NIL;
}

} // end of namespace Grim
