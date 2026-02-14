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
#include "common/config-manager.h"
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
#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/fonts/dosfont.h"
#include "graphics/cursorman.h"
#include <math.h>

namespace Colony {



ColonyEngine::ColonyEngine(OSystem *syst, const ADGameDescription *gd) : Engine(syst), _gameDescription(gd), _randomSource("colony") {
	_level = 0;
	_robotNum = 0;
	_gfx = nullptr;
	_width = 640;
	_height = 350;
	_centerX = _width / 2;
	_centerY = _height / 2;
	_mouseSensitivity = 1;
	_showDashBoard = true;
	_crosshair = true;
	_insight = false;
	_hasKeycard = false;
	_unlocked = false;
	_weapons = 0;
	_wireframe = true;
	_widescreen = ConfMan.getBool("widescreen_mod");
	
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

	// Animation system init
	_backgroundMask = nullptr;
	_backgroundFG = nullptr;
	_backgroundActive = false;
	_divideBG = 0;
	_animationRunning = false;

	for (int i = 0; i < 4; i++) {
		_decode1[i] = _decode2[i] = _decode3[i] = 0;
	}
	for (int i = 0; i < 6; i++) _animDisplay[i] = 1;
	for (int i = 0; i < 2; i++) _coreState[i] = _coreHeight[i] = 0;
	for (int i = 0; i < 3; i++) _corePower[i] = 0;
	_coreIndex = 0;
	_orbit = 0;
	_armor = 0;
	_gametest = false;

	initTrig();
}

ColonyEngine::~ColonyEngine() {
	deleteAnimation();
	delete _gfx;
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
	// Compute standard sin/cos lookup tables (256 steps = full circle, scaled by 128)
	for (int i = 0; i < 256; i++) {
		float rad = (float)i * 2.0f * M_PI / 256.0f;
		_sint[i] = (int)roundf(128.0f * sinf(rad));
		_cost[i] = (int)roundf(128.0f * cosf(rad));
	}
}

Common::Error ColonyEngine::run() {
	_width = 640;
	_height = 350;

	if (_widescreen) {
		_width = _height * 16 / 9;
	}

	_gfx = createRenderer(_system, _width, _height);
	if (!_gfx)
		return Common::kUserCanceled;

	updateViewportLayout();
	const Graphics::PixelFormat format = _system->getScreenFormat();
	debug("Screen format: %d bytesPerPixel. Actual size: %dx%d", format.bytesPerPixel, _width, _height);

	// Setup a palette with standard 16 colors followed by grayscale
	byte pal[256 * 3];
	static const byte ega_colors[16][3] = {
		{0, 0, 0}, {0, 0, 170}, {0, 170, 0}, {0, 170, 170},
		{170, 0, 0}, {170, 0, 170}, {170, 85, 0}, {170, 170, 170},
		{85, 85, 85}, {85, 85, 255}, {85, 255, 85}, {85, 255, 255},
		{255, 85, 85}, {255, 85, 255}, {255, 255, 85}, {255, 255, 255}
	};
	for (int i = 0; i < 16; i++) {
		pal[i * 3 + 0] = ega_colors[i][0];
		pal[i * 3 + 1] = ega_colors[i][1];
		pal[i * 3 + 2] = ega_colors[i][2];
	}
	for (int i = 16; i < 256; i++) {
		pal[i * 3 + 0] = i;
		pal[i * 3 + 1] = i;
		pal[i * 3 + 2] = i;
	}
	_gfx->setPalette(pal, 0, 256);
	_gfx->setWireframe(_wireframe);
	
	scrollInfo();

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
					case Common::KEYCODE_w:
					{
						int xnew = _me.xloc + (_cost[_me.look] >> 2);
						int ynew = _me.yloc + (_sint[_me.look] >> 2);
						cCommand(xnew, ynew, allowInteraction);
						break;
					}
					case Common::KEYCODE_DOWN:
					case Common::KEYCODE_s:
					{
						int xnew = _me.xloc - (_cost[_me.look] >> 2);
						int ynew = _me.yloc - (_sint[_me.look] >> 2);
						cCommand(xnew, ynew, allowInteraction);
						break;
					}
					case Common::KEYCODE_LEFT:
					case Common::KEYCODE_a:
					{
						uint8 strafeAngle = (uint8)((int)_me.look + 64);
						int xnew = _me.xloc + (_cost[strafeAngle] >> 2);
						int ynew = _me.yloc + (_sint[strafeAngle] >> 2);
						cCommand(xnew, ynew, allowInteraction);
						break;
					}
					case Common::KEYCODE_RIGHT:
					case Common::KEYCODE_d:
					{
						uint8 strafeAngle = (uint8)((int)_me.look - 64);
						int xnew = _me.xloc + (_cost[strafeAngle] >> 2);
						int ynew = _me.yloc + (_sint[strafeAngle] >> 2);
						cCommand(xnew, ynew, allowInteraction);
						break;
					}
					case Common::KEYCODE_F7:
						_wireframe = !_wireframe;
						_gfx->setWireframe(_wireframe);
						break;
					default:
						break;
					}
				debug("Me: x=%d y=%d look=%d", _me.xloc, _me.yloc, _me.look);
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				if (event.relMouse.x != 0) {
					_me.look = (uint8)((int)_me.look - (event.relMouse.x / 2));
				}
				if (event.relMouse.y != 0) {
					_me.lookY = (int8)CLIP<int>((int)_me.lookY - (event.relMouse.y / 2), -64, 64);
				}
				// Warp back to center and purge remaining mouse events
				// to prevent the warp from generating phantom deltas (Freescape pattern)
				_system->warpMouse(_centerX, _centerY);
				_system->getEventManager()->purgeMouseEvents();
			} else if (event.type == Common::EVENT_SCREEN_CHANGED) {
				_gfx->computeScreenViewport();
			}
		}

		_gfx->clear(_gfx->black());
		
		corridor();
		drawDashboardStep1();
		drawCrosshair();
		
		_gfx->copyToScreen();
		_system->delayMillis(10);
	}

	return Common::kNoError;
}

void ColonyEngine::scrollInfo() {
	const char *story[] = {
		"Mankind has left the",
		"cradle of earth and",
		"is beginning to eye",
		"the galaxy. He has",
		"begun to colonize",
		"distant planets but has",
		"yet to meet any alien",
		"life forms.",
		"****",
		"Until now...",
		"****",
		"Press any key to begin",
		"the Adventure..."
	};
	const int storyLength = ARRAYSIZE(story);

	_gfx->clear(_gfx->black());
	Graphics::DosFont dosFont;
	const Graphics::Font *font = &dosFont;

	int centerY = _height / 2;
	centerY -= (storyLength * 10) / 2;
	centerY += 5;

	for (int i = 0; i < storyLength; i++) {
		_gfx->drawString(font, story[i], _width / 2, centerY + 10 * i, 9, Graphics::kTextAlignCenter);
	}
	_gfx->copyToScreen();

	bool waiting = true;
	while (waiting && !shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN)
				waiting = false;
		}
		_system->delayMillis(10);
	}

	_gfx->fillRect(_screenR, 0);
	_gfx->copyToScreen();
}

bool ColonyEngine::loadAnimation(const Common::String &name) {
	_animationName = name;
	for (int i = 0; i < 6; i++) _animDisplay[i] = 1;
	Common::String fileName = name + ".pic";
	Common::File file;
	if (!file.open(Common::Path(fileName))) {
		warning("Could not open animation file %s", fileName.c_str());
		return false;
	}

	deleteAnimation();

	// Read background data
	file.read(_topBG, 8);
	file.read(_bottomBG, 8);
	_divideBG = file.readSint16LE();
	_backgroundActive = file.readSint16LE() != 0;
	if (_backgroundActive) {
		_backgroundClip = readRect(file);
		_backgroundLocate = readRect(file);
		_backgroundMask = loadImage(file);
		_backgroundFG = loadImage(file);
	}

	// Read sprite data
	int16 maxsprite = file.readSint16LE();
	file.readSint16LE(); // locSprite
	for (int i = 0; i < maxsprite; i++) {
		Sprite *s = new Sprite();
		s->fg = loadImage(file);
		s->mask = loadImage(file);
		s->used = file.readSint16LE() != 0;
		s->clip = readRect(file);
		s->locate = readRect(file);
		_cSprites.push_back(s);
	}

	// Read complex sprite data
	int16 maxLSprite = file.readSint16LE();
	file.readSint16LE(); // anum
	for (int i = 0; i < maxLSprite; i++) {
		ComplexSprite *ls = new ComplexSprite();
		int16 size = file.readSint16LE();
		for (int j = 0; j < size; j++) {
			ComplexSprite::SubObject sub;
			sub.spritenum = file.readSint16LE();
			sub.xloc = file.readSint16LE();
			sub.yloc = file.readSint16LE();
			ls->objects.push_back(sub);
		}
		ls->bounds = readRect(file);
		ls->visible = file.readSint16LE() != 0;
		ls->current = file.readSint16LE();
		ls->xloc = file.readSint16LE();
		ls->yloc = file.readSint16LE();
		ls->acurrent = file.readSint16LE();
		ls->axloc = file.readSint16LE();
		ls->ayloc = file.readSint16LE();
		ls->type = file.readByte();
		ls->frozen = file.readByte();
		ls->locked = file.readByte();
		ls->link = file.readSint16LE();
		ls->key = file.readSint16LE();
		ls->lock = file.readSint16LE();
		ls->onoff = true;
		_lSprites.push_back(ls);
	}

	return true;
}

void ColonyEngine::deleteAnimation() {
	delete _backgroundMask; _backgroundMask = nullptr;
	delete _backgroundFG; _backgroundFG = nullptr;
	for (uint i = 0; i < _cSprites.size(); i++) delete _cSprites[i];
	_cSprites.clear();
	for (uint i = 0; i < _lSprites.size(); i++) delete _lSprites[i];
	_lSprites.clear();
}

void ColonyEngine::playAnimation() {
	_animationRunning = true;
	_system->lockMouse(false);
	_system->showMouse(true);
	_system->warpMouse(_centerX, _centerY);
	CursorMan.setDefaultArrowCursor(true);
	CursorMan.showMouse(true);
	_system->updateScreen();

	if (_animationName == "security" && !_unlocked) {
		for (int i = 0; i < 4; i++) {
			_decode1[i] = (uint8)(2 + _randomSource.getRandomNumber(3));
			SetObjectState(27 + i, _decode1[i]);
		}
	} else if (_animationName == "reactor") {
		for (int i = 0; i < 6; i++) {
			SetObjectOnOff(14 + i * 2, false);
			SetObjectState(13 + i * 2, 1);
		}
	} else if (_animationName == "controls") {
		switch (_corePower[_coreIndex]) {
		case 0: SetObjectState(2, 1); SetObjectState(5, 1); break;
		case 1: SetObjectState(2, 1); SetObjectState(5, 2); break;
		case 2: SetObjectState(2, 2); SetObjectState(5, 1); break;
		}
	}

	while (_animationRunning && !shouldQuit()) {
		updateAnimation();
		drawAnimation();
		_gfx->copyToScreen();

		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONDOWN) {
				int item = whichSprite(event.mouse);
				if (item > 0) {
					handleAnimationClick(item);
				}
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				debug(5, "Animation Mouse: %d, %d", event.mouse.x, event.mouse.y);
			} else if (event.type == Common::EVENT_KEYDOWN) {
				int item = 0;
				if (event.kbd.keycode >= Common::KEYCODE_0 && event.kbd.keycode <= Common::KEYCODE_9) {
					item = 1 + (event.kbd.keycode - Common::KEYCODE_0);
				} else if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
					item = 1 + (event.kbd.keycode - Common::KEYCODE_KP0);
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					item = 12; // Enter
				} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE || event.kbd.keycode == Common::KEYCODE_DELETE) {
					item = 11; // Clear
				} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_animationRunning = false;
				}

				if (item > 0) {
					handleAnimationClick(item);
				}
			}
		}
		_system->delayMillis(20);
	}

	_system->lockMouse(true);
	_system->showMouse(false);
	CursorMan.showMouse(false);
	CursorMan.popAllCursors();
	deleteAnimation();
}

void ColonyEngine::updateAnimation() {
	static uint32 lastUpdate = 0;
	uint32 now = _system->getMillis();
	if (now - lastUpdate < 50) // Reduced to 50ms (20 fps) to make it "move"
		return;
	lastUpdate = now;

	for (uint i = 0; i < _lSprites.size(); i++) {
		ComplexSprite *ls = _lSprites[i];
		// type 0 are displays that auto-animate
		// Original NoShowIt ONLY checked !ls->locked
		if (ls->onoff && ls->type == 0 && !ls->locked && ls->objects.size() > 1) {
			ls->current++;
			if (ls->current >= (int)ls->objects.size())
				ls->current = 0;
		}
	}
}

void ColonyEngine::drawAnimation() {
	_gfx->clear(0); // Ensure entire screen is black

	// Center 416x264 animation area on screen (from original InitDejaVu)
	int ox = (_width - 416) / 2;
	ox = (ox / 8) * 8; // Round to 8 as in original code
	int oy = (_height - 264) / 2;

	// Fill background patterns (416x264 area)
	for (int y = 0; y < 264; y++) {
		byte *pat = (y < _divideBG) ? _topBG : _bottomBG;
		byte row = pat[y % 8];
		for (int x = 0; x < 416; x++) {
			bool set = (row & (0x80 >> (x % 8))) != 0;
			// Pattern bit 1 is background color (15), bit 0 is foreground (0)
			// matching original FillRect with inverted data.
			_gfx->setPixel(ox + x, oy + y, set ? 15 : 0);
		}
	}

	// Draw background if active
	if (_backgroundActive && _backgroundFG) {
		drawAnimationImage(_backgroundFG, _backgroundMask, ox + _backgroundLocate.left, oy + _backgroundLocate.top);
	}

	// Draw complex sprites
	for (uint i = 0; i < _lSprites.size(); i++) {
		if (_lSprites[i]->onoff)
			drawComplexSprite(i, ox, oy);
	}
}

void ColonyEngine::drawComplexSprite(int index, int ox, int oy) {
	ComplexSprite *ls = _lSprites[index];
	if (!ls->onoff) return;

	int cnum = ls->current;
	if (cnum < 0 || cnum >= (int)ls->objects.size()) return;

	int spriteIdx = ls->objects[cnum].spritenum;
	if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size()) return;

	Sprite *s = _cSprites[spriteIdx];
	int x = ox + ls->xloc + ls->objects[cnum].xloc;
	int y = oy + ls->yloc + ls->objects[cnum].yloc;

	drawAnimationImage(s->fg, s->mask, x, y);
}

void ColonyEngine::drawAnimationImage(Image *img, Image *mask, int x, int y) {
	if (!img || !img->data) return;

	for (int iy = 0; iy < img->height; iy++) {
		for (int ix = 0; ix < img->width; ix++) {
			int byteIdx = iy * img->rowBytes + (ix / 8);
			int bitIdx = 7 - (ix % 8);
			
			bool maskSet = true;
			if (mask && mask->data) {
				maskSet = (mask->data[byteIdx] & (1 << bitIdx)) != 0;
			}

			if (!maskSet) continue;

			bool fgSet = (img->data[byteIdx] & (1 << bitIdx)) != 0;
			uint32 color = fgSet ? 15 : 0;

			_gfx->setPixel(x + ix, y + iy, color);
		}
	}
}

Image *ColonyEngine::loadImage(Common::SeekableReadStream &file) {
	Image *im = new Image();
	im->width = file.readSint16LE();
	im->height = file.readSint16LE();
	im->align = file.readSint16LE();
	im->rowBytes = file.readSint16LE();
	im->bits = file.readByte();
	im->planes = file.readByte();

	int16 tf = file.readSint16LE();
	uint32 size;
	if (tf) {
		/* uint32 bsize = */ file.readUint32LE();
		size = file.readUint32LE();
		im->data = (byte *)malloc(size);
		unpackBytes(file, im->data, size);
	} else {
		size = file.readUint32LE();
		im->data = (byte *)malloc(size);
		file.read(im->data, size);
	}
	return im;
}

void ColonyEngine::unpackBytes(Common::SeekableReadStream &file, byte *dst, uint32 len) {
	uint32 i = 0;
	while (i < len) {
		byte count = file.readByte();
		byte value = file.readByte();
		for (int j = 0; j < count && i < len; j++) {
			dst[i++] = value;
		}
	}
}

Common::Rect ColonyEngine::readRect(Common::SeekableReadStream &file) {
	int16 top = file.readSint16LE();
	int16 left = file.readSint16LE();
	int16 bottom = file.readSint16LE();
	int16 right = file.readSint16LE();
	return Common::Rect(left, top, right, bottom);
}

int ColonyEngine::whichSprite(const Common::Point &p) {
	int ox = (_width - 416) / 2;
	ox = (ox / 8) * 8;
	int oy = (_height - 264) / 2;
	Common::Point pt(p.x - ox, p.y - oy);

	for (int i = _lSprites.size() - 1; i >= 0; i--) {
		if (_lSprites[i]->onoff && _lSprites[i]->bounds.contains(pt)) {
			return i + 1;
		}
	}
	return 0;
}

void ColonyEngine::handleAnimationClick(int item) {
	debug("Animation click on item %d", item);

	if (_animationName == "desk") {
		if (item >= 2 && item <= 5) {
			int idx = item - 2;
			uint8 *decode = (_level == 1) ? _decode2 : _decode3;
			if (decode[idx] == 0) {
				decode[idx] = (uint8)(2 + (_randomSource.getRandomNumber(3)));
				_lSprites[item - 1]->current = decode[idx] - 1;
				drawAnimation();
				_gfx->copyToScreen();
			}
			return;
		}
	} else if (_animationName == "reactor" || _animationName == "security") {
		if (item >= 1 && item <= 10) {
			for (int i = 5; i >= 1; i--) _animDisplay[i] = _animDisplay[i - 1];
			_animDisplay[0] = (uint8)(item + 1);
			refreshAnimationDisplay();
			drawAnimation();
			_gfx->copyToScreen();
			return;
		} else if (item == 11) { // Clear
			for (int i = 0; i < 6; i++) _animDisplay[i] = 1;
			refreshAnimationDisplay();
			drawAnimation();
			_gfx->copyToScreen();
			return;
		} else if (item == 12) { // Enter
			uint8 testarray[6];
			if (_animationName == "reactor") {
				if (_level == 1)
					crypt(testarray, _decode2[3] - 2, _decode2[2] - 2, _decode2[1] - 2, _decode2[0] - 2);
				else
					crypt(testarray, _decode3[3] - 2, _decode3[2] - 2, _decode3[1] - 2, _decode3[0] - 2);

				bool match = true;
				for (int i = 0; i < 6; i++) {
					if (testarray[i] != _animDisplay[5 - i]) match = false;
				}
				if (match) {
					if (_coreState[_coreIndex] == 0) _coreState[_coreIndex] = 1;
					else if (_coreState[_coreIndex] == 1) _coreState[_coreIndex] = 0;
					_gametest = true;
				}
				_animationRunning = false;
			} else { // security
				crypt(testarray, _decode1[0] - 2, _decode1[1] - 2, _decode1[2] - 2, _decode1[3] - 2);
				bool match = true;
				for (int i = 0; i < 6; i++) {
					if (testarray[i] != _animDisplay[5 - i]) match = false;
				}
				if (match) {
					_unlocked = true;
					_gametest = true;
				}
				_animationRunning = false;
			}
			return;
		}
	} else if (_animationName == "controls") {
		switch (item) {
		case 4: // Accelerator
			if (_corePower[_coreIndex] >= 2 && _coreState[_coreIndex] == 0 && !_orbit) {
				_orbit = 1;
				debug("Taking off!");
				_animationRunning = false;
			} else {
				debug("Accelerator failed: power=%d, state=%d", _corePower[_coreIndex], _coreState[_coreIndex]);
			}
			break;
		case 5: // Emergency power
			if (_coreState[_coreIndex] < 2) {
				if (_corePower[_coreIndex] == 0) _corePower[_coreIndex] = 1;
				else if (_corePower[_coreIndex] == 1) _corePower[_coreIndex] = 0;
			}
			// Update power visual state
			switch (_corePower[_coreIndex]) {
			case 0: SetObjectState(2, 1); SetObjectState(5, 1); break;
			case 1: SetObjectState(2, 1); SetObjectState(5, 2); break;
			case 2: SetObjectState(2, 2); SetObjectState(5, 1); break;
			}
			drawAnimation();
			_gfx->copyToScreen();
			break;
		case 6: // Ship weapons
			if (!_orbit) {
				debug("Firing ship weapons on ground! Game Over soon.");
				_animationRunning = false;
			} else {
				debug("Firing ship weapons in orbit.");
				_animationRunning = false;
			}
			break;
		case 7: // Damage report
			debug("Damage report button clicked.");
			break;
		}
	} else if (_animationName == "suit") {
		if (item == 1) { // Armor
			_armor = (_armor + 1) % 4;
			SetObjectState(1, _armor * 2 + 1);
			SetObjectState(3, _armor + 1);
			drawAnimation();
			_gfx->copyToScreen();
		} else if (item == 2) { // Weapons
			_weapons = (_weapons + 1) % 4;
			SetObjectState(2, _weapons * 2 + 1);
			SetObjectState(4, _weapons + 1);
			drawAnimation();
			_gfx->copyToScreen();
		}
	}

	dolSprite(item - 1);
}

void ColonyEngine::dolSprite(int index) {
	if (index < 0 || index >= (int)_lSprites.size())
		return;

	ComplexSprite *ls = _lSprites[index];
	switch (ls->type) {
	case 1: // Key and control
		if (ls->frozen) {
			// Control logic
			if (!ls->locked || ls->current) {
				if (ls->current > 0) {
					while (ls->current > 0) {
						ls->current--;
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(50);
					}
				} else {
					while (ls->current < (int)ls->objects.size() - 1) {
						ls->current++;
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(50);
					}
				}
			}
		}
		break;
	case 2: // Container and object
		if (ls->frozen) {
			if (!ls->locked) {
				if (ls->current > 0) {
					while (ls->current > 0) {
						ls->current--;
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(50);
					}
				} else {
					while (ls->current < (int)ls->objects.size() - 1) {
						ls->current++;
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(50);
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

void ColonyEngine::SetObjectState(int num, int state) {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		_lSprites[num]->current = state - 1;
}

void ColonyEngine::SetObjectOnOff(int num, bool on) {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		_lSprites[num]->onoff = on;
}

void ColonyEngine::refreshAnimationDisplay() {
	for (int i = 0; i < 6; i++) {
		if (_animDisplay[i] < 9) {
			SetObjectOnOff(13 + i * 2, true);
			SetObjectOnOff(14 + i * 2, false);
			SetObjectState(13 + i * 2, _animDisplay[i]);
		} else {
			SetObjectOnOff(14 + i * 2, true);
			SetObjectOnOff(13 + i * 2, false);
			SetObjectState(14 + i * 2, _animDisplay[i] - 8);
		}
	}
}

void ColonyEngine::crypt(uint8 sarray[6], int i, int j, int k, int l) {
	int res[6];
	res[0] = ((3 * l) ^ i ^ j ^ k) % 10;
	res[1] = ((i * 3) ^ (j * 7) ^ (k * 11) ^ (l * 13)) % 10;
	res[2] = (3 + (l * 17) ^ (j * 19) ^ (k * 23) ^ (i * 29)) % 10;
	res[3] = ((l * 19) ^ (j * 23) ^ (k * 29) ^ (i * 31)) % 10;
	res[4] = ((l * 17) | (j * 19) | (k * 23) | (i * 29)) % 10;
	res[5] = (29 + (l * 17) - (j * 19) - (k * 23) - (i * 29)) % 10;
	for (int m = 0; m < 6; m++) {
		if (res[m] < 0) res[m] = -res[m];
		sarray[m] = (uint8)(res[m] + 2);
	}
}

} // End of namespace Colony
