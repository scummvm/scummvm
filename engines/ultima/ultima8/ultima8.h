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

#ifndef ULTIMA8_ULTIMA8
#define ULTIMA8_ULTIMA8

#include "common/scummsys.h"
#include "common/system.h"
#include "common/archive.h"
#include "common/error.h"
#include "common/random.h"
#include "common/hash-str.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"
#include "ultima/detection.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/engine/ultima.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/args.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/kernel/hid_keys.h"
#include "ultima/ultima8/misc/console.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"
#include "ultima/ultima8/graphics/point_scaler.h"
#include "common/events.h"

namespace Ultima {
namespace Ultima8 {

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

class Kernel;
class MemoryManager;
class UCMachine;
class Game;
class Gump;
class ConsoleGump;
class GameMapGump;
class ScalerGump;
class InverterGump;
class RenderSurface;
class PaletteManager;
class GameData;
class World;
class ObjectManager;
class FontManager;
class HIDManager;
class Mouse;
class AvatarMoverProcess;
class IDataSource;
class ODataSource;
struct Texture;

namespace Pentagram {
class AudioMixer;
}

class Ultima8Engine : public Shared::UltimaEngine, public CoreApp {
private:
	Std::list<ObjId> textmodes;      //!< Gumps that want text mode

	/**
	 * Does engine deinitialization
	 */
	void deinitialize() override;

	// Load and save games from arbitrary filenames from the console
	static void ConCmd_saveGame(const Console::ArgvType &argv);         //!< "Ultima8Engine::saveGame <optional filename>" console command
	static void ConCmd_loadGame(const Console::ArgvType &argv);         //!< "Ultima8Engine::loadGame <optional filename>" console command
	static void ConCmd_newGame(const Console::ArgvType &argv);          //!< "Ultima8Engine::newGame" console command
//	static void ConCmd_ForceQuit(const Console::ArgvType &argv);

	static void ConCmd_quit(const Console::ArgvType &argv);             //!< "quit" console command

	static void ConCmd_changeGame(const Console::ArgvType &argv);       //!< "Ultima8Engine::changeGame" console command
	static void ConCmd_listGames(const Console::ArgvType &argv);            //!< "Ultima8Engine::listGames" console command

	static void ConCmd_setVideoMode(const Console::ArgvType &argv);     //!< "Ultima8Engine::setVideoMode" console command

	// This should be a console variable once they are implemented
	bool drawRenderStats;
	static void ConCmd_drawRenderStats(const Console::ArgvType &argv);  //!< "Ultima8Engine::drawRenderStats" console command

	static void ConCmd_engineStats(const Console::ArgvType &argv);  //!< "Ultima8Engine::engineStats" console command

	static void ConCmd_toggleAvatarInStasis(const Console::ArgvType &argv); //!< "Ultima8Engine::toggleAvatarInStasis" console command
	static void ConCmd_togglePaintEditorItems(const Console::ArgvType &argv);   //!< "Ultima8Engine::togglePaintEditorItems" console command
	static void ConCmd_toggleShowTouchingItems(const Console::ArgvType &argv);  //!< "Ultima8Engine::toggleShowTouchingItems" console command

	static void ConCmd_closeItemGumps(const Console::ArgvType &argv);   //!< "Ultima8Engine::closeItemGumps" console command

	static void ConCmd_toggleCheatMode(const Console::ArgvType &argv);  //!< "Cheat::toggle" console command

	static void ConCmd_memberVar(const Console::ArgvType &argv);    //!< "Ultima8Engine::memberVar <member> [newvalue] [updateini]" console command

	bool ttfoverrides;

	// Audio Mixer
	Pentagram::AudioMixer *audiomixer;
private:
	uint32 save_count;

	//! write savegame info (time, ..., game-specifics)
	void writeSaveInfo(ODataSource *ods);

	//! save CoreApp/Ultima8Engine data
	void save(ODataSource *ods);

	//! load CoreApp/Ultima8Engine data
	bool load(IDataSource *ids, uint32 version);

	//! reset engine (including World, UCMachine, a.o.)
	void resetEngine();

	//! create core gumps (DesktopGump, GameMapGump, ConsoleGump, ...)
	void setupCoreGumps();

	//! Does a Full reset of the Engine (including shutting down Video)
//	void fullReset();

	// full system
	Game *game;
	Pentagram::istring change_gamename;
	Std::string error_message;
	Std::string error_title;

	Kernel *kernel;
	MemoryManager *_memoryManager;
	ObjectManager *objectmanager;
	HIDManager *hidmanager;
	UCMachine *ucmachine;
	RenderSurface *screen;
	bool fullscreen;
	Mouse *_mouse;
	PaletteManager *palettemanager;
	GameData *gamedata;
	World *world;
	FontManager *fontmanager;

	Gump *desktopGump;
	ConsoleGump *consoleGump;
	GameMapGump *gameMapGump;
	ScalerGump *scalerGump;
	InverterGump *inverterGump;
	AvatarMoverProcess *avatarMoverProcess;

	// called depending upon command line arguments
	void GraphicSysInit(); // starts/restarts the graphics subsystem
	bool LoadConsoleFont(Std::string confontini); // loads the console font

	void handleDelayedEvents();

	// Various dependancy flags
	bool runSDLInit;

	// Timing stuff
	int32 lerpFactor;       //!< Interpolation factor for this frame (0-256)
	bool inBetweenFrame;    //!< Set true if we are doing an inbetween frame

	bool frameSkip;         //!< Set to true to enable frame skipping (default false)
	bool frameLimit;        //!< Set to true to enable frame limiting (default true)
	bool interpolate;       //!< Set to true to enable interpolation (default true)
	int32 animationRate;    //!< The animation rate. Affects all processes! (default 100)

	// Sort of Camera Related Stuff, move somewhere else

	bool avatarInStasis;    //!< If this is set to true, Avatar can't move,
	//!< nor can Avatar start more usecode
	bool paintEditorItems;  //!< If true, paint items with the SI_EDITOR flag

	bool painting;          //!< Set true when painting

	bool showTouching;          //!< If true, highlight items touching Avatar

	static void conAutoPaint(void);
private:
	int32 timeOffset;
	bool has_cheated;
	bool cheats_enabled;

	uint32 _lastDown[HID_LAST];
	bool _down[HID_LAST];
	unsigned int inversion;
protected:
	// Engine APIs
	virtual Common::Error run() override;

	virtual bool initialize() override;

	virtual void DeclareArgs() override;

	/**
	 * Returns the data archive folder and version that's required
	 */
	virtual bool isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) override;
public:
	Pentagram::PointScaler point_scaler;
public:
	ENABLE_RUNTIME_CLASSTYPE()

	Ultima8Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	virtual ~Ultima8Engine();
	void GUIError(const Common::String &msg);

	static Ultima8Engine *get_instance() {
		return p_dynamic_cast<Ultima8Engine *>(application);
	}

	void startup();
	void shutdown();

	void startupGame();
	void startupPentagramMenu();
	void shutdownGame(bool reloading = true);
	void changeGame(Pentagram::istring newgame);

	// When in the Pentagram Menu, load minimal amount of data for the specific game
	// Used to enable access to the games gumps and shapes
	void menuInitMinimal(Pentagram::istring game);

	void changeVideoMode(int width, int height, int fullscreen = -1); // -1 = no change, -2 = fullscreen toggle
	RenderSurface *getRenderScreen() {
		return screen;
	}

	virtual void runGame() override;
	virtual void handleEvent(const Common::Event &event);

	virtual void paint() override;
	virtual bool isPainting() override {
		return painting;
	}


	INTRINSIC(I_getCurrentTimerTick);
	INTRINSIC(I_setAvatarInStasis);
	INTRINSIC(I_getAvatarInStasis);
	INTRINSIC(I_getTimeInGameHours);
	INTRINSIC(I_getTimeInMinutes);
	INTRINSIC(I_getTimeInSeconds);
	INTRINSIC(I_setTimeInGameHours);
	INTRINSIC(I_avatarCanCheat);
	INTRINSIC(I_makeAvatarACheater);
	INTRINSIC(I_closeItemGumps);

	void setAvatarInStasis(bool stat) {
		avatarInStasis = stat;
	}
	bool isAvatarInStasis() const {
		return avatarInStasis;
	}
	void toggleAvatarInStasis() {
		avatarInStasis = !avatarInStasis;
	}
	bool isPaintEditorItems() const {
		return paintEditorItems;
	}
	void togglePaintEditorItems() {
		paintEditorItems = !paintEditorItems;
	}
	bool isShowTouchingItems() const {
		return showTouching;
	}
	void toggleShowTouchingItems() {
		showTouching = !showTouching;
	}

	uint32 getGameTimeInSeconds();

	GameMapGump *getGameMapGump() {
		return gameMapGump;
	}
	ConsoleGump *getConsoleGump() {
		return consoleGump;
	}
	Gump *getDesktopGump() {
		return desktopGump;
	}
	Gump *getGump(uint16 gumpid);

	//! add a gump to the right core gump (such as desktopgump)
	//! \param gump the gump to be added
	void addGump(Gump *gump);

	AvatarMoverProcess *getAvatarMoverProcess() {
		return avatarMoverProcess;
	}

	/**
	 * Notifies the engine that the sound settings may have changed
	 */
	virtual void syncSoundSettings() override;

	/**
	 * Returns true if a savegame can be loaded
	 */
	virtual bool canLoadGameStateCurrently(bool isAutosave = false) override { return true; }

	/**
	 * Returns true if the game can be saved
	 */
	virtual bool canSaveGameStateCurrently(bool isAutosave = false) override;

	/**
	 * Load a game state
	 */
	virtual Common::Error loadGameState(int slot) override;

	/**
	 * Save a game state.
	 * @param slot	the slot into which the savestate should be stored
	 * @param desc	a description for the savestate, entered by the user
	 * @param isAutosave If true, autosave is being created
	 * @return returns kNoError on success, else an error code.
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave) override;

	bool saveGame() override;

	//! save a game
	//! \param filename the file to save to
	//! \return true if succesful
	bool saveGame(Std::string filename, Std::string desc,
	              bool ignore_modals = false);

	bool loadGame() override;

	//! load a game
	//! \param filename the savegame to load
	//! \return true if succesful.
	bool loadGame(Std::string filename);

	//! start a new game
	//! \return true if succesful.
	bool newGame(const Std::string &savegame);

	//! Enter gump text mode (aka SDL Unicode keyhandling)
	void enterTextMode(Gump *);

	//! Leave gump text mode (aka SDL Unicode keyhandling)
	void leaveTextMode(Gump *);

	//! Display an error message box
	//! \param message The message to display on the box
	//! \param exit_to_menu If true, then exit to the Pentagram menu then display the message
	void Error(Std::string message, Std::string title = Std::string(), bool exit_to_menu = false);
public:
	unsigned int getInversion() const {
		return inversion;
	}
	void setInversion(unsigned int i) {
		inversion = i & 0xFFFF;
	}
	bool isInverted() {
		return (inversion >= 0x4000 && inversion < 0xC000);
	}
public:
	bool areCheatsEnabled() const {
		return cheats_enabled;
	}
	void setCheatMode(bool enabled) {
		cheats_enabled = enabled;
	}
	bool hasCheated() const {
		return has_cheated;
	}
	void makeCheater() {
		has_cheated = true;
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
