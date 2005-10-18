/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/system.h"
#include "kyra/screen.h"
#include "kyra/kyra.h"
#include "kyra/sprites.h"
#include "kyra/resource.h"

namespace Kyra {

Sprites::Sprites(KyraEngine *engine, OSystem *system) {
	_engine = engine;
	_res = engine->resource();
	_screen = engine->screen();
	_system = system;
	_dat = 0;
	memset(_anims, 0, sizeof(_anims));
	_animDelay = 16;
}

Sprites::~Sprites() {
	delete[] _dat;
}

Sprite Sprites::getSprite(uint8 spriteID) {
	assert( spriteID < MAX_NUM_SPRITES);
	return _sprites[spriteID];
}

void Sprites::drawSprites(uint8 srcPage, uint8 dstPage) {
	int flags;

	for (int i = 0; i < MAX_NUM_ANIMS; i++) {
		if (_anims[i].script == 0 || !_anims[i].play)
			break;
		if (_anims[i].sprite >= 0) {
			assert( _anims[i].sprite < MAX_NUM_SPRITES);
			Sprite sprite = _sprites[_anims[i].sprite];

			//debug(1, "Drawing from X %i, Y %i, to X %i, Y %i, width %i, height %i, srcPage %i, dstPage %i",
			//	sprite.x, sprite.y, _anims[i].x, _anims[i].y, sprite.width, sprite.height, srcPage, dstPage);
			flags = Screen::CR_CLIPPED;
			if (_anims[i].flipX)
				flags |= Screen::CR_X_FLIPPED;
 
			_screen->copyRegion(sprite.x, sprite.y, _anims[i].x, _anims[i].y, sprite.width, sprite.height, srcPage, dstPage, flags);
		}
	}
}

void Sprites::doAnims() {
	uint32 currTime = _system->getMillis();
	for (int i = 0; i < MAX_NUM_ANIMS; i++) {
		if (_anims[i].script == 0 || !_anims[i].play || _anims[i].nextRun != 0 && _anims[i].nextRun > currTime)
			continue;

		uint8 *data;

		if (_anims[i].reentry == 0) {
			data = _anims[i].script;

			//debug(1, "---Start of anim script---");
			assert( READ_LE_UINT16(data) == 0xFF86 );
			data += 2;

			//debug(1, "Default X of sprite: %i", READ_LE_UINT16(data + 0x12) );
			_anims[i].x = READ_LE_UINT16(data + 0x12);
			//debug(1, "Default Y of sprite: %i", READ_LE_UINT16(data + 0x16) );
			_anims[i].y = READ_LE_UINT16(data + 0x16);

			//debug(1, "Anim %i flags: 22h: %i, 1ah: %i", i, READ_LE_UINT16(data + 0x22), READ_LE_UINT16(data + 0x1a));

			/*
			debug(1, "Anim %i data: 0h: %i, 2h: %i,4h: %i,6h: %i,8h: %i,ah: %i,ch: %i", i, READ_LE_UINT16(data + 0x0),
			READ_LE_UINT16(data + 0x2), READ_LE_UINT16(data + 0x4),READ_LE_UINT16(data + 0x6),READ_LE_UINT16(data + 0x8),
			READ_LE_UINT16(data + 0xa),READ_LE_UINT16(data + 0xc));

			debug(1, "Anim %i data: eh: %i, 10h: %i,12h: %i,14h: %i,16h: %i,18h: %i,1ah: %i", i, READ_LE_UINT16(data + 0xe),
			READ_LE_UINT16(data + 0x10), READ_LE_UINT16(data + 0x12),READ_LE_UINT16(data + 0x14),READ_LE_UINT16(data + 0x16),
			READ_LE_UINT16(data + 0x18),READ_LE_UINT16(data + 0x1a));

			debug(1, "Anim %i data: 1ch: %i, 1fh: %i,22h: %i,24h: %i,26h: %i,28h: %i,2ah: %i", i, READ_LE_UINT16(data + 0x1c),
			READ_LE_UINT16(data + 0x1f), READ_LE_UINT16(data + 0x22),READ_LE_UINT16(data + 0x24),READ_LE_UINT16(data + 0x26),
			READ_LE_UINT16(data + 0x28),READ_LE_UINT16(data + 0x2a));
			*/


			// TODO: Find out what the rest of this data (next 38h bytes) does.
			data += 0x38;
		} else {
			data = _anims[i].reentry;
			_anims[i].reentry = 0;
		}

		bool endLoop = false;
		
		while (READ_LE_UINT16(data) != 0xFF87 && !endLoop) {
			uint16 rndNr;
			assert((data - _anims[i].script) < _anims[i].length);
			switch (READ_LE_UINT16(data)) {
			case 0xFF88:
				data += 2;
				debug(5, "func: Set sprite image.");
				debug(5, "Sprite index %i", READ_LE_UINT16(data));
				_anims[i].sprite = READ_LE_UINT16(data);
				data += 2;
				//debug(5, "Unused %i", READ_LE_UINT16(data));
				data += 2;
				debug(5, "X %i", READ_LE_UINT16(data));
				_anims[i].x = READ_LE_UINT16(data);
				data += 2;
				debug(5, "Y %i", READ_LE_UINT16(data));
				_anims[i].y = READ_LE_UINT16(data);
				data += 2;
				_anims[i].flipX = false;
				break;
			case 0xFF8D:
				data += 2;
				debug(5, "func: Set sprite image, flipped.");
				debug(5, "Sprite index %i", READ_LE_UINT16(data));
				_anims[i].sprite = READ_LE_UINT16(data);
				data += 2;
				//debug(9, "Unused %i", READ_LE_UINT16(data));
				data += 2;
				debug(5, "X %i", READ_LE_UINT16(data));
				_anims[i].x = READ_LE_UINT16(data);
				data += 2;
				debug(5, "Y %i", READ_LE_UINT16(data));
				_anims[i].y = READ_LE_UINT16(data);
				data += 2;
				_anims[i].flipX = true;
				break;
			case 0xFF8A:
				data += 2;
				debug(5, "func: Set time to wait");
				debug(5, "Time %i", READ_LE_UINT16(data));
				_anims[i].nextRun = _system->getMillis() + READ_LE_UINT16(data) * _animDelay;
				data += 2;
				break;
			case 0xFFB3:
				data += 2;
				debug(5, "func: Set time to wait to random value");
				rndNr = READ_LE_UINT16(data) + _rnd.getRandomNumber( READ_LE_UINT16(data) + 2);
				debug(5, "Minimum time %i", READ_LE_UINT16(data));
				data += 2;
				debug(5, "Maximum time %i", READ_LE_UINT16(data));
				data += 2;
				_anims[i].nextRun = _system->getMillis() + rndNr * _animDelay; 								
				break;
			case 0xFF8C:
				data += 2;
				debug(5, "func: Wait until wait time has elapsed");
				_anims[i].reentry = data;
				endLoop = true;
				//assert( _anims[i].nextRun > _system->getMillis());
				break;
			case 0xFF99:
				data += 2;
				debug(1, "TODO func: Set value of animation property 32h to 1");
				break;
			case 0xFF9A:
				data += 2;
				debug(1, "TODO func: Set value of animation property 32h to 0");
				break;		
			case 0xFF97:
				data += 2;
				debug(5, "func: Set default X coordinate of sprite");
				debug(5, "X %i", READ_LE_UINT16(data));
				_anims[i].x = READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF98:
				data += 2;
				debug(5, "func: Set default Y coordinate of sprite");
				debug(5, "Y %i", READ_LE_UINT16(data));
				_anims[i].y = READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF8B:
				debug(5, "func: Jump to start of script section");
				//data = scriptStart;
				_anims[i].nextRun = _system->getMillis();
				endLoop = true;
				break;
			case 0xFF8E:
				data += 2;
				debug(5, "func: Begin for () loop");
				debug(5, "Iterations: %i", READ_LE_UINT16(data));
				_anims[i].loopsLeft = READ_LE_UINT16(data);
				data += 2;
				_anims[i].loopStart = data;
				break;
			case 0xFF8F:
				data += 2;
				debug(5, "func: End for () loop");
				if (_anims[i].loopsLeft > 0) {
					_anims[i].loopsLeft--;
					data = _anims[i].loopStart;
				}
				break;
			case 0xFF90:
				data += 2;
				debug(5, "func: Set sprite image using default X and Y");
				debug(5, "Sprite index %i", READ_LE_UINT16(data));
				_anims[i].sprite = READ_LE_UINT16(data);
				_anims[i].flipX = false;
				data += 2;
				break;
			case 0xFF91:
				data += 2;
				debug(5, "func: Set sprite image using default X and Y, flipped.");
				debug(5, "Sprite index %i", READ_LE_UINT16(data));
				_anims[i].sprite = READ_LE_UINT16(data);
				_anims[i].flipX = true;
				data += 2;
				break;
			case 0xFF92:
				data += 2;
				debug(5, "func: Increase value of default X-coordinate");
				debug(5, "Increment %i", READ_LE_UINT16(data));
				_anims[i].x += READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF93:
				data += 2;
				debug(5, "func: Increase value of default Y-coordinate");
				debug(5, "Increment %i", READ_LE_UINT16(data));
				_anims[i].y += READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF94:
				data += 2;
				debug(5, "func: Decrease value of default X-coordinate");
				debug(5, "Decrement %i", READ_LE_UINT16(data));
				_anims[i].x -= READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF95:
				data += 2;
				debug(5, "func: Decrease value of default Y-coordinate");
				debug(5, "Decrement %i", READ_LE_UINT16(data));
				_anims[i].y -= READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF96:
				data += 2;
				debug(9, "func: Stop animation");
				debug(9, "Animation index %i", READ_LE_UINT16(data));
				uint16 anim = READ_LE_UINT16(data);
				data += 2;
				_anims[anim].play = false;
				_anims[anim].sprite = -1;
				//debug(1, "Arg2 %i", READ_LE_UINT16(data));
				//data += 2;
				break;
/*			case 0xFF97:
				data += 2;
				debug(1, "func: Set value of animation property 34h to 0");
				break;*/
			case 0xFFAD:
				data += 2;
				debug(1, "TODO func: Set Brandon's X coordinate");
				debug(1, "X %i", READ_LE_UINT16(data));
				data += 2;
				break;
			case 0xFFAE:
				data += 2;
				debug(1, "TODO func: Set Brandon's Y coordinate");
				debug(1, "Y %i", READ_LE_UINT16(data));
				data += 2;
				break;
			case 0xFFAF:
				data += 2;
				debug(1, "TODO func: Set Brandon's X sprite");
				debug(1, "Sprite %i", READ_LE_UINT16(data));
				data += 2;
				break;
			case 0xFFAA:
				data += 2;
				debug(1, "TODO func: Reset Brandon's sprite");
				break;
			case 0xFFAB:
				data += 2;
				debug(1, "TODO func: Update Brandon's sprite");
				break;
			case 0xFFB0:
				data += 2;
				debug(1, "TODO func: Play sound");
				debug(1, "Sound index %i", READ_LE_UINT16(data));
				data += 2;
				break;
			case 0xFFB1:
				data += 2;
				debug(1, "TODO func: Set unknown global bit");
				break;
			case 0xFFB2:
				data += 2;
				debug(1, "TODO func: Reset unknown global bit");
				break;
			case 0xFFB4:
				data += 2;
				debug(1, "TODO func: Play (at random) a certain sound at a certain percentage of time");
				debug(1, "Sound index %i", READ_LE_UINT16(data));
				data += 2;
				debug(1, "Percentage %i", READ_LE_UINT16(data));
				data += 2;
				break;
			case 0xFFA7:
				data += 2;
				debug(1, "TODO func: Unknown FFA7");
				debug(1, " %i", READ_LE_UINT16(data));
				data += 2;
				break;
			default:
				debug(1, "Unsupported anim command %X", READ_LE_UINT16(data));
				//endLoop = true;
				data += 1;
				break;
			}
		}

		if (READ_LE_UINT16(data) == 0xFF87)
			_anims[i].play = false;

		//debug(1, "---End of anim script---");
	}
}

void Sprites::loadDAT(const char *filename) {
	debug(9, "Sprites::loadDat('%s')", filename);
	uint32 fileSize;
	uint8 spritesLoaded = 0;

	delete[] _dat;

	_dat = _res->fileData(filename, &fileSize);

	memset(_anims, 0, sizeof(Anim) * MAX_NUM_ANIMS);
	uint8 nextAnim = 0;

	assert(fileSize > 0x6D);

	memcpy(_screen->_currentPalette + 745 - 0x3D, _dat + 0x17, 0x3D);
	_screen->setScreenPalette(_screen->_currentPalette);
	uint8 *data = _dat + 0x6B;

	uint16 length = READ_LE_UINT16(data);
	data += 2;

	//debug(1, "DAT body length: %i, filesize %i, current spot %i", length, fileSize, data - _dat);

	if (length > 2) {
		assert( length < fileSize);
		uint8 *animstart;
		uint8 *start = data;

		while (1) {
			if (((uint16)(data - _dat) >= fileSize) || (data - start) >= length)
				break;	

			if (READ_LE_UINT16(data) == 0xFF83) {
				//debug(1, "Body section end.");
				data += 2;
				break;
			}

			switch (READ_LE_UINT16(data)) {
			case 0xFF81:
				data += 2;
				//debug(1, "Body section start");
				break;
			case 0xFF82:
				data += 2;
				//debug(1, "Unknown 0xFF82 section");
				break;
			case 0xFF84:
				data += 2;
				while (READ_LE_UINT16(data) != 0xFF85) {
					uint16 spriteNum = READ_LE_UINT16(data);
					//debug(1, "Spritenum: %i", spriteNum);
					assert(spriteNum < MAX_NUM_SPRITES);
					data += 2;
					_sprites[spriteNum].x = READ_LE_UINT16(data) * 8;
					data += 2;
					_sprites[spriteNum].y = READ_LE_UINT16(data);
					data += 2;
					_sprites[spriteNum].width = READ_LE_UINT16(data) * 8;
					data += 2;
					_sprites[spriteNum].height = READ_LE_UINT16(data);
					data += 2;
					spritesLoaded++;
					//debug(1, "Got sprite index: %i", spriteNum);
					//debug(1, "X: %i", _sprites[spriteNum].x);
					//debug(1, "Y: %i", _sprites[spriteNum].y);
					//debug(1, "Width: %i", _sprites[spriteNum].width);
					//debug(1, "Height: %i", _sprites[spriteNum].height);
				}
				//debug(1, "End of sprite images.");
				data += 2;
				break;
			case 0xFF86:
				assert(nextAnim < MAX_NUM_ANIMS);
				_anims[nextAnim].script = data;
				_anims[nextAnim].sprite = -1;
				_anims[nextAnim].play = true;
				animstart = data;
				data += 2;
				while (READ_LE_UINT16(data) != 0xFF87) {
					assert((uint16)(data - _dat) < fileSize);
					data += 2;
				}
				_anims[nextAnim].length = data - animstart;
				//debug(1, "Found an anim script of length %i!", _anims[nextAnim].length);
				nextAnim++;
				data += 2;
				break;
			default:
				debug(1, "Unknown code in DAT file: %x", READ_LE_UINT16(data));
				data += 2;
				break;
			}
		}
	} else {
		data += 2;
	}

	debug(1, "Room DAT file loaded. Found %i sprite and %i animation scripts.", spritesLoaded, nextAnim);
	
	//debug(1, "Remainder after script: %i", fileSize - (data - _dat));
	assert(fileSize - (data - _dat) == 0xC);

	//TODO: Read in character entry coords here
}

} // end of namespace Kyra
