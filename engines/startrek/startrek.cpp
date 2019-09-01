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


#include "base/plugins.h"
#include "base/version.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "engines/util.h"
#include "video/qt_decoder.h"

#include "startrek/console.h"
#include "startrek/iwfile.h"
#include "startrek/lzss.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

StarTrekEngine::StarTrekEngine(OSystem *syst, const StarTrekGameDescription *gamedesc) :
	Engine(syst),
	_gameDescription(gamedesc),
	_randomSource("Star Trek"),
	_kirkActor(&_actorList[0]),
	_spockActor(&_actorList[1]),
	_mccoyActor(&_actorList[2]),
	_redshirtActor(&_actorList[3]),
	_sineTable(1024) {

	if (getPlatform() != Common::kPlatformDOS)
		error("Only DOS versions of Star Trek: 25th Anniversary are currently supported");
	else if (getGameType() == GType_STJR)
		error("Star Trek: Judgment Rites not yet supported");

	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics");
	DebugMan.addDebugChannel(kDebugSavegame, "savegame", "Savegames");
	DebugMan.addDebugChannel(kDebugSpace, "space", "Space and Pseudo-3D");
	DebugMan.addDebugChannel(kDebugGeneral, "general", "General");

	_gfx = nullptr;
	_activeMenu = nullptr;
	_sound = nullptr;
	_macResFork = nullptr;
	_room = nullptr;

	memset(_actionOnWalkCompletionInUse, 0, sizeof(_actionOnWalkCompletionInUse));

	_clockTicks = 0;

	_musicEnabled = true;
	_sfxEnabled = true;
	_word_467a6 = true;
	_musicWorking = true;
	_sfxWorking = true;
	_finishedPlayingSpeech = false;

	_lookActionBitmapIndex = 0;

	_mouseControllingShip = false;
	_keyboardControlsMouse = true;

	_inQuitGameMenu = false;
	_textDisplayMode = TEXTDISPLAY_SUBTITLES;
	_textboxVar2 = 0;
	_textboxVar6 = 0;
	_textboxHasMultipleChoices = false;

	_missionToLoad = "DEMON";
	_roomIndexToLoad = 0;
	_mapFile = nullptr;

	_showSubtitles = true;
	Common::fill(_r3List, _r3List + NUM_SPACE_OBJECTS, (R3 *)nullptr);
	Common::fill(_orderedR3List, _orderedR3List + NUM_SPACE_OBJECTS, (R3 *)nullptr);

	for (int i = 0; i < NUM_OBJECTS; i++)
		_itemList[i] = g_itemList[i];
}

StarTrekEngine::~StarTrekEngine() {
	delete _activeMenu->nextMenu;
	delete _activeMenu;

	delete _console;
	delete _gfx;
	delete _sound;
	delete _macResFork;
}

Common::Error StarTrekEngine::run() {
	_gfx = new Graphics(this);
	_sound = new Sound(this);
	_console = new Console(this);

	if (getPlatform() == Common::kPlatformMacintosh) {
		_macResFork = new Common::MacResManager();
		if (!_macResFork->open("Star Trek Data"))
			error("Could not load Star Trek Data");
		assert(_macResFork->hasDataFork() && _macResFork->hasResFork());
	}

	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	initializeEventsAndMouse();

	bool shouldPlayIntro = true;
	bool loadedSave = false;

	if (ConfMan.hasKey("save_slot")) {
		if (!loadGame(ConfMan.getInt("save_slot")))
			error("Failed to load savegame %d", ConfMan.getInt("save_slot"));
		shouldPlayIntro = false;
		loadedSave = true;
		_roomIndexToLoad = -1;
	}

	if (!loadedSave) {
		if (shouldPlayIntro) {
			_frameIndex = 0;
			playIntro();
		}

		_frameIndex = 0;

		_gameMode = -1;
		_lastGameMode = -1;
	}

	if (loadedSave)
		runGameMode(_gameMode, true);
	else
		runGameMode(GAMEMODE_AWAYMISSION, false);
	return Common::kNoError;
}

Common::Error StarTrekEngine::runGameMode(int mode, bool resume) {
	if (!resume) { // Only run this if not just resuming from a savefile
		_gameMode = mode;

		_sound->stopAllVocSounds();

		_resetGameMode = true;
		if (_gameMode == GAMEMODE_START)
			_gameMode = GAMEMODE_BRIDGE;
	}

	while (true) {
		if (_resetGameMode) {
			if (!_sound->_loopingAudioName.empty())
				_sound->playVoc(_sound->_loopingAudioName);
			_resetGameMode = false;
		}
		TrekEvent event;

		if (_gameMode != _lastGameMode) {
			// Cleanup previous game mode
			switch (_lastGameMode) {
			case GAMEMODE_BRIDGE:
				//cleanupBridge();
				break;

			case GAMEMODE_AWAYMISSION:
				cleanupAwayMission();
				break;

			case GAMEMODE_BEAMDOWN:
			case GAMEMODE_BEAMUP:
				break;
			}

			_lastGameMode = _gameMode;

			// Load next game mode
			switch (_gameMode) {
			case GAMEMODE_BRIDGE:
				_sound->loadMusicFile("bridge");
				//initBridge();
				break;

			case GAMEMODE_AWAYMISSION:
				initAwayMission();
				break;

			case GAMEMODE_BEAMDOWN:
				_awayMission.redshirtDead = false;
				_sound->loadMusicFile("ground");
				runTransportSequence("teled");
				_gameMode = GAMEMODE_AWAYMISSION;
				continue; // Back to start of loop

			case GAMEMODE_BEAMUP:
				runTransportSequence("teleb");
				_gameMode = GAMEMODE_BRIDGE;
				//sub_15c61();
				_sound->stopAllVocSounds();
				_sound->playVoc("bridloop");
				continue; // Back to start of loop
			}
		}

		// Run current game mode
		switch (_gameMode) {
		case GAMEMODE_BRIDGE:
			popNextEvent(&event);
			//runBridge();
			break;

		case GAMEMODE_AWAYMISSION:
			runAwayMission();
			break;

		case GAMEMODE_BEAMDOWN:
		case GAMEMODE_BEAMUP:
			error("Can't be here.");
			break;
		}
	}

	return Common::kNoError;
}

void StarTrekEngine::runTransportSequence(const Common::String &name) {
	const uint16 crewmanTransportPositions[][2] = {
		{ 0x8e, 0x7c },
		{ 0xbe, 0x7c },
		{ 0x7e, 0x72 },
		{ 0xaa, 0x72 }
	};

	_sound->stopAllVocSounds();
	_gfx->fadeoutScreen();
	actorFunc1();
	initActors();

	SharedPtr<Bitmap> bgImage = _gfx->loadBitmap("transprt");
	_gfx->setBackgroundImage(bgImage);
	_gfx->clearPri();
	_gfx->loadPalette("palette");
	_gfx->drawDirectToScreen(bgImage);
	_system->updateScreen();
	_system->delayMillis(10);

	for (int i = 0; i < (_awayMission.redshirtDead ? 3 : 4); i++) {
		Common::String filename = getCrewmanAnimFilename(i, name);
		int x = crewmanTransportPositions[i][0];
		int y = crewmanTransportPositions[i][1];
		loadActorAnim(i, filename, x, y, 1.0);
		_actorList[i].animationString.clear();
	}

	if (_missionToLoad.equalsIgnoreCase("feather") && name[4] == 'b') {
		loadActorAnim(9, "qteleb", 0x61, 0x79, 1.0);
	} else if (_missionToLoad.equalsIgnoreCase("trial")) {
		if (name[4] == 'd') {
			loadActorAnim(9, "qteled", 0x61, 0x79, 1.0);
		}
		/* TODO
		else if (word_51156 >= 3) {
			loadActorAnim(9, "qteleb", 0x61, 0x79, 1.0);
		}
		*/
	}

	loadActorAnim(8, "transc", 0, 0, 1.0);

	// TODO: redraw mouse and sprite_52c4e?

	_gfx->drawAllSprites();
	_gfx->fadeinScreen();

	playSoundEffectIndex(0x0a);

	if (name.equalsIgnoreCase("teled"))
		playSoundEffectIndex(0x08);
	else
		playSoundEffectIndex(0x09);

	while (_actorList[0].field62 == 0) {
		TrekEvent event;
		if (popNextEvent(&event)) {
			if (event.type == TREKEVENT_TICK) {
				// TODO: redraw sprite_52c4e?
				_frameIndex++;
				updateActorAnimations();
				_gfx->drawAllSprites();
			}
		}
	}

	// TODO: redraw sprite_52c4e?

	_gfx->drawAllSprites();
	_gfx->fadeoutScreen();
	actorFunc1();
	initActors();
}

void StarTrekEngine::playSoundEffectIndex(int index) {
	if (!(getFeatures() & GF_CDROM))
		_sound->playMidiTrack(index);
	else {
		switch (index) {
		case 0x04:
			_sound->playVoc("tricorde");
			break;
		case 0x05:
			_sound->playVoc("STDOOR1");
			break;
		case 0x06:
			_sound->playVoc("PHASSHOT");
			break;
		case 0x07:
			_sound->playMidiTrack(index);
			break;
		case 0x08:
			_sound->playVoc("TRANSDEM");
			break;
		case 0x09: // Beaming in?
			_sound->playVoc("TRANSMAT");
			break;
		case 0x0a: // Beaming out?
			_sound->playVoc("TRANSENE");
			break;
		case 0x10: // Menu selection sound
			_sound->playMidiTrack(index);
			break;
		case 0x22:
			_sound->playVoc("HAILING");
			break;
		case 0x24:
			_sound->playVoc("PHASSHOT");
			break;
		case 0x25:
			_sound->playVoc("PHOTSHOT");
			break;
		case 0x26:
			_sound->playVoc("HITSHIEL");
			break;
		case 0x27:
			_sound->playMidiTrack(index);
			break;
		case 0x28:
			_sound->playVoc("REDALERT");
			break;
		case 0x29:
			_sound->playVoc("WARP");
			break;
		default:
			debugC(kDebugSound, 6, "Unmapped sound 0x%x", index);
			break;
		}
	}
}

void StarTrekEngine::playMidiMusicTracks(int startTrack, int loopTrack) {
	_sound->playMidiMusicTracks(startTrack, loopTrack);
}

void StarTrekEngine::playSpeech(const Common::String &filename) {
	_sound->playSpeech(filename.c_str());
}

void StarTrekEngine::stopPlayingSpeech() {
	_sound->stopPlayingSpeech();
}

/**
 * TODO:
 *   - Should return nullptr on failure to open a file?
 *   - This is supposed to cache results, return same FileStream on multiple accesses.
 *   - This is supposed to read from a "patches" folder which overrides files in the
 *     packed blob.
 */
Common::MemoryReadStreamEndian *StarTrekEngine::loadFile(Common::String filename, int fileIndex) {
	filename.toUppercase();

	Common::String basename, extension;

	bool bigEndian = getPlatform() == Common::kPlatformAmiga;

	for (int i = filename.size() - 1; ; i--) {
		if (filename[i] == '.') {
			basename = filename;
			extension = filename;
			basename.replace(i, filename.size() - i, "");
			extension.replace(0, i + 1, "");
			break;
		}
	}

	// FIXME: don't know if this is right, or if it goes here
	while (!basename.empty() && basename.lastChar() == ' ') {
		basename.erase(basename.size() - 1, 1);
	}

	filename = basename + '.' + extension;

	// The Judgment Rites demo has its files not in the standard archive
	if (getGameType() == GType_STJR && (getFeatures() & GF_DEMO)) {
		Common::File *file = new Common::File();
		if (!file->open(filename.c_str())) {
			delete file;
			error("Could not find file \'%s\'", filename.c_str());
		}
		int32 size = file->size();
		byte *data = (byte *)malloc(size);
		file->read(data, size);
		delete file;
		return new Common::MemoryReadStreamEndian(data, size, bigEndian);
	}

	Common::SeekableReadStream *indexFile = 0;

	if (getPlatform() == Common::kPlatformAmiga) {
		indexFile = SearchMan.createReadStreamForMember("data000.dir");
		if (!indexFile)
			error("Could not open data000.dir");
	} else if (getPlatform() == Common::kPlatformMacintosh) {
		indexFile = _macResFork->getResource("Directory");
		if (!indexFile)
			error("Could not find 'Directory' resource in 'Star Trek Data'");
	} else {
		indexFile = SearchMan.createReadStreamForMember("data.dir");
		if (!indexFile)
			error("Could not open data.dir");
	}

	uint32 indexOffset = 0;
	bool foundData = false;
	uint16 fileCount = 1;
	uint16 uncompressedSize = 0;

	while (!indexFile->eos() && !indexFile->err()) {
		Common::String testfile;
		for (byte i = 0; i < 8; i++) {
			char c = indexFile->readByte();
			if (c)
				testfile += c;
		}
		testfile += '.';

		for (byte i = 0; i < 3; i++)
			testfile += indexFile->readByte();

		if (getFeatures() & GF_DEMO && getPlatform() == Common::kPlatformDOS) {
			indexFile->readByte(); // Always 0?
			fileCount = indexFile->readUint16LE(); // Always 1
			indexOffset = indexFile->readUint32LE();
			uncompressedSize = indexFile->readUint16LE();
		} else {
			if (getPlatform() == Common::kPlatformAmiga)
				indexOffset = (indexFile->readByte() << 16) + (indexFile->readByte() << 8) + indexFile->readByte();
			else
				indexOffset = indexFile->readByte() + (indexFile->readByte() << 8) + (indexFile->readByte() << 16);

			if (indexOffset & (1 << 23)) {
				fileCount = (indexOffset >> 16) & 0x7F;
				indexOffset = indexOffset & 0xFFFF;
				assert(fileCount > 1);
			} else {
				fileCount = 1;
			}
		}

		if (filename.matchString(testfile)) {
			foundData = true;
			break;
		}
	}

	delete indexFile;

	if (!foundData) {
		// Files can be accessed "sequentially" if their filenames are the same except for
		// the last character being incremented by one.
		if ((basename.lastChar() >= '1' && basename.lastChar() <= '9') ||
		        (basename.lastChar() >= 'B' && basename.lastChar() <= 'Z')) {
			basename.setChar(basename.lastChar() - 1, basename.size() - 1);
			return loadFile(basename + "." + extension, fileIndex + 1);
		} else
			error("Could not find file \'%s\'", filename.c_str());
	}

	if (fileIndex >= fileCount)
		error("Tried to access file index %d for file '%s' which doesn't exist.", fileIndex, filename.c_str());

	Common::SeekableReadStream *dataFile = 0;
	Common::SeekableReadStream *dataRunFile = 0; // FIXME: Amiga & Mac need this implemented

	if (getPlatform() == Common::kPlatformAmiga) {
		dataFile = SearchMan.createReadStreamForMember("data.000");
		if (!dataFile)
			error("Could not open data.000");
	} else if (getPlatform() == Common::kPlatformMacintosh) {
		dataFile = _macResFork->getDataFork();
		if (!dataFile)
			error("Could not get 'Star Trek Data' data fork");
	} else {
		dataFile = SearchMan.createReadStreamForMember("data.001");
		if (!dataFile)
			error("Could not open data.001");
		dataRunFile = SearchMan.createReadStreamForMember("data.run");
		if (!dataFile)
			error("Could not open data.run");
	}

	Common::SeekableReadStream *stream;
	if (getFeatures() & GF_DEMO && getPlatform() == Common::kPlatformDOS) {
		assert(fileCount == 1); // Sanity check...
		stream = dataFile->readStream(uncompressedSize);
	} else {
		if (fileCount != 1) {
			dataRunFile->seek(indexOffset);

			indexOffset = dataRunFile->readByte() + (dataRunFile->readByte() << 8) + (dataRunFile->readByte() << 16);
			//indexOffset &= 0xFFFFFE;

			for (uint16 i = 0; i < fileIndex; i++) {
				uint16 size = dataRunFile->readUint16LE();
				indexOffset += size;
			}
		}
		dataFile->seek(indexOffset);

		uncompressedSize = (getPlatform() == Common::kPlatformAmiga) ? dataFile->readUint16BE() : dataFile->readUint16LE();
		uint16 compressedSize = (getPlatform() == Common::kPlatformAmiga) ? dataFile->readUint16BE() : dataFile->readUint16LE();

		stream = decodeLZSS(dataFile->readStream(compressedSize), uncompressedSize);
	}

	delete dataFile;
	delete dataRunFile;

	int32 size = stream->size();
	byte *data = (byte *)malloc(size);
	stream->read(data, size);
	delete stream;

	return new Common::MemoryReadStreamEndian(data, size, bigEndian);
}

Common::MemoryReadStreamEndian *StarTrekEngine::loadFileWithParams(Common::String filename, bool unk1, bool unk2, bool unk3) {
	return loadFile(filename);
}

void StarTrekEngine::playMovie(Common::String filename) {
	if (getPlatform() == Common::kPlatformMacintosh)
		playMovieMac(filename);
	else
		error("Interplay MVE not yet supported");
}

void StarTrekEngine::playMovieMac(Common::String filename) {
	// Swap to 16bpp mode
	initGraphics(512, 384, NULL);

	Video::QuickTimeDecoder *qtDecoder = new Video::QuickTimeDecoder();

	if (!qtDecoder->loadFile(filename))
		error("Could not open '%s'", filename.c_str());

	bool continuePlaying = true;

	qtDecoder->start();

	while (!qtDecoder->endOfVideo() && !shouldQuit() && continuePlaying) {
		if (qtDecoder->needsUpdate()) {
			const ::Graphics::Surface *frame = qtDecoder->decodeNextFrame();

			if (frame) {
				::Graphics::Surface *convertedFrame = frame->convertTo(_system->getScreenFormat());
				_system->copyRectToScreen(convertedFrame->getPixels(), convertedFrame->pitch, 0, 0, convertedFrame->w, convertedFrame->h);
				_system->updateScreen();
				convertedFrame->free();
				delete convertedFrame;
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event))
			if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)
				continuePlaying = false;

		_system->delayMillis(10);
	}

	delete qtDecoder;

	// Swap back to 8bpp mode
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
}

uint16 StarTrekEngine::getRandomWord() {
	return _randomSource.getRandomNumber(0xffff);
}

Common::String StarTrekEngine::getLoadedText(int textIndex) {
	Common::MemoryReadStreamEndian *txtFile = loadFile(_txtFilename + ".txt");

	Common::String str;
	byte cur;
	int curIndex = 0;

	while (!txtFile->eos()) {
		do {
			cur = txtFile->readByte();
			str += cur;
		} while (cur != '\0');

		if (curIndex == textIndex) {
			delete txtFile;
			return str;
		}

		curIndex++;
		str = "";
	}
	
	delete txtFile;
	return "";
}

} // End of namespace StarTrek
