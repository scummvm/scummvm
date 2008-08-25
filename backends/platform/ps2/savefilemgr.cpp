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

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileio.h>
#include <malloc.h>
#include <ucl/ucl.h>
#include <libmc.h>
#include "backends/platform/ps2/savefile.h"
#include "backends/platform/ps2/Gs2dScreen.h"
#include "backends/platform/ps2/systemps2.h"
#include "backends/fs/abstract-fs.h"
#include "backends/platform/ps2/ps2debug.h"

#include "common/scummsys.h"

#include "backends/platform/ps2/savefilemgr.h"
#include "backends/platform/ps2/savefile.h"

extern void *_gp;

#define PORT 0
#define SLOT 0
// port 0, slot 0: memory card in first slot.

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

#define MAX_MC_ENTRIES	16

void runSaveThread(Ps2SaveFileManager *param);

Ps2SaveFileManager::Ps2SaveFileManager(OSystem_PS2 *system, Gs2dScreen *screen) {
	_system = system;
	_screen = screen;
	_mc = new McAccess(PORT, SLOT);

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
			dbg_printf("MC okay, result = %d. Type %d, Free %d, Format %d\n", res, mcType, mcFree, mcFormat);
			checkMainDirectory();
			break;
		} else {
			_mcPresent = false;
			dbg_printf("MC failed, not present or not formatted, code %d\n", res);
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
	dbg_printf("/ScummVM/* res = %d\n", ret);
	if (ret <= 0) { // assume directory doesn't exist
		dbg_printf("Dir doesn't exist\n");
		ret = _mc->mkDir("/ScummVM");
		if (ret >= 0) {
			fd = _mc->open("/ScummVM/scummvm.icn", O_WRONLY | O_CREAT);
			if (fd >= 0) {
				uint16 icoSize;
				uint16 *icoBuf = decompressIconData(&icoSize);
				ret = _mc->write(fd, icoBuf, icoSize * 2);
				_mc->close(fd);
				free(icoBuf);

				dbg_printf(".icn written\n");
				setupIcon("/ScummVM/icon.sys", "scummvm.icn", "ScummVM", "Configuration");
			} else
				dbg_printf("Can't create icon file: %d\n", fd);
		} else
			dbg_printf("can't create scummvm directory: %d\n", ret);
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
				dbg_printf("MC not found, error code %d\n", mcResult);
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
	dbg_printf("openForLoading: \"%s\" => \"%s\" + \"%s\"\n", filename, dir, name);
	if (mcReadyForDir(dir)) {
		dbg_printf("Ready\n");
		bool fileExists = false;
		for (int i = 0; i < _mcEntries; i++)
			if (strcmp(name, (char*)_mcDirList[i].name) == 0)
				fileExists = true;
		if (fileExists) {
			dbg_printf("Found!\n");
			char fullName[256];
			sprintf(fullName, "/ScummVM-%s/%s", dir, name);
			UclInSaveFile *file = new UclInSaveFile(fullName, _screen, _mc);
			if (file) {
				if (!file->ioFailed())
					return file;
				else {
					dbg_printf("IoFailed\n");
					delete file;
				}
			}
		} else
			dbg_printf("file %s (%s) doesn't exist\n", filename, name);
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
			} else {
				dbg_printf("UCL out create failed!\n");
				delete file;
			}
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
					dbg_printf("unexpected end %s in name %s, search %s\n", stopCh, (char*)mcEntries[i].name, prefix);
				if (destNum < num)
					marks[destNum] = true;
			}
		free(mcEntries);
	}
	_screen->wantAnim(false);
}

Common::StringList Ps2SaveFileManager::listSavefiles(const char *regex) {
	_screen->wantAnim(true);
	Common::StringList results;
	int mcType, mcFree, mcFormat, mcResult;

	dbg_printf("listSavefiles -> regex=%s\n", regex);

	mcResult = _mc->getInfo(&mcType, &mcFree, &mcFormat);

	if ((mcResult == 0) || (mcResult == -1)) {
		// there's a memory card in the slot.
		if (mcResult == -1)
			_mcNeedsUpdate = true;

		mcTable *mcEntries = (mcTable*)memalign(64, sizeof(mcTable) * MAX_MC_ENTRIES);
    
		char temp[64], key[64], mcSearchStr[64], *dir, *ext;
		strcpy(temp, regex);
		dir = strdup(strtok(temp, "."));
		ext = strdup(strtok(NULL, "."));

		if (strcmp(ext, "???") == 0) {
			free(ext);
			ext = strdup("*"); // workaround for kyra in ScummVM > 0.11.1
			                   // legitimte in PS2 cause there are only
			                   // saved games inside "dir" so "*" will
		    	               // always give us what we are looking for ;-)
		}

		dbg_printf("dir = %s - ext = %s\n", dir, ext);

		sprintf(mcSearchStr, "/ScummVM-%s/%s", dir, ext);
		sprintf(key, "%s.ucl", ext);

		dbg_printf("path = %s - key = %s\n", mcSearchStr, key);

		int numEntries = _mc->getDir(mcSearchStr, 0, MAX_MC_ENTRIES, mcEntries);
		char *name;
        for (int i = 0; i < numEntries; i++) {
			name = (char*)mcEntries[i].name;

            if ((name[0] != '.') && stricmp(name, "icon.sys")) {
				dbg_printf(" name = %s\n", (char*)mcEntries[i].name);
				if (Common::matchString(name, key)) {
					sprintf(temp, "%s.%c%c%c", dir, name[0], name[1], name[2]);
					results.push_back(temp);
					dbg_printf("  -> match [%s] ;-)\n", temp);
				}
				else {
					results.push_back(name); // ;-)
					dbg_printf("  -> no match :-(\n");
				}
			}
		}
		free(mcEntries);
		free(dir);
		free(ext);
    }

    _screen->wantAnim(false);

	return results;
}

bool Ps2SaveFileManager::removeSavefile(const char *filename) {

	char dir[64], name[64], fullPath[128];

	splitPath(filename, dir, name);	
	sprintf(fullPath, "/ScummVM-%s/%s", dir, name);

	int res = _mc->remove(fullPath);
	return (res == 0);
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

McAccess *Ps2SaveFileManager::getMcAccess(void) {
	return _mc;
}


