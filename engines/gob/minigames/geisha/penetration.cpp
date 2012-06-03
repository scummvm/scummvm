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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/decfile.h"
#include "gob/cmpfile.h"
#include "gob/anifile.h"
#include "gob/aniobject.h"

#include "gob/minigames/geisha/penetration.h"
#include "gob/minigames/geisha/meter.h"

namespace Gob {

namespace Geisha {

static const byte kPalette[48] = {
	0x16,  0x16,  0x16,
	0x12,  0x14,  0x16,
	0x34,  0x00,  0x25,
	0x1D,  0x1F,  0x22,
	0x24,  0x27,  0x2A,
	0x2C,  0x0D,  0x22,
	0x2B,  0x2E,  0x32,
	0x12,  0x09,  0x20,
	0x3D,  0x3F,  0x00,
	0x3F,  0x3F,  0x3F,
	0x00,  0x00,  0x00,
	0x15,  0x15,  0x3F,
	0x25,  0x22,  0x2F,
	0x1A,  0x14,  0x28,
	0x3F,  0x00,  0x00,
	0x15,  0x3F,  0x15
};

static const int kColorShield = 11;
static const int kColorHealth = 15;
static const int kColorBlack  = 10;
static const int kColorFloor  = 13;

enum Animation {
	kAnimationDriveS   =  4,
	kAnimationDriveE   =  5,
	kAnimationDriveN   =  6,
	kAnimationDriveW   =  7,
	kAnimationDriveSE  =  8,
	kAnimationDriveNE  =  9,
	kAnimationDriveSW  = 10,
	kAnimationDriveNW  = 11,
	kAnimationShootS   = 12,
	kAnimationShootN   = 13,
	kAnimationShootW   = 14,
	kAnimationShootE   = 15,
	kAnimationShootNE  = 16,
	kAnimationShootSE  = 17,
	kAnimationShootSW  = 18,
	kAnimationShootNW  = 19,
	kAnimationExplodeN = 28,
	kAnimationExplodeS = 29,
	kAnimationExplodeW = 30,
	kAnimationExplodeE = 31,
	kAnimationExit     = 32
};

static const int kMapTileWidth  = 24;
static const int kMapTileHeight = 24;

static const int kPlayAreaX      = 120;
static const int kPlayAreaY      =   7;
static const int kPlayAreaWidth  = 192;
static const int kPlayAreaHeight = 113;

static const int kPlayAreaBorderWidth  = kPlayAreaWidth  / 2;
static const int kPlayAreaBorderHeight = kPlayAreaHeight / 2;

const byte Penetration::kMaps[kModeCount][kFloorCount][kMapWidth * kMapHeight] = {
	{
		{ // Real mode, floor 0
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,
			50, 50,  0,  0, 52, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0, 50, 50,
			50,  0,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0,  0, 50,  0,  0, 52, 53,  0,  0,  0,  0,  0,  0, 50,  0, 50,
			50,  0, 50,  0,  0, 50, 50, 50, 50,  0, 54, 55,  0,  0, 50,  0, 50,
			50,  0, 50, 49,  0, 50,  0, 52, 53,  0, 50, 50, 50,  0,  0,  0, 50,
			50, 57,  0, 50,  0,  0,  0, 50, 50, 50,  0,  0, 56, 50, 54, 55, 50,
			50, 50,  0,  0, 50, 50, 50,  0,  0,  0,  0, 50,  0,  0, 50,  0, 50,
			50, 51, 50,  0, 54, 55,  0,  0, 50, 50, 50, 50, 52, 53, 50,  0, 50,
			50,  0, 50,  0,  0,  0,  0,  0, 54, 55,  0,  0,  0, 50,  0,  0, 50,
			50,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0,  0, 50,
			50, 50,  0, 52, 53,  0,  0,  0,  0,  0,  0, 52, 53,  0,  0, 50, 50,
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0
		},
		{ // Real mode, floor 1
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,
			50,  0, 52, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 50,
			50,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0, 50, 51, 52, 53,  0,  0, 52, 53,  0,  0,  0,  0, 50,  0, 50,
			50,  0, 50,  0, 50, 50,  0, 50,  0, 50,  0, 50, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 52, 53,  0,  0,  0,  0,  0, 52, 53,  0, 52, 53, 50,
			50, 57, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0, 50, 52, 53,  0,  0, 52, 53,  0,  0,  0,  0,  0, 54, 55, 50,
			50,  0, 50,  0, 50,  0, 50, 50,  0, 50, 50,  0, 50,  0, 50, 50, 50,
			50,  0, 50, 49,  0,  0, 52, 53,  0, 52, 53,  0,  0,  0, 50, 56, 50,
			50,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0,  0,  0,  0,  0,  0,  0, 54, 55,  0,  0,  0,  0,  0,  0, 50,
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0
		},
		{ // Real mode, floor 2
			 0, 50, 50, 50, 50, 50, 50, 50,  0, 50, 50, 50, 50, 50, 50, 50,  0,
			50, 52, 53,  0,  0,  0,  0, 50, 50, 50,  0,  0,  0,  0, 52, 53, 50,
			50,  0, 50, 50, 50,  0,  0,  0, 50,  0,  0,  0, 50, 50, 50,  0, 50,
			50,  0, 50, 52, 53, 50, 50, 52, 53,  0, 50, 50, 54, 55, 50,  0, 50,
			50,  0, 50,  0,  0,  0,  0, 50,  0, 50,  0,  0,  0,  0, 50,  0, 50,
			50,  0,  0,  0, 50,  0,  0,  0, 50,  0,  0,  0, 50,  0, 52, 53, 50,
			 0, 50,  0, 50, 50, 50,  0, 57, 50, 51,  0, 50, 50, 50,  0, 50,  0,
			50,  0,  0,  0, 50,  0,  0,  0, 50,  0, 52, 53, 50,  0,  0,  0, 50,
			50,  0, 50,  0,  0,  0,  0, 50, 56, 50,  0,  0,  0,  0, 50,  0, 50,
			50,  0, 50, 54, 55, 50, 50,  0,  0,  0, 50, 50, 54, 55, 50,  0, 50,
			50,  0, 50, 50, 50,  0,  0,  0, 50,  0,  0,  0, 50, 50, 50,  0, 50,
			50, 52, 53,  0,  0,  0,  0, 50, 50, 50,  0,  0,  0,  0, 52, 53, 50,
			 0, 50, 50, 50, 50, 50, 50, 50,  0, 50, 50, 50, 50, 50, 50, 50,  0
		}
	},
	{
		{ // Test mode, floor 0
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
			50, 56,  0, 50,  0,  0, 52, 53,  0,  0,  0,  0, 52, 53,  0, 51, 50,
			50,  0,  0, 50,  0,  0,  0, 50,  0, 54, 55, 50,  0, 50, 50, 50, 50,
			50, 52, 53, 50, 50,  0,  0, 50, 50, 50, 50, 50,  0, 50,  0,  0, 50,
			50,  0,  0,  0,  0, 56,  0,  0,  0,  0,  0, 50, 49, 50,  0,  0, 50,
			50,  0, 54, 55,  0, 50, 50, 54, 55,  0, 50, 50, 50,  0,  0,  0, 50,
			50,  0,  0,  0,  0,  0,  0,  0,  0,  0, 52, 53,  0,  0, 54, 55, 50,
			50,  0, 50,  0, 50,  0,  0, 50,  0,  0,  0, 50,  0,  0,  0,  0, 50,
			50,  0, 50,  0, 50, 54, 55, 50,  0, 50, 50, 50,  0, 50,  0,  0, 50,
			50, 50, 50, 50, 50,  0,  0, 50,  0,  0,  0,  0,  0, 50, 54, 55, 50,
			50,  0,  0,  0,  0,  0,  0,  0, 50, 50, 50, 50, 50,  0,  0,  0, 50,
			50, 57,  0, 52, 53,  0,  0,  0,  0, 54, 55,  0,  0,  0,  0, 56, 50,
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
		},
		{ // Test mode, floor 1
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
			50, 52, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 50,
			50,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 54, 55,  0, 50,
			50,  0, 50, 52, 53,  0,  0, 50,  0,  0, 54, 55, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0,  0, 52, 53,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50, 50, 50, 50, 50, 49, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0,  0, 50, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50, 51,  0,  0, 52, 53, 50,  0, 50,  0, 50,
			50, 57, 50,  0, 50,  0, 50, 50, 50, 50, 50, 50, 50,  0, 50,  0, 50,
			50, 50, 50,  0, 50, 56,  0,  0,  0, 54, 55,  0,  0,  0, 50,  0, 50,
			50, 56,  0,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0, 50,
			50, 50, 50, 50,  0,  0,  0,  0, 52, 53,  0,  0,  0,  0,  0,  0, 50,
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
		},
		{ // Test mode, floor 2
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
			50, 57, 50, 54, 55,  0, 50, 54, 55,  0, 50,  0, 52, 53, 50, 51, 50,
			50,  0, 50,  0, 50,  0, 50,  0,  0,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50, 52, 53,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0,  0,  0, 50,  0, 50,  0, 50,  0,  0,  0, 50,  0, 50,  0, 50,
			50,  0,  0,  0, 50, 52, 53,  0, 50, 52, 53, 56, 50,  0, 54, 55, 50,
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
		}
	}
};


Penetration::Position::Position(uint16 pX, uint16 pY) : x(pX), y(pY) {
}


Penetration::Penetration(GobEngine *vm) : _vm(vm), _background(0), _sprites(0), _objects(0), _sub(0),
	_shieldMeter(0), _healthMeter(0), _floor(0), _mapUpdate(false), _mapX(0), _mapY(0),
	_subTileX(0), _subTileY(0) {

	_background = new Surface(320, 200, 1);

	_shieldMeter = new Meter(11, 119, 92, 3, kColorShield, kColorBlack, 1020, Meter::kFillToRight);
	_healthMeter = new Meter(11, 137, 92, 3, kColorHealth, kColorBlack, 1020, Meter::kFillToRight);

	_map = new Surface(kMapWidth  * kMapTileWidth  + kPlayAreaWidth ,
	                   kMapHeight * kMapTileHeight + kPlayAreaHeight, 1);
}

Penetration::~Penetration() {
	deinit();

	delete _map;

	delete _shieldMeter;
	delete _healthMeter;

	delete _background;
}

bool Penetration::play(bool hasAccessPass, bool hasMaxEnergy, bool testMode) {
	_hasAccessPass = hasAccessPass;
	_hasMaxEnergy  = hasMaxEnergy;
	_testMode      = testMode;

	init();
	initScreen();

	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();

	while (!_vm->shouldQuit()) {
		updateAnims();

		// Draw and wait for the end of the frame
		_vm->_draw->blitInvalidated();
		_vm->_util->waitEndFrame();

		// Handle input
		_vm->_util->processInput();

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);
		// Aborting the game
		if (key == kKeyEscape)
			break;

		// Handle the sub movement
		handleSub(key);
	}

	deinit();
	return false;
}

void Penetration::init() {
	_background->clear();

	_vm->_video->drawPackedSprite("hyprmef2.cmp", *_background);

	_sprites = new CMPFile(_vm, "tcifplai.cmp", 320, 200);
	_objects = new ANIFile(_vm, "tcite.ani", 320);

	// The shield starts down
	_shieldMeter->setValue(0);

	// If we don't have the max energy tokens, the health starts at 1/3 strength
	if (_hasMaxEnergy)
		_healthMeter->setMaxValue();
	else
		_healthMeter->setValue(_healthMeter->getMaxValue() / 3);

	_floor = 0;

	createMap();

	_sub = new ANIObject(*_objects);

	_sub->setAnimation(kAnimationDriveN);
	_sub->setPosition(kPlayAreaX + kPlayAreaBorderWidth, kPlayAreaY + kPlayAreaBorderHeight);
	_sub->setVisible(true);

	_anims.push_back(_sub);
}

void Penetration::deinit() {
	_anims.clear();

	delete _sub;

	delete _objects;
	delete _sprites;

	_objects = 0;
	_sprites = 0;
}

void Penetration::createMap() {
	if (_floor >= kFloorCount)
		error("Geisha: Invalid floor %d in minigame penetration", _floor);

	// Copy the correct map
	memcpy(_mapTiles, kMaps[_testMode ? 1 : 0][_floor], kMapWidth * kMapHeight);

	_shields.clear();

	_map->fill(kColorBlack);

	// Draw the map tiles
	for (int y = 0; y < kMapHeight; y++) {
		for (int x = 0; x < kMapWidth; x++) {
			byte *mapTile = _mapTiles + (y * kMapWidth + x);

			const int posX = kPlayAreaBorderWidth  + x * kMapTileWidth;
			const int posY = kPlayAreaBorderHeight + y * kMapTileHeight;

			switch (*mapTile) {
			case 0: // Floor
				_sprites->draw(*_map, 30, posX, posY);
				break;

			case 49: // Emergency exit (needs access pass)

				if (_hasAccessPass) {
					// Draw an exit. Now works like a regular exit
					_sprites->draw(*_map, 29, posX, posY);
					*mapTile = 51;
				} else
					// Draw a wall
					_sprites->draw(*_map, 31, posX, posY);

				break;

			case 50: // Wall
				_sprites->draw(*_map, 31, posX, posY);
				break;

			case 51: // Regular exit

				if (!_testMode) {
					// When we're not in test mode, the last exit only works with an access pass

					if (_floor == 2) {
						if (!_hasAccessPass) {
							// It's now a wall
							_sprites->draw(*_map, 31, posX, posY);
							*mapTile = 50;
						} else
							_sprites->draw(*_map, 29, posX, posY);

					} else
						_sprites->draw(*_map, 29, posX, posY);

				} else
					// Always works in test mode
					_sprites->draw(*_map, 29, posX, posY);

				break;

			case 52: // Left side of biting mouth
				_sprites->draw(*_map, 32, posX, posY);
				break;

			case 53: // Right side of biting mouth
				*mapTile = 0; // Works like a floor
				break;

			case 54: // Left side of kissing mouth
				_sprites->draw(*_map, 33, posX, posY);
				break;

			case 55: // Right side of kissing mouth
				*mapTile = 0; // Works like a floor
				break;

			case 56: // Shield lying on the floor
				_sprites->draw(*_map, 30, posX    , posY    ); // Floor
				_sprites->draw(*_map, 25, posX + 4, posY + 8); // Shield

				_map->fillRect(posX +  4, posY + 8, posX +  7, posY + 18, kColorFloor); // Area left to shield
				_map->fillRect(posX + 17, posY + 8, posX + 20, posY + 18, kColorFloor); // Area right to shield

				_shields.push_back(Position(x, y));
				break;

			case 57: // Start position
				_sprites->draw(*_map, 30, posX, posY);
				*mapTile = 0;

				_subTileX = x;
				_subTileY = y;

				_mapX = _subTileX * kMapTileWidth;
				_mapY = _subTileY * kMapTileHeight;
				break;
			}
		}
	}

	_mapUpdate = true;
}

void Penetration::initScreen() {
	_vm->_util->setFrameRate(15);

	memcpy(_vm->_draw->_vgaPalette     , kPalette, 48);
	memcpy(_vm->_draw->_vgaSmallPalette, kPalette, 48);

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	// Draw the shield meter
	_sprites->draw(*_background,   0,   0,  95,   6, 9, 117, 0); // Meter frame
	_sprites->draw(*_background, 271, 176, 282, 183, 9, 108, 0); // Shield

	// Draw the health meter
	_sprites->draw(*_background,   0,   0,  95,   6, 9, 135, 0); // Meter frame
	_sprites->draw(*_background, 283, 176, 292, 184, 9, 126, 0); // Heart

	_vm->_draw->_backSurface->blit(*_background);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);
}

int16 Penetration::checkInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons) {
	_vm->_util->getMouseState(&mouseX, &mouseY, &mouseButtons);

	return _vm->_util->checkKey();
}

bool Penetration::isWalkable(byte tile) const {
	// Only walls are nonwalkable

	if (tile == 50)
		return false;

	return true;
}

void Penetration::handleSub(int16 key) {
	if      (key == kKeyLeft)
		moveSub(-5,  0, kAnimationDriveW);
	else if (key == kKeyRight)
		moveSub( 5,  0, kAnimationDriveE);
	else if (key == kKeyUp)
		moveSub( 0, -5, kAnimationDriveN);
	else if (key == kKeyDown)
		moveSub( 0,  5, kAnimationDriveS);
}

void Penetration::moveSub(int x, int y, uint16 animation) {
	// Limit the movement to walkable tiles

	int16 minX = 0;
	if ((_subTileX > 0) && !isWalkable(_mapTiles[_subTileY * kMapWidth + (_subTileX - 1)]))
		minX = _subTileX * kMapTileWidth;

	int16 maxX = kMapWidth * kMapTileWidth;
	if ((_subTileX < (kMapWidth - 1)) && !isWalkable(_mapTiles[_subTileY * kMapWidth + (_subTileX + 1)]))
		maxX = _subTileX * kMapTileWidth;

	int16 minY = 0;
	if ((_subTileY > 0) && !isWalkable(_mapTiles[(_subTileY - 1) * kMapWidth + _subTileX]))
		minY = _subTileY * kMapTileHeight;

	int16 maxY = kMapHeight * kMapTileHeight;
	if ((_subTileY < (kMapHeight - 1)) && !isWalkable(_mapTiles[(_subTileY + 1) * kMapWidth + _subTileX]))
		maxY = _subTileY * kMapTileHeight;

	_mapX = CLIP<int16>(_mapX + x, minX, maxX);
	_mapY = CLIP<int16>(_mapY + y, minY, maxY);

	// The tile the sub is on is where its mid-point is
	_subTileX = (_mapX + (kMapTileWidth  / 2)) / kMapTileWidth;
	_subTileY = (_mapY + (kMapTileHeight / 2)) / kMapTileHeight;

	_mapUpdate = true;

	if (_sub->getAnimation() != animation)
		_sub->setAnimation(animation);

	checkShields();
}

void Penetration::checkShields() {
	for (Common::List<Position>::iterator pos = _shields.begin(); pos != _shields.end(); ++pos) {
		if ((pos->x == _subTileX) && (pos->y == _subTileY)) {
			// Charge shields
			_shieldMeter->setMaxValue();

			// Erase the shield from the map
			const int mapX = kPlayAreaBorderWidth  + pos->x * kMapTileWidth;
			const int mapY = kPlayAreaBorderHeight + pos->y * kMapTileHeight;
			_sprites->draw(*_map, 30, mapX, mapY);

			_shields.erase(pos);
			break;
		}
	}
}

void Penetration::updateAnims() {
	int16 left, top, right, bottom;

	// Clear the previous animation frames
	for (Common::List<ANIObject *>::iterator a = _anims.reverse_begin();
			 a != _anims.end(); --a) {

		(*a)->clear(*_vm->_draw->_backSurface, left, top, right, bottom);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
	}

	if (_mapUpdate) {
		_vm->_draw->_backSurface->blit(*_map, _mapX, _mapY,
				_mapX + kPlayAreaWidth - 1, _mapY + kPlayAreaHeight - 1, kPlayAreaX, kPlayAreaY);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, kPlayAreaX, kPlayAreaY,
				kPlayAreaX + kPlayAreaWidth - 1, kPlayAreaY + kPlayAreaHeight - 1);
	}

	_mapUpdate = false;

	// Draw the current animation frames
	for (Common::List<ANIObject *>::iterator a = _anims.begin();
			 a != _anims.end(); ++a) {

		(*a)->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

		(*a)->advance();
	}

	// Draw the meters
	_shieldMeter->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

	_healthMeter->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
}

} // End of namespace Geisha

} // End of namespace Gob
