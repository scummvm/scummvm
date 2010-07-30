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

#ifndef SWORD25_SWIMAGE_H
#define SWORD25_SWIMAGE_H

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/graphicengine.h"


// -----------------------------------------------------------------------------
// CLASS DEFINITION
// -----------------------------------------------------------------------------

class BS_SWImage : public BS_Image
{
public:
	BS_SWImage(const std::string & Filename, bool & Result);
	virtual ~BS_SWImage();

	virtual int GetWidth() const { return m_Width; }
	virtual int GetHeight() const { return m_Height; }
	virtual BS_GraphicEngine::COLOR_FORMATS GetColorFormat() const { return BS_GraphicEngine::CF_ARGB32; }

	virtual bool Blit(int PosX = 0, int PosY = 0, 
					  int Flipping = BS_Image::FLIP_NONE, 
					  BS_Rect* pPartRect = NULL,
					  unsigned int Color = BS_ARGB(255, 255, 255, 255),
					  int Width = -1, int Height = -1);
	virtual bool Fill(const BS_Rect* FillRectPtr, unsigned int Color);
	virtual bool SetContent(const std::vector<unsigned char> & Pixeldata, unsigned int Offset, unsigned int Stride);
	virtual unsigned int GetPixel(int X, int Y);

	virtual bool IsBlitSource() const				{ return false; }
	virtual bool IsBlitTarget() const				{ return false; }
	virtual bool IsScalingAllowed() const			{ return false; }
	virtual bool IsFillingAllowed() const			{ return false; }
	virtual bool IsAlphaAllowed() const				{ return false; }
	virtual bool IsColorModulationAllowed() const	{ return false; }
	virtual bool IsSetContentAllowed() const		{ return false; }
private:
	unsigned int * _ImageDataPtr;

	int	m_Width;
	int m_Height;
};

#endif
