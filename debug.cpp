// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "debug.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <SDL.h>

const char *tag2str(uint32 tag) {
	static char str[5];
	str[0] = (char)(tag >> 24);
	str[1] = (char)(tag >> 16);
	str[2] = (char)(tag >> 8);
	str[3] = (char)tag;
	str[4] = '\0';
	return str;
}

void hexdump(const byte * data, int len, int bytesPerLine) {
	assert(1 <= bytesPerLine && bytesPerLine <= 32);
	int i;
	byte c;
	int offset = 0;

	while (len >= bytesPerLine) {
		printf("%06x: ", offset);
		for (i = 0; i < bytesPerLine; i++) {
			printf("%02x ", data[i]);
			if (i % 4 == 3)
				printf(" ");
		}
		printf(" |");
		for (i = 0; i < bytesPerLine; i++) {
			c = data[i];
			if (c < 32 || c >= 127)
				c = '.';
			printf("%c", c);
		}
		printf("|\n");
		data += bytesPerLine;
		len -= bytesPerLine;
		offset += bytesPerLine;
	}

	if (len <= 0)
		return;

	printf("%06x: ", offset);
	for (i = 0; i < bytesPerLine; i++) {
		if (i < len)
			printf("%02x ", data[i]);
		else
			printf("   ");
		if (i % 4 == 3)
			printf(" ");
	}
	printf(" |");
	for (i = 0; i < len; i++) {
		c = data[i];
		if (c < 32 || c >= 127)
			c = '.';
		printf("%c", c);
	}
	for (; i < bytesPerLine; i++)
		printf(" ");
	printf("|\n");
}

void warning(const char *fmt, ...) {
	std::fprintf(stderr, "WARNING: ");

	std::va_list va;

	va_start(va, fmt);
	std::vfprintf(stderr, fmt, va);
	va_end(va);
	std::fprintf(stderr, "\n");
}

void error(const char *fmt, ...) {
	std::fprintf(stderr, "ERROR: ");

	std::va_list va;

	va_start(va, fmt);
	std::vfprintf(stderr, fmt, va);
	va_end(va);
	std::fprintf(stderr, "\n");

	exit(1);
}

const char *debug_levels[] = {
        "NONE",
        "NORMAL",
        "WARN",
        "ERROR",
        "FUNC",
        "BITMAP",
        "MODEL",
        "STUB",
        "SMUSH",
        "IMUSE",
        "CHORE",
        "ALL"
};

const char *debug_descriptions[] = {
        "No debug messages will be printed (default)",
        "\"Normal\" debug messages will be printed",
        "Warning debug messages will be printed",
        "Error debug messages will be printed",
        "Function (normal and stub) debug messages will be printed",
        "Bitmap debug messages will be printed",
        "Model debug messages will be printed",
        "Stub (missing function) debug messages will be printed",
        "SMUSH (video) debug messages will be printed",
        "IMUSE (audio) debug messages will be printed",
        "Chore debug messages will be printed",
        "All debug messages will be printed",
};

