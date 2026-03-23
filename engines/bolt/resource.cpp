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

#include "common/memstream.h"

#include "bolt/bolt.h"

namespace Bolt {

void BoltEngine::decompress(byte *dest, uint32 decompSize, byte *src) {
	byte cmd;
	byte fillByte;
	uint16 count;
	uint16 offset;
	byte *backRef;

	for (;;) {
		cmd = *src++;

		switch (cmd & 0xC0) {
		case 0x00:
			count = 0x1F - (cmd & 0x1F);
			decompSize -= count;
			count++;
			while (--count)
				*dest++ = *src++;
			break;

		case 0x40:
			count = 0x23 - (cmd & 0x1F);
			decompSize -= count;
			offset = ((cmd & 0x20) << 3) + *src++;
			backRef = dest - offset;
			count++;
			while (--count)
				*dest++ = *backRef++;
			break;

		case 0x80:
			count = (0x20 - (cmd & 0x1F)) << 2;
			if (cmd & 0x20)
				count += 2;
			decompSize -= count;
			offset = *src++ * 2;
			backRef = dest - offset;
			count++;
			while (--count)
				*dest++ = *backRef++;
			break;

		case 0xC0:
			if (cmd & 0x20) {
				if ((int32)decompSize <= 0)
					return;
			} else {
				count = (0x20 - (cmd & 0x1F) + (*src++ << 5)) << 2;
				decompSize -= count;
				src++;
				fillByte = *src++;
				count++;
				while (--count)
					*dest++ = fillByte;
			}
			break;
		}
	}
}

bool BoltEngine::libRead(Common::File *fileHandle, uint32 offset, byte *dest, uint32 size) {
	uint32 bytesRead = size;

	if (_cachedFileHandle != fileHandle || _cachedFilePos != offset) {
		if (!_xp->setFilePos(fileHandle, offset, 0))
			return false;
	}

	if (_xp->readFile(fileHandle, dest, &bytesRead)) {
		if (bytesRead == size) {
			_cachedFileHandle = fileHandle;
			_cachedFilePos = offset + bytesRead;
			return true;
		}
	}

	_cachedFileHandle = nullptr;
	return false;
}

void BoltEngine::resolveIt(uint32 *ref) {
	if (*ref == 0xFFFFFFFF) {
		*ref = 0;
		return;
	}

	uint32 swapped = FROM_BE_32(*ref);

	byte *resolved = memberAddrOffset(_boltCurrentLib, swapped);

	if (resolved != nullptr) {
		uint32 idx = _resolvedPtrs.size();
		_resolvedPtrs.push_back(resolved);
		*ref = idx | 0x80000000; // High bit marks as resolved index
	} else {
		// Target not loaded yet, queue for later...
		*ref = swapped;
		if (_pendingFixupCount < _resourceIndexCount) {
			_resourceIndex[_pendingFixupCount] = ref;
			_pendingFixupCount++;
		}
	}
}

void BoltEngine::resolvePendingFixups() {
	while (_pendingFixupCount > 0) {
		_pendingFixupCount--;
		uint32 *ref = (uint32 *)_resourceIndex[_pendingFixupCount];

		byte *resolved = memberAddrOffset(_boltCurrentLib, *ref);

		uint32 idx = _resolvedPtrs.size();
		_resolvedPtrs.push_back(resolved);
		*ref = idx | 0x80000000;
	}
}

void BoltEngine::resolveFunction(uint32 *ref) {
	if (*ref == 0xFFFFFFFF) {
		*ref = 0;
		return;
	}

	uint16 index = (uint16)*ref;
	*ref = index;
}

void BoltEngine::resolveAllRefs() {
	byte *data = _boltCurrentMemberEntry->dataPtr;
	if (data == nullptr)
		return;

	uint32 count = _boltCurrentMemberEntry->decompSize / 4;
	uint32 *ptr = (uint32 *)data;
	for (uint32 i = 0; i < count; i++) {
		resolveIt(&ptr[i]);
	}
}

byte *BoltEngine::getResolvedPtr(byte *data, int offset) {
	uint32 val = READ_UINT32(data + offset);

	if (val == 0)
		return nullptr;

	if (val & 0x80000000)
		return _resolvedPtrs[val & 0x7FFFFFFF];

	return nullptr;
}

bool BoltEngine::openBOLTLib(BOLTLib **libPtr, BOLTCallbacks *callbacks, const char *fileName) {
	Common::File *fileHandle = nullptr;
	int16 groupCount = 0;

	// If already open, bump reference count...
	if (*libPtr) {
		(*libPtr)->refCount++;
		_boltCurrentLib = *libPtr;
		return true;
	}

	_boltCurrentLib = nullptr;

	fileHandle = _xp->openFile(fileName, 1);
	if (fileHandle) {
		// Read 16-byte BOLT file header...
		byte headerData[16];

		if (libRead(fileHandle, 0, headerData, 16)) {
			Common::SeekableReadStream *headerDataStream = new Common::MemoryReadStream(headerData, 16, DisposeAfterUse::NO);

			headerDataStream->read(_boltFileHeader.header, sizeof(_boltFileHeader.header));
			_boltFileHeader.groupCount = headerDataStream->readByte();
			_boltFileHeader.groupDirOffset = headerDataStream->readUint32BE();

			groupCount = _boltFileHeader.groupCount;
			if (groupCount == 0)
				groupCount = 256;

			delete headerDataStream;

			*libPtr = new BOLTLib(groupCount);

			_boltCurrentLib = *libPtr;
			if (_boltCurrentLib) {
				(*libPtr)->refCount = 0;
				(*libPtr)->groupCount = _boltFileHeader.groupCount;
				(*libPtr)->fileHandle = fileHandle;

				(*libPtr)->callbacks.typeLoadCallbacks = callbacks->typeLoadCallbacks;
				(*libPtr)->callbacks.typeFreeCallbacks = callbacks->typeFreeCallbacks;
				(*libPtr)->callbacks.memberLoadCallbacks = callbacks->memberLoadCallbacks;
				(*libPtr)->callbacks.memberFreeCallbacks = callbacks->memberFreeCallbacks;
				(*libPtr)->callbacks.groupLoadCallbacks = callbacks->groupLoadCallbacks;
				(*libPtr)->callbacks.groupFreeCallbacks = callbacks->groupFreeCallbacks;

				byte *groupEntryRawData = (byte *)_xp->allocMem(groupCount * 16);

				if (libRead(fileHandle, 0x10, groupEntryRawData, groupCount * 16)) {
					Common::SeekableReadStream *groupEntryDataStream = new Common::MemoryReadStream(groupEntryRawData, groupCount * 16, DisposeAfterUse::NO);

					for (int16 i = 0; i < groupCount; i++) {
						(*libPtr)->groups[i].flags = groupEntryDataStream->readByte();
						(*libPtr)->groups[i].loadCbIndex = groupEntryDataStream->readByte();
						(*libPtr)->groups[i].freeCbIndex = groupEntryDataStream->readByte();
						(*libPtr)->groups[i].memberCount = groupEntryDataStream->readByte();
						(*libPtr)->groups[i].memberDirOffset = groupEntryDataStream->readUint32BE();
						(*libPtr)->groups[i].memberDataOffset = groupEntryDataStream->readUint32BE();
						(*libPtr)->groups[i].groupDataPtrPlaceholder = groupEntryDataStream->readUint32BE();
						(*libPtr)->groups[i].memberData = nullptr;

						(*libPtr)->groups[i].initMembers((*libPtr)->groups[i].memberCount);
					}

					delete groupEntryDataStream;

					return true;
				}
			}
		}
	}

	if (fileHandle != 0)
		_xp->closeFile(fileHandle);

	if (_boltCurrentLib != nullptr) {
		delete _boltCurrentLib;
		_boltCurrentLib = nullptr;
	}

	return false;
}

bool BoltEngine::closeBOLTLib(BOLTLib **libPtr) {
	if (!*libPtr)
		return true;

	if ((*libPtr)->refCount > 0) {
		(*libPtr)->refCount--;
		return true;
	}

	int16 groupCount = (*libPtr)->groupCount;
	if (groupCount == 0) {
		groupCount = 256;
	}

	// Free all groups in reverse order...
	int16 groupId = groupCount << 8;
	while (groupCount-- > 0) {
		groupId -= (1 << 8);
		freeBOLTGroup(*libPtr, groupId, 0);
	}

	_xp->closeFile((*libPtr)->fileHandle);
	delete *libPtr;
	*libPtr = nullptr;

	return true;
}

bool BoltEngine::attemptFreeIndex(BOLTLib *lib, int16 groupId) {
	BOLTGroupEntry *groupEntry = &lib->groups[groupId >> 8];

	if (!groupEntry->memberData)
		return true;

	int16 memberCount = _boltCurrentGroupEntry->memberCount;
	if (memberCount == 0)
		memberCount = 256;

	for (int16 i = memberCount - 1; i >= 0; i--) {
		BOLTMemberEntry *member = &groupEntry->members[i];
		if (member->dataPtr)
			return false; // Still in use
	}

	_xp->freeMem(groupEntry->memberData);
	groupEntry->memberData = nullptr;

	return true;
}

bool BoltEngine::loadGroupDirectory() {
	int16 memberCount = _boltCurrentGroupEntry->memberCount;
	if (memberCount == 0)
		memberCount = 256;

	if (_boltCurrentGroupEntry->memberData) {
		_boltRawMemberData = _boltCurrentGroupEntry->memberData;
		return true;
	}

	byte *rawMemberData = (byte *)_xp->allocMem((uint32)memberCount * 16 + 4);
	_boltRawMemberData = rawMemberData;
	_boltCurrentGroupEntry->memberData = _boltRawMemberData;

	if (rawMemberData) {
		if (libRead(_boltCurrentLib->fileHandle, _boltCurrentGroupEntry->memberDataOffset, rawMemberData + 4, (uint32)memberCount * 16)) {
			Common::SeekableReadStream *memberEntryStream = new Common::MemoryReadStream(rawMemberData + 4, (uint32)memberCount * 16, DisposeAfterUse::NO);

			for (int16 i = 0; i < memberCount; i++) {
				_boltCurrentGroupEntry->members[i].flags = memberEntryStream->readByte();
				_boltCurrentGroupEntry->members[i].preLoadCbIndex = memberEntryStream->readByte();
				_boltCurrentGroupEntry->members[i].preFreeCbIndex = memberEntryStream->readByte();
				_boltCurrentGroupEntry->members[i].typeCbIndex = memberEntryStream->readByte();
				_boltCurrentGroupEntry->members[i].decompSize = memberEntryStream->readUint32BE();
				_boltCurrentGroupEntry->members[i].fileOffset = memberEntryStream->readUint32BE();
				_boltCurrentGroupEntry->members[i].dataPtrPlaceholder = memberEntryStream->readUint32BE();
				_boltCurrentGroupEntry->members[i].dataPtr = nullptr;
			}

			delete memberEntryStream;

			_boltRawMemberData = _boltCurrentGroupEntry->memberData;
			return true;
		}
	}

	if (_boltRawMemberData)
		_xp->freeMem(_boltRawMemberData);

	_boltRawMemberData = nullptr;
	_boltCurrentGroupEntry->memberData = nullptr;
	return false;
}

bool BoltEngine::getBOLTGroup(BOLTLib *lib, int16 groupId, int16 flags) {
	_boltLoadDepth++;
	int16 resId = groupId & 0xFF00;

	if (lib) {
		_boltCurrentLib = lib;
		_boltCurrentGroupEntry = &lib->groups[groupId >> 8];

		int16 memberCount = _boltCurrentGroupEntry->memberCount;
		if (memberCount == 0)
			memberCount = 256;

		if (loadGroupDirectory()) {
			if (_boltCurrentGroupEntry->loadCbIndex != 0) {
				lib->callbacks.groupLoadCallbacks[_boltCurrentGroupEntry->loadCbIndex]();
			}

			for (int16 i = 0; i < memberCount; i++) {
				if (!getBOLTMember(lib, resId)) {
					freeBOLTGroup(lib, resId, 0);
					_boltLoadDepth--;
					return false;
				}

				resId++;
			}

			resolvePendingFixups();
			_boltLoadDepth--;
			return true;
		}
	}

	freeBOLTGroup(lib, resId, 0);
	_boltLoadDepth--;
	return false;
}

void BoltEngine::freeBOLTGroup(BOLTLib *lib, int16 groupId, int16 flags) {
	if (!lib)
		return;

	_boltCurrentGroupEntry = &lib->groups[groupId >> 8];
	_boltRawMemberData = _boltCurrentGroupEntry->memberData;

	if (_boltCurrentGroupEntry->freeCbIndex != 0) {
		lib->callbacks.groupFreeCallbacks[_boltCurrentGroupEntry->freeCbIndex]();
	}

	if (!_boltRawMemberData)
		return;

	int16 memberCount = _boltCurrentGroupEntry->memberCount;
	if (memberCount == 0)
		memberCount = 256;

	int16 resId = groupId & 0xFF00;
	for (int16 i = 0; i < memberCount; i++) {
		freeBOLTMember(lib, resId);
		resId++;
	}

	attemptFreeIndex(lib, resId);
}

byte *BoltEngine::getBOLTMember(BOLTLib *lib, int16 resId) {
	byte memberIndex = (byte)resId;
	uint32 compressedSize;
	byte *tempBuf;

	if (lib) {
		_boltCurrentLib = lib;
		_boltCurrentGroupEntry = &lib->groups[resId >> 8];

		int16 memberCount = _boltCurrentGroupEntry->memberCount;
		if (memberCount == 0)
			memberCount = 256;

		if (loadGroupDirectory()) {
			_boltCurrentMemberEntry = &_boltCurrentGroupEntry->members[memberIndex];

			if (_boltCurrentMemberEntry->preLoadCbIndex != 0) {
				lib->callbacks.memberLoadCallbacks[_boltCurrentMemberEntry->preLoadCbIndex]();
			}

			if (_boltCurrentMemberEntry->dataPtr != nullptr)
				return _boltCurrentMemberEntry->dataPtr;

			_boltCurrentMemberEntry->dataPtr = (byte *)_xp->allocMem(_boltCurrentMemberEntry->decompSize);
			if (_boltCurrentMemberEntry->dataPtr) {
				// Bit 3 in flags = uncompressed
				if (_boltCurrentMemberEntry->flags & 0x08) {
					// Read directly into destination
					if (!libRead(lib->fileHandle, _boltCurrentMemberEntry->fileOffset, _boltCurrentMemberEntry->dataPtr, _boltCurrentMemberEntry->decompSize)) {
						if (_boltCurrentMemberEntry->dataPtr != nullptr) {
							_xp->freeMem(_boltCurrentMemberEntry->dataPtr);
							_boltCurrentMemberEntry->dataPtr = nullptr;
						}

						return nullptr;
					}
				} else {
					// Compressed member
					if (memberIndex + 1 < memberCount) {
						BOLTMemberEntry *nextEntry = &_boltCurrentGroupEntry->members[memberIndex + 1];
						compressedSize = nextEntry->fileOffset;
					} else {
						// Last entry: total = memberDirSize + memberDirOffset + memberDataOffset
						compressedSize = (uint32)(memberCount * 16) + _boltCurrentGroupEntry->memberDataOffset + _boltCurrentGroupEntry->memberDirOffset;
					}

					compressedSize -= _boltCurrentMemberEntry->fileOffset;

					tempBuf = (byte *)_xp->allocMem(compressedSize);
					if (!tempBuf) {
						if (_boltCurrentMemberEntry->dataPtr != nullptr) {
							_xp->freeMem(_boltCurrentMemberEntry->dataPtr);
							_boltCurrentMemberEntry->dataPtr = nullptr;
						}

						return nullptr;
					}

					if (!libRead(lib->fileHandle, _boltCurrentMemberEntry->fileOffset, tempBuf, compressedSize)) {
						_xp->freeMem(tempBuf);

						if (_boltCurrentMemberEntry->dataPtr != nullptr) {
							_xp->freeMem(_boltCurrentMemberEntry->dataPtr);
							_boltCurrentMemberEntry->dataPtr = nullptr;
						}

						return nullptr;
					}

					decompress(_boltCurrentMemberEntry->dataPtr, _boltCurrentMemberEntry->decompSize, tempBuf);

					_xp->freeMem(tempBuf);
				}

				lib->callbacks.typeLoadCallbacks[_boltCurrentMemberEntry->typeCbIndex]();

				if (_boltLoadDepth == 0)
					resolvePendingFixups();

				return _boltCurrentMemberEntry->dataPtr;
			}
		}
	}

	if (_boltCurrentMemberEntry->dataPtr != nullptr) {
		_xp->freeMem(_boltCurrentMemberEntry->dataPtr);
		_boltCurrentMemberEntry->dataPtr = nullptr;
	}

	return nullptr;
}

bool BoltEngine::freeBOLTMember(BOLTLib *lib, int16 resId) {
	if (!lib)
		return true;

	_boltCurrentGroupEntry = &lib->groups[resId >> 8];
	_boltRawMemberData = _boltCurrentGroupEntry->memberData;

	if (_boltRawMemberData == nullptr)
		return true;

	_boltCurrentMemberEntry = &_boltCurrentGroupEntry->members[(resId & 0xFF)];

	if (_boltCurrentMemberEntry->preFreeCbIndex != 0) {
		lib->callbacks.memberFreeCallbacks[_boltCurrentMemberEntry->preFreeCbIndex]();
	}

	if (_boltCurrentMemberEntry->dataPtr == nullptr)
		return true;

	lib->callbacks.typeFreeCallbacks[_boltCurrentMemberEntry->typeCbIndex]();

	_xp->freeMem(_boltCurrentMemberEntry->dataPtr);
	_boltCurrentMemberEntry->dataPtr = nullptr;

	return true;
}

Common::Rect BoltEngine::memberToRect(byte *data) {
	int16 x = (int16)READ_UINT16(data);
	int16 y = (int16)READ_UINT16(data + 2);
	int16 w = (int16)READ_UINT16(data + 4);
	int16 h = (int16)READ_UINT16(data + 6);
	return Common::Rect(x, y, x + w, y + h);
}

byte *BoltEngine::memberAddr(BOLTLib *lib, int16 resId) {
	if (!lib)
		return nullptr;

	BOLTGroupEntry *groupEntry = &lib->groups[resId >> 8];

	if (!groupEntry->memberData)
		return nullptr;

	BOLTMemberEntry *member = &groupEntry->members[(resId & 0xFF)];

	return member->dataPtr;
}

byte *BoltEngine::memberAddrOffset(BOLTLib *lib, uint32 resIdAndOffset) {
	if (!lib)
		return nullptr;

	int16 resId = (int16)(resIdAndOffset >> 16);
	int16 offset = (int16)(resIdAndOffset & 0xFFFF);

	BOLTGroupEntry *groupEntry = &lib->groups[resId >> 8];

	if (!groupEntry->memberData)
		return nullptr;

	BOLTMemberEntry *member = &groupEntry->members[(resId & 0xFF)];
	return (byte *)&member->dataPtr[offset];
}

uint32 BoltEngine::memberSize(BOLTLib *lib, int16 resId) {
	if (!lib)
		return 0;

	BOLTGroupEntry *groupEntry = &lib->groups[resId >> 8];

	if (!groupEntry->memberData)
		return 0;

	BOLTMemberEntry *member = &groupEntry->members[(resId & 0xFF)];
	return member->decompSize;
}

byte *BoltEngine::groupAddr(BOLTLib *lib, int16 groupId) {
	if (!lib)
		return nullptr;

	return lib->groups[groupId >> 8].memberData;
}

bool BoltEngine::allocResourceIndex() {
	_resourceIndex = (uint32 **)_xp->allocMem(_resourceIndexCount * sizeof(uintptr));
	if (!_resourceIndex)
		return false;

	return true;
}

void BoltEngine::freeResourceIndex() {
	if (_resourceIndex) {
		_xp->freeMem(_resourceIndex);
		_resourceIndex = nullptr;
	}
}

void BoltEngine::swapAllWords() {
	byte *data = _boltCurrentMemberEntry->dataPtr;
	if (!data)
		return;

	int16 count = int16(_boltCurrentMemberEntry->decompSize / 2);
	for (int16 i = 0; i < count; i++) {
		WRITE_UINT16(data, READ_BE_INT16(data));
		data += 2;
	}
}

void BoltEngine::swapAllLongs() {
	byte *data = _boltCurrentMemberEntry->dataPtr;
	if (!data)
		return;

	int16 count = (int16)(_boltCurrentMemberEntry->decompSize / 4);
	for (int16 i = 0; i < count; i++) {
		WRITE_UINT32(data, READ_BE_UINT32(data));
		data += 4;
	}
}

BOLTCallback BoltEngine::_defaultTypeLoadCallbacks[25];
BOLTCallback BoltEngine::_defaultTypeFreeCallbacks[25];
BOLTCallback BoltEngine::_defaultMemberLoadCallbacks[25];
BOLTCallback BoltEngine::_defaultMemberFreeCallbacks[25];
BOLTCallback BoltEngine::_defaultGroupLoadCallbacks[25];
BOLTCallback BoltEngine::_defaultGroupFreeCallbacks[25];

BOLTCallback BoltEngine::_fredTypeLoadCallbacks[28];
BOLTCallback BoltEngine::_fredTypeFreeCallbacks[28];

BOLTCallback BoltEngine::_georgeTypeLoadCallbacks[28];
BOLTCallback BoltEngine::_georgeTypeFreeCallbacks[28];

BOLTCallback BoltEngine::_huckTypeLoadCallbacks[27];
BOLTCallback BoltEngine::_huckTypeFreeCallbacks[27];

BOLTCallback BoltEngine::_scoobyTypeLoadCallbacks[28];
BOLTCallback BoltEngine::_scoobyTypeFreeCallbacks[28];

BOLTCallback BoltEngine::_topCatTypeLoadCallbacks[26];
BOLTCallback BoltEngine::_topCatTypeFreeCallbacks[26];

BOLTCallback BoltEngine::_yogiTypeLoadCallbacks[27];
BOLTCallback BoltEngine::_yogiTypeFreeCallbacks[27];

void BoltEngine::noOpCb() {}
void BoltEngine::swapAllWordsCb() { ((BoltEngine *)g_engine)->swapAllWords(); }
void BoltEngine::swapAllLongsCb() { ((BoltEngine *)g_engine)->swapAllLongs(); }
void BoltEngine::swapPicHeaderCb() { ((BoltEngine *)g_engine)->swapPicHeader(); }
void BoltEngine::swapAndResolvePicDescCb() { ((BoltEngine *)g_engine)->swapAndResolvePicDesc(); }
void BoltEngine::swapFirstWordCb() { ((BoltEngine *)g_engine)->swapFirstWord(); }
void BoltEngine::swapFirstTwoWordsCb() { ((BoltEngine *)g_engine)->swapFirstTwoWords(); }
void BoltEngine::swapFirstFourWordsCb() { ((BoltEngine *)g_engine)->swapFirstFourWords(); }
void BoltEngine::swapSpriteHeaderCb() { ((BoltEngine *)g_engine)->swapSpriteHeader(); }
void BoltEngine::freeSpriteCleanUpCb() { ((BoltEngine *)g_engine)->freeSpriteCleanUp(); }

void BoltEngine::resolveAllRefsCb() { ((BoltEngine *)g_engine)->resolveAllRefs(); }
void BoltEngine::swapFredAnimEntryCb() { ((BoltEngine *)g_engine)->swapFredAnimEntry(); }
void BoltEngine::swapFredAnimDescCb() { ((BoltEngine *)g_engine)->swapFredAnimDesc(); }
void BoltEngine::swapFredLevelDescCb() { ((BoltEngine *)g_engine)->swapFredLevelDesc(); }

void BoltEngine::swapGeorgeFrameArrayCb() { ((BoltEngine *)g_engine)->swapGeorgeFrameArray(); }
void BoltEngine::swapGeorgeHelpEntryCb() { ((BoltEngine *)g_engine)->swapGeorgeHelpEntry(); }
void BoltEngine::swapGeorgeThresholdsCb() { ((BoltEngine *)g_engine)->swapGeorgeThresholds(); }

void BoltEngine::swapHuckWordArrayCb() { ((BoltEngine *)g_engine)->swapHuckWordArray(); }
void BoltEngine::swapHuckWordsCb() { ((BoltEngine *)g_engine)->swapHuckWords(); }

void BoltEngine::swapScoobyHelpEntryCb() { ((BoltEngine *)g_engine)->swapScoobyHelpEntry(); }
void BoltEngine::swapScoobyWordArrayCb() { ((BoltEngine *)g_engine)->swapScoobyWordArray(); }

void BoltEngine::swapTopCatHelpEntryCb() { ((BoltEngine *)g_engine)->swapTopCatHelpEntry(); }

void BoltEngine::swapYogiAllWordsCb() { ((BoltEngine *)g_engine)->swapYogiAllWords(); }
void BoltEngine::swapYogiFirstWordCb() { ((BoltEngine *)g_engine)->swapYogiFirstWord(); }

void BoltEngine::initCallbacks() {
	// --- BOOTHS ---
	for (int i = 0; i < ARRAYSIZE(_defaultTypeLoadCallbacks); i++) {
		_defaultTypeLoadCallbacks[i] = noOpCb;
	}

	_defaultTypeLoadCallbacks[2] = swapAllWordsCb;
	_defaultTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_defaultTypeLoadCallbacks[10] = swapPicHeaderCb;
	_defaultTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_defaultTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_defaultTypeLoadCallbacks[14] = swapFirstFourWordsCb;

	for (int i = 0; i < ARRAYSIZE(_defaultTypeFreeCallbacks); i++) {
		_defaultTypeFreeCallbacks[i] = noOpCb;
	}

	_defaultTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	for (int i = 0; i < ARRAYSIZE(_defaultMemberLoadCallbacks); i++) {
		_defaultMemberLoadCallbacks[i] = noOpCb;
	}

	for (int i = 0; i < ARRAYSIZE(_defaultMemberFreeCallbacks); i++) {
		_defaultMemberFreeCallbacks[i] = noOpCb;
	}

	for (int i = 0; i < ARRAYSIZE(_defaultGroupLoadCallbacks); i++) {
		_defaultGroupLoadCallbacks[i] = noOpCb;
	}

	for (int i = 0; i < ARRAYSIZE(_defaultGroupFreeCallbacks); i++) {
		_defaultGroupFreeCallbacks[i] = noOpCb;
	}

	_boothsBoltCallbacks.typeLoadCallbacks = _defaultTypeLoadCallbacks;
	_boothsBoltCallbacks.typeFreeCallbacks = _defaultTypeFreeCallbacks;
	_boothsBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_boothsBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_boothsBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_boothsBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- FRED ---
	for (int i = 0; i < ARRAYSIZE(_fredTypeLoadCallbacks); i++) {
		_fredTypeLoadCallbacks[i] = noOpCb;
	}

	_fredTypeLoadCallbacks[2] = swapAllWordsCb;
	_fredTypeLoadCallbacks[6] = resolveAllRefsCb;
	_fredTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_fredTypeLoadCallbacks[10] = swapPicHeaderCb;
	_fredTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_fredTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_fredTypeLoadCallbacks[14] = swapFirstFourWordsCb;
	_fredTypeLoadCallbacks[25] = swapFredAnimEntryCb;
	_fredTypeLoadCallbacks[26] = swapFredAnimDescCb;
	_fredTypeLoadCallbacks[27] = swapFredLevelDescCb;

	for (int i = 0; i < ARRAYSIZE(_fredTypeFreeCallbacks); i++) {
		_fredTypeFreeCallbacks[i] = noOpCb;
	}

	_fredTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_fredBoltCallbacks.typeLoadCallbacks = _fredTypeLoadCallbacks;
	_fredBoltCallbacks.typeFreeCallbacks = _fredTypeFreeCallbacks;
	_fredBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_fredBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_fredBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_fredBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- GEORGE ---
	for (int i = 0; i < ARRAYSIZE(_georgeTypeLoadCallbacks); i++) {
		_georgeTypeLoadCallbacks[i] = noOpCb;
	}

	_georgeTypeLoadCallbacks[2] = swapAllWordsCb;
	_georgeTypeLoadCallbacks[6] = resolveAllRefsCb;
	_georgeTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_georgeTypeLoadCallbacks[10] = swapPicHeaderCb;
	_georgeTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_georgeTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_georgeTypeLoadCallbacks[14] = swapFirstFourWordsCb;
	_georgeTypeLoadCallbacks[25] = swapGeorgeFrameArrayCb;
	_georgeTypeLoadCallbacks[26] = swapGeorgeHelpEntryCb;
	_georgeTypeLoadCallbacks[27] = swapGeorgeThresholdsCb;

	for (int i = 0; i < ARRAYSIZE(_georgeTypeFreeCallbacks); i++) {
		_georgeTypeFreeCallbacks[i] = noOpCb;
	}

	_georgeTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_georgeBoltCallbacks.typeLoadCallbacks = _georgeTypeLoadCallbacks;
	_georgeBoltCallbacks.typeFreeCallbacks = _georgeTypeFreeCallbacks;
	_georgeBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_georgeBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_georgeBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_georgeBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- HUCK ---
	for (int i = 0; i < ARRAYSIZE(_huckTypeLoadCallbacks); i++) {
		_huckTypeLoadCallbacks[i] = noOpCb;
	}

	_huckTypeLoadCallbacks[2] = swapAllWordsCb;
	_huckTypeLoadCallbacks[6] = resolveAllRefsCb;
	_huckTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_huckTypeLoadCallbacks[10] = swapPicHeaderCb;
	_huckTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_huckTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_huckTypeLoadCallbacks[14] = swapFirstFourWordsCb;
	_huckTypeLoadCallbacks[25] = swapHuckWordArrayCb;
	_huckTypeLoadCallbacks[26] = swapHuckWordsCb;

	for (int i = 0; i < ARRAYSIZE(_huckTypeFreeCallbacks); i++) {
		_huckTypeFreeCallbacks[i] = noOpCb;
	}

	_huckTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_huckBoltCallbacks.typeLoadCallbacks = _huckTypeLoadCallbacks;
	_huckBoltCallbacks.typeFreeCallbacks = _huckTypeFreeCallbacks;
	_huckBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_huckBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_huckBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_huckBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- SCOOBY ---
	for (int i = 0; i < ARRAYSIZE(_scoobyTypeLoadCallbacks); i++) {
		_scoobyTypeLoadCallbacks[i] = noOpCb;
	}

	_scoobyTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_scoobyTypeLoadCallbacks[10] = swapPicHeaderCb;
	_scoobyTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_scoobyTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_scoobyTypeLoadCallbacks[26] = swapScoobyHelpEntryCb;
	_scoobyTypeLoadCallbacks[27] = swapScoobyWordArrayCb;

	for (int i = 0; i < ARRAYSIZE(_scoobyTypeFreeCallbacks); i++) {
		_scoobyTypeFreeCallbacks[i] = noOpCb;
	}

	_scoobyTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_scoobyBoltCallbacks.typeLoadCallbacks = _scoobyTypeLoadCallbacks;
	_scoobyBoltCallbacks.typeFreeCallbacks = _scoobyTypeFreeCallbacks;
	_scoobyBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_scoobyBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_scoobyBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_scoobyBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- TOPCAT ---
	for (int i = 0; i < ARRAYSIZE(_topCatTypeLoadCallbacks); i++) {
		_topCatTypeLoadCallbacks[i] = noOpCb;
	}

	_topCatTypeLoadCallbacks[2] = swapAllWordsCb;
	_topCatTypeLoadCallbacks[4] = swapAllLongsCb;
	_topCatTypeLoadCallbacks[6] = resolveAllRefsCb;
	_topCatTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_topCatTypeLoadCallbacks[10] = swapPicHeaderCb;
	_topCatTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_topCatTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_topCatTypeLoadCallbacks[25] = swapTopCatHelpEntryCb;

	for (int i = 0; i < ARRAYSIZE(_topCatTypeFreeCallbacks); i++) {
		_topCatTypeFreeCallbacks[i] = noOpCb;
	}

	_topCatTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_topCatBoltCallbacks.typeLoadCallbacks = _topCatTypeLoadCallbacks;
	_topCatBoltCallbacks.typeFreeCallbacks = _topCatTypeFreeCallbacks;
	_topCatBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_topCatBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_topCatBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_topCatBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- YOGI ---
	for (int i = 0; i < ARRAYSIZE(_yogiTypeLoadCallbacks); i++) {
		_yogiTypeLoadCallbacks[i] = noOpCb;
	}

	_yogiTypeLoadCallbacks[2] = swapAllWordsCb;
	_yogiTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_yogiTypeLoadCallbacks[10] = swapPicHeaderCb;
	_yogiTypeLoadCallbacks[14] = swapFirstFourWordsCb;
	_yogiTypeLoadCallbacks[25] = swapYogiFirstWordCb;
	_yogiTypeLoadCallbacks[26] = swapYogiAllWordsCb;

	for (int i = 0; i < ARRAYSIZE(_yogiTypeFreeCallbacks); i++) {
		_yogiTypeFreeCallbacks[i] = noOpCb;
	}

	_yogiTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_yogiBoltCallbacks.typeLoadCallbacks = _yogiTypeLoadCallbacks;
	_yogiBoltCallbacks.typeFreeCallbacks = _yogiTypeFreeCallbacks;
	_yogiBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_yogiBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_yogiBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_yogiBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;
}

} // End of namespace Bolt
