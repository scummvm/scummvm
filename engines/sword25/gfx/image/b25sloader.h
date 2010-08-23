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

#ifndef SWORD25_B25SLOADER_H
#define SWORD25_B25SLOADER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/image/imageloader.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

class B25SLoader : public ImageLoader {
public:
	static ImageLoader *CreateInstance() {
#include "sword25/kernel/memlog_off.h"
		return static_cast<ImageLoader *>(new B25SLoader());
#include "sword25/kernel/memlog_on.h"
	}

protected:
	virtual bool IsCorrectImageFormat(const byte *FileDataPtr, unsigned int FileSize);
	virtual bool DecodeImage(const byte *FileDataPtr, unsigned int FileSize,  GraphicEngine::COLOR_FORMATS ColorFormat, byte *&UncompressedDataPtr,
	                         int &Width, int &Height, int &Pitch);
	virtual bool ImageProperties(const byte *FileDataPtr, unsigned int FileSize, GraphicEngine::COLOR_FORMATS &ColorFormat, int &Width, int &Height);

};

} // End of namespace Sword25

#endif
