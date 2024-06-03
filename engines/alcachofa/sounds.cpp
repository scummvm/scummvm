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

Sounds::Playback::Playback(uint32 id, SoundHandle handle, Mixer::SoundType type)
	: _id(id), _handle(handle), _type(type) {}

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
			(channels == 2 ? FLAG_STEREO : 0) | (bitsPerSample == 16 ? FLAG_16BITS | FLAG_LITTLE_ENDIAN : FLAG_UNSIGNED));
	else if (format == 17 && channels <= 2)
		return makeADPCMStream(subStream, DisposeAfterUse::YES, 0, kADPCMMSIma, (int)freq, (int)channels, (uint32)bytesPerBlock);
	error("Invalid SND file, format: %u, channels: %u, freq: %u, bps: %u", format, channels, freq, bitsPerSample);
}

static AudioStream *openAudio(const String &fileName) {
	String path = String::format("Sonidos/%s.SND", fileName.c_str());
	File *file = new File();
	if (file->open(path.c_str()))
		return loadSND(file);
	path.setChar('W', path.size() - 3);
	path.setChar('A', path.size() - 2);
	path.setChar('V', path.size() - 1);
	if (file->open(path.c_str()))
		return makeWAVStream(file, DisposeAfterUse::YES);
	delete file;
	error("Could not open audio file: %s", fileName.c_str());
}

SoundID Sounds::playVoice(const String &fileName, byte volume) {
	AudioStream *stream = openAudio(fileName);
	SoundHandle handle;
	_mixer->playStream(Mixer::kSpeechSoundType, &handle, stream, -1, volume);
	SoundID id = _nextID++;
	_playbacks.push_back({ id, handle, Mixer::kSpeechSoundType });
	return id;
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
	MainCharacterKind processCharacter,
	Character *speakingCharacter) {
	static constexpr byte kAlmostMaxVolume = Mixer::kMaxChannelVolume * 9 / 10;

	auto &player = g_engine->player();
	byte newVolume;
	if (player.activeCharacter() == nullptr || player.activeCharacter() == speakingCharacter)
		newVolume = Mixer::kMaxChannelVolume;
	else if (speakingCharacter != nullptr && speakingCharacter->room() == player.currentRoom())
		newVolume = kAlmostMaxVolume;
	else if (g_engine->world().getMainCharacterByKind(processCharacter).room() == player.currentRoom())
		newVolume = kAlmostMaxVolume;
	else
		newVolume = 0;
	setVolume(id, newVolume);
}

void Sounds::fadeOut(SoundID id, uint32 duration) {
	Playback *playback = getPlaybackById(id);
	if (playback != nullptr) {
		playback->_fadeStart = g_system->getMillis();
		playback->_fadeDuration = MAX<uint32>(duration, 1);
	}
}

}
