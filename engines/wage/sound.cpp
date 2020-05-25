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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "common/stream.h"
#include "common/system.h"

#include "wage/wage.h"
#include "wage/entities.h"
#include "wage/sound.h"
#include "wage/world.h"

namespace Wage {

static const int8 deltas[] = { 0,-49,-36,-25,-16,-9,-4,-1,0,1,4,9,16,25,36,49 };

Sound::Sound(Common::String name, Common::SeekableReadStream *data) : _name(name) {
	_size = data->size() - 20;
	_data = (byte *)calloc(2 * _size, 1);

	data->skip(20); // Skip header

	byte value = 0x80;
	for (uint i = 0; i < _size; i++) {
		byte d = data->readByte();
		value += deltas[d & 0xf];
		_data[i * 2] = value;
		value += deltas[(d >> 4) & 0xf];
		_data[i * 2 + 1] = value;
	}
}

Sound::~Sound() {
	free(_data);
}

void WageEngine::playSound(Common::String soundName) {
	soundName.toLowercase();

	if (!_world->_sounds.contains(soundName)) {
		warning("playSound: Sound '%s' does not exist", soundName.c_str());
		return;
	}

	Sound *s = _world->_sounds[soundName];

	Audio::AudioStream *stream = Audio::makeRawStream(s->_data, s->_size, 11000, Audio::FLAG_UNSIGNED);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, stream,
		-1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);

	while (_mixer->isSoundHandleActive(_soundHandle) && !_shouldQuit) {
		Common::Event event;

		if (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				if (saveDialog())
					_shouldQuit = true;
				break;
			default:
				break;
			}
		}

		_system->updateScreen();
		_system->delayMillis(10);
	}
}

void WageEngine::updateSoundTimerForScene(Scene *scene, bool firstTime) {
	//warning("STUB: WageEngine::updateSoundTimerForScene()");
	if (_world->_player->_currentScene != scene)
			return;

	if (scene->_soundFrequency > 0 && !scene->_soundName.empty()) {
		Common::String soundName(scene->_soundName);

		soundName.toLowercase();

		if (!_world->_sounds.contains(soundName)) {
			warning("updateSoundTimerForScene: Sound '%s' does not exist", soundName.c_str());
			return;
		}

		warning("STUB: updateSoundTimerForScene: sound: '%s', %s", soundName.c_str(),
				scene->_soundType == Scene::PERIODIC ? "PERIODIC" : "RANDOM");

#if 0
		soundTimer = new Timer();
		switch (scene.getSoundType()) {
		case Scene.PERIODIC:
			if (firstTime)
				soundTimer.schedule(new PlaySoundTask(scene, sound), 0);
			int delay = 60000 / scene.getSoundFrequency();
			soundTimer.schedule(new PlaySoundTask(scene, sound), delay);
			soundTimer.schedule(new UpdateSoundTimerTask(scene), delay + 1);
			break;
		case Scene.RANDOM:
			for (int i = 0; i < scene.getSoundFrequency(); i++)
				soundTimer.schedule(new PlaySoundTask(scene, sound), (int)(Math.random() * 60000));
			soundTimer.schedule(new UpdateSoundTimerTask(scene), 60000);
			break;
		}
#endif
	}

}


} // End of namespace Wage
