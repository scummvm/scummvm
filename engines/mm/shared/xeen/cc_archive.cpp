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

#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "mm/shared/xeen/cc_archive.h"

namespace MM {
namespace Shared {
namespace Xeen {

uint16 BaseCCArchive::convertNameToId(const Common::String &resourceName) {
	if (resourceName.empty())
		return 0xffff;

	Common::String name = resourceName;
	name.toUppercase();

	// Check if a resource number is being directly specified
	if (name.size() == 4) {
		char *endPtr;
		uint16 num = (uint16)strtol(name.c_str(), &endPtr, 16);
		if (!*endPtr)
			return num;
	}

	const byte *msgP = (const byte *)name.c_str();
	int total = *msgP++;
	for (; *msgP; total += *msgP++) {
		// Rotate the bits in 'total' right 7 places
		total = (total & 0x007F) << 9 | (total & 0xFF80) >> 7;
	}

	return total;
}

void BaseCCArchive::loadIndex(Common::SeekableReadStream &stream) {
	int count = stream.readUint16LE();
	size_t size = count * 8;

	// Read in the data for the archive's index
	byte *rawIndex = new byte[size];

	if (stream.read(rawIndex, size) != size) {
		delete[] rawIndex;
		error("Failed to read %zu bytes from CC file", size);
	}

	// Decrypt the index
	int seed = 0xac;
	for (int i = 0; i < count * 8; ++i, seed += 0x67) {
		rawIndex[i] = (byte)((((rawIndex[i] << 2) | (rawIndex[i] >> 6)) + seed) & 0xff);
	}

	// Extract the index data into entry structures
	_index.resize(count);
	const byte *entryP = &rawIndex[0];
	for (int idx = 0; idx < count; ++idx, entryP += 8) {
		CCEntry entry;
		entry._id = READ_LE_UINT16(entryP);
		entry._offset = READ_LE_UINT32(entryP + 2) & 0xffffff;
		entry._size = READ_LE_UINT16(entryP + 5);
		assert(!entryP[7]);

		_index[idx] = entry;
	}

	delete[] rawIndex;
}

void BaseCCArchive::saveIndex(Common::WriteStream &stream) {
	// Fill up the data for the index entries into a raw data block
	byte *rawIndex = new byte[_index.size() * 8];
	byte b;

	byte *entryP = rawIndex;
	for (uint i = 0; i < _index.size(); ++i, entryP += 8) {
		CCEntry &entry = _index[i];
		WRITE_LE_UINT16(&entryP[0], entry._id);
		WRITE_LE_UINT32(&entryP[2], entry._writeOffset);
		WRITE_LE_UINT16(&entryP[5], entry._size);
		entryP[7] = 0;
	}

	// Encrypt the index
	int seed = 0xac;
	for (uint i = 0; i < _index.size() * 8; ++i, seed += 0x67) {
		b = (rawIndex[i] - seed) & 0xff;
		b = (byte)((b >> 2) | (b << 6));

		assert(rawIndex[i] == (byte)((((b << 2) | (b >> 6)) + seed) & 0xff));
		rawIndex[i] = b;
	}

	// Write out the number of entries and the encrypted index data
	stream.writeUint16LE(_index.size());
	stream.write(rawIndex, _index.size() * 8);

	delete[] rawIndex;
}

bool BaseCCArchive::hasFile(const Common::Path &path) const {
	Common::String name = path.toString();
	CCEntry ccEntry;
	return getHeaderEntry(name, ccEntry);
}

bool BaseCCArchive::getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const {
	return getHeaderEntry(convertNameToId(resourceName), ccEntry);
}

bool BaseCCArchive::getHeaderEntry(uint16 id, CCEntry &ccEntry) const {
	// Loop through the index
	for (uint i = 0; i < _index.size(); ++i) {
		if (_index[i]._id == id) {
			ccEntry = _index[i];
			return true;
		}
	}

	// Could not find an entry
	return false;
}

const Common::ArchiveMemberPtr BaseCCArchive::getMember(const Common::Path &path) const {
	Common::String name = path.toString();
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(Common::String(name), *this));
}

int BaseCCArchive::listMembers(Common::ArchiveMemberList &list) const {
	// CC files don't maintain the original filenames, so we can't list it
	return 0;
}

/*------------------------------------------------------------------------*/

CCArchive::CCArchive(const Common::String &filename, bool encoded) :
	BaseCCArchive(), _filename(filename), _encoded(encoded) {

	Common::File f;
	if (!f.open(filename, SearchMan))
		error("Could not open file - %s", filename.c_str());

	loadIndex(f);
}

CCArchive::CCArchive(const Common::String &filename, const Common::String &prefix,
	bool encoded) : BaseCCArchive(), _filename(filename),
	_prefix(prefix), _encoded(encoded) {
	_prefix.toLowercase();

	Common::File f;
	if (!f.open(filename, SearchMan))
		error("Could not open file - %s", filename.c_str());

	loadIndex(f);
}

CCArchive::~CCArchive() {
}

bool CCArchive::getHeaderEntry(const Common::String &resourceName, Shared::Xeen::CCEntry &ccEntry) const {
	Common::String resName = resourceName;

	if (!_prefix.empty() && resName.contains('|')) {
		resName.toLowercase();
		Common::String prefix = _prefix + "|";

		if (!strncmp(resName.c_str(), prefix.c_str(), prefix.size()))
			// Matching CC prefix, so strip it off and allow processing to
			// continue onto the base getHeaderEntry method
			resName = Common::String(resName.c_str() + prefix.size());
		else
			// Not matching prefix, so don't allow a match
			return false;
	}

	return BaseCCArchive::getHeaderEntry(resName, ccEntry);
}

Common::SeekableReadStream *CCArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::String name = path.toString();
	Shared::Xeen::CCEntry ccEntry;

	if (getHeaderEntry(name, ccEntry)) {
		// Open the correct CC file
		Common::File f;
		if (!f.open(_filename))
			error("Could not open CC file");

		// Read in the data for the specific resource
		if (!f.seek(ccEntry._offset))
			error("Failed to seek to %d bytes in CC file", ccEntry._offset);

		byte *data = (byte *)malloc(ccEntry._size);

		if (f.read(data, ccEntry._size) != ccEntry._size) {
			free(data);
			error("Failed to read %hu bytes in CC file", ccEntry._size);
		}

		if (_encoded) {
			// Decrypt the data
			for (int i = 0; i < ccEntry._size; ++i)
				data[i] ^= 0x35;
		}

		// Return the data as a stream
		return new Common::MemoryReadStream(data, ccEntry._size, DisposeAfterUse::YES);
	}

	return nullptr;
}

} // namespace Xeen
} // namespace Shared
} // namespace MM
