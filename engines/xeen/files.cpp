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
#include "xeen/files.h"
#include "xeen/saves.h"

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

void BaseCCArchive::loadIndex(Common::SeekableReadStream *stream) {
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

bool BaseCCArchive::hasFile(const Common::String &name) const {
	CCEntry ccEntry;
	return getHeaderEntry(name, ccEntry);
}

bool BaseCCArchive::getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const {
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

const Common::ArchiveMemberPtr BaseCCArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

int BaseCCArchive::listMembers(Common::ArchiveMemberList &list) const {
	// CC files don't maintain the original filenames, so we can't list it
	return 0;
}

/*------------------------------------------------------------------------*/

CCArchive::CCArchive(const Common::String &filename, bool encoded):
		BaseCCArchive(), _filename(filename), _encoded(encoded) {
	File f(filename, SearchMan);
	loadIndex(&f);
}

CCArchive::CCArchive(const Common::String &filename, const Common::String &prefix,
		bool encoded): BaseCCArchive(), _filename(filename),
		_prefix(prefix), _encoded(encoded) {
	_prefix.toLowercase();
	File f(filename, SearchMan);
	loadIndex(&f);
}

CCArchive::~CCArchive() {
}

bool CCArchive::getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const {
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

Common::SeekableReadStream *CCArchive::createReadStreamForMember(const Common::String &name) const {
	CCEntry ccEntry;

	if (getHeaderEntry(name, ccEntry)) {
		// Open the correct CC file
		Common::File f;
		if (!f.open(_filename))
			error("Could not open CC file");

		// Read in the data for the specific resource
		f.seek(ccEntry._offset);
		byte *data = (byte *)malloc(ccEntry._size);
		f.read(data, ccEntry._size);

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

/*------------------------------------------------------------------------*/

FileManager::FileManager(XeenEngine *vm) {
	Common::File f;
	int sideNum = 0;

	_isDarkCc = vm->getGameID() == GType_DarkSide;
	
	File::_xeenCc = (vm->getGameID() == GType_DarkSide) ? nullptr :
		new CCArchive("xeen.cc", "xeen", true);
	File::_darkCc = (vm->getGameID() == GType_Clouds) ? nullptr :
		new CCArchive("dark.cc", "dark", true);
	if (Common::File::exists("intro.cc")) {
		CCArchive *introCc = new CCArchive("intro.cc", "intro", true);
		SearchMan.add("intro", introCc);
	}

	File::_currentArchive = vm->getGameID() == GType_DarkSide ?
		File::_darkCc : File::_xeenCc;
	assert(File::_currentArchive);
}

FileManager::~FileManager() {
	SearchMan.remove("intro");
	delete File::_xeenCc;
	delete File::_darkCc;
}

void FileManager::setGameCc(int ccMode) {
	if (g_vm->getGameID() != GType_WorldOfXeen)
		ccMode = 1;

	File::setCurrentArchive(ccMode);
	_isDarkCc = ccMode != 0;
}

/*------------------------------------------------------------------------*/

CCArchive *File::_currentArchive;
CCArchive *File::_xeenCc;
CCArchive *File::_darkCc;

File::File(const Common::String &filename) {
	File::open(filename);
}

File::File(const Common::String &filename, Common::Archive &archive) {
	File::open(filename, archive);
}

File::File(const Common::String &filename, int ccMode) {
	File::open(filename, ccMode);
}

bool File::open(const Common::String &filename) {
	if (!g_vm->_saves || !Common::File::open(filename, *g_vm->_saves)) {
		if (!Common::File::open(filename, *_currentArchive)) {
			// Could not find in current archive, so try intro.cc or in folder
			if (!Common::File::open(filename))
				error("Could not open file - %s", filename.c_str());
		}
	}

	return true;
}

bool File::open(const Common::String &filename, Common::Archive &archive) {
	if (!Common::File::open(filename, archive))
		error("Could not open file - %s", filename.c_str());
	return true;
}

bool File::open(const Common::String &filename, int ccMode) {
	FileManager &files = *g_vm->_files;
	int oldMode = files._isDarkCc ? 1 : 0;

	files.setGameCc(ccMode);
	File::open(filename);
	files.setGameCc(oldMode);

	return true;
}

void File::setCurrentArchive(int ccMode) {
	switch (ccMode) {
	case 0:
		_currentArchive = _xeenCc;
		break;

	case 1:
		_currentArchive = _darkCc;
		break;

	default:
		break;
	}

	assert(_currentArchive);
}

Common::String File::readString() {
	Common::String result;
	char c;

	while (pos() < size() && (c = (char)readByte()) != '\0')
		result += c;

	return result;
}

bool File::exists(const Common::String &filename) {
	if (!g_vm->_saves || !g_vm->_saves->hasFile(filename)) {
		if (!_currentArchive->hasFile(filename)) {
			// Could not find in current archive, so try intro.cc or in folder
			return Common::File::exists(filename);
		}
	}

	return true;
}

bool File::exists(const Common::String &filename, int ccMode) {
	FileManager &files = *g_vm->_files;
	int oldMode = files._isDarkCc ? 1 : 0;

	files.setGameCc(ccMode);
	bool result = exists(filename);
	files.setGameCc(oldMode);

	return result;
}

/*------------------------------------------------------------------------*/

void StringArray::load(const Common::String &name) {
	File f(name);
	clear();
	while (f.pos() < f.size())
		push_back(f.readString());
}

void StringArray::load(const Common::String &name, int ccMode) {
	File f(name, ccMode);
	clear();
	while (f.pos() < f.size())
		push_back(f.readString());
}

} // End of namespace Xeen
