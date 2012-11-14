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

#include "common/debug.h"
#include "common/scummsys.h"
#include "common/error.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "director/director.h"
#include "director/resource.h"

namespace Director {

DirectorEngine::DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	if (!_mixer->isReady())
		error("Sound initialization failed");

	// Setup mixer
	syncSoundSettings();

	_mainArchive = 0;
}

DirectorEngine::~DirectorEngine() {
	delete _mainArchive;
}

Common::Error DirectorEngine::run() {
	debug("Starting v%d Director game", getVersion());

	if (getPlatform() == Common::kPlatformWindows)
		loadEXE();

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
	}
}

void DirectorEngine::loadEXEv3(Common::SeekableReadStream *stream) {
	stream->readUint32LE(); // unknown
	stream->readUint16LE(); // unknown
	stream->readUint32LE(); // Main MMM size
	stream->readByte();     // zero
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

void DirectorEngine::loadEXERIFX(Common::SeekableReadStream *stream, uint32 offset) {
	stream->seek(offset);

	_mainArchive = new RIFXArchive();

	if (!_mainArchive->openStream(stream))
		error("Failed to load RIFX from EXE");
}

Common::String DirectorEngine::readPascalString(Common::SeekableReadStream &stream) {
	byte length = stream.readByte();
	Common::String x;

	while (length--)
		x += (char)stream.readByte();

	return x;
}

} // End of namespace Director
