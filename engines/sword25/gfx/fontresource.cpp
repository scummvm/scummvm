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

#define BS_LOG_PREFIX "FONTRESOURCE"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <memory>

#include "kernel/kernel.h"
#include "kernel/string.h"
#include "package/packagemanager.h"
#include "util/tinyxml/tinyxml.h"

#include "fontresource.h"

// -----------------------------------------------------------------------------
// Konstanten
// -----------------------------------------------------------------------------

static const unsigned int DEFAULT_LINEHEIGHT = 20;
static const unsigned int DEFAULT_GAPWIDTH = 1;

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_FontResource::BS_FontResource(BS_Kernel* pKernel, const std::string& FileName) :
	_pKernel(pKernel),
	_Valid(false),
	BS_Resource(FileName, BS_Resource::TYPE_FONT)
{
	// XML Fontdatei parsen
	TiXmlDocument Doc;
	if (!_ParseXMLDocument(FileName, Doc))
	{
		BS_LOG_ERRORLN("The following TinyXML-Error occured while parsing \"%s\": %s", GetFileName().c_str(), Doc.ErrorDesc());
		return;
	}

	// Font-Tag finden
	TiXmlElement* pElement = Doc.FirstChildElement("font");
	if (!pElement)
	{
		BS_LOG_ERRORLN("No <font> tag found in \"%s\".", GetFileName().c_str());
		return;
	}

	// Font-Tag parsen
	std::string BitmapFileName;
	if (!_ParseFontTag(*pElement, BitmapFileName, _LineHeight, _GapWidth))
	{
		BS_LOG_ERRORLN("An error occurred while parsing <font> tag in \"%s\".", GetFileName().c_str());
		return;
	}

	// Absoluten, eindeutigen Pfad zur Bitmapdatei bestimmen und dabei auf vorhandensein prüfen
	{
		// Pointer auf den Package-Manager bekommen
		BS_ASSERT(_pKernel);
		BS_PackageManager* pPackage = static_cast<BS_PackageManager *>(_pKernel->GetService("package"));
		BS_ASSERT(pPackage);

		// Absoluten, eindeutigen Pfad bestimmen
		_BitmapFileName = pPackage->GetAbsolutePath(BitmapFileName);
		if (_BitmapFileName == "")
		{
			BS_LOG_ERRORLN("Image file \"%s\" was specified in <font> tag of \"%s\" but could not be found.",
						   _BitmapFileName.c_str(), GetFileName().c_str());
			return;
		}

		// Bitmapdatei cachen
		if (!_pKernel->GetResourceManager()->PrecacheResource(_BitmapFileName))
		{
			BS_LOG_ERRORLN("Could not precache \"%s\".", _BitmapFileName.c_str());
			return;
		}
	}
	
	// Das Erste Character-Tag finden
	pElement = pElement->FirstChildElement("character");
	if (!pElement)
	{
		BS_LOG_ERRORLN("No <character> tag found in \"%s\".", GetFileName().c_str());
		return;
	}

	// Alle Character-Tags parsen
	while (pElement)
	{
		int		CharCode;
		BS_Rect CharRect;

		// Aktuelles Character-Tag parsen
		if (!_ParseCharacterTag(*pElement, CharCode, CharRect))
		{
			BS_LOG_ERRORLN("An error occured while parsing a <character> tag in \"%s\".", GetFileName().c_str());
			return;
		}

		// Ausgelesene Daten in das _CharacterRects-Array eintragen
		BS_ASSERT(CharCode < 256);
		_CharacterRects[CharCode] = CharRect;

		// Zum nächsten Character-Tag iterieren
		pElement = pElement->NextSiblingElement("character");
	}

	// Erfolg signalisieren
	_Valid = true;
}

// -----------------------------------------------------------------------------

bool BS_FontResource::_ParseXMLDocument(const std::string & FileName, TiXmlDocument & Doc) const
{
	// Pointer auf den Package-Manager bekommen
	BS_ASSERT(_pKernel);
	BS_PackageManager* pPackage = static_cast<BS_PackageManager *>(_pKernel->GetService("package"));
	BS_ASSERT(pPackage);

	// Die Daten werden zunächst über den Package-Manager gelesen und dann in einen um ein Byte größeren Buffer kopiert
	// und NULL-Terminiert, da TinyXML NULL-Terminierte Daten benötigt.
	unsigned int FileSize;
	char * LoadBuffer = (char*) pPackage->GetFile(GetFileName(), &FileSize);
	if (!LoadBuffer)
	{
		BS_LOG_ERRORLN("Could not read \"%s\".", GetFileName().c_str());
		return false;
	}

	// Daten kopieren und NULL-terminieren
	std::vector<char> WorkBuffer(FileSize + 1);
	memcpy(&WorkBuffer[0], LoadBuffer, FileSize);
	delete LoadBuffer;
	WorkBuffer[FileSize] = '\0';

	// Daten parsen
	Doc.Parse(&WorkBuffer[0]);

	return !Doc.Error();
}

// -----------------------------------------------------------------------------

bool BS_FontResource::_ParseFontTag(TiXmlElement & Tag, std::string & BitmapFileName, int & Lineheight, int & GapWidth) const
{
	// Bitmap Attribut auslesen
	const char * BitmapString = Tag.Attribute("bitmap");
	if (!BitmapString)
	{
		BS_LOG_ERRORLN("<font> tag without bitmap attribute occurred in \"%s\".", GetFileName().c_str());
		return false;
	}
	BitmapFileName = BitmapString;

	// Lineheight Attribut auslesen
	const char * LineheightString = Tag.Attribute("lineheight");
	if (!LineheightString || !BS_String::ToInt(std::string(LineheightString), Lineheight) || Lineheight < 0)
	{
		BS_LOG_WARNINGLN("Illegal or missing lineheight attribute in <font> tag in \"%s\". Assuming default (\"%d\").",
						 GetFileName().c_str(), DEFAULT_LINEHEIGHT);
		Lineheight = DEFAULT_LINEHEIGHT;
	}


	// Gap Attribut auslesen
	const char * GapString = Tag.Attribute("gap");
	if (!GapString || !BS_String::ToInt(std::string(GapString), GapWidth) || GapWidth < 0)
	{
		BS_LOG_WARNINGLN("Illegal or missing gap attribute in <font> tag in \"%s\". Assuming default (\"%d\").",
						 GetFileName().c_str(), DEFAULT_GAPWIDTH);
		GapWidth = DEFAULT_GAPWIDTH;
	}

	return true;
}

// -----------------------------------------------------------------------------

bool BS_FontResource::_ParseCharacterTag(TiXmlElement & Tag, int & Code, BS_Rect & Rect) const
{
	// Code Attribut auslesen
	const char * CodeString = Tag.Attribute("code");
	if (!CodeString || !BS_String::ToInt(std::string(CodeString), Code) || Code < 0 || Code >= 256)
	{
		BS_LOG_ERRORLN("Illegal or missing code attribute in <character> tag in \"%s\".", GetFileName().c_str());
		return false;
	}

	// Left Attribut auslesen
	const char * LeftString = Tag.Attribute("left");
	if (!LeftString || !BS_String::ToInt(std::string(LeftString), Rect.left) || Rect.left < 0)
	{
		BS_LOG_ERRORLN("Illegal or missing left attribute in <character> tag in \"%s\".", GetFileName().c_str());
		return false;
	}

	// Right Attribut auslesen
	const char * RightString = Tag.Attribute("right");
	if (!RightString || !BS_String::ToInt(RightString, Rect.right) || Rect.right < 0)
	{
		BS_LOG_ERRORLN("Illegal or missing right attribute in <character> tag in \"%s\".", GetFileName().c_str());
		return false;
	}

	// Top Attribut auslesen
	const char * TopString = Tag.Attribute("top");
	if (!TopString || !BS_String::ToInt(TopString, Rect.top) || Rect.top < 0)
	{
		BS_LOG_ERRORLN("Illegal or missing top attribute in <character> tag in \"%s\".", GetFileName().c_str());
		return false;
	}

	// Bottom Attribut auslesen
	const char * BottomString = Tag.Attribute("bottom");
	if (!BottomString || !BS_String::ToInt(BottomString, Rect.bottom) || Rect.bottom < 0)
	{
		BS_LOG_ERRORLN("Illegal or missing bottom attribute in <character> tag in \"%s\".", GetFileName().c_str());
		return false;
	}

	return true;
}
