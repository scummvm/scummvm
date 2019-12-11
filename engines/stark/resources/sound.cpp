/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/resources/sound.h"

#include "audio/decoders/vorbis.h"

#include "common/system.h"

#include "engines/stark/formats/iss.h"
#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"

namespace Stark {
namespace Resources {

Sound::~Sound() {
}

Sound::Sound(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_enabled(0),
		_looping(false),
		_field_64(0),
		_loopIndefinitely(false),
		_loadFromFile(true),
		_maxDuration(0),
		_stockSoundType(0),
		_field_6C(0),
		_soundType(0),
		_pan(0),
		_volume(0),
		_fadeDurationRemaining(0),
		_fadeTargetVolume(0.0),
		_fadeTargetPan(0.0),
		_shouldStopOnDestroy(true) {
	_type = TYPE;
}

Audio::RewindableAudioStream *Sound::makeAudioStream() {
	Common::SeekableReadStream *stream = nullptr;
	Audio::RewindableAudioStream *audioStream = nullptr;

	// First try the .iss / isn files
	if (_loadFromFile) {
		stream = StarkArchiveLoader->getExternalFile(_filename, _archiveName);
	} else {
		stream = StarkArchiveLoader->getFile(_filename, _archiveName);
	}

	if (stream) {
		audioStream = Formats::makeISSStream(stream, DisposeAfterUse::YES);
	}

	if (!audioStream) {
		// The 2 CD version uses Ogg Vorbis
		Common::String filename = _filename;
		if (_filename.hasSuffix(".iss") || _filename.hasSuffix(".isn")) {
			filename = Common::String(_filename.c_str(), _filename.size() - 4) + ".ovs";
		}

		stream = StarkArchiveLoader->getExternalFile(filename, _archiveName);
		if (stream) {
#ifdef USE_VORBIS
			audioStream = Audio::makeVorbisStream(stream, DisposeAfterUse::YES);
#else
			warning("Cannot decode sound '%s', Vorbis support is not compiled in", filename.c_str());
			delete stream;
#endif
		}
	}

	if (!audioStream) {
		warning("Unable to load sound '%s'", _filename.c_str());
	}

	return audioStream;
}

Audio::Mixer::SoundType Sound::getMixerSoundType() {
	switch (_soundType) {
	case kSoundTypeVoice:
		return Audio::Mixer::kSpeechSoundType;
	case kSoundTypeEffect:
		return Audio::Mixer::kSFXSoundType;
	case kSoundTypeMusic:
		return Audio::Mixer::kMusicSoundType;
	default:
		error("Unknown sound type '%d'", _soundType);
	}
}

void Sound::play() {
	if (isPlaying()) {
		return;
	}

	Audio::RewindableAudioStream *rewindableStream = makeAudioStream();

	if (!rewindableStream) {
		return;
	}

	Audio::AudioStream *playStream;
	if (_looping) {
		playStream = Audio::makeLoopingAudioStream(rewindableStream, 0);
	} else {
		playStream = rewindableStream;
	}

	g_system->getMixer()->playStream(getMixerSoundType(), &_handle, playStream, -1,
	                                 _volume * Audio::Mixer::kMaxChannelVolume, _pan * 127);
}

bool Sound::isPlaying() {
	return g_system->getMixer()->isSoundHandleActive(_handle);
}

void Sound::stop() {
	g_system->getMixer()->stopHandle(_handle);
	_handle = Audio::SoundHandle();
}

void Sound::onPreDestroy() {
	Object::onPreDestroy();

	if (_shouldStopOnDestroy) {
		stop();
	}
}

void Sound::readData(Formats::XRCReadStream *stream) {
	_filename = stream->readString();
	_enabled = stream->readUint32LE();
	_looping = stream->readBool();
	_field_64 = stream->readUint32LE();
	_loopIndefinitely = stream->readBool();
	_maxDuration = stream->readUint32LE();
	_loadFromFile = stream->readBool(); // Used only in the 4CD version
	_stockSoundType = stream->readUint32LE();
	_soundName = stream->readString();
	_field_6C = stream->readUint32LE();
	_soundType = stream->readUint32LE();
	_pan = stream->readFloatLE();
	_volume = stream->readFloatLE();
	_archiveName = stream->getArchiveName();
}

void Sound::printData() {
	debug("filename: %s", _filename.c_str());
	debug("enabled: %d", _enabled);
	debug("looping: %d", _looping);
	debug("field_64: %d", _field_64);
	debug("loopIndefinitely: %d", _loopIndefinitely);
	debug("maxDuration: %d", _maxDuration);
	debug("loadFromFile: %d", _loadFromFile);
	debug("stockSoundType: %d", _stockSoundType);
	debug("soundName: %s", _soundName.c_str());
	debug("field_6C: %d", _field_6C);
	debug("soundType: %d", _soundType);
	debug("pan: %f", _pan);
	debug("volume: %f", _volume);
}

void Sound::onGameLoop() {
	Object::onGameLoop();

	if (_subType == kSoundBackground && !isPlaying()) {
		Location *location = StarkGlobal->getCurrent()->getLocation();
		if (location->getName() != "Amongst Stalls" || StarkGlobal->getCurrentChapter() < 100) {
			play();
		}
	}

	if (_looping && !_loopIndefinitely) {
		// Automatically stop after the maximum run time has been reached
		uint32 elapsedTime = g_system->getMixer()->getSoundElapsedTime(_handle);
		if (elapsedTime > _maxDuration) {
			stop();
		}
	}

	if (_fadeDurationRemaining > 0 && isPlaying()) {
		_volume += (_fadeTargetVolume - _volume) * StarkGlobal->getMillisecondsPerGameloop() / (float) _fadeDurationRemaining;
		_pan += (_fadeTargetPan - _pan) * StarkGlobal->getMillisecondsPerGameloop() / (float) _fadeDurationRemaining;

		_fadeDurationRemaining -= StarkGlobal->getMillisecondsPerGameloop();

		if (_fadeDurationRemaining <= 0) {
			_fadeDurationRemaining = 0;

			_volume = _fadeTargetVolume;
			_pan = _fadeTargetPan;
		}

		g_system->getMixer()->setChannelVolume(_handle, _volume * Audio::Mixer::kMaxChannelVolume);
		g_system->getMixer()->setChannelBalance(_handle, _pan * 127);
	}
}

uint32 Sound::getStockSoundType() const {
	return _stockSoundType;
}

void Sound::changeVolumePan(int32 volume, int32 pan, int32 duration) {
	if (isPlaying()) {
		_fadeDurationRemaining = duration;

		if (_fadeDurationRemaining > 0) {
			_fadeTargetVolume = volume / 100.0f;
			_fadeTargetPan = pan / 100.0f;
		} else {
			_volume = volume / 100.0f;
			_pan = pan / 100.0f;

			g_system->getMixer()->setChannelVolume(_handle, _volume * Audio::Mixer::kMaxChannelVolume);
			g_system->getMixer()->setChannelBalance(_handle, _pan * 127);
		}
	} else {
		if (_fadeDurationRemaining == 0) {
			_volume = volume / 100.0f;
			_pan = pan / 100.0f;
		}
	}
}

void Sound::saveLoadCurrent(ResourceSerializer *serializer) {
	bool playing = isPlaying();
	serializer->syncAsUint32LE(playing);

	if (_subType != kSoundBackground && playing) {
		uint32 elapsed = g_system->getMixer()->getSoundElapsedTime(_handle);
		serializer->syncAsUint32LE(elapsed);
		serializer->syncAsFloat(_volume);
		serializer->syncAsFloat(_pan);
		serializer->syncAsUint32LE(_fadeDurationRemaining);
		serializer->syncAsFloat(_fadeTargetVolume);
		serializer->syncAsFloat(_fadeTargetPan);

		if (serializer->isLoading()) {
			play();
			// TODO: Seek to the "elapsed" position
		}
	}
}

void Sound::onEnginePause(bool pause) {
	g_system->getMixer()->pauseHandle(_handle, pause);
}

void Sound::setStopOnDestroy(bool stopOnDestroy) {
	_shouldStopOnDestroy = stopOnDestroy;
}

} // End of namespace Resources
} // End of namespace Stark
