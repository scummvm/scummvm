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

#include "common/debug.h"
#include "common/memstream.h"
#include "common/substream.h"

namespace Myst3 {

void Archive::decryptHeader(Common::SeekableReadStream &inStream, Common::WriteStream &outStream) {
	static const uint32 addKey = 0x3C6EF35F;
	static const uint32 multKey = 0x0019660D;

	inStream.seek(0);
	uint32 size = inStream.readUint32LE();

	bool encrypted = size > 1000000;
	
	inStream.seek(0);

	if (encrypted) {
		uint32 decryptedSize = size ^ addKey;

		uint32 currentKey = 0;
		for (uint i = 0; i < decryptedSize; i++) {
			currentKey += addKey;
			outStream.writeUint32LE(inStream.readUint32LE() ^ currentKey);
			currentKey *= multKey;
		}
	} else {
		for (uint i = 0; i < size; i++) {
			outStream.writeUint32LE(inStream.readUint32LE());
		}
	}
}

static Common::String readFixedString(Common::ReadStream &stream, uint32 length) {
	Common::String value;

	for (uint i = 0; i < length; i++) {
		value += stream.readByte();
	}

	return value;
}

static uint32 readUint24(Common::ReadStream &stream) {
	uint32 value = stream.readUint16LE();
	value |= stream.readByte() << 16;
	return value;
}

Archive::DirectorySubEntry Archive::readSubEntry(Common::ReadStream &stream) {
	DirectorySubEntry subEntry;

	subEntry.offset = stream.readUint32LE();
	subEntry.size = stream.readUint32LE();
	uint16 metadataSize = stream.readUint16LE();
	subEntry.face = stream.readByte();
	subEntry.type = static_cast<ResourceType>(stream.readByte());

	subEntry.metadata.resize(metadataSize);
	for (uint i = 0; i < metadataSize; i++) {
		subEntry.metadata[i] = stream.readUint32LE();
	}

	return subEntry;
}

Archive::DirectoryEntry Archive::readEntry(Common::ReadStream &stream) {
	DirectoryEntry entry;
	if (_roomName.empty()) {
		entry.roomName = readFixedString(stream, 4);
	} else {
		entry.roomName = _roomName;
	}
	entry.index = readUint24(stream);

	byte subItemCount = stream.readByte();
	entry.subentries.resize(subItemCount);

	for (uint i = 0; i < subItemCount; i++) {
		entry.subentries[i] = readSubEntry(stream);
	}

	return entry;
}

void Archive::readDirectory() {
	Common::MemoryWriteStreamDynamic buf(DisposeAfterUse::YES);
	decryptHeader(_file, buf);

	Common::MemoryReadStream directory(buf.getData(), buf.size());
	_directorySize = directory.readUint32LE();

	while (directory.pos() + 4 < directory.size()) {
		_directory.push_back(readEntry(directory));
	}
}

void Archive::visit(ArchiveVisitor &visitor) {
	visitor.visitArchive(*this);

	for (uint i = 0; i < _directory.size(); i++) {
		visitor.visitDirectoryEntry(_directory[i]);

		for (uint j = 0; j < _directory[i].subentries.size(); j++) {
			visitor.visitDirectorySubEntry(_directory[i].subentries[j]);
		}
	}
}

Common::SeekableReadStream *Archive::dumpToMemory(uint32 offset, uint32 size) {
	_file.seek(offset);
	return _file.readStream(size);
}

uint32 Archive::copyTo(uint32 offset, uint32 size, Common::WriteStream &out) {
	Common::SeekableSubReadStream subStream(&_file, offset, offset + size);
	subStream.seek(0);
	return out.writeStream(&subStream);
}

const Archive::DirectoryEntry *Archive::getEntry(const Common::String &room, uint32 index) const {
	for (uint i = 0; i < _directory.size(); i++) {
		const DirectoryEntry &entry = _directory[i];
		if (entry.index == index && entry.roomName == room) {
			return &entry;
		}
	}

	return nullptr;
}

ResourceDescription Archive::getDescription(const Common::String &room, uint32 index, uint16 face,
                                                 ResourceType type) {
	const DirectoryEntry *entry = getEntry(room, index);
	if (!entry) {
		return ResourceDescription();
	}

	for (uint i = 0; i < entry->subentries.size(); i++) {
		const DirectorySubEntry &subentry = entry->subentries[i];
		if (subentry.face == face && subentry.type == type) {
			return ResourceDescription(this, subentry);
		}
	}

	return ResourceDescription();
}

ResourceDescriptionArray Archive::listFilesMatching(const Common::String &room, uint32 index, uint16 face,
                                                 ResourceType type) {
	const DirectoryEntry *entry = getEntry(room, index);
	if (!entry) {
		return ResourceDescriptionArray();
	}

	ResourceDescriptionArray list;
	for (uint i = 0; i < entry->subentries.size(); i++) {
		const DirectorySubEntry &subentry = entry->subentries[i];
		if (subentry.face == face && subentry.type == type) {
			list.push_back(ResourceDescription(this, subentry));
		}
	}

	return list;
}

bool Archive::open(const char *fileName, const char *room) {
	// If the room name is not provided, it is assumed that
	// we are opening a multi-room archive
	if (room) {
		_roomName = room;
	}

	if (_file.open(fileName)) {
		readDirectory();
		return true;
	}
	
	return false;
}

void Archive::close() {
	_directorySize = 0;
	_roomName.clear();
	_directory.clear();
	_file.close();
}

ResourceDescription::ResourceDescription() :
		_archive(nullptr),
		_subentry(nullptr) {
}

ResourceDescription::ResourceDescription(Archive *archive, const Archive::DirectorySubEntry &subentry) :
		_archive(archive),
		_subentry(&subentry) {
}

Common::SeekableReadStream *ResourceDescription::getData() const {
	return _archive->dumpToMemory(_subentry->offset, _subentry->size);
}

ResourceDescription::SpotItemData ResourceDescription::getSpotItemData() const {
	assert(_subentry->type == Archive::kSpotItem || _subentry->type == Archive::kLocalizedSpotItem);

	SpotItemData spotItemData;
	spotItemData.u = _subentry->metadata[0];
	spotItemData.v = _subentry->metadata[1];

	return spotItemData;
}

ResourceDescription::VideoData ResourceDescription::getVideoData() const {
	VideoData videoData;

	if (_subentry->type == Archive::kMovie || _subentry->type == Archive::kMultitrackMovie) {
		videoData.v1.setValue(0, static_cast<int32>(_subentry->metadata[0]) * 0.000001f);
		videoData.v1.setValue(1, static_cast<int32>(_subentry->metadata[1]) * 0.000001f);
		videoData.v1.setValue(2, static_cast<int32>(_subentry->metadata[2]) * 0.000001f);

		videoData.v2.setValue(0, static_cast<int32>(_subentry->metadata[3]) * 0.000001f);
		videoData.v2.setValue(1, static_cast<int32>(_subentry->metadata[4]) * 0.000001f);
		videoData.v2.setValue(2, static_cast<int32>(_subentry->metadata[5]) * 0.000001f);

		videoData.u      = static_cast<int32>(_subentry->metadata[6]);
		videoData.v      = static_cast<int32>(_subentry->metadata[7]);
		videoData.width  = static_cast<int32>(_subentry->metadata[8]);
		videoData.height = static_cast<int32>(_subentry->metadata[9]);
	}

	return videoData;
}

uint32 ResourceDescription::getMiscData(uint index) const {
	assert(_subentry->type == Archive::kNumMetadata || _subentry->type == Archive::kTextMetadata);

	if (index == 0) {
		return _subentry->offset;
	} else if (index == 1) {
		return _subentry->size;
	} else {
		return _subentry->metadata[index - 2];
	}
}

Common::String ResourceDescription::getTextData(uint index) const {
	assert(_subentry->type == Archive::kTextMetadata);

	uint8 key = 35;
	uint8 cnt = 0;
	uint8 decrypted[89];
	memset(decrypted, 0, sizeof(decrypted));

	uint8 *out = &decrypted[0];
	while (cnt / 4 < (_subentry->metadata.size() + 2) && cnt < 89) {
		// XORed text stored in little endian 32 bit words
		*out++ = (getMiscData(cnt / 4) >> (8 * (3 - (cnt % 4)))) ^ key++;
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

ArchiveVisitor::~ArchiveVisitor() {
}

} // End of namespace Myst3
