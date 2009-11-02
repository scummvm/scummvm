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

#ifndef CINE_H
#define CINE_H


#include "common/scummsys.h"
#include "common/file.h"
#include "common/util.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

#include "engines/engine.h"

#include "cine/texte.h"
#include "cine/rel.h"
#include "cine/script.h"
#include "cine/part.h"
#include "cine/prc.h"
#include "cine/msg.h"
#include "cine/bg.h"
#include "cine/pal.h"
#include "cine/gfx.h"
#include "cine/anim.h"

//#define DUMP_SCRIPTS

namespace Cine {

enum CineGameType {
	GType_FW = 1,
	GType_OS
};

enum CineGameFeatures {
	GF_CD =   1 << 0,
	GF_DEMO = 1 << 1,
	GF_ALT_FONT = 1 << 2,
	GF_CRYPTED_BOOT_PRC = 1 << 3
};

struct CINEGameDescription;

typedef Common::HashMap<Common::String, const char *> StringPtrHashMap;

class CineEngine : public Engine {

protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;

	void shutdown();

	bool initGame();

public:
	CineEngine(OSystem *syst, const CINEGameDescription *gameDesc);
	virtual ~CineEngine();

	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	bool loadSaveDirectory();
	void makeSystemMenu();
	int modifyGameSpeed(int speedChange);
	int getTimerDelay() const;
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const char *desc);
	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();

	const CINEGameDescription *_gameDescription;
	Common::File _partFileHandle;

	Common::RandomSource _rnd;

	Common::StringList _volumeResourceFiles;
	StringPtrHashMap _volumeEntriesMap;
	TextHandler _textHandler;

private:
	void initialize();
	void resetEngine();
	bool loadPlainSaveFW(Common::SeekableReadStream &in, CineSaveGameFormat saveGameFormat);
	bool loadTempSaveOS(Common::SeekableReadStream &in);
	bool makeLoad(char *saveName);
	void makeSaveFW(Common::OutSaveFile &out);
	void makeSaveOS(Common::OutSaveFile &out);
	void makeSave(char *saveFileName);
	void mainLoop(int bootScriptIdx);
	void readVolCnf();

	bool _preLoad;
	int _timerDelayMultiplier;
};

extern CineEngine *g_cine;

#define BOOT_PRC_NAME "AUTO00.PRC"
#define COPY_PROT_FAIL_PRC_NAME "L201.ANI"

enum {
	// Both FW and OS
	VAR_MOUSE_X_POS = 249,
	VAR_MOUSE_Y_POS = 250,
	// FW only
	VAR_MOUSE_X_MODE = 253,
	VAR_MOUSE_Y_MODE = 251,
	// OS only
	VAR_MOUSE_X_POS_2ND = 251, // Many times used in conjunction with VAR_MOUSE_X_POS
	VAR_MOUSE_Y_POS_2ND = 252, // Many times used in conjunction with VAR_MOUSE_Y_POS
	VAR_BYPASS_PROTECTION = 255,
	VAR_LOW_MEMORY = 0
};

enum {
	MOUSE_CURSOR_NORMAL = 0,
	MOUSE_CURSOR_DISK,
	MOUSE_CURSOR_CROSS
};

enum {
	kCineDebugScript = 1 << 0,
	kCineDebugPart   = 1 << 1,
	kCineDebugSound  = 1 << 2
};

enum {
	kCmpEQ = (1 << 0),
	kCmpGT = (1 << 1),
	kCmpLT = (1 << 2)
};


extern Common::SaveFileManager *g_saveFileMan; // TEMP

} // End of namespace Cine

#endif
