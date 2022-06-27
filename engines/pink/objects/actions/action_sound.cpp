/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"

#include "pink/archive.h"
#include "pink/pink.h"
#include "pink/director.h"
#include "pink/sound.h"
#include "pink/objects/actions/action_sound.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

ActionSound::~ActionSound() {
	end();
}

void ActionSound::deserialize(Archive &archive) {
	Action::deserialize(archive);
	_fileName = archive.readString();
	_volume = archive.readDWORD();
	assert(_volume <= 100);
	_isLoop = (bool)archive.readDWORD();
	_isBackground = (bool)archive.readDWORD();
}

void ActionSound::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\tActionSound: _name = %s, _fileName = %s, _volume = %u, _isLoop = %u,"
				  " _isBackground = %u", _name.c_str(), _fileName.c_str(), _volume, _isLoop, _isBackground);
}

void ActionSound::start() {
	Audio::Mixer::SoundType soundType =  _isBackground ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType;

	Page *page = _actor->getPage();
	if (!_isLoop) {
		Screen *screen = page->getGame()->getScreen();
		screen->addSound(this);
	} else
		_actor->endAction();

	_sound.play(page->getResourceStream(_fileName), soundType, _volume, 0, _isLoop);

	debugC(6, kPinkDebugActions, "Actor %s has now ActionSound %s", _actor->getName().c_str(), _name.c_str());
}

void ActionSound::end() {
	_sound.stop();
	if (!_isLoop) {
		Screen *screen = _actor->getPage()->getGame()->getScreen();
		screen->removeSound(this);
	}

	debugC(6, kPinkDebugActions, "ActionSound %s of Actor %s is ended", _name.c_str(), _actor->getName().c_str());
}

void ActionSound::update() {
	if (!_sound.isPlaying())
		_actor->endAction();
}

void ActionSound::pause(bool paused) {
	_sound.pause(paused);
}

} // End of namespace Pink
