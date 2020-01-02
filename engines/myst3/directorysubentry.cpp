/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/archive.h"
#include "engines/myst3/directorysubentry.h"

#include "common/str.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"

namespace Myst3 {

DirectorySubEntry::DirectorySubEntry(Archive *archive) :
		_archive(archive) {
}

void DirectorySubEntry::readFromStream(Common::SeekableReadStream &inStream) {
	_offset = inStream.readUint32LE();
	_size = inStream.readUint32LE();
	_metadataSize = inStream.readUint16LE();
	_face = inStream.readByte();
	_type = static_cast<ResourceType>(inStream.readByte());

	if (_metadataSize == 2 && (_type == kSpotItem || _type == kLocalizedSpotItem)) {
		_spotItemData.u = inStream.readUint32LE();
		_spotItemData.v = inStream.readUint32LE();
	} else if (_metadataSize == 10 && (_type == kMovie || _type == kMultitrackMovie)) {
		_videoData.v1.setValue(0, inStream.readSint32LE() * 0.000001f);
		_videoData.v1.setValue(1, inStream.readSint32LE() * 0.000001f);
		_videoData.v1.setValue(2, inStream.readSint32LE() * 0.000001f);

		_videoData.v2.setValue(0, inStream.readSint32LE() * 0.000001f);
		_videoData.v2.setValue(1, inStream.readSint32LE() * 0.000001f);
		_videoData.v2.setValue(2, inStream.readSint32LE() * 0.000001f);

		_videoData.u = inStream.readSint32LE();
		_videoData.v = inStream.readSint32LE();
		_videoData.width = inStream.readSint32LE();
		_videoData.height = inStream.readSint32LE();
	} else if (_type == kNumMetadata || _type == kTextMetadata) {
		_miscData.resize(_metadataSize + 2);
		_miscData[0] = _offset;
		_miscData[1] = _size;

		for (uint i = 0; i < _metadataSize; i++)
			_miscData[i + 2] = inStream.readUint32LE();
	} else if (_metadataSize != 0) {
		warning("Metadata not read for type %d, size %d", _type, _metadataSize);
		inStream.skip(_metadataSize * sizeof(uint32));
	}
}

void DirectorySubEntry::dumpToFile(Common::SeekableReadStream &inStream, const char* room, uint32 index) {
	char fileName[255];
	
	switch (_type) {
		case kNumMetadata:
		case kTextMetadata:
			return; // These types are pure metadata and can't be extracted
		case kCubeFace:
		case kSpotItem:
		case kLocalizedSpotItem:
		case kFrame:
			sprintf(fileName, "dump/%s-%d-%d.jpg", room, index, _face);
			break;
		case kWaterEffectMask:
			sprintf(fileName, "dump/%s-%d-%d.mask", room, index, _face);
			break;
		case kMovie:
		case kStillMovie:
		case kDialogMovie:
		case kMultitrackMovie:
			sprintf(fileName, "dump/%s-%d.bik", room, index);
			break;
		default:
			sprintf(fileName, "dump/%s-%d-%d.%d", room, index, _face, _type);
			break;
	}
	
	
	debug("Extracted %s", fileName);
	
	Common::DumpFile outFile;
	if (!outFile.open(fileName, true))
		error("Unable to open file '%s' for writing", fileName);
	
	inStream.seek(_offset);
	
	uint8 *buf = new uint8[_size];
	
	inStream.read(buf, _size);
	outFile.write(buf, _size);
	
	delete[] buf;
	
	outFile.close();
}

Common::MemoryReadStream *DirectorySubEntry::getData() const {
	return _archive->dumpToMemory(_offset, _size);
}

uint32 DirectorySubEntry::getMiscData(uint index) const {
	return _miscData[index];
}

Common::String DirectorySubEntry::getTextData(uint index) const {
	uint8 key = 35;
	uint8 cnt = 0;
	uint8 decrypted[89];
	memset(decrypted, 0, sizeof(decrypted));

	uint8 *out = &decrypted[0];
	while (cnt / 4 < _miscData.size() && cnt < 89) {
		// XORed text stored in little endian 32 bit words
		*out++ = (_miscData[cnt / 4] >> (8 * (3 - (cnt % 4)))) ^ key++;
		cnt++;
	}

	// decrypted contains a null separated string array
	// extract the wanted one
	cnt = 0;
	int i = 0;
	Common::String text;
	while (cnt <= index && i < 89) {
		if (cnt == index)
			text += decrypted[i];

		if (!decrypted[i])
			cnt++;

		i++;
	}

	return text;
}

} // End of namespace Myst3
