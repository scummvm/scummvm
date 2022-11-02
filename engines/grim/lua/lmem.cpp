/*
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp

#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/lua.h"

namespace Grim {

int32 luaM_growaux(void **block, int32 nelems, int32 size, const char *errormsg, int32 limit) {
	if (nelems >= limit)
		lua_error(errormsg);
	nelems = (nelems == 0) ? 32 : nelems * 2;
	if (nelems > limit)
		nelems = limit;
	*block = luaM_realloc(*block, nelems * size);
	return (int32)nelems;
}

#ifndef LUA_DEBUG

/*
** generic allocation routine.
** real ANSI systems do not need some of these tests,
** since realloc(NULL, s)==malloc(s) and realloc(b, 0)==free(b).
** But some systems (e.g. Sun OS) are not that ANSI...
*/
void *luaM_realloc(void *block, int32 size) {
	if (size == 0) {
		free(block);
		return nullptr;
	}
	block = block ? realloc(block, size) : malloc(size);
	if (!block)
		lua_error(memEM);
	return block;
}

#else
/* LUA_DEBUG */

#define MARK    55

int32 numblocks = 0;
int32 totalmem = 0;

static void *checkblock(void *block) {
	int32 *b = (int32 *)block - 1;
	int32 size = *b;
	assert(*(((char *)b) + size + sizeof(int32)) == MARK);
	numblocks--;
	totalmem -= size;
	return b;
}

void *luaM_realloc(void *block, int32 size) {
	int32 realsize = HEADER + size + 1;
	int32 realsize = sizeof(int32) + size + sizeof(char);
	if (realsize != (size_t)realsize)
		lua_error("Allocation Error: Block too big");
	if (size == 0) {  // ANSI dosen't need this, but some machines...
		if (block) {
			memset(block, -1, *((int32 *)block - 1));  // erase block
			block = checkblock(block);
			free(block);
		}
		return NULL;
	}
	if (block) {
		block = checkblock(block);
		block = (int32 *)realloc(block, realsize);
	} else
		block = (int32 *)malloc(realsize);
	if (!block)
		lua_error(memEM);
	totalmem += size;
	numblocks++;
	*(int32 *)block = size;
	*(((char *)block) + size + sizeof(int32)) = MARK;
	return (int32 *)block+1;
}

#endif

} // end of namespace Grim
