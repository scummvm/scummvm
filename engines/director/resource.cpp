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
#include "common/error.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/substream.h"
#include "common/winexe.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/util.h"

namespace Director {

Archive *DirectorEngine::createArchive() {
	if (getPlatform() == Common::kPlatformMacintosh) {
		if (getVersion() < 400)
			return new MacArchive();
		else
			return new RIFXArchive();
	} else {
		if (getVersion() < 400)
			return new RIFFArchive();
		else
			return new RIFXArchive();
	}
}

Common::Error Window::loadInitialMovie() {
	debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debug(0, "@@@@   Loading initial movie");
	debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	Common::String movie = (_vm->getGameGID() == GID_TESTALL) ? getNextMovieFromQueue().movie : _vm->getEXEName();

	probeProjector(movie);

	if (g_director->getPlatform() == Common::kPlatformWindows)
		loadEXE(movie);
	else
		loadMac(movie);

	if (!_mainArchive) {
		warning("Cannot open main movie");
		return Common::kNoGameDataFoundError;
	}

	_currentMovie = new Movie(this);
	_currentPath = getPath(movie, _currentPath);
	_currentMovie->loadSharedCastsFrom(_currentPath + g_director->_sharedCastFile);
	_currentMovie->setArchive(_mainArchive);

	return Common::kNoError;
}

void Window::probeProjector(const Common::String &movie) {
	if (g_director->getPlatform() == Common::kPlatformWindows)
		return;

	Director::MacArchive *archive = new MacArchive();

	if (!archive->openFile(movie)) {
		delete archive;

		return;
	}

	// Let's check if it is a projector file
	// So far tested with Spaceship Warlock, D2
	if (archive->hasResource(MKTAG('B', 'N', 'D', 'L'), "Projector")) {
		warning("Detected Projector file");

		if (archive->hasResource(MKTAG('v', 'e', 'r', 's'), -1)) {
			Common::Array<uint16> vers = archive->getResourceIDList(MKTAG('v', 'e', 'r', 's'));
			for (Common::Array<uint16>::iterator iterator = vers.begin(); iterator != vers.end(); ++iterator) {
				Common::SeekableReadStreamEndian *vvers = archive->getResource(MKTAG('v', 'e', 'r', 's'), *iterator);
				Common::MacResManager::MacVers *v = Common::MacResManager::parseVers(vvers);

				debug(0, "Detected vers %d.%d %s.%d region %d '%s' '%s'", v->majorVer, v->minorVer, v->devStr.c_str(),
					v->preReleaseVer, v->region, v->str.c_str(), v->msg.c_str());

				delete v;
			}
		}

		if (archive->hasResource(MKTAG('X', 'C', 'O', 'D'), -1)) {
			Common::Array<uint16> xcod = archive->getResourceIDList(MKTAG('X', 'C', 'O', 'D'));
			for (Common::Array<uint16>::iterator iterator = xcod.begin(); iterator != xcod.end(); ++iterator) {
				Resource res = archive->getResourceDetail(MKTAG('X', 'C', 'O', 'D'), *iterator);
				debug(0, "Detected XObject '%s'", res.name.c_str());
				g_lingo->openXLib(res.name, kXObj);
			}
		}

		if (archive->hasResource(MKTAG('S', 'T', 'R', '#'), 0)) {
			if (_currentMovie)
				_currentMovie->setArchive(archive);

			Common::SeekableReadStreamEndian *name = archive->getResource(MKTAG('S', 'T', 'R', '#'), 0);
			int num = name->readUint16();
			if (num != 1) {
				warning("Incorrect number of strings in Projector file");
			}

			if (num == 0)
				error("No strings in Projector file");

			Common::String sname = name->readPascalString();

			_nextMovie.movie = pathMakeRelative(sname);
			warning("Replaced score name with: %s (from %s)", _nextMovie.movie.c_str(), sname.c_str());

			delete _currentMovie;
			_currentMovie = nullptr;

			delete name;
		}
	}

	delete archive;
}

Archive *Window::openMainArchive(const Common::String movie) {
	debug(1, "openMainArchive(\"%s\")", movie.c_str());

	_mainArchive = g_director->createArchive();

	if (!_mainArchive->openFile(movie)) {
		delete _mainArchive;
		_mainArchive = nullptr;

		warning("openMainArchive(): Could not open '%s'", movie.c_str());
		return nullptr;
	}

	return _mainArchive;
}

void Window::loadEXE(const Common::String movie) {
	Common::SeekableReadStream *iniStream = SearchMan.createReadStreamForMember("LINGO.INI");
	if (iniStream) {
		char *script = (char *)calloc(iniStream->size() + 1, 1);
		iniStream->read(script, iniStream->size());

		_currentMovie = new Movie(this);
		_currentMovie->getMainLingoArch()->addCode(script, kMovieScript, 0);
		_currentMovie->processEvent(kEventStartUp);
		delete _currentMovie;
		_currentMovie = nullptr;

		free(script);
	} else {
		warning("No LINGO.INI");
	}

	Common::SeekableReadStream *exeStream = SearchMan.createReadStreamForMember(movie);
	if (!exeStream)
		error("Failed to open EXE '%s'", g_director->getEXEName().c_str());

	uint32 initialTag = exeStream->readUint32LE();
	if (initialTag == MKTAG('R', 'I', 'F', 'X') || initialTag == MKTAG('X', 'F', 'I', 'R')) {
		// we've encountered a movie saved from Director, not a projector.
		loadEXERIFX(exeStream, 0);
	} else if (initialTag == MKTAG('R', 'I', 'F', 'F') || initialTag == MKTAG('F', 'F', 'I', 'R')) { // This is just a normal movie
		_mainArchive = new RIFFArchive();

		if (!_mainArchive->openStream(exeStream, 0))
			error("Failed to load RIFF");
	} else {
		Common::WinResources *exe = Common::WinResources::createFromEXE(movie);
		if (!exe)
			error("Failed to open EXE '%s'", g_director->getEXEName().c_str());

		const Common::Array<Common::WinResourceID> versions = exe->getIDList(Common::kWinVersion);
		for (uint i = 0; i < versions.size(); i++) {
			Common::SeekableReadStream *res = exe->getResource(Common::kWinVersion, versions[i]);

			Common::WinResources::VersionHash *versionMap = Common::WinResources::parseVersionInfo(res);

			for (Common::WinResources::VersionHash::const_iterator it = versionMap->begin(); it != versionMap->end(); ++it)
				warning("info <%s>: <%s>", it->_key.c_str(), it->_value.encode().c_str());

			delete versionMap;
			delete res;

		}
		delete exe;

		exeStream->seek(-4, SEEK_END);
		exeStream->seek(exeStream->readUint32LE());

		if (g_director->getVersion() >= 700) {
			loadEXEv7(exeStream);
		} else if (g_director->getVersion() >= 500) {
			loadEXEv5(exeStream);
		} else if (g_director->getVersion() >= 400) {
			loadEXEv4(exeStream);
		} else if (g_director->getVersion() >= 200) {
			loadEXEv3(exeStream);
		} else {
			error("Unhandled Windows EXE version %d", g_director->getVersion());
		}
	}

	if (_mainArchive)
		_mainArchive->setPathName(movie);
}

void Window::loadEXEv3(Common::SeekableReadStream *stream) {
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


			if (!out.open(fname.c_str(), true)) {
				warning("Window::loadEXEv3(): Can not open dump file %s", fname.c_str());
			} else {
				out.write(buf, mmmSize);

				out.flush();
				out.close();
			}

			free(buf);
		}


		_mainArchive = new RIFFArchive();

		if (!_mainArchive->openStream(stream, riffOffset))
			warning("Failed to load RIFF from EXE");
		else
			return;
	}

	openMainArchive(mmmFileName);
}

void Window::loadEXEv4(Common::SeekableReadStream *stream) {
	if (stream->readUint32BE() != MKTAG('P', 'J', '9', '3'))
		error("Invalid projector tag found in v4 EXE");

	uint32 rifxOffset = stream->readUint32LE();
	/* uint32 fontMapOffset = */ stream->readUint32LE();
	/* uint32 resourceForkOffset1 = */ stream->readUint32LE();
	/* uint32 resourceForkOffset2 = */ stream->readUint32LE();
	stream->readUint32LE(); // graphics DLL offset
	stream->readUint32LE(); // sound DLL offset
	/* uint32 rifxOffsetAlt = */ stream->readUint32LE(); // equivalent to rifxOffset
	uint32 flags = stream->readUint32LE();

	warning("PJ93 projector flags: %08x", flags);

	loadEXERIFX(stream, rifxOffset);
}

void Window::loadEXEv5(Common::SeekableReadStream *stream) {
	uint32 ver = stream->readUint32LE();

	if (ver != MKTAG('P', 'J', '9', '5'))
		error("Invalid projector tag found in v5 EXE [%s]", tag2str(ver));

	uint32 rifxOffset = stream->readUint32LE();
	uint32 pflags = stream->readUint32LE();
	uint32 flags = stream->readUint32LE();
	stream->readUint16LE();	// x
	stream->readUint16LE(); // y
	stream->readUint16LE(); // screenWidth
	stream->readUint16LE(); // screenHeight
	stream->readUint32LE(); // number of components
	stream->readUint32LE(); // number of driver files
	stream->readUint32LE(); // fontMapOffset

	warning("PJ95 projector pflags: %08x  flags: %08x", pflags, flags);

	loadEXERIFX(stream, rifxOffset);
}

void Window::loadEXEv7(Common::SeekableReadStream *stream) {
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

void Window::loadEXERIFX(Common::SeekableReadStream *stream, uint32 offset) {
	_mainArchive = new RIFXArchive();

	if (!_mainArchive->openStream(stream, offset))
		error("Failed to load RIFX from EXE");
}

void Window::loadMac(const Common::String movie) {
	if (g_director->getVersion() < 400) {
		// The data is part of the resource fork of the executable
		openMainArchive(movie);
	} else {
		// The RIFX is located in the data fork of the executable
		_macBinary = new Common::MacResManager();

		if (!_macBinary->open(movie) || !_macBinary->hasDataFork())
			error("Failed to open Mac binary '%s'", movie.c_str());

		Common::SeekableReadStream *dataFork = _macBinary->getDataFork();
		_mainArchive = new RIFXArchive();
		_mainArchive->setPathName(movie);

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

		if (!_mainArchive->openStream(dataFork, startOffset)) {
			warning("Failed to load RIFX from Mac binary");
			delete _currentMovie;
			_currentMovie = nullptr;
		}
	}
}

} // End of namespace Director
