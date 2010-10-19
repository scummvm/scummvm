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
    imageloader_ids.h
    -----------------
    In dieser Datei sind alle ImageLoader verzeichnet.
    JEDER neuer ImageLoader muss hier eingetragen werden, ansonsten wird er beim Laden eines Bildes nicht berücksichtigt.

    Autor: Malte Thiesen
*/

#include "sword25/gfx/image/imageloader.h"

// Die Headerdateien der ImageLoader müssen hier eingebunden werden
#include "sword25/gfx/image/pngloader.h"
#include "sword25/gfx/image/b25sloader.h"

namespace Sword25 {

// Die Tabelle enthält Pointer auf statische Member-Funktionen innerhalb der Klassen, die eine Instanz der Klasse
// erzeugen
typedef ImageLoader*(*BS_IMAGELOADER_NEW)();
const BS_IMAGELOADER_NEW BS_IMAGELOADER_IDS[] = {
	PNGLoader::createInstance,
	B25SLoader::createInstance,
};
const int BS_IMAGELOADER_COUNT = sizeof(BS_IMAGELOADER_IDS) / sizeof(BS_IMAGELOADER_NEW);


} // End of namespace Sword25
