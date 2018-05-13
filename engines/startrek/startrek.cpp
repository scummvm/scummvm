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
#include "graphics/surface.h"
#include "engines/util.h"
#include "video/qt_decoder.h"

#include "startrek/filestream.h"
#include "startrek/lzss.h"
#include "startrek/startrek.h"

namespace StarTrek {

StarTrekEngine::StarTrekEngine(OSystem *syst, const StarTrekGameDescription *gamedesc) :
	Engine(syst),
	_gameDescription(gamedesc),
	_kirkObject(&_objectList[0]),
	_spockObject(&_objectList[1]),
	_mccoyObject(&_objectList[2]),
	_redshirtObject(&_objectList[3]) {

	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics");

	_gfx = nullptr;
	_sound = nullptr;
	_macResFork = nullptr;
	_room = nullptr;

	_redshirtDead = false;

	_clockTicks = 0;

	_musicEnabled = true;
	_sfxEnabled = true;
	_word_467a6 = true;
	_musicWorking = true;
	_sfxWorking = true;
	_finishedPlayingSpeech = false;

	_mouseControllingShip = false;
	_keyboardControlsMouse = true;

	_inQuitGameMenu = false;
}

StarTrekEngine::~StarTrekEngine() {
	delete _gfx;
	delete _sound;
	delete _macResFork;

	delete _room;
}

Common::Error StarTrekEngine::run() {
	_gfx = new Graphics(this);
	_sound = new Sound(this);

	if (getPlatform() == Common::kPlatformMacintosh) {
		_macResFork = new Common::MacResManager();
		if (!_macResFork->open("Star Trek Data"))
			error("Could not load Star Trek Data");
		assert(_macResFork->hasDataFork() && _macResFork->hasResFork());
	}

	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	initializeEventsAndMouse();

	_frameIndex = 0;

	_gameMode = -1;
	_lastGameMode = -1;

	runGameMode(GAMEMODE_BEAMDOWN);
	return Common::kNoError;


#if 1
	_room = new Room(this, "DEMON0");
	_gfx->loadPalette("PALETTE");
	_gfx->loadPri("DEMON0.PRI");

	_sound->loadMusicFile("GROUND");


	while (true) {
		_gfx->showOptionsMenu(0, 0);
	}
	
	while (!shouldQuit()) {
		pollSystemEvents();
	}

	return Common::kNoError;
#endif
}

Common::Error StarTrekEngine::runGameMode(int mode) {
	_gameMode = mode;

	_sound->stopAllVocSounds();
	if (!_sound->_loopingAudioName.empty())
		_sound->playVoc(_sound->_loopingAudioName);

	if (_gameMode == GAMEMODE_START)
		_gameMode = GAMEMODE_BRIDGE;

	while (true) {
		TrekEvent event;

		if (_gameMode != _lastGameMode) {
			// Cleanup previous game mode
			switch (_lastGameMode) {
			case GAMEMODE_BRIDGE:
				//cleanupBridgeMode();
				break;

			case GAMEMODE_AWAYMISSION:
				//cleanupAwayMissionMode();
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
				//initAwayMission();
				break;

			case GAMEMODE_BEAMDOWN:
				_redshirtDead = false;
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
			popNextEvent(&event);
			_system->updateScreen();
			//runAwayMission();
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
	// sub_1e70d();
	objectFunc1();
	initObjects();

	SharedPtr<Bitmap> bgImage = _gfx->loadBitmap("transprt");
	_gfx->setBackgroundImage(bgImage);
	_gfx->clearPri();
	_gfx->loadPalette("palette");
	_gfx->drawDirectToScreen(bgImage);
	_system->updateScreen();

	for (int i = 0; i < (_redshirtDead ? 3 : 4); i++) {
		Common::String filename = getCrewmanAnimFilename(i, name);
		int x = crewmanTransportPositions[i][0];
		int y = crewmanTransportPositions[i][1];
		loadAnimationForObject(i, filename, x, y, 256);
		_objectList[i].animationString[0] = '\0';
	}

	if (_missionToLoad.equalsIgnoreCase("feather") && name[4] == 'b') {
		loadAnimationForObject(9, "qteleb", 0x61, 0x79, 0x100);
	}
	else if (_missionToLoad.equalsIgnoreCase("trial")) {
		if (name[4] == 'd') {
			loadAnimationForObject(9, "qteled", 0x61, 0x79, 0x100);
		}
		/* TODO
		else if (word_51156 >= 3) {
			loadAnimationForObject(9, "qteleb", 0x61, 0x79, 0x100);
		}
		*/
	}

	loadAnimationForObject(8, "transc", 0, 0, 0x100);

	// TODO: redraw mouse and sprite_52c4e?

	_gfx->drawAllSprites();
	// sub_1e6ab();

	playSoundEffectIndex(0x0a);

	if (name.equalsIgnoreCase("teled"))
		playSoundEffectIndex(0x08);
	else
		playSoundEffectIndex(0x09);

	while (_objectList[0].field62 == 0) {
		TrekEvent event;
		if (popNextEvent(&event)) {
			if (event.type == TREKEVENT_TICK) {
				// TODO: redraw sprite_52c4e?
				_frameIndex++;
				updateObjectAnimations();
				_gfx->drawAllSprites();
			}
		}
	}

	// TODO: redraw sprite_52c4e?

	_gfx->drawAllSprites();
	// sub_1e70d();
	objectFunc1();
	initObjects();
}

Room *StarTrekEngine::getRoom() {
	return _room;
}

void StarTrekEngine::playSoundEffectIndex(int index) {
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
	case 0x09:
		_sound->playVoc("TRANSMAT");
		break;
	case 0x0a:
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
	    break;
	}
}

void StarTrekEngine::playSpeech(const Common::String &filename) {
	_sound->playSpeech(filename.c_str());
}

void StarTrekEngine::stopPlayingSpeech() {
	_sound->stopPlayingSpeech();
}

void StarTrekEngine::initObjects() {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		_objectList[i] = Object();
	}
	for (int i = 0; i < MAX_OBJECTS / 2; i++)
		_objectBanFiles[i].reset();

	strcpy(_kirkObject->animationString, "kstnd");
	strcpy(_spockObject->animationString, "sstnd");
	strcpy(_mccoyObject->animationString, "mstnd");
	strcpy(_redshirtObject->animationString, "rstnd");
}

int StarTrekEngine::loadAnimationForObject(int objectIndex, const Common::String &animName, uint16 x, uint16 y, uint16 arg8) {
	debugC(6, kDebugGraphics, "Load animation '%s' on object %d", animName.c_str(), objectIndex);

	Object *object;

	if (objectIndex == -1) {
		// TODO
	}
	else
		object = &_objectList[objectIndex];

	if (object->spriteDrawn) {
		releaseAnim(object);
		drawObjectToScreen(object, animName, x, y, arg8, false);
	}
	else {
		drawObjectToScreen(object, animName, x, y, arg8, true);
	}

	object->field64 = 0;
	object->field66 = 0;

	return objectIndex;
}

void StarTrekEngine::updateObjectAnimations() {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		Object *object = &_objectList[i];
		if (!object->spriteDrawn)
			continue;

		switch (object->animType) {
		case 0:
		case 2:
			if (_frameIndex >= object->frameToStartNextAnim) {
				int nextAnimIndex = 0; // TODO: "chooseNextAnimFrame" function
				object->animFile->seek(18 + nextAnimIndex + object->animFrame * 22, SEEK_SET);
				byte nextAnimFrame = object->animFile->readByte();

				debugC(7, kDebugGraphics, "Object %d animation frame %d", i, nextAnimFrame);

				if (object->animFrame != nextAnimFrame) {
					if (nextAnimFrame == object->numAnimFrames - 1) {
						object->field62++;
						if (object->field64 != 0) {
							// sub_20099(10, object->field66, 0, 0);
						}
					}
				}

				object->animFrame = nextAnimFrame;
				if (object->animFrame >= object->numAnimFrames) {
					if (object->animationString[0] == '\0')
						removeObjectFromScreen(i);
					/*
					else // TODO
						initStandAnim(i);
						*/
				}
				else {
					Sprite *sprite = &object->sprite;

					object->animFile->seek(object->animFrame * 22, SEEK_SET);
					char animFrameFilename[16];
					object->animFile->read(animFrameFilename, 16);
					sprite->setBitmap(loadAnimationFrame(animFrameFilename, object->scale));

					memset(object->animationString4, 0, 16);
					strncpy(object->animationString4, animFrameFilename, 15);

					object->animFile->seek(10 + object->animFrame * 22, SEEK_SET);
					uint16 xOffset = object->animFile->readUint16();
					uint16 yOffset = object->animFile->readUint16();
					uint16 basePriority = object->animFile->readUint16();
					uint16 frames = object->animFile->readUint16();

					sprite->pos.x = xOffset + object->field5e;
					sprite->pos.y = yOffset + object->field60;
					sprite->drawPriority = _gfx->getPriValue(0, yOffset + object->field60) + basePriority;
					sprite->bitmapChanged = true;

					object->frameToStartNextAnim = frames + _frameIndex;
				}
			}
			break;
		case 1: // TODO
			warning("Unimplemented anim type %d", object->animType);
			break;
		default:
			error("Invalid anim type.");
			break;
		}
	}
}

void StarTrekEngine::removeObjectFromScreen(int objectIndex) {
	Object *object = &_objectList[objectIndex];

	if (object->spriteDrawn != 1)
		return;

	debugC(6, kDebugGraphics, "Stop drawing object %d", objectIndex);

	Sprite *sprite = &object->sprite;
	sprite->field16 = true;
	sprite->bitmapChanged = true;
	_gfx->drawAllSprites();
	_gfx->delSprite(sprite);
	releaseAnim(object);
}

void StarTrekEngine::objectFunc1() {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (_objectList[i].spriteDrawn == 1) {
			removeObjectFromScreen(i);
		}
	}

	for (int i = 0; i < MAX_OBJECTS / 2; i++) {
		_objectBanFiles[i].reset();
	}
}

void StarTrekEngine::drawObjectToScreen(Object *object, const Common::String &_animName, uint16 x, uint16 y, uint16 arg8, bool addSprite) {
	Common::String animFilename = _animName;
	if (_animName.hasPrefixIgnoreCase("stnd") /* && word_45d20 == -1 */) // TODO
		animFilename += 'j';
	memcpy(object->animationString3, _animName.c_str(), sizeof(object->animationString3));

	object->animType = 2;
	object->animFile = loadFile(animFilename + ".anm");
	object->numAnimFrames = object->animFile->size() / 22;
	object->animFrame = 0;
	object->field5e = x;
	object->field60 = y;
	object->field62 = 0;
	object->scale = arg8;

	object->animFile->seek(16, SEEK_SET);
	object->frameToStartNextAnim = object->animFile->readUint16() + _frameIndex;

	char firstFrameFilename[10];
	object->animFile->seek(0, SEEK_SET);
	object->animFile->read(firstFrameFilename, 10);

	Sprite *sprite = &object->sprite;
	if (addSprite)
		_gfx->addSprite(sprite);

	sprite->setBitmap(loadAnimationFrame(firstFrameFilename, arg8));
	memset(object->animationString4, 0, sizeof(char) * 10);
	strncpy(object->animationString4, firstFrameFilename, sizeof(char) * 9);

	object->scale = arg8;

	object->animFile->seek(10, SEEK_SET);
	uint16 xOffset = object->animFile->readUint16();
	uint16 yOffset = object->animFile->readUint16();
	uint16 basePriority = object->animFile->readUint16();

	sprite->pos.x = xOffset + object->field5e;
	sprite->pos.y = yOffset + object->field60;
	sprite->drawPriority = _gfx->getPriValue(0, yOffset + object->field60) + basePriority;
	sprite->bitmapChanged = true;

	object->spriteDrawn = 1;
}

void StarTrekEngine::releaseAnim(Object *object) {
	switch (object->animType) {
	case 0:
	case 2:
		object->sprite.bitmap.reset();
		object->animFile.reset();
		break;
	case 1: // TODO
		warning("Unimplemented anim type %d", object->animType);
		break;
	default:
		error("Invalid anim type.");
		break;
	}

	object->spriteDrawn = 0;
}

SharedPtr<Bitmap> StarTrekEngine::loadAnimationFrame(const Common::String &filename, uint16 arg2) {
	SharedPtr<Bitmap> bitmapToReturn;

	char basename[5];
	strncpy(basename, filename.c_str()+1, 4);
	basename[4] = '\0';

	char c = filename[0];
	if ((strcmp(basename, "stnd") == 0 || strcmp(basename, "tele") == 0)
			&& (c == 'm' || c == 's' || c == 'k' || c == 'r')) {
		if (c == 'm') {
			bitmapToReturn = _gfx->loadBitmap(filename);
		}
		else {
			// bitmapToReturn = _gfx->loadBitmap(filename + ".$bm"); // FIXME: should be this?
			if (bitmapToReturn == nullptr) {
				Common::String newFilename = filename;
				newFilename.setChar('m', 0); // FIXME: original writes directly to argument; does that affect anything?
				bitmapToReturn = _gfx->loadBitmap(newFilename);
			}
		}
	}
	else {
		// TODO: when loading a bitmap, it passes a different argument than is standard to
		// the "file loading with cache" function...
		bitmapToReturn = _gfx->loadBitmap(filename);
	}

	if (arg2 != 256) {
		// TODO
		// bitmapToReturn = scaleBitmap(bitmapToReturn, arg2);
	}

	return bitmapToReturn;
}

Common::String StarTrekEngine::getCrewmanAnimFilename(int objectIndex, const Common::String &basename) {
	const char *crewmanChars = "ksmr";
	assert(objectIndex >= 0 && objectIndex < 4);
	return crewmanChars[objectIndex] + basename;
}

void StarTrekEngine::updateClockTicks() {
	// TODO (based on DOS interrupt 1A, AH=0; read system clock counter)

	_clockTicks = 0;
}

/**
 * TODO:
 *   - Should return nullptr on failure to open a file?
 *   - This is supposed to cache results, return same FileStream on multiple accesses.
 *   - This is supposed to read from a "patches" folder which overrides files in the
 *     packed blob.
 */
SharedPtr<FileStream> StarTrekEngine::loadFile(Common::String filename, int fileIndex) {
	filename.toUppercase();

	Common::String basename, extension;

	bool bigEndian = getPlatform() == Common::kPlatformAmiga;

	for (int i=filename.size()-1; ; i--) {
		if (filename[i] == '.') {
			basename = filename;
			extension = filename;
			basename.replace(i, filename.size()-i, "");
			extension.replace(0, i+1, "");
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
		if (!file->open(filename.c_str()))
			error("Could not find file \'%s\'", filename.c_str());
		return SharedPtr<FileStream>(new FileStream(file, bigEndian));
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
			basename.setChar(basename.lastChar()-1, basename.size()-1);
			return loadFile(basename + "." + extension, fileIndex+1);
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

	if (getFeatures() & GF_DEMO && getPlatform() == Common::kPlatformDOS) {
		assert(fileCount == 1); // Sanity check...
		Common::SeekableReadStream *stream = dataFile->readStream(uncompressedSize);
		delete dataFile;
		delete dataRunFile;
		return SharedPtr<FileStream>(new FileStream(stream, bigEndian));
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

		Common::SeekableReadStream *stream = decodeLZSS(dataFile->readStream(compressedSize), uncompressedSize);

		delete dataFile;
		delete dataRunFile;
		return SharedPtr<FileStream>(new FileStream(stream, bigEndian));
	}

	// We should not get to this point...
	error("Could not find data for \'%s\'", filename.c_str());

	return SharedPtr<FileStream>();
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

		g_system->delayMillis(10);
	}

	delete qtDecoder;

	// Swap back to 8bpp mode
	initGraphics(320, 200);
}

} // End of namespace StarTrek
