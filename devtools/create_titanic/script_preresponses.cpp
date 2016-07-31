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
#include "script_preresponses.h"

static const PreResponse BARBOT_PRERESPONSES[] = {
	{ 0x0000CA02, 0x0003D102 },
	{ 0x0000CA68, 0x0003D102 },
	{ 0x0000C9DA, 0x0003D102 },
	{ 0x0000CA6A, 0x0003D103 },
	{ 0x0000C922, 0x0003D099 },
	{ 0x0000C97C, 0x0003D099 },
	{ 0x0000CA0B, 0x0003D099 },
	{ 0x0000CA72, 0x0003D099 },
	{ 0x0000CA0E, 0x0003D107 },
	{ 0x0000CA73, 0x0003D107 },
	{ 0x0000CA12, 0x0003D108 },
	{ 0x0000CA1C, 0x0003D10E },
	{ 0x0000CA83, 0x0003D10E },
	{ 0x0000CA1F, 0x0003D110 },
	{ 0x0000CA86, 0x0003D110 },
	{ 0x0000CA23, 0x0003D112 },
	{ 0x0000CA8A, 0x0003D112 },
	{ 0x0000CA92, 0x0003D122 },
	{ 0x0000CA30, 0x0003D116 },
	{ 0x0000CA96, 0x0003D116 },
	{ 0x0000CA36, 0x0003D117 },
	{ 0x0000C9FC, 0x0003D117 },
	{ 0x0000CA9B, 0x0003D117 },
	{ 0x0000CA63, 0x0003D117 },
	{ 0x0000CA38, 0x0003D11B },
	{ 0x0000CA15, 0x0003D109 },
	{ 0x0000CA7B, 0x0003D109 },
	{ 0x0000CA2E, 0x0003D115 },
	{ 0x0000CA94, 0x0003D115 },
	{ 0x0000CA5C, 0x0003D115 },
	{ 0x0000CA21, 0x0003D111 },
	{ 0x0000CA88, 0x0003D111 },
	{ 0x0000CA2A, 0x0003D114 },
	{ 0x0000CA28, 0x0003D119 },
	{ 0x0000CA8E, 0x0003D119 },
	{ 0x0000CA17, 0x0003D10B },
	{ 0x0000CA7D, 0x0003D10B },
	{ 0x0000CA4C, 0x0003D10B },
	{ 0x0000CA06, 0x0003D105 },
	{ 0x0000CA6C, 0x0003D105 },
	{ 0x0000CA0A, 0x0003D106 },
	{ 0x0000CA70, 0x0003D106 },
	{ 0x0000CA19, 0x0003D10C },
	{ 0x0000CA7F, 0x0003D10C },
	{ 0x0000C9FF, 0x0003D101 },
	{ 0x0000CA65, 0x0003D101 },
	{ 0x00000000, 0x00000000 }
};

static const PreResponse BELLBOT_PRERESPONSES[] = {
	{ 0x000052DC, 0x00030D40 },
	{ 0x000054E9, 0x00030D40 },
	{ 0x000054EC, 0x00030D40 },
	{ 0x000054F0, 0x00030D40 },
	{ 0x0000532C, 0x00031625 },
	{ 0x00005330, 0x00031625 },
	{ 0x00005368, 0x00031625 },
	{ 0x00005369, 0x00031625 },
	{ 0x0000536A, 0x00031625 },
	{ 0x0000536B, 0x00031625 },
	{ 0x0000536C, 0x00031625 },
	{ 0x0000536D, 0x00031625 },
	{ 0x000053A4, 0x00031625 },
	{ 0x0000558A, 0x00031625 },
	{ 0x00005485, 0x00031625 },
	{ 0x00004EE7, 0x00031625 },
	{ 0x00004EE8, 0x00031625 },
	{ 0x0000530A, 0x00031625 },
	{ 0x0000530B, 0x00031625 },
	{ 0x000053F6, 0x00031625 },
	{ 0x000053F7, 0x00031625 },
	{ 0x000053F8, 0x00031625 },
	{ 0x000053F9, 0x00031625 },
	{ 0x000053FA, 0x00031625 },
	{ 0x000053FB, 0x00031625 },
	{ 0x000053FC, 0x00031625 },
	{ 0x000053FD, 0x00031625 },
	{ 0x0000556B, 0x00031041 },
	{ 0x00005499, 0x00030D40 },
	{ 0x000053E9, 0x00030E01 },
	{ 0x000053EB, 0x00030E01 },
	{ 0x000053EC, 0x00030E01 },
	{ 0x000053ED, 0x00030E01 },
	{ 0x000053EE, 0x00030E01 },
	{ 0x000053EF, 0x00030E01 },
	{ 0x000053F0, 0x00030E01 },
	{ 0x000053F1, 0x00030E01 },
	{ 0x000053F2, 0x00030E01 },
	{ 0x000053EA, 0x00030E01 },
	{ 0x00005441, 0x00030F00 },
	{ 0x00005444, 0x00030F00 },
	{ 0x00005445, 0x00030F00 },
	{ 0x00005443, 0x00030F00 },
	{ 0x00005446, 0x00030F00 },
	{ 0x00005447, 0x00030F00 },
	{ 0x00005448, 0x00030F00 },
	{ 0x00005449, 0x00030F00 },
	{ 0x0000544A, 0x00030F00 },
	{ 0x0000544B, 0x00030F00 },
	{ 0x00005442, 0x00030F00 },
	{ 0x0000527C, 0x000315C8 },
	{ 0x00000000, 0x00000000 }
};

void writeScriptPreResponses(const char *name, const PreResponse *states) {
	outputFile.seek(dataOffset);

	for (; states->_src; ++states) {
		outputFile.writeLong(states->_src);
		outputFile.writeLong(states->_dest);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;
}

void writeAllScriptPreResponses() {
	writeScriptPreResponses("PreResponses/Barbot", BARBOT_PRERESPONSES);
	writeScriptPreResponses("PreResponses/Bellbot", BELLBOT_PRERESPONSES);
}