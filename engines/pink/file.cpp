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

#include "pink/pink.h"

namespace Pink {

void ObjectDescription::load(Common::File &file) {
	file.read(name, sizeof(name));

	objectsOffset = file.readUint32LE();
	/* objectsCount = */ file.readUint32LE();
	resourcesOffset = file.readUint32LE();
	resourcesCount = file.readUint32LE();
}

void ResourceDescription::load(Common::File &file) {
	file.read(name, sizeof(name));

	offset = file.readUint32LE();
	size = file.readUint32LE();
	inBro = (bool)file.readUint16LE();
}

OrbFile::OrbFile()
	: File(), _timestamp(0),
	  _tableSize(0),
	  _table(nullptr) {}

OrbFile::~OrbFile() {
	delete[] _table;
}

bool OrbFile::open(const Common::String &name) {
	if (!File::open(name) || readUint32BE() != MKTAG('O', 'R', 'B', '\0'))
		return false;

	uint16 minor = readUint16LE();
	uint16 major = readUint16LE();

	if (major != kOrbMajorVersion || minor != kOrbMinorVersion)
		return false;

	if (!(_timestamp = readUint32LE()))
		return false;

	uint32 tableOffset = readUint32LE();

	_tableSize = readUint32LE();
	_table = new ObjectDescription[_tableSize];

	seek(tableOffset);

	for (uint i = 0; i < _tableSize; ++i) {
		_table[i].load(*this);
	}

	return true;
}

void OrbFile::loadGame(PinkEngine *game) {
	seekToObject(kPinkGame);
	Archive archive(this);
	archive.mapObject(reinterpret_cast<Object *>(game)); // hack
	game->load(archive);
}

void OrbFile::loadObject(Object *obj, const Common::String &name) {
	seekToObject(name.c_str());
	Archive archive(this);
	obj->load(archive);
}

void OrbFile::loadObject(Object *obj, ObjectDescription *objDesc) {
	seek(objDesc->objectsOffset);
	Archive archive(this);
	obj->load(archive);
}

static int objDescComp(const void *a, const void *b) {
	return scumm_stricmp((const char *)a, (const char *)b);
}

ObjectDescription *OrbFile::getObjDesc(const char *name) {
	ObjectDescription *desc = (ObjectDescription *)bsearch(name, _table, _tableSize, sizeof(ObjectDescription), objDescComp);
	assert(desc != nullptr);
	return desc;
}

ResourceDescription *OrbFile::createResDescTable(ObjectDescription *objDesc) {
	ResourceDescription *table = new ResourceDescription[objDesc->resourcesCount];
	seek(objDesc->resourcesOffset);

	for (uint i = 0; i < objDesc->resourcesCount; ++i) {
		table[i].load(*this);
	}

	return table;
}

void OrbFile::seekToObject(const char *name) {
	ObjectDescription *desc = getObjDesc(name);
	seek(desc->objectsOffset);
}

bool BroFile::open(const Common::String &name) {
	if (!File::open(name) || readUint32BE() != MKTAG('B', 'R', 'O', '\0'))
		return false;

	uint16 minor = readUint16LE();
	uint16 major = readUint16LE();

	if (major != kBroMajorVersion || minor != kBroMinorVersion)
		return false;

	_timestamp = readUint32LE();

	return true;
}

} // End of namespace Pink
