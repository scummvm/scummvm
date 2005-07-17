// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
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

enum enDebugLevels {
	DEBUG_NONE,
	DEBUG_NORMAL,
	DEBUG_WARN,
	DEBUG_ERROR,
	DEBUG_FUNC,
	DEBUG_BITMAPS,
	DEBUG_MODEL,
	DEBUG_STUB,
	DEBUG_SMUSH,
	DEBUG_IMUSE,
	DEBUG_CHORES,
	DEBUG_ALL
};
extern enDebugLevels debugLevel;
static const char *debug_levels[] = {
	"NONE",
	"NORMAL",
	"WARN",
	"ERROR",
	"FUNC",
	"BITMAP",
	"MODEL",
	"STUB",
	"SMUSH",
	"IMUSE",
	"CHORE",
	"ALL"
};
static const char *debug_descriptions[] = {
	"No debug messages will be printed (default)",
	"\"Normal\" debug messages will be printed",
	"Warning debug messages will be printed",
	"Error debug messages will be printed",
	"Function (normal and stub) debug messages will be printed",
	"Bitmap debug messages will be printed",
	"Model debug messages will be printed",
	"Stub (missing function) debug messages will be printed",
	"SMUSH (video) debug messages will be printed",
	"IMUSE (audio) debug messages will be printed",
	"Chore debug messages will be printed",
	"All debug messages will be printed",
};
// Hacky toggles for experimental / debug code (defined/set in main.cpp)
extern bool ZBUFFER_GLOBAL, SHOWFPS_GLOBAL;

void warning(const char *fmt, ...);
void error(const char *fmt, ...);

const char *tag2str(uint32 tag);
void hexdump(const byte * data, int len, int bytesPerLine);

#endif
