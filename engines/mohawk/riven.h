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

#ifndef MOHAWK_RIVEN_H
#define MOHAWK_RIVEN_H

#include "mohawk/installer_archive.h"
#include "mohawk/mohawk.h"

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/rect.h"

#include "graphics/surface.h"

namespace Mohawk {

struct MohawkGameDescription;
class MohawkArchive;
class RivenGraphics;
class RivenConsole;
class RivenSaveLoad;
class RivenOptionsDialog;
class RivenStack;
class RivenCard;
class RivenHotspot;
class RivenScriptManager;
class RivenSoundManager;
class RivenInventory;
class RivenVideoManager;

// Riven Stack Types
enum {
	kStackUnknown = 0, // Default value for ReturnStackID
	kStackOspit = 1,   // 233rd Age (Gehn's Office)
	kStackPspit = 2,   // Prison Island
	kStackRspit = 3,   // Temple Island
	kStackTspit = 4,   // Rebel Age (Tay)
	kStackBspit = 5,   // Book-Making Island
	kStackGspit = 6,   // Garden Island
	kStackJspit = 7,   // Jungle Island
	kStackAspit = 8,   // Main Menu, Books, Setup

	kStackFirst = kStackOspit,
	kStackLast = kStackAspit
};

// Engine Debug Flags
enum {
	kRivenDebugScript   = (1 << 0),
	kRivenDebugPatches  = (1 << 1)
};

struct ZipMode {
	Common::String name;
	uint16 id;
	bool operator== (const ZipMode& z) const;
};

typedef Common::HashMap<Common::String, uint32, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> RivenVariableMap;

class MohawkEngine_Riven : public MohawkEngine {
protected:
	Common::Error run() override;

public:
	MohawkEngine_Riven(OSystem *syst, const MohawkGameDescription *gamedesc);
	~MohawkEngine_Riven() override;

	RivenVideoManager *_video;
	RivenSoundManager *_sound;
	RivenGraphics *_gfx;
	Common::RandomSource *_rnd;
	RivenScriptManager *_scriptMan;
	RivenInventory *_inventory;

	// Display debug rectangles around the hotspots
	bool _showHotspots;

	GUI::Debugger *getDebugger() override;

	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc) override;
	bool hasFeature(EngineFeature f) const override;

	void doFrame();
	void processInput();

private:
	// Datafiles
	MohawkArchive *_extrasFile; // We need a separate handle for the extra data
	const char **listExpectedDatafiles() const;
	void loadLanguageDatafile(char prefix, uint16 stackId);
	bool checkDatafiles();

	RivenConsole *_console;
	RivenSaveLoad *_saveLoad;
	RivenOptionsDialog *_optionsDialog;
	InstallerArchive _installerArchive;

	// Stack/Card-related functions and variables
	RivenCard *_card;
	RivenStack *_stack;

	int _menuSavedCard;
	int _menuSavedStack;
	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> _menuThumbnail;

	bool _gameEnded;
	uint32 _lastSaveTime;

	// Variables
	void initVars();

	void pauseEngineIntern(bool) override;
public:
	// Stack/card/script funtions
	RivenStack *constructStackById(uint16 id);
	void changeToCard(uint16 dest);
	void changeToStack(uint16 stackId);
	RivenCard *getCard() const { return _card; }
	RivenStack *getStack() const { return _stack; }

	// Hotspot functions/variables
	Common::Array<ZipMode> _zipModeData;
	void addZipVisitedCard(uint16 cardId, uint16 cardNameId);
	bool isZipVisitedCard(const Common::String &hotspotName) const;

	// Variables
	RivenVariableMap _vars;
	uint32 &getStackVar(uint32 index);

	// Miscellaneous
	Common::Array<uint16> getResourceIDList(uint32 type) const;
	Common::SeekableReadStream *getExtrasResource(uint32 tag, uint16 id);
	bool _activatedPLST;
	bool _activatedSLST;
	void delay(uint32 ms);
	void runOptionsDialog();

	// Save / Load
	void runLoadDialog();
	void runSaveDialog();
	void tryAutoSaving();
	void loadGameStateAndDisplayError(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc, bool autosave);
	void saveGameStateAndDisplayError(int slot, const Common::String &desc);

	/**
	 * Has the game ended, or has the user requested to quit?
	 */
	bool hasGameEnded() const;

	/**
	 * End the game gracefully
	 */
	void setGameEnded();

	// Main menu handling
	void goToMainMenu();
	void resumeFromMainMenu();
	bool isInMainMenu() const;
	bool isGameStarted() const;
	void startNewGame();
};

} // End of namespace Mohawk

#endif
