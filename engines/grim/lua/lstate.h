/*
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

/* The pause status for the scripts is stored in one byte in the savegame.
 *
 * The "single script pause" status will be stored in the highest significant bit.
 * For GRIM that flag is never set because GRIM does not pause single scripts.
 *
 * The "all scripts pause" status is a number which counts, how often
 * pause_scripts(TRUE) was called without a corresponding unpause_scripts() call.
 * The value is stored in the lower 7 bit of the pause status byte.
 * For GRIM, since the (un)pause_scripts() are called symmetrically, only
 * the lowest bit will be used.
 */
#define LUA_SG_ALL_PAUSED 0x7f
#define LUA_SG_PAUSED     0x80

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

struct lua_Task;

extern GCnode rootproto;
extern GCnode rootcl;
extern GCnode rootglobal;
extern GCnode roottable;
extern struct ref *refArray;
extern int32 refSize;
extern int32 GCthreshold;
extern int32 nblocks;
extern int32 Mbuffsize;
extern int32 Mbuffnext;
extern char *Mbuffbase;
extern char *Mbuffer;
extern TObject errorim;
extern stringtable *string_root;
extern int32 last_tag;
extern struct IM *IMtable;
extern int32 IMtable_size;

struct LState {
	LState *prev; // handle to previous state in list
	LState *next; // handle to next state in list
	int all_paused; // counter of often pause_scripts(TRUE) was called
	bool paused;    // true if this particular script has been paused
	int32 state_counter1;
	int32 state_counter2;
	bool updated;
	Stack stack;  // Lua stack
	C_Lua_Stack Cstack;  // C2lua struct
	struct FuncState *mainState, *currState;  // point to local structs in yacc
	struct LexState *lexstate;  // point to local struct in yacc
	jmp_buf *errorJmp;  // current error recover point
	lua_Task *task; // handle to task
	lua_Task *some_task;
	uint32 id; // current id of task
	TObject	taskFunc;
	struct C_Lua_Stack Cblocks[MAX_C_BLOCKS];
	int numCblocks; // number of nested Cblocks
	int sleepFor;
};

extern LState *lua_state, *lua_rootState;

extern int globalTaskSerialId;

void lua_stateinit(LState *state);
void lua_statedeinit(LState *state);
void lua_resetglobals();

} // end of namespace Grim

#endif
