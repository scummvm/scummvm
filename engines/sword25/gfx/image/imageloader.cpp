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
#include "sword25/gfx/image/imageloader_ids.h"

namespace Sword25 {

#define BS_LOG_PREFIX "IMAGELOADER"

// Statische Elemente der Klasse BS_ImageLoader intialisieren.
Common::List<ImageLoader *> ImageLoader::_ImageLoaderList;
bool ImageLoader::_ImageLoaderListInitialized = false;

// Lade Methode
// ------------

bool ImageLoader::LoadImage(const byte *pFileData, unsigned int FileSize,
                               GraphicEngine::COLOR_FORMATS ColorFormat,
                               byte *&pUncompressedData,
                               int &Width, int &Height,
                               int &Pitch) {
	// Falls die Liste der BS_ImageLoader noch nicht initialisiert wurde, wird dies getan.
	if (!_ImageLoaderListInitialized)
		_InitializeLoaderList();

	// Passenden BS_ImageLoader finden und Bild dekodieren
	ImageLoader *pLoader = _FindSuitableImageLoader(pFileData, FileSize);
	if (pLoader) {
		return pLoader->DecodeImage(pFileData, FileSize,
		                            ColorFormat,
		                            pUncompressedData,
		                            Width, Height,
		                            Pitch);
	}

	return false;
}

// Info Methode
// ------------

bool ImageLoader::ExtractImageProperties(const byte *pFileData, unsigned int FileSize,
        GraphicEngine::COLOR_FORMATS &ColorFormat,
        int &Width, int &Height) {
	// Falls die Liste der BS_ImageLoader noch nicht initialisiert wurde, wird dies getan.
	if (!_ImageLoaderListInitialized)
		_InitializeLoaderList();

	// Passenden BS_ImageLoader finden und Bildeigenschaften auslesen.
	ImageLoader *pLoader = _FindSuitableImageLoader(pFileData, FileSize);
	if (pLoader) {
		return pLoader->ImageProperties(pFileData, FileSize,
		                                ColorFormat,
		                                Width, Height);
	}

	return false;
}

// Verwaltungs Methoden
// --------------------

void ImageLoader::_InitializeLoaderList() {
	// Von jedem BS_ImageLoader wird eine Instanz erzeugt, diese fügen sich selbständig in die BS_ImageLoader-Liste ein.
	for (int i = 0; i < BS_IMAGELOADER_COUNT; i++)
		BS_IMAGELOADER_IDS[i]();

	// Die Liste als gefüllt markieren.
	_ImageLoaderListInitialized = true;

	// Sicherstellen, dass beim Beenden alle BS_ImageLoader Instanzen zerstört werden.
	atexit(ImageLoader::_DeinitializeLoaderList);
}

void ImageLoader::_DeinitializeLoaderList() {
	while (!_ImageLoaderList.empty()) {
		delete _ImageLoaderList.back();
		_ImageLoaderList.pop_back();
	}
}

ImageLoader *ImageLoader::_FindSuitableImageLoader(const byte *pFileData, unsigned int FileSize) {
	// Alle BS_ImageLoader-Objekte durchgehen, bis eins gefunden wurde, dass das Bild laden kann
	Common::List<ImageLoader *>::iterator Iter = _ImageLoaderList.begin();
	for (; Iter != _ImageLoaderList.end(); ++Iter) {
		// Falls ein geeigneter BS-ImageLoader gefunden wurde, wird er zurückgegeben.
		if ((*Iter)->IsCorrectImageFormat(pFileData, FileSize)) {
			return (*Iter);
		}
	}

	// Es konnte kein passender BS_ImageLoader gefunden werden.
	BS_LOG_ERRORLN("Could not find suitable image loader for image data.");
	return NULL;
}

} // End of namespace Sword25
