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

#include "common/macresman.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/lingo/lingo.h"

namespace Director {

Archive *DirectorEngine::createArchive() {
	if (getPlatform() == Common::kPlatformMacintosh) {
		if (getVersion() < 4)
			return new MacArchive();
		else
			return new RIFXArchive();
	} else {
		return new RIFFArchive();
	}
}

void DirectorEngine::loadMainArchive() {
	if (getPlatform() == Common::kPlatformWindows)
		loadEXE();
	else
		loadMac();
}

void DirectorEngine::cleanupMainArchive() {
	delete _mainArchive;
	delete _macBinary;
}

void DirectorEngine::loadEXE() {
	Common::SeekableReadStream *exeStream = SearchMan.createReadStreamForMember(getEXEName());
	if (!exeStream)
		error("Failed to open EXE '%s'", getEXEName().c_str());

	_lingo->processEvent(kEventStart, kMovieScript, 0);

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

	debugC(1, kDebugLoading, "Main MMM: '%s'", mmmFileName.c_str());
	debugC(1, kDebugLoading, "Directory Name: '%s'", directoryName.c_str());

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

void DirectorEngine::loadSharedCastsFrom(Common::String filename) {
	Archive *shardcst = createArchive();

	debugC(1, kDebugLoading, "Loading Shared cast '%s'", filename.c_str());

	if (!shardcst->openFile(filename)) {
		warning("No shared cast %s", filename.c_str());

		_sharedCasts = new Common::HashMap<int, Cast *>;

		return;
	}

	_sharedDIB = new Common::HashMap<int, Common::SeekableSubReadStreamEndian *>;
	_sharedSTXT = new Common::HashMap<int, Common::SeekableSubReadStreamEndian *>;
	_sharedSound = new Common::HashMap<int, Common::SeekableSubReadStreamEndian *>;
	_sharedBMP = new Common::HashMap<int, Common::SeekableSubReadStreamEndian *>;

	Score *castScore = new Score(this, shardcst);

	castScore->loadConfig(*shardcst->getResource(MKTAG('V','W','C','F'), 1024));

	if (getVersion() < 4)
		castScore->loadCastDataVWCR(*shardcst->getResource(MKTAG('V','W','C','R'), 1024));

	Common::Array<uint16> cast = shardcst->getResourceIDList(MKTAG('C','A','S','t'));
	if (cast.size() > 0) {
		for (Common::Array<uint16>::iterator iterator = cast.begin(); iterator != cast.end(); ++iterator)
			castScore->loadCastData(*shardcst->getResource(MKTAG('C','A','S','t'), *iterator), *iterator, NULL);
	}

	castScore->setSpriteCasts();

	_sharedCasts = &castScore->_casts;

	Common::Array<uint16> dib = shardcst->getResourceIDList(MKTAG('D','I','B',' '));
	if (dib.size() != 0) {
		debugC(3, kDebugLoading, "Loading %d DIBs", dib.size());

		for (Common::Array<uint16>::iterator iterator = dib.begin(); iterator != dib.end(); ++iterator) {
			debugC(3, kDebugLoading, "Shared DIB %d", *iterator);
			_sharedDIB->setVal(*iterator, shardcst->getResource(MKTAG('D','I','B',' '), *iterator));
		}
	}

	Common::Array<uint16> stxt = shardcst->getResourceIDList(MKTAG('S','T','X','T'));
	if (stxt.size() != 0) {
		debugC(3, kDebugLoading, "Loading %d STXTs", stxt.size());

		for (Common::Array<uint16>::iterator iterator = stxt.begin(); iterator != stxt.end(); ++iterator) {
			debugC(3, kDebugLoading, "Shared STXT %d", *iterator);
			_sharedSTXT->setVal(*iterator, shardcst->getResource(MKTAG('S','T','X','T'), *iterator));
		}
	}

	Common::Array<uint16> bmp = shardcst->getResourceIDList(MKTAG('B','I','T','D'));
	if (bmp.size() != 0) {
		debugC(3, kDebugLoading, "Loading %d BITDs", bmp.size());
		for (Common::Array<uint16>::iterator iterator = bmp.begin(); iterator != bmp.end(); ++iterator) {
			_sharedBMP->setVal(*iterator, shardcst->getResource(MKTAG('B','I','T','D'), *iterator));
		}
	}

	Common::Array<uint16> sound = shardcst->getResourceIDList(MKTAG('S','N','D',' '));
	if (stxt.size() != 0) {
		debugC(3, kDebugLoading, "Loading %d SNDs", sound.size());
		for (Common::Array<uint16>::iterator iterator = sound.begin(); iterator != sound.end(); ++iterator) {
			_sharedSound->setVal(*iterator, shardcst->getResource(MKTAG('S','N','D',' '), *iterator));
		}
	}
}

} // End of namespace Director
