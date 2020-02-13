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

	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return false; }
	bool endOfData() const override;
	int getRate() const override { return 22050; }
	Audio::Timestamp getLength() const override { return Audio::Timestamp(); }
	bool seek(const Audio::Timestamp &where) override { return false; }
};

int AudioBufferStream::readBuffer(int16 *buffer, const int numSamples) {
	return _audioBuffer->read(buffer, numSamples);
}

bool AudioBufferStream::endOfData() const {
	return _audioBuffer->isFinished();
}

/*------------------------------------------------------------------------*/

CWaveFile::CWaveFile(Audio::Mixer *mixer) : _mixer(mixer), _pendingAudioStream(nullptr),
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
	// Delete any pending audio stream if it wasn't used
	delete _pendingAudioStream;

	if (_disposeAudioBuffer == DisposeAfterUse::YES && _audioBuffer)
		delete _audioBuffer;

	free(_waveData);
}

uint CWaveFile::getDurationTicks() const {
	if (!_rate)
		return 0;

	// FIXME: The original uses acmStreamSize to calculate
	// a desired size. Since I have no idea how the system API
	// method works, for now I'm using a simple ratio of a
	// sample output to input value
	double newSize = (double)_dataSize * (1475712.0 / 199836.0);
	return (uint)(newSize * 1000.0 / _rate);
}

bool CWaveFile::loadSound(const CString &name) {
	StdCWadFile file;
	if (!file.open(name))
		return false;

	Common::SeekableReadStream *stream = file.readStream();
	uint wavSize = stream->size();
	byte *data = (byte *)malloc(wavSize);
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

	_pendingAudioStream = new AudioBufferStream(_audioBuffer);
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

Audio::SeekableAudioStream *CWaveFile::createAudioStream() {
	Audio::SeekableAudioStream *stream;

	if (_pendingAudioStream) {
		stream = _pendingAudioStream;
		_pendingAudioStream = nullptr;
	} else {
		// Create a new ScummVM audio stream for the wave file data
		stream = Audio::makeWAVStream(
			new Common::MemoryReadStream(_waveData, _waveSize, DisposeAfterUse::NO),
			DisposeAfterUse::YES);
	}

	_rate = stream->getRate();
	return stream;
}


const int16 *CWaveFile::lock() {
	enum { kWaveFormatPCM = 1 };

	switch (_loadMode) {
	case LOADMODE_SCUMMVM:
		// Sanity checking that only raw 16-bit LE 22Khz waves can be locked
		assert(_waveData && _rate == AUDIO_SAMPLING_RATE);
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

Audio::SoundHandle CWaveFile::play(int numLoops, byte volume) {
	Audio::SeekableAudioStream *audioStream = createAudioStream();
	Audio::SoundHandle handle;

	Audio::AudioStream *stream = audioStream;
	if (numLoops != 0)
		stream = new Audio::LoopingAudioStream(audioStream,
			(numLoops == -1) ? 0 : numLoops);

	_mixer->playStream(_soundType, &handle, stream, -1,
		volume, 0, DisposeAfterUse::YES);
	return handle;
}

} // End of namespace Titanic
