/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef CHUNK_TYPE_H
#define CHUNK_TYPE_H

#include "chunk.h"

#define MAKE_TYPE(a,b,c,d) (Chunk::type)( ((a) << 24) | ((b) << 16) | ((c) << 8) | (d) )

static const Chunk::type TYPE_ANIM = MAKE_TYPE('A', 'N', 'I', 'M');
static const Chunk::type TYPE_AHDR = MAKE_TYPE('A', 'H', 'D', 'R');
static const Chunk::type TYPE_FRME = MAKE_TYPE('F', 'R', 'M', 'E');
static const Chunk::type TYPE_NPAL = MAKE_TYPE('N', 'P', 'A', 'L');
static const Chunk::type TYPE_FOBJ = MAKE_TYPE('F', 'O', 'B', 'J');
static const Chunk::type TYPE_PSAD = MAKE_TYPE('P', 'S', 'A', 'D');
static const Chunk::type TYPE_TRES = MAKE_TYPE('T', 'R', 'E', 'S');
static const Chunk::type TYPE_XPAL = MAKE_TYPE('X', 'P', 'A', 'L');
static const Chunk::type TYPE_IACT = MAKE_TYPE('I', 'A', 'C', 'T');
static const Chunk::type TYPE_STOR = MAKE_TYPE('S', 'T', 'O', 'R');
static const Chunk::type TYPE_FTCH = MAKE_TYPE('F', 'T', 'C', 'H');
static const Chunk::type TYPE_SKIP = MAKE_TYPE('S', 'K', 'I', 'P');
static const Chunk::type TYPE_STRK = MAKE_TYPE('S', 'T', 'R', 'K');
static const Chunk::type TYPE_SMRK = MAKE_TYPE('S', 'M', 'R', 'K');
static const Chunk::type TYPE_SHDR = MAKE_TYPE('S', 'H', 'D', 'R');
static const Chunk::type TYPE_SDAT = MAKE_TYPE('S', 'D', 'A', 'T');
static const Chunk::type TYPE_SAUD = MAKE_TYPE('S', 'A', 'U', 'D');
static const Chunk::type TYPE_iMUS = MAKE_TYPE('i', 'M', 'U', 'S');
static const Chunk::type TYPE_FRMT = MAKE_TYPE('F', 'R', 'M', 'T');
static const Chunk::type TYPE_TEXT = MAKE_TYPE('T', 'E', 'X', 'T');
static const Chunk::type TYPE_REGN = MAKE_TYPE('R', 'E', 'G', 'N');
static const Chunk::type TYPE_STOP = MAKE_TYPE('S', 'T', 'O', 'P');
static const Chunk::type TYPE_MAP_ = MAKE_TYPE('M', 'A', 'P', ' ');
static const Chunk::type TYPE_DATA = MAKE_TYPE('D', 'A', 'T', 'A');
static const Chunk::type TYPE_ETRS = MAKE_TYPE('E', 'T', 'R', 'S');

#undef MAKE_TYPE

#endif
