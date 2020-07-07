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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WINTERMUTE_WINTERMUTE_H
#define WINTERMUTE_WINTERMUTE_H

#include "engines/engine.h"
#include "gui/debugger.h"
#include "common/fs.h"

namespace Wintermute {

class Console;
class BaseGame;
class SystemClassRegistry;
class DebuggerController;
struct WMEGameDescription;

const int INT_MAX_VALUE = 0x7fffffff;

// our engine debug channels
enum {
	kWintermuteDebugLog = 1 << 0, // The debug-logs from the original engine
	kWintermuteDebugSaveGame = 1 << 1,
	kWintermuteDebugFont = 1 << 2, // next new channel must be 1 << 2 (4)
	kWintermuteDebugFileAccess = 1 << 3, // the current limitation is 32 debug channels (1 << 31 is the last one)
	kWintermuteDebugAudio = 1 << 4,
	kWintermuteDebugGeneral = 1 << 5
};

enum WintermuteGameFeatures {
 	/** A game with low-spec resources. */
 	GF_LOWSPEC_ASSETS       = 1 << 0,
 	GF_IGNORE_SD_FILES      = 1 << 1,
 	GF_IGNORE_HD_FILES      = 1 << 2,
 	GF_3D                   = 1 << 3
};

class WintermuteEngine : public Engine {
public:
	WintermuteEngine(OSystem *syst, const WMEGameDescription *desc);
	WintermuteEngine();
	~WintermuteEngine() override;

	virtual Wintermute::Console *getConsole() { return _debugger; }

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	Common::SaveFileManager *getSaveFileMan() { return _saveFileMan; }
	Common::Error loadGameState(int slot) override;
	bool canLoadGameStateCurrently() override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canSaveGameStateCurrently() override;
	// For detection-purposes:
	static bool getGameInfo(const Common::FSList &fslist, Common::String &name, Common::String &caption);
private:
	int init();
	void deinit();
	int messageLoop();
	Wintermute::Console *_debugger;
	BaseGame *_game;
	Wintermute::DebuggerController *_dbgController;
	const WMEGameDescription *_gameDescription;

	friend class Console;
	friend class DebuggerController;
};

} // End of namespace Wintermute

#endif
