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

#include "audio/mixer.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/scummsys.h"
#include "common/error.h"
#include "common/events.h"
#include "common/macresman.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/fs.h"

#include "engines/util.h"

#include "graphics/surface.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/dib.h"
#include "director/resource.h"
#include "director/score.h"
#include "director/lingo/lingo.h"
#include "director/sound.h"

namespace Director {

DirectorEngine::DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc),
		_rnd("director") {
	if (!_mixer->isReady())
		error("Sound initialization failed");

	// Setup mixer
	syncSoundSettings();

	_sharedCasts = nullptr;
	_sharedSound = nullptr;
	_sharedBMP = nullptr;
	_sharedSTXT = nullptr;
	_sharedDIB = nullptr;

	_mainArchive = nullptr;
	_macBinary = nullptr;

	_movies = nullptr;

	_wm = nullptr;

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(gameDataDir, "install");
}

DirectorEngine::~DirectorEngine() {
	delete _sharedCasts;
	delete _sharedSound;
	delete _sharedBMP;
	delete _sharedSTXT;
	delete _sharedDIB;
	delete _movies;

	delete _mainArchive;
	delete _macBinary;
	delete _soundManager;
	delete _lingo;
	delete _currentScore;
	delete _currentPalette;
}

Common::Error DirectorEngine::run() {
	debug("Starting v%d Director game", getVersion());

	//FIXME
	_sharedMMM = "SHARDCST.MMM";

	_currentPalette = nullptr;

	_macBinary = nullptr;
	_soundManager = nullptr;

	_wm = new Graphics::MacWindowManager;

	_lingo = new Lingo(this);
	_soundManager = new DirectorSound();

	if (getGameID() == GID_TEST) {
		_mainArchive = nullptr;
		_currentScore = nullptr;

		_lingo->runTests();

		return Common::kNoError;
	}

	//FIXME
	//_mainArchive = new RIFFArchive();
	//_mainArchive->openFile("bookshelf_example.mmm");

	if (getPlatform() == Common::kPlatformWindows)
		loadEXE();
	else
		loadMac();

	_currentScore = new Score(this);
	debug(0, "Score name %s", _currentScore->getMacName().c_str());

	_currentScore->loadArchive();
	_currentScore->startLoop();

	return Common::kNoError;
}

Common::HashMap<Common::String, Score *> DirectorEngine::loadMMMNames(Common::String folder) {
	Common::FSNode directory(folder);
	Common::FSList movies;

	Common::HashMap<Common::String, Score *> nameMap;
	directory.getChildren(movies, Common::FSNode::kListFilesOnly);

	if (!movies.empty()) {
		for (Common::FSList::const_iterator i = movies.begin(); i != movies.end(); ++i) {
			if (i->getName() == _sharedMMM) {
				loadSharedCastsFrom(i->getPath());
				continue;
			}

			RIFFArchive *arc = new RIFFArchive();
			arc->openFile(i->getPath());
			Score *sc = new Score(this);
			nameMap[sc->getMacName()] = sc;
		}
	}

	return nameMap;
}

void DirectorEngine::loadEXE() {
	Common::SeekableReadStream *exeStream = SearchMan.createReadStreamForMember(getEXEName());
	if (!exeStream)
		error("Failed to open EXE '%s'", getEXEName().c_str());

	_lingo->processEvent(kEventStart, 0);

	exeStream->seek(-4, SEEK_END);
	exeStream->seek(exeStream->readUint32LE());

	switch (getVersion()) {
	case 3:
		loadEXEv3(exeStream);
		break;
	case 4:
		loadEXEv4(exeStream);
		break;
	case 5:
		loadEXEv5(exeStream);
		break;
	case 7:
		loadEXEv7(exeStream);
		break;
	default:
		error("Unhandled Windows EXE version %d", getVersion());
	}
}

void DirectorEngine::loadEXEv3(Common::SeekableReadStream *stream) {
	uint16 entryCount = stream->readUint16LE();
	if (entryCount != 1)
		error("Unhandled multiple entry v3 EXE");

	stream->skip(5); // unknown

	stream->readUint32LE(); // Main MMM size
	Common::String mmmFileName = readPascalString(*stream);
	Common::String directoryName = readPascalString(*stream);

	debug("Main MMM: '%s'", mmmFileName.c_str());
	debug("Directory Name: '%s'", directoryName.c_str());

	_mainArchive = new RIFFArchive();

	if (!_mainArchive->openFile(mmmFileName))
		error("Could not open '%s'", mmmFileName.c_str());

	delete stream;
}

void DirectorEngine::loadEXEv4(Common::SeekableReadStream *stream) {
	if (stream->readUint32BE() != MKTAG('P', 'J', '9', '3'))
		error("Invalid projector tag found in v4 EXE");

	uint32 rifxOffset = stream->readUint32LE();
	/* uint32 fontMapOffset = */ stream->readUint32LE();
	/* uint32 resourceForkOffset1 = */ stream->readUint32LE();
	/* uint32 resourceForkOffset2 = */ stream->readUint32LE();
	stream->readUint32LE(); // graphics DLL offset
	stream->readUint32LE(); // sound DLL offset
	/* uint32 rifxOffsetAlt = */ stream->readUint32LE(); // equivalent to rifxOffset

	loadEXERIFX(stream, rifxOffset);
}

void DirectorEngine::loadEXEv5(Common::SeekableReadStream *stream) {
	if (stream->readUint32LE() != MKTAG('P', 'J', '9', '5'))
		error("Invalid projector tag found in v5 EXE");

	uint32 rifxOffset = stream->readUint32LE();
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // unknown
	/* uint16 screenWidth = */ stream->readUint16LE();
	/* uint16 screenHeight = */ stream->readUint16LE();
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // unknown
	/* uint32 fontMapOffset = */ stream->readUint32LE();

	loadEXERIFX(stream, rifxOffset);
}

void DirectorEngine::loadEXEv7(Common::SeekableReadStream *stream) {
	if (stream->readUint32LE() != MKTAG('P', 'J', '0', '0'))
		error("Invalid projector tag found in v7 EXE");

	uint32 rifxOffset = stream->readUint32LE();
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // some DLL offset

	loadEXERIFX(stream, rifxOffset);
}

void DirectorEngine::loadEXERIFX(Common::SeekableReadStream *stream, uint32 offset) {
	_mainArchive = new RIFXArchive();

	if (!_mainArchive->openStream(stream, offset))
		error("Failed to load RIFX from EXE");
}

void DirectorEngine::loadMac() {
	if (getVersion() < 4) {
		// The data is part of the resource fork of the executable
		_mainArchive = new MacArchive();

		if (!_mainArchive->openFile(getEXEName()))
			error("Failed to open Mac binary '%s'", getEXEName().c_str());
	} else {
		// The RIFX is located in the data fork of the executable
		_macBinary = new Common::MacResManager();

		if (!_macBinary->open(getEXEName()) || !_macBinary->hasDataFork())
			error("Failed to open Mac binary '%s'", getEXEName().c_str());

		Common::SeekableReadStream *dataFork = _macBinary->getDataFork();
		_mainArchive = new RIFXArchive();

		// First we need to detect PPC vs. 68k

		uint32 tag = dataFork->readUint32BE();
		uint32 startOffset;

		if (SWAP_BYTES_32(tag) == MKTAG('P', 'J', '9', '3') || tag == MKTAG('P', 'J', '9', '5') || tag == MKTAG('P', 'J', '0', '0')) {
			// PPC: The RIFX shares the data fork with the binary
			startOffset = dataFork->readUint32BE();
		} else {
			// 68k: The RIFX is the only thing in the data fork
			startOffset = 0;
		}

		if (!_mainArchive->openStream(dataFork, startOffset))
			error("Failed to load RIFX from Mac binary");
	}
}

Common::String DirectorEngine::readPascalString(Common::SeekableReadStream &stream) {
	byte length = stream.readByte();
	Common::String x;

	while (length--)
		x += (char)stream.readByte();

	return x;
}

void DirectorEngine::setPalette(byte *palette, uint16 count) {
	_currentPalette = palette;
	_currentPaletteLength = count;
}

void DirectorEngine::loadSharedCastsFrom(Common::String filename) {
	Archive *shardcst;

	if (getVersion() < 4) {
		shardcst = new RIFFArchive();
	} else {
		shardcst = new RIFXArchive();
	}

	shardcst->openFile(filename);

	Score *castScore = new Score(this);
	Common::SeekableSubReadStreamEndian *castStream = shardcst->getResource(MKTAG('V','W','C','R'), 1024);

	castScore->loadCastData(*castStream);
	*_sharedCasts = castScore->_casts;

	Common::Array<uint16> dib = shardcst->getResourceIDList(MKTAG('D','I','B',' '));

	if (dib.size() != 0) {
		Common::Array<uint16>::iterator iterator;
		for (iterator = dib.begin(); iterator != dib.end(); ++iterator) {
			debug(3, "Shared DIB %d", *iterator);
			_sharedDIB->setVal(*iterator, shardcst->getResource(MKTAG('D','I','B',' '), *iterator));
		}
	}

	Common::Array<uint16> stxt = shardcst->getResourceIDList(MKTAG('S','T','X','T'));

	if (stxt.size() != 0) {
		Common::Array<uint16>::iterator iterator;
		for (iterator = stxt.begin(); iterator != stxt.end(); ++iterator) {
			debug(3, "Shared STXT %d", *iterator);
			_sharedSTXT->setVal(*iterator, shardcst->getResource(MKTAG('S','T','X','T'), *iterator));
		}
	}

	Common::Array<uint16> bmp = shardcst->getResourceIDList(MKTAG('B','I','T','D'));

	if (bmp.size() != 0) {
		Common::Array<uint16>::iterator iterator;
		for (iterator = bmp.begin(); iterator != bmp.end(); ++iterator) {
			_sharedBMP->setVal(*iterator, shardcst->getResource(MKTAG('B','I','T','D'), *iterator));
		}
	}

	Common::Array<uint16> sound = shardcst->getResourceIDList(MKTAG('S','N','D',' '));

	if (stxt.size() != 0) {
		Common::Array<uint16>::iterator iterator;
		for (iterator = sound.begin(); iterator != sound.end(); ++iterator) {
			_sharedSound->setVal(*iterator, shardcst->getResource(MKTAG('S','N','D',' '), *iterator));
		}
	}
}

} // End of namespace Director
