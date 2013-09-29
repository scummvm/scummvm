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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* Original name: CELER		The unit for updating the screen pics. */

#include "avalanche/avalanche.h"
#include "avalanche/background.h"
#include "avalanche/animation.h"

#include "common/textconsole.h"

namespace Avalanche {

const int16 Background::kOnDisk = -1;

Background::Background(AvalancheEngine *vm) {
	_vm = vm;
	_spriteNum = 0;
}

Background::~Background() {
	release();
}

/**
 * @remarks	Originally called 'pics_link'
 */
void Background::update() {
	if (_vm->_menu->isActive())
		return; // No animation when the menus are up.

	switch (_vm->_room) {
	case kRoomOutsideArgentPub:
		if ((_vm->_roomTime % 12) == 0)
			draw(-1, -1, (_vm->_roomTime / 12) % 4);
		break;
	case kRoomBrummieRoad:
		if ((_vm->_roomTime % 2) == 0)
			draw(-1, -1, (_vm->_roomTime / 2) % 4);
		break;
	case kRoomBridge:
		if ((_vm->_roomTime % 2) == 0)
			draw(-1, -1, 3 + (_vm->_roomTime / 2) % 4);
		break;
	case kRoomYours:
		if ((!_vm->_avvyIsAwake) && ((_vm->_roomTime % 4) == 0))
			draw(-1, -1, (_vm->_roomTime / 12) % 2);
		break;
	case kRoomArgentPub:
		if (((_vm->_roomTime % 7) == 1) && (_vm->_malagauche != 177)) {
			// Malagauche cycle.
			_vm->_malagauche++;
			switch (_vm->_malagauche) {
			case 1:
			case 11:
			case 21:
				draw(-1, -1, 11); // Looks forwards.
				break;
			case 8:
			case 18:
			case 28:
			case 32:
				draw(-1, -1, 10); // Looks at you.
				break;
			case 30:
				draw(-1, -1, 12); // Winks.
				break;
			case 33:
				_vm->_malagauche = 0;
				break;
			}
		}

		switch (_vm->_roomTime % 200) {
		case 179:
		case 197:
			draw(-1, -1, 4); // Dogfood's drinking cycle.
			break;
		case 182:
		case 194:
			draw(-1, -1, 5);
			break;
		case 185:
			draw(-1, -1, 6);
			break;
		case 199:
			_vm->_npcFacing = 177; // Impossible value for this.
			break;
		}

		if ((_vm->_roomTime % 200 >= 0) && (_vm->_roomTime % 200 <= 178)) { // Normally.
			byte direction = 0;
			uint16 angle = _vm->bearing(1);
			if (((angle >= 1) && (angle <= 90)) || ((angle >= 358) && (angle <= 360)))
				direction = 3;
			else if ((angle >= 293) && (angle <= 357))
				direction = 2;
			else if ((angle >= 271) && (angle <= 292))
				direction = 4;

			if (direction != _vm->_npcFacing) { // Dogfood.
				draw(-1, -1, direction - 1);
				_vm->_npcFacing = direction;
			}
		}
		break;
	case kRoomWestHall:
		if ((_vm->_roomTime % 3) == 0) {
			switch ((_vm->_roomTime / int32(3)) % int32(6)) {
			case 4:
				draw(-1, -1, 0);
				break;
			case 1:
			case 3:
			case 5:
				draw(-1, -1, 1);
				break;
			case 0:
			case 2:
				draw(-1, -1, 2);
				break;
			}
		}
		break;
	case kRoomLustiesRoom:
		if (!(_vm->_lustieIsAsleep)) {
			byte direction = 0;
			uint16 angle = _vm->bearing(1);
			if ((_vm->_roomTime % 45) > 42)
				direction = 4; // du Lustie blinks.
			// Bearing of Avvy from du Lustie.
			else if ((angle <= 45) || ((angle >= 315) && (angle <= 360)))
					direction = 1; // Middle.
			else if ((angle >= 45) && (angle <= 180))
					direction = 2; // Left.
			else if ((angle >= 181) && (angle <= 314))
				direction = 3; // Right.

			if (direction != _vm->_npcFacing) { // du Lustie.
				draw(-1, -1, direction - 1);
				_vm->_npcFacing = direction;
			}
		}
		break;
	case kRoomAylesOffice:
		if ((!_vm->_aylesIsAwake) && (_vm->_roomTime % 14 == 0)) {
			switch ((_vm->_roomTime / 14) % 2) {
			case 0:
				draw(-1, -1, 0);  // Frame 2: EGA.
				break;
			case 1:
				draw(-1, -1, 2); // Frame 1: Natural.
				break;
			}
		}
		break;
	case kRoomRobins:
		if (_vm->_tiedUp) {
			switch (_vm->_roomTime % 54) {
			case 20:
				draw(-1, -1, 3); // Frame 4: Avalot blinks.
				break;
			case 23:
				draw(-1, -1, 1); // Frame 1: Back to normal.
				break;
			}
		}
		break;
	case kRoomNottsPub: {
		// Bearing of Avvy from Port.
		byte direction = 0;
		uint16 angle = _vm->bearing(4);
		if ((angle <= 45) || ((angle >= 315) && (angle <= 360)))
			direction = 2; // Middle.
		else if ((angle >= 45) && (angle <= 180))
			direction = 6; // Left.
		else if ((angle >= 181) && (angle <= 314))
			direction = 8; // Right.

		if ((_vm->_roomTime % 60) > 57)
			direction--; // Blinks.

		if (direction != _vm->_npcFacing) { // Port.
			draw(-1, -1, direction - 1);
			_vm->_npcFacing = direction;
		}

		switch (_vm->_roomTime % 50) {
		case 45 :
			draw(-1, -1, 8); // Spurge blinks.
			break;
		case 49 :
			draw(-1, -1, 9);
			break;
		}
		break;
	  }
	case kRoomDucks: {
		if ((_vm->_roomTime % 3) == 0) // The fire flickers.
			draw(-1, -1, (_vm->_roomTime / 3) % 3);

		// Bearing of Avvy from Duck.
		byte direction = 0;
		uint16 angle = _vm->bearing(1);
		if ((angle <= 45) || ((angle >= 315) && (angle <= 360)))
			direction = 4; // Middle.
		else if ((angle >= 45) && (angle <= 180))
			direction = 6; // Left.
		else if ((angle >= 181) && (angle <= 314))
			direction = 8; // Right.

		if ((_vm->_roomTime % 45) > 42)
			direction++; // Duck blinks.

		if (direction != _vm->_npcFacing) { // Duck.
			draw(-1, -1, direction - 1);
			_vm->_npcFacing = direction;
		}
		break;
	   }
	default:
		break;
	}

	if ((_vm->_bellsAreRinging) && (_vm->setFlag('B'))) {
		// They're ringing the bells.
		switch (_vm->_roomTime % 4) {
		case 1:
			if (_vm->_nextBell < 5)
				_vm->_nextBell = 12;
			_vm->_nextBell--;
			// CHECKME: 2 is a guess. No length in the original? 
			_vm->_sound->playNote(_vm->kNotes[_vm->_nextBell], 2);
			break;
		case 2:
			_vm->_sound->stopSound();
			break;
		}
	}
}

void Background::load(byte number) {
	Common::File f;
	_filename = _filename.format("chunk%d.avd", number);
	if (!f.open(_filename))
		return; // We skip because some rooms don't have sprites in the background.

	f.seek(44);
	_spriteNum = f.readByte();
	for (int i = 0; i < _spriteNum; i++)
		_offsets[i] = f.readSint32LE();

	for (int i = 0; i < _spriteNum; i++) {
		f.seek(_offsets[i]);

		SpriteType sprite;
		sprite._type = (PictureType)(f.readByte());
		sprite._x = f.readSint16LE();
		sprite._y = f.readSint16LE();
		sprite._xl = f.readSint16LE();
		sprite._yl = f.readSint16LE();
		sprite._size = f.readSint32LE();
		bool natural = f.readByte();
		bool memorize = f.readByte();

		if (memorize) {
			_sprites[i]._x = sprite._x;
			_sprites[i]._xl = sprite._xl;
			_sprites[i]._y = sprite._y;
			_sprites[i]._yl = sprite._yl;
			_sprites[i]._type = sprite._type;

			if (natural) {
				_sprites[i]._type = kNaturalImage; // We simply read from the screen and later, in drawSprite() we draw it right back.
				_sprites[i]._size = _sprites[i]._xl * 8 * _sprites[i]._yl + 1;
				_sprites[i]._picture.create(_sprites[i]._xl * 8, _sprites[i]._yl + 1, Graphics::PixelFormat::createFormatCLUT8());

				for (uint16 y = 0; y < _sprites[i]._yl + 1; y++) {
					for (uint16 x = 0; x < _sprites[i]._xl * 8; x++)
						*(byte *)_sprites[i]._picture.getBasePtr(x, y) = *(byte *)_vm->_graphics->_surface.getBasePtr(_sprites[i]._x * 8 + x, _sprites[i]._y + y);
				}
			} else {
				_sprites[i]._size = sprite._size;
				_sprites[i]._picture = _vm->_graphics->loadPictureRow(f, _sprites[i]._xl * 8, _sprites[i]._yl + 1);
			}
		} else
			_sprites[i]._x = kOnDisk;
	}
	f.close();
}

void Background::release() {
	for (int i = 0; i < _spriteNum; i++) {
		if (_sprites[i]._x > kOnDisk)
			_sprites[i]._picture.free();
	}
}

/**
 * Draw background animation
 * @remarks	Originally called 'show_one'
 */
void Background::draw(int16 destX, int16 destY, byte sprId) {
	if (_sprites[sprId]._x > kOnDisk) {
		if (destX < 0) {
			destX = _sprites[sprId]._x * 8;
			destY = _sprites[sprId]._y;
		}
		drawSprite(destX, destY, _sprites[sprId]);
	} else {
		Common::File f;
		if (!f.open(_filename)) // Filename was set in loadBackgroundSprites().
			return; // We skip because some rooms don't have sprites in the background.

		f.seek(_offsets[sprId]);

		SpriteType sprite;
		sprite._type = (PictureType)(f.readByte());
		sprite._x = f.readSint16LE();
		sprite._y = f.readSint16LE();
		sprite._xl = f.readSint16LE();
		sprite._yl = f.readSint16LE();
		sprite._size = f.readSint32LE();
		f.skip(2); // Natural and Memorize are used in Load()
		sprite._picture = _vm->_graphics->loadPictureRow(f, sprite._xl * 8, sprite._yl + 1);

		if (destX < 0) {
			destX = sprite._x * 8;
			destY = sprite._y;
		}
		drawSprite(destX, destY, sprite);

		sprite._picture.free();
		f.close();
	}
}

void Background::drawSprite(int16 x, int16 y, SpriteType &sprite) {
	_r._x1 = x;
	_r._y1 = y;
	_r._y2 = y + sprite._yl;

	switch (sprite._type) {
	case kNaturalImage: // Allow fallthrough on purpose.
	case kBgi:
		_r._x2 = x + sprite._xl + 1;
		break;
	case kEga:
		_r._x2 = x + sprite._xl;
		break;
	}

	// These pictures are practically parts of the background. -10 is for the drop-down menu.
	_vm->_graphics->drawBackgroundSprite(x, y - 10, sprite);
}

} // End of namespace Avalanche.
