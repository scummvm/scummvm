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

#ifndef DGDS_DGDS_H
#define DGDS_DGDS_H

#include "common/error.h"
#include "common/events.h"
#include "common/platform.h"
#include "common/random.h"
#include "common/serializer.h"

#include "graphics/surface.h"
#include "graphics/managed_surface.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "gui/debugger.h"

#include "dgds/resource.h"
#include "dgds/clock.h"
#include "dgds/menu.h"


namespace Dgds {

class Console;
class ResourceManager;
class Decompressor;
class Image;
class GamePalettes;
class FontManager;
class Inventory;
class SDSScene;
class GDSScene;
class Sound;
class REQFileData;
class Menu;
struct DgdsADS;
class ADSInterpreter;
class Globals;
class ShellGame;
class DragonArcade;
class HocIntro;
class ChinaTank;
class ChinaTrain;

const float MS_PER_FRAME = 16.6667f;

enum DgdsGameId {
	GID_DRAGON,
	GID_HOC,
	GID_WILLY,
	GID_SQ5DEMO,
	GID_COMINGATTRACTIONS,
	GID_QUARKY,
	GID_CASTAWAY,
	GID_INVALID,
};

enum DgdsDetailLevel {
	kDgdsDetailLow = 0,
	kDgdsDetailHigh = 1
};

enum DgdsKeyEvent {
	kDgdsKeyLoad,
	kDgdsKeySave,
	kDgdsKeyToggleMenu,
	kDgdsKeyToggleClock,
	kDgdsKeyNextChoice,
	kDgdsKeyPrevChoice,
	kDgdsKeyNextItem,
	kDgdsKeyPrevItem,
	kDgdsKeyPickUp,
	kDgdsKeyLook,
	kDgdsKeyActivate,
};

// TODO: Enable keymapper for dragon arcade sequences
/*
enum DragonArcadeKeyEvent {
	kDragonArcadeKeyLeft,
	kDragonArcadeKeyRight,
	kDragonArcadeKeyUp,
	kDragonArcadeKeyDown,
	kDragonArcadeKeyLeftUp,
	kDragonArcadeKeyRightUp,
	kDragonArcadeKeyLeftDown,
	kDragonArcadeKeyRightDown,
	kDragonArcadeKeyJumpMode,
	kDragonArcadeKeyFire,
};
*/


class DgdsEngine : public Engine {
public:
	Common::Platform _platform;
	Common::Language _gameLang;
	Sound *_soundPlayer;
	Graphics::ManagedSurface _compositionBuffer;

	static const byte HOC_CHAR_SWAP_ICONS[];

private:
	Console *_console;

	ResourceManager *_resource;
	Decompressor *_decompressor;

	DgdsGameId _gameId;
	Graphics::ManagedSurface _backgroundBuffer;
	Common::String _backgroundFile;		// Record the background file name for save games.
	Graphics::ManagedSurface _storedAreaBuffer;
	SDSScene *_scene;
	GDSScene *_gdsScene;
	Menu *_menu;

	ADSInterpreter *_adsInterp;
	GamePalettes *_gamePals;
	Globals *_gameGlobals;
	Inventory *_inventory;

	// Dragon only
	DragonArcade *_dragonArcade;

	// HoC only
	ShellGame *_shellGame;
	HocIntro *_hocIntro;
	ChinaTank *_chinaTank;
	ChinaTrain *_chinaTrain;

	FontManager *_fontManager;
	Common::SharedPtr<Image> _corners;
	Common::SharedPtr<Image> _icons;

	// Settings which we should integrate with ScummVM settings UI
	DgdsDetailLevel _detailLevel;
	int _textSpeed;
	int _difficulty;

	bool _justChangedScene1;
	bool _justChangedScene2;

	Common::RandomSource _random;
	Common::Point _lastMouse; // originals start mouse at 0,0.
	int _currentCursor;
	Common::Point _currentCursorHot;

	Clock _clock;

	MenuId _menuToTrigger;

	bool _isLoading;
	const char *_rstFileName;

	bool _isDemo;
	bool _isEGA;
	bool _flipMode;
	bool _skipNextFrame;
	uint32 _thisFrameMs;
	int16 _lastGlobalFade; // Only used in Willy Beamish
	uint _lastGlobalFadedPal;

public:
	DgdsEngine(OSystem *syst, const ADGameDescription *gameDesc);
	virtual ~DgdsEngine();

	virtual Common::Error run() override;

	void restartGame();

	DgdsGameId getGameId() const { return _gameId; }
	Common::Language getGameLang() const { return _gameLang; }
	Common::Platform getPlatform() const { return _platform; }

	Graphics::ManagedSurface &getBackgroundBuffer() { return _backgroundBuffer; }
	Graphics::ManagedSurface &getStoredAreaBuffer() { return _storedAreaBuffer; }

	// Various game engine singletons
	Common::SeekableReadStream *getResource(const Common::String &name, bool ignorePatches);
	ResourceManager *getResourceManager() { return _resource; }
	Decompressor *getDecompressor() { return _decompressor; }
	const SDSScene *getScene() const { return _scene; }
	SDSScene *getScene() { return _scene; }
	GDSScene *getGDSScene() { return _gdsScene; }
	const FontManager *getFontMan() const { return _fontManager; }
	const Common::SharedPtr<Image> &getUICorners() { return _corners; }
	const Common::SharedPtr<Image> &getIcons() { return _icons; }
	GamePalettes *getGamePals() { return _gamePals; }
	Globals *getGameGlobals() { return _gameGlobals; }
	Inventory *getInventory() { return _inventory; }
	Clock &getClock() { return _clock; }
	ADSInterpreter *adsInterpreter() { return _adsInterp; }
	Common::RandomSource &getRandom() { return _random; }

	bool changeScene(int sceneNum);
	void setMouseCursor(int num);

	int getTextSpeed() const { return _textSpeed; }
	void setTextSpeed(int16 speed) { _textSpeed = speed; }
	int16 getDifficulty() const { return _difficulty; }
	void setDifficulty(int16 difficulty) { _difficulty = difficulty; }
	DgdsDetailLevel getDetailLevel() const { return _detailLevel; }
	void setDetailLevel(DgdsDetailLevel level) { _detailLevel = level; }

	void setShowClock(bool val);
	bool justChangedScene1() const { return _justChangedScene1; }
	bool justChangedScene2() const { return _justChangedScene2; }
	Common::Point getLastMouse() const { return _lastMouse; }
	Common::Point getLastMouseMinusHot() const;

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveAutosaveCurrently() override;

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}

	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

	bool hasFeature(EngineFeature f) const override {
		return
			(f == kSupportsReturnToLauncher) ||
			(f == kSupportsLoadingDuringRuntime) ||
			(f == kSupportsSavingDuringRuntime);
	};

	void setBackgroundFile(const Common::String &name) { _backgroundFile = name; }
	const Common::String &getBackgroundFile() const { return _backgroundFile; }
	void setMenuToTrigger(MenuId menu) { _menuToTrigger = menu; }
	bool isInvButtonVisible() const;
	ShellGame *getShellGame() { return _shellGame; }
	HocIntro *getHocIntro() { return _hocIntro; }
	ChinaTrain *getChinaTrain() { return _chinaTrain; }
	ChinaTank *getChinaTank() { return _chinaTank; }
	DragonArcade *getDragonArcade() { return _dragonArcade; }
	void setSkipNextFrame() { _skipNextFrame = true; }
	uint32 getThisFrameMs() const { return _thisFrameMs; }

	static DgdsEngine *getInstance() { return static_cast<DgdsEngine *>(g_engine); }
	void setFlipMode(bool mode) { _flipMode = mode; }

	bool isEGA() const { return _isEGA; }

	void enableKeymapper();
	void disableKeymapper();

private:
	Common::Error syncGame(Common::Serializer &s);

	void loadCorners(const Common::String &filename);
	void loadIcons();
	void checkDrawInventoryButton();

	void init(bool restarting);
	void loadGameFiles();
	void loadRestartFile();
};

} // End of namespace Dgds

#endif // DGDS_DGDS_H
