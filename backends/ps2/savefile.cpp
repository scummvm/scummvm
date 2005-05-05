/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileio.h>
#include <malloc.h>
#include <ucl/ucl.h>
#include "backends/ps2/savefile.h"
#include "backends/ps2/Gs2dScreen.h"
#include "backends/ps2/asyncfio.h"
#include "backends/ps2/systemps2.h"
#include "scummsys.h"

extern AsyncFio fio;

class UclOutSaveFile : public Common::WriteStream {
public:
	UclOutSaveFile(const char *filename, Gs2dScreen *screen);
	virtual ~UclOutSaveFile(void);
	virtual uint32 write(const void *ptr, uint32 size);
	virtual int flush(void);
	virtual bool ioFailed(void);
	virtual void clearIOFailed(void);
private:
	Gs2dScreen *_screen;
	int _fd;
	uint8 *_buf;
	uint32 _bufSize, _bufPos;
	bool _ioFailed;
};

class UclInSaveFile : public Common::ReadStream {
public:
	UclInSaveFile(const char *filename, Gs2dScreen *screen);
	virtual ~UclInSaveFile(void);
	virtual bool eos(void) const;
	virtual uint32 read(void *ptr, uint32 size);
	virtual bool ioFailed(void);
	virtual void clearIOFailed(void);	
private:
	Gs2dScreen *_screen;
	uint8 *_buf;
	uint32 _bufSize, _bufPos;
	bool _ioFailed;
};

#define MAX_MC_ENTRIES	16

Ps2SaveFileManager::Ps2SaveFileManager(OSystem_PS2 *system, Gs2dScreen *screen) {
	_system = system;
	_screen = screen;
	assert(mcInit(MC_TYPE_MC) >= 0);

	_mcDirList = (mcTable*)memalign(64, MAX_MC_ENTRIES * sizeof(mcTable));
	_mcDirName[0] = '\0';
	_mcCheckTime = 0;
	_mcNeedsUpdate = true;

	int mcCheckCount;
	int res = -10;
	for (int mcCheckCount = 0; mcCheckCount < 3; mcCheckCount++) {
		/* retry mcGetInfo 3 times. It slows down startup without mc considerably,
		   but cheap 3rd party memory cards apparently fail to get detected once in a while */

		int mcType, mcFree, mcFormat, res;
		mcGetInfo(0, 0, &mcType, &mcFree, &mcFormat);
		mcSync(0, NULL, &res);
		if ((res == 0) || (res == -1)) { // mc okay
			_mcPresent = true;
			printf("MC okay, result = %d. Type %d, Free %d, Format %d\n", res, mcType, mcFree, mcFormat);
			checkMainDirectory();
			break;
		} else {
			_mcPresent = false;
			printf("MC failed, not present or not formatted, code %d\n", res);
		}
	}
}

Ps2SaveFileManager::~Ps2SaveFileManager(void) {
}

void Ps2SaveFileManager::checkMainDirectory(void) {
	// verify that the main directory (scummvm config + icon) exists
	int ret;
	mcGetDir(0, 0, "/ScummVM/*", 0, MAX_MC_ENTRIES, _mcDirList);
	mcSync(0, NULL, &ret);
	printf("/ScummVM/* res = %d\n", ret);
	if (ret <= 0) { // assume directory doesn't exist
		printf("Dir doesn't exist\n");
		fio.mkdir("mc0:ScummVM");
		int fd = fio.open("mc0:ScummVM/scummvm.icn", O_WRONLY | O_CREAT | O_TRUNC);
		if (fd >= 0) {
			uint16 icoSize;
			uint16 *icoBuf = decompressIconData(&icoSize);
			fio.write(fd, icoBuf, icoSize * 2);
			fio.sync(fd);
			free(icoBuf);
			fio.close(fd);
			printf(".icn written\n");
			setupIcon("mc0:ScummVM/icon.sys", "scummvm.icn", "ScummVM", "Configuration");
		} else
			printf("unable to write icon data\n");
	}
}

void Ps2SaveFileManager::splitPath(const char *fileName, char *dir, char *name) {
	strcpy(dir, fileName);
	char *ext = strchr(dir, '.');
	if (ext) {
		*ext = '\0';
		ext++;
	}
	if (ext && *ext)
		sprintf(name, "%s.ucl", ext);
	else
		strcpy(name, "save.ucl");
}

bool Ps2SaveFileManager::mcReadyForDir(const char *dir) {
	if (_mcNeedsUpdate || ((_system->getMillis() - _mcCheckTime) > 1000) || !_mcPresent) {
		// check if memory card was exchanged/removed in the meantime
		int mcType, mcFree, mcFormat, mcResult;
		mcGetInfo(0, 0, &mcType, &mcFree, &mcFormat);
		mcSync(0, NULL, &mcResult);
		if (mcResult != 0) { // memory card was exchanged
			_mcNeedsUpdate = true;
			if (mcResult != -1) {
				_mcPresent = false;
				printf("MC not found, error code %d\n", mcResult);
				return false;
			}
		}
		_mcPresent = true;
	}
	if (_mcNeedsUpdate || strcmp(_mcDirName, dir)) {
		strcpy(_mcDirName, dir);
		char dirStr[256];
		sprintf(dirStr, "/ScummVM-%s/*", dir);
		mcGetDir(0, 0, dirStr, 0, MAX_MC_ENTRIES, _mcDirList);
		mcSync(0, NULL, &_mcEntries);
		return (_mcEntries >= 0);
	} else
		return true;
}

InSaveFile *Ps2SaveFileManager::openForLoading(const char *filename) {
	_screen->wantAnim(true);

	char dir[256], name[256];
	splitPath(filename, dir, name);
	if (mcReadyForDir(dir)) {
		bool fileExists = false;
		for (int i = 0; i < _mcEntries; i++)
			if (strcmp(name, (char*)_mcDirList[i].name) == 0)
				fileExists = true;
		if (fileExists) {
			char fullName[256];
			sprintf(fullName, "mc0:ScummVM-%s/%s", dir, name);
			UclInSaveFile *file = new UclInSaveFile(fullName, _screen);
			if (file) {
				if (!file->ioFailed()) {
					return (InSaveFile*)file;
				} else
					delete file;
			}
		} else
			printf("file %s (%s) doesn't exist\n", filename, name);
	}
	_screen->wantAnim(false);
	return NULL;
}

OutSaveFile *Ps2SaveFileManager::openForSaving(const char *filename) {
	_screen->wantAnim(true);
	char dir[256], name[256];
	splitPath(filename, dir, name);

	if (!mcReadyForDir(dir)) {
		if (_mcPresent) { // directory doesn't seem to exist yet
			char fullPath[256];
			sprintf(fullPath, "mc0:ScummVM-%s", dir);
			fio.mkdir(fullPath);

			char icoSysDest[256], saveDesc[256];
			sprintf(icoSysDest, "%s/icon.sys", fullPath);
			strcpy(saveDesc, dir);
			if ((saveDesc[0] >= 'a') && (saveDesc[0] <= 'z'))
				saveDesc[0] += 'A' - 'a';
			setupIcon(icoSysDest, "../ScummVM/scummvm.icn", saveDesc, "Savegames");
		}
	}

	if (_mcPresent) {
		char fullPath[256];
		sprintf(fullPath, "mc0:ScummVM-%s/%s", dir, name);
		UclOutSaveFile *file = new UclOutSaveFile(fullPath, _screen);
		if (!file->ioFailed()) {
			// we're creating a file, mc will have to be updated next time
			_mcNeedsUpdate = true;
			return (OutSaveFile*)file;
		} else
			delete file;
	}

	_screen->wantAnim(false);
	return NULL;
}

void Ps2SaveFileManager::listSavefiles(const char *prefix, bool *marks, int num) {
	_screen->wantAnim(true);

	int mcType, mcFree, mcFormat, mcResult;
	mcGetInfo(0, 0, &mcType, &mcFree, &mcFormat);
	mcSync(0, NULL, &mcResult);
	
	memset(marks, false, num * sizeof(bool));

	if ((mcResult == 0) || (mcResult == -1)) {
		// there's a memory card in the slot.
		if (mcResult == -1)
			_mcNeedsUpdate = true;

		mcTable *mcEntries = (mcTable*)memalign(64, sizeof(mcTable) * MAX_MC_ENTRIES);

		char dirStr[256], ext[256], mcSearchStr[256];
		strcpy(dirStr, prefix);
		char *pos = strchr(dirStr, '.');
		if (pos) {
			strcpy(ext, pos + 1);
			*pos = '\0';
		} else
			ext[0] = '\0';
		sprintf(mcSearchStr, "/ScummVM-%s/%s*", dirStr, ext);

		int numEntries;
		mcGetDir(0, 0, mcSearchStr, 0, MAX_MC_ENTRIES, mcEntries);
		mcSync(0, NULL, &numEntries);
		
		int searchLen = strlen(ext);
		for (int i = 0; i < numEntries; i++)
			if ((((char*)mcEntries[i].name)[0] != '.') && stricmp((char*)mcEntries[i].name, "icon.sys")) {
				char *stopCh;
				int destNum = (int)strtoul((char*)mcEntries[i].name + searchLen, &stopCh, 10);
				if ((!stopCh) || strcmp(stopCh, ".ucl"))
					printf("unexpected end %s in name %s, search %s\n", stopCh, (char*)mcEntries[i].name, prefix);
				if (destNum < num)
					marks[destNum] = true;
			}
		free(mcEntries);
	}
	_screen->wantAnim(false);
}

const char *Ps2SaveFileManager::getSavePath(void) const {
	return "mc0:";
}

bool Ps2SaveFileManager::setupIcon(const char *dest, const char *ico, const char *descr1, const char *descr2) {
	mcIcon icon_sys;
	memset(&icon_sys, 0, sizeof(mcIcon));
	memcpy(icon_sys.head, "PS2D", 4);
	char title[256];
	if (!stricmp("SAVEGAME", descr1)) { // these are broken sword 1 savegames
		sprintf(title, "BSword1\n%s", descr2);
		icon_sys.nlOffset = 8;
	} else {
		sprintf(title, "%s\n%s", descr1, descr2);
		icon_sys.nlOffset = strlen(descr1) + 1;
	}
	strcpy_sjis((short*)&(icon_sys.title), title);
	icon_sys.trans = 0x10;
	memcpy(icon_sys.bgCol, _bgcolor, sizeof(_bgcolor));
	memcpy(icon_sys.lightDir, _lightdir, sizeof(_lightdir));
	memcpy(icon_sys.lightCol, _lightcol, sizeof(_lightcol));
	memcpy(icon_sys.lightAmbient, _ambient, sizeof(_ambient));
	strcpy((char*)icon_sys.view, ico);
	strcpy((char*)icon_sys.copy, ico);
	strcpy((char*)icon_sys.del, ico);

	int fd = fio.open(dest, O_WRONLY | O_CREAT | O_TRUNC);
	if (fd >= 0) {
		fio.write(fd, &icon_sys, sizeof(icon_sys));
		int res = fio.sync(fd);
		fio.close(fd);
		return (res == sizeof(icon_sys));
	} else
		return false;
}

uint16 *Ps2SaveFileManager::decompressIconData(uint16 *size) {
	uint16 inPos = 1;
	uint16 *rleData = (uint16*)_rleIcoData;
	uint16 resSize = rleData[0];
	uint16 *resData = (uint16*)malloc(resSize * sizeof(uint16));
	uint16 outPos = 0;
	while (outPos < resSize) {
		uint16 len = rleData[inPos++];
		while (len--)
			resData[outPos++] = 0x7FFF;
		len = rleData[inPos++];
		while (len--)
			resData[outPos++] = rleData[inPos++];
	}
	*size = resSize;
	assert(outPos == resSize);
	return resData;
}

UclInSaveFile::UclInSaveFile(const char *filename, Gs2dScreen *screen) {
	_screen = screen;
	int fd = fio.open(filename, O_RDONLY);
	_buf = NULL;
	_bufSize = _bufPos = 0;
	_ioFailed = false;

	if (fd >= 0) {
		int srcSize = fio.seek(fd, 0, SEEK_END);
		fio.seek(fd, 0, SEEK_SET);
		if (srcSize > 4) {
			int res;
			uint8 *tmpBuf = (uint8*)malloc(srcSize);
			fio.read(fd, tmpBuf, srcSize);
			res = fio.sync(fd);
			if (res == srcSize) {
				uint32 resLen = _bufSize = *(uint32*)tmpBuf;
				_buf = (uint8*)malloc(_bufSize + 2048);
				res = ucl_nrv2e_decompress_8(tmpBuf + 4, srcSize - 4, _buf, &resLen, NULL);
				if ((res < 0) || (resLen != _bufSize)) {
					printf("Unable to decompress file %s (%d -> %d) error code %d\n", filename, srcSize, _bufSize, res);
					free(_buf);
					_buf = NULL;
					_bufSize = 0;
				}
			}
			free(tmpBuf);
		}
		if (!_buf) {
			printf("Invalid savegame %s\n", filename);
			_ioFailed = true;
		}
		fio.close(fd);        
	}
}

UclInSaveFile::~UclInSaveFile(void) {
	if (_buf)
		free(_buf);
	_screen->wantAnim(false);
}

bool UclInSaveFile::ioFailed(void) {
	return _ioFailed;
}

void UclInSaveFile::clearIOFailed(void) {
	_ioFailed = false;
}

bool UclInSaveFile::eos(void) const {
	return _bufPos == _bufSize;
}

uint32 UclInSaveFile::read(void *ptr, uint32 size) {
	uint32 bytesRemain = _bufSize - _bufPos;
	if (size > bytesRemain) {
		size = bytesRemain;
		_ioFailed = true;
	}
	memcpy(ptr, _buf + _bufPos, size);
	_bufPos += size;
	return size;
}

UclOutSaveFile::UclOutSaveFile(const char *filename, Gs2dScreen *screen) {
	_screen = screen;
	_bufPos = 0;
	_fd = fio.open(filename, O_WRONLY | O_CREAT | O_TRUNC);
	if (_fd >= 0) {
		_bufSize = 65536;
		_buf = (uint8*)malloc(_bufSize);
		_ioFailed = false;
	} else {
		_ioFailed = true;
		_bufSize = 0;
		_buf = NULL;
	}
}

UclOutSaveFile::~UclOutSaveFile(void) {
	if (_buf) {
		if (flush() < 0)
			printf("~UclOutSaveFile: Flush failed!\n");
		free(_buf);
	}
	if (_fd >= 0)
		fio.close(_fd);
	_screen->wantAnim(false);
}

bool UclOutSaveFile::ioFailed(void) {
	return _ioFailed;
}

void UclOutSaveFile::clearIOFailed(void) {
	_ioFailed = false;
}

int UclOutSaveFile::flush(void) {
	if (_bufPos == 0)
		return 0; // no data to flush
	if (_buf) {
		uint8 *compBuf = (uint8*)malloc(_bufPos * 2);
		uint32 compSize = _bufPos * 2;
		int res = ucl_nrv2e_99_compress(_buf, _bufPos, compBuf, &compSize, NULL, 10, NULL, NULL);
		if (res >= 0) {
			fio.write(_fd, &_bufPos, 4);
			if (fio.sync(_fd) == 4) {
				fio.write(_fd, compBuf, compSize);
				if (fio.sync(_fd) != compSize)
					res = -1;
			} else
				res = -1;
		} else
			printf("Unable to compress %d bytes of savedata, errorcode %d\n", _bufPos, res);
		free(compBuf);

		if (res >= 0) {
			_bufPos = 0;
			return 0;
		}
	}
	_ioFailed = true;
	printf("UclOutSaveFile::flush failed!\n");
	return -1;
}


uint32 UclOutSaveFile::write(const void *ptr, uint32 size) {
	assert(_bufPos <= _bufSize);
	uint32 bytesFree = _bufSize - _bufPos;
	if (bytesFree < size) {
		uint32 allocBytes = (size > 32 * 1024) ? size : 32 * 1024;
		_bufSize += allocBytes;
		_buf = (uint8*)realloc(_buf, _bufSize);
		bytesFree = _bufSize - _bufPos;
	}
	assert(bytesFree >= size);
	memcpy(_buf + _bufPos, ptr, size);
	_bufPos += size;
	return size;
}


