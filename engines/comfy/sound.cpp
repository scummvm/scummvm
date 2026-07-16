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

#define COMFY_ANM_COMMAND_END 0x2145
#define COMFY_ANM_COMMAND_STORE_FRAME_BYTES 0x4356
#define COMFY_ANM_COMMAND_FRAME 0x5246

namespace Comfy {

struct SoundDecoderState {
public:
	// soundPrepareDecoderState() collects an embedded ANM sound into one temporary buffer.
	// This constructor copies those bytes, so decoding no longer needs the engine or a file...
	SoundDecoderState(const byte *data, uint32 size, bool compressed) {
		_data.resize(size);
		if (size)
			memcpy(&_data[0], data, size);

		_compressed = compressed;
		_end = size;
		_windowEnd = size;

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

	// A regular VOC entry remains in VOCFILE.DAT. This constructor retains the engine and
	// file object so loadSourceWindow() can read each block through objFileReadTiledCore().
	SoundDecoderState(ComfyEngine *engine, ComfyEngine::ObjFile *objectFile, uint32 sourceBase, uint32 size, bool compressed) {
		_engine = engine;
		_objectFile = objectFile;
		_sourceBase = sourceBase;
		_compressed = compressed;
		_end = size;

		if (!loadSourceWindow(6)) {
			_fault = true;
			return;
		}

		if (_compressed) {
			if (size < 5) {
				_fault = true;
				return;
			}

			setSample(sourceByte(4));
			_position = 5;
		} else if (size >= 6 && sourceByte(0) == 1) {
			setSample(sourceByte(4));
		} else {
			setSample(_sample);
		}
	}

	uint32 decode(byte *buffer, uint32 size) {
		_decodeBlockSize = size;
		if (_objectFile && !_started && !loadSourceWindow(size))
			return 0;

		uint32 written = 0;
		while (written < size && !_fault && !_finished) {
			byte sample = 0x80;
			if (!readSample(sample))
				break;

			buffer[written++] = sample;
		}

		return written;
	}

	uint32 getRate() {
		return _sampleRate;
	}

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

		while (nextCue < cues.size() && packedCueCount < 4 && cues[nextCue].streamPosition < playbackPosition) {
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

		if (_position > _end || _loopPosition > _end || packedEnd > _end)
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

	bool loadSourceWindow(uint32 size) {
		if (!_objectFile)
			return true;

		if (!_engine || _position > _end)
			return false;

		uint32 remaining = _end - _position;
		uint32 readSize = MIN<uint32>(size, remaining);

		if (!_compressed)
			readSize += 0xC8;

		uint32 fileOffset = _sourceBase + _position;
		if (fileOffset > _objectFile->fileSize)
			return false;

		if (readSize > _objectFile->fileSize - fileOffset)
			readSize = _objectFile->fileSize - fileOffset;

		if (!readSize) {
			_data.clear();
			_windowStart = _position;
			_windowEnd = _position;
			return true;
		}

		byte *source = _engine->objFileReadTiledCore(fileOffset, readSize, _objectFile);
		if (!source)
			return false;

		_data.resize(readSize);
		memcpy(&_data[0], source, readSize);
		_windowStart = _position;
		_windowEnd = _position + readSize;
		return true;
	}

	byte sourceByte(uint32 position) {
		if (position >= _end) {
			_fault = true;
			return 0;
		}

		if (_objectFile && (position < _windowStart || position >= _windowEnd)) {
			uint32 savedPosition = _position;
			_position = position;
			bool loaded = loadSourceWindow(_decodeBlockSize ? _decodeBlockSize : 6);
			_position = savedPosition;

			if (!loaded) {
				_fault = true;
				return 0;
			}
		}

		uint32 offset = position - _windowStart;
		if (offset >= _data.size()) {
			_fault = true;
			return 0;
		}

		return _data[offset];
	}

	static uint32 calcSampleRate(byte sample) {
		uint32 denominator = 256 - sample;
		uint32 rate = denominator ? 1000000 / denominator : 11025;
		if (rate > 10000 && rate < 12000)
			return 11025;

		if (rate > 20000 && rate < 24000)
			return 22050;

		return rate;
	}

	void setSample(byte sample) {
		_sample = sample;
		_sampleRate = calcSampleRate(sample);
		if (_sampleRate == 11025)
			_sample = 0xA5;
		else if (_sampleRate == 22050)
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
					// The original reloads the loop position twice...
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

		if (_objectFile && !loadSourceWindow(_decodeBlockSize))
			_fault = true;
	}

	uint16 parseCommand() {
		_eventArgument = 0;
		if (!_compressed) {
			if (_position >= _end) {
				_fault = true;
				return 0;
			}

			uint16 command = sourceByte(_position++);
			if (command) {
				if (_end - _position < 3) {
					_fault = true;
					return 0;
				}

				_eventArgument = (uint32)sourceByte(_position) | ((uint32)sourceByte(_position + 1) << 8) |
					((uint32)sourceByte(_position + 2) << 16);
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
		setSample(sourceByte(_position));
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

			sample = sourceByte(_position++);
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
					byte predictor = _history.size() >= _predictorDistance ? _history[_history.size() - _predictorDistance] : 0x80;
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

			value |= (uint32)((sourceByte(_position) >> _bitOffset) & 1) << bit;
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

		uint16 value = (uint16)(sourceByte(_position) | ((uint16)sourceByte(_position + 1) << 8));
		_position += 2;
		return value;
	}

	byte readByte() {
		if (_position >= _end) {
			_fault = true;
			return 0;
		}

		return sourceByte(_position++);
	}

	void pushCue(uint16 value) {
		// The original keeps at most 0x13 pending pitch entries, but consumes them
		// while refilling wave buffers. Our decoder stores the full cue timeline,
		// so applying that pending-entry limit here would drop later cues...
		ComfyEngine::SoundCue cue;
		cue.value = value;
		cue.streamPosition = _streamPosition;
		cue.counterThreshold = _sampleRate ? (_streamPosition * 100) / _sampleRate : 0;
		_cues.push_back(cue);
	}

	Common::Array<byte> _data;
	Common::Array<byte> _history;
	Common::Array<ComfyEngine::SoundCue> _cues;
	ComfyEngine *_engine = nullptr;
	ComfyEngine::ObjFile *_objectFile = nullptr;
	uint32 _sourceBase = 0;
	uint32 _windowStart = 0;
	uint32 _windowEnd = 0;
	uint32 _decodeBlockSize = 0;
	uint32 _end = 0;
	uint32 _position = 0;
	uint32 _streamPosition = 0;
	uint32 _eventArgument = 0;
	uint32 _eventRemaining = 0;
	uint32 _fillRemaining = 0;
	uint32 _loopPosition = 0;
	uint32 _sampleRate = 11025;
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

void ComfyEngine::setWaveBalancePercent(uint16 value) {
	if ((int16)value < 0)
		value = 0;
	else if (value > 100)
		value = 100;

	_v3SceneWaveBalancePercent = (byte)value;
	_v3WaveBalancePercent = value;
}

void ComfyEngine::setWaveLeftPercent(uint16 value) {
	if (value == 0x00FF)
		return;

	if ((int16)value < 0)
		value = 0;
	else if (value > 100)
		value = 100;

	_v3SceneWaveLeftPercent = (byte)value;
	_v3WaveLeftPercent = value;
}

void ComfyEngine::setWaveRightPercent(uint16 value) {
	if (value == 0x00FF)
		return;

	if ((int16)value < 0)
		value = 0;
	else if (value > 100)
		value = 100;

	_v3SceneWaveRightPercent = (byte)value;
	_v3WaveRightPercent = value;
}

void ComfyEngine::setMixerVolumePercent(uint16 value) {
	if (value == 0x00FF)
		return;

	if ((int16)value < 0)
		value = 0;
	else if (value > 100)
		value = 100;

	_v3SceneMixerVolumePercent = (byte)value;
	_v3MixerVolumePercent = value;
}

void ComfyEngine::setMediaRangePercent(uint16 value) {
	if ((int16)value < 0)
		value = 0;
	else if (value > 100)
		value = 100;

	_v3MixerAltPercent = value;
}

void ComfyEngine::setMediaMode(byte mode) {
	if (mode == 1 || mode == 3) {
		_v3SceneMediaModeEnabled = 1;

		if (!_v3MediaValueAvailable) {
			_v3MediaValue = 0;
			_v3MediaValueAvailable = true;
		}
	} else if (mode == 2) {
		_v3SceneMediaModeEnabled = 0;
		_v3MediaValue = 0;
		_v3MediaValueAvailable = false;
	}
}

void ComfyEngine::restoreWaveStateAfterSceneStart() {
	setWaveBalancePercent(_v3SceneWaveBalancePercent);
	setWaveLeftPercent(_v3SceneWaveLeftPercent);
	setWaveRightPercent(_v3SceneWaveRightPercent);
	setMixerVolumePercent(_v3SceneMixerVolumePercent);

	if (_v3SceneMediaModeEnabled) {
		setMediaMode(2);
		setMediaMode(1);
	} else {
		setMediaMode(2);
	}
}

bool ComfyEngine::soundOpenVocFile() {
	objFileClose(_vocFile);
	uint32 maximumBytes = 0x40000;
	if (sysGetExtMemKB() < 4000)
		maximumBytes /= 2;

	_vocFile = objFileOpen(Common::Path("VOCFILE.DAT"), 0x8000, maximumBytes);
	if (!_vocFile)
		return false;

	byte header[4];
	memset(header, 0, sizeof(header));
	objFileReadFieldCore(header, 0, sizeof(header), _vocFile);

	if (header[0] != 'C' || (header[1] != 'V' && header[1] != 'W')) {
		objFileClose(_vocFile);
		return false;
	}

	_soundCompressed = header[1] == 'W';
	_soundUsesAnimData = false;
	_soundEntryCount = READ_LE_UINT16(header + 2);
	return true;
}

bool ComfyEngine::soundInit() {
	if (!soundOpenVocFile())
		return false;

	_soundEventIndex = 0;
	_soundEventMaximum = 0;
	_soundEventSubIndex = 0xFFFF;
	_soundEventPreviousSubIndex = 0xFFFF;
	_soundTimingPaused = false;
	_soundVocBlockCount = 0;
	_soundVocCounterSnapshot = 0;
	_soundVocTimingDelta = 0;

	for (uint i = 0; i < COMFY_VOC_QUEUE_CAPACITY; i++) {
		_vocQueue[i] = VocQueueEntry();
		_vocQueueV3[i] = VocQueueEntryV3();
	}

	_v3SceneWaveBalancePercent = 0;
	_v3SceneMediaModeEnabled = 0;
	_v3SceneWaveLeftPercent = 0;
	_v3SceneWaveRightPercent = 0;
	_v3SceneMixerVolumePercent = 0;
	return true;
}

void ComfyEngine::soundShutdown() {
	_mixer->stopHandle(_soundHandle);
	_soundQueueStream = nullptr;
	delete _soundDecoderState;
	_soundDecoderState = nullptr;
	_soundCues.clear();
	objFileClose(_vocFile);
	_soundEntryCount = 0;
	_soundNextCue = 0;
	_soundPaused = false;
	_soundUsesAnimData = false;
	_soundTimingPaused = false;
	_soundVocBlockCount = 0;
	_soundVocCounterSnapshot = 0;
	_soundVocTimingDelta = 0;

	if (_midiPlyrDriver)
		_midiPlyrDriver->setIncreaseVocCounter(0);
}

void ComfyEngine::soundHeaderReadFromXms(byte *destination, uint16 index, uint16 size) {
	objHdrReadFromXms(destination, _headerXmsSoundHeadersBase, size, index);
}

void ComfyEngine::soundUpdateVocTiming() {
	if (_soundTimingPaused || !_midiPlyrDriver)
		return;

	uint32 timeFrac = 0;
	uint16 blockNo = 0;

	_midiPlyrDriver->vocSrGetCounters(timeFrac, blockNo);

	if (!blockNo)
		return;

	_soundVocBlockCount += blockNo;
	timeFrac -= _soundVocCounterSnapshot;
	_midiPlyrDriver->vocSrResetBlockNo();
	uint32 sampleRate = _soundSampleRate ? _soundSampleRate : 11025;
	uint32 expected = ((uint32)_soundVocBlockCount * 0x186A00) / sampleRate;
	_soundVocTimingDelta = (int16)((uint16)expected - (uint16)timeFrac);
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
	soundHeaderReadFromXms(header, index, sizeof(header));
	uint32 size = READ_LE_UINT32(header);
	uint32 offset = READ_LE_UINT32(header + 4);
	return _vocFile && offset <= _vocFile->fileSize && size <= _vocFile->fileSize - offset;
}

bool ComfyEngine::soundPrepareDecoderState(uint16 index) {
	Common::Array<byte> animationSoundData;
	byte *data = nullptr;
	ObjFile *objectFile = nullptr;
	uint32 fileSize = 0;
	uint32 dataSize = 0;
	uint32 dataOffset = 0;
	bool compressed = _soundCompressed;

	if (index == 0xFFFF) {
		uint32 storedSize = READ_LE_UINT32(_animFrameHeader + 4);
		if (!storedSize)
			return false;

		animationSoundData.resize(storedSize);
		if (storedSize <= _animFrameStorage.size()) {
			memcpy(&animationSoundData[0], &_animFrameStorage[0], storedSize);
		} else {
			// The original streams embedded ANM sound through the file object while VC
			// chunks arrive; our decoder owns its source buffer, so we rebuild only
			// the embedded sound stream without changing the animation command cursor...
			if (_animCurrentIndex >= _animIndexTable.size())
				return false;

			uint32 copied = 0;
			uint32 position = _animIndexTable[_animCurrentIndex] + COMFY_ANMFILE_HEADER_BYTES;
			while (copied < storedSize) {
				uint32 headerSize = _animPantherFormat ? 8 : 4;

				if (position > _animFileData.size() || headerSize > _animFileData.size() - position)
					return false;

				byte *header = &_animFileData[position];
				uint16 command = READ_LE_UINT16(header);
				uint32 rawCommandSize = _animPantherFormat ? READ_LE_UINT32(header + 2) : READ_LE_UINT16(header + 2);
				int32 signedCommandSize = _animPantherFormat ? (int32)rawCommandSize : (int32)(uint16)rawCommandSize;
				uint32 commandSize = rawCommandSize;

				if (_animPantherFormat && command == COMFY_ANM_COMMAND_FRAME && signedCommandSize <= (int32)headerSize) {
					position += headerSize;
					continue;
				}

				if (commandSize < headerSize || commandSize > _animFileData.size() - position)
					return false;

				uint32 payloadSize = commandSize - headerSize;
				if (command == COMFY_ANM_COMMAND_STORE_FRAME_BYTES) {
					uint32 copySize = MIN<uint32>(payloadSize, storedSize - copied);
					if (copySize)
						memcpy(&animationSoundData[copied], header + headerSize, copySize);

					copied += copySize;
				} else if (command == COMFY_ANM_COMMAND_END) {
					break;
				}

				position += commandSize;
			}

			if (copied < storedSize)
				return false;
		}

		data = &animationSoundData[0];
		fileSize = animationSoundData.size();
		dataSize = fileSize;
		compressed = false;
		_soundUsesAnimData = true;
	} else {
		byte header[8];
		memset(header, 0, sizeof(header));
		soundHeaderReadFromXms(header, index, sizeof(header));
		dataSize = READ_LE_UINT32(header);
		dataOffset = READ_LE_UINT32(header + 4);

		if (!_vocFile || dataOffset > _vocFile->fileSize || dataSize > _vocFile->fileSize - dataOffset)
			return false;

		objectFile = _vocFile;
		fileSize = _vocFile->fileSize;
		_soundUsesAnimData = false;
	}

	if (dataOffset > fileSize || dataSize > fileSize - dataOffset)
		return false;

	delete _soundDecoderState;

	if (objectFile) {
		_soundDecoderState = new SoundDecoderState(this, objectFile, dataOffset, dataSize, compressed);
	} else {
		_soundDecoderState = new SoundDecoderState(data + dataOffset, dataSize, compressed);
	}

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

	uint32 vocCounter = _midiPlyrDriver ? _midiPlyrDriver->getVocCounter() : (uint32)((_mixer->getSoundElapsedTime(_soundHandle) + 9) / 10);
	uint32 playbackPosition = (vocCounter * _soundSampleRate) / 100;
	uint16 bufferIndex = _soundWaveBufferIndex;
	if ((int32)(_soundDecoderState->getBufferStreamPosition(bufferIndex) - COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES) < (int32)playbackPosition)
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

void ComfyEngine::soundPlayEntry(uint16 index) {
	_mixer->stopHandle(_soundHandle);
	_soundQueueStream = nullptr;

	if (!soundPrepareDecoderState(index)) {
		_soundEventSubIndex = 0;
		return;
	}

	_soundTimingPaused = false;
	_soundVocBlockCount = 0;
	_soundVocCounterSnapshot = 0;
	_soundVocTimingDelta = 0;

	byte *buffers[2] = {nullptr, nullptr};
	uint32 decodedSizes[2] = {0, 0};

	if (!soundDecodePcmBlock(0, buffers[0], decodedSizes[0]) || !soundDecodePcmBlock(1, buffers[1], decodedSizes[1])) {
		free(buffers[0]);
		free(buffers[1]);
		delete _soundDecoderState;
		_soundDecoderState = nullptr;
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
		_soundEventSubIndex = 0;
		return;
	}

	_soundQueueStream->queueBuffer(buffers[0], decodedSizes[0], DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	_soundQueueStream->queueBuffer(buffers[1], decodedSizes[1], DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, _soundQueueStream, -1,
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);

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

	soundUpdateVocTiming();

	if (!_mixer->isSoundHandleActive(_soundHandle)) {
		_soundQueueStream = nullptr;

		if (_soundDecoderState && _soundEventSubIndex != 0) {
			_soundEventSubIndex = 0;
		}

		return;
	}

	uint32 counter = _midiPlyrDriver ? _midiPlyrDriver->getVocCounter() : (uint32)((_mixer->getSoundElapsedTime(_soundHandle) + 9) / 10);
	soundServiceWaveBuffers();

	int32 compareCounter = (int32)(counter - 2);
	bool phaseCueSeen = false;

	while (_soundNextCue < _soundCues.size() && compareCounter > (int32)_soundCues[_soundNextCue].counterThreshold &&
			(_soundCues[_soundNextCue].value == 1 || _soundCues[_soundNextCue].value == 2)) {
		phaseCueSeen = true;
		_soundEventSubIndex = _soundCues[_soundNextCue].value;
		_soundNextCue++;
	}

	if (phaseCueSeen || _soundNextCue == _soundCues.size() || compareCounter <= (int32)_soundCues[_soundNextCue].counterThreshold)
		return;

	_soundEventSubIndex = _soundCues[_soundNextCue].value;
	_soundNextCue++;

	if (!_soundEventSubIndex) {
		_mixer->stopHandle(_soundHandle);
		_soundQueueStream = nullptr;
		_soundCues.clear();
		_soundNextCue = 0;
		_soundVocTimingDelta = 0;
		if (_midiPlyrDriver) {
			_midiPlyrDriver->vocSrResetBlockNo();
		}
	}
}

} // End of namespace Comfy
