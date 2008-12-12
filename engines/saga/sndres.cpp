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
 * $URL$
 * $Id$
 *
 */

// Sound resource management class

#include "saga/saga.h"

#include "saga/itedata.h"
#include "saga/rscfile.h"
#include "saga/sndres.h"
#include "saga/sound.h"

#include "common/file.h"

#include "sound/voc.h"
#include "sound/wave.h"
#include "sound/adpcm.h"
#include "sound/aiff.h"
#include "sound/audiostream.h"

namespace Saga {

#define RID_IHNM_SFX_LUT 265
#define RID_IHNMDEMO_SFX_LUT 222

SndRes::SndRes(SagaEngine *vm) : _vm(vm) {
	// Load sound module resource file contexts
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

		if (_vm->getGameId() == GID_IHNM_DEMO) {
			_vm->_resource->loadResource(resourceContext, RID_IHNMDEMO_SFX_LUT,
									 resourcePointer, resourceLength);
		} else {
			_vm->_resource->loadResource(resourceContext, RID_IHNM_SFX_LUT,
									 resourcePointer, resourceLength);
		}

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

void SndRes::setVoiceBank(int serial) {
	if (_voiceSerial == serial)
		return;

	// If we got the Macintosh version of IHNM, just set the voice bank
	// so that we know which voices* subfolder to look for later
	if (_vm->getGameType() == GType_IHNM && _vm->isMacResources()) {
		_voiceSerial = serial;
		// Set a dummy voice context
		_voiceContext = new ResourceContext();
		_voiceContext->fileType = GAME_VOICEFILE;
		_voiceContext->count = 0;
		_voiceContext->serial = 0;
		return;
	}

	// If there are no voice files present, don't set the voice bank
	if (!_vm->_voiceFilesExist)
		return;

	// Close previous voice bank file
	if (_voiceSerial >= 0 && _voiceContext->file->isOpen())
		_voiceContext->file->close();

	_voiceSerial = serial;

	_voiceContext = _vm->_resource->getContext(GAME_VOICEFILE, _voiceSerial);

	// Open new voice bank file
	if (!_voiceContext->file->isOpen())
		_voiceContext->file->open(_voiceContext->fileName);
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

	if (!(_vm->_voiceFilesExist))
		return;

	if (_vm->getGameType() == GType_IHNM && !(_vm->_voicesEnabled))
		return;

	debug(4, "SndRes::playVoice %i", resourceId);

	if (!load(_voiceContext, resourceId, buffer, false)) {
		warning("Failed to load voice");
		return;
	}

	_vm->_sound->playVoice(buffer);
}

bool SndRes::load(ResourceContext *context, uint32 resourceId, SoundBuffer &buffer, bool onlyHeader) {
	Audio::AudioStream *voxStream;
	size_t soundResourceLength;
	bool result = false;
	GameSoundTypes resourceType;
	byte *data;
	int rate;
	int size;
	byte flags;
	size_t voxSize;
	const GameSoundInfo *soundInfo;
	Common::File* file;

	if (resourceId == (uint32)-1) {
		return false;
	}

	if (_vm->getGameType() == GType_IHNM && _vm->isMacResources()) {
		char soundFileName[40];
		int dirIndex = resourceId / 64;
	
		if ((context->fileType & GAME_VOICEFILE) != 0) {
			if (_voiceSerial == 0) {
				sprintf(soundFileName, "Voices/VoicesS/Voices%d/VoicesS%03x", dirIndex, resourceId);
			} else {
				sprintf(soundFileName, "Voices/Voices%d/Voices%d/Voices%d%03x", _voiceSerial, dirIndex, _voiceSerial, resourceId);
			}
		} else {
			sprintf(soundFileName, "SFX/SFX%d/SFX%03x", dirIndex, resourceId);
		}
		
		file = new Common::File();

		file->open(soundFileName);
		soundResourceLength = file->size();
	} else {

		ResourceData* resourceData = context->getResourceData(resourceId);
		file = context->getFile(resourceData);
	
		file->seek(resourceData->offset);
		soundResourceLength = resourceData->size;

	}

	Common::SeekableReadStream& readS = *file;

	if ((context->fileType & GAME_VOICEFILE) != 0) {
		soundInfo = _vm->getVoiceInfo();
	} else {
		soundInfo = _vm->getSfxInfo();
	}

	if (_vm->getGameType() == GType_IHNM && _vm->isMacResources() && (context->fileType & GAME_VOICEFILE) != 0) {
		// No sound patch data for the voice files in the Mac version of IHNM
	} else {
		context->table[resourceId].fillSoundPatch(soundInfo);
	}


	resourceType = soundInfo->resourceType;

	if (soundResourceLength >= 8) {
		byte header[8];

		readS.read(&header, 8);
		readS.seek(readS.pos() - 8);

		if (!memcmp(header, "Creative", 8)) {
			resourceType = kSoundVOC;
		} else if (!memcmp(header, "RIFF", 4) != 0) {
			resourceType = kSoundWAV;
		} else if (!memcmp(header, "FORM", 4) != 0) {
			resourceType = kSoundAIFF;
		}

		bool uncompressedSound = false;
		// If patch data exists for sound resource 4 (used in ITE intro), don't treat this sound as compressed
		// Patch data for this resource is in file p2_a.iaf or p2_a.voc
		if (_vm->getGameType() == GType_ITE && resourceId == 4 && context->table[resourceId].patchData != NULL)
			uncompressedSound = true;

		// FIXME: Currently, the SFX.RES file in IHNM cannot be compressed
		if (_vm->getGameType() == GType_IHNM && (context->fileType & GAME_SOUNDFILE))
			uncompressedSound = true;

		if ((_vm->getFeatures() & GF_COMPRESSED_SOUNDS) && !uncompressedSound) {
			if (header[0] == char(0)) {
				resourceType = kSoundMP3;
			} else if (header[0] == char(1)) {
				resourceType = kSoundOGG;
			} else if (header[0] == char(2)) {
				resourceType = kSoundFLAC;
			}
		}

	}

	buffer.isBigEndian = soundInfo->isBigEndian;
	buffer.soundType = resourceType;
	buffer.originalSize = 0;

	switch (resourceType) {
	case kSoundPCM:
		buffer.frequency = 22050;
		buffer.isSigned = soundInfo->isSigned;
		buffer.sampleBits = soundInfo->sampleBits;
		buffer.size = soundResourceLength;
		buffer.stereo = false;
		if (onlyHeader) {
			buffer.buffer = NULL;
		} else {
			buffer.buffer = (byte *) malloc(soundResourceLength);
			readS.read(buffer.buffer, soundResourceLength);
		}
		result = true;
		break;
	case kSoundMacPCM:
		buffer.frequency = 22050;
		buffer.isSigned = soundInfo->isSigned;
		buffer.sampleBits = soundInfo->sampleBits;
		buffer.size = soundResourceLength - 36;
		buffer.stereo = false;
		if (onlyHeader) {
			buffer.buffer = NULL;
		} else {
			buffer.buffer = (byte *)malloc(buffer.size);

			readS.seek(readS.pos() + 36);
			readS.read(buffer.buffer, buffer.size);
		}
		result = true;
		break;
	case kSoundVOX:
		buffer.frequency = 22050;
		buffer.isSigned = soundInfo->isSigned;
		buffer.sampleBits = soundInfo->sampleBits;
		buffer.stereo = false;
		buffer.size = soundResourceLength * 4;
		if (onlyHeader) {
			buffer.buffer = NULL;
		} else {
			voxStream = Audio::makeADPCMStream(&readS, false, soundResourceLength, Audio::kADPCMOki);
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
		data = Audio::loadVOCFromStream(readS, size, rate);
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
		break;
	case kSoundWAV:
		if (Audio::loadWAVFromStream(readS, size, rate, flags)) {
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
		break;
	case kSoundAIFF:
		if (Audio::loadAIFFFromStream(readS, size, rate, flags)) {
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
		break;
	case kSoundMP3:
	case kSoundOGG:
	case kSoundFLAC:
		ResourceData *resourceData;
		resourceData = context->getResourceData(resourceId);

		// Read compressed sfx header
		readS.readByte();	// Skip compression identifier byte
		buffer.frequency = readS.readUint16LE();
		buffer.originalSize = readS.readUint32LE();
		buffer.sampleBits = readS.readByte();
		buffer.stereo = (readS.readByte() == char(0)) ? false : true;

		buffer.size = soundResourceLength;
		buffer.soundType = resourceType;
		buffer.soundFile = context->getFile(resourceData);
		buffer.fileOffset = resourceData->offset + 9; // skip compressed sfx header: byte + uint16 + uint32 + byte + byte

		buffer.buffer = NULL;

		result = true;
		break;
	default:
		error("SndRes::load Unknown sound type");
	}


	if (_vm->getGameType() == GType_IHNM && _vm->isMacResources()) {
		delete file;
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

	if (!(_vm->_voiceFilesExist))
		return -1;

	if (!load(_voiceContext, resourceId, buffer, true)) {
		return -1;
	}

	if (!(_vm->getFeatures() & GF_COMPRESSED_SOUNDS) || buffer.originalSize == 0)
		msDouble = (double)buffer.size;
	else
		msDouble = (double)buffer.originalSize;
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
