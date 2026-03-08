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

#include "bolt/bolt.h"

#include "common/memstream.h"

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

	if (g_cachedFileHandle != fileHandle || g_cachedFilePos != offset) {
		if (!_xp->setFilePos(fileHandle, offset, 0))
			return false;
	}

	if (_xp->readFile(fileHandle, dest, &bytesRead)) {
		if (bytesRead == size) {
			g_cachedFileHandle = fileHandle;
			g_cachedFilePos = offset + bytesRead;
			return true;
		}
	}

	g_cachedFileHandle = nullptr;
	return false;
}

void BoltEngine::resolveIt(uint32 *ref) {
	if (*ref == 0xFFFFFFFF) {
		*ref = 0;
		return;
	}

	uint32 swapped = FROM_BE_32(*ref);

	byte *resolved = memberAddrOffset(g_boltCurrentLib, swapped);

	if (resolved != nullptr) {
		uint32 idx = g_resolvedPtrs.size();
		g_resolvedPtrs.push_back(resolved);
		*ref = idx | 0x80000000; // High bit marks as resolved index
	} else {
		// Target not loaded yet, queue for later...
		*ref = swapped;
		if (g_pendingFixupCount < g_resourceIndexCount) {
			g_resourceIndex[g_pendingFixupCount] = ref;
			g_pendingFixupCount++;
		}
	}
}

void BoltEngine::resolvePendingFixups() {
	while (g_pendingFixupCount > 0) {
		g_pendingFixupCount--;
		uint32 *ref = (uint32 *)g_resourceIndex[g_pendingFixupCount];

		byte *resolved = memberAddrOffset(g_boltCurrentLib, *ref);

		uint32 idx = g_resolvedPtrs.size();
		g_resolvedPtrs.push_back(resolved);
		*ref = idx | 0x80000000;
	}
}

void BoltEngine::resolveFunction(uint32 *ref) {
	if (*ref == 0xFFFFFFFF) {
		*ref = 0;
		return;
	}

	// TODO!
	warning("BoltEngine::resolveFunction(): TODO!");
	uint16 index = (uint16)*ref;
	*ref = index;
}

void BoltEngine::resolveAllRefs() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (data == nullptr)
		return;

	uint32 count = g_boltCurrentMemberEntry->decompSize / 4;
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
		return g_resolvedPtrs[val & 0x7FFFFFFF];

	return nullptr;
}

bool BoltEngine::openBOLTLib(BOLTLib **libPtr, BOLTCallbacks *callbacks, const char *fileName) {
	Common::File *fileHandle = nullptr;
	int16 groupCount = 0;

	// If already open, bump reference count...
	if (*libPtr) {
		(*libPtr)->refCount++;
		g_boltCurrentLib = *libPtr;
		return true;
	}

	g_boltCurrentLib = nullptr;

	fileHandle = _xp->openFile(fileName, 1);
	if (fileHandle) {
		// Read 16-byte BOLT file header...
		byte headerData[16];

		if (libRead(fileHandle, 0, headerData, 16)) {
			Common::SeekableReadStream *headerDataStream = new Common::MemoryReadStream(headerData, 16, DisposeAfterUse::NO);

			headerDataStream->read(g_boltFileHeader.header, sizeof(g_boltFileHeader.header));
			g_boltFileHeader.groupCount = headerDataStream->readByte();
			g_boltFileHeader.groupDirOffset = headerDataStream->readUint32BE();

			groupCount = g_boltFileHeader.groupCount;
			if (groupCount == 0)
				groupCount = 256;

			delete headerDataStream;

			*libPtr = new BOLTLib(groupCount);

			g_boltCurrentLib = *libPtr;
			if (g_boltCurrentLib) {
				(*libPtr)->refCount = 0;
				(*libPtr)->groupCount = g_boltFileHeader.groupCount;
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

	if (g_boltCurrentLib != nullptr) {
		_xp->freeMem(g_boltCurrentLib);
		g_boltCurrentLib = nullptr;
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
		warning("DEBUG, closeBOLTLib(): CHECK FOR OVERFLOW ");
	}

	// Free all groups in reverse order...
	int16 groupId = groupCount << 8;
	while (groupCount-- > 0) {
		groupId -= (1 << 8);
		freeBOLTGroup(*libPtr, groupId, 0);
	}

	_xp->closeFile((*libPtr)->fileHandle);
	_xp->freeMem(*libPtr);
	*libPtr = nullptr;

	return true;
}

bool BoltEngine::attemptFreeIndex(BOLTLib *lib, int16 groupId) {
	BOLTGroupEntry *groupEntry = &lib->groups[groupId >> 8];

	if (!groupEntry->memberData)
		return true;

	int16 memberCount = g_boltCurrentGroupEntry->memberCount;
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
	int16 memberCount = g_boltCurrentGroupEntry->memberCount;
	if (memberCount == 0)
		memberCount = 256;

	if (g_boltCurrentGroupEntry->memberData) {
		g_boltRawMemberData = g_boltCurrentGroupEntry->memberData;
		return true;
	}

	byte *rawMemberData = (byte *)_xp->allocMem((uint32)memberCount * 16 + 4);
	g_boltRawMemberData = rawMemberData;
	g_boltCurrentGroupEntry->memberData = g_boltRawMemberData;

	if (rawMemberData) {
		if (libRead(g_boltCurrentLib->fileHandle, g_boltCurrentGroupEntry->memberDataOffset, rawMemberData + 4, (uint32)memberCount * 16)) {
			Common::SeekableReadStream *memberEntryStream = new Common::MemoryReadStream(rawMemberData + 4, (uint32)memberCount * 16, DisposeAfterUse::NO);

			for (int16 i = 0; i < memberCount; i++) {
				g_boltCurrentGroupEntry->members[i].flags = memberEntryStream->readByte();
				g_boltCurrentGroupEntry->members[i].preLoadCbIndex = memberEntryStream->readByte();
				g_boltCurrentGroupEntry->members[i].preFreeCbIndex = memberEntryStream->readByte();
				g_boltCurrentGroupEntry->members[i].typeCbIndex = memberEntryStream->readByte();
				g_boltCurrentGroupEntry->members[i].decompSize = memberEntryStream->readUint32BE();
				g_boltCurrentGroupEntry->members[i].fileOffset = memberEntryStream->readUint32BE();
				g_boltCurrentGroupEntry->members[i].dataPtrPlaceholder = memberEntryStream->readUint32BE();
				g_boltCurrentGroupEntry->members[i].dataPtr = nullptr;
			}

			delete memberEntryStream;

			g_boltRawMemberData = g_boltCurrentGroupEntry->memberData;
			return true;
		}
	}

	if (g_boltRawMemberData)
		_xp->freeMem(g_boltRawMemberData);

	g_boltRawMemberData = nullptr;
	g_boltCurrentGroupEntry->memberData = nullptr;
	return false;
}

bool BoltEngine::getBOLTGroup(BOLTLib *lib, int16 groupId, int16 flags) {
	g_boltLoadDepth++;
	int16 resId = groupId & 0xFF00;

	if (lib) {
		g_boltCurrentLib = lib;
		g_boltCurrentGroupEntry = &lib->groups[groupId >> 8];

		int16 memberCount = g_boltCurrentGroupEntry->memberCount;
		if (memberCount == 0)
			memberCount = 256;

		if (loadGroupDirectory()) {
			if (g_boltCurrentGroupEntry->loadCbIndex != 0) {
				lib->callbacks.groupLoadCallbacks[g_boltCurrentGroupEntry->loadCbIndex]();
			}

			for (int16 i = 0; i < memberCount; i++) {
				if (!getBOLTMember(lib, resId)) {
					freeBOLTGroup(lib, resId, 0);
					g_boltLoadDepth--;
					return false;
				}

				resId++;
			}

			resolvePendingFixups();
			g_boltLoadDepth--;
			return true;
		}
	}

	freeBOLTGroup(lib, resId, 0);
	g_boltLoadDepth--;
	return false;
}

void BoltEngine::freeBOLTGroup(BOLTLib *lib, int16 groupId, int16 flags) {
	if (!lib)
		return;

	g_boltCurrentGroupEntry = &lib->groups[groupId >> 8];
	g_boltRawMemberData = g_boltCurrentGroupEntry->memberData;

	if (g_boltCurrentGroupEntry->freeCbIndex != 0) {
		lib->callbacks.groupFreeCallbacks[g_boltCurrentGroupEntry->freeCbIndex]();
	}

	if (!g_boltRawMemberData)
		return;

	int16 memberCount = g_boltCurrentGroupEntry->memberCount;
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
		g_boltCurrentLib = lib;
		g_boltCurrentGroupEntry = &lib->groups[resId >> 8];

		int16 memberCount = g_boltCurrentGroupEntry->memberCount;
		if (memberCount == 0)
			memberCount = 256;

		if (loadGroupDirectory()) {
			g_boltCurrentMemberEntry = &g_boltCurrentGroupEntry->members[memberIndex];

			if (g_boltCurrentMemberEntry->preLoadCbIndex != 0) {
				lib->callbacks.memberLoadCallbacks[g_boltCurrentMemberEntry->preLoadCbIndex]();
			}

			if (g_boltCurrentMemberEntry->dataPtr != nullptr)
				return g_boltCurrentMemberEntry->dataPtr;

			g_boltCurrentMemberEntry->dataPtr = (byte *)_xp->allocMem(g_boltCurrentMemberEntry->decompSize);
			if (g_boltCurrentMemberEntry->dataPtr) {
				// Bit 3 in flags = uncompressed
				if (g_boltCurrentMemberEntry->flags & 0x08) {
					// Read directly into destination
					if (!libRead(lib->fileHandle, g_boltCurrentMemberEntry->fileOffset, g_boltCurrentMemberEntry->dataPtr, g_boltCurrentMemberEntry->decompSize)) {
						if (g_boltCurrentMemberEntry->dataPtr != nullptr) {
							_xp->freeMem(g_boltCurrentMemberEntry->dataPtr);
							g_boltCurrentMemberEntry->dataPtr = nullptr;
						}

						return nullptr;
					}
				} else {
					// Compressed member
					if (memberIndex + 1 < memberCount) {
						BOLTMemberEntry *nextEntry = &g_boltCurrentGroupEntry->members[memberIndex + 1];
						compressedSize = nextEntry->fileOffset;
					} else {
						// Last entry: total = memberDirSize + memberDirOffset + memberDataOffset
						warning("is this even working?");
						compressedSize = (uint32)(memberCount * 16) + g_boltCurrentGroupEntry->memberDataOffset + g_boltCurrentGroupEntry->memberDirOffset;
					}

					compressedSize -= g_boltCurrentMemberEntry->fileOffset;

					tempBuf = (byte *)_xp->allocMem(compressedSize);
					if (!tempBuf) {
						if (g_boltCurrentMemberEntry->dataPtr != nullptr) {
							_xp->freeMem(g_boltCurrentMemberEntry->dataPtr);
							g_boltCurrentMemberEntry->dataPtr = nullptr;
						}

						return nullptr;
					}

					if (!libRead(lib->fileHandle, g_boltCurrentMemberEntry->fileOffset, tempBuf, compressedSize)) {
						_xp->freeMem(tempBuf);

						if (g_boltCurrentMemberEntry->dataPtr != nullptr) {
							_xp->freeMem(g_boltCurrentMemberEntry->dataPtr);
							g_boltCurrentMemberEntry->dataPtr = nullptr;
						}

						return nullptr;
					}

					decompress(g_boltCurrentMemberEntry->dataPtr, g_boltCurrentMemberEntry->decompSize, tempBuf);

					_xp->freeMem(tempBuf);
				}

				lib->callbacks.typeLoadCallbacks[g_boltCurrentMemberEntry->typeCbIndex]();

				if (g_boltLoadDepth == 0)
					resolvePendingFixups();

				return g_boltCurrentMemberEntry->dataPtr;
			}
		}
	}

	if (g_boltCurrentMemberEntry->dataPtr != nullptr) {
		_xp->freeMem(g_boltCurrentMemberEntry->dataPtr);
		g_boltCurrentMemberEntry->dataPtr = nullptr;
	}

	return nullptr;
}

bool BoltEngine::freeBOLTMember(BOLTLib *lib, int16 resId) {
	if (!lib)
		return true;

	g_boltCurrentGroupEntry = &lib->groups[resId >> 8];
	g_boltRawMemberData = g_boltCurrentGroupEntry->memberData;

	if (g_boltRawMemberData == nullptr)
		return true;

	g_boltCurrentMemberEntry = &g_boltCurrentGroupEntry->members[(resId & 0xFF)];

	if (g_boltCurrentMemberEntry->preFreeCbIndex != 0) {
		lib->callbacks.memberFreeCallbacks[g_boltCurrentMemberEntry->preFreeCbIndex]();
	}

	if (g_boltCurrentMemberEntry->dataPtr == nullptr)
		return true;

	lib->callbacks.typeFreeCallbacks[g_boltCurrentMemberEntry->typeCbIndex]();

	_xp->freeMem(g_boltCurrentMemberEntry->dataPtr);
	g_boltCurrentMemberEntry->dataPtr = nullptr;

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
	g_resourceIndex = (uint32 **)_xp->allocMem(g_resourceIndexCount * sizeof(uintptr));
	if (!g_resourceIndex)
		return false;

	return true;
}

void BoltEngine::freeResourceIndex() {
	if (g_resourceIndex) {
		_xp->freeMem(g_resourceIndex);
		g_resourceIndex = nullptr;
	}
}

void BoltEngine::swapAllWords() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (!data)
		return;

	int16 count = int16(g_boltCurrentMemberEntry->decompSize / 2);
	for (int16 i = 0; i < count; i++) {
		WRITE_UINT16(data, READ_BE_INT16(data));
		data += 2;
	}
}

void BoltEngine::swapAllLongs() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (!data)
		return;

	int16 count = (int16)(g_boltCurrentMemberEntry->decompSize / 4);
	for (int16 i = 0; i < count; i++) {
		WRITE_UINT32(data, READ_BE_UINT32(data));
		data += 4;
	}
}

BOLTCallback BoltEngine::g_defaultTypeLoadCallbacks[25];
BOLTCallback BoltEngine::g_defaultTypeFreeCallbacks[25];
BOLTCallback BoltEngine::g_defaultMemberLoadCallbacks[25];
BOLTCallback BoltEngine::g_defaultMemberFreeCallbacks[25];
BOLTCallback BoltEngine::g_defaultGroupLoadCallbacks[25];
BOLTCallback BoltEngine::g_defaultGroupFreeCallbacks[25];

BOLTCallback BoltEngine::g_fredTypeLoadCallbacks[28];
BOLTCallback BoltEngine::g_fredTypeFreeCallbacks[28];

BOLTCallback BoltEngine::g_georgeTypeLoadCallbacks[28];
BOLTCallback BoltEngine::g_georgeTypeFreeCallbacks[28];

BOLTCallback BoltEngine::g_scoobyTypeLoadCallbacks[28];
BOLTCallback BoltEngine::g_scoobyTypeFreeCallbacks[28];

BOLTCallback BoltEngine::g_topCatTypeLoadCallbacks[26];
BOLTCallback BoltEngine::g_topCatTypeFreeCallbacks[26];

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

void BoltEngine::swapScoobyHelpEntryCb() { ((BoltEngine *)g_engine)->swapScoobyHelpEntry(); }
void BoltEngine::swapScoobyWordArrayCb() { ((BoltEngine *)g_engine)->swapScoobyWordArray(); }

void BoltEngine::swapTopCatHelpEntryCb() { ((BoltEngine *)g_engine)->swapTopCatHelpEntry(); }

void BoltEngine::swapGeorgeFrameArrayCb() { ((BoltEngine *)g_engine)->swapGeorgeFrameArray(); }
void BoltEngine::swapGeorgeHelpEntryCb() { ((BoltEngine *)g_engine)->swapGeorgeHelpEntry(); }
void BoltEngine::swapGeorgeThresholdsCb() { ((BoltEngine *)g_engine)->swapGeorgeThresholds(); }

void BoltEngine::initCallbacks() {
	// --- BOOTHS ---
	for (int i = 0; i < ARRAYSIZE(g_defaultTypeLoadCallbacks); i++) {
		g_defaultTypeLoadCallbacks[i] = noOpCb;
	}

	g_defaultTypeLoadCallbacks[2] = swapAllWordsCb;
	g_defaultTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	g_defaultTypeLoadCallbacks[10] = swapPicHeaderCb;
	g_defaultTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	g_defaultTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	g_defaultTypeLoadCallbacks[14] = swapFirstFourWordsCb;

	for (int i = 0; i < ARRAYSIZE(g_defaultTypeFreeCallbacks); i++) {
		g_defaultTypeFreeCallbacks[i] = noOpCb;
	}

	g_defaultTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	for (int i = 0; i < ARRAYSIZE(g_defaultMemberLoadCallbacks); i++) {
		g_defaultMemberLoadCallbacks[i] = noOpCb;
	}

	for (int i = 0; i < ARRAYSIZE(g_defaultMemberFreeCallbacks); i++) {
		g_defaultMemberFreeCallbacks[i] = noOpCb;
	}

	for (int i = 0; i < ARRAYSIZE(g_defaultGroupLoadCallbacks); i++) {
		g_defaultGroupLoadCallbacks[i] = noOpCb;
	}

	for (int i = 0; i < ARRAYSIZE(g_defaultGroupFreeCallbacks); i++) {
		g_defaultGroupFreeCallbacks[i] = noOpCb;
	}

	g_boothsBoltCallbacks.typeLoadCallbacks = g_defaultTypeLoadCallbacks;
	g_boothsBoltCallbacks.typeFreeCallbacks = g_defaultTypeFreeCallbacks;
	g_boothsBoltCallbacks.memberLoadCallbacks = g_defaultMemberLoadCallbacks;
	g_boothsBoltCallbacks.memberFreeCallbacks = g_defaultMemberFreeCallbacks;
	g_boothsBoltCallbacks.groupLoadCallbacks = g_defaultGroupLoadCallbacks;
	g_boothsBoltCallbacks.groupFreeCallbacks = g_defaultGroupFreeCallbacks;

	// --- FRED ---
	for (int i = 0; i < ARRAYSIZE(g_fredTypeLoadCallbacks); i++) {
		g_fredTypeLoadCallbacks[i] = noOpCb;
	}

	g_fredTypeLoadCallbacks[2] = swapAllWordsCb;
	g_fredTypeLoadCallbacks[6] = resolveAllRefsCb;
	g_fredTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	g_fredTypeLoadCallbacks[10] = swapPicHeaderCb;
	g_fredTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	g_fredTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	g_fredTypeLoadCallbacks[14] = swapFirstFourWordsCb;
	g_fredTypeLoadCallbacks[25] = swapFredAnimEntryCb;
	g_fredTypeLoadCallbacks[26] = swapFredAnimDescCb;
	g_fredTypeLoadCallbacks[27] = swapFredLevelDescCb;

	for (int i = 0; i < ARRAYSIZE(g_fredTypeFreeCallbacks); i++) {
		g_fredTypeFreeCallbacks[i] = noOpCb;
	}

	g_fredTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	g_fredBoltCallbacks.typeLoadCallbacks = g_fredTypeLoadCallbacks;
	g_fredBoltCallbacks.typeFreeCallbacks = g_fredTypeFreeCallbacks;
	g_fredBoltCallbacks.memberLoadCallbacks = g_defaultMemberLoadCallbacks;
	g_fredBoltCallbacks.memberFreeCallbacks = g_defaultMemberFreeCallbacks;
	g_fredBoltCallbacks.groupLoadCallbacks = g_defaultGroupLoadCallbacks;
	g_fredBoltCallbacks.groupFreeCallbacks = g_defaultGroupFreeCallbacks;

	// --- GEORGE ---
	for (int i = 0; i < ARRAYSIZE(g_georgeTypeLoadCallbacks); i++) {
		g_georgeTypeLoadCallbacks[i] = noOpCb;
	}

	g_georgeTypeLoadCallbacks[2] = swapAllWordsCb;
	g_georgeTypeLoadCallbacks[6] = resolveAllRefsCb;
	g_georgeTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	g_georgeTypeLoadCallbacks[10] = swapPicHeaderCb;
	g_georgeTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	g_georgeTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	g_georgeTypeLoadCallbacks[14] = swapFirstFourWordsCb;
	g_georgeTypeLoadCallbacks[25] = swapGeorgeFrameArrayCb;
	g_georgeTypeLoadCallbacks[26] = swapGeorgeHelpEntryCb;
	g_georgeTypeLoadCallbacks[27] = swapGeorgeThresholdsCb;

	for (int i = 0; i < ARRAYSIZE(g_georgeTypeFreeCallbacks); i++) {
		g_georgeTypeFreeCallbacks[i] = noOpCb;
	}

	g_georgeTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	g_georgeBoltCallbacks.typeLoadCallbacks = g_georgeTypeLoadCallbacks;
	g_georgeBoltCallbacks.typeFreeCallbacks = g_georgeTypeFreeCallbacks;
	g_georgeBoltCallbacks.memberLoadCallbacks = g_defaultMemberLoadCallbacks;
	g_georgeBoltCallbacks.memberFreeCallbacks = g_defaultMemberFreeCallbacks;
	g_georgeBoltCallbacks.groupLoadCallbacks = g_defaultGroupLoadCallbacks;
	g_georgeBoltCallbacks.groupFreeCallbacks = g_defaultGroupFreeCallbacks;

	// --- SCOOBY ---
	for (int i = 0; i < ARRAYSIZE(g_scoobyTypeLoadCallbacks); i++) {
		g_scoobyTypeLoadCallbacks[i] = noOpCb;
	}

	g_scoobyTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	g_scoobyTypeLoadCallbacks[10] = swapPicHeaderCb;
	g_scoobyTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	g_scoobyTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	g_scoobyTypeLoadCallbacks[26] = swapScoobyHelpEntryCb;
	g_scoobyTypeLoadCallbacks[27] = swapScoobyWordArrayCb;

	for (int i = 0; i < ARRAYSIZE(g_scoobyTypeFreeCallbacks); i++) {
		g_scoobyTypeFreeCallbacks[i] = noOpCb;
	}

	g_scoobyTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	g_scoobyBoltCallbacks.typeLoadCallbacks = g_scoobyTypeLoadCallbacks;
	g_scoobyBoltCallbacks.typeFreeCallbacks = g_scoobyTypeFreeCallbacks;
	g_scoobyBoltCallbacks.memberLoadCallbacks = g_defaultMemberLoadCallbacks;
	g_scoobyBoltCallbacks.memberFreeCallbacks = g_defaultMemberFreeCallbacks;
	g_scoobyBoltCallbacks.groupLoadCallbacks = g_defaultGroupLoadCallbacks;
	g_scoobyBoltCallbacks.groupFreeCallbacks = g_defaultGroupFreeCallbacks;

	// --- TOPCAT ---
	for (int i = 0; i < ARRAYSIZE(g_topCatTypeLoadCallbacks); i++) {
		g_topCatTypeLoadCallbacks[i] = noOpCb;
	}

	g_topCatTypeLoadCallbacks[2] = swapAllWordsCb;
	g_topCatTypeLoadCallbacks[4] = swapAllLongsCb;
	g_topCatTypeLoadCallbacks[6] = resolveAllRefsCb;
	g_topCatTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	g_topCatTypeLoadCallbacks[10] = swapPicHeaderCb;
	g_topCatTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	g_topCatTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	g_topCatTypeLoadCallbacks[25] = swapTopCatHelpEntryCb;

	for (int i = 0; i < ARRAYSIZE(g_topCatTypeFreeCallbacks); i++) {
		g_topCatTypeFreeCallbacks[i] = noOpCb;
	}

	g_topCatTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	g_topCatBoltCallbacks.typeLoadCallbacks = g_topCatTypeLoadCallbacks;
	g_topCatBoltCallbacks.typeFreeCallbacks = g_topCatTypeFreeCallbacks;
	g_topCatBoltCallbacks.memberLoadCallbacks = g_defaultMemberLoadCallbacks;
	g_topCatBoltCallbacks.memberFreeCallbacks = g_defaultMemberFreeCallbacks;
	g_topCatBoltCallbacks.groupLoadCallbacks = g_defaultGroupLoadCallbacks;
	g_topCatBoltCallbacks.groupFreeCallbacks = g_defaultGroupFreeCallbacks;
}

} // End of namespace Bolt
