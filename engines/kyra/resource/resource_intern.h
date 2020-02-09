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

#ifndef KYRA_RESOURCE_INTERN_H
#define KYRA_RESOURCE_INTERN_H

#include "common/archive.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/list.h"
#include "common/stream.h"

namespace Kyra {

class Resource;

class PlainArchive : public Common::Archive {
public:
	struct Entry {
		Entry() : offset(0), size(0) {}
		Entry(uint32 o, uint32 s) : offset(o), size(s) {}

		uint32 offset;
		uint32 size;
	};

	PlainArchive(Common::ArchiveMemberPtr file);

	void addFileEntry(const Common::String &name, const Entry entry);
	Entry getFileEntry(const Common::String &name) const;

	// Common::Archive API implementation
	bool hasFile(const Common::String &name) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
private:
	typedef Common::HashMap<Common::String, Entry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	Common::ArchiveMemberPtr _file;
	FileMap _files;
};

class TlkArchive : public Common::Archive {
public:
	TlkArchive(Common::ArchiveMemberPtr file, uint16 entryCount, const uint32 *fileEntries);
	~TlkArchive() override;

	bool hasFile(const Common::String &name) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
private:
	Common::ArchiveMemberPtr _file;

	const uint32 *findFile(const Common::String &name) const;

	const uint16 _entryCount;
	const uint32 *const _fileEntries;
};

class CachedArchive : public Common::Archive {
public:
	struct InputEntry {
		Common::String name;

		byte *data;
		uint32 size;
	};

	typedef Common::List<InputEntry> FileInputList;

	CachedArchive(const FileInputList &files);
	~CachedArchive() override;

	bool hasFile(const Common::String &name) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
private:
	struct Entry {
		byte *data;
		uint32 size;
	};

	typedef Common::HashMap<Common::String, Entry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;
	FileMap _files;
};


class ResArchiveLoader {
public:
	virtual ~ResArchiveLoader() {}
	virtual bool checkFilename(Common::String filename) const = 0;
	virtual bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const = 0;
	virtual Common::Archive *load(Common::ArchiveMemberPtr file, Common::SeekableReadStream &stream) const = 0;
};

class ResLoaderPak : public ResArchiveLoader {
public:
	bool checkFilename(Common::String filename) const override;
	bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const override;
	Common::Archive *load(Common::ArchiveMemberPtr file, Common::SeekableReadStream &stream) const override;
};

class ResLoaderInsMalcolm : public ResArchiveLoader {
public:
	bool checkFilename(Common::String filename) const override;
	bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const override;
	Common::Archive *load(Common::ArchiveMemberPtr file, Common::SeekableReadStream &stream) const override;
};

class ResLoaderTlk : public ResArchiveLoader {
public:
	bool checkFilename(Common::String filename) const override;
	bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const override;
	Common::Archive *load(Common::ArchiveMemberPtr file, Common::SeekableReadStream &stream) const override;
};

class InstallerLoader {
public:
	static Common::Archive *load(Resource *owner, const Common::String &filename, const Common::String &extension, const uint8 offset);
};

class EndianAwareStreamWrapper : public Common::SeekableReadStreamEndian {
public:
	EndianAwareStreamWrapper(Common::SeekableReadStream *stream, bool bigEndian, bool disposeAfterUse = true) : Common::SeekableReadStreamEndian(bigEndian), Common::ReadStreamEndian(bigEndian), _stream(stream), _dispose(disposeAfterUse) {}
	~EndianAwareStreamWrapper() override { if (_dispose) delete _stream; }

	// Common::Stream interface
	bool err() const override { return _stream->err(); }

	// Common::ReadStream interface
	bool eos() const override { return _stream->eos(); }
	uint32 read(void *dataPtr, uint32 dataSize) override { return _stream->read(dataPtr, dataSize); }

	// Common::SeekableReadStream interface
	int32 pos() const override { return _stream->pos(); }
	int32 size() const override { return _stream->size(); }
	bool seek(int32 offset, int whence = SEEK_SET) override { return _stream->seek(offset, whence); }

private:
	Common::SeekableReadStream *_stream;
	bool _dispose;
};

} // End of namespace Kyra

#endif
