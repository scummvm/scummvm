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

#include "sword25/util/glsprites/glsprites.h"
#include "sword25/package/packagemanager.h"
#include "sword25/gfx/image/imageloader.h"
#include "sword25/gfx/opengl/openglgfx.h"
#include "sword25/gfx/opengl/glimage.h"

namespace Sword25 {

#define BS_LOG_PREFIX "GLIMAGE"

// -----------------------------------------------------------------------------
// CONSTRUCTION / DESTRUCTION
// -----------------------------------------------------------------------------

BS_GLImage::BS_GLImage(const Common::String &Filename, bool &Result) :
	m_Sprite(0),
	m_Width(0),
	m_Height(0) {
	Result = false;

	BS_PackageManager *pPackage = static_cast<BS_PackageManager *>(BS_Kernel::GetInstance()->GetService("package"));
	BS_ASSERT(pPackage);

	// Datei laden
	char *pFileData;
	unsigned int FileSize;
	if (!(pFileData = (char *) pPackage->GetFile(Filename, &FileSize))) {
		BS_LOG_ERRORLN("File \"%s\" could not be loaded.", Filename.c_str());
		return;
	}

	// Bildeigenschaften bestimmen
	BS_GraphicEngine::COLOR_FORMATS ColorFormat;
	int Pitch;
	if (!BS_ImageLoader::ExtractImageProperties(pFileData, FileSize, ColorFormat, m_Width, m_Height)) {
		BS_LOG_ERRORLN("Could not read image properties.");
		return;
	}

	// Das Bild dekomprimieren
	char *pUncompressedData;
	if (!BS_ImageLoader::LoadImage(pFileData, FileSize, BS_GraphicEngine::CF_ABGR32, pUncompressedData, m_Width, m_Height, Pitch)) {
		BS_LOG_ERRORLN("Could not decode image.");
		return;
	}

	// Dateidaten freigeben
	delete[] pFileData;

	// GLS-Sprite mit den Bilddaten erstellen
	GLS_Result GLSResult = GLS_NewSprite(m_Width, m_Height,
	                                     (ColorFormat == BS_GraphicEngine::CF_ARGB32) ? GLS_True : GLS_False,
	                                     pUncompressedData,
	                                     &m_Sprite);
	if (Result != GLS_OK) {
		BS_LOG_ERRORLN("Could not create GLS_Sprite. Reason: %s", GLS_ResultString(GLSResult));
		return;
	}

	// Bilddaten freigeben
	delete[] pUncompressedData;

	Result = true;
	return;
}

// -----------------------------------------------------------------------------

BS_GLImage::BS_GLImage(unsigned int Width, unsigned int Height, bool &Result) :
	m_Sprite(0),
	m_Width(Width),
	m_Height(Height) {
	Result = false;

	// GLS-Sprite mit den Bilddaten erstellen
	GLS_Result GLSResult = GLS_NewSprite(m_Width, m_Height,
	                                     GLS_True,
	                                     0,
	                                     &m_Sprite);
	if (GLSResult != GLS_OK) {
		BS_LOG_ERRORLN("Could not create GLS_Sprite. Reason: %s", GLS_ResultString(GLSResult));
		return;
	}

	Result = true;
	return;
}

// -----------------------------------------------------------------------------

BS_GLImage::~BS_GLImage() {
	if (m_Sprite) GLS_DeleteSprite(m_Sprite);
}

// -----------------------------------------------------------------------------

bool BS_GLImage::Fill(const BS_Rect *pFillRect, unsigned int Color) {
	BS_LOG_ERRORLN("Fill() is not supported.");
	return false;
}

// -----------------------------------------------------------------------------

bool BS_GLImage::SetContent(const byte *Pixeldata, unsigned int Offset, unsigned int Stride) {
	// Überprüfen, ob PixelData ausreichend viele Pixel enthält um ein Bild der Größe Width * Height zu erzeugen
	if (Pixeldata.size() < static_cast<unsigned int>(m_Width * m_Height * 4)) {
		BS_LOG_ERRORLN("PixelData vector is too small to define a 32 bit %dx%d image.", m_Width, m_Height);
		return false;
	}

	// GLS-Sprite mit den Bilddaten füllen
	GLS_Result GLSResult = GLS_SetSpriteData(m_Sprite, m_Width, m_Height, &Pixeldata[Offset], Stride / 4);
	if (GLSResult != GLS_OK) {
		BS_LOG_ERRORLN("CGLS_SetSpriteData() failed. Reason: %s", GLS_ResultString(GLSResult));
		return false;
	}

	return true;
}

// -----------------------------------------------------------------------------

unsigned int BS_GLImage::GetPixel(int X, int Y) {
	BS_LOG_ERRORLN("GetPixel() is not supported. Returning black.");
	return 0;
}

// -----------------------------------------------------------------------------

bool BS_GLImage::Blit(int PosX, int PosY,
                      int Flipping,
                      BS_Rect *pPartRect,
                      unsigned int Color,
                      int Width, int Height) {
	// BS_Rect nach GLS_Rect konvertieren
	GLS_Rect SubImage;
	if (pPartRect) {
		SubImage.x1 = pPartRect->left;
		SubImage.y1 = pPartRect->top;
		SubImage.x2 = pPartRect->right;
		SubImage.y2 = pPartRect->bottom;
	}

	// Farbe nach GLS_Color konvertieren
	GLS_Color GLSColor;
	GLSColor.a = Color >> 24;
	GLSColor.r = (Color >> 16) & 0xff;
	GLSColor.g = (Color >> 8) & 0xff;
	GLSColor.b = Color & 0xff;

	// Skalierungen berechnen
	GLS_Float ScaleX, ScaleY;
	if (Width == -1) Width = m_Width;
	ScaleX = (GLS_Float) Width / (GLS_Float) m_Width;

	if (Height == -1) Height = m_Height;
	ScaleY = (GLS_Float) Height / (GLS_Float) m_Height;

	// Rendern
	// TODO:
	// Die Bedeutung von FLIP_V und FLIP_H ist vertauscht. Allerdings glaubt der Rest der Engine auch daran, daher war es einfacher diesen Fehler
	// weiterzuführen. Bei Gelegenheit ist dieses aber zu ändern.
	GLS_Result Result = GLS_Blit(m_Sprite,
	                             PosX, PosY,
	                             pPartRect ? &SubImage : 0, &GLSColor,
	                             (Flipping & BS_Image::FLIP_V) ? GLS_True : GLS_False,
	                             (Flipping & BS_Image::FLIP_H) ? GLS_True : GLS_False,
	                             ScaleX, ScaleY);
	if (Result != GLS_OK) BS_LOG_ERRORLN("GLS_Blit() failed. Reason: %s", GLS_ResultString(Result));

	return Result == GLS_OK;
}

} // End of namespace Sword25
