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

#include "ultima/nuvie/lua/scummvm_file.h"

namespace Ultima {
namespace Nuvie {

ScummVMFile *ScummVMFile::open(const Common::String &filename, const Common::String &mode) {
    Common::File *f = new Common::File();
    if (f->open(filename)) {
        assert(mode == "r" || mode == "rb");
        return new ScummVMFile(f, mode == "r");
    } else {
        delete f;
        return nullptr;
    }
}

bool ScummVMFile::eof() const {
    return _file->eos();
}

int ScummVMFile::error() const {
    return _file->err() ? 1 : 0;
}

size_t ScummVMFile::read(void *buf, size_t size) {
    if (_isText) {
        // Text mode, so we need to skip the CR in CRLF line terminated files
        
        if (eof())
            return 0;
        
        byte *destP = (byte *)buf;
        size_t bytesRead = 0;
        while (size > 0) {
            // Get the next byte
            byte c = _file->readByte();
            if (_file->eos())
                break;
            
            if (c != '\r') {
                *destP++ = c;
                ++bytesRead;
                --size;
            }
        }

        return bytesRead;
    } else {
        return _file->read(buf, size);
    }
}

int ScummVMFile::getChar() {
    byte c = '\r';
    while (!_file->eos() && c == '\r')
        c = _file->readByte();

    return _file->eos() ? EOF : c;
}


} // End of namespace Nuvie
} // End of namespace Ultima
