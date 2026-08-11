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

#include "engines/tinsel/psx_archive.h"

#include "common/file.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/substream.h"

namespace Tinsel {

PsxArchive::PsxArchive() : Common::Archive() {
}

PsxArchive::~PsxArchive() {
	close();
}

bool PsxArchive::open(const Common::Path &indexFilePath, const Common::Path &dataFilePath, uint16 tinselVersion) {
	close();

    // open index file
    Common::ScopedPtr<Common::SeekableReadStream> indexStream(SearchMan.createReadStreamForMember(indexFilePath));
    if (!indexStream) {
        return false;
    }

    // read file entries from index file.
    //   c[12]  file name, zero terminated unless 12 characters
    //   u32LE  file size
    //   u32LE  file offset, shifted right 11 bits in DW1
    int fileCount = indexStream->size() / (12 + 4 + 4);
    for (int i = 0; i < fileCount; i++) {
        int entryPos = indexStream->pos();
        Common::String fileName = indexStream->readString('\0', 12);
        indexStream->seek(entryPos + 12);

        FileEntry fileEntry;
        fileEntry.size = indexStream->readUint32LE();
        fileEntry.offset = indexStream->readUint32LE();
        if (tinselVersion == 1) {
            fileEntry.offset <<= 11;
        }
        _map[Common::Path(fileName)] = fileEntry;
    }

    // verify data file exists
    _dataFilePath = dataFilePath;
    if (!SearchMan.hasFile(_dataFilePath)) {
        close();
        return false;
    }
    return true;
}

void PsxArchive::close() {
	_dataFilePath.clear();
	_map.clear();
}

bool PsxArchive::hasFile(const Common::Path &path) const {
	return _map.contains(path);
}

int PsxArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (const auto &file : _map) {
		list.push_back(getMember(file._key));
    }

	return _map.size();
}

const Common::ArchiveMemberPtr PsxArchive::getMember(const Common::Path &path) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *PsxArchive::createReadStreamForMember(const Common::Path &path) const {
	if (!_map.contains(path))
		return nullptr;

    // open data file
    Common::File *dataFile = new Common::File();
    if (!dataFile->open(_dataFilePath)) {
        delete dataFile;
        return nullptr;
    }

    // return a substream using offset and size from index
	const FileEntry &entry = _map[path];
    return new Common::SeekableSubReadStream(dataFile, entry.offset, entry.offset + entry.size, DisposeAfterUse::YES);
}

} // End of namespace Tinsel
