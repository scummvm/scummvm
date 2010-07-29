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

/*
	imageloader_ids.h
	-----------------
	In dieser Datei sind alle ImageLoader verzeichnet.
	JEDER neuer ImageLoader muss hier eingetragen werden, ansonsten wird er beim Laden eines Bildes nicht berücksichtigt.

	Autor: Malte Thiesen
*/

#include "imageloader.h"

// Die Headerdateien der ImageLoader müssen hier eingebunden werden
#include "pngloader.h"
#include "b25sloader.h"

// Die Tabelle enthält Pointer auf statische Member-Funktionen innerhalb der Klassen, die eine Instanz der Klasse
// erzeugen
typedef BS_ImageLoader* (*BS_IMAGELOADER_NEW)();
const BS_IMAGELOADER_NEW BS_IMAGELOADER_IDS[] =
{
	BS_PNGLoader::CreateInstance,
	BS_B25SLoader::CreateInstance,
};
const int BS_IMAGELOADER_COUNT = sizeof(BS_IMAGELOADER_IDS) / sizeof(BS_IMAGELOADER_NEW);

