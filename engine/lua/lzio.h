/*
** $Id$
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef lzio_h
#define lzio_h

#include <common/sys.h>



/* For Lua only */
#define zFopen	luaZ_Fopen
#define zsopen	luaZ_sopen
#define zmopen	luaZ_mopen

#define EOZ	(-1)			/* end of stream */

typedef struct zio ZIO;

ZIO* zFopen (ZIO* z, FILE* f, const char *name);		/* open FILEs */
ZIO* zsopen (ZIO* z, const char* s, const char *name);		/* string */
ZIO* zmopen (ZIO* z, const char* b, int32 size, const char *name);	/* memory */

int32 zread (ZIO* z, void* b, int32 n);	/* read next n bytes */

#define zgetc(z)	(--(z)->n>=0 ? ((int32)*(z)->p++): (z)->filbuf(z))
#define zungetc(z)	(++(z)->n,--(z)->p)
#define zname(z)	((z)->name)


/* --------- Private Part ------------------ */

#define ZBSIZE	256			/* buffer size */

struct zio {
 int32 n;					/* bytes still unread */
 const byte* p;			/* current position in buffer */
 int32 (*filbuf)(ZIO* z);
 void* u;				/* additional data */
 const char *name;
 byte buffer[ZBSIZE];		/* buffer */
};


#endif
