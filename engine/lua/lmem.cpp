/*
** $Id$
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/



#include "lmem.h"
#include "lstate.h"
#include "lua.h"



int32 luaM_growaux (void **block, int32 nelems, int32 size,
                       const char *errormsg, int32 limit)
{
  if (nelems >= limit)
    lua_error(errormsg);
  nelems = (nelems == 0) ? 32 : nelems*2;
  if (nelems > limit)
    nelems = limit;
  *block = luaM_realloc(*block, nelems*size);
  return (int32)nelems;
}



#ifndef DEBUG

/*
** generic allocation routine.
** real ANSI systems do not need some of these tests,
** since realloc(NULL, s)==malloc(s) and realloc(b, 0)==free(b).
** But some systems (e.g. Sun OS) are not that ANSI...
*/
void *luaM_realloc (void *block, int32 size)
{
  if (size == 0) {
    if (block) {
      free(block);
    }
    return NULL;
  }
  block = block ? realloc(block, size) : malloc(size);
  if (block == NULL)
    lua_error(memEM);
  return block;
}



#else
/* DEBUG */

#include <string.h>


#define HEADER	(sizeof(double))

#define MARK    55

int32 numblocks = 0;
int32 totalmem = 0;


static void *checkblock (void *block)
{
  int32 *b = (uint32 *)((char *)block - HEADER);
  int32 size = *b;
  LUA_ASSERT(*(((char *)b)+size+HEADER) == MARK, 
             "corrupted block");
  numblocks--;
  totalmem -= size;
  return b;
}


void *luaM_realloc (void *block, int32 size)
{
  int32 realsize = HEADER+size+1;
  if (realsize != (size_t)realsize)
    lua_error("Allocation Error: Block too big");
  if (size == 0) {  /* ANSI dosen't need this, but some machines... */
    if (block) {
      int32 *b = (int32 *)((char *)block - HEADER);
      memset(block, -1, *b);  /* erase block */
      block = checkblock(block);
      free(block);
    }
    return NULL;
  }
  if (block) {
    block = checkblock(block);
    block = (int32 *)realloc(block, realsize);
  }
  else
    block = (int32 *)malloc(realsize);
  if (block == NULL)
    lua_error(memEM);
  totalmem += size;
  numblocks++;
  *(int32 *)block = size;
  *(((char *)block)+size+HEADER) = MARK;
  return (int32 *)((char *)block+HEADER);
}


#endif
