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

#define BS_LOG_PREFIX "FONTRESOURCE"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/string.h"
#include "sword25/package/packagemanager.h"

#include "sword25/gfx/fontresource.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Konstanten
// -----------------------------------------------------------------------------

static const uint DEFAULT_LINEHEIGHT = 20;
static const uint DEFAULT_GAPWIDTH = 1;

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

FontResource::FontResource(Kernel *pKernel, const Common::String &FileName) :
	_pKernel(pKernel),
	_Valid(false),
	Resource(FileName, Resource::TYPE_FONT),
	Common::XMLParser() {

	// Get a pointer to the package manager
	BS_ASSERT(_pKernel);
	PackageManager *pPackage = static_cast<PackageManager *>(_pKernel->GetService("package"));
	BS_ASSERT(pPackage);

	// Load the contents of the file
	uint fileSize;
	char *xmlData = pPackage->GetXmlFile(GetFileName(), &fileSize);
	if (!xmlData) {
		BS_LOG_ERRORLN("Could not read \"%s\".", GetFileName().c_str());
		return;
	}

	// Parse the contents
	if (!loadBuffer((const byte *)xmlData, fileSize))
		return;

	_Valid = parse();
	close();
	free(xmlData);
}

// -----------------------------------------------------------------------------

bool FontResource::parserCallback_font(ParserNode *node) {
	// Get the attributes of the font
	Common::String bitmapFilename = node->values["bitmap"];

	if (!parseIntegerKey(node->values["lineheight"].c_str(), 1, &_LineHeight)) {
		BS_LOG_WARNINGLN("Illegal or missing lineheight attribute in <font> tag in \"%s\". Assuming default (\"%d\").",
		                 GetFileName().c_str(), DEFAULT_LINEHEIGHT);
		_LineHeight = DEFAULT_LINEHEIGHT;
	}

	if (!parseIntegerKey(node->values["gap"].c_str(), 1, &_GapWidth)) {
		BS_LOG_WARNINGLN("Illegal or missing gap attribute in <font> tag in \"%s\". Assuming default (\"%d\").",
		                 GetFileName().c_str(), DEFAULT_GAPWIDTH);
		_GapWidth = DEFAULT_GAPWIDTH;
	}
	
	// Get a reference to the package manager
	BS_ASSERT(_pKernel);
	PackageManager *pPackage = static_cast<PackageManager *>(_pKernel->GetService("package"));
	BS_ASSERT(pPackage);

	// Get the full path and filename for the bitmap resource
	_BitmapFileName = pPackage->GetAbsolutePath(bitmapFilename);
	if (_BitmapFileName == "") {
		BS_LOG_ERRORLN("Image file \"%s\" was specified in <font> tag of \"%s\" but could not be found.",
		               _BitmapFileName.c_str(), GetFileName().c_str());
	}

	// Pre-cache the resource
	if (!_pKernel->GetResourceManager()->PrecacheResource(_BitmapFileName)) {
		BS_LOG_ERRORLN("Could not precache \"%s\".", _BitmapFileName.c_str());
	}

	return true;
}

// -----------------------------------------------------------------------------

bool FontResource::parserCallback_character(ParserNode *node) {
	// Get the attributes of the character
	int charCode, top, left, right, bottom;

	if (!parseIntegerKey(node->values["code"].c_str(), 1, &charCode) || (charCode < 0) || (charCode >= 256)) {
		return parserError("Illegal or missing code attribute in <character> tag in \"%s\".", GetFileName().c_str());
	}

	if (!parseIntegerKey(node->values["top"].c_str(), 1, &top) || (top < 0)) {
		return parserError("Illegal or missing top attribute in <character> tag in \"%s\".", GetFileName().c_str());
	}
	if (!parseIntegerKey(node->values["left"].c_str(), 1, &left) || (left < 0)) {
		return parserError("Illegal or missing left attribute in <character> tag in \"%s\".", GetFileName().c_str());
	}
	if (!parseIntegerKey(node->values["right"].c_str(), 1, &right) || (right < 0)) {
		return parserError("Illegal or missing right attribute in <character> tag in \"%s\".", GetFileName().c_str());
	}
	if (!parseIntegerKey(node->values["bottom"].c_str(), 1, &bottom) || (bottom < 0)) {
		return parserError("Illegal or missing bottom attribute in <character> tag in \"%s\".", GetFileName().c_str());
	}

	this->_CharacterRects[charCode] = Common::Rect(left, top, right, bottom);
	return true;
}

} // End of namespace Sword25
