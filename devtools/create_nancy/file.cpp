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

#include "file.h"

#include "common/endian.h"

bool File::open(const char *filename, AccessMode mode) {
    _memPtr = nullptr;
    _f = fopen(filename, (mode == kFileReadMode) ? "rb" : "wb+");
    return (_f != NULL);
}

bool File::open(const byte *data, uint size) {
    close();
    _f = nullptr;
    _memPtr = data;
    _size = size;
    return true;
}

void File::close() {
    if (_f) {
        fclose(_f);
    }

    _f = nullptr;
    delete[] _memPtr;
    _memPtr = nullptr;
}

uint File::pos() const {
    if (_f) {
        return ftell(_f);
    } else {
        return _offset;
    }
}

uint File::size() const {
    if (_f) {
        uint currentPos = pos();
        fseek(_f, 0, SEEK_END);
        uint result = pos();
        fseek(_f, currentPos, SEEK_SET);
        return result;
    } else if (_memPtr) {
        return _size;
    } else {
        return 0;
    }
}

bool File::eof() const {
    if (_f) {
        return feof(_f) != 0;
    } else if (_memPtr) {
        return _offset >= _size;
    }

    return false;
}

int File::seek(int offset, int whence) {
    if (_f) {
        return fseek(_f, offset, whence);
    }

    switch (whence) {
        case SEEK_SET:
            _offset = offset;
            break;
        case SEEK_CUR:
            _offset += offset;
            break;
        case SEEK_END:
            _offset = _size + offset;
            break;
        default:
            break;
    }

    return _offset;
}

void File::skip(int offset) {
    if (_f) {
        fseek(_f, offset, SEEK_CUR);
    } else {
        _offset += offset;
    }
}

long File::read(void *buffer, size_t len) {
    if (_f) {
        return fread(buffer, 1, len, _f);
    }

    uint bytesToRead = CLIP(len, (size_t)0, _size - _offset);
    memcpy(buffer, &_memPtr[_offset], bytesToRead);
    _offset += bytesToRead;
    return bytesToRead;
}

byte File::readByte() {
    byte v;
    read(&v, sizeof(byte));
    return v;
}

void File::write(const void *buffer, size_t len) {
    assert(_f);
    fwrite(buffer, 1, len, _f);
}

void File::writeByte(byte v) {
    write(&v, sizeof(byte));
}

void File::writeByte(byte v, int len) {
    byte *b = new byte[len];
    memset(b, v, len);
    write(b, len);
    delete[] b;
}

void File::writeUint16(uint16 v) {
    uint16 vTemp = TO_LE_16(v);
    write(&vTemp, sizeof(uint16));
}

void File::writeUint32(uint v) {
    uint vTemp = TO_LE_32(v);
    write(&vTemp, sizeof(uint));
}

void File::writeString(const char *msg) {
    if (!msg) {
        writeByte(0);
    } else {
        do {
            writeByte(*msg);
        } while (*msg++);
    }
}

template<>
void writeToFile(File &file, const Common::Array<const char *> &obj) {
    file.writeUint16(obj.size());
    for (uint i = 0; i < obj.size(); ++i) {
        file.writeString(obj[i]);
    }
}

template<>
void writeToFile(File &file, const EventFlagDescription &obj) {
    file.writeUint16((uint)obj.label);
    file.writeByte(obj.flag);
}

template<>
void writeToFile(File &file, const SceneChangeDescription &obj) {
    file.writeUint16(obj.sceneID);
    file.writeUint16(obj.frameID);
    file.writeUint16(obj.verticalOffset);
    file.writeUint16(obj.doNotStartSound);
}

template<>
void writeToFile(File &file, const ConditionalDialogue &obj) {
    file.writeByte(obj.textID);
    file.writeUint16(obj.sceneID);
    file.writeString(obj.soundID);
    writeToFile(file, obj.flagConditions);
    writeToFile(file, obj.inventoryConditions);
}

template<>
void writeToFile(File &file, const GoodbyeSceneChange &obj) {
    writeToFile(file, obj.sceneIDs);
    writeToFile(file, obj.flagConditions);
    writeToFile(file, obj.flagToSet);
}

template<>
void writeToFile(File &file, const Goodbye &obj) {
    file.writeString(obj.soundID);
    writeToFile(file, obj.sceneChanges);
}

template<>
void writeToFile(File &file, const Hint &obj) {
    file.writeByte(obj.textID);
    file.writeUint16((uint16)obj.hintWeight);
    writeToFile(file, obj.sceneChange);
    // always three
    file.writeString(obj.soundIDs[0]);
    file.writeString(obj.soundIDs[1]);
    file.writeString(obj.soundIDs[2]);
    writeToFile(file, obj.flagConditions);
    writeToFile(file, obj.inventoryConditions);
}

void writeMultilangArray(File &file, const Common::Array<Common::Array<const char *>> &array) {
    Common::Array<uint32> offsets;
    uint32 offsetsOffset = file.pos();

    file.skip(array.size() * 4 + 4 + 2);

    for (uint i = 0; i < array.size(); ++i) {
        offsets.push_back(file.pos());
        writeToFile(file, array[i]);
    }

    uint end = file.pos();
    file.seek(offsetsOffset);

    file.writeUint16(array.size());
    file.writeUint32(end);
    for (uint i = 0; i < array.size(); ++i) {
        file.writeUint32(offsets[i]);
    }

    file.seek(end);
}
