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
// INCLUDES
// -----------------------------------------------------------------------------

#include "sword25/package/packagemanager.h"
#include "sword25/gfx/image/imageloader.h"
#include "sword25/gfx/opengl/openglgfx.h"
#include "sword25/gfx/opengl/glimage.h"

#include "common/system.h"

namespace Sword25 {

#define BS_LOG_PREFIX "GLIMAGE"

// -----------------------------------------------------------------------------
// CONSTRUCTION / DESTRUCTION
// -----------------------------------------------------------------------------

GLImage::GLImage(const Common::String &filename, bool &result) :
	_data(0),
	_width(0),
	_height(0) {
	result = false;

	PackageManager *pPackage = static_cast<PackageManager *>(Kernel::GetInstance()->GetService("package"));
	BS_ASSERT(pPackage);

	_backSurface = (static_cast<GraphicEngine *>(Kernel::GetInstance()->GetService("gfx")))->getSurface();

	// Datei laden
	byte *pFileData;
	uint fileSize;
	if (!(pFileData = (byte *)pPackage->GetFile(filename, &fileSize))) {
		BS_LOG_ERRORLN("File \"%s\" could not be loaded.", filename.c_str());
		return;
	}

	// Bildeigenschaften bestimmen
	GraphicEngine::COLOR_FORMATS colorFormat;
	int pitch;
	if (!ImageLoader::ExtractImageProperties(pFileData, fileSize, colorFormat, _width, _height)) {
		BS_LOG_ERRORLN("Could not read image properties.");
		return;
	}

	// Das Bild dekomprimieren
	if (!ImageLoader::LoadImage(pFileData, fileSize, GraphicEngine::CF_ARGB32, _data, _width, _height, pitch)) {
		BS_LOG_ERRORLN("Could not decode image.");
		return;
	}

	// Dateidaten freigeben
	delete[] pFileData;

	result = true;
	return;
}

// -----------------------------------------------------------------------------

GLImage::GLImage(uint width, uint height, bool &result) :
	_width(width),
	_height(height) {
	result = false;

	_data = new byte[width * height * 4];

	_backSurface = (static_cast<GraphicEngine *>(Kernel::GetInstance()->GetService("gfx")))->getSurface();

	result = true;
	return;
}

// -----------------------------------------------------------------------------

GLImage::~GLImage() {
	delete[] _data;
}

// -----------------------------------------------------------------------------

bool GLImage::fill(const Common::Rect *pFillRect, uint color) {
	BS_LOG_ERRORLN("Fill() is not supported.");
	return false;
}

// -----------------------------------------------------------------------------

bool GLImage::setContent(const byte *pixeldata, uint size, uint offset, uint stride) {
	// Überprüfen, ob PixelData ausreichend viele Pixel enthält um ein Bild der Größe Width * Height zu erzeugen
	if (size < static_cast<uint>(_width * _height * 4)) {
		BS_LOG_ERRORLN("PixelData vector is too small to define a 32 bit %dx%d image.", _width, _height);
		return false;
	}

	const byte *in = &pixeldata[offset];
	byte *out = _data;

	for (int i = 0; i < _height; i++) {
		memcpy(out, in, _width * 4);
		out += _width * 4;
		in += stride;
	}

	return true;
}

// -----------------------------------------------------------------------------

uint GLImage::getPixel(int x, int y) {
	BS_LOG_ERRORLN("GetPixel() is not supported. Returning black.");
	return 0;
}

// -----------------------------------------------------------------------------

bool GLImage::blit(int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height) {
	int x1 = 0, y1 = 0;
	int w = _width, h = _height;
	if (pPartRect) {
		x1 = pPartRect->left;
		y1 = pPartRect->top;
		w = pPartRect->right - pPartRect->left;
		h = pPartRect->bottom - pPartRect->top;
	}

	debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %d, %d, %d)", posX, posY, flipping, x1, y1, w, h, color, width, height);

	// Skalierungen berechnen
	float scaleX, scaleY;
	if (width == -1)
		width = _width;
	scaleX = (float)width / (float)_width;

	if (height == -1)
		height = _height;
	scaleY = (float)height / (float)_height;

	if ((color & 0xff000000) != 0xff000000) {
		warning("STUB: Image transparent bg color: %x", color);
	}
	int cr = (color >> 16) & 0xff;
	int cg = (color >> 8) & 0xff;
	int cb = (color >> 0) & 0xff;

	if (scaleX != 1.0 || scaleY != 1.0) {
		warning("STUB: Sprite scaling (%f x %f)", scaleX, scaleY);
	}

	if (posX < 0) {
		w -= posX;
		x1 = -posX;
		posX = 0;
	}

	if (posY < 0) {
		h -= posY;
		y1 = -posY;
		posY = 0;
	}

	w = CLIP(w, 0, MAX((int)_backSurface->w - posX - 1, 0));
	h = CLIP(h, 0, MAX((int)_backSurface->h - posY - 1, 0));

	if (w == 0 || h == 0)
		return true;

	// Rendern
	// TODO:
	// Die Bedeutung von FLIP_V und FLIP_H ist vertauscht. Allerdings glaubt der Rest der Engine auch daran, daher war es einfacher diesen Fehler
	// weiterzuführen. Bei Gelegenheit ist dieses aber zu ändern.

	// TODO: scaling
	int inStep = 4;
	int inoStep = _width * 4;
	if (flipping & Image::FLIP_V) {
		inStep = -inStep;
		x1 = x1 + w - 1;
	}

	if (flipping & Image::FLIP_H) {
		inoStep = -inoStep;
		y1 = y1 + h - 1;
	}

	byte *ino = &_data[y1 * _width * 4 + x1 * 4];
	byte *outo = (byte *)_backSurface->getBasePtr(posX, posY);
	byte *in, *out;

	for (int i = 0; i < h; i++) {
		out = outo;
		in = ino;
		for (int j = 0; j < w; j++) {
			int r = in[0];
			int g = in[1];
			int b = in[2];
			int a = in[3];
			in += inStep;

			switch (a) {
			case 0: // Full transparency
				out += 4;
				break;
			case 255: // Full opacity
				if (cr != 255)
					*out++ = (r * cr) >> 8;
				else
					*out++ = r;

				if (cg != 255)
					*out++ = (g * cg) >> 8;
				else
					*out++ = g;

				if (cb != 255)
					*out++ = (b * cb) >> 8;
				else
					*out++ = b;

				*out++ = a;
				break;
			default: // alpha blending
				if (cr != 255)
					*out += ((r - *out) * a * cr) >> 16;
				else
					*out += ((r - *out) * a) >> 8;
				out++;
				if (cg != 255)
					*out += ((g - *out) * a * cg) >> 16;
				else
					*out += ((g - *out) * a) >> 8;
				out++;
				if (cb != 255)
					*out += ((b - *out) * a * cb) >> 16;
				else
					*out += ((b - *out) * a) >> 8;
				out++;
				*out = 255;
				out++;
			}
		}
		outo += _backSurface->pitch;
		ino += inoStep;
	}

	g_system->copyRectToScreen((byte *)_backSurface->getBasePtr(posX, posY), _backSurface->pitch, posX, posY, w, h);

	return true;
}

} // End of namespace Sword25
