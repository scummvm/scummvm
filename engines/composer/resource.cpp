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

#include "composer/resource.h"

#include "common/debug.h"
#include "common/substream.h"
#include "common/util.h"
#include "common/textconsole.h"

namespace Composer {

// Base Archive code
// (copied from clone2727's mohawk code)

Archive::Archive() {
	_stream = 0;
}

Archive::~Archive() {
	close();
}

bool Archive::openFile(const Common::String &fileName) {
	Common::File *file = new Common::File();

	if (!file->open(fileName)) {
		delete file;
		return false;
	}

	if (!openStream(file)) {
		close();
		return false;
	}

	return true;
}

void Archive::close() {
	_types.clear();
	delete _stream; _stream = 0;
}

bool Archive::hasResource(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		return false;

	return _types[tag].contains(id);
}

bool Archive::hasResource(uint32 tag, const Common::String &resName) const {
	if (!_types.contains(tag) || resName.empty())
		return false;

	const ResourceMap &resMap = _types[tag];

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++)
		if (it->_value.name.matchString(resName))
			return true;

	return false;
}

Common::SeekableReadStream *Archive::getResource(uint32 tag, uint16 id) {
	if (!_types.contains(tag))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const Resource &res = resMap[id];

	return new Common::SeekableSubReadStream(_stream, res.offset, res.offset + res.size);
}

uint32 Archive::getResourceFlags(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const Resource &res = resMap[id];

	return res.flags;
}

uint32 Archive::getOffset(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	return resMap[id].offset;
}

uint16 Archive::findResourceID(uint32 tag, const Common::String &resName) const {
	if (!_types.contains(tag) || resName.empty())
		return 0xFFFF;

	const ResourceMap &resMap = _types[tag];

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++)
		if (it->_value.name.matchString(resName))
			return it->_key;

	return 0xFFFF;
}

Common::String Archive::getName(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	return resMap[id].name;
}

Common::Array<uint32> Archive::getResourceTypeList() const {
	Common::Array<uint32> typeList;

	for (TypeMap::const_iterator it = _types.begin(); it != _types.end(); it++)
		typeList.push_back(it->_key);

	return typeList;
}

Common::Array<uint16> Archive::getResourceIDList(uint32 type) const {
	Common::Array<uint16> idList;

	if (!_types.contains(type))
		return idList;

	const ResourceMap &resMap = _types[type];

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++)
		idList.push_back(it->_key);

	return idList;
}

// Composer Archive code

bool ComposerArchive::openStream(Common::SeekableReadStream *stream) {
	// Make sure no other file is open...
	close();

	bool newStyle = false;
	uint32 headerSize = stream->readUint32LE();
	if (headerSize == SWAP_CONSTANT_32(ID_LBRC)) {
		// new-style file
		newStyle = true;
		headerSize = stream->readUint32LE();
		uint32 zeros = stream->readUint32LE();
		if (zeros != 0)
			error("invalid LBRC header (%d instead of zeros)", zeros);
	}

	uint16 numResourceTypes = stream->readUint16LE();
	if (newStyle) {
		uint16 unknown = stream->readUint16LE();
		debug(4, "skipping unknown %04x", unknown);
	}

	debug(4, "Reading LBRC resource table with %d entries", numResourceTypes);
	for (uint i = 0; i < numResourceTypes; i++) {
		uint32 tag = stream->readUint32BE();
		uint32 tableOffset = stream->readUint32LE();
		debug(4, "Type '%s' at offset %d", tag2str(tag), tableOffset);
		// starting from the start of the resource table, which differs
		// according to whether we have the 10 extra bytes for newStyle
		if (newStyle)
			tableOffset += 16;
		else
			tableOffset += 6;

		ResourceMap &resMap = _types[tag];

		uint32 oldPos = stream->pos();
		stream->seek(tableOffset);

		while (true) {
			if (stream->eos())
				error("LBRC file ran out of stream");

			uint32 offset, size, id, flags;
			if (newStyle) {
				offset = stream->readUint32LE();
				if (!offset)
					break;
				size = stream->readUint32LE();
				id = stream->readUint16LE();
				flags = stream->readUint16LE(); // set to 1 for preload, otherwise no preload
				/*uint32 junk = */ stream->readUint32LE();
			} else {
				id = stream->readUint16LE();
				if (!id)
					break;
				offset = stream->readUint32LE();
				offset += headerSize;
				size = stream->readUint32LE();
				flags = stream->readUint16LE(); // FIXME

			}

			Resource &res = resMap[id];
			res.offset = offset;
			res.size = size;
			res.flags = flags;
			debug(4, "Id %d, offset %d, size %d, flags %08x", id, offset, size, flags);
		}
		
		stream->seek(oldPos);
	}

	_stream = stream;
	return true;
}

}	// End of namespace Composer
