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

#ifndef PINK_ARCHIVE_H
#define PINK_ARCHIVE_H

#include <engines/pink/objects/object.h>
#include <common/str-array.h>

namespace Common {

class File;

}

namespace Pink {

class Archive {
public:
    Archive(Common::File &file);
    ~Archive();

    void mapObject(Object *obj);

    int readCount();
    uint32 readDWORD();
    uint16 readWORD();

    Object *readObject();
    Common::String readString();

private:
    uint findObjectId(const char *name);

    Object *parseObject(bool &isCopyReturned);

    Common::Array<Object *> _objectMap;
    Common::Array<uint> _objectIdMap;
    Common::File &_file;
};

template <typename T>
inline Archive &operator>>(Archive &archive, Common::Array<T> &arr){
    uint size = archive.readCount();
    arr.resize(size);
    for (uint i = 0; i < size; ++i) {
        arr[i] = reinterpret_cast<T> (archive.readObject());
    }
    return archive;
}

template <typename T>
inline Archive &operator>>(Archive &archive, Object &obj){
    obj.load(archive);
    return archive;
}

inline Archive &operator>>(Archive &archive, Common::String &string){
    string = archive.readString();
    return archive;
}

inline Archive &operator>>(Archive &archive, uint32 &num){
    num = archive.readDWORD();
    return archive;
}

inline Archive &operator>>(Archive &archive, uint16 &num){
    num = archive.readWORD();
    return archive;
}

inline Archive &operator>>(Archive &archive, Common::StringArray &array){
    uint32 size = archive.readCount();
    array.resize(size);
    for (uint i = 0; i < size; ++i) {
        array[i] = archive.readString();
    }
    return archive;
}

} // End of namespace Pink

#endif
