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

#ifndef KYRA_RESOURCE_INTERN_H
#define KYRA_RESOURCE_INTERN_H

#include "common/archive.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/list.h"
#include "common/macresman.h"
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

	void addFileEntry(const Common::Path &name, const Entry entry);
	Entry getFileEntry(const Common::Path &name) const;

	// Common::Archive API implementation
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
private:
	typedef Common::HashMap<Common::Path, Entry, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> FileMap;

	Common::ArchiveMemberPtr _file;
	FileMap _files;
};

class TlkArchive : public Common::Archive {
public:
	TlkArchive(Common::ArchiveMemberPtr file, uint16 entryCount, const uint32 *fileEntries);
	~TlkArchive() override;

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
private:
	Common::ArchiveMemberPtr _file;

	const uint32 *findFile(const Common::Path &path) const;

	const uint16 _entryCount;
	const uint32 *const _fileEntries;
};

class CachedArchive : public Common::Archive {
public:
	struct InputEntry {
		Common::Path name;

		byte *data;
		uint32 size;
	};

	typedef Common::List<InputEntry> FileInputList;

	CachedArchive(const FileInputList &files);
	~CachedArchive() override;

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
private:
	struct Entry {
		byte *data;
		uint32 size;
	};

	typedef Common::HashMap<Common::Path, Entry, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> FileMap;
	FileMap _files;
};


class ResArchiveLoader {
public:
	virtual ~ResArchiveLoader() {}
	virtual bool checkFilename(const Common::String &filename) const = 0;
	virtual bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const = 0;
	virtual Common::Archive *load(Common::ArchiveMemberPtr file, Common::SeekableReadStream &stream) const = 0;
};

class ResLoaderPak : public ResArchiveLoader {
public:
	bool checkFilename(const Common::String &filename) const override;
	bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const override;
	Common::Archive *load(Common::ArchiveMemberPtr file, Common::SeekableReadStream &stream) const override;
};

class ResLoaderInsMalcolm : public ResArchiveLoader {
public:
	bool checkFilename(const Common::String &filename) const override;
	bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const override;
	Common::Archive *load(Common::ArchiveMemberPtr file, Common::SeekableReadStream &stream) const override;
};

class ResLoaderTlk : public ResArchiveLoader {
public:
	bool checkFilename(const Common::String &filename) const override;
	bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const override;
	Common::Archive *load(Common::ArchiveMemberPtr file, Common::SeekableReadStream &stream) const override;
};

class InstallerLoader {
public:
	static Common::Archive *load(Resource *owner, const Common::Path &filename, const Common::String &extension, const uint8 offset);
};

class StuffItLoader {
public:
	static Common::Archive *load(Resource *owner, const Common::Path &filename);
	static Common::Archive *load(Resource *owner, Common::SeekableReadStream *stream, const Common::String &debugName);
};

} // End of namespace Kyra

#endif
