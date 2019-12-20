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
#include <common/textconsole.h>
#include <common/debug.h>
#include <audio/decoders/xa.h>
#include <audio/audiostream.h>
#include <audio/mixer.h>
#include "common/memstream.h"
#include "VabSound.h"
#include "dragons.h"

namespace Dragons {
	VabSound::VabSound(Common::SeekableReadStream *msfData, const DragonsEngine *_vm): _toneAttrs(NULL), _vbData(NULL) {
		loadHeader(msfData);

		auto dataSize = msfData->size() - msfData->pos();
		_vbData = new byte[dataSize];
		msfData->read(_vbData, dataSize);

//		_vbData = new Common::MemoryReadStream(newData, dataSize, DisposeAfterUse::YES);
//
//		Audio::AudioStream *str = Audio::makeXAStream(_vbData, 11025);
//		Audio::SoundHandle _speechHandle;
//		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speechHandle, str);

		delete msfData;
	}

	VabSound::VabSound(Common::SeekableReadStream *vhData, Common::SeekableReadStream *vbData): _toneAttrs(NULL), _vbData(NULL) {
		loadHeader(vhData);

		assert(vhData->pos() == vhData->size());

		_vbData = new byte[vbData->size()];
		vbData->read(_vbData, vbData->size());

		delete vhData;
		delete vbData;
	}

	void VabSound::loadHeader(Common::SeekableReadStream *vhData) {
		vhData->seek(0);
		vhData->read(&_header, sizeof(_header)); //TODO this is not endian safe!
		if (strncmp(_header.magic, "pBAV", 4) != 0) {
			error("Invalid VAB file");
		}
		// TODO: is sizeof(array) the right thing to do here?
		vhData->read(&_programAttrs, sizeof(_programAttrs));

		const int numTones = 16 * _header.numPrograms;
		_toneAttrs = new VabToneAttr[numTones];
		vhData->read(_toneAttrs, sizeof(VabToneAttr) * numTones);

		uint16 tempOffsets[0x100];
		vhData->read(tempOffsets, sizeof(tempOffsets));
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
		assert(program < _header.numVAG);
		// TODO
		uint16 vagID = 0;
		for (int i = 0; i < _header.numPrograms * 16; i++) {
			if (_toneAttrs[i].prog == program) {
				vagID = _toneAttrs[i].vag;
			}
		}
		debug("Playing program %d, numTones: %d, key %d vagID %d, vagOffset: %x, size: %x", program, _programAttrs[program].tones, key, vagID, _vagOffsets[vagID], _vagSizes[vagID]);
		Audio::AudioStream *str = Audio::makeXAStream(
				new Common::MemoryReadStream(&_vbData[_vagOffsets[vagID]], _vagSizes[vagID], DisposeAfterUse::NO),
				11025,
				DisposeAfterUse::YES);
		return str;
	}
} // End of namespace Dragons