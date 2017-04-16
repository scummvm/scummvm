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

#ifndef TITANIC_TITANIC_H
#define TITANIC_TITANIC_H

#include "common/scummsys.h"
#include "common/random.h"
#include "common/str-array.h"
#include "common/system.h"
#include "common/serializer.h"
#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "graphics/screen.h"
#include "titanic/debugger.h"
#include "titanic/events.h"
#include "titanic/support/files_manager.h"
#include "titanic/main_game_window.h"
#include "titanic/support/exe_resources.h"
#include "titanic/support/movie_manager.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/string.h"
#include "titanic/true_talk/tt_script_base.h"

/**
 * This is the namespace of the Titanic engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Starship Titanic
 */
namespace Titanic {

enum TitanicDebugChannels {
	kDebugCore      = 1 << 0,
	kDebugScripts	= 1 << 1,
	kDebugGraphics	= 1 << 2,
	kDebugStarfield = 1 << 3
};

#define TITANIC_SAVEGAME_VERSION 1

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

#define TOTAL_ITEMS 46
#define TOTAL_ROOMS 34

#define MAX_SAVES 99

struct TitanicGameDescription;
class TitanicEngine;

class TitanicEngine : public Engine {
private:
	/**
	 * Handles basic initialization
	 */
	void initialize();

	/**
	 * Handles game deinitialization
	 */
	void deinitialize();

	/**
	 * Sets up the item names, short, and long descriptions
	 */
	void setItemNames();

	/**
	 * Sets up the list of room names
	 */
	void setRoomNames();
protected:
	const TitanicGameDescription *_gameDescription;
	int _loadSaveSlot;

	// Engine APIs
	virtual void initializePath(const Common::FSNode &gamePath);
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	Debugger *_debugger;
	Events *_events;
	CFilesManager *_filesManager;
	CMovieManager _movieManager;
	Graphics::Screen *_screen;
	OSScreenManager *_screenManager;
	CMainGameWindow *_window;
	Common::RandomSource _randomSource;
	CScriptHandler *_scriptHandler;
	TTscriptBase *_script;
	CTrueTalkManager *_trueTalkManager;
	CExeResources _exeResources;
	StringArray _itemNames;
	StringArray _itemDescriptions;
	CString _itemObjects[TOTAL_ITEMS];
	StringArray _itemIds;
	StringArray _roomNames;
	Strings _strings;
	CString _stateRoomExitView;
public:
	TitanicEngine(OSystem *syst, const TitanicGameDescription *gameDesc);
	virtual ~TitanicEngine();


	/**
	 * Returns true if a savegame can be loaded
	 */
	virtual bool canLoadGameStateCurrently();

	/**
	 * Returns true if the game can be saved
	 */
	virtual bool canSaveGameStateCurrently();

	/**
	 * Called by the GMM to load a savegame
	 */
	virtual Common::Error loadGameState(int slot);

	/**
	 * Called by the GMM to save the game
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc);

	uint32 getFeatures() const;
	bool isDemo() const;
	Common::Language getLanguage() const;

	/**
	 * Returns true if it's the German version
	 */
	bool isGerman() const { return getLanguage() == Common::DE_DEU; }

	/**
	 * Gets a random number
	 */
	uint getRandomNumber(uint max) { return _randomSource.getRandomNumber(max); }

	/**
	 * Returns a random floating point number between 0.0 to 65535.0
	 */
	double getRandomFloat() { return getRandomNumber(0xfffffffe) * 0.000015259022; }

	/**
	 * Support method that generates a savegame name
	 * @param slot		Slot number
	 */
	CString generateSaveName(int slot);

	/**
	 * Checks whether a savegame exists for the given slot,
	 * and if it exists, returns it's description
	 */
	CString getSavegameName(int slot);
};

extern TitanicEngine *g_vm;

} // End of namespace Titanic

#endif /* TITANIC_TITANIC_H */
