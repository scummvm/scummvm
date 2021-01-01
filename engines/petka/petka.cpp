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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/ini-file.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/file.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "graphics/surface.h"

#include "video/avi_decoder.h"

#include "petka/file_mgr.h"
#include "petka/video.h"
#include "petka/sound.h"
#include "petka/petka.h"
#include "petka/q_manager.h"
#include "petka/interfaces/interface.h"
#include "petka/q_system.h"
#include "petka/big_dialogue.h"

namespace Petka {

PetkaEngine *g_vm;

PetkaEngine::PetkaEngine(OSystem *system, const ADGameDescription *desc)
	: Engine(system), _console(nullptr), _fileMgr(nullptr), _resMgr(nullptr),
	_qsystem(nullptr), _vsys(nullptr), _desc(desc), _videoDec(nullptr), _rnd("petka") {

	DebugMan.addDebugChannel(kPetkaDebugGeneral, "general", "General issues");
	DebugMan.addDebugChannel(kPetkaDebugResources, "resources", "Resources");
	DebugMan.addDebugChannel(kPetkaDebugMessagingSystem, "message_system", "Engine message system");
	DebugMan.addDebugChannel(kPetkaDebugDialogs, "dialogs", "Dialogs");

	_part = 0xFF;
	_chapter = 0;
	_shouldChangePart = false;
	_nextPart = 0;
	_saveSlot = -1;
	g_vm = this;

	debug("PetkaEngine::ctor");
}

PetkaEngine::~PetkaEngine() {
	debug("PetkaEngine::dtor");
	DebugMan.clearAllDebugChannels();
}

Common::Error PetkaEngine::run() {
	debug("PetkaEngine::run");
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(640, 480, &format);

	const char *const videos[] = {"buka.avi", "skif.avi", "adv.avi"};
	for (uint i = 0; i < sizeof(videos) / sizeof(char *); ++i) {
		Common::ScopedPtr<Common::File> file(new Common::File);
		if (file->open(videos[i])) {
			playVideo(file.release());
		} else {
			debugC(kPetkaDebugResources, "Video file %s can't be opened", videos[i]);
		}
	}

	_console.reset(new Console(this));
	_fileMgr.reset(new FileMgr());
	_soundMgr.reset(new SoundMgr(*this));
	_vsys.reset(new VideoSystem(*this));
	_resMgr.reset(new QManager(*this));

	loadPart(isDemo() ? 1 : 0);

	if (ConfMan.hasKey("save_slot")) {
		loadGameState(ConfMan.getInt("save_slot"));
	}

	while (!shouldQuit()) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				return Common::kNoError;
			default:
				_qsystem->onEvent(event);
				break;
			}
		}
		_qsystem->update();

		if (_shouldChangePart) {
			loadPart(_nextPart);
			if (_saveSlot != -1)
				loadGameState(_saveSlot);
			_saveSlot = -1;
			_shouldChangePart = false;
			_vsys->makeAllDirty();
		}

		_vsys->update();
		_system->delayMillis(20);
	}
	return Common::kNoError;
}

Common::SeekableReadStream *PetkaEngine::openFile(const Common::String &name, bool addCurrentPath) {
	if (name.empty()) {
		return nullptr;
	}
	return _fileMgr->getFileStream(addCurrentPath ? _currentPath + name : name);
}

void PetkaEngine::loadStores() {
	debug("PetkaEngine::loadStores");
	_fileMgr->closeAll();

	_fileMgr->openStore("patch.str");
	_fileMgr->openStore("main.str");

	Common::INIFile parts;
	Common::ScopedPtr<Common::SeekableReadStream> stream(_fileMgr->getFileStream("PARTS.INI"));

	if (!stream || !parts.loadFromStream(*stream)) {
		debugC(kPetkaDebugResources, "PARTS.INI opening failed");
		return;
	}

	const char *const names[] = {"Background", "Flics", "Wav", "SFX", "Music", "Speech"};
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
	PauseToken token = pauseEngine();
	Graphics::PixelFormat fmt = _system->getScreenFormat();

	_videoDec.reset(new Video::AVIDecoder);
	if (!_videoDec->loadStream(stream)) {
		_videoDec.reset();
		return;
	}

	_videoDec->start();

	while (!_videoDec->endOfVideo() && !shouldQuit()) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RETURN_TO_LAUNCHER:
			case Common::EVENT_QUIT:
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
			case Common::EVENT_KEYDOWN:
				_videoDec.reset();
				return;
			default:
				break;
			}
		}

		if (_videoDec->needsUpdate()) {
			const Graphics::Surface *frame = _videoDec->decodeNextFrame();
			if (frame) {
				Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> f(frame->convertTo(fmt));
				_system->copyRectToScreen(f->getPixels(), f->pitch, 0, 0, f->w, f->h);
			}
		}

		_system->updateScreen();
		_system->delayMillis(15);
	}

	_videoDec.reset();
}

bool PetkaEngine::isDemo() const {
	return _desc->flags & ADGF_DEMO;
}

bool PetkaEngine::isPetka2() const {
	return strcmp(_desc->gameId, "petka2") == 0;
}

SoundMgr *PetkaEngine::soundMgr() const {
	return _soundMgr.get();
}

QManager *PetkaEngine::resMgr() const {
	return _resMgr.get();
}

VideoSystem *PetkaEngine::videoSystem() const {
	return _vsys.get();
}

byte PetkaEngine::getPart() {
	return _part;
}

void PetkaEngine::loadPart(byte part) {
	debug("PetkaEngine::loadPart %d", part);
	_part = part;

	_soundMgr->removeAll();
	loadStores();

	_resMgr.reset(new QManager(*this));
	_resMgr->init();
	_dialogMan.reset(new BigDialogue(*this));
	_qsystem.reset(new QSystem(*this));
	_qsystem->init();
}

void PetkaEngine::loadPartAtNextFrame(byte part) {
	_shouldChangePart = true;
	_nextPart = part;
	_chapter = 1;
	_saveSlot = -1;
}

void PetkaEngine::loadChapter(byte chapter) {
	Common::INIFile parts;
	Common::ScopedPtr<Common::SeekableReadStream> stream(_fileMgr->getFileStream("PARTS.INI"));

	if (!stream || !parts.loadFromStream(*stream)) {
		debugC(kPetkaDebugResources, "PARTS.INI opening failed");
		return;
	}

	_fileMgr->closeStore(_chapterStoreName);

	const Common::String section = Common::String::format("Part %d Chapter %d", _part, chapter);
	parts.getKey("Chapter", section, _chapterStoreName);
	if (_chapterStoreName.empty())
		return;

	_fileMgr->openStore(_chapterStoreName);

	Common::ScopedPtr<Common::SeekableReadStream> namesStream(openFile("Names.ini", true));
	Common::ScopedPtr<Common::SeekableReadStream> castStream(openFile("Cast.ini", true));

	Common::INIFile namesIni;
	Common::INIFile castIni;

	namesIni.allowNonEnglishCharacters();
	castIni.allowNonEnglishCharacters();

	if (namesStream)
		namesIni.loadFromStream(*namesStream);
	if (castStream)
		castIni.loadFromStream(*castStream);

	for (uint i = 0; i < _qsystem->_allObjects.size(); ++i) {
		QMessageObject *obj = _qsystem->_allObjects[i];
		obj->readInisData(namesIni, castIni, nullptr);
	}
	_chapter = chapter;
}

BigDialogue *PetkaEngine::getBigDialogue() const {
	return _dialogMan.get();
}

const Common::String &PetkaEngine::getSpeechPath() {
	return _speechPath;
}

bool PetkaEngine::hasFeature(EngineFeature f) const {
	return
		f == kSupportsReturnToLauncher ||
		f == kSupportsLoadingDuringRuntime ||
		f == kSupportsSavingDuringRuntime ||
		f == kSupportsChangingOptionsDuringRuntime;
}

void PetkaEngine::pauseEngineIntern(bool pause) {
	if (!pause && _vsys)
		_vsys->updateTime();

	if (_videoDec)
		_videoDec->pauseVideo(pause);

	Engine::pauseEngineIntern(pause);
}

} // End of namespace Petka
