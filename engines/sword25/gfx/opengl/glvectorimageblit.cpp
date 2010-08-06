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

#include "sword25/gfx/image/vectorimage.h"
#include "sword25/gfx/image/vectorimagerenderer.h"
#include "sword25/util/glsprites/glsprites.h"

#include <vector>

namespace Sword25 {

using namespace std;

#define BS_LOG_PREFIX "GLVECTORIMAGEBLIT"

// -----------------------------------------------------------------------------

namespace {
const float LINE_SCALE_FACTOR = 1.0f;
}

// -----------------------------------------------------------------------------

bool BS_VectorImage::Blit(int PosX, int PosY,
                          int Flipping,
                          BS_Rect *pPartRect,
                          unsigned int Color,
                          int Width, int Height) {
	static BS_VectorImageRenderer VectorImageRenderer;
	static vector<char> PixelData;
	static GLS_Sprite Sprite = 0;
	static BS_VectorImage *OldThis = 0;
	static int              OldWidth;
	static int              OldHeight;
	static GLS_Rect         OldSubImage;

	// Falls Breite oder Höhe 0 sind, muss nichts dargestellt werden.
	if (Width == 0 || Height == 0) return true;

	// Sprite erstellen, falls es noch nicht erstellt wurde
	if (Sprite == 0) {
		GLS_Result Result = GLS_NewSprite(512, 512, GLS_True, 0, &Sprite);
		if (Result != GLS_OK) {
			BS_LOG_ERRORLN("Could not create GLS_Sprite. Reason: %s", GLS_ResultString(Result));
			return false;
		}
	}

	// Feststellen, ob das alte Bild im Cache nicht wiederbenutzt werden kann und neu Berechnet werden muss
	if (!(OldThis == this && OldWidth == Width && OldHeight == Height && Sprite != 0)) {
		float ScaleFactorX = (Width == - 1) ? 1 : static_cast<float>(Width) / static_cast<float>(GetWidth());
		float ScaleFactorY = (Height == - 1) ? 1 : static_cast<float>(Height) / static_cast<float>(GetHeight());

		unsigned int RenderedWidth;
		unsigned int RenderedHeight;
		if (!VectorImageRenderer.Render(*this, ScaleFactorX, ScaleFactorY, RenderedWidth, RenderedHeight, PixelData, LINE_SCALE_FACTOR)) {
			BS_LOG_ERRORLN("Call to BS_VectorImageRenderer::Render() failed.");
			return false;
		}

		if (RenderedWidth > 512 || RenderedHeight > 512) {
			BS_LOG_WARNINGLN("Currently the maximum size for scaled vector images is 512x512.");
			return true;
		}

		GLS_Result Result = GLS_SetSpriteData(Sprite, RenderedWidth, RenderedHeight, &PixelData[0], 0);
		if (Result != GLS_OK) {
			BS_LOG_ERRORLN("Call to GLS_SetSpriteData() failed. Reason: %s", GLS_ResultString(Result));
			return false;
		}

		OldThis = this;
		OldHeight = Height;
		OldWidth = Width;

		OldSubImage.x1 = 0;
		OldSubImage.y1 = 0;
		OldSubImage.x2 = RenderedWidth;
		OldSubImage.y2 = RenderedHeight;
	}

	// Rendern
	// -------

	// pDest wird ignoriert. Es wird einfach angenommen, dass der Backbuffer gemeint ist, da nur auf den Backbuffer gerendert werden kann.
	// Ebenso werden pPartRect ignoriert. Es wird immer das gesamte Sprite gerendert.

	// Farbe nach GLS_Color konvertieren
	GLS_Color GLSColor;
	GLSColor.a = Color >> 24;
	GLSColor.r = (Color >> 16) & 0xff;
	GLSColor.g = (Color >> 8) & 0xff;
	GLSColor.b = Color & 0xff;

	// Rendern
	// TODO:
	// Die Bedeutung von FLIP_V und FLIP_H ist vertauscht. Allerdings glaubt der Rest der Engine auch daran, daher war es einfacher diesen Fehler
	// weiterzuführen. Bei Gelegenheit ist dieses aber zu ändern.
	GLS_Result Result = GLS_Blit(Sprite,
	                             PosX, PosY,
	                             &OldSubImage, &GLSColor,
	                             (Flipping & BS_Image::FLIP_V) ? GLS_True : GLS_False,
	                             (Flipping & BS_Image::FLIP_H) ? GLS_True : GLS_False,
	                             1.0f, 1.0f);
	if (Result != GLS_OK) BS_LOG_ERRORLN("GLS_Blit() failed. Reason: %s", GLS_ResultString(Result));

	return Result == GLS_OK;
}

} // End of namespace Sword25
