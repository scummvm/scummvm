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

#define BS_LOG_PREFIX "SCREENSHOT"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/gfx/screenshot.h"
#include <png.h>

using namespace std;

// -----------------------------------------------------------------------------

struct RGB_PIXEL
{
	RGB_PIXEL(unsigned char _Red, unsigned char _Green, unsigned char _Blue) :
		Red(_Red),
		Green(_Green),
		Blue(_Blue)
		{};

	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
};

bool BS_Screenshot::SaveToFile(unsigned int Width, unsigned int Height, const vector<unsigned int> & Data, const string & Filename)
{
	BS_ASSERT(Data.size() == Width * Height);

	// Buffer für Bildschirminhalt in RGB reservieren
	vector<RGB_PIXEL> PixelBuffer;
	PixelBuffer.reserve(Width * Height);

	// Framebufferdaten pixelweise von RGBA nach RGB konvertieren
	vector<unsigned int>::const_iterator it = Data.begin();
	for (unsigned int y = 0; y < Height; y++)
	{
		for (unsigned int x = 0; x < Width; x++)
		{
			unsigned int SrcPixel = *it++;
			PixelBuffer.push_back(RGB_PIXEL((SrcPixel >> 16) & 0xff, (SrcPixel >> 8) & 0xff, SrcPixel & 0xff));
		}
	}
	BS_ASSERT(it == Data.end());
	BS_ASSERT(Data.size() == PixelBuffer.size());

	// Variablen für die PNG-Erstellung
	FILE * OutFile = 0;
	png_structp png_ptr = 0;
	png_infop info_ptr = 0;

	try
	{
		OutFile = fopen(Filename.c_str(), "wb");
		if (!OutFile)
		{
			BS_LOG_ERRORLN("Could not create screenshot-file \"%s\".", Filename.c_str());
			throw(0);
		}

		png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr)
		{
			BS_LOG_ERRORLN("Could not create PNG write-struct.");
			throw(0);
		}

		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr)
		{
			BS_LOG_ERRORLN("Could not create PNG info-struct.");
			throw(0);
		}

		// Der Kompressionsbuffer muss groß genug sein um das gesamte Bild zu beinhalten.
		// Dieses stellt sicher, dass nur ein IDAT Chunk erstellt wird.
		// Als Buffergröße wird 110% der Rohdatengröße verwandt, um sicher zu gehen.
		png_set_compression_buffer_size(png_ptr, (Width * Height * 3 * 110) / 100);

		// PNG-Info Struktur initialisieren
		png_set_IHDR(png_ptr, info_ptr, 
			Width,							// Breite
			Height,							// Höhe
			8,								// Bittiefe pro Kanal
			PNG_COLOR_TYPE_RGB,				// Farbformat
			PNG_INTERLACE_NONE,				// Interlacing-Typ
			PNG_COMPRESSION_TYPE_DEFAULT,	// Kompressions-Typ
			PNG_FILTER_TYPE_DEFAULT);		// Filter-Typ

		// Rowpointer erstellen
		vector<png_bytep> RowPointers;
		RowPointers.reserve(Height);
		for (unsigned int i = 0; i < Height; i++)
		{
			RowPointers.push_back((png_bytep)(&PixelBuffer[Width * i]));
		}
		png_set_rows(png_ptr, info_ptr, &RowPointers[0]);

		png_init_io(png_ptr, OutFile);

		// Bild schreiben
		png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(OutFile);
	}

	catch (int)
	{
		// Wenn die Datei bereits erstellt wurde, Datei schließen und löschen.
		if (OutFile)
		{
			fclose(OutFile);
			remove(Filename.c_str());
		}

		if (info_ptr) png_destroy_write_struct(0, &info_ptr);
		if (png_ptr) png_destroy_write_struct(&png_ptr, (png_infopp) 0);

		BS_LOG_ERRORLN("Could not create screenshot (\"%s\").", Filename.c_str());
		return false;
	}

	return true;
}

// -----------------------------------------------------------------------------

bool BS_Screenshot::SaveThumbnailToFile(unsigned int Width, unsigned int Height, const vector<unsigned int> & Data, const string & Filename)
{
	//
	// Diese Methode nimmt ein Screenshot mit den Maßen von 800x600 und erzeugt einen Screenshot mit den Maßen von 200x125.
	// Dabei werden je 50 Pixel oben und unten abgeschnitten (die Interface-Leisten im Spiel). Das verbleibende Bild von 800x500 wird auf
	// ein 16tel seiner Größe reduziert, indem es in 4x4 Pixelblöcke ausgeteilt wird und der Durchschnitt jedes Blockes einen Pixel des Zielbildes generiert.
	// Abschließend wird das Ergebnis als PNG-Datei unter dem übergebenen Dateinamen gespeichert.
	//

	// Die Ausgangsgröße muss 800x600 sein.
	if (Width != 800 || Height != 600)
	{
		BS_LOG_ERRORLN("The sreenshot dimensions have to be 800x600 in order to be saved as a thumbnail.");
		return false;
	}
	
	// Buffer für die Zieldaten erstellen (RGBA Bild mit den Maßen 200x125).
	vector<unsigned int> ThumbnailData(200 * 125);

	// Über das Zielbild iterieren und einen Pixel zur Zeit berechnen.
	unsigned int x, y;
	x = y = 0;
	for(vector<unsigned int>::iterator Iter = ThumbnailData.begin(); Iter != ThumbnailData.end(); ++Iter)
	{
		// Durchschnitt über 4x4 Pixelblock im Quellbild bilden.
		unsigned int Alpha, Red, Green, Blue;
		Alpha = Red = Green = Blue = 0;
		for (unsigned int j = 0; j < 4; ++j)
		{
			for (unsigned int i = 0; i < 4; ++i)
			{
				unsigned int Pixel = Data[((y * 4) + j + 50) * 800 + ((x * 4) + i)];
				Alpha += (Pixel >> 24);
				Red += (Pixel >> 16) & 0xff;
				Green += (Pixel >> 8) & 0xff;
				Blue += Pixel & 0xff;
			}
		}

		// Zielpixel schreiben.
		*Iter = ((Alpha / 16) << 24) | ((Red / 16) << 16) | ((Green / 16) << 8) | (Blue / 16);

		// Mitzählen an welcher Stelle im Zielbild wir uns befinden.
		++x;
		if (x == 200)
		{
			x = 0;
			++y;
		}
	}

	// Bild als PNG Speichern.
	return SaveToFile(200, 125, ThumbnailData, Filename);
}
