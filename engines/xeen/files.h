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

namespace Xeen {

class XeenEngine;
class CCArchive;
class BaseCCArchive;
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

/**
 * Details of a single entry in a CC file index
 */
struct CCEntry {
	uint16 _id;
	int _offset;
	uint16 _size;
	int _writeOffset;

	CCEntry() : _id(0), _offset(0), _size(0), _writeOffset(0) {}
	CCEntry(uint16 id, uint32 offset, uint32 size)
		: _id(id), _offset(offset), _size(size) {
	}
};

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
	bool open(const Common::String &filename) override;

	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	bool open(const Common::String &filename, Common::Archive &archive) override;

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
	int32 pos() const override { return _parentStream->pos() - _begin; }
};

class StringArray : public Common::StringArray {
public:
	StringArray() {}
	StringArray(const Common::String &name) { load(name); }

	/**
	 * Loads a string array from the specified file
	 */
	void load(const Common::String &name);

	/**
	 * Loads a string array from the specified file
	 */
	void load(const Common::String &name, int ccMode);
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

/**
 * Base Xeen CC file implementation
 */
class BaseCCArchive : public Common::Archive {
protected:
	Common::Array<CCEntry> _index;

	/**
	 * Load the index of a given CC file
	 */
	void loadIndex(Common::SeekableReadStream &stream);

	/**
	 * Saves out the contents of the index. Used when creating savegames
	 */
	void saveIndex(Common::WriteStream &stream);

	/**
	 * Given a resource name, returns whether an entry exists, and returns
	 * the header index data for that entry
	 */
	virtual bool getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const;

	/**
	 * Given a resource Id, returns whether an entry exists, and returns
	 * the header index data for that entry
	 */
	virtual bool getHeaderEntry(uint16 id, CCEntry &ccEntry) const;
public:
	/**
	 * Hash a given filename to produce the Id that represents it
	 */
	static uint16 convertNameToId(const Common::String &resourceName);
public:
	BaseCCArchive() {}

	// Archive implementation
	bool hasFile(const Common::String &name) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;
};

/**
 * Xeen CC file implementation
 */
class CCArchive : public BaseCCArchive {
private:
	Common::String _filename;
	Common::String _prefix;
	bool _encoded;
protected:
	bool getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const override;
public:
	CCArchive(const Common::String &filename, bool encoded);
	CCArchive(const Common::String &filename, const Common::String &prefix, bool encoded);
	~CCArchive() override;

	// Archive implementation
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
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
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;

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
	int32 pos() const override;
};

} // End of namespace Xeen

#endif /* XEEN_FILES_H */
