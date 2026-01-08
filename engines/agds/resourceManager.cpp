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

#include "agds/resourceManager.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/path.h"
#include "common/ptr.h"
#include "graphics/surface.h"
#include "image/bmp.h"
#include "image/pcx.h"
#include "video/flic_decoder.h"

namespace AGDS {
ResourceManager::ResourceManager(int version) : _version(version) {}

ResourceManager::~ResourceManager() {}

void ResourceManager::decrypt(uint8 *data, unsigned size) {
	static const char *kKey = "Vyvojovy tym AGDS varuje: Hackerovani skodi obchodu!";
	const char *ptr = kKey;
	while (size--) {
		*data++ ^= 0xff ^ *ptr++;
		if (*ptr == 0)
			ptr = kKey;
	}
}

bool ResourceManager::GrpFile::load(const Common::Path &grpPath) {
	static const char *kSignature = "AGDS group file\x1a";
	static const uint32 kMagic = 0x1a03c9e6;
	static const uint32 kVersion1 = 44;
	static const uint32 kVersion2 = 2;

	debug("adding path %s", grpPath.toString().c_str());
	if (!_file.open(grpPath)) {
		warning("failing opening grp file %s", grpPath.toString().c_str());
		return false;
	}
	uint8 header[0x2c];
	if (_file.read(header, sizeof(header)) != sizeof(header)) {
		warning("short read from header");
		return false;
	}

	if (strncmp(reinterpret_cast<const char *>(header), kSignature, 0x10) != 0) {
		decrypt(header, 0x10);
		if (strncmp(reinterpret_cast<const char *>(header), kSignature, 0x10) != 0) {
			warning("invalid signature");
			return false;
		}
		debug("load grp file %s, encrypted", grpPath.toString().c_str());
		_encrypted = true;
	} else {
		debug("load grp file %s, unencrypted", grpPath.toString().c_str());
		_encrypted = false;
	}

	Common::MemoryReadStreamEndian reader(header + 0x10, sizeof(header) - 0x10, false);
	uint32 version1 = reader.readUint32();
	if (version1 != kVersion1) {
		warning("invalid version 1 (%d)", version1);
		return false;
	}

	uint32 magic = reader.readUint32();
	if (magic != kMagic) {
		warning("invalid magic (0x%08x)", magic);
		return false;
	}

	uint32 version2 = reader.readUint32();
	if (version2 != kVersion2) {
		warning("invalid version 2 (%d)", version2);
		return false;
	}

	unsigned dirCount = reader.readUint32();
	if (!reader.skip(3 * 4))
		return false;

	// debug("+%u files in index", dirCount);
	while (dirCount--) {
		uint8 dirData[0x31];
		uint8 *dirDataEnd = dirData + sizeof(dirData);

		if (_file.read(dirData, sizeof(dirData)) != sizeof(dirData)) {
			warning("short read, corrupted file");
			return false;
		}

		uint8 *nameEnd = Common::find(dirData, dirDataEnd, 0);
		if (nameEnd == dirDataEnd) {
			warning("corrupted entry at %d", (int)_file.pos() - 0x31);
			continue;
		}

		unsigned nameLength = nameEnd - dirData;
		if (_encrypted)
			decrypt(dirData, nameLength);
		Common::String name(reinterpret_cast<char *>(dirData), nameLength);

		Common::MemoryReadStreamEndian dirReader(dirData + 0x21, 8, false);

		uint32 offset = dirReader.readSint32();
		uint32 size = dirReader.readSint32();
		// debug("\t\tfile %s %u %u", name.c_str(), offset, size);
		ArchiveMemberPtr resource(new ArchiveMember(this, name, offset, size));
		_members.setVal(name, resource);
	}

	debug("%s: %u files in index", grpPath.toString().c_str(), _members.size());
	return true;
}

int ResourceManager::GrpFile::listMembers(Common::ArchiveMemberList &list) const {
	int size = 0;
	for (MembersType::const_iterator i = _members.begin(); i != _members.end(); ++i, ++size)
		list.push_back(i->_value);
	return size;
}

bool ResourceManager::GrpFile::hasFile(const Common::Path &name) const {
	return _members.find(name.toString()) != _members.end();
}

const Common::ArchiveMemberPtr ResourceManager::GrpFile::getMember(const Common::Path &name) const {
	Common::ArchiveMemberPtr member;
	MembersType::const_iterator i = _members.find(name.toString());
	if (i != _members.end())
		member = i->_value;
	return member;
}

Common::SeekableReadStream *ResourceManager::GrpFile::createReadStreamForMember(const Common::Path &name) const {
	Common::ArchiveMemberPtr member = getMember(name);
	return member ? member->createReadStream() : NULL;
}

bool ResourceManager::addPath(const Common::Path &grpFilename) {
	Common::ScopedPtr<GrpFile> grpFile(new GrpFile());
	if (!grpFile->load(grpFilename)) {
		return false;
	}

	SearchMan.add(grpFilename.toString(), grpFile.release(), 0, true);
	return true;
}

Common::SeekableReadStream *ResourceManager::ArchiveMember::createReadStream() const {
	Common::SeekableReadStream &file = _parent->getArchiveStream();
	file.seek(_offset);
	return file.readStream(_size);
}

Common::SeekableReadStream *ResourceManager::getResource(const Common::String &name) const {
	Common::File file;
	return (file.open(Common::Path{name})) ? file.readStream(file.size()) : NULL;
}

bool ResourceManager::IsBMP(Common::SeekableReadStream &stream) {
	auto b0 = stream.readByte();
	auto b1 = stream.readByte();
	stream.seek(-2, SEEK_CUR);
	return b0 == 'B' && b1 == 'M';
}

Graphics::Surface *ResourceManager::loadPicture(const Common::String &name, const Graphics::PixelFormat &format) {
	Common::SeekableReadStream *stream = getResource(name);
	if (!stream)
		return NULL;

	auto is_bmp = IsBMP(*stream);

	Common::String lname = name;
	lname.toLowercase();

	if (lname.hasSuffix(".bmp") || is_bmp) {
		Image::BitmapDecoder bmp;
		return bmp.loadStream(*stream) ? bmp.getSurface()->convertTo(format) : NULL;
	} else if (lname.hasSuffix(".pcx")) {
		Image::PCXDecoder pcx;
		return pcx.loadStream(*stream) ? pcx.getSurface()->convertTo(format) : NULL;
	} else if (lname.hasSuffix(".flc")) {
		Video::FlicDecoder flic;
		if (!flic.loadStream(stream)) {
			warning("flic decoder failed to load %s", name.c_str());
			return NULL;
		}
		const Graphics::Surface *surface = flic.decodeNextFrame();
		return surface ? surface->convertTo(format, flic.getPalette()) : NULL;
	} else
		warning("unknown extensions for resource %s", name.c_str());
	return NULL;
}

Common::String ResourceManager::loadText(Common::SeekableReadStream &stream) const {
	Common::Array<char> text(stream.size());
	if (stream.read(text.data(), text.size()) != text.size())
		error("short read from text resource");

	if (text.empty())
		return Common::String();

	char *begin = text.data();
	char *end = begin + text.size();
	while (begin != end && end[-1] == 0)
		--end;

	if (_version != 0 || (text[0] < ' ' || text[0] >= 0x7f))
		decrypt(reinterpret_cast<uint8 *>(text.data()), end - begin);

	while (begin != end && end[-1] == 0)
		--end;

	return Common::String(begin, end);
}

Common::String ResourceManager::loadText(const Common::String &name) const {
	Common::ScopedPtr<Common::SeekableReadStream> stream(getResource(name));
	if (!stream)
		error("no text resource %s", name.c_str());
	return loadText(*stream);
}

Common::String readString(Common::ReadStream &stream, uint size) {
	Common::Array<char> text(size);
	if (stream.read(text.data(), text.size()) != text.size())
		error("readString: short read");

	return Common::String(text.data(), strlen(text.data()));
}

void writeString(Common::WriteStream &stream, const Common::String &string, uint size) {
	Common::Array<char> text(size);
	memcpy(text.data(), string.c_str(), MIN(string.size(), size));

	if (stream.write(text.data(), text.size()) != text.size())
		error("writeString: short write");
}

} // End of namespace AGDS
