/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_PARSER_H
#define PARALLACTION_PARSER_H

#include "parallaction/defs.h"

namespace Parallaction {

struct ArchivedFile;

void	parseInit(char *s);
char   *parseNextLine(char *s, uint16 count);
char   *parseComment(ArchivedFile *file);
uint16	parseFillBuffers();
char   *parseNextToken(char *s, char *tok, uint16 count, const char *brk);

extern char _tokens[][40];

} // namespace Parallaction

#endif

