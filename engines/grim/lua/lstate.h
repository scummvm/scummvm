/*
** $Id$
** Global State
** See Copyright Notice in lua.h
*/

#ifndef GRIM_LSTATE_H
#define GRIM_LSTATE_H

#include "engines/grim/lua/lobject.h"

#include <setjmp.h>

namespace Grim {

#define MAX_C_BLOCKS 10
#define GARBAGE_BLOCK 150

typedef int32 StkId;  /* index to stack elements */

struct Stack {
	TObject *top;
	TObject *stack;
	TObject *last;
};

struct C_Lua_Stack {
	StkId base;  // when Lua calls C or C calls Lua, points to
	// the first slot after the last parameter.
	StkId lua2C; // points to first element of "array" lua2C
	int32 num;     // size of "array" lua2C
};


typedef struct {
	int32 size;
	int32 nuse;  // number of elements (including EMPTYs)
	TaggedString **hash;
} stringtable;

enum Status { LOCK, HOLD, FREE, COLLECTED };

struct ref {
	TObject o;
	enum Status status;
};

struct CallInfo {
	Closure *c;
	TProtoFunc *tf;  // Set to NULL for C function
	StkId base;
	byte *pc;
	int32 nResults;
};

enum TaskState { RUN, YIELD, PAUSE, DONE };

struct lua_Task {
	Stack stack;
	C_Lua_Stack Cstack;
	jmp_buf *errorJmp;
	CallInfo *ci;
	CallInfo *base_ci;
	int32 base_ci_size;
	CallInfo *end_ci;
	char *Mbuffer;
	char *Mbuffbase;
	int32 Mbuffsize;
	int32 Mbuffnext;
	C_Lua_Stack Cblocks[MAX_C_BLOCKS];
	int32 numCblocks;
	enum TaskState Tstate;
	struct lua_Task *next;
	int32 id;
};

struct LState {
	Stack stack;  // Lua stack
	C_Lua_Stack Cstack;  // C2lua struct
	CallInfo *ci;  // call info for current function
	CallInfo *base_ci;  // array of CallInfo's
	int32 base_ci_size;
	CallInfo *end_ci;  // points after end of ci array
	enum TaskState Tstate;  // state of current thread
	jmp_buf *errorJmp;  // current error recover point
	lua_Task *root_task;  // first task created
	lua_Task *curr_task;
	lua_Task *last_task;
	TObject errorim;  // error tag method
	GCnode rootproto;  // list of all prototypes
	GCnode rootcl;  // list of all closures
	GCnode roottable;  // list of all tables
	GCnode rootglobal;  // list of strings with global values
	stringtable *string_root;  // array of hash tables for strings and udata
	struct IM *IMtable;  // table for tag methods
	int32 IMtable_size;  // size of IMtable
	int32 last_tag;  // last used tag in IMtable
	struct FuncState *mainState, *currState;  // point to local structs in yacc
	struct LexState *lexstate;  // point to local struct in yacc
	ref *refArray;  // locked objects
	int32 refSize;  // size of refArray
	int32 GCthreshold;
	int32 nblocks;  // number of 'blocks' currently allocated
	char *Mbuffer;  /* global buffer */
	char *Mbuffbase;  /* current first position of Mbuffer */
	int Mbuffsize;  /* size of Mbuffer */
	int Mbuffnext;  /* next position to fill in Mbuffer */
	struct C_Lua_Stack Cblocks[MAX_C_BLOCKS];
	int numCblocks;  /* number of nested Cblocks */
};

extern LState *lua_state;
extern int32 globalTaskSerialId;

void lua_resetglobals();

// Switch to the given task */
void luaI_switchtask(lua_Task *t);

// Create a new task and switch to it
lua_Task *luaI_newtask();

} // end of namespace Grim

#endif
