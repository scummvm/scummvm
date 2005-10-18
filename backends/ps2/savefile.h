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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef __PS2_SAVEFILE__
#define __PS2_SAVEFILE__

#include "common/savefile.h"
#include <libmc.h>

enum SaveMode {
	TO_HOST = 0,
	TO_MC,
	TO_HDD
};

class Gs2dScreen;
class OSystem_PS2;

class Ps2SaveFileManager : public Common::SaveFileManager {
public:
	Ps2SaveFileManager(OSystem_PS2 *system, Gs2dScreen *screen);
	virtual ~Ps2SaveFileManager();

	virtual Common::InSaveFile *openForLoading(const char *filename);
	virtual Common::OutSaveFile *openForSaving(const char *filename);
	virtual void listSavefiles(const char *prefix, bool *marks, int num);

	/** Get the path to the save game directory. */
	virtual const char *getSavePath() const;
private:
	static bool setupIcon(const char *dest, const char *ico, const char *descr1, const char *descr2);

	bool mcReadyForDir(const char *dir);

	void checkMainDirectory(void);
	void splitPath(const char *fileName, char *dir, char *name);
	uint16 *decompressIconData(uint16 *size);

	Gs2dScreen *_screen;
	OSystem_PS2 *_system;

	mcTable *_mcDirList;
	int		_mcEntries;
	char	_mcDirName[256];
	bool	_mcNeedsUpdate, _mcPresent;
	uint32	_mcCheckTime;

	static const uint8 _rleIcoData[14018];
	static const iconIVECTOR _bgcolor[4];
	static const iconFVECTOR _lightdir[3], _lightcol[3], _ambient;
};

#endif // __PS2_SAVEFILE__
