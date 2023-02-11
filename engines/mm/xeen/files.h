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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef XEEN_FILES_H
#define XEEN_FILES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str-array.h"
#include "graphics/surface.h"
#include "mm/shared/xeen/cc_archive.h"

namespace MM {
namespace Xeen {

using Shared::Xeen::BaseCCArchive;
using Shared::Xeen::CCArchive;
using Shared::Xeen::CCEntry;

class XeenEngine;
class File;
class SaveArchive;
class Party;
class OutFile;
class SavesManager;

#define SYNC_AS(SUFFIX,STREAM,TYPE,SIZE) \
	template<typename T> \
	void syncAs ## SUFFIX(T &val, Version minVersion = 0, Version maxVersion = kLastVersion) { \
		if (_version < minVersion || _version > maxVersion) \
			return;	\
		if (_loadStream) \
			val = static_cast<TYPE>(_loadStream->read ## STREAM()); \
		else { \
			TYPE tmp = (TYPE)val; \
			_saveStream->write ## STREAM(tmp); \
		} \
		_bytesSynced += SIZE; \
	}

/*
 * Main resource manager
 */
class FileManager {
public:
	int _ccNum;
public:
	/**
	 * Constructor
	 */
	FileManager(XeenEngine *vm);

	/**
	 * Destructor
	 */
	~FileManager();

	/**
	 * Sets up the CC files
	 * @returns		Returns true if the setup was successful
	 */
	bool setup();

	/**
	 * Set which game side files to use
	 * @param ccMode	0=Clouds, 1=Dark Side
	 */
	void setGameCc(int ccMode);

	/**
	 * Loads a save archive from a stream
	 */
	void load(Common::SeekableReadStream &stream);

	/**
	 * Saves a save archive to a savegame
	 */
	void save(Common::WriteStream &s);
};

/**
 * Derived file class
 */
class File : public Common::File {
	friend class FileManager;
	friend class OutFile;
	friend class SavesManager;
	friend class Debugger;
private:
	static CCArchive *_xeenCc, *_darkCc, *_introCc;
	static SaveArchive *_xeenSave, *_darkSave;
	static BaseCCArchive *_currentArchive;
	static SaveArchive *_currentSave;
public:
	/**
	 * Sets which archive is used by default
	 */
	static void setCurrentArchive(int ccMode);

	/**
	 * Synchronizes a boolean array as a bitfield set
	 */
	static void syncBitFlags(Common::Serializer &s, bool *startP, bool *endP);
public:
	File() : Common::File() {}
	File(const Common::String &filename);
	File(const Common::String &filename, int ccMode);
	File(const Common::String &filename, Common::Archive &archive);
	~File() override {}

	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	bool open(const Common::Path &filename) override;

	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	bool open(const Common::Path &filename, Common::Archive &archive) override;

	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	virtual bool open(const Common::String &filename, int ccMode);

	/**
	 * Opens the given file
	 */
	bool open(const Common::FSNode &node) override {
		return Common::File::open(node);
	}

	/**
	 * Opens the given file
	 */
	bool open(SeekableReadStream *stream, const Common::String &name) override {
		return Common::File::open(stream, name);
	}

	/**
	 * Reads in a null terminated string
	 */
	Common::String readString();

	/**
	 * Checks if a given file exists
	 *
	 * @param	filename	the file to check for
	 * @return	true if the file exists, false otherwise
	 */
	static bool exists(const Common::String &filename);

	/**
	 * Checks if a given file exists
	 *
	 * @param	filename	the file to check for
	 * @param	ccMode		Archive to use
	 * @return	true if the file exists, false otherwise
	 */
	static bool exists(const Common::String &filename, int ccMode);

	/**
	 * Checks if a given file exists
	 *
	 * @param	filename	the file to check for
	 * @param	archive		Archive to use
	 * @return	true if the file exists, false otherwise
	 */
	static bool exists(const Common::String &filename, Common::Archive &archive);
};

/**
 * SubWriteStream provides a way of compartmentalizing writing to a subsection of
 * a file. This is primarily useful for the pos() function which can, for example,
 * be used in asserts to ensure writing is being done at the correct offset within
 * the bounds of the structure being written.
*/
class SubWriteStream : virtual public Common::WriteStream {
protected:
	Common::WriteStream *_parentStream;
	uint32 _begin;
public:
	SubWriteStream(Common::WriteStream *parentStream) :
		_parentStream(parentStream),  _begin(parentStream->pos()) {
	}

	uint32 write(const void *dataPtr, uint32 dataSize) override {
		return _parentStream->write(dataPtr, dataSize);
	}
	bool flush() override { return _parentStream->flush(); }
	void finalize() override {}
	int64 pos() const override { return _parentStream->pos() - _begin; }
};

class XeenSerializer : public Common::Serializer {
private:
	Common::SeekableReadStream *_in;
	int _filesize;
public:
	XeenSerializer(Common::SeekableReadStream *in, Common::WriteStream *out) :
		Common::Serializer(in, out), _in(in), _filesize(-1) {}

	SYNC_AS(Sint8, Byte, int8, 1)

	bool finished() {
		if (_in && _filesize == -1)
			_filesize = _in->size();
		return _in != nullptr && _in->pos() >= _filesize;
	}
};

class SaveArchive : public BaseCCArchive {
	friend class OutFile;
private:
	Party *_party;
	byte *_data;
	uint32 _dataSize;
	Common::HashMap<uint16, Common::MemoryWriteStreamDynamic *> _newData;
public:
	SaveArchive(Party *party);
	~SaveArchive() override;

	/**
	* Sets up the dynamic data for the game for a new game
	*/
	void reset(CCArchive *src);

	/**
	 * Archive implementation
	 */
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	/**
	 * Archive implementation
	 */
	virtual Common::SeekableReadStream *createReadStreamForMember(uint16 id) const;

	/**
	 * Loads a save archive from a stream
	 */
	void load(Common::SeekableReadStream &stream);

	/**
	 * Saves a save archive to a savegame
	 */
	void save(Common::WriteStream &s);

	/**
	 * Load the character roster and party
	 */
	void loadParty();

	/**
	 * Sets a new resource entry
	 */
	void replaceEntry(uint16 id, const byte *data, size_t size);
};

/**
 * Provides an interface to updating files within the in-memory save state
 */
class OutFile : public Common::WriteStream {
private:
	SaveArchive *_archive;
	Common::String _filename;
	Common::MemoryWriteStreamDynamic _backingStream;
public:
	OutFile(const Common::String &filename);
	OutFile(const Common::String &filename, SaveArchive *archive);
	OutFile(const Common::String &filename, int ccMode);

	/**
	 * Finishes any pending writes, pushing out the written data
	 */
	void finalize() override;

	/**
	 * Writes data
	 */
	uint32 write(const void *dataPtr, uint32 dataSize) override;

	/**
	 * Returns the current position
	 */
	int64 pos() const override;
};

} // End of namespace Xeen
} // End of namespace MM

#endif
