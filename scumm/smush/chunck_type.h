/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifndef __CHUNCK_TYPE_H
#define __CHUNCK_TYPE_H

#include "chunck.h"

#define MAKE_TYPE(a,b,c,d) (Chunck::type)( ((a) << 24) | ((b) << 16) | ((c) << 8) | (d) )

static const Chunck::type TYPE_ANIM = MAKE_TYPE('A', 'N', 'I', 'M');
static const Chunck::type TYPE_AHDR = MAKE_TYPE('A', 'H', 'D', 'R');
static const Chunck::type TYPE_FRME = MAKE_TYPE('F', 'R', 'M', 'E');
static const Chunck::type TYPE_NPAL = MAKE_TYPE('N', 'P', 'A', 'L');
static const Chunck::type TYPE_FOBJ = MAKE_TYPE('F', 'O', 'B', 'J');
static const Chunck::type TYPE_PSAD = MAKE_TYPE('P', 'S', 'A', 'D');
static const Chunck::type TYPE_TRES = MAKE_TYPE('T', 'R', 'E', 'S');
static const Chunck::type TYPE_XPAL = MAKE_TYPE('X', 'P', 'A', 'L');
static const Chunck::type TYPE_IACT = MAKE_TYPE('I', 'A', 'C', 'T');
static const Chunck::type TYPE_STOR = MAKE_TYPE('S', 'T', 'O', 'R');
static const Chunck::type TYPE_FTCH = MAKE_TYPE('F', 'T', 'C', 'H');
static const Chunck::type TYPE_SKIP = MAKE_TYPE('S', 'K', 'I', 'P');
static const Chunck::type TYPE_STRK = MAKE_TYPE('S', 'T', 'R', 'K');
static const Chunck::type TYPE_SMRK = MAKE_TYPE('S', 'M', 'R', 'K');
static const Chunck::type TYPE_SHDR = MAKE_TYPE('S', 'H', 'D', 'R');
static const Chunck::type TYPE_SDAT = MAKE_TYPE('S', 'D', 'A', 'T');
static const Chunck::type TYPE_SAUD = MAKE_TYPE('S', 'A', 'U', 'D');
static const Chunck::type TYPE_iMUS = MAKE_TYPE('i', 'M', 'U', 'S');
static const Chunck::type TYPE_FRMT = MAKE_TYPE('F', 'R', 'M', 'T');
static const Chunck::type TYPE_TEXT = MAKE_TYPE('T', 'E', 'X', 'T');
static const Chunck::type TYPE_REGN = MAKE_TYPE('R', 'E', 'G', 'N');
static const Chunck::type TYPE_STOP = MAKE_TYPE('S', 'T', 'O', 'P');
static const Chunck::type TYPE_MAP_ = MAKE_TYPE('M', 'A', 'P', ' ');
static const Chunck::type TYPE_DATA = MAKE_TYPE('D', 'A', 'T', 'A');
static const Chunck::type TYPE_ETRS = MAKE_TYPE('E', 'T', 'R', 'S');

#undef MAKE_TYPE

#endif
