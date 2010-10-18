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

// TODO:
// Entweder Fontfile absolut abspeichern, oder Verzeichniswechseln verbieten
// Eine relative Fontfile-Angabe könnte verwandt werden nachdem das Verzeichnis bereits gewechselt wurde und die Datei würde nicht mehr gefunden

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/gfx/fontresource.h"
#include "sword25/gfx/bitmapresource.h"

#include "sword25/gfx/text.h"

namespace Sword25 {

#define BS_LOG_PREFIX "TEXT"

// -----------------------------------------------------------------------------
// Konstanten
// -----------------------------------------------------------------------------

namespace {
const uint AUTO_WRAP_THRESHOLD_DEFAULT = 300;
}

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

Text::Text(RenderObjectPtr<RenderObject> ParentPtr) :
	RenderObject(ParentPtr, RenderObject::TYPE_TEXT),
	_modulationColor(0xffffffff),
	m_AutoWrap(false),
	m_AutoWrapThreshold(AUTO_WRAP_THRESHOLD_DEFAULT) {

}

// -----------------------------------------------------------------------------

Text::Text(InputPersistenceBlock &Reader, RenderObjectPtr<RenderObject> ParentPtr, uint Handle) :
		RenderObject(ParentPtr, TYPE_TEXT, Handle),
		// Temporarily set fields prior to unpersisting actual values
		_modulationColor(0xffffffff),
		m_AutoWrap(false),
		m_AutoWrapThreshold(AUTO_WRAP_THRESHOLD_DEFAULT) {	

	// Unpersist the fields
	_initSuccess = unpersist(Reader);
}

// -----------------------------------------------------------------------------

bool Text::SetFont(const Common::String &Font) {
	// Font precachen.
	if (GetResourceManager()->PrecacheResource(Font)) {
		m_Font = Font;
		UpdateFormat();
		forceRefresh();
		return true;
	} else {
		BS_LOG_ERRORLN("Could not precache font \"%s\". Font probably does not exist.", Font.c_str());
		return false;
	}

}

// -----------------------------------------------------------------------------

void Text::SetText(const Common::String &text) {
	m_Text = text;
	UpdateFormat();
	forceRefresh();
}

// -----------------------------------------------------------------------------

void Text::setColor(uint modulationColor) {
	uint newModulationColor = (modulationColor & 0x00ffffff) | (_modulationColor & 0xff000000);
	if (newModulationColor != _modulationColor) {
		_modulationColor = newModulationColor;
		forceRefresh();
	}
}

// -----------------------------------------------------------------------------

void Text::setAlpha(int alpha) {
	BS_ASSERT(alpha >= 0 && alpha < 256);
	uint newModulationColor = (_modulationColor & 0x00ffffff) | alpha << 24;
	if (newModulationColor != _modulationColor) {
		_modulationColor = newModulationColor;
		forceRefresh();
	}
}

// -----------------------------------------------------------------------------

void Text::SetAutoWrap(bool AutoWrap) {
	if (AutoWrap != m_AutoWrap) {
		m_AutoWrap = AutoWrap;
		UpdateFormat();
		forceRefresh();
	}
}

// -----------------------------------------------------------------------------

void Text::SetAutoWrapThreshold(uint AutoWrapThreshold) {
	if (AutoWrapThreshold != m_AutoWrapThreshold) {
		m_AutoWrapThreshold = AutoWrapThreshold;
		UpdateFormat();
		forceRefresh();
	}
}

// -----------------------------------------------------------------------------

bool Text::doRender() {
	// Font-Resource locken.
	FontResource *FontPtr = LockFontResource();
	if (!FontPtr) return false;

	// Charactermap-Resource locken.
	ResourceManager *RMPtr = GetResourceManager();
	BitmapResource *CharMapPtr;
	{
		Resource *pResource = RMPtr->RequestResource(FontPtr->GetCharactermapFileName());
		if (!pResource) {
			BS_LOG_ERRORLN("Could not request resource \"%s\".", FontPtr->GetCharactermapFileName().c_str());
			return false;
		}
		if (pResource->GetType() != Resource::TYPE_BITMAP) {
			BS_LOG_ERRORLN("Requested resource \"%s\" is not a bitmap.", FontPtr->GetCharactermapFileName().c_str());
			return false;
		}

		CharMapPtr = static_cast<BitmapResource *>(pResource);
	}

	// Framebufferobjekt holen.
	GraphicEngine *GfxPtr = Kernel::GetInstance()->GetGfx();
	BS_ASSERT(GfxPtr);

	bool Result = true;
	Common::Array<LINE>::iterator Iter = m_Lines.begin();
	for (; Iter != m_Lines.end(); ++Iter) {
		// Feststellen, ob überhaupt Buchstaben der aktuellen Zeile vom Update betroffen sind.
		Common::Rect CheckRect = (*Iter).BBox;
		CheckRect.translate(_absoluteX, _absoluteY);

		// Jeden Buchstaben einzeln Rendern.
		int CurX = _absoluteX + (*Iter).BBox.left;
		int CurY = _absoluteY + (*Iter).BBox.top;
		for (uint i = 0; i < (*Iter).Text.size(); ++i) {
			Common::Rect CurRect = FontPtr->GetCharacterRect((byte)(*Iter).Text[i]);

			Common::Rect RenderRect(CurX, CurY, CurX + CurRect.width(), CurY + CurRect.height());
			int RenderX = CurX + (RenderRect.left - RenderRect.left);
			int RenderY = CurY + (RenderRect.top - RenderRect.top);
			RenderRect.translate(CurRect.left - CurX, CurRect.top - CurY);
			Result = CharMapPtr->blit(RenderX, RenderY, Image::FLIP_NONE, &RenderRect, _modulationColor);
			if (!Result) break;

			CurX += CurRect.width() + FontPtr->GetGapWidth();
		}
	}

	// Charactermap-Resource freigeben.
	CharMapPtr->release();

	// Font-Resource freigeben.
	FontPtr->release();

	return Result;
}

// -----------------------------------------------------------------------------

ResourceManager *Text::GetResourceManager() {
	// Pointer auf den Resource-Manager holen.
	return Kernel::GetInstance()->GetResourceManager();
}

// -----------------------------------------------------------------------------

FontResource *Text::LockFontResource() {
	ResourceManager *RMPtr = GetResourceManager();

	// Font-Resource locken.
	FontResource *FontPtr;
	{
		Resource *ResourcePtr = RMPtr->RequestResource(m_Font);
		if (!ResourcePtr) {
			BS_LOG_ERRORLN("Could not request resource \"%s\".", m_Font.c_str());
			return NULL;
		}
		if (ResourcePtr->GetType() != Resource::TYPE_FONT) {
			BS_LOG_ERRORLN("Requested resource \"%s\" is not a font.", m_Font.c_str());
			return NULL;
		}

		FontPtr = static_cast<FontResource *>(ResourcePtr);
	}

	return FontPtr;
}

// -----------------------------------------------------------------------------

void Text::UpdateFormat() {
	FontResource *FontPtr = LockFontResource();
	BS_ASSERT(FontPtr);

	UpdateMetrics(*FontPtr);

	m_Lines.resize(1);
	if (m_AutoWrap && (uint) _width >= m_AutoWrapThreshold && m_Text.size() >= 2) {
		_width = 0;
		uint CurLineWidth = 0;
		uint CurLineHeight = 0;
		uint CurLine = 0;
		uint TempLineWidth = 0;
		uint LastSpace = 0; // we need at least 1 space character to start a new line...
		m_Lines[0].Text = "";
		for (uint i = 0; i < m_Text.size(); ++i) {
			uint j;
			TempLineWidth = 0;
			LastSpace = 0;
			for (j = i; j < m_Text.size(); ++j) {
				if ((byte)m_Text[j] == ' ') LastSpace = j;

				const Common::Rect &CurCharRect = FontPtr->GetCharacterRect((byte)m_Text[j]);
				TempLineWidth += CurCharRect.width();
				TempLineWidth += FontPtr->GetGapWidth();

				if ((TempLineWidth >= m_AutoWrapThreshold) && (LastSpace > 0))
					break;
			}

			if (j == m_Text.size()) LastSpace = m_Text.size(); // everything in 1 line.

			CurLineWidth = 0;
			CurLineHeight = 0;
			for (j = i; j < LastSpace; ++j) {
				m_Lines[CurLine].Text += m_Text[j];

				const Common::Rect &CurCharRect = FontPtr->GetCharacterRect((byte)m_Text[j]);
				CurLineWidth += CurCharRect.width();
				CurLineWidth += FontPtr->GetGapWidth();
				if ((uint) CurCharRect.height() > CurLineHeight) CurLineHeight = CurCharRect.height();
			}

			m_Lines[CurLine].BBox.right = CurLineWidth;
			m_Lines[CurLine].BBox.bottom = CurLineHeight;
			if ((uint) _width < CurLineWidth) _width = CurLineWidth;

			if (LastSpace < m_Text.size()) {
				++CurLine;
				BS_ASSERT(CurLine == m_Lines.size());
				m_Lines.resize(CurLine + 1);
				m_Lines[CurLine].Text = "";
			}

			i = LastSpace;
		}

		// Bounding-Box der einzelnen Zeilen relativ zur ersten festlegen (vor allem zentrieren).
		_height = 0;
		Common::Array<LINE>::iterator Iter = m_Lines.begin();
		for (; Iter != m_Lines.end(); ++Iter) {
			Common::Rect &BBox = (*Iter).BBox;
			BBox.left = (_width - BBox.right) / 2;
			BBox.right = BBox.left + BBox.right;
			BBox.top = (Iter - m_Lines.begin()) * FontPtr->GetLineHeight();
			BBox.bottom = BBox.top + BBox.bottom;
			_height += BBox.height();
		}
	} else {
		// Keine automatische Formatierung, also wird der gesamte Text in nur eine Zeile kopiert.
		m_Lines[0].Text = m_Text;
		m_Lines[0].BBox = Common::Rect(0, 0, _width, _height);
	}

	FontPtr->release();
}

// -----------------------------------------------------------------------------

void Text::UpdateMetrics(FontResource &fontResource) {
	_width = 0;
	_height = 0;

	for (uint i = 0; i < m_Text.size(); ++i) {
		const Common::Rect &CurRect = fontResource.GetCharacterRect((byte)m_Text[i]);
		_width += CurRect.width();
		if (i != m_Text.size() - 1) _width += fontResource.GetGapWidth();
		if (_height < CurRect.height()) _height = CurRect.height();
	}
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool Text::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	result &= RenderObject::persist(writer);

	writer.write(_modulationColor);
	writer.write(m_Font);
	writer.write(m_Text);
	writer.write(m_AutoWrap);
	writer.write(m_AutoWrapThreshold);

	result &= RenderObject::persistChildren(writer);

	return result;
}

bool Text::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	result &= RenderObject::unpersist(reader);

	// Farbe und Alpha einlesen.
	reader.read(_modulationColor);

	// Beim Laden der anderen Member werden die Set-Methoden benutzt statt der tatsächlichen Member.
	// So wird das Layout automatisch aktualisiert und auch alle anderen notwendigen Methoden ausgeführt.

	Common::String Font;
	reader.read(Font);
	SetFont(Font);

	Common::String text;
	reader.read(text);
	SetText(text);

	bool AutoWrap;
	reader.read(AutoWrap);
	SetAutoWrap(AutoWrap);

	uint AutoWrapThreshold;
	reader.read(AutoWrapThreshold);
	SetAutoWrapThreshold(AutoWrapThreshold);

	result &= RenderObject::unpersistChildren(reader);

	return reader.isGood() && result;
}

} // End of namespace Sword25
