/*
** $Id$
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/


#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"
#include "lstate.h"
#include "ltm.h"
#include "lua.h"
#include "luadebug.h"
#include "lundump.h"
#include "lvm.h"
#include "lzio.h"



#ifndef STACK_LIMIT
#define STACK_LIMIT     6000
#endif



/* Extra stack size to run a function: LUA_T_LINE(1), TM calls(2), ... */
#define	EXTRA_STACK	5


/*
** Error messages
*/

static void stderrorim (void)
{
  fprintf(stderr, "lua error: %s\n", lua_getstring(lua_getparam(1)));
}



#define STACK_UNIT	128

/* Initial size for CallInfo array */
#define BASIC_CI_SIZE	8

void luaD_init (void)
{
  ttype(&L->errorim) = LUA_T_CPROTO;
  fvalue(&L->errorim) = stderrorim;
}

void luaD_initthr (void)
{
  L->stack.stack = luaM_newvector(STACK_UNIT, TObject);
  L->stack.top = L->stack.stack;
  L->stack.last = L->stack.stack+(STACK_UNIT-1);
  L->base_ci = luaM_newvector(BASIC_CI_SIZE, struct CallInfo);
  L->ci = L->base_ci;
  L->ci->tf = NULL;
  L->end_ci = L->base_ci + BASIC_CI_SIZE;
}


void luaD_checkstack (int n)
{
  struct Stack *S = &L->stack;
  if (S->last-S->top <= n) {
    StkId top = S->top-S->stack;
    int stacksize = (S->last-S->stack)+1+STACK_UNIT+n;
    S->stack = luaM_reallocvector(S->stack, stacksize, TObject);
    S->last = S->stack+(stacksize-1);
    S->top = S->stack + top;
    if (stacksize >= STACK_LIMIT) {  /* stack overflow? */
      if (lua_stackedfunction(100) == LUA_NOOBJECT)  /* 100 funcs on stack? */
        lua_error("Lua2C - C2Lua overflow"); /* doesn't look like a rec. loop */
      else
        lua_error("stack size overflow");
    }
  }
}


/*
** Adjust stack. Set top to the given value, pushing NILs if needed.
*/
void luaD_adjusttop (StkId newtop)
{
  int diff = newtop-(L->stack.top-L->stack.stack);
  if (diff <= 0)
    L->stack.top += diff;
  else {
    luaD_checkstack(diff);
    while (diff--)
      ttype(L->stack.top++) = LUA_T_NIL;
  }
}


/*
** Open a hole below "nelems" from the L->stack.top.
*/
void luaD_openstack (int nelems)
{
  luaO_memup(L->stack.top-nelems+1, L->stack.top-nelems,
             nelems*sizeof(TObject));
  incr_top;
}


void luaD_lineHook (int line)
{
  struct C_Lua_Stack oldCLS = L->Cstack;
  StkId old_top = L->Cstack.lua2C = L->Cstack.base = L->stack.top-L->stack.stack;
  L->Cstack.num = 0;
  (*lua_linehook)(line);
  L->stack.top = L->stack.stack+old_top;
  L->Cstack = oldCLS;
}


void luaD_callHook (StkId base, TProtoFunc *tf, int isreturn)
{
  struct C_Lua_Stack oldCLS = L->Cstack;
  StkId old_top = L->Cstack.lua2C = L->Cstack.base = L->stack.top-L->stack.stack;
  L->Cstack.num = 0;
  if (isreturn)
    (*lua_callhook)(LUA_NOOBJECT, "(return)", 0);
  else {
    TObject *f = L->stack.stack+base-1;
    if (tf)
      (*lua_callhook)(Ref(f), tf->fileName->str, tf->lineDefined);
    else
      (*lua_callhook)(Ref(f), "(C)", -1);
  }
  L->stack.top = L->stack.stack+old_top;
  L->Cstack = oldCLS;
}


/*
** Call a C function.
** Cstack.num is the number of arguments; Cstack.lua2C points to the
** first argument. Returns an index to the first result from C.
*/
static StkId callC (lua_CFunction f, StkId base)
{
  struct C_Lua_Stack *CS = &L->Cstack;
  struct C_Lua_Stack oldCLS = *CS;
  StkId firstResult;
  int numarg = (L->stack.top-L->stack.stack) - base;
  CS->num = numarg;
  CS->lua2C = base;
  CS->base = base+numarg;  /* == top-stack */
  if (lua_callhook)
    luaD_callHook(base, NULL, 0);
  (*f)();  /* do the actual call */
  if (lua_callhook)  /* func may have changed lua_callhook */
    luaD_callHook(base, NULL, 1);
  firstResult = CS->base;
  *CS = oldCLS;
  return firstResult;
}


static StkId callCclosure (struct Closure *cl, lua_CFunction f, StkId base)
{
  TObject *pbase;
  int nup = cl->nelems;  /* number of upvalues */
  luaD_checkstack(nup);
  pbase = L->stack.stack+base;  /* care: previous call may change this */
  /* open space for upvalues as extra arguments */
  luaO_memup(pbase+nup, pbase, (L->stack.top-pbase)*sizeof(TObject));
  /* copy upvalues into stack */
  memcpy(pbase, cl->consts+1, nup*sizeof(TObject));
  L->stack.top += nup;
  return callC(f, base);
}


void luaD_callTM (TObject *f, int nParams, int nResults)
{
  luaD_openstack(nParams);
  *(L->stack.top-nParams-1) = *f;
  luaD_call((L->stack.top-L->stack.stack)-nParams, nResults);
}

static void adjust_varargs (StkId first_extra_arg)
{
  TObject arg;
  luaV_pack(first_extra_arg,
       (L->stack.top-L->stack.stack)-first_extra_arg, &arg);
  luaD_adjusttop(first_extra_arg);
  *L->stack.top++ = arg;
}



/*
** Prepare the stack for calling a Lua function.
*/
void luaD_precall (TObject *f, StkId base, int nResults)
{
  /* Create a new CallInfo record */
  if (L->ci+1 == L->end_ci) {
    int size_ci = L->end_ci - L->base_ci;
    int index_ci = L->ci - L->base_ci;
    L->base_ci = luaM_reallocvector(L->base_ci, size_ci * 2,
				    struct CallInfo);
    L->ci = L->base_ci + index_ci;
    L->end_ci = L->base_ci + size_ci * 2;
  }
  L->ci++;
  if (ttype(f) == LUA_T_CLOSURE) {
    L->ci->c = clvalue(f);
    f = &L->ci->c->consts[0];  /* Get the actual function */
  }
  else
    L->ci->c = NULL;
  L->ci->base = base;
  L->ci->nResults = nResults;
  if (ttype(f)==LUA_T_CPROTO) {
    L->ci->tf = NULL;
    L->ci->pc = NULL;
  }
  else {
    Byte *pc = tfvalue(f)->code;
    if (lua_callhook)
      luaD_callHook(base, tfvalue(f), 0);
    luaD_checkstack((*pc++)+EXTRA_STACK);
    if (*pc < ZEROVARARG)
      luaD_adjusttop(base+*(pc++));
    else {  /* varargs */
      luaC_checkGC();
      adjust_varargs(base+(*pc++)-ZEROVARARG);
    }
    L->ci->tf = tfvalue(f);
    L->ci->pc = pc;
  }
}

/*
** Adjust the stack to the desired number of results
*/
void luaD_postret (StkId firstResult) {
  int i;
  StkId base = L->ci->base;
  int nResults = L->ci->nResults;
  if (L->ci == L->base_ci)
    lua_error("call stack underflow");
  /* adjust the number of results */
  if (nResults != MULT_RET)
    luaD_adjusttop(firstResult+nResults);
  /* move results to base-1 (to erase parameters and function) */
  base--;
  nResults = L->stack.top - (L->stack.stack+firstResult);  /* actual number of results */
  for (i=0; i<nResults; i++)
    *(L->stack.stack+base+i) = *(L->stack.stack+firstResult+i);
  L->stack.top -= firstResult-base;
  /* pop off the current CallInfo */
  L->ci--;
}

/*
** Call a function (C or Lua). The parameters must be on the L->stack.stack,
** between [L->stack.stack+base,L->stack.top). The function to be called is at L->stack.stack+base-1.
** When returns, the results are on the L->stack.stack, between [L->stack.stack+base-1,L->stack.top).
** The number of results is nResults, unless nResults=MULT_RET.
*/
void luaD_call (StkId base, int nResults)
{
  StkId firstResult;
  TObject *func = L->stack.stack+base-1;
  switch (ttype(func)) {
    case LUA_T_CPROTO:
      luaD_precall(func, base, nResults);
      ttype(func) = LUA_T_CMARK;
      firstResult = callC(fvalue(func), base);
      break;
    case LUA_T_PROTO:
      luaD_precall(func, base, nResults);
      ttype(func) = LUA_T_PMARK;
      firstResult = luaV_execute(L->ci);
      break;
    case LUA_T_CLOSURE: {
      Closure *c = clvalue(func);
      TObject *proto = &(c->consts[0]);
      luaD_precall(func, base, nResults);
      ttype(func) = LUA_T_CLMARK;
      firstResult = (ttype(proto) == LUA_T_CPROTO) ?
                       callCclosure(c, fvalue(proto), base) :
                       luaV_execute(L->ci);
      break;
    }
    default: { /* func is not a function */
      /* Check the tag method for invalid functions */
      TObject *im = luaT_getimbyObj(func, IM_FUNCTION);
      if (ttype(im) == LUA_T_NIL)
        lua_error("call expression not a function");
      luaD_callTM(im, (L->stack.top-L->stack.stack)-(base-1), nResults);
      return;
    }
  }
  luaD_postret(firstResult);
}


static void travstack (struct Stack *S, int (*fn)(TObject *)) {
  StkId i;
  for (i = (S->top-1)-S->stack; i>=0; i--)
    fn(S->stack+i);
}

/*
** Traverse all objects on L->stack.stack, and all other active stacks
*/
void luaD_travstack (int (*fn)(TObject *))
{
  struct lua_Task *t;
  travstack(&L->stack, fn);
  for (t = L->root_task; t != NULL; t = t->next)
    if (t != L->curr_task && t->Tstate != DONE)
      travstack(&t->stack, fn);
}



static void message (char *s)
{
  TObject im = L->errorim;
  if (ttype(&im) != LUA_T_NIL) {
    lua_pushstring(s);
    luaD_callTM(&im, 1, 0);
  }
}

/*
** Reports an error, and jumps up to the available recover label
*/
void lua_error (char *s)
{
  if (s) message(s);
  if (L->errorJmp)
    longjmp(*((jmp_buf *)L->errorJmp), 1);
  else {
    fprintf (stderr, "lua: exit(1). Unable to recover\n");
    exit(1);
  }
}

/*
** Call the function at L->Cstack.base, and incorporate results on
** the Lua2C structure.
*/
static void do_callinc (int nResults)
{
  StkId base = L->Cstack.base;
  luaD_call(base+1, nResults);
  L->Cstack.lua2C = base;  /* position of the luaM_new results */
  L->Cstack.num = (L->stack.top-L->stack.stack) - base;  /* number of results */
  L->Cstack.base = base + L->Cstack.num;  /* incorporate results on L->stack.stack */
}


/*
** Execute a protected call. Assumes that function is at L->Cstack.base and
** parameters are on top of it. Leave nResults on the stack.
*/
int luaD_protectedrun (int nResults)
{
  jmp_buf myErrorJmp;
  int status;
  volatile struct C_Lua_Stack oldCLS = L->Cstack;
  jmp_buf *volatile oldErr = L->errorJmp;
  volatile int ci_len = L->ci - L->base_ci;
  L->errorJmp = &myErrorJmp;
  if (setjmp(myErrorJmp) == 0) {
    do_callinc(nResults);
    status = 0;
  }
  else { /* an error occurred: restore L->Cstack and L->stack.top */
    L->Cstack = oldCLS;
    L->stack.top = L->stack.stack+L->Cstack.base;
    L->ci = L->base_ci + ci_len;
    status = 1;
  }
  L->errorJmp = oldErr;
  return status;
}


/*
** returns 0 = chunk loaded; 1 = error; 2 = no more chunks to load
*/
static int protectedparser (ZIO *z, int bin)
{
  volatile int status;
  TProtoFunc *volatile tf;
  jmp_buf myErrorJmp;
  jmp_buf *volatile oldErr = L->errorJmp;
  L->errorJmp = &myErrorJmp;
  if (setjmp(myErrorJmp) == 0) {
    tf = bin ? luaU_undump1(z) : luaY_parser(z);
    status = 0;
  }
  else {
    tf = NULL;
    status = 1;
  }
  L->errorJmp = oldErr;
  if (status) return 1;  /* error code */
  if (tf == NULL) return 2;  /* 'natural' end */
  luaD_adjusttop(L->Cstack.base+1);  /* one slot for the pseudo-function */
  L->stack.stack[L->Cstack.base].ttype = LUA_T_PROTO;
  L->stack.stack[L->Cstack.base].value.tf = tf;
  luaV_closure(0);
  return 0;
}


static int do_main (ZIO *z, int bin)
{
  int status;
  do {
    long old_blocks = (luaC_checkGC(), L->nblocks);
    status = protectedparser(z, bin);
    if (status == 1) return 1;  /* error */
    else if (status == 2) return 0;  /* 'natural' end */
    else {
      unsigned long newelems2 = 2*(L->nblocks-old_blocks);
      L->GCthreshold += newelems2;
      status = luaD_protectedrun(MULT_RET);
      L->GCthreshold -= newelems2;
    }
  } while (bin && status == 0);
  return status;
}


void luaD_gcIM (TObject *o)
{
  TObject *im = luaT_getimbyObj(o, IM_GC);
  if (ttype(im) != LUA_T_NIL) {
    *L->stack.top = *o;
    incr_top;
    luaD_callTM(im, 1, 0);
  }
}


int lua_dofile (char *filename)
{
  ZIO z;
  int status;
  int c;
  int bin;
  FILE *f = (filename == NULL) ? stdin : fopen(filename, "r");
  if (f == NULL)
    return 2;
  if (filename == NULL)
    filename = "(stdin)";
  c = fgetc(f);
  ungetc(c, f);
  bin = (c == ID_CHUNK);
  if (bin)
    f = freopen(filename, "rb", f);  /* set binary mode */
  luaZ_Fopen(&z, f, filename);
  status = do_main(&z, bin);
  if (f != stdin)
    fclose(f);
  return status;
}


#define SIZE_PREF 20  /* size of string prefix to appear in error messages */
#define SSIZE_PREF "20"


static void build_name (char *str, char *name) {
  if (str == NULL || *str == ID_CHUNK)
    strcpy(name, "(buffer)");
  else {
    char *temp;
    sprintf(name, "(dostring) >> \"%." SSIZE_PREF "s\"", str);
    temp = strchr(name, '\n');
    if (temp) {  /* end string after first line */
     *temp = '"';
     *(temp+1) = 0;
    }
  }
}


int lua_dostring (char *str) {
  return lua_dobuffer(str, strlen(str), NULL);
}


int lua_dobuffer (char *buff, int size, char *name) {
  char newname[SIZE_PREF+25];
  ZIO z;
  int status;
  if (name==NULL) {
    build_name(buff, newname);
    name = newname;
  }
  luaZ_mopen(&z, buff, size, name);
  status = do_main(&z, buff[0]==ID_CHUNK);
  return status;
}

