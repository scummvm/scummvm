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

/**
 * This creates a ScummVM audio stream around a CAudioBuffer buffer,
 * allowing for streaming audio output for the music room music
 */
class AudioBufferStream : public Audio::SeekableAudioStream {
private:
	CAudioBuffer *_audioBuffer;
public:
	AudioBufferStream(CAudioBuffer *audioBuffer) : _audioBuffer(audioBuffer) {}

	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return false; }
	virtual bool endOfData() const;
	virtual int getRate() const { return 22050; }
	virtual Audio::Timestamp getLength() const { return Audio::Timestamp(); }
	virtual bool seek(const Audio::Timestamp &where) { return false; }
};

int AudioBufferStream::readBuffer(int16 *buffer, const int numSamples) {
	_audioBuffer->enterCriticalSection();
	int samplesToRead = MIN((const int)numSamples, (const int)_audioBuffer->size());

	for (int idx = 0; idx < samplesToRead; ++idx)
		*buffer++ = _audioBuffer->pop();

	_audioBuffer->leaveCriticalSection();
	return samplesToRead;
}

bool AudioBufferStream::endOfData() const {
	return _audioBuffer->_finished;
}

/*------------------------------------------------------------------------*/

CWaveFile::CWaveFile() : _audioStream(nullptr),
		_waveData(nullptr), _waveSize(0), _dataSize(0), _headerSize(0),
		_rate(0), _flags(0), _wavType(0), _soundType(Audio::Mixer::kPlainSoundType) {
	setup();
}

CWaveFile::CWaveFile(QSoundManager *owner) : _audioStream(nullptr),
		_waveData(nullptr), _waveSize(0), _dataSize(0), _headerSize(0),
		_rate(0), _flags(0), _wavType(0), _soundType(Audio::Mixer::kPlainSoundType) {
	setup();
}

void CWaveFile::setup() {
	_loadMode = LOADMODE_SCUMMVM;
	_dataSize = 0;
	_audioBuffer = nullptr;
	_disposeAudioBuffer = DisposeAfterUse::NO;
	_channel = -1;
}

CWaveFile::~CWaveFile() {
	if (_audioStream) {
		//_soundManager->soundFreed(_soundHandle);
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
	uint wavSize = stream->size();
	byte *data = new byte[wavSize];
	stream->read(data, wavSize);

	load(data, wavSize);
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
	uint wavSize = stream->size();
	byte *data = new byte[wavSize];
	stream->read(data, wavSize);
	delete stream;

	load(data, wavSize);
	_soundType = Audio::Mixer::kMusicSoundType;
	return true;
}

bool CWaveFile::loadMusic(CAudioBuffer *buffer, DisposeAfterUse::Flag disposeAfterUse) {
	_audioBuffer = buffer;
	_disposeAudioBuffer = disposeAfterUse;
	_loadMode = LOADMODE_AUDIO_BUFFER;

	_audioStream = new AudioBufferStream(_audioBuffer);
	return true;
}

void CWaveFile::load(byte *data, uint dataSize) {
	_waveData = data;
	_waveSize = dataSize;

	// Parse the wave header
	Common::MemoryReadStream wavStream(data, dataSize, DisposeAfterUse::NO);
	if (!Audio::loadWAVFromStream(wavStream, _dataSize, _rate, _flags, &_wavType))
		error("Invalid wave file");
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

const int16 *CWaveFile::lock() {
	enum { kWaveFormatPCM = 1 };

	switch (_loadMode) {
	case LOADMODE_SCUMMVM:
		// Sanity checking that only raw 16-bit LE 22Khz waves can be locked
		assert(_waveData && _rate == 22050);
		assert(_flags == (Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_16BITS));
		assert(_wavType == kWaveFormatPCM);

		// Return a pointer to the data section of the wave file
		return (const int16 *)(_waveData + _headerSize);

	default:
		return nullptr;
	}
}

void CWaveFile::unlock(const int16 *ptr) {
	// No implementation needed in ScummVM
}

} // End of namespace Titanic z
