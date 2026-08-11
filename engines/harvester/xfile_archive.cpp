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

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/substream.h"
#include "harvester/detection.h"
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
	if (!indexStream) {
		debugC(1, kDebugResources, "Harvester: missing archive index %s", indexPath.c_str());
		return false;
	}

	_dataPath = Common::Path(dataPath, '/');
	if (!SearchMan.hasFile(_dataPath)) {
		debugC(1, kDebugResources, "Harvester: missing archive data %s", dataPath.c_str());
		close();
		return false;
	}

	_dataFile.reset(new Common::File());
	if (!_dataFile->open(_dataPath)) {
		debugC(1, kDebugResources, "Harvester: unable to open archive data %s", dataPath.c_str());
		close();
		return false;
	}

	const uint32 fileCount = indexStream->size() / kEntrySize;
	for (uint32 i = 0; i < fileCount; ++i) {
		if (indexStream->readUint32BE() != kSignatureXfle) {
			debugC(1, kDebugResources, "Harvester: invalid XFLE signature in %s at entry %u", indexPath.c_str(), i);
			close();
			return false;
		}

		char resourcePathBuffer[0x80];
		indexStream->read(resourcePathBuffer, sizeof(resourcePathBuffer));

		uint32 resourcePathLength = 0;
		while (resourcePathLength < sizeof(resourcePathBuffer) && resourcePathBuffer[resourcePathLength] != '\0')
			++resourcePathLength;

		Common::String resourcePath(resourcePathBuffer, resourcePathLength);

		Entry entry;
		entry._archiveOffset = indexStream->readUint32LE();
		entry._storedSize = indexStream->readUint32LE();
		entry._packedFlag = indexStream->readUint32LE();
		entry._unpackedSize = indexStream->readUint32LE();

		const Common::String normalized = normalizeHarvesterResourcePath(resourcePath);
		if (!normalized.empty())
			_entries[Common::Path(normalized, '/')] = entry;
	}

	debugC(1, kDebugResources, "Harvester: indexed %u members from %s", (uint)_entries.size(), indexPath.c_str());

	return !_entries.empty();
}

void XFileArchive::close() {
	_dataPath.clear();
	_dataFile.reset();
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
	if (!_dataFile)
		return nullptr;

	return new Common::SafeMutexedSeekableSubReadStream(
		_dataFile.get(), entry._archiveOffset, entry._archiveOffset + entry._storedSize,
		DisposeAfterUse::NO, _dataFileMutex);
}

Common::SeekableReadStream *XFileArchive::openPackedEntry(const Entry &entry) const {
	if (!_dataFile)
		return nullptr;

	Common::ScopedPtr<Common::SeekableReadStream> dataStream(new Common::SafeMutexedSeekableSubReadStream(
		_dataFile.get(), entry._archiveOffset, entry._archiveOffset + entry._storedSize,
		DisposeAfterUse::NO, _dataFileMutex));

	Common::Array<byte> compressed;
	compressed.resize(entry._storedSize);
	if (!compressed.empty())
		dataStream->read(compressed.data(), compressed.size());

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
