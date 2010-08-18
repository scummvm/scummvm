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

#include "sword25/gfx/dynamicbitmap.h"
#include "sword25/gfx/bitmapresource.h"
#include "sword25/package/packagemanager.h"
#include "sword25/kernel/inputpersistenceblock.h"

#include <vector>

namespace Sword25 {

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "DYNAMICBITMAP"

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

DynamicBitmap::DynamicBitmap(RenderObjectPtr<RenderObject> ParentPtr, unsigned int Width, unsigned int Height) :
	Bitmap(ParentPtr, TYPE_DYNAMICBITMAP) {
	// Das BS_Bitmap konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!m_InitSuccess) return;

	m_InitSuccess = CreateGLImage(Width, Height);
}

// -----------------------------------------------------------------------------

DynamicBitmap::DynamicBitmap(BS_InputPersistenceBlock &Reader, RenderObjectPtr<RenderObject> ParentPtr, unsigned int Handle) :
	Bitmap(ParentPtr, TYPE_DYNAMICBITMAP, Handle) {
	m_InitSuccess = Unpersist(Reader);
}

// -----------------------------------------------------------------------------

bool DynamicBitmap::CreateGLImage(unsigned int Width, unsigned int Height) {
	// GLImage mit den gewünschten Maßen erstellen
	bool Result;
	m_Image.reset(new GLImage(Width, Height, Result));

	m_OriginalWidth = m_Width = Width;
	m_OriginalHeight = m_Height = Height;

	return Result;
}

// -----------------------------------------------------------------------------

DynamicBitmap::~DynamicBitmap() {
}

// -----------------------------------------------------------------------------

unsigned int DynamicBitmap::GetPixel(int X, int Y) const {
	BS_ASSERT(X >= 0 && X < m_Width);
	BS_ASSERT(Y >= 0 && Y < m_Height);

	return m_Image->GetPixel(X, Y);
}

// -----------------------------------------------------------------------------

bool DynamicBitmap::DoRender() {
	// Framebufferobjekt holen
	GraphicEngine *pGfx = static_cast<GraphicEngine *>(BS_Kernel::GetInstance()->GetService("gfx"));
	BS_ASSERT(pGfx);

	// Bitmap zeichnen
	bool Result;
	if (m_ScaleFactorX == 1.0f && m_ScaleFactorY == 1.0f) {
		Result = m_Image->Blit(m_AbsoluteX, m_AbsoluteY,
		                       (m_FlipV ? BitmapResource::FLIP_V : 0) |
		                       (m_FlipH ? BitmapResource::FLIP_H : 0),
		                       0, m_ModulationColor, -1, -1);
	} else {
		Result = m_Image->Blit(m_AbsoluteX, m_AbsoluteY,
		                       (m_FlipV ? BitmapResource::FLIP_V : 0) |
		                       (m_FlipH ? BitmapResource::FLIP_H : 0),
		                       0, m_ModulationColor, m_Width, m_Height);
	}

	return Result;
}

// -----------------------------------------------------------------------------

bool DynamicBitmap::SetContent(const byte *Pixeldata, uint size, unsigned int Offset, unsigned int Stride) {
	return m_Image->SetContent(Pixeldata, size, Offset, Stride);
}

// -----------------------------------------------------------------------------
// Auskunftsmethoden
// -----------------------------------------------------------------------------

bool DynamicBitmap::IsScalingAllowed() const {
	return m_Image->IsScalingAllowed();
}

// -----------------------------------------------------------------------------

bool DynamicBitmap::IsAlphaAllowed() const {
	return m_Image->IsAlphaAllowed();
}

// -----------------------------------------------------------------------------

bool DynamicBitmap::IsColorModulationAllowed() const {
	return m_Image->IsColorModulationAllowed();
}

// -----------------------------------------------------------------------------

bool DynamicBitmap::IsSetContentAllowed() const {
	return true;
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool DynamicBitmap::Persist(BS_OutputPersistenceBlock &Writer) {
	bool Result = true;

	Result &= Bitmap::Persist(Writer);

	// Bilddaten werden nicht gespeichert. Dies ist auch nicht weiter von bedeutung, da BS_DynamicBitmap nur vom Videoplayer benutzt wird.
	// Während ein Video abläuft kann niemals gespeichert werden. BS_DynamicBitmap kann nur der Vollständigkeit halber persistiert werden.
	BS_LOG_WARNINGLN("Persisting a BS_DynamicBitmap. Bitmap content is not persisted.");

	Result &= RenderObject::PersistChildren(Writer);

	return Result;
}

bool DynamicBitmap::Unpersist(BS_InputPersistenceBlock &Reader) {
	bool Result = true;

	Result &= Bitmap::Unpersist(Reader);

	// Ein BS_GLImage mit den gespeicherten Maßen erstellen.
	Result &= CreateGLImage(m_Width, m_Height);

	// Bilddaten werden nicht gespeichert (s.o.).
	BS_LOG_WARNINGLN("Unpersisting a BS_DynamicBitmap. Bitmap contents are missing.");

	// Bild mit durchsichtigen Bilddaten initialisieren.
	byte *TransparentImageData = (byte *)calloc(m_Width * m_Height * 4, 1);
	m_Image->SetContent(TransparentImageData, m_Width * m_Height);
	free(TransparentImageData);

	Result &= RenderObject::UnpersistChildren(Reader);

	return Reader.IsGood() && Result;
}

} // End of namespace Sword25
