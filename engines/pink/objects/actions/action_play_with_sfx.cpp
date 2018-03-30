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
#include <pink/objects/pages/game_page.h>
#include <pink/sound.h>
#include <pink/objects/actors/actor.h>
#include <pink/cel_decoder.h>

namespace Pink {

void ActionPlayWithSfx::deserialize(Pink::Archive &archive) {
    ActionPlay::deserialize(archive);
    archive >> _isLoop >> _sfxArray;
}

void ActionPlayWithSfx::toConsole() {
    debug("\tActionPlayWithSfx: _name = %s, _fileName = %s, z = %u, _startFrame = %u,"
                  " _endFrame = %u, _isLoop = %u", _name.c_str(), _fileName.c_str(), _z, _startFrame, _stopFrame);
    for (int i = 0; i < _sfxArray.size(); ++i) {
        _sfxArray[i]->toConsole();
    }
}

void ActionPlayWithSfx::update() {
    if (_decoder->endOfVideo() && _isLoop) {
        _decoder->rewind();
    } else if (_decoder->endOfVideo()) {
        _decoder->stop();
        _actor->endAction();
    }
    updateSound();
}

void ActionPlayWithSfx::onStart() {
    ActionPlay::onStart();
    if (_isLoop) {
        _actor->endAction();
    }
    updateSound();
}

void ActionPlayWithSfx::updateSound() {
    for (int i = 0; i < _sfxArray.size(); ++i) {
        if (_sfxArray[i]->getFrame() == _decoder->getCurFrame()) {
            _sfxArray[i]->play(_actor->getPage());
        }
    }
}

ActionPlayWithSfx::~ActionPlayWithSfx() {
    for (int i = 0; i < _sfxArray.size(); ++i) {
        delete _sfxArray[i];
    }
}

void ActionPlayWithSfx::end() {
    ActionPlay::end();
    for (int i = 0; i < _sfxArray.size(); ++i) {
        _sfxArray[i]->end();
    }
}

void ActionSfx::deserialize(Pink::Archive &archive) {
    archive >> _frame >> _volume >> _sfxName;
    archive.readObject();
}

void ActionSfx::toConsole() {
    debug("\t\tActionSfx: _sfx = %s, _volume = %u, _frame = %u", _sfxName.c_str(), _volume, _frame);
}

void ActionSfx::play(GamePage *page) {
    if (!_sound)
        _sound = page->loadSound(_sfxName);

    _sound->play(Audio::Mixer::SoundType::kSFXSoundType, _volume, 0);
}

ActionSfx::~ActionSfx() {
    end();
}

uint32 ActionSfx::getFrame() {
    return _frame;
}

ActionSfx::ActionSfx()
    : _sound(nullptr)
{}

void ActionSfx::end() {
    delete _sound;
    _sound = nullptr;
}

} // End of namespace Pink