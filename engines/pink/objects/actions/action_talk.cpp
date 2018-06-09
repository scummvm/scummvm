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
 
#include "pink/archive.h"
#include "pink/cel_decoder.h"
#include "pink/sound.h"
#include "pink/objects/actions/action_talk.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

void ActionTalk::deserialize(Archive &archive) {
	ActionLoop::deserialize(archive);
	_vox = archive.readString();
}

void ActionTalk::toConsole() {
	debug("\tActionTalk: _name = %s, _fileName = %s, z = %u, _startFrame = %u,"
				  " _endFrame = %d, _intro = %u, _style = %u, _vox = %s",
		  _name.c_str(), _fileName.c_str(), _z, _startFrame, _stopFrame, _intro, _style, _vox.c_str());
}

void ActionTalk::update() {
	ActionLoop::update();
	if (!_sound.isPlaying()) {
		_decoder->stop();
		_actor->endAction();
	}
}

void ActionTalk::end() {
	ActionPlay::end();
	_sound.stop();
}

void ActionTalk::pause(bool paused) {
	ActionCEL::pause(paused);
	_sound.pause(paused);
}

void ActionTalk::onStart() {
	ActionPlay::onStart();
	//sound balance is calculated different than in ActionSfx(probably a bug in original)
	// 30.0 - x * -0.0625  disasm (0 - 100)
	int8 balance = (_decoder->getX() * 396875 / 1000000) - 127;
	_sound.play(_actor->getPage()->getResourceStream(_vox), Audio::Mixer::kSpeechSoundType, 100, balance);
}

} // End of namespace Pink
