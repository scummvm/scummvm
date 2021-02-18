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

#include "snatcher/graphics.h"
#include "snatcher/snatcher.h"
#include "snatcher/sound.h"
#include "common/error.h"

namespace Snatcher {

SnatcherEngine::SnatcherEngine(OSystem *system, GameDescription &dsc) : Engine(system), _game(dsc), _gfx(0), _snd(0) {

}

SnatcherEngine::~SnatcherEngine() {
	delete _gfx;
	delete _snd;
}

Common::Error SnatcherEngine::run() {
	registerDefaultSettings();

	if (!initResource())
		return Common::Error(Common::kReadingFailed);

	if (!initSound(_game.platform, _game.soundOptions))
		return Common::Error(Common::kAudioDeviceInitFailed);

	if (!initGraphics(_game.platform))
		return Common::Error(Common::kUnknownError);

	return Common::Error(start() ? Common::kNoError : Common::kUnknownError);
}

bool SnatcherEngine::initResource() {
	return true;
}

bool SnatcherEngine::initGraphics(Common::Platform platform) {
	_gfx = new GraphicsEngine(platform);
	return _gfx;
}

bool SnatcherEngine::initSound(Common::Platform platform, int soundOptions) {
	_snd = new SoundEngine(platform, soundOptions);
	return _snd;
}

bool SnatcherEngine::start() {
	return true;
}

void SnatcherEngine::registerDefaultSettings() {
	//ConfMan.registerDefault("cdaudio", true);
}

void SnatcherEngine::syncSoundSettings() {

}

void SnatcherEngine::pauseEngineIntern(bool pause) {

}

} // End of namespace Snatcher
