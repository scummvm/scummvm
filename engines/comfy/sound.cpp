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

#include "comfy/comfy.h"
#include "comfy/midiplyr/midiplyr.h"

#include "audio/decoders/raw.h"
#include "audio/audiostream.h"
#include "common/debug.h"
#include "common/endian.h"

namespace Comfy {

class SoundBitReader {
public:
	SoundBitReader(byte *data, uint32 size) : _data(data), _size(size), _bitPosition(0), _fault(false) {}

	uint32 read(uint bits) {
		uint32 value = 0;
		for (uint bit = 0; bit < bits; bit++) {
			uint32 bytePosition = _bitPosition >> 3;
			if (bytePosition >= _size) {
				_fault = true;
				return value;
			}

			value |= (uint32)((_data[bytePosition] >> (_bitPosition & 7)) & 1) << bit;
			_bitPosition++;
		}

		return value;
	}

	uint32 position() { return _bitPosition; }
	void setPosition(uint32 position) { _bitPosition = position; }
	bool fault() { return _fault; }

private:
	byte *_data;
	uint32 _size;
	uint32 _bitPosition;
	bool _fault;
};

struct SoundDecoderState {
public:
	SoundDecoderState(const byte *data, uint32 size, bool compressed) {
		_data.resize(size);
		if (size)
			memcpy(&_data[0], data, size);

		_compressed = compressed;
		_end = size;
		if (_compressed) {
			if (size < 5) {
				_fault = true;
				return;
			}

			setSample(_data[4]);
			_position = 5;
		} else if (size >= 6 && _data[0] == 1) {
			setSample(_data[4]);
		} else {
			setSample(_sample);
		}
	}

	uint32 decode(byte *buffer, uint32 size) {
		uint32 written = 0;
		while (written < size && !_fault && !_finished) {
			byte sample = 0x80;
			if (!readSample(sample))
				break;

			buffer[written++] = sample;
		}

		return written;
	}

	uint32 getRate() { return _sampleRate; }
	bool finished() { return _finished; }

	void copyCues(Common::Array<ComfyEngine::SoundCue> &cues) {
		cues = _cues;
	}

	bool fault() {
		return _fault;
	}

	void saveBufferState(uint16 bufferIndex) {
		if (bufferIndex >= 2)
			return;

		BufferState &state = _bufferStates[bufferIndex];
		state.streamPosition = _streamPosition;
		state.position = _position;
		state.loopPosition = _loopPosition;
		state.eventRemaining = _eventRemaining;
		state.fillRemaining = _fillRemaining;
		state.loopCount = _loopCount;
		state.started = _started;
	}

	uint32 getBufferStreamPosition(uint16 bufferIndex) {
		return bufferIndex < 2 ? _bufferStates[bufferIndex].streamPosition : 0;
	}

	void packState(byte *packedState, uint16 bufferIndex, uint32 playbackPosition, uint32 vocCounter,
			bool stopped, bool usesAnimData, Common::Array<ComfyEngine::SoundCue> &cues, uint nextCue) {
		memset(packedState, 0, 0x52);
		packedState[0x3F] = stopped;
		if (stopped || bufferIndex >= 2)
			return;

		BufferState &state = _bufferStates[bufferIndex];
		WRITE_LE_UINT32(packedState, state.streamPosition);
		packedState[4] = state.started;
		WRITE_LE_UINT16(packedState + 5, (uint16)state.fillRemaining);
		WRITE_LE_UINT32(packedState + 7, state.eventRemaining);
		WRITE_LE_UINT16(packedState + 0x0B, (uint16)state.loopCount);
		WRITE_LE_UINT32(packedState + 0x0D, playbackPosition);
		WRITE_LE_UINT32(packedState + 0x11, vocCounter);
		uint16 packedCueCount = 0;
		while (nextCue < cues.size() && packedCueCount < 4 &&
				cues[nextCue].streamPosition < playbackPosition) {
			WRITE_LE_UINT16(packedState + 0x15 + packedCueCount * 2, cues[nextCue].value);
			WRITE_LE_UINT32(packedState + 0x1D + packedCueCount * 4, cues[nextCue].streamPosition);
			WRITE_LE_UINT32(packedState + 0x2D + packedCueCount * 4, cues[nextCue].counterThreshold);
			packedCueCount++;
			nextCue++;
		}

		WRITE_LE_UINT16(packedState + 0x3D, packedCueCount);
		packedState[0x40] = _sample;
		WRITE_LE_UINT32(packedState + 0x41, state.position);
		WRITE_LE_UINT32(packedState + 0x45, state.loopPosition);
		WRITE_LE_UINT32(packedState + 0x49, _end);
		WRITE_LE_UINT32(packedState + 0x4D, 0);
		packedState[0x51] = usesAnimData;
	}

	bool unpackState(byte *packedState, Common::Array<ComfyEngine::SoundCue> &cues,
			uint32 &vocCounter) {
		if (packedState[0x3F])
			return false;

		_streamPosition = READ_LE_UINT32(packedState);
		_started = packedState[4] != 0;
		_fillRemaining = READ_LE_UINT16(packedState + 5);
		_eventRemaining = READ_LE_UINT32(packedState + 7);
		_loopCount = (int16)READ_LE_UINT16(packedState + 0x0B);
		uint32 playbackPosition = READ_LE_UINT32(packedState + 0x0D);
		vocCounter = READ_LE_UINT32(packedState + 0x11);
		setSample(packedState[0x40]);
		_position = READ_LE_UINT32(packedState + 0x41);
		_loopPosition = READ_LE_UINT32(packedState + 0x45);
		uint32 packedEnd = READ_LE_UINT32(packedState + 0x49);
		if (_position > _data.size() || _loopPosition > _data.size() || packedEnd > _data.size())
			return false;

		_end = packedEnd;
		_bitOffset = 0;
		_loopBitOffset = 0;
		_blockOffset = 0;
		_fault = false;
		_finished = false;
		_cues.clear();
		uint16 advanceSize = (uint16)(playbackPosition - _streamPosition);
		if (advanceSize > COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES)
			advanceSize = COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES;

		if (advanceSize) {
			Common::Array<byte> discardedPcm;
			discardedPcm.resize(advanceSize);
			if (decode(&discardedPcm[0], advanceSize) != advanceSize)
				return false;
		}

		_cues.clear();
		cues.clear();
		uint16 cueCount = MIN<uint16>(READ_LE_UINT16(packedState + 0x3D), 4);
		for (uint16 i = 0; i < cueCount; i++) {
			ComfyEngine::SoundCue cue;
			cue.value = READ_LE_UINT16(packedState + 0x15 + i * 2);
			cue.streamPosition = READ_LE_UINT32(packedState + 0x1D + i * 4);
			cue.counterThreshold = READ_LE_UINT32(packedState + 0x2D + i * 4);
			_cues.push_back(cue);
			cues.push_back(cue);
		}

		return true;
	}

private:
	struct BufferState {
		uint32 streamPosition = 0;
		uint32 position = 0;
		uint32 loopPosition = 0;
		uint32 eventRemaining = 0;
		uint32 fillRemaining = 0;
		int16 loopCount = 0;
		bool started = false;
	};

	static uint32 calcSampleRate(byte sample) {
		uint32 denominator = 0x100 - sample;
		uint32 rate = denominator ? 0x0F4240 / denominator : 0x2B11;
		if (rate > 0x2710 && rate < 0x2EE0)
			return 0x2B11;

		if (rate > 0x4E20 && rate < 0x5DC0)
			return 0x5622;

		return rate;
	}

	void setSample(byte sample) {
		_sample = sample;
		_sampleRate = calcSampleRate(sample);
		if (_sampleRate == 0x2B11)
			_sample = 0xA5;
		else if (_sampleRate == 0x5622)
			_sample = 0xD3;
	}

	bool readSample(byte &sample) {
		for (;;) {
			if (_started) {
				_streamPosition++;
				sample = 0x80;
				return true;
			}

			if (_fillRemaining) {
				_fillRemaining--;
				_streamPosition++;
				sample = 0x80;
				return true;
			}

			if (_eventRemaining) {
				if (!copySample(sample))
					return false;

				_eventRemaining--;
				_streamPosition++;
				return true;
			}

			uint16 command = parseCommand();
			if (_fault)
				return false;

			switch (command) {
			case 0:
				pushCue(0);
				_started = true;
				break;

			case 1:
				beginEvent(false);
				break;

			case 2:
				beginEvent(true);
				break;

			case 3:
				readFillCommand();
				break;

			case 4:
				pushCue(readParam());
				break;

			case 6:
				_loopCount = (int16)readWord();
				_loopPosition = _position;
				_loopBitOffset = _bitOffset;
				break;

			case 7:
				if (_loopCount > 0) {
					_loopCount--;
					// The original reloads the loop position twice.
					seekToLoop();
					seekToLoop();
				}
				break;

			case 99:
				setSample((byte)readBits(8));
				break;

			default:
				break;
			}
		}
	}

	void seekToLoop() {
		_position = _loopPosition;
		_bitOffset = _loopBitOffset;
	}

	uint16 parseCommand() {
		_eventArgument = 0;
		if (!_compressed) {
			if (_position >= _end) {
				_fault = true;
				return 0;
			}

			uint16 command = _data[_position++];
			if (command) {
				if (_end - _position < 3) {
					_fault = true;
					return 0;
				}

				_eventArgument = (uint32)_data[_position] | ((uint32)_data[_position + 1] << 8) |
					((uint32)_data[_position + 2] << 16);
				_position += 3;
			}

			return command;
		}

		uint16 command = (uint16)readBits(2);
		if (!command) {
			uint16 subcommand = (uint16)readBits(2);
			if (!subcommand)
				return 0;

			if (subcommand == 1) {
				command = (uint16)readBits(8);
				_eventArgument = readBits(20);
				return command;
			}

			return subcommand == 2 ? 7 : 99;
		}

		if (command == 1) {
			_eventArgument = readBits(20);
			return 1;
		}

		return command == 2 ? 3 : 4;
	}

	void beginEvent(bool loop) {
		_eventRemaining = _eventArgument;
		if (_compressed) {
			_needInit = true;
			return;
		}

		if (loop)
			return;

		if (_eventRemaining < 2 || _end - _position < 2) {
			_fault = true;
			return;
		}

		_eventRemaining -= 2;
		setSample(_data[_position]);
		_position += 2;
	}

	void readFillCommand() {
		uint16 count = readWord();
		byte nextSample = _compressed ? _sample : readByte();
		if (_fault)
			return;

		_fillRemaining = count;
		if (nextSample != _sample) {
			uint32 oldRate = calcSampleRate(_sample);
			uint32 newRate = calcSampleRate(nextSample);
			if (newRate)
				_fillRemaining = ((uint32)_fillRemaining * oldRate) / newRate;
		}
	}

	bool copySample(byte &sample) {
		if (!_compressed) {
			if (_position >= _end) {
				_fault = true;
				return false;
			}

			sample = _data[_position++];
			_history.push_back(sample);
			return true;
		}

		if (_needInit) {
			_needInit = false;
			_bitWidth = 4;
			_predictorDistance = 1;
			sample = (byte)readBits(8);
			_blockOffset = 1;
		} else {
			if (_blockOffset == 0 || _blockOffset >= 0x20) {
				if (readBits(1))
					_bitWidth = (byte)readBits(3);

				if (_bitWidth && readBits(1))
					_predictorDistance = (byte)readBits(7);
				_blockOffset = 0;
			}

			if (!_bitWidth) {
				sample = (byte)readBits(8);
			} else {
				uint16 delta = (uint16)readBits(_bitWidth);
				if (!delta) {
					sample = (byte)readBits(8);
				} else {
					byte predictor = _history.size() >= _predictorDistance ?
						_history[_history.size() - _predictorDistance] : 0x80;
					sample = predictor + (byte)(delta - (1 << (_bitWidth - 1)));
				}
			}

			_blockOffset++;
		}

		_history.push_back(sample);
		return !_fault;
	}

	uint32 readBits(uint bits) {
		uint32 value = 0;
		for (uint bit = 0; bit < bits; bit++) {
			if (_position >= _end) {
				_fault = true;
				return value;
			}

			value |= (uint32)((_data[_position] >> _bitOffset) & 1) << bit;
			_bitOffset++;
			if (_bitOffset == 8) {
				_bitOffset = 0;
				_position++;
			}
		}

		return value;
	}

	uint16 readParam() {
		if (!_compressed)
			return readWord();

		switch (readBits(2)) {
		case 0:
			return 1;
		case 1:
			return 2;
		case 2:
			return (uint16)readBits(8);
		case 3:
			return (uint16)readBits(16);
		default:
			return 0;
		}
	}

	uint16 readWord() {
		if (_compressed)
			return (uint16)readBits(16);

		if (_end - _position < 2) {
			_fault = true;
			return 0;
		}

		uint16 value = READ_LE_UINT16(&_data[_position]);
		_position += 2;
		return value;
	}

	byte readByte() {
		if (_position >= _end) {
			_fault = true;
			return 0;
		}

		return _data[_position++];
	}

	void pushCue(uint16 value) {
		if (_cues.size() >= COMFY_SOUND_PITCH_COUNT_LIMIT)
			return;

		ComfyEngine::SoundCue cue;
		cue.value = value;
		cue.streamPosition = _streamPosition;
		cue.counterThreshold = _sampleRate ? (_streamPosition * 100) / _sampleRate : 0;
		_cues.push_back(cue);
	}

	Common::Array<byte> _data;
	Common::Array<byte> _history;
	Common::Array<ComfyEngine::SoundCue> _cues;
	uint32 _end = 0;
	uint32 _position = 0;
	uint32 _streamPosition = 0;
	uint32 _eventArgument = 0;
	uint32 _eventRemaining = 0;
	uint32 _fillRemaining = 0;
	uint32 _loopPosition = 0;
	uint32 _sampleRate = 0x2B11;
	int16 _loopCount = 0;
	byte _sample = 0xA5;
	byte _bitOffset = 0;
	byte _loopBitOffset = 0;
	byte _bitWidth = 4;
	byte _predictorDistance = 1;
	byte _blockOffset = 0;
	bool _compressed = false;
	bool _needInit = false;
	bool _started = false;
	bool _finished = false;
	bool _fault = false;
	BufferState _bufferStates[2];
};

bool ComfyEngine::soundInit() {
	if (!readAssetFile(Common::Path("VOCFILE.DAT"), _vocFileData) || _vocFileData.size() < 4)
		return false;

	if (_vocFileData[0] != 'C' || (_vocFileData[1] != 'V' && _vocFileData[1] != 'W'))
		return false;

	_soundCompressed = _vocFileData[1] == 'W';
	_soundUsesAnimData = false;
	_soundEntryCount = assetsReadLe16At(_vocFileData, 2);
	_soundEventIndex = 0;
	_soundEventMaximum = 0;
	_soundEventSubIndex = 0xFFFF;
	_soundEventPreviousSubIndex = 0xFFFF;
	for (uint i = 0; i < COMFY_VOC_QUEUE_CAPACITY; i++) {
		_vocQueue[i] = VocQueueEntry();
		_vocQueue1999[i] = VocQueueEntry1999();
	}

	_wcomfy99VocState0 = 0;
	_wcomfy99VocState1 = 0;
	_wcomfy99VocState2 = 0;
	_wcomfy99VocState3 = 0;
	_wcomfy99VocState6 = 0;
	return true;
}

void ComfyEngine::soundShutdown() {
	_mixer->stopHandle(_soundHandle);
	_soundQueueStream = nullptr;
	delete _soundDecoderState;
	_soundDecoderState = nullptr;
	_soundPcm.clear();
	_soundCues.clear();
	_vocFileData.clear();
	_soundEntryCount = 0;
	_soundNextCue = 0;
	_soundPaused = false;
	_soundUsesAnimData = false;
	if (_midiPlyrDriver)
		_midiPlyrDriver->setIncreaseVocCounter(0);
}

void ComfyEngine::soundHdrReadFromXms(byte *destination, uint16 index, uint16 size) {
	objHdrReadFromXms(destination, _headerXmsSoundHeadersBase, size, index);
}

void ComfyEngine::soundServiceWaveBuffers() {
	if (!_soundQueueStream || !_mixer->isSoundHandleActive(_soundHandle))
		return;

	uint32 queuedBlocks = _soundQueueStream->numQueuedStreams();
	if (_midiPlyrDriver) {
		for (uint32 i = queuedBlocks; i < 2; i++)
			_midiPlyrDriver->vocSrUpdateCounters();
	}

	while (_soundQueueStream->numQueuedStreams() < 2) {
		if (!soundQueuePcmBlock(_soundWaveBufferIndex))
			return;

		_soundWaveBufferIndex = 1 - _soundWaveBufferIndex;
	}
}

bool ComfyEngine::soundLoadEntry(uint16 index) {
	byte header[8];
	memset(header, 0, sizeof(header));
	soundHdrReadFromXms(header, index, sizeof(header));
	uint32 size = READ_LE_UINT32(header);
	uint32 offset = READ_LE_UINT32(header + 4);
	return offset <= _vocFileData.size() && size <= _vocFileData.size() - offset;
}

bool ComfyEngine::soundPrepareDecoderState(uint16 index) {
	Common::Array<byte> animationSoundData;
	byte *data = nullptr;
	uint32 fileSize = 0;
	uint32 dataSize = 0;
	uint32 dataOffset = 0;
	bool compressed = _soundCompressed;
	if (index == 0xFFFF) {
		uint32 storedSize = READ_LE_UINT32(_animFrameHeader + 4);
		if (!storedSize || storedSize > _animFrameStorage.size())
			return false;

		animationSoundData.resize(storedSize);
		if (storedSize)
			memcpy(&animationSoundData[0], &_animFrameStorage[0], storedSize);

		data = &animationSoundData[0];
		fileSize = animationSoundData.size();
		dataSize = fileSize;
		compressed = false;
		_soundUsesAnimData = true;
	} else {
		byte header[8];
		memset(header, 0, sizeof(header));
		soundHdrReadFromXms(header, index, sizeof(header));
		dataSize = READ_LE_UINT32(header);
		dataOffset = READ_LE_UINT32(header + 4);
		data = &_vocFileData[0];
		fileSize = _vocFileData.size();
		_soundUsesAnimData = false;
	}

	if (dataOffset > fileSize || dataSize > fileSize - dataOffset)
		return false;

	delete _soundDecoderState;
	_soundDecoderState = new SoundDecoderState(data + dataOffset, dataSize, compressed);
	if (_soundDecoderState->fault()) {
		delete _soundDecoderState;
		_soundDecoderState = nullptr;
		return false;
	}

	_soundSampleRate = _soundDecoderState->getRate();
	_soundCues.clear();
	_soundNextCue = 0;
	return true;
}

bool ComfyEngine::soundDecodePcmBlock(uint16 bufferIndex, byte *&buffer, uint32 &decodedSize) {
	buffer = nullptr;
	decodedSize = 0;
	if (!_soundDecoderState)
		return false;

	_soundDecoderState->saveBufferState(bufferIndex);
	uint32 blockSize = _isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES;
	buffer = (byte *)malloc(blockSize);
	if (!buffer)
		return false;

	decodedSize = _soundDecoderState->decode(buffer, blockSize);
	_soundDecoderState->copyCues(_soundCues);
	if (!decodedSize) {
		free(buffer);
		buffer = nullptr;
		return false;
	}

	return true;
}

bool ComfyEngine::soundQueuePcmBlock(uint16 bufferIndex) {
	if (!_soundQueueStream)
		return false;

	byte *buffer = nullptr;
	uint32 decodedSize = 0;
	if (!soundDecodePcmBlock(bufferIndex, buffer, decodedSize))
		return false;

	_soundQueueStream->queueBuffer(buffer, decodedSize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	return true;
}

void ComfyEngine::soundPackState(byte *state) {
	bool stopped = !_soundDecoderState || !_mixer->isSoundHandleActive(_soundHandle);
	if (stopped) {
		memset(state, 0, 0x52);
		state[0x3F] = 1;
		return;
	}

	uint32 vocCounter = _midiPlyrDriver ? _midiPlyrDriver->getVocCounter() :
		(uint32)((_mixer->getSoundElapsedTime(_soundHandle) + 9) / 10);
	uint32 playbackPosition = (vocCounter * _soundSampleRate) / 100;
	uint16 bufferIndex = _soundWaveBufferIndex;
	if ((int32)(_soundDecoderState->getBufferStreamPosition(bufferIndex) -
			COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES) < (int32)playbackPosition)
		bufferIndex = 1 - bufferIndex;

	_soundDecoderState->packState(state, bufferIndex, playbackPosition, vocCounter,
		stopped, _soundUsesAnimData, _soundCues, _soundNextCue);
}

void ComfyEngine::soundUnpackState(byte *state) {
	_mixer->stopHandle(_soundHandle);
	_soundQueueStream = nullptr;
	if (state[0x3F] || !_soundDecoderState) {
		if (_midiPlyrDriver) {
			_midiPlyrDriver->setIncreaseVocCounter(0);
			_midiPlyrDriver->vocSrResetBlockNo();
		}

		return;
	}

	uint32 vocCounter = 0;
	if (!_soundDecoderState->unpackState(state, _soundCues, vocCounter)) {
		_soundEventSubIndex = 0;
		return;
	}

	_soundUsesAnimData = state[0x51] != 0;
	_soundNextCue = 0;
	byte *buffers[2] = {nullptr, nullptr};
	uint32 decodedSizes[2] = {0, 0};
	if (!soundDecodePcmBlock(0, buffers[0], decodedSizes[0]) ||
			!soundDecodePcmBlock(1, buffers[1], decodedSizes[1])) {
		free(buffers[0]);
		free(buffers[1]);
		_soundEventSubIndex = 0;
		return;
	}

	_soundSampleRate = _soundDecoderState->getRate();
	_soundQueueStream = Audio::makeQueuingAudioStream(_soundSampleRate, false);
	if (!_soundQueueStream) {
		free(buffers[0]);
		free(buffers[1]);
		_soundEventSubIndex = 0;
		return;
	}

	_soundQueueStream->queueBuffer(buffers[0], decodedSizes[0], DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	_soundQueueStream->queueBuffer(buffers[1], decodedSizes[1], DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	_soundWaveBufferIndex = 0;
	_soundEventSubIndex = 0xFFFF;
	if (_midiPlyrDriver) {
		_midiPlyrDriver->setVocCounter(vocCounter);
		_midiPlyrDriver->setIncreaseVocCounter(1);
		_midiPlyrDriver->vocSrResetCounters();
	}

	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, _soundQueueStream, -1,
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
}

bool ComfyEngine::soundDecodeEntry(uint16 index) {
	_soundPcm.clear();
	_soundCues.clear();
	_soundNextCue = 0;
	Common::Array<byte> animationSoundData;
	byte *data = nullptr;
	uint32 fileSize = 0;
	uint32 dataSize = 0;
	uint32 dataOffset = 0;
	bool compressed = _soundCompressed;
	if (index == 0xFFFF) {
		uint32 storedSize = READ_LE_UINT32(_animFrameHeader + 4);
		if (_animFrameStorage.size() < 2 || storedSize > _animFrameStorage.size() - 1)
			return false;

		animationSoundData.resize(storedSize + 1);
		animationSoundData[0] = 1;
		memcpy(&animationSoundData[1], &_animFrameStorage[1], storedSize);
		data = &animationSoundData[0];
		fileSize = animationSoundData.size();
		dataSize = fileSize;
		compressed = false;
	} else {
		byte header[8];
		memset(header, 0, sizeof(header));
		soundHdrReadFromXms(header, index, sizeof(header));
		dataSize = READ_LE_UINT32(header);
		dataOffset = READ_LE_UINT32(header + 4);
		data = &_vocFileData[0];
		fileSize = _vocFileData.size();
	}

	debug(5, "COMFY VOC: decode id=%u offset=%u size=%u compressed=%u fileSize=%u",
		index, dataOffset, dataSize, compressed ? 1 : 0, fileSize);
	if (dataOffset > fileSize || dataSize > fileSize - dataOffset) {
		debug(5, "COMFY VOC: decode id=%u rejected invalid range", index);
		return false;
	}

	if (compressed) {
		bool decoded = soundDecodeCompressedEntry(dataOffset, dataSize);
		debug(5, "COMFY VOC: decode id=%u result=%u pcm=%u rate=%u cues=%u",
			index, decoded ? 1 : 0, (uint)_soundPcm.size(), _soundSampleRate, (uint)_soundCues.size());
		return decoded;
	}

	uint32 pc = dataOffset;
	uint32 end = dataOffset + dataSize;
	byte sample = 0xA5;
	_soundSampleRate = 0x2B11;
	uint32 loopPc = 0;
	int16 loopCount = 0;
	uint32 commandCount = 0;
	while (pc < end && commandCount++ < dataSize) {
		byte command = data[pc++];
		if (!command) {
			SoundCue cue = {0, (uint32)_soundPcm.size(), (uint32)((_soundPcm.size() * 100) / _soundSampleRate)};
			_soundCues.push_back(cue);
			break;
		}

		if (3 > end - pc)
			return false;

		uint32 argument = (uint32)data[pc] | ((uint32)data[pc + 1] << 8) |
			((uint32)data[pc + 2] << 16);
		pc += 3;
		if (command == 1 || command == 2) {
			uint32 length = argument;
			if (command == 1) {
				if (length < 2 || 2 > end - pc)
					return false;

				sample = data[pc];
				pc += 2;
				length -= 2;
				uint32 denominator = 0x100 - sample;
				_soundSampleRate = denominator ? 0x0F4240 / denominator : 0x2B11;
				if (_soundSampleRate > 0x2710 && _soundSampleRate < 0x2EE0) {
					_soundSampleRate = 0x2B11;
					sample = 0xA5;
				} else if (_soundSampleRate > 0x4E20 && _soundSampleRate < 0x5DC0) {
					_soundSampleRate = 0x5622;
					sample = 0xD3;
				}
			}

			if (length > end - pc)
				return false;

			uint32 oldSize = _soundPcm.size();
			_soundPcm.resize(oldSize + length);
			memcpy(&_soundPcm[oldSize], data + pc, length);
			pc += length;
		} else if (command == 3) {
			if (3 > end - pc)
				return false;

			uint16 fill = READ_LE_UINT16(data + pc);
			byte nextSample = data[pc + 2];
			pc += 3;
			if (nextSample != sample) {
				uint32 oldRate = _soundSampleRate;
				uint32 newRate = 0x0F4240 / (0x100 - nextSample);
				fill = (uint16)(((uint32)fill * oldRate) / newRate);
			}

			uint32 oldSize = _soundPcm.size();
			_soundPcm.resize(oldSize + fill);
			memset(&_soundPcm[oldSize], 0x80, fill);
		} else if (command == 4) {
			if (2 > end - pc)
				return false;

			SoundCue cue = {READ_LE_UINT16(data + pc), (uint32)_soundPcm.size(),
				(uint32)((_soundPcm.size() * 100) / _soundSampleRate)};
			_soundCues.push_back(cue);
			pc += 2;
		} else if (command == 6) {
			if (2 > end - pc)
				return false;

			loopCount = READ_LE_UINT16(data + pc);
			pc += 2;
			loopPc = pc;
		} else if (command == 7) {
			if (loopCount > 0 && loopPc) {
				loopCount--;
				pc = loopPc;
			}
		} else if (command == 99) {
			if (pc >= end)
				return false;

			sample = data[pc++];
		}
	}

	bool decoded = !_soundPcm.empty();
	debug(5, "COMFY VOC: decode id=%u result=%u pcm=%u rate=%u cues=%u",
		index, decoded ? 1 : 0, (uint)_soundPcm.size(), _soundSampleRate, (uint)_soundCues.size());
	return decoded;
}

bool ComfyEngine::soundDecodeCompressedEntry(uint32 dataOffset, uint32 dataSize) {
	if (dataSize < 5)
		return false;

	byte sample = _vocFileData[dataOffset + 4];
	uint32 denominator = 0x100 - sample;
	_soundSampleRate = denominator ? 0x0F4240 / denominator : 0x2B11;
	if (_soundSampleRate > 0x2710 && _soundSampleRate < 0x2EE0) {
		_soundSampleRate = 0x2B11;
		sample = 0xA5;
	} else if (_soundSampleRate > 0x4E20 && _soundSampleRate < 0x5DC0) {
		_soundSampleRate = 0x5622;
		sample = 0xD3;
	}

	SoundBitReader bits(&_vocFileData[dataOffset + 5], dataSize - 5);
	uint32 loopPosition = 0;
	int16 loopCount = 0;
	byte width = 4;
	byte predictorDistance = 1;
	bool needInit = false;
	uint32 commandCount = 0;
	while (!bits.fault() && commandCount++ < dataSize * 8) {
		uint16 command = bits.read(2);
		uint32 argument = 0;
		if (!command) {
			uint16 subcommand = bits.read(2);
			if (!subcommand)
				command = 0;
			else if (subcommand == 1) {
				command = bits.read(8);
				argument = bits.read(20);
			} else if (subcommand == 2)
				command = 7;
			else
				command = 99;
		} else if (command == 1) {
			argument = bits.read(20);
		} else if (command == 2) {
			command = 3;
		} else {
			command = 4;
		}

		if (command == 0) {
			SoundCue cue = {0, (uint32)_soundPcm.size(), (uint32)((_soundPcm.size() * 100) / _soundSampleRate)};
			_soundCues.push_back(cue);
			break;
		} else if (command == 1 || command == 2) {
			uint32 length = argument;
			if (command == 1)
				needInit = true;

			uint32 oldSize = _soundPcm.size();
			_soundPcm.resize(oldSize + length);
			uint32 output = oldSize;
			if (needInit && length) {
				needInit = false;
				width = 4;
				predictorDistance = 1;
				_soundPcm[output++] = bits.read(8);
				length--;
			}

			while (length && !bits.fault()) {
				uint32 chunk = MIN<uint32>(length, 0x20);
				if (bits.read(1))
					width = bits.read(3);

				if (width && bits.read(1))
					predictorDistance = bits.read(7);

				for (uint i = 0; i < chunk; i++) {
					byte value;
					if (!width) {
						value = bits.read(8);
					} else {
						uint16 delta = bits.read(width);
						if (!delta)
							value = bits.read(8);
						else {
							byte predictor = output >= predictorDistance ? _soundPcm[output - predictorDistance] : 0x80;
							value = predictor + (byte)(delta - (1 << (width - 1)));
						}
					}

					_soundPcm[output++] = value;
				}

				length -= chunk;
			}
		} else if (command == 3) {
			uint16 fill = bits.read(16);
			uint32 oldSize = _soundPcm.size();
			_soundPcm.resize(oldSize + fill);
			memset(&_soundPcm[oldSize], 0x80, fill);
		} else if (command == 4) {
			uint16 mode = bits.read(2);
			uint16 value = mode == 0 ? 1 : mode == 1 ? 2 : mode == 2 ? bits.read(8) : bits.read(16);
			SoundCue cue = {value, (uint32)_soundPcm.size(),
				(uint32)((_soundPcm.size() * 100) / _soundSampleRate)};
			_soundCues.push_back(cue);
		} else if (command == 6) {
			loopCount = bits.read(16);
			loopPosition = bits.position();
		} else if (command == 7) {
			if (loopCount > 0 && loopPosition) {
				loopCount--;
				bits.setPosition(loopPosition);
			}
		} else if (command == 99) {
			sample = bits.read(8);
		}
	}

	return !_soundPcm.empty() && !bits.fault();
}

void ComfyEngine::soundPlayEntry(uint16 index) {
	debug(5, "COMFY VOC: play id=%u", index);
	_mixer->stopHandle(_soundHandle);
	_soundQueueStream = nullptr;
	if (!soundPrepareDecoderState(index)) {
		debug(5, "COMFY VOC: play id=%u decode failed", index);
		_soundEventSubIndex = 0;
		return;
	}

	byte *buffers[2] = {nullptr, nullptr};
	uint32 decodedSizes[2] = {0, 0};
	if (!soundDecodePcmBlock(0, buffers[0], decodedSizes[0]) ||
			!soundDecodePcmBlock(1, buffers[1], decodedSizes[1])) {
		free(buffers[0]);
		free(buffers[1]);
		delete _soundDecoderState;
		_soundDecoderState = nullptr;
		debug(5, "COMFY VOC: play id=%u initial buffer fill failed", index);
		_soundEventSubIndex = 0;
		return;
	}

	_soundSampleRate = _soundDecoderState->getRate();
	_soundQueueStream = Audio::makeQueuingAudioStream(_soundSampleRate, false);
	if (!_soundQueueStream) {
		free(buffers[0]);
		free(buffers[1]);
		delete _soundDecoderState;
		_soundDecoderState = nullptr;
		debug(5, "COMFY VOC: play id=%u queue creation failed", index);
		_soundEventSubIndex = 0;
		return;
	}

	_soundQueueStream->queueBuffer(buffers[0], decodedSizes[0], DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	_soundQueueStream->queueBuffer(buffers[1], decodedSizes[1], DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, _soundQueueStream, -1,
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	debug(5, "COMFY VOC: play id=%u submitted active=%u rate=%u cues=%u",
		index, _mixer->isSoundHandleActive(_soundHandle) ? 1 : 0,
		_soundSampleRate, (uint)_soundCues.size());
	_soundPaused = false;
	_soundWaveBufferIndex = 0;
	_soundEventSubIndex = 0xFFFF;
	if (_midiPlyrDriver) {
		_midiPlyrDriver->vocSrResetCounters();
		_midiPlyrDriver->setVocCounter(0);
		_midiPlyrDriver->setIncreaseVocCounter(1);
	}
}

void ComfyEngine::soundAdvanceTick() {
	if (_soundPaused)
		return;

	if (!_mixer->isSoundHandleActive(_soundHandle)) {
		_soundQueueStream = nullptr;
		if (_soundDecoderState && _soundEventSubIndex != 0) {
			debug(5, "COMFY VOC: mixer became inactive; marking entry complete");
			_soundEventSubIndex = 0;
		}

		return;
	}

	uint32 counter = _midiPlyrDriver ? _midiPlyrDriver->getVocCounter() :
		(uint32)((_mixer->getSoundElapsedTime(_soundHandle) + 9) / 10);
	soundServiceWaveBuffers();

	int32 compareCounter = (int32)(counter - 2);
	bool phaseCueSeen = false;
	while (_soundNextCue < _soundCues.size() && compareCounter > (int32)_soundCues[_soundNextCue].counterThreshold &&
			(_soundCues[_soundNextCue].value == 1 || _soundCues[_soundNextCue].value == 2)) {
		phaseCueSeen = true;
		_soundEventSubIndex = _soundCues[_soundNextCue].value;
		debug(5, "COMFY VOC: cue=%u threshold=%u counter=%u index=%u/%u",
			_soundEventSubIndex, _soundCues[_soundNextCue].counterThreshold, counter,
			_soundNextCue, (uint)_soundCues.size());
		_soundNextCue++;
	}

	if (phaseCueSeen || _soundNextCue == _soundCues.size() ||
			compareCounter <= (int32)_soundCues[_soundNextCue].counterThreshold)
		return;

	_soundEventSubIndex = _soundCues[_soundNextCue].value;
	debug(5, "COMFY VOC: cue=%u threshold=%u counter=%u index=%u/%u",
		_soundEventSubIndex, _soundCues[_soundNextCue].counterThreshold, counter,
		_soundNextCue, (uint)_soundCues.size());
	_soundNextCue++;
	if (!_soundEventSubIndex) {
		_mixer->stopHandle(_soundHandle);
		_soundQueueStream = nullptr;
		_soundCues.clear();
		_soundNextCue = 0;
		if (_midiPlyrDriver) {
			_midiPlyrDriver->vocSrResetBlockNo();
		}
	}
}


} // End of namespace Comfy
