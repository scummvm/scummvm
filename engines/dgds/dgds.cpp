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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/platform.h"
#include "common/str-array.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/system.h"

#include "backends/keymapper/keymapper.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "dgds/console.h"
#include "dgds/ads.h"
#include "dgds/decompress.h"
#include "dgds/scene.h"
#include "dgds/detection_tables.h"
#include "dgds/dgds.h"
#include "dgds/font.h"
#include "dgds/globals.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/inventory.h"
#include "dgds/menu.h"
#include "dgds/parser.h"
#include "dgds/request.h"
#include "dgds/resource.h"
#include "dgds/scripts.h"
#include "dgds/sound.h"
#include "dgds/game_palettes.h"
#include "dgds/minigames/dragon_arcade.h"
#include "dgds/minigames/china_tank.h"
#include "dgds/minigames/china_train.h"
#include "dgds/hoc_intro.h"

// for frame contents debugging
//#define DUMP_FRAME_DATA 1

#ifdef DUMP_FRAME_DATA
#include "graphics/paletteman.h"
#include "image/png.h"
#endif

namespace Dgds {

/*static*/
const byte DgdsEngine::HOC_CHAR_SWAP_ICONS[] = { 0, 20, 21, 22 };


DgdsEngine::DgdsEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _fontManager(nullptr), _console(nullptr), _inventory(nullptr),
	_soundPlayer(nullptr), _decompressor(nullptr), _scene(nullptr), _shellGame(nullptr),
	_hocIntro(nullptr), _gdsScene(nullptr), _resource(nullptr), _gamePals(nullptr), _gameGlobals(nullptr),
	_detailLevel(kDgdsDetailHigh), _textSpeed(1), _justChangedScene1(false), _justChangedScene2(false),
	_random("dgds"), _currentCursor(-1), _menuToTrigger(kMenuNone), _isLoading(true), _flipMode(false),
	_rstFileName(nullptr), _difficulty(1), _menu(nullptr), _adsInterp(nullptr), _isDemo(false),
_dragonArcade(nullptr), _chinaTank(nullptr), _chinaTrain(nullptr), _skipNextFrame(false), _gameId(GID_INVALID) {

	_platform = gameDesc->platform;
	_gameLang = gameDesc->language;

	if (!strcmp(gameDesc->gameId, "rise")) {
		_gameId = GID_DRAGON;
	} else if (!strcmp(gameDesc->gameId, "china")) {
		_gameId = GID_HOC;
	} else if (!strcmp(gameDesc->gameId, "beamish")) {
		_isDemo = (gameDesc->flags & ADGF_DEMO);
		_gameId = GID_WILLY;
	} else if (!strcmp(gameDesc->gameId, "quarky")) {
		_gameId = GID_QUARKY;
	} else if (!strcmp(gameDesc->gameId, "sq5demo")) {
		_isDemo = true;
		_gameId = GID_SQ5DEMO;
	} else if (!strcmp(gameDesc->gameId, "comingattractions")) {
		_isDemo = true;
		_gameId = GID_COMINGATTRACTIONS;
	} else if (!strcmp(gameDesc->gameId, "castaway")) {
		_isDemo = true;
		_gameId = GID_CASTAWAY;
	} else {
		error("Unknown game ID");
	}

	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "patches");
}

DgdsEngine::~DgdsEngine() {
	DebugMan.removeAllDebugChannels();

	delete _gamePals;
	delete _decompressor;
	delete _resource;
	delete _scene;
	delete _gdsScene;
	delete _soundPlayer;
	delete _fontManager;
	delete _menu;
	delete _inventory;
	delete _shellGame;
	delete _hocIntro;
	delete _dragonArcade;
	delete _chinaTank;
	delete _chinaTrain;

	_icons.reset();
	_corners.reset();

	_compositionBuffer.free();
	_storedAreaBuffer.free();
	_backgroundBuffer.free();
}


void DgdsEngine::loadCorners(const Common::String &filename) {
	_corners.reset(new Image(_resource, _decompressor));
	_corners->loadBitmap(filename);
}

void DgdsEngine::loadIcons() {
	const Common::String &iconFileName = _gdsScene->getIconFile();

	if (iconFileName.empty())
		return;

	_icons.reset(new Image(_resource, _decompressor));
	_icons->loadBitmap(iconFileName);
}

bool DgdsEngine::changeScene(int sceneNum) {
	assert(_scene && _adsInterp);

	debug("CHANGE SCENE %d -> %d (clock %s)", _scene->getNum(), sceneNum, _clock.dump().c_str());

	if (sceneNum == _scene->getNum()) {
		warning("Tried to change from scene %d to itself, doing nothing.", sceneNum);
		return false;
	}

	if (sceneNum != 2 && _scene->getNum() != 2 && _inventory->isOpen()) {
		// not going to or from inventory, ensure it's closed and clear drag item.
		_inventory->close();
		_scene->setDragItem(nullptr);
	}

	const Common::String sceneFile = Common::String::format("S%d.SDS", sceneNum);
	if (!_resource->hasResource(sceneFile)) {
		warning("Tried to switch to non-existent scene %d", sceneNum);
		return false;
	}

	_gameGlobals->setLastSceneNum(sceneNum);

	// Save the current foreground if we are going to the inventory,
	// *except* for HoC zoomed inventory - that also clears background
	// because it uses a different palette for the zoomed item view.
	if (sceneNum == 2 && (!(getGameId() == GID_HOC && _inventory->isZoomVisible()))) {
		// Force-draw the inv button here to keep it in background
		checkDrawInventoryButton();
		_backgroundBuffer.blitFrom(_compositionBuffer);
	} else {
		_backgroundBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
	}

	_scene->runLeaveSceneOps();

	// store the last non-inventory scene num
	if (_scene->getNum() != 2)
		_gameGlobals->setGlobal(0x61, _scene->getNum());

	_scene->unload();
	_backgroundFile.clear();
	_soundPlayer->stopAllSfx();

	_gdsScene->runChangeSceneOps();

	if (!_scene->getDragItem())
		setMouseCursor(_gdsScene->getDefaultMouseCursor());

	_storedAreaBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);

	_scene->load(sceneFile, _resource, _decompressor);
	// These are done inside the load function in the original.. cleaner here..
	if (!_isDemo)
		_scene->addInvButtonToHotAreaList();
	if (_gameId == GID_DRAGON)
		_clock.setVisibleScript(true);

	if (_scene->getMagic() != _gdsScene->getMagic())
		error("Scene %s magic does (0x%08x) not match GDS magic (0x%08x)", sceneFile.c_str(), _scene->getMagic(), _gdsScene->getMagic());

	Common::String adsFile = _scene->getAdsFile();
	adsFile.trim();

	if (!adsFile.empty())
		_adsInterp->load(adsFile);
	else
		_adsInterp->unload();

	debug("%s", _scene->dump("").c_str());
	_scene->runEnterSceneOps();

	_justChangedScene1 = true;
	_justChangedScene2 = true;

	return true;
}

void DgdsEngine::setMouseCursor(uint num) {
	if (!_icons || (int)num >= _icons->loadedFrameCount())
		return;

	if ((int)num == _currentCursor)
		return;

	const Common::Array<MouseCursor> &cursors = _gdsScene->getCursorList();

	if (num >= cursors.size())
		error("Not enough cursor info, need %d have %d", num, cursors.size());

	_currentCursorHot = cursors[num].getHot();

	/*
	// Adjust mouse location so hot pixel is in the same place as before?
	uint16 lastHotX = _currentCursor >= 0 ? cursors[_currentCursor]._hotX : 0;
	uint16 lastHotY = _currentCursor >= 0 ? cursors[_currentCursor]._hotY : 0;

	int16 newMouseX = _lastMouse.x - lastHotX + hotX;
	int16 newMouseY = _lastMouse.y - lastHotY + hotY;

	g_system->warpMouse(newMouseX, newMouseY);
	*/

	CursorMan.replaceCursor(*(_icons->getSurface(num)->surfacePtr()), _currentCursorHot.x, _currentCursorHot.y, 0, 0);
	CursorMan.showMouse(true);

	_currentCursor = num;
}

Common::Point DgdsEngine::getLastMouseMinusHot() const {
	return _lastMouse - _currentCursorHot;
}

void DgdsEngine::setShowClock(bool val) {
	_clock.setVisibleScript(val);
}

bool DgdsEngine::isInvButtonVisible() const {
	return (_gdsScene->getCursorList().size() >= 2 && _icons && _icons->loadedFrameCount() >= 2 &&
			!_scene->getHotAreas().empty() && _scene->getHotAreas().front()._num == 0);
}

void DgdsEngine::checkDrawInventoryButton() {
	if (!isInvButtonVisible())
		return;
	static const Common::Rect drawWin(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	int16 invButtonIcon = 2;
	if (getGameId() == GID_HOC) {
		static const byte HOC_INV_ICONS[] = { 0, 2, 18, 19 };
		invButtonIcon = HOC_INV_ICONS[_gdsScene->getGlobal(0x33)];

		// draw the swap char button if needed
		int16 otherChar = _gdsScene->getGlobal(0x34);
		if (otherChar) {
			// FIXME: This list repeated in scene too
			int16 swapCharIcon = HOC_CHAR_SWAP_ICONS[otherChar];
			int sy = SCREEN_HEIGHT - _icons->height(swapCharIcon) - 5;
			_icons->drawBitmap(swapCharIcon, 5, sy, drawWin, _compositionBuffer);
		}
	}

	int x = SCREEN_WIDTH - _icons->width(invButtonIcon) - 5;
	int y = SCREEN_HEIGHT - _icons->height(invButtonIcon) - 5;
	_icons->drawBitmap(invButtonIcon, x, y, drawWin, _compositionBuffer);
}

void DgdsEngine::init(bool restarting) {
	if (!restarting) {
		// Init things with no state only once
		initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);

		_console = new Console(this);
		_resource = new ResourceManager();
		_decompressor = new Decompressor();

		setDebugger(_console);
	} else {
		// Reset the stateful objects
		delete _gamePals;
		delete _soundPlayer;
		delete _scene;
		delete _gdsScene;
		delete _fontManager;
		delete _menu;
		delete _adsInterp;
		delete _inventory;
		delete _dragonArcade;
		delete _shellGame;
		delete _hocIntro;
		delete _chinaTank;
		delete _chinaTrain;
	}

	_gamePals = new GamePalettes(_resource, _decompressor);
	_soundPlayer = new Sound(_mixer, _resource, _decompressor);
	_scene = new SDSScene();
	_gdsScene = new GDSScene();
	_fontManager = new FontManager();
	_menu = new Menu();
	_adsInterp = new ADSInterpreter(this);
	_inventory = new Inventory();
	if (_gameId == GID_DRAGON)
		_dragonArcade = new DragonArcade();
	else if (_gameId == GID_HOC) {
		_shellGame = new ShellGame();
		_hocIntro = new HocIntro();
		_chinaTank = new ChinaTank();
		_chinaTrain = new ChinaTrain();
	}

	_backgroundBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_storedAreaBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_compositionBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	g_system->fillScreen(0);
}

void DgdsEngine::loadGameFiles() {
	REQFileData invRequestData;
	REQFileData vcrRequestData;
	RequestParser reqParser(_resource, _decompressor);

	_fontManager->loadFonts(getGameId(), _resource, _decompressor);

	switch (getGameId()) {
	case GID_DRAGON:
		_soundPlayer->loadSFX("SOUNDS.SNG");
		_gameGlobals = new DragonGlobals(_clock);
		_gamePals->loadPalette("DRAGON.PAL");
		_gdsScene->load("DRAGON.GDS", _resource, _decompressor);
		_rstFileName = "DRAGON.RST";

		debug("%s", _gdsScene->dump("").c_str());

		loadCorners("DCORNERS.BMP");
		reqParser.parse(&invRequestData, "DINV.REQ");
		reqParser.parse(&vcrRequestData, "DVCR.REQ");
		break;
	case GID_HOC:
		_soundPlayer->loadSFX("SOUNDS1.SNG");
		_gameGlobals = new HocGlobals(_clock);
		_gamePals->loadPalette("HOC.PAL");
		_gdsScene->load("HOC.GDS", _resource, _decompressor);
		_rstFileName = "HOC.RST";

		debug("%s", _gdsScene->dump("").c_str());

		loadCorners("HCORNERS.BMP");
		reqParser.parse(&invRequestData, "HINV.REQ");
		reqParser.parse(&vcrRequestData, "HVCR.REQ");
		break;
	case GID_WILLY:
		_gameGlobals = new WillyGlobals(_clock);
		if (_resource->hasResource("WILLY.GDS")) {
			_gdsScene->load("WILLY.GDS", _resource, _decompressor);
			_rstFileName = "WILLY.RST";
			_gamePals->loadPalette("WILLY.PAL");
			loadCorners("WCORNERS.BMP");
		} else {
			_gdsScene->load("SOWILLY.GDS", _resource, _decompressor);
			_rstFileName = "SOWILLY.RST";
			_gamePals->loadPalette("SOWILLY.PAL");
			loadCorners("SOWCORNERS.BMP");
		}

		debug("%s", _gdsScene->dump("").c_str());

		reqParser.parse(&invRequestData, "WINV.REQ");
		reqParser.parse(&vcrRequestData, "WVCR.REQ");

		break;
	case GID_QUARKY:
		_gameGlobals = new Globals(_clock);
		_gamePals->loadPalette("MRALLY.PAL");
		_gdsScene->load("MRALLY.GDS", _resource, _decompressor);

		debug("%s", _gdsScene->dump("").c_str());

		loadCorners("MCORNERS.BMP");
		reqParser.parse(&invRequestData, "TOOLINFO.REQ");
		reqParser.parse(&vcrRequestData, "MVCR.REQ");
		break;
	case GID_SQ5DEMO:
		_gameGlobals = new Globals(_clock);
		_gamePals->loadPalette("NORMAL.PAL");
		_adsInterp->load("CESDEMO.ADS");
		_adsInterp->segmentOrState(1, 3);
		break;
	case GID_COMINGATTRACTIONS:
		_gameGlobals = new Globals(_clock);
		_gamePals->loadPalette("DYNAMIX.PAL");
		_adsInterp->load("DEMO.ADS");
		_adsInterp->segmentOrState(1, 3);
		break;
	case GID_CASTAWAY:
		_gameGlobals = new Globals(_clock);
		_gamePals->loadPalette("JOHNCAST.PAL");
		_adsInterp->load("JOHNNY.ADS");
		_adsInterp->segmentOrState(1, 3);
		break;
	default:
		error("Unsupported game type in loadGameFiles");
	}

	_gdsScene->runStartGameOps();
	loadIcons();
	_gdsScene->initIconSizes();
	setMouseCursor(_gdsScene->getDefaultMouseCursor());

	_inventory->setRequestData(invRequestData);
	_menu->setRequestData(vcrRequestData);

	debug("Parsed Inv Request:\n%s", invRequestData.dump().c_str());
	debug("Parsed VCR Request:\n%s", vcrRequestData.dump().c_str());
}

void DgdsEngine::loadRestartFile() {
	if (!_rstFileName)
		error("Trying to restart game but no rst file name set!");

	_gdsScene->loadRestart(_rstFileName, _resource, _decompressor);
}

static void _dumpFrame(const Graphics::ManagedSurface &surf, const char *name) {
#ifdef DUMP_FRAME_DATA
	/* For debugging, dump the frame contents.. */
	Common::DumpFile outf;
	uint32 now = g_engine->getTotalPlayTime();

	byte palbuf[768];
	g_system->getPaletteManager()->grabPalette(palbuf, 0, 256);

	outf.open(Common::Path(Common::String::format("/tmp/%07d-%s.png", now, name)));
	// Operator magic - convert ManagedSurface reg to Surface ref.
	::Image::writePNG(outf, *(&surf), palbuf);
	outf.close();
#endif
}


Common::Error DgdsEngine::run() {
	syncSoundSettings();
	_isLoading = true;
	init(false);
	loadGameFiles();

	// If a savegame was selected from the launcher, load it now.
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		loadGameState(saveSlot);

	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Event ev;

	_isLoading = false;

	uint32 startMillis = g_system->getMillis();
	uint32 frameCount = 0;

	while (!shouldQuit()) {
		Common::EventType mouseEvent = Common::EVENT_INVALID;
		while (eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				switch ((DgdsKeyEvent)ev.customType) {
				case kDgdsKeyToggleMenu:
					_menuToTrigger = kMenuMain;
					break;
				case kDgdsKeySave:
					saveGameDialog();
					break;
				case kDgdsKeyLoad:
					loadGameDialog();
					break;
				case kDgdsKeyToggleClock:
					_clock.toggleVisibleUser();
					break;
				case kDgdsKeyNextChoice:
					if (_menu->menuShown())
						_menu->nextChoice();
					else if (_scene->hasVisibleDialog())
						_scene->nextChoice();
					break;
				case kDgdsKeyPrevChoice:
					if (_menu->menuShown())
						_menu->prevChoice();
					else if (_scene->hasVisibleDialog())
						_scene->prevChoice();
					break;
				case kDgdsKeyNextItem:
					warning("TODO: Implement kDgdsKeyNextItem");
					break;
				case kDgdsKeyPrevItem:
					warning("TODO: Implement kDgdsKeyPrevItem");
					break;
				case kDgdsKeyPickUp:
					if (_menu->menuShown())
						_menu->activateChoice();
					else if (_scene->hasVisibleDialog())
						_scene->activateChoice();
					else
						warning("TODO: Implement kDgdsKeyPickUp");
					break;
				case kDgdsKeyLook:
					if (_menu->menuShown())
						_menu->activateChoice();
					else if (_scene->hasVisibleDialog())
						_scene->activateChoice();
					else
						warning("TODO: Implement kDgdsKeyLook");
					break;
				case kDgdsKeyActivate:
					warning("TODO: Implement kDgdsKeyActivate");
					break;
				default:
					break;
				}
			} else if (ev.type == Common::EVENT_LBUTTONDOWN || ev.type == Common::EVENT_LBUTTONUP
					|| ev.type == Common::EVENT_RBUTTONDOWN || ev.type == Common::EVENT_RBUTTONUP
					|| ev.type == Common::EVENT_MOUSEMOVE) {
				mouseEvent = ev.type;
				_lastMouse = ev.mouse;
			} else if (ev.type == Common::EVENT_KEYDOWN) {
				if (_dragonArcade)
					_dragonArcade->onKeyDown(ev.kbd);
			} else if (ev.type == Common::EVENT_KEYUP) {
				if (_dragonArcade)
					_dragonArcade->onKeyUp(ev.kbd);
			}
		}

		if (_menuToTrigger != kMenuNone) {
			if (_inventory->isOpen()) {
				_inventory->close();
			} else if (!_menu->menuShown()) {
				_menu->setScreenBuffer();
				// force mouse on
				CursorMan.showMouse(true);
				setMouseCursor(_gdsScene->getDefaultMouseCursor());
				_menu->drawMenu(_menuToTrigger);
			} else {
				_menu->hideMenu();
			}

			_menuToTrigger = kMenuNone;
		}

		if (_menu->menuShown()) {
			switch (mouseEvent) {
				case Common::EVENT_LBUTTONUP:
					_menu->onMouseLUp(_lastMouse);
					break;
				case Common::EVENT_LBUTTONDOWN:
					_menu->onMouseLDown(_lastMouse);
					break;
				case Common::EVENT_MOUSEMOVE:
					_menu->onMouseMove(_lastMouse);
					break;
				default:
					break;
			}
			_clock.update(false);
		} else {

			_scene->checkForClearedDialogs();

			_gdsScene->runPreTickOps();
			_scene->runPreTickOps();

			_compositionBuffer.blitFrom(_backgroundBuffer);

			if (_inventory->isOpen() && _scene->getNum() == 2) {
				int invCount = _gdsScene->countItemsInScene2();
				_inventory->draw(_compositionBuffer, invCount);
			}

			_compositionBuffer.transBlitFrom(_storedAreaBuffer);

			//
			// The originals do something about drawing the background of dialogs here
			// but that causes graphical glitches in scenes when a region is saved as it
			// saves the dialog background too (eg, dragon scene 79).
			//
			// Instead we just draw the background and foreground of dialogs at the end.
			//
			//_scene->drawActiveDialogBgs(&_compositionBuffer);

			if (_scene->getNum() != 2 || _inventory->isZoomVisible())
				_adsInterp->run();

			if (mouseEvent != Common::EVENT_INVALID) {
				if (_inventory->isOpen()) {
					switch (mouseEvent) {
					case Common::EVENT_MOUSEMOVE:
						_inventory->mouseMoved(_lastMouse);
						break;
					case Common::EVENT_LBUTTONDOWN:
						_inventory->mouseLDown(_lastMouse);
						break;
					case Common::EVENT_LBUTTONUP:
						_inventory->mouseLUp(_lastMouse);
						break;
					case Common::EVENT_RBUTTONUP:
						_inventory->mouseRUp(_lastMouse);
						break;
					default:
						break;
					}
				} else {
					switch (mouseEvent) {
					case Common::EVENT_MOUSEMOVE:
						_scene->mouseMoved(_lastMouse);
						break;
					case Common::EVENT_LBUTTONDOWN:
						_scene->mouseLDown(_lastMouse);
						break;
					case Common::EVENT_LBUTTONUP:
						_scene->mouseLUp(_lastMouse);
						break;
					case Common::EVENT_RBUTTONDOWN:
						_scene->mouseRDown(_lastMouse);
						break;
					case Common::EVENT_RBUTTONUP:
						_scene->mouseRUp(_lastMouse);
						break;
					default:
						break;
					}
				}
			}

			// TODO: Hard-coded logic to match Rise of the Dragon, check others
			if (getGameId() != GID_DRAGON || _scene->getNum() != 55)
				_gdsScene->runPostTickOps();

			_scene->runPostTickOps();
			_scene->checkTriggers();

			_dumpFrame(_backgroundBuffer, "back");
			_dumpFrame(_storedAreaBuffer, "stor");
			_dumpFrame(_compositionBuffer, "comp");

			if (!_inventory->isOpen()) {
				_gdsScene->drawItems(_compositionBuffer);
				checkDrawInventoryButton();
			}

			if (getGameId() == GID_DRAGON)
				_clock.draw(_compositionBuffer);

			bool haveActiveDialog = _scene->checkDialogActive();

			_scene->drawAndUpdateDialogs(&_compositionBuffer);
			_scene->drawVisibleHeads(&_compositionBuffer);

			_dumpFrame(_compositionBuffer, "comp-with-dlg");

			bool gameRunning = (!haveActiveDialog && _gameGlobals->getGlobal(0x57) /* TODO: && _dragItem == nullptr*/);
			_clock.update(gameRunning);

			g_system->copyRectToScreen(_compositionBuffer.getPixels(), SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			_justChangedScene1 = false;
			_justChangedScene2 = false;
		}

		g_system->updateScreen();

		// Limit to 30 FPS
		const uint32 thisFrameMillis = g_system->getMillis();
		frameCount++;
		if (_skipNextFrame) {
			frameCount++;
			_skipNextFrame = false;
		}
		const uint32 elapsedMillis = thisFrameMillis - startMillis;
		const uint32 targetMillis = (frameCount * 1000 / 30);
		if (targetMillis > elapsedMillis) {
			// too fast, delay
			g_system->delayMillis(targetMillis - elapsedMillis);
		} else if (targetMillis < elapsedMillis) {
			// too slow.. adjust expectations? :)
			startMillis = g_system->getMillis();
			frameCount = 0;
		}

	}
	return Common::kNoError;
}

void DgdsEngine::restartGame() {
	_isLoading = true;
	init(true);
	loadGameFiles();
	loadRestartFile();
	_gameGlobals->setGlobal(0x57, 1);
	_isLoading = false;
}

Common::SeekableReadStream *DgdsEngine::getResource(const Common::String &name, bool ignorePatches) {
	return _resource->getResource(name, ignorePatches);
}


bool DgdsEngine::canLoadGameStateCurrently(Common::U32String *msg /*= nullptr*/) {
	return !_isDemo && _gdsScene != nullptr;
}


bool DgdsEngine::canSaveGameStateCurrently(Common::U32String *msg /*= nullptr*/) {
	// The demos are all non-interactive, so it doesn't make sense to save them.
	return !_isDemo && _gdsScene && _scene && _scene->getNum() != 2
			&& _scene->getDragItem() == nullptr && !_isLoading;
}

bool DgdsEngine::canSaveAutosaveCurrently() {
	return canSaveGameStateCurrently() && !_scene->hasVisibleDialog() && !_menu->menuShown() && _gameGlobals->getGameIsInteractiveGlobal();
}

void DgdsEngine::enableKeymapper() {
	_eventMan->getKeymapper()->setEnabledKeymapType(Common::Keymap::kKeymapTypeGame);
}

void DgdsEngine::disableKeymapper() {
	// Don't totally disable keymapper, as we still want the console and screenshot keys to work.
	_eventMan->getKeymapper()->setEnabledKeymapType(Common::Keymap::kKeymapTypeGui);
}

Common::Error DgdsEngine::syncGame(Common::Serializer &s) {
	//
	// Version history:
	//
	// 1: First version
	// 2: Added GameItem.flags
	// 3: Stopped saving ADS/TTM state
	// 4: Stopped saving palette state
	//

	assert(_scene && _gdsScene);

	_menu->hideMenu();

	if (!s.syncVersion(4))
		error("Save game version too new: %d", s.getVersion());

	Common::Error result;

	result = _gdsScene->syncState(s);
	if (result.getCode() != Common::kNoError) return result;

	int sceneNum = _scene->getNum();
	s.syncAsUint16LE(sceneNum);
	if (s.isLoading()) {
		// load and prepare scene data before syncing the rest of the state
		const Common::String sceneFile = Common::String::format("S%d.SDS", sceneNum);
		if (!_resource->hasResource(sceneFile))
			error("Game references non-existent scene %d", sceneNum);

		_soundPlayer->stopAllSfx();
		_scene->unload();
		_adsInterp->unload();

		// Clear arcade state completely.
		if (getGameId() == GID_DRAGON) {
			delete _dragonArcade;
			_dragonArcade = new DragonArcade();
		}

		_scene->load(sceneFile, _resource, _decompressor);
	}

	result = _scene->syncState(s);
	if (result.getCode() != Common::kNoError) return result;

	result = _gameGlobals->syncState(s);
	if (result.getCode() != Common::kNoError) return result;

	result = _clock.syncState(s);
	if (result.getCode() != Common::kNoError) return result;

	result = _inventory->syncState(s);
	if (result.getCode() != Common::kNoError) return result;

	// Add inv button - we deferred this to now to make sure globals etc
	// are in the right state.
	if (s.isLoading())
		_scene->addInvButtonToHotAreaList();

	if (s.getVersion() < 4) {
		result = _gamePals->syncState(s);
		if (result.getCode() != Common::kNoError) return result;
	} else if (s.isLoading()) {
		_gamePals->reset();
	}

	result = _adsInterp->syncState(s);
	if (result.getCode() != Common::kNoError) return result;

	s.syncAsSint16LE(_textSpeed);
	s.syncAsByte(_justChangedScene1);
	s.syncAsByte(_justChangedScene2);

	// sync engine play time to ensure various events run correctly.
	uint32 playtime = g_engine->getTotalPlayTime();
	s.syncAsUint32LE(playtime);
	g_engine->setTotalPlayTime(playtime);

	s.syncString(_backgroundFile);
	if (s.isLoading()) {
		Image(_resource, _decompressor).drawScreen(_backgroundFile, _backgroundBuffer);
		_storedAreaBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
	}

	debug("%s", _scene->dump("").c_str());
	_scene->runEnterSceneOps();

	return Common::kNoError;
}

} // End of namespace Dgds
