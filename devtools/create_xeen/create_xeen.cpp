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

#define VERSION_NUMBER 5

void NORETURN_PRE error(const char *s, ...) {
	va_list ap;

	va_start(ap, s);
	vfprintf(stderr, s, ap);
	va_end(ap);

	fputc('\n', stderr);

	exit(1);
}

static void writeVersion(CCArchive &cc) {
	Common::MemFile f;
	f.writeLong(VERSION_NUMBER);
	cc.add("VERSION", f);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		error("Format: %s dark.cc \"swords xeen.dat\"", argv[0]);
	}

	Common::File outputFile;
	if (!outputFile.open("xeen.ccs", Common::kFileWriteMode)) {
		error("Could not open input file");
	}

	CCArchive cc(outputFile, kWrite);
	writeVersion(cc);
	writeConstants(cc);
	writeMap(cc);

	const char *darkName = argv[1];
	writeCloudsData(cc, darkName);

	const char *swordsDatName = argv[2];
	writeSwordsData(cc, swordsDatName);

	cc.close();
	return 0;
}
