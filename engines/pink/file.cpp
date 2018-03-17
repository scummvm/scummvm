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

#include <common/str.h>
#include "objects/page.h"
#include "pink.h"

namespace Pink {

OrbFile::OrbFile()
    : File(), _timestamp(0),
      _tableOffset(0),
      _tableSize(0),
      _table(nullptr)
{
    debug("Object Description size: %u", sizeof(ObjectDescription));
    debug("Resource Description size: %u", sizeof(ResourceDescription));
    debug("OrbFile size: %lu", sizeof(OrbFile));
    debug("BroFile size: %lu", sizeof(BroFile));
}

OrbFile::~OrbFile() {
    delete[] _table;
}

bool OrbFile::open(const Common::String &name) {
    if (!File::open(name))
        return false;

    if (readUint32BE() != 'ORB\0'){
        close();
        return false;
    }

    uint16 minor = readUint16LE();
    uint16 major = readUint16LE();

    debug("Orb v%hu.%hu loaded", major, minor);

    if (minor || major != 2){
        return false;
    }

    _timestamp = readUint32LE();
    if (!_timestamp){
        return false;
    }

    _tableOffset = readUint32LE();
    _tableSize = readUint32LE();
    _table = new ObjectDescription[_tableSize];

    debug("Orb has %u object descriptions", _tableSize);

    seek(_tableOffset);

    for (size_t i = 0; i < _tableSize; ++i) {
        _table[i].load(*this);
        debug("Object description %s loaded", _table[i].name);
    }

    return true;
}

void OrbFile::loadGame(PinkEngine *game) {
    seekToObject("PinkGame");

    Archive archive(*this);
    archive.mapObject((Object *) game); // hack

    game->load(archive);
}

void OrbFile::loadObject(Object *obj, const Common::String &name) {
    seekToObject(name.c_str());
    Archive archive(*this);
    obj->load(archive);
}

void OrbFile::loadObject(Object *obj, ObjectDescription *objDesc) {
    seek(objDesc->objectsOffset);
    Archive archive(*this);
    obj->load(archive);
}


uint32 OrbFile::getTimestamp() {
    return _timestamp;
}

void OrbFile::seekToObject(const char *name) {
    ObjectDescription *desc = getObjDesc(name);
    seek(desc->objectsOffset);
}


ObjectDescription *OrbFile::getObjDesc(const char *name){
    ObjectDescription *desc = static_cast<ObjectDescription*>(bsearch(name, _table, _tableSize, sizeof(ObjectDescription),
                                                                      [] (const void *a, const void *b) {
                                                                          return scumm_stricmp((char *) a, (char *) b); }));
    assert(desc != nullptr);
    return desc;
}

ResourceDescription *OrbFile::getResDescTable(ObjectDescription *objDesc){
    const uint32 size = objDesc->objectsCount;
    ResourceDescription *table = new ResourceDescription[size];

    for (uint i = 0; i < size; ++i) {
        table[i].load(*this);
    }

    return table;
}


bool BroFile::open(Common::String &name, uint32 orbTimestamp) {
    if (!File::open(name) || readUint32BE() != 'BRO\0')
        return false;

    uint16 minor = readUint16LE();
    uint16 major = readUint16LE();

    debug("Bro v%hu.%hu loaded", major, minor);

    if (minor || major != 1){
        return false;
    }

    uint32 _timestamp = readUint32LE();

    return _timestamp == orbTimestamp;
}

void ObjectDescription::load(Common::File &file) {
    file.read(name, sizeof(name));
    file.read(&objectsOffset, sizeof(objectsOffset));
    file.read(&objectsCount, sizeof(objectsCount));
    file.read(&resourcesOffset, sizeof(resourcesOffset));
    file.read(&resourcesCount, sizeof(resourcesCount));
}

void ResourceDescription::load(Common::File &file) {
    file.read(name, sizeof(name));
    file.read(&offset, sizeof(offset));
    file.read(&size, sizeof(offset));

    uint16 temp;
    file.read(&temp, sizeof(temp));
    InBro = temp ? true : false;
}

} // End of namespace Pink