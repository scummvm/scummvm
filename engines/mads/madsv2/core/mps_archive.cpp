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
#include "common/compression/unzip.h"
#include "mads/madsv2/core/mps_archive.h"

namespace MADS {
namespace MADSV2 {

static constexpr int HEADER_SIZE = 30;
static constexpr int ENTRY_SIZE = 98;

MpsArchive *MpsArchive::open(const char *baseName) {
	Common::File indexFile, dataFile;
	Common::String indexFilename = Common::String::format("%s.idx", baseName);
	Common::String dataFilename;

	if (!indexFile.open(indexFilename.c_str()))
		error("Could not open %s", indexFilename.c_str());

	indexFile.skip(2);
	int totalFiles = indexFile.readUint16LE();
	int numZips = indexFile.readUint16LE();

	if (indexFile.readString('\0', 13) != "MPSLABS")
		error("Invalid mpslabs index file");

	// The mpslabs.idx file consists of a 30 byte header, followed by 98 byte
	// entries for all the files. However, this is followed by extra entries
	// specifying overall wad files within the msplabs.00? files, which are
	// basically just bog-standard zip files
	MpsArchive *result = new MpsArchive();
	indexFile.seek(HEADER_SIZE + totalFiles * ENTRY_SIZE);
	byte entry[ENTRY_SIZE];
	int currentNum = -1;

	for (int zipNum = 0; zipNum < numZips; ++zipNum) {
		// Read the next entry
		indexFile.read(entry, ENTRY_SIZE);
		const int mpsNum = READ_LE_UINT16(entry + 0x54);
		const int offset = READ_LE_UINT32(entry + 0x56);
		const int size = READ_LE_UINT32(entry + 0x5a);

		// Ensure the correct mpslabs .00? file is open
		if (mpsNum != currentNum) {
			currentNum = mpsNum;
			dataFilename = Common::String::format("%s.%03d", baseName, mpsNum);
			if (!dataFile.open(dataFilename.c_str()))
				error("Could not open %s", dataFilename.c_str());
		}

		// Get the section within the archive
		dataFile.seek(offset);
		Common::SeekableReadStream *zipStream = dataFile.readStream(size);
		Common::Archive *zip = Common::makeZipArchive(zipStream);

		if (!zip)
			error("Could not decompress data archive");

		// Add the archive to the list
		result->_zips.push_back(zip);
	}

	return result;
}

MpsArchive::~MpsArchive() {
	for (Common::Archive *zip : _zips)
		delete zip;
}

bool MpsArchive::hasFile(const Common::Path &path) const {
	for (const Common::Archive *zip : _zips) {
		if (zip->hasFile(path))
			return true;
	}

	return false;
}

int MpsArchive::listMembers(Common::ArchiveMemberList &list) const {
	int total = 0;
	for (const Common::Archive *zip : _zips)
		total += zip->listMembers(list);

	return total;
}

const Common::ArchiveMemberPtr MpsArchive::getMember(const Common::Path &path) const {
	for (const Common::Archive *zip : _zips) {
		if (zip->hasFile(path))
			return zip->getMember(path);
	}

	return nullptr;
}

Common::SeekableReadStream *MpsArchive::createReadStreamForMember(const Common::Path &path) const {
	for (const Common::Archive *zip : _zips) {
		Common::SeekableReadStream *result = zip->createReadStreamForMember(path);
		if (result)
			return result;
	}

	return nullptr;
}

} // namespace MADSV2
} // namespace MADS
