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
#include <stdlib.h>
#include "allfiles.h"

void *allKnownMem[3000];
int allKnownNum = 0;

void outputKnownMem() {
	FILE *debu = fopen("debuTURN.txt", "at");

	fprintf(debu, "%i lumps:", allKnownNum);
	for (int i = 0; i < allKnownNum; i ++) {
		fprintf(debu, " %p", allKnownMem[i]);
	}
	fprintf(debu, "\n");
	fclose(debu);
}

void adding(void *mem) {
	allKnownMem[allKnownNum] = mem;
	allKnownNum ++;

	outputKnownMem();
	if (allKnownNum == 3000) {
		//db ("Error! Array too full!");
		exit(1);
	}
}

void deleting(void *mem) {
	allKnownNum --;
	for (int i = 0; i <= allKnownNum; i ++) {
		if (allKnownMem[i] == mem) {
			allKnownMem[i] = allKnownMem[allKnownNum];
			outputKnownMem();
			return;
		}
	}
	//db ("Error! Deleted a block which hasn't been allocated!");
	exit(1);
}
