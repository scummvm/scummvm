/***************************************************************************
 vocabdump.c Copyright (C) 1999 Christoph Reichenbach, TU Darmstadt


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CJR) [creichen@rbg.informatik.tu-darmstadt.de]

 History:

   990504 - created (CJR)

***************************************************************************/

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
