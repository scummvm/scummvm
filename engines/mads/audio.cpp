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

#include "mads/audio.h"
#include "mads/compression.h"

#include "common/stream.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

namespace MADS {

AudioPlayer::AudioPlayer(Audio::Mixer *mixer, uint32 gameID) : _mixer(mixer), _gameID(gameID) {
	setVolume(Audio::Mixer::kMaxChannelVolume);
	setDefaultSoundGroup();
}

AudioPlayer::~AudioPlayer() {
	_dsrEntries.clear();
	_filename = "";
}

bool AudioPlayer::isPlaying() const {
	return _mixer->isSoundHandleActive(_handle);
}

void AudioPlayer::setVolume(int volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume);
}

void AudioPlayer::setDefaultSoundGroup() {
	switch (_gameID) {
	case GType_RexNebular:
		setSoundGroup("rex009.dsr");
		break;
	case GType_Dragonsphere:
		setSoundGroup("drag009.dsr");
		break;
	case GType_Phantom:
		setSoundGroup("phan009.dsr");
		break;
	default:
		error("setDefaultSoundGroup: Unknown game");
	}
}

void AudioPlayer::setSoundGroup(const Common::String &filename) {
	if (_filename != filename) {
		_dsrEntries.clear();

		_filename = filename;
		_dsrFile.open(filename);

		// Read header
		uint16 entryCount = _dsrFile.readUint16LE();

		for (uint16 i = 0; i < entryCount; i++) {
			DSREntry newEntry;
			newEntry.frequency = _dsrFile.readUint16LE();
			newEntry.channels = _dsrFile.readUint32LE();
			newEntry.compSize = _dsrFile.readUint32LE();
			newEntry.uncompSize = _dsrFile.readUint32LE();
			newEntry.offset = _dsrFile.readUint32LE();
			_dsrEntries.push_back(newEntry);
		}

		_dsrFile.close();
	}
}

void AudioPlayer::playSound(int soundIndex, bool loop) {
	if (_dsrEntries.empty()) {
		warning("DSR file not loaded, not playing sound");
		return;
	}

	if (soundIndex < 0 || soundIndex > (int)_dsrEntries.size() - 1) {
		warning("Invalid sound index: %i (max %i), not playing sound", soundIndex, _dsrEntries.size() - 1);
		return;
	}

	// Get sound data
	FabDecompressor fab;
	int32 compSize = _dsrEntries[soundIndex].compSize;
	int32 uncompSize = _dsrEntries[soundIndex].uncompSize;
	int32 offset = _dsrEntries[soundIndex].offset;
	int16 frequency = _dsrEntries[soundIndex].frequency;
	byte *compData = new byte[compSize];
	byte *buffer = new byte[uncompSize];
	_dsrFile.open(_filename);
	_dsrFile.seek(offset, SEEK_SET);
	_dsrFile.read(compData, compSize);
	_dsrFile.close();

	fab.decompress(compData, compSize, buffer, uncompSize);

	// Play sound
	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
				Audio::makeRawStream(buffer, uncompSize, frequency, Audio::FLAG_UNSIGNED),
				loop ? 0 : 1);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handle, stream, -1,  Audio::Mixer::kMaxChannelVolume);

	/*
	// Dump the sound file
	FILE *destFile = fopen("sound.raw", "wb");
	fwrite(_dsrFile.dsrEntries[soundIndex]->data, _dsrFile.dsrEntries[soundIndex].uncompSize, 1, destFile);
	fclose(destFile);
	*/
}

void AudioPlayer::stop() {
	_mixer->stopHandle(_handle);
}

} // End of namespace M4
