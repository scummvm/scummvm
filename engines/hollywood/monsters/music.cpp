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
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/util.h"

#include "hollywood/debug.h"
#include "hollywood/monsters/hollywood.h"
#include "hollywood/monsters/music.h"

namespace Hollywood {

const char *const kIntroMusicArchiveName = "RESOURCE.M09";
const uint16 kIntroMusicCueId = 0x000d;
const char *const kSpeechArchiveName = "RESOURCE.004";
const char *const kIntroSoundBank0ArchiveName = "RESOURCE.S09";
const uint kMusicCueTableSize = 0x190;
const int kMusicSampleRate = 11025;
const uint kSpeechCueTableSize = 0x3e80;
const int kSpanishSpeechSampleRate = 22050;
const int kItalianSpeechSampleRate = 11025;
const uint kSoundBank0CueTableSize = 0xfa0;
const int kSoundBank0SampleRate = 11025;
const char *const kResidentSoundArchiveName = "RESOURCE.000";
const uint kResource000HeaderByteCount = 1;
const uint kResource000TableByteCount = 400;
const int kResidentSoundSampleRate = 11025;

// Resource entries for effect IDs 1-16; repeats mirror the original buffer aliases.
const byte kResidentSoundResourceEntries[] = {
	0x55, 0x56, 0x57, 0x58, 0x55, 0x59, 0x55, 0x5a,
	0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62
};

const byte kSpanishDemoResidentSoundResourceEntries[] = {
	0x5c, 0x5c, 0x5f, 0x5f, 0x5c, 0x5c, 0x5c, 0x5f,
	0x5f, 0x60, 0x5c, 0x5f, 0x5c, 0x5c, 0x5f, 0x5f
};

byte percentToMixerVolume(byte volumePercent) {
	const uint volume = MIN<uint>(volumePercent, 100);
	return (byte)((volume * Audio::Mixer::kMaxChannelVolume) / 100);
}

MusicPlayer::MusicPlayer() :
		_archiveName(kIntroMusicArchiveName),
		_currentCueId(0),
		_currentVolumePercent(100),
		_currentLoop(false),
		_hasCurrentCue(false) {
}

MusicPlayer::~MusicPlayer() {
	stop();
}

void MusicPlayer::setArchive(const Common::Path &archiveName) {
	if (_archiveName == archiveName)
		return;

	stop();
	_archiveName = archiveName;
	_currentCueId = 0;
	_currentVolumePercent = 100;
	_currentLoop = false;
	_hasCurrentCue = false;
}

bool MusicPlayer::playIntroMusic() {
	setArchive(Common::Path(kIntroMusicArchiveName));
	return playMusicCue(kIntroMusicCueId);
}

bool MusicPlayer::playMusicCue(uint16 cueId, byte volumePercent, bool loop) {
	stop();

	uint32 start = 0;
	uint32 size = 0;
	if (!readCueSpan(_archiveName, cueId, start, size))
		return false;

	Common::File *file = new Common::File();
	if (!file->open(_archiveName)) {
		warning("Failed to open %s", _archiveName.toString().c_str());
		delete file;
		return false;
	}

	Common::SeekableReadStream *sampleStream = new Common::SeekableSubReadStream(file, start, start + size, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(sampleStream, kMusicSampleRate,
		Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Failed to create raw stream for %s cue %u", _archiveName.toString().c_str(), cueId);
		delete sampleStream;
		return false;
	}

	Audio::AudioStream *playbackStream = loop ? Audio::makeLoopingAudioStream(audioStream, 0) : audioStream;
	_musicChannel.play(g_system->getMixer(), Audio::Mixer::kMusicSoundType, playbackStream,
		percentToMixerVolume(volumePercent));

	_currentCueId = cueId;
	_currentVolumePercent = volumePercent;
	_currentLoop = loop;
	_hasCurrentCue = true;

	debugC(1, kDebugResources, "Started music %s cue %u: offset=%u size=%u loop=%u",
		_archiveName.toString().c_str(), cueId, start, size, loop ? 1 : 0);
	return true;
}

bool MusicPlayer::resumeLastCue() {
	if (!_hasCurrentCue)
		return false;

	return playMusicCue(_currentCueId, _currentVolumePercent, _currentLoop);
}

void MusicPlayer::setVolume(byte volumePercent) {
	_currentVolumePercent = volumePercent;
	_musicChannel.setVolume(percentToMixerVolume(volumePercent));
}

void MusicPlayer::stop() {
	_musicChannel.stop();
}

bool MusicPlayer::isPlaying() const {
	return _musicChannel.isPlaying();
}

bool MusicPlayer::readCueSpan(const Common::Path &fileName, uint16 cueId, uint32 &start, uint32 &size) const {
	if ((cueId + 1U) * 4 >= kMusicCueTableSize) {
		warning("Invalid %s cue id %u", fileName.toString().c_str(), cueId);
		return false;
	}

	Common::File file;
	if (!file.open(fileName)) {
		warning("Failed to open %s", fileName.toString().c_str());
		return false;
	}

	if (file.size() < (int32)kMusicCueTableSize) {
		warning("%s is too small for the cue table", fileName.toString().c_str());
		return false;
	}

	file.seek(cueId * 4);
	start = file.readUint32LE();
	const uint32 end = file.readUint32LE();

	if (start >= end || end > (uint32)file.size()) {
		warning("Invalid %s cue %u span: start=%u end=%u fileSize=%u",
			fileName.toString().c_str(), cueId, start, end, (uint)file.size());
		return false;
	}

	size = end - start;
	if (size & 1)
		size--;

	if (size == 0) {
		warning("%s cue %u is empty", fileName.toString().c_str(), cueId);
		return false;
	}

	return true;
}

SpeechPlayer::SpeechPlayer(Common::Language language, bool enabled) :
		_sampleRate(language == Common::IT_ITA ? kItalianSpeechSampleRate : kSpanishSpeechSampleRate),
		_rawFlags(language == Common::IT_ITA ? Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN : Audio::FLAG_UNSIGNED),
		_bytesPerSample(language == Common::IT_ITA ? 2 : 1),
		_lastSampleDurationMillis(0),
		_enabled(enabled) {
}

SpeechPlayer::~SpeechPlayer() {
	stop();
}

bool SpeechPlayer::playSampleAt(uint16 sampleId, int worldX, int viewportX, byte volumePercent) {
	const Common::Point position(worldX, HollywoodEngine::kScreenWidth / 2);
	return playSample(sampleId, volumePercent, false,
		ConfMan.getBool("positional_audio") ? &position : nullptr,
		viewportX + HollywoodEngine::kScreenWidth / 2);
}

void SpeechPlayer::setViewportPosition(int viewportX) {
	_speechChannel.setListenerPosition(viewportX + HollywoodEngine::kScreenWidth / 2);
}

bool SpeechPlayer::playSample(uint16 sampleId, byte volumePercent, bool loop,
		const Common::Point *position, int listenerX) {
	stop();
	_lastSampleDurationMillis = 0;
	if (!_enabled)
		return false;

	uint32 start = 0;
	uint32 size = 0;
	if (!readSampleSpan(sampleId, start, size))
		return false;

	Common::File *file = new Common::File();
	if (!file->open(Common::Path(kSpeechArchiveName))) {
		warning("Failed to open %s", kSpeechArchiveName);
		delete file;
		return false;
	}

	Common::SeekableReadStream *sampleStream = new Common::SeekableSubReadStream(file, start, start + size, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(sampleStream, _sampleRate,
		_rawFlags, DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Failed to create raw stream for %s sample %u", kSpeechArchiveName, sampleId);
		delete sampleStream;
		return false;
	}

	_lastSampleDurationMillis = (uint32)(((uint64)size * 1000) / (_sampleRate * _bytesPerSample));
	Audio::AudioStream *playbackStream = loop ? Audio::makeLoopingAudioStream(audioStream, 0) : audioStream;
	_speechChannel.play(g_system->getMixer(), Audio::Mixer::kSpeechSoundType, playbackStream,
		percentToMixerVolume(volumePercent), position, listenerX, 0.0f, SoundChannel::kStereoPan);

	debugC(1, kDebugResources,
		"Started speech %s sample %u: offset=%u size=%u format=%u-bit/%uHz duration=%u ms loop=%u",
		kSpeechArchiveName, sampleId, start, size, _bytesPerSample * 8, _sampleRate,
		_lastSampleDurationMillis, loop ? 1 : 0);
	return true;
}

void SpeechPlayer::setVolume(byte volumePercent) {
	_speechChannel.setVolume(percentToMixerVolume(volumePercent));
}

void SpeechPlayer::stop() {
	_speechChannel.stop();
}

bool SpeechPlayer::isPlaying() const {
	return _speechChannel.isPlaying();
}

bool SpeechPlayer::readSampleSpan(uint16 sampleId, uint32 &start, uint32 &size) const {
	if ((sampleId + 1U) * 4 >= kSpeechCueTableSize) {
		warning("Invalid %s sample id %u", kSpeechArchiveName, sampleId);
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kSpeechArchiveName))) {
		warning("Failed to open %s", kSpeechArchiveName);
		return false;
	}

	if (file.size() < (int32)kSpeechCueTableSize) {
		warning("%s is too small for the cue table", kSpeechArchiveName);
		return false;
	}

	file.seek(sampleId * 4);
	start = file.readUint32LE();
	const uint32 end = file.readUint32LE();

	if (start >= end || end > (uint32)file.size()) {
		warning("Invalid %s sample %u span: start=%u end=%u fileSize=%u",
			kSpeechArchiveName, sampleId, start, end, (uint)file.size());
		return false;
	}

	size = end - start;
	return size != 0;
}

SoundBank0Player::SoundBank0Player() :
		_archiveName(kIntroSoundBank0ArchiveName) {
}

SoundBank0Player::~SoundBank0Player() {
	stop();
}

void SoundBank0Player::setArchive(const Common::Path &archiveName) {
	if (_archiveName == archiveName)
		return;

	stop();
	_archiveName = archiveName;
}

bool SoundBank0Player::playSample(uint16 sampleId, byte volumePercent, bool loop) {
	stop();

	uint32 start = 0;
	uint32 size = 0;
	if (!readSampleSpan(sampleId, start, size))
		return false;

	Common::File *file = new Common::File();
	if (!file->open(_archiveName)) {
		warning("Failed to open %s", _archiveName.toString().c_str());
		delete file;
		return false;
	}

	Common::SeekableReadStream *sampleStream = new Common::SeekableSubReadStream(file, start, start + size, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(sampleStream, kSoundBank0SampleRate,
		Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Failed to create raw stream for %s sample %u", _archiveName.toString().c_str(), sampleId);
		delete sampleStream;
		return false;
	}

	Audio::AudioStream *playbackStream = loop ? Audio::makeLoopingAudioStream(audioStream, 0) : audioStream;
	_soundChannel.play(g_system->getMixer(), Audio::Mixer::kSFXSoundType, playbackStream,
		percentToMixerVolume(volumePercent));

	debugC(1, kDebugResources, "Started sound %s sample %u: offset=%u size=%u loop=%u",
		_archiveName.toString().c_str(), sampleId, start, size, loop ? 1 : 0);
	return true;
}

bool SoundBank0Player::playSampleLooping(uint16 sampleId, byte volumePercent) {
	return playSample(sampleId, volumePercent, true);
}

void SoundBank0Player::setVolume(byte volumePercent) {
	_soundChannel.setVolume(percentToMixerVolume(volumePercent));
}

void SoundBank0Player::stop() {
	_soundChannel.stop();
}

bool SoundBank0Player::isPlaying() const {
	return _soundChannel.isPlaying();
}

bool SoundBank0Player::readSampleSpan(uint16 sampleId, uint32 &start, uint32 &size) const {
	if ((sampleId + 1U) * 4 >= kSoundBank0CueTableSize) {
		warning("Invalid %s sample id %u", _archiveName.toString().c_str(), sampleId);
		return false;
	}

	Common::File file;
	if (!file.open(_archiveName)) {
		warning("Failed to open %s", _archiveName.toString().c_str());
		return false;
	}

	if (file.size() < (int32)kSoundBank0CueTableSize) {
		warning("%s is too small for the cue table", _archiveName.toString().c_str());
		return false;
	}

	file.seek(sampleId * 4);
	start = file.readUint32LE();
	const uint32 end = file.readUint32LE();

	if (start >= end || end > (uint32)file.size()) {
		warning("Invalid %s sample %u span: start=%u end=%u fileSize=%u",
			_archiveName.toString().c_str(), sampleId, start, end, (uint)file.size());
		return false;
	}

	size = end - start;
	return size != 0;
}

ResidentSoundEffectPlayer::ResidentSoundEffectPlayer(bool useSpanishDemoLayout) :
		_resourceEntries(useSpanishDemoLayout ?
			kSpanishDemoResidentSoundResourceEntries : kResidentSoundResourceEntries) {
}

ResidentSoundEffectPlayer::~ResidentSoundEffectPlayer() {
	stop();
}

bool ResidentSoundEffectPlayer::playSampleAt(byte soundEffectId, int worldX, int viewportX, byte volumePercent) {
	const Common::Point position(worldX, HollywoodEngine::kScreenWidth / 2);
	return playSample(soundEffectId, volumePercent,
		ConfMan.getBool("positional_audio") ? &position : nullptr,
		viewportX + HollywoodEngine::kScreenWidth / 2);
}

void ResidentSoundEffectPlayer::setViewportPosition(int viewportX) {
	for (uint i = 0; i < ARRAYSIZE(_soundChannels); ++i) {
		_soundChannels[i].setListenerPosition(viewportX + HollywoodEngine::kScreenWidth / 2);
	}
}

bool ResidentSoundEffectPlayer::playSample(byte soundEffectId, byte volumePercent,
		const Common::Point *position, int listenerX) {
	uint32 start = 0;
	uint32 size = 0;
	if (!readSampleSpan(soundEffectId, start, size))
		return false;

	Common::File *file = new Common::File();
	if (!file->open(Common::Path(kResidentSoundArchiveName))) {
		warning("Failed to open %s", kResidentSoundArchiveName);
		delete file;
		return false;
	}

	Common::SeekableReadStream *sampleStream = new Common::SeekableSubReadStream(file,
		start, start + size, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(sampleStream,
		kResidentSoundSampleRate, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN,
		DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Failed to create raw stream for %s resident effect %u",
			kResidentSoundArchiveName, soundEffectId);
		delete sampleStream;
		return false;
	}

	stopSample(soundEffectId);
	_soundChannels[soundEffectId - 1].play(g_system->getMixer(), Audio::Mixer::kSFXSoundType,
		audioStream, percentToMixerVolume(volumePercent), position, listenerX, 0.0f, SoundChannel::kStereoPan);

	debugC(1, kDebugResources, "Started resident sound %s effect %u: offset=%u size=%u",
		kResidentSoundArchiveName, soundEffectId, start, size);
	return true;
}

void ResidentSoundEffectPlayer::stop() {
	for (byte soundEffectId = 1; soundEffectId <= kSoundEffectCount; ++soundEffectId)
		stopSample(soundEffectId);
}

void ResidentSoundEffectPlayer::stopSample(byte soundEffectId) {
	if (soundEffectId > 0 && soundEffectId <= kSoundEffectCount) {
		_soundChannels[soundEffectId - 1].stop();
	}
}

bool ResidentSoundEffectPlayer::readSampleSpan(byte soundEffectId,
		uint32 &start, uint32 &size) const {
	if (soundEffectId == 0)
		return false;
	if (soundEffectId > ARRAYSIZE(kResidentSoundResourceEntries)) {
		warning("Invalid %s resident effect id %u", kResidentSoundArchiveName, soundEffectId);
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kResidentSoundArchiveName))) {
		warning("Failed to open %s", kResidentSoundArchiveName);
		return false;
	}

	const uint tableEntry = _resourceEntries[soundEffectId - 1];
	const uint offsetPosition = kResource000HeaderByteCount + tableEntry * 4;
	const uint sizePosition = kResource000HeaderByteCount + kResource000TableByteCount + tableEntry * 4;
	if (sizePosition + 4 > (uint32)file.size()) {
		warning("%s resident effect %u table entry is out of range",
			kResidentSoundArchiveName, soundEffectId);
		return false;
	}

	file.seek(offsetPosition);
	start = file.readUint32LE();
	file.seek(sizePosition);
	size = file.readUint32LE();
	if (start > (uint32)file.size() || size > (uint32)file.size() - start) {
		warning("Invalid %s resident effect %u span: start=%u size=%u fileSize=%u",
			kResidentSoundArchiveName, soundEffectId, start, size, (uint)file.size());
		return false;
	}

	size &= ~1U;
	return size != 0;
}

} // End of namespace Hollywood
