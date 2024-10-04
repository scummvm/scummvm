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
#include "common/fs.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/bufferedstream.h"
#include "common/substream.h"
#include "common/formats/winexe.h"
#include "director/types.h"
#include "graphics/wincursor.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/util.h"
#include "director/window.h"

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
	Common::String movie = (_vm->getGameGID() == GID_TESTALL) ? getNextMovieFromQueue().movie : _vm->getEXEName();

	debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debug(0, "@@@@   Loading initial movie '%s'", movie.c_str());
	debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	if (movie.empty())
		return Common::kPathNotFile;

	loadINIStream();
	Common::Path path = findPath(movie);
	_mainArchive = g_director->openArchive(path);

	if (!_mainArchive) {
		warning("Cannot open main movie");
		return Common::kNoGameDataFoundError;
	}
	probeResources(_mainArchive);

	// Load multiple-resources based executable file (Projector)
	Common::String rawEXE = _vm->getRawEXEName();
	ProjectorArchive *multiArchive = new ProjectorArchive(findPath(rawEXE));
	if (multiArchive->isLoaded()) {
		// A valid projector archive, add to SearchMan
		SearchMan.add(_vm->getRawEXEName(), multiArchive);

		if (ConfMan.getBool("dump_scripts"))
			multiArchive->dumpArchive(Common::Path("./dumps").join(encodePathForDump(movie)));

	} else {
		delete multiArchive;
	}

	_currentMovie = new Movie(this);
	_currentPath = getPath(movie, _currentPath);
	Common::Path sharedCastPath = getSharedCastPath();
	if (!sharedCastPath.empty() && !(sharedCastPath == path))
		_currentMovie->loadSharedCastsFrom(sharedCastPath);

	// load startup movie
	Common::Path startupPath = g_director->getStartupPath();
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
	_currentMovie->getScore()->_skipTransition = true;
	// XLibs are usually loaded in the initial movie.
	// These may not be present if a --start-movie is specified, so
	// we sometimes need to load them manually.
	if (!g_director->getStartMovie().startMovie.empty())
		loadStartMovieXLibs();

	return Common::kNoError;
}

void Window::probeResources(Archive *archive) {
	if (archive->hasResource(MKTAG('B', 'N', 'D', 'L'), "Projector")) {
		warning("Detected Projector file");

		if (archive->hasResource(MKTAG('v', 'e', 'r', 's'), -1)) {
			Common::Array<uint16> vers = archive->getResourceIDList(MKTAG('v', 'e', 'r', 's'));
			for (auto &iterator : vers) {
				Common::SeekableReadStreamEndian *vvers = archive->getResource(MKTAG('v', 'e', 'r', 's'), iterator);
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
			Common::Path moviePath = findMoviePath(sname);
			if (!moviePath.empty()) {
				_nextMovie.movie = moviePath.toString(g_director->_dirSeparator);
				warning("Replaced score name with: %s (from %s)", _nextMovie.movie.c_str(), sname.c_str());

				if (_currentMovie) {
					delete _currentMovie;
					_currentMovie = nullptr;
				}

				Archive *subMovie = g_director->openArchive(moviePath);
				if (subMovie) {
					probeResources(subMovie);
				}
			} else {
				warning("Couldn't find score with name: %s", sname.c_str());
			}
			delete name;
		}
	}

	if (g_director->getPlatform() == Common::kPlatformMacintosh) {
		// On Macintosh, you can add additional chunks to the resource
		// fork of the file to state which XObject or HyperCard XCMD/XFCNs
		// need to be loaded in.
		MacArchive *resFork = new MacArchive();
		Common::Path resForkPathName = findPath(archive->getPathName());
		if (resFork->openFile(resForkPathName)) {
			if (resFork->hasResource(MKTAG('X', 'C', 'O', 'D'), -1)) {
				Common::Array<uint16> xcod = resFork->getResourceIDList(MKTAG('X', 'C', 'O', 'D'));
				for (auto &iterator : xcod) {
					Resource res = resFork->getResourceDetail(MKTAG('X', 'C', 'O', 'D'), iterator);
					debug(0, "Detected XObject '%s'", res.name.c_str());
					g_lingo->openXLib(res.name, kXObj, resForkPathName);
				}
			}
			if (resFork->hasResource(MKTAG('X', 'C', 'M', 'D'), -1)) {
				Common::Array<uint16> xcmd = resFork->getResourceIDList(MKTAG('X', 'C', 'M', 'D'));
				for (auto &iterator : xcmd) {
					Resource res = resFork->getResourceDetail(MKTAG('X', 'C', 'M', 'D'), iterator);
					debug(0, "Detected XCMD '%s'", res.name.c_str());
					g_lingo->openXLib(res.name, kXObj, resForkPathName);
				}
			}
			if (resFork->hasResource(MKTAG('X', 'F', 'C', 'N'), -1)) {
				Common::Array<uint16> xfcn = resFork->getResourceIDList(MKTAG('X', 'F', 'C', 'N'));
				for (auto &iterator : xfcn) {
					Resource res = resFork->getResourceDetail(MKTAG('X', 'F', 'C', 'N'), iterator);
					debug(0, "Detected XFCN '%s'", res.name.c_str());
					g_lingo->openXLib(res.name, kXObj, resForkPathName);
				}
			}
		}
		delete resFork;
	}

	// Xtras
	if (g_director->getVersion() >= 500) {
		Common::Path basePath(g_director->getEXEName(), g_director->_dirSeparator);
		basePath = basePath.getParent().appendComponent("Xtras");
		basePath = findPath(basePath, false, false, true);
		if (!basePath.empty()) {
			Common::StringArray directory_list = basePath.splitComponents();
			Common::FSNode d = Common::FSNode(*g_director->getGameDataDir());
			bool escape = false;
			for (auto &it : directory_list) {
				d = d.getChild(it);
				if (!d.exists()) {
					escape = true;
					break;
				}
			}
			if (!escape) {
				debug(0, "Detected Xtras folder");
				Common::FSList xtras;
				d.getChildren(xtras, Common::FSNode::kListFilesOnly);
				for (auto &it : xtras) {
					debug(0, "Detected Xtra '%s'", it.getName().c_str());
					g_lingo->openXLib(it.getName(), kXtraObj, basePath.appendComponent(it.getName()));
				}
			}
		}
	}
}

void DirectorEngine::addArchiveToOpenList(const Common::Path &path) {
	// First, remove it if it is present
	_allOpenResFiles.remove(path);

	// And add it to the list front
	_allOpenResFiles.push_front(path);
}

Archive *DirectorEngine::openArchive(const Common::Path &path) {
	debug(1, "DirectorEngine::openArchive(\"%s\")", path.toString().c_str());

	// If the archive is already open, don't reopen it;
	// just init from the existing archive. This prevents errors that
	// can happen when trying to load the same archive more than once.
	if (_allSeenResFiles.contains(path)) {
		addArchiveToOpenList(path);
		return _allSeenResFiles.getVal(path);
	}

	Archive *result = nullptr;
	if (getPlatform() == Common::kPlatformWindows) {
		result = loadEXE(path);
	} else {
		result = loadMac(path);
	}
	if (!result) {
		result = createArchive();
		if (!result->openFile(path)) {
			delete result;
			return nullptr;
		}
	}
	result->setPathName(path);
	_allSeenResFiles.setVal(path, result);

	addArchiveToOpenList(path);

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

Archive *DirectorEngine::loadEXE(const Common::Path &movie) {
	Common::SeekableReadStream *exeStream = SearchMan.createReadStreamForMember(movie);
	if (!exeStream) {
		debugC(5, kDebugLoading, "DirectorEngine::loadEXE(): Failed to open file '%s'", movie.toString().c_str());
		return nullptr;
	}

	Archive *result = nullptr;

	uint32 initialTag = exeStream->readUint32LE();
	if (initialTag == MKTAG('R', 'I', 'F', 'X') || initialTag == MKTAG('X', 'F', 'I', 'R')) {
		// we've encountered a movie saved from Director, not a projector.
		result = loadEXERIFX(exeStream, 0);
		// ownership handed to loadEXERIFX
		exeStream = nullptr;
	} else if (initialTag == MKTAG('R', 'I', 'F', 'F') || initialTag == MKTAG('F', 'F', 'I', 'R')) { // This is just a normal movie
		result = new RIFFArchive();

		if (!result->openStream(exeStream, 0)) {
			debugC(5, kDebugLoading, "DirectorEngine::loadEXE(): Failed to load RIFF from '%s'", movie.toString().c_str());
			delete result;
			return nullptr;
		}
		// ownership handed to RIFFArchive
		exeStream = nullptr;
	} else {
		Common::WinResources *exe = Common::WinResources::createFromEXE(movie);
		if (!exe) {
			debugC(5, kDebugLoading, "DirectorEngine::loadEXE(): Failed to open EXE '%s'", movie.toString().c_str());
			delete exeStream;
			return nullptr;
		}

		const Common::Array<Common::WinResourceID> versions = exe->getIDList(Common::kWinVersion);
		for (uint i = 0; i < versions.size(); i++) {
			Common::WinResources::VersionInfo *info = exe->getVersionResource(versions[i]);

			for (Common::WinResources::VersionHash::const_iterator it = info->hash.begin(); it != info->hash.end(); ++it)
				debugC(5, kDebugLoading, "DirectorEngine::loadEXE(): info <%s>: <%s>", it->_key.c_str(), it->_value.encode().c_str());

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
			warning("DirectorEngine::loadEXE(): Unhandled Windows EXE version %d", g_director->getVersion());
			delete exeStream;
			return nullptr;
		}
		// ownership passed to an EXE loader
		exeStream = nullptr;
	}

	if (result)
		result->setPathName(movie);
	else if (exeStream)
		delete exeStream;

	return result;
}

Archive *DirectorEngine::loadEXEv3(Common::SeekableReadStream *stream) {
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
				warning("DirectorEngine::loadEXEv3(): Failed to find MMM '%s'", mmmFileName_.c_str());
			else {
				Common::SeekableReadStream *const mmmFile_ = SearchMan.createReadStreamForMember(Common::Path(mmmFileName_, g_director->_dirSeparator));
				uint32 mmmFileSize_ = mmmFile_->size();
				if (mmmSize_ != mmmFileSize_)
					warning("DirectorEngine::loadEXEv3(): File size for '%s' doesn't match. Got %d (0x%x), want %d (0x%x)", mmmFileName_.c_str(), mmmFileSize_, mmmFileSize_, mmmSize_, mmmSize_);
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
				warning("DirectorEngine::loadEXEv3(): Can not open dump file %s", fname.c_str());
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

		warning("DirectorEngine::loadEXEv3(): Failed to load RIFF from EXE");
		// ownership of stream is passed to result, which will clean it up
		delete result;
		result = nullptr;
	}

	Common::String fullPathStr = directoryName + mmmFileName;
	fullPathStr = convertPath(fullPathStr);
	Common::Path fullPath = findMoviePath(fullPathStr);
	if (fullPath.empty()) {
		warning("DirectorEngine::loadEXEv3(): Could not find '%s'", fullPathStr.c_str());
		return nullptr;
	}
	// The EXE is kicking us to a different movie on startup;
	// and we want to treat it as a proper movie change
	// (instead of pretending that the EXE is this movie) so that
	// elements like the search path are correct.
	getCurrentWindow()->setNextMovie(fullPathStr);
	// Return an empty archive to avoid "Game data not found".
	result = createArchive();
	return result;
}

Archive *DirectorEngine::loadEXEv4(Common::SeekableReadStream *stream) {
	uint32 ver = stream->readUint32BE();

	if (ver != MKTAG('P', 'J', '9', '3')) {
		warning("DirectorEngine::loadEXEv4(): Invalid projector tag found in v4 EXE [%s]", tag2str(ver));
		delete stream;
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

	warning("DirectorEngine::loadEXEv4(): PJ93 projector flags: %08x", flags);

	return loadEXERIFX(stream, rifxOffset);
}

Archive *DirectorEngine::loadEXEv5(Common::SeekableReadStream *stream) {
	uint32 ver = stream->readUint32LE();

	if (ver != MKTAG('P', 'J', '9', '5')) {
		warning("DirectorEngine::loadEXEv5(): Invalid projector tag found in v5 EXE [%s]", tag2str(ver));
		delete stream;
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

	warning("DirectorEngine::loadEXEv5(): PJ95 projector pflags: %08x  flags: %08x", pflags, flags);

	return loadEXERIFX(stream, rifxOffset);
}

Archive *DirectorEngine::loadEXEv7(Common::SeekableReadStream *stream) {
	uint32 ver = stream->readUint32LE();

	if (ver != MKTAG('P', 'J', '0', '0') && ver != MKTAG('P', 'J', '0', '1')) {
		warning("DirectorEngine::loadEXEv7(): Invalid projector tag found in v7 EXE [%s]", tag2str(ver));
		delete stream;
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

Archive *DirectorEngine::loadEXERIFX(Common::SeekableReadStream *stream, uint32 offset) {
	Archive *result = new RIFXArchive();

	if (!result->openStream(stream, offset)) {
		warning("DirectorEngine::loadEXERIFX(): Failed to load RIFX from EXE");
		delete result;
		result = nullptr;
	}
	return result;
}

Archive *DirectorEngine::loadMac(const Common::Path &movie) {
	Archive *result = nullptr;
	if (g_director->getVersion() < 400) {
		// The data is part of the resource fork of the executable
		result = createArchive();

		if (!result->openFile(movie)) {
			delete result;
			result = nullptr;
			debugC(5, kDebugLoading, "DirectorEngine::loadMac(): Could not open '%s'", movie.toString().c_str());
		}
	} else {
		// The RIFX is located in the data fork of the executable
		Common::SeekableReadStream *dataFork = Common::MacResManager::openFileOrDataFork(movie);
		if (!dataFork) {
			debugC(5, kDebugLoading, "DirectorEngine::loadMac(): Failed to open Mac binary '%s'", movie.toString().c_str());
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
			debugC(5, kDebugLoading, "DirectorEngine::loadMac(): Failed to load RIFX from Mac binary");
			delete result;
			result = nullptr;
		}
	}
	return result;
}

void Window::loadStartMovieXLibs() {
	if (strcmp(g_director->getGameId(), "warlock") == 0 && g_director->getPlatform() != Common::kPlatformWindows) {
		g_lingo->openXLib("FPlayXObj", kXObj, Common::Path());
	}
	g_lingo->openXLib("SerialPort", kXObj, Common::Path());
}

ProjectorArchive::ProjectorArchive(Common::Path path)
	: _path(path), _files() {

	// Buffer 100K into memory
	Common::SeekableReadStream *stream = createBufferedReadStream();
	if (!stream) {
		_isLoaded = false;
		return;
	}

	// Build our filemap using the buffered stream
	_isLoaded = loadArchive(stream);

	delete stream;
}

Common::SeekableReadStream *ProjectorArchive::createBufferedReadStream() {
	const uint32 READ_BUFFER_SIZE = 1024 * 100;

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(_path);
	if (!stream) {
		warning("ProjectorArchive::createBufferedReadStream(): Cannot open %s", _path.toString().c_str());
		return nullptr;
	}

	return Common::wrapBufferedSeekableReadStream(stream, READ_BUFFER_SIZE, DisposeAfterUse::YES);
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
		debugC(5, kDebugLoading, "ProjectorArchive::loadArchive(): Projector Tag not found");
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

	debugC(1, kDebugLoading, "File: %s off: 0x%x (%u), tag: %s rifx: 0x%x (%d)", _path.toString().c_str(), off, off, tag2str(tag), rifxOffset, rifxOffset);

	// Try to locate the very next Dict tag(byte-by-byte)
	tag = stream->readUint32BE();
	found = false;

	// This loop has negligible performance impact due to the stream being buffered.
	// Furthermore, comparing 4 bytes at a time should be pretty fast on modern systems.
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

		Common::Path path = toSafePath(arr[i]);

		debugC(1, kDebugLoading, "Entry: %s offset %lX (%ld) tag %s size %d", path.toString().c_str(), long(stream->pos() - 8), long(stream->pos() - 8), tag2str(tag), size);

		Entry entry;

		// subtract 8 since we want to include tag and size as well
		entry.offset = static_cast<uint32>(stream->pos() - 8);
		entry.size = size + 8;
		_files[path] = entry;

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
	return (_files.find(path) != _files.end());
}

int ProjectorArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (FileMap::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, *this)));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr ProjectorArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *ProjectorArchive::createReadStreamForMember(const Common::Path &path) const {
	FileMap::const_iterator fDesc = _files.find(path);

	if (fDesc == _files.end())
		return nullptr;

	debugC(3, kDebugLoading, "ProjectorArchive::createReadStreamForMember(): Opening built-in file '%s' @ 0x%08x, %d bytes",
		path.toString().c_str(), fDesc->_value.offset, fDesc->_value.size);

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(_path);

	stream->seek(fDesc->_value.offset, SEEK_SET);
	byte *data = (byte *)malloc(fDesc->_value.size);
	stream->read(data, fDesc->_value.size);
	delete stream;

	// Now we need to patch the mmap offset

	// First, do sanity checking for the file
	byte *ptr = data;
	uint32 headerTag = READ_UINT32(ptr);
	ptr += 4;
	bool formatOK = true;
	bool isBigEndian = false; // LE

	if (headerTag == MKTAG('X', 'F', 'I', 'R')) {
		isBigEndian = false;
	} else if (headerTag == MKTAG('R', 'I', 'F', 'X')) {
		isBigEndian = true;
	} else {
		formatOK = false;
	}

	// 4 bytes size
	ptr += 4;

	uint32 rifxType = isBigEndian ? READ_LE_UINT32(ptr) : READ_BE_UINT32(ptr);
	ptr += 4;

	uint32 mmapOffset;
	uint32 resCount;

	int patchedOffests = 0;

	switch (rifxType) {
	case MKTAG('M', 'V', '9', '3'):
	case MKTAG('M', 'C', '9', '5'):
	case MKTAG('A', 'P', 'P', 'L'):
		if ((isBigEndian ? READ_LE_UINT32(ptr) : READ_BE_UINT32(ptr)) != MKTAG('i', 'm', 'a', 'p')) {
			formatOK = false;
			break;
		}
		ptr += 4;

		// patch it
		ptr += 4; // imap length
		ptr += 4; // unknown
		mmapOffset = isBigEndian ? READ_LE_UINT32(ptr) : READ_BE_UINT32(ptr);

		debugC(8, kDebugLoading, "ProjectorArchive::createReadStreamForMember(): Patched offset at 0x%08x from %u to %u",
			(uint32)(ptr - data), mmapOffset, mmapOffset - fDesc->_value.offset);

		mmapOffset -= fDesc->_value.offset;

		isBigEndian ? WRITE_LE_UINT32(ptr, mmapOffset) : WRITE_BE_UINT32(ptr, mmapOffset);
		patchedOffests++;

		ptr = data + mmapOffset;

		if ((isBigEndian ? READ_LE_UINT32(ptr) : READ_BE_UINT32(ptr)) != MKTAG('m', 'm', 'a', 'p')) {
			formatOK = false;
			break;
		}

		ptr += 4; // mmap length
		ptr += 4; // unknown
		ptr += 4; // unknown
		ptr += 4; // resCount + empty entries
		resCount = isBigEndian ? READ_LE_UINT32(ptr) : READ_BE_UINT32(ptr);
		ptr += 4;
		ptr += 8; // all 0xFF
		ptr += 4; // id of the first free resource, -1 if none.

		for (uint32 i = 0; i < resCount; i++) {
			ptr += 4; // tag
			uint32 size = isBigEndian ? READ_LE_UINT32(ptr) : READ_BE_UINT32(ptr);
			ptr += 4;
			uint32 offset = isBigEndian ? READ_LE_UINT32(ptr) : READ_BE_UINT32(ptr);

			if (size > 0 || offset >= fDesc->_value.offset)  {
				debugC(8, kDebugLoading, "ProjectorArchive::createReadStreamForMember(): Patched offset at 0x%08x from %u to %u",
					(uint32)(ptr - data), offset, offset - fDesc->_value.offset);

				offset -= fDesc->_value.offset;
				patchedOffests++;

				isBigEndian ? WRITE_LE_UINT32(ptr, offset) : WRITE_BE_UINT32(ptr, offset);
			}
			ptr += 4;

			ptr += 2; // flags
			ptr += 2; // unk1
			ptr += 4; // nextFreeResourceId
		}

		break;

	default:
		formatOK = false;
	}

	if (!formatOK)
		warning("ProjectorArchive::createReadStreamForMember(): File %s has unsupported format", path.toString().c_str());

	debugC(3, kDebugLoading, "ProjectorArchive::createReadStreamForMember(): Patched %d offsets", patchedOffests);

	return new Common::MemoryReadStream(data, fDesc->_value.size, DisposeAfterUse::YES);
}
} // End of namespace Director
