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
#include "graphics/surface.h"
#include "xeen/xsurface.h"

namespace Xeen {

class XeenEngine;
class CCArchive;

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
	bool _isDarkCc;
	CCArchive *_sideArchives[2];
public:
	FileManager(XeenEngine *vm);

	void setGameCc(bool isDarkCc) { _isDarkCc = isDarkCc; }
};

/**
 * Derived file class
 */
class File : public Common::File {
public:
	File() : Common::File() {}
	File(const Common::String &filename) { openFile(filename); }
	File(const Common::String &filename, Common::Archive &archive) {
		openFile(filename, archive);
	}
	virtual ~File() {}

	void openFile(const Common::String &filename);
	void openFile(const Common::String &filename, Common::Archive &archive);
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

	void loadIndex(Common::SeekableReadStream *stream);

	virtual bool getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const;
public:
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
