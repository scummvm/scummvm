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

#include "sword25/gfx/image/imageloader.h"

#include "sword25/gfx/image/pngloader.h"
#include "sword25/gfx/image/b25sloader.h"

DECLARE_SINGLETON(Sword25::ImageLoaderManager)

namespace Sword25 {

#define BS_LOG_PREFIX "IMAGELOADER"


bool ImageLoaderManager::loadImage(const byte *pFileData, uint fileSize,
                            GraphicEngine::COLOR_FORMATS colorFormat,
                            byte *&pUncompressedData,
                            int &width, int &height,
                            int &pitch) {

	// Find a suitable ImageLoader and decode the image
	ImageLoader *pLoader = findSuitableImageLoader(pFileData, fileSize);
	if (pLoader) {
		return pLoader->decodeImage(pFileData, fileSize,
		                            colorFormat,
		                            pUncompressedData,
		                            width, height,
		                            pitch);
	}

	return false;
}

bool ImageLoaderManager::extractImageProperties(const byte *pFileData, uint fileSize,
        GraphicEngine::COLOR_FORMATS &colorFormat,
        int &width, int &height) {

	// Find a suitable ImageLoader and extra the image properties
	ImageLoader *pLoader = findSuitableImageLoader(pFileData, fileSize);
	if (pLoader) {
		return pLoader->imageProperties(pFileData, fileSize,
		                                colorFormat,
		                                width, height);
	}

	return false;
}

ImageLoaderManager::ImageLoaderManager() {
	_imageLoaderList.push_back(new PNGLoader());
	_imageLoaderList.push_back(new B25SLoader());
}

ImageLoaderManager::~ImageLoaderManager() {
	while (!_imageLoaderList.empty()) {
		delete _imageLoaderList.back();
		_imageLoaderList.pop_back();
	}
}

ImageLoader *ImageLoaderManager::findSuitableImageLoader(const byte *pFileData, uint fileSize) {
	// Iterate over all registered ImageLoader instances until we find
	// one that supports the image.
	Common::List<ImageLoader *>::iterator iter = _imageLoaderList.begin();
	for (; iter != _imageLoaderList.end(); ++iter) {
		if ((*iter)->isCorrectImageFormat(pFileData, fileSize)) {
			return (*iter);
		}
	}

	// No suitable ImageLoader was found :-(
	BS_LOG_ERRORLN("Could not find suitable image loader for image data.");
	return NULL;
}

} // End of namespace Sword25
