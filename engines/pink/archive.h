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

#include "common/hash-str.h"
#include "common/str-array.h"
#include "common/stream.h"

#include "pink/objects/object.h"

namespace Common {

class File;

}

namespace Pink {

class Archive {
public:
    Archive(Common::SeekableReadStream *stream);
    Archive(Common::WriteStream *stream);
    ~Archive();

    void mapObject(Object *obj);

    int readCount();
    uint32 readDWORD();
    uint16 readWORD();

    void writeDWORD(uint32 val);
    void writeWORD(uint16 val);

    Object *readObject();
    Common::String readString();
    void writeString(const Common::String &string);

private:
    uint findObjectId(const char *name);

    Object *parseObject(bool &isCopyReturned);

    Common::Array<Object *> _objectMap;
    Common::Array<uint> _objectIdMap;
    Common::SeekableReadStream *_readStream;
    Common::WriteStream *_writeStream;
};

template <typename T>
inline Archive &operator>>(Archive &archive, Common::Array<T> &arr){
    uint size = archive.readCount();
    arr.resize(size);
    for (uint i = 0; i < size; ++i) {
        arr[i] =  dynamic_cast<T>(archive.readObject());
        assert(arr[i]);
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

inline Archive &operator>>(Archive &archive, Common::StringMap &map){
    uint size = archive.readCount();
    for (int i = 0; i < size; ++i) {
        Common::String key = archive.readString();
        Common::String val = archive.readString();
        map.setVal(key, val);
    }
    return archive;
}

inline Archive &operator<<(Archive &archive, Common::String &string){
    archive.writeString(string);
    return archive;
}

inline Archive &operator<<(Archive &archive, uint32 &num){
    archive.writeDWORD(num);
    return archive;
}

inline Archive &operator<<(Archive &archive, uint16 &num){
    archive.writeWORD(num);
    return archive;
}

inline Archive &operator<<(Archive &archive, Common::StringArray &array){
    archive.writeWORD(array.size());
    for (uint i = 0; i < array.size(); ++i) {
        archive.writeString(array[i]);
    }
    return archive;
}

inline Archive &operator<<(Archive &archive, Common::StringMap &map){
    archive.writeWORD(map.size());
	for (Common::StringMap::const_iterator it = map.begin(); it != map.end(); ++it) {
		archive.writeString(it->_key);
		archive.writeString(it->_value);
	}
    return archive;
}

} // End of namespace Pink

#endif
