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

#include "sounds.h"
#include "rooms.h"
#include "alcachofa.h"

#include "common/file.h"
#include "common/substream.h"
#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/raw.h"

using namespace Common;
using namespace Audio;

namespace Alcachofa {

void Sounds::Playback::fadeOut(uint32 duration) {
	_fadeStart = g_system->getMillis();
	_fadeDuration = MAX<uint32>(duration, 1);
}

Sounds::Sounds() : _mixer(g_system->getMixer()) {
	assert(_mixer != nullptr);
}

Sounds::~Sounds() {
	_mixer->stopAll();
}

Sounds::Playback *Sounds::getPlaybackById(SoundID id) {
	if (_playbacks.empty())
		return nullptr;
	uint first = 0, last = _playbacks.size() - 1;
	while (first < last) {
		uint mid = (first + last) / 2;
		if (_playbacks[mid]._id == id)
			return _playbacks.data() + mid;
		else if (_playbacks[mid]._id < id)
			first = mid + 1;
		else
			last = mid == 0 ? 0 : mid - 1;
	}
	return first == last && first < _playbacks.size()
		? _playbacks.data() + first
		: nullptr;
}

void Sounds::update() {
	for (uint i = _playbacks.size(); i > 0; i--) {
		Playback &playback = _playbacks[i - 1];
		if (!_mixer->isSoundHandleActive(playback._handle))
			_playbacks.erase(_playbacks.begin() + i - 1);
		else if (playback._fadeDuration != 0) {
			if (g_system->getMillis() >= playback._fadeStart + playback._fadeDuration) {
				_mixer->stopHandle(playback._handle);
				_playbacks.erase(_playbacks.begin() + i - 1);
			}
			else {
				byte newVolume = (g_system->getMillis() - playback._fadeStart) * Mixer::kMaxChannelVolume / playback._fadeDuration;
				_mixer->setChannelVolume(playback._handle, Mixer::kMaxChannelVolume - newVolume);
			}
		}
	}
}

static AudioStream *loadSND(File *file) {
	// SND files are just WAV files with removed headers
	const uint32 endOfFormat = file->readUint32LE() + 2 * sizeof(uint32);
	if (endOfFormat < 24)
		error("Invalid SND format size");
	uint16 format = file->readUint16LE();
	uint16 channels = file->readUint16LE();
	uint32 freq = file->readUint32LE();
	file->skip(sizeof(uint32)); // bytesPerSecond, unnecessary for us
	uint16 bytesPerBlock = file->readUint16LE();
	uint16 bitsPerSample = file->readUint16LE();
	if (endOfFormat >= 2 * sizeof(uint32) + 20) {
		file->skip(sizeof(uint16)); // size of extra data
		uint16 extra = file->readUint16LE();
		bytesPerBlock = 4 * channels * ((extra + 14) / 8);
	}
	file->seek(endOfFormat, SEEK_SET);
	auto subStream = new SeekableSubReadStream(file, (uint32)file->pos(), (uint32)file->size(), DisposeAfterUse::YES);
	if (format == 1 && channels <= 2 && (bitsPerSample == 8 || bitsPerSample == 16))
		return makeRawStream(subStream, (int)freq,
			(channels == 2 ? FLAG_STEREO : 0) |
			(bitsPerSample == 16 ? FLAG_16BITS | FLAG_LITTLE_ENDIAN : FLAG_UNSIGNED));
	else if (format == 17 && channels <= 2)
		return makeADPCMStream(subStream, DisposeAfterUse::YES, 0, kADPCMMSIma, (int)freq, (int)channels, (uint32)bytesPerBlock);
	else {
		delete file;
		g_engine->game().invalidSNDFormat(format, channels, freq, bitsPerSample);
		return nullptr;
	}
}

static AudioStream *openAudio(const String &fileName) {
	String path = String::format("Sonidos/%s.SND", fileName.c_str());
	File *file = new File();
	if (file->open(path.c_str()))
		return file->size() == 0
			? makeSilentAudioStream(8000, false) // Movie Adventure has some null-size audio files, they are treated like infinite samples
			: loadSND(file);
	path.setChar('W', path.size() - 3);
	path.setChar('A', path.size() - 2);
	path.setChar('V', path.size() - 1);
	if (file->open(path.c_str()))
		return makeWAVStream(file, DisposeAfterUse::YES);
	delete file;

	g_engine->game().missingSound(fileName);
	return nullptr;
}

SoundID Sounds::playSoundInternal(const String &fileName, byte volume, Mixer::SoundType type) {
	AudioStream *stream = openAudio(fileName);
	if (stream == nullptr)
		return UINT32_MAX;

	Array<int16> samples;
	SeekableAudioStream *seekStream = dynamic_cast<SeekableAudioStream *>(stream);
	if (type == Mixer::kSpeechSoundType && seekStream != nullptr) {
		// for lip-sync we need access to the samples so we decode the entire stream now
		int sampleCount = seekStream->getLength().totalNumberOfFrames();
		if (sampleCount > 0) {
			// we actually got a length
			samples.resize((uint)sampleCount);
			sampleCount = seekStream->readBuffer(samples.data(), sampleCount);
			if (sampleCount < 0)
				samples.clear();
			samples.resize((uint)sampleCount); // we might have gotten less samples
		}
		else {
			// we did not, now it is getting inefficient
			const int bufferSize = 2048;
			int16 buffer[bufferSize];
			int chunkSampleCount;
			do {
				chunkSampleCount = seekStream->readBuffer(buffer, bufferSize);
				if (chunkSampleCount <= 0)
					break;
				samples.resize(samples.size() + chunkSampleCount);
				copy(buffer, buffer + chunkSampleCount, samples.data() + sampleCount);
				sampleCount += chunkSampleCount;
			} while (chunkSampleCount >= bufferSize);
		}

		if (sampleCount > 0) {
			stream = makeRawStream(
				(byte *)samples.data(),
				samples.size() * sizeof(int16),
				seekStream->getRate(),
				FLAG_16BITS |
#ifdef SCUMM_LITTLE_ENDIAN
				FLAG_LITTLE_ENDIAN | // readBuffer returns native endian
#endif
				(seekStream->isStereo() ? FLAG_STEREO : 0),
				DisposeAfterUse::NO);
			delete seekStream;
		}
	}

	Playback playback;
	_mixer->playStream(type, &playback._handle, stream, -1, volume);
	playback._id = _nextID++;
	playback._type = type;
	playback._inputRate = stream->getRate();
	playback._samples = std::move(samples);
	_playbacks.push_back(std::move(playback));
	return playback._id;
}

SoundID Sounds::playVoice(const String &fileName, byte volume) {
	return playSoundInternal(fileName, volume, Mixer::kSpeechSoundType);
}

SoundID Sounds::playSFX(const String &fileName, byte volume) {
	return playSoundInternal(fileName, volume, Mixer::kSFXSoundType);
}

void Sounds::stopVoice() {
	for (uint i = _playbacks.size(); i > 0; i--) {
		if (_playbacks[i - 1]._type == Mixer::kSpeechSoundType) {
			_mixer->stopHandle(_playbacks[i - 1]._handle);
			_playbacks.erase(_playbacks.begin() + i - 1);
		}
	}
}

bool Sounds::isAlive(SoundID id) {
	Playback *playback = getPlaybackById(id);
	return playback != nullptr && _mixer->isSoundHandleActive(playback->_handle);
}

void Sounds::setVolume(SoundID id, byte volume) {
	Playback *playback = getPlaybackById(id);
	if (playback != nullptr)
		_mixer->setChannelVolume(playback->_handle, volume);
}

void Sounds::setAppropriateVolume(SoundID id,
	MainCharacterKind processCharacterKind,
	Character *speakingCharacter) {
	static constexpr byte kAlmostMaxVolume = Mixer::kMaxChannelVolume * 9 / 10;

	auto &player = g_engine->player();
	auto processCharacter = processCharacterKind == MainCharacterKind::None ? nullptr
		: &g_engine->world().getMainCharacterByKind(processCharacterKind);
	byte newVolume;
	if (processCharacter == nullptr || processCharacter == player.activeCharacter())
		newVolume = Mixer::kMaxChannelVolume;
	else if (speakingCharacter != nullptr && speakingCharacter->room() == player.currentRoom())
		newVolume = kAlmostMaxVolume;
	else if (processCharacter->room() == player.currentRoom())
		newVolume = kAlmostMaxVolume;
	else
		newVolume = 0;
	setVolume(id, newVolume);
}

void Sounds::fadeOut(SoundID id, uint32 duration) {
	Playback *playback = getPlaybackById(id);
	if (playback != nullptr)
		playback->fadeOut(duration);
}

void Sounds::fadeOutVoiceAndSFX(uint32 duration) {
	for (auto &playback : _playbacks) {
		if (playback._type == Mixer::kSpeechSoundType || playback._type == Mixer::kSFXSoundType)
			playback.fadeOut(duration);
	}
}

bool Sounds::isNoisy(SoundID id, float windowSize, float minDifferences) {
	assert(windowSize > 0 && minDifferences > 0);
	const Playback *playback = getPlaybackById(id);
	if (playback == nullptr ||
		playback->_samples.empty() ||
		!_mixer->isSoundHandleActive(playback->_handle))
		return false;

	minDifferences *= windowSize;
	uint windowSizeInSamples = (uint)(windowSize * 0.001f * playback->_inputRate);
	uint samplePosition = (uint)_mixer->getElapsedTime(playback->_handle)
		.convertToFramerate(playback->_inputRate)
		.totalNumberOfFrames();
	uint endPosition = MIN(playback->_samples.size(), samplePosition + windowSizeInSamples);
	if (samplePosition >= endPosition)
		return false;

	/* While both ScummVM and the original engine use signed int16 samples
	 * For this noise detection the samples are reinterpret as uint16
	 * This causes changes going through zero to be much more significant.
	 */
	float sumOfDifferences = 0;
	const uint16 *samplePtr = (const uint16 *)playback->_samples.data();
	for (uint i = samplePosition; i < endPosition - 1; i++)
		// cast to int before to not be constrained by uint16
		sumOfDifferences += ABS((int)samplePtr[i + 1] - samplePtr[i]);

	return sumOfDifferences / 256.0f >= minDifferences;
}

PlaySoundTask::PlaySoundTask(Process &process, SoundID soundID)
	: Task(process)
	, _soundID(soundID) {
}

TaskReturn PlaySoundTask::run() {
	auto &sounds = g_engine->sounds();
	if (sounds.isAlive(_soundID))
	{
		sounds.setAppropriateVolume(_soundID, process().character(), nullptr);
		return TaskReturn::yield();
	}
	else
		return TaskReturn::finish(1);
}

void PlaySoundTask::debugPrint() {
	g_engine->console().debugPrintf("PlaySound %u\n", _soundID);
}

}
