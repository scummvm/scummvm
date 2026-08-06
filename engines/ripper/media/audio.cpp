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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/media.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "common/debug.h"
#include "common/system.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const uint kBlockingAudioCursor = 0x13;

} // End of anonymous namespace

bool MediaPlayer::playBlockingAudio(const Common::String &path) {
	Common::String source;
	Common::SeekableReadStream *audioStream =
		openSource(path, kSourceBlockingAudio, source);
	if (!audioStream) {
		warning("Ripper: could not open blocking audio '%s' from the filesystem or sound library",
			path.c_str());
		return false;
	}
	Audio::SeekableAudioStream *stream = Audio::makeWAVStream(audioStream, DisposeAfterUse::YES);
	if (!stream)
		return false;

	Audio::SoundHandle handle;
	// PlayBlockingAudioClip at 0x1f0ea is part of the same presentation path as
	// packetized dialogue/video audio, which the Remote Control names VIDEO VOL.
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &handle, stream);
	_engine->getCursor()->update(kBlockingAudioCursor);
	g_system->updateScreen();
	debugC(2, kDebugAudio,
		"Ripper: started blocking audio '%s' source=%s cursor=%u input=keyboard-only presentation=serviced",
		path.c_str(), source.c_str(), kBlockingAudioCursor);
	bool stoppedByEscape = false;
	while (!_engine->shouldQuit() && _mixer->isSoundHandleActive(handle)) {
		_engine->getCursor()->update(kBlockingAudioCursor);
		if (_input->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_input->hasPendingKey()) {
			const uint16 command = _input->consumeKey();
			if (command == 0x1b) {
				stoppedByEscape = true;
				break;
			}
		}
		if (stoppedByEscape)
			break;
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	_mixer->stopHandle(handle);
	_input->discardMouseTransitions();
	debugC(2, kDebugAudio,
		"Ripper: completed blocking audio '%s' source=%s stoppedByEscape=%d",
		path.c_str(), source.c_str(), stoppedByEscape);
	return !_engine->shouldQuit();
}

bool MediaPlayer::playSoundEffect(const Common::String &path, Audio::SoundHandle &handle,
		uint volumePercent, bool loop) {
	return playAudioClip(path, handle, Audio::Mixer::kSFXSoundType,
		volumePercent, loop, "sound effect");
}

bool MediaPlayer::playSoundEffectStream(Common::SeekableReadStream *audioStream,
		const Common::String &name, Audio::SoundHandle &handle,
		uint volumePercent, bool loop) {
	return playAudioStream(audioStream, name, "asset library", handle,
		Audio::Mixer::kSFXSoundType, volumePercent, loop,
		"archived sound effect");
}

bool MediaPlayer::playRawSoundEffect(const Common::Array<byte> &data,
		uint sampleRate, byte flags, Audio::SoundHandle &handle,
		uint volumePercent) {
	if (data.empty() || sampleRate == 0)
		return false;

	byte *copy = (byte *)malloc(data.size());
	if (!copy)
		return false;
	memcpy(copy, data.data(), data.size());
	Audio::SeekableAudioStream *stream = Audio::makeRawStream(copy,
		data.size(), sampleRate, flags, DisposeAfterUse::YES);
	if (!stream) {
		free(copy);
		return false;
	}

	stopSoundEffect(handle);
	const byte volume = (byte)(MIN<uint>(volumePercent, 100) *
		Audio::Mixer::kMaxChannelVolume / 100);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, stream, -1, volume);
	debugC(2, kDebugAudio,
		"Ripper: started raw sound effect bytes=%u rate=%u flags=0x%x volume=%u",
		data.size(), sampleRate, flags, volumePercent);
	return true;
}

bool MediaPlayer::playVoiceClip(const Common::String &path, Audio::SoundHandle &handle,
		uint volumePercent) {
	return playAudioClip(path, handle, Audio::Mixer::kSpeechSoundType,
		volumePercent, false, "voice clip");
}

bool MediaPlayer::playVoiceClipStream(Common::SeekableReadStream *audioStream,
		const Common::String &name, Audio::SoundHandle &handle,
		uint volumePercent) {
	return playAudioStream(audioStream, name, "asset library", handle,
		Audio::Mixer::kSpeechSoundType, volumePercent, false,
		"archived voice clip");
}

bool MediaPlayer::playAudioClip(const Common::String &path, Audio::SoundHandle &handle,
		Audio::Mixer::SoundType soundType, uint volumePercent, bool loop,
		const char *description) {
	Common::String source;
	Common::SeekableReadStream *audioStream =
		openSource(path, kSourceSoundEffect, source);
	if (!audioStream) {
		warning("Ripper: could not open %s '%s' from the filesystem or sound library",
			description, path.c_str());
		return false;
	}

	return playAudioStream(audioStream, path, source, handle, soundType,
		volumePercent, loop, description);
}

bool MediaPlayer::playAudioStream(Common::SeekableReadStream *audioStream,
		const Common::String &name, const Common::String &source,
		Audio::SoundHandle &handle, Audio::Mixer::SoundType soundType,
		uint volumePercent, bool loop, const char *description) {
	if (!audioStream)
		return false;
	Audio::SeekableAudioStream *stream = Audio::makeWAVStream(
		audioStream, DisposeAfterUse::YES);
	if (!stream)
		return false;
	stopSoundEffect(handle);
	const byte volume = (byte)(MIN<uint>(volumePercent, 100) * Audio::Mixer::kMaxChannelVolume / 100);
	Audio::AudioStream *playbackStream = stream;
	if (loop)
		playbackStream = Audio::makeLoopingAudioStream(stream, 0);
	_mixer->playStream(soundType, &handle, playbackStream, -1, volume);
	debugC(2, kDebugAudio,
		"Ripper: started %s '%s' source=%s volume=%u loop=%d mixerType=%d",
		description, name.c_str(), source.c_str(), volumePercent, loop, soundType);
	return true;
}

bool MediaPlayer::stopSoundEffect(Audio::SoundHandle &handle) {
	const bool active = _mixer->isSoundHandleActive(handle);
	if (active)
		_mixer->stopHandle(handle);
	return active;
}

void MediaPlayer::setSoundEffectVolume(Audio::SoundHandle &handle, uint volumePercent) {
	_mixer->setChannelVolume(handle,
		(byte)(MIN<uint>(volumePercent, 100) * Audio::Mixer::kMaxChannelVolume / 100));
}

bool MediaPlayer::isSoundEffectActive(const Audio::SoundHandle &handle) const {
	return _mixer->isSoundHandleActive(handle);
}

uint32 MediaPlayer::getSoundEffectElapsedTime(
		const Audio::SoundHandle &handle) const {
	return _mixer->getSoundElapsedTime(handle);
}

} // End of namespace Ripper
