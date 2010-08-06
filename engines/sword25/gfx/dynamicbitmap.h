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

#ifndef SWORD25_DYNAMIC_BITMAP_H
#define SWORD25_DYNAMIC_BITMAP_H


// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/memlog_off.h"
#include <memory>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"
#include "sword25/gfx/bitmap.h"
#include "sword25/gfx/opengl/glimage.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

class BS_DynamicBitmap : public BS_Bitmap
{
friend class BS_RenderObject;

public:
	virtual ~BS_DynamicBitmap();

	virtual unsigned int GetPixel(int X, int Y) const;

	virtual bool	SetContent(const byte *Pixeldata, unsigned int Offset, unsigned int Stride);

	virtual bool	IsScalingAllowed() const;
	virtual bool	IsAlphaAllowed() const;
	virtual	bool	IsColorModulationAllowed() const;
	virtual bool	IsSetContentAllowed() const;

	virtual bool	Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool	Unpersist(BS_InputPersistenceBlock & Reader);

protected:
	virtual bool	DoRender();

private:
	BS_DynamicBitmap(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Width, unsigned int Height);
	BS_DynamicBitmap(BS_InputPersistenceBlock & Reader, BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Handle);

	bool CreateGLImage(unsigned int Width, unsigned int Height);

	std::auto_ptr<BS_GLImage> m_Image;
};

} // End of namespace Sword25

#endif
