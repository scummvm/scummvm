/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileio.h>
#include <malloc.h>
#include <ucl/ucl.h>
#include <libmc.h>
#include "backends/ps2/savefile.h"
#include "backends/ps2/Gs2dScreen.h"
#include "backends/ps2/systemps2.h"
#include "common/scummsys.h"

extern void *_gp;

#define UCL_MAGIC 0x314C4355

#define PORT 0
#define SLOT 0
// port 0, slot 0: memory card in first slot.

void sioprintf(const char *zFormat, ...);

class McAccess {
public:
	McAccess(int port, int slot);
	~McAccess(void);
	int open(const char *name, int mode);
	int close(int fd);
	int size(int fd);
	int read(int fd, void *buf, int size);
	int write(int fd, const void *buf, int size);
	int mkDir(const char *name);
	int getDir(const char *name, unsigned int mode, int max, void *dest);    
	int getInfo(int *type, int *free, int *format);
	int remove(const char *name);
private:
	int _sema;
	int _port, _slot;
};

McAccess::McAccess(int port, int slot) {
	_port = port;
	_slot = slot;
	ee_sema_t newSema;
	newSema.init_count = 1;
	newSema.max_count = 1;
	_sema = CreateSema(&newSema);

	assert(mcInit(MC_TYPE_MC) >= 0);
}

McAccess::~McAccess(void) {
	DeleteSema(_sema);
}

int McAccess::open(const char *name, int mode) {
	int res;
	WaitSema(_sema);
	mcOpen(_port, _slot, name, mode);
	mcSync(0, NULL, &res);
	SignalSema(_sema);
	return res;
}

int McAccess::close(int fd) {
	int res;
	WaitSema(_sema);
	mcClose(fd);
	mcSync(0, NULL, &res);
	SignalSema(_sema);
	return res;
}

int McAccess::size(int fd) {
	int res, size;
	WaitSema(_sema);
	mcSeek(fd, 0, SEEK_END);
	mcSync(0, NULL, &size);
	mcSeek(fd, 0, SEEK_SET);
	mcSync(0, NULL, &res);
	SignalSema(_sema);
	assert(res == 0);
	return size;
}

int McAccess::read(int fd, void *buf, int size) {
	int res;
	WaitSema(_sema);
	mcRead(fd, buf, size);
	mcSync(0, NULL, &res);
	SignalSema(_sema);
	return res;
}

int McAccess::write(int fd, const void *buf, int size) {
	int res;
	WaitSema(_sema);
	mcWrite(fd, buf, size);
	mcSync(0, NULL, &res);
	SignalSema(_sema);
	return res;
}

int McAccess::mkDir(const char *name) {
	int res;
	WaitSema(_sema);
	mcMkDir(_port, _slot, name);
	mcSync(0, NULL, &res);
	SignalSema(_sema);
	return res;
}

int McAccess::remove(const char *name) {
	int res;
	WaitSema(_sema);
	mcDelete(_port, _slot, name);
	mcSync(0, NULL, &res);
	SignalSema(_sema);
	return res;
}

int McAccess::getDir(const char *name, unsigned int mode, int max, void *dest) {
	int res;
	WaitSema(_sema);
	mcGetDir(_port, _slot, name, mode, max, (mcTable*)dest);
	mcSync(0, NULL, &res);
	SignalSema(_sema);
	return res;
}

int McAccess::getInfo(int *type, int *free, int *format) {
	int res;
	WaitSema(_sema);
	mcGetInfo(_port, _slot, type, free, format);
	mcSync(0, NULL, &res);
	SignalSema(_sema);
	return res;
}

class UclOutSaveFile : public Common::OutSaveFile {
public:
	UclOutSaveFile(const char *filename, OSystem_PS2 *system, Gs2dScreen *screen, McAccess *mc);
	virtual ~UclOutSaveFile(void);
	virtual uint32 write(const void *ptr, uint32 size);
	virtual void flush(void);
	virtual bool ioFailed(void) const;
	virtual void clearIOFailed(void);
private:
	OSystem_PS2 *_system;
	Gs2dScreen *_screen;
	McAccess *_mc;
	int _fd;
	uint8 *_buf;
	uint32 _bufSize, _bufPos;
	bool _ioFailed, _wasFlushed;
	char _fileName[128];
};

class UclInSaveFile : public Common::InSaveFile {
public:
	UclInSaveFile(const char *filename, Gs2dScreen *screen, McAccess *mc);
	virtual ~UclInSaveFile(void);
	virtual bool eos(void) const;
	virtual uint32 read(void *ptr, uint32 size);
	virtual bool ioFailed(void) const;
	virtual void clearIOFailed(void);
	virtual void skip(uint32 offset);

	virtual uint32 pos(void) const;
	virtual uint32 size(void) const;
	virtual void seek(int pos, int whence = SEEK_SET);
private:
	Gs2dScreen *_screen;
	McAccess *_mc;
	uint8 *_buf;
	uint32 _bufSize, _bufPos;
	bool _ioFailed;
};

class AutoSaveFile : public Common::OutSaveFile {
public:
	AutoSaveFile(Ps2SaveFileManager *saveMan, const char *filename);
	~AutoSaveFile(void);
	virtual uint32 write(const void *ptr, uint32 size);
	virtual void flush(void) { };
	virtual bool ioFailed(void) { return false; };
	virtual void clearIOFailed(void) {};
private:
	Ps2SaveFileManager *_saveMan;
	char _fileName[256];
	uint8 *_buf;
	uint32 _bufSize, _bufPos;
};

AutoSaveFile::AutoSaveFile(Ps2SaveFileManager *saveMan, const char *filename) {
	strcpy(_fileName, filename);
	_saveMan = saveMan;
	_bufSize = 65536;
	_buf = (uint8*)memalign(64, _bufSize);
	_bufPos = 0;
}

AutoSaveFile::~AutoSaveFile(void) {
	_saveMan->writeSaveNonblocking(_fileName, _buf, _bufPos);
	free(_buf);
}

uint32 AutoSaveFile::write(const void *ptr, uint32 size) {
	uint32 bytesFree = _bufSize - _bufPos;
	if (bytesFree < size) {
		uint32 allocBytes = (size > 32 * 1024) ? size : 32 * 1024;
		_bufSize += allocBytes;
		_buf = (uint8*)realloc(_buf, _bufSize);
		bytesFree = _bufSize - _bufPos;
	}
	memcpy(_buf + _bufPos, ptr, size);
	_bufPos += size;
	return size;
}

#define MAX_MC_ENTRIES	16

void runSaveThread(Ps2SaveFileManager *param);

Ps2SaveFileManager::Ps2SaveFileManager(OSystem_PS2 *system, Gs2dScreen *screen) {
	_system = system;
	_screen = screen;
	_mc = new McAccess(0, 0);

	_mcDirList = (mcTable*)memalign(64, MAX_MC_ENTRIES * sizeof(mcTable));
	_mcDirName[0] = '\0';
	_mcCheckTime = 0;
	_mcNeedsUpdate = true;

	for (int mcCheckCount = 0; mcCheckCount < 3; mcCheckCount++) {
		/* retry mcGetInfo 3 times. It slows down startup without mc considerably,
		   but cheap 3rd party memory cards apparently fail to get detected once in a while */

		int mcType, mcFree, mcFormat;
		int res = _mc->getInfo(&mcType, &mcFree, &mcFormat);

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

	// create save thread
	ee_sema_t newSema;
	newSema.init_count = 0;
	newSema.max_count = 1;
	_autoSaveSignal = CreateSema(&newSema);
	_autoSaveBuf = NULL;
	_autoSaveSize = 0;
	_systemQuit = false;

	ee_thread_t saveThread, thisThread;
	ReferThreadStatus(GetThreadId(), &thisThread);

	saveThread.initial_priority = thisThread.current_priority + 1;
	saveThread.stack_size = 8 * 1024;
	_autoSaveStack = malloc(saveThread.stack_size);	
	saveThread.stack  = _autoSaveStack;
	saveThread.func   = (void *)runSaveThread;
	saveThread.gp_reg = &_gp;

	_autoSaveTid = CreateThread(&saveThread);
	assert(_autoSaveTid >= 0);
	StartThread(_autoSaveTid, this);
}

Ps2SaveFileManager::~Ps2SaveFileManager(void) {
}

void Ps2SaveFileManager::checkMainDirectory(void) {
	// verify that the main directory (scummvm config + icon) exists
	int ret, fd;
	_mcNeedsUpdate = true;
	ret = _mc->getDir("/ScummVM/*", 0, MAX_MC_ENTRIES, _mcDirList);
	printf("/ScummVM/* res = %d\n", ret);
	if (ret <= 0) { // assume directory doesn't exist
		printf("Dir doesn't exist\n");
		ret = _mc->mkDir("/ScummVM");
		if (ret >= 0) {
			fd = _mc->open("/ScummVM/scummvm.icn", O_WRONLY | O_CREAT);
			if (fd >= 0) {
				uint16 icoSize;
				uint16 *icoBuf = decompressIconData(&icoSize);
				ret = _mc->write(fd, icoBuf, icoSize * 2);
				_mc->close(fd);
				free(icoBuf);

				printf(".icn written\n");
				setupIcon("/ScummVM/icon.sys", "scummvm.icn", "ScummVM", "Configuration");
			} else
				printf("Can't create icon file: %d\n", fd);
		} else
			printf("can't create scummvm directory: %d\n", ret);
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
	if (_mcNeedsUpdate || ((_system->getMillis() - _mcCheckTime) > 2000) || !_mcPresent) {
		// check if memory card was exchanged/removed in the meantime
		int mcType, mcFree, mcFormat, mcResult;
		mcResult = _mc->getInfo(&mcType, &mcFree, &mcFormat);
		if (mcResult != 0) { // memory card was exchanged
			_mcNeedsUpdate = true;
			if (mcResult == -1) { // yes, it was exchanged
				checkMainDirectory(); // make sure ScummVM dir and icon are there
			} else { // no memorycard in slot or not formatted or something like that
				_mcPresent = false;
				printf("MC not found, error code %d\n", mcResult);
				return false;
			}
		}
		_mcPresent = true;
		_mcCheckTime = _system->getMillis();
	}
	if (_mcNeedsUpdate || strcmp(_mcDirName, dir)) {
		strcpy(_mcDirName, dir);
		char dirStr[256];
		sprintf(dirStr, "/ScummVM-%s/*", dir);
		_mcEntries = _mc->getDir(dirStr, 0, MAX_MC_ENTRIES, _mcDirList);
		_mcNeedsUpdate = false;
	}
	return (_mcEntries >= 0);
}

Common::InSaveFile *Ps2SaveFileManager::openForLoading(const char *filename) {
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
			sprintf(fullName, "/ScummVM-%s/%s", dir, name);
			UclInSaveFile *file = new UclInSaveFile(fullName, _screen, _mc);
			if (file) {
				if (!file->ioFailed())
					return file;
				else
					delete file;
			}
		} else
			printf("file %s (%s) doesn't exist\n", filename, name);
	}
	_screen->wantAnim(false);
	return NULL;
}

Common::OutSaveFile *Ps2SaveFileManager::openForSaving(const char *filename) {
	int res;
	char dir[256], name[256];

	_screen->wantAnim(true);
	splitPath(filename, dir, name);

	if (!mcReadyForDir(dir)) {
		if (_mcPresent) { // directory doesn't seem to exist yet
			char fullPath[256];
			sprintf(fullPath, "/ScummVM-%s", dir);
			res = _mc->mkDir(fullPath);

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
		sprintf(fullPath, "/ScummVM-%s/%s", dir, name);
		if (strstr(filename, ".s00") || strstr(filename, ".ASD") || strstr(filename, ".asd")) {
			// this is an autosave
			AutoSaveFile *file = new AutoSaveFile(this, fullPath);
			return file;
		} else {
			UclOutSaveFile *file = new UclOutSaveFile(fullPath, _system, _screen, _mc);
			if (!file->ioFailed()) {
				// we're creating a file, mc will have to be updated next time
				_mcNeedsUpdate = true;
				return file;
			} else
				delete file;
		}
	}

	_screen->wantAnim(false);
	return NULL;
}

void Ps2SaveFileManager::listSavefiles(const char *prefix, bool *marks, int num) {
	_screen->wantAnim(true);

	int mcType, mcFree, mcFormat, mcResult;
	mcResult = _mc->getInfo(&mcType, &mcFree, &mcFormat);

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

		int numEntries = _mc->getDir(mcSearchStr, 0, MAX_MC_ENTRIES, mcEntries);

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

	int fd, res;
	fd = _mc->open(dest, O_WRONLY | O_CREAT);
	if (fd >= 0) {
		res = _mc->write(fd, &icon_sys, sizeof(icon_sys));
		_mc->close(fd);
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

void runSaveThread(Ps2SaveFileManager *param) {
	param->saveThread();
}

void Ps2SaveFileManager::writeSaveNonblocking(char *name, void *buf, uint32 size) {
	if (buf && size && !_systemQuit) {
		strcpy(_autoSaveName, name);
		assert(!_autoSaveBuf);
		_autoSaveBuf = (uint8*)malloc(size);
		memcpy(_autoSaveBuf, buf, size);
		_autoSaveSize = size;
		SignalSema(_autoSaveSignal);
	}
}

void Ps2SaveFileManager::saveThread(void) {
	while (!_systemQuit) {
		WaitSema(_autoSaveSignal);
		if (_autoSaveBuf && _autoSaveSize) {
			UclOutSaveFile *outSave = new UclOutSaveFile(_autoSaveName, _system, _screen, _mc);
			if (!outSave->ioFailed()) {
				outSave->write(_autoSaveBuf, _autoSaveSize);
				outSave->flush();
			}
			if (outSave->ioFailed())
				_system->msgPrintf(5000, "Writing autosave to %s failed", _autoSaveName);
			delete outSave;
			free(_autoSaveBuf);
			_autoSaveBuf = NULL;
			_autoSaveSize = 0;
			_mcNeedsUpdate = true; // we've created a file, mc will have to be updated
			_screen->wantAnim(false);
		}
	}
	ExitThread();
}

void Ps2SaveFileManager::quit(void) {
	_systemQuit = true;
	ee_thread_t statSave, statThis;
	ReferThreadStatus(GetThreadId(), &statThis);
	ChangeThreadPriority(_autoSaveTid, statThis.current_priority - 1);

	do {	// wait until thread called ExitThread()
		SignalSema(_autoSaveSignal);
		ReferThreadStatus(_autoSaveTid, &statSave);
	} while (statSave.status != 0x10);

	DeleteThread(_autoSaveTid);
    free(_autoSaveStack);
}

UclInSaveFile::UclInSaveFile(const char *filename, Gs2dScreen *screen, McAccess *mc) {
	_screen = screen;
	_mc = mc;
	int fd = _mc->open(filename, O_RDONLY);
	_buf = NULL;
	_bufSize = _bufPos = 0;
	_ioFailed = false;

	if (fd >= 0) {
		int srcSize = _mc->size(fd);
		if (srcSize > 8) {
			int res;
			uint8 *tmpBuf = (uint8*)memalign(64, srcSize);
			res = _mc->read(fd, tmpBuf, srcSize);
			if ((res == srcSize) && (*(uint32*)tmpBuf == UCL_MAGIC)) {
				uint32 resLen = _bufSize = *(uint32*)(tmpBuf + 4);
				_buf = (uint8*)malloc(_bufSize + 2048);
				res = ucl_nrv2e_decompress_8(tmpBuf + 8, srcSize - 8, _buf, &resLen, NULL);
				if ((res < 0) || (resLen != _bufSize)) {
					printf("Unable to decompress file %s (%d -> %d) error code %d\n", filename, srcSize, _bufSize, res);
					free(_buf);
					_buf = NULL;
					_bufSize = 0;
				}
			}
			free(tmpBuf);
		}
		_mc->close(fd);
	}
	if (!_buf) {
		printf("Invalid savegame %s\n", filename);
		_ioFailed = true;
	}
}

UclInSaveFile::~UclInSaveFile(void) {
	if (_buf)
		free(_buf);
	_screen->wantAnim(false);
}

bool UclInSaveFile::ioFailed(void) const {
	return _ioFailed;
}

void UclInSaveFile::clearIOFailed(void) {
	_ioFailed = false;
}

bool UclInSaveFile::eos(void) const {
	return _bufPos == _bufSize;
}

uint32 UclInSaveFile::pos(void) const {
	return _bufPos;
}

uint32 UclInSaveFile::size(void) const {
	return _bufSize;
}

void UclInSaveFile::seek(int pos, int whence) {
	int destPos;
	switch (whence) {
		case SEEK_SET:
			destPos = pos;
			break;
		case SEEK_CUR:
			destPos = _bufPos + pos;
			break;
		case SEEK_END:
			destPos = _bufSize + pos;
			break;
		default:
			return;
	}
	if ((destPos >= 0) && (destPos <= (int)_bufSize))
		_bufPos = (uint32)destPos;
}

uint32 UclInSaveFile::read(void *ptr, uint32 size) {
	if (_buf) {
		uint32 bytesRemain = _bufSize - _bufPos;
		if (size > bytesRemain) {
			size = bytesRemain;
			_ioFailed = true;
		}
		memcpy(ptr, _buf + _bufPos, size);
		_bufPos += size;
		return size;
	} else {
		_ioFailed = true;
		return 0;
	}
}

void UclInSaveFile::skip(uint32 offset) {
	if (_buf) {
		if (_bufPos + offset <= _bufSize)
			_bufPos += offset;
		else
			_bufPos = _bufSize;
	}
}

UclOutSaveFile::UclOutSaveFile(const char *filename, OSystem_PS2 *system, Gs2dScreen *screen, McAccess *mc) {
	_screen = screen;
	_system = system;
	_mc = mc;
	_bufPos = 0;
	_fd = _mc->open(filename, O_WRONLY | O_CREAT);
	if (_fd >= 0) {
		_bufSize = 65536;
		_buf = (uint8*)malloc(_bufSize);
		_ioFailed = false;
		strcpy(_fileName, filename);
	} else {
		_ioFailed = true;
		_bufSize = 0;
		_buf = NULL;
	}
	_wasFlushed = false;
}

UclOutSaveFile::~UclOutSaveFile(void) {
	if (_buf) {
		if (_bufPos) {
			printf("Engine didn't call SaveFile::flush()\n");
			flush();
			if (ioFailed()) {
				// unable to save to memory card and it's too late to return an error code to the engine
				_system->msgPrintf(5000, "!WARNING!\nCan't write to memory card.\nGame was NOT saved.");
				printf("~UclOutSaveFile: Flush failed!\n");
			}
		}
		free(_buf);
	}
	if (_fd >= 0)
		_mc->close(_fd);
	_screen->wantAnim(false);
}

bool UclOutSaveFile::ioFailed(void) const {
	return _ioFailed;
}

void UclOutSaveFile::clearIOFailed(void) {
	_ioFailed = false;
}

void UclOutSaveFile::flush(void) {
	int res;

	if (_bufPos) {
		if (_wasFlushed) {
			// the engine flushed this file and afterwards wrote more data.
			// this is unsupported because it results in savefiles that consist
			// of two or more compressed segments.
			printf("Error: 2nd call to UclOutSaveFile::flush!\n");
			res = -1;
		} else {
			uint32 compSize = _bufPos * 2;
			uint8 *compBuf = (uint8*)memalign(64, compSize + 8);
			*(uint32*)(compBuf + 0) = UCL_MAGIC;
			*(uint32*)(compBuf + 4) = _bufPos; // uncompressed size
			res = ucl_nrv2e_99_compress(_buf, _bufPos, compBuf + 8, &compSize, NULL, 10, NULL, NULL);
			if (res >= 0) {
				res = _mc->write(_fd, compBuf, compSize + 8);
				if (res != (int)compSize + 8) {
					printf("flush: write failed, %d != %d\n", res, compSize + 8);
					res = -1;
				}
			} else
				printf("Unable to compress %d bytes of savedata, errorcode %d\n", _bufPos, res);
			free(compBuf);
			_bufPos = 0;
		}

		if (res < 0) {
			_ioFailed = true;
			printf("UclOutSaveFile::flush failed!\n");
			if (_fd >= 0) {
				// the file is broken; delete it
				_mc->close(_fd);
				res = _mc->remove(_fileName);
				if (res == 0)
					printf("File %s: remove ok\n", _fileName);
				else
					printf("File %s: remove error %d\n", _fileName, res);
				_fd = -1;
			}
		}
	}
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


