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

class Ps2SaveFileManager : public SaveFileManager {
public:
	Ps2SaveFileManager(const char *path, SaveMode mode, Gs2dScreen *screen);
	virtual ~Ps2SaveFileManager();

	virtual SaveFile *openSavefile(const char *filename, bool saveOrLoad);
	virtual void listSavefiles(const char * /* prefix */, bool *marks, int num);

	/** Get the path to the save game directory. */
	virtual const char *getSavePath() const;

	void setSavePath(const char *path);
	static bool setupIcon(const char *dest, const char *ico, const char *descr1, const char *descr2);

	void checkMainDirectory(void);
private:
	uint16 *decompressIconData(uint16 *size);

	Gs2dScreen *_screen;

	static const uint8 _rleIcoData[14018];
	SaveMode _mode;
	char _savePath[256];

	static const iconIVECTOR _bgcolor[4];
	static const iconFVECTOR _lightdir[3], _lightcol[3], _ambient;
};

#endif // __PS2_SAVEFILE__
