#include "ltask.h"
#include "lmem.h"
#include "ldo.h"
#include "lvm.h"

int task_tag;

void start_script (void) {
  struct lua_Task *old_task = L->curr_task, *new_task;
  TObject *f;
  int i;

  f = L->stack.stack + L->Cstack.lua2C;
  if (ttype(f) == LUA_T_CLOSURE)
    f = &clvalue(f)->consts[0];
  if (ttype(f) != LUA_T_PROTO)
    lua_error("can only start_script with a Lua function");

  /* Create a new task with an empty stack */
  new_task = luaI_newtask();

  /* Put the function and arguments onto the new task's stack */
  for (i = 0; i < old_task->Cstack.num; i++) {
    *(L->stack.top) = *(old_task->stack.stack + old_task->Cstack.lua2C + i);
    incr_top;
  }

  /* Create a CallInfo frame */
  luaD_precall(L->stack.stack, 1, MULT_RET);
  ttype(L->stack.stack) = (ttype(L->stack.stack) == LUA_T_CLOSURE) ?
    LUA_T_CLMARK : LUA_T_PMARK;

  /* Switch back to the old task */
  L->Tstate = YIELD;
  luaI_switchtask(old_task);
  L->curr_task = old_task;

  /* Insert new task at end of list */
  L->last_task->next = new_task;
  L->last_task = new_task;

  /* Return task handle */
  lua_pushusertag(new_task, task_tag);
}

void stop_script (void) {
  struct lua_Task *prev, *t;
  TObject *f = L->stack.stack + L->Cstack.lua2C;
  int match;

  if (ttype(f) != LUA_T_CLOSURE && ttype(f) != LUA_T_PROTO &&
      ! (ttype(f) == LUA_T_USERDATA && f->value.ts->u.d.tag == task_tag))
    lua_error("Bad argument to stop_script");
  prev = L->root_task;
  while ((t = prev->next) != NULL) {
    switch (ttype(f)) {
    case LUA_T_CLOSURE:
      match = (ttype(t->stack.stack) == LUA_T_CLMARK &&
	       clvalue(t->stack.stack) == clvalue(f));
      break;
    case LUA_T_PROTO:
      match = (ttype(t->stack.stack) == LUA_T_PMARK &&
	       tfvalue(t->stack.stack) == tfvalue(f));
      break;
    case LUA_T_USERDATA:
      match = (t == f->value.ts->u.d.v);
      break;
    default:  /* Shut up gcc */
      break;
    }
    if (match) {
      prev->next = t->next;  /* Remove from list of active tasks */
      t->next = NULL;
      if (prev->next == NULL)
	L->last_task = prev;
      if (t == L->curr_task)
	L->Tstate = DONE;
      else {
	t->Tstate = DONE;
	if (t->auto_delete)
	  luaM_free(t);
      }
    }
    else
      prev = t;
  }
}

void next_script (void) {
  struct lua_Task *t;

  if (lua_isnil(lua_getparam(1)))
    t = L->root_task;
  else if (lua_isuserdata(lua_getparam(1)) &&
	   lua_tag(lua_getparam(1)) == task_tag)
    t = (struct lua_Task *) lua_getuserdata(lua_getparam(1));
  else
    lua_error("Bad argument to next_script");
  t = t->next;
  if (t == NULL)
    lua_pushnil();
  else {
    t->auto_delete = 0;
    lua_pushusertag(t, task_tag);
  }
}

void identify_script (void) {
  struct lua_Task *t;

  if (! lua_isuserdata(lua_getparam(1)) ||
      lua_tag(lua_getparam(1)) != task_tag)
    lua_error("Bad argument to identify_script");
  t = (struct lua_Task *) lua_getuserdata(lua_getparam(1));
  if (t->Tstate == DONE)
    ttype(L->stack.top) = LUA_T_NIL;
  else
    *L->stack.top = *t->stack.stack;
  incr_top;
}

void find_script (void) {
  struct lua_Task *t;
  TObject *f = L->stack.stack + L->Cstack.lua2C;

  switch (ttype(f)) {
  case LUA_T_CLOSURE:
    for (t = L->root_task->next; t != NULL; t = t->next)
      if (ttype(t->stack.stack) == LUA_T_CLOSURE &&
	  clvalue(t->stack.stack) == clvalue(f))
	break;
    break;
  case LUA_T_PROTO:
    for (t = L->root_task->next; t != NULL; t = t->next)
      if (ttype(t->stack.stack) == LUA_T_PROTO &&
	  tfvalue(t->stack.stack) == tfvalue(f))
	break;
    break;
  default:
    lua_error("Bad argument to find_script");
  }
  if (t == NULL)
    lua_pushnil();
  else {
    t->auto_delete = 0;
    lua_pushusertag(t, task_tag);
  }
}

void break_here (void) {
  struct CallInfo *ci;

  if (L->curr_task == L->root_task)
    lua_error("Cannot break in root thread");
  /* Check for any C functions in the call stack */
  for (ci = L->ci-1; ci > L->base_ci; ci--)
    if (ci->tf == NULL)
      lua_error("Cannot yield through C function");

  L->Tstate = YIELD;
}

void lua_runtasks (void) {
  struct lua_Task *t, *prev;
  struct lua_Task *old_task = L->curr_task;
  jmp_buf myErrorJmp;

  prev = L->root_task;
  while ((t = prev->next) != NULL) {
    luaI_switchtask(t);
    L->errorJmp = &myErrorJmp;
    L->Tstate = RUN;
    if (setjmp(myErrorJmp) == 0) {
      luaV_execute(L->base_ci+1);
      if (L->Tstate == RUN)	/* Must have run to completion */
	L->Tstate = DONE;
    }
    else /* an error occurred */
      L->Tstate = DONE;
    L->errorJmp = NULL;
    if (L->Tstate == DONE) {	/* Remove from list of active tasks */
      luaI_switchtask(old_task);
      prev->next = t->next;
      t->next = NULL;
      if (prev->next == NULL)
	L->last_task = prev;
      if (t->auto_delete)
	luaM_free(t);
    }
    else
      prev = t;
  }
  if (L->curr_task != old_task)
    luaI_switchtask(old_task);
}

void gc_task (void) {
  struct lua_Task *t = (struct lua_Task *) lua_getuserdata(lua_getparam(1));

  t->auto_delete = 1;
  if (t != L->curr_task && t->Tstate == DONE)
    luaM_free(t);
}
