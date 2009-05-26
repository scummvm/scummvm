/*
** $Id$
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
void luaY_error(char *s);
void luaY_syntaxerror(char *s, char *token);

} // end of namespace Grim

#endif
