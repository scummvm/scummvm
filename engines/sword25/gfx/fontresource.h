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

#ifndef SWORD25_FONTRESOURCE_H
#define SWORD25_FONTRESOURCE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/resource.h"
#include "sword25/math/rect.h"

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

class BS_Kernel;
class TiXmlDocument;
class TiXmlElement;

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_FontResource : public BS_Resource
{
public:
	/**
		@brief Erzeugt eine neues Exemplar von BS_FontResource
		@param pKernel ein Pointer auf den Kernel
		@param FileName der Dateiname der zu ladenen Resource
		@remark Wenn der Konstruktor erfolgreich ausgeführt werden konnte gibt die Methode IsValid true zurück.
	*/
	BS_FontResource(BS_Kernel * pKernel, const std::string & FileName);

	/**
		@brief Gibt true zurück, wenn das Objekt korrekt initialisiert wurde.

		Diese Methode kann dazu benutzt werden um festzustellen, ob der Konstruktor erfolgreich ausgeführt wurde.
	*/
	bool	IsValid() const { return _Valid; }

	/**
		@brief Gibt die Zeilenhöhe des Fonts in Pixeln zurück.

		Die Zeilenhöhe ist der Wert, der zur Y-Koordinate addiert wird, wenn ein Zeilenumbruch auftritt.
	*/
	int		GetLineHeight() const { return _LineHeight; }

	/**
		@brief Gibt den Buchstabenabstand der Fonts in Pixeln zurück.

		Der Buchstabenabstand ist der Wert, der zwischen zwei Buchstaben freigelassen wird.
	*/
	int		GetGapWidth() const { return _GapWidth; }

	/**
		@brief Gibt das Bounding-Rect eines Zeichens auf der Charactermap zurück.
		@param Character der ASCII-Code des Zeichens
		@return Das Bounding-Rect des übergebenen Zeichens auf der Charactermap.
	*/
	const BS_Rect & GetCharacterRect(int Character) const { BS_ASSERT(Character >= 0 && Character < 256); return _CharacterRects[Character]; }

	/**
		@brief Gibt den Dateinamen der Charactermap zurück.
	*/
	const std::string & GetCharactermapFileName() const { return _BitmapFileName; }

private:
	BS_Kernel * _pKernel;
	bool		_Valid;
	std::string	_BitmapFileName;
	int			_LineHeight;
	int			_GapWidth;
	BS_Rect		_CharacterRects[256];

	// -----------------------------------------------------------------------------
	// Hilfsmethoden
	// -----------------------------------------------------------------------------
	
	bool _ParseXMLDocument(const std::string & FileName, TiXmlDocument & Doc) const;
	bool _ParseFontTag(TiXmlElement & Tag, std::string & BitmapFileName, int & LineHeight, int & GapWidth) const;
	bool _ParseCharacterTag(TiXmlElement & Tag, int & Code, BS_Rect & Rect) const;
};

#endif
