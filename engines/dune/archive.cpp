/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "dune/archive.h"
#include "dune/hsq.h"

#include "common/endian.h"
#include "common/memstream.h"

namespace Dune {

bool Archive::openArchive(const Common::String &archivePath) {
	_members.clear();
	_archive.close();
	if (!_archive.open(Common::Path(archivePath)))
		return false;

	const int64 archiveSize = _archive.size();
	int64 directoryEnd = archiveSize;
	// The header count is a hint. The first empty name terminates the directory.
	if (_archive.seek(2)) {
		while (_archive.pos() < directoryEnd) {
			byte entry[25];
			if (_archive.read(entry, 1) != 1)
				break;
			if (entry[0] == 0)
				return true;

			if (directoryEnd - _archive.pos() < 24 || _archive.read(entry + 1, 24) != 24)
				break;
			Member member;
			memcpy(member.name, entry, sizeof(member.name));
			member.size = READ_LE_INT32(entry + 16);
			member.offset = READ_LE_INT32(entry + 20);
			member.flag = entry[24];
			if (member.size < 0 || member.offset < _archive.pos() ||
					(int64)member.offset + member.size > archiveSize)
				break;
			if (member.offset < directoryEnd)
				directoryEnd = member.offset;
			_members.push_back(member);
		}
	}

	_members.clear();
	_archive.close();
	return false;
}

Resource Archive::openMember(const Common::String &name) {
	for (const Member &member : _members) {
		if (strncmp(name.c_str(), (const char *)member.name, sizeof(member.name)) != 0)
			continue;

		const int size = member.size;
		byte *buf = (byte *)malloc(size ? size : 1);
		if (!buf)
			return nullptr;

		if (!_archive.seek(member.offset) || _archive.read(buf, size) != (uint32)size) {
			free(buf);
			return nullptr;
		}

		if (size >= 6) {
			byte checksum = buf[0] + buf[1] + buf[3] + buf[4] + buf[5];
			if (buf[2] == 0 && checksum == 0xab) {
				Common::MemoryReadStream r(buf, size, DisposeAfterUse::YES);

				int unpackedLength = r.readUint16LE();
				r.readByte();
				int packedLength = r.readUint16LE();
				r.readByte();

				if (size != packedLength)
					return nullptr;

				byte *unpackedBuf = (byte *)malloc(unpackedLength ? unpackedLength : 1);
				if (!unpackedBuf)
					return nullptr;
				decompressHSQ(&r, packedLength, unpackedBuf, unpackedLength);

				return Resource(new Common::MemoryReadStream(unpackedBuf, unpackedLength, DisposeAfterUse::YES));
			}
		}

		return Resource(new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES));
	}

	return nullptr;
}

} // End of namespace Dune
