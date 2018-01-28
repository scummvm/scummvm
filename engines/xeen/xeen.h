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

#ifndef XEEN_XEEN_H
#define XEEN_XEEN_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "xeen/combat.h"
#include "xeen/debugger.h"
#include "xeen/dialogs.h"
#include "xeen/events.h"
#include "xeen/files.h"
#include "xeen/interface.h"
#include "xeen/locations.h"
#include "xeen/map.h"
#include "xeen/party.h"
#include "xeen/resources.h"
#include "xeen/saves.h"
#include "xeen/screen.h"
#include "xeen/scripts.h"
#include "xeen/sound.h"
#include "xeen/spells.h"
#include "xeen/window.h"

/**
 * This is the namespace of the Xeen engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Might & Magic 4: Clouds of Xeen
 * - Might & Magic 5: Dark Side of Xeen
 * - Might & Magic: World of Xeen
 * - Might & Magic: Swords of Xeen
 */
namespace Xeen {

enum {
	GType_Clouds = 1,
	GType_DarkSide = 2,
	GType_WorldOfXeen = 3,
	GType_Swords = 4
};

enum XeenDebugChannels {
	kDebugPath      = 1 << 0,
	kDebugScripts	= 1 << 1,
	kDebugGraphics	= 1 << 2,
	kDebugSound     = 1 << 3
};

enum Mode {
	MODE_FF = -1,
	MODE_0 = 0,
	MODE_1 = 1,
	MODE_COMBAT = 2,
	MODE_3 = 3,
	MODE_4 = 4,
	MODE_SLEEPING = 5,
	MODE_6 = 6,
	MODE_7 = 7,
	MODE_8 = 8,
	MODE_RECORD_EVENTS = 9,
	MODE_CHARACTER_INFO = 10,
	MODE_12 = 12,
	MODE_DIALOG_123 = 13,
	MODE_17 = 17,
	MODE_86 = 86
};

enum QuitMode {
	QMODE_NONE = 0,
	QMODE_QUIT = 1,
	QMODE_MENU = 2
};

struct XeenGameDescription;

#define XEEN_SAVEGAME_VERSION 1

class XeenEngine : public Engine {
private:
	const XeenGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
private:
	void initialize();

	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;

	void play();

	void pleaseWait();

	void gameLoop();
protected:
	int _loadSaveSlot;
protected:
	/**
	 * Outer gameplay loop responsible for dispatching control to game-specific
	 * intros, main menus, or to play the actual game
	 */
	virtual void outerGameLoop() = 0;

	/**
	 * Play the game
	 */
	virtual void playGame();

	/**
	 * Death cutscene
	 */
	virtual void death() = 0;
public:
	Combat *_combat;
	Debugger *_debugger;
	EventsManager *_events;
	FileManager *_files;
	Interface *_interface;
	LocationManager *_locations;
	Map *_map;
	Party *_party;
	Resources *_resources;
	SavesManager *_saves;
	Screen *_screen;
	Scripts *_scripts;
	Sound *_sound;
	Spells *_spells;
	Windows *_windows;
	Mode _mode;
	GameEvent _gameEvent;
	Common::SeekableReadStream *_eventData;
	QuitMode _quitMode;
	bool _noDirectionSense;
	bool _startupWindowActive;
	uint _endingScore;
public:
	XeenEngine(OSystem *syst, const XeenGameDescription *gameDesc);
	virtual ~XeenEngine();

	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	uint32 getGameID() const;
	uint32 getGameFeatures() const;

	int getRandomNumber(int maxNumber);

	int getRandomNumber(int minNumber, int maxNumber);

	/**
	 * Load a savegame
	 */
	virtual Common::Error loadGameState(int slot);

	/**
	 * Save the game
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc);

	/**
	 * Updates sound settings
	 */
	virtual void syncSoundSettings();

	/**
	 * Returns true if a savegame can currently be loaded
	 */
	bool canLoadGameStateCurrently();

	/**
	* Returns true if the game can currently be saved
	*/
	bool canSaveGameStateCurrently();

	static Common::String printMil(uint value);

	static Common::String printK(uint value);

	static Common::String printK2(uint value);
};

extern XeenEngine *g_vm;

} // End of namespace Xeen

#endif /* XEEN_XEEN_H */
