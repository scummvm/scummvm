/*
** $Id$
** Global State
** See Copyright Notice in lua.h
*/

#ifndef lstate_h
#define lstate_h

#include <setjmp.h>

#include "lobject.h"
#include "lua.h"


#define MAX_C_BLOCKS 10

#define GARBAGE_BLOCK 150


typedef int StkId;  /* index to stack elements */

struct Stack {
  TObject *top;
  TObject *stack;
  TObject *last;
};

struct C_Lua_Stack {
  StkId base;  /* when Lua calls C or C calls Lua, points to */
               /* the first slot after the last parameter. */
  StkId lua2C; /* points to first element of "array" lua2C */
  int num;     /* size of "array" lua2C */
};


typedef struct {
  int size;
  int nuse;  /* number of elements (including EMPTYs) */
  TaggedString **hash;
} stringtable;


enum Status {LOCK, HOLD, FREE, COLLECTED};

struct ref {
  TObject o;
  enum Status status;
};


struct CallInfo {
  Closure *c;
  TProtoFunc *tf;  /* Set to NULL for C function */
  StkId base;
  Byte *pc;
  int nResults;
};


enum TaskState {RUN, YIELD, DONE};

struct lua_Task {
  struct Stack stack;
  struct C_Lua_Stack Cstack;
  jmp_buf *errorJmp;
  struct CallInfo *ci;
  struct CallInfo *base_ci;
  struct CallInfo *end_ci;
  char *Mbuffer;
  char *Mbuffbase;
  int Mbuffsize;
  int Mbuffnext;
  struct C_Lua_Stack Cblocks[MAX_C_BLOCKS];
  int numCblocks;
  enum TaskState Tstate;
  struct lua_Task *next;
  int auto_delete;  /* Set to 1 if no userdata references this task */
};

struct lua_State {
  /* thread-specific state */
  struct Stack stack;  /* Lua stack */
  struct C_Lua_Stack Cstack;  /* C2lua struct */
  jmp_buf *errorJmp;  /* current error recover point */
  struct CallInfo *ci;  /* call info for current function */
  struct CallInfo *base_ci;  /* array of CallInfo's */
  struct CallInfo *end_ci;  /* points after end of ci array */
  char *Mbuffer;  /* global buffer */
  char *Mbuffbase;  /* current first position of Mbuffer */
  int Mbuffsize;  /* size of Mbuffer */
  int Mbuffnext;  /* next position to fill in Mbuffer */
  struct C_Lua_Stack Cblocks[MAX_C_BLOCKS];
  int numCblocks;  /* number of nested Cblocks */
  enum TaskState Tstate;  /* state of current thread */
  /* global state */
  struct lua_Task *root_task;  /* first task created */
  struct lua_Task *curr_task;
  struct lua_Task *last_task;
  TObject errorim;  /* error tag method */
  GCnode rootproto;  /* list of all prototypes */
  GCnode rootcl;  /* list of all closures */
  GCnode roottable;  /* list of all tables */
  GCnode rootglobal;  /* list of strings with global values */
  stringtable *string_root;  /* array of hash tables for strings and udata */
  struct IM *IMtable;  /* table for tag methods */
  int IMtable_size;  /* size of IMtable */
  int last_tag;  /* last used tag in IMtable */
  struct ref *refArray;  /* locked objects */
  int refSize;  /* size of refArray */
  unsigned long GCthreshold;
  unsigned long nblocks;  /* number of 'blocks' currently allocated */
};


extern lua_State *lua_state;


#define L	lua_state


/* Switch to the given task */
void luaI_switchtask(struct lua_Task *t);

/* Create a new task and switch to it */
struct lua_Task *luaI_newtask(void);

#endif
