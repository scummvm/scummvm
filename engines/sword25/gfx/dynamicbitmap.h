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

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

class BS_DynamicBitmap : public BS_Bitmap
{
friend class BS_RenderObject;

public:
	virtual ~BS_DynamicBitmap();

	virtual unsigned int GetPixel(int X, int Y) const;

	virtual bool	SetContent(const std::vector<unsigned char> & Pixeldata, unsigned int Offset, unsigned int Stride);

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

#endif
