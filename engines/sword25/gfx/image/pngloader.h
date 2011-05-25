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
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_PNGLOADER2_H
#define SWORD25_PNGLOADER2_H

#include "sword25/kernel/common.h"
#include "sword25/gfx/graphicengine.h"

namespace Sword25 {

// Define to use ScummVM's PNG decoder, instead of libpng
#define USE_INTERNAL_PNG_DECODER

/**
 * Class for loading PNG files, and PNG data embedded into savegames.
 *
 * Originally written by Malte Thiesen.
 */
class PNGLoader {
protected:
	PNGLoader() {}	// Protected constructor to prevent instances

	static bool doDecodeImage(const byte *fileDataPtr, uint fileSize, byte *&uncompressedDataPtr, int &width, int &height, int &pitch);
#ifndef USE_INTERNAL_PNG_DECODER
	static bool doImageProperties(const byte *fileDataPtr, uint fileSize, int &width, int &height);
#endif

public:

	/**
	 * Decode an image.
	 * @param[in] fileDatePtr	pointer to the image data
	 * @param[in] fileSize		size of the image data in bytes
	 * @param[out] pUncompressedData	if successful, this is set to a pointer containing the decoded image data
	 * @param[out] width		if successful, this is set to the width of the image
	 * @param[out] height		if successful, this is set to the height of the image
	 * @param[out] pitch		if successful, this is set to the number of bytes per scanline in the image
	 * @return false in case of an error
	 *
	 * @remark The size of the output data equals pitch * height.
	 * @remark This function does not free the image buffer passed to it,
	 *         it is the callers responsibility to do so.
	 */
	static bool decodeImage(const byte *pFileData, uint fileSize,
	                        byte *&pUncompressedData,
	                        int &width, int &height,
	                        int &pitch);

#ifndef USE_INTERNAL_PNG_DECODER
	/**
	 * Extract the properties of an image.
	 * @param[in] fileDatePtr	pointer to the image data
	 * @param[in] fileSize		size of the image data in bytes
	 * @param[out] width		if successful, this is set to the width of the image
	 * @param[out] height		if successful, this is set to the height of the image
	 * @return returns true if extraction of the properties was successful, false in case of an error
	 *
	 * @remark This function does not free the image buffer passed to it,
	 *         it is the callers responsibility to do so.
	 */
	static bool imageProperties(const byte *fileDatePtr, uint fileSize,
	                            int &width,
	                            int &height);
#endif

};

} // End of namespace Sword25

#endif
