/*
** load pre-compiled Lua chunks
** See Copyright Notice in lua.h
*/

#ifndef GRIM_LUNDUMP_H
#define GRIM_LUNDUMP_H

#include "engines/grim/lua/lobject.h"
#include "engines/grim/lua/lzio.h"

namespace Grim {

#define ID_CHUNK		27              // ESC
#define ID_FUNCTION		'#'
#define ID_END			'$'
#define ID_NUM			'N'
#define ID_STR			'S'
#define ID_FUN			'F'
#define	SIGNATURE		"Lua"
#define	VERSION			0x31		// last format change was in 3.1
#define TEST_FLOAT		0.123456789e-23 // a float for testing representation

#define IsMain(f)			(f->lineDefined == 0)

TProtoFunc* luaU_undump1(ZIO* Z);      // load one chunk

} // end of namespace Grim

#endif


