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

#include "alcachofa/sounds.h"
#include "alcachofa/rooms.h"
#include "alcachofa/alcachofa.h"
#include "alcachofa/detection.h"

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

Sounds::Sounds()
	: _mixer(g_system->getMixer())
	, _musicSemaphore("music") {
	assert(_mixer != nullptr);
}

Sounds::~Sounds() {
	_mixer->stopAll();
}

Sounds::Playback *Sounds::getPlaybackById(SoundHandle id) {
	auto itPlayback = find_if(_playbacks.begin(), _playbacks.end(),
		[&] (const Playback &playback) { return playback._handle == id; });
	return itPlayback == _playbacks.end() ? nullptr : itPlayback;
}

void Sounds::update() {
	if (_isMusicPlaying && !isAlive(_musicSoundID)) {
		if (_nextMusicID < 0)
			fadeMusic();
		else
			startMusic(_nextMusicID);
	}

	for (uint i = _playbacks.size(); i > 0; i--) {
		Playback &playback = _playbacks[i - 1];
		if (!_mixer->isSoundHandleActive(playback._handle))
			_playbacks.erase(_playbacks.begin() + i - 1);
		else if (playback._fadeDuration != 0) {
			if (g_system->getMillis() >= playback._fadeStart + playback._fadeDuration) {
				_mixer->stopHandle(playback._handle);
				_playbacks.erase(_playbacks.begin() + i - 1);
			} else {
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
		delete subStream;
		g_engine->game().invalidSNDFormat(format, channels, freq, bitsPerSample);
		return nullptr;
	}
}

static AudioStream *openAudio(const char *fileName) {
	String path = String::format("Sonidos/%s.SND", fileName);
	File *file = new File();
	if (file->open(path.c_str()))
		return file->size() == 0 // Movie Adventure has some null-size audio files, they are treated like infinite silence
			? makeSilentAudioStream(8000, false) 
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

SoundHandle Sounds::playSoundInternal(const char *fileName, byte volume, Mixer::SoundType type) {
	AudioStream *stream = openAudio(fileName);
	if (stream == nullptr && (type == Mixer::kSpeechSoundType || type == Mixer::kMusicSoundType)) {
		/* If voice files are missing, the player could still read the subtitle
		 * For this we return infinite silent audio which the user has to skip
		 * But only do this for speech as there is no skipping for sound effects
		 * so those would live on forever and block up mixer channels
		 * Music is fine as well as we clean up the music playack explicitly
		 */
		stream = makeSilentAudioStream(8000, false);
	}
	if (stream == nullptr)
		return {};

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
		} else {
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

	SoundHandle handle;
	_mixer->playStream(type, &handle, stream, -1, volume);
	Playback playback;
	playback._handle = handle;
	playback._type = type;
	playback._inputRate = stream->getRate();
	playback._samples = Common::move(samples);
	_playbacks.push_back(Common::move(playback));
	return handle;
}

SoundHandle Sounds::playVoice(const String &fileName, byte volume) {
	debugC(1, kDebugSounds, "Play voice: %s at %d", fileName.c_str(), (int)volume);
	return playSoundInternal(fileName.c_str(), volume, Mixer::kSpeechSoundType);
}

SoundHandle Sounds::playSFX(const String &fileName, byte volume) {
	debugC(1, kDebugSounds, "Play SFX: %s at %d", fileName.c_str(), (int)volume);
	return playSoundInternal(fileName.c_str(), volume, Mixer::kSFXSoundType);
}

void Sounds::stopAll() {
	debugC(1, kDebugSounds, "Stop all sounds");
	_mixer->stopAll();
	_playbacks.clear();
}

void Sounds::stopVoice() {
	debugC(1, kDebugSounds, "Stop all voices");
	for (uint i = _playbacks.size(); i > 0; i--) {
		if (_playbacks[i - 1]._type == Mixer::kSpeechSoundType) {
			_mixer->stopHandle(_playbacks[i - 1]._handle);
			_playbacks.erase(_playbacks.begin() + i - 1);
		}
	}
}

void Sounds::pauseAll(bool paused) {
	_mixer->pauseAll(paused);
}

bool Sounds::isAlive(SoundHandle id) {
	Playback *playback = getPlaybackById(id);
	return playback != nullptr && _mixer->isSoundHandleActive(playback->_handle);
}

void Sounds::setVolume(SoundHandle id, byte volume) {
	Playback *playback = getPlaybackById(id);
	if (playback != nullptr)
		_mixer->setChannelVolume(playback->_handle, volume);
}

void Sounds::setAppropriateVolume(SoundHandle id,
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

void Sounds::fadeOut(SoundHandle id, uint32 duration) {
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

bool Sounds::isNoisy(SoundHandle id, float windowSize, float minDifferences) {
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

void Sounds::startMusic(int musicId) {
	debugC(2, kDebugSounds, "startMusic %d", musicId);
	assert(musicId >= 0);
	fadeMusic();
	constexpr size_t kBufferSize = 16;
	char filenameBuffer[kBufferSize];
	snprintf(filenameBuffer, kBufferSize, "T%d", musicId);
	_musicSoundID = playSoundInternal(filenameBuffer, Mixer::kMaxChannelVolume, Mixer::kMusicSoundType);
	_isMusicPlaying = true;
	_nextMusicID = musicId;
}

void Sounds::queueMusic(int musicId) {
	debugC(2, kDebugSounds, "queueMusic %d", musicId);
	_nextMusicID = musicId;
}

void Sounds::fadeMusic(uint32 duration) {
	debugC(2, kDebugSounds, "fadeMusic");
	fadeOut(_musicSoundID, duration);
	_isMusicPlaying = false;
	_nextMusicID = -1;
	_musicSoundID = {};
}

void Sounds::setMusicToRoom(int roomMusicId) {
	// Alcachofa Soft used IDs > 200 to mean "no change in music"
	if (roomMusicId == _nextMusicID || roomMusicId > 200) {
		debugC(1, kDebugSounds, "setMusicToRoom: from %d to %d, not executed", _nextMusicID, roomMusicId);
		return;
	}
	debugC(1, kDebugSounds, "setMusicToRoom: from %d to %d", _nextMusicID, roomMusicId);
	if (roomMusicId > 0)
		startMusic(roomMusicId);
	else
		fadeMusic();
}

Task *Sounds::waitForMusicToEnd(Process &process) {
	return new WaitForMusicTask(process);
}

PlaySoundTask::PlaySoundTask(Process &process, SoundHandle SoundHandle)
	: Task(process)
	, _soundHandle(SoundHandle) {}

PlaySoundTask::PlaySoundTask(Process &process, Serializer &s)
	: Task(process)
	, _soundHandle({}) {
	// playing sounds are not persisted in the savestates,
	// this task will stop at the next frame
	syncGame(s);
}

TaskReturn PlaySoundTask::run() {
	auto &sounds = g_engine->sounds();
	if (sounds.isAlive(_soundHandle)) {
		sounds.setAppropriateVolume(_soundHandle, process().character(), nullptr);
		return TaskReturn::yield();
	} else
		return TaskReturn::finish(1);
}

void PlaySoundTask::debugPrint() {
	// unfortunately SoundHandle is not castable to something we could display here safely
	g_engine->console().debugPrintf("PlaySound\n");
}

DECLARE_TASK(PlaySoundTask)

WaitForMusicTask::WaitForMusicTask(Process &process)
	: Task(process)
	, _lock("wait-for-music", g_engine->sounds().musicSemaphore()) {}

WaitForMusicTask::WaitForMusicTask(Process &process, Serializer &s)
	: Task(process)
	, _lock("wait-for-music", g_engine->sounds().musicSemaphore()) {
	syncGame(s);
}

TaskReturn WaitForMusicTask::run() {
	g_engine->sounds().queueMusic(-1);
	return g_engine->sounds().isMusicPlaying()
		? TaskReturn::yield()
		: TaskReturn::finish(0);
}

void WaitForMusicTask::debugPrint() {
	g_engine->console().debugPrintf("WaitForMusic\n");
}

DECLARE_TASK(WaitForMusicTask)

}
