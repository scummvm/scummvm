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

#include "engines/stark/formats/biff.h"

#include "engines/stark/services/archiveloader.h"

namespace Stark {
namespace Formats {

BiffArchive::BiffArchive(ArchiveReadStream *stream, ObjectBuilder objectBuilder) :
	_objectBuilder(objectBuilder) {
	read(stream);
}

BiffArchive::~BiffArchive() {
	for (uint i = 0; i < _rootObjects.size(); i++) {
		delete _rootObjects[i];
	}
}

void BiffArchive::read(ArchiveReadStream *stream) {
	uint32 id = stream->readUint32BE();
	if (id != MKTAG('B', 'I', 'F', 'F')) {
		error("Wrong magic while reading biff archive");
	}

	_version = stream->readUint32LE();
	/*uint32 u1 = */stream->readUint32LE();
	/*uint32 u2 = */stream->readUint32LE();

	uint32 len = stream->readUint32LE();
	for (uint32 i = 0; i < len; i++) {
		BiffObject *object = readObject(stream, nullptr);

		_rootObjects.push_back(object);
	}
}

BiffObject *BiffArchive::readObject(ArchiveReadStream *stream, BiffObject *parent) {
	uint32 marker = stream->readUint32LE();
	if (marker != 0xf0f0f0f0) {
		error("Wrong magic while reading biff archive");
	}

	uint32 type = stream->readUint32LE();
	BiffObject *object = _objectBuilder(type);
	if (!object) {
		error("Unimplemented BIFF object type %x", type);
	}

	object->_parent = parent;
	object->_u3 = stream->readUint32LE();
	uint32 size = stream->readUint32LE();

	if (_version >= 2) {
		object->_version = stream->readUint32LE();
	}

	object->readData(stream, size);

	marker = stream->readUint32LE();
	if (marker != 0x0f0f0f0f) {
		error("Wrong magic while reading biff archive");
	}

	uint32 len = stream->readUint32LE();
	for (uint32 i = 0; i < len; ++i) {
		BiffObject *child = readObject(stream, object);
		object->addChild(child);
	}

	return object;
}

Common::Array<BiffObject *> BiffArchive::listObjects() {
	return _rootObjects;
}

BiffObject::BiffObject() :
	_u3(0),
	_version(0),
	_parent(nullptr),
	_type(0) {

}

uint32 BiffObject::getType() const {
	return _type;
}

void BiffObject::addChild(BiffObject *child) {
	_children.push_back(child);
}

BiffObject::~BiffObject() {
	// Delete the children objects
	Common::Array<BiffObject *>::iterator i = _children.begin();
	while (i != _children.end()) {
		delete *i;
		i++;
	}
}

} // End of namespace Formats
} // End of namespace Stark
