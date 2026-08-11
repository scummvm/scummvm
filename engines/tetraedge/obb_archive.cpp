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

#include "common/endian.h"
#include "common/debug.h"
#include "common/str.h"
#include "common/file.h"
#include "common/util.h"
#include "common/substream.h"
#include "common/compression/dcl.h"
#include "tetraedge/obb_archive.h"

namespace Tetraedge {

bool ObbArchive::readFileMap(Common::SeekableReadStream &indexFile, FileMap &files) {
	byte ver = indexFile.readByte();
	if (ver != 1) {
		warning("Unsupported tetraedge OBB version %d", ver);
		return false;
	}

	uint32 filecnt = indexFile.readUint32LE();

	for (uint i = 0; i < filecnt; i++) {
		uint32 namelen = indexFile.readUint32LE();

		if (namelen > 0x8000) {
			warning("ObbArchive::readFileMap: Name too long: 0x%x",
				namelen);
			return false;
		}

		char *buf = (char *)malloc(namelen + 1);
		assert(buf);
		indexFile.read(buf, namelen);
		buf[namelen] = 0;
		uint32 offset = indexFile.readUint32LE();
		uint32 sz = indexFile.readUint32LE();
		if (offset > indexFile.size() || sz > indexFile.size()
		    || offset + sz > indexFile.size()) {
			warning("ObbArchive::readFileMap: File outside of container: 0x%x+0x%x > 0x%llx",
				offset, sz, (unsigned long long) indexFile.size());
			return false;
		}
		files[buf] = FileDescriptor(offset, sz);

		free(buf);
	}

	return true;
}

ObbArchive* ObbArchive::open(const Common::Path& obbName) {
	Common::File indexFile;
	FileMap files;

	if (!indexFile.open(obbName))
		return nullptr;

	if (!readFileMap(indexFile, files))
		return nullptr;

	return new ObbArchive(files, obbName);
}

bool ObbArchive::hasFile(const Common::Path &path) const {
	return _files.contains(path.toString('/'));
}

int ObbArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (FileMap::const_iterator i = _files.begin(), end = _files.end(); i != end; ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, *this)));
	}

	return _files.size();
}

const Common::ArchiveMemberPtr ObbArchive::getMember(const Common::Path &path) const {
	FileMap::const_iterator i = _files.find(path.toString('/'));
	if (i == _files.end())
		return nullptr;

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(i->_key, *this));
}

Common::SeekableReadStream *ObbArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::String translated = path.toString('/');
	if (!_files.contains(translated))
		return nullptr;
	FileDescriptor desc = _files.getVal(translated);

	Common::File *f = new Common::File();
	if (!f->open(_obbName)) {
		delete f;
		return nullptr;
	}

	return new Common::SeekableSubReadStream(f, desc._fileOffset, desc._fileOffset + desc._fileSize, DisposeAfterUse::YES);
}
}
