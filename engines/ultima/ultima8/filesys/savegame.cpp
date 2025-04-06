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

#include "ultima/ultima8/filesys/savegame.h"
#include "common/bufferedstream.h"
#include "common/compression/unzip.h"

namespace Ultima {
namespace Ultima8 {

#define SAVEGAME_IDENT MKTAG('V', 'M', 'U', '8')
#define PKZIP_IDENT MKTAG('P', 'K', 3, 4)
#define SAVEGAME_VERSION 6
#define SAVEGAME_MIN_VERSION 2

class FileEntryArchive : public Common::Archive {
	struct FileEntry {
		uint _offset;
		uint _size;
		FileEntry() : _offset(0), _size(0) {}
	};
private:
	typedef Common::HashMap<Common::Path, FileEntry, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> IndexMap;
	IndexMap _index;
	Common::SeekableReadStream *_file;

public:
	FileEntryArchive(Common::SeekableReadStream *rs);
	~FileEntryArchive() override;

	// Common::Archive API implementation
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
};

FileEntryArchive::FileEntryArchive(Common::SeekableReadStream *rs) : _file(rs) {
	// Load the index
	uint count = _file->readUint16LE();

	for (uint idx = 0; idx < count; ++idx) {
		char name[12];
		_file->read(name, 12);
		name[11] = '\0';

		FileEntry fe;
		fe._size = _file->readUint32LE();
		fe._offset = _file->pos();

		_index[Common::Path(name, Common::Path::kNoSeparator)] = fe;
		_file->skip(fe._size);
	}
}

FileEntryArchive::~FileEntryArchive() {
}

bool FileEntryArchive::hasFile(const Common::Path &path) const {
	return _index.contains(path);
}

int FileEntryArchive::listMembers(Common::ArchiveMemberList &list) const {
	list.clear();
	for (const auto &member : _index)
		list.push_back(Common::ArchiveMemberPtr(new Common::GenericArchiveMember(member._key, *this)));

	return list.size();
}

const Common::ArchiveMemberPtr FileEntryArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return nullptr;

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *FileEntryArchive::createReadStreamForMember(const Common::Path &path) const {
	assert(_index.contains(path));

	const FileEntry &fe = _index[path];
	uint8 *data = (uint8 *)malloc(fe._size);
	_file->seek(fe._offset);
	_file->read(data, fe._size);

	return new Common::MemoryReadStream(data, fe._size, DisposeAfterUse::YES);
}

SavegameReader::SavegameReader(Common::SeekableReadStream *rs, bool metadataOnly) : _archive(nullptr), _version(0) {
	// Validate the identifier for a valid savegame
	uint32 ident = rs->readUint32LE();
	if (ident == SAVEGAME_IDENT) {
		_version = rs->readUint32LE();

		if (!MetaEngine::readSavegameHeader(rs, &_header))
			return;

		if (metadataOnly)
			return;

		_archive = new FileEntryArchive(rs);
	} else if (SWAP_BYTES_32(ident) == PKZIP_IDENT) {
		// Note: Pentagram save description is the zip global comment
		_header.description = "Pentagram Save";

		// Hack to pull the comment if length < 255
		char data[256];
		uint16 size = sizeof(data);
		rs->seek(-size, SEEK_END);
		rs->read(data, size);
		for (uint16 i = size; i >= 2; i--) {
			uint16 length = size - i;
			if (data[i - 2] == length && data[i - 1] == 0) {
				if (length > 0)
					_header.description = Common::String(data + i, length);
				break;
			}
		}

		Common::SeekableReadStream *stream = wrapBufferedSeekableReadStream(rs, 4096, DisposeAfterUse::NO);
		_archive = Common::makeZipArchive(stream);
		if (!_archive)
			return;

		Common::ArchiveMemberPtr member = _archive->getMember("VERSION");
		if (member) {
			_version = member->createReadStream()->readUint32LE();
			_header.version = _version;
		}

		if (metadataOnly) {
			delete _archive;
			_archive = nullptr;
			return;
		}
	}
}

SavegameReader::~SavegameReader() {
	if (_archive)
		delete _archive;
}

SavegameReader::State SavegameReader::isValid() const {
	if (_version == 0)
		return SAVE_CORRUPT;
	else if (_version < SAVEGAME_MIN_VERSION)
		return SAVE_OUT_OF_DATE;
	else if (_version > SAVEGAME_VERSION)
		return SAVE_TOO_RECENT;

	return SAVE_VALID;
}

Common::SeekableReadStream *SavegameReader::getDataSource(const Common::Path &name) {
	assert(_archive);

	return _archive->createReadStreamForMember(name);
}


SavegameWriter::SavegameWriter(Common::WriteStream *ws) : _file(ws) {
	assert(_file);
}

SavegameWriter::~SavegameWriter() {
}

bool SavegameWriter::finish() {
	 // Write ident and savegame version
	_file->writeUint32LE(SAVEGAME_IDENT);
	_file->writeUint32LE(SAVEGAME_VERSION);

	// Iterate through writing out the files
	_file->writeUint16LE(_index.size());
	for (uint idx = 0; idx < _index.size(); ++idx) {
		// Set up a 12 byte space containing the resource name
		FileEntry &fe = _index[idx];
		char name[12];
		Common::fill(&name[0], &name[12], '\0');
		strncpy(name, fe._name.c_str(), 11);

		// Write out name, size, and data
		_file->write(name, 12);
		_file->writeUint32LE(fe.size());
		_file->write(&fe[0], fe.size());
	}

	return true;
}

bool SavegameWriter::writeFile(const Std::string &name, const uint8 *data, uint32 size) {
	assert(name.size() <= 11);
	_index.push_back(FileEntry());

	FileEntry &fe = _index.back();
	fe._name = name;
	fe.resize(size);
	Common::copy(data, data + size, &fe[0]);

	return true;
}

bool SavegameWriter::writeFile(const Std::string &name, Common::MemoryWriteStreamDynamic *buf) {
	return writeFile(name, buf->getData(), buf->pos());
}

} // End of namespace Ultima8
} // End of namespace Ultima
