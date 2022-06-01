
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

#ifndef ULTIMA8_ULTIMA8
#define ULTIMA8_ULTIMA8

#include "common/stream.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/engine/ultima.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/games/game_info.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/detection.h"

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
class FileSystem;
class ConfigFileManager;
struct GameInfo;

#define GAME_IS_U8 (Ultima8Engine::get_instance()->getGameInfo()->_type == GameInfo::GAME_U8)
#define GAME_IS_REMORSE (Ultima8Engine::get_instance()->getGameInfo()->_type == GameInfo::GAME_REMORSE)
#define GAME_IS_REGRET (Ultima8Engine::get_instance()->getGameInfo()->_type == GameInfo::GAME_REGRET)
#define GAME_IS_CRUSADER (GAME_IS_REMORSE || GAME_IS_REGRET)

class Ultima8Engine : public Shared::UltimaEngine {
	friend class Debugger;
private:
	bool _isRunning;
	GameInfo *_gameInfo;

	// minimal system
	FileSystem *_fileSystem;
	ConfigFileManager *_configFileMan;

	static Ultima8Engine *_instance;

	bool _fontOverride;
	bool _fontAntialiasing;
	// Audio Mixer
	AudioMixer *_audioMixer;
	uint32 _saveCount;

	// full system
	Game *_game;
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

	bool _highRes;			 //!< Set to true to enable larger screen size
	bool _frameSkip;         //!< Set to true to enable frame skipping (default false)
	bool _frameLimit;        //!< Set to true to enable frame limiting (default true)
	bool _interpolate;       //!< Set to true to enable interpolation (default true)
	int32 _animationRate;    //!< The animation rate, frames per second in "fast" ticks (3000 per second). Affects all processes! (default 100 = 30 fps)

	// Sort of Camera Related Stuff, move somewhere else

	bool _avatarInStasis;    //!< If this is set to true, Avatar can't move,
	//!< nor can Avatar start more usecode
	bool _paintEditorItems;  //!< If true, paint items with the SI_EDITOR flag
	bool _showTouching;          //!< If true, highlight items touching Avatar
	int32 _timeOffset;
	bool _hasCheated;
	bool _cheatsEnabled;
	unsigned int _inversion;
	bool _unkCrusaderFlag; //!< not sure what this is but it's only used in usecode for crusader, so just keep track of it..
	uint32 _moveKeyFrame; //!< An imperfect way for the Crusader usecode to stop remote camera viewing.
	bool _cruStasis; //!< A slightly different kind of stasis for Crusader that stops some keyboard events
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

	//! Fill a GameInfo struct for the give game name
	//! \param game The id of the game to check (from pentagram.cfg)
	//! \param gameinfo The GameInfo struct to fill
	//! \return true if detected all the fields, false if detection failed
	bool getGameInfo(const istring &game, GameInfo *gameinfo);

protected:
	// Engine APIs
	Common::Error run() override;

	bool initialize() override;

	void pauseEngineIntern(bool pause) override;

	/**
	 * Returns the data archive folder and version that's required
	 */
	bool isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) override;

public:
	Ultima8Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima8Engine() override;

	static Ultima8Engine *get_instance() {
		return _instance;
	}

	bool hasFeature(EngineFeature f) const override;

	bool startup();
	void shutdown();

	bool setupGame();
	bool startupGame();
	void shutdownGame(bool reloading = true);

	void changeVideoMode(int width, int height);

	//! Get current GameInfo struct
	const GameInfo *getGameInfo() const {
		return _gameInfo;
	}

	RenderSurface *getRenderScreen() {
		return _screen;
	}

	Graphics::Screen *getScreen() const override;

	bool runGame();
	virtual void handleEvent(const Common::Event &event);

	void paint();

	static const int U8_DEFAULT_SCREEN_WIDTH = 320;
	static const int U8_DEFAULT_SCREEN_HEIGHT = 200;
	static const int CRUSADER_DEFAULT_SCREEN_WIDTH = 640;
	static const int CRUSADER_DEFAULT_SCREEN_HEIGHT = 480;

	static const int U8_HIRES_SCREEN_WIDTH = 640;
	static const int U8_HIRES_SCREEN_HEIGHT = 400;
	static const int CRUSADER_HIRES_SCREEN_WIDTH = 1024;
	static const int CRUSADER_HIRES_SCREEN_HEIGHT = 768;

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
	void setCruStasis(bool flag) {
		_cruStasis = flag;
	}
	bool isCruStasis() const {
		return _cruStasis;
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
	* Notifies the engine that the game settings may have changed
	*/
	void applyGameSettings() override;

	/**
	* Opens the config dialog and apply setting after close
	*/
	void openConfigDialog();

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
	bool saveGame(int slot, const Std::string &desc);

	//! start a new game
	//! \return true if succesful.
	bool newGame(int saveSlot = -1);

	//! Display an error message box
	//! \param message The message to display on the box
	void Error(Std::string message, Std::string title = Std::string());
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
	bool isInterpolationEnabled() const {
		return _interpolate;
	}
public:
	U8PixelFormat _renderFormat;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
