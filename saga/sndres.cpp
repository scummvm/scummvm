/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

// Sound resource management class

#include "saga/saga.h"

#include "saga/itedata.h"
#include "saga/resnames.h"
#include "saga/rscfile.h"
#include "saga/sndres.h"
#include "saga/sound.h"
#include "saga/stream.h"

#include "common/file.h"

#include "sound/voc.h"
#include "sound/wave.h"
#include "sound/adpcm.h"
#include "sound/audiostream.h"

namespace Saga {

SndRes::SndRes(SagaEngine *vm) : _vm(vm) {
	/* Load sound module resource file contexts */
	_sfxContext = _vm->_resource->getContext(GAME_SOUNDFILE);
	if (_sfxContext == NULL) {
		error("SndRes::SndRes resource context not found");
	}

	_voiceSerial = -1;

	setVoiceBank(0);

	if (_vm->getGameType() == GType_ITE) {
		_fxTable = ITE_SfxTable;
		_fxTableLen = ITE_SFXCOUNT;
	} else {
		ResourceContext *resourceContext;

		resourceContext = _vm->_resource->getContext(GAME_SOUNDFILE);
		if (resourceContext == NULL) {
			error("Resource::loadGlobalResources() resource context not found");
		}

		byte *resourcePointer;
		size_t resourceLength;

		_vm->_resource->loadResource(resourceContext, RID_IHNM_SFX_LUT,
								 resourcePointer, resourceLength);

		if (resourceLength == 0) {
			error("Sndres::SndRes can't read SfxIDs table");
		}

		_fxTableIDsLen = resourceLength / 2;
		_fxTableIDs = (int16 *)malloc(_fxTableIDsLen * sizeof(int16));

		MemoryReadStream metaS(resourcePointer, resourceLength);
		for (int i = 0; i < _fxTableIDsLen; i++)
			_fxTableIDs[i] = metaS.readSint16LE();

		free(resourcePointer);

		_fxTable = 0;
		_fxTableLen = 0;
	}
}

SndRes::~SndRes() {
	if (_vm->getGameType() == GType_IHNM) {
		free(_fxTable);
		free(_fxTableIDs);
	}
}

void SndRes::setVoiceBank(int serial)
{
	if (_voiceSerial == serial) return;

	_voiceSerial = serial;

	_voiceContext = _vm->_resource->getContext(GAME_VOICEFILE, _voiceSerial);
	if (_voiceContext == NULL) {
		error("SndRes::SndRes resource context not found");
	}


}

void SndRes::playSound(uint32 resourceId, int volume, bool loop) {
	SoundBuffer buffer;

	debug(4, "SndRes::playSound %i", resourceId);

	if (!load(_sfxContext, resourceId, buffer, false)) {
		warning("Failed to load sound");
		return;
	}

	_vm->_sound->playSound(buffer, volume, loop);
}

void SndRes::playVoice(uint32 resourceId) {
	SoundBuffer buffer;

	debug(4, "SndRes::playVoice %i", resourceId);

	if (!load(_voiceContext, resourceId, buffer, false)) {
		warning("Failed to load voice");
		return;
	}

	_vm->_sound->playVoice(buffer);
}

bool SndRes::load(ResourceContext *context, uint32 resourceId, SoundBuffer &buffer, bool onlyHeader) {
	byte *soundResource;
	AudioStream *voxStream;
	size_t soundResourceLength;
	bool result = false;
	GameSoundTypes resourceType;
	byte *data;
	int rate;
	int size;
	byte flags;
	size_t voxSize;
	const GameSoundInfo *soundInfo;

	if (resourceId == (uint32)-1) {
		return false;
	}


	_vm->_resource->loadResource(context, resourceId, soundResource, soundResourceLength);

	if ((context->fileType & GAME_VOICEFILE) != 0) {
		soundInfo = _vm->getVoiceInfo();
	} else {
		soundInfo = _vm->getSfxInfo();
	}
	
	context->table[resourceId].fillSoundPatch(soundInfo);

	MemoryReadStream readS(soundResource, soundResourceLength);

	resourceType = soundInfo->resourceType;
	buffer.isBigEndian = soundInfo->isBigEndian;

	if (soundResourceLength >= 8) {
		if (!memcmp(soundResource, "Creative", 8)) {
			resourceType = kSoundVOC;
		} else 	if (!memcmp(soundResource, "RIFF", 4) != 0) {
			resourceType = kSoundWAV;
		}
	}


	switch (resourceType) {
	case kSoundPCM:
		buffer.frequency = soundInfo->frequency;
		buffer.isSigned = soundInfo->isSigned;
		buffer.sampleBits = soundInfo->sampleBits;
		buffer.size = soundResourceLength;
		buffer.stereo = soundInfo->stereo;
		if (onlyHeader) {
			buffer.buffer = NULL;
			free(soundResource);
		} else {
			buffer.buffer = soundResource;
		}
		result = true;
		break;
	case kSoundMacPCM:
		buffer.frequency = soundInfo->frequency;
		buffer.isSigned = soundInfo->isSigned;
		buffer.sampleBits = soundInfo->sampleBits;
		buffer.size = soundResourceLength - 36;
		buffer.stereo = soundInfo->stereo;
		if (onlyHeader) {
			buffer.buffer = NULL;
		} else {
			buffer.buffer = (byte *)malloc(buffer.size);
			memcpy(buffer.buffer, soundResource + 36, buffer.size);
		}
		free(soundResource);
		result = true;
		break;
	case kSoundVOX:
		buffer.frequency = soundInfo->frequency;
		buffer.isSigned = soundInfo->isSigned;
		buffer.sampleBits = soundInfo->sampleBits;
		buffer.stereo = soundInfo->stereo;
		buffer.size = soundResourceLength * 4;
		if (onlyHeader) {
			buffer.buffer = NULL;
			free(soundResource);
		} else {
			voxStream = new ADPCMInputStream(&readS, soundResourceLength, kADPCMOki);
			buffer.buffer = (byte *)malloc(buffer.size);
			voxSize = voxStream->readBuffer((int16*)buffer.buffer, soundResourceLength * 2);
			if (voxSize != soundResourceLength * 2) {
				error("SndRes::load() wrong VOX output size");
			}
			delete voxStream;
		}
		result = true;
		break;
	case kSoundVOC:
		data = loadVOCFromStream(readS, size, rate);
		if (data) {
			buffer.frequency = rate;
			buffer.sampleBits = 8;
			buffer.stereo = false;
			buffer.isSigned = false;
			buffer.size = size;
			if (onlyHeader) {
				buffer.buffer = NULL;
				free(data);
			} else {
				buffer.buffer = data;
			}
			result = true;
		}
		free(soundResource);
		break;
	case kSoundWAV:
		if (loadWAVFromStream(readS, size, rate, flags)) {
			buffer.frequency = rate;
			buffer.sampleBits = 16;
			buffer.stereo = ((flags & Audio::Mixer::FLAG_STEREO) != 0);
			buffer.isSigned = true;
			buffer.size = size;
			if (onlyHeader) {
				buffer.buffer = NULL;
			} else {
				buffer.buffer = (byte *)malloc(size);
				readS.read(buffer.buffer, size);
			}
			result = true;
		}
		free(soundResource);
		break;
	default:
		error("SndRes::load Unknown sound type");
	}

	// In ITE CD De some voices are absent and contain just 5 bytes header
	// Round it to even number so soundmanager will not crash.
	// See bug #1256701
	buffer.size &= ~(0x1);

	return result;
}

int SndRes::getVoiceLength(uint32 resourceId) {
	double msDouble;
	SoundBuffer buffer;

	if (!load(_voiceContext, resourceId, buffer, true)) {
		return -1;
	}

	msDouble = (double)buffer.size;
	if (buffer.sampleBits == 16) {
		msDouble /= 2.0;
	}
	if (buffer.stereo) {
		msDouble /= 2.0;
	}

	msDouble = msDouble / buffer.frequency * 1000.0;
	return (int)msDouble;
}

} // End of namespace Saga
