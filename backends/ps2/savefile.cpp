/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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

#include "backends/ps2/savefile.h"
#include "backends/ps2/Gs2dScreen.h"

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileio.h>
#include <malloc.h>
#include <ucl/ucl.h>
#include "scummsys.h"

class StdioSaveFile : public SaveFile {
public:
	StdioSaveFile(const char *filename, bool saveOrLoad);
	virtual ~StdioSaveFile();
	virtual bool isOpen() const;
	virtual uint32 read(void *buf, uint32 cnt);
	virtual uint32 write(const void *buf, uint32 cnt);
private:
	FILE *_fh;
	bool _saving;
};

class UclSaveFile : public SaveFile {
public:
	UclSaveFile(const char *filename, bool saveOrLoad, Gs2dScreen *screen);
	virtual ~UclSaveFile();
	virtual bool isOpen() const;
	virtual uint32 read(void *buf, uint32 cnt);
	virtual uint32 write(const void *buf, uint32 cnt);
private:
	Gs2dScreen *_screen;
	FILE *_fh;
	bool _saving;
	uint8 *_buf;
	uint32 _bufSize, _bufPos;
};

#define ARRAY_ENTRIES	16
static mcTable mcDir[ARRAY_ENTRIES] __attribute__((aligned(64)));
static int     mcEntries;
static bool    mcNeedUpdate = true;

Ps2SaveFileManager::Ps2SaveFileManager(const char *path, SaveMode mode, Gs2dScreen *screen) {
	_screen = screen;
	if (mcInit(MC_TYPE_MC) < 0) {
		printf("Can't init libmc!\n");
		SleepThread();
	}
	
	if (path)
		strcpy(_savePath, path);
	else
		_savePath[0] = '\0';

	_mode = mode;
	if (mode == TO_HOST) {
		printf("saving to host:/\n");
	} else if (mode == TO_MC) {
		int mcType, mcFree, mcFormat, res;
		mcGetInfo(0, 0, &mcType, &mcFree, &mcFormat);
		mcSync(0, NULL, &res);
		if ((res == 0) || (res == -1)) // mc okay
			printf("MC okay, result = %d. Type %d, Free %d, Format %d\n", res, mcType, mcFree, mcFormat);
		else
			printf("MC failed, not present or not formatted, code %d\n", res);
	} else {
		printf("HDD not implemented yet\n");
		SleepThread();
	}

	checkMainDirectory();
}

Ps2SaveFileManager::~Ps2SaveFileManager(void) {
}

void Ps2SaveFileManager::checkMainDirectory(void) {
	// verify that the main directory (scummvm config + icon) exists
	int ret;
	mcGetDir(0, 0, "/ScummVM/*", 0, ARRAY_ENTRIES, mcDir);
	mcSync(0, NULL, &ret);
	printf("/ScummVM/* res = %d\n", ret);
	if (ret <= 0) { // assume directory doesn't exist
		printf("Dir doesn't exist\n");
		fioMkdir("mc0:ScummVM");
		FILE *outf = fopen("mc0:ScummVM/scummvm.icn", "wb");
		if (outf) {
			uint16 icoSize;
			uint16 *icoBuf = decompressIconData(&icoSize);
			fwrite(icoBuf, 2, icoSize, outf);
			fclose(outf);
			printf(".icn written\n");
			free(icoBuf);

			setupIcon("mc0:ScummVM/icon.sys", "scummvm.icn", "ScummVM", "Configuration");
		} else
			printf("unable to write icon data\n");
	}
}

SaveFile *Ps2SaveFileManager::openSavefile(const char *filename, bool saveOrLoad) {
	char *defaultExt = "SAVE";
	char nameBase[256];
	strcpy(nameBase, filename);
	char *ext = strchr(nameBase, '.');
	if (ext) {	
		*ext = '\0';
		ext++;
		if (!*ext)
			ext = defaultExt;
	} else
		ext = defaultExt;

	if (_mode == TO_HOST) {
		char hostName[256];
		sprintf(hostName, "%s%s", _savePath, filename);
		SaveFile *res = new StdioSaveFile(hostName, saveOrLoad);
		if (!res->isOpen()) {
			printf("unable to open savefile %s for %s\n", hostName, saveOrLoad ? "saving" : "loading");
			delete res;
			return NULL;
		}
		printf("Savefile %s opened for %s\n", hostName, saveOrLoad ? "saving" : "loading");
		return res;
	} else if (_mode == TO_MC) {
		_screen->wantAnim(true);
		int mcType, mcFree, mcFormat, mcResult;
		mcGetInfo(0, 0, &mcType, &mcFree, &mcFormat);
		mcSync(0, NULL, &mcResult);
		if (mcResult == -1) // memory card was exchanged
			mcNeedUpdate = true;
		else if (mcResult != 0) {
			printf("Memory card is not ready\n");
			return NULL;
		}

		char dirStr[256];
		sprintf(dirStr, "/ScummVM-%s/*", nameBase);
		if (saveOrLoad) { // saving
			mcGetDir(0, 0, dirStr, 0, ARRAY_ENTRIES, mcDir);
			mcSync(0, NULL, &mcEntries);
			mcNeedUpdate = true;
			if (mcEntries <= 0) { // directory is empty or doesn't exist.
				sprintf(dirStr, "mc0:ScummVM-%s", nameBase);
				printf("Creating directory %s\n", dirStr);
				if (mcEntries < 0) { // directory doesn't exist
					if (fioMkdir(dirStr) < 0) {
						printf("unable to create directory %s\n", dirStr);
						_screen->wantAnim(false);
						return NULL; // unable to create directory
					}
				}
				char icoSysDest[256], saveDesc[256];
				sprintf(icoSysDest, "%s/icon.sys", dirStr);
				strcpy(saveDesc, nameBase);
				if ((saveDesc[0] >= 'a') && (saveDesc[0] <= 'z'))
					saveDesc[0] += 'A' - 'a'; 
				setupIcon(icoSysDest, "../ScummVM/scummvm.icn", saveDesc, "Savegames");
			}
		} else {
			// scumm engine tries to open hundreds of files to search for savegames.
			if (mcNeedUpdate) {
				mcGetDir(0, 0, dirStr, 0, ARRAY_ENTRIES, mcDir);
				mcSync(0, NULL, &mcEntries);
				mcNeedUpdate = false;
			}
			bool fileExists = false;
			char searchName[32];
			sprintf(searchName, "%s.bin", ext);
			for (int cnt = 0; (cnt < mcEntries) && !fileExists; cnt++)
				if (strcmp(searchName, (char*)mcDir[cnt].name) == 0)
					fileExists = true;

			if (!fileExists) {
				_screen->wantAnim(false);
				return NULL;
			}
		}
		sprintf(dirStr, "mc0:ScummVM-%s/%s.bin", nameBase, ext);
		SaveFile *file = new UclSaveFile(dirStr, saveOrLoad, _screen);
		if (!file->isOpen()) {
			printf("unable to open savefile %s for %s\n", dirStr, saveOrLoad ? "saving" : "loading");
            delete file;
			_screen->wantAnim(false);
			return NULL;
		}
		return file;
	} else {
		printf("HDD not implemented yet\n");
		return NULL;
	}
}

void Ps2SaveFileManager::listSavefiles(const char * /* prefix */, bool *marks, int num) {
	memset(marks, true, num * sizeof(bool));
}

const char *Ps2SaveFileManager::getSavePath(void) const {
	return _savePath;
}

void Ps2SaveFileManager::setSavePath(const char *path) {
	strcpy(_savePath, path);
}

bool Ps2SaveFileManager::setupIcon(const char *dest, const char *ico, const char *descr1, const char *descr2) {
	mcIcon icon_sys;
	memset(&icon_sys, 0, sizeof(mcIcon));
	memcpy(icon_sys.head, "PS2D", 4);
	char title[256];
	sprintf(title, "%s\n%s", descr1, descr2);
	strcpy_sjis((short*)&(icon_sys.title), title);
	icon_sys.nlOffset = strlen(descr1) + 1;
	icon_sys.trans = 0x10;
	memcpy(icon_sys.bgCol, _bgcolor, sizeof(_bgcolor));
	memcpy(icon_sys.lightDir, _lightdir, sizeof(_lightdir));
	memcpy(icon_sys.lightCol, _lightcol, sizeof(_lightcol));
	memcpy(icon_sys.lightAmbient, _ambient, sizeof(_ambient));
	strcpy((char*)icon_sys.view, ico);
	strcpy((char*)icon_sys.copy, ico);
	strcpy((char*)icon_sys.del, ico);

	FILE *outf = fopen(dest, "wb");
	if (outf) {
		fwrite(&icon_sys, 1, sizeof(icon_sys), outf);
		fclose(outf);
		return true;
	} else
		return false;
}

uint16 *Ps2SaveFileManager::decompressIconData(uint16 *size) {
	uint16 inPos = 1;
	uint16 *rleData = (uint16*)_rleIcoData;
	uint16 resSize = rleData[0];
	uint16 *resData = (uint16*)malloc(resSize * sizeof(uint16*));
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


StdioSaveFile::StdioSaveFile(const char *filename, bool saveOrLoad)  { 
	_fh = ::fopen(filename, (saveOrLoad? "wb" : "rb")); 
	_saving = saveOrLoad;
}

StdioSaveFile::~StdioSaveFile(void)  { 
	if (_fh)
		::fclose(_fh);
}

bool StdioSaveFile::isOpen(void) const {
	return _fh != NULL;
}

uint32 StdioSaveFile::read(void *buf, uint32 cnt) {
	assert(!_saving);
	return ::fread(buf, 1, cnt, _fh);
}

uint32 StdioSaveFile::write(const void *buf, uint32 cnt) {
	assert(_saving);
	return ::fwrite(buf, 1, cnt, _fh);
}

UclSaveFile::UclSaveFile(const char *filename, bool saveOrLoad, Gs2dScreen *screen) {
	_fh = ::fopen(filename, (saveOrLoad? "wb" : "rb")); 
	_saving = saveOrLoad;
	_bufPos = 0;
	_screen = screen;
	if (_fh) {
		if (_saving) {
			_buf = (uint8*)malloc(65536);
			_bufSize = 65536;
		} else {
			uint32 srcSize = ::fsize(_fh);
			uint8 *srcBuf = (uint8*)malloc(srcSize);
			int res = ::fread(srcBuf, 1, srcSize, _fh);
			assert(res == srcSize);

			uint32 resLen = _bufSize = *(uint32*)srcBuf;
			_buf = (uint8*)malloc(_bufSize + 2048);
			res = ucl_nrv2e_decompress_8(srcBuf + 4, srcSize - 4, _buf, &resLen, NULL);
			if ((res < 0) || (resLen != _bufSize)) {
				printf("Unable to decompress file %s (%d -> %d) error code %d\n", filename, srcSize, _bufSize, res);
                free(_buf);
				_buf = NULL;
				_bufSize = 0;
			}
			::fclose(_fh);
			_fh = NULL;
			free(srcBuf);
		}
	} else {
		printf("Savefile %s doesn't exist\n", filename);
		_buf = NULL;
	}
}

UclSaveFile::~UclSaveFile(void) {
	if (_saving) {
		uint8 *compBuf = (uint8*)malloc(_bufPos * 2);
		uint32 compSize = _bufPos * 2;
		int res = ucl_nrv2e_99_compress(_buf, _bufPos, compBuf, &compSize, NULL, 10, NULL, NULL);
		if (res >= 0) {
			fwrite(&_bufPos, 1, 4, _fh);
			fwrite(compBuf, 1, compSize, _fh);
		} else {
            printf("unable to compress %d bytes of savedata, errorcode %d\n", _bufPos, res);
		}
        free(compBuf);
	}
	if (_buf)
		free(_buf);
	if (_fh)
		::fclose(_fh);
	_screen->wantAnim(false);
}

bool UclSaveFile::isOpen(void) const {
	return (_buf != NULL);
}

uint32 UclSaveFile::read(void *buf, uint32 cnt) {
	assert(!_saving);
	uint32 numBytes = (cnt > _bufSize - _bufPos) ? (_bufSize - _bufPos) : cnt;
	memcpy(buf, _buf + _bufPos, numBytes);
	_bufPos += numBytes;
	return numBytes;
}

uint32 UclSaveFile::write(const void *buf, uint32 cnt) {
	assert(_saving);
	if (_bufSize - _bufPos < cnt) {
		_bufSize += (cnt > 65536) ? cnt : 65536;
		_buf = (uint8*)realloc(_buf, _bufSize);
	}
	memcpy(_buf + _bufPos, buf, cnt);
	_bufPos += cnt;
	return cnt;
}

