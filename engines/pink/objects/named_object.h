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

#ifndef PINK_NAMED_OBJECT_H
#define PINK_NAMED_OBJECT_H

#include "../archive.h"
#include <common/str.h>
#include "object.h"
#include <common/debug.h>

namespace Pink {

class NamedObject : public Object {
public:
    NamedObject(){};
    NamedObject(const Common::String &name)
            : _name(name)
    {}

    void deserialize(Archive &archive){
        _name = archive.readString();
        debug("NamedObject %s loaded", _name.c_str());
    }
    void store(Archive &archive){

    }

    const Common::String &getName() const {
        return _name;
    }

private:
    Common::String _name;
};

} // End of namespace Pink

#endif