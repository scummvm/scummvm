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
 */

#include "common/scummsys.h"
#include "common/error.h"
#include "common/fs.h"
#ifdef MACOSX
#include "common/macresman.h"
#endif
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"

#include "buried/buried.h"
#include "buried/database.h"

namespace Buried {

BuriedEngine::BuriedEngine(OSystem *syst, const BuriedGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_mainEXE = 0;
	_library = 0;
}

BuriedEngine::~BuriedEngine() {
	delete _mainEXE;
	delete _library;
}

Common::Error BuriedEngine::run() {
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

	return Common::kNoError;
}

Graphics::Font *BuriedEngine::createFont(int size) const {
	Common::SeekableReadStream *stream = 0;

	// HACK: Try to load the system font
	// TODO: MS Gothic for the Japanese version (please buy for clone2727)
	// Arial for everything else (???)
#if defined(WIN32)
	Common::FSNode fontPath("C:/WINDOWS/Fonts/arial.ttf");

	if (fontPath.exists() && !fontPath.isDirectory() && fontPath.isReadable())
		stream = fontPath.createReadStream();

	if (!stream) {
		Common::FSNode win2kFontPath("C:/WINNT/Fonts/arial.ttf");

		if (win2kFontPath.exists() && !win2kFontPath.isDirectory() && win2kFontPath.isReadable())
			stream = win2kFontPath.createReadStream();
	}
#elif defined(MACOSX)
	// Attempt to load the font from the Arial.ttf font first
	Common::FSNode fontPath("/Library/Fonts/Arial.ttf");

	if (fontPath.exists() && !fontPath.isDirectory() && fontPath.isReadable())
		stream = fontPath.createReadStream();

	if (!stream) {
		// Try the suitcase on the system
		Common::FSNode fontDirectory("/Library/Fonts");
		Common::MacResManager resFork;

		// DOUBLE HACK WARNING: Just assume it's 0x1000
		// (it should always be this, the first font, but parsing the FOND would be better)
		if (fontDirectory.exists() && fontDirectory.isDirectory() && resFork.open(fontPath, "Arial") && resFork.hasResFork())
			stream = resFork.getResource(MKTAG('s', 'f', 'n', 't'), 0x1000);

		// ...and one last try
		if (!stream) {
			Common::FSNode msFontDirectory("/Library/Fonts/Microsoft");
			if (fontDirectory.exists() && fontDirectory.isDirectory() && resFork.open(fontPath, "Arial") && resFork.hasResFork())
				stream = resFork.getResource(MKTAG('s', 'f', 'n', 't'), 0x1000);
		}
	}
#endif

	if (!stream) {
		// TODO: Try to load an equivalent font from the theme
		return 0;
	}

	// TODO: Make the monochrome mode optional
	// Win3.1 obviously only had raster fonts, but BIT Win3.1 will render
	// with the TrueType font on Win7/Win8 (at least)
	// TODO: The mapping is code page 1252, but it should be 1:1 to Unicode
	// for the characters we need.
	// TODO: shift-jis (code page 932) for the Japanese version (again, buy for clone2727)
	Graphics::Font *font = Graphics::loadTTFFont(*stream, size, 0, Graphics::kTTFRenderModeMonochrome, 0);
	delete stream;
	return font;
}

} // End of namespace Buried
