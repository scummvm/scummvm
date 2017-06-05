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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"

#include "sludge/allfiles.h"

namespace Sludge {

void *allKnownMem[3000];
int allKnownNum = 0;

void outputKnownMem() {
#if 0
	FILE *debu = fopen("debuTURN.txt", "at");

	fprintf(debu, "%i lumps:", allKnownNum);
	for (int i = 0; i < allKnownNum; i ++) {
		fprintf(debu, " %p", allKnownMem[i]);
	}
	fprintf(debu, "\n");
	fclose(debu);
#endif
}

void adding(void *mem) {
	allKnownMem[allKnownNum] = mem;
	allKnownNum++;

	outputKnownMem();
	if (allKnownNum == 3000) {
		debug("Error! Array too full!");
#if 0
		exit(1);
#endif
	}
}

void deleting(void *mem) {
	allKnownNum--;
	for (int i = 0; i <= allKnownNum; i++) {
		if (allKnownMem[i] == mem) {
			allKnownMem[i] = allKnownMem[allKnownNum];
			outputKnownMem();
			return;
		}
	}
#if 0
	//db ("Error! Deleted a block which hasn't been allocated!");
	exit(1);
#endif
}

} // End of namespace Sludge
