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

#include "harvester/media/media_manager.h"

#include "common/ptr.h"
#include "common/system.h"
#include "graphics/screen.h"
#include "harvester/art.h"
#include "harvester/resources.h"
#include "harvester/entity.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static int clampStartupOptionLevel(int level) {
	if (level < 0)
		return 0;
	if (level > 9)
		return 9;

	return level;
}

static int mapStartupVolumeLevelToMixerVolume(int level) {
	return (clampStartupOptionLevel(level) * Audio::Mixer::kMaxMixerVolume + 4) / 9;
}

} // End of anonymous namespace

MediaManager::MediaManager(ResourceManager &resources) : _resources(resources) {
	_entityManager = new EntityManager(_resources);
}

MediaManager::~MediaManager() {
	stopMusic();
	stopSound();
	delete _text;
	delete _art;
	delete _screen;
	delete _entityManager;
}

float MediaManager::mapGammaLevelToBrightnessScale(int level) {
	return 1.0f + 0.1f * clampStartupOptionLevel(level);
}

void MediaManager::resetScreen(int width, int height) {
	delete _screen;
	_screen = new Graphics::Screen();
	_displayWidth = width;
	_displayHeight = height;
}

void MediaManager::finishMoviePlayback() {
	_roomPaletteFadePending = _fadeRoomAfterNextMovie;
	_fadeRoomAfterNextMovie = false;
}

void MediaManager::resetMoviePaletteState() {
	_fadeRoomAfterNextMovie = false;
	_roomPaletteFadePending = false;
}

bool MediaManager::loadArt() {
	Common::ScopedPtr<Art> art(new Art());
	if (!art->load(_resources))
		return false;

	delete _art;
	_art = art.release();
	return true;
}

bool MediaManager::loadText() {
	Common::ScopedPtr<Text> text(new Text());
	if (!text->load(_resources))
		return false;

	delete _text;
	_text = text.release();
	return true;
}

bool MediaManager::loadQuickTipsResources(bool useTextboxPanel) {
	return _art && _art->loadQuickTipsResources(_resources, useTextboxPanel);
}

void MediaManager::drawWaitFrame() const {
	if (_art && _screen)
		_art->drawWaitFrame(*_screen);
}

void MediaManager::applyMixerLevels(int fxLevel, int musicLevel) {
	if (!g_system || !g_system->getMixer())
		return;

	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType,
		mapStartupVolumeLevelToMixerVolume(fxLevel));
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType,
		mapStartupVolumeLevelToMixerVolume(musicLevel));

	if (g_system->getMixer()->isSoundHandleActive(_musicHandle))
		g_system->getMixer()->pauseHandle(_musicHandle, musicLevel == 0);
}

} // End of namespace Harvester
