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

#include "harvester/xfile_archive.h"

#include "common/file.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/substream.h"
#include "harvester/resources.h"

namespace Harvester {

namespace {

static const uint32 kEntrySize = 0x94;
static const uint32 kSignatureXfle = MKTAG('X', 'F', 'L', 'E');

} // End of anonymous namespace

XFileArchive::XFileArchive() : Common::Archive() {
}

XFileArchive::~XFileArchive() {
	close();
}

bool XFileArchive::open(const Common::String &indexPath, const Common::String &dataPath) {
	close();

	Common::ScopedPtr<Common::SeekableReadStream> indexStream(SearchMan.createReadStreamForMember(Common::Path(indexPath, '/')));
	if (!indexStream)
		return false;

	_dataPath = Common::Path(dataPath, '/');
	if (!SearchMan.hasFile(_dataPath)) {
		close();
		return false;
	}

	const uint32 fileCount = indexStream->size() / kEntrySize;
	for (uint32 i = 0; i < fileCount; ++i) {
		if (indexStream->readUint32LE() != kSignatureXfle) {
			close();
			return false;
		}

		Common::String resourcePath;
		for (uint32 j = 0; j < 0x80; ++j) {
			const char c = indexStream->readByte();
			if (c != '\0')
				resourcePath += c;
		}

		Entry entry;
		entry._archiveOffset = indexStream->readUint32LE();
		entry._storedSize = indexStream->readUint32LE();
		entry._packedFlag = indexStream->readUint32LE();
		entry._unpackedSize = indexStream->readUint32LE();

		const Common::String normalized = normalizeHarvesterResourcePath(resourcePath);
		if (!normalized.empty())
			_entries[Common::Path(normalized, '/')] = entry;
	}

	return !_entries.empty();
}

void XFileArchive::close() {
	_dataPath.clear();
	_entries.clear();
}

bool XFileArchive::hasFile(const Common::Path &path) const {
	return _entries.contains(path);
}

int XFileArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (const auto &entry : _entries)
		list.push_back(getMember(entry._key));

	return _entries.size();
}

const Common::ArchiveMemberPtr XFileArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *XFileArchive::createReadStreamForMember(const Common::Path &path) const {
	if (!_entries.contains(path))
		return nullptr;

	const Entry &entry = _entries[path];
	if (entry._packedFlag == 0)
		return openStoredEntry(entry);

	return openPackedEntry(entry);
}

Common::SeekableReadStream *XFileArchive::openStoredEntry(const Entry &entry) const {
	Common::File *dataFile = new Common::File();
	if (!dataFile->open(_dataPath)) {
		delete dataFile;
		return nullptr;
	}

	return new Common::SeekableSubReadStream(dataFile, entry._archiveOffset, entry._archiveOffset + entry._storedSize, DisposeAfterUse::YES);
}

Common::SeekableReadStream *XFileArchive::openPackedEntry(const Entry &entry) const {
	Common::File dataFile;
	if (!dataFile.open(_dataPath))
		return nullptr;

	dataFile.seek(entry._archiveOffset);

	Common::Array<byte> compressed;
	compressed.resize(entry._storedSize);
	if (!compressed.empty())
		dataFile.read(compressed.data(), compressed.size());

	byte *output = (byte *)malloc(entry._unpackedSize);
	if (!output)
		return nullptr;

	uint32 src = 0;
	uint32 dst = 0;
	while (src < compressed.size() && dst < entry._unpackedSize) {
		const byte control = compressed[src++];
		if (control < 0x81) {
			const uint32 literalCount = MIN<uint32>(control, MIN<uint32>(compressed.size() - src, entry._unpackedSize - dst));
			for (uint32 i = 0; i < literalCount; ++i)
				output[dst++] = compressed[src++];
		} else {
			if (src >= compressed.size())
				break;

			const byte value = compressed[src++];
			const uint32 repeatCount = MIN<uint32>(control - 0x80, entry._unpackedSize - dst);
			memset(output + dst, value, repeatCount);
			dst += repeatCount;
		}
	}

	if (dst != entry._unpackedSize) {
		free(output);
		return nullptr;
	}

	return new Common::MemoryReadStream(output, entry._unpackedSize, DisposeAfterUse::YES);
}

} // End of namespace Harvester
