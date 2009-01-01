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

#ifndef __SAVEFILEMGR_H__
#define __SAVEFILEMGR_H__

#include <libmc.h>
#include "common/savefile.h"

class Gs2dScreen;
class OSystem_PS2;


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

class Ps2SaveFileManager : public Common::SaveFileManager {
public:
	Ps2SaveFileManager(OSystem_PS2 *system, Gs2dScreen *screen);
	virtual ~Ps2SaveFileManager();

	virtual Common::InSaveFile *openForLoading(const char *filename);
	virtual Common::OutSaveFile *openForSaving(const char *filename);
	virtual void listSavefiles(const char *prefix, bool *marks, int num);

	virtual Common::StringList listSavefiles(const char *regex);
	virtual bool removeSavefile(const char *filename);

	/** Get the path to the save game directory. */
	virtual const char *getSavePath() const;

	void writeSaveNonblocking(char *name, void *buf, uint32 size);
	void saveThread(void);
	void quit(void);

	McAccess *getMcAccess(void);
private:
	bool setupIcon(const char *dest, const char *ico, const char *descr1, const char *descr2);

	bool mcReadyForDir(const char *dir);

	void checkMainDirectory(void);
	void splitPath(const char *fileName, char *dir, char *name);
	uint16 *decompressIconData(uint16 *size);

	Gs2dScreen *_screen;
	OSystem_PS2 *_system;
	McAccess	*_mc;

	int _autoSaveTid;
	int _autoSaveSignal;
	void *_autoSaveStack;
	volatile bool _systemQuit;
	uint8 *_autoSaveBuf;
	uint32 _autoSaveSize;
	char _autoSaveName[256];

	mcTable *_mcDirList;
	int		_mcEntries;
	char	_mcDirName[256];
	bool	_mcNeedsUpdate, _mcPresent;
	uint32	_mcCheckTime;

	static const uint8 _rleIcoData[14018];
	static const iconIVECTOR _bgcolor[4];
	static const iconFVECTOR _lightdir[3], _lightcol[3], _ambient;
};

#endif // __SAVEFILE_MGR_H__
