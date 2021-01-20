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

#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "audio/audiostream.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/voc.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"
#include "common/file.h"

namespace Ultima {
namespace Ultima4 {

SoundManager *g_sound;

void soundPlay(Sound sound, bool onlyOnce, int specificDurationInTicks) {
	g_sound->play(sound, onlyOnce, specificDurationInTicks);
}

void soundStop(int channel) {
	g_sound->stop(channel);
}

/*-------------------------------------------------------------------*/

SoundManager::SoundManager(Audio::Mixer *mixer) : _mixer(mixer) {
	g_sound = this;

	// Load sound track filenames from xml config file
	const Config *config = Config::getInstance();
	_soundFilenames.reserve(SOUND_MAX);
	_sounds.resize(SOUND_MAX);

	Std::vector<ConfigElement> soundConfs = config->getElement("sound").getChildren();
	Std::vector<ConfigElement>::const_iterator i = soundConfs.begin();
	Std::vector<ConfigElement>::const_iterator theEnd = soundConfs.end();
	for (; i != theEnd; ++i) {
		if (i->getName() != "track")
			continue;

		_soundFilenames.push_back(i->getString("file"));
	}
}

SoundManager::~SoundManager() {
	g_sound = nullptr;
	_mixer->stopHandle(_soundHandle);

	for (uint idx = 0; idx < _sounds.size(); ++idx)
		delete _sounds[idx];
}

bool SoundManager::load(Sound sound) {
	assertMsg(sound < SOUND_MAX, "Attempted to load an invalid sound");

	if (_sounds[sound] == nullptr) {
		Common::String pathname("data/sound/" + _soundFilenames[sound]);
		Common::String basename = pathname.substr(pathname.findLastOf("/") + 1);
		if (!basename.empty())
			return load_sys(sound, pathname);
	}

	return true;
}

void SoundManager::play(Sound sound, bool onlyOnce, int specificDurationInTicks) {
	assertMsg(sound < SOUND_MAX, "Attempted to play an invalid sound");

	if (_sounds[sound] == nullptr) {
		if (!load(sound)) {
			return;
		}
	}

	play_sys(sound, onlyOnce, specificDurationInTicks);
}

void SoundManager::stop(int channel) {
	stop_sys(channel);
}

bool SoundManager::load_sys(Sound sound, const Common::String &filename) {
	Common::File f;
	if (!f.open(filename))
		return false;

	Audio::SeekableAudioStream *audioStream = nullptr;

#ifdef USE_FLAC
	if (filename.hasSuffixIgnoreCase(".fla"))
		audioStream = Audio::makeFLACStream(f.readStream(f.size()), DisposeAfterUse::YES);
#endif
#ifdef USE_VORBIS
	if (filename.hasSuffixIgnoreCase(".ogg"))
		audioStream = Audio::makeVorbisStream(f.readStream(f.size()), DisposeAfterUse::YES);
#endif
#ifdef USE_MAD
	if (filename.hasSuffixIgnoreCase(".mp3"))
		audioStream = Audio::makeMP3Stream(f.readStream(f.size()), DisposeAfterUse::YES);
#endif
	if (filename.hasSuffixIgnoreCase(".wav"))
		audioStream = Audio::makeWAVStream(f.readStream(f.size()), DisposeAfterUse::YES);
	if (filename.hasSuffixIgnoreCase(".voc"))
		audioStream = Audio::makeVOCStream(f.readStream(f.size()), DisposeAfterUse::YES);

	_sounds[sound] = audioStream;
	return audioStream != nullptr;
}

void SoundManager::play_sys(Sound sound, bool onlyOnce, int specificDurationMilli) {
	// Don't allow once only sounds if another sound is already playing
	if (onlyOnce && _mixer->isSoundHandleActive(_soundHandle))
		return;

	// Ensure sound is stopped, and rewinded
	_mixer->stopHandle(_soundHandle);
	_sounds[sound]->rewind();

	if (specificDurationMilli == -1) {
		// Play a single sound effect
		_mixer->playStream(Audio::Mixer::kSFXSoundType,
			&_soundHandle, _sounds[sound], -1, Audio::Mixer::kMaxChannelVolume,
			0, DisposeAfterUse::NO);
	} else {
		// Play a sound effect, looping if necessary, for a given duration
		// TODO: Better handle cases where a number of loops won't fit
		// exactly to give a desired duration
		int duration = _sounds[sound]->getLength().msecs();
		int loops = (specificDurationMilli + duration - 1) / duration;
		assert(loops >= 0);

		Audio::AudioStream *audioStream = new Audio::LoopingAudioStream(
			_sounds[sound], loops, DisposeAfterUse::NO);

		_mixer->playStream(Audio::Mixer::kSFXSoundType,
			&_soundHandle, audioStream, -1, Audio::Mixer::kMaxChannelVolume,
			0, DisposeAfterUse::NO);
	}
}

void SoundManager::stop_sys(int channel) {
	// Channel 1 is the dummy channel number used for sound effects
	if (channel == 1)
		_mixer->stopHandle(_soundHandle);
}

} // End of namespace Ultima4
} // End of namespace Ultima
