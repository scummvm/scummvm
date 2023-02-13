/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// AsciiCptCompile.cpp
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "TextFile.h"

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

void doCompile(FILE *inf, FILE *debOutf, FILE *resOutf, TextFile *cptDef, FILE *sve);

int main(int argc, char* argv[])
{
	volatile uint32 t = 0x11223344;
	if (*((volatile uint8 *)&t) != 0x44) {
		printf("Sorry, this program only works on little endian systems.\nGoodbye.\n");
		return 0;
	}
	TextFile *cptDef = new TextFile("COMPACT.TXT");
	FILE *inf = fopen("COMPACT.TXT", "r");
	FILE *dbg = fopen("compact.dbg", "wb");
	FILE *out = fopen("compact.bin", "wb");
	FILE *sve = fopen("savedata.txt", "r");
	assert(inf && dbg && out && sve);
	doCompile(inf, dbg, out, cptDef, sve);
	fclose(inf);
	fclose(dbg);
	fclose(out);
	fclose(sve);
	printf("done\n");
	return 0;
}
