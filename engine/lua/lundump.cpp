/*
** $Id$
** load bytecodes from files
** See Copyright Notice in lua.h
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lauxlib.h"
#include "lfunc.h"
#include "lmem.h"
#include "lstring.h"
#include "lundump.h"

#define	LoadBlock(b,size,Z)	ezread(Z,b,size)
#define	LoadNative(t,Z)		LoadBlock(&t,sizeof(t),Z)

#define doLoadNumber(f,Z)	f=LoadNumber(Z)


static float conv_float(const char *data) {
        const unsigned char *udata = (const unsigned char *)(data);
        unsigned char fdata[4];
        fdata[0] = udata[3];
        fdata[1] = udata[2];
        fdata[2] = udata[1];
        fdata[3] = udata[0];
        return *(const float *)(fdata);
}

static void unexpectedEOZ(ZIO* Z)
{
 luaL_verror("unexpected end of file in %s",zname(Z));
}

static int ezgetc(ZIO* Z)
{
 int c=zgetc(Z);
 if (c==EOZ) unexpectedEOZ(Z);
 return c;
}

static void ezread(ZIO* Z, void* b, int n)
{
 int r=zread(Z,b,n);
 if (r!=0) unexpectedEOZ(Z);
}

static unsigned int LoadWord(ZIO* Z)
{
 unsigned int hi=ezgetc(Z);
 unsigned int lo=ezgetc(Z);
 return (hi<<8)|lo;
}

static unsigned long LoadLong(ZIO* Z)
{
 unsigned long hi=LoadWord(Z);
 unsigned long lo=LoadWord(Z);
 return (hi<<16)|lo;
}

static float LoadFloat(ZIO* Z)
{
 unsigned long l=LoadLong(Z);
 return conv_float((const char *)&l);
}

static Byte* LoadCode(ZIO* Z)
{
 unsigned long size=LoadLong(Z);
 unsigned int s=size;
 void* b;
 if (s!=size) luaL_verror("code too long (%ld bytes) in %s",size,zname(Z));
 b=luaM_malloc(size);
 LoadBlock(b,size,Z);
 return (Byte *)b;
}

static TaggedString* LoadTString(ZIO* Z)
{
 int size=LoadWord(Z);
 int i;
 if (size==0)
  return NULL;
 else
 {
  char* s=luaL_openspace(size);
  LoadBlock(s,size,Z);
  for (i=0; i<size; i++)
   s[i]^=0xff;
  return luaS_newlstr(s,size-1);
 }
}

static void LoadLocals(TProtoFunc* tf, ZIO* Z)
{
 int i,n=LoadWord(Z);
 if (n==0) return;
 tf->locvars=luaM_newvector(n+1,LocVar);
 for (i=0; i<n; i++)
 {
  tf->locvars[i].line=LoadWord(Z);
  tf->locvars[i].varname=LoadTString(Z);
 }
 tf->locvars[i].line=-1;		/* flag end of vector */
 tf->locvars[i].varname=NULL;
}

static TProtoFunc* LoadFunction(ZIO* Z);

static void LoadConstants(TProtoFunc* tf, ZIO* Z)
{
 int i,n=LoadWord(Z);
 tf->nconsts=n;
 if (n==0) return;
 tf->consts=luaM_newvector(n,TObject);
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  ttype(o)=(lua_Type)-ezgetc(Z);
  switch ((unsigned)ttype(o))
  {
   case -'N':
        ttype(o)=LUA_T_NUMBER;
   case LUA_T_NUMBER:
	doLoadNumber(nvalue(o),Z);
	break;
   case -'S':
	ttype(o)=LUA_T_STRING;
   case LUA_T_STRING:
	tsvalue(o)=LoadTString(Z);
	break;
   case -'F':
	ttype(o)=LUA_T_PROTO;
   case LUA_T_PROTO:
	break;
   case LUA_T_NIL:
	break;
   default:
	luaL_verror("bad constant #%d in %s: type=%d [%s]",
		i,zname(Z),ttype(o),luaO_typename(o));
	break;
  }
 }
}

static void LoadSubfunctions(TProtoFunc* tf, ZIO* Z) {
  char t;
  do {
    t = ezgetc(Z);
    switch (t) {
    case '#': {
      int i = LoadWord(Z);
      if (ttype(tf->consts+i) != LUA_T_PROTO)
	luaL_verror("trying to load function into nonfunction constant (type=%d)",
		    ttype(tf->consts+i));
      tfvalue(tf->consts+i) = LoadFunction(Z);
      break;
    }
    case '$':
      break;
    default:
      luaL_verror("invalid subfunction type %c",t);
    }
  } while (t != '$');
}

static TProtoFunc* LoadFunction(ZIO* Z)
{
 TProtoFunc* tf=luaF_newproto();
 tf->lineDefined=LoadWord(Z);
 tf->fileName=LoadTString(Z);
 tf->code=LoadCode(Z);
 LoadConstants(tf,Z);
 LoadLocals(tf,Z);
 LoadSubfunctions(tf,Z);
 return tf;
}

static void LoadSignature(ZIO* Z)
{
 char* s=SIGNATURE;
 while (*s!=0 && ezgetc(Z)==*s)
  ++s;
 if (*s!=0) luaL_verror("bad signature in %s",zname(Z));
}

static void LoadHeader(ZIO* Z)
{
 int version,id,sizeofR;
#if 0
 real f=(real)-TEST_NUMBER,tf=(real)TEST_NUMBER;
#endif
 LoadSignature(Z);
 version=ezgetc(Z);
 if (version>VERSION)
  luaL_verror(
	"%s too new: version=0x%02x; expected at most 0x%02x",
	zname(Z),version,VERSION);
 if (version<VERSION0)			/* check last major change */
  luaL_verror(
	"%s too old: version=0x%02x; expected at least 0x%02x",
	zname(Z),version,VERSION0);
 sizeofR=ezgetc(Z);			/* test number representation */
 id=ezgetc(Z);
 if (id!=ID_NUMBER || sizeofR!=sizeof(real))
 {
  luaL_verror("unknown number signature in %s: "
	"read 0x%02x%02x; expected 0x%02x%02x",
	zname(Z),id,sizeofR,ID_NUMBER,sizeof(real));
 }
#if 0
 doLoadNumber(f,Z);
 if (f!=tf)
  luaL_verror("unknown number representation in %s: "
	"read " NUMBER_FMT "; expected " NUMBER_FMT,	/* LUA_NUMBER */
	zname(Z),f,tf);
#endif
  ezgetc(Z); ezgetc(Z); ezgetc(Z);
}

static TProtoFunc* LoadChunk(ZIO* Z)
{
 LoadHeader(Z);
 return LoadFunction(Z);
}

/*
** load one chunk from a file or buffer
** return main if ok and NULL at EOF
*/
TProtoFunc* luaU_undump1(ZIO* Z)
{
 int c=zgetc(Z);
 if (c==ID_CHUNK)
  return LoadChunk(Z);
 else if (c!=EOZ)
  luaL_verror("%s is not a Lua binary file",zname(Z));
 return NULL;
}
