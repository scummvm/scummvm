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

#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "common/memstream.h"
#include "titanic/sound/wave_file.h"
#include "titanic/sound/sound_manager.h"
#include "titanic/support/simple_file.h"

namespace Titanic {

CWaveFile::CWaveFile() : _soundManager(nullptr), _audioStream(nullptr),
		_waveData(nullptr), _waveSize(0), _dataSize(0), _headerSize(0),
		_rate(0), _flags(0), _soundType(Audio::Mixer::kPlainSoundType) {
	setup();
}

CWaveFile::CWaveFile(QSoundManager *owner) : _soundManager(owner), _audioStream(nullptr),
		_waveData(nullptr), _waveSize(0), _dataSize(0), _headerSize(0),
		_rate(0), _flags(0), _soundType(Audio::Mixer::kPlainSoundType) {
	setup();
}

void CWaveFile::setup() {
	_loadMode = LOADMODE_SCUMMVM;
	_field4 = 0;
	_field14 = 1;
	_dataSize = 0;
	_audioBuffer = nullptr;
	_disposeAudioBuffer = DisposeAfterUse::NO;
	_channel = -1;
}

CWaveFile::~CWaveFile() {
	if (_audioStream) {
		_soundManager->soundFreed(_soundHandle);
		delete _audioStream;
	}

	if (_disposeAudioBuffer == DisposeAfterUse::YES && _audioBuffer)
		delete _audioBuffer;

	delete[] _waveData;
}

uint CWaveFile::getDurationTicks() const {
	if (!_audioStream)
		return 0;

	// FIXME: The original uses acmStreamSize to calculate
	// a desired size. Since I have no idea how the system API
	// method works, for now I'm using a simple ratio of a
	// sample output to input value
	double newSize = (double)_dataSize * (1475712.0 / 199836.0);
	return (uint)(newSize * 1000.0 / _audioStream->getRate());
}

bool CWaveFile::loadSound(const CString &name) {
	assert(!_audioStream);

	StdCWadFile file;
	if (!file.open(name))
		return false;

	Common::SeekableReadStream *stream = file.readStream();
	uint size = stream->size();
	byte *data = new byte[size];
	stream->read(data, size);

	load(data, size);
	_soundType = Audio::Mixer::kSFXSoundType;
	return true;
}

bool CWaveFile::loadSpeech(CDialogueFile *dialogueFile, int speechIndex) {
	DialogueResource *res = dialogueFile->openWaveEntry(speechIndex);
	if (!res)
		return false;

	byte *data = (byte *)malloc(res->_size);
	dialogueFile->read(res, data, res->_size);
	load(data, res->_size);

	_soundType = Audio::Mixer::kSpeechSoundType;
	return true;
}

bool CWaveFile::loadMusic(const CString &name) {
	assert(!_audioStream);

	StdCWadFile file;
	if (!file.open(name))
		return false;

	Common::SeekableReadStream *stream = file.readStream();
	uint size = stream->size();
	byte *data = new byte[size];
	stream->read(data, size);
	delete stream;

	load(data, size);
	_soundType = Audio::Mixer::kMusicSoundType;
	return true;
}

bool CWaveFile::loadMusic(CAudioBuffer *buffer, DisposeAfterUse::Flag disposeAfterUse) {
	_audioBuffer = buffer;
	_disposeAudioBuffer = disposeAfterUse;
	_loadMode = LOADMODE_AUDIO_BUFFER;
	_field14 = 0;

	return true;
}

void CWaveFile::load(byte *data, uint size) {
	_waveData = data;
	_waveSize = size;

	// Parse the wave header
	Common::MemoryReadStream wavStream(data, size, DisposeAfterUse::NO);
	Audio::loadWAVFromStream(wavStream, _dataSize, _rate, _flags);
	_headerSize = wavStream.pos();
}

Audio::SeekableAudioStream *CWaveFile::audioStream() {
	if (!_audioStream) {
		// No stream yet, so create one and give it control of the raw wave data
		assert(_waveData);
		_audioStream = Audio::makeWAVStream(
			new Common::MemoryReadStream(_waveData, _waveSize, DisposeAfterUse::YES),
			DisposeAfterUse::YES);
		_waveData = nullptr;
	}

	return _audioStream;
}


uint CWaveFile::getFrequency() {
	return audioStream()->getRate();
}

void CWaveFile::reset() {
	audioStream()->rewind();
}

const uint16 *CWaveFile::lock() {
	switch (_loadMode) {
	case LOADMODE_SCUMMVM:
		assert(_waveData && _rate == 22050);
		assert(_flags == (Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_16BITS));
		return (uint16 *)(_waveData + _headerSize);

	default:
		return nullptr;
	}
}

void CWaveFile::unlock(const uint16 *ptr) {
	// No implementation needed in ScummVM
}

} // End of namespace Titanic z
