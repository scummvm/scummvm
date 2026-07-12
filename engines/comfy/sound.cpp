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

bool ComfyEngine::soundInit() {
	if (!readAssetFile(Common::Path("VOCFILE.DAT"), _vocFileData) || _vocFileData.size() < 4)
		return false;

	if (_vocFileData[0] != 'C' || (_vocFileData[1] != 'V' && _vocFileData[1] != 'W'))
		return false;

	_soundCompressed = _vocFileData[1] == 'W';
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
	_soundPcm.clear();
	_soundCues.clear();
	_vocFileData.clear();
	_soundEntryCount = 0;
	_soundNextCue = 0;
	_soundPaused = false;
	if (_midiPlyrDriver)
		_midiPlyrDriver->setIncreaseVocCounter(0);
}

void ComfyEngine::soundHdrReadFromXms(byte *destination, uint16 index, uint16 size) {
	objHdrReadFromXms(destination, _headerXmsSoundHeadersBase, size, index);
}

void ComfyEngine::soundUnprepareHeader() {
	soundAdvanceTick();
}

bool ComfyEngine::soundLoadEntry(uint16 index) {
	byte header[8];
	memset(header, 0, sizeof(header));
	soundHdrReadFromXms(header, index, sizeof(header));
	uint32 size = READ_LE_UINT32(header);
	uint32 offset = READ_LE_UINT32(header + 4);
	return offset <= _vocFileData.size() && size <= _vocFileData.size() - offset;
}

bool ComfyEngine::soundDecodeEntry(uint16 index) {
	_soundPcm.clear();
	_soundCues.clear();
	_soundNextCue = 0;
	byte header[8];
	memset(header, 0, sizeof(header));
	soundHdrReadFromXms(header, index, sizeof(header));
	uint32 dataSize = READ_LE_UINT32(header);
	uint32 dataOffset = READ_LE_UINT32(header + 4);
	debug(5, "COMFY VOC: decode id=%u offset=%u size=%u compressed=%u fileSize=%u",
		index, dataOffset, dataSize, _soundCompressed ? 1 : 0, (uint)_vocFileData.size());
	if (dataOffset > _vocFileData.size() || dataSize > _vocFileData.size() - dataOffset) {
		debug(5, "COMFY VOC: decode id=%u rejected invalid range", index);
		return false;
	}

	if (_soundCompressed) {
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
		byte command = _vocFileData[pc++];
		if (!command) {
			SoundCue cue = {0, (uint32)((_soundPcm.size() * 100) / _soundSampleRate)};
			_soundCues.push_back(cue);
			break;
		}

		if (3 > end - pc)
			return false;

		uint32 argument = (uint32)_vocFileData[pc] | ((uint32)_vocFileData[pc + 1] << 8) |
			((uint32)_vocFileData[pc + 2] << 16);
		pc += 3;
		if (command == 1 || command == 2) {
			uint32 length = argument;
			if (command == 1) {
				if (length < 2 || 2 > end - pc)
					return false;

				sample = _vocFileData[pc];
				pc += 2;
				length -= 2;
				uint32 denominator = 0x100 - sample;
				_soundSampleRate = denominator ? 0x0F4240 / denominator : 0x2B11;
				if (_soundSampleRate > 0x2710 && _soundSampleRate < 0x2EE0)
					_soundSampleRate = 0x2B11;
				else if (_soundSampleRate > 0x4E20 && _soundSampleRate < 0x5DC0)
					_soundSampleRate = 0x5622;
			}

			if (length > end - pc)
				return false;

			uint32 oldSize = _soundPcm.size();
			_soundPcm.resize(oldSize + length);
			memcpy(&_soundPcm[oldSize], &_vocFileData[pc], length);
			pc += length;
		} else if (command == 3) {
			if (3 > end - pc)
				return false;

			uint16 fill = READ_LE_UINT16(&_vocFileData[pc]);
			byte nextSample = _vocFileData[pc + 2];
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

			SoundCue cue = {READ_LE_UINT16(&_vocFileData[pc]), (uint32)((_soundPcm.size() * 100) / _soundSampleRate)};
			_soundCues.push_back(cue);
			pc += 2;
		} else if (command == 6) {
			if (2 > end - pc)
				return false;

			loopCount = READ_LE_UINT16(&_vocFileData[pc]);
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

			sample = _vocFileData[pc++];
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
	if (_soundSampleRate > 0x2710 && _soundSampleRate < 0x2EE0)
		_soundSampleRate = 0x2B11;
	else if (_soundSampleRate > 0x4E20 && _soundSampleRate < 0x5DC0)
		_soundSampleRate = 0x5622;

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
			SoundCue cue = {0, (uint32)((_soundPcm.size() * 100) / _soundSampleRate)};
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
			SoundCue cue = {value, (uint32)((_soundPcm.size() * 100) / _soundSampleRate)};
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
	if (!soundDecodeEntry(index)) {
		debug(5, "COMFY VOC: play id=%u decode failed", index);
		_soundEventSubIndex = 0;
		return;
	}

	Audio::AudioStream *stream = Audio::makeRawStream(&_soundPcm[0], _soundPcm.size(), _soundSampleRate,
		Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	if (!stream) {
		debug(5, "COMFY VOC: play id=%u raw stream creation failed", index);
		_soundPcm.clear();
		_soundEventSubIndex = 0;
		return;
	}

	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, stream, -1,
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	debug(5, "COMFY VOC: play id=%u submitted active=%u pcm=%u rate=%u cues=%u",
		index, _mixer->isSoundHandleActive(_soundHandle) ? 1 : 0,
		(uint)_soundPcm.size(), _soundSampleRate, (uint)_soundCues.size());
	_soundPaused = false;
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
		if (!_soundPcm.empty() && _soundEventSubIndex != 0) {
			debug(5, "COMFY VOC: mixer became inactive; marking entry complete");
			_soundEventSubIndex = 0;
		}

		return;
	}

	uint32 counter = _midiPlyrDriver ? _midiPlyrDriver->getVocCounter() :
		(uint32)((_mixer->getSoundElapsedTime(_soundHandle) + 9) / 10);
	uint32 compareCounter = counter >= 2 ? counter - 2 : 0;
	bool phaseCueSeen = false;
	while (_soundNextCue < _soundCues.size() && compareCounter > _soundCues[_soundNextCue].counterThreshold &&
			(_soundCues[_soundNextCue].value == 1 || _soundCues[_soundNextCue].value == 2)) {
		phaseCueSeen = true;
		_soundEventSubIndex = _soundCues[_soundNextCue].value;
		debug(5, "COMFY VOC: cue=%u threshold=%u counter=%u index=%u/%u",
			_soundEventSubIndex, _soundCues[_soundNextCue].counterThreshold, counter,
			_soundNextCue, (uint)_soundCues.size());
		_soundNextCue++;
	}

	if (phaseCueSeen || _soundNextCue == _soundCues.size() ||
			compareCounter <= _soundCues[_soundNextCue].counterThreshold)
		return;

	_soundEventSubIndex = _soundCues[_soundNextCue].value;
	debug(5, "COMFY VOC: cue=%u threshold=%u counter=%u index=%u/%u",
		_soundEventSubIndex, _soundCues[_soundNextCue].counterThreshold, counter,
		_soundNextCue, (uint)_soundCues.size());
	_soundNextCue++;
	if (!_soundEventSubIndex) {
		_mixer->stopHandle(_soundHandle);
		if (_midiPlyrDriver)
			_midiPlyrDriver->setIncreaseVocCounter(0);
	}
}


} // End of namespace Comfy
