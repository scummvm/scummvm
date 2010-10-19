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

#include "sword25/gfx/image/b25sloader.h"
#include "sword25/gfx/image/pngloader.h"

namespace Sword25 {

#define BS_LOG_PREFIX "B25SLOADER"

namespace {
static Common::String loadString(Common::ReadStream &in, uint maxSize = 999) {
	Common::String result;

	while (!in.eos() && (result.size() < maxSize)) {
		char ch = (char)in.readByte();
		if ((ch == '\0') || (ch == ' '))
			break;

		result += ch;
	}

	return result;
}

uint findEmbeddedPNG(const byte *fileDataPtr, uint fileSize) {
	assert(fileSize >= 100);
	if (memcmp(fileDataPtr, "BS25SAVEGAME", 12))
		return 0;

	// Read in the header
	Common::MemoryReadStream stream(fileDataPtr, fileSize);
	stream.seek(0, SEEK_SET);

	// Headerinformationen der Spielstandes einlesen.
	uint compressedGamedataSize;
	loadString(stream);
	loadString(stream);
	Common::String gameSize = loadString(stream);
	compressedGamedataSize = atoi(gameSize.c_str());
	loadString(stream);

	// Return the offset of where the thumbnail starts
	return static_cast<uint>(stream.pos() + compressedGamedataSize);
}
}

bool B25SLoader::isCorrectImageFormat(const byte *fileDataPtr, uint fileSize) {
	// PNG innerhalb des Spielstandes finden und den Methodenaufruf zu BS_PNGLoader weiterreichen.
	uint pngOffset = findEmbeddedPNG(fileDataPtr, fileSize);
	if (pngOffset > 0) {
		return PNGLoader::doIsCorrectImageFormat(fileDataPtr + pngOffset, fileSize - pngOffset);
	}

	return false;
}

bool B25SLoader::decodeImage(const byte *fileDataPtr, uint fileSize, GraphicEngine::COLOR_FORMATS colorFormat, byte *&uncompressedDataPtr, int &width, int &height, int &pitch) {
	// PNG innerhalb des Spielstandes finden und den Methodenaufruf zu BS_PNGLoader weiterreichen.
	uint pngOffset = findEmbeddedPNG(fileDataPtr, fileSize);
	if (pngOffset > 0) {
		return PNGLoader::doDecodeImage(fileDataPtr + pngOffset, fileSize - pngOffset, colorFormat, uncompressedDataPtr, width, height, pitch);
	}

	return false;
}

bool B25SLoader::imageProperties(const byte *fileDataPtr, uint fileSize, GraphicEngine::COLOR_FORMATS &colorFormat, int &width, int &height) {
	// PNG innerhalb des Spielstandes finden und den Methodenaufruf zu BS_PNGLoader weiterreichen.
	uint pngOffset = findEmbeddedPNG(fileDataPtr, fileSize);
	if (pngOffset > 0) {
		return PNGLoader::doImageProperties(fileDataPtr + pngOffset, fileSize - pngOffset, colorFormat, width, height);
	}

	return false;
}

} // End of namespace Sword25
