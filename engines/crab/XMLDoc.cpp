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

#include "crab/XMLDoc.h"

namespace rapidxml {
	void parse_error_handler(char const* what, void* where) {
		warning("RapidXML error handler: %s", what);
	}
}

namespace Crab {

void XMLDoc::load(const Common::String &filename) {
	const Common::Path path(filename);

	if (ready())
		_doc.clear();

	if (fileOpen(path, _text) && _text)
		_doc.parse<0>(_text);
}

const rapidxml::xml_document<> *XMLDoc::doc() const {
	if (_text != nullptr)
		return &_doc;
	else
		return NULL;
}

} // End of namespace Crab
