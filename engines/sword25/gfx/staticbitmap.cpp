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

#include "sword25/gfx/staticbitmap.h"
#include "sword25/gfx/bitmapresource.h"
#include "sword25/package/packagemanager.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "STATICBITMAP"

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_StaticBitmap::BS_StaticBitmap(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, const Common::String& Filename) :
	BS_Bitmap(ParentPtr, TYPE_STATICBITMAP)
{
	// Das BS_Bitmap konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!m_InitSuccess) return;

	m_InitSuccess = InitBitmapResource(Filename);
}

// -----------------------------------------------------------------------------

BS_StaticBitmap::BS_StaticBitmap(BS_InputPersistenceBlock & Reader, BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Handle) :
	BS_Bitmap(ParentPtr, TYPE_STATICBITMAP, Handle)
{
	m_InitSuccess = Unpersist(Reader);
}

// -----------------------------------------------------------------------------

bool BS_StaticBitmap::InitBitmapResource(const Common::String & Filename)
{
	// Bild-Resource laden
	BS_Resource* ResourcePtr = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(Filename);
	if (!ResourcePtr)
	{
		BS_LOG_ERRORLN("Could not request resource \"%s\".", Filename.c_str());
		return false;
	}
	if (ResourcePtr->GetType() != BS_Resource::TYPE_BITMAP)
	{
		BS_LOG_ERRORLN("Requested resource \"%s\" is not a bitmap.", Filename.c_str());
		return false;
	}

	BS_BitmapResource * BitmapPtr = static_cast<BS_BitmapResource*>(ResourcePtr);

	// Den eindeutigen Dateinamen zum späteren Referenzieren speichern
	m_ResourceFilename = BitmapPtr->GetFileName();

	// RenderObject Eigenschaften aktualisieren
	m_OriginalWidth = m_Width = BitmapPtr->GetWidth();
	m_OriginalHeight = m_Height = BitmapPtr->GetHeight();

	// Bild-Resource freigeben
	BitmapPtr->Release();

	return true;
}

// -----------------------------------------------------------------------------

BS_StaticBitmap::~BS_StaticBitmap()
{
}

// -----------------------------------------------------------------------------

bool BS_StaticBitmap::DoRender()
{
	// Bitmap holen
	BS_Resource* ResourcePtr = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(m_ResourceFilename);
	BS_ASSERT(ResourcePtr);
	BS_ASSERT(ResourcePtr->GetType() == BS_Resource::TYPE_BITMAP);
	BS_BitmapResource * BitmapResourcePtr = static_cast<BS_BitmapResource*>(ResourcePtr);

	// Framebufferobjekt holen
	BS_GraphicEngine * GfxPtr = static_cast<BS_GraphicEngine *>(BS_Kernel::GetInstance()->GetService("gfx"));
	BS_ASSERT(GfxPtr);

	// Bitmap zeichnen
	bool Result;
	if (m_ScaleFactorX == 1.0f && m_ScaleFactorY == 1.0f)
	{
		Result = BitmapResourcePtr->Blit(m_AbsoluteX, m_AbsoluteY,
									   (m_FlipV ? BS_BitmapResource::FLIP_V : 0) |
									   (m_FlipH ? BS_BitmapResource::FLIP_H : 0),
									   0, m_ModulationColor, -1, -1);
	}
	else
	{
		Result = BitmapResourcePtr->Blit(m_AbsoluteX, m_AbsoluteY,
									   (m_FlipV ? BS_BitmapResource::FLIP_V : 0) |
									   (m_FlipH ? BS_BitmapResource::FLIP_H : 0),
									   0, m_ModulationColor, m_Width, m_Height);
	}

	// Resource freigeben
	BitmapResourcePtr->Release();

	return Result;
}

// -----------------------------------------------------------------------------

unsigned int BS_StaticBitmap::GetPixel(int X, int Y) const
{
	BS_ASSERT(X >= 0 && X < m_Width);
	BS_ASSERT(Y >= 0 && Y < m_Height);

	BS_Resource* pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(m_ResourceFilename);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	BS_BitmapResource* pBitmapResource = static_cast<BS_BitmapResource*>(pResource);
	unsigned int Result = pBitmapResource->GetPixel(X, Y);
	pResource->Release();
	return Result;
}

// -----------------------------------------------------------------------------

bool BS_StaticBitmap::SetContent(const std::vector<unsigned char> & Pixeldata, unsigned int Offset, unsigned int Stride)
{
	BS_LOG_ERRORLN("SetContent() ist not supported with this object.");
	return false;
}

// -----------------------------------------------------------------------------
// Auskunftsmethoden
// -----------------------------------------------------------------------------

bool BS_StaticBitmap::IsAlphaAllowed() const
{
	BS_Resource* pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(m_ResourceFilename);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	bool Result = static_cast<BS_BitmapResource*>(pResource)->IsAlphaAllowed();
	pResource->Release();
	return Result;
}

// -----------------------------------------------------------------------------

bool BS_StaticBitmap::IsColorModulationAllowed() const
{
	BS_Resource* pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(m_ResourceFilename);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	bool Result = static_cast<BS_BitmapResource*>(pResource)->IsColorModulationAllowed();
	pResource->Release();
	return Result;
}

// -----------------------------------------------------------------------------

bool BS_StaticBitmap::IsScalingAllowed() const
{
	BS_Resource* pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(m_ResourceFilename);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	bool Result = static_cast<BS_BitmapResource*>(pResource)->IsScalingAllowed();
	pResource->Release();
	return Result;
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool BS_StaticBitmap::Persist(BS_OutputPersistenceBlock & Writer)
{
	bool Result = true;

	Result &= BS_Bitmap::Persist(Writer);
	Writer.Write(m_ResourceFilename);

	Result &= BS_RenderObject::PersistChildren(Writer);

	return Result;
}

bool BS_StaticBitmap::Unpersist(BS_InputPersistenceBlock & Reader)
{
	bool Result = true;

	Result &= BS_Bitmap::Unpersist(Reader);
	Common::String ResourceFilename;
	Reader.Read(ResourceFilename);
	Result &= InitBitmapResource(ResourceFilename);

	Result &= BS_RenderObject::UnpersistChildren(Reader);

	return Reader.IsGood() && Result;
}

} // End of namespace Sword25
