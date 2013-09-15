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

/* CELER	The unit for updating the screen pics. */

#include "avalanche/avalanche.h"

#include "avalanche/celer.h"
#include "avalanche/animation.h"
#include "avalanche/lucerna.h"
#include "avalanche/gyro.h"
#include "avalanche/roomnums.h"

#include "common/textconsole.h"

namespace Avalanche {

const int16 Celer::kOnDisk = -1;

Celer::Celer(AvalancheEngine *vm) {
	_vm = vm;
	_spriteNum = 0;
}

Celer::~Celer() {
	forgetBackgroundSprites();
}

/**
 * @remarks	Originally called 'pics_link'
 */
void Celer::updateBackgroundSprites() {
	if (_vm->_gyro->_dropdownActive)
		return; // No animation when the menus are up.

	switch (_vm->_gyro->_room) {
	case kRoomOutsideArgentPub:
		if ((_vm->_gyro->_roomTime % 12) == 0)
			drawBackgroundSprite(-1, -1, 1 + (_vm->_gyro->_roomTime / 12) % 4);
		break;
	case kRoomBrummieRoad:
		if ((_vm->_gyro->_roomTime % 2) == 0)
			drawBackgroundSprite(-1, -1, 1 + (_vm->_gyro->_roomTime / 2) % 4);
		break;
	case kRoomBridge:
		if ((_vm->_gyro->_roomTime % 2) == 0)
			drawBackgroundSprite(-1, -1, 4 + (_vm->_gyro->_roomTime / 2) % 4);
		break;
	case kRoomYours:
		if ((!_vm->_gyro->_avvyIsAwake) && ((_vm->_gyro->_roomTime % 4) == 0))
			drawBackgroundSprite(-1, -1, 1 + (_vm->_gyro->_roomTime / 12) % 2);
		break;
	case kRoomArgentPub:
		if (((_vm->_gyro->_roomTime % 7) == 1) && (_vm->_gyro->_malagauche != 177)) {
			// Malagauche cycle.
			_vm->_gyro->_malagauche += 1;
			switch (_vm->_gyro->_malagauche) {
			case 1:
			case 11:
			case 21:
				drawBackgroundSprite(-1, -1, 12); // Looks forwards.
				break;
			case 8:
			case 18:
			case 28:
			case 32:
				drawBackgroundSprite(-1, -1, 11); // Looks at you.
				break;
			case 30:
				drawBackgroundSprite(-1, -1, 13); // Winks.
				break;
			case 33:
				_vm->_gyro->_malagauche = 0;
				break;
			}
		}

		switch (_vm->_gyro->_roomTime % 200) {
		case 179:
		case 197:
			drawBackgroundSprite(-1, -1, 5); // Dogfood's drinking cycle.
			break;
		case 182:
		case 194:
			drawBackgroundSprite(-1, -1, 6);
			break;
		case 185:
			drawBackgroundSprite(-1, -1, 7);
			break;
		case 199:
			_vm->_gyro->_npcFacing = 177; // Impossible value for this.
			break;
		}

		if ((_vm->_gyro->_roomTime % 200 >= 0) && (_vm->_gyro->_roomTime % 200 <= 178)) { // Normally.
			byte direction = 0;
			uint16 angle = _vm->_lucerna->bearing(1);
			if (((angle >= 1) && (angle <= 90)) || ((angle >= 358) && (angle <= 360)))
				direction = 3;
			else if ((angle >= 293) && (angle <= 357))
				direction = 2;
			else if ((angle >= 271) && (angle <= 292))
				direction = 4;

			if (direction != _vm->_gyro->_npcFacing) { // Dogfood.
				drawBackgroundSprite(-1, -1, direction);
				_vm->_gyro->_npcFacing = direction;
			}
		}
		break;
	case kRoomWestHall:
		if ((_vm->_gyro->_roomTime % 3) == 0) {
			switch ((_vm->_gyro->_roomTime / int32(3)) % int32(6)) {
			case 4:
				drawBackgroundSprite(-1, -1, 1);
				break;
			case 1:
			case 3:
			case 5:
				drawBackgroundSprite(-1, -1, 2);
				break;
			case 0:
			case 2:
				drawBackgroundSprite(-1, -1, 3);
				break;
			}
		}
		break;
	case kRoomLustiesRoom:
		if (!(_vm->_gyro->_lustieIsAsleep)) {
			byte direction = 0;
			uint16 angle = _vm->_lucerna->bearing(1);
			if ((_vm->_gyro->_roomTime % 45) > 42)
				direction = 4; // du Lustie blinks.
			// Bearing of Avvy from du Lustie.
			else if ((angle <= 45) || ((angle >= 315) && (angle <= 360)))
					direction = 1; // Middle.
			else if ((angle >= 45) && (angle <= 180))
					direction = 2; // Left.
			else if ((angle >= 181) && (angle <= 314))
				direction = 3; // Right.

			if (direction != _vm->_gyro->_npcFacing) { // du Lustie.
				drawBackgroundSprite(-1, -1, direction);
				_vm->_gyro->_npcFacing = direction;
			}
		}
		break;
	case kRoomAylesOffice:
		if ((!_vm->_gyro->_aylesIsAwake) && (_vm->_gyro->_roomTime % 14 == 0)) {
			switch ((_vm->_gyro->_roomTime / 14) % 2) {
			case 0:
				drawBackgroundSprite(-1, -1, 1);  // Frame 2: EGA.
				break;
			case 1:
				drawBackgroundSprite(-1, -1, 3); // Frame 1: Natural.
				break;
			}
		}
		break;
	case kRoomRobins:
		if (_vm->_gyro->_tiedUp) {
			switch (_vm->_gyro->_roomTime % 54) {
			case 20:
				drawBackgroundSprite(-1, -1, 4); // Frame 4: Avalot blinks.
				break;
			case 23:
				drawBackgroundSprite(-1, -1, 2); // Frame 1: Back to normal.
				break;
			}
		}
		break;
	case kRoomNottsPub: {
		// Bearing of Avvy from Port.
		byte direction = 0;
		uint16 angle = _vm->_lucerna->bearing(4);
		if ((angle <= 45) || ((angle >= 315) && (angle <= 360)))
			direction = 2; // Middle.
		else if ((angle >= 45) && (angle <= 180))
			direction = 6; // Left.
		else if ((angle >= 181) && (angle <= 314))
			direction = 8; // Right.

		if ((_vm->_gyro->_roomTime % 60) > 57)
			direction--; // Blinks.

		if (direction != _vm->_gyro->_npcFacing) { // Port.
			drawBackgroundSprite(-1, -1, direction);
			_vm->_gyro->_npcFacing = direction;
		}

		switch (_vm->_gyro->_roomTime % 50) {
		case 45 :
			drawBackgroundSprite(-1, -1, 9); // Spurge blinks.
			break;
		case 49 :
			drawBackgroundSprite(-1, -1, 10);
			break;
		}
		break;
	  }
	case kRoomDucks: {
		if ((_vm->_gyro->_roomTime % 3) == 0) // The fire flickers.
			drawBackgroundSprite(-1, -1, 1 + (_vm->_gyro->_roomTime / 3) % 3);

		// Bearing of Avvy from Duck.
		byte direction = 0;
		uint16 angle = _vm->_lucerna->bearing(1);
		if ((angle <= 45) || ((angle >= 315) && (angle <= 360)))
			direction = 4; // Middle.
		else if ((angle >= 45) && (angle <= 180))
			direction = 6; // Left.
		else if ((angle >= 181) && (angle <= 314))
			direction = 8; // Right.

		if ((_vm->_gyro->_roomTime % 45) > 42)
			direction++; // Duck blinks.

		if (direction != _vm->_gyro->_npcFacing) { // Duck.
			drawBackgroundSprite(-1, -1, direction);
			_vm->_gyro->_npcFacing = direction;
		}
		break;
	   }
	}

	if ((_vm->_gyro->_bellsAreRinging) && (_vm->_gyro->setFlag('B'))) {
		// They're ringing the bells.
		switch (_vm->_gyro->_roomTime % 4) {
		case 1:
			if (_vm->_gyro->_nextBell < 5)
				_vm->_gyro->_nextBell = 12;
			_vm->_gyro->_nextBell--;
			_vm->_gyro->note(_vm->_gyro->kNotes[_vm->_gyro->_nextBell]);
			break;
		case 2:
			//nosound();
			updateBackgroundSprites();
			break;
		}
	}
}

void Celer::loadBackgroundSprites(byte number) {
	Common::File f;
	_filename = _filename.format("chunk%d.avd", number);
	if (!f.open(_filename))
		return; // We skip because some rooms don't have sprites in the background.

	f.seek(44);
	_spriteNum = f.readByte();
	for (byte i = 0; i < _spriteNum; i++)
		_offsets[i] = f.readSint32LE();

	for (byte i = 0; i < _spriteNum; i++) {
		f.seek(_offsets[i]);

		SpriteType sprite;
		sprite._type = PictureType(f.readByte());
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
				_sprites[i]._picture.create(_sprites[i]._xl * 8, _sprites[i]._yl + 1, ::Graphics::PixelFormat::createFormatCLUT8());

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

void Celer::forgetBackgroundSprites() {
	for (byte i = 0; i < _spriteNum; i++) {
		if (_sprites[i]._x > kOnDisk)
			_sprites[i]._picture.free();
	}
}

void Celer::drawBackgroundSprite(int16 destX, int16 destY, byte which) {
	which--; // For the difference between the Pascal and C array indexes.
	//setactivepage(3);
	warning("STUB: Celer::show_one()");

	if (_sprites[which]._x > kOnDisk) {
		if (destX < 0) {
			destX = _sprites[which]._x * 8;
			destY = _sprites[which]._y;
		}
		drawSprite(destX, destY, _sprites[which]);
	} else {
		Common::File f;
		if (!f.open(_filename)) // Filename was set in loadBackgroundSprites().
			error("AVALANCHE: Celer: File not found: %s", _filename.c_str());

		f.seek(_offsets[which]);

		SpriteType sprite;
		sprite._type = PictureType(f.readByte());
		sprite._x = f.readSint16LE();
		sprite._y = f.readSint16LE();
		sprite._xl = f.readSint16LE();
		sprite._yl = f.readSint16LE();
		sprite._size = f.readSint32LE();
		f.skip(2); // For the now not existing natural and memorize data members of the SpriteType (called chunkblocktype in the original).
		sprite._picture = _vm->_graphics->loadPictureRow(f, sprite._xl * 8, sprite._yl + 1);

		if (destX < 0) {
			destX = sprite._x * 8;
			destY = sprite._y;
		}
		drawSprite(destX, destY, sprite);

		sprite._picture.free();
		f.close();
	}

	//setactivepage(1 - cp);
	warning("STUB: Celer::show_one()");
}



void Celer::drawSprite(int16 x, int16 y, SpriteType &sprite) {
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
	_vm->_graphics->drawPicture(_vm->_graphics->_background, sprite._picture, x, y - 10);
}

} // End of namespace Avalanche.
