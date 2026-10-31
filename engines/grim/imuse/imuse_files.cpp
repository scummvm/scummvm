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
#include "common/system.h"

#include "engines/grim/resource.h"
#include "engines/grim/movie/codecs/vima.h"
#include "engines/grim/imuse/imuse_engine.h"

namespace Grim {

int IMuse::imInitFilelist() {
	for (int i = 0; i < IMUSE_MAX_SOUND_LIST; i++) {
		_mainSoundList[i].prev = nullptr;
		_mainSoundList[i].next = nullptr;
		_mainSoundList[i].soundId = 0;
	}

	_fileListInitialized = true;
	return 0;
}

void IMuse::imSaveLoadFilelist(Common::Serializer &ser) {
	IMuseSoundList *sound;
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
		for (int i = 0; i < IMUSE_MAX_SOUND_LIST; i++) {
			_mainSoundList[i].prev = nullptr;
			_mainSoundList[i].next = nullptr;
			_mainSoundList[i].soundId = 0;
		}
	}

	ser.syncAsSint32LE(loadedCount, VER(30));
	ser.syncAsSint32LE(openedCount, VER(30));

	sound = _loadedSoundList;
	for (int i = 0; i < loadedCount; i++) {
		if (ser.isSaving()) {
			soundId = sound->soundId;
			refCount = sound->refCount;
			Common::strlcpy(filename, sound->filename, sizeof(filename));
			sound = sound->next;
		}

		ser.syncAsSint32LE(soundId, VER(30));
		ser.syncArray(filename, sizeof(filename), Common::Serializer::SByte, VER(30));
		ser.syncAsSint32LE(refCount, VER(30));

		if (ser.isLoading()) {
			if (!imLoadSound(filename, soundId)) {
				debug(5, "IMuse::imSaveLoadFilelist(): ERROR: couldn't load file %s", filename);
				return;
			}

			if (soundId != _loadedSoundList->soundId) {
				debug(5, "IMuse::imSaveLoadFilelist(): ERROR: detected modified sound number");
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

		ser.syncAsSint32LE(soundId, VER(30));
		ser.syncArray(filename, sizeof(filename), Common::Serializer::SByte, VER(30));
		ser.syncAsSint32LE(refCount, VER(30));

		if (ser.isLoading()) {
			if (!imOpenSound(filename, soundId)) {
				debug(5, "IMuse::imSaveLoadFilelist(): ERROR: couldn't open file %s", filename);
			}

			if (soundId != _openedSoundsList->soundId) {
				debug(5, "IMuse::imSaveLoadFilelist(): ERROR: detected modified sound number");
				return;
			}

			_openedSoundsList->refCount = refCount;
		}
	}
}

int IMuse::imLoadSound(const char *soundName, int requestedSoundId) {
	IMuseSoundList *sound;
	int soundId;
	int nameIndex;

	if (!_fileListInitialized || !*soundName)
		return 0;

	soundId = imFindSound(soundName);
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
		if (++sound >= &_mainSoundList[IMUSE_MAX_SOUND_LIST]) {
			debug(5, "IMuse::imLoadSound(): Sound List FULL!");
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
			imAddSoundToList(&_loadedSoundList, sound);
			return soundId;
		}

		debug(5, "IMuse::imLoadSound(): Name too long: %s", soundName);
	} else {
		debug(5, "IMuse::imLoadSound(): Host couldn't load sound %s", soundName);
	}

	return 0;
}

void IMuse::imUnloadSound(int soundId) {
	IMuseSoundList *sound;

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

		imFlushSounds();
	}
}

void IMuse::imUnloadAll() {
	IMuseSoundList *sound;

	if (_fileListInitialized) {
		for (sound = _loadedSoundList; sound; sound = sound->next)
			sound->refCount = 0;

		imFlushSounds();
	}
}

int IMuse::imOpenSound(const char *filename, int requestedSoundId) {
	IMuseSoundList *sound;
	int soundId;
	int nameIndex;

	if (!_fileListInitialized || !*filename)
		return 0;

	soundId = imFindSound(filename);
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
		if (++sound >= &_mainSoundList[IMUSE_MAX_SOUND_LIST]) {
			debug(5, "IMuse::imOpenSound(): Sound List FULL!");
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
			imAddSoundToList(&_openedSoundsList, sound);
			return soundId;
		}

		debug(5, "IMuse::imOpenSound(): Name too long: %s", filename);
	} else {
		debug(5, "IMuse::imOpenSound(): Host couldn't open sound %s", filename);
	}

	return 0;
}

void IMuse::imCloseSound(int soundId) {
	IMuseSoundList *sound;

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

		imFlushSounds();
	}
}

void IMuse::imCloseAll() {
	IMuseSoundList *sound;

	if (_fileListInitialized) {
		for (sound = _openedSoundsList; sound; sound = sound->next)
			sound->refCount = 0;

		imFlushSounds();
	}
}

int IMuse::imFindSound(const char *soundName) {
	IMuseSoundList *sound;

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

void IMuse::imFlushSounds() {
	IMuseSoundList *sound, *next;

	for (sound = _loadedSoundList; sound; sound = next) {
		next = sound->next;
		if (!sound->refCount && !imGetParam(sound->soundId, IMUSE_P_SND_TRACK_NUM) && !imGetParam(sound->soundId, IMUSE_P_TRIGS_SNDS)) {
			ihostUnloadSound(sound->soundId);
			sound->soundId = 0;
			imRemoveSoundFromList(&_loadedSoundList, sound);
		}
	}

	for (sound = _openedSoundsList; sound; sound = next) {
		next = sound->next;
		if (!sound->refCount && !imGetParam(sound->soundId, IMUSE_P_SND_TRACK_NUM) && !imGetParam(sound->soundId, IMUSE_P_TRIGS_SNDS)) {
			ihostCloseSound(sound->soundId);
			sound->soundId = 0;
			imRemoveSoundFromList(&_openedSoundsList, sound);
		}
	}
}

int IMuse::imAddSoundToList(IMuseSoundList **listPtr, IMuseSoundList *item) {
	if (!item || item->prev || item->next) {
		debug(5, "IMuse::imAddSoundToList(): ERROR: list arg err when adding");
		return -5;
	}

	item->next = *listPtr;

	if (*listPtr)
		(*listPtr)->prev = item;

	item->prev = nullptr;
	*listPtr = item;

	return 0;
}

int IMuse::imRemoveSoundFromList(IMuseSoundList **listPtr, IMuseSoundList *item) {
	IMuseSoundList *curItem = *listPtr;

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
			debug(5, "IMuse::imRemoveSoundFromList(): ERROR: item not on list");
			return -3;
		}
	} else {
		debug(5, "IMuse::imRemoveSoundFromList(): ERROR: list arg err when removing");
		return -5;
	}
}

bool IMuse::filesCheckRange(int soundId) {
	return soundId > 0;
}

uint8 *IMuse::filesGetSoundAddrData(int soundId) {
	if (filesCheckRange(soundId))
		return ihostGetSoundAddr(soundId);

	return nullptr;
}

uint8 *IMuse::filesFetchMap(int soundId) {
	if (filesCheckRange(soundId))
		return ihostGetMapAddr(soundId);

	return nullptr;
}

int IMuse::filesSeek(int soundId, int32 offset, int mode) {
	if (filesCheckRange(soundId))
		return ihostFseek(soundId, offset, mode);

	return 0;
}

int IMuse::filesRead(int soundId, uint8 *buf, int32 size) {
	if (filesCheckRange(soundId))
		return ihostRead(soundId, buf, size);

	return 0;
}

IMuseSndBuffer *IMuse::filesGetBufInfo(int bufId) {
	if (bufId)
		return ihostGetBufferInfo(bufId);

	return nullptr;
}

int IMuse::ihostLoadSound(const char *soundName, int requestedSoundId) {
	return 0;
}

int IMuse::ihostUnloadSound(int soundId) {
	return 0;
}

int IMuse::ihostOpenSound(const char *filename, int requestedSoundId) {
	Common::StackLock lock(*_mutex);
	IMuseResource *res;
	int index;

	if (requestedSoundId) {
		index = requestedSoundId - 10000;
		_openAudioFiles[index].resourcePtr = soundsOpenFile(filename);
		Common::strlcpy(_openAudioFiles[index].soundName, filename, sizeof(_openAudioFiles[index].soundName));
		_openAudioFiles[index].refCount = 1;
		return requestedSoundId;
	}

	for (index = 1; index < IMUSE_MAX_OPEN_FILES; index++) {
		if (!_openAudioFiles[index].resourcePtr)
			break;

		if (_openAudioFiles[index].soundName[0] && !strcmp(_openAudioFiles[index].soundName, filename)) {
			_openAudioFiles[index].refCount++;
			return index + 10000;
		}
	}

	if (index >= IMUSE_MAX_OPEN_FILES)
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

int IMuse::ihostCloseSound(int soundId) {
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

uint8 *IMuse::ihostGetMapAddr(int soundId) {
	if (soundId < 100 || soundId >= 100 + IMUSE_MAX_CACHED_SOUNDS)
		return nullptr;

	return _soundCache[soundId - 100].soundMap;
}

uint8 *IMuse::ihostGetSoundAddr(int soundId) {
	if (soundId < 100 || soundId >= 100 + IMUSE_MAX_CACHED_SOUNDS)
		return nullptr;

	return _soundCache[soundId - 100].soundAddrData;
}

IMuseSndBuffer *IMuse::ihostGetBufferInfo(int bufId) {
	return &_soundBuffers[bufId];
}

int IMuse::ihostFseek(int soundId, int32 offset, int mode) {
	Common::StackLock lock(*_mutex);

	if (soundId >= 10000 && soundId < 10000 + IMUSE_MAX_OPEN_FILES) {
		IMuseResource *res = _openAudioFiles[soundId - 10000].resourcePtr;
		if (res) {
			soundsFileSeek(res, offset, mode);

			if (soundId == kTalkSoundID && _curEffectId && mode == SEEK_END)
				return _curEffectId * (soundsFileTell(res) + IMUSE_HMI_TAIL_SIZE - 44) + 44;

			if (soundId != kTalkSoundID || !_curEffectId || mode != SEEK_SET)
				return soundsFileTell(res);

			int32 dryOffset = offset;
			if (offset > 44)
				dryOffset = (offset - 44) / _curEffectId + 44;

			soundsFileSeek(res, dryOffset, SEEK_SET);
			return offset;
		}
	}

	return 0;
}

int IMuse::ihostRead(int soundId, uint8 *destBuf, int32 size) {
	Common::StackLock lock(*_mutex);

	IMuseResource *res = _openAudioFiles[soundId - 10000].resourcePtr;
	if (!res)
		return 0;

	if (soundId != kTalkSoundID || !_curEffectId)
		return copyDataFromResource(res, destBuf, size);

	bool isLastChunk = false;
	uint8 *effectBuf = nullptr;
	int32 effectSize = 0;

	int32 curPos = soundsFileTell(res);
	soundsFileSeek(res, 0, SEEK_END);
	int32 fileSize = soundsFileTell(res);
	soundsFileSeek(res, curPos, SEEK_SET);

	if (_maxChunkRequested < size) {
		free(_voiceSrcBuf);
		_voiceSrcBuf = (uint8 *)malloc(size);
		_maxChunkRequested = size;
	}

	// Past the end of the original file we only serve the tail of the effect
	if (curPos >= fileSize) {
		if (!_voiceRemainderSize) {
			memset(destBuf, 0, size);
			return size;
		}

		int32 copySize = MIN(_voiceRemainderSize - _voiceRemainderPos, size);
		memcpy(destBuf, _voiceRemainderBuf + _voiceRemainderPos, copySize);
		_voiceRemainderPos += copySize;

		if (copySize < size)
			memset(destBuf + copySize, 0, size - copySize);

		return size;
	}

	if (!curPos) {
		// The first chunk starts with the WAV header, which gets adapted to the processed audio
		int32 dataSize = size - 44;
		int32 readSize = copyDataFromResource(res, _voiceSrcBuf, dataSize / _curEffectId + 44);
		if (soundsFileTell(res) >= fileSize)
			isLastChunk = true;

		memcpy(destBuf, _voiceSrcBuf, 44);
		WRITE_LE_UINT32(destBuf + 40, (IMUSE_HMI_TAIL_SIZE + READ_LE_UINT32(destBuf + 40)) * _curEffectId);
		WRITE_LE_UINT32(destBuf + 28, READ_LE_UINT32(destBuf + 28) * _curEffectId);

		if (_curEffectId > 1) {
			destBuf[32] *= 2;
			destBuf[22] *= 2;
		}

		processSound(_voiceSrcBuf + 44, readSize - 44, destBuf + 44, &effectBuf, &effectSize, isLastChunk);
		if (isLastChunk && effectBuf) {
			memcpy(destBuf + 44, effectBuf, MIN(dataSize, effectSize));

			if (dataSize > effectSize)
				memset(destBuf + 44 + effectSize, 0, dataSize - effectSize);

			if (dataSize < effectSize) {
				_voiceRemainderSize = effectSize - dataSize;
				_voiceRemainderPos = 0;

				if (_voiceRemainderSize > _voiceRemainderCapacity) {
					free(_voiceRemainderBuf);
					_voiceRemainderBuf = (uint8 *)malloc(_voiceRemainderSize);
					_voiceRemainderCapacity = _voiceRemainderSize;
				}

				memcpy(_voiceRemainderBuf, effectBuf + dataSize, _voiceRemainderSize);
			} else {
				_voiceRemainderSize = 0;
			}

			free(effectBuf);
		}

		return size;
	}

	int32 readSize = copyDataFromResource(res, _voiceSrcBuf, size / _curEffectId);
	if (soundsFileTell(res) >= fileSize)
		isLastChunk = true;

	processSound(_voiceSrcBuf, readSize, destBuf, &effectBuf, &effectSize, isLastChunk);
	if (!isLastChunk || !effectBuf)
		return size;

	memcpy(destBuf, effectBuf, MIN(size, effectSize));

	if (size > effectSize)
		memset(destBuf + effectSize, 0, size - effectSize);

	if (size < effectSize) {
		_voiceRemainderSize = effectSize - size;
		_voiceRemainderPos = 0;

		if (_voiceRemainderSize > _voiceRemainderCapacity) {
			free(_voiceRemainderBuf);
			_voiceRemainderBuf = (uint8 *)malloc(_voiceRemainderSize);
			_voiceRemainderCapacity = _voiceRemainderSize;
		}

		memcpy(_voiceRemainderBuf, effectBuf + size, _voiceRemainderSize);
	} else {
		_voiceRemainderSize = 0;
	}

	free(effectBuf);
	return size;
}

void IMuse::soundsAllocateBuffer(int bufferId, int32 bufSize, int32 loadSize, int32 criticalSize) {
	IMuseSndBuffer *soundBuf = &_soundBuffers[bufferId];

	soundBuf->buffer = (uint8 *)malloc(bufSize);
	soundBuf->bufSize = bufSize;
	soundBuf->loadSize = loadSize;
	soundBuf->criticalSize = criticalSize;
}

void IMuse::soundsDeallocateBuffer(int bufferId) {
	IMuseSndBuffer *soundBuf = &_soundBuffers[bufferId];

	free(soundBuf->buffer);
	soundBuf->buffer = nullptr;
}

IMuseResource *IMuse::soundsOpenFile(const char *filename) {
	Common::String lowerName(filename);
	lowerName.toLowercase();

	Common::SeekableReadStream *stream = g_resourceloader->openNewStreamFile(lowerName.c_str());
	if (!stream)
		return nullptr;

	IMuseResource *res = new IMuseResource();
	memset(res, 0, sizeof(IMuseResource));
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

void IMuse::soundsCloseFile(IMuseResource *res) {
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

void IMuse::soundsFileSeek(IMuseResource *res, int32 offset, int mode) {
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

int32 IMuse::soundsFileTell(IMuseResource *res) {
	if (!res->hasMcmp)
		return res->resId->pos();

	return res->fileCurOffset;
}

int IMuse::soundsGetCompBlockId(IMuseResource *res, int32 offset) {
	if (offset >= 0 && offset < res->resSize) {
		for (int i = 0; i < res->compBlocksNum; i++) {
			if (offset >= res->compBlockOffset[i] && offset < res->compBlockOffset[i + 1])
				return i;
		}
	}

	return -1;
}

int32 IMuse::copyDataFromResource(IMuseResource *res, uint8 *dest, int32 size) {
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

int IMuse::cacheGetSoundId(const char *soundName) {
	uintptr soundKey = (uintptr)soundName;
	IMuseCachedSound *sound;
	int slot, soundId, loadSlot = -1;
	bool isLuaSoundId;
	bool allocated = false;

	if (!soundName)
		return 0;

	if (_demo) {
		// The cache of the demo has 16 slots and sounds are only looked up by name: there are
		// no Lua sound IDs (so a number can't be a valid name), and no lipsync data is loaded here
		if (soundKey <= 0xFFFF)
			return 0;

		for (slot = 15; slot >= 0; slot--) {
			if (!scumm_stricmp(_soundCache[slot].soundFilename, soundName))
				return slot + 100;

			if (!_soundCache[slot].soundFilename[0])
				loadSlot = slot;
		}

		// The sound is not in the cache: pick an empty slot or purge the oldest one
		if (loadSlot < 0) {
			loadSlot = cachePurgeOldestStoppedSound();
			if (loadSlot < 0)
				return 0;
		}

		sound = &_soundCache[loadSlot];
		Common::strlcpy(sound->soundFilename, soundName, sizeof(sound->soundFilename));
		sound->tsOfAllocation = g_system->getMillis();
		allocated = true;
	} else {
		if (soundKey == 10000)
			return 10000;

		if (soundKey <= 1111)
			return 0;

		isLuaSoundId = soundKey < 1367;
		if (soundKey > 1367) {
			if (strlen(soundName) <= 4) {
				soundId = atoi(soundName);
				if (soundId == 10000)
					return soundId;

				if (soundId >= 1111 && soundId < 1367)
					return 0;
			}

			isLuaSoundId = false;
		}

		sound = &_soundCache[IMUSE_MAX_CACHED_SOUNDS - 1];
		slot = IMUSE_MAX_CACHED_SOUNDS - 1;

		if (isLuaSoundId) {
			while ((uintptr)sound->luaSoundId != soundKey) {
				--sound;
				if (!slot--)
					return 0;
			}

			if (sound->soundUncompData)
				return slot + 100;

			loadSlot = slot;
		} else {
			while (scumm_stricmp(sound->soundFilename, soundName)) {
				if (!sound->soundFilename[0])
					loadSlot = slot;

				--sound;
				if (!slot--) {
					// The sound is not in the cache: pick an empty slot or purge the oldest one
					if (loadSlot < 0) {
						loadSlot = cachePurgeOldestStoppedSound();
						if (loadSlot < 0)
							return 0;
					}

					sound = &_soundCache[loadSlot];
					Common::strlcpy(sound->soundFilename, soundName, sizeof(sound->soundFilename));

					int slotCounter = _currentSoundSlot;
					bool collision;
					do {
						collision = false;
						int candidateId = abs(slotCounter) % 256 + 1111;
						for (int i = 0; i < IMUSE_MAX_CACHED_SOUNDS; i++) {
							if (_soundCache[i].soundFilename[0] && _soundCache[i].luaSoundId == candidateId) {
								collision = true;
								slotCounter++;
								break;
							}
						}
					} while (collision);

					_currentSoundSlot = slotCounter;
					sound->luaSoundId = abs(slotCounter) % 256 + 1111;
					++_currentSoundSlot;

					allocated = true;
					break;
				}
			}

			if (!allocated) {
				if (sound->soundUncompData)
					return slot + 100;

				loadSlot = slot;
			}
		}
	}

	// Load the sound in the cache
	sound->flag = 0;

	IMuseResource *res = soundsOpenFile(sound->soundFilename);
	if (!res) {
		// (The demo leaves the name of the sound in the slot)
		if (!_demo)
			memset(sound, 0, sizeof(IMuseCachedSound));

		return 0;
	}

	sound->tsOfAllocation = g_system->getMillis();

	if (!_demo) {
		char lipsyncFilename[32];
		Common::strlcpy(lipsyncFilename, sound->soundFilename, sizeof(lipsyncFilename));
		char *extension = strrchr(lipsyncFilename, '.');
		if (extension) {
			Common::strlcpy(extension, ".lip", lipsyncFilename + sizeof(lipsyncFilename) - extension);
			loadLip(lipsyncFilename, sound->luaSoundId);
		}
	}

	soundsFileSeek(res, 0, SEEK_END);
	sound->uncompResourceSize = soundsFileTell(res);
	soundsFileSeek(res, 0, SEEK_SET);

	sound->soundUncompData = (uint8 *)malloc(sound->uncompResourceSize + 80);
	if (sound->soundUncompData)
		copyDataFromResource(res, sound->soundUncompData, sound->uncompResourceSize);

	soundsCloseFile(res);

	int formatId, sampleRate, wordSize, numChannels, swapEndianFlag, dataLength;
	uint8 *extFormatBuffer = soundsGetExtFormatBuffer(sound->soundUncompData, &formatId, &sampleRate, &wordSize, &numChannels, &swapEndianFlag, &dataLength);

	if (formatId == 3 || formatId == 5) {
		if (wordSize == 8) {
			// Convert the 8-bit samples to 16-bit
			int headerSize = extFormatBuffer - sound->soundUncompData;
			int dataSize = sound->uncompResourceSize - headerSize;
			uint8 *convertedData = (uint8 *)malloc(2 * sound->uncompResourceSize + 80);

			memcpy(convertedData, sound->soundUncompData, headerSize);

			uint8 *dest = &convertedData[headerSize];
			for (int i = 0; i < dataSize; i++) {
				*dest++ = 0x80;
				*dest++ = extFormatBuffer[i] ^ 0x80;
			}

			sound->uncompResourceSize = headerSize + 2 * dataSize;
			free(sound->soundUncompData);
			sound->soundUncompData = convertedData;
			extFormatBuffer = &convertedData[headerSize];
			dataLength *= 2;
			swapEndianFlag = 1;
			wordSize = 16;
			formatId = 0;
		}

		if (formatId == 3) {
			sound->soundMap = sound->soundUncompData;
			sound->soundAddrData = sound->soundUncompData;
		} else {
			uint8 *newHeader = &sound->soundUncompData[sound->uncompResourceSize];
			createSoundHeader(newHeader, 3, sampleRate, 16, numChannels, dataLength);
			sound->soundMap = newHeader;
			sound->soundAddrData = extFormatBuffer - 80;

			if (!_demo) {
				newHeader[31] = swapEndianFlag;
			} else if (!swapEndianFlag) {
				// The demo swaps the samples right away
				uint32 numSamples = (uint32)(sound->soundUncompData + dataLength - extFormatBuffer) >> 1;
				for (uint32 i = 0; i < numSamples; i++)
					SWAP(extFormatBuffer[2 * i], extFormatBuffer[2 * i + 1]);
			}
		}

		return loadSlot + 100;
	}

	free(sound->soundUncompData);
	soundsResetLipsyncState(sound->luaSoundId);
	memset(sound, 0, sizeof(IMuseCachedSound));
	return 0;
}

int IMuse::cachePurgeOldestStoppedSound() {
	int foundSlot = -1;
	int emptySlot = -1;
	uint32 oldestTimestamp = g_system->getMillis();
	int numSlots = _demo ? 16 : IMUSE_MAX_CACHED_SOUNDS;

	for (int slot = numSlots - 1; slot >= 0; slot--) {
		IMuseCachedSound *sound = &_soundCache[slot];

		if (sound->soundFilename[0] && sound->soundUncompData) {
			if (!imGetParam(slot + 100, IMUSE_P_SND_TRACK_NUM) &&
				!imGetParam(slot + 100, IMUSE_P_TRIGS_SNDS) &&
				(foundSlot < 0 || oldestTimestamp > sound->tsOfAllocation)) {
				foundSlot = slot;
				oldestTimestamp = sound->tsOfAllocation;
			}
		} else {
			emptySlot = slot;
		}
	}

	if (foundSlot >= 0) {
		IMuseCachedSound *sound = &_soundCache[foundSlot];
		sound->soundFilename[0] = 0;
		if (!_demo)
			soundsResetLipsyncState(sound->luaSoundId);

		free(sound->soundUncompData);
		memset(sound, 0, sizeof(IMuseCachedSound));
		return foundSlot;
	}

	return emptySlot;
}

void IMuse::cacheReset() {
	for (int slot = IMUSE_MAX_CACHED_SOUNDS - 1; slot >= 0; slot--) {
		if (_soundCache[slot].soundUncompData) {
			soundsResetLipsyncState(_soundCache[slot].luaSoundId);
			free(_soundCache[slot].soundUncompData);
		}
	}

	_currentSoundSlot = 0;
	memset(_soundCache, 0, sizeof(_soundCache));
}

uint8 *IMuse::soundsGetExtFormatBuffer(uint8 *streamHeader, int *formatId, int *targetSampleRate, int *targetWordSize, int *targetNumChannels, int *swapEndianFlag, int *regionLength) {
	bool isMCMP = false;
	uint8 *header = streamHeader;

	if (formatId)
		*formatId = 0;

	if (!header)
		return nullptr;

	if (!memcmp(header, "MCMP", 4)) {
		int numCompBlocks = READ_BE_UINT16(header + 4);
		uint8 *codecNamesSize = header + 9 * numCompBlocks + 6;
		isMCMP = true;
		header = codecNamesSize + READ_BE_UINT16(codecNamesSize) + 2;
	}

	if (!memcmp(header, "RIFF", 4)) {
		uint8 *endOfFile = header + READ_LE_UINT32(header + 4) + 8;

		if (memcmp(header + 8, "WAVEfmt ", 8))
			return nullptr;

		if (READ_LE_UINT16(header + 20) != 1)
			return nullptr;

		if (formatId)
			*formatId = (isMCMP ? 1 : 0) + 5;

		uint8 *chunk = header + 16;
		uint32 chunkSize = READ_LE_UINT32(chunk);

		if (targetSampleRate)
			*targetSampleRate = READ_LE_UINT32(chunk + 8);

		if (targetWordSize)
			*targetWordSize = (int16)READ_LE_UINT16(chunk + 18);

		if (targetNumChannels)
			*targetNumChannels = (int16)READ_LE_UINT16(chunk + 6);

		if (swapEndianFlag)
			*swapEndianFlag = 1;

		while (1) {
			chunk += chunkSize + 8;
			chunkSize = READ_LE_UINT32(chunk);

			if (chunk >= endOfFile)
				break;

			if (!memcmp(chunk - 4, "data", 4)) {
				if (regionLength)
					*regionLength = chunkSize;

				return chunk + 4;
			}
		}

		return nullptr;
	}

	if (!memcmp(header, "iMUS", 4)) {
		uint32 mapSize = READ_BE_UINT32(header + 12);

		if (memcmp(header + 16, "FRMT", 4))
			return nullptr;

		if (formatId)
			*formatId = (isMCMP ? 1 : 0) + 3;

		uint8 *frmtBlock = header + 32;

		if (targetWordSize)
			*targetWordSize = READ_BE_UINT32(frmtBlock);

		if (targetSampleRate)
			*targetSampleRate = READ_BE_UINT32(frmtBlock + 4);

		if (targetNumChannels)
			*targetNumChannels = READ_BE_UINT32(frmtBlock + 8);

		if (swapEndianFlag)
			*swapEndianFlag = 0;

		if (regionLength)
			*regionLength = READ_BE_UINT32(frmtBlock + mapSize - 12);

		return frmtBlock + mapSize - 8;
	}

	if (isMCMP)
		return nullptr;

	if (!memcmp(header, "FORM", 4)) {
		uint8 *endOfFile = header + READ_BE_UINT32(header + 4) + 8;

		if (memcmp(header + 8, "AIFFCOMM", 8))
			return nullptr;

		if (formatId)
			*formatId = 7;

		uint8 *chunk = header + 16;
		uint32 chunkSize = READ_BE_UINT32(chunk);

		if (targetSampleRate) {
			if (chunk[13] == 14)
				*targetSampleRate = 44100;
			else if (chunk[13] == 12)
				*targetSampleRate = 11025;
			else
				*targetSampleRate = 22050;
		}

		if (targetWordSize)
			*targetWordSize = chunk[11];

		if (targetNumChannels)
			*targetNumChannels = chunk[5];

		if (swapEndianFlag)
			*swapEndianFlag = 0;

		while (1) {
			chunk += chunkSize + 8;
			chunkSize = READ_BE_UINT32(chunk);

			if (chunk >= endOfFile)
				break;

			if (!memcmp(chunk - 4, "SSND", 4)) {
				if (regionLength)
					*regionLength = chunkSize - 8;

				return chunk + 12;
			}
		}

		return nullptr;
	}

	// The original guesses the format of headerless audio (CreateAudioChunk), but no caller accepts it
	return nullptr;
}

int IMuse::createSoundHeader(uint8 *buffer, int mode, int sampleRate, int wordSize, int numChannels, int regionLength) {
	if (mode == 3) {
		memcpy(buffer, "iMUS", 4);
		WRITE_BE_UINT32(buffer + 4, regionLength + 80);
		memcpy(buffer + 8, "MAP ", 4);
		WRITE_BE_UINT32(buffer + 12, 56);
		memcpy(buffer + 16, "FRMT", 4);
		WRITE_BE_UINT32(buffer + 20, 20);
		WRITE_BE_UINT32(buffer + 24, 80);
		WRITE_BE_UINT32(buffer + 28, 0); // Endianness
		WRITE_BE_UINT32(buffer + 32, wordSize);
		WRITE_BE_UINT32(buffer + 36, sampleRate);
		WRITE_BE_UINT32(buffer + 40, numChannels);
		memcpy(buffer + 44, "REGN", 4);
		WRITE_BE_UINT32(buffer + 48, 8);
		WRITE_BE_UINT32(buffer + 52, 80);
		WRITE_BE_UINT32(buffer + 56, regionLength);
		memcpy(buffer + 60, "STOP", 4);
		WRITE_BE_UINT32(buffer + 64, 4);
		WRITE_BE_UINT32(buffer + 68, regionLength + 80);
		memcpy(buffer + 72, "DATA", 4);
		WRITE_BE_UINT32(buffer + 76, regionLength);
		return 80;
	}

	return 0;
}

int IMuse::soundsLoadStartSound(const char *soundName, int priority, int group) {
	int soundId = cacheGetSoundId(soundName);
	if (!soundId)
		return 0;

	if (_demo) {
		imStartSound(soundId, priority);
		return imSetParam(soundId, IMUSE_P_GROUP, group);
	}

	if (priority > 124)
		priority = 124;

	imStartSound(soundId, priority);
	imSetParam(soundId, IMUSE_P_GROUP, group);
	_soundCache[soundId - 100].priority = priority;
	return _soundCache[soundId - 100].luaSoundId;
}

int IMuse::soundsLoadProcessStartSound(const char *soundName, int priority, int group) {
	int soundId = cacheGetSoundId(soundName);
	if (!soundId)
		return 0;

	if (priority > 124)
		priority = 124;

	if (_curEffectId) {
		IMuseCachedSound *sound = &_soundCache[soundId - 100];
		if (sound->luaSoundId && !sound->flag)
			allocateProcessedSound(sound->luaSoundId);
	}

	imStartSound(soundId, priority);
	imSetParam(soundId, IMUSE_P_GROUP, group);
	_soundCache[soundId - 100].priority = priority;
	return _soundCache[soundId - 100].luaSoundId;
}

void IMuse::soundsStopSound(const char *soundName) {
	int soundId = cacheGetSoundId(soundName);
	if (soundId)
		imStopSound(soundId);
}

int IMuse::soundsGetParam(const char *soundName, int paramId) {
	int soundId = cacheGetSoundId(soundName);
	if (soundId)
		return imGetParam(soundId, paramId);

	return 1;
}

void IMuse::soundsSetParam(const char *soundName, int paramId, int value) {
	int soundId = cacheGetSoundId(soundName);
	if (soundId)
		imSetParam(soundId, paramId, value);
}

int IMuse::soundsFadeParam(const char *soundName, int paramId, int value, int length) {
	int soundId = cacheGetSoundId(soundName);
	if (soundId)
		return imFadeParam(soundId, paramId, value, _demo ? length : 60 * length / 1000);

	return 1;
}

bool IMuse::soundsIsSoundInUse(const char *soundName) {
	uint32 soundIndex = cacheGetSoundId(soundName) - 100;
	if (soundIndex >= IMUSE_MAX_CACHED_SOUNDS || !_soundCache[soundIndex].soundFilename[0] || !_soundCache[soundIndex].soundUncompData)
		return false;

	int soundId = soundIndex + 100;
	return imGetParam(soundId, IMUSE_P_SND_TRACK_NUM) > 0 || imGetParam(soundId, IMUSE_P_TRIGS_SNDS) > 0;
}

bool IMuse::soundsIsSoundPlaying(const char *soundName) {
	int soundId = cacheGetSoundId(soundName);
	return imGetParam(soundId, IMUSE_P_SND_TRACK_NUM) > 0;
}

int IMuse::iMUSESetHook(const char *soundName, int hookId) {
	int soundId = cacheGetSoundId(soundName);
	if (soundId)
		return imSetHook(soundId, hookId);

	return -1;
}

int IMuse::iMUSEGetHook(const char *soundName) {
	int soundId = cacheGetSoundId(soundName);
	if (soundId)
		return imGetHook(soundId);

	return -1;
}

int IMuse::iMUSEGetMemoryFootprint() {
	// The original variable is never set
	return 0;
}

int IMuse::iMUSEGetSoundCacheSize() {
	// The original variable is never set
	return 0;
}

void IMuse::iMUSESetVoiceEffect(const char *effectName) {
	Common::strlcpy(_curEffectName, effectName, sizeof(_curEffectName));

	if (!_hmiFxInitialized || !scumm_stricmp(effectName, "OFF"))
		_curEffectId = 0;
	else
		_curEffectId = registerHMIEffect(effectName);
}

void IMuse::allocateProcessedSound(int luaSoundId) {
	if (luaSoundId < 1111 || luaSoundId >= 1367)
		return;

	int slot = IMUSE_MAX_CACHED_SOUNDS - 1;
	while (_soundCache[slot].luaSoundId != luaSoundId) {
		if (!slot--)
			return;
	}

	IMuseCachedSound *sound = &_soundCache[slot];
	if (!sound->soundUncompData || !sound->soundAddrData)
		return;

	int sampleRate = 0, wordSize = 0, numChannels = 0, dataLength = 0;
	soundsGetExtFormatBuffer(sound->soundMap, nullptr, &sampleRate, &wordSize, &numChannels, nullptr, &dataLength);

	if (sampleRate == 22050 && wordSize == 16 && numChannels == 1 && dataLength > 0) {
		uint8 *processedBuf = (uint8 *)malloc(2 * dataLength + 80);
		if (processedBuf) {
			int effectChannels = processSound(sound->soundAddrData + 80, dataLength, processedBuf, nullptr, nullptr, false);

			free(sound->soundUncompData);
			sound->soundUncompData = processedBuf;

			int regionLength = dataLength;
			if (effectChannels == 2)
				regionLength *= 2;

			sound->uncompResourceSize = regionLength;
			createSoundHeader(&processedBuf[regionLength], 3, 22050, 16, effectChannels, regionLength);

			sound->soundMap = &processedBuf[regionLength];
			sound->soundMap[31] = 1;
			sound->soundAddrData = processedBuf - 80;
			sound->flag = 1;
		}

		sound->tsOfAllocation = g_system->getMillis();
	}
}

void IMuse::soundsPeriodic() {
	int32 cachedSize = 0;

	imProcessStreams();
	imRefreshScript();

	for (int slot = (_demo ? 16 : IMUSE_MAX_CACHED_SOUNDS) - 1; slot >= 0; slot--) {
		if (_soundCache[slot].soundFilename[0] && _soundCache[slot].soundUncompData)
			cachedSize += _soundCache[slot].uncompResourceSize;
	}

	if (cachedSize > IMUSE_SOUNDS_MEMORY_LIMIT)
		cachePurgeOldestStoppedSound();
}

int IMuse::soundsPause() {
	imPause();
	_globalVolume = imSetGroupVol(0, -1);
	imSetGroupVol(0, 0);
	imSetAttribute(-1, 0);
	return 0;
}

int IMuse::soundsResume() {
	imSetGroupVol(0, _globalVolume);
	imResume();
	imSetAttribute(-1, 1);
	return 0;
}

int IMuse::soundsRegisterVoice(const char *filename) {
	char lipsyncFilename[32];

	Common::strlcpy(_curSpeechFile, filename, sizeof(_curSpeechFile));
	Common::strlcpy(lipsyncFilename, filename, sizeof(lipsyncFilename));

	// The demo doesn't register the lipsync here, and doesn't compute the duration of the file
	if (!_demo) {
		char *extension = strrchr(lipsyncFilename, '.');
		if (extension) {
			Common::strlcpy(extension, ".lip", lipsyncFilename + sizeof(lipsyncFilename) - extension);
			loadLip(lipsyncFilename, kTalkSoundID);
		}
	}

	IMuseResource *res = soundsOpenFile(filename);
	_openAudioFiles[0].resourcePtr = res;
	if (_demo)
		return res ? 1 : 0;

	if (!res) {
		_millisecsInCurSpeechFile = 0;
		return 0;
	}

	soundsFileSeek(res, 0, SEEK_END);
	int dataSamplesInFile = soundsFileTell(res) - 44;
	soundsFileSeek(res, 0, SEEK_SET);
	_millisecsInCurSpeechFile = 10 * dataSamplesInFile / 441;
	return 1;
}

void IMuse::soundsCloseVoiceFile() {
	if (_openAudioFiles[0].resourcePtr) {
		soundsCloseFile(_openAudioFiles[0].resourcePtr);
		if (!_demo)
			soundsResetLipsyncState(kTalkSoundID);
		_openAudioFiles[0].resourcePtr = nullptr;
	}
}

void IMuse::soundsReOpenVoiceFile() {
	soundsCloseVoiceFile();
	if (_curSpeechFile[0])
		soundsRegisterVoice(_curSpeechFile);
}

int IMuse::soundStartVoiceStream(const char *filename, int volume, int pan) {
	imStopSound(kTalkSoundID);
	soundsCloseVoiceFile();
	if (!soundsRegisterVoice(filename))
		return 1;

	soundsFillStreamsWhileMusicCritical(5);
	imStartStream(kTalkSoundID, 127, IMUSE_BUFFER_SPEECH);
	imSetParam(kTalkSoundID, IMUSE_P_GROUP, IMUSE_GROUP_SPEECH);
	imSetParam(kTalkSoundID, IMUSE_P_VOLUME, volume);
	imSetParam(kTalkSoundID, IMUSE_P_PAN, pan);
	imProcessStreams();
	imProcessStreams();
	return 0;
}

int IMuse::soundStopVoiceStream() {
	imStopSound(kTalkSoundID);
	return ihostCloseSound(kTalkSoundID);
}

int IMuse::soundGetVoiceTime() {
	return imGetParam(kTalkSoundID, IMUSE_P_SND_POS_IN_MS);
}

int IMuse::loadLip(const char *lipsyncFilename, int soundId) {
	Common::String lowerName(lipsyncFilename);
	lowerName.toLowercase();

	Common::SeekableReadStream *lipsyncFile = g_resourceloader->openNewStreamFile(lowerName.c_str());
	if (!lipsyncFile) {
		soundsResetLipsyncState(soundId);
		return 0;
	}

	if (!soundId) {
		delete lipsyncFile;
		return 0;
	}

	for (int i = 1; i < IMUSE_MAX_LIPSYNCS; i++) {
		if (soundId == _lipsyncSoundIds[i]) {
			delete lipsyncFile;
			return 1;
		}
	}

	int slot = -1;
	if (soundId == kTalkSoundID) {
		slot = 0;
	} else {
		for (int i = 1; i < IMUSE_MAX_LIPSYNCS; i++) {
			if (!_lipsyncSoundIds[i]) {
				slot = i;
				break;
			}
		}
	}

	if (slot >= 0) {
		_lipsyncSoundIds[slot] = soundId;

		int fileSize = lipsyncFile->size();
		if (fileSize <= IMUSE_LIPSYNC_MAX_SIZE) {
			// Clear the old data, or it would be read after the end of the new file
			memset(_lipsyncBufs[slot], 0, sizeof(_lipsyncBufs[slot]));
			lipsyncFile->read(_lipsyncBufs[slot], fileSize);

			if (memcmp(_lipsyncBufs[slot], "LIP!", 4))
				_lipsyncSoundIds[slot] = 0;
		}
	}

	delete lipsyncFile;
	return 0;
}

void IMuse::soundsResetLipsyncState(int soundId) {
	for (int i = 0; i < IMUSE_MAX_LIPSYNCS; i++) {
		if (soundId == _lipsyncSoundIds[i])
			_lipsyncSoundIds[i] = 0;
	}
}

int IMuse::soundsGetPhoneme(int soundId, int offset) {
	uint8 *lipsyncBuffer;

	if (soundId == kTalkSoundID) {
		if (!_lipsyncSoundIds[0])
			return 0;

		lipsyncBuffer = _lipsyncBufs[0];
	} else if (soundId == _lipsyncSoundIds[1]) {
		lipsyncBuffer = _lipsyncBufs[1];
	} else if (soundId == _lipsyncSoundIds[2]) {
		lipsyncBuffer = _lipsyncBufs[2];
	} else if (soundId == _lipsyncSoundIds[3]) {
		lipsyncBuffer = _lipsyncBufs[3];
	} else {
		return 0;
	}

	int numEntries = READ_LE_INT32(lipsyncBuffer + 4);

	// The number of entries in the file is often bigger than the real one, don't read out of the buffer
	int maxEntries = (sizeof(_lipsyncBufs[0]) - 12) / 4;
	if (numEntries > maxEntries)
		numEntries = maxEntries;

	int offset16MsTicks = offset >> 4;
	uint8 *entry = lipsyncBuffer + 8;

	for (int i = 0; i < numEntries; i++) {
		if (offset16MsTicks < READ_LE_INT16(entry))
			break;

		if (offset16MsTicks < READ_LE_INT16(entry + 4))
			return READ_LE_INT16(entry + 2);

		entry += 4;
	}

	return IMUSE_PHONEME_NONE;
}

void IMuse::soundsSaveLoad(Common::Serializer &ser) {
	Common::StackLock lock(*_mutex);

	char effectName[sizeof(_curEffectName)];

	if (ser.isSaving()) {
		Common::strlcpy(effectName, _curEffectId ? _curEffectName : "OFF", sizeof(effectName));
		_curSpeechFile[0] = '\0';
	}

	ser.syncArray(effectName, sizeof(effectName), Common::Serializer::SByte, VER(30));

	if (ser.isLoading()) {
		imStopAllSounds();
		imFlushSounds();
		soundsCloseVoiceFile();
		cacheReset();
		iMUSESetVoiceEffect(effectName);
		_curSpeechFile[0] = '\0';
	}

	imSaveLoad(ser);
	imSaveLoadScript(ser);

	if (ser.isLoading()) {
		for (int soundId = imGetNextSound(0); soundId; soundId = imGetNextSound(soundId)) {
			if (soundId <= kTalkSoundID)
				imStopSound(soundId);
		}

		soundsCloseVoiceFile();
	}

	int32 numSounds = 0;
	int savedSlots[IMUSE_MAX_CACHED_SOUNDS];

	if (ser.isSaving()) {
		// Only active sounds with a native iMUS map are saved...
		for (int slot = 0; slot < IMUSE_MAX_CACHED_SOUNDS; slot++) {
			IMuseCachedSound *sound = &_soundCache[slot];

			if (sound->soundFilename[0] && sound->soundUncompData &&
				imGetParam(slot + 100, IMUSE_P_SND_TRACK_NUM) > 0 &&
				!memcmp(sound->soundMap, "iMUS", 4) && READ_BE_UINT16(sound->soundMap + 14) != 56)
				savedSlots[numSounds++] = slot;
		}
	}

	ser.syncAsSint32LE(numSounds, VER(30));
	if (numSounds < 0 || numSounds > IMUSE_MAX_CACHED_SOUNDS)
		return;

	// The cached sounds are restarted with the same ids, volume, pan and hook
	for (int i = 0; i < numSounds; i++) {
		char soundName[32] = {};
		int32 luaSoundId = 0, priority = 0;

		if (ser.isSaving()) {
			Common::strlcpy(soundName, _soundCache[savedSlots[i]].soundFilename, sizeof(soundName));
			luaSoundId = _soundCache[savedSlots[i]].luaSoundId;
			priority = _soundCache[savedSlots[i]].priority;
		}

		ser.syncArray(soundName, sizeof(soundName), Common::Serializer::SByte, VER(30));
		ser.syncAsSint32LE(luaSoundId, VER(30));
		ser.syncAsSint32LE(priority, VER(30));

		if (ser.isLoading()) {
			_currentSoundSlot = luaSoundId - 1111;
			soundsLoadStartSound(soundName, priority, IMUSE_GROUP_SFX);
		}
	}

	for (int i = 0; i < numSounds; i++) {
		int32 luaSoundId = 0, volume = 0, pan = 0;

		if (ser.isSaving()) {
			luaSoundId = _soundCache[savedSlots[i]].luaSoundId;
			volume = imGetParam(savedSlots[i] + 100, IMUSE_P_VOLUME);
			pan = imGetParam(savedSlots[i] + 100, IMUSE_P_PAN);
		}

		ser.syncAsSint32LE(luaSoundId, VER(30));
		ser.syncAsSint32LE(volume, VER(30));
		ser.syncAsSint32LE(pan, VER(30));

		if (ser.isLoading()) {
			soundsSetParam((const char *)(intptr)luaSoundId, IMUSE_P_VOLUME, volume);
			soundsSetParam((const char *)(intptr)luaSoundId, IMUSE_P_PAN, pan);
		}
	}

	for (int i = 0; i < numSounds; i++) {
		int32 luaSoundId = 0, hookId = 0;

		if (ser.isSaving()) {
			luaSoundId = _soundCache[savedSlots[i]].luaSoundId;
			hookId = imGetHook(savedSlots[i] + 100);
		}

		ser.syncAsSint32LE(luaSoundId, VER(30));
		ser.syncAsSint32LE(hookId, VER(30));

		if (ser.isLoading())
			iMUSESetHook((const char *)(intptr)luaSoundId, hookId);
	}
}

void IMuse::soundsRestoreLegacy(int32 stateIndex, int32 seqIndex) {
	Common::StackLock lock(*_mutex);

	// The savegames of the old iMUSE only stored the music state and sequence, so
	// the rest is reset and the music is started again as it was
	imStopAllSounds();
	imFlushSounds();
	soundsCloseVoiceFile();
	cacheReset();
	_curSpeechFile[0] = '\0';

	int stateId = grimLegacyStateSoundId(stateIndex);
	int seqId = (seqIndex >= 0 && seqIndex < ARRAYSIZE(_grimSeqMusicTable)) ? _grimSeqMusicTable[seqIndex].soundId : -1;

	_curMusicState = 0;
	_curMusicSeq = 0;
	_nextSeqToPlay = 0;
	memset(_attributes, 0, sizeof(_attributes));

	scriptSetSequence(seqId);
	scriptSetState(stateId);
}

int IMuse::iMUSEGetPhoneme(int lipsyncId, int offset) {
	if (lipsyncId == _lipsyncSoundIds[0])
		return soundsGetPhoneme(lipsyncId, offset + soundGetVoiceTime());

	if (lipsyncId == _lipsyncSoundIds[1] || lipsyncId == _lipsyncSoundIds[2] || lipsyncId == _lipsyncSoundIds[3]) {
		int soundId = 0;
		for (int slot = 0; slot < IMUSE_MAX_CACHED_SOUNDS && !soundId; slot++) {
			if (_soundCache[slot].luaSoundId == lipsyncId)
				soundId = slot + 100;
		}

		if (!soundId)
			return 0;

		return soundsGetPhoneme(lipsyncId, offset + imGetParam(soundId, IMUSE_P_SND_POS_IN_MS));
	}

	return 0;
}

} // End of namespace Grim
