/*
** $Id$
** Global State
** See Copyright Notice in lua.h
*/


#include "lbuiltin.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "llex.h"
#include "lmem.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"


lua_State *lua_state = NULL;

static void lua_openthr (void)
{
  L->numCblocks = 0;
  L->Cstack.base = 0;
  L->Cstack.lua2C = 0;
  L->Cstack.num = 0;
  L->errorJmp = NULL;
  L->Mbuffsize = 0;
  L->Mbuffnext = 0;
  L->Mbuffbase = NULL;
  L->Mbuffer = NULL;
  L->Tstate = RUN;
}

void lua_open (void)
{
  if (lua_state) return;
  lua_state = luaM_new(lua_State);
  lua_openthr();
  L->rootproto.next = NULL;
  L->rootproto.marked = 0;
  L->rootcl.next = NULL;
  L->rootcl.marked = 0;
  L->rootglobal.next = NULL;
  L->rootglobal.marked = 0;
  L->roottable.next = NULL;
  L->roottable.marked = 0;
  L->refArray = NULL;
  L->refSize = 0;
  L->GCthreshold = GARBAGE_BLOCK;
  L->nblocks = 0;
  L->root_task = luaM_new(struct lua_Task);
  L->root_task->next = NULL;
  L->last_task = L->root_task;
  L->curr_task = L->root_task;
  luaD_init();
  luaD_initthr();
  luaS_init();
  luaX_init();
  luaT_init();
  luaB_predefine();
}


void lua_close (void)
{
  TaggedString *alludata = luaS_collectudata();
  L->GCthreshold = MAX_INT;  /* to avoid GC during GC */
  luaC_hashcallIM((Hash *)L->roottable.next);  /* GC t.methods for tables */
  luaC_strcallIM(alludata);  /* GC tag methods for userdata */
  luaD_gcIM(&luaO_nilobject);  /* GC tag method for nil (signal end of GC) */
  luaH_free((Hash *)L->roottable.next);
  luaF_freeproto((TProtoFunc *)L->rootproto.next);
  luaF_freeclosure((Closure *)L->rootcl.next);
  luaS_free(alludata);
  luaS_freeall();
  luaM_free(L->stack.stack);
  luaM_free(L->IMtable);
  luaM_free(L->refArray);
  luaM_free(L->Mbuffer);
  luaM_free(L);
  L = NULL;
#ifdef DEBUG
  printf("total de blocos: %ld\n", numblocks);
  printf("total de memoria: %ld\n", totalmem);
#endif
}


lua_State *lua_setstate (lua_State *st) {
  lua_State *old = lua_state;
  lua_state = st;
  return old;
}


static void savetask (struct lua_Task *t) {
  t->stack = L->stack;
  t->Cstack = L->Cstack;
  t->errorJmp = L->errorJmp;
  t->ci = L->ci;
  t->base_ci = L->base_ci;
  t->end_ci = L->end_ci;
  t->Mbuffer = L->Mbuffer;
  t->Mbuffbase = L->Mbuffbase;
  t->Mbuffsize = L->Mbuffsize;
  t->Mbuffnext = L->Mbuffnext;
  memcpy(t->Cblocks, L->Cblocks, sizeof(L->Cblocks));;
  t->numCblocks = L->numCblocks;
  t->Tstate = L->Tstate;
}

static void loadtask (struct lua_Task *t) {
  L->stack = t->stack;
  L->Cstack = t->Cstack;
  L->errorJmp = t->errorJmp;
  L->ci = t->ci;
  L->base_ci = t->base_ci;
  L->end_ci = t->end_ci;
  L->Mbuffer = t->Mbuffer;
  L->Mbuffbase = t->Mbuffbase;
  L->Mbuffsize = t->Mbuffsize;
  L->Mbuffnext = t->Mbuffnext;
  memcpy(L->Cblocks, t->Cblocks, sizeof(L->Cblocks));
  L->numCblocks = t->numCblocks;
  L->Tstate = t->Tstate;
}

void luaI_switchtask(struct lua_Task *t) {
  savetask(L->curr_task);
  L->curr_task = t;
  loadtask(t);
}

struct lua_Task *luaI_newtask (void) {
  struct lua_Task *result;

  savetask(L->curr_task);
  result = luaM_new(struct lua_Task);
  L->curr_task = result;
  result->next = NULL;
  result->auto_delete = 0;
  lua_openthr();
  luaD_initthr();
  return result;
}
