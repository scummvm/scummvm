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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "gamos/gamos.h"

namespace Gamos {

Archive::Archive() {
};

Archive::~Archive() {
};

bool Archive::open(const Common::Path &name) {
    bool res = File::open(name);

    if (!res)
        return false;

    seek(-12, SEEK_END);

    _dirOffset = 12 + readUint32LE();
    skip(4);
    uint32 magic = readUint32LE();

    if (magic != 0x3d53563d) // =VS=
        return false;

    seek(-_dirOffset, SEEK_END);

    _dirCount = readUint32LE();
    _dataOffset = readUint32LE();

    seek(-(_dirOffset + _dirCount * 5), SEEK_END);

    _directories.resize(_dirCount);

    for(uint i = 0; i < _dirCount; ++i) {
        ArchiveDir &dir = _directories[i];

        dir.offset = readUint32LE();
        dir.id = readByte();
    }

    return true;
}

bool Archive::seekDir(uint id) {
    int16 idx = findDirByID(id);
    if (idx < 0)
        return false;

    const ArchiveDir &dir = _directories[idx];

    if ( !seek(_dataOffset + dir.offset, SEEK_SET) )
        return false;

    return true;
}

int32 Archive::readPackedInt() {
    byte b = readByte();
    if ( !(b & 0x80) )
        return b;

    byte num = 0;
    byte skipsz = 0;
    if ( !(b & 0x20) )
        num = b & 0x1f;
    else
        num = 1 + ((b >> 2) & 3);

    if (num > 4) {
        skipsz = num - 4;
        num = 4;
    }

    int32 val = 0;
    for(int i = 0; i < num; ++i)
        val |= readByte() << (i << 3);

    if (skipsz) {
        skip(skipsz);
        /* warning !!!! */
        printf("readPackedInt skipped %d\n", skipsz);
    }

    static int32 negs[4] {0, -1, -1025, -263169};
    static int32 adds[4] {0, 0x80, 0x480, 0x40480};

    if (b & 0x20) {
        val += (b & 3) * (1 << ((num << 3) & 0x1f));
        if (b & 0x10)
            val = negs[num] - val;
        else
            val += adds[num];
    }

    return val;
}

RawData *Archive::readCompressedData() {
    RawData *data = new RawData();
    if (!readCompressedData(data)) {
        delete data;
        data = nullptr;
    }
    return data;
}

bool Archive::readCompressedData(RawData *out) {
    const byte t = readByte();
    if ((t & 0x80) == 0)
        return false;

    _lastReadDecompressedSize = 0;
    _lastReadSize = 0;

    if (t & 0x40) {
        /* small uncompressed data */
    _lastReadSize = t & 0x1F;
    } else {
        /* read size */
        const byte szsize = (t & 3) + 1;

        /* big data size */
        for (uint i = 0; i < szsize; ++i)
            _lastReadSize |= readByte() << (i << 3);

        /* is compressed */
        if (t & 0xC) {
            for (uint i = 0; i < szsize; ++i)
            _lastReadDecompressedSize |= readByte() << (i << 3);
        }
    }

    if (!_lastReadSize)
        return false;

    _lastReadDataOffset = pos();
    out->resize(_lastReadSize);
    read(out->data(), _lastReadSize);

    if (!_lastReadDecompressedSize)
        return true;

    /* looks hacky but we just allocate array for decompressed and swap it with compressed */
    RawData compressed(_lastReadDecompressedSize);
    out->swap(compressed);

    decompress(&compressed, out);
    return true;
}

void Archive::decompress(RawData const *in, RawData *out) {
    uint pos = 0;
    uint outPos = 0;

    while (pos < in->size()) {
        byte ctrlBits = (*in)[pos];
        pos++;

        for(int bitsLeft = 8; bitsLeft > 0; --bitsLeft) {
            if (pos >= in->size())
                return;

            if (ctrlBits & 1) {
                (*out)[outPos] = (*in)[pos];
                outPos++;
                pos++;
            } else {
                byte b1 = (*in)[pos];
                byte b2 = (*in)[pos + 1];
                pos += 2;

                byte num = (b2 & 0xF) + 3;
                uint16 distance = b1 | ((b2 & 0xF0) << 4);

                for(int i = 0; i < num; ++i) {
                    (*out)[outPos] = (*out)[outPos - distance];
                    outPos++;
                }
            }

            ctrlBits >>= 1;
        }
    }
}


};
