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
#include "common/config-manager.h"
#include "common/stream.h"
#include "common/system.h"

#include "wage/wage.h"
#include "wage/entities.h"
#include "wage/gui.h"
#include "wage/sound.h"
#include "wage/world.h"

namespace Wage {

static const int8 deltas[] = { 0,-49,-36,-25,-16,-9,-4,-1,0,1,4,9,16,25,36,49 };

Sound::Sound(Common::String name, Common::SeekableReadStream *data) : _name(name) {
	_size = data->size() - 20;
	_data = (byte *)calloc(2 * _size, 1);

	debugC(1, kDebugSound, "Sound::Sound: loading sound '%s', size %d", name.c_str(), _size);

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

void WageEngine::playSound(Common::String soundName, bool blocking) {
	soundName.toLowercase();

	if (!_world->_sounds.contains(soundName)) {
		if (!soundName.empty())
			warning("playSound: Sound '%s' does not exist", soundName.c_str());

		return;
	}

	debugC(1, kDebugSound, "WageEngine::playSound: playing sound '%s'%s", soundName.c_str(), blocking ? " blocking" : "");

	Sound *s = _world->_sounds[soundName];

	Audio::AudioStream *stream = Audio::makeRawStream(s->_data, 2 * s->_size, 11000, Audio::FLAG_UNSIGNED);

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream,
		-1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);

	while (_mixer->isSoundHandleActive(_soundHandle) && !_shouldQuit && blocking) {
		Common::Event event;

		if (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				if (ConfMan.hasKey("confirm_exit") && ConfMan.getBool("confirm_exit")) {
					if (!_shouldQuit) {
						g_system->getEventManager()->resetQuit();
						if (_gui->saveDialog()) {
							_shouldQuit = true;
							g_system->getEventManager()->pushEvent(event);
						}
					}
				} else {
					_shouldQuit = true;
				}
				break;
			default:
				break;
			}
		}

		_system->updateScreen();
		_system->delayMillis(10);
	}
}

static void soundTimer(void *refCon) {
	Scene *scene = (Scene *)refCon;
	WageEngine *engine = (WageEngine *)g_engine;

	if (!engine)
		return;

	g_system->getTimerManager()->removeTimerProc(&soundTimer);

	if (engine->_world->_player->_currentScene != scene)
		return;

	if (engine->_soundQueue.empty() && engine->_soundToPlay.empty()) {
		if (scene->_soundType == Scene::PERIODIC) {
			engine->_soundToPlay = scene->_soundName; // We cannot play sound here because that goes recursively

			uint32 nextRun = 60000 / scene->_soundFrequency;
			g_system->getTimerManager()->installTimerProc(&soundTimer, nextRun * 1000, scene, "WageEngine::soundTimer");

			debugC(1, kDebugSound, "soundTimer: enqueuing next periodic sound in %d ms (%s)", nextRun, scene->_soundName.c_str());
		} else if (scene->_soundType == Scene::RANDOM) {
			for (int i = 0; i < scene->_soundFrequency; i++) {
				uint32 nextRun = g_system->getMillis() + engine->_rnd->getRandomNumber(60000);
				engine->_soundQueue.push_back(nextRun);

				debugC(1, kDebugSound, "soundTimer: enqueuing next random sound in %d ms (%s)", nextRun, scene->_soundName.c_str());
			}

			Common::sort(engine->_soundQueue.begin(), engine->_soundQueue.end());

			int nextRun = engine->_soundQueue.front() - g_system->getMillis();
			engine->_soundQueue.pop_front();

			if (nextRun < 0)
				nextRun = 1;

			g_system->getTimerManager()->installTimerProc(&soundTimer, nextRun * 1000, scene, "WageEngine::soundTimer");
		} else {
			warning("updateSoundTimerForScene: Unknown sound type %d", scene->_soundType);
		}
	} else {
		// Do not play sound if another is still playing
		if (!engine->_soundToPlay.empty()) {
			g_system->getTimerManager()->installTimerProc(&soundTimer, g_system->getMillis() + 100, scene, "WageEngine::soundTimer");
			return;
		}

		int nextRun = engine->_soundQueue.front();
		engine->_soundQueue.pop_front();

		g_system->getTimerManager()->installTimerProc(&soundTimer, (nextRun - g_system->getMillis()) * 1000, scene, "WageEngine::soundTimer");

		engine->_soundToPlay = scene->_soundName; // We cannot play sound here because that goes recursively

		debugC(1, kDebugSound, "soundTimer: preparing next sound in %d ms (%s)", nextRun - g_system->getMillis(), scene->_soundName.c_str());
	}
}

void WageEngine::updateSoundTimerForScene(Scene *scene, bool firstTime) {
	if (_world->_player->_currentScene != scene)
			return;

	if (scene->_soundFrequency > 0 && !scene->_soundName.empty()) {
		Common::String soundName(scene->_soundName);

		soundName.toLowercase();

		if (!_world->_sounds.contains(soundName)) {
			warning("updateSoundTimerForScene: Sound '%s' does not exist", soundName.c_str());
			return;
		}

		// Launch the process
		soundTimer(scene);
	}

}


} // End of namespace Wage
