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

#include "common/random.h" // getRandomNumber and getRandomFloat
#include "engines/engine.h" // class Engine
#include "titanic/support/exe_resources.h" // class CExeResources
#include "titanic/support/movie_manager.h" // class CMovieManager
#include "titanic/support/string.h" // class StringArray;
#include "titanic/support/strings.h" // class Strings;
#include "common/language.h" // Language enum

/**
 * This is the namespace of the Titanic engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Starship Titanic
 */

class OSystem;

namespace Graphics {
class Screen;
}

namespace Common {
class Error;
class FSNode;
}

namespace Titanic {

#define TITANIC_SAVEGAME_VERSION 1

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define TOTAL_ITEMS 46
#define TOTAL_ROOMS 34

#define MAX_SAVES 99

// If enabled, fixes an original bug where dispensed chickens weren't
// meant to be hot unless the Yellow fuse was left in the Fusebox.
// This is being left disabled for now, since most walkthroughs for
// the game redundantly suggest removing the fuse, which is wrong
//#define FIX_DISPENSOR_TEMPATURE

class CFilesManager;
class CMainGameWindow;
class CString;
class CTrueTalkManager;
class Debugger;
class Events;
class OSScreenManager;
class CScriptHandler;
class TTscriptBase;
struct TitanicGameDescription;

class TitanicEngine : public Engine {
private:
	/**
	 * Handles basic initialization
	 */
	bool initialize();

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
	StringArray _itemIds;
	StringArray _roomNames;
	Strings _strings;
	CString _stateRoomExitView;
	int _loadSaveSlot;
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

	/**
	 * Handles updates to the sound levels
	 */
	virtual void syncSoundSettings();

	/**
	 * Gets the game features
	 */
	uint32 getFeatures() const;

	/**
	 * Returns the language for the game
	 */
	Common::Language getLanguage() const;

	/**
	 * Returns true if it's the German version
	 */
	bool isGerman() const { return getLanguage() == Common::DE_DEU; }

	/**
	 * Returns a uniform random unsigned integer in the interval [0, max]
	 */
	uint getRandomNumber(uint max) { return _randomSource.getRandomNumber(max); }

	/**
	 * Returns a uniform random floating point number in the interval [0.0, 65535.0]
	 */
	double getRandomFloat() { return getRandomNumber(0xfffffffe) * 0.00001525855623540901; } // fffffffe=4294967294 and 0.00001525855623540901 ~= 1/65537.0 

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

	/**
	 * Shows the ScummVM GMM save dialog
	 */
	void showScummVMSaveDialog();

	/**
	 * Shows the ScummVM GMM load dialog
	 */
	void showScummVMRestoreDialog();
};

extern TitanicEngine *g_vm;

} // End of namespace Titanic

#endif /* TITANIC_TITANIC_H */
