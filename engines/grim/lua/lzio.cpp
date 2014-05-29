/*
** a generic input stream interface
** See Copyright Notice in lua.h
*/


#include "engines/grim/lua/lzio.h"

#include "common/file.h"

namespace Grim {

int32 zgeteoz(ZIO *) {
	return EOZ;
}

ZIO *zopen(ZIO *z, const char *b, int32 size, const char *name) {
	if (!b)
		return nullptr;
	z->n = size;
	z->p = (const byte *)b;
	z->name = name;
	return z;
}

int32 zread (ZIO *z, void *b, int32 n) {
	while (n) {
		int32 m;
		if (z->n == 0) {
			return n;
		}
		m = (n <= z->n) ? n : z->n;
		memcpy(b, z->p, m);
		z->n -= m;
		z->p += m;
		b = (char *)b + m;
		n -= m;
	}
	return 0;
}

} // end of namespace Grim
