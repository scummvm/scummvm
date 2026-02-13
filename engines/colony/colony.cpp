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

#include "colony/colony.h"
#include "colony/gfx.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include <math.h>

namespace Colony {

static const int16 g_sintTable[256] = {
	90,  93,  95,  97,  99, 101, 103, 105,
	106, 108, 110, 111, 113, 114, 116, 117,
	118, 119, 121, 122, 122, 123, 124, 125,
	126, 126, 127, 127, 127, 128, 128, 128,
	128, 128, 128, 128, 127, 127, 127, 126,
	126, 125, 124, 123, 122, 122, 121, 119,
	118, 117, 116, 114, 113, 111, 110, 108,
	106, 105, 103, 101,  99,  97,  95,  93,
	90,  88,  86,  84,  81,  79,  76,  74,
	71,  68,  66,  63,  60,  56,  54,  52,
	49,  46,  43,  40,  37,  34,  31,  28,
	25,  23,  19,  16,  13,   9,   6,   3,
	0,  -3,  -6,  -9, -13, -16, -19, -23,
	-25, -28, -31, -34, -37, -40, -43, -46,
	-49, -52, -54, -56, -60, -63, -66, -68,
	-71, -74, -76, -79, -81, -84, -86, -88,
	-88, -90, -93, -95, -97, -99,-101,-103,
	-105,-106,-108,-110,-111,-113,-114,-116,
	-117,-118,-119,-121,-122,-122,-123,-124,
	-125,-126,-126,-127,-127,-127,-128,-128,
	-128,-128,-128,-128,-127,-127,-127,-126,
	-126,-125,-124,-123,-122,-122,-121,-119,
	-118,-117,-116,-114,-113,-111,-110,-108,
	-106,-105,-103,-101, -99, -97, -95, -93,
	-90, -88, -86, -84, -81, -79, -76, -74,
	-71, -68, -66, -63, -60, -56, -54, -52,
	-49, -46, -43, -40, -37, -34, -31, -28,
	-25, -23, -19, -16, -13,  -9,  -6,  -3,
	0,   3,   6,   9,  13,  16,  19,  23,
	25,  28,  31,  34,  37,  40,  43,  46,
	49,  52,  54,  56,  60,  63,  66,  68,
	71,  74,  76,  79,  81,  84,  86,  88
};

ColonyEngine::ColonyEngine(OSystem *syst, const ADGameDescription *gd) : Engine(syst), _gameDescription(gd) {
	_level = 0;
	_robotNum = 0;
	_gfx = nullptr;
	_width = 640;
	_height = 480;
	_centerX = _width / 2;
	_centerY = _height / 2;
	_flip = false;
	_mouseSensitivity = 1;
	_change = true;
	_showDashBoard = true;
	_crosshair = true;
	_insight = false;
	_hasKeycard = false;
	_unlocked = false;
	_weapons = 0;
	
	memset(_wall, 0, sizeof(_wall));
	memset(_mapData, 0, sizeof(_mapData));
	memset(_robotArray, 0, sizeof(_robotArray));
	memset(_foodArray, 0, sizeof(_foodArray));

	_screenR = Common::Rect(0, 0, _width, _height);
	_clip = _screenR;
	_dashBoardRect = Common::Rect(0, 0, 0, 0);
	_compassRect = Common::Rect(0, 0, 0, 0);
	_headsUpRect = Common::Rect(0, 0, 0, 0);
	_powerRect = Common::Rect(0, 0, 0, 0);
	
	// DOS gameInit(): Me.ang=Me.look=32; Me.xloc=4400; Me.yloc=4400.
	memset(&_me, 0, sizeof(_me));
	_me.xloc = 4400;
	_me.yloc = 4400;
	_me.xindex = _me.xloc >> 8;
	_me.yindex = _me.yloc >> 8;
	_me.look = 32;
	_me.ang = 32;
	_me.type = MENUM;

	initTrig();
}

ColonyEngine::~ColonyEngine() {
}

void ColonyEngine::loadMap(int mnum) {
	Common::String mapName = Common::String::format("MAP.%d", mnum);
	Common::File file;
	if (!file.open(Common::Path(mapName))) {
		warning("Could not open map file %s", mapName.c_str());
		return;
	}

	file.readUint32BE(); // "DAVE" header
	int16 mapDefs[10];
	for (int i = 0; i < 10; i++) {
		mapDefs[i] = file.readSint16BE(); // Swapped in original code
	}

	uint16 bLength = file.readUint16BE(); // Swapped in original code
	uint8 *buffer = (uint8 *)malloc(bLength);
	if (!buffer) {
		error("Out of memory loading map");
	}
	file.read(buffer, bLength);
	file.close();

	memset(_mapData, 0, sizeof(_mapData));
	memset(_robotArray, 0, sizeof(_robotArray));
	memset(_foodArray, 0, sizeof(_foodArray));
	_objects.clear();

	// expand logic
	int c = 0;
	_robotNum = MENUM + 1;
	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < 32; j++) {
			_wall[i][j] = buffer[c++];
			if (i < 31 && j < 31) {
				for (int k = 0; k < 5; k++) {
					if (_wall[i][j] & (1 << (k + 2))) {
						for (int l = 0; l < 5; l++) {
							_mapData[i][j][k][l] = buffer[c++];
						}
						// PACKIT.C: center feature type 6 marks static map objects.
						if (k == 4 && _mapData[i][j][4][0] == 6 && i < 31 && j < 31) {
							Thing obj;
							memset(&obj, 0, sizeof(obj));
							obj.alive = 1;
							obj.visible = 0;
							obj.type = _mapData[i][j][4][1] + BASEOBJECT;
							obj.where.xloc = (i << 8) + 128;
							obj.where.yloc = (j << 8) + 128;
							obj.where.xindex = i;
							obj.where.yindex = j;
							obj.where.ang = (uint8)(_mapData[i][j][4][2] + 32);
							obj.where.look = obj.where.ang;
							_objects.push_back(obj);
							const int objNum = (int)_objects.size(); // 1-based, DOS-style robot slots
							if (objNum > 0 && objNum < 256 && _robotArray[i][j] == 0)
								_robotArray[i][j] = (uint8)objNum;
						}
					} else {
						_mapData[i][j][k][0] = 0;
					}
				}
			}
		}
	}
	free(buffer);
	_level = mnum;
	_me.type = MENUM;
	if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
		_robotArray[_me.xindex][_me.yindex] = MENUM;
	debug("Successfully loaded map %d (static objects: %d)", mnum, (int)_objects.size());
}

void ColonyEngine::initTrig() {
	for (int i = 0; i < 256; i++) {
		_sint[i] = g_sintTable[i];
		_cost[i] = g_sintTable[(i + 64) & 0xFF];
	}

	_rtable[0] = 32000;
	for (int i = 1; i < 11585; i++) {
		_rtable[i] = (160 * 128) / i;
	}
}

Common::Error ColonyEngine::run() {
	Graphics::PixelFormat format8bpp = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(_width, _height, &format8bpp);

	_width = _system->getWidth();
	_height = _system->getHeight();
	updateViewportLayout();
	const Graphics::PixelFormat format = _system->getScreenFormat();
	debug("Screen format: %d bytesPerPixel. Actual size: %dx%d", format.bytesPerPixel, _width, _height);

	// Setup a grayscale palette
	byte pal[256 * 3];
	for (int i = 0; i < 256; i++) {
		pal[i * 3 + 0] = i;
		pal[i * 3 + 1] = i;
		pal[i * 3 + 2] = i;
	}
	_system->getPaletteManager()->setPalette(pal, 0, 256);

	_gfx = new Gfx(_system, _width, _height);
	
	loadMap(1); // Try to load the first map
	_system->lockMouse(true);
	_system->warpMouse(_centerX, _centerY);

	while (!shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
				if (event.type == Common::EVENT_KEYDOWN) {
					debug("Key down: %d", event.kbd.keycode);
					const bool allowInteraction = (event.kbd.flags & Common::KBD_CTRL) == 0;
					switch (event.kbd.keycode) {
					case Common::KEYCODE_UP:
					{
						int xnew = _me.xloc + (_cost[_me.look] >> 2);
						int ynew = _me.yloc + (_sint[_me.look] >> 2);
						cCommand(xnew, ynew, allowInteraction);
						break;
					}
					case Common::KEYCODE_DOWN:
					{
						int xnew = _me.xloc - (_cost[_me.look] >> 2);
						int ynew = _me.yloc - (_sint[_me.look] >> 2);
						cCommand(xnew, ynew, allowInteraction);
						break;
					}
				case Common::KEYCODE_LEFT:
					_me.look = (uint8)((int)_me.look + 8);
					_change = true;
					break;
					case Common::KEYCODE_RIGHT:
						_me.look = (uint8)((int)_me.look - 8);
						_change = true;
						break;
					case Common::KEYCODE_F7:
						_showDashBoard = !_showDashBoard;
						updateViewportLayout();
						_system->warpMouse(_centerX, _centerY);
						_change = true;
						break;
					default:
						break;
					}
				debug("Me: x=%d y=%d look=%d", _me.xloc, _me.yloc, _me.look);
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				if (event.relMouse.x != 0) {
					// Subtract because increasing look turns left, but mouse right is positive rel X
					// Reduced sensitivity by half
					_me.look = (uint8)((int)_me.look - (event.relMouse.x / 2));
					_change = true;
				}
			}
		}
		_system->warpMouse(_centerX, _centerY);

		_gfx->clear(_gfx->black());
		for (uint i = 0; i < _objects.size(); i++)
			_objects[i].visible = 0;
		
		corridor();
		drawStaticObjects();
		drawDashboardStep1();
		drawCrosshair();
		
		_gfx->copyToScreen();
		_system->delayMillis(10);
	}

	delete _gfx;
	return Common::kNoError;
}

} // End of namespace Colony
