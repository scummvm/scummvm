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

#include "common/stream.h"

#include "engines/grim/resource.h"
#include "engines/grim/movie/codecs/vima.h"
#include "engines/grim/imuse/dimuse_engine.h"

namespace Grim {

int IMuseDigital::diMUSEInitFilelist() {
	for (int i = 0; i < DIMUSE_MAX_SOUND_LIST; i++) {
		_mainSoundList[i].prev = nullptr;
		_mainSoundList[i].next = nullptr;
		_mainSoundList[i].soundId = 0;
	}

	_fileListInitialized = true;
	return 0;
}

void IMuseDigital::diMUSESaveLoadFilelist(Common::Serializer &ser) {
	IMuseDigiSoundList *sound;
	int loadedCount = 0;
	int openedCount = 0;
	int soundId = 0;
	int refCount = 0;
	char filename[36] = {};

	if (ser.isSaving()) {
		for (sound = _loadedSoundList; sound; sound = sound->next)
			loadedCount++;

		for (sound = _openedSoundsList; sound; sound = sound->next)
			openedCount++;
	} else {
		for (int i = 0; i < DIMUSE_MAX_SOUND_LIST; i++) {
			_mainSoundList[i].prev = nullptr;
			_mainSoundList[i].next = nullptr;
			_mainSoundList[i].soundId = 0;
		}
	}

	ser.syncAsSint32LE(loadedCount, VER(103));
	ser.syncAsSint32LE(openedCount, VER(103));

	sound = _loadedSoundList;
	for (int i = 0; i < loadedCount; i++) {
		if (ser.isSaving()) {
			soundId = sound->soundId;
			refCount = sound->refCount;
			Common::strlcpy(filename, sound->filename, sizeof(filename));
			sound = sound->next;
		}

		ser.syncAsSint32LE(soundId, VER(103));
		ser.syncArray(filename, sizeof(filename), Common::Serializer::SByte, VER(103));
		ser.syncAsSint32LE(refCount, VER(103));

		if (ser.isLoading()) {
			if (!diMUSELoadSound(filename, soundId)) {
				debug(5, "IMuseDigital::diMUSESaveLoadFilelist(): ERROR: couldn't load file %s", filename);
				return;
			}

			if (soundId != _loadedSoundList->soundId) {
				debug(5, "IMuseDigital::diMUSESaveLoadFilelist(): ERROR: detected modified sound number");
				return;
			}

			_loadedSoundList->refCount = refCount;
		}
	}

	sound = _openedSoundsList;
	for (int i = 0; i < openedCount; i++) {
		if (ser.isSaving()) {
			soundId = sound->soundId;
			refCount = sound->refCount;
			Common::strlcpy(filename, sound->filename, sizeof(filename));
			sound = sound->next;
		}

		ser.syncAsSint32LE(soundId, VER(103));
		ser.syncArray(filename, sizeof(filename), Common::Serializer::SByte, VER(103));
		ser.syncAsSint32LE(refCount, VER(103));

		if (ser.isLoading()) {
			if (!diMUSEOpenSound(filename, soundId)) {
				debug(5, "IMuseDigital::diMUSESaveLoadFilelist(): ERROR: couldn't open file %s", filename);
			}

			if (soundId != _openedSoundsList->soundId) {
				debug(5, "IMuseDigital::diMUSESaveLoadFilelist(): ERROR: detected modified sound number");
				return;
			}

			_openedSoundsList->refCount = refCount;
		}
	}
}

int IMuseDigital::diMUSELoadSound(const char *soundName, int requestedSoundId) {
	IMuseDigiSoundList *sound;
	int soundId;
	int nameIndex;

	if (!_fileListInitialized || !*soundName)
		return 0;

	soundId = diMUSEFindSound(soundName);
	if (soundId && _loadedSoundList) {
		sound = _loadedSoundList;
		while (soundId != sound->soundId) {
			sound = sound->next;

			if (!sound)
				break;
		}

		if (sound) {
			sound->refCount++;
			return soundId;
		}
	}

	sound = _mainSoundList;
	while (sound->soundId) {
		if (++sound >= &_mainSoundList[DIMUSE_MAX_SOUND_LIST]) {
			debug(5, "IMuseDigital::diMUSELoadSound(): Sound List FULL!");
			return 0;
		}
	}

	soundId = ihostLoadSound(soundName, requestedSoundId);
	if (soundId) {
		for (nameIndex = 0; nameIndex < 32; ) {
			sound->filename[nameIndex] = soundName[nameIndex];
			if (!soundName[++nameIndex])
				break;
		}

		if (!soundName[nameIndex]) {
			sound->filename[nameIndex] = 0;
			sound->soundId = soundId;
			sound->refCount = 1;
			diMUSEAddSoundToList(&_loadedSoundList, sound);
			return soundId;
		}

		debug(5, "IMuseDigital::diMUSELoadSound(): Name too long: %s", soundName);
	} else {
		debug(5, "IMuseDigital::diMUSELoadSound(): Host couldn't load sound %s", soundName);
	}

	return 0;
}

void IMuseDigital::diMUSEUnloadSound(int soundId) {
	IMuseDigiSoundList *sound;

	if (_fileListInitialized) {
		sound = _loadedSoundList;
		while (sound) {
			if (sound->soundId == soundId) {
				if (sound->refCount)
					sound->refCount--;

				break;
			}

			sound = sound->next;
		}

		diMUSEFlushSounds();
	}
}

void IMuseDigital::diMUSEUnloadAll() {
	IMuseDigiSoundList *sound;

	if (_fileListInitialized) {
		for (sound = _loadedSoundList; sound; sound = sound->next)
			sound->refCount = 0;

		diMUSEFlushSounds();
	}
}

int IMuseDigital::diMUSEOpenSound(const char *filename, int requestedSoundId) {
	IMuseDigiSoundList *sound;
	int soundId;
	int nameIndex;

	if (!_fileListInitialized || !*filename)
		return 0;

	soundId = diMUSEFindSound(filename);
	if (soundId && _openedSoundsList) {
		sound = _openedSoundsList;
		while (soundId != sound->soundId) {
			sound = sound->next;
			if (!sound)
				break;
		}

		if (sound) {
			sound->refCount++;
			return soundId;
		}
	}

	sound = _mainSoundList;
	while (sound->soundId) {
		if (++sound >= &_mainSoundList[DIMUSE_MAX_SOUND_LIST]) {
			debug(5, "IMuseDigital::diMUSEOpenSound(): Sound List FULL!");
			return 0;
		}
	}

	soundId = ihostOpenSound(filename, requestedSoundId);
	if (soundId) {
		for (nameIndex = 0; nameIndex < 32; ) {
			sound->filename[nameIndex] = filename[nameIndex];

			if (!filename[++nameIndex])
				break;
		}

		if (!filename[nameIndex]) {
			sound->filename[nameIndex] = 0;
			sound->soundId = soundId;
			sound->refCount = 1;
			diMUSEAddSoundToList(&_openedSoundsList, sound);
			return soundId;
		}

		debug(5, "IMuseDigital::diMUSEOpenSound(): Name too long: %s", filename);
	} else {
		debug(5, "IMuseDigital::diMUSEOpenSound(): Host couldn't open sound %s", filename);
	}

	return 0;
}

void IMuseDigital::diMUSECloseSound(int soundId) {
	IMuseDigiSoundList *sound;

	if (_fileListInitialized) {
		sound = _openedSoundsList;
		while (sound) {
			if (sound->soundId == soundId) {
				if (sound->refCount)
					sound->refCount--;
				break;
			}
			sound = sound->next;
		}

		diMUSEFlushSounds();
	}
}

void IMuseDigital::diMUSECloseAll() {
	IMuseDigiSoundList *sound;

	if (_fileListInitialized) {
		for (sound = _openedSoundsList; sound; sound = sound->next)
			sound->refCount = 0;

		diMUSEFlushSounds();
	}
}

int IMuseDigital::diMUSEFindSound(const char *soundName) {
	IMuseDigiSoundList *sound;

	for (sound = _loadedSoundList; sound; sound = sound->next) {
		if (*soundName && !strcmp(sound->filename, soundName))
			return sound->soundId;
	}

	for (sound = _openedSoundsList; sound; sound = sound->next) {
		if (*soundName && !strcmp(sound->filename, soundName))
			return sound->soundId;
	}

	return 0;
}

void IMuseDigital::diMUSEFlushSounds() {
	IMuseDigiSoundList *sound, *next;

	for (sound = _loadedSoundList; sound; sound = next) {
		next = sound->next;
		if (!sound->refCount && !diMUSEGetParam(sound->soundId, DIMUSE_P_SND_TRACK_NUM) && !diMUSEGetParam(sound->soundId, DIMUSE_P_TRIGS_SNDS)) {
			ihostUnloadSound(sound->soundId);
			sound->soundId = 0;
			diMUSERemoveSoundFromList(&_loadedSoundList, sound);
		}
	}

	for (sound = _openedSoundsList; sound; sound = next) {
		next = sound->next;
		if (!sound->refCount && !diMUSEGetParam(sound->soundId, DIMUSE_P_SND_TRACK_NUM) && !diMUSEGetParam(sound->soundId, DIMUSE_P_TRIGS_SNDS)) {
			ihostCloseSound(sound->soundId);
			sound->soundId = 0;
			diMUSERemoveSoundFromList(&_openedSoundsList, sound);
		}
	}
}

int IMuseDigital::diMUSEAddSoundToList(IMuseDigiSoundList **listPtr, IMuseDigiSoundList *item) {
	if (!item || item->prev || item->next) {
		debug(5, "IMuseDigital::diMUSEAddSoundToList(): ERROR: list arg err when adding");
		return -5;
	}

	item->next = *listPtr;

	if (*listPtr)
		(*listPtr)->prev = item;

	item->prev = nullptr;
	*listPtr = item;

	return 0;
}

int IMuseDigital::diMUSERemoveSoundFromList(IMuseDigiSoundList **listPtr, IMuseDigiSoundList *item) {
	IMuseDigiSoundList *curItem = *listPtr;

	if (item && curItem) {
		while (curItem) {
			if (curItem == item)
				break;
			curItem = curItem->next;
		}

		if (curItem) {
			if (item->next)
				item->next->prev = item->prev;

			if (item->prev)
				item->prev->next = item->next;
			else
				*listPtr = item->next;

			item->next = nullptr;
			item->prev = nullptr;

			return 0;
		} else {
			debug(5, "IMuseDigital::diMUSERemoveSoundFromList(): ERROR: item not on list");
			return -3;
		}
	} else {
		debug(5, "IMuseDigital::diMUSERemoveSoundFromList(): ERROR: list arg err when removing");
		return -5;
	}
}

bool IMuseDigital::filesCheckRange(int soundId) {
	return soundId && (uint32)soundId < 0xFFFFFFF0;
}

uint8 *IMuseDigital::filesGetSoundAddrData(int soundId) {
	if (filesCheckRange(soundId))
		return ihostGetSoundAddr(soundId);

	return nullptr;
}

uint8 *IMuseDigital::filesFetchMap(int soundId) {
	if (filesCheckRange(soundId))
		return ihostGetMapAddr(soundId);

	return nullptr;
}

int IMuseDigital::filesSeek(int soundId, int32 offset, int mode) {
	if (filesCheckRange(soundId))
		return ihostFseek(soundId, offset, mode);

	return 0;
}

int IMuseDigital::filesRead(int soundId, uint8 *buf, int32 size) {
	if (filesCheckRange(soundId))
		return ihostRead(soundId, buf, size);

	return 0;
}

IMuseDigiSndBuffer *IMuseDigital::filesGetBufInfo(int bufId) {
	if (bufId)
		return ihostGetBufferInfo(bufId);

	return nullptr;
}

int IMuseDigital::ihostLoadSound(const char *soundName, int requestedSoundId) {
	return 0;
}

int IMuseDigital::ihostUnloadSound(int soundId) {
	return 0;
}

int IMuseDigital::ihostOpenSound(const char *filename, int requestedSoundId) {
	Common::StackLock lock(*_mutex);
	IMuseDigiResource *res;
	int index;

	if (requestedSoundId) {
		index = requestedSoundId - 10000;
		_openAudioFiles[index].resourcePtr = soundsOpenFile(filename);
		Common::strlcpy(_openAudioFiles[index].soundName, filename, sizeof(_openAudioFiles[index].soundName));
		_openAudioFiles[index].refCount = 1;
		return requestedSoundId;
	}

	for (index = 1; index < DIMUSE_MAX_OPEN_FILES; index++) {
		if (!_openAudioFiles[index].resourcePtr)
			break;

		if (_openAudioFiles[index].soundName[0] && !strcmp(_openAudioFiles[index].soundName, filename)) {
			_openAudioFiles[index].refCount++;
			return index + 10000;
		}
	}

	if (index >= DIMUSE_MAX_OPEN_FILES)
		return 0;

	res = soundsOpenFile(filename);
	_openAudioFiles[index].resourcePtr = res;
	if (res) {
		Common::strlcpy(_openAudioFiles[index].soundName, filename, sizeof(_openAudioFiles[index].soundName));
		_openAudioFiles[index].refCount = 1;
		return index + 10000;
	}

	return 0;
}

int IMuseDigital::ihostCloseSound(int soundId) {
	Common::StackLock lock(*_mutex);
	int index = soundId - 10000;

	if (!_openAudioFiles[index].resourcePtr)
		return 1;

	if (_openAudioFiles[index].refCount > 1) {
		_openAudioFiles[index].refCount--;
		return 0;
	}

	soundsCloseFile(_openAudioFiles[index].resourcePtr);
	_openAudioFiles[index].resourcePtr = nullptr;
	_openAudioFiles[index].soundName[0] = 0;
	_openAudioFiles[index].refCount = 0;
	return 0;
}

uint8 *IMuseDigital::ihostGetMapAddr(int soundId) {
	// TODO
	return nullptr;
}

uint8 *IMuseDigital::ihostGetSoundAddr(int soundId) {
	// TODO
	return nullptr;
}

IMuseDigiSndBuffer *IMuseDigital::ihostGetBufferInfo(int bufId) {
	return &_soundBuffers[bufId];
}

int IMuseDigital::ihostFseek(int soundId, int32 offset, int mode) {
	Common::StackLock lock(*_mutex);

	if (soundId >= 10000 && soundId < 10000 + DIMUSE_MAX_OPEN_FILES) {
		IMuseDigiResource *res = _openAudioFiles[soundId - 10000].resourcePtr;
		if (res) {
			soundsFileSeek(res, offset, mode);
			return soundsFileTell(res);
		}
	}

	return 0;
}

int IMuseDigital::ihostRead(int soundId, uint8 *destBuf, int32 size) {
	Common::StackLock lock(*_mutex);

	IMuseDigiResource *res = _openAudioFiles[soundId - 10000].resourcePtr;
	if (!res)
		return 0;

	// TODO
	// Voice effects processing (hmiFX) for sound 10000

	return copyDataFromResource(res, destBuf, size);
}

void IMuseDigital::soundsAllocateBuffer(int bufferId, int32 bufSize, int32 loadSize, int32 criticalSize) {
	IMuseDigiSndBuffer *soundBuf = &_soundBuffers[bufferId];

	soundBuf->buffer = (uint8 *)malloc(bufSize);
	soundBuf->bufSize = bufSize;
	soundBuf->loadSize = loadSize;
	soundBuf->criticalSize = criticalSize;
}

void IMuseDigital::soundsDeallocateBuffer(int bufferId) {
	IMuseDigiSndBuffer *soundBuf = &_soundBuffers[bufferId];

	free(soundBuf->buffer);
	soundBuf->buffer = nullptr;
}

IMuseDigiResource *IMuseDigital::soundsOpenFile(const char *filename) {
	Common::String lowerName(filename);
	lowerName.toLowercase();

	Common::SeekableReadStream *stream = g_resourceloader->openNewStreamFile(lowerName.c_str());
	if (!stream)
		return nullptr;

	IMuseDigiResource *res = new IMuseDigiResource();
	memset(res, 0, sizeof(IMuseDigiResource));
	res->resId = stream;
	res->curProcessedCompBlock = -1;
	Common::strlcpy(res->filename, filename, sizeof(res->filename));

	if (stream->readUint32BE() != MKTAG('M', 'C', 'M', 'P')) {
		stream->seek(0, SEEK_SET);
		res->hasMcmp = false;
		res->resSize = stream->size();
		return res;
	}

	res->hasMcmp = true;

	int numCompBlocks = stream->readUint16BE();
	res->compBlocksNum = numCompBlocks;

	stream->seek(9 * numCompBlocks + 6, SEEK_SET);
	int codecNamesSize = stream->readUint16BE();

	uint8 *mcmpBlock = (uint8 *)malloc(9 * numCompBlocks);
	res->mcmpBlockPtr = (uint8 *)malloc(codecNamesSize + 1);
	res->compBlockPtr = new uint8 *[numCompBlocks + 1];
	res->decompBlockSize = new int32[numCompBlocks + 1];
	res->compBlockOffset = new int32[numCompBlocks + 1];
	res->compBlockSize = new int32[numCompBlocks + 1];
	res->vimaOffsets = new int32[numCompBlocks + 1];

	stream->read(res->mcmpBlockPtr, codecNamesSize);
	res->mcmpBlockPtr[codecNamesSize] = 0;

	stream->seek(6, SEEK_SET);
	stream->read(mcmpBlock, 9 * numCompBlocks);

	int32 cumulativeSize = 0;
	int32 vimaOffset = 9 * numCompBlocks + codecNamesSize + 8;
	uint8 *mcmpBlockPtr = mcmpBlock;

	for (int i = 0; i < numCompBlocks; i++) {
		uint8 *codecName = res->mcmpBlockPtr;
		int codec = *mcmpBlockPtr;

		for (int j = 0; j < codec; j++) {
			if (*codecName++)
				codecName += strlen((const char *)codecName) + 1;
		}

		res->compBlockPtr[i] = codecName;
		res->decompBlockSize[i] = READ_BE_UINT32(mcmpBlockPtr + 1);
		res->compBlockOffset[i] = cumulativeSize;
		cumulativeSize += res->decompBlockSize[i];
		res->compBlockSize[i] = READ_BE_UINT32(mcmpBlockPtr + 5);
		res->vimaOffsets[i] = vimaOffset;
		vimaOffset += res->compBlockSize[i];
		mcmpBlockPtr += 9;
	}

	res->compBlockOffset[numCompBlocks] = cumulativeSize;
	res->resSize = cumulativeSize;
	res->vimaOffsets[numCompBlocks] = vimaOffset;

	free(mcmpBlock);

	res->curDecompBlockSize = 0;
	res->decompDataBuffer = nullptr;
	res->curProcessedCompBlock = -1;
	return res;
}

void IMuseDigital::soundsCloseFile(IMuseDigiResource *res) {
	if (res->hasMcmp) {
		free(res->mcmpBlockPtr);
		delete[] res->compBlockPtr;
		delete[] res->decompBlockSize;
		delete[] res->compBlockOffset;
		delete[] res->compBlockSize;
		delete[] res->vimaOffsets;
		free(res->decompDataBuffer);
	}

	delete res->resId;
	delete res;
}

void IMuseDigital::soundsFileSeek(IMuseDigiResource *res, int32 offset, int mode) {
	if (!res->hasMcmp) {
		res->resId->seek(offset, mode);
	} else if (mode == SEEK_SET) {
		res->fileCurOffset = offset;
	} else if (mode == SEEK_CUR) {
		res->fileCurOffset += offset;
	} else if (mode == SEEK_END) {
		res->fileCurOffset = offset + res->resSize;
	}
}

int32 IMuseDigital::soundsFileTell(IMuseDigiResource *res) {
	if (!res->hasMcmp)
		return res->resId->pos();

	return res->fileCurOffset;
}

int IMuseDigital::soundsGetCompBlockId(IMuseDigiResource *res, int32 offset) {
	if ((uint32)offset < (uint32)res->resSize) {
		for (int i = 0; i < res->compBlocksNum; i++) {
			if ((uint32)offset >= (uint32)res->compBlockOffset[i] && (uint32)offset < (uint32)res->compBlockOffset[i + 1])
				return i;
		}
	}

	return -1;
}

int32 IMuseDigital::copyDataFromResource(IMuseDigiResource *res, uint8 *dest, int32 size) {
	uint8 *cmpDataTmpBuf = nullptr;
	uint32 curCompBlockSize = 0;
	uint8 *curDest = dest;
	int32 resSize, blockOffset, copySize;
	int curBlockIndex, firstBlockIndex, lastBlockIndex, compBlocksMinus1;

	if (!res->hasMcmp)
		return res->resId->read(dest, size);

	resSize = size;
	curBlockIndex = soundsGetCompBlockId(res, res->fileCurOffset);
	firstBlockIndex = curBlockIndex;
	lastBlockIndex = soundsGetCompBlockId(res, size + res->fileCurOffset - 1);
	compBlocksMinus1 = lastBlockIndex;

	if (curBlockIndex < 0)
		return 0;

	if (lastBlockIndex < 0) {
		compBlocksMinus1 = res->compBlocksNum - 1;
		size = res->resSize - res->fileCurOffset;
		resSize = size;
		lastBlockIndex = compBlocksMinus1;
	}

	if (resSize) {
		if (curBlockIndex <= lastBlockIndex) {
			while (1) {
				if (!res->decompDataBuffer || curBlockIndex != res->curProcessedCompBlock) {
					uint32 compBlockSize = res->compBlockSize[curBlockIndex];
					if (curCompBlockSize < compBlockSize) {
						cmpDataTmpBuf = (uint8 *)realloc(cmpDataTmpBuf, compBlockSize + 2);
						curCompBlockSize = compBlockSize;
					}

					res->resId->seek(res->vimaOffsets[curBlockIndex], SEEK_SET);
					res->resId->read(cmpDataTmpBuf, compBlockSize);

					// Two more zero bytes at the end of the input buffer
					cmpDataTmpBuf[compBlockSize] = 0;
					cmpDataTmpBuf[compBlockSize + 1] = 0;

					int32 decompBlockSize = res->decompBlockSize[curBlockIndex];
					if (res->curDecompBlockSize < decompBlockSize) {
						res->decompDataBuffer = (uint8 *)realloc(res->decompDataBuffer, decompBlockSize);
						res->curDecompBlockSize = decompBlockSize;
					}

					if (!memcmp(res->compBlockPtr[curBlockIndex], "VIMA", 4)) {
						decompressVima(cmpDataTmpBuf, (int16 *)res->decompDataBuffer, decompBlockSize, _imuseDestTable, false);
					} else {
						copySize = compBlockSize;
						if (decompBlockSize < copySize)
							copySize = decompBlockSize;

						memcpy(res->decompDataBuffer, cmpDataTmpBuf, copySize);
					}

					resSize = size;
					lastBlockIndex = compBlocksMinus1;
					res->curProcessedCompBlock = curBlockIndex;
				}

				if (firstBlockIndex == lastBlockIndex) {
					copySize = resSize;
					blockOffset = res->fileCurOffset - res->compBlockOffset[curBlockIndex];
				} else if (curBlockIndex == firstBlockIndex) {
					blockOffset = res->fileCurOffset - res->compBlockOffset[curBlockIndex];
					copySize = res->decompBlockSize[curBlockIndex] - blockOffset;
				} else {
					blockOffset = 0;
					if (curBlockIndex == lastBlockIndex)
						copySize = res->fileCurOffset + resSize - res->compBlockOffset[curBlockIndex];
					else
						copySize = res->decompBlockSize[curBlockIndex];
				}

				memcpy(curDest, &res->decompDataBuffer[blockOffset], copySize);
				curBlockIndex++;
				resSize = size;
				curDest += copySize;

				if (curBlockIndex > compBlocksMinus1)
					break;

				lastBlockIndex = compBlocksMinus1;
			}
		}

		free(cmpDataTmpBuf);
		res->fileCurOffset += resSize;
	}

	return resSize;
}

} // End of namespace Grim
