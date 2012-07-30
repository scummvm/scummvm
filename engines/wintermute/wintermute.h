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
 */

#ifndef WINTERMUTE_H
#define WINTERMUTE_H

#include "common/random.h"
#include "engines/engine.h"
#include "engines/advancedDetector.h"
#include "gui/debugger.h"

namespace WinterMute {

class Console;
class BaseGame;
class SystemClassRegistry;
// our engine debug channels
enum {
    kWinterMuteDebugLog = 1 << 0, // The debug-logs from the original engine
    kWinterMuteDebugSaveGame = 1 << 1,
    kWinterMuteDebugFont = 1 << 2, // next new channel must be 1 << 2 (4)
    kWinterMuteDebugFileAccess = 1 << 3, // the current limitation is 32 debug channels (1 << 31 is the last one)
    kWinterMuteDebugAudio = 1 << 4
};

class WinterMuteEngine : public Engine {
public:
	WinterMuteEngine(OSystem *syst, const ADGameDescription *desc);
	WinterMuteEngine();
	~WinterMuteEngine();

	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
	Common::SaveFileManager *getSaveFileMan() { return _saveFileMan; }
	SystemClassRegistry *getClassRegistry(){ return _classReg; }
	uint32 randInt(int from, int to);
	virtual Common::Error loadGameState(int slot);
	virtual bool canLoadGameStateCurrently();
	virtual Common::Error saveGameState(int slot, const Common::String &desc);
	virtual bool canSaveGameStateCurrently();
	// For detection-purposes:
	static bool getGameInfo(const Common::FSList &fslist, Common::String &name, Common::String &caption);
private:
	int init();
	void deinit();
	int messageLoop();
	Console *_console;
	BaseGame *_game;
	SystemClassRegistry *_classReg;
	// We need random numbers
	Common::RandomSource *_rnd;
	const ADGameDescription *_gameDescription;
};

// Example console class
class Console : public GUI::Debugger {
public:
	Console(WinterMuteEngine *vm) {}
	virtual ~Console(void) {}
};

// Mainly used for randInt()
extern WinterMuteEngine *g_wintermute;

} // End of namespace Wintermute

#endif
