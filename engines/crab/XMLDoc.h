/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_XMLDOC_H
#define CRAB_XMLDOC_H

#include "crab/common_header.h"
#include "crab/filesystem.h"

namespace Crab {

class XMLDoc {
	rapidxml::xml_document<char> _doc;
	char *_text;

public:
	XMLDoc() {
		_text = NULL;
	}

	XMLDoc(const Common::String &filename) {
		_text = NULL;
		load(filename);
	}

	XMLDoc(uint8 *data) {
		_text = (char*)data;
		_doc.parse<0>(_text);
	}

	~XMLDoc() {
		delete[] _text;
	}

	// Load the text from the specified file into the rapidxml format
	// Each function that references it must parse it there
	void load(const Common::String &filename);

	// Check if document is ready for parsing
	bool ready() const {
		return _text != NULL;
	}

	const rapidxml::xml_document<> *doc() const;
};

} // End of namespace Crab

#endif // CRAB_XMLDOC_H
