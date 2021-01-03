
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
#include "common/stream.h"
#include "common/system.h"
#include "common/archive.h"
#include "common/error.h"
#include "common/random.h"
#include "common/hash-str.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"
#include "gui/debugger.h"
#include "ultima/detection.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/engine/ultima.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/args.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"
#include "common/events.h"

namespace Ultima {
namespace Ultima8 {

class Debugger;
class Kernel;
class UCMachine;
class Game;
class Gump;
class GameMapGump;
class MenuGump;
class InverterGump;
class RenderSurface;
class PaletteManager;
class GameData;
class World;
class ObjectManager;
class FontManager;
class Mouse;
class AvatarMoverProcess;
class Texture;
class AudioMixer;

class Ultima8Engine : public Shared::UltimaEngine, public CoreApp {
	friend class Debugger;
private:
	Std::list<ObjId> _textModes;      //!< Gumps that want text mode
	bool _ttfOverrides;
	// Audio Mixer
	AudioMixer *_audioMixer;
	uint32 _saveCount;

	// full system
	Game *_game;
	istring _changeGameName;
	Std::string _errorMessage;
	Std::string _errorTitle;

	Kernel *_kernel;
	ObjectManager *_objectManager;
	UCMachine *_ucMachine;
	RenderSurface *_screen;
	Mouse *_mouse;
	PaletteManager *_paletteManager;
	GameData *_gameData;
	World *_world;
	FontManager *_fontManager;

	Gump *_desktopGump;
	GameMapGump *_gameMapGump;
	InverterGump *_inverterGump;
	AvatarMoverProcess *_avatarMoverProcess;

	// Various dependancy flags
	// Timing stuff
	int32 _lerpFactor;       //!< Interpolation factor for this frame (0-256)
	bool _inBetweenFrame;    //!< Set true if we are doing an inbetween frame

	bool _frameSkip;         //!< Set to true to enable frame skipping (default false)
	bool _frameLimit;        //!< Set to true to enable frame limiting (default true)
	bool _interpolate;       //!< Set to true to enable interpolation (default true)
	int32 _animationRate;    //!< The animation rate, frames per second in "fast" ticks (3000 per second). Affects all processes! (default 100 = 30 fps)

	// Sort of Camera Related Stuff, move somewhere else

	bool _avatarInStasis;    //!< If this is set to true, Avatar can't move,
	//!< nor can Avatar start more usecode
	bool _paintEditorItems;  //!< If true, paint items with the SI_EDITOR flag
	bool _painting;          //!< Set true when painting
	bool _showTouching;          //!< If true, highlight items touching Avatar
	int32 _timeOffset;
	bool _hasCheated;
	bool _cheatsEnabled;
	unsigned int _inversion;
	bool _unkCrusaderFlag; //!< not sure what this is but it's only used in usecode for crusader, so just keep track of it..
	uint32 _moveKeyFrame; //!< An imperfect way for the Crusader usecode to stop remote camera viewing.
private:
	/**
	 * Does engine deinitialization
	 */
	void deinitialize() override;

	/**
	 * Shows the Pentagram splash screen
	 */
	void showSplashScreen();

private:
	//! write savegame info (time, ..., game-specifics)
	void writeSaveInfo(Common::WriteStream *ws);

	//! save CoreApp/Ultima8Engine data
	void save(Common::WriteStream *ws);

	//! load CoreApp/Ultima8Engine data
	bool load(Common::ReadStream *rs, uint32 version);

	//! reset engine (including World, UCMachine, a.o.)
	void resetEngine();

	//! create core gumps (DesktopGump, GameMapGump, ConsoleGump, ...)
	void setupCoreGumps();

	//! Does a Full reset of the Engine (including shutting down Video)
//	void fullReset();

	// called depending upon command line arguments
	void GraphicSysInit(); // starts/restarts the graphics subsystem

	void handleDelayedEvents();
protected:
	// Engine APIs
	Common::Error run() override;

	bool initialize() override;

	/**
	 * Returns the data archive folder and version that's required
	 */
	bool isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) override;

public:
	Ultima8Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima8Engine() override;
	void GUIError(const Common::String &msg);

	static Ultima8Engine *get_instance() {
		return dynamic_cast<Ultima8Engine *>(_application);
	}

	bool startup();
	void shutdown();

	bool startupGame();
	void startupPentagramMenu();
	void shutdownGame(bool reloading = true);
	void changeGame(istring newgame);

	// When in the Pentagram Menu, load minimal amount of data for the specific game
	// Used to enable access to the games gumps and shapes
	void menuInitMinimal(istring game);

	void changeVideoMode(int width, int height);

	RenderSurface *getRenderScreen() {
		return _screen;
	}

	Graphics::Screen *getScreen() const override;

	bool runGame() override;
	virtual void handleEvent(const Common::Event &event);

	void paint() override;
	bool isPainting() override {
		return _painting;
	}

	static const int U8_DEFAULT_SCREEN_WIDTH = 320;
	static const int U8_DEFAULT_SCREEN_HEIGHT = 200;
	static const int CRUSADER_DEFAULT_SCREEN_WIDTH = 640;
	static const int CRUSADER_DEFAULT_SCREEN_HEIGHT = 480;

	INTRINSIC(I_getCurrentTimerTick);
	INTRINSIC(I_setAvatarInStasis);
	INTRINSIC(I_getAvatarInStasis);
	INTRINSIC(I_getTimeInGameHours);
	INTRINSIC(I_getTimeInMinutes);
	INTRINSIC(I_getTimeInSeconds);
	INTRINSIC(I_setTimeInGameHours);
	INTRINSIC(I_avatarCanCheat);
	INTRINSIC(I_getUnkCrusaderFlag);
	INTRINSIC(I_setUnkCrusaderFlag);
	INTRINSIC(I_clrUnkCrusaderFlag);
	INTRINSIC(I_makeAvatarACheater);
	INTRINSIC(I_closeItemGumps);
	INTRINSIC(I_setCruStasis);
	INTRINSIC(I_clrCruStasis);
	INTRINSIC(I_moveKeyDownRecently);

	void setAvatarInStasis(bool stat) {
		_avatarInStasis = stat;
	}
	bool isAvatarInStasis() const {
		return _avatarInStasis;
	}
	void toggleAvatarInStasis() {
		_avatarInStasis = !_avatarInStasis;
	}
	bool isPaintEditorItems() const {
		return _paintEditorItems;
	}
	void togglePaintEditorItems() {
		_paintEditorItems = !_paintEditorItems;
	}
	bool isShowTouchingItems() const {
		return _showTouching;
	}
	void toggleShowTouchingItems() {
		_showTouching = !_showTouching;
	}

	bool isUnkCrusaderFlag() const {
		return _unkCrusaderFlag;
	}
	void setUnkCrusaderFlag(bool flag) {
		_unkCrusaderFlag = flag;
	}

	void moveKeyEvent();
	bool moveKeyDownRecently();

	uint32 getGameTimeInSeconds();

	GameMapGump *getGameMapGump() {
		return _gameMapGump;
	}
	Gump *getDesktopGump() {
		return _desktopGump;
	}
	Gump *getGump(uint16 gumpid);

	//! add a gump to the right core gump (such as desktopgump)
	//! \param gump the gump to be added
	void addGump(Gump *gump);

	AvatarMoverProcess *getAvatarMoverProcess() {
		return _avatarMoverProcess;
	}

	/**
	 * Notifies the engine that the sound settings may have changed
	 */
	void syncSoundSettings() override;

	/**
	 * Returns true if a savegame can be loaded
	 */
	bool canLoadGameStateCurrently(bool isAutosave = false) override { return true; }

	/**
	 * Returns true if the game can be saved
	 */
	bool canSaveGameStateCurrently(bool isAutosave = false) override;

	/**
	 * Load a game
	 */
	Common::Error loadGameState(int slot) override;

	/**
	 * Saves the game
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

	/**
	 * Load a game state
	 */
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	/**
	 * Handles saving savegame state to a stream
	 */
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave) override;

	//! save a game
	//! \param filename the file to save to
	//! \return true if succesful
	bool saveGame(int slot, const Std::string &desc, bool ignore_modals = false);

	//! start a new game
	//! \return true if succesful.
	bool newGame(int saveSlot = -1);

	//! Enter gump text mode (aka Unicode keyhandling)
	void enterTextMode(Gump *);

	//! Leave gump text mode (aka Unicode keyhandling)
	void leaveTextMode(Gump *);

	//! Display an error message box
	//! \param message The message to display on the box
	//! \param exit_to_menu If true, then exit to the Pentagram menu then display the message
	void Error(Std::string message, Std::string title = Std::string(), bool exit_to_menu = false);
public:
	unsigned int getInversion() const {
		return _inversion;
	}
	void setInversion(unsigned int i) {
		_inversion = i & 0xFFFF;
	}
	bool isInverted() {
		return (_inversion >= 0x4000 && _inversion < 0xC000);
	}
public:
	bool areCheatsEnabled() const {
		return _cheatsEnabled;
	}
	void setCheatMode(bool enabled) {
		_cheatsEnabled = enabled;
	}
	bool hasCheated() const {
		return _hasCheated;
	}
	void makeCheater() {
		_hasCheated = true;
	}
	Gump *getMenuGump() const;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
