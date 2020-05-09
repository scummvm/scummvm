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

#ifndef ULTIMA4_H
#define ULTIMA4_H

#include "ultima/shared/engine/ultima.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class Armors;
class Codex;
class Config;
class Context;
class Death;
class DialogueLoaders;
class ImageLoaders;
class Items;
class GameController;
class MapLoaders;
class Moongates;
class Music;
class ResponseParts;
struct SaveGame;
class Screen;
class Shrines;
class SoundManager;
class Spells;
class TileMaps;
class TileRules;
class TileSets;
class Weapons;

class Ultima4Engine : public Shared::UltimaEngine {
private:
	int _saveSlotToLoad;
private:
	void startup();
protected:
	// Engine APIs
	Common::Error run() override;

	bool initialize() override;

	/**
	 * Returns the data archive folder and version that's required
	 */
	bool isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) override;
public:
	Armors *_armors;
	Codex *_codex;
	Config *_config;
	Context *_context;
	Death *_death;
	DialogueLoaders *_dialogueLoaders;
	ImageLoaders *_imageLoaders;
	GameController *_game;
	Items *_items;
	MapLoaders *_mapLoaders;
	Moongates *_moongates;
	Music *_music;
	ResponseParts *_responseParts;
	SaveGame *_saveGame;
	Screen *_screen;
	Shrines *_shrines;
	SoundManager *_soundManager;
	Spells *_spells;
	TileMaps *_tileMaps;
	TileRules *_tileRules;
	TileSets *_tileSets;
	Weapons *_weapons;
	Std::vector<Common::String> _hawkwindText;
public:
	Ultima4Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima4Engine() override;

	/**
	 * Returns true if a savegame can be loaded
	 */
	bool canLoadGameStateCurrently(bool isAutosave = false) override;

	/**
	 * Returns true if the game can be saved
	 */
	bool canSaveGameStateCurrently(bool isAutosave = false) override;

	/**
	 * Save a game state.
	 * @param slot	the slot into which the savestate should be stored
	 * @param desc	a description for the savestate, entered by the user
	 * @param isAutosave	Expected to be true if an autosave is being created
	 * @return returns kNoError on success, else an error code.
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

	/**
	 * Load a game state.
	 * @param stream	the stream to load the savestate from
	 * @return returns kNoError on success, else an error code.
	 */
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	/**
	 * Save a game state.
	 * @param stream	The write stream to save the savegame data to
	 * @param isAutosave	Expected to be true if an autosave is being created
	 * @return returns kNoError on success, else an error code.
	 */
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

	/**
	 * Specifies to load the previous save as the actual game starts. Used by
	 * the main menu when the Journey Onwards option is selected
	 */
	void setToJourneyOnwards();

	/**
	 * Flags to quit the game
	 */
	void quitGame();
};

extern Ultima4Engine *g_ultima;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
