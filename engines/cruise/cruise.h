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

#ifndef CRUISE_CRUISE_H
#define CRUISE_CRUISE_H

#include "common/scummsys.h"
#include "common/util.h"

#include "engines/engine.h"

#include "cruise/cruise_main.h"

namespace Cruise {

enum CruiseGameType {
	GType_CRUISE = 1
};

struct CRUISEGameDescription;

class CruiseEngine:public Engine {

protected:
	// Engine APIs
	virtual Common::Error init();
	virtual Common::Error go();

	void shutdown();

	bool initGame();

public:
	CruiseEngine(OSystem * syst, const CRUISEGameDescription *gameDesc);
	virtual ~ CruiseEngine();

	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	bool loadSaveDirectory(void);
	void makeSystemMenu(void);

	const CRUISEGameDescription *_gameDescription;

	Common::RandomSource _rnd;

private:
	void initialize(void);
	bool makeLoad(char *saveName);
	void mainLoop(int bootScriptIdx);

	bool _preLoad;
};

extern CruiseEngine *g_cruise;

#define BOOT_PRC_NAME "AUTO00.PRC"

enum {
	VAR_MOUSE_X_MODE = 253,
	VAR_MOUSE_X_POS = 249,
	VAR_MOUSE_Y_MODE = 251,
	VAR_MOUSE_Y_POS = 250
};

enum {
	MOUSE_CURSOR_NORMAL = 0,
	MOUSE_CURSOR_DISK,
	MOUSE_CURSOR_CROSS
};

enum {
	kCruiseDebugScript = 1 << 0
};

enum {
	kCmpEQ = (1 << 0),
	kCmpGT = (1 << 1),
	kCmpLT = (1 << 2)
};

extern Common::SaveFileManager * g_saveFileMan;	// TEMP

} // End of namespace Cruise

#endif
