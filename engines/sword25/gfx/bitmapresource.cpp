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

#include <memory>

#include "sword25/gfx/bitmapresource.h"
#include "sword25/kernel/kernel.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/image/imageloader.h"
#include "sword25/package/packagemanager.h"

#define BS_LOG_PREFIX "BITMAP"

// Konstruktion / Destruktion
// --------------------------

BS_BitmapResource::BS_BitmapResource(const std::string & Filename, BS_Image * pImage) :
	m_Valid(false),
	m_pImage(pImage),
	BS_Resource(Filename, BS_Resource::TYPE_BITMAP)
{
	m_Valid = m_pImage != 0;
}

BS_BitmapResource::~BS_BitmapResource()
{
	delete m_pImage;
}

// -----------------------------------------------------------------------------

unsigned int BS_BitmapResource::GetPixel(int X, int Y) const
{
	BS_ASSERT(X >= 0 && X < m_pImage->GetWidth());
	BS_ASSERT(Y >= 0 && Y < m_pImage->GetHeight());

	return m_pImage->GetPixel(X, Y);
}
