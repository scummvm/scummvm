/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HOPKINS_HOPKINS_H
#define HOPKINS_HOPKINS_H

#include "hopkins/anim.h"
#include "hopkins/computer.h"
#include "hopkins/debugger.h"
#include "hopkins/dialogs.h"
#include "hopkins/events.h"
#include "hopkins/files.h"
#include "hopkins/font.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/lines.h"
#include "hopkins/menu.h"
#include "hopkins/objects.h"
#include "hopkins/saveload.h"
#include "hopkins/script.h"
#include "hopkins/sound.h"
#include "hopkins/talk.h"

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/hash-str.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"

/**
 * This is the namespace of the Hopkins engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Hopkins FBI
 */
namespace Hopkins {

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

enum HOPKINSAction {
	kActionNone,
	kActionEscape,
	kActionInventory,
	kActionSave,
	kActionLoad,
	kActionOptions
};

enum HopkinsDebugChannels {
	kDebugPath     = 1 << 0,
	kDebugGraphics = 1 << 1
};

/**
 * A wrapper macro used around three character constants, like 'END', to
 * ensure portability. Typical usage: MKTAG24('E','N','D').
 */
#define MKTAG24(a0,a1,a2) ((uint32)((a2) | (a1) << 8 | ((a0) << 16)))

struct HopkinsGameDescription;

class HopkinsEngine : public Engine {
private:
	const HopkinsGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	void initializeSystem();

	void displayNotAvailable();
	void restoreSystem();
	void endLinuxDemo();
	void displayEndDemo();
	void bombExplosion();
	void handleConflagration();
	void playSubmarineCutscene();
	void playUnderwaterBaseCutscene();
	void playPlaneCutscene();
	void playEnding();
	bool isUnderwaterSubScene();

	/**
	 * Displays the map screen in the underground base.
	 */
	int  handleBaseMap();

	/**
	 * Loads the base map from the PBASE file
	 */
	void loadBaseMap();

	/**
	 * Draws a simple base map for the Windows version, which implemented a 'Wolfenstein 3D'
	 * style shooter for the base, rather than having a map
	 */
	void drawBaseMap();

	void handleOceanMouseEvents();
	void setSubmarineSprites();
	void handleOceanMaze(int16 curExitId, const Common::Path &backgroundFilename, Directions defaultDirection, int16 exit1, int16 exit2, int16 exit3, int16 exit4, int16 soundId);
	void loadCredits();
	void displayCredits(int startPosY, byte *buffer, char color);
	void displayCredits();
	void handleNotAvailable(int nextScreen);

	bool runWin95Demo();
	bool runLinuxDemo();
	bool runFull();

	/**
	 * Show warning screen about the game being adults only.
	 */
	bool displayAdultDisclaimer();
protected:
	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;

public:
	AnimationManager *_animMan;
	ComputerManager *_computer;
	DialogsManager *_dialog;
	EventsManager *_events;
	FileManager *_fileIO;
	FontManager *_fontMan;
	Globals *_globals;
	GraphicsManager *_graphicsMan;
	LinesManager *_linesMan;
	MenuManager *_menuMan;
	ObjectsManager *_objectsMan;
	SaveLoadManager *_saveLoad;
	ScriptManager *_script;
	SoundManager *_soundMan;
	TalkManager *_talkMan;

public:
	HopkinsEngine(OSystem *syst, const HopkinsGameDescription *gameDesc);
	~HopkinsEngine() override;
	void GUIError(const Common::String &msg);

	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	bool getIsDemo() const;
	const Common::String &getTargetName() const;

	int getRandomNumber(int maxNumber);
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

	int _startGameSlot;

	/**
	 * Run the introduction sequence
	 */
	void playIntro();

	/**
	 * Synchronizes the sound settings from ScummVM into the engine
	 */
	void syncSoundSettings() override;
};

} // End of namespace Hopkins

#endif /* HOPKINS_HOPKINS_H */
