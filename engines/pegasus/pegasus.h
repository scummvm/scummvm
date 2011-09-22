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
	class QuickTimeDecoder;
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

	// Menu
	void useMenu(GameMenu *menu);
	bool checkGameMenu();

	// Misc.
	bool isDemo() const;
	void addIdler(Idler *idler);
	void removeIdler(Idler *idler);
	void addTimeBase(TimeBase *timeBase);
	void removeTimeBase(TimeBase *timeBase);
	void swapSaveAllowed(bool allow) { _saveAllowed = allow; }
	void swapLoadAllowed(bool allow) { _loadAllowed = allow; }
	void delayShell(TimeValue time, TimeScale scale);
	void resetIntroTimer();

protected:
	Common::Error run();

	Cursor *_cursor;

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
	void checkCallBacks();

	// Save/Load
	bool loadFromStream(Common::ReadStream *stream);
	bool writeToStream(Common::WriteStream *stream, int saveType);
	void makeContinuePoint();
	void loadFromContinuePoint();
	Common::ReadStream *_continuePoint;
	bool _saveAllowed, _loadAllowed; // It's so nice that this was in the original code already :P

	// Misc.
	Hotspot _returnHotspot;
	void showLoadDialog();
	void showTempScreen(const Common::String &fileName);

	// Menu
	GameMenu *_gameMenu;
	void doGameMenuCommand(const tGameMenuCommand);
	void doInterfaceOverview();
};

} // End of namespace Pegasus

#endif
