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

#include "common/debug.h"

#include "pink/archive.h"
#include "pink/cel_decoder.h"
#include "pink/objects/actions/action_play.h"
#include "pink/objects/actors/actor.h"

namespace Pink {

void ActionPlay::deserialize(Archive &archive) {
    ActionStill::deserialize(archive);
    _stopFrame = archive.readDWORD();
}

void ActionPlay::toConsole() {
    debug("\tActionPlay: _name = %s, _fileName = %s, z = %u, _startFrame = %u,"
                  " _endFrame = %d", _name.c_str(), _fileName.c_str(), _z, _startFrame, _stopFrame);
}

void ActionPlay::end() {
    ActionCEL::end();
    debug("ActionPlay %s of Actor %s is ended", _name.c_str(), _actor->getName().c_str());
}

void ActionPlay::onStart() {
    debug("Actor %s has now ActionPlay %s", _actor->getName().c_str(), _name.c_str());
    _decoder->start();
    for (uint i = 0; i < _startFrame; ++i) {
        _decoder->skipFrame();
    }
    _decoder->decodeNextFrame();
}

void ActionPlay::update() {
    if (_decoder->endOfVideo() || _decoder->getCurFrame() == _stopFrame){
        _decoder->stop();
        _actor->endAction();
    }
}

} // End of namespace Pink
