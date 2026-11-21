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
#include "hollywood/runaway/graphics.h"

namespace Hollywood {

RunawayAudio::~RunawayAudio() {
	stopMusic();
	stopSpeech();
	stopEffect();
}

void RunawayAudio::setChapter(byte chapter) {
	assert(chapter >= 1 && chapter <= 6);
	if (_chapter == chapter)
		return;
	stopMusic();
	stopSpeech();
	stopEffect();
	_chapter = chapter;
}

uint32 RunawayAudio::play(const char *archive, uint16 cue, uint tableCount, bool pairedTables,
		byte flags, Audio::Mixer::SoundType type, SoundChannel &channel, byte volume, bool loop, uint rate, const Common::Point *position) {
	channel.stop();
	const uint headerSize = tableCount * (pairedTables ? 8 : 4);
	Common::File *file = new Common::File();
	if (cue >= tableCount - (pairedTables ? 0 : 1) || !file->open(archive) || file->size() < headerSize) {
		warning("Runaway: cannot read %s cue %u", archive, cue);
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
		warning("Runaway: invalid %s cue %u range", archive, cue);
		delete file;
		return 0;
	}
	Common::SeekableReadStream *stream = new Common::SeekableSubReadStream(file, start, start + size, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *pcm = Audio::makeRawStream(stream, rate, flags, DisposeAfterUse::YES);
	if (!pcm) {
		delete stream;
		return 0;
	}
	Audio::AudioStream *playback = loop ? Audio::makeLoopingAudioStream(pcm, 0) : pcm;
	// Retail 0x408120 sets DirectSound's rolloff to 0.0005, with a
	// minimum distance of 1. Positions and the listener share world units.
	channel.play(_mixer, type, playback, volume, _museum ? nullptr : position, _listenerX, 0.0005f);
	const uint32 duration = (uint64)size * 1000 / (rate * frameSize);
	debugC(1, kDebugResources, "Runaway audio: %s cue %u, %u Hz, %u-bit %s, %u ms, offset=%u size=%u",
		archive, cue, rate, (flags & Audio::FLAG_16BITS) ? 16 : 8,
		(flags & Audio::FLAG_STEREO) ? "stereo" : "mono", duration, start, size);
	return duration;
}

void RunawayAudio::playIntroMusic(uint16 cue) {
	// The Pendulo logo uses RESOURCE.002 cue 2 on native channel 7, which
	// has the same 16 kHz stereo PCM format as the room music channels.
	// Cue 4 accompanies the retail narrator's montage.
	play("RESOURCE.002", cue, 100, false, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO,
		Audio::Mixer::kMusicSoundType, _music, Audio::Mixer::kMaxChannelVolume, false, 16000);
}

uint32 RunawayAudio::playMusic(uint16 cue, bool loop) {
	// Retail/cabin PCM is 16 kHz; the old museum demo uses 22.05 kHz music.
	const Common::String archive = Common::String::format("RESOURCE.M%02u", _museum ? 4 : _chapter);
	return play(archive.c_str(), cue, 100, false, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO,
		Audio::Mixer::kMusicSoundType, _music, 96, loop, _museum ? 22050 : 16000);
}

void RunawayAudio::setMusicVolume(byte volume) {
	_music.setVolume(volume);
}

void RunawayAudio::setListenerPosition(int x) {
	_listenerX = x;
	_speech.setListenerPosition(x);
	for (uint i = 0; i < ARRAYSIZE(_footsteps); ++i) {
		_footsteps[i].setListenerPosition(x);
	}
	for (uint i = 0; i < ARRAYSIZE(_effects); ++i) {
		_effects[i].setListenerPosition(x);
	}
}

void RunawayAudio::playMusicEffect(uint16 cue, byte volume, byte channel, bool loop) {
	assert(channel < ARRAYSIZE(_effects));
	// Notes and some ambient effects are stereo samples in the music archive.
	const Common::String archive = Common::String::format("RESOURCE.M%02u", _chapter);
	play(archive.c_str(), cue, 100, false, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO,
		Audio::Mixer::kSFXSoundType, _effects[channel], volume, loop, 16000);
}

uint32 RunawayAudio::playSpeech(uint16 sample, bool global) {
	if (_museum)
		return sample ? play("RESOURCE.004", sample, 4000, false, Audio::FLAG_UNSIGNED,
			Audio::Mixer::kSpeechSoundType, _speech, Audio::Mixer::kMaxChannelVolume, false, 22050) : 0;
	// DATAACA archives have independent 6000-entry offset and length tables. Their global
	// sample IDs include stale offsets for other chapters, so never derive a
	// length by subtracting adjacent offsets as with the M/S archives.
	const Common::String archive = Common::String::format("DATAACA%u.000", global ? 0 : _chapter);
	// 0x4139b0 / 0x414110 place dialogue at the camera center when it starts,
	// two units in front of the listener, independently of the speaker.
	const Common::Point position(_listenerX, 2);
	return play(archive.c_str(), sample, 6000, true, Audio::FLAG_UNSIGNED,
		Audio::Mixer::kSpeechSoundType, _speech, Audio::Mixer::kMaxChannelVolume, false, 16000, &position);
}

void RunawayAudio::playBackgroundSpeech(uint16 sample, byte channel) {
	assert(channel < ARRAYSIZE(_effects));
	const Common::String archive = Common::String::format("DATAACA%u.000", _chapter);
	play(archive.c_str(), sample, 6000, true, Audio::FLAG_UNSIGNED,
		Audio::Mixer::kSpeechSoundType, _effects[channel], Audio::Mixer::kMaxChannelVolume, false, 16000);
}

void RunawayAudio::playEffect(uint16 cue, byte volume, byte channel, bool loop) {
	playEffect(cue, volume, channel, loop, nullptr);
}

void RunawayAudio::playEffectAt(uint16 cue, Common::Point position, byte volume, byte channel, bool loop) {
	playEffect(cue, volume, channel, loop, &position);
}

void RunawayAudio::playEffect(uint16 cue, byte volume, byte channel, bool loop, const Common::Point *position) {
	assert(channel < ARRAYSIZE(_effects));
	// Museum effects use unsigned 8-bit PCM (native 0x43ae40).
	byte flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
	if (_museum)
		flags = Audio::FLAG_UNSIGNED;
	const Common::String archive = Common::String::format("RESOURCE.S%02u", _museum ? 4 : _chapter);
	play(archive.c_str(), cue, 1000, false, flags,
		Audio::Mixer::kSFXSoundType, _effects[channel], volume, loop, _museum ? 11025 : 16000, position);
}

void RunawayAudio::playFootstep(byte surface, byte variant, byte volume, int x) {
	if (_museum)
		return;
	assert(surface >= 1 && surface <= 7 && variant < 2);
	// Retail 0x40a370 / cabin demo 0x407410 select one of two mono samples
	// per surface, starting at cue 21 in the shared music archive.
	// Each sample has its own playback buffer, so different steps can overlap.
	const uint footstep = (surface - 1) * 2 + variant;
	const Common::Point position(x, 200);
	play("RESOURCE.002", 21 + footstep, 100, false,
		Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, Audio::Mixer::kSFXSoundType, _footsteps[footstep], volume, false, 16000,
		x >= 0 ? &position : nullptr);
}

void RunawayAudio::stopMusic() {
	_music.stop();
}

void RunawayAudio::playPointer() {
	if (_museum)
		return;
	// Native 0x405d14 / 0x407530: RESOURCE.000 chunk 150, 16 kHz,
	// signed 16-bit mono. This short effect accompanies a cursor change.
	Common::Array<byte> data;
	if (!readRunawayGlobalChunk(150, data) || data.size() % 2)
		return;
	byte *samples = (byte *)malloc(data.size());
	if (!samples) {
		warning("Runaway demo: cannot allocate pointer sound buffer");
		return;
	}
	memcpy(samples, data.data(), data.size());
	_pointer.stop();
	_pointer.play(_mixer, Audio::Mixer::kSFXSoundType,
		Audio::makeRawStream(samples, data.size(), 16000, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN), 3);
}

void RunawayAudio::playMenuMusic() {
	if (_museum) {
		playMusic(11);
		return;
	}
	// Options volume preview and credits use shared music cue 1, channel 7.
	play("RESOURCE.002", 1, 100, false, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO,
		Audio::Mixer::kMusicSoundType, _music, 255, true, 16000);
}

void RunawayAudio::stopSpeech() {
	_speech.stop();
}

void RunawayAudio::stopEffect() {
	_pointer.stop();
	for (uint i = 0; i < ARRAYSIZE(_footsteps); ++i) {
		_footsteps[i].stop();
	}
	for (uint i = 0; i < ARRAYSIZE(_effects); ++i)
		_effects[i].stop();
}

void RunawayAudio::stopEffect(byte channel) {
	assert(channel < ARRAYSIZE(_effects));
	_effects[channel].stop();
}

bool RunawayAudio::musicPlaying() const {
	return _music.isPlaying();
}

bool RunawayAudio::speechPlaying() const {
	return _speech.isPlaying();
}

bool RunawayAudio::effectPlaying(byte channel) const {
	assert(channel < ARRAYSIZE(_effects));
	return _effects[channel].isPlaying();
}

} // End of namespace Hollywood
