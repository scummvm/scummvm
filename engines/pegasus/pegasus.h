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

#include "common/macresman.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/rect.h"
#include "common/util.h"

#include "engines/engine.h"

#include "pegasus/graphics.h"
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

enum ItemLocation {
	kItemLocationCaldoria = 0,
	kItemLocationTSA = 1,
	kItemLocationNorad = 4, // ???
	kItemLocationMars = 5,
	kItemLocationWSC = 6,
	kItemLocationPrehistoric = 7,
	kItemLocationBuiltIn = 0xffff
};

static const int kViewScreenOffset = 64;

struct ItemLocationData {
	uint16 id;
	ItemLocation location;
	uint16 u0;
	byte u1;
};

struct InventoryPanelEntry {
	uint32 startTime;
	uint32 endTime;
};

struct InventoryItemData {
	uint32 leftFrameTime;
	uint32 rightStartTime;
	uint32 rightEndTime;
	uint16 pict;        // Cannot use item at this spot
	uint16 usablePict;  // Can use item at this spot
};

struct InventoryExtra {
	uint32 id;
	uint16 movie;
	uint32 startTime;
	uint32 endTime;
};

struct LeftAreaData {
	uint16 frame;
	uint32 time;
};

struct MiddleAreaData {
	uint16 id;
	uint32 time;
};

struct RightAreaData {
	uint16 frame;
	uint32 time;
};

struct OverviewHotspot {
	Common::Rect rect;
	uint32 time;
};

// Taken from JMP PP Resources
enum Item {
	kAIBiochip = 128,
	kInterfaceBiochip = 129, // NOT USED!
	kMapBiochip = 130,
	kOpticalBiochip = 131,
	kPegasusBiochip = 132,
	kRetinalScanBiochip = 133,
	kShieldBiochip = 134,
	kAirMask = 135,
	kAntidote = 136,
	kArgonCanister = 137,
	kCardBomb = 138,
	kCrowbar = 139,
	kGasCanister = 140,
	kHistoricalLog = 141,
	kJourneymanKey = 142,
	kKeyCard = 143,
	kMachineGun = 144, // What the hell is this?
	kMarsCard = 145,
	kNitrogenCanister = 146,
	kOrangeJuiceGlassFull = 147,
	kOrangeJuiceGlassEmpty = 148,
	kPoisonDart = 149,
	kSinclairKey = 150,
	kStunGun = 151,
	kArgonPickup = 152 // ???
};

enum GameMode {
	kIntroMode,
	kMainMenuMode,
	kMainGameMode,
	kQuitMode
};

class PegasusEngine : public ::Engine {
protected:
	Common::Error run();

public:
	PegasusEngine(OSystem *syst, const PegasusGameDescription *gamedesc);
	virtual ~PegasusEngine();
	
	const PegasusGameDescription *_gameDescription;
	bool hasFeature(EngineFeature f) const;
	GUI::Debugger *getDebugger();
	
	VideoManager *_video;
	GraphicsManager *_gfx;
	Common::MacResManager *_resFork, *_inventoryLid, *_biochipLid;

	bool isDemo() const;

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
	void loadItemLocationData();
	void changeLocation(tNeighborhoodID neighborhood);

	// Misc Functions
	static Common::String getTimeZoneFolder(tNeighborhoodID neighborhood);
	static Common::String getTimeZoneDesc(tNeighborhoodID neighborhood);

	// Game Variables
	bool _adventureMode;
	GameMode _gameMode;
	tNeighborhoodID _neighborhood;
	Common::Array<ItemLocationData> _itemLocationData;

	// Console
	PegasusConsole *_console;

	// Intro Directory Code
	bool detectOpeningClosingDirectory();
	Common::String _introDirectory;
};

} // End of namespace Pegasus

#endif
