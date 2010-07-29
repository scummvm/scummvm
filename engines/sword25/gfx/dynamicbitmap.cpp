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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "dynamicbitmap.h"
#include "bitmapresource.h"
#include "package/packagemanager.h"
#include "kernel/inputpersistenceblock.h"

#include <vector>

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "DYNAMICBITMAP"

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_DynamicBitmap::BS_DynamicBitmap(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Width, unsigned int Height) :
	BS_Bitmap(ParentPtr, TYPE_DYNAMICBITMAP)
{
	// Das BS_Bitmap konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!m_InitSuccess) return;

	m_InitSuccess = CreateGLImage(Width, Height);
}

// -----------------------------------------------------------------------------

BS_DynamicBitmap::BS_DynamicBitmap(BS_InputPersistenceBlock & Reader, BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Handle) :
	BS_Bitmap(ParentPtr, TYPE_DYNAMICBITMAP, Handle)
{
	m_InitSuccess = Unpersist(Reader);
}

// -----------------------------------------------------------------------------

bool BS_DynamicBitmap::CreateGLImage(unsigned int Width, unsigned int Height)
{
	// GLImage mit den gewünschten Maßen erstellen
	bool Result;
	m_Image.reset(new BS_GLImage(Width, Height, Result));

	m_OriginalWidth = m_Width = Width;
	m_OriginalHeight = m_Height = Height;

	return Result;
}

// -----------------------------------------------------------------------------

BS_DynamicBitmap::~BS_DynamicBitmap()
{
}

// -----------------------------------------------------------------------------

unsigned int BS_DynamicBitmap::GetPixel(int X, int Y) const
{
	BS_ASSERT(X >= 0 && X < m_Width);
	BS_ASSERT(Y >= 0 && Y < m_Height);

	return m_Image->GetPixel(X, Y);
}

// -----------------------------------------------------------------------------

bool BS_DynamicBitmap::DoRender()
{
	// Framebufferobjekt holen
	BS_GraphicEngine * pGfx = static_cast<BS_GraphicEngine *>(BS_Kernel::GetInstance()->GetService("gfx"));
	BS_ASSERT(pGfx);

	// Bitmap zeichnen
	bool Result;
	if (m_ScaleFactorX == 1.0f && m_ScaleFactorY == 1.0f)
	{
		Result = m_Image->Blit(m_AbsoluteX, m_AbsoluteY,
							   (m_FlipV ? BS_BitmapResource::FLIP_V : 0) |
							   (m_FlipH ? BS_BitmapResource::FLIP_H : 0),
							   0, m_ModulationColor, -1, -1);
	}
	else
	{
		Result = m_Image->Blit(m_AbsoluteX, m_AbsoluteY,
							   (m_FlipV ? BS_BitmapResource::FLIP_V : 0) |
							   (m_FlipH ? BS_BitmapResource::FLIP_H : 0),
							   0, m_ModulationColor, m_Width, m_Height);
	}

	return Result;
}

// -----------------------------------------------------------------------------

bool BS_DynamicBitmap::SetContent(const std::vector<unsigned char> & Pixeldata, unsigned int Offset, unsigned int Stride)
{
	return m_Image->SetContent(Pixeldata, Offset, Stride);
}

// -----------------------------------------------------------------------------
// Auskunftsmethoden
// -----------------------------------------------------------------------------

bool BS_DynamicBitmap::IsScalingAllowed() const
{
	return m_Image->IsScalingAllowed();
}

// -----------------------------------------------------------------------------

bool BS_DynamicBitmap::IsAlphaAllowed() const
{
	return m_Image->IsAlphaAllowed();
}

// -----------------------------------------------------------------------------

bool BS_DynamicBitmap::IsColorModulationAllowed() const
{
	return m_Image->IsColorModulationAllowed();
}

// -----------------------------------------------------------------------------

bool BS_DynamicBitmap::IsSetContentAllowed() const
{
	return true;
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool BS_DynamicBitmap::Persist(BS_OutputPersistenceBlock & Writer)
{
	bool Result = true;

	Result &= BS_Bitmap::Persist(Writer);
	
	// Bilddaten werden nicht gespeichert. Dies ist auch nicht weiter von bedeutung, da BS_DynamicBitmap nur vom Videoplayer benutzt wird.
	// Während ein Video abläuft kann niemals gespeichert werden. BS_DynamicBitmap kann nur der Vollständigkeit halber persistiert werden.
	BS_LOG_WARNINGLN("Persisting a BS_DynamicBitmap. Bitmap content is not persisted.");

	Result &= BS_RenderObject::PersistChildren(Writer);

	return Result;
}

bool BS_DynamicBitmap::Unpersist(BS_InputPersistenceBlock & Reader)
{
	bool Result = true;

	Result &= BS_Bitmap::Unpersist(Reader);

	// Ein BS_GLImage mit den gespeicherten Maßen erstellen.
	Result &= CreateGLImage(m_Width, m_Height);

	// Bilddaten werden nicht gespeichert (s.o.).
	BS_LOG_WARNINGLN("Unpersisting a BS_DynamicBitmap. Bitmap contents are missing.");

	// Bild mit durchsichtigen Bilddaten initialisieren.
	std::vector<unsigned char> TransparentImageData(m_Width * m_Height * 4);
	m_Image->SetContent(TransparentImageData);

	Result &= BS_RenderObject::UnpersistChildren(Reader);

	return Reader.IsGood() && Result;
}
