/*
** $Id$
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

// ScummVM changes to the stock loader:
//
// The stock loader only accepts chunks made on a machine like the one it runs
// on: it compares the header of a chunk with one built for the host, and then
// reads integers, sizes, numbers and instructions as raw memory. Games ship
// chunks compiled for the machine they were released on (32-bit, little
// endian), which have to load on every platform ScummVM runs on.
//
// So LoadHeader() takes the byte order and the sizes of int and size_t from the
// header of the chunk, and everything is read one value at a time through
// LoadUnsigned(), which puts the bytes together in that order. Instructions
// have to be 4 bytes, and numbers 8-byte IEEE doubles.
//
// Chunks can only be loaded; ldump.c is not part of ScummVM, so luaU_dump()
// has no implementation and lua_dump() fails. luaU_header(), which made the
// header of the host for the comparison and for ldump.c, is gone as well.

#include <string.h>

#define lundump_c
#define LUA_CORE

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstring.h"
#include "lundump.h"
#include "lzio.h"

typedef struct {
 lua_State* L;
 ZIO* Z;
 Mbuffer* b;
 const char* name;
 int littleEndian;				/* ScummVM: the format of the chunk */
 int sizeInt;
 int sizeSizeT;
} LoadState;

#ifdef LUAC_TRUST_BINARIES
#define IF(c,s)
#define error(S,s)
#else
#define IF(c,s)		if (c) error(S,s)

static void error(LoadState* S, const char* why)
{
 luaO_pushfstring(S->L,"%s: %s in precompiled chunk",S->name,why);
 luaD_throw(S->L,LUA_ERRSYNTAX);
}
#endif

#define LoadMem(S,b,n,size)	LoadBlock(S,b,(n)*(size))
#define	LoadByte(S)		(lu_byte)LoadChar(S)
#define LoadVar(S,x)		LoadMem(S,&x,1,sizeof(x))
#define LoadVector(S,b,n,size)	LoadMem(S,b,n,size)

static void LoadBlock(LoadState* S, void* b, size_t size)
{
 size_t r=luaZ_read(S->Z,b,size);
 IF (r!=0, "unexpected end");
}

static int LoadChar(LoadState* S)
{
 char x;
 LoadVar(S,x);
 return x;
}

/* ScummVM: reads an unsigned integer of the given size, in the byte order of the chunk */
static uint64 LoadUnsigned(LoadState* S, int size)
{
 lu_byte b[8];
 uint64 x=0;
 int i;
 LoadBlock(S,b,size);
 for (i=0; i<size; i++)
  x|=((uint64)b[i])<<(8*(S->littleEndian ? i : size-1-i));
 return x;
}

static int LoadInt(LoadState* S)
{
 uint64 x=LoadUnsigned(S,S->sizeInt);
 IF (x>(uint64)MAX_INT, "bad integer");
 return (int)x;
}

static lua_Number LoadNumber(LoadState* S)
{
 uint64 bits=LoadUnsigned(S,8);
 double x;
 memcpy(&x,&bits,sizeof(x));
 return (lua_Number)x;
}

static TString* LoadString(LoadState* S)
{
 uint64 size64=LoadUnsigned(S,S->sizeSizeT);
 size_t size=(size_t)size64;
 IF (size64>(uint64)MAX_SIZET, "bad size");
 if (size==0)
  return NULL;
 else
 {
  char* s=luaZ_openspace(S->L,S->b,size);
  LoadBlock(S,s,size);
  return luaS_newlstr(S->L,s,size-1);		/* remove trailing '\0' */
 }
}

static void LoadCode(LoadState* S, Proto* f)
{
 int i;
 int n=LoadInt(S);
 f->code=luaM_newvector(S->L,n,Instruction);
 f->sizecode=n;
 for (i=0; i<n; i++) f->code[i]=(Instruction)LoadUnsigned(S,4);
}

static Proto* LoadFunction(LoadState* S, TString* p);

static void LoadConstants(LoadState* S, Proto* f)
{
 int i,n;
 n=LoadInt(S);
 f->k=luaM_newvector(S->L,n,TValue);
 f->sizek=n;
 for (i=0; i<n; i++) setnilvalue(&f->k[i]);
 for (i=0; i<n; i++)
 {
  TValue* o=&f->k[i];
  int t=LoadChar(S);
  switch (t)
  {
   case LUA_TNIL:
    setnilvalue(o);
    break;
   case LUA_TBOOLEAN:
    setbvalue(o,LoadChar(S)!=0);
    break;
   case LUA_TNUMBER:
    setnvalue(o,LoadNumber(S));
    break;
   case LUA_TSTRING:
    setsvalue2n(S->L,o,LoadString(S));
    break;
   default:
    error(S,"bad constant");
    break;
  }
 }
 n=LoadInt(S);
 f->p=luaM_newvector(S->L,n,Proto*);
 f->sizep=n;
 for (i=0; i<n; i++) f->p[i]=NULL;
 for (i=0; i<n; i++) f->p[i]=LoadFunction(S,f->source);
}

static void LoadDebug(LoadState* S, Proto* f)
{
 int i,n;
 n=LoadInt(S);
 f->lineinfo=luaM_newvector(S->L,n,int);
 f->sizelineinfo=n;
 for (i=0; i<n; i++) f->lineinfo[i]=LoadInt(S);
 n=LoadInt(S);
 f->locvars=luaM_newvector(S->L,n,LocVar);
 f->sizelocvars=n;
 for (i=0; i<n; i++) f->locvars[i].varname=NULL;
 for (i=0; i<n; i++)
 {
  f->locvars[i].varname=LoadString(S);
  f->locvars[i].startpc=LoadInt(S);
  f->locvars[i].endpc=LoadInt(S);
 }
 n=LoadInt(S);
 f->upvalues=luaM_newvector(S->L,n,TString*);
 f->sizeupvalues=n;
 for (i=0; i<n; i++) f->upvalues[i]=NULL;
 for (i=0; i<n; i++) f->upvalues[i]=LoadString(S);
}

static Proto* LoadFunction(LoadState* S, TString* p)
{
 Proto* f;
 if (++S->L->nCcalls > LUAI_MAXCCALLS) error(S,"code too deep");
 f=luaF_newproto(S->L);
 setptvalue2s(S->L,S->L->top,f); incr_top(S->L);
 f->source=LoadString(S); if (f->source==NULL) f->source=p;
 f->linedefined=LoadInt(S);
 f->lastlinedefined=LoadInt(S);
 f->nups=LoadByte(S);
 f->numparams=LoadByte(S);
 f->is_vararg=LoadByte(S);
 f->maxstacksize=LoadByte(S);
 LoadCode(S,f);
 LoadConstants(S,f);
 LoadDebug(S,f);
 IF (!luaG_checkcode(f), "bad code");
 S->L->top--;
 S->L->nCcalls--;
 return f;
}

static void LoadHeader(LoadState* S)
{
 /* ScummVM: the chunk says how it was written, instead of having to match the host */
 lu_byte s[LUAC_HEADERSIZE];
 LoadBlock(S,s,LUAC_HEADERSIZE);
 IF (memcmp(s,LUA_SIGNATURE,sizeof(LUA_SIGNATURE)-1)!=0, "bad header");
 IF (s[4]!=LUAC_VERSION || s[5]!=LUAC_FORMAT, "bad header");
 S->littleEndian=s[6];
 S->sizeInt=s[7];
 S->sizeSizeT=s[8];
 IF (S->sizeInt!=4, "unsupported integer size");
 IF (S->sizeSizeT!=4 && S->sizeSizeT!=8, "unsupported size_t size");
 IF (s[9]!=4, "unsupported instruction size");
 IF (s[10]!=8 || s[11]!=0 || sizeof(lua_Number)!=8, "unsupported number format");
}

/*
** load precompiled chunk
*/
Proto* luaU_undump (lua_State* L, ZIO* Z, Mbuffer* buff, const char* name)
{
 LoadState S;
 if (*name=='@' || *name=='=')
  S.name=name+1;
 else if (*name==LUA_SIGNATURE[0])
  S.name="binary string";
 else
  S.name=name;
 S.L=L;
 S.Z=Z;
 S.b=buff;
 LoadHeader(&S);
 return LoadFunction(&S,luaS_newliteral(L,"=?"));
}
