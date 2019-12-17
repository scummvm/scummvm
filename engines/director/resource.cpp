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
#include "common/macresman.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/util.h"
#include "director/lingo/lingo.h"

namespace Director {

Archive *DirectorEngine::createArchive() {
	if (getPlatform() == Common::kPlatformMacintosh) {
		if (getVersion() < 4)
			return new MacArchive();
		else
			return new RIFXArchive();
	} else {
		if (getVersion() < 4)
			return new RIFFArchive();
		else
			return new RIFXArchive();
	}
}

void DirectorEngine::loadInitialMovie(const Common::String movie) {
	if (getPlatform() == Common::kPlatformWindows)
		loadEXE(movie);
	else
		loadMac(movie);
}

Archive *DirectorEngine::openMainArchive(const Common::String movie) {
	delete _mainArchive;

	_mainArchive = createArchive();

	if (!_mainArchive->openFile(movie))
		error("Could not open '%s'", movie.c_str());

	return _mainArchive;
}

void DirectorEngine::cleanupMainArchive() {
	delete _mainArchive;
	delete _macBinary;
}

void DirectorEngine::loadEXE(const Common::String movie) {
	Common::SeekableReadStream *exeStream = SearchMan.createReadStreamForMember(movie);
	if (!exeStream)
		error("Failed to open EXE '%s'", getEXEName().c_str());

	_lingo->processEvent(kEventStart);

	uint32 initialTag = exeStream->readUint32LE();
	if (initialTag == MKTAG('R', 'I', 'F', 'X')) {
		// we've encountered a movie saved from Director, not a projector.
		loadEXERIFX(exeStream, 0);
	} else {
		exeStream->seek(-4, SEEK_END);
		exeStream->seek(exeStream->readUint32LE());

		switch (getVersion()) {
		case 2:
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
}

void DirectorEngine::loadEXEv3(Common::SeekableReadStream *stream) {
	uint16 entryCount = stream->readUint16LE();
	if (entryCount != 1)
		error("Unhandled multiple entry v3 EXE");

	stream->skip(5); // unknown

	uint32 mmmSize = stream->readUint32LE(); // Main MMM size

	Common::String mmmFileName = stream->readPascalString();
	Common::String directoryName = stream->readPascalString();

	debugC(1, kDebugLoading, "Main MMM: '%s'", mmmFileName.c_str());
	debugC(1, kDebugLoading, "Directory Name: '%s'", directoryName.c_str());
	debugC(1, kDebugLoading, "Main mmmSize: %d (0x%x)", mmmSize, mmmSize);

	if (mmmSize) {
		uint32 riffOffset = stream->pos();

		debugC(1, kDebugLoading, "RIFF offset: %d (%x)", riffOffset, riffOffset);

		if (ConfMan.getBool("dump_scripts")) {
			Common::DumpFile out;
			byte *buf = (byte *)malloc(mmmSize);
			stream->read(buf, mmmSize);
			stream->seek(riffOffset);
			Common::String fname = Common::String::format("./dumps/%s", mmmFileName.c_str());


			if (!out.open(fname.c_str())) {
				warning("Can not open dump file %s", fname.c_str());
				return;
			}

			out.write(buf, mmmSize);

			out.flush();
			out.close();

			free(buf);
		}


		_mainArchive = new RIFFArchive();

		if (!_mainArchive->openStream(stream, riffOffset))
			error("Failed to load RIFF from EXE");

		return;
	}

	openMainArchive(mmmFileName);
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
	uint32 ver = stream->readUint32LE();

	if (ver != MKTAG('P', 'J', '9', '5'))
		error("Invalid projector tag found in v5 EXE [%s]", tag2str(ver));

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

void DirectorEngine::loadMac(const Common::String movie) {
	if (getVersion() < 4) {
		// The data is part of the resource fork of the executable
		openMainArchive(movie);
	} else {
		// The RIFX is located in the data fork of the executable
		_macBinary = new Common::MacResManager();

		if (!_macBinary->open(movie) || !_macBinary->hasDataFork())
			error("Failed to open Mac binary '%s'", movie.c_str());

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

	debug(0, "****** Loading Shared cast '%s'", filename.c_str());

	_sharedDIB = new Common::HashMap<int, Common::SeekableSubReadStreamEndian *>;
	_sharedSTXT = new Common::HashMap<int, Common::SeekableSubReadStreamEndian *>;
	_sharedSound = new Common::HashMap<int, Common::SeekableSubReadStreamEndian *>;
	_sharedBMP = new Common::HashMap<int, Common::SeekableSubReadStreamEndian *>;

	if (!shardcst->openFile(filename)) {
		warning("No shared cast %s", filename.c_str());

		return;
	}

	_sharedScore = new Score(this);
	_sharedScore->setArchive(shardcst);

	if (shardcst->hasResource(MKTAG('F', 'O', 'N', 'D'), -1)) {
		debug("Shared cast has fonts. Loading....");

		_wm->_fontMan->loadFonts(filename);
	}

	_sharedScore->loadConfig(*shardcst->getResource(MKTAG('V','W','C','F'), 1024));

	if (getVersion() < 4)
		_sharedScore->loadCastDataVWCR(*shardcst->getResource(MKTAG('V','W','C','R'), 1024));

	Common::Array<uint16> cast = shardcst->getResourceIDList(MKTAG('C','A','S','t'));
	if (cast.size() > 0) {
		debug(0, "****** Loading %d CASt resources", cast.size());

		for (Common::Array<uint16>::iterator iterator = cast.begin(); iterator != cast.end(); ++iterator) {
			Resource res = shardcst->getResourceDetail(MKTAG('C', 'A', 'S', 't'), *iterator);
			_sharedScore->loadCastData(*shardcst->getResource(MKTAG('C', 'A', 'S', 't'), *iterator), *iterator, &res);
		}
	}

	Common::Array<uint16> vwci = shardcst->getResourceIDList(MKTAG('V', 'W', 'C', 'I'));
	if (vwci.size() > 0) {
		debug(0, "****** Loading %d CastInfo resources", vwci.size());

		for (Common::Array<uint16>::iterator iterator = vwci.begin(); iterator != vwci.end(); ++iterator)
			_sharedScore->loadCastInfo(*shardcst->getResource(MKTAG('V', 'W', 'C', 'I'), *iterator), *iterator);
	}

	_sharedScore->setSpriteCasts();

	Common::Array<uint16> dib = shardcst->getResourceIDList(MKTAG('D','I','B',' '));
	if (dib.size() != 0) {
		debugC(3, kDebugLoading, "****** Loading %d DIBs", dib.size());

		for (Common::Array<uint16>::iterator iterator = dib.begin(); iterator != dib.end(); ++iterator) {
			debugC(3, kDebugLoading, "Shared DIB %d", *iterator);
			_sharedDIB->setVal(*iterator, shardcst->getResource(MKTAG('D','I','B',' '), *iterator));
		}
	}

	Common::Array<uint16> stxt = shardcst->getResourceIDList(MKTAG('S','T','X','T'));
	if (stxt.size() != 0) {
		debugC(3, kDebugLoading, "****** Loading %d STXTs", stxt.size());

		for (Common::Array<uint16>::iterator iterator = stxt.begin(); iterator != stxt.end(); ++iterator) {
			debugC(3, kDebugLoading, "Shared STXT %d", *iterator);
			_sharedSTXT->setVal(*iterator, shardcst->getResource(MKTAG('S','T','X','T'), *iterator));
		}
	}

	Common::Array<uint16> bmp = shardcst->getResourceIDList(MKTAG('B','I','T','D'));
	if (bmp.size() != 0) {
		debugC(3, kDebugLoading, "****** Loading %d BITDs", bmp.size());
		for (Common::Array<uint16>::iterator iterator = bmp.begin(); iterator != bmp.end(); ++iterator) {
			debugC(3, kDebugLoading, "Shared BITD %d (%s)", *iterator, numToCastNum(*iterator - 1024));
			_sharedBMP->setVal(*iterator, shardcst->getResource(MKTAG('B','I','T','D'), *iterator));
		}
	}

	Common::Array<uint16> sound = shardcst->getResourceIDList(MKTAG('S','N','D',' '));
	if (sound.size() != 0) {
		debugC(3, kDebugLoading, "****** Loading %d SNDs", sound.size());
		for (Common::Array<uint16>::iterator iterator = sound.begin(); iterator != sound.end(); ++iterator) {
			debugC(3, kDebugLoading, "Shared SND  %d", *iterator);
			_sharedSound->setVal(*iterator, shardcst->getResource(MKTAG('S','N','D',' '), *iterator));
		}
	}

	_sharedScore->loadSpriteImages(true);
}

} // End of namespace Director
