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

#ifndef MADS_GAME_H
#define MADS_GAME_H

#include "common/scummsys.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/serializer.h"
#include "mads/audio.h"
#include "mads/scene.h"
#include "mads/game_data.h"
#include "mads/globals.h"
#include "mads/inventory.h"
#include "mads/player.h"
#include "mads/screen.h"
#include "mads/camera.h"

namespace MADS {

class MADSEngine;

enum KernelMode {
	KERNEL_GAME_LOAD = 0, KERNEL_SECTION_PRELOAD = 1, KERNEL_SECTION_INIT = 2,
	KERNEL_ROOM_PRELOAD = 3, KERNEL_ROOM_INIT = 4, KERNEL_ACTIVE_CODE = 5
};

enum SyncType {
	SYNC_SEQ = 1, SYNC_PLAYER = 2, SYNC_ANIM = 3, SYNC_CLOCK = 4
};


#define MADS_SAVEGAME_VERSION 1

struct MADSSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class Game {
private:
	/**
	 * Main game loop
	 */
	void gameLoop();

	/**
	 * Inner game loop for executing gameplay within a game section
	 */
	void sectionLoop();

	/**
	 * Load quotes data
	 */
	void loadQuotes();
protected:
	MADSEngine *_vm;
	MSurface *_surface;
	int _statusFlag;
	Common::StringArray _quotes;
	bool _quoteEmergency;
	bool _vocabEmergency;
	bool _anyEmergency;
	int _lastSave;
	Common::String _saveName;
	Common::InSaveFile *_saveFile;
	Graphics::Surface *_saveThumb;

	/**
	 * Constructor
	 */
	Game(MADSEngine *vm);

	/**
	 * Initializes the current section number of the game
	 */
	void initSection(int sectionNumber);

	//@{
	/** @name Virtual Method list */

	/**
	 * Perform any game-specifcic startup
	 */
	virtual void startGame() = 0;

	/**
	 * Initializes global variables for a new game
	 */
	virtual void initializeGlobals() = 0;

	/**
	 * Set up the section handler specific to each section
	 */
	virtual void setSectionHandler() = 0;

	/**
	 * Checks for whether to show a dialog
	 */
	virtual void checkShowDialog() = 0;

	//@}

public:
	static Game *init(MADSEngine *vm);

public:
	Player _player;
	ScreenObjects _screenObjects;
	int _sectionNumber;
	int _priorSectionNumber;
	int _currentSectionNumber;
	InventoryObjects _objects;
	SectionHandler *_sectionHandler;
	VisitedScenes _visitedScenes;
	Scene _scene;
	KernelMode _kernelMode;
	int _trigger;
	ScreenTransition _fx;
	TriggerMode _triggerMode;
	TriggerMode _triggerSetupMode;
	uint32 _priorFrameTimer;
	Common::String _aaName;
	int _winStatus;
	int _widepipeCtr;
	int _loadGameSlot;
	int _panningSpeed;
	Camera _camX, _camY;

public:
	virtual ~Game();

	/**
	 * Main outer loop for the game
	 */
	void run();

	/**
	 * Return the number of quotes
	 */
	uint32 getQuotesSize() { return _quotes.size(); }

	/**
	 * Get a specific quote string
	 */
	const Common::String &getQuote(uint32 index) { return _quotes[index - 1]; }

	/**
	 * Split a quote into two lines for display on-screen
	 */
	void splitQuote(const Common::String &source, Common::String &line1, Common::String &line2);

	Common::StringArray getMessage(uint32 id);

	/**
	 * Returns the globals for the game
	 */
	virtual Globals &globals() = 0;

	/**
	* Standard object handling across the game
	*/
	virtual void doObjectAction() = 0;

	/**
	 * Fallback handler for any action that isn't explicitly handled
	 */
	virtual void unhandledAction() = 0;

	/**
	 * Global game step
	 */
	virtual void step() = 0;

	/**
	 * Synchronize the game data
	 * @param s			Serializer
	 * @param phase1	If true, it's synchronizing the basic scene information
	 */
	virtual void synchronize(Common::Serializer &s, bool phase1);

	virtual void setNaughtyMode(bool naughtyMode) {}
	virtual bool getNaughtyMode() const { return true; }

	// DEPRECATED: ScummVM re-implementation keeps all the quotes loaded, so the methods below are stubs
	void clearQuotes() {}
	void loadQuoteRange(int startNum, int endNum) {}
	void loadQuoteSet(...) {}
	void loadQuote(int quoteNum) {}

	/**
	* Handle a keyboard event
	*/
	void handleKeypress(const Common::KeyState &kbd);

	/**
	 * Starts a savegame loading.
	 * @remarks	Due to the way the engine is implemented, loading is done in two
	 * parts, the second part after the specific scene has been loaded
	 */
	void loadGame(int slotNumber);

	/**
	 * Save the current game
	 */
	void saveGame(int slotNumber, const Common::String &saveName);

	/**
	 * Write out a savegame header
	 */
	void writeSavegameHeader(Common::OutSaveFile *out, MADSSavegameHeader &header);

	/**
	 * Read in a savegame header
	 */
	WARN_UNUSED_RESULT static bool readSavegameHeader(Common::InSaveFile *in, MADSSavegameHeader &header, bool skipThumbnail = true);

	/**
	 * Creates a temporary thumbnail for use in saving games
	 */
	void createThumbnail();

	void syncTimers(SyncType slaveType, int slaveId, SyncType masterType, int masterId);

	void camInitDefault();
	void camSetSpeed();
	void camUpdate();
};

} // End of namespace MADS

#endif /* MADS_GAME_H */
