/*
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef GRIM_LZIO_H
#define GRIM_LZIO_H

#include "common/scummsys.h"

namespace Common {
	class File;
}

namespace Grim {

// For Lua only
#define zopen	luaZ_mopen

#define EOZ	(-1)			// end of stream

typedef struct zio ZIO;

ZIO *zopen(ZIO *z, const char *b, int32 size, const char *name);
int32 zread(ZIO *z, void *b, int32 n);	// read next n bytes
int32 zgeteoz(ZIO *);

#define zgetc(z)	(--(z)->n >= 0 ? ((int32)*(z)->p++): zgeteoz(z))
#define zungetc(z)	(++(z)->n, --(z)->p)
#define zname(z)	((z)->name)


// --------- Private Part ------------------

#define ZBSIZE	256			// buffer size

struct zio {
	int32 n;					// bytes still unread
	const byte *p;				// current position in buffer
	const char *name;
	byte buffer[ZBSIZE];		// buffer
};

} // end of namespace Grim

#endif
