/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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
 */

#include "common/scummsys.h"
#include "common/error.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/util.h"

#include "buried/buried.h"
#include "buried/database.h"
#include "buried/graphics.h"
#include "buried/sound.h"

namespace Buried {

BuriedEngine::BuriedEngine(OSystem *syst, const BuriedGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_gfx = 0;
	_mainEXE = 0;
	_library = 0;
	_sound = 0;
}

BuriedEngine::~BuriedEngine() {
	delete _gfx;
	delete _mainEXE;
	delete _library;
	delete _sound;
}

Common::Error BuriedEngine::run() {
	if (isTrueColor()) {
#ifndef USE_RGB_COLOR
		// Can't play 24bpp version without support
		return Common::kUnsupportedColorMode;
#else
		initGraphics(640, 480, true, 0);

		if (_system->getScreenFormat().bytesPerPixel == 1)
			return Common::kUnsupportedColorMode;
#endif
	} else {
		initGraphics(640, 480, true);
	}

	if (isWin95()) {
		error("TODO: Win95 version");
	} else if (isCompressed()) {
		_mainEXE = new DatabaseNECompressed();
		_library = new DatabaseNECompressed();
	} else {
		_mainEXE = new DatabaseNE();

		// Demo only uses the main EXE
		if (!isDemo())
			_library = new DatabaseNE();
	}

	if (!_mainEXE->load(getEXEName()))
		error("Failed to load main EXE '%s'", getEXEName().c_str());

	if (_library && !_library->load(getLibraryName()))
		error("Failed to load library DLL '%s'", getLibraryName().c_str());

	_gfx = new GraphicsManager(this);
	_sound = new SoundManager(this);

	return Common::kNoError;
}

Common::String BuriedEngine::getFilePath(uint32 stringID) {
	Common::String path = _mainEXE->loadString(stringID);
	Common::String output;

	if (path.empty())
		return output;

	uint i = 0;

	// The non-demo paths have CD info followed by a backspace
	// We ignore this
	if (!isDemo())
		i += 2;

	for (; i < path.size(); i++) {
		if (path[i] == '\\')
			output += '/';
		else
			output += path[i];
	}

	return output;
}

Common::SeekableReadStream *BuriedEngine::getBitmapStream(uint32 bitmapID) {
	// The demo's bitmaps are in the main EXE
	if (isDemo())
		return _mainEXE->getBitmapStream(bitmapID);

	// The rest in the database library
	return _library->getBitmapStream(bitmapID);
}

} // End of namespace Buried
