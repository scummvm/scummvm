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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_TEXT_H
#define LAB_TEXT_H


#include "lab/stddefines.h"

namespace Lab {

#if defined(WIN32)
#pragma pack(push, 1)
#endif

struct TextFont {
	uint32 DataLength;
	uint16 Height;
	byte Widths[256];
	uint16 Offsets[256];
	byte *data;
};

#if defined(WIN32)
#pragma pack(pop)
#endif

bool openFontMem(const char *TextFontPath, TextFont *tf, byte *fontbuffer);

bool openFont(const char *TextFontPath, TextFont **tf);

void closeFont(TextFont *tf);

uint16 textLength(TextFont *tf, const char *text, uint16 numchars);

uint16 textHeight(TextFont *tf);

void text(TextFont *tf, uint16 x, uint16 y, uint16 color, const char *text, uint16 numchars);

} // End of namespace Lab

#endif /* LAB_TEXT_H */
