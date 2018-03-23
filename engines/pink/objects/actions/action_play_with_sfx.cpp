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

#include "action_play_with_sfx.h"
#include <pink/archive.h>
#include <common/debug.h>

namespace Pink {

void Pink::ActionPlayWithSfx::deserialize(Pink::Archive &archive) {
    ActionPlay::deserialize(archive);
    archive >> _isLoop >> _sfxArray;
}

void Pink::ActionPlayWithSfx::toConsole() {
    debug("\tActionPlayWithSfx: _name = %s, _fileName = %s, z = %u, _startFrame = %u,"
                  " _endFrame = %u, _isLoop = %u", _name.c_str(), _fileName.c_str(), _z, _startFrame, _stopFrame);
    for (int i = 0; i < _sfxArray.size(); ++i) {
        _sfxArray[i]->toConsole();
    }
}

void Pink::ActionSfx::deserialize(Pink::Archive &archive) {
    archive >> _sfx >> _volume >> _frame;
}

void Pink::ActionSfx::toConsole() {
    debug("\t\tActionSfx: _name = %s, _volume = %u, _frame = %u");
}

} // End of namespace Pink