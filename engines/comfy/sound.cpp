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

#define SOUND_CMD_START 0
#define SOUND_CMD_BEGIN_EVENT 1
#define SOUND_CMD_BEGIN_LOOP_EVENT 2
#define SOUND_CMD_FILL_OR_SAMPLE_DELAY 3
#define SOUND_CMD_PITCH_CHANGE 4
#define SOUND_CMD_LOOP_START 6
#define SOUND_CMD_LOOP_END 7
#define SOUND_CMD_SET_SAMPLE 99

namespace Comfy {

byte ComfyEngine::soundOpenEntry(uint16 index) {
	uint32 dataSize = 0;
	uint32 dataOffset = 0;

	_soundFault = false;
	_soundFinished = false;
	_soundHistory.clear();
	_soundSourceWindowStart = 0;
	_soundSourceWindowEnd = 0;
	_soundDecoderData.clear();
	_soundUsesAnimData = index == 0xFFFF;
	_soundRate = 0;

	if (_soundUsesAnimData) {
		uint32 storedSize = READ_LE_UINT32(_animFrameHeader + 4);
		if (!storedSize) {
			_soundFault = true;
			return 0;
		}

		dataSize = storedSize;
		_soundDataBase = 0;
		_soundHeaderEnd = dataSize;
		_soundDataEnd = dataSize;
		_soundCompressed = false;
		animFileRestoreSoundStorage(0);

		if (_soundFault)
			return 0;
	} else {
		byte header[8];
		memset(header, 0, sizeof(header));
		soundReadHeader(header, index, sizeof(header));
		dataSize = READ_LE_UINT32(header);
		dataOffset = READ_LE_UINT32(header + 4);

		if (!_vocFile || dataOffset > _vocFile->fileSize || dataSize > _vocFile->fileSize - dataOffset) {
			_soundFault = true;
			return 0;
		}

		_soundDataBase = dataOffset;
		_soundSourceBase = dataOffset;
		_soundHeaderEnd = dataSize;
	}

	_soundDataEnd = dataSize;
	_soundPlayPosition = 0;
	_soundCursor = 0;
	_soundBlockBase = 0;
	_soundLoopPosition = 0;
	_soundBitOffset = 0;
	_soundLoopBitOffset = 0;
	_soundBlockOffset = 0;
	_soundBitAccumulator = 1;
	_soundBitWidth = 4;
	_soundPredictorDistance = 1;
	_soundNeedInit = false;
	_soundFillCount = 0;
	_soundEventLength = 0;
	_soundEventRemaining = 0;
	_soundLoopCount = 0;
	_soundStarted = false;
	_soundDecodeBlockSize = _isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES :
		(_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES);

	if (_soundCompressed) {
		if (dataSize < 5) {
			_soundFault = true;
			return 0;
		}

		byte compressedHeader[4];
		objFileReadFieldCore(compressedHeader, dataOffset, sizeof(compressedHeader), _vocFile);
		_soundHeaderEnd = READ_LE_UINT32(compressedHeader);
		objFileReadFieldCore(&_soundSample, dataOffset + 4, 1, _vocFile);
		_soundPlayPosition = 5;
	}

	return _soundFault ? 0 : _soundSample;
}


void ComfyEngine::setWaveBalancePercent(uint16 value) {
	if ((int16)value < 0) {
		value = 0;
	} else if (value > 100) {
		value = 100;
	}

	_v3SceneWaveBalancePercent = (byte)value;
	_v3WaveBalancePercent = value;
}

void ComfyEngine::setWaveLeftPercent(uint16 value) {
	if (value == 0x00FF)
		return;

	if ((int16)value < 0) {
		value = 0;
	} else if (value > 100) {
		value = 100;
	}

	_v3SceneWaveLeftPercent = (byte)value;
	_v3WaveLeftPercent = value;
}

void ComfyEngine::setWaveRightPercent(uint16 value) {
	if (value == 0x00FF)
		return;

	if ((int16)value < 0) {
		value = 0;
	} else if (value > 100) {
		value = 100;
	}

	_v3SceneWaveRightPercent = (byte)value;
	_v3WaveRightPercent = value;
}

void ComfyEngine::setMixerVolumePercent(uint16 value) {
	if (value == 0x00FF)
		return;

	if ((int16)value < 0) {
		value = 0;
	} else if (value > 100) {
		value = 100;
	}

	_v3SceneMixerVolumePercent = (byte)value;
	_v3MixerVolumePercent = value;
}

void ComfyEngine::setMediaRangePercent(uint16 value) {
	if ((int16)value < 0) {
		value = 0;
	} else if (value > 100) {
		value = 100;
	}

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
	soundFreeBuffer();
	uint32 maximumBytes = 0x40000;

	if (sysGetExtMemKB() < 4000)
		maximumBytes /= 2;

	_vocFile = soundLoadSoundData(Common::Path("VOCFILE.DAT"), 0x8000, maximumBytes);
	if (!_vocFile)
		return false;

	byte header[4];
	memset(header, 0, sizeof(header));
	objFileReadFieldCore(header, 0, sizeof(header), _vocFile);

	if (header[0] != 'C' || (header[1] != 'V' && header[1] != 'W')) {
		objFileClose(_vocFile);
		return false;
	}

	_soundCompressed = !_isPanther && header[1] == 'W';
	_soundUsesAnimData = false;
	_soundTileStride = READ_LE_UINT16(header + 2);
	return true;
}

bool ComfyEngine::soundInit() {
	_soundEventIndex = 0;
	_soundEventMaximum = 0;
	waveOutOpen();

	return soundOpenVocFile();
}

bool ComfyEngine::vocQueueInit() {
	_soundEventIndex = 0;
	_soundEventMaximum = 0;

	if (_isPanther) {
		soundInitRuntimeState(&_sceneMemoryBlock[_sceneSoundStateOffset + 0x6C]);
	} else if (_engineVersion == 3) {
		soundInitRuntimeState(&_sceneMemoryBlock[_sceneMidiInstanceOffset + COMFY_SCENE_MIDI_INSTANCE_BYTES + 0x1FB]);
	}

	soundInitOutput();

	if (!soundOpenVocFile())
		return false;

	if (_engineVersion == 3)
		setMediaMode(2);

	return true;
}

void ComfyEngine::soundInitOutput() {
	uint32 blockSize = _isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES :
		(_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES);

	soundAllocBuffers((uint16)blockSize);
	_soundStopped = true;
}

void ComfyEngine::waveOutOpen() {
	soundAllocBuffers((uint16)COMFY_SOUND_PCM_BLOCK_BYTES);
	_soundStopped = true;
}

void ComfyEngine::soundAllocBuffers(uint16 size) {
	// The original allocates one block with 0x80 bytes of headroom and uses two
	// aliases into it for the compressed decoder's interleave operation...
	_soundPcmBlocks[0].resize(size);
	_soundPcmBlocks[1].resize(size);
	_soundInterleaveStorage.resize((uint32)size * 4 + 0x80);
	_soundInterleavePointer = &_soundInterleaveStorage[0] + 0x80;
}

void ComfyEngine::soundFreePcmBuf() {
	_soundPcmBlocks[0].clear();
	_soundPcmBlocks[1].clear();
	_soundInterleaveStorage.clear();
	_soundInterleavePointer = nullptr;
	_soundRate = 0;
}

void ComfyEngine::soundPauseOutput() {
	if (_engineVersion == 3)
		_soundCounterEnabled = false;

	if (_midiPlyrDriver)
		_midiPlyrDriver->setIncreaseVocCounter(0);

	_soundPaused = true;

	if ((_engineVersion != 3 && !_isPanther) || !_soundStopped)
		waveOutPause();
}

void ComfyEngine::soundResumeOutput() {
	if (_engineVersion == 3) {
		_soundCounterEnabled = true;
	}

	if (_midiPlyrDriver)
		_midiPlyrDriver->setIncreaseVocCounter(1);

	_soundPaused = false;

	if (_engineVersion == 3) {
		if (!_soundStopped)
			waveOutRestart();

		_soundTimingPaused = true;
	} else if (!_isPanther || !_soundStopped) {
		waveOutRestart();
	}

	if (_isPanther)
		_soundTimingPaused = true;
}

void ComfyEngine::soundPauseOrResume(byte pause) {
	if (pause) {
		soundPauseOutput();
	} else {
		soundResumeOutput();
	}
}

void ComfyEngine::soundStopAllAndReset() {
	_soundStopped = true;
	waveOutReset();
	_soundCues.clear();
}

void ComfyEngine::soundPrepareQueuePlayback() {
	soundStopAllAndReset();
	_soundCounterEnabled = false;
	_soundEventSubIndex = 0;
	_midiCounterAdjustment = 0;

	if (_midiPlyrDriver)
		_midiPlyrDriver->vocSrResetBlockNo();
}

void ComfyEngine::soundStopPlayback() {
	soundStopAllAndReset();

	if (_isPanther) {
		_soundVocTimingDelta = 0;

		if (_midiPlyrDriver)
			_midiPlyrDriver->vocSrResetBlockNo();
	}
}

void ComfyEngine::soundCloseVocFile() {
	objFileClose(_vocFile);
}

void ComfyEngine::soundFreeBuffer() {
	objFileClose(_vocFile);
}

ComfyEngine::ObjFile *ComfyEngine::soundLoadSoundData(const Common::Path &path, uint16 blockSize, uint32 maximumBytes) {
	return objFileOpen(path, blockSize, maximumBytes);
}

void ComfyEngine::waveOutReset() {
	_mixer->stopHandle(_soundHandle);
	_soundQueueStream = nullptr;
}

void ComfyEngine::waveOutPause() {
	_mixer->pauseHandle(_soundHandle, true);
}

void ComfyEngine::waveOutRestart() {
	_mixer->pauseHandle(_soundHandle, false);
}

void ComfyEngine::soundShutdown() {
	if (_isPanther) {
		soundStopPlayback();
	} else {
		soundStopAllAndReset();
	}

	soundFreePcmBuf();
	_soundDecoderData.clear();
	_soundHistory.clear();
	_soundFault = false;
	_soundFinished = false;
	_soundCues.clear();

	if (_isPanther) {
		soundCloseVocFile();
	} else {
		soundFreeBuffer();
	}

	_soundTileStride = 0;
	_soundHeaderEnd = 0;
	_soundStopped = true;
	_soundPaused = false;
	_soundCounterEnabled = false;
	_soundUsesAnimData = false;
	_soundTimingPaused = false;
	_soundVocBlockCount = 0;
	_soundVocCounterSnapshot = 0;
	_soundPackedState = nullptr;
	_soundBufferState = nullptr;

	if (_midiPlyrDriver)
		_midiPlyrDriver->setIncreaseVocCounter(0);
}

void ComfyEngine::soundReadHeader(byte *destination, uint16 index, uint16 size) {
	objHdrReadFromXms(destination, _headerXmsSoundHeadersBase, size, index);
}

byte *ComfyEngine::soundReadTiledData() {
	return _vocFile ? objFileReadTiled(4, (uint32)_soundTileStride * 8, _vocFile) : nullptr;
}

void ComfyEngine::soundGetTileParams(uint16 *tileStride, uint16 *fieldCount) {
	if (tileStride)
		*tileStride = _soundTileStride;

	if (fieldCount)
		*fieldCount = 8;
}

bool ComfyEngine::soundLoadSourceWindow(uint32 size) {
	if (_soundUsesAnimData)
		return _soundPlayPosition <= _soundDataEnd;

	if (!_vocFile || _soundPlayPosition > _soundDataEnd)
		return false;

	uint32 remaining = _soundDataEnd - _soundPlayPosition;
	uint32 readSize = MIN<uint32>(size, remaining);

	if (!_soundCompressed)
		readSize += 0xC8;

	uint32 fileOffset = _soundSourceBase + _soundPlayPosition;
	if (fileOffset > _vocFile->fileSize)
		return false;

	if (readSize > _vocFile->fileSize - fileOffset)
		readSize = _vocFile->fileSize - fileOffset;

	if (!readSize) {
		_soundDecoderData.clear();
		_soundSourceWindowStart = _soundPlayPosition;
		_soundSourceWindowEnd = _soundPlayPosition;
		return true;
	}

	byte *source = objFileReadTiledCore(fileOffset, readSize, _vocFile);
	if (!source)
		return false;

	_soundDecoderData.resize(readSize);
	memcpy(&_soundDecoderData[0], source, readSize);
	_soundSourceWindowStart = _soundPlayPosition;
	_soundSourceWindowEnd = _soundPlayPosition + readSize;

	return true;
}

byte ComfyEngine::soundReadSourceByte(uint32 position) {
	if (position >= _soundDataEnd) {
		_soundFault = true;
		return 0;
	}

	if (_soundUsesAnimData) {
		if (position >= _soundDecoderData.size()) {
			_soundFault = true;
			return 0;
		}

		return _soundDecoderData[position];
	}

	if (position < _soundSourceWindowStart || position >= _soundSourceWindowEnd) {
		bool loaded = false;

		if (_isPanther || _engineVersion == 3) {
			uint32 savedPosition = _soundPlayPosition;
			uint32 savedCursor = _soundCursor;
			uint32 savedBlockBase = _soundBlockBase;
			uint16 savedBitOffset = _soundBitOffset;
			_soundPlayPosition = position;
			soundLoadDataBlock((uint16)(_soundDecodeBlockSize ? _soundDecodeBlockSize : 6));
			loaded = !_soundFault;
			_soundPlayPosition = savedPosition;
			_soundCursor = savedCursor;
			_soundBlockBase = savedBlockBase;
			_soundBitOffset = savedBitOffset;
		} else {
			uint32 savedPosition = _soundPlayPosition;
			_soundPlayPosition = position;
			loaded = soundLoadSourceWindow(_soundDecodeBlockSize ? _soundDecodeBlockSize : 6);
			_soundPlayPosition = savedPosition;
		}

		if (!loaded) {
			_soundFault = true;
			return 0;
		}
	}

	uint32 offset = position - _soundSourceWindowStart;
	if (offset >= _soundDecoderData.size()) {
		_soundFault = true;
		return 0;
	}

	return _soundDecoderData[offset];
}

uint32 ComfyEngine::soundCalcSampleRate(byte &sample) {
	if (_engineVersion != 3 && !_isPanther && _midiPlyrDriver) {
		uint32 timeFrac = 0;
		uint16 blockNo = 0;

		_midiPlyrDriver->vocSrGetCounters(timeFrac, blockNo);

		if (blockNo && timeFrac)
			return ((uint32)blockNo * 0x1E8480) / timeFrac;
	}

	uint32 denominator = 0x100 - sample;
	uint32 rate = 1000000 / denominator;

	if (rate > 10000 && rate < 12000) {
		rate = 11025;
		sample = 0xA5;
	} else if (rate > 20000 && rate < 24000) {
		rate = 22050;
		sample = 0xD3;
	}

	return rate;
}

uint16 ComfyEngine::soundBitRead(uint16 bits) {
	uint16 raw = (uint16)soundReadSourceByte(_soundCursor) | (uint16)(soundReadSourceByte(_soundCursor + 1) << 8);
	raw >>= _soundBitOffset;
	raw &= (uint16)((1U << bits) - 1);
	_soundBitOffset = (uint16)(_soundBitOffset + bits);
	_soundCursor += _soundBitOffset >> 3;
	_soundBitOffset &= 7;

	return raw;
}

uint32 ComfyEngine::soundBitRead32(uint16 bits) {
	uint32 raw = (uint32)soundReadSourceByte(_soundCursor) |
			((uint32)soundReadSourceByte(_soundCursor + 1) << 8) |
			((uint32)soundReadSourceByte(_soundCursor + 2) << 16) |
			((uint32)soundReadSourceByte(_soundCursor + 3) << 24);

	raw >>= _soundBitOffset;
	raw &= (1U << bits) - 1;
	_soundCursor += bits >> 3;
	_soundBitOffset = (uint16)(_soundBitOffset + (bits & 7));

	if (_soundBitOffset >= 8) {
		_soundBitOffset = (uint16)(_soundBitOffset - 8);
		_soundCursor++;
	}

	return raw;
}

void ComfyEngine::soundResetBitState() {
	_soundBitAccumulator = 1;
	_soundBitWidth = 4;
}

void ComfyEngine::soundReadFreqWord(uint16 &value) {
	if (_soundCompressed) {
		value = (uint16)soundBitRead32(16);
		return;
	}

	value = (uint16)(soundReadSourceByte(_soundCursor) | (soundReadSourceByte(_soundCursor + 1) << 8));
	_soundCursor += 2;
}

void ComfyEngine::soundReadParam(uint16 &value) {
	if (!_soundCompressed) {
		soundReadFreqWord(value);
		return;
	}

	switch (soundBitRead(2)) {
	case 0:
		value = 1;
		break;
	case 1:
		value = 2;
		break;
	case 2:
		value = soundBitRead(8);
		break;
	case 3:
		value = soundBitRead(16);
		break;
	default:
		value = 0;
		break;
	}
}

byte ComfyEngine::soundReadSampleByte() {
	return (byte)soundBitRead(8);
}

void ComfyEngine::soundSaveLoopPosition() {
	_soundLoopPosition = _soundPlayPosition;
	if (_soundCompressed)
		_soundLoopBitOffset = _soundBitOffset;
}

void ComfyEngine::soundSeekToLoop(uint16 size) {
	_soundPlayPosition = _soundLoopPosition;

	if (_isPanther || _engineVersion == 3) {
		soundLoadDataBlock(size);
	} else {
		soundDecompressBlock(size);
	}

	if (_soundCompressed)
		_soundBitOffset = _soundLoopBitOffset;
}

void ComfyEngine::soundReadLoopEntry(uint16 &frequency, byte &sample) {
	if (_soundCompressed) {
		frequency = (uint16)soundBitRead32(0x10);
		sample = _soundSample;
		return;
	}

	frequency = (uint16)(soundReadSourceByte(_soundCursor) | (soundReadSourceByte(_soundCursor + 1) << 8));
	sample = soundReadSourceByte(_soundCursor + 2);
	_soundCursor += 3;
}

void ComfyEngine::soundBeginEvent(bool loop) {
	_soundEventLength = (int32)_soundEventArgument;
	if (_soundCompressed) {
		_soundNeedInit = true;
		return;
	}

	if (loop)
		return;

	if (_soundEventLength < 2 || _soundCursor + 2 > _soundDataEnd) {
		_soundFault = true;
		return;
	}

	_soundEventLength -= 2;
	_soundSample = soundReadSourceByte(_soundCursor);
	_soundCursor += 2;
}

uint16 ComfyEngine::soundParseCommand() {
	_soundEventArgument = 0;

	if (!_soundCompressed) {
		uint16 command = soundReadSourceByte(_soundCursor++);
		if (command) {
			_soundEventArgument = (uint32)soundReadSourceByte(_soundCursor) |
					((uint32)soundReadSourceByte(_soundCursor + 1) << 8) |
					((uint32)soundReadSourceByte(_soundCursor + 2) << 16);
		}

		_soundCursor += 3;
		return command;
	}

	uint16 command = soundBitRead(2);
	bool hasArgument = false;
	switch (command) {
	case 0:
		switch (soundBitRead(2)) {
		case 0:
			command = SOUND_CMD_START;
			break;
		case 1:
			command = soundBitRead(8);
			hasArgument = true;
			break;
		case 2:
			command = SOUND_CMD_LOOP_END;
			break;
		case 3:
			command = SOUND_CMD_SET_SAMPLE;
			break;
		}

		break;
	case 1:
		command = SOUND_CMD_BEGIN_EVENT;
		hasArgument = true;
		break;
	case 2:
		command = SOUND_CMD_FILL_OR_SAMPLE_DELAY;
		break;
	case 3:
		command = SOUND_CMD_PITCH_CHANGE;
		break;
	}

	if (hasArgument)
		_soundEventArgument = soundBitRead32(20);

	return command;
}

void ComfyEngine::soundDecodeBits(uint16 size, byte mode) {
	if (!_soundWritePointer || !size)
		return;

	uint32 source = _soundCursor;
	uint16 bitOffset = _soundBitOffset;
	uint32 raw = (uint32)soundReadSourceByte(source) |
			((uint32)soundReadSourceByte(source + 1) << 8) |
			((uint32)soundReadSourceByte(source + 2) << 16) |
			((uint32)soundReadSourceByte(source + 3) << 24);

	raw >>= bitOffset & 7;

	if (mode & 1) {
		if (raw & 1) {
			raw >>= 1;
			_soundBitWidth = (byte)(raw & 7);
			bitOffset = (uint16)(bitOffset + 3);
			raw >>= 2;
		}

		raw >>= 1;
		bitOffset = (uint16)(bitOffset + 1);

		if (!_soundBitWidth) {
			source += bitOffset >> 3;
			bitOffset &= 7;

			while (size--) {
				uint16 sampleWord = (uint16)(soundReadSourceByte(source) | (soundReadSourceByte(source + 1) << 8));
				byte sample = (byte)(sampleWord >> bitOffset);
				_soundHistory.push_back(sample);
				*_soundWritePointer++ = sample;
				source++;
			}

			_soundCursor = source;
			_soundBitOffset = bitOffset;
			return;
		}

		if (raw & 1) {
			raw >>= 1;
			bitOffset = (uint16)(bitOffset + 7);
			// The original writes only the low byte of this word-sized global...
			_soundBitAccumulator = (byte)raw;
		}

		bitOffset = (uint16)(bitOffset + 1);
	}

	if (!_soundBitWidth) {
		source += bitOffset >> 3;
		bitOffset &= 7;

		while (size--) {
			uint16 sampleWord = (uint16)(soundReadSourceByte(source) | (soundReadSourceByte(source + 1) << 8));
			byte sample = (byte)(sampleWord >> bitOffset);
			_soundHistory.push_back(sample);
			*_soundWritePointer++ = sample;
			source++;
		}

		_soundCursor = source;
		_soundBitOffset = bitOffset;
		return;
	}

	uint32 predictorIndex = _soundHistory.size() >= _soundBitAccumulator ? _soundHistory.size() - _soundBitAccumulator : 0;
	uint16 mask = (uint16)((1U << _soundBitWidth) - 1);
	uint16 sign = (uint16)(1U << (_soundBitWidth - 1));

	if (source & 1) {
		source--;
		bitOffset = (uint16)(bitOffset + 8);
	}

	raw = (uint32)soundReadSourceByte(source) |
		((uint32)soundReadSourceByte(source + 1) << 8) |
		((uint32)soundReadSourceByte(source + 2) << 16) |
		((uint32)soundReadSourceByte(source + 3) << 24);

	raw >>= bitOffset;

	while (size--) {
		if (bitOffset & 0x10) {
			raw <<= bitOffset;
			source += 2;
			raw = (raw & 0xFFFF0000) |
				(uint16)(soundReadSourceByte(source + 2) |
				((uint16)soundReadSourceByte(source + 3) << 8));

			uint16 rotate = bitOffset & 31;

			if (rotate)
				raw = (raw >> rotate) | (raw << (32 - rotate));

			bitOffset = (uint16)(bitOffset - 0x10);
		}

		uint16 delta = (uint16)raw & mask;
		bitOffset = (uint16)(bitOffset + _soundBitWidth);
		raw >>= _soundBitWidth;
		byte sample;

		if (!delta) {
			sample = (byte)raw;
			raw >>= 8;
			bitOffset = (uint16)(bitOffset + 8);
		} else {
			byte predictor = predictorIndex < _soundHistory.size() ? _soundHistory[predictorIndex] : 0x80;
			sample = predictor + (byte)(delta - sign);
		}

		_soundHistory.push_back(sample);
		*_soundWritePointer++ = sample;
		predictorIndex++;
	}

	source += bitOffset >> 3;
	_soundCursor = source;
	_soundBitOffset = bitOffset & 7;
}

void ComfyEngine::soundCopyPcm(byte *destination, uint16 size) {
	_soundWritePointer = destination;

	if (_soundCompressed) {
		if (_soundNeedInit) {
			_soundNeedInit = false;
			soundResetBitState();

			if (size) {
				*_soundWritePointer++ = (byte)soundBitRead(8);
				_soundHistory.push_back(*(_soundWritePointer - 1));
				--size;
			}
		} else if (_soundBlockOffset < 0x20 && size) {
			uint16 copied = MIN<uint16>((uint16)(0x20 - _soundBlockOffset), size);
			soundDecodeBits(copied, 0);
			_soundBlockOffset = copied;
			size = (uint16)(size - copied);
		}

		while (size) {
			uint16 copied = MIN<uint16>(0x20, size);
			soundDecodeBits(copied, 1);
			size = (uint16)(size - copied);
			_soundBlockOffset = copied;
		}

		_soundWritePointer = nullptr;
		return;
	}

	while (size--) {
		byte sample = soundReadSourceByte(_soundCursor++);
		*_soundWritePointer++ = sample;
	}

	_soundWritePointer = nullptr;
}

void ComfyEngine::soundAdvancePlayPosition() {
	_soundPlayPosition += (uint16)(_soundCursor - _soundBlockBase);
}

void ComfyEngine::soundDecompressBlock(uint16 size) {
	if (_soundPlayPosition >= _soundDataEnd) {
		_soundFault = true;
		return;
	}

	_soundBlockBase = _soundPlayPosition;
	_soundCursor = _soundPlayPosition;
	_soundDecodeBlockSize = size;

	if (!_soundUsesAnimData && !soundLoadSourceWindow(size)) {
		_soundFault = true;
		return;
	}

	if (_soundCompressed) {
		if ((_soundRate & 1) == 0 && _soundRate > 0 && _soundInterleavePointer) {
			memcpy(_soundInterleavePointer - 0x80, _soundInterleavePointer + (uint32)size * 2 - 0x80, 0x80);
		}

		_soundRate++;
	}

}

void ComfyEngine::soundLoadDataBlock(uint16 size) {
	if (_soundPlayPosition >= _soundDataEnd) {
		_soundFault = true;
		return;
	}

	if (_soundUsesAnimData) {
		// The original reads this range through ANMFILE storage; We keep the
		// current reconstructed sound range in _soundDecoderData instead...
		_soundBlockBase = _soundPlayPosition;
		_soundCursor = _soundPlayPosition;
		_soundSourceWindowStart = 0;
		_soundSourceWindowEnd = _soundDecoderData.size();
		_soundDecodeBlockSize = size;
		return;
	}

	if (!_vocFile) {
		_soundFault = true;
		return;
	}

	uint32 sourcePosition = _soundDataBase + _soundPlayPosition;
	uint16 alignment = (sourcePosition & 1) ? 1 : 0;
	if (sourcePosition < alignment)
		alignment = 0;

	sourcePosition -= alignment;

	uint32 available = _soundDataEnd - _soundPlayPosition + alignment;
	uint32 readSize = MIN<uint32>((uint32)size + 0xC8, available);

	if (sourcePosition > _vocFile->fileSize) {
		readSize = 0;
	} else if (readSize > _vocFile->fileSize - sourcePosition) {
		readSize = _vocFile->fileSize - sourcePosition;
	}

	if (!readSize) {
		_soundFault = true;
		return;
	}

	byte *source = objFileReadTiledCore(sourcePosition, readSize, _vocFile);
	if (!source) {
		_soundFault = true;
		return;
	}

	uint32 logicalSize = readSize - alignment;
	_soundDecoderData.resize(logicalSize);
	memcpy(&_soundDecoderData[0], source + alignment, logicalSize);
	_soundSourceWindowStart = _soundPlayPosition;
	_soundSourceWindowEnd = _soundPlayPosition + logicalSize;
	_soundBlockBase = _soundPlayPosition;
	_soundCursor = _soundPlayPosition;
	_soundDecodeBlockSize = size;
}

void ComfyEngine::soundLoadChunk() {
	if (_soundUsesAnimData || !_soundDataEnd || !_vocFile)
		return;

	uint32 preloadOffset = _engineVersion == 3 ? 0x4000 : 0x5000;
	uint32 endMargin = _engineVersion == 3 ? 2 : 1;
	uint32 position = _soundPlayPosition + preloadOffset;
	uint32 lastPosition = _soundDataEnd > endMargin ? _soundDataEnd - endMargin : 0;

	if (position >= lastPosition)
		position = lastPosition;

	uint32 fileOffset = _soundDataBase + position;
	if (fileOffset > _vocFile->fileSize || 2 > _vocFile->fileSize - fileOffset)
		return;

	byte probe[2];
	objFileReadFieldCore(probe, fileOffset, sizeof(probe), _vocFile);
}

void ComfyEngine::soundPreloadChunk() {
	if (!_isPanther || _soundUsesAnimData || !_soundDataEnd || !_vocFile)
		return;

	uint32 position = _soundPlayPosition + 0x4000;
	if (position >= _soundDataEnd)
		position = _soundDataEnd > 2 ? _soundDataEnd - 2 : 0;

	uint32 fileOffset = _soundDataBase + position;
	if (fileOffset > _vocFile->fileSize || 2 > _vocFile->fileSize - fileOffset)
		return;

	byte preload[2];
	objFileReadFieldCore(preload, fileOffset, sizeof(preload), _vocFile);
}

void ComfyEngine::soundInitRuntimeState(byte *state) {
	_soundPackedState = state;
	soundSetBufferStatePointer(state ? state + 0x41 : nullptr);
}

void ComfyEngine::soundSetBufferStatePointer(byte *state) {
	_soundBufferState = state;
}

void ComfyEngine::soundSaveStreamPositions(uint16 bufferIndex) {
	bufferIndex &= 1;
	_soundBufferPosition[bufferIndex] = _soundPlayPosition;
	_soundBufferLoopPosition[bufferIndex] = _soundLoopPosition;
}

void ComfyEngine::soundRestoreBufferState() {
	if (!_soundBufferState)
		return;

	_soundPlayPosition = READ_LE_UINT32(_soundBufferState);
	_soundLoopPosition = READ_LE_UINT32(_soundBufferState + 4);
	_soundDataEnd = READ_LE_UINT32(_soundBufferState + 8);
	_soundDataBase = READ_LE_UINT32(_soundBufferState + 0x0C);
	_soundSourceBase = _soundDataBase;
	_soundUsesAnimData = _soundBufferState[0x10] != 0;

	if (_soundUsesAnimData) {
		animFileRestoreSoundStorage(_soundPlayPosition);
		_soundSourceWindowStart = 0;
		_soundSourceWindowEnd = _soundDecoderData.size();
	} else {
		_soundSourceWindowStart = 0;
		_soundSourceWindowEnd = 0;
	}
}

void ComfyEngine::soundSaveActiveBufferState(uint16 bufferIndex) {
	if (!_soundBufferState)
		return;

	_soundBufferStateIndex = bufferIndex & 1;
	WRITE_LE_UINT32(_soundBufferState, _soundBufferPosition[_soundBufferStateIndex]);
	WRITE_LE_UINT32(_soundBufferState + 4, _soundBufferLoopPosition[_soundBufferStateIndex]);
	WRITE_LE_UINT32(_soundBufferState + 8, _soundDataEnd);
	WRITE_LE_UINT32(_soundBufferState + 0x0C, _soundDataBase);
	_soundBufferState[0x10] = _soundUsesAnimData;
}

void ComfyEngine::soundSaveBufferState(uint16 bufferIndex) {
	_soundBufferStateIndex = bufferIndex & 1;
	_soundBufferStreamPosition[bufferIndex] = _soundStreamPosition;
	_soundBufferPosition[bufferIndex] = _soundPlayPosition;
	_soundBufferLoopPosition[bufferIndex] = _soundLoopPosition;
	_soundBufferEventRemaining[bufferIndex] = (uint32)_soundEventLength;
	_soundBufferFillRemaining[bufferIndex] = _soundFillCount;
	_soundBufferLoopCount[bufferIndex] = _soundLoopCount;
	_soundBufferStarted[bufferIndex] = _soundStarted;

	soundSaveStreamPositions(bufferIndex);
}

void ComfyEngine::soundPackCueEntries(byte *state, uint32 playbackPosition) {
	uint16 packedCueCount = 0;

	while (packedCueCount < _soundCues.size() && _soundCues[packedCueCount].streamPosition < playbackPosition) {
		SoundCue &cue = _soundCues[packedCueCount];
		WRITE_LE_UINT16(state + 0x15 + packedCueCount * 2, cue.value);
		WRITE_LE_UINT32(state + 0x1D + packedCueCount * 4, cue.streamPosition);
		WRITE_LE_UINT32(state + 0x2D + packedCueCount * 4, cue.counterThreshold);
		packedCueCount++;
	}

	WRITE_LE_UINT16(state + 0x3D, packedCueCount);
}

void ComfyEngine::soundPushCue(uint16 value, int32 streamPosition) {
	if (_engineVersion == 3) {
		if (_soundCues.size() == COMFY_V3_SOUND_MAX_CUES) {
			_soundCues.remove_at(_soundCues.size() - 1);
		}

		if (!_soundCues.empty() && _soundCues[_soundCues.size() - 1].value == value) {
			return;
		}
	} else if (_soundCues.size() >= COMFY_SOUND_MAX_CUES) {
		return;
	}

	_soundSampleRate = soundCalcSampleRate(_soundSample);
	SoundCue cue;
	cue.value = value;
	cue.streamPosition = streamPosition;
	cue.counterThreshold = _soundSampleRate ? (streamPosition * 100) / (int32)_soundSampleRate : 0;
	_soundCues.push_back(cue);
}

void ComfyEngine::soundFillPcmBuffer(uint16 bufferIndex, uint16 size) {
	if (bufferIndex >= 2 || _soundStopped)
		return;

	uint32 blockSize = _isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES :
		(_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES);

	uint16 fillSize = (_isPanther || _engineVersion == 3) ? size : (uint16)COMFY_SOUND_PCM_BLOCK_BYTES;

	if (_soundPcmBlocks[bufferIndex].size() != blockSize)
		_soundPcmBlocks[bufferIndex].resize(blockSize);

	if (_isPanther || _engineVersion == 3)
		soundSaveBufferState(bufferIndex);

	byte *destination = &_soundPcmBlocks[bufferIndex][0];
	uint16 destinationPosition = 0;
	uint16 remaining = fillSize;

	if (!_soundStarted) {
		if (_isPanther || _engineVersion == 3) {
			soundLoadDataBlock(size);
		} else {
			soundDecompressBlock((uint16)blockSize);
		}
	}

	while (remaining && !_soundFault) {
		uint16 copied = 0;
		if (_soundStarted) {
			_soundFillCount = fillSize;
		}

		if (_soundFillCount) {
			copied = MIN<uint16>(_soundFillCount, remaining);
			_soundFillCount = (uint16)(_soundFillCount - copied);
			memset(destination + destinationPosition, 0x80, copied);

			if (_soundCompressed) {
				for (uint16 i = 0; i < copied; i++)
				_soundHistory.push_back(0x80);
			}
		} else if (_soundEventLength) {
			if ((int32)remaining <= _soundEventLength) {
				copied = remaining;
			} else {
				copied = (uint16)_soundEventLength;
			}

			_soundEventLength -= copied;
			soundCopyPcm(destination + destinationPosition, copied);
		}

		if (copied) {
			destinationPosition = (uint16)(destinationPosition + copied);
			remaining = (uint16)(remaining - copied);
			_soundStreamPosition += copied;
			continue;
		}

		uint16 command = soundParseCommand();

		switch (command) {
		case SOUND_CMD_BEGIN_EVENT:
			soundBeginEvent(false);
			break;
		case SOUND_CMD_BEGIN_LOOP_EVENT:
			soundBeginEvent(true);
			break;
		case SOUND_CMD_PITCH_CHANGE: {
			uint16 value = 0;
			soundReadParam(value);
			soundPushCue(value, (int32)_soundStreamPosition);
			break;
		}
		case SOUND_CMD_FILL_OR_SAMPLE_DELAY: {
			uint16 fillCount = 0;
			byte nextSample = 0;
			soundReadLoopEntry(fillCount, nextSample);
			_soundFillCount = fillCount;

			if (_engineVersion == 3 && !_soundSample)
				_soundSample = nextSample;

			if (nextSample != _soundSample) {
				uint32 oldRate = soundCalcSampleRate(_soundSample);
				uint32 newRate = 1000000 / (0x100 - nextSample);
				if (newRate)
					_soundFillCount = (uint16)(((uint32)_soundFillCount * oldRate) / newRate);
			}

			break;
		}
		case SOUND_CMD_LOOP_START: {
			uint16 loopCount = 0;
			soundReadFreqWord(loopCount);
			_soundLoopCount = (int16)loopCount;
			soundSaveLoopPosition();
			break;
		}
		case SOUND_CMD_LOOP_END:
			if (_soundLoopCount > 0) {
				_soundLoopCount--;
				soundSeekToLoop((uint16)blockSize);
				soundSeekToLoop((uint16)blockSize);
			}

			break;
		case SOUND_CMD_SET_SAMPLE:
			_soundSample = soundReadSampleByte();
			break;
		case SOUND_CMD_START:
			soundPushCue(0, (int32)_soundStreamPosition);
			_soundStarted = true;
			break;
		default:
			break;
		}
	}

	_soundEventRemaining = (uint32)_soundEventLength;
	soundAdvancePlayPosition();
}

void ComfyEngine::soundUpdateVocTiming() {
	if (_soundTimingPaused || (_engineVersion == 3 && !_soundCounterEnabled) || !_midiPlyrDriver)
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

	if (_isPanther || _engineVersion == 3)
		sampleRate = soundCalcSampleRate(_soundSample);

	uint32 expected = ((uint32)_soundVocBlockCount * 0x186A00) / sampleRate;
	int16 timingDelta = (int16)((uint16)expected - (uint16)timeFrac);

	if (_isPanther) {
		_soundVocTimingDelta = timingDelta;
	} else {
		_midiCounterAdjustment = timingDelta;
	}
}

void ComfyEngine::soundServiceWaveBuffers() {
	if (!_soundQueueStream || !_mixer->isSoundHandleActive(_soundHandle))
		return;

	uint32 queuedBlocks = _soundQueueStream->numQueuedStreams();
	if (queuedBlocks >= 2)
		return;

	if (_soundStopped)
		return;

	if (_midiPlyrDriver) {
		_midiPlyrDriver->vocSrUpdateCounters();
	}

	if (_isPanther || _engineVersion == 3) {
		soundFillPcmBuffer(_soundWaveBufferIndex, (uint16)(_isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES : COMFY_V3_SOUND_PCM_BLOCK_BYTES));
		if (_soundFault)
			return;
	}

	if (!waveOutSubmitBuffer(_soundWaveBufferIndex))
		return;

	_soundWaveBufferIndex = 1 - _soundWaveBufferIndex;

	if (_isPanther) {
		soundPreloadChunk();
	} else if (_engineVersion == 3) {
		soundLoadChunk();
	}
}

bool ComfyEngine::soundLoadEntry(uint16 index) {
	byte header[8];
	byte scratch[2];
	memset(header, 0, sizeof(header));
	memset(scratch, 0, sizeof(scratch));

	soundReadHeader(header, index, sizeof(header));

	uint32 size = READ_LE_UINT32(header);
	uint32 offset = READ_LE_UINT32(header + 4);

	if (!_vocFile || offset > _vocFile->fileSize || size > _vocFile->fileSize - offset)
		return false;

	objFileReadFieldCore(scratch, offset, sizeof(scratch), _vocFile);

	return true;
}

void ComfyEngine::soundPlayFromPtr(const byte *source) {
	if (!source || !_vocFile)
		return;

	uint16 tileId = READ_LE_UINT16(source);
	tileLoadAndCache(_vocFile, source + 2, tileId);
}

bool ComfyEngine::waveOutSubmitBuffer(uint16 bufferIndex) {
	if (!_soundQueueStream)
		return false;

	if (bufferIndex >= 2)
		return false;

	if (!_isPanther && _engineVersion != 3) {
		soundFillPcmBuffer(bufferIndex, COMFY_SOUND_PCM_BLOCK_BYTES);
		if (_soundFault)
			return false;
	}

	uint32 blockSize = _isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES :
		(_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES);

	_soundQueueStream->queueBuffer(&_soundPcmBlocks[bufferIndex][0], blockSize, DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);

	return true;
}

void ComfyEngine::soundSetSampleRate(uint32 sampleRate) {
	if (sampleRate == _soundSampleRate)
		return;

	if (sampleRate > 9000 && sampleRate < 12000) {
		sampleRate = 11025;
	} else if (sampleRate > 20000 && sampleRate < 23000) {
		sampleRate = 22050;
	}

	_soundSampleRate = sampleRate;
}

bool ComfyEngine::waveOutStartPlayback() {
	soundSetSampleRate(soundCalcSampleRate(_soundSample));
	_soundQueueStream = Audio::makeQueuingAudioStream(_soundSampleRate, false);
	if (!_soundQueueStream)
		return false;

	uint32 blockSize = _isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES :
		(_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES);

	_soundQueueStream->queueBuffer(&_soundPcmBlocks[0][0], blockSize, DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);
	_soundQueueStream->queueBuffer(&_soundPcmBlocks[1][0], blockSize, DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _soundQueueStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	return true;
}

void ComfyEngine::soundPackState() {
	if (!_soundPackedState)
		return;

	if (_engineVersion == 3 && _musicEnabled)
		return;

	if (_soundStopped) {
		_soundPackedState[0x3F] = 1;
		return;
	}

	_soundSampleRate = soundCalcSampleRate(_soundSample);
	uint32 vocCounter = _midiPlyrDriver ? _midiPlyrDriver->getVocCounter() : (uint32)((_mixer->getSoundElapsedTime(_soundHandle) + 9) / 10);
	uint32 playbackPosition = (vocCounter * _soundSampleRate) / 100;
	uint16 bufferIndex = _soundWaveBufferIndex;
	uint32 blockSize = _isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES : (_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES);
	if ((int32)(_soundStreamPosition - blockSize) < (int32)playbackPosition)
		bufferIndex = 1 - bufferIndex;

	soundSaveActiveBufferState(bufferIndex);

	_soundPackedState[0x3F] = 0;
	WRITE_LE_UINT32(_soundPackedState, _soundBufferStreamPosition[bufferIndex]);
	_soundPackedState[4] = _soundBufferStarted[bufferIndex];
	WRITE_LE_UINT16(_soundPackedState + 5, _soundBufferFillRemaining[bufferIndex]);
	WRITE_LE_UINT32(_soundPackedState + 7, _soundBufferEventRemaining[bufferIndex]);
	WRITE_LE_UINT16(_soundPackedState + 0x0B, (uint16)_soundBufferLoopCount[bufferIndex]);
	WRITE_LE_UINT32(_soundPackedState + 0x0D, playbackPosition);
	WRITE_LE_UINT32(_soundPackedState + 0x11, vocCounter);

	soundPackCueEntries(_soundPackedState, playbackPosition);
	_soundPackedState[0x40] = _soundSample;
}

uint16 ComfyEngine::soundGetStateSize() {
	return _isPanther ? COMFY_PANTHER_SOUND_STATE_BYTES : 0x52;
}

void ComfyEngine::soundUnpackState() {
	if (!_soundPackedState)
		return;

	if (_engineVersion == 3) {
		if (_musicEnabled)
			return;

		soundPrepareQueuePlayback();

		if (_soundPackedState[0x3F]) {
			_soundStopped = true;
			_soundCues.clear();
			return;
		}

		_soundStopped = false;
		_soundFault = false;
		soundRestoreBufferState();

		if (_soundFault)
			return;

		_soundStreamPosition = READ_LE_UINT32(_soundPackedState);
		_soundStarted = _soundPackedState[4] != 0;
		_soundFillCount = READ_LE_UINT16(_soundPackedState + 5);
		_soundEventRemaining = READ_LE_UINT32(_soundPackedState + 7);
		_soundEventLength = (int32)_soundEventRemaining;
		_soundLoopCount = (int16)READ_LE_UINT16(_soundPackedState + 0x0B);
		uint32 playbackPosition = READ_LE_UINT32(_soundPackedState + 0x0D);
		uint32 vocCounter = READ_LE_UINT32(_soundPackedState + 0x11);
		_soundSample = _soundPackedState[0x40];
		_soundCursor = _soundPlayPosition;
		_soundBitOffset = 0;
		_soundLoopBitOffset = 0;
		_soundBlockOffset = 0;
		_soundFinished = false;
		_soundHistory.clear();
		_soundCues.clear();

		uint16 advance = (uint16)(playbackPosition - _soundStreamPosition);
		if (advance >= COMFY_V3_SOUND_PCM_BLOCK_BYTES)
			advance = COMFY_V3_SOUND_PCM_BLOCK_BYTES;

		soundFillPcmBuffer(0, advance);
		if (_soundFault)
			return;

		uint16 cueCount = MIN<uint16>(READ_LE_UINT16(_soundPackedState + 0x3D), COMFY_V3_SOUND_MAX_CUES);
		for (uint16 i = 0; i < cueCount; i++) {
			SoundCue cue;
			cue.value = READ_LE_UINT16(_soundPackedState + 0x15 + i * 2);
			cue.streamPosition = READ_LE_UINT32(_soundPackedState + 0x1D + i * 4);
			cue.counterThreshold = READ_LE_UINT32(_soundPackedState + 0x2D + i * 4);
			_soundCues.push_back(cue);
		}

		_soundEventSubIndex = 0xFFFF;

		soundFillPcmBuffer(0, COMFY_V3_SOUND_PCM_BLOCK_BYTES);
		if (_soundFault)
			return;

		soundFillPcmBuffer(1, COMFY_V3_SOUND_PCM_BLOCK_BYTES);
		if (_soundFault)
			return;

		if (!waveOutStartPlayback())
			return;

		_soundWaveBufferIndex = 0;
		_soundVocCounterSnapshot = vocCounter;
		_soundVocBlockCount = 0;

		if (_midiPlyrDriver) {
			_midiPlyrDriver->setVocCounter(vocCounter);
			_midiPlyrDriver->setIncreaseVocCounter(1);
			_midiPlyrDriver->vocSrResetCounters();
		}

		_soundCounterEnabled = true;
		return;
	}

	soundStopPlayback();

	if (_soundPackedState[0x3F]) {
		_soundStopped = true;
		_soundCues.clear();

		if (_midiPlyrDriver) {
			_midiPlyrDriver->setIncreaseVocCounter(0);
			_midiPlyrDriver->vocSrResetBlockNo();
		}

		return;
	}

	_soundStopped = false;
	_soundDecodeBlockSize = _isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES : (_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES);
	_soundFault = false;

	soundRestoreBufferState();

	if (_soundFault) {
		_soundEventSubIndex = 0;
		return;
	}

	_soundStreamPosition = READ_LE_UINT32(_soundPackedState);
	_soundStarted = _soundPackedState[4] != 0;
	_soundFillCount = READ_LE_UINT16(_soundPackedState + 5);
	_soundEventRemaining = READ_LE_UINT32(_soundPackedState + 7);
	_soundEventLength = (int32)_soundEventRemaining;
	_soundLoopCount = (int16)READ_LE_UINT16(_soundPackedState + 0x0B);
	uint32 playbackPosition = READ_LE_UINT32(_soundPackedState + 0x0D);
	uint32 vocCounter = READ_LE_UINT32(_soundPackedState + 0x11);
	_soundSample = _soundPackedState[0x40];
	_soundCursor = _soundPlayPosition;
	_soundBitOffset = 0;
	_soundLoopBitOffset = 0;
	_soundBlockOffset = 0;
	_soundFinished = false;
	_soundHistory.clear();
	_soundCues.clear();

	if (_isPanther || _engineVersion == 3) {
		uint16 advance = (uint16)(playbackPosition - _soundStreamPosition);
		uint16 blockSize = (uint16)(_isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES : COMFY_V3_SOUND_PCM_BLOCK_BYTES);

		if (advance >= blockSize)
			advance = blockSize;

		soundFillPcmBuffer(0, advance);
	} else {
		soundFillPcmBuffer(0, COMFY_SOUND_PCM_BLOCK_BYTES);
	}

	if (_soundFault) {
		_soundEventSubIndex = 0;
		return;
	}

	uint16 cueCount = MIN<uint16>(READ_LE_UINT16(_soundPackedState + 0x3D), COMFY_SOUND_MAX_CUES);
	for (uint16 i = 0; i < cueCount; i++) {
		SoundCue cue;
		cue.value = READ_LE_UINT16(_soundPackedState + 0x15 + i * 2);
		cue.streamPosition = READ_LE_UINT32(_soundPackedState + 0x1D + i * 4);
		cue.counterThreshold = READ_LE_UINT32(_soundPackedState + 0x2D + i * 4);
		_soundCues.push_back(cue);
	}

	_soundEventSubIndex = 0xFFFF;
	soundFillPcmBuffer(0, (uint16)(_isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES : (_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES)));

	if (_soundFault) {
		_soundEventSubIndex = 0;
		return;
	}

	soundFillPcmBuffer(1, (uint16)(_isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES : (_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES)));

	if (_soundFault) {
		_soundEventSubIndex = 0;
		return;
	}

	if (!waveOutStartPlayback()) {
		_soundEventSubIndex = 0;
		return;
	}

	_soundWaveBufferIndex = 0;
	_soundVocCounterSnapshot = vocCounter;
	_soundVocBlockCount = 0;

	if (_midiPlyrDriver) {
		_midiPlyrDriver->setVocCounter(vocCounter);
		_midiPlyrDriver->setIncreaseVocCounter(1);
		_midiPlyrDriver->vocSrResetCounters();
	}
}

void ComfyEngine::soundPlayEntry(uint16 index) {
	waveOutReset();

	_soundEventSubIndex = 0xFFFF;
	_soundTimingPaused = false;
	_soundVocBlockCount = 0;
	_soundVocCounterSnapshot = 0;
	_soundStreamPosition = 0;
	_soundPlayPosition = 0;
	_soundFillCount = 0;
	_soundEventLength = 0;
	_soundEventRemaining = 0;
	_soundLoopCount = 0;
	_soundStopped = false;
	_soundStarted = false;

	if (_engineVersion == 3)
		_soundCounterEnabled = true;

	_soundFault = false;
	_soundCues.clear();

	if (_midiPlyrDriver)
		_midiPlyrDriver->vocSrResetCounters();

	byte initialSample = soundOpenEntry(index);
	if (!initialSample && _soundFault) {
		_soundEventSubIndex = 0;
		_soundStopped = true;
		return;
	}

	_soundSample = _engineVersion == 3 ? 0 : (_isPanther ? 0xA5 : initialSample);
	if (!_soundSample)
		_soundSample = 0xA5;

	soundFillPcmBuffer(0, (uint16)(_isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES :
		(_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES)));

	if (_soundFault) {
		_soundEventSubIndex = 0;
		return;
	}

	if (_soundNextSample)
		_soundSample = _soundNextSample;

	soundFillPcmBuffer(1, (uint16)(_isPanther ? COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES :
		(_engineVersion == 3 ? COMFY_V3_SOUND_PCM_BLOCK_BYTES : COMFY_SOUND_PCM_BLOCK_BYTES)));

	if (_soundFault) {
		_soundEventSubIndex = 0;
		return;
	}

	if (_soundNextSample)
		_soundSample = _soundNextSample;

	_soundWaveBufferIndex = 0;

	if (_midiPlyrDriver) {
		_midiPlyrDriver->setVocCounter(0);
		_midiPlyrDriver->setIncreaseVocCounter(1);
	}

	_soundSampleRate = soundCalcSampleRate(_soundSample);

	if (!waveOutStartPlayback()) {
		_soundEventSubIndex = 0;
		return;
	}

	_soundPaused = false;
	_soundStopped = false;
	_soundEventSubIndex = 0xFFFF;
}

void ComfyEngine::soundUnprepareHeader() {
	if (_soundPaused)
		return;

	if (!_mixer->isSoundHandleActive(_soundHandle)) {
		_soundQueueStream = nullptr;
		return;
	}

	uint32 queuedBlocks = _soundQueueStream->numQueuedStreams();
	soundServiceWaveBuffers();

	if (!_isPanther && _soundQueueStream->numQueuedStreams() > queuedBlocks)
		soundLoadChunk();
}

void ComfyEngine::soundCueTick() {
	uint32 counter = _midiPlyrDriver ? _midiPlyrDriver->getVocCounter() : (uint32)((_mixer->getSoundElapsedTime(_soundHandle) + 9) / 10);
	int32 compareCounter = (int32)(counter - 2);

	bool removedPhaseCue = false;

	while (!_soundCues.empty() && compareCounter > (int32)_soundCues[0].counterThreshold &&
			(_soundCues[0].value == 1 || _soundCues[0].value == 2)) {
		_soundEventSubIndex = _soundCues[0].value;
		_soundCues.remove_at(0);
		removedPhaseCue = true;
	}

	if (removedPhaseCue || _soundCues.empty() || compareCounter <= (int32)_soundCues[0].counterThreshold)
		return;

	_soundEventSubIndex = _soundCues[0].value;
	if (!_soundEventSubIndex) {
		if (_engineVersion == 3) {
			soundPrepareQueuePlayback();
		} else if (_isPanther) {
			soundStopPlayback();
		} else {
			waveOutReset();
		}

		_soundStopped = true;
		_soundCues.clear();
		return;
	}

	_soundCues.remove_at(0);
}

void ComfyEngine::soundAdvanceTick() {
	if (_engineVersion == 3) {
		if (_soundCounterEnabled)
			soundUpdateVocTiming();

		soundServiceWaveBuffers();
		soundCueTick();
		return;
	}

	if (_isPanther) {
		soundUpdateVocTiming();
		soundServiceWaveBuffers();
	} else {
		soundUnprepareHeader();
	}

	soundCueTick();
}

} // End of namespace Comfy
