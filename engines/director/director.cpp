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
#include "common/macresman.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "director/dib.h"

#include "director/director.h"
#include "director/resource.h"

namespace Director {

DirectorEngine::DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	if (!_mixer->isReady())
		error("Sound initialization failed");

	// Setup mixer
	syncSoundSettings();

	_mainArchive = 0;
	_macBinary = 0;

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(gameDataDir, "install");

	//FIXME
	RIFFArchive riff;
	riff.openFile("bookshelf_example.mmm");
	Director::DIBDecoder img;
	Common::SeekableReadStream *dib = riff.getResource(MKTAG('D', 'I', 'B', ' '), 1103);
	img.loadStream(*dib);
	Common::SeekableReadStream *pal = riff.getResource(MKTAG('C', 'L', 'U', 'T'), 1025);
	img.loadPalette(*pal);
}

DirectorEngine::~DirectorEngine() {
	delete _mainArchive;
	delete _macBinary;
}

Common::Error DirectorEngine::run() {
	debug("Starting v%d Director game", getVersion());

	if (getPlatform() == Common::kPlatformWindows)
		loadEXE();
	else
		loadMac();

	return Common::kNoError;
}

void DirectorEngine::loadEXE() {
	Common::SeekableReadStream *exeStream = SearchMan.createReadStreamForMember(getEXEName());
	if (!exeStream)
		error("Failed to open EXE '%s'", getEXEName().c_str());

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

} // End of namespace Director
