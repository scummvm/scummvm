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

/*
    BS_PNGLoader
    ------------
    BS_ImageLoader-Klasse zum Laden von PNG-Dateien

    Autor: Malte Thiesen
*/

#ifndef SWORD25_PNGLOADER2_H
#define SWORD25_PNGLOADER2_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/gfx/image/imageloader.h"

namespace Sword25 {

// Klassendefinition
class BS_PNGLoader : public BS_ImageLoader {
public:
	static BS_ImageLoader *CreateInstance() {
#include "sword25/kernel/memlog_off.h"
		return (BS_ImageLoader *) new BS_PNGLoader();
#include "sword25/kernel/memlog_on.h"
	}

	// Alle virtuellen Methoden von BS_ImageLoader sind hier als static-Methode implementiert, damit sie von BS_B25SLoader aufgerufen werden können.
	// Die virtuellen Methoden rufen diese Methoden auf.
	static bool DoIsCorrectImageFormat(const char *FileDataPtr, unsigned int FileSize);
	static bool DoDecodeImage(const char *FileDataPtr, unsigned int FileSize,  BS_GraphicEngine::COLOR_FORMATS ColorFormat, byte *&UncompressedDataPtr,
	                          int &Width, int &Height, int &Pitch);
	static bool DoImageProperties(const char *FileDataPtr, unsigned int FileSize, BS_GraphicEngine::COLOR_FORMATS &ColorFormat, int &Width, int &Height);

protected:
	BS_PNGLoader();
	bool DecodeImage(const char *pFileData, unsigned int FileSize,
	                 BS_GraphicEngine::COLOR_FORMATS ColorFormat,
	                 byte *&pUncompressedData,
	                 int &Width, int &Height,
	                 int &Pitch);
	bool IsCorrectImageFormat(const char *FileDataPtr, unsigned int FileSize);
	bool ImageProperties(const char *FileDatePtr, unsigned int FileSize, BS_GraphicEngine::COLOR_FORMATS &ColorFormat, int &Width, int &Height);
};

} // End of namespace Sword25

#endif
