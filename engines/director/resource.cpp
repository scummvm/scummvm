/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/error.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/bufferedstream.h"
#include "common/substream.h"
#include "common/formats/winexe.h"
#include "graphics/wincursor.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/util.h"

namespace Director {

Archive *DirectorEngine::createArchive() {
	if (getVersion() < 400) {
		if (getPlatform() != Common::kPlatformWindows)
			return new MacArchive();
		return new RIFFArchive();
	}
	return new RIFXArchive();
}

Common::Error Window::loadInitialMovie() {
	debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debug(0, "@@@@   Loading initial movie");
	debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	Common::String movie = (_vm->getGameGID() == GID_TESTALL) ? getNextMovieFromQueue().movie : _vm->getEXEName();

	loadINIStream();
	_mainArchive = openArchive(movie);

	if (!_mainArchive) {
		warning("Cannot open main movie");
		return Common::kNoGameDataFoundError;
	}

	_currentMovie = new Movie(this);
	_currentPath = getPath(movie, _currentPath);
	Common::String sharedCastPath = getSharedCastPath();
	if (!sharedCastPath.empty() && !sharedCastPath.equalsIgnoreCase(movie))
		_currentMovie->loadSharedCastsFrom(sharedCastPath);

	// load startup movie
	Common::String startupPath = g_director->getStartupPath();
	if (!startupPath.empty()) {
		Common::SeekableReadStream *const stream = SearchMan.createReadStreamForMember(startupPath);
		if (stream) {
			uint size = stream->size();
			char *script = (char *)calloc(size + 1, 1);

			stream->read(script, size);

			LingoArchive *mainArchive = g_director->getCurrentMovie()->getMainLingoArch();
			mainArchive->addCode(Common::U32String(script, Common::kMacRoman), kMovieScript, 65535);
			_currentMovie->processEvent(kEventStartUp);

			free(script);
			delete stream;
		} else {
			warning("Window::LoadInitialMovie: failed to load startup scripts");
		}
	}

	_currentMovie->setArchive(_mainArchive);

	// XLibs are usually loaded in the initial movie.
	// These may not be present if a --start-movie is specified, so
	// we sometimes need to load them manually.
	if (!g_director->getStartMovie().startMovie.empty())
		loadStartMovieXLibs();

	return Common::kNoError;
}

void Window::probeProjector(const Common::String &movie) {
	if (g_director->getPlatform() == Common::kPlatformWindows)
		return;

	MacArchive *archive = new MacArchive();
	if (!archive->openFile(movie)) {
		delete archive;
		return;
	}

	probeMacBinary(archive);
}

void Window::probeMacBinary(MacArchive *archive) {
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
				delete vvers;
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

			Common::String sname = decodePlatformEncoding(name->readPascalString());
			Common::String moviePath = pathMakeRelative(sname);
			if (testPath(moviePath)) {
				_nextMovie.movie = moviePath;
				warning("Replaced score name with: %s (from %s)", _nextMovie.movie.c_str(), sname.c_str());

				if (_currentMovie) {
					delete _currentMovie;
					_currentMovie = nullptr;
				}

				probeProjector(moviePath);
			} else {
				warning("Couldn't find score with name: %s", sname.c_str());
			}
			delete name;
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
	if (archive->hasResource(MKTAG('X', 'C', 'M', 'D'), -1)) {
		Common::Array<uint16> xcmd = archive->getResourceIDList(MKTAG('X', 'C', 'M', 'D'));
		for (Common::Array<uint16>::iterator iterator = xcmd.begin(); iterator != xcmd.end(); ++iterator) {
			Resource res = archive->getResourceDetail(MKTAG('X', 'C', 'M', 'D'), *iterator);
			debug(0, "Detected XCMD '%s'", res.name.c_str());
			g_lingo->openXLib(res.name, kXObj);
		}
	}
	if (archive->hasResource(MKTAG('X', 'F', 'C', 'N'), -1)) {
		Common::Array<uint16> xfcn = archive->getResourceIDList(MKTAG('X', 'F', 'C', 'N'));
		for (Common::Array<uint16>::iterator iterator = xfcn.begin(); iterator != xfcn.end(); ++iterator) {
			Resource res = archive->getResourceDetail(MKTAG('X', 'F', 'C', 'N'), *iterator);
			debug(0, "Detected XFCN '%s'", res.name.c_str());
			g_lingo->openXLib(res.name, kXObj);
		}
	}
	// Register the resfile so that Cursor::readFromResource can find it
	g_director->_allOpenResFiles.setVal(archive->getPathName(), archive);
}

Archive *Window::openArchive(const Common::String movie) {
	debug(1, "openArchive(\"%s\")", movie.c_str());

	// If the archive is already open, don't reopen it;
	// just init from the existing archive. This prevents errors that
	// can happen when trying to load the same archive more than once.
	if (g_director->_allOpenResFiles.contains(movie) && SearchMan.hasFile(movie)) {
		return g_director->_allOpenResFiles.getVal(movie);
	}

	Archive *result = nullptr;
	if (g_director->getPlatform() == Common::kPlatformWindows) {
		result = loadEXE(movie);
	} else {
		probeProjector(movie);
		result = loadMac(movie);
	}
	if (!result) {
		result = g_director->createArchive();
		if (!result->openFile(movie)) {
			delete result;
			result = nullptr;
		}
	}
	return result;
}

void Window::loadINIStream() {
	Common::SeekableReadStream *iniStream = SearchMan.createReadStreamForMember("LINGO.INI");
	if (iniStream) {
		char *script = (char *)calloc(iniStream->size() + 1, 1);
		iniStream->read(script, iniStream->size());

		_currentMovie = new Movie(this);
		_currentMovie->getMainLingoArch()->addCode(Common::U32String(script, Common::kWindows1252), kMovieScript, 0);
		_currentMovie->processEvent(kEventStartUp);
		delete _currentMovie;
		_currentMovie = nullptr;

		free(script);
		delete iniStream;
	} else {
		warning("No LINGO.INI");
	}
}

Archive *Window::loadEXE(const Common::String movie) {
	Common::SeekableReadStream *exeStream = SearchMan.createReadStreamForMember(Common::Path(movie, g_director->_dirSeparator));
	if (!exeStream) {
		warning("Window::loadEXE(): Failed to open EXE '%s'", g_director->getEXEName().c_str());
		return nullptr;
	}

	Archive *result = nullptr;

	uint32 initialTag = exeStream->readUint32LE();
	if (initialTag == MKTAG('R', 'I', 'F', 'X') || initialTag == MKTAG('X', 'F', 'I', 'R')) {
		// we've encountered a movie saved from Director, not a projector.
		result = loadEXERIFX(exeStream, 0);
	} else if (initialTag == MKTAG('R', 'I', 'F', 'F') || initialTag == MKTAG('F', 'F', 'I', 'R')) { // This is just a normal movie
		result = new RIFFArchive();

		if (!result->openStream(exeStream, 0)) {
			warning("Window::loadEXE(): Failed to load RIFF");
			delete result;
			return nullptr;
		}
	} else {
		Common::WinResources *exe = Common::WinResources::createFromEXE(movie);
		if (!exe) {
			warning("Window::loadEXE(): Failed to open EXE '%s'", g_director->getEXEName().c_str());
			delete exeStream;
			return nullptr;
		}

		const Common::Array<Common::WinResourceID> versions = exe->getIDList(Common::kWinVersion);
		for (uint i = 0; i < versions.size(); i++) {
			Common::WinResources::VersionInfo *info = exe->getVersionResource(versions[i]);

			for (Common::WinResources::VersionHash::const_iterator it = info->hash.begin(); it != info->hash.end(); ++it)
				warning("Window::loadEXE(): info <%s>: <%s>", it->_key.c_str(), it->_value.encode().c_str());

			delete info;

		}

		Common::Array<Common::WinResourceID> idList = exe->getIDList(Common::kWinGroupCursor);
		for (uint i = 0; i < idList.size(); i++) {
			Graphics::WinCursorGroup *group = Graphics::WinCursorGroup::createCursorGroup(exe, idList[i]);
			g_director->_winCursor.push_back(group);
		}

		delete exe;

		exeStream->seek(-4, SEEK_END);
		exeStream->seek(exeStream->readUint32LE());

		if (g_director->getVersion() >= 700) {
			result = loadEXEv7(exeStream);
		} else if (g_director->getVersion() >= 500) {
			result = loadEXEv5(exeStream);
		} else if (g_director->getVersion() >= 400) {
			result = loadEXEv4(exeStream);
		} else if (g_director->getVersion() >= 200) {
			result = loadEXEv3(exeStream);
		} else {
			warning("Window::loadEXE(): Unhandled Windows EXE version %d", g_director->getVersion());
			delete exeStream;
			return nullptr;
		}
	}

	if (result)
		result->setPathName(movie);
	else
		delete exeStream;

	return result;
}

Archive *Window::loadEXEv3(Common::SeekableReadStream *stream) {
	uint32 mmmSize = 0;
	Common::String mmmFileName;
	Common::String directoryName;

	uint16 entryCount = stream->readUint16LE();

	stream->skip(5); // unknown

	for (int i = 0; i < entryCount; ++i) {
		uint32 mmmSize_ = stream->readUint32LE(); // Main MMM size

		Common::String mmmFileName_ = stream->readPascalString();
		Common::String directoryName_ = stream->readPascalString();

		debugC(1, kDebugLoading, "MMM #%d: '%s'", i, mmmFileName_.c_str());
		debugC(1, kDebugLoading, "Directory Name: '%s'", directoryName_.c_str());
		debugC(1, kDebugLoading, "MMM size: %d (0x%x)", mmmSize_, mmmSize_);
		if (i == 0) {
			mmmSize = mmmSize_;
			mmmFileName = mmmFileName_;
			directoryName = directoryName_;
		} else {
			if (!SearchMan.hasFile(Common::Path(mmmFileName_, g_director->_dirSeparator)))
				warning("Window::loadEXEv3(): Failed to find MMM '%s'", mmmFileName_.c_str());
			else {
				Common::SeekableReadStream *const mmmFile_ = SearchMan.createReadStreamForMember(Common::Path(mmmFileName_, g_director->_dirSeparator));
				uint32 mmmFileSize_ = mmmFile_->size();
				if (mmmSize_ != mmmFileSize_)
					warning("Window::loadEXEv3(): File size for '%s' doesn't match. Got %d (0x%x), want %d (0x%x)", mmmFileName_.c_str(), mmmFileSize_, mmmFileSize_, mmmSize_, mmmSize_);
				delete mmmFile_;
			}
		}
		// Print a blank line to separate the entries, format a blank string to silence gcc warning
		debugC(1, kDebugLoading, "%s", "");
	}

	Archive *result = nullptr;
	if (mmmSize) {
		uint32 riffOffset = stream->pos();

		debugC(1, kDebugLoading, "RIFF offset: %d (%x)", riffOffset, riffOffset);

		if (ConfMan.getBool("dump_scripts")) {
			Common::DumpFile out;
			byte *buf = (byte *)malloc(mmmSize);
			stream->read(buf, mmmSize);
			stream->seek(riffOffset);
			Common::String fname = Common::String::format("./dumps/%s", encodePathForDump(mmmFileName).c_str());


			if (!out.open(fname.c_str(), true)) {
				warning("Window::loadEXEv3(): Can not open dump file %s", fname.c_str());
			} else {
				out.write(buf, mmmSize);

				out.flush();
				out.close();
			}

			free(buf);
		}


		result = new RIFFArchive();

		if (result->openStream(stream, riffOffset))
			return result;

		warning("Window::loadEXEv3(): Failed to load RIFF from EXE");
		delete result;
		result = nullptr;
		delete stream;
	}

	result = g_director->createArchive();

	if (!result->openFile(mmmFileName)) {
		warning("Window::loadEXEv3(): Could not open '%s'", mmmFileName.c_str());
		delete result;
		result = nullptr;
	}
	return result;
}

Archive *Window::loadEXEv4(Common::SeekableReadStream *stream) {
	uint32 ver = stream->readUint32BE();

	if (ver != MKTAG('P', 'J', '9', '3')) {
		warning("Window::loadEXEv4(): Invalid projector tag found in v4 EXE [%s]", tag2str(ver));
		return nullptr;
	}

	uint32 rifxOffset = stream->readUint32LE();
	/* uint32 fontMapOffset = */ stream->readUint32LE();
	/* uint32 resourceForkOffset1 = */ stream->readUint32LE();
	/* uint32 resourceForkOffset2 = */ stream->readUint32LE();
	stream->readUint32LE(); // graphics DLL offset
	stream->readUint32LE(); // sound DLL offset
	/* uint32 rifxOffsetAlt = */ stream->readUint32LE(); // equivalent to rifxOffset
	uint32 flags = stream->readUint32LE();

	warning("Window::loadEXEv4(): PJ93 projector flags: %08x", flags);

	return loadEXERIFX(stream, rifxOffset);
}

Archive *Window::loadEXEv5(Common::SeekableReadStream *stream) {
	uint32 ver = stream->readUint32LE();

	if (ver != MKTAG('P', 'J', '9', '5')) {
		warning("Window::loadEXEv5(): Invalid projector tag found in v5 EXE [%s]", tag2str(ver));
		return nullptr;
	}

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

	warning("Window::loadEXEv5(): PJ95 projector pflags: %08x  flags: %08x", pflags, flags);

	return loadEXERIFX(stream, rifxOffset);
}

Archive *Window::loadEXEv7(Common::SeekableReadStream *stream) {
	uint32 ver = stream->readUint32LE();

	if (ver != MKTAG('P', 'J', '0', '0') && ver != MKTAG('P', 'J', '0', '1')) {
		warning("Window::loadEXEv7(): Invalid projector tag found in v7 EXE [%s]", tag2str(ver));
		return nullptr;
	}

	uint32 rifxOffset = stream->readUint32LE();
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // unknown
	stream->readUint32LE(); // some DLL offset

	return loadEXERIFX(stream, rifxOffset);
}

Archive *Window::loadEXERIFX(Common::SeekableReadStream *stream, uint32 offset) {
	Archive *result = new RIFXArchive();

	if (!result->openStream(stream, offset)) {
		warning("Window::loadEXERIFX(): Failed to load RIFX from EXE");
		delete result;
		result = nullptr;
	}
	return result;
}

Archive *Window::loadMac(const Common::String movie) {
	Archive *result = nullptr;
	if (g_director->getVersion() < 400) {
		// The data is part of the resource fork of the executable
		result = g_director->createArchive();

		if (!result->openFile(movie)) {
			delete result;
			result = nullptr;
			warning("Window::loadMac(): Could not open '%s'", movie.c_str());
		}
	} else {
		// The RIFX is located in the data fork of the executable
		Common::SeekableReadStream *dataFork = Common::MacResManager::openFileOrDataFork(Common::Path(movie, g_director->_dirSeparator));
		if (!dataFork) {
			warning("Window::loadMac(): Failed to open Mac binary '%s'", movie.c_str());
			return nullptr;
		}
		result = new RIFXArchive();
		result->setPathName(movie);

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

		if (!result->openStream(dataFork, startOffset)) {
			warning("Window::loadMac(): Failed to load RIFX from Mac binary");
			delete result;
			result = nullptr;
			if (_currentMovie) {
				delete _currentMovie;
				_currentMovie = nullptr;
			}
		}
	}
	return result;
}

void Window::loadStartMovieXLibs() {
	if (strcmp(g_director->getGameId(), "warlock") == 0 && g_director->getPlatform() != Common::kPlatformWindows) {
		g_lingo->openXLib("FPlayXObj", kXObj);
	}
	g_lingo->openXLib("SerialPort", kXObj);
}

/*******************************************
 *
 * Projector Archive
 *
 *******************************************/

class ProjectorArchive : public Common::Archive {
public:
	ProjectorArchive(Common::String path);
	~ProjectorArchive() override;

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	Common::SeekableReadStream *createBufferedReadStream();
	bool loadArchive(Common::SeekableReadStream *stream);

	struct Entry {
		uint32 offset;
		uint32 size;
	};
	typedef Common::HashMap<Common::String, Entry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;
	FileMap _files;
	Common::String _path;
};

ProjectorArchive::ProjectorArchive(Common::String path)
	: _path(path), _files() {

	// Buffer 100K into memory
	Common::SeekableReadStream *stream = createBufferedReadStream();

	// Build our filemap using the buffered stream
	loadArchive(stream);

	delete stream;
}

Common::SeekableReadStream *ProjectorArchive::createBufferedReadStream() {
	const uint32 READ_BUFFER_SIZE = 1024 * 100;

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(_path);
	if (!stream)
		error("ProjectorArchive::createBufferedReadStream(): Cannot open %s", _path.c_str());

	return Common::wrapBufferedSeekableReadStream(stream, READ_BUFFER_SIZE, DisposeAfterUse::NO);
}

ProjectorArchive::~ProjectorArchive() {
	_files.clear();
}

bool ProjectorArchive::loadArchive(Common::SeekableReadStream *stream) {
	bool bigEndian = false, found = false;
	uint32 off, tag, rifxOffset;

	stream->seek(-4, SEEK_END);
	off = stream->readUint32LE();
	stream->seek(off);
	tag = stream->readUint32BE();

	// Check whether we got a 'PJ' tag while ignoring the version and accounting for endianness
	if (((tag & 0xffff0000) != MKTAG('P','J', 0, 0)) && ((tag & 0x0000ffff) != MKTAG(0, 0, 'J','P'))) {
		warning("ProjectorArchive::loadArchive(): Projector Tag not found");
		return false;
	}

	rifxOffset = stream->readUint32LE();

	// if value is bigger than the stream size this is most likely a endianness issue.
	if (rifxOffset > stream->size()) {
		bigEndian = true;
		rifxOffset = SWAP_BYTES_32(rifxOffset);
	}

	stream->seek(rifxOffset);
	tag = stream->readUint32BE();

	debugC(1, kDebugLoading, "File: %s off: 0x%x, tag: %s rifx: 0x%x", _path.c_str(), off, tag2str(tag), rifxOffset);

	// Try to locate the very next Dict tag(byte-by-byte)
	tag = stream->readUint32BE();
	found = false;

	// This loop has neglible performance impact due to the stream being buffered.
	// Furthermore, comparing 4 bytes at a time should be pretty fast on mordern systems.
	while (!stream->eos()) {
		if (tag == MKTAG('D', 'i', 'c', 't') || tag == MKTAG('t', 'c', 'i', 'D')) {
			found = true;
			break;
		}

		stream->seek(-3, SEEK_CUR);
		tag = stream->readUint32BE();
	}

	// Return if dict tag is not found
	if (!found) {
		warning("ProjectorArchive::loadArchive(): Dict Tag not found.");
		return false;
	}

	uint32 size = bigEndian ? stream->readUint32BE() : stream->readUint32LE();
	uint32 dictOff = bigEndian ? stream->readUint32BE() : stream->readUint32LE();

	// Jump to the actual dict block and parse it
	if (!stream->seek(dictOff)) {
		warning("BUILDBOT: ProjectorArchive::loadArchive(): Incorrect dict offset (0x%x)", dictOff);
		return false;
	}

	tag = bigEndian ? stream->readUint32BE() : stream->readUint32LE();
	size = bigEndian ? stream->readUint32BE() : stream->readUint32LE();
	stream->seek(dictOff + 24);
	uint32 cnt = bigEndian ? stream->readUint32BE() : stream->readUint32LE();

	// Correction for when there is only a single element present according to the dict
	uint8 offsetDict = bigEndian && cnt == 1 ? 2 : 0;
	bool oBigEndian = bigEndian;

	// For 16-bit win projector
	if (cnt > 0xFFFF) {
		cnt = SWAP_BYTES_32(cnt);
		offsetDict = 2;
		bigEndian = true;
	}

	debugC(1, kDebugLoading, "Dict off: 0x%x, Size: 0x%x cnt: %d", dictOff, size, cnt);

	uint32 pt = (cnt * 8) + (64 - offsetDict);
	Common::StringArray arr(cnt);

	for (uint32 i = 0; i < cnt; i++) {
		// if this fails it likely means that the dict was invalid.
		if (!stream->seek(dictOff + pt, SEEK_SET)) {
			warning("ProjectorArchive::loadArchive(): Incorrect entry name offset (0x%x)", dictOff + pt);
			return false;
		}

		uint32 namelen = bigEndian ? stream->readUint32BE() : stream->readUint32LE();
		arr[i] = stream->readString(0, namelen);

		if (i < cnt - 1) {
			int sub = (namelen % 4) ? (namelen % 4) : 4;
			pt += 4 + namelen + (4 - sub);
		}
	}

	bigEndian = oBigEndian;

	// Jump to the first block which should start right after the dict while making sure size is aligned.
	size += (size % 2);
	stream->seek(dictOff + size + 8, SEEK_SET);

	for (uint32 i = 0; i < cnt; i++) {
		tag = stream->readUint32BE();
		size = bigEndian ? stream->readUint32BE() : stream->readUint32LE();

		// endianness issue, swap size and continue
		if (size > stream->pos()) {
			bigEndian = !bigEndian;
			size = SWAP_BYTES_32(size);
		}

		debugC(1, kDebugLoading, "Entry: %s offset %lX tag %s size %d", arr[i].c_str(), stream->pos() - 8, tag2str(tag), size);

		Entry entry;

		// subtract 8 since we want to include tag and size as well
		entry.offset = static_cast<uint32>(stream->pos() - 8);
		entry.size = size + 8;
		_files[arr[i]] = entry;

		// Align size for the next seek.
		size += (size % 2);

		// if this fails it suggests something is either wrong with the dict or the file itself.
		if (!stream->seek(size, SEEK_CUR)) {
			warning("ProjectorArchive::loadArchive(): Could not read next block (0x%x) Prev Block(0x%x : %d)",
			        entry.offset + entry.size, entry.offset, entry.size);
			return false;
		}
	}

	return true;
}

bool ProjectorArchive::hasFile(const Common::Path &path) const {
	Common::String name = path.toString();
	return (_files.find(name) != _files.end());
}

int ProjectorArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (FileMap::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, this)));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr ProjectorArchive::getMember(const Common::Path &path) const {
	Common::String name = path.toString();

	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *ProjectorArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::String name = path.toString();
	FileMap::const_iterator fDesc = _files.find(name);

	if (fDesc == _files.end())
		return nullptr;

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(_path);

	stream->seek(fDesc->_value.offset, SEEK_SET);
	byte *data = (byte *)malloc(fDesc->_value.size);
	stream->read(data, fDesc->_value.size);
	delete stream;

	return new Common::MemoryReadStream(data, fDesc->_value.size, DisposeAfterUse::YES);
}
} // End of namespace Director
