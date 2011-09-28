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
#include "pegasus/hotspot.h"
#include "pegasus/input.h"
#include "pegasus/notification.h"
#include "pegasus/items/inventory.h"
#include "pegasus/neighborhood/neighborhood.h"

namespace Video {
	class SeekableVideoDecoder;
}

namespace Pegasus {

class PegasusConsole;
struct PegasusGameDescription;
class SoundManager;
class GraphicsManager;
class Idler;
class Cursor;
class TimeBase;
class GameMenu;
class InventoryItem;
class BiochipItem;
class Neighborhood;

class PegasusEngine : public ::Engine, public InputHandler, public NotificationManager {
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
	GraphicsManager *_gfx;
	Common::MacResManager *_resFork;
	Cursor *_cursor;

	// Menu
	void useMenu(GameMenu *menu);
	bool checkGameMenu();

	// Misc.
	bool isDemo() const;
	void addIdler(Idler *idler);
	void removeIdler(Idler *idler);
	void addTimeBase(TimeBase *timeBase);
	void removeTimeBase(TimeBase *timeBase);
	void delayShell(TimeValue time, TimeScale scale);
	void resetIntroTimer();
	void refreshDisplay();
	bool playerAlive();
	void processShell();
	void checkCallBacks();
	void createInterface();
	void setGameMode(const tGameMode);
	tGameMode getGameMode() const { return _gameMode; }

	// Energy
	void setLastEnergyValue(const int32 value) { _savedEnergyValue = value; }
	int32 getSavedEnergyValue() { return _savedEnergyValue; }

	// Death
	void setEnergyDeathReason(const tDeathReason reason) { _deathReason = reason; } 
	tDeathReason getEnergyDeathReason() { return _deathReason; }
	void resetEnergyDeathReason();
	void die(const tDeathReason);

	// Volume
	uint16 getSoundFXLevel() { return _FXLevel; }
	uint16 getAmbienceLevel() { return _ambientLevel; }

	// Items
	bool playerHasItem(const Item *);
	bool playerHasItemID(const tItemID);
	void checkFlashlight();
	bool itemInLocation(const tItemID, const tNeighborhoodID, const tRoomID, const tDirectionConstant);

	// Inventory Items
	InventoryItem *getCurrentInventoryItem();
	bool itemInInventory(InventoryItem *);
	bool itemInInventory(tItemID);
	Inventory *getItemsInventory() { return &_items; }
	tInventoryResult addItemToInventory(InventoryItem *);

	// Biochips
	BiochipItem *getCurrentBiochip();
	bool itemInBiochips(BiochipItem *);
	bool itemInBiochips(tItemID);
	Inventory *getBiochipsInventory() { return &_biochips; }

	// AI
	Common::String getBriefingMovie();
	Common::String getEnvScanMovie();
	uint getNumHints();
	Common::String getHintMovie(uint);
	bool canSolve();
	void prepareForAIHint(const Common::String &);
	void cleanUpAfterAIHint(const Common::String &);

	// Neighborhood
	void jumpToNewEnvironment(const tNeighborhoodID, const tRoomID, const tDirectionConstant);

	// Dragging
	void dragItem(const Input &, Item *, tDragType);
	tDragType getDragType() const { return (tDragType)0; } // TODO
	Item *getDraggingItem() const { return 0; } // TODO

	// Save/Load
	void makeContinuePoint();
	bool swapSaveAllowed(bool allow) {
		bool old = _saveAllowed;
		_saveAllowed = allow;
		return old;
	}
	bool swapLoadAllowed(bool allow) {
		bool old = _loadAllowed;
		_loadAllowed = allow;
		return old;
	}

protected:
	Common::Error run();

	Notification _shellNotification;
	virtual void receiveNotification(Notification *notification, const tNotificationFlags flags);

	void handleInput(const Input &input, const Hotspot *cursorSpot);

private:
	// Console
	PegasusConsole *_console;

	// Intro
	void runIntro();
	bool detectOpeningClosingDirectory();
	Common::String _introDirectory;

	// Idlers
	Common::List<Idler *> _idlers;
	void giveIdleTime();

	// Items
	void createItems();
	void createItem(tItemID itemID, tNeighborhoodID neighborhoodID, tRoomID roomID, tDirectionConstant direction);
	Inventory _items;
	Inventory _biochips;

	// TimeBases
	Common::List<TimeBase *> _timeBases;

	// Save/Load
	bool loadFromStream(Common::ReadStream *stream);
	bool writeToStream(Common::WriteStream *stream, int saveType);
	void loadFromContinuePoint();
	Common::ReadStream *_continuePoint;
	bool _saveAllowed, _loadAllowed; // It's so nice that this was in the original code already :P

	// Misc.
	Hotspot _returnHotspot;
	void showLoadDialog();
	void showTempScreen(const Common::String &fileName);
	bool playMovieScaled(Video::SeekableVideoDecoder *video, uint16 x, uint16 y);
	void throwAwayEverything();

	// Menu
	GameMenu *_gameMenu;
	void doGameMenuCommand(const tGameMenuCommand);
	void doInterfaceOverview();

	// Energy
	int32 _savedEnergyValue;

	// Death
	tDeathReason _deathReason;
	void doDeath();

	// Neighborhood
	Neighborhood *_neighborhood;
	void useNeighborhood(Neighborhood *neighborhood);

	// Sound
	uint16 _ambientLevel;
	uint16 _FXLevel;

	// Game Mode
	tGameMode _gameMode;
	void switchGameMode(const tGameMode, const tGameMode);
	bool canSwitchGameMode(const tGameMode, const tGameMode);
};

} // End of namespace Pegasus

#endif
