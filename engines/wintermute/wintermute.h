/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WINTERMUTE_WINTERMUTE_H
#define WINTERMUTE_WINTERMUTE_H

#include "engines/engine.h"
#include "gui/debugger.h"
#include "common/fs.h"
#include "wintermute/detection.h"

namespace Wintermute {

class Console;
class BaseGame;
class SystemClassRegistry;
class DebuggerController;

const int INT_MAX_VALUE  = 0x7fffffff;
const int INT_MIN_VALUE  = -INT_MAX_VALUE - 1;
const uint UINT_MAX_VALUE = 0xffffffff;

// our engine debug channels
enum {
	kWintermuteDebugLog = 1 << 0, // The debug-logs from the original engine
	kWintermuteDebugSaveGame = 1 << 1,
	kWintermuteDebugFont = 1 << 2, // next new channel must be 1 << 2 (4)
	kWintermuteDebugFileAccess = 1 << 3, // the current limitation is 32 debug channels (1 << 31 is the last one)
	kWintermuteDebugAudio = 1 << 4,
	kWintermuteDebugGeneral = 1 << 5
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
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveAutosaveCurrently() override;
	void savingEnable(bool enable);
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
	bool _savingEnabled{};

	friend class Console;
	friend class DebuggerController;
};

} // End of namespace Wintermute

#endif
