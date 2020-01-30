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

#ifndef NUVIE_LUA_SCUMMVM_FILE
#define NUVIE_LUA_SCUMMVM_FILE

#include "common/file.h"

namespace Ultima {
namespace Nuvie {

class ScummVMFile {
private:
    Common::File *_file;
    bool _isText;

    ScummVMFile(Common::File *f, bool isText) : _file(f), _isText(isText) {
    }
public:
    static ScummVMFile *open(const Common::String &filename, const Common::String &mode);
public:
    ~ScummVMFile() {
        delete _file;
    }

    /**
     * Returns true if the end of the file has been reached
     */
    bool eof() const;

    /**
     * Returns non-zero for any error having occurred
     */
    int error() const;

    /**
     * Reads bytes from the file
     */
    size_t read(void *buf, size_t size);

    /**
     * Gets the next character/byte
     */
    int getChar();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
