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

// TODO:
// Entweder Fontfile absolut abspeichern, oder Verzeichniswechseln verbieten
// Eine relative Fontfile-Angabe könnte verwandt werden nachdem das Verzeichnis bereits gewechselt wurde und die Datei würde nicht mehr gefunden

#define BS_LOG_PREFIX "TEXT"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/gfx/fontresource.h"
#include "sword25/gfx/bitmapresource.h"

#include "sword25/gfx/text.h"


// -----------------------------------------------------------------------------
// Konstanten
// -----------------------------------------------------------------------------

namespace
{
	const unsigned int AUTO_WRAP_THRESHOLD_DEFAULT = 300;
}

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_Text::BS_Text(BS_RenderObjectPtr<BS_RenderObject> ParentPtr) :
	BS_RenderObject(ParentPtr, BS_RenderObject::TYPE_TEXT),
	m_ModulationColor(0xffffffff),
	m_AutoWrap(false),
	m_AutoWrapThreshold(AUTO_WRAP_THRESHOLD_DEFAULT)
{
	
}

// -----------------------------------------------------------------------------

BS_Text::BS_Text(BS_InputPersistenceBlock & Reader, BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Handle) :
	BS_RenderObject(ParentPtr, TYPE_TEXT, Handle)
{
	m_InitSuccess = Unpersist(Reader);
}

// -----------------------------------------------------------------------------

bool BS_Text::SetFont(const std::string & Font)
{
	// Font precachen.
	if (GetResourceManager()->PrecacheResource(Font))
	{
		m_Font = Font;
		UpdateFormat();
		ForceRefresh();
		return true;
	}
	else
	{
		BS_LOG_ERRORLN("Could not precache font \"%s\". Font probably does not exist.", Font.c_str());
		return false;
	}

}

// -----------------------------------------------------------------------------

void BS_Text::SetText(const std::string & Text)
{
	m_Text = Text;
	UpdateFormat();
	ForceRefresh();
}

// -----------------------------------------------------------------------------

void BS_Text::SetColor(unsigned int ModulationColor)
{
	unsigned int NewModulationColor = (ModulationColor & 0x00ffffff) | (m_ModulationColor & 0xff000000);
	if (NewModulationColor != m_ModulationColor)
	{
		m_ModulationColor = NewModulationColor;
		ForceRefresh();
	}
}

// -----------------------------------------------------------------------------

void BS_Text::SetAlpha(int Alpha)
{
	BS_ASSERT(Alpha >= 0 && Alpha < 256);
	unsigned int NewModulationColor = (m_ModulationColor & 0x00ffffff) | Alpha << 24;
	if (NewModulationColor != m_ModulationColor)
	{
		m_ModulationColor = NewModulationColor;
		ForceRefresh();
	}
}

// -----------------------------------------------------------------------------

void BS_Text::SetAutoWrap(bool AutoWrap)
{
	if (AutoWrap != m_AutoWrap)
	{
		m_AutoWrap = AutoWrap;
		UpdateFormat();
		ForceRefresh();
	}
}

// -----------------------------------------------------------------------------

void BS_Text::SetAutoWrapThreshold(unsigned int AutoWrapThreshold)
{
	if (AutoWrapThreshold != m_AutoWrapThreshold)
	{
		m_AutoWrapThreshold = AutoWrapThreshold;
		UpdateFormat();
		ForceRefresh();
	}
}

// -----------------------------------------------------------------------------

bool BS_Text::DoRender()
{
	// Font-Resource locken.
	BS_FontResource * FontPtr = LockFontResource();
	if (!FontPtr) return false;

	// Charactermap-Resource locken.
	BS_ResourceManager * RMPtr = GetResourceManager();
	BS_BitmapResource * CharMapPtr;
	{
		BS_Resource * pResource = RMPtr->RequestResource(FontPtr->GetCharactermapFileName());
		if (!pResource)
		{
			BS_LOG_ERRORLN("Could not request resource \"%s\".", FontPtr->GetCharactermapFileName().c_str());
			return false;
		}
		if (pResource->GetType() != BS_Resource::TYPE_BITMAP)
		{
			BS_LOG_ERRORLN("Requested resource \"%s\" is not a bitmap.", FontPtr->GetCharactermapFileName().c_str());
			return false;
		}

		CharMapPtr = static_cast<BS_BitmapResource*>(pResource);
	}

	// Framebufferobjekt holen.
	BS_GraphicEngine * GfxPtr = static_cast<BS_GraphicEngine *>(BS_Kernel::GetInstance()->GetService("gfx"));
	BS_ASSERT(GfxPtr);

	bool Result = true;
	std::vector<LINE>::iterator Iter = m_Lines.begin();
	for (; Iter != m_Lines.end(); ++Iter)
	{
		// Feststellen, ob überhaupt Buchstaben der aktuellen Zeile vom Update betroffen sind.
		BS_Rect CheckRect = (*Iter).BBox;
		CheckRect.Move(m_AbsoluteX, m_AbsoluteY);

		// Jeden Buchstaben einzeln Rendern.
		int CurX = m_AbsoluteX + (*Iter).BBox.left;
		int CurY = m_AbsoluteY + (*Iter).BBox.top;
		for (unsigned int i = 0; i < (*Iter).Text.size(); ++i)
		{
			BS_Rect CurRect = FontPtr->GetCharacterRect((unsigned char)(*Iter).Text.at(i));

			BS_Rect RenderRect(CurX, CurY, CurX + CurRect.GetWidth(), CurY + CurRect.GetHeight());
			int RenderX = CurX + (RenderRect.left - RenderRect.left);
			int RenderY = CurY + (RenderRect.top - RenderRect.top);
			RenderRect.Move(CurRect.left - CurX, CurRect.top - CurY);
			Result = CharMapPtr->Blit(RenderX, RenderY, BS_Image::FLIP_NONE, &RenderRect, m_ModulationColor);
			if (!Result) break;

			CurX += CurRect.GetWidth() + FontPtr->GetGapWidth();		
		}
	}

	// Charactermap-Resource freigeben.
	CharMapPtr->Release();

	// Font-Resource freigeben.
	FontPtr->Release();

	return Result;
}

// -----------------------------------------------------------------------------

BS_ResourceManager * BS_Text::GetResourceManager()
{
	// Pointer auf den Resource-Manager holen.
	return BS_Kernel::GetInstance()->GetResourceManager();
}

// -----------------------------------------------------------------------------

BS_FontResource * BS_Text::LockFontResource()
{
	BS_ResourceManager * RMPtr = GetResourceManager();

	// Font-Resource locken.
	BS_FontResource * FontPtr;
	{
		BS_Resource * ResourcePtr = RMPtr->RequestResource(m_Font);
		if (!ResourcePtr)
		{
			BS_LOG_ERRORLN("Could not request resource \"%s\".", m_Font.c_str());
			return NULL;
		}
		if (ResourcePtr->GetType() != BS_Resource::TYPE_FONT)
		{
			BS_LOG_ERRORLN("Requested resource \"%s\" is not a font.", m_Font.c_str());
			return NULL;
		}

		FontPtr = static_cast<BS_FontResource*>(ResourcePtr);
	}

	return FontPtr;
}

// -----------------------------------------------------------------------------

void BS_Text::UpdateFormat()
{
	BS_FontResource * FontPtr = LockFontResource();
	BS_ASSERT(FontPtr);

	UpdateMetrics(*FontPtr);

	m_Lines.resize(1);
	if (m_AutoWrap && (unsigned int) m_Width >= m_AutoWrapThreshold && m_Text.size() >= 2)
	{
		m_Width = 0;
		unsigned int CurLineWidth = 0;
		unsigned int CurLineHeight = 0;
		unsigned int CurLine = 0;
		unsigned int TempLineWidth = 0;
		unsigned int LastSpace = 0; // we need at least 1 space character to start a new line...
		m_Lines[0].Text = "";
		for (unsigned int i = 0; i < m_Text.size(); ++i)
		{
			unsigned int j;
			TempLineWidth = 0;
			LastSpace = 0;
			for (j = i; j < m_Text.size(); ++j)
			{
				if ((unsigned char)m_Text[j] == ' ') LastSpace = j;

				const BS_Rect & CurCharRect = FontPtr->GetCharacterRect((unsigned char)m_Text[j]);
				TempLineWidth += CurCharRect.GetWidth();
				TempLineWidth += FontPtr->GetGapWidth();
				
				if ((TempLineWidth >= m_AutoWrapThreshold) && (LastSpace > 0))
					break;
			}

			if (j == m_Text.size()) LastSpace = m_Text.size(); // everything in 1 line.

			CurLineWidth = 0;
			CurLineHeight = 0;
			for (j = i; j < LastSpace; ++j)
			{
				m_Lines[CurLine].Text += m_Text[j];

				const BS_Rect & CurCharRect = FontPtr->GetCharacterRect((unsigned char)m_Text[j]);
				CurLineWidth += CurCharRect.GetWidth();
				CurLineWidth += FontPtr->GetGapWidth();
				if ((unsigned int) CurCharRect.GetHeight() > CurLineHeight) CurLineHeight = CurCharRect.GetHeight();
			}			

			m_Lines[CurLine].BBox.right = CurLineWidth;
			m_Lines[CurLine].BBox.bottom = CurLineHeight;
			if ((unsigned int) m_Width < CurLineWidth) m_Width = CurLineWidth;
					
			if(LastSpace < m_Text.size())
			{
				++CurLine;
				BS_ASSERT(CurLine == m_Lines.size());
				m_Lines.resize(CurLine + 1);
				m_Lines[CurLine].Text = "";				
			}			
			
			i = LastSpace;
		}

		// Bounding-Box der einzelnen Zeilen relativ zur ersten festlegen (vor allem zentrieren).
		m_Height = 0;
		std::vector<LINE>::iterator Iter = m_Lines.begin();
		for (; Iter != m_Lines.end(); ++Iter)
		{
			BS_Rect & BBox = (*Iter).BBox;
			BBox.left = (m_Width - BBox.right) / 2;
			BBox.right = BBox.left + BBox.right;
			BBox.top =  (Iter - m_Lines.begin()) * FontPtr->GetLineHeight();
			BBox.bottom = BBox.top + BBox.bottom;
			m_Height += BBox.GetHeight();
		}
	}
	else
	{
		// Keine automatische Formatierung, also wird der gesamte Text in nur eine Zeile kopiert.
		m_Lines[0].Text = m_Text;
		m_Lines[0].BBox = BS_Rect(0, 0, m_Width, m_Height);
	}

	FontPtr->Release();
}

// -----------------------------------------------------------------------------

void BS_Text::UpdateMetrics(BS_FontResource & FontResource)
{
	m_Width = 0;
	m_Height = 0;

	for (unsigned int i = 0; i < m_Text.size(); ++i)
	{
		const BS_Rect & CurRect = FontResource.GetCharacterRect((unsigned char)m_Text.at(i));
		m_Width += CurRect.GetWidth();
		if (i != m_Text.size() - 1) m_Width += FontResource.GetGapWidth();
		if (m_Height < CurRect.GetHeight()) m_Height = CurRect.GetHeight();
	}
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool BS_Text::Persist(BS_OutputPersistenceBlock & Writer)
{
	bool Result = true;

	Result &= BS_RenderObject::Persist(Writer);

	Writer.Write(m_ModulationColor);
	Writer.Write(m_Font);
	Writer.Write(m_Text);
	Writer.Write(m_AutoWrap);
	Writer.Write(m_AutoWrapThreshold);

	Result &= BS_RenderObject::PersistChildren(Writer);

	return Result;
}

bool BS_Text::Unpersist(BS_InputPersistenceBlock & Reader)
{
	bool Result = true;

	Result &= BS_RenderObject::Unpersist(Reader);

	// Farbe und Alpha einlesen.
	Reader.Read(m_ModulationColor);

	// Beim Laden der anderen Member werden die Set-Methoden benutzt statt der tatsächlichen Member.
	// So wird das Layout automatisch aktualisiert und auch alle anderen notwendigen Methoden ausgeführt.

	std::string Font;
	Reader.Read(Font);
	SetFont(Font);

	std::string Text;
	Reader.Read(Text);
	SetText(Text);

	bool AutoWrap;
	Reader.Read(AutoWrap);
	SetAutoWrap(AutoWrap);

	unsigned int AutoWrapThreshold;
	Reader.Read(AutoWrapThreshold);
	SetAutoWrapThreshold(AutoWrapThreshold);

	Result &= BS_RenderObject::UnpersistChildren(Reader);

	return Reader.IsGood() && Result;
}
