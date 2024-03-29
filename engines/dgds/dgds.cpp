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
#include "dgds/menu.h"
#include "dgds/parser.h"
#include "dgds/request.h"
#include "dgds/resource.h"
#include "dgds/scripts.h"
#include "dgds/sound.h"

namespace Dgds {

DgdsEngine::DgdsEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _fontManager(nullptr), _console(nullptr),
	_soundPlayer(nullptr), _decompressor(nullptr), _scene(nullptr),
	_gdsScene(nullptr), _resource(nullptr), _gamePals(nullptr), _gameGlobals(nullptr),
	_detailLevel(kDgdsDetailHigh), _showClockUser(false), _showClockScript(false),
	_textSpeed(1), _justChangedScene1(false), _justChangedScene2(false), _random("dgds") {
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

	_icons.reset();
	_corners.reset();

	_resData.free();
	_topBuffer.free();
	_bottomBuffer.free();
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

bool DgdsEngine::changeScene(int sceneNum, bool runChangeOps) {
	assert(_scene && _adsInterp);

	if (sceneNum == _scene->getNum()) {
		warning("Tried to change from scene %d to itself, doing nothing.", sceneNum);
		return false;
	}

	const Common::String sceneFile = Common::String::format("S%d.SDS", sceneNum);
	if (!_resource->hasResource(sceneFile)) {
		warning("Tried to switch to non-existant scene %d", sceneNum);
		return false;
	}

	_scene->runLeaveSceneOps();

	// store the last scene num
	_gameGlobals->setGlobal(0x61, _scene->getNum());

	_scene->unload();
	_soundPlayer->unloadMusic();

	_gdsScene->runChangeSceneOps();

	setMouseCursor(0);

	_scene->load(sceneFile, _resource, _decompressor);

	if (_scene->getMagic() != _gdsScene->getMagic())
		error("Scene %s magic does (0x%08x) not match GDS magic (0x%08x)", sceneFile.c_str(), _scene->getMagic(), _gdsScene->getMagic());

	if (!_scene->getAdsFile().empty())
		_adsInterp->load(_scene->getAdsFile());

	_scene->runEnterSceneOps();
	debug("%s", _scene->dump("").c_str());

	_justChangedScene1 = true;
	_justChangedScene2 = true;

	return true;
}

void DgdsEngine::setMouseCursor(uint num) {
	if (!_icons || (int)num >= _icons->loadedFrameCount())
		return;

	// TODO: Get mouse cursors from _gdsScene for hotspot info??
	CursorMan.popAllCursors();
	CursorMan.pushCursor(*(_icons->getSurface(num)->surfacePtr()), 0, 0, 0, 0);
	CursorMan.showMouse(true);
}

Common::Error DgdsEngine::run() {
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

	setDebugger(_console);

	_bottomBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_topBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_resData.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	g_system->fillScreen(0);

	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Event ev;

	REQFileData invRequestData;
	REQFileData vcrRequestData;
	RequestParser reqParser(_resource, _decompressor);

	_fontManager->loadFonts(getGameId(), _resource, _decompressor);

	if (getGameId() == GID_DRAGON) {
		_soundPlayer->loadSFX("SOUNDS.SNG");
		_gameGlobals = new DragonGlobals();
		_showClockUser = true;
		_gamePals->loadPalette("DRAGON.PAL");
		_gdsScene->load("DRAGON.GDS", _resource, _decompressor);

		debug("%s", _gdsScene->dump("").c_str());

		loadCorners("DCORNERS.BMP");
		reqParser.parse(&invRequestData, "DINV.REQ");
		reqParser.parse(&vcrRequestData, "DVCR.REQ");

		_gdsScene->runStartGameOps();

		// To skip credits for testing
		changeScene(6, true);

	} else if (getGameId() == GID_CHINA) {
		_gameGlobals = new Globals();
		_gamePals->loadPalette("HOC.PAL");
		_gdsScene->load("HOC.GDS", _resource, _decompressor);

		//debug("%s", _gdsScene->dump("").c_str());

		reqParser.parse(&invRequestData, "HINV.REQ");
		reqParser.parse(&vcrRequestData, "HVCR.REQ");

		//_scene->load("S101.SDS", _resource, _decompressor);
		//_adsInterp->load("TITLE.ADS");
		_gdsScene->runStartGameOps();

		loadCorners("HCORNERS.BMP");
	} else if (getGameId() == GID_BEAMISH) {
		_gameGlobals = new Globals();
		_gamePals->loadPalette("WILLY.PAL");
		// TODO: This doesn't parse correctly yet.
		//_gdsScene->load("WILLY.GDS", _resource, _decompressor);

		reqParser.parse(&invRequestData, "WINV.REQ");
		reqParser.parse(&vcrRequestData, "WVCR.REQ");

		//_scene->load("S34.SDS", _resource, _decompressor);
		_adsInterp->load("TITLE.ADS");
		loadCorners("WCORNERS.BMP");
	}

	loadIcons();
	setMouseCursor(0);

	//getDebugger()->attach();

	//debug("Parsed Inv Request:\n%s", invRequestData.dump().c_str());
	//debug("Parsed VCR Request:\n%s", vcrRequestData.dump().c_str());

	bool moveToNext = false;
	bool triggerMenu = false;
	bool mouseLClicked = false;
	bool mouseRClicked = false;
	bool mouseMoved = false;

	while (!shouldQuit()) {
		while (eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					if (_menu->menuShown())
						triggerMenu = true;
					else
						moveToNext = true;
					break;
				case Common::KEYCODE_F5:
					triggerMenu = true;
					break;
				default:
					break;
				}
			} else if (ev.type == Common::EVENT_LBUTTONUP) {
				mouseLClicked = true;
				_lastMouse = ev.mouse;
			} else if (ev.type == Common::EVENT_RBUTTONUP) {
				mouseRClicked = true;
				_lastMouse = ev.mouse;
			} else if (ev.type == Common::EVENT_MOUSEMOVE) {
				mouseMoved = true;
				_lastMouse = ev.mouse;
			}
		}

		if (triggerMenu) {
			if (!_menu->menuShown()) {
				_menu->setScreenBuffer();

				CursorMan.showMouse(true);
				_menu->drawMenu(vcrRequestData);
			} else {
				_menu->hideMenu();
				CursorMan.showMouse(false);
			}

			triggerMenu = false;
		}

		if (_menu->menuShown()) {
			if (mouseLClicked) {
				_menu->handleMenu(vcrRequestData, _lastMouse);
				mouseLClicked = false;
			}
			g_system->updateScreen();
			g_system->delayMillis(10);
			continue;
		}

		if (getGameId() == GID_DRAGON || getGameId() == GID_CHINA) {
			_gdsScene->runPreTickOps();
			_scene->runPreTickOps();

			_scene->drawActiveDialogBgs(&_resData);

			if (moveToNext || !_adsInterp->run()) {
				moveToNext = false;
			}

			if (mouseMoved) {
				_scene->mouseMoved(_lastMouse);
				mouseMoved = false;
			} else if (mouseLClicked) {
				_scene->mouseLClicked(_lastMouse);
				mouseLClicked = false;
			} else if (mouseRClicked) {
				_scene->mouseRClicked(_lastMouse);
				mouseRClicked = false;
			}

			// Note: Hard-coded logic for DRAGON, check others
			//if (getGameId() != GID_DRAGON || _scene->getNum() != 55)
			//	_gdsScene->runPostTickOps();

			_scene->runPostTickOps();
			_scene->checkTriggers();
			_scene->checkDialogActive();

			_scene->drawAndUpdateDialogs(&_resData);
		} else if (getGameId() == GID_BEAMISH) {
			if (!_adsInterp->run())
				return Common::kNoError;
		}

		g_system->copyRectToScreen(_resData.getPixels(), SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
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



// Parts of the old parse file code, kept temporarily for reference
#if 0
void DgdsEngine::parseFileInner(Common::Platform platform, Common::SeekableReadStream &file, const char *name) {
	const char *dot;
	DGDS_EX ex = 0;

	if ((dot = strrchr(name, '.'))) {
		ex = MKTAG24(dot[1], dot[2], dot[3]);
	}

	uint parent = 0;

	if (ex == EX_VIN || ex == EX_AMG) {
		Common::String line = file.readLine();
		while (!file.eos()) {
			if (!line.empty())
				debug("    \"%s\"", line.c_str());
			line = file.readLine();
		}
	} else {
		DgdsChunkReader chunk(&file);
		while (chunk.readNextHeader(ex, name)) {
			if (chunk.isContainer()) {
				parent = chunk.getId();
				continue;
			}

			chunk.readContent(_decompressor);
			Common::SeekableReadStream *stream = chunk.getContent();

			switch (ex) {
			case EX_TDS:
				/* Heart of China. */
				if (chunk.isSection(ID_THD)) {
					uint32 mark = stream->readUint32LE();
					debug("    0x%X", mark);

					char version[7];
					stream->read(version, sizeof(version));
					debug("    \"%s\"", version);

					byte ch;
					Common::String bmpName;
					while ((ch = stream->readByte()))
						bmpName += ch;
					debug("    \"%s\"", bmpName.c_str());

					Common::String personName;
					while ((ch = stream->readByte()))
						personName += ch;
					debug("    \"%s\"", personName.c_str());
				}
				break;
			case EX_DDS:
				/* Heart of China. */
				if (chunk.isSection(ID_DDS)) {
					uint32 mark;

					mark = stream->readUint32LE();
					debug("    0x%X", mark);

					char version[7];
					stream->read(version, sizeof(version));
					debug("    \"%s\"", version);

					byte ch;
					Common::String tag;
					while ((ch = stream->readByte()))
						tag += ch;
					debug("    \"%s\"", tag.c_str());
				}
				break;
			default:
				break;
			}
		}
	}
}
#endif

} // End of namespace Dgds
