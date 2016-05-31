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

 // Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "file.h"
#include "script_tags.h"

static const int DESKBOT_TAGS[][5] = {
	{ MKTAG('P', 'K', 'U', 'P'), 240573, 0, 0, 0 },
	{ MKTAG('S', 'E', 'X', '1'), 240573, 0, 0, 0 },
	{ MKTAG('R', 'E', 'S', '1'), 240354, 0, 0, 0 },
	{ MKTAG('R', 'E', 'S', '2'), 240547, 0, 0, 0 },
	{ MKTAG('S', 'W', 'E', 'R'), 240578, 0, 0, 0 },
	{ MKTAG('B', 'Y', 'Z', 'A'), 241173, 0, 0, 0 },
	{ MKTAG('S', 'A', 'S', 'S'), 240986, 0, 0, 0 },
	{ MKTAG('S', 'H', 'M', 'S'), 240453, 0, 0, 0 },
	{ MKTAG('F', 'O', 'O', 'D'), 240849, 0, 0, 0 },
	{ MKTAG('J', 'F', 'O', 'D'), 240849, 0, 0, 0 },

	{ MKTAG('C', 'H', 'S', 'E'), 240849, 0, 0, 0 },
	{ MKTAG('A', 'C', 'T', 'R'), 240654, 0, 0, 0 },
	{ MKTAG('A', 'C', 'T', 'S'), 240655, 0, 0, 0 },
	{ MKTAG('M', 'U', 'S', 'I'), 240681, 240849, 0, 0 },
	{ MKTAG('S', 'A', 'N', 'G'), 240681, 240657, 0, 0 },
	{ MKTAG('S', 'O', 'A', 'P'), 240681, 0, 0, 0 },
	{ MKTAG('T', 'V', 'S', 'H'), 240681, 0, 0, 0 },
	{ MKTAG('A', 'R', 'T', 'I'), 240657, 0, 0, 0 },
	{ MKTAG('A', 'U', 'T', 'H'), 240657, 0, 0, 0 },
	{ MKTAG('C', 'O', 'M', 'D'), 240657, 240785, 0, 0 },

	{ MKTAG('C', 'O', 'O', 'K'), 240657, 0, 0, 0 },
	{ MKTAG('C', 'O', 'P', 'S'), 240657, 0, 0, 0 },
	{ MKTAG('H', 'E', 'R', 'O'), 240657, 0, 0, 0 },
	{ MKTAG('H', 'O', 'S', 'T'), 240657, 0, 0, 0 },
	{ MKTAG('P', 'T', 'I', 'C'), 240657, 0, 0, 0 },
	{ MKTAG('S', 'C', 'I', 'T'), 240657, 0, 0, 0 },
	{ MKTAG('E', 'A', 'R', 'T'), 240728, 0, 0, 0 },
	{ MKTAG('P', 'L', 'A', 'N'), 240728, 0, 0, 0 },
	{ MKTAG('F', 'I', 'L', 'M'), 240939, 0, 0, 0 },
	{ MKTAG('F', 'I', 'S', 'H'), 240437, 0, 0, 0 },

	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },

	{ MKTAG('H', 'H', 'G', 'Q'), 241065, 240453, 0, 0 },
	{ MKTAG('L', 'I', 'Q', 'D'), 241167, 0, 0, 0 },
	{ MKTAG('P', 'H', 'I', 'L'), 240607, 0, 0, 0 },
	{ MKTAG('S', 'I', 'C', 'K'), 241170, 0, 0, 0 },
	{ MKTAG('T', 'W', 'A', 'T'), 240975, 0, 0, 0 },
	{ MKTAG('H', 'A', 'H', 'A'), 240785, 0, 0, 0 },
	{ MKTAG('S', 'C', 'I', 'T'), 240968, 241617, 0, 0 },
	{ MKTAG('S', 'C', 'I', 'E'), 240967, 241616, 0, 0 },
	{ MKTAG('S', 'L', 'O', 'W'), 241614, 0, 0, 0 },
	{ MKTAG('T', 'H', 'A', 'T'), 240760,  241615, 0, 0 },

	{ MKTAG('T', 'D', 'U', 'P'), 241161, 241618, 0, 0 },
	{ MKTAG('T', 'I', 'T', 'A'), 241619, 0, 0, 0 },
	{ MKTAG('C', 'S', 'P', 'Y'), 241620, 0, 0, 0 },
	{ MKTAG('M', 'I', 'N', 'S'), 241621, 0, 0, 0 },
	{ MKTAG('M', 'C', 'P', 'Y'), 241622, 0, 0, 0 },
	{ MKTAG('D', 'N', 'C', 'E'), 241623, 0, 0, 0 },
	{ MKTAG('N', 'A', 'U', 'T'), 241624, 0, 0, 0 },
	{ MKTAG('A', 'D', 'V', 'T'), 240939, 241622, 0, 0 },
	{ MKTAG('A', 'N', 'S', 'W'), 240453, 0, 0, 0 },
	{ MKTAG('A', 'R', 'T', 'Y'), 240658, 0, 0, 0 },

	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },

	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },

	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },

	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },

	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },

	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },

	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 },
	{ MKTAG('x', 'x', 'x', 'x'), 0, 0, 0, 0 }
};

void writeScriptTags(const char *name, const int *tags, uint count) {
	outputFile.seek(dataOffset);

	for (int idx = 0; idx < count * 5; ++idx, ++tags)
		outputFile.writeLong(*tags);
	
	writeEntryHeader(name, dataOffset, count * 5 * 4);
	dataOffset += count * 5 * 4;
}

void writeAllScriptTags() {
	writeScriptTags("Tags/103", &DESKBOT_TAGS[0][0], 129);
}