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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_named_object.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseNamedObject::BaseNamedObject(BaseGame *inGame) : BaseClass(inGame) {
	_name = nullptr;
}

//////////////////////////////////////////////////////////////////////////
BaseNamedObject::BaseNamedObject() : BaseClass() {
	_name = nullptr;
}


//////////////////////////////////////////////////////////////////////////
BaseNamedObject::BaseNamedObject(TDynamicConstructor, TDynamicConstructor) {
	_name = nullptr;
}

//////////////////////////////////////////////////////////////////////////
BaseNamedObject::~BaseNamedObject(void) {
	delete[] _name;
	_name = nullptr;
}


//////////////////////////////////////////////////////////////////////
void BaseNamedObject::setName(const char *name) {
	delete[] _name;
	_name = nullptr;

	if (name == nullptr) {
		return;
	}

	size_t nameSize = strlen(name) + 1;
	_name = new char [nameSize];
	Common::strcpy_s(_name, nameSize, name);
}

} // End of namespace Wintermute
