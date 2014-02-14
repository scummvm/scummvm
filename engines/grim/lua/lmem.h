/*
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#ifndef GRIM_LMEM_H
#define GRIM_LMEM_H

#include "common/scummsys.h"

namespace Grim {

// memory error messages
#define codeEM		"code size overflow"
#define constantEM	"constant table overflow"
#define refEM		"reference table overflow"
#define tableEM		"table overflow"
#define memEM		"not enough memory"

void *luaM_realloc (void *oldblock, int32 size);
int32 luaM_growaux (void **block, int32 nelems, int32 size, const char *errormsg, int32 limit);

#define luaM_free(b)						free((b))
#define luaM_malloc(t)						malloc((t))
#define luaM_new(t)							((t *)malloc(sizeof(t)))
#define luaM_newvector(n, t)				((t *)malloc((n) * sizeof(t)))
#define luaM_growvector(old, n, t, e, l)	(luaM_growaux((void**)old, n, sizeof(t), e, l))
#define luaM_reallocvector(v, n, t)			((t *)realloc(v,(n) * sizeof(t)))

#ifdef LUA_DEBUG
extern int32 numblocks;
extern int32 totalmem;
#endif

} // end of namespace Grim

#endif

