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

#include "sword25/gfx/image/image.h"
#include "sword25/gfx/image/pngloader.h"
#include <png.h>

namespace Sword25 {

#define BS_LOG_PREFIX "PNGLOADER"

// -----------------------------------------------------------------------------
// Konstruktor / Destruktor
// -----------------------------------------------------------------------------

PNGLoader::PNGLoader() {
}

// -----------------------------------------------------------------------------
// Laden
// -----------------------------------------------------------------------------

static void png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	memcpy(data, (char *)png_ptr->io_ptr, length);
	png_ptr->io_ptr = (void *)((png_size_t)png_ptr->io_ptr + length);
}

// -----------------------------------------------------------------------------

bool PNGLoader::DoDecodeImage(const byte *FileDataPtr, uint FileSize,  GraphicEngine::COLOR_FORMATS ColorFormat, byte *&UncompressedDataPtr,
                                 int &Width, int &Height, int &Pitch) {
	png_structp png_ptr = NULL;
	png_infop   info_ptr = NULL;
	png_bytep   RawDataBuffer = NULL;
	png_bytep  *pRowPtr = NULL;

	int         BitDepth;
	int         ColorType;
	int         InterlaceType;
	int         i;

	// Zielfarbformat überprüfen
	if (ColorFormat != GraphicEngine::CF_ARGB32) {
		BS_LOG_ERRORLN("Illegal or unsupported color format.");
		return false;
	}

	// PNG Signatur überprüfen
	if (!png_check_sig(reinterpret_cast<png_bytep>(const_cast<byte *>(FileDataPtr)), 8)) {
		error("png_check_sig failed");
	}

	// Die beiden PNG Strukturen erstellen
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		error("Could not create libpng read struct.");
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		error("Could not create libpng info struct.");
	}

	// Alternative Lesefunktion benutzen
	png_set_read_fn(png_ptr, (void *)FileDataPtr, png_user_read_data);

	// PNG Header einlesen
	png_read_info(png_ptr, info_ptr);

	// PNG Informationen auslesen
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&Width, (png_uint_32 *)&Height, &BitDepth, &ColorType, &InterlaceType, NULL, NULL);

	// Pitch des Ausgabebildes berechnen
	Pitch = GraphicEngine::CalcPitch(ColorFormat, Width);

	// Speicher für die endgültigen Bilddaten reservieren
	// Dieses geschieht vor dem reservieren von Speicher für temporäre Bilddaten um die Fragmentierung des Speichers gering zu halten
	UncompressedDataPtr = new byte[Pitch * Height];
	if (!UncompressedDataPtr) {
		error("Could not allocate memory for output image.");
	}

	// Bilder jeglicher Farbformate werden zunächst in ARGB Bilder umgewandelt
	if (BitDepth == 16)
		png_set_strip_16(png_ptr);
	if (ColorType == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	if (BitDepth < 8)
		png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);
	if (ColorType == PNG_COLOR_TYPE_GRAY ||
		ColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	png_set_bgr(png_ptr);

	if (ColorType != PNG_COLOR_TYPE_RGB_ALPHA)
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	// Nachdem die Transformationen registriert wurden, werden die Bilddaten erneut eingelesen
	png_read_update_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&Width, (png_uint_32 *)&Height, &BitDepth, &ColorType, NULL, NULL, NULL);

	// PNGs ohne Interlacing werden Zeilenweise eingelesen
	if (InterlaceType == PNG_INTERLACE_NONE) {
		// Speicher für eine Bildzeile reservieren
		RawDataBuffer = new png_byte[png_get_rowbytes(png_ptr, info_ptr)];
		if (!RawDataBuffer) {
			error("Could not allocate memory for row buffer.");
		}

		// Bilddaten zeilenweise einlesen und in das gewünschte Zielformat konvertieren
		for (i = 0; i < Height; i++) {
			// Zeile einlesen
			png_read_row(png_ptr, RawDataBuffer, NULL);

			// Zeile konvertieren
			switch (ColorFormat) {
			case GraphicEngine::CF_ARGB32:
				memcpy(&UncompressedDataPtr[i * Pitch], RawDataBuffer, Pitch);
				break;
			default:
				assert(0);
			}
		}
	} else {
		// PNGs mit Interlacing werden an einem Stück eingelesen
		// Speicher für das komplette Bild reservieren
		RawDataBuffer = new png_byte[png_get_rowbytes(png_ptr, info_ptr) * Height];
		if (!RawDataBuffer) {
			error("Could not allocate memory for raw image buffer.");
		}

		// Speicher für die Rowpointer reservieren
		pRowPtr = new png_bytep[Height];
		if (!pRowPtr) {
			error("Could not allocate memory for row pointers.");
		}

		// Alle Rowpointer mit den richtigen Offsets initialisieren
		for (i = 0; i < Height; i++)
			pRowPtr[i] = RawDataBuffer + i * png_get_rowbytes(png_ptr, info_ptr);

		// Bild einlesen
		png_read_image(png_ptr, pRowPtr);

		// Bilddaten zeilenweise in das gewünschte Ausgabeformat konvertieren
		switch (ColorFormat) {
		case GraphicEngine::CF_ARGB32:
			for (i = 0; i < Height; i++)
				memcpy(&UncompressedDataPtr[i * Pitch], &RawDataBuffer[i * png_get_rowbytes(png_ptr, info_ptr)], Pitch);
			break;
		default:
			error("Unhandled case in DoDecodeImage");
			break;
		}
	}
	
	// Die zusätzlichen Daten am Ende des Bildes lesen
	png_read_end(png_ptr, NULL);

	// Die Strukturen freigeben
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	// Temporäre Buffer freigeben
	delete[] pRowPtr;
	delete[] RawDataBuffer;

	// Der Funktionsaufruf war erfolgreich
	return true;
}

// -----------------------------------------------------------------------------

bool PNGLoader::DecodeImage(const byte *FileDataPtr, uint FileSize,  GraphicEngine::COLOR_FORMATS ColorFormat, byte *&UncompressedDataPtr,
                               int &Width, int &Height, int &Pitch) {
	return DoDecodeImage(FileDataPtr, FileSize, ColorFormat, UncompressedDataPtr, Width, Height, Pitch);
}

// -----------------------------------------------------------------------------

bool PNGLoader::DoImageProperties(const byte *FileDataPtr, uint FileSize, GraphicEngine::COLOR_FORMATS &ColorFormat, int &Width, int &Height) {
	// PNG Signatur überprüfen
	if (!DoIsCorrectImageFormat(FileDataPtr, FileSize)) return false;

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;

	// Die beiden PNG Strukturen erstellen
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		error("Could not create libpng read struct.");
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		error("Could not create libpng info struct.");
	}

	// Alternative Lesefunktion benutzen
	png_set_read_fn(png_ptr, (void *)FileDataPtr, png_user_read_data);

	// PNG Header einlesen
	png_read_info(png_ptr, info_ptr);

	// PNG Informationen auslesen
	int BitDepth;
	int ColorType;
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&Width, (png_uint_32 *)&Height, &BitDepth, &ColorType, NULL, NULL, NULL);

	// PNG-ColorType in BS ColorFormat konvertieren.
	if (ColorType & PNG_COLOR_MASK_ALPHA || png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		ColorFormat = GraphicEngine::CF_ARGB32;
	else
		ColorFormat = GraphicEngine::CF_RGB24;

	// Die Strukturen freigeben
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return true;

}

// -----------------------------------------------------------------------------

bool PNGLoader::ImageProperties(const byte *FileDataPtr, uint FileSize,  GraphicEngine::COLOR_FORMATS &ColorFormat, int &Width, int &Height) {
	return DoImageProperties(FileDataPtr, FileSize, ColorFormat, Width, Height);
}

// -----------------------------------------------------------------------------
// Header überprüfen
// -----------------------------------------------------------------------------

bool PNGLoader::DoIsCorrectImageFormat(const byte *FileDataPtr, uint FileSize) {
	if (FileSize > 8)
		return png_check_sig(const_cast<byte *>(FileDataPtr), 8) ? true : false;
	else
		return false;
}

// -----------------------------------------------------------------------------

bool PNGLoader::IsCorrectImageFormat(const byte *FileDataPtr, uint FileSize) {
	return DoIsCorrectImageFormat(FileDataPtr, FileSize);
}

} // End of namespace Sword25
