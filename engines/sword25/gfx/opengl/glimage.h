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

#ifndef SWORD25_GL_IMAGE_H
#define SWORD25_GL_IMAGE_H

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/graphicengine.h"

#include <vector>

namespace Sword25 {

// -----------------------------------------------------------------------------
// FORWARD DECLARATION
// -----------------------------------------------------------------------------

typedef void *GLS_Sprite;

// -----------------------------------------------------------------------------
// CLASS DEFINITION
// -----------------------------------------------------------------------------

class BS_GLImage : public BS_Image {
public:
	BS_GLImage(const Common::String &Filename, bool &Result);

	/**
	    @brief Erzeugt ein leeres BS_GLImage

	    @param Width die Breite des zu erzeugenden Bildes.
	    @param Height die Höhe des zu erzeugenden Bildes
	    @param Result gibt dem Aufrufer bekannt, ob der Konstruktor erfolgreich ausgeführt wurde. Wenn es nach dem Aufruf false enthalten sollte,
	                  dürfen keine Methoden am Objekt aufgerufen werden und das Objekt ist sofort zu zerstören.
	*/
	BS_GLImage(unsigned int Width, unsigned int Height, bool &Result);
	virtual ~BS_GLImage();

	virtual int GetWidth() const {
		return m_Width;
	}
	virtual int GetHeight() const {
		return m_Height;
	}
	virtual BS_GraphicEngine::COLOR_FORMATS GetColorFormat() const {
		return BS_GraphicEngine::CF_ARGB32;
	}

	virtual bool Blit(int PosX = 0, int PosY = 0,
	                  int Flipping = BS_Image::FLIP_NONE,
	                  BS_Rect *pPartRect = NULL,
	                  unsigned int Color = BS_ARGB(255, 255, 255, 255),
	                  int Width = -1, int Height = -1);
	virtual bool Fill(const BS_Rect *pFillRect, unsigned int Color);
	virtual bool SetContent(const byte *Pixeldata, uint size, unsigned int Offset = 0, unsigned int Stride = 0);
	virtual unsigned int GetPixel(int X, int Y);

	virtual bool IsBlitSource() const               {
		return true;
	}
	virtual bool IsBlitTarget() const               {
		return false;
	}
	virtual bool IsScalingAllowed() const           {
		return true;
	}
	virtual bool IsFillingAllowed() const           {
		return false;
	}
	virtual bool IsAlphaAllowed() const             {
		return true;
	}
	virtual bool IsColorModulationAllowed() const   {
		return true;
	}
	virtual bool IsSetContentAllowed() const        {
		return true;
	}
private:
	byte *_data;
	int         m_Width;
	int         m_Height;
};

} // End of namespace Sword25

#endif
