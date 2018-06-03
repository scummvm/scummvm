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

#include "pink/cel_decoder.h"
#include "pink/sound.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/actions/action_play_with_sfx.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

ActionPlayWithSfx::~ActionPlayWithSfx() {
	ActionPlay::end();
	for (uint i = 0; i < _sfxArray.size(); ++i) {
		delete _sfxArray[i];
	}
}

void ActionPlayWithSfx::deserialize(Pink::Archive &archive) {
	ActionPlay::deserialize(archive);
	_isLoop = archive.readDWORD();
	_sfxArray.deserialize(archive);
}

void ActionPlayWithSfx::toConsole() {
	debug("\tActionPlayWithSfx: _name = %s, _fileName = %s, z = %u, _startFrame = %u,"
				  " _endFrame = %d, _isLoop = %u", _name.c_str(), _fileName.c_str(), _z, _startFrame, _stopFrame, _isLoop);
	for (uint i = 0; i < _sfxArray.size(); ++i) {
		_sfxArray[i]->toConsole();
	}
}

void ActionPlayWithSfx::update() {
	if ((_decoder->endOfVideo() || _decoder->getCurFrame() == _stopFrame) && _isLoop) {
		_decoder->rewind();
	} else if (_decoder->endOfVideo() || _decoder->getCurFrame() == _stopFrame) {
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
	if (!_actor->isPlaying() && !_isLoop)
		return;

	for (uint i = 0; i < _sfxArray.size(); ++i) {
		if (_sfxArray[i]->getFrame() == _decoder->getCurFrame())
			_sfxArray[i]->play(_actor->getPage());
	}
}

ActionSfx::ActionSfx()
		: _sound(nullptr) {}

ActionSfx::~ActionSfx() {
	end();
}

void ActionSfx::deserialize(Pink::Archive &archive) {
	_frame = archive.readDWORD();
	_volume = archive.readDWORD();
	_sfxName = archive.readString();
	archive.readObject(); // pointer of ActionPlayWithSfx
}

void ActionSfx::toConsole() {
	debug("\t\tActionSfx: _sfx = %s, _volume = %u, _frame = %u", _sfxName.c_str(), _volume, _frame);
}

void ActionSfx::play(Page *page) {
	if (!_sound)
		_sound = page->loadSound(_sfxName);

	if (!_sound->isPlaying())
		_sound->play(Audio::Mixer::kSFXSoundType, _volume, 0);
}

void ActionSfx::end() {
	delete _sound;
	_sound = nullptr;
}

uint32 ActionSfx::getFrame() {
	return _frame;
}

} // End of namespace Pink
