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

#include "common/formats/iff_container.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "dgds/console.h"
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

// for frame contents debugging
//#define DUMP_FRAME_DATA 1

#ifdef DUMP_FRAME_DATA
#include "graphics/paletteman.h"
#include "image/png.h"
#endif

namespace Dgds {

DgdsEngine::DgdsEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _fontManager(nullptr), _console(nullptr), _inventory(nullptr),
	_soundPlayer(nullptr), _decompressor(nullptr), _scene(nullptr),
	_gdsScene(nullptr), _resource(nullptr), _gamePals(nullptr), _gameGlobals(nullptr),
	_detailLevel(kDgdsDetailHigh), _textSpeed(1), _justChangedScene1(false), _justChangedScene2(false),
	_random("dgds"), _currentCursor(-1), _menuToTrigger(kMenuNone) {
	syncSoundSettings();

	_platform = gameDesc->platform;

	if (!strcmp(gameDesc->gameId, "rise"))
		_gameId = GID_DRAGON;
	else if (!strcmp(gameDesc->gameId, "china"))
		_gameId = GID_CHINA;
	else if (!strcmp(gameDesc->gameId, "beamish"))
		_gameId = GID_BEAMISH;
	else
		error("Unknown game ID");

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

	_icons.reset();
	_corners.reset();

	_compositionBuffer.free();
	_foregroundBuffer.free();
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
		warning("Tried to switch to non-existant scene %d", sceneNum);
		return false;
	}

	// Save the current foreground if we are going to the inventory, clear it otherwise.
	if (sceneNum == 2)
		_backgroundBuffer.blitFrom(_compositionBuffer);
	else
		_backgroundBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);

	_scene->runLeaveSceneOps();

	// store the last scene num
	_gameGlobals->setGlobal(0x61, _scene->getNum());

	_scene->unload();
	_backgroundFile.clear();
	_soundPlayer->unloadMusic();

	_gdsScene->runChangeSceneOps();

	if (!_scene->getDragItem())
		setMouseCursor(0);

	_foregroundBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
	_storedAreaBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);

	_scene->load(sceneFile, _resource, _decompressor);
	// These are done inside the load function in the original.. cleaner here..
	_scene->addInvButtonToHotAreaList();

	if (_scene->getMagic() != _gdsScene->getMagic())
		error("Scene %s magic does (0x%08x) not match GDS magic (0x%08x)", sceneFile.c_str(), _scene->getMagic(), _gdsScene->getMagic());

	if (!_scene->getAdsFile().empty())
		_adsInterp->load(_scene->getAdsFile());
	else
		_adsInterp->unload();

	_scene->runEnterSceneOps();
	debug("%s", _scene->dump("").c_str());

	_justChangedScene1 = true;
	_justChangedScene2 = true;

	return true;
}

void DgdsEngine::setMouseCursor(uint num) {
	if (!_icons || (int)num >= _icons->loadedFrameCount())
		return;

	if ((int)num == _currentCursor)
		return;

	// TODO: Get mouse cursors from _gdsScene for hotspot info??
	const Common::Array<MouseCursor> &cursors = _gdsScene->getCursorList();

	if (num >= cursors.size())
		error("Not enough cursor info, need %d have %d", num, cursors.size());

	uint16 hotX = cursors[num]._hotX;
	uint16 hotY = cursors[num]._hotY;

	/*
	// Adjust mouse location so hot pixel is in the same place as before?
	uint16 lastHotX = _currentCursor >= 0 ? cursors[_currentCursor]._hotX : 0;
	uint16 lastHotY = _currentCursor >= 0 ? cursors[_currentCursor]._hotY : 0;

	int16 newMouseX = _lastMouse.x - lastHotX + hotX;
	int16 newMouseY = _lastMouse.y - lastHotY + hotY;

	g_system->warpMouse(newMouseX, newMouseY);
	*/

	CursorMan.replaceCursor(*(_icons->getSurface(num)->surfacePtr()), hotX, hotY, 0, 0);
	CursorMan.showMouse(true);

	_currentCursor = num;
}

void DgdsEngine::setShowClock(bool val) {
	_clock.setVisibleScript(val);
}

void DgdsEngine::checkDrawInventoryButton() {
	if (_gdsScene->getCursorList().size() < 2 || _icons->loadedFrameCount() < 2 ||
			_scene->getHotAreas().size() < 1 || _scene->getHotAreas()[0]._num != 0)
		return;

	int x = SCREEN_WIDTH - _icons->width(2) - 5;
	int y = SCREEN_HEIGHT - _icons->height(2) - 5;
	static const Common::Rect drawWin(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_icons->drawBitmap(2, x, y, drawWin, _compositionBuffer);
}

void DgdsEngine::init() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);

	_console = new Console(this);
	_resource = new ResourceManager();
	_decompressor = new Decompressor();
	_gamePals = new GamePalettes(_resource, _decompressor);
	_soundPlayer = new Sound(_mixer, _resource, _decompressor);
	_scene = new SDSScene();
	_gdsScene = new GDSScene();
	_fontManager = new FontManager();
	_menu = new Menu();
	_adsInterp = new ADSInterpreter(this);
	_inventory = new Inventory();

	setDebugger(_console);

	_backgroundBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_storedAreaBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_foregroundBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_compositionBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	g_system->fillScreen(0);
}

void DgdsEngine::loadGameFiles() {
	REQFileData invRequestData;
	REQFileData vcrRequestData;
	RequestParser reqParser(_resource, _decompressor);

	_fontManager->loadFonts(getGameId(), _resource, _decompressor);

	if (getGameId() == GID_DRAGON) {
		_soundPlayer->loadSFX("SOUNDS.SNG");
		_gameGlobals = new DragonGlobals(_clock);
		_gamePals->loadPalette("DRAGON.PAL");
		_gdsScene->load("DRAGON.GDS", _resource, _decompressor);

		debug("%s", _gdsScene->dump("").c_str());

		loadCorners("DCORNERS.BMP");
		reqParser.parse(&invRequestData, "DINV.REQ");
		reqParser.parse(&vcrRequestData, "DVCR.REQ");
	} else if (getGameId() == GID_CHINA) {
		// TODO: Create a better type for this..
		_gameGlobals = new DragonGlobals(_clock);
		_gamePals->loadPalette("HOC.PAL");
		_gdsScene->load("HOC.GDS", _resource, _decompressor);

		//debug("%s", _gdsScene->dump("").c_str());

		loadCorners("HCORNERS.BMP");
		reqParser.parse(&invRequestData, "HINV.REQ");
		reqParser.parse(&vcrRequestData, "HVCR.REQ");
	} else if (getGameId() == GID_BEAMISH) {
		// TODO: Create a better type for this..
		_gameGlobals = new DragonGlobals(_clock);
		_gamePals->loadPalette("WILLY.PAL");
		// TODO: This doesn't parse correctly yet.
		//_gdsScene->load("WILLY.GDS", _resource, _decompressor);

		loadCorners("WCORNERS.BMP");
		reqParser.parse(&invRequestData, "WINV.REQ");
		reqParser.parse(&vcrRequestData, "WVCR.REQ");

		//_scene->load("S34.SDS", _resource, _decompressor);
		_adsInterp->load("TITLE.ADS");
	}

	_gdsScene->runStartGameOps();
	loadIcons();
	setMouseCursor(0);

	_inventory->setRequestData(invRequestData);
	_menu->setRequestData(vcrRequestData);

	//getDebugger()->attach();

	debug("Parsed Inv Request:\n%s", invRequestData.dump().c_str());
	debug("Parsed VCR Request:\n%s", vcrRequestData.dump().c_str());
}

Common::Error DgdsEngine::run() {
	init();
	loadGameFiles();

	// changeScene(55); // to test DRAGON intro sequence (after credits)

	// If a savegame was selected from the launcher, load it now.
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		loadGameState(saveSlot);

	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Event ev;

	while (!shouldQuit()) {
		Common::EventType mouseEvent = Common::EVENT_INVALID;
		while (eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					_menuToTrigger = kMenuMain;
					break;
				case Common::KEYCODE_F5:
					_menuToTrigger = kMenuMain;
					break;
				case Common::KEYCODE_s:
					if (ev.kbd.hasFlags(Common::KBD_CTRL))
						saveGameDialog();
					break;
				case Common::KEYCODE_l:
					if (ev.kbd.hasFlags(Common::KBD_CTRL))
						loadGameDialog();
					break;
				case Common::KEYCODE_c:
					_clock.toggleVisibleUser();
					break;
				default:
					break;
				}
			} else if (ev.type == Common::EVENT_LBUTTONDOWN || ev.type == Common::EVENT_LBUTTONUP
					|| ev.type == Common::EVENT_RBUTTONUP || ev.type == Common::EVENT_MOUSEMOVE) {
				mouseEvent = ev.type;
				_lastMouse = ev.mouse;
			}
		}

		if (_menuToTrigger != kMenuNone) {
			if (_inventory->isOpen()) {
				_inventory->close();
			} else if (!_menu->menuShown()) {
				_menu->setScreenBuffer();
				// force mouse on
				CursorMan.showMouse(true);
				setMouseCursor(0);
				_menu->drawMenu(_menuToTrigger);
			} else {
				_menu->hideMenu();
			}

			_menuToTrigger = kMenuNone;
		}

		if (_menu->menuShown()) {
			if (mouseEvent == Common::EVENT_LBUTTONUP) {
				_menu->handleMenu(_lastMouse);
				mouseEvent = Common::EVENT_INVALID;
			}
			g_system->updateScreen();
			g_system->delayMillis(10);
			continue;
		}

		if (getGameId() == GID_DRAGON || getGameId() == GID_CHINA) {
			_scene->checkForClearedDialogs();

			_gdsScene->runPreTickOps();
			_scene->runPreTickOps();

			_scene->drawActiveDialogBgs(&_compositionBuffer);

			if (!_inventory->isOpen() || _inventory->isZoomVisible())
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

			// Now we start to assemble the rendered scene.
			_compositionBuffer.blitFrom(_backgroundBuffer);

			if (_inventory->isOpen()) {
				int invCount = _gdsScene->countItemsInScene2();
				_inventory->draw(_compositionBuffer, invCount);
			}

			_compositionBuffer.transBlitFrom(_storedAreaBuffer);
			_compositionBuffer.transBlitFrom(_foregroundBuffer);

#ifdef DUMP_FRAME_DATA
			/* For debugging, dump the frame contents.. */
			{
				Common::DumpFile outf;
				uint32 now = g_engine->getTotalPlayTime();

				byte palbuf[768];
				g_system->getPaletteManager()->grabPalette(palbuf, 0, 256);

				outf.open(Common::Path(Common::String::format("/tmp/%07d-back.png", now)));
				::Image::writePNG(outf, *_backgroundBuffer.surfacePtr(), palbuf);
				outf.close();

				outf.open(Common::Path(Common::String::format("/tmp/%07d-fore.png", now)));
				::Image::writePNG(outf, *_foregroundBuffer.surfacePtr(), palbuf);
				outf.close();

				outf.open(Common::Path(Common::String::format("/tmp/%07d-stor.png", now)));
				::Image::writePNG(outf, *_storedAreaBuffer.surfacePtr(), palbuf);
				outf.close();

				outf.open(Common::Path(Common::String::format("/tmp/%07d-comp.png", now)));
				::Image::writePNG(outf, *_compositionBuffer.surfacePtr(), palbuf);
				outf.close();
			}
#endif

			_foregroundBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);

			if (!_inventory->isOpen()) {
				_gdsScene->drawItems(_compositionBuffer);
				checkDrawInventoryButton();
			}
			_clock.draw(_compositionBuffer);
			bool haveActiveDialog = _scene->checkDialogActive();

			_scene->drawAndUpdateDialogs(&_compositionBuffer);

			bool gameRunning = (!haveActiveDialog && _gameGlobals->getGlobal(0x57) /* TODO: && _dragItem == nullptr*/);
			_clock.update(gameRunning);

		} else if (getGameId() == GID_BEAMISH) {
			if (!_adsInterp->run())
				return Common::kNoError;
		}

		g_system->copyRectToScreen(_compositionBuffer.getPixels(), SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		g_system->updateScreen();
		g_system->delayMillis(10);

		_justChangedScene1 = false;
		_justChangedScene2 = false;
	}
	return Common::kNoError;
}

Common::SeekableReadStream *DgdsEngine::getResource(const Common::String &name, bool ignorePatches) {
	return _resource->getResource(name, ignorePatches);
}


bool DgdsEngine::canLoadGameStateCurrently(Common::U32String *msg /*= nullptr*/) {
	return _gdsScene != nullptr;
}


bool DgdsEngine::canSaveGameStateCurrently(Common::U32String *msg /*= nullptr*/) {
	return _gdsScene && _scene && _scene->getNum() != 2
			&& !_scene->hasVisibleDialog() && !_menu->menuShown()
			&& _scene->getDragItem() == nullptr;
}


Common::Error DgdsEngine::syncGame(Common::Serializer &s) {
	//
	// Version history:
	//
	// 1: First version
	//

	assert(_scene && _gdsScene);

	if (!s.syncVersion(1))
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
			error("Game references non-existant scene %d", sceneNum);

		_scene->unload();
		_adsInterp->unload();
		_scene->load(sceneFile, _resource, _decompressor);
		_scene->addInvButtonToHotAreaList();
	}

	result = _scene->syncState(s);
	if (result.getCode() != Common::kNoError) return result;

	result = _gameGlobals->syncState(s);
	if (result.getCode() != Common::kNoError) return result;

	result = _clock.syncState(s);
	if (result.getCode() != Common::kNoError) return result;

	result = _inventory->syncState(s);
	if (result.getCode() != Common::kNoError) return result;

	result = _gamePals->syncState(s);
	if (result.getCode() != Common::kNoError) return result;

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
		_foregroundBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		_storedAreaBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
	}

	return Common::kNoError;
}

} // End of namespace Dgds
