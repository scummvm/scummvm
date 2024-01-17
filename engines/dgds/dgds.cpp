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
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/parser.h"
#include "dgds/request.h"
#include "dgds/resource.h"
#include "dgds/scripts.h"
#include "dgds/sound.h"

namespace Dgds {

enum MenuIds {
	kMenuNone = -1,
	kMenuMain = 0,
	kMenuControls = 1,
	kMenuOptions = 2,
	kMenuCalibrate = 3,
	kMenuRestart = 4,
	// 5: you cannot save your game right now
	// 6: game over
	kMenuFiles = 7,
	// 8: save game not saved because disk is full
	// 9: all game entries are full
	kMenuSave = 10,
	// 11: change directory - create directory
	// 12: change directory - invalid directory specified
	kMenuChangeDirectory = 13,
	kMenuJoystick = 14,
	kMenuMouse = 15,
	kMenuQuit = 16
	// 17: I'm frustrated - keep trying / win arcade
	// 18: skip introduction / play introduction
	// 19: save game before arcade
	// 20: replay arcade
};

enum MenuButtonIds {
	kMenuMainPlay = 120,
	kMenuMainControls = 20,
	kMenuMainOptions = 121,
	kMenuMainCalibrate = 118,
	kMenuMainFiles = 119,
	kMenuMainQuit = 122,

	kMenuControlsVCR = 127,
	kMenuControlsPlay = 128,

	kMenuOptionsJoystickOnOff = 139,
	kMenuOptionsMouseOnOff = 138,
	kMenuOptionsSoundsOnOff = 137,
	kMenuOptionsMusicOnOff = 140,
	kMenuOptionsVCR = 135,
	kMenuOptionsPlay = 136,

	kMenuCalibrateJoystick = 145,
	kMenuCalibrateMouse = 146,
	kMenuCalibrateVCR = 144,
	kMenuCalibratePlay = 147,

	kMenuFilesSave = 107,
	kMenuFilesRestore = 106,
	kMenuFilesRestart = 105,
	kMenuFilesVCR = 103,
	kMenuFilesPlay = 130,

	kMenuSavePrevious = 58,
	kMenuSaveNext = 59,
	kMenuSaveSave = 53,
	kMenuSaveCancel = 54,
	kMenuSaveChangeDirectory = 55,

	kMenuChangeDirectoryOK = 95,
	kMenuChangeDirectoryCancel = 96,

	kMenuMouseCalibrationCalibrate = 157,
	kMenuMouseCalibrationPlay = 155,

	kMenuJoystickCalibrationOK = 132,

	kMenuQuitYes = 134,
	kMenuQuitNo = 133,

	kMenuRestartYes = 163,
	kMenuRestartNo = 164
};

DgdsEngine::DgdsEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _image(nullptr), _fontManager(nullptr), _console(nullptr),
	_soundPlayer(nullptr), _decompressor(nullptr), _scene(nullptr), _gdsScene(nullptr),
	_resource(nullptr) {
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

	delete _image;
	delete _decompressor;
	delete _resource;
	delete _scene;
	delete _gdsScene;
	delete _soundPlayer;
	delete _fontManager;

	_screenBuffer.free();
	_resData.free();
	_topBuffer.free();
	_bottomBuffer.free();
}

void readStrings(Common::SeekableReadStream *stream) {
	uint16 count = stream->readUint16LE();
	debug("        %u:", count);

	for (uint16 k = 0; k < count; k++) {
		byte ch;
		uint16 idx = stream->readUint16LE();

		Common::String str;
		while ((ch = stream->readByte()))
			str += ch;
		debug("        %2u: %2u, \"%s\"", k, idx, str.c_str());
	}
}

void DgdsEngine::drawMenu(REQFileData &vcrRequestData, int16 menu) {
	_curMenu = menu;

	Common::Array<Common::SharedPtr<Gadget> > gadgets = vcrRequestData._requests[_curMenu]._gadgets;
	Graphics::Surface *dst = g_system->lockScreen();

	// Restore background when drawing submenus
	dst->copyFrom(_screenBuffer);

	vcrRequestData._requests[_curMenu].draw(dst);

	for (Common::SharedPtr<Gadget> &gptr : gadgets) {
		Gadget *gadget = gptr.get();
		if (gadget->_gadgetType == kGadgetButton || gadget->_gadgetType == kGadgetSlider)
			gadget->draw(dst);
	}

	g_system->unlockScreen();
	g_system->updateScreen();
}

int16 DgdsEngine::getClickedMenuItem(REQFileData& vcrRequestData, Common::Point mouseClick) {
	Common::Array<Common::SharedPtr<Gadget> > gadgets = vcrRequestData._requests[_curMenu]._gadgets;

	for (Common::SharedPtr<Gadget> &gptr : gadgets) {
		Gadget *gadget = gptr.get();
		if (gadget->_gadgetType == kGadgetButton || gadget->_gadgetType == kGadgetSlider) {
			int16 x = gadget->_x + gadget->_parentX;
			int16 y = gadget->_y + gadget->_parentY;
			int16 right = x + gadget->_width;
			int16 bottom = (y + gadget->_height) - 1;
			Common::Rect gadgetRect(x, y, right, bottom);
			if (gadgetRect.contains(mouseClick))
				return (int16)gadget->_gadgetNo;
		}
	}

	return -1;
}

void DgdsEngine::handleMenu(REQFileData &vcrRequestData, Common::Point &mouse) {
	const int16 clickedMenuItem = getClickedMenuItem(vcrRequestData, mouse);
	switch (clickedMenuItem) {
	case kMenuMainPlay:
	case kMenuControlsPlay:
	case kMenuOptionsPlay:
	case kMenuCalibratePlay:
	case kMenuFilesPlay:
	case kMenuMouseCalibrationPlay:
		_curMenu = kMenuNone;
		CursorMan.showMouse(false);
		break;
	case kMenuMainControls:
		drawMenu(vcrRequestData, kMenuControls);
		break;
	case kMenuMainOptions:
		drawMenu(vcrRequestData, kMenuOptions);
		break;
	case kMenuMainCalibrate:
	case kMenuJoystickCalibrationOK:
	case kMenuMouseCalibrationCalibrate:
		drawMenu(vcrRequestData, kMenuCalibrate);
		break;
	case kMenuMainFiles:
	case kMenuSaveCancel:
		drawMenu(vcrRequestData, kMenuFiles);
		break;
	case kMenuMainQuit:
		drawMenu(vcrRequestData, kMenuQuit);
		break;
	case kMenuControlsVCR:
	case kMenuOptionsVCR:
	case kMenuCalibrateVCR:
	case kMenuFilesVCR:
	case kMenuQuitNo:
	case kMenuRestartNo:
		drawMenu(vcrRequestData, kMenuMain);
		break;
	case kMenuOptionsJoystickOnOff:
	case kMenuOptionsMouseOnOff:
	case kMenuOptionsSoundsOnOff:
	case kMenuOptionsMusicOnOff:
		// TODO
		debug("Clicked option with ID %d", clickedMenuItem);
		break;
	case kMenuCalibrateJoystick:
		drawMenu(vcrRequestData, kMenuJoystick);
		break;
	case kMenuCalibrateMouse:
		drawMenu(vcrRequestData, kMenuMouse);
		break;
	case kMenuFilesSave:
	case kMenuChangeDirectoryCancel:
		drawMenu(vcrRequestData, kMenuSave);
		break;
	case kMenuFilesRestore:
		// TODO
		debug("Clicked Files - Restore %d", clickedMenuItem);
		break;
	case kMenuFilesRestart:
		drawMenu(vcrRequestData, kMenuRestart);
		break;
	case kMenuSavePrevious:
	case kMenuSaveNext:
	case kMenuSaveSave:
		// TODO
		debug("Clicked Save - %d", clickedMenuItem);
		break;
	case kMenuSaveChangeDirectory:
		drawMenu(vcrRequestData, kMenuChangeDirectory);
		break;
	case kMenuChangeDirectoryOK:
		// TODO
		debug("Clicked change directory - %d", clickedMenuItem);
		break;
	case kMenuQuitYes:
		g_engine->quitGame();
		break;
	case kMenuRestartYes:
		// TODO
		debug("Clicked Restart - Yes %d", clickedMenuItem);
		break;
	default:
		debug("Clicked ID %d", clickedMenuItem);
		break;
	}
}

void DgdsEngine::loadCorners(const Common::String &filename, int numImgs) {
	_corners.resize(numImgs);
	for (int i = 0; i < numImgs; i++) {
		Image *img = new Image(_resource, _decompressor);
		img->loadBitmap(filename, i);
		_corners[i].reset(img);
	}
}

// TODO: Temporary placeholder cursor - replace!
static const byte mouseData[] = {
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 7, 1, 0, 0, 0, 0, 0, 0, 0,
	1, 7, 7, 1, 0, 0, 0, 0, 0, 0,
	1, 7, 7, 7, 1, 0, 0, 0, 0, 0,
	1, 7, 7, 7, 7, 1, 0, 0, 0, 0,
	1, 7, 7, 7, 7, 7, 1, 0, 0, 0,
	1, 7, 7, 7, 7, 7, 7, 1, 0, 0,
	1, 7, 7, 7, 7, 7, 7, 7, 1, 0,
	1, 7, 7, 7, 7, 7, 1, 1, 1, 1,
	1, 7, 7, 1, 7, 7, 1, 0, 0, 0,
	1, 7, 1, 0, 1, 7, 7, 1, 0, 0,
	1, 1, 0, 0, 1, 7, 7, 1, 0, 0,
	0, 0, 0, 0, 0, 1, 7, 7, 1, 0,
	0, 0, 0, 0, 0, 1, 7, 7, 1, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 0, 0
};

Common::Error DgdsEngine::run() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);

	_console = new Console(this);
	_resource = new ResourceManager();
	_decompressor = new Decompressor();
	_image = new Image(_resource, _decompressor);
	_soundPlayer = new Sound(_mixer, _resource, _decompressor);
	_scene = new SDSScene();
	_gdsScene = new GDSScene();
	_fontManager = new FontManager();

	CursorMan.pushCursor(mouseData, 10, 15, 0, 0, 0);

	setDebugger(_console);

	_bottomBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_topBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_resData.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_screenBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	g_system->fillScreen(0);

	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Event ev;

	ADSInterpreter interpIntro(this);
	bool creditsShown = false;
	REQFileData invRequestData;
	REQFileData vcrRequestData;
	RequestParser reqParser(_resource, _decompressor);

	_fontManager->loadFonts(getGameId(), _resource, _decompressor);

	if (getGameId() == GID_DRAGON) {
		_gdsScene->load("DRAGON.GDS", _resource, _decompressor);

		reqParser.parse(&invRequestData, "DINV.REQ");
		reqParser.parse(&vcrRequestData, "DVCR.REQ");

		interpIntro.load("TITLE1.ADS");
		loadCorners("DCORNERS.BMP", 29);
	} else if (getGameId() == GID_CHINA) {
		_gdsScene->load("HOC.GDS", _resource, _decompressor);

		reqParser.parse(&invRequestData, "HINV.REQ");
		reqParser.parse(&vcrRequestData, "HVCR.REQ");

		//_scene->load("S101.SDS", _resource, _decompressor);
		interpIntro.load("TITLE.ADS");
		loadCorners("HCORNERS.BMP", 29);
	} else if (getGameId() == GID_BEAMISH) {
		// TODO: This doesn't parse correctly yet.
		//_gdsScene->load("WILLY.GDS", _resource, _decompressor);

		reqParser.parse(&invRequestData, "WINV.REQ");
		reqParser.parse(&vcrRequestData, "WVCR.REQ");

		//_scene->load("S34.SDS", _resource, _decompressor);
		interpIntro.load("TITLE.ADS");
		//loadCorners("WCORNERS.BMP", 29);	// TODO: Currently crashes
	}

	debug("Parsed Inv Request:\n%s", invRequestData.dump().c_str());
	debug("Parsed VCR Request:\n%s", vcrRequestData.dump().c_str());

	bool moveToNext = false;
	bool triggerMenu = false;
	bool mouseEvent = false;

	while (!shouldQuit()) {
		while (eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					if (_curMenu >= 0)
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
				mouseEvent = true;
			}
		}

		if (triggerMenu) {
			if (_curMenu == kMenuNone) {
				Graphics::Surface *dst = g_system->lockScreen();
				_screenBuffer.copyFrom(*dst);
				g_system->unlockScreen();

				CursorMan.showMouse(true);
				drawMenu(vcrRequestData, 0);
			} else {
				_curMenu = kMenuNone;
				CursorMan.showMouse(false);
			}

			triggerMenu = false;
		}

		if (mouseEvent) {
			handleMenu(vcrRequestData, ev.mouse);
			mouseEvent = false;
		}

		if (_curMenu != kMenuNone) {
			g_system->updateScreen();
			g_system->delayMillis(10);
			continue;
		}

		if (getGameId() == GID_DRAGON || getGameId() == GID_CHINA) {
			if (moveToNext || !interpIntro.run()) {
				moveToNext = false;

				if (!creditsShown) {
					creditsShown = true;
					if (getGameId() == GID_DRAGON)
						_scene->load("S55.SDS", _resource, _decompressor); // FIXME: Removing this breaks the Bahumat scene dialog
					interpIntro.load("INTRO.ADS");
				} else {
					return Common::kNoError;
				}
			}
		} else if (getGameId() == GID_BEAMISH) {
			if (!interpIntro.run())
				return Common::kNoError;
		}

		g_system->delayMillis(40);
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
