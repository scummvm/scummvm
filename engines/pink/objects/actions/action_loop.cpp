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


#include "action_loop.h"
#include <common/debug.h>
#include <pink/archive.h>

namespace Pink {

void ActionLoop::deserialize(Archive &archive) {
    ActionPlay::deserialize(archive);
    uint32 style;
    archive >> _intro >> style;
    assert(style <= 2);
    _style = static_cast<Style>(style);
}

void ActionLoop::toConsole() {
    debug("\tActionLoop: _name = %s, _fileName = %s, z = %u, _startFrame = %u,"
                  " _endFrame = %u, _intro = %u, style = %u",
          _name.c_str(), _fileName.c_str(), _z, _startFrame, _stopFrame, _intro, _style);
}

} // End of namespace Pink