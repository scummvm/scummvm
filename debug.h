// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "bits.h"

#ifndef DEBUG_H
#define DEBUG_H
// Hacky toggles for experimental / debug code (defined/set in main.cpp)
extern int ZBUFFER_GLOBAL, SCREENBLOCKS_GLOBAL;

void warning(const char *fmt, ...);
void error(const char *fmt, ...);

const char *tag2str(uint32 tag);
void hexdump(const byte * data, int len, int bytesPerLine);
#endif
