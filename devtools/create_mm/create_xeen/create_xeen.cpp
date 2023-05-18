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

 // Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cc.h"
#include "file.h"
#include "clouds.h"
#include "swords.h"
#include "constants.h"
#include "map.h"

void NORETURN_PRE error(const char *s, ...) {
	va_list ap;

	va_start(ap, s);
	vfprintf(stderr, s, ap);
	va_end(ap);

	fputc('\n', stderr);

	exit(1);
}

static void writeVersion() {
	Common::File f;
	if (!f.open("version.txt", Common::kFileWriteMode))
		error("Could not create version.txt");

	f.write("1.1\n", 4);
	f.close();
}

/**
 * Creates the different files in the xeen/ subfolder of
 * the files/ folder. The files folder overall is zipped
 * up to form the xeen data file
 */
int main(int argc, char *argv[]) {
	if (argc != 3) {
		error("Format: %s dark.cc \"swords xeen.dat\"", argv[0]);
	}

	writeVersion();
	writeConstants();
	writeMap();

	const char *darkName = argv[1];
	writeCloudsData(darkName);

	const char *swordsDatName = argv[2];
	writeSwordsData(swordsDatName);

	printf("File creation done");
	return 0;
}
