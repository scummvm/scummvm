/*
** Syntax analizer and code generator
** See Copyright Notice in lua.h
*/

#ifndef GRIM_LPARSER_H
#define GRIM_LPARSER_H

#include "lobject.h"
#include "lzio.h"

namespace Grim {

void luaY_codedebugline(int32 line);
TProtoFunc *luaY_parser(ZIO *z);
void luaY_error(const char *s);
void luaY_syntaxerror(const char *s, const char *token);

} // end of namespace Grim

#endif
