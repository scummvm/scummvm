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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/archive.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/stream.h"


#include "graphics/surface.h"

#include "prince/object.h"

namespace Prince {

Object::Object() : _surface(NULL) {
}

void Object::loadSurface(Common::SeekableReadStream &stream) {
    stream.skip(4);

    _surface = new Graphics::Surface();
    _surface->create(stream.readUint16LE(), stream.readUint16LE(), Graphics::PixelFormat::createFormatCLUT8());
    for (int h = 0; h < _surface->h; ++h) {
        stream.read(_surface->getBasePtr(0, h), _surface->w);
    }

}

bool Object::loadFromStream(Common::SeekableReadStream &stream) {
   
    int32 pos = stream.pos();
    uint16 x = stream.readUint16LE();
    if (x == 0xFFFF)
        return false;
    _x = x;
    _y = stream.readUint16LE();

    const Common::String obStreamName = Common::String::format("OB%02d", stream.readUint16LE());
    Common::SeekableReadStream *obStream = SearchMan.createReadStreamForMember(obStreamName);
    if (!obStream) {
        error("Can't load %s", obStreamName.c_str());
        return false;
    }

    loadSurface(*obStream);
    delete obStream;

    _z = stream.readUint16LE();
    
    stream.seek(pos + 16);

    debug("Object x %d, y %d, z %d", _x, _y, _z);

    return true;
}

}
