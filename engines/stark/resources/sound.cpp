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
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/services.h"

namespace Stark {
namespace Resources {

Sound::~Sound() {
}

Sound::Sound(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_enabled(0),
		_looping(0),
		_field_64(0),
		_playUntilComplete(0),
		_maxDuration(0),
		_stockSoundType(0),
		_field_6C(0),
		_soundType(0),
		_pan(0),
		_volume(0) {
	_type = TYPE;
}

Audio::RewindableAudioStream *Sound::makeAudioStream() {
	// Get the archive loader service
	ArchiveLoader *archiveLoader = StarkServices::instance().archiveLoader;

	Audio::RewindableAudioStream *audioStream = nullptr;

	// First try the .iss / isn files
	Common::SeekableReadStream *stream = archiveLoader->getExternalFile(_filename, _archiveName);
	if (stream) {
		audioStream = makeISSStream(stream, DisposeAfterUse::YES);
	}

#ifdef USE_VORBIS
	if (!audioStream) {
		// The 2 CD version uses Ogg Vorbis
		Common::String filename = _filename;
		if (_filename.hasSuffix(".iss") || _filename.hasSuffix(".isn")) {
			filename = Common::String(_filename.c_str(), _filename.size() - 4) + ".ovs";
		}

		stream = archiveLoader->getExternalFile(filename, _archiveName);
		if (stream) {
			audioStream = Audio::makeVorbisStream(stream, DisposeAfterUse::YES);
		}
	}
#endif

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

	g_system->getMixer()->playStream(getMixerSoundType(), &_handle, playStream, -1, _volume * Audio::Mixer::kMaxChannelVolume);
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
	stop();
}

void Sound::readData(XRCReadStream *stream) {
	_filename = stream->readString();
	_enabled = stream->readUint32LE();
	_looping = stream->readUint32LE();
	_field_64 = stream->readUint32LE();
	_playUntilComplete = stream->readUint32LE();
	_maxDuration = stream->readUint32LE();
	stream->readUint32LE(); // Skipped ?
	_stockSoundType = stream->readUint32LE();
	_soundName = stream->readString();
	_field_6C = stream->readUint32LE();
	_soundType = stream->readUint32LE();
	_pan = stream->readUint32LE();
	_volume = stream->readFloat();
	_archiveName = stream->getArchiveName();
}

void Sound::printData() {
	debug("filename: %s", _filename.c_str());
	debug("enabled: %d", _enabled);
	debug("looping: %d", _looping);
	debug("field_64: %d", _field_64);
	debug("playUntilComplete: %d", _playUntilComplete);
	debug("maxDuration: %d", _maxDuration);
	debug("stockSoundType: %d", _stockSoundType);
	debug("soundName: %s", _soundName.c_str());
	debug("field_6C: %d", _field_6C);
	debug("soundType: %d", _soundType);
	debug("pan: %d", _pan);
	debug("volume: %f", _volume);
}

} // End of namespace Resources
} // End of namespace Stark
