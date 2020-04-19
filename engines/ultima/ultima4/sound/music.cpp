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

#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/filesys/u4file.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/shared/core/file.h"
#include "audio/decoders/mp3.h"
#include "audio/mods/mod_xm_s3m.h"

namespace Ultima {
namespace Ultima4 {

/*
 * Static variables
 */
Music *g_music;

bool Music::_fading;
bool Music::_on;
bool Music::_functional;

/*
 * Constructors/Destructors
 */

Music::Music() : _introMid(TOWNS), _current(NONE), _playing(nullptr) {
	g_music = this;
	_fading = false;
	_on = false;
	_functional = true;

	_filenames.reserve(MAX);
	_filenames.push_back("");    // filename for MUSIC_NONE;

	/*
	 * load music track filenames from xml config file
	 */
	const Config *config = Config::getInstance();

	Std::vector<ConfigElement> musicConfs = config->getElement("music").getChildren();
	Std::vector<ConfigElement>::const_iterator i = musicConfs.begin();
	Std::vector<ConfigElement>::const_iterator theEnd = musicConfs.end();
	for (; i != theEnd; ++i) {
		if (i->getName() != "track")
			continue;

		_filenames.push_back(i->getString("file"));
	}

	create_sys(); // Call the Sound System specific creation file.

	// Set up the volume.
	_on = settings._musicVol;
}

Music::~Music() {
	g_music = nullptr;
	eventHandler->getTimer()->remove(&Music::callback);
	destroy_sys(); // Call the Sound System specific destruction file.
}

bool Music::isPlaying() {
	return g_music->isPlaying_sys();
}


bool Music::load(Type music) {
	ASSERT(music < MAX, "Attempted to load an invalid piece of music in Music::load()");

	/* music already loaded */
	if (music == _current) {
		/* tell calling function it didn't load correctly (because it's already playing) */
		if (isPlaying())
			return false;
		/* it loaded correctly */
		else
			return true;
	}

	Common::String pathName(u4find_music(_filenames[music]));
	if (!pathName.empty()) {
		bool status = load_sys(pathName);
		if (status)
			_current = music;
		return status;
	}
	return false;
}

void Music::callback(void *data) {
	eventHandler->getTimer()->remove(&Music::callback);

	if (g_music->_on && !isPlaying())
		g_music->play();
	else if (!g_music->_on && isPlaying())
		g_music->stop();
}

void Music::play() {
	playMid(g_context->_location->_map->_music);
}

void Music::introSwitch(int n) {
	if (n > NONE && n < MAX) {
		_introMid = static_cast<Type>(n);
		intro();
	}
}

bool Music::toggle() {
	eventHandler->getTimer()->remove(&Music::callback);

	_on = !_on;
	if (!_on)
		fadeOut(1000);
	else
		fadeIn(1000, true);

	eventHandler->getTimer()->add(&Music::callback, settings._gameCyclesPerSecond);
	return _on;
}

void Music::fadeOut(int msecs) {
	// fade the music out even if '_on' is false
	if (!_functional)
		return;

	if (isPlaying()) {
		if (!settings._volumeFades)
			stop();
		else {
			fadeOut_sys(msecs);
		}
	}
}

void Music::fadeIn(int msecs, bool loadFromMap) {
	if (!_functional || !_on)
		return;

	if (!isPlaying()) {
		// make sure we've got something loaded to play
		if (loadFromMap || !_playing)
			load(g_context->_location->_map->_music);

		if (!settings._volumeFades)
			play();
		else {
			fadeIn_sys(msecs, loadFromMap);
		}
	}
}

int Music::increaseMusicVolume() {
	if (++settings._musicVol > MAX_VOLUME)
		settings._musicVol = MAX_VOLUME;
	else
		setMusicVolume(settings._musicVol);
	return (settings._musicVol * 100 / MAX_VOLUME);  // percentage
}

int Music::decreaseMusicVolume() {
	if (--settings._musicVol < 0)
		settings._musicVol = 0;
	else
		setMusicVolume(settings._musicVol);
	return (settings._musicVol * 100 / MAX_VOLUME);  // percentage
}

int Music::increaseSoundVolume() {
	if (++settings._soundVol > MAX_VOLUME)
		settings._soundVol = MAX_VOLUME;
	else
		setSoundVolume(settings._soundVol);
	return (settings._soundVol * 100 / MAX_VOLUME);  // percentage
}

int Music::decreaseSoundVolume() {
	if (--settings._soundVol < 0)
		settings._soundVol = 0;
	else
		setSoundVolume(settings._soundVol);
	return (settings._soundVol * 100 / MAX_VOLUME);  // percentage
}


void Music::create_sys() {
	_functional = true;
}

void Music::destroy_sys() {
}

bool Music::load_sys(const Common::String &pathName) {
	delete _playing;
	_playing = nullptr;

	if (pathName.hasSuffixIgnoreCase(".it")) {
		warning("TODO: Play music file - %s", pathName.c_str());
		return true;
	}

	Shared::File f;
	if (!f.open(pathName)) {
		warning("unable to load music file %s", pathName.c_str());
		return false;
	}

	if (pathName.hasSuffixIgnoreCase(".mp3")) {
#ifdef USE_MAD
		Common::SeekableReadStream *s = f.readStream(f.size());
		_playing = Audio::makeMP3Stream(s, DisposeAfterUse::YES);
#endif
	} else if (pathName.hasSuffixIgnoreCase(".it"))
		_playing = nullptr;
	else
		error("Unknown sound file");

	return true;
}

void Music::playMid(Type music) {
	if (!_functional || !_on)
		return;

	// loaded a new piece of music
	if (load(music)) {
		stopMid();
		g_ultima->_mixer->playStream(Audio::Mixer::kMusicSoundType,  &_soundHandle, _playing,
			-1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	}
}

void Music::stopMid() {
	g_ultima->_mixer->stopHandle(_soundHandle);
}

void Music::setSoundVolume_sys(int volume) {
	uint vol = 255 * volume / MAX_VOLUME;
	g_ultima->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, vol);
}

bool Music::isPlaying_sys() {
	return g_ultima->_mixer->isSoundHandleActive(_soundHandle);
}

void Music::setMusicVolume_sys(int volume) {
	uint vol = 255 * volume / MAX_VOLUME;
	g_ultima->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol);
}

void Music::fadeIn_sys(int msecs, bool loadFromMap) {
#ifdef TODO
	if (Mix_FadeInMusic(playing, NLOOPS, msecs) == -1)
		errorWarning("Mix_FadeInMusic: %s\n", Mix_GetError());
#endif
}

void Music::fadeOut_sys(int msecs) {
#ifdef TODO
	if (Mix_FadeOutMusic(msecs) == -1)
		errorWarning("Mix_FadeOutMusic: %s\n", Mix_GetError());
#endif
}

} // End of namespace Ultima4
} // End of namespace Ultima
