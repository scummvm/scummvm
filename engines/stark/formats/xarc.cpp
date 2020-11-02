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

// Based on the Xentax Wiki documentation:
// http://wiki.xentax.com/index.php?title=The_Longest_Journey_XARC

#include "engines/stark/formats/xarc.h"
#include "engines/stark/debug.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"

namespace Stark {
namespace Formats {

// ARCHIVE MEMBER

class XARCMember : public Common::ArchiveMember {
public:
	XARCMember(const XARCArchive *xarc, Common::ReadStream &stream, uint32 offset);

	Common::SeekableReadStream *createReadStream() const;
	Common::String getName() const { return _name; }
	uint32 getLength() const { return _length; }
	uint32 getOffset() const { return _offset; }

private:
	const XARCArchive *_xarc;
	Common::String _name;
	uint32 _offset;
	uint32 _length;

	// Helper function
	Common::String readString(Common::ReadStream &stream);
};

XARCMember::XARCMember(const XARCArchive *xarc, Common::ReadStream &stream, uint32 offset) {
	_xarc = xarc;

	// Read the information about this archive member
	_name = readString(stream);
	_offset = offset;
	_length = stream.readUint32LE();
	debugC(20, kDebugArchive, "Stark::XARC Member: \"%s\" starts at offset=%d and has length=%d", _name.c_str(), _offset, _length);

	// Unknown value. English: 0, others: 1
	uint32 unknown = stream.readUint32LE();
	debugC(kDebugUnknown, "Stark::XARC Member: \"%s\" has unknown=%d", _name.c_str(), unknown);
	if (unknown != 0 && unknown != 1) {
		warning("Stark::XARC Member: \"%s\" has unknown=%d with unknown meaning", _name.c_str(), unknown);
	}
}

Common::SeekableReadStream *XARCMember::createReadStream() const {
	return _xarc->createReadStreamForMember(this);
}

Common::String XARCMember::readString(Common::ReadStream &stream) {
	Common::String str;

	// Read until we find a 0
	char c = 1;
	while (c != 0) {
		c = stream.readByte();
		if (stream.eos()) {
			c = 0;
		}
		if (c != 0) {
			str += c;
		}
	}

	return str;
}


// ARCHIVE

bool XARCArchive::open(const Common::String &filename) {
	Common::File stream;
	if (!stream.open(filename)) {
		return false;
	}

	_filename = filename;

	// Unknown: always 1? version?
	uint32 unknown = stream.readUint32LE();
	debugC(kDebugUnknown, "Stark::XARC: \"%s\" has unknown=%d", _filename.c_str(), unknown);
	if (unknown != 1) {
		warning("Stark::XARC: \"%s\" has unknown=%d with unknown meaning", _filename.c_str(), unknown);
	}

	// Read the number of contained files
	uint32 numFiles = stream.readUint32LE();
	debugC(20, kDebugArchive, "Stark::XARC: \"%s\" contains %d files", _filename.c_str(), numFiles);

	// Read the offset to the contents of the first file
	uint32 offset = stream.readUint32LE();
	debugC(20, kDebugArchive, "Stark::XARC: \"%s\"'s first file has offset=%d", _filename.c_str(), offset);

	for (uint32 i = 0; i < numFiles; i++) {
		XARCMember *member = new XARCMember(this, stream, offset);
		_members.push_back(Common::ArchiveMemberPtr(member));

		// Set the offset to the next member
		offset += member->getLength();
	}

	return true;
}

Common::String XARCArchive::getFilename() const {
	return _filename;
}

bool XARCArchive::hasFile(const Common::String &name) const {
	for (Common::ArchiveMemberList::const_iterator it = _members.begin(); it != _members.end(); ++it) {
		if ((*it)->getName() == name) {
			// Found it
			return true;
		}
	}

	// Not found
	return false;
}

int XARCArchive::listMatchingMembers(Common::ArchiveMemberList &list, const Common::String &pattern) const {
	int matches = 0;
	for (Common::ArchiveMemberList::const_iterator it = _members.begin(); it != _members.end(); ++it) {
		if ((*it)->getName().matchString(pattern)) {
			// This file matches, add it
			list.push_back(*it);
			matches++;
		}
	}

	return matches;
}

int XARCArchive::listMembers(Common::ArchiveMemberList &list) const {
	int files = 0;
	for (Common::ArchiveMemberList::const_iterator it = _members.begin(); it != _members.end(); ++it) {
		// Add all the members to the list
		list.push_back(*it);
		files++;
	}

	return files;
}

const Common::ArchiveMemberPtr XARCArchive::getMember(const Common::String &name) const {
	for (Common::ArchiveMemberList::const_iterator it = _members.begin(); it != _members.end(); ++it) {
		if ((*it)->getName() == name) {
			// Found it
			return *it;
		}
	}

	// Not found, return an empty ptr
	return Common::ArchiveMemberPtr();
}

Common::SeekableReadStream *XARCArchive::createReadStreamForMember(const Common::String &name) const {
	for (Common::ArchiveMemberList::const_iterator it = _members.begin(); it != _members.end(); ++it) {
		if ((*it)->getName() == name) {
			// Found it
			return createReadStreamForMember((const XARCMember *)it->get());
		}
	}

	// Not found
	return 0;
}

Common::SeekableReadStream *XARCArchive::createReadStreamForMember(const XARCMember *member) const {
	// Open the xarc file
	Common::File *f = new Common::File;
	if (!f)
		return NULL;

	if (!f->open(_filename)) {
		delete f;
		return NULL;
	}

	// Return the substream that contains the archive member
	uint32 offset = member->getOffset();
	uint32 length = member->getLength();
	return new Common::SeekableSubReadStream(f, offset, offset + length, DisposeAfterUse::YES);

	// Different approach: keep the archive open and read full resources to memory
	//f.seek(member->getOffset());
	//return f.readStream(member->getLength());
}

} // End of namespace Formats
} // End of namespace Stark
