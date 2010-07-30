// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

#include "sword25/gfx/image/b25sloader.h"
#include "sword25/gfx/image/pngloader.h"

#define BS_LOG_PREFIX "B25SLOADER"

// -----------------------------------------------------------------------------

namespace
{
	unsigned int FindEmbeddedPNG(const char * FileDataPtr, unsigned int FileSize)
	{
		// Einen Stringstream mit dem Anfang der Datei intialisieren. 512 Byte sollten hierfür genügen.
		istringstream StringStream(string(FileDataPtr, FileDataPtr + min(static_cast<unsigned int>(512), FileSize)));

		// Headerinformationen der Spielstandes einlesen.
		string Marker, VersionID;
		unsigned int CompressedGamedataSize, UncompressedGamedataSize;
		StringStream >> Marker >> VersionID >> CompressedGamedataSize >> UncompressedGamedataSize;
		if (!StringStream.good()) return 0;

		// Testen, ob wir tatsächlich einen Spielstand haben.
		if (Marker == "BS25SAVEGAME")
		{
			// Offset zum PNG innerhalb des Spielstandes berechnen und zurückgeben.
			return static_cast<unsigned int>(StringStream.tellg()) + CompressedGamedataSize + 1;
		}

		return 0;
	}
}

// -----------------------------------------------------------------------------

bool BS_B25SLoader::IsCorrectImageFormat(const char * FileDataPtr, unsigned int FileSize)
{
	// PNG innerhalb des Spielstandes finden und den Methodenaufruf zu BS_PNGLoader weiterreichen.
	unsigned int PNGOffset = FindEmbeddedPNG(FileDataPtr, FileSize);
	if (PNGOffset > 0)
	{
		return BS_PNGLoader::DoIsCorrectImageFormat(FileDataPtr + PNGOffset, FileSize - PNGOffset);
	}

	return false;
}

// -----------------------------------------------------------------------------

bool BS_B25SLoader::DecodeImage(const char * FileDataPtr, unsigned int FileSize, BS_GraphicEngine::COLOR_FORMATS ColorFormat, char * & UncompressedDataPtr,
								int & Width, int & Height, int & Pitch)
{
	// PNG innerhalb des Spielstandes finden und den Methodenaufruf zu BS_PNGLoader weiterreichen.
	unsigned int PNGOffset = FindEmbeddedPNG(FileDataPtr, FileSize);
	if (PNGOffset > 0)
	{
		return BS_PNGLoader::DoDecodeImage(FileDataPtr + PNGOffset, FileSize - PNGOffset, ColorFormat, UncompressedDataPtr, Width, Height, Pitch);
	}

	return false;
}

// -----------------------------------------------------------------------------

bool BS_B25SLoader::ImageProperties(const char * FileDataPtr, unsigned int FileSize, BS_GraphicEngine::COLOR_FORMATS & ColorFormat, int & Width, int & Height)
{
	// PNG innerhalb des Spielstandes finden und den Methodenaufruf zu BS_PNGLoader weiterreichen.
	unsigned int PNGOffset = FindEmbeddedPNG(FileDataPtr, FileSize);
	if (PNGOffset > 0)
	{
		return BS_PNGLoader::DoImageProperties(FileDataPtr + PNGOffset, FileSize - PNGOffset, ColorFormat, Width, Height);
	}

	return false;
}
