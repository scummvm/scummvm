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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"

#include "hollywood/debug.h"
#include "hollywood/runaway/audio.h"

namespace Hollywood {

RunawayAudio::~RunawayAudio() {
	stopMusic();
	stopSpeech();
	stopEffect();
}

uint32 RunawayAudio::play(const char *archive, uint16 cue, uint tableCount, bool pairedTables,
		byte flags, Audio::Mixer::SoundType type, Audio::SoundHandle &handle, byte volume, bool loop) {
	_mixer->stopHandle(handle);
	const uint headerSize = tableCount * (pairedTables ? 8 : 4);
	Common::File *file = new Common::File();
	if (cue >= tableCount - (pairedTables ? 0 : 1) || !file->open(archive) || file->size() < headerSize) {
		warning("Runaway demo: cannot read %s cue %u", archive, cue);
		delete file;
		return 0;
	}
	file->seek(cue * 4);
	const uint32 start = file->readUint32LE();
	uint32 size;
	if (pairedTables) {
		file->seek((tableCount + cue) * 4);
		size = file->readUint32LE();
	} else {
		const uint32 end = file->readUint32LE();
		size = end > start ? end - start : 0;
	}
	const uint frameSize = ((flags & Audio::FLAG_16BITS) ? 2 : 1) * ((flags & Audio::FLAG_STEREO) ? 2 : 1);
	if (file->err() || start < headerSize || !size || start > file->size() ||
			size > file->size() - start || size % frameSize) {
		warning("Runaway demo: invalid %s cue %u range", archive, cue);
		delete file;
		return 0;
	}
	// The DirectSound setup at 0x4051f0 uses 16000 Hz for all three formats:
	// speech: unsigned 8-bit mono; SFX: signed 16-bit mono; music: 16-bit stereo.
	const uint rate = 16000;
	Common::SeekableReadStream *stream = new Common::SeekableSubReadStream(file, start, start + size, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *pcm = Audio::makeRawStream(stream, rate, flags, DisposeAfterUse::YES);
	if (!pcm) {
		delete stream;
		return 0;
	}
	Audio::AudioStream *playback = loop ? Audio::makeLoopingAudioStream(pcm, 0) : pcm;
	_mixer->playStream(type, &handle, playback, -1, volume, 0, DisposeAfterUse::YES);
	const uint32 duration = (uint64)size * 1000 / (rate * frameSize);
	debugC(1, kDebugResources, "Runaway audio: %s cue %u, %u Hz, %u-bit %s, %u ms, offset=%u size=%u",
		archive, cue, rate, (flags & Audio::FLAG_16BITS) ? 16 : 8,
		(flags & Audio::FLAG_STEREO) ? "stereo" : "mono", duration, start, size);
	return duration;
}

void RunawayAudio::playIntroMusic() {
	// The Pendulo logo uses RESOURCE.002 cue 2 on native channel 7, which
	// has the same 16 kHz stereo PCM format as the room music channels.
	play("RESOURCE.002", 2, 100, false, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO,
		Audio::Mixer::kMusicSoundType, _music, Audio::Mixer::kMaxChannelVolume, false);
}

void RunawayAudio::playMusic(uint16 cue) {
	play("RESOURCE.M03", cue, 100, false, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO,
		Audio::Mixer::kMusicSoundType, _music, 96, true);
}

uint32 RunawayAudio::playSpeech(uint16 sample) {
	// DATAACA3 has independent 6000-entry offset and length tables. Its global
	// sample IDs include stale offsets for other chapters, so never derive a
	// length by subtracting adjacent offsets as with the M/S archives.
	return play("DATAACA3.000", sample, 6000, true, Audio::FLAG_UNSIGNED,
		Audio::Mixer::kSpeechSoundType, _speech, Audio::Mixer::kMaxChannelVolume, false);
}

void RunawayAudio::playEffect(uint16 cue, byte volume) {
	play("RESOURCE.S03", cue, 1000, false, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN,
		Audio::Mixer::kSFXSoundType, _effect, volume, false);
}

void RunawayAudio::stopMusic() {
	_mixer->stopHandle(_music);
}

void RunawayAudio::stopSpeech() {
	_mixer->stopHandle(_speech);
}

void RunawayAudio::stopEffect() {
	_mixer->stopHandle(_effect);
}

bool RunawayAudio::musicPlaying() const {
	return _mixer->isSoundHandleActive(_music);
}

bool RunawayAudio::speechPlaying() const {
	return _mixer->isSoundHandleActive(_speech);
}

} // End of namespace Hollywood
