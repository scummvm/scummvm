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
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/gfx/xfile.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
XFile::XFile(BaseGame *inGame) : BaseClass(inGame) {
	_xfile = nullptr;
}

//////////////////////////////////////////////////////////////////////////
XFile::~XFile() {
	closeFile();
}

//////////////////////////////////////////////////////////////////////////
bool XFile::closeFile() {
	delete _xfile;
	_xfile = nullptr;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XFile::openFile(const Common::String &filename) {
	closeFile();

	// load file
	uint32 size;
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename, &size);
	if (!buffer) {
		closeFile();
		return false;
	}

	_xfile = new XFileLoader();
	if (!_xfile) {
		delete[] buffer;
		return false;
	}

	bool res = _xfile->load(buffer, size);
	delete[] buffer;
	if (!res) {
		BaseEngine::LOG(0, "Error loading X file '%s'", filename.c_str());
		return false;
	}

	// create enum object
	if (!res || !_xfile->createEnumObject(_xenum)) {
		BaseEngine::LOG(res, "Error creating XFile enum object for '%s'", filename.c_str());
		closeFile();
		return false;
	}

	return true;
}

} // namespace Wintermute
