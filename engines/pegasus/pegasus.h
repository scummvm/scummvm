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

#ifndef PEGASUS_H
#define PEGASUS_H

#include "common/list.h"
#include "common/macresman.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/rect.h"
#include "common/util.h"

#include "engines/engine.h"

#include "pegasus/graphics.h"
#include "pegasus/input.h"
#include "pegasus/video.h"
#include "pegasus/neighborhood/neighborhood.h"

namespace Video {
	class Video::QuickTimeDecoder;
}

namespace Pegasus {

class PegasusConsole;
struct PegasusGameDescription;
class SoundManager;
class VideoManager;
class GraphicsManager;
class Idler;
class Cursor;
class TimeBase;

static const int kViewScreenOffset = 64;

struct OverviewHotspot {
	Common::Rect rect;
	uint32 time;
};

enum GameMode {
	kIntroMode,
	kMainMenuMode,
	kMainGameMode,
	kQuitMode
};

class PegasusEngine : public ::Engine, public InputHandler {
friend class InputHandler;

public:
	PegasusEngine(OSystem *syst, const PegasusGameDescription *gamedesc);
	virtual ~PegasusEngine();

	// Engine stuff
	const PegasusGameDescription *_gameDescription;
	bool hasFeature(EngineFeature f) const;
	GUI::Debugger *getDebugger();
	bool canLoadGameStateCurrently() { return _loadAllowed; }
	bool canSaveGameStateCurrently() { return _saveAllowed; }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);

	// Base classes
	VideoManager *_video;
	GraphicsManager *_gfx;
	Common::MacResManager *_resFork, *_inventoryLid, *_biochipLid;

	// Misc.
	bool isDemo() const;
	void addIdler(Idler *idler);
	void removeIdler(Idler *idler);
	void addTimeBase(TimeBase *timeBase);
	void removeTimeBase(TimeBase *timeBase);
	void swapSaveAllowed(bool allow) { _saveAllowed = allow; }
	void swapLoadAllowed(bool allow) { _loadAllowed = allow; }

protected:
	Common::Error run();

	Cursor *_cursor;

private:
	// Intro
	void runIntro();
	void runMainMenu();
	void drawMenu(int buttonSelected);
	void drawMenuButtonHighlighted(int buttonSelected);
	void drawMenuButtonSelected(int buttonSelected);
	//void runInterfaceOverview();
	void setGameMode(int buttonSelected);

	// Interface
	void drawInterface();
	//void drawCompass();
	//void runPauseMenu();
	void showLoadDialog();

	// Interface Overview
	void runInterfaceOverview();
	void drawInterfaceOverview(const OverviewHotspot &hotspot, Video::QuickTimeDecoder *video);

	// Credits
	void runCredits();
	void drawCredits(int button, bool highlight, int frame, Video::QuickTimeDecoder *video);
	void runDemoCredits();

	// Main Game Functions
	void mainGameLoop();
	void changeLocation(tNeighborhoodID neighborhood);

	// Misc Functions
	static Common::String getTimeZoneFolder(tNeighborhoodID neighborhood);
	static Common::String getTimeZoneDesc(tNeighborhoodID neighborhood);

	// Game Variables
	bool _adventureMode;
	GameMode _gameMode;

	// Console
	PegasusConsole *_console;

	// Intro Directory Code
	bool detectOpeningClosingDirectory();
	Common::String _introDirectory;

	// Idlers
	Common::List<Idler *> _idlers;
	void giveIdleTime();

	// Items
	void createItems();
	void createItem(tItemID itemID, tNeighborhoodID neighborhoodID, tRoomID roomID, tDirectionConstant direction);

	// TimeBases
	Common::List<TimeBase *> _timeBases;

	// Save/Load
	bool loadFromStream(Common::ReadStream *stream);
	bool writeToStream(Common::WriteStream *stream, int saveType);
	void makeContinuePoint();
	void loadFromContinuePoint();
	Common::ReadStream *_continuePoint;
	bool _saveAllowed, _loadAllowed; // It's so nice that this was in the original code already :P
};

} // End of namespace Pegasus

#endif
