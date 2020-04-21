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

#ifndef SCI_ENGINE_FILE_H
#define SCI_ENGINE_FILE_H

#include "common/memstream.h"
#include "common/str-array.h"
#include "common/stream.h"

namespace Sci {

enum kFileOpenMode {
	kFileOpenModeOpenOrCreate = 0,
	kFileOpenModeOpenOrFail = 1,
	kFileOpenModeCreate = 2
};

enum {
	kMaxSaveNameLength = 36, ///< Maximum length of a savegame name (including optional terminator character).
	kMaxNumSaveGames = 20 ///< Maximum number of savegames
};

enum {
	kAutoSaveId = 0,  ///< The save game slot number for autosaves
	kNewGameId = 999, ///< The save game slot number for a "new game" save

	// SCI engine expects game IDs to start at 0, but slot 0 in ScummVM is
	// reserved for autosave, so non-autosave games get their IDs shifted up
	// when saving or restoring, and shifted down when enumerating save games.
	// ScummVM slot 0 can't be shifted down as -1 is an illegal SCI save ID
	// so it is instead wrapped around to 99 and then back to 0 when shifting up.
	kSaveIdShift = 1,
	kMaxShiftedSaveId = 99
};

enum {
	kVirtualFileHandleStart = 32000,
	kVirtualFileHandleSci32Save = 32100,
	kVirtualFileHandleSciAudio = 32300,
	kVirtualFileHandleEnd = 32300
};

struct SavegameDesc {
	int16 id;
	int virtualId; // straight numbered, according to id but w/o gaps
	int date;
	int time;
	int version;
	char name[kMaxSaveNameLength];
	Common::String gameVersion;
	uint32 script0Size;
	uint32 gameObjectOffset;
#ifdef ENABLE_SCI32
	// Used by Shivers 1
	uint16 lowScore;
	uint16 highScore;
	// Used by MGDX
	uint8 avatarId;
#endif
};

class FileHandle {
public:
	Common::String _name;
	Common::SeekableReadStream *_in;
	Common::WriteStream *_out;

public:
	FileHandle();
	~FileHandle();

	void close();
	bool isOpen() const;
};


class DirSeeker {
protected:
	reg_t _outbuffer;
	Common::StringArray _files;
	Common::StringArray _virtualFiles;
	Common::StringArray::const_iterator _iter;

public:
	DirSeeker() {
		_outbuffer = NULL_REG;
		_iter = _files.begin();
	}

	reg_t firstFile(const Common::String &mask, reg_t buffer, SegManager *segMan);
	reg_t nextFile(SegManager *segMan);

	Common::String getVirtualFilename(uint fileNumber);

private:
	void addAsVirtualFiles(Common::String title, Common::String fileMask);
};

#ifdef ENABLE_SCI32
/**
 * A MemoryWriteStreamDynamic with additional read functionality.
 * The read and write functions share a single stream position.
 */
class MemoryDynamicRWStream : public Common::MemoryWriteStreamDynamic, public Common::SeekableReadStream {
public:
	MemoryDynamicRWStream(DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO) : MemoryWriteStreamDynamic(disposeMemory), _eos(false) { }

	uint32 read(void *dataPtr, uint32 dataSize) override;

	bool eos() const override { return _eos; }
	int32 pos() const override { return _pos; }
	int32 size() const override { return _size; }
	void clearErr() override { _eos = false; Common::MemoryWriteStreamDynamic::clearErr(); }
	bool seek(int32 offs, int whence = SEEK_SET) override { return Common::MemoryWriteStreamDynamic::seek(offs, whence); }

protected:
	bool _eos;
};

/**
 * A MemoryDynamicRWStream intended to re-write a file.
 * It reads the contents of `inFile` in the constructor, and writes back
 * the changes to `fileName` in the destructor (and when calling commit() ).
 */
class SaveFileRewriteStream : public MemoryDynamicRWStream {
public:
	SaveFileRewriteStream(const Common::String &fileName,
	                      Common::SeekableReadStream *inFile,
	                      kFileOpenMode mode, bool compress);
	~SaveFileRewriteStream() override;

	uint32 write(const void *dataPtr, uint32 dataSize) override { _changed = true; return MemoryDynamicRWStream::write(dataPtr, dataSize); }

	void commit(); //< Save back to disk

protected:
	Common::String _fileName;
	bool _compress;
	bool _changed;
};

#endif

uint findFreeFileHandle(EngineState *s);
reg_t file_open(EngineState *s, const Common::String &filename, kFileOpenMode mode, bool unwrapFilename);
FileHandle *getFileFromHandle(EngineState *s, uint handle);
int fgets_wrapper(EngineState *s, char *dest, int maxsize, int handle);
void listSavegames(Common::Array<SavegameDesc> &saves);
int findSavegame(Common::Array<SavegameDesc> &saves, int16 savegameId);
bool fillSavegameDesc(const Common::String &filename, SavegameDesc &desc);

#ifdef ENABLE_SCI32
/**
 * Constructs an in-memory stream from the ScummVM save game list that is
 * compatible with game scripts' game catalogue readers.
 */
Common::MemoryReadStream *makeCatalogue(const uint maxNumSaves, const uint gameNameSize, const Common::String &fileNamePattern, const bool ramaFormat);

int shiftSciToScummVMSaveId(int saveId);
int shiftScummVMToSciSaveId(int saveId);
#endif

} // End of namespace Sci

#endif // SCI_ENGINE_FILE_H
