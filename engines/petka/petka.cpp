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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug-channels.h"
#include "common/error.h"
#include "common/ini-file.h"
#include "common/stream.h"

#include "engines/util.h"

#include "petka/file_mgr.h"
#include "petka/petka.h"
#include "petka/q_manager.h"

namespace Petka {

PetkaEngine::PetkaEngine(OSystem *system, const ADGameDescription *desc)
	: Engine(system), _console(nullptr), _fileMgr(nullptr), _resMgr(nullptr), _desc(desc) {
	DebugMan.addDebugChannel(kPetkaDebugGeneral, "general", "General issues");
	_part = 0;
	_chapter = 0;
}

PetkaEngine::~PetkaEngine() {
	DebugMan.clearAllDebugChannels();
}

Common::Error PetkaEngine::run() {
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(640, 480, &format);

	_console.reset(new Console(this));
	_fileMgr.reset(new FileMgr());
	_resMgr.reset(new QManager(*this));

	loadStores();
	if (!_resMgr->init())
		return Common::kNoGameDataFoundError;

	return Common::kNoError;
}

Common::SeekableReadStream *PetkaEngine::openFile(const Common::String &name, bool addCurrentPath) {
	return _fileMgr->getFileStream(addCurrentPath ? _currentPath + name : name);
}

void PetkaEngine::loadStores() {
	_fileMgr->closeAll();

	_fileMgr->openStore("patch.str");
	_fileMgr->openStore("main.str");

	Common::INIFile parts;
	Common::ScopedPtr<Common::SeekableReadStream> stream(_fileMgr->getFileStream("PARTS.INI"));

	if (!stream || !parts.loadFromStream(*stream)) {
		return;
	}

	const char *const names[] = {"Background", "Flics", "Wavs", "SFX", "Music", "Speech"};
	const Common::String section = Common::String::format("Part %d", _part);

	parts.getKey("CurrentPath", section, _currentPath);
	parts.getKey("PathSpeech", section, _speechPath);

	Common::String storeName;
	for (uint i = 0; i < sizeof(names) / sizeof(char *); ++i) {
		parts.getKey(names[i], section, storeName);
		_fileMgr->openStore(storeName);
	}

	parts.getKey("Chapter", Common::String::format("Part %d Chapter %d", _part, _chapter), _chapterStoreName);
	_fileMgr->openStore(_chapterStoreName);
}

} // End of namespace Petka
