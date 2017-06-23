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

#include "common/savefile.h"
#include "common/stream.h"
#include "common/memstream.h"

#include "sci/sci.h"
#include "sci/engine/file.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"

namespace Sci {

#ifdef ENABLE_SCI32
/**
 * A MemoryWriteStreamDynamic with additional read functionality.
 * The read and write functions share a single stream position.
 */
class MemoryDynamicRWStream : public Common::MemoryWriteStreamDynamic, public Common::SeekableReadStream {
protected:
	bool _eos;
public:
	MemoryDynamicRWStream(DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO) : MemoryWriteStreamDynamic(disposeMemory), _eos(false) { }

	uint32 read(void *dataPtr, uint32 dataSize);

	bool eos() const { return _eos; }
	int32 pos() const { return _pos; }
	int32 size() const { return _size; }
	void clearErr() { _eos = false; Common::MemoryWriteStreamDynamic::clearErr(); }
	bool seek(int32 offs, int whence = SEEK_SET) { return Common::MemoryWriteStreamDynamic::seek(offs, whence); }

};

uint32 MemoryDynamicRWStream::read(void *dataPtr, uint32 dataSize)
{
	// Read at most as many bytes as are still available...
	if (dataSize > _size - _pos) {
		dataSize = _size - _pos;
		_eos = true;
	}
	memcpy(dataPtr, _ptr, dataSize);

	_ptr += dataSize;
	_pos += dataSize;

	return dataSize;
}

/**
 * A MemoryDynamicRWStream intended to re-write a file.
 * It reads the contents of `inFile` in the constructor, and writes back
 * the changes to `fileName` in the destructor (and when calling commit() ).
 */
class SaveFileRewriteStream : public MemoryDynamicRWStream {
public:
	SaveFileRewriteStream(Common::String fileName,
	                      Common::SeekableReadStream *inFile,
	                      kFileOpenMode mode, bool compress);
	virtual ~SaveFileRewriteStream();

	virtual uint32 write(const void *dataPtr, uint32 dataSize) { _changed = true; return MemoryDynamicRWStream::write(dataPtr, dataSize); }

	void commit(); //< Save back to disk

protected:
	Common::String _fileName;
	bool _compress;
	bool _changed;
};

SaveFileRewriteStream::SaveFileRewriteStream(Common::String fileName,
                                             Common::SeekableReadStream *inFile,
                                             kFileOpenMode mode,
                                             bool compress)
: MemoryDynamicRWStream(DisposeAfterUse::YES),
  _fileName(fileName), _compress(compress)
{
	const bool truncate = mode == _K_FILE_MODE_CREATE;
	const bool seekToEnd = mode == _K_FILE_MODE_OPEN_OR_CREATE;

	if (!truncate && inFile) {
		unsigned int s = inFile->size();
		ensureCapacity(s);
		inFile->read(_data, s);
		if (seekToEnd) {
			seek(0, SEEK_END);
		}
		_changed = false;
	} else {
		_changed = true;
	}
}

SaveFileRewriteStream::~SaveFileRewriteStream() {
	commit();
}

void SaveFileRewriteStream::commit() {
	// Write contents of buffer back to file

	if (_changed) {
		Common::WriteStream *outFile = g_sci->getSaveFileManager()->openForSaving(_fileName, _compress);
		outFile->write(_data, _size);
		delete outFile;
		_changed = false;
	}
}

#endif

uint findFreeFileHandle(EngineState *s) {
	// Find a free file handle
	uint handle = 1; // Ignore _fileHandles[0]
	while ((handle < s->_fileHandles.size()) && s->_fileHandles[handle].isOpen())
		handle++;

	if (handle == s->_fileHandles.size()) {
		// Hit size limit => Allocate more space
		s->_fileHandles.resize(s->_fileHandles.size() + 1);
	}

	return handle;
}

/*
 * Note on how file I/O is implemented: In ScummVM, one can not create/write
 * arbitrary data files, simply because many of our target platforms do not
 * support this. The only files one can create are savestates. But SCI has an
 * opcode to create and write to seemingly 'arbitrary' files. This is mainly
 * used in LSL3 for LARRY3.DRV (which is a game data file, not a driver, used
 * for persisting the results of the "age quiz" across restarts) and in LSL5
 * for MEMORY.DRV (which is again a game data file and contains the game's
 * password, XOR encrypted).
 * To implement that opcode, we combine the SaveFileManager with regular file
 * code, similarly to how the SCUMM HE engine does it.
 *
 * To handle opening a file called "foobar", what we do is this: First, we
 * create an 'augmented file name', by prepending the game target and a dash,
 * so if we running game target sq1sci, the name becomes "sq1sci-foobar".
 * Next, we check if such a file is known to the SaveFileManager. If so, we
 * we use that for reading/writing, delete it, whatever.
 *
 * If no such file is present but we were only asked to *read* the file,
 * we fallback to looking for a regular file called "foobar", and open that
 * for reading only.
 */

reg_t file_open(EngineState *s, const Common::String &filename, kFileOpenMode mode, bool unwrapFilename) {
	Common::String englishName = g_sci->getSciLanguageString(filename, K_LANG_ENGLISH);
	englishName.toLowercase();

	Common::String wrappedName = unwrapFilename ? g_sci->wrapFilename(englishName) : englishName;
	Common::SeekableReadStream *inFile = 0;
	Common::WriteStream *outFile = 0;
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();

	bool isCompressed = true;
	const SciGameId gameId = g_sci->getGameId();

	// QFG Characters are saved via the CharSave object.
	// We leave them uncompressed so that they can be imported in later QFG
	// games, even when using the original interpreter.
	// We check for room numbers in here, because the file suffix can be changed by the user.
	// Rooms/Scripts: QFG1(EGA/VGA): 601, QFG2: 840, QFG3/4: 52
	switch (gameId) {
	case GID_QFG1:
	case GID_QFG1VGA:
		if (s->currentRoomNumber() == 601)
			isCompressed = false;
		break;
	case GID_QFG2:
		if (s->currentRoomNumber() == 840)
			isCompressed = false;
		break;
	case GID_QFG3:
	case GID_QFG4:
		if (s->currentRoomNumber() == 52)
			isCompressed = false;
		break;
#ifdef ENABLE_SCI32
	// Hoyle5 has no save games, but creates very simple text-based game options
	// files that do not need to be compressed
	case GID_HOYLE5:
	// Phantasmagoria game scripts create their own save files, so they are
	// interoperable with the original interpreter just by renaming them as long
	// as they are not compressed. They are also never larger than a couple
	// hundred bytes, so compression does not do much here anyway
	case GID_PHANTASMAGORIA:
		isCompressed = false;
		break;
#endif
	default:
		break;
	}

#ifdef ENABLE_SCI32
	if ((g_sci->getGameId() == GID_PHANTASMAGORIA && (filename == "phantsg.dir" || filename == "chase.dat" || filename == "tmp.dat")) ||
	    (g_sci->getGameId() == GID_PQSWAT && filename == "swat.dat")) {
		debugC(kDebugLevelFile, "  -> file_open opening %s for rewriting", wrappedName.c_str());

		inFile = saveFileMan->openForLoading(wrappedName);
		// If no matching savestate exists: fall back to reading from a regular
		// file
		if (!inFile)
			inFile = SearchMan.createReadStreamForMember(englishName);

		if (mode == _K_FILE_MODE_OPEN_OR_FAIL && !inFile) {
			debugC(kDebugLevelFile, "  -> file_open(_K_FILE_MODE_OPEN_OR_FAIL): failed to open file '%s'", englishName.c_str());
			return SIGNAL_REG;
		}

		SaveFileRewriteStream *stream;
		stream = new SaveFileRewriteStream(wrappedName, inFile, mode, isCompressed);

		delete inFile;

		inFile = stream;
		outFile = stream;
	} else
#endif
	if (mode == _K_FILE_MODE_OPEN_OR_FAIL) {
		// Try to open file, abort if not possible
		inFile = saveFileMan->openForLoading(wrappedName);
		// If no matching savestate exists: fall back to reading from a regular
		// file
		if (!inFile)
			inFile = SearchMan.createReadStreamForMember(englishName);

		if (!inFile)
			debugC(kDebugLevelFile, "  -> file_open(_K_FILE_MODE_OPEN_OR_FAIL): failed to open file '%s'", englishName.c_str());
	} else if (mode == _K_FILE_MODE_CREATE) {
		// Create the file, destroying any content it might have had
		outFile = saveFileMan->openForSaving(wrappedName, isCompressed);
		if (!outFile)
			debugC(kDebugLevelFile, "  -> file_open(_K_FILE_MODE_CREATE): failed to create file '%s'", englishName.c_str());
	} else if (mode == _K_FILE_MODE_OPEN_OR_CREATE) {
		// Try to open file, create it if it doesn't exist
		outFile = saveFileMan->openForSaving(wrappedName, isCompressed);
		if (!outFile)
			debugC(kDebugLevelFile, "  -> file_open(_K_FILE_MODE_CREATE): failed to create file '%s'", englishName.c_str());

		// QfG1 opens the character export file with _K_FILE_MODE_CREATE first,
		// closes it immediately and opens it again with this here. Perhaps
		// other games use this for read access as well. I guess changing this
		// whole code into using virtual files and writing them after close
		// would be more appropriate.
	} else {
		error("file_open: unsupported mode %d (filename '%s')", mode, englishName.c_str());
	}

	if (!inFile && !outFile) { // Failed
		debugC(kDebugLevelFile, "  -> file_open() failed");
		return SIGNAL_REG;
	}

	uint handle = findFreeFileHandle(s);

	s->_fileHandles[handle]._in = inFile;
	s->_fileHandles[handle]._out = outFile;
	s->_fileHandles[handle]._name = englishName;

	debugC(kDebugLevelFile, "  -> opened file '%s' with handle %d", englishName.c_str(), handle);
	return make_reg(0, handle);
}

FileHandle *getFileFromHandle(EngineState *s, uint handle) {
	if ((handle == 0) || ((handle >= VIRTUALFILE_HANDLE_START) && (handle <= VIRTUALFILE_HANDLE_END))) {
		error("Attempt to use invalid file handle (%d)", handle);
		return 0;
	}

	if ((handle >= s->_fileHandles.size()) || !s->_fileHandles[handle].isOpen()) {
		warning("Attempt to use invalid/unused file handle %d", handle);
		return 0;
	}

	return &s->_fileHandles[handle];
}

int fgets_wrapper(EngineState *s, char *dest, int maxsize, int handle) {
	FileHandle *f = getFileFromHandle(s, handle);
	if (!f)
		return 0;

	if (!f->_in) {
		error("fgets_wrapper: Trying to read from file '%s' opened for writing", f->_name.c_str());
		return 0;
	}
	int readBytes = 0;
	if (maxsize > 1) {
		memset(dest, 0, maxsize);
		f->_in->readLine(dest, maxsize);
		readBytes = Common::strnlen(dest, maxsize); // FIXME: sierra sci returned byte count and didn't react on NUL characters
		// The returned string must not have an ending LF
		if (readBytes > 0) {
			if (dest[readBytes - 1] == 0x0A)
				dest[readBytes - 1] = 0;
		}
	} else {
		*dest = 0;
	}

	debugC(kDebugLevelFile, "  -> FGets'ed \"%s\"", dest);
	return readBytes;
}

static bool _savegame_sort_byDate(const SavegameDesc &l, const SavegameDesc &r) {
	if (l.date != r.date)
		return (l.date > r.date);
	return (l.time > r.time);
}

bool fillSavegameDesc(const Common::String &filename, SavegameDesc *desc) {
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	Common::SeekableReadStream *in;
	if ((in = saveFileMan->openForLoading(filename)) == nullptr) {
		return false;
	}

	SavegameMetadata meta;
	if (!get_savegame_metadata(in, &meta) || meta.name.empty()) {
		// invalid
		delete in;
		return false;
	}
	delete in;

	const int id = strtol(filename.end() - 3, NULL, 10);
	desc->id = id;
	desc->date = meta.saveDate;
	// We need to fix date in here, because we save DDMMYYYY instead of
	// YYYYMMDD, so sorting wouldn't work
	desc->date = ((desc->date & 0xFFFF) << 16) | ((desc->date & 0xFF0000) >> 8) | ((desc->date & 0xFF000000) >> 24);
	desc->time = meta.saveTime;
	desc->version = meta.version;
	desc->gameVersion = meta.gameVersion;
	desc->script0Size = meta.script0Size;
	desc->gameObjectOffset = meta.gameObjectOffset;
#ifdef ENABLE_SCI32
	if (g_sci->getGameId() == GID_SHIVERS) {
		desc->lowScore = meta.lowScore;
		desc->highScore = meta.highScore;
	} else if (g_sci->getGameId() == GID_MOTHERGOOSEHIRES) {
		desc->avatarId = meta.avatarId;
	}
#endif

	if (meta.name.lastChar() == '\n')
		meta.name.deleteLastChar();

	Common::strlcpy(desc->name, meta.name.c_str(), SCI_MAX_SAVENAME_LENGTH);

	return desc;
}

// Create an array containing all found savedgames, sorted by creation date
void listSavegames(Common::Array<SavegameDesc> &saves) {
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	Common::StringArray saveNames = saveFileMan->listSavefiles(g_sci->getSavegamePattern());

	for (Common::StringArray::const_iterator iter = saveNames.begin(); iter != saveNames.end(); ++iter) {
		const Common::String &filename = *iter;

#ifdef ENABLE_SCI32
		const int id = strtol(filename.end() - 3, NULL, 10);
		if (id == kNewGameId || id == kAutoSaveId) {
			continue;
		}
#endif

		SavegameDesc desc;
		fillSavegameDesc(filename, &desc);
		saves.push_back(desc);
	}

	// Sort the list by creation date of the saves
	Common::sort(saves.begin(), saves.end(), _savegame_sort_byDate);
}

// Find a savedgame according to virtualId and return the position within our array
int findSavegame(Common::Array<SavegameDesc> &saves, int16 savegameId) {
	for (uint saveNr = 0; saveNr < saves.size(); saveNr++) {
		if (saves[saveNr].id == savegameId)
			return saveNr;
	}
	return -1;
}


FileHandle::FileHandle() : _in(0), _out(0) {
}

FileHandle::~FileHandle() {
	close();
}

void FileHandle::close() {
	// NB: It is possible _in and _out are both non-null, but
	// then they point to the same object.
	if (_in)
		delete _in;
	else
		delete _out;
	_in = 0;
	_out = 0;
	_name.clear();
}

bool FileHandle::isOpen() const {
	return _in || _out;
}


void DirSeeker::addAsVirtualFiles(Common::String title, Common::String fileMask) {
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	Common::StringArray foundFiles = saveFileMan->listSavefiles(fileMask);
	if (!foundFiles.empty()) {
		// Sort all filenames alphabetically
		Common::sort(foundFiles.begin(), foundFiles.end());

		_files.push_back(title);
		_virtualFiles.push_back("");
		Common::StringArray::iterator it;
		Common::StringArray::iterator it_end = foundFiles.end();

		for (it = foundFiles.begin(); it != it_end; it++) {
			Common::String regularFilename = *it;
			Common::String wrappedFilename = Common::String(regularFilename.c_str() + fileMask.size() - 1);

			Common::SeekableReadStream *testfile = saveFileMan->openForLoading(regularFilename);
			int32 testfileSize = testfile->size();
			delete testfile;
			if (testfileSize > 1024) // check, if larger than 1k. in that case its a saved game.
				continue; // and we dont want to have those in the list
			// We need to remove the prefix for display purposes
			_files.push_back(wrappedFilename);
			// but remember the actual name as well
			_virtualFiles.push_back(regularFilename);
		}
	}
}

Common::String DirSeeker::getVirtualFilename(uint fileNumber) {
	if (fileNumber >= _virtualFiles.size())
		error("invalid virtual filename access");
	return _virtualFiles[fileNumber];
}

reg_t DirSeeker::firstFile(const Common::String &mask, reg_t buffer, SegManager *segMan) {
	// Verify that we are given a valid buffer
	if (!buffer.getSegment()) {
		error("DirSeeker::firstFile('%s') invoked with invalid buffer", mask.c_str());
		return NULL_REG;
	}
	_outbuffer = buffer;
	_files.clear();
	_virtualFiles.clear();

	int QfGImport = g_sci->inQfGImportRoom();
	if (QfGImport) {
		_files.clear();
		addAsVirtualFiles("-QfG1-", "qfg1-*");
		addAsVirtualFiles("-QfG1VGA-", "qfg1vga-*");
		if (QfGImport > 2)
			addAsVirtualFiles("-QfG2-", "qfg2-*");
		if (QfGImport > 3)
			addAsVirtualFiles("-QfG3-", "qfg3-*");

		if (QfGImport == 3) {
			// QfG3 sorts the filelisting itself, we can't let that happen otherwise our
			//  virtual list would go out-of-sync
			reg_t savedHeros = segMan->findObjectByName("savedHeros");
			if (!savedHeros.isNull())
				writeSelectorValue(segMan, savedHeros, SELECTOR(sort), 0);
		}

	} else {
		// Prefix the mask
		const Common::String wrappedMask = g_sci->wrapFilename(mask);

		// Obtain a list of all files matching the given mask
		Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
		_files = saveFileMan->listSavefiles(wrappedMask);
	}

	// Reset the list iterator and write the first match to the output buffer,
	// if any.
	_iter = _files.begin();
	return nextFile(segMan);
}

reg_t DirSeeker::nextFile(SegManager *segMan) {
	if (_iter == _files.end()) {
		return NULL_REG;
	}

	Common::String string;

	if (_virtualFiles.empty()) {
		// Strip the prefix, if we don't got a virtual filelisting
		const Common::String wrappedString = *_iter;
		string = g_sci->unwrapFilename(wrappedString);
	} else {
		string = *_iter;
	}
	if (string.size() > 12)
		string = Common::String(string.c_str(), 12);
	segMan->strcpy(_outbuffer, string.c_str());

	// Return the result and advance the list iterator :)
	++_iter;
	return _outbuffer;
}

} // End of namespace Sci
