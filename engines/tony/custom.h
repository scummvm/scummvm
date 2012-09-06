/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 */

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_CUSTOM_H
#define TONY_CUSTOM_H

#include "common/str.h"
#include "tony/mpal/mpal.h"

namespace Tony {

using namespace MPAL;

struct MusicFileEntry {
	const char *_name;
	int _sync;
};

static const char *kAmbianceFile[] =  {
	"None",
	"1.ADP",    // Grilli.WAV
	"2.ADP",    // Grilli-Ovattati.WAV
	"3.ADP",    // Grilli-Vento.WAV
	"3.ADP",    // Grilli-Vento1.WAV
	"5.ADP",    // Vento1.WAV
	"4.ADP",    // Mare1.WAV
	"6.ADP"     // Mare1.WAV half volume
};

static const MusicFileEntry kMusicFiles[] =  {
	{"00.ADP", 0},    {"01.ADP", 0},    {"02.ADP", 0},    {"03.ADP", 0},
	{"04.ADP", 0},    {"05.ADP", 0},    {"06.ADP", 0},    {"07.ADP", 0},
	{"08.ADP", 2450}, {"09.ADP", 0},    {"10.ADP", 0},    {"11.ADP", 0},
	{"12.ADP", 0},    {"13.ADP", 0},    {"14.ADP", 0},    {"15.ADP", 0},
	{"16.ADP", 0},    {"17.ADP", 0},    {"18.ADP", 0},    {"19.ADP", 0},
	{"20.ADP", 0},    {"21.ADP", 0},    {"22.ADP", 0},    {"23.ADP", 0},
	{"24.ADP", 0},    {"25.ADP", 0},    {"26.ADP", 0},    {"27.ADP", 0},
	{"28.ADP", 1670}, {"29.ADP", 0},    {"30.ADP", 0},    {"31.ADP", 0},
	{"32.ADP", 2900}, {"33.ADP", 0},    {"34.ADP", 0},    {"35.ADP", 0},
	{"36.ADP", 0},    {"37.ADP", 0},    {"38.ADP", 0},    {"39.ADP", 0},
	{"40.ADP", 0},    {"41.ADP", 1920}, {"42.ADP", 1560}, {"43.ADP", 1920},
	{"44.ADP", 1920}, {"45.ADP", 1920}, {"46.ADP", 1920}, {"47.ADP", 1920},
	{"48.ADP", 1920}, {"49.ADP", 1920}, {"50.ADP", 1920}, {"51.ADP", 1920},
	{"52.ADP", 1920}, {"53.ADP", 0},    {"54.ADP", 0},    {"55.ADP", 0},
	{"56.ADP", 0},    {"57.ADP", 0},    {"58.ADP", 0},    {"59.ADP", 0}
};


static const char *kJingleFileNames[] = {
	"S00.ADP", "S01.ADP", "S02.ADP", "S03.ADP", "S04.ADP", 
	"S05.ADP", "S06.ADP", "S07.ADP", "S08.ADP", "S09.ADP", 
	"S10.ADP", "S11.ADP", "S12.ADP", "S13.ADP", "S14.ADP", 
	"S15.ADP", "S16.ADP", "S17.ADP", "S18.ADP"
};

#define INIT_CUSTOM_FUNCTION            MapCustomFunctions

#define DECLARE_CUSTOM_FUNCTION(x)      void x

#define BEGIN_CUSTOM_FUNCTION_MAP()                                            \
	static void AssignError(int num) {                                           \
		error("Custom function %u has been already assigned!", num);               \
	}                                                                            \
	void INIT_CUSTOM_FUNCTION(LPCUSTOMFUNCTION *lpMap, Common::String *lpStrMap) \
	{

#define END_CUSTOM_FUNCTION_MAP()                                              \
	}

#define ASSIGN(num, func)                                                      \
	if (lpMap[num] != NULL)                                                      \
		AssignError(num);                                                          \
	lpMap[num] = func;                                                           \
	lpStrMap[num] = #func;

class RMTony;
class RMPointer;
class RMGameBoxes;
class RMLocation;
class RMInventory;
class RMInput;

void INIT_CUSTOM_FUNCTION(LPCUSTOMFUNCTION *lpMap, Common::String *lpStrMap);
void setupGlobalVars(RMTony *tony, RMPointer *ptr, RMGameBoxes *box, RMLocation *loc, RMInventory *inv, RMInput *input);

#endif

} // end of namespace Tony
