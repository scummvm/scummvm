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
 * $URL$
 * $Id$
 *
 */

#include <engine.h>
#include "sciunpack.h"

int
vocab_dump() {
	char **names;
	opcode *opcodes;
	int i = 0, count;
	int *classes;

	printf("Selectors:\n");
	names = vocabulary_get_snames(resmgr, NULL, 0);
	while (names[i]) {
		printf("0x%02X: %s\n", i, names[i]);
		i++;
	}
	vocabulary_free_snames(names);

	i = 0;
	printf("\nOpcodes:\n");
	opcodes = vocabulary_get_opcodes(resmgr);
	while ((i < 256) && (opcodes[i].name)) {
		printf("%s: Type %i, Number %i\n", opcodes[i].name,
		       opcodes[i].type, opcodes[i].number);
		i++;
	}

	names = vocabulary_get_knames(resmgr, &count);
	printf("\nKernel names:\n");
	if (names == 0) printf("Error loading kernel names\n");
	else {
		for (i = 0; i < count; i++) printf("0x%02X: %s\n", i, names[i]);
		vocabulary_free_knames(names);
	}

	classes = vocabulary_get_classes(resmgr, &count);
	printf("\nClasses:\n");
	if (classes == 0) printf("Error loading classes\n");
	else {
		for (i = 0; i < count; i++) printf("0x%02X: script %i\n", i, classes [i]);
		free(classes);
	}

	return 0;
}
