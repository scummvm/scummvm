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

#include "common/debug.h"
#include "common/memstream.h"

namespace Dune {

Archive::Archive()
	: _count(0)
{}

bool Archive::openArchive(const Common::String &archivePath) {
	if (!_archive.open(Common::Path(archivePath))) {
		return false;
	}

	int count   = _archive.readUint16LE();

	_members.reserve(count);

	byte name[16];

	for (;;) {
		_archive.read(name, 16);
		if (name[0] == 0) {
			break;
		}

		Member m;
		memcpy(m.name, name, 16);
		m.size   = _archive.readSint32LE();
		m.offset = _archive.readSint32LE();
		m.flag   = _archive.readByte();

		_members.push_back(m);

		// debug("%18s %8x %8x %2x", m.name, m.size, m.offset, m.flag);

		++_count;
	}

	return true;
}

Resource Archive::openMember(const Common::String &name) {
	for (int i = 0; i != _count; ++i) {
		if (strncmp(name.c_str(), (char *)_members[i].name, 16) != 0) {
			continue;
		}

		int size = _members[i].size;
		byte *buf = new byte[size];

		_archive.seek(_members[i].offset);
		_archive.read(buf, size);

		if (size >= 6) {
			byte checksum = buf[0] + buf[1] + buf[3] + buf[4] + buf[5];
			if (buf[2] == 0 && checksum == 0xab) {
				Common::MemoryReadStream r(buf, size, DisposeAfterUse::YES);

				if (false) {
					Common::DumpFile f;
					f.open(Common::Path(name + ".packed"));

					int size = r.size();
					byte *buf = new byte[size];
					r.read(buf, size);

					f.write(buf, size);
					r.seek(0);
				}


				int unpackedLength = r.readUint16LE();
				r.readByte();
				int packedLength = r.readUint16LE();
				r.readByte();

				assert(size == packedLength);

				byte *unpackedBuf = new byte[unpackedLength];
				decompressHSQ(&r, packedLength, unpackedBuf, unpackedLength);

				return Resource(new Common::MemoryReadStream(unpackedBuf, unpackedLength, DisposeAfterUse::YES));
			}
		}

		return Resource(new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES));
	}

	return nullptr;
}

} // End of namespace Dune
