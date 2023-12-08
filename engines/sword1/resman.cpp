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


#include "common/debug.h"
#include "common/textconsole.h"

#include "sword1/memman.h"
#include "sword1/resman.h"
#include "sword1/sword1.h"
#include "sword1/swordres.h"

#include "gui/message.h"

namespace Sword1 {
void guiFatalError(char *msg) {
	// Displays a dialog on-screen before terminating the engine.
	// TODO: We really need to setup a special palette for cases when
	// the engine is erroring before setting one... otherwise invisible cursor :)

	GUI::MessageDialog dialog(msg);
	dialog.runModal();
	error("%s", msg);
}

#define MAX_PATH_LEN 260

ResMan::ResMan(const char *fileName, bool isMacFile, bool isKorean) {
	_openCluStart = _openCluEnd = nullptr;
	_openClus = 0;
	_isBigEndian = isMacFile;
	_isKorTrs = isKorean;
	_memMan = new MemMan();
	loadCluDescript(fileName);
}

ResMan::~ResMan() {
#if 0
	for (uint32 clusCnt = 0; clusCnt < _prj.noClu; clusCnt++) {
		Clu *cluster = _prj.clu[clusCnt];
		if (cluster) {
			for (uint32 grpCnt = 0; grpCnt < cluster->noGrp; grpCnt++) {
				Grp *group = cluster->grp[grpCnt];
				if (group) {
					for (uint32 resCnt = 0; resCnt < group->noRes; resCnt++) {
						if (group->resHandle[resCnt].cond == MEM_DONT_FREE) {
							warning("ResMan::~ResMan: Resource %02X.%04X.%02X is still open",
							        clusCnt + 1, grpCnt, resCnt);
						}
					}
				}
			}
		}
	}
	debug(0, "ResMan closed\n");
#endif
	flush();
	freeCluDescript();
	delete _memMan;
}

void ResMan::loadCluDescript(const char *fileName) {
	// The cluster description file is always little endian (even on the mac version, whose cluster files are big endian)
	Common::File file;
	file.open(fileName);

	if (!file.isOpen()) {
		char msg[512];
		Common::sprintf_s(msg, "Couldn't open CLU description '%s'\n\nIf you are running from CD, please ensure you have read the ScummVM documentation regarding multi-cd games.", fileName);
		guiFatalError(msg);
	}


	_prj.noClu = file.readUint32LE();
	_prj.clu = new Clu[_prj.noClu]();

	uint32 *cluIndex = (uint32 *)malloc(_prj.noClu * 4);
	file.read(cluIndex, _prj.noClu * 4);

	for (uint32 clusCnt = 0; clusCnt < _prj.noClu; clusCnt++)
		if (cluIndex[clusCnt]) {
			Clu *cluster = _prj.clu + clusCnt;
			file.read(cluster->label, MAX_LABEL_SIZE);

			cluster->file = nullptr;
			cluster->noGrp = file.readUint32LE();
			cluster->grp = new Grp[cluster->noGrp];
			cluster->nextOpen = nullptr;
			memset(cluster->grp, 0, cluster->noGrp * sizeof(Grp));
			cluster->refCount = 0;

			uint32 *grpIndex = (uint32 *)malloc(cluster->noGrp * 4);
			file.read(grpIndex, cluster->noGrp * 4);

			for (uint32 grpCnt = 0; grpCnt < cluster->noGrp; grpCnt++)
				if (grpIndex[grpCnt]) {
					Grp *group = cluster->grp + grpCnt;
					group->noRes = file.readUint32LE();
					group->resHandle = new MemHandle[group->noRes];
					group->offset = new uint32[group->noRes];
					group->length = new uint32[group->noRes];
					uint32 *resIdIdx = (uint32 *)malloc(group->noRes * 4);
					file.read(resIdIdx, group->noRes * 4);

					for (uint32 resCnt = 0; resCnt < group->noRes; resCnt++) {
						if (resIdIdx[resCnt]) {
							group->offset[resCnt] = file.readUint32LE();
							group->length[resCnt] = file.readUint32LE();
							_memMan->initHandle(group->resHandle + resCnt);
						} else {
							group->offset[resCnt] = 0xFFFFFFFF;
							group->length[resCnt] = 0;
							_memMan->initHandle(group->resHandle + resCnt);
						}
					}
					free(resIdIdx);
				}
			free(grpIndex);
		}
	free(cluIndex);

	if (_prj.clu[3].grp[5].noRes == 29)
		for (uint8 cnt = 0; cnt < 29; cnt++)
			_srIdList[cnt] = 0x04050000 | cnt;

	if (_isKorTrs) {
		Common::File cluFile;
		cluFile.open("korean.clu");
		if (cluFile.isOpen()) {
			for (uint32 resCnt = 0, resOffset = 0; resCnt < _prj.clu[2].grp[0].noRes; resCnt++) {
				Header header;
				cluFile.read(&header, sizeof(Header));
				_prj.clu[2].grp[0].offset[resCnt] = resOffset;
				_prj.clu[2].grp[0].length[resCnt] = header.comp_length;
				resOffset += header.comp_length;
				cluFile.seek(header.decomp_length, SEEK_CUR);
			}
			Common::strcpy_s(_prj.clu[2].label, "korean");
		} else {
			_isKorTrs = false;
		}
	}
}

void ResMan::freeCluDescript() {

	for (uint32 clusCnt = 0; clusCnt < _prj.noClu; clusCnt++) {
		Clu *cluster = _prj.clu + clusCnt;
		for (uint32 grpCnt = 0; grpCnt < cluster->noGrp; grpCnt++) {
			Grp *group = cluster->grp + grpCnt;
			if (group->resHandle != nullptr) {
				for (uint32 resCnt = 0; resCnt < group->noRes; resCnt++)
					_memMan->freeNow(group->resHandle + resCnt);

				delete[] group->resHandle;
				delete[] group->offset;
				delete[] group->length;
			}
		}
		delete[] cluster->grp;
		delete cluster->file;
	}
	delete[] _prj.clu;
}

void ResMan::flush() {
	Common::StackLock lock(_resourceAccessMutex);
	for (uint32 clusCnt = 0; clusCnt < _prj.noClu; clusCnt++) {
		Clu *cluster = _prj.clu + clusCnt;
		for (uint32 grpCnt = 0; grpCnt < cluster->noGrp; grpCnt++) {
			Grp *group = cluster->grp + grpCnt;
			for (uint32 resCnt = 0; resCnt < group->noRes; resCnt++)
				if (group->resHandle[resCnt].cond != MEM_FREED) {
					_memMan->setCondition(group->resHandle + resCnt, MEM_CAN_FREE);
					group->resHandle[resCnt].refCount = 0;
				}
		}
		if (cluster->file) {
			cluster->file->close();
			delete cluster->file;
			cluster->file = nullptr;
			cluster->refCount = 0;
		}
	}
	_openClus = 0;
	_openCluStart = _openCluEnd = nullptr;
	// the memory manager cached the blocks we asked it to free, so explicitly make it free them
	_memMan->flush();
}

void *ResMan::fetchRes(uint32 id) {
	MemHandle *memHandle = resHandle(id);

	if (!memHandle) {
		warning("fetchRes:: resource %d out of bounds", id);
		return nullptr;
	}
	if (!memHandle->data)
		error("fetchRes:: resource %d is not open", id);
	return memHandle->data;
}

void *ResMan::openFetchRes(uint32 id) {
	resOpen(id);
	return fetchRes(id);
}

void ResMan::dumpRes(uint32 id) {
	char outn[30];
	Common::sprintf_s(outn, "DUMP%08X.BIN", id);
	Common::DumpFile outf;
	if (outf.open(outn)) {
		resOpen(id);
		MemHandle *memHandle = resHandle(id);
		if (memHandle) {
			outf.write(memHandle->data, memHandle->size);
			outf.close();
		}
		resClose(id);
	}
}

Header *ResMan::lockScript(uint32 scrID) {
	if (!_scriptList[scrID / ITM_PER_SEC])
		error("Script id %d not found", scrID);
	scrID = _scriptList[scrID / ITM_PER_SEC];
#ifdef SCUMM_BIG_ENDIAN
	openScriptResourceBigEndian(scrID);
#else
	openScriptResourceLittleEndian(scrID);
#endif
	MemHandle *handle = resHandle(scrID);
	if (!handle)
		error("Script resource handle %d not found", scrID);
	return (Header *)handle->data;
}

void ResMan::unlockScript(uint32 scrID) {
	resClose(_scriptList[scrID / ITM_PER_SEC]);
}

void *ResMan::cptResOpen(uint32 id) {
#ifdef SCUMM_BIG_ENDIAN
	openCptResourceBigEndian(id);
#else
	openCptResourceLittleEndian(id);
#endif
	MemHandle *handle = resHandle(id);
	return handle != nullptr ? handle->data : nullptr;
}

void ResMan::resOpen(uint32 id) {  // load resource ID into memory
	Common::StackLock lock(_resourceAccessMutex);
	MemHandle *memHandle = resHandle(id);

	if (!memHandle)
		return;
	if (memHandle->cond == MEM_FREED) { // memory has been freed
		if (id == GAME_FONT && _isKorTrs) {
			// Load Korean Font
			uint32 size = resLength(id);
			uint32 korFontSize = 0;
			Common::File korFontFile;

			korFontFile.open("bs1k.fnt");
			if (korFontFile.isOpen()) {
				korFontSize = korFontFile.size();
			}
			_memMan->alloc(memHandle, size + korFontSize);
			Common::File *clusFile = resFile(id);
			assert(clusFile);
			clusFile->seek(resOffset(id));
			clusFile->read(memHandle->data, size);
			if (clusFile->err() || clusFile->eos()) {
				error("Can't read %d bytes from offset %d from cluster file %s\nResource ID: %d (%08X)", size, resOffset(id), _prj.clu[(id >> 24) - 1].label, id, id);
			}

			if (korFontSize > 0) {
				korFontFile.read((uint8 *)memHandle->data + size, korFontSize);
			}
		} else {
			uint32 size = resLength(id);
			_memMan->alloc(memHandle, size);
			Common::File *clusFile = resFile(id);
			assert(clusFile);
			clusFile->seek(resOffset(id));
			clusFile->read(memHandle->data, size);
			if (clusFile->err() || clusFile->eos()) {
				error("Can't read %d bytes from offset %d from cluster file %s\nResource ID: %d (%08X)", size, resOffset(id), _prj.clu[(id >> 24) - 1].label, id, id);
			}
		}
	} else
		_memMan->setCondition(memHandle, MEM_DONT_FREE);

	memHandle->refCount++;
	if (memHandle->refCount > 20) {
		debug(1, "%d references to id %d. Guess there's something wrong.", memHandle->refCount, id);
	}
}

void ResMan::resClose(uint32 id) {
	Common::StackLock lock(_resourceAccessMutex);
	MemHandle *handle = resHandle(id);
	if (!handle)
		return;
	if (!handle->refCount) {
		warning("Resource Manager fail: unlocking object with refCount 0. Id: %d", id);
	} else {
		handle->refCount--;
		if (!handle->refCount)
			_memMan->setCondition(handle, MEM_CAN_FREE);
	}
}

FrameHeader *ResMan::fetchFrame(void *resourceData, uint32 frameNo) {
	uint8 *frameFile = (uint8 *)resourceData;
	uint8 *idxData = frameFile + sizeof(Header);
	if (_isBigEndian) {
		if (frameNo >= READ_BE_UINT32(idxData))
			error("fetchFrame:: frame %d doesn't exist in resource.", frameNo);
		frameFile += READ_BE_UINT32(idxData + (frameNo + 1) * 4);
	} else {
		if (frameNo >= READ_LE_UINT32(idxData))
			error("fetchFrame:: frame %d doesn't exist in resource.", frameNo);
		frameFile += READ_LE_UINT32(idxData + (frameNo + 1) * 4);
	}
	return (FrameHeader *)frameFile;
}

Common::File *ResMan::resFile(uint32 id) {
	Clu *cluster = _prj.clu + ((id >> 24) - 1);
	if (cluster->file == nullptr) {
		_openClus++;
		if (_openCluEnd == nullptr) {
			_openCluStart = _openCluEnd = cluster;
		} else {
			_openCluEnd->nextOpen = cluster;
			_openCluEnd = cluster;
		}
		cluster->file = new Common::File();
		char fileName[36];
		// Supposes that big endian means mac cluster file and little endian means PC cluster file.
		// This works, but we may want to separate the file name from the endianess or try .CLM extension if opening.clu file fail.
		if (_isBigEndian)
			Common::sprintf_s(fileName, "%s.CLM", _prj.clu[(id >> 24) - 1].label);
		else
			Common::sprintf_s(fileName, "%s.CLU", _prj.clu[(id >> 24) - 1].label);
		cluster->file->open(fileName);
		if (!cluster->file->isOpen()) {
			char msg[512];
			Common::sprintf_s(msg, "Couldn't open game cluster file '%s'\n\nIf you are running from CD, please ensure you have read the ScummVM documentation regarding multi-cd games.", fileName);
			guiFatalError(msg);
		}
		while (_openClus > MAX_OPEN_CLUS) {
			assert(_openCluStart);
			Clu *closeClu = _openCluStart;
			_openCluStart = _openCluStart->nextOpen;

			if (closeClu->file)
				closeClu->file->close();
			delete closeClu->file;
			closeClu->file = nullptr;
			closeClu->nextOpen = nullptr;

			_openClus--;
		}
	}
	return cluster->file;
}

MemHandle *ResMan::resHandle(uint32 id) {
	if ((id >> 16) == 0x0405)
		id = _srIdList[id & 0xFFFF];
	uint8 cluster = (uint8)((id >> 24) - 1);
	uint8 group = (uint8)(id >> 16);

	// There is a known case of reading beyond array boundaries when trying to use
	// portuguese subtitles (cluster file 2, group 6) with a version that does not
	// contain subtitles for this languages (i.e. has only 6 languages and not 7).
	if (cluster >= _prj.noClu || group >= _prj.clu[cluster].noGrp)
		return nullptr;

	return &(_prj.clu[cluster].grp[group].resHandle[id & 0xFFFF]);
}

uint32 ResMan::resLength(uint32 id) {
	if ((id >> 16) == 0x0405)
		id = _srIdList[id & 0xFFFF];
	uint8 cluster = (uint8)((id >> 24) - 1);
	uint8 group = (uint8)(id >> 16);

	if (cluster >= _prj.noClu || group >= _prj.clu[cluster].noGrp)
		return 0;

	return _prj.clu[cluster].grp[group].length[id & 0xFFFF];
}

uint32 ResMan::resOffset(uint32 id) {
	if ((id >> 16) == 0x0405)
		id = _srIdList[id & 0xFFFF];
	uint8 cluster = (uint8)((id >> 24) - 1);
	uint8 group = (uint8)(id >> 16);

	if (cluster >= _prj.noClu || group >= _prj.clu[cluster].noGrp)
		return 0;

	return _prj.clu[cluster].grp[group].offset[id & 0xFFFF];
}

void ResMan::openCptResourceBigEndian(uint32 id) {
	bool needByteSwap = false;
	if (!_isBigEndian) {
		// Cluster files are in little endian fomat.
		// If the resource are not in memory anymore, and therefore will be read
		// from disk, they will need to be byte swaped.
		MemHandle *memHandle = resHandle(id);
		if (memHandle)
			needByteSwap = (memHandle->cond == MEM_FREED);
	}
	resOpen(id);
	if (needByteSwap) {
		MemHandle *handle = resHandle(id);
		if (!handle)
			return;
		uint32 totSize = handle->size;
		uint32 *data = (uint32 *)((uint8 *)handle->data + sizeof(Header));
		totSize -= sizeof(Header);
		if (totSize & 3)
			error("Illegal compact size for id %d: %d", id, totSize);
		totSize /= 4;
		for (uint32 cnt = 0; cnt < totSize; cnt++) {
			*data = READ_LE_UINT32(data);
			data++;
		}
	}
}

void ResMan::openCptResourceLittleEndian(uint32 id) {
	bool needByteSwap = false;
	if (_isBigEndian) {
		// Cluster files are in big endian fomat.
		// If the resource are not in memory anymore, and therefore will be read
		// from disk, they will need to be byte swaped.
		MemHandle *memHandle = resHandle(id);
		if (memHandle)
			needByteSwap = (memHandle->cond == MEM_FREED);
	}
	resOpen(id);
	if (needByteSwap) {
		MemHandle *handle = resHandle(id);
		if (!handle)
			return;
		uint32 totSize = handle->size;
		uint32 *data = (uint32 *)((uint8 *)handle->data + sizeof(Header));
		totSize -= sizeof(Header);
		if (totSize & 3)
			error("Illegal compact size for id %d: %d", id, totSize);
		totSize /= 4;
		for (uint32 cnt = 0; cnt < totSize; cnt++) {
			*data = READ_BE_UINT32(data);
			data++;
		}
	}
}

void ResMan::openScriptResourceBigEndian(uint32 id) {
	bool needByteSwap = false;
	if (!_isBigEndian) {
		// Cluster files are in little endian fomat.
		// If the resource are not in memory anymore, and therefore will be read
		// from disk, they will need to be byte swaped.
		MemHandle *memHandle = resHandle(id);
		if (memHandle)
			needByteSwap = (memHandle->cond == MEM_FREED);
	}
	resOpen(id);
	if (needByteSwap) {
		MemHandle *handle = resHandle(id);
		if (!handle)
			return;
		// uint32 totSize = handle->size;
		Header *head = (Header *)handle->data;
		head->comp_length = FROM_LE_32(head->comp_length);
		head->decomp_length = FROM_LE_32(head->decomp_length);
		head->version = FROM_LE_16(head->version);
		uint32 *data = (uint32 *)((uint8 *)handle->data + sizeof(Header));
		uint32 size = handle->size - sizeof(Header);
		if (size & 3)
			error("Odd size during script endian conversion. Resource ID =%d, size = %d", id, size);
		size >>= 2;
		for (uint32 cnt = 0; cnt < size; cnt++) {
			*data = READ_LE_UINT32(data);
			data++;
		}
	}
}

void ResMan::openScriptResourceLittleEndian(uint32 id) {
	bool needByteSwap = false;
	if (_isBigEndian) {
		// Cluster files are in big endian fomat.
		// If the resource are not in memory anymore, and therefore will be read
		// from disk, they will need to be byte swaped.
		MemHandle *memHandle = resHandle(id);
		if (memHandle)
			needByteSwap = (memHandle->cond == MEM_FREED);
	}
	resOpen(id);
	if (needByteSwap) {
		MemHandle *handle = resHandle(id);
		if (!handle)
			return;
		// uint32 totSize = handle->size;
		Header *head = (Header *)handle->data;
		head->comp_length = FROM_BE_32(head->comp_length);
		head->decomp_length = FROM_BE_32(head->decomp_length);
		head->version = FROM_BE_16(head->version);
		uint32 *data = (uint32 *)((uint8 *)handle->data + sizeof(Header));
		uint32 size = handle->size - sizeof(Header);
		if (size & 3)
			error("Odd size during script endian conversion. Resource ID =%d, size = %d", id, size);
		size >>= 2;
		for (uint32 cnt = 0; cnt < size; cnt++) {
			*data = READ_BE_UINT32(data);
			data++;
		}
	}
}

uint32 ResMan::getDeathFontId() {
	// At some point in the releases (as evidenced by the disasms of all
	// known executables, and by the source code in our possession), Revolution
	// changed the resource offsets for some files. I have tried EVERYTHING to
	// try and discern which file we are dealing with and spectacularly failed.
	// The only choice which seems to work correctly is to check the file size,
	// as the newer GENERAL.CLU file is bigger. Sorry.
	if (SwordEngine::isPsx())
		return SR_FONT;

	Common::File fp;
	if (fp.open(SwordEngine::isMac() ? "GENERAL.CLM" : "GENERAL.CLU")) {
		fp.seek(0, SEEK_END);
		int64 fileSize = fp.pos();

		if (SwordEngine::_systemVars.realLanguage == Common::RU_RUS) {
			switch (fileSize) {
			case 6081261: // Akella
				return SR_DEATHFONT;
			case 6354790: // Mediahauz
				return SR_FONT;
			case 6350630: // Novy Disk
				return SR_DEATHFONT_ALT;
			default:
				warning("ResMan::getDeathFontId(): Unrecognized version of russian GENERAL.CLU, size %d", (int)fileSize);
				break;
			}

			return SR_FONT;
		} else if (fileSize < 6295679) {
			return SR_DEATHFONT;
		} else {
			return SR_DEATHFONT_ALT;
		}
	}

	return 0;
}

} // End of namespace Sword1
