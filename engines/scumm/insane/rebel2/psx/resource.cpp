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
 */

#include "common/endian.h"
#include "common/stream.h"

#include "scumm/insane/rebel2/psx/psx.h"

namespace Scumm {

bool RA2PSXArchive::load(Common::SeekableReadStream &stream) {
	_data.clear();
	if (stream.size() <= 0 || stream.size() > 64 * 1024 * 1024)
		return false;
	_data.resize((uint32)stream.size());
	stream.seek(0);
	return stream.read(_data.data(), _data.size()) == _data.size();
}

bool RA2PSXArchive::findTopLevelEntry(const Common::String &name, Entry &entry) const {
	for (uint32 index = 0; (index + 1) * 16 <= _data.size(); ++index) {
		const uint32 record = index * 16;
		if (_data[record] == 0)
			break;

		Common::String entryName((const char *)_data.data() + record, 8);
		while (!entryName.empty() && entryName.lastChar() == 0)
			entryName.deleteLastChar();
		if (!entryName.equalsIgnoreCase(name))
			continue;

		const uint32 nextRecord = record + 16;
		if (nextRecord + 16 > _data.size())
			return false;
		entry.name = entryName;
		entry.unpackedSize = READ_LE_UINT32(_data.data() + record + 8);
		entry.offset = (READ_LE_UINT32(_data.data() + record + 12) & 0x7fffffffU) + record;
		entry.endOffset = (READ_LE_UINT32(_data.data() + nextRecord + 12) & 0x7fffffffU) + nextRecord;
		return entry.offset <= entry.endOffset && entry.endOffset <= _data.size();
	}
	return false;
}

bool RA2PSXArchive::decompress(const byte *source, uint32 sourceSize, uint32 expectedSize,
		Common::Array<byte> &data) {
	if (sourceSize < 4)
		return false;

	const byte mode = source[0];
	const uint32 declaredSize = ((uint32)source[1] << 16) | ((uint32)source[2] << 8) | source[3];
	if (declaredSize != expectedSize || declaredSize > 32 * 1024 * 1024)
		return false;

	data.clear();
	data.reserve(declaredSize);
	uint32 sourcePos = 4;
	while (data.size() < declaredSize) {
		if (sourcePos >= sourceSize)
			return false;
		const byte control = source[sourcePos++];
		if (!(control & 0x80)) {
			const uint32 count = (control & 0x7f) + 1;
			if (sourcePos + count > sourceSize || data.size() + count > declaredSize)
				return false;
			for (uint32 i = 0; i < count; ++i)
				data.push_back(source[sourcePos++]);
			continue;
		}

		uint32 distance;
		uint32 count;
		if (mode == 1) {
			if (sourcePos >= sourceSize)
				return false;
			distance = (((uint32)control & 7) << 8) | source[sourcePos++];
			distance++;
			count = ((control & 0x7f) >> 3) + 3;
		} else {
			if (sourcePos + 2 > sourceSize)
				return false;
			distance = READ_BE_UINT16(source + sourcePos) + 1;
			sourcePos += 2;
			count = (control & 0x7f) + 4;
		}
		if (!distance || distance > data.size() || data.size() + count > declaredSize)
			return false;
		for (uint32 i = 0; i < count; ++i)
			data.push_back(data[data.size() - distance]);
	}
	return true;
}

bool RA2PSXArchive::unpack(const Entry &entry, Common::Array<byte> &data) const {
	if (!entry.unpackedSize) {
		data.clear();
		return true;
	}
	return decompress(_data.data() + entry.offset, entry.endOffset - entry.offset,
			entry.unpackedSize, data);
}

bool RA2PSXArchive::findNestedMember(const Common::Array<byte> &container,
		const Common::String &path, Common::Array<byte> &data) const {
	uint32 directory = 0;
	uint32 boundary = container.size();
	uint32 componentStart = 0;

	while (componentStart < path.size()) {
		uint32 slash = path.findFirstOf('/', componentStart);
		if (slash == Common::String::npos)
			slash = path.size();
		const Common::String component = path.substr(componentStart, slash - componentStart);
		if (component.empty())
			return false;

		bool found = false;
		uint32 selectedOffset = 0;
		uint32 selectedEnd = boundary;
		for (uint32 record = directory; record + 16 <= boundary; record += 16) {
			if (container[record] == 0)
				break;
			Common::String entryName((const char *)container.data() + record, 12);
			while (!entryName.empty() && entryName.lastChar() == 0)
				entryName.deleteLastChar();
			const uint32 relativeOffset = READ_LE_UINT32(container.data() + record + 12);
			const uint32 absoluteOffset = directory + relativeOffset;
			if (absoluteOffset < directory || absoluteOffset > boundary)
				return false;
			if (entryName.equalsIgnoreCase(component)) {
				found = true;
				selectedOffset = absoluteOffset;
			}
		}
		if (!found)
			return false;

		for (uint32 record = directory; record + 16 <= boundary; record += 16) {
			if (container[record] == 0)
				break;
			const uint32 absoluteOffset = directory + READ_LE_UINT32(container.data() + record + 12);
			if (absoluteOffset > selectedOffset && absoluteOffset < selectedEnd)
				selectedEnd = absoluteOffset;
		}

		if (slash == path.size()) {
			// Named offsets share the rest of their parent block.
			if (boundary < selectedOffset)
				return false;
			data.resize(boundary - selectedOffset);
			if (!data.empty())
				memcpy(data.data(), container.data() + selectedOffset, data.size());
			return true;
		}

		directory = selectedOffset;
		boundary = selectedEnd;
		componentStart = slash + 1;
	}
	return false;
}

bool RA2PSXArchive::getMember(const Common::String &path, Common::Array<byte> &data) const {
	uint32 slash = path.findFirstOf('/');
	const Common::String topName = slash == Common::String::npos ? path : path.substr(0, slash);
	Entry entry;
	if (!findTopLevelEntry(topName, entry))
		return false;

	Common::Array<byte> container;
	if (!unpack(entry, container))
		return false;
	if (slash == Common::String::npos) {
		data = container;
		return true;
	}
	return findNestedMember(container, path.substr(slash + 1), data);
}

} // End of namespace Scumm
