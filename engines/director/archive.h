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

#ifndef DIRECTOR_ARCHIVE_H
#define DIRECTOR_ARCHIVE_H

#include "common/file.h"

namespace Common {
class MacResManager;
class SeekableMemoryWriteStream;
class SeekableReadStreamEndian;
class SeekableReadStream;
class Path;
}

namespace Director {

// Completely ripped off of Mohawk's Archive code

struct Resource {
	uint32 index;
	int32 offset;
	uint32 size;
	uint32 uncompSize;
	uint32 compressionType;
	uint32 castId;
	uint32 libResourceId;
	uint32 tag;
	Common::String name;
	Common::Array<Resource> children;
	bool accessed;
};

class Archive {
public:
	Archive();
	virtual ~Archive();

	virtual bool openFile(const Common::Path &path);
	virtual bool openStream(Common::SeekableReadStream *stream, uint32 offset = 0) = 0;
	virtual void close();

	Common::Path getPathName() const { return _pathName; }
	Common::String getFileName() const;
	void setPathName(const Common::Path &name) { _pathName = name; }
	virtual uint32 getFileSize();

	bool isOpen() const { return _stream != 0; }

	bool hasResource(uint32 tag, int id) const;
	bool hasResource(uint32 tag, const Common::String &resName) const;
	virtual Common::SeekableReadStreamEndian *getResource(uint32 tag, uint16 id);
	virtual Common::SeekableReadStreamEndian *getFirstResource(uint32 tag);
	virtual Common::SeekableReadStreamEndian *getFirstResource(uint32 tag, uint16 parentId);
	virtual Resource getResourceDetail(uint32 tag, uint16 id);
	uint32 getOffset(uint32 tag, uint16 id) const;
	uint16 findResourceID(uint32 tag, const Common::String &resName, bool ignoreCase = false) const;
	Common::String getName(uint32 tag, uint16 id) const;
	Common::SeekableReadStreamEndian *getMovieResourceIfPresent(uint32 tag);

	Common::Array<uint32> getResourceTypeList() const;
	Common::Array<uint16> getResourceIDList(uint32 type) const;
	bool _isBigEndian;
	static uint32 convertTagToUppercase(uint32 tag);

	virtual Common::String formatArchiveInfo();

	void listUnaccessedChunks();

protected:
	void dumpChunk(Resource &res, Common::DumpFile &out);
	Common::SeekableReadStream *_stream;
	typedef Common::HashMap<uint16, Resource> ResourceMap;
	typedef Common::HashMap<uint32, ResourceMap> TypeMap;
	typedef Common::HashMap<uint32, uint32> MovieChunkMap;
	TypeMap _types;
	MovieChunkMap _movieChunks;

	Common::Path _pathName;
};

class MacArchive : public Archive {
public:
	MacArchive();
	~MacArchive() override;

	uint32 getFileSize() override;
	void close() override;
	bool openFile(const Common::Path &path) override;
	bool openStream(Common::SeekableReadStream *stream, uint32 startOffset = 0) override;
	Common::SeekableReadStreamEndian *getResource(uint32 tag, uint16 id) override;
	Common::String formatArchiveInfo() override;

private:
	Common::MacResManager *_resFork;

	void readTags();
};

class RIFFArchive : public Archive {
public:
	RIFFArchive() : Archive() { _startOffset = 0; }
	~RIFFArchive() override {}

	bool openStream(Common::SeekableReadStream *stream, uint32 startOffset = 0) override;
	Common::SeekableReadStreamEndian *getResource(uint32 tag, uint16 id) override;
	Common::String formatArchiveInfo() override;

	uint32 _startOffset;
};

class RIFXArchive : public Archive {
public:
	RIFXArchive();
	~RIFXArchive() override;

	bool openStream(Common::SeekableReadStream *stream, uint32 startOffset = 0) override;
	Common::SeekableReadStreamEndian *getFirstResource(uint32 tag) override;
	virtual Common::SeekableReadStreamEndian *getFirstResource(uint32 tag, bool fileEndianness);
	Common::SeekableReadStreamEndian *getFirstResource(uint32 tag, uint16 parentId) override;
	Common::SeekableReadStreamEndian *getResource(uint32 tag, uint16 id) override;
	virtual Common::SeekableReadStreamEndian *getResource(uint32 tag, uint16 id, bool fileEndianness);
	Resource getResourceDetail(uint32 tag, uint16 id) override;
	Common::String formatArchiveInfo() override;

private:
	bool readMemoryMap(Common::SeekableReadStreamEndian &stream, uint32 moreOffset, Common::SeekableMemoryWriteStream *dumpStream, uint32 movieStartOffset);
	bool readAfterburnerMap(Common::SeekableReadStreamEndian &stream, uint32 moreOffset);
	void readCast(Common::SeekableReadStreamEndian &casStream, uint16 libResourceId);
	void readKeyTable(Common::SeekableReadStreamEndian &keyStream);

protected:
	uint32 _rifxType;
	Common::Array<Resource *> _resources;
	Common::HashMap<uint32, byte *> _ilsData;
	uint32 _ilsBodyOffset;
	typedef Common::Array<uint32> KeyArray;
	typedef Common::HashMap<uint32, KeyArray> KeyMap;
	Common::HashMap<uint32, KeyMap> _keyData;
};

/*******************************************
 *
 * Projector Archive
 *
 *******************************************/

class ProjectorArchive : public Common::Archive {
public:
	ProjectorArchive(Common::Path path);
	~ProjectorArchive() override;

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
	bool isLoaded() { return _isLoaded; }
private:
	Common::SeekableReadStream *createBufferedReadStream();
	bool loadArchive(Common::SeekableReadStream *stream);

	struct Entry {
		uint32 offset;
		uint32 size;
	};
	typedef Common::HashMap<Common::Path, Entry, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> FileMap;
	FileMap _files;
	Common::Path _path;

	bool _isLoaded;
};
} // End of namespace Director

#endif
