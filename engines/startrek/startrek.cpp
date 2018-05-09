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

StarTrekEngine::StarTrekEngine(OSystem *syst, const StarTrekGameDescription *gamedesc) : Engine(syst), _gameDescription(gamedesc) {
	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound");

	_gfx = nullptr;
	_sound = nullptr;
	_macResFork = nullptr;
	_room = nullptr;

	_clockTicks = 0;

	_musicEnabled = true;
	_sfxEnabled = true;
	_word_467a6 = true;
	_musicWorking = true;
	_sfxWorking = true;
	_finishedPlayingSpeech = false;

	_mouseControllingShip = false;
	_keyboardControlsMouse = true;
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

	initGraphics(320, 200);

	initializeEventsAndMouse();
	
// Test graphics/music:

// Music Status:
// DOS Full: Adlib and MT-32 Sounds supported
// DOS Demo: Adlib and MT-32 Sounds supported
// Amiga: Sound effects supported
// Macintosh: MIDI and sound effects playable, format not handled.

// Graphics Status:
// DOS/Amiga/Macintosh/Demo Graphics: 100%
// Judgment Rites Backgrounds supported too
// EGA not supported
#if 1
	_room = new Room(this, "DEMON0");
	if (getGameType() == GType_ST25) {
		_gfx->loadPalette("PALETTE");
		_gfx->loadPri("DEMON0.PRI");
		_gfx->redrawScreen();
		
		_sound->loadMusicFile("GROUND");
	} else {
		_gfx->drawBackgroundImage("BRIDGE.BGD");
	}

	// Sprite tests

	// Draw mode 0
	Sprite *spr = new Sprite;
	_gfx->addSprite(spr);
	spr->bitmap = _gfx->loadBitmap("MWALKE00");
	spr->drawPriority = 1;
	spr->pos.x = 150;
	spr->pos.y = 100;
	spr->drawMode = 0;

	// Draw mode 2 (translucent background)
	spr = new Sprite;
	_gfx->addSprite(spr);
	spr->bitmap = _gfx->loadBitmap("KWALKS00");
	spr->drawPriority = 1;
	spr->pos.x = 230;
	spr->pos.y = 100;
	spr->drawMode = 2;

	/*
	// Draw mode 3 (text)
	spr = new Sprite;
	_gfx->addSprite(spr);
	spr->bitmap = SharedPtr<Bitmap>(new TextBitmap(8*8,8*8));
	for (int i=0;i<8*8;i++)
		spr->bitmap->pixels[i] = 0x40+i;
	spr->pos.x = 8*10;
	spr->pos.y = 50;
	spr->textColor = 0xb3;
	spr->drawMode = 3;

	// initTextSprite function
	spr = new Sprite;
	int x=0,y=0;
	_gfx->initTextSprite(&x, &y, 0xb3, 3, false, spr);
	spr->pos.y = 150;
	*/


	while (true) {
		_gfx->showOptionsMenu(0, 0);
	}
	_gfx->showText(&Graphics::readTextFromRdf, 0x2220, 150, 160, 0xb3, 0, 10, 0);
	
	while (!shouldQuit()) {
		pollSystemEvents();
	}
#endif

	return Common::kNoError;
}

Room *StarTrekEngine::getRoom() {
	return _room;
}

void StarTrekEngine::pollSystemEvents() {
	Common::Event event;
	TrekEvent trekEvent;

	while (_eventMan->pollEvent(event)) {
		trekEvent.mouse = event.mouse;
		trekEvent.kbd = event.kbd;

		switch (event.type) {
		case Common::EVENT_QUIT:
			_system->quit();
			break;

		case Common::EVENT_MOUSEMOVE:
			trekEvent.type = TREKEVENT_MOUSEMOVE;
			addEventToQueue(trekEvent);
			break;
		case Common::EVENT_LBUTTONDOWN:
			trekEvent.type = TREKEVENT_LBUTTONDOWN;
			addEventToQueue(trekEvent);
			break;
		default:
			break;
		}
	}
	_gfx->drawAllSprites();

	_system->delayMillis(1000/60);
}

void StarTrekEngine::playSoundEffectIndex(int index) {
	switch(index) {
	case 0x04:
		_sound->playSoundEffect("tricorde");
		break;
	case 0x05:
		_sound->playSoundEffect("STDOOR1");
		break;
	case 0x06:
		_sound->playSoundEffect("PHASSHOT");
		break;
	case 0x07:
		_sound->playMidiTrack(index);
		break;
	case 0x08:
		_sound->playSoundEffect("TRANSDEM");
		break;
	case 0x09:
		_sound->playSoundEffect("TRANSMAT");
		break;
	case 0x0a:
		_sound->playSoundEffect("TRANSENE");
		break;
	case 0x10: // Menu selection sound
		_sound->playMidiTrack(index);
		break;
	case 0x22:
		_sound->playSoundEffect("HAILING");
		break;
	case 0x24:
		_sound->playSoundEffect("PHASSHOT");
		break;
	case 0x25:
		_sound->playSoundEffect("PHOTSHOT");
		break;
	case 0x26:
		_sound->playSoundEffect("HITSHIEL");
		break;
	case 0x27:
		_sound->playMidiTrack(index);
		break;
	case 0x28:
		_sound->playSoundEffect("REDALERT");
		break;
	case 0x29:
		_sound->playSoundEffect("WARP");
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

void StarTrekEngine::updateClockTicks() {
	// TODO (based on DOS interrupt 1A, AH=0; read system clock counter)

	_clockTicks = 0;
}

SharedPtr<FileStream> StarTrekEngine::openFile(Common::String filename, int fileIndex) {
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

	// The Judgment Rites demo has its files not in the standard archive
	if (getGameType() == GType_STJR && (getFeatures() & GF_DEMO)) {
		Common::File *file = new Common::File();
		if (!file->open(filename.c_str()))
			error ("Could not find file \'%s\'", filename.c_str());
		return SharedPtr<FileStream>(new FileStream(file, bigEndian));
	}

	Common::SeekableReadStream *indexFile = 0;

	if (getPlatform() == Common::kPlatformAmiga) {
		indexFile = SearchMan.createReadStreamForMember("data000.dir");
		if (!indexFile)
			error ("Could not open data000.dir");
	} else if (getPlatform() == Common::kPlatformMacintosh) {
		indexFile = _macResFork->getResource("Directory");
		if (!indexFile)
			error("Could not find 'Directory' resource in 'Star Trek Data'");
	} else {
		indexFile = SearchMan.createReadStreamForMember("data.dir");
		if (!indexFile)
			error ("Could not open data.dir");
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
			return openFile(basename + "." + extension, fileIndex+1);
		} else
			error ("Could not find file \'%s\'", filename.c_str());
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
