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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/gfx/image/b25sloader.h"
#include "sword25/gfx/image/pngloader.h"

namespace Sword25 {

#define BS_LOG_PREFIX "B25SLOADER"

// -----------------------------------------------------------------------------

namespace {
static Common::String LoadString(Common::ReadStream &In, uint MaxSize = 999) {
	Common::String Result;

	char ch = (char)In.readByte();
	while ((ch != '\0') && (ch != ' ')) {
		Result += ch;
		if (Result.size() >= MaxSize) break;
		ch = (char)In.readByte();
	}

	return Result;
}

uint FindEmbeddedPNG(const byte *FileDataPtr, uint FileSize) {
	if (memcmp(FileDataPtr, "BS25SAVEGAME", 12))
		return 0;

	// Read in the header
	Common::MemoryReadStream stream(FileDataPtr, FileSize);

	// Headerinformationen der Spielstandes einlesen.
	uint compressedGamedataSize;
	LoadString(stream);
	LoadString(stream);
	Common::String gameSize = LoadString(stream);
	compressedGamedataSize = atoi(gameSize.c_str());
	LoadString(stream);

	// Return the offset of where the thumbnail starts
	return static_cast<uint>(stream.pos() + compressedGamedataSize);
}
}

// -----------------------------------------------------------------------------

bool B25SLoader::IsCorrectImageFormat(const byte *FileDataPtr, uint FileSize) {
	// PNG innerhalb des Spielstandes finden und den Methodenaufruf zu BS_PNGLoader weiterreichen.
	uint PNGOffset = FindEmbeddedPNG(FileDataPtr, FileSize);
	if (PNGOffset > 0) {
		return PNGLoader::DoIsCorrectImageFormat(FileDataPtr + PNGOffset, FileSize - PNGOffset);
	}

	return false;
}

// -----------------------------------------------------------------------------

bool B25SLoader::DecodeImage(const byte *FileDataPtr, uint FileSize, GraphicEngine::COLOR_FORMATS ColorFormat, byte *&UncompressedDataPtr,
                             int &Width, int &Height, int &Pitch) {
	// PNG innerhalb des Spielstandes finden und den Methodenaufruf zu BS_PNGLoader weiterreichen.
	uint PNGOffset = FindEmbeddedPNG(FileDataPtr, FileSize);
	if (PNGOffset > 0) {
		return PNGLoader::DoDecodeImage(FileDataPtr + PNGOffset, FileSize - PNGOffset, ColorFormat, UncompressedDataPtr, Width, Height, Pitch);
	}

	return false;
}

// -----------------------------------------------------------------------------

bool B25SLoader::ImageProperties(const byte *FileDataPtr, uint FileSize, GraphicEngine::COLOR_FORMATS &ColorFormat, int &Width, int &Height) {
	// PNG innerhalb des Spielstandes finden und den Methodenaufruf zu BS_PNGLoader weiterreichen.
	uint PNGOffset = FindEmbeddedPNG(FileDataPtr, FileSize);
	if (PNGOffset > 0) {
		return PNGLoader::DoImageProperties(FileDataPtr + PNGOffset, FileSize - PNGOffset, ColorFormat, Width, Height);
	}

	return false;
}

} // End of namespace Sword25
