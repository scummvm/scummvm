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

#include "sword25/gfx/bitmapresource.h"
#include "sword25/kernel/kernel.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/image/imageloader.h"
#include "sword25/package/packagemanager.h"

namespace Sword25 {

#define BS_LOG_PREFIX "BITMAP"

// Konstruktion / Destruktion
// --------------------------

BitmapResource::BitmapResource(const Common::String &Filename, Image *pImage) :
	m_Valid(false),
	m_pImage(pImage),
	Resource(Filename, Resource::TYPE_BITMAP) {
	m_Valid = m_pImage != 0;
}

BitmapResource::~BitmapResource() {
	delete m_pImage;
}

// -----------------------------------------------------------------------------

unsigned int BitmapResource::GetPixel(int X, int Y) const {
	BS_ASSERT(X >= 0 && X < m_pImage->GetWidth());
	BS_ASSERT(Y >= 0 && Y < m_pImage->GetHeight());

	return m_pImage->GetPixel(X, Y);
}

} // End of namespace Sword25
