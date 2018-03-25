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
#include "action_sound.h"
#include "engines/pink/archive.h"
#include <engines/pink/objects/actors/actor.h>
#include <engines/pink/objects/pages/game_page.h>
#include <engines/pink/sound.h>
#include "engines/pink/pink.h"


namespace Pink {

ActionSound::ActionSound()
    : _sound(nullptr), _isStopped(1)
{}

void ActionSound::deserialize(Archive &archive) {
    Action::deserialize(archive);
    archive >> _fileName;
    _volume = archive.readDWORD();
    _isLoop = (bool) archive.readDWORD();
    _isBackground = (bool) archive.readDWORD();
}

void ActionSound::toConsole() {
    debug("\tActionSound: _name = %s, _fileName = %s, _volume = %u, _isLoop = %u,"
                  " _isBackground = %u", _name.c_str(), _fileName.c_str(), _volume, _isLoop, _isBackground);
}

void ActionSound::start(bool unk) {
    assert(!_sound);
    _sound = _actor->getPage()->loadSound(_fileName);

    Audio::Mixer::SoundType soundType =  _isBackground ? Audio::Mixer::SoundType::kMusicSoundType
                                                       : Audio::Mixer::SoundType::kSpeechSoundType;

    Director *director = _actor->getPage()->getGame()->getDirector();
    director->addSound(this);

    _sound->play(soundType, _volume, _isLoop);
    if (_isLoop)
        _actor->endAction();

    debug("Actor %s has now ActionSound %s", _actor->getName().c_str(), _name.c_str());
}

void ActionSound::end() {
    debug("ActionSound %s of Actor %s is ended", _name.c_str(), _actor->getName().c_str());

    Director *director = _actor->getPage()->getGame()->getDirector();
    director->removeSound(this);

    _sound->stop();
    delete _sound;
    _sound = nullptr;
}

void ActionSound::update() {
    if (!_sound->isPlaying())
        _actor->endAction();
}

} // End of namespace Pink