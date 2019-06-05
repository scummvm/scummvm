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
#include "common/events.h"
#include "common/ini-file.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/file.h"

#include "engines/util.h"

#include "graphics/surface.h"

#include "video/avi_decoder.h"

#include "petka/file_mgr.h"
#include "petka/petka.h"
#include "petka/q_manager.h"
#include "petka/q_system.h"

namespace Petka {

PetkaEngine *g_vm;

PetkaEngine::PetkaEngine(OSystem *system, const ADGameDescription *desc)
	: Engine(system), _console(nullptr), _fileMgr(nullptr), _resMgr(nullptr),
	_qsystem(nullptr), _desc(desc), _rnd("petka") {
	DebugMan.addDebugChannel(kPetkaDebugGeneral, "general", "General issues");
	_part = 0;
	_chapter = 0;
	g_vm = this;
}

PetkaEngine::~PetkaEngine() {
	DebugMan.clearAllDebugChannels();
}

Common::Error PetkaEngine::run() {
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(640, 480, &format);

	const char *const videos[] = {"buka.avi", "skif.avi", "adv.avi"};
	for (uint i = 0; i < sizeof(videos) / sizeof(char *); ++i) {
		Common::File *file = new Common::File;
		if (file->open(videos[i])) {
			playVideo(file);
		} else {
			delete file;
		}
	}

	_console.reset(new Console(this));
	_fileMgr.reset(new FileMgr());
	_resMgr.reset(new QManager(*this));
	_qsystem.reset(new QSystem(*this));

	loadStores();
	if (!_resMgr->init())
		return Common::kNoGameDataFoundError;
	_qsystem->init();

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

QSystem *PetkaEngine::getQSystem() const {
	return _qsystem.get();
}

Common::RandomSource &PetkaEngine::getRnd() {
	return _rnd;
}

void PetkaEngine::playVideo(Common::SeekableReadStream *stream) {
	Graphics::PixelFormat fmt = _system->getScreenFormat();

	Video::AVIDecoder decoder;
	if (!decoder.loadStream(stream)) {
		return;
	}

	decoder.start();
	while (!decoder.endOfVideo()) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
			case Common::EVENT_KEYDOWN:
				decoder.close();
				break;
			default:
				break;
			}
		}

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			if (frame) {
				Common::ScopedPtr<Graphics::Surface> f(frame->convertTo(fmt));
				_system->copyRectToScreen(f->getPixels(), f->pitch, 0, 0, f->w, f->h);
			}
		}

		_system->updateScreen();
		_system->delayMillis(50);
	}
}

} // End of namespace Petka
