/*
** load bytecodes from files
** See Copyright Notice in lua.h
*/

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lfunc.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lstring.h"
#include "engines/grim/lua/lundump.h"

namespace Grim {

static float conv_float(const byte *data) {
	float f;
	byte *fdata = (byte *)(&f);
	fdata[0] = data[3];
	fdata[1] = data[2];
	fdata[2] = data[1];
	fdata[3] = data[0];
	return f;
}

static void unexpectedEOZ(ZIO *Z) {
	luaL_verror("unexpected end of file in %s", zname(Z));
}

static int32 ezgetc(ZIO *Z) {
	int32 c = zgetc(Z);
	if (c == EOZ)
		unexpectedEOZ(Z);
	return c;
}

static void ezread(ZIO *Z, void *b, int32 n) {
	int32 r = zread(Z, b, n);
	if (r)
		unexpectedEOZ(Z);
}

static uint16 LoadWord(ZIO *Z) {
	uint16 hi = ezgetc(Z);
	uint16 lo = ezgetc(Z);
	return (hi << 8) | lo;
}

static void *LoadBlock(int size, ZIO *Z) {
	void *b = luaM_malloc(size);
	ezread(Z, b, size);
	return b;
}

static uint32 LoadSize(ZIO *Z) {
	uint32 hi = LoadWord(Z);
	uint32 lo = LoadWord(Z);
	return (hi << 16) | lo;
}

static float LoadFloat(ZIO *Z) {
	uint32 l = LoadSize(Z);
	return conv_float((const byte *)&l);
}

static TaggedString *LoadTString(ZIO *Z) {
	int32 size = LoadWord(Z);
	int32 i;

	if (size == 0)
		return nullptr;
	else {
		char *s = luaL_openspace(size);
		ezread(Z, s, size);
		for (i = 0; i < size; i++)
			s[i] ^= 0xff;
		return luaS_new(s);
	}
}

static void LoadLocals(TProtoFunc *tf, ZIO *Z) {
	int32 i, n = LoadWord(Z);
	if (n == 0)
		return;
	tf->locvars = luaM_newvector(n + 1, LocVar);
	for (i = 0; i < n; i++) {
		tf->locvars[i].line = LoadWord(Z);
		tf->locvars[i].varname = LoadTString(Z);
	}
	tf->locvars[i].line = -1;		// flag end of vector
	tf->locvars[i].varname = nullptr;
}

static void LoadConstants(TProtoFunc *tf, ZIO *Z) {
	int32 i, n = LoadWord(Z);
	tf->nconsts = n;
	if (n == 0)
		return;
	tf->consts = luaM_newvector(n, TObject);
	for (i = 0; i < n; i++) {
		TObject *o = tf->consts + i;
		int c = ezgetc(Z);
		switch (c) {
		case ID_NUM:
			ttype(o) = LUA_T_NUMBER;
			nvalue(o) = LoadFloat(Z);
			break;
		case ID_STR:
			ttype(o) = LUA_T_STRING;
			tsvalue(o) = LoadTString(Z);
			break;
		case ID_FUN:
			ttype(o) = LUA_T_PROTO;
			tfvalue(o) = nullptr;
		default:
			break;
		}
	}
}

static void LoadFunctions(TProtoFunc *tf, ZIO *Z);

static TProtoFunc *LoadFunction(ZIO *Z) {
	TProtoFunc *tf = luaF_newproto();
	tf->lineDefined = LoadWord(Z);
	tf->fileName = LoadTString(Z);
	tf->code = (byte *)LoadBlock(LoadSize(Z), Z);
	LoadConstants(tf, Z);
	LoadLocals(tf, Z);
	LoadFunctions(tf, Z);

	return tf;
}

static void LoadFunctions(TProtoFunc *tf, ZIO *Z) {
	while (ezgetc(Z) == ID_FUNCTION) {
		int32 i = LoadWord(Z);
		TProtoFunc *t = LoadFunction(Z);
		TObject *o = tf->consts + i;
		tfvalue(o) = t;
	}
}

static void LoadSignature(ZIO *Z) {
	const char *s = SIGNATURE;

	while (*s && ezgetc(Z) == *s)
		++s;
	if (*s)
		luaL_verror("bad signature in %s", zname(Z));
}

static void LoadHeader(ZIO *Z) {
	int32 version, sizeofR;

	LoadSignature(Z);
	version = ezgetc(Z);
	if (version > VERSION)
		luaL_verror("%s too new: version=0x%02x; expected at most 0x%02x", zname(Z), version, VERSION);
	if (version < VERSION)			// check last major change
		luaL_verror("%s too old: version=0x%02x; expected at least 0x%02x", zname(Z), version, VERSION);
	sizeofR = ezgetc(Z);			// test number representation
	if (sizeofR != sizeof(float))
		luaL_verror("number expected float in %s", zname(Z));
	ezgetc(Z);
	ezgetc(Z);
	ezgetc(Z);
	ezgetc(Z);
}

static TProtoFunc *LoadChunk(ZIO *Z) {
	LoadHeader(Z);
	return LoadFunction(Z);
}

/*
** load one chunk from a file or buffer
** return main if ok and NULL at EOF
*/
TProtoFunc *luaU_undump1(ZIO *Z) {
	int32 c = zgetc(Z);
	if (c == ID_CHUNK)
		return LoadChunk(Z);
	else if (c != EOZ)
		luaL_verror("%s is not a Lua binary file", zname(Z));
	return nullptr;
}

} // end of namespace Grim
