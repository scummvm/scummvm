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

StaticBitmap::StaticBitmap(RenderObjectPtr<RenderObject> ParentPtr, const Common::String &Filename) :
	Bitmap(ParentPtr, TYPE_STATICBITMAP) {
	// Das BS_Bitmap konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!m_InitSuccess) return;

	m_InitSuccess = InitBitmapResource(Filename);
}

// -----------------------------------------------------------------------------

StaticBitmap::StaticBitmap(BS_InputPersistenceBlock &Reader, RenderObjectPtr<RenderObject> ParentPtr, unsigned int Handle) :
	Bitmap(ParentPtr, TYPE_STATICBITMAP, Handle) {
	m_InitSuccess = Unpersist(Reader);
}

// -----------------------------------------------------------------------------

bool StaticBitmap::InitBitmapResource(const Common::String &Filename) {
	// Bild-Resource laden
	BS_Resource *ResourcePtr = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(Filename);
	if (!ResourcePtr) {
		BS_LOG_ERRORLN("Could not request resource \"%s\".", Filename.c_str());
		return false;
	}
	if (ResourcePtr->GetType() != BS_Resource::TYPE_BITMAP) {
		BS_LOG_ERRORLN("Requested resource \"%s\" is not a bitmap.", Filename.c_str());
		return false;
	}

	BitmapResource *BitmapPtr = static_cast<BitmapResource *>(ResourcePtr);

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

StaticBitmap::~StaticBitmap() {
}

// -----------------------------------------------------------------------------

bool StaticBitmap::DoRender() {
	// Bitmap holen
	BS_Resource *ResourcePtr = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(m_ResourceFilename);
	BS_ASSERT(ResourcePtr);
	BS_ASSERT(ResourcePtr->GetType() == BS_Resource::TYPE_BITMAP);
	BitmapResource *BitmapResourcePtr = static_cast<BitmapResource *>(ResourcePtr);

	// Framebufferobjekt holen
	GraphicEngine *GfxPtr = static_cast<GraphicEngine *>(BS_Kernel::GetInstance()->GetService("gfx"));
	BS_ASSERT(GfxPtr);

	// Bitmap zeichnen
	bool Result;
	if (m_ScaleFactorX == 1.0f && m_ScaleFactorY == 1.0f) {
		Result = BitmapResourcePtr->Blit(m_AbsoluteX, m_AbsoluteY,
		                                 (m_FlipV ? BitmapResource::FLIP_V : 0) |
		                                 (m_FlipH ? BitmapResource::FLIP_H : 0),
		                                 0, m_ModulationColor, -1, -1);
	} else {
		Result = BitmapResourcePtr->Blit(m_AbsoluteX, m_AbsoluteY,
		                                 (m_FlipV ? BitmapResource::FLIP_V : 0) |
		                                 (m_FlipH ? BitmapResource::FLIP_H : 0),
		                                 0, m_ModulationColor, m_Width, m_Height);
	}

	// Resource freigeben
	BitmapResourcePtr->Release();

	return Result;
}

// -----------------------------------------------------------------------------

unsigned int StaticBitmap::GetPixel(int X, int Y) const {
	BS_ASSERT(X >= 0 && X < m_Width);
	BS_ASSERT(Y >= 0 && Y < m_Height);

	BS_Resource *pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(m_ResourceFilename);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	BitmapResource *pBitmapResource = static_cast<BitmapResource *>(pResource);
	unsigned int Result = pBitmapResource->GetPixel(X, Y);
	pResource->Release();
	return Result;
}

// -----------------------------------------------------------------------------

bool StaticBitmap::SetContent(const byte *Pixeldata, uint size, unsigned int Offset, unsigned int Stride) {
	BS_LOG_ERRORLN("SetContent() ist not supported with this object.");
	return false;
}

// -----------------------------------------------------------------------------
// Auskunftsmethoden
// -----------------------------------------------------------------------------

bool StaticBitmap::IsAlphaAllowed() const {
	BS_Resource *pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(m_ResourceFilename);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	bool Result = static_cast<BitmapResource *>(pResource)->IsAlphaAllowed();
	pResource->Release();
	return Result;
}

// -----------------------------------------------------------------------------

bool StaticBitmap::IsColorModulationAllowed() const {
	BS_Resource *pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(m_ResourceFilename);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	bool Result = static_cast<BitmapResource *>(pResource)->IsColorModulationAllowed();
	pResource->Release();
	return Result;
}

// -----------------------------------------------------------------------------

bool StaticBitmap::IsScalingAllowed() const {
	BS_Resource *pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(m_ResourceFilename);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	bool Result = static_cast<BitmapResource *>(pResource)->IsScalingAllowed();
	pResource->Release();
	return Result;
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool StaticBitmap::Persist(BS_OutputPersistenceBlock &Writer) {
	bool Result = true;

	Result &= Bitmap::Persist(Writer);
	Writer.Write(m_ResourceFilename);

	Result &= RenderObject::PersistChildren(Writer);

	return Result;
}

bool StaticBitmap::Unpersist(BS_InputPersistenceBlock &Reader) {
	bool Result = true;

	Result &= Bitmap::Unpersist(Reader);
	Common::String ResourceFilename;
	Reader.Read(ResourceFilename);
	Result &= InitBitmapResource(ResourceFilename);

	Result &= RenderObject::UnpersistChildren(Reader);

	return Reader.IsGood() && Result;
}

} // End of namespace Sword25
