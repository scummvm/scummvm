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
#include "common/serializer.h"
#include "common/str-array.h"
#include "graphics/surface.h"

namespace Xeen {

enum ArchiveType {
	ANY_ARCHIVE = -1, GAME_ARCHIVE = 0, ALTSIDE_ARCHIVE = 1,
	INTRO_ARCHIVE = 2
};

class XeenEngine;
class CCArchive;
class File;

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
	friend class File;
private:
	static CCArchive *_archives[3];
public:
	bool _isDarkCc;
public:
	/**
	 * Instantiates the resource manager
	 */
	FileManager(XeenEngine *vm);

	/**
	 * Set which game side files to use
	 */
	void setGameCc(bool isDarkCc);
};

/**
 * Derived file class
 */
class File : public Common::File {
public:
	static ArchiveType _currentArchive;

	/**
	 * Sets which archive is used by default
	 */
	static void setCurrentArchive(ArchiveType arcType) { _currentArchive = arcType; }
public:
	File() : Common::File() {}
	File(const Common::String &filename);
	File(const Common::String &filename, ArchiveType archiveType);
	File(const Common::String &filename, Common::Archive &archive);
	virtual ~File() {}

	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	virtual bool open(const Common::String &filename);

	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	virtual bool open(const Common::String &filename, ArchiveType archiveType);

	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	virtual bool open(const Common::String &filename, Common::Archive &archive);

	/**
	 * Opens the given file
	 */
	virtual bool open(const Common::FSNode &node) {
		return Common::File::open(node);
	}

	/**
	 * Opens the given file
	 */
	virtual bool open(SeekableReadStream *stream, const Common::String &name) {
		return Common::File::open(stream, name);
	}

	Common::String readString();
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
	void load(const Common::String &name, ArchiveType archiveType);
};

class XeenSerializer : public Common::Serializer {
private:
	Common::SeekableReadStream *_in;
public:
	XeenSerializer(Common::SeekableReadStream *in, Common::WriteStream *out) :
		Common::Serializer(in, out), _in(in) {}

	SYNC_AS(Sint8, Byte, int8, 1)

	bool finished() const { return _in != nullptr && _in->pos() >= _in->size(); }
};

/**
* Details of a single entry in a CC file index
*/
struct CCEntry {
	uint16 _id;
	uint32 _offset;
	uint16 _size;

	CCEntry() : _id(0), _offset(0), _size(0) {}
	CCEntry(uint16 id, uint32 offset, uint32 size)
		: _id(id), _offset(offset), _size(size) {
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
	void loadIndex(Common::SeekableReadStream *stream);

	/**
	 * Given a resource name, returns whether an entry exists, and returns
	 * the header index data for that entry
	 */
	virtual bool getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const;
public:
	/**
	 * Hash a given filename to produce the Id that represents it
	 */
	static uint16 convertNameToId(const Common::String &resourceName);
public:
	BaseCCArchive() {}

	// Archive implementation
	virtual bool hasFile(const Common::String &name) const;
	virtual int listMembers(Common::ArchiveMemberList &list) const;
	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
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
	virtual bool getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const;
public:
	CCArchive(const Common::String &filename, bool encoded);
	CCArchive(const Common::String &filename, const Common::String &prefix, bool encoded);
	virtual ~CCArchive();

	// Archive implementation
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;
};

} // End of namespace Xeen

#endif /* XEEN_FILES_H */
