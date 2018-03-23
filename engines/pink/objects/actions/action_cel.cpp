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

#include <common/debug.h>
#include "action_cel.h"
#include <pink/objects/actors/actor.h>
#include "engines/pink/archive.h"
#include "engines/pink/objects/pages/game_page.h"
#include "pink/pink.h"

namespace Pink {

ActionCEL::ActionCEL()
    : _flicDecoder(nullptr) {

}

void ActionCEL::deserialize(Archive &archive) {
    Action::deserialize(archive);
    archive >> _fileName >> _z;
}

void ActionCEL::start(bool unk) {
    if (!_flicDecoder)
        _flicDecoder = _actor->getPage()->loadCel(_fileName);
    _actor->getPage()->getGame()->getDirector()->addSprite(this);
    this->onStart();
}

void ActionCEL::end() {
    _actor->getPage()->getGame()->getDirector()->removeSprite(this);
    delete _flicDecoder;
    _flicDecoder = nullptr;
}

uint32 ActionCEL::getZ() {
    return _z;
}

Video::FlicDecoder *ActionCEL::getDecoder() {
    return _flicDecoder;
}

bool ActionCEL::initPallete(Director *director) {
    _flicDecoder = _actor->getPage()->loadCel(_fileName);
    _flicDecoder->decodeNextFrame();
    director->setPallette(_flicDecoder->getPalette());
    return 1;
}

} // End of namespace Pink