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

#include "engines/nancy/state/logo.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/state/help.h"
#include "engines/nancy/state/map.h"
#include "engines/nancy/state/credits.h"

#include "engines/nancy/action/sliderpuzzle.h"
#include "engines/nancy/action/primaryvideo.h"
#include "engines/nancy/action/secondarymovie.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/cheat.h"

#include "common/system.h"
#include "common/random.h"
#include "common/error.h"
#include "common/events.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/memstream.h"
#include "common/installshield_cab.h"
#include "common/str.h"
#include "common/savefile.h"
#include "common/serializer.h"

#include "graphics/surface.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

#include "engines/util.h"

namespace Nancy {

NancyEngine::NancyEngine(OSystem *syst, const NancyGameDescription *gd) : Engine(syst), _gameDescription(gd), _system(syst) {
	DebugMan.addDebugChannel(kDebugEngine, "Engine", "Engine debug level");
	DebugMan.addDebugChannel(kDebugActionRecord, "ActionRecord", "Action Record debug level");
	DebugMan.addDebugChannel(kDebugScene, "Scene", "Scene debug level");

	_console = new NancyConsole();
	_rnd = new Common::RandomSource("Nancy");
	_rnd->setSeed(_rnd->getSeed());

	input = new InputManager();
	sound = new SoundManager();
	graphicsManager = new GraphicsManager();
	cursorManager = new CursorManager();

	launchConsole = false;
}

NancyEngine::~NancyEngine() {
	clearBootChunks();
	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _rnd;

	delete graphicsManager;
	delete input;
	delete sound;
}

GUI::Debugger *NancyEngine::getDebugger() {
	return _console;
}

bool NancyEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *NancyEngine::getCopyrightString() const {
	return "Copyright 1989-1997 David P Gray, All Rights Reserved.";
}

GameType NancyEngine::getGameType() const {
	return _gameType;
}

Common::Platform NancyEngine::getPlatform() const {
	return _platform;
}

Common::Error NancyEngine::run() {
	initGraphics(640, 480, &GraphicsManager::pixelFormat);

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "game");
	SearchMan.addSubDirectoryMatching(gameDataDir, "datafiles");
	SearchMan.addSubDirectoryMatching(gameDataDir, "hdsound");
	SearchMan.addSubDirectoryMatching(gameDataDir, "cdsound");
	SearchMan.addSubDirectoryMatching(gameDataDir, "hdvideo");
	SearchMan.addSubDirectoryMatching(gameDataDir, "cdvideo");
	
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember("data1.cab");
		if (!stream)
			error("Failed to open data1.cab");

		Common::Archive *cab = Common::makeInstallShieldArchive(stream);
	if (cab)
		SearchMan.add("data1.hdr", cab);
	
	resource = new ResourceManager();
	resource->initialize();

	// Setup mixer
	syncSoundSettings();

	setState(kBoot);

	while (!shouldQuit()) {
		cursorManager->setCursorType(CursorManager::kNormalArrow);
		input->processEvents();
		
		if (_gameFlow.currentState) {
			_gameFlow.currentState->process();
		}

		graphicsManager->draw();

		if (launchConsole) {
			_console->attach();
			launchConsole = false;
		}
		_console->onFrame();

		_system->updateScreen();
		_system->delayMillis(16);
	}

	return Common::kNoError;
}

void NancyEngine::bootGameEngine() {
	clearBootChunks();
	IFF *boot = new IFF("boot");
	if (!boot->load())
		error("Failed to load boot script");
	preloadCals(*boot);

	addBootChunk("BSUM", boot->getChunkStream("BSUM"));
	readBootSummary(*boot);

	Common::String names[] = {
		"INTR", "HINT", "LOGO", "SPUZ", "INV",
		"FONT", "MENU", "HELP", "CRED", "LOAD",
		"MAP", "CD", "TBOX", "CURS", "VIEW", "MSND",
		"BUOK", "BUDE", "BULS", "GLOB", "SLID",
		"SET", "CURT", "CANT", "TH1", "TH2",
		"QUOT", "TMOD"
		};

	for (auto const &n : names) {
		addBootChunk(n, boot->getChunkStream(n));
	}

	// The FR, LG and OB chunks get added here	

	Common::SeekableReadStream *font = getBootChunkStream("FONT");
	if (_fontSize != font->size()) {
		error("Mismatch NumFonts and FONT memory... %i, %i", _fontSize, font->size());
	}
	
	// TODO reset some vars
	// TODO reset some more vars

	// These originally get loaded inside Logo
	SoundDescription desc;
	desc.read(*NanEngine.getBootChunkStream("BUOK"), SoundDescription::kNormal);
	NanEngine.sound->loadSound(desc);
	desc.read(*NanEngine.getBootChunkStream("BUDE"), SoundDescription::kNormal);
	NanEngine.sound->loadSound(desc);
	desc.read(*NanEngine.getBootChunkStream("BULS"), SoundDescription::kNormal);
	NanEngine.sound->loadSound(desc);
	desc.read(*NanEngine.getBootChunkStream("GLOB"), SoundDescription::kNormal);
	NanEngine.sound->loadSound(desc);
	desc.read(*NanEngine.getBootChunkStream("CURT"), SoundDescription::kNormal);
	NanEngine.sound->loadSound(desc);
	desc.read(*NanEngine.getBootChunkStream("CANT"), SoundDescription::kNormal);
	NanEngine.sound->loadSound(desc);

	delete boot;
}

State::State *NancyEngine::getStateObject(GameState state) {
	switch (state) {
	case kLogo:
		return &State::Logo::instance();
	case kCredits:
		return &State::Credits::instance();
	case kMap:
		return &State::Map::instance();
	case kHelp:
		return &State::Help::instance();
	case kScene:
		return &State::Scene::instance();
	default:
		return nullptr;
	}
}

bool NancyEngine::addBootChunk(const Common::String &name, Common::SeekableReadStream *stream) {
	if (!stream)
		return false;
	_bootChunks[name] = stream;
	return true;
}

Common::SeekableReadStream *NancyEngine::getBootChunkStream(const Common::String &name) {
	if (_bootChunks.contains(name)) {
		return _bootChunks[name];
	}
	else return nullptr;
}

void NancyEngine::stopAndUnloadSpecificSounds() {
	// TODO missing if
	
	sound->stopSound(NancyLogoState.MSNDchannelID);

	for (uint i = 0; i < 10; ++i) {
		sound->stopSound(i);
	}
}
void NancyEngine::setMouseEnabled(bool enabled) {
	cursorManager->showCursor(enabled); input->setMouseInputEnabled(enabled);
}

Common::Error NancyEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer ser(stream, nullptr);
	return synchronize(ser);
}

bool NancyEngine::canSaveGameStateCurrently() {
	// TODO also disable during secondary movie
	return Action::PlayPrimaryVideoChan0::activePrimaryVideo == nullptr;
}

Common::Error NancyEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer ser(nullptr, stream);

	return synchronize(ser);
}

void NancyEngine::clearBootChunks() {
	for (auto const& i : _bootChunks) {
		delete i._value;
	}
	_bootChunks.clear();
}

Common::Error NancyEngine::synchronize(Common::Serializer &ser) {
	Common::SeekableReadStream *bsum = getBootChunkStream("BSUM");
	bsum->seek(0);
	
	if (ser.isLoading()) {
		byte buf[90];
		byte bsumBuf[90];
		ser.syncBytes(buf, 90);
		bsum->read(bsumBuf, 90);
		if (Common::String((char *)bsumBuf) != (char *)buf) {
			return Common::kReadingFailed;
		}
	} else if (ser.isSaving()) {
		byte buf[90];
		bsum->read(buf, 90);
		ser.syncBytes(buf, 90);
	}

	NancySceneState.synchronize(ser);
	NancySceneState._actionManager.synchronize(ser);
	Action::SliderPuzzle::synchronize(ser);

	return Common::kNoError;
}

void NancyEngine::preloadCals(const IFF &boot) {
	const byte *buf;
	uint size;
	buf = boot.getChunk(ID_PCAL, size);

	if (buf) {
		Common::MemoryReadStream stream(buf, size);
		uint16 count = stream.readUint16LE();
		debugC(1, kDebugEngine, "Preloading %d CALs", count);
		int nameLen = size / count;

		char *name = new char[nameLen];

		for (uint i = 0; i < count; i++) {
			stream.read(name, nameLen);
			name[nameLen - 1] = 0;
			debugC(1, kDebugEngine, "Preloading CAL '%s'", name);
			if (!resource->loadCifTree(name, "cal"))
				error("Failed to preload CAL '%s'", name);
		}

		delete[] name;

		if (stream.err())
			error("Error reading PCAL chunk");
	} else
		debugC(1, kDebugEngine, "No PCAL chunk found");
}

void NancyEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

//	_sound->syncVolume();
}

Common::String NancyEngine::readFilename(Common::ReadStream *stream) const {
	char buf[kMaxFilenameLen + 1];
	int read = stream->read(buf, getFilenameLen());
	buf[read] = 0;
	return Common::String(buf);
}

void NancyEngine::readImageList(const IFF &boot, const Common::String &prefix, ImageList &list) {
	Common::SeekableReadStream *bsum = getBootChunkStream("BSUM");
	byte count = bsum->readByte();
	debugC(1, kDebugEngine, "Found %i %s images", count, prefix.c_str());

	for (int i = 0; i < count; ++i) {
		Common::String chunkName = Common::String::format("%s%d", prefix.c_str(), i);
		Common::SeekableReadStream *chunkStream = boot.getChunkStream(chunkName);

		if (!chunkStream)
			error("Failed to read BOOT %s", chunkName.c_str());

		Image image;
		image.name = readFilename(chunkStream);
		chunkStream->skip(1);
		image.width = chunkStream->readUint16LE();
		image.height = chunkStream->readUint16LE();

		if (chunkStream->err())
			error("Error reading %s%d", prefix.c_str(), i);

		debugC(1, kDebugEngine, "Adding %s (%dx%d)", image.name.c_str(), image.width, image.height);
		list.push_back(image);

		delete chunkStream;
	}
}

void NancyEngine::setState(GameState state, GameState overridePrevious) {
	// Handle special cases first
	switch (state) {
	case kBoot:
		bootGameEngine();
		graphicsManager->init();
		cursorManager->init();
		setState(kLogo);
		return;
	case kMainMenu:
		if (_gameFlow.currentState) {
			if (_gameFlow.currentState->onStateExit()) {
				_gameFlow.currentState = nullptr;
			}
		}
		
		// TODO until the game's own menus are implemented we simply open the GMM
		openMainMenuDialog();

		if (shouldQuit()) {
			return;
		}

		if (_gameFlow.currentState) {
			_gameFlow.currentState->onStateEnter();
		}

		return;
	case kCheat:
		if (_cheatTypeIsEventFlag) {
			EventFlagDialog *dialog = new EventFlagDialog();
			runDialog(*dialog);
			delete dialog;
		} else {
			CheatDialog *dialog = new CheatDialog();
			runDialog(*dialog);
			delete dialog;
		}
		input->forceCleanInput();
		return;
	default:
		break;
	}

	graphicsManager->clearObjects();

	_gameFlow.previousState = _gameFlow.currentState;
	_gameFlow.currentState = getStateObject(state);

	if (_gameFlow.previousState) {
		_gameFlow.previousState->onStateExit();
	}

	if (_gameFlow.currentState) {
		_gameFlow.currentState->onStateEnter();
	}

	if (overridePrevious != kNone) {
		_gameFlow.previousState = getStateObject(state);
	}
}

void NancyEngine::setPreviousState() {
	if (_gameFlow.currentState) {
		_gameFlow.currentState->onStateExit();
	}

	if (_gameFlow.previousState) {
		_gameFlow.previousState->onStateEnter();
	}

	SWAP<Nancy::State::State *>(_gameFlow.currentState, _gameFlow.previousState);
}

class NancyEngine_v0 : public NancyEngine {
public:
	NancyEngine_v0(OSystem *syst, const NancyGameDescription *gd) : NancyEngine(syst, gd) { }

private:
	virtual uint getFilenameLen() const { return 9; }
	virtual void readBootSummary(const IFF &boot);
};

void NancyEngine_v0::readBootSummary(const IFF &boot) {
	Common::SeekableReadStream *bsum = getBootChunkStream("BSUM");
	bsum->seek(0xa3);
	firstSceneID = bsum->readUint16LE();
	bsum->skip(4);
	startTimeHours = bsum->readUint16LE(); // this is a whole Time struct but we just take the hours for now
	bsum->seek(0x151);
	readImageList(boot, "FR", _frames);
	readImageList(boot, "LG", _logos);
	readImageList(boot, "OB", _objects);
	bsum->seek(0x1D1);
	_fontSize = bsum->readSint32LE() * 1346;
	bsum->seek(0x1ED);
	NancySceneState.playerTimeMinuteLength = bsum->readSint16LE();
	bsum->seek(0x1F1);
    overrideMovementTimeDeltas = bsum->readByte();

    if (overrideMovementTimeDeltas) {
        slowMovementTimeDelta = bsum->readUint16LE();
        fastMovementTimeDelta = bsum->readUint16LE();
    }
}

class NancyEngine_v1 : public NancyEngine_v0 {
public:
	NancyEngine_v1(OSystem *syst, const NancyGameDescription *gd) : NancyEngine_v0(syst, gd) { }

private:
	virtual void readBootSummary(const IFF &boot);
};

void NancyEngine_v1::readBootSummary(const IFF &boot) {
	Common::SeekableReadStream *bsum = getBootChunkStream("BSUM");
	bsum->seek(0xa3);
	firstSceneID = bsum->readUint16LE();
	bsum->seek(0x14b);
	readImageList(boot, "FR", _frames);
	readImageList(boot, "LG", _logos);
}

class NancyEngine_v2 : public NancyEngine_v1 {
public:
	NancyEngine_v2(OSystem *syst, const NancyGameDescription *gd) : NancyEngine_v1(syst, gd) { }

private:
	virtual uint getFilenameLen() const { return 32; }
	virtual void readBootSummary(const IFF &boot);
};

void NancyEngine_v2::readBootSummary(const IFF &boot) {
	Common::SeekableReadStream *bsum = getBootChunkStream("BSUM");
	bsum->seek(0xa7);
	readImageList(boot, "FR", _frames);
	readImageList(boot, "LG", _logos);
}

NancyEngine *NancyEngine::create(GameType type, OSystem *syst, const NancyGameDescription *gd) {
	switch (type) {
	case kGameTypeNancy1:
		return new NancyEngine_v0(syst, gd);
	case kGameTypeNancy2:
		return new NancyEngine_v1(syst, gd);
	case kGameTypeNancy3:
		return new NancyEngine_v2(syst, gd);
	default:
		error("Unknown GameType");
	}
}

} // End of namespace Nancy
