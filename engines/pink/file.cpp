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
#include "pink.h"

namespace Pink {

OrbFile::OrbFile()
    : File(), _timestamp(0),
      _tableOffset(0),
      _tableSize(0),
      _table(nullptr)
{}

OrbFile::~OrbFile() {
    delete[] _table;
}

bool OrbFile::open(Common::String &name) {
    if (!File::open(name))
        return false;

    if (readUint32BE() != 'ORB\0'){
        close();
        return false;
    }

    uint16 minor = readUint16LE();
    uint16 major = readUint16LE();
    //output
    if (minor || major != 2){
        return false;
    }

    _timestamp = readUint32LE();
    if (!_timestamp){
        return false;
    }
    //convert to date
    //output into debug

    _tableOffset = readUint32LE();
    _tableSize = readUint32LE();
    _table = new ObjectDescription[_tableSize];

    for (size_t i = 0; i < _tableSize; ++i) {
        _table[i].deserialize(*this);
    }

    return true;
}

void OrbFile::LoadGame(PinkEngine *game) {

}

void OrbFile::LoadObject(void *obj, Common::String &name) {

}

uint32 OrbFile::getTimestamp() {
    return _timestamp;
}

bool BroFile::open(Common::String &name, uint32 orbTimestamp) {
    if (!File::open(name) || readUint32BE() != 'BRO\0')
        return false;

    uint16 minor = readUint16LE();
    uint16 major = readUint16LE();
    // do output

    if (minor || major != 1){
        return false;
    }

    uint32 _timestamp = readUint32LE();

    return _timestamp == orbTimestamp;
}

void ObjectDescription::deserialize(Common::File &file) {
    file.read(name, sizeof(name));
    file.read(&objectsOffset, sizeof(objectsOffset));
    file.read(&objectsCount, sizeof(objectsCount));
    file.read(&resourcesOffset, sizeof(resourcesOffset));
    file.read(&resourcesCount, sizeof(resourcesCount));
}

void ResourseDescription::deserialize(Common::File &file) {
    file.read(name, sizeof(name));
    file.read(&offset, sizeof(offset));
    file.read(&size, sizeof(offset));

    uint16 temp;
    file.read(&temp, sizeof(temp));
    InBro = temp ? true : false;
}

} // End of namespace Pink