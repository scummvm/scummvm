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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "common/textconsole.h"
#include "common/debug.h"
#include "audio/decoders/xa.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/memstream.h"
#include "dragons/vabsound.h"
#include "dragons/dragons.h"

namespace Dragons {

VabSound::VabSound(Common::SeekableReadStream *msfData, const DragonsEngine *_vm): _toneAttrs(nullptr), _vbData(nullptr) {
	loadHeader(msfData);

	int32 dataSize = msfData->size() - msfData->pos();
	_vbData = new byte[dataSize];
	msfData->read(_vbData, dataSize);

//		_vbData = new Common::MemoryReadStream(newData, dataSize, DisposeAfterUse::YES);
//
//		Audio::AudioStream *str = Audio::makeXAStream(_vbData, 11025);
//		Audio::SoundHandle _speechHandle;
//		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speechHandle, str);

	delete msfData;
}

VabSound::VabSound(Common::SeekableReadStream *vhData, Common::SeekableReadStream *vbData): _toneAttrs(nullptr), _vbData(nullptr) {
	loadHeader(vhData);

	assert(vhData->pos() == vhData->size());

	_vbData = new byte[vbData->size()];
	vbData->read(_vbData, vbData->size());

	delete vhData;
	delete vbData;
}

void VabSound::loadHeader(Common::SeekableReadStream *vhData) {
	vhData->seek(0);
	vhData->read(&_header.magic, 4);
	_header.version = vhData->readUint32LE();
	_header.vabId = vhData->readUint32LE();
	_header.waveformSize = vhData->readUint32LE();

	_header.reserved0 = vhData->readUint16LE();
	_header.numPrograms = vhData->readUint16LE();
	_header.numTones = vhData->readUint16LE();
	_header.numVAG = vhData->readUint16LE();

	_header.masterVolume = vhData->readByte();
	_header.masterPan = vhData->readByte();
	_header.bankAttr1 = vhData->readByte();
	_header.bankAttr2 = vhData->readByte();

	_header.reserved1 = vhData->readUint32LE();

	if (strncmp(_header.magic, "pBAV", 4) != 0) {
		error("Invalid VAB file");
	}

	loadProgramAttributes(vhData);
	loadToneAttributes(vhData);

	uint16 tempOffsets[0x100];
	for (int i = 0; i < 0x100; i++) {
		tempOffsets[i] = vhData->readUint16LE();
	}
	_vagOffsets[0] = tempOffsets[0] << 3u;
	for (int j = 1; j < 0x100; ++j) {
		const int vagSize = tempOffsets[j] << 3u;
		_vagSizes[j - 1] = vagSize;
		_vagOffsets[j] = vagSize + _vagOffsets[j - 1];
	}
}

VabSound::~VabSound() {
	delete _toneAttrs;
	delete _vbData;
}

Audio::AudioStream *VabSound::getAudioStream(uint16 program, uint16 key) {
	int16 vagID = getVagID(program, key);
	if (vagID < 0) {
		return nullptr;
	}
	int16 baseKey = getBaseToneKey(program, key);
	int sampleRate = getAdjustedSampleRate(key, baseKey);
	debug(3, "Playing program %d, Key %d, numTones: %d, vagID %d, vagOffset: %x, size: %x adjustedSampleRate: %d",
	   program, key, _programAttrs[program].tones, vagID, _vagOffsets[vagID], _vagSizes[vagID], sampleRate);
	Audio::AudioStream *str = Audio::makeXAStream(
			new Common::MemoryReadStream(&_vbData[_vagOffsets[vagID]], _vagSizes[vagID], DisposeAfterUse::NO),
			sampleRate,
			DisposeAfterUse::YES);
	return str;
}

void VabSound::loadProgramAttributes(Common::SeekableReadStream *vhData) {
	for (int i = 0; i < DRAGONS_VAB_NUM_PROG_ATTRS; i++) {
		_programAttrs[i].tones = vhData->readByte();
		_programAttrs[i].mvol = vhData->readByte();
		_programAttrs[i].prior = vhData->readByte();
		_programAttrs[i].mode = vhData->readByte();
		_programAttrs[i].mpan = vhData->readByte();
		_programAttrs[i].reserved0 = vhData->readByte();
		_programAttrs[i].attr = vhData->readUint16LE();
		_programAttrs[i].reserved1 = vhData->readUint32LE();
		_programAttrs[i].reserved2 = vhData->readUint32LE();
	}
}

void VabSound::loadToneAttributes(Common::SeekableReadStream *vhData) {
	const int numTones = 16 * _header.numPrograms;
	_toneAttrs = new VabToneAttr[numTones];
	VabToneAttr *pVabToneAttr = _toneAttrs;
	for (int i = 0; i < numTones; i++, pVabToneAttr++) {
		pVabToneAttr->prior = vhData->readByte();
		pVabToneAttr->mode = vhData->readByte();
		pVabToneAttr->vol = vhData->readByte();
		pVabToneAttr->pan = vhData->readByte();
		pVabToneAttr->center = vhData->readByte();
		pVabToneAttr->shift = vhData->readByte();
		pVabToneAttr->min = vhData->readByte();
		pVabToneAttr->max = vhData->readByte();
		pVabToneAttr->vibW = vhData->readByte();
		pVabToneAttr->vibT = vhData->readByte();
		pVabToneAttr->porW = vhData->readByte();
		pVabToneAttr->porT = vhData->readByte();
		pVabToneAttr->pbmin = vhData->readByte();
		pVabToneAttr->pbmax = vhData->readByte();
		pVabToneAttr->reserved1 = vhData->readByte();
		pVabToneAttr->reserved2 = vhData->readByte();
		pVabToneAttr->adsr1 = vhData->readUint16LE();
		pVabToneAttr->adsr2 = vhData->readUint16LE();
		pVabToneAttr->prog = vhData->readSint16LE();
		pVabToneAttr->vag = vhData->readSint16LE();
		for (int j = 0; j < 4; j++) {
			pVabToneAttr->reserved[j] = vhData->readSint16LE();
		}
	}
}

int16 VabSound::getVagID(uint16 program, uint16 key) {
	if (program < _header.numVAG) {
		for (int i = 0; i < _programAttrs[program].tones; i++) {
			if (_toneAttrs[i].prog == program && _toneAttrs[i].min <= key && _toneAttrs[i].max >= key) {
				return _toneAttrs[i].vag - 1;
			}
		}
	} else {
		warning("program >= _header.numVAG %d %d", program, _header.numVAG);
	}

	return -1;
}

int16 VabSound::getBaseToneKey(uint16 program, uint16 key) {
	if (program < _header.numVAG) {
		for (int i = 0; i < _programAttrs[program].tones; i++) {
			if (_toneAttrs[i].prog == program && _toneAttrs[i].min <= key && _toneAttrs[i].max >= key) {
				debug("tone key %d center %d mode %d shift %d min %d, max %d adsr 1 %d adsr 2 %d pbmin %d pbmax %d",
		  			key, _toneAttrs[i].center, _toneAttrs[i].mode, _toneAttrs[i].shift, _toneAttrs[i].min, _toneAttrs[i].max,
					  _toneAttrs[i].adsr1, _toneAttrs[i].adsr2, _toneAttrs[i].pbmin, _toneAttrs[i].pbmax);
				return _toneAttrs[i].center;
			}
		}
	}
	return -1;
}

bool VabSound::hasSound(uint16 program, uint16 key) {
	return getVagID(program, key) != -1;
}

int VabSound::getAdjustedSampleRate(int16 desiredKey, int16 baseToneKey) {
	if (desiredKey == baseToneKey) {
		return 44100;
	}
	float diff = pow(2, (float)(desiredKey - baseToneKey) / 12);
	return (int)((float)44100 * diff);
}

} // End of namespace Dragons
