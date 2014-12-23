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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/archive.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "xeen/xeen.h"
#include "xeen/resources.h"

namespace Xeen {

/**
 * Xeen CC file implementation
 */
class CCArchive : public Common::Archive {
private:
	/**
	 * Details of a single entry in a CC file index
	 */
	struct CCEntry {
		uint16 _id;
		uint32 _offset;
		uint16 _size;

		CCEntry() : _id(0), _offset(0), _size(0) {}
		CCEntry(uint16 id, uint32 offset, uint32 size)
			: _id(id), _offset(offset), _size(size) {
		}
	};

	Common::Array<CCEntry> _index;
	Common::String _filename;

	uint16 convertNameToId(const Common::String &resourceName) const;

	void loadIndex(Common::SeekableReadStream *stream);

	bool getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const;
public:
	CCArchive(const Common::String &filename);
	virtual ~CCArchive();

	// Archive implementation
	virtual bool hasFile(const Common::String &name) const;
	virtual int listMembers(Common::ArchiveMemberList &list) const;
	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;
};

CCArchive::CCArchive(const Common::String &filename): _filename(filename) {
	File f(filename);
	loadIndex(&f);
}

CCArchive::~CCArchive() {
}

// Archive implementation
bool CCArchive::hasFile(const Common::String &name) const {
	CCEntry ccEntry;
	return getHeaderEntry(name, ccEntry);
}

int CCArchive::listMembers(Common::ArchiveMemberList &list) const {
	// CC files don't maintain the original filenames, so we can't list it
	return 0;
}

const Common::ArchiveMemberPtr CCArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *CCArchive::createReadStreamForMember(const Common::String &name) const {
	CCEntry ccEntry;

	if (getHeaderEntry(name, ccEntry)) {
		// Open the correct CC file
		Common::File f;
		if (!f.open(_filename))
			error("Could not open CC file");

		// Read in the data for the specific resource
		f.seek(ccEntry._offset);
		byte *data = new byte[ccEntry._size];
		f.read(data, ccEntry._size);

		// Decrypt the data
		for (int i = 0; i < ccEntry._size; ++i)
			data[i] ^= 0x35;

		// Return the data as a stream
		return new Common::MemoryReadStream(data, ccEntry._size, DisposeAfterUse::YES);
	}

	return nullptr;
}

/**
 * Hash a given filename to produce the Id that represents it
 */
uint16 CCArchive::convertNameToId(const Common::String &resourceName) const {
	if (resourceName.empty())
		return 0xffff;

	Common::String name = resourceName;
	name.toUppercase();

	const byte *msgP = (const byte *)name.c_str();
	int total = *msgP++;
	for (; *msgP; total += *msgP++) {
		// Rotate the bits in 'total' right 7 places
		total = (total & 0x007F) << 9 | (total & 0xFF80) >> 7;
	}

	return total;
}

/**
 * Load the index of a given CC file
 */
void CCArchive::loadIndex(Common::SeekableReadStream *stream) {
	int count = stream->readUint16LE();

	// Read in the data for the archive's index
	byte *rawIndex = new byte[count * 8];
	stream->read(rawIndex, count * 8);

	// Decrypt the index
	int ah = 0xac;
	for (int i = 0; i < count * 8; ++i) {
		rawIndex[i] = (byte)(((rawIndex[i] << 2 | rawIndex[i] >> 6) + ah) & 0xff);
		ah += 0x67;
	}

	// Extract the index data into entry structures
	_index.reserve(count);
	const byte *entryP = &rawIndex[0];
	for (int i = 0; i < count; ++i, entryP += 8) {
		CCEntry entry;
		entry._id = READ_LE_UINT16(entryP);
		entry._offset = READ_LE_UINT32(entryP + 2) & 0xffffff;
		entry._size = READ_LE_UINT16(entryP + 5);
		assert(!entryP[7]);

		_index.push_back(entry);
	}

	delete[] rawIndex;
}

/**
* Given a resource name, returns whether an entry exists, and returns
* the header index data for that entry
*/
bool CCArchive::getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const {
	uint16 id = convertNameToId(resourceName);

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

/*------------------------------------------------------------------------*/

void Resources::init(XeenEngine *vm) {
	Common::File f;

	const char *ARCHIVES[3] = { "xeen", "dark", "intro" };
	for (int i = 0; i < 3; ++i) {
		Common::String filename = Common::String::format("%s.cc", ARCHIVES[i]);
		if (f.exists(filename)) {
			SearchMan.add(ARCHIVES[i], new CCArchive(filename));
		}
	}
}

/*------------------------------------------------------------------------*/

void File::openFile(const Common::String &filename) {
	if (!Common::File::open(filename))
		error("Could not open file - %s", filename.c_str());
}

} // End of namespace Xeen
