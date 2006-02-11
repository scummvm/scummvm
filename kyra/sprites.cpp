/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $URL$
 * $Id$
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
#include "kyra/animator.h"

namespace Kyra {

Sprites::Sprites(KyraEngine *engine, OSystem *system) {
	_engine = engine;
	_res = engine->resource();
	_screen = engine->screen();
	_system = system;
	_dat = 0;
	memset(_anims, 0, sizeof(_anims));
	memset(_sceneShapes, 0, sizeof(_sceneShapes));
	_animDelay = 16;
	_spriteDefStart = 0;
	memset(_drawLayerTable, 0, sizeof(_drawLayerTable));
	_sceneAnimatorBeaconFlag = 0;
}

Sprites::~Sprites() {
	delete[] _dat;
	freeSceneShapes();
	for (int i = 0; i < MAX_NUM_ANIMS; i++) {
		if (_anims[i].background)
			free(_anims[i].background);
	}
}

void Sprites::setupSceneAnims() {
	debug(9, "Sprites::setupSceneAnims()");
	uint8 *data;

	for (int i = 0; i < MAX_NUM_ANIMS; i++) {
		if (_anims[i].background) {
			free(_anims[i].background);
			_anims[i].background = 0;
		}

		if (_anims[i].script != 0) {
			data = _anims[i].script;

			assert( READ_LE_UINT16(data) == 0xFF86 );
			data += 4;

			_anims[i].disable = READ_LE_UINT16(data) != 0;
			data += 4;
			_anims[i].unk2 = READ_LE_UINT16(data);
			data += 4;

			if (_engine->_northExitHeight > READ_LE_UINT16(data))
				_anims[i].drawY = _engine->_northExitHeight;
			else
				_anims[i].drawY = READ_LE_UINT16(data);
			data += 4;

			//sceneUnk2[i] = READ_LE_UINT16(data);
			data += 4;
			
			_anims[i].x = READ_LE_UINT16(data);
			data += 4;
			_anims[i].y = READ_LE_UINT16(data);
			data += 4;
			_anims[i].width = *(data) - 1;
			data += 4;
			_anims[i].height = *(data);
			data += 4;
			_anims[i].sprite = READ_LE_UINT16(data);
			data += 4;
			_anims[i].flipX = READ_LE_UINT16(data) != 0;
			data += 4;
			_anims[i].width2 = *(data);
			data += 4;
			_anims[i].height2 = *(data);
			data += 4;
			_anims[i].unk1 = READ_LE_UINT16(data) != 0;
			data += 4;
			_anims[i].play = READ_LE_UINT16(data) != 0;
			data += 2;

			_anims[i].script = data;

			int bkgdWidth = _anims[i].width << 3;
			int bkgdHeight = _anims[i].height;

			if (_anims[i].width2)
				bkgdWidth += _anims[i].width2 << 3;

			if (_anims[i].height2)
				bkgdHeight += _anims[i].height2;

			_anims[i].background = (uint8 *)malloc(_screen->getRectSize(bkgdWidth + 1, bkgdHeight));
			memset(_anims[i].background, 0, _screen->getRectSize(bkgdWidth + 1, bkgdHeight));

			assert(_anims[i].background);
		}
	}
}

void Sprites::updateSceneAnims() {
	debug(9, "Sprites::updateSceneAnims()");
	uint32 currTime = _system->getMillis();
	uint8 *data;
	bool endLoop;
	uint16 rndNr;
	uint16 anim;
	uint16 sound;

	for (int i = 0; i < MAX_NUM_ANIMS; i++) {
		if (_anims[i].script == 0 || !_anims[i].play || _anims[i].nextRun != 0 && _anims[i].nextRun > currTime)
			continue;

		if (_anims[i].reentry == 0) {
			data = _anims[i].script;
			if (READ_LE_UINT16(data) == 0xFF8B)
				continue;
		} else {
			data = _anims[i].reentry;
			_anims[i].reentry = 0;
		}

		endLoop = false;
		while (READ_LE_UINT16(data) != 0xFF87 && !endLoop) {
			assert((data - _anims[i].script) < _anims[i].length);
			switch (READ_LE_UINT16(data)) {
			case 0xFF88:
				data += 2;
				debug(6, "func: Set sprite image.");
				debug(6, "Sprite index %i", READ_LE_UINT16(data));
				_anims[i].sprite = READ_LE_UINT16(data);
				data += 2;
				//debug(6, "Unused %i", READ_LE_UINT16(data));
				data += 2;
				debug(6, "X %i", READ_LE_UINT16(data));
				_anims[i].x = READ_LE_UINT16(data);
				data += 2;
				debug(6, "Y %i", READ_LE_UINT16(data));
				_anims[i].y = READ_LE_UINT16(data);
				data += 2;
				_anims[i].flipX = false;
				refreshSceneAnimObject(i, _anims[i].sprite, _anims[i].x, _anims[i].y, _anims[i].flipX, _anims[i].unk1 != 0);
				break;
			case 0xFF8D:
				data += 2;
				debug(6, "func: Set sprite image, flipped.");
				debug(6, "Sprite index %i", READ_LE_UINT16(data));
				_anims[i].sprite = READ_LE_UINT16(data);
				data += 2;
				//debug(9, "Unused %i", READ_LE_UINT16(data));
				data += 2;
				debug(6, "X %i", READ_LE_UINT16(data));
				_anims[i].x = READ_LE_UINT16(data);
				data += 2;
				debug(6, "Y %i", READ_LE_UINT16(data));
				_anims[i].y = READ_LE_UINT16(data);
				data += 2;
				_anims[i].flipX = true;
				refreshSceneAnimObject(i, _anims[i].sprite, _anims[i].x, _anims[i].y, _anims[i].flipX, _anims[i].unk1 != 0);
				break;
			case 0xFF8A:
				data += 2;
				debug(6, "func: Set time to wait");
				debug(6, "Time %i", READ_LE_UINT16(data));
				_anims[i].nextRun = _system->getMillis() + READ_LE_UINT16(data) * _animDelay;
				data += 2;
				break;
			case 0xFFB3:
				data += 2;
				debug(6, "func: Set time to wait to random value");
				rndNr = READ_LE_UINT16(data) + _rnd.getRandomNumber( READ_LE_UINT16(data) + 2);
				debug(6, "Minimum time %i", READ_LE_UINT16(data));
				data += 2;
				debug(6, "Maximum time %i", READ_LE_UINT16(data));
				data += 2;
				_anims[i].nextRun = _system->getMillis() + rndNr * _animDelay;
				break;
			case 0xFF8C:
				data += 2;
				debug(6, "func: Wait until wait time has elapsed");
				_anims[i].reentry = data;
				endLoop = true;
				//assert( _anims[i].nextRun > _system->getMillis());
				break;
			case 0xFF99:
				data += 2;
				debug(1, "func: Set value of unknown animation property to 1");
				_anims[i].unk1 = 1;
				break;
			case 0xFF9A:
				data += 2;
				debug(1, "func: Set value of unknown animation property to 0");
				_anims[i].unk1 = 0;
				break;		
			case 0xFF97:
				data += 2;
				debug(6, "func: Set default X coordinate of sprite");
				debug(6, "X %i", READ_LE_UINT16(data));
				_anims[i].x = READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF98:
				data += 2;
				debug(6, "func: Set default Y coordinate of sprite");
				debug(6, "Y %i", READ_LE_UINT16(data));
				_anims[i].y = READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF8B:
				debug(6, "func: Jump to start of script section");
				//data = scriptStart;
				_anims[i].nextRun = _system->getMillis();
				endLoop = true;
				break;
			case 0xFF8E:
				data += 2;
				debug(6, "func: Begin for () loop");
				debug(6, "Iterations: %i", READ_LE_UINT16(data));
				_anims[i].loopsLeft = READ_LE_UINT16(data);
				data += 2;
				_anims[i].loopStart = data;
				break;
			case 0xFF8F:
				data += 2;
				debug(6, "func: End for () loop");
				if (_anims[i].loopsLeft > 0) {
					_anims[i].loopsLeft--;
					data = _anims[i].loopStart;
				}
				break;
			case 0xFF90:
				data += 2;
				debug(6, "func: Set sprite image using default X and Y");
				debug(6, "Sprite index %i", READ_LE_UINT16(data));
				_anims[i].sprite = READ_LE_UINT16(data);
				_anims[i].flipX = false;
				data += 2;
				refreshSceneAnimObject(i, _anims[i].sprite, _anims[i].x, _anims[i].y, _anims[i].flipX, _anims[i].unk1 != 0);
				break;
			case 0xFF91:
				data += 2;
				debug(6, "func: Set sprite image using default X and Y, flipped.");
				debug(6, "Sprite index %i", READ_LE_UINT16(data));
				_anims[i].sprite = READ_LE_UINT16(data);
				_anims[i].flipX = true;
				data += 2;
				refreshSceneAnimObject(i, _anims[i].sprite, _anims[i].x, _anims[i].y, _anims[i].flipX, _anims[i].unk1 != 0);
				break;
			case 0xFF92:
				data += 2;
				debug(6, "func: Increase value of default X-coordinate");
				debug(6, "Increment %i", READ_LE_UINT16(data));
				_anims[i].x += READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF93:
				data += 2;
				debug(6, "func: Increase value of default Y-coordinate");
				debug(6, "Increment %i", READ_LE_UINT16(data));
				_anims[i].y += READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF94:
				data += 2;
				debug(6, "func: Decrease value of default X-coordinate");
				debug(6, "Decrement %i", READ_LE_UINT16(data));
				_anims[i].x -= READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF95:
				data += 2;
				debug(6, "func: Decrease value of default Y-coordinate");
				debug(6, "Decrement %i", READ_LE_UINT16(data));
				_anims[i].y -= READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFF96:
				data += 2;
				debug(9, "func: Stop animation");
				debug(9, "Animation index %i", READ_LE_UINT16(data));
				anim = READ_LE_UINT16(data);
				data += 2;
				_anims[anim].play = false;
				_anims[anim].sprite = -1;
				break;
/*			case 0xFF97:
				data += 2;
				debug(1, "func: Set value of animation property 34h to 0");
				break;*/
			case 0xFFAD:
				data += 2;
				debug(6, "func: Set Brandon's X coordinate");
				debug(6, "X %i", READ_LE_UINT16(data));
				_engine->_currentCharacter->x1 = READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFFAE:
				data += 2;
				debug(6, "func: Set Brandon's Y coordinate");
				debug(6, "Y %i", READ_LE_UINT16(data));
				_engine->_currentCharacter->y1 = READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFFAF:
				data += 2;
				debug(6, "func: Set Brandon's sprite");
				debug(6, "Sprite %i", READ_LE_UINT16(data));
				_engine->_currentCharacter->currentAnimFrame = READ_LE_UINT16(data);
				data += 2;
				break;
			case 0xFFAA:
				data += 2;
				debug(1, "TODO func: Reset Brandon's sprite");
				break;
			case 0xFFAB:
				data += 2;
				debug(6, "func: Update Brandon's sprite");
				_engine->animRefreshNPC(0);
				_engine->animator()->flagAllObjectsForRefresh();
				_engine->animator()->updateAllObjectShapes();
				break;
			case 0xFFB0:
				data += 2;
				debug(6, "func: Play sound");
				debug(6, "Sound index %i", READ_LE_UINT16(data));
				_engine->snd_playSoundEffect(READ_LE_UINT16(data));
				data += 2;
				break;
			case 0xFFB1:
				data += 2;
				_sceneAnimatorBeaconFlag = 1;
				break;
			case 0xFFB2:
				data += 2;
				_sceneAnimatorBeaconFlag = 0;
				break;
			case 0xFFB4:
				data += 2;
				debug(6, "func: Play (at random) a certain sound at a certain percentage of time");
				debug(6, "Sound index %i", READ_LE_UINT16(data));
				sound = READ_LE_UINT16(data);
				data += 2;
				debug(6, "Percentage %i", READ_LE_UINT16(data));
				rndNr = _rnd.getRandomNumber(100);
				if (rndNr <= READ_LE_UINT16(data))
					_engine->snd_playSoundEffect(sound);
				data += 2;
				break;
			case 0xFFA7:
				data += 2;
				_anims[READ_LE_UINT16(data)].play = 1;
				data += 2;
				break;
			default:
				debug(1, "Unsupported anim command %X in script %i", READ_LE_UINT16(data), i);
				//endLoop = true;
				data += 1;
				break;
			}
		}

		if (READ_LE_UINT16(data) == 0xFF87)
			_anims[i].play = false;
	}
}

void Sprites::loadDAT(const char *filename, SceneExits &exits) {
	debug(9, "Sprites::loadDat('%s')", filename);
	uint32 fileSize;

	delete[] _dat;
	_spriteDefStart = 0;

	_dat = _res->fileData(filename, &fileSize);

	memset(_anims, 0, sizeof(_anims));
	uint8 nextAnim = 0;

	assert(fileSize > 0x6D);

	memcpy(_drawLayerTable, (_dat + 0x0D), 8);
	_engine->_northExitHeight = READ_LE_UINT16(_dat + 0x15);
	if (_engine->_northExitHeight & 1)
		_engine->_northExitHeight += 1;
	// XXX
	memcpy(_screen->_currentPalette + 744 - 60, _dat + 0x17, 60);
	uint8 *data = _dat + 0x6B;

	uint16 length = READ_LE_UINT16(data);
	data += 2;

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
				_spriteDefStart = data;
				while (READ_LE_UINT16(data) != 0xFF85) {
					data += 2;
				}
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

	assert(fileSize - (data - _dat) == 0xC);

	exits.northXPos = READ_LE_UINT16(data) & 0xFFFC; data += 2;
	exits.northYPos = *data++ & 0xFFFE;
	exits.eastXPos = READ_LE_UINT16(data) & 0xFFFC; data += 2;
	exits.eastYPos = *data++ & 0xFFFE;
	exits.southXPos = READ_LE_UINT16(data) & 0xFFFC; data += 2;
	exits.southYPos = *data++ & 0xFFFE;
	exits.westXPos = READ_LE_UINT16(data) & 0xFFFC; data += 2;
	exits.westYPos = *data++ & 0xFFFE;
}

void Sprites::freeSceneShapes() {
	debug(9, "Sprites::freeSceneShapes()");
	for (int i = 0; i < ARRAYSIZE(_sceneShapes); i++ ) {
		free(_sceneShapes[i]);
		_sceneShapes[i] = 0;
	}
}

void Sprites::loadSceneShapes() {
	debug(9, "Sprites::loadSceneShapes()");
	uint8 *data = _spriteDefStart;
	int spriteNum, x, y, width, height;

	freeSceneShapes();
	memset( _sceneShapes, 0, sizeof(_sceneShapes));

	if (_spriteDefStart == 0)
		return;

	int bakPage = _screen->_curPage;
	_screen->_curPage = 3;

	while (READ_LE_UINT16(data) != 0xFF85) {
		spriteNum = READ_LE_UINT16(data);
		assert(spriteNum < ARRAYSIZE(_sceneShapes));
		data += 2;
		x = READ_LE_UINT16(data) * 8;
		data += 2;
		y = READ_LE_UINT16(data);
		data += 2;
		width = READ_LE_UINT16(data) * 8;
		data += 2;
		height = READ_LE_UINT16(data);
		data += 2;
		_sceneShapes[spriteNum] = _screen->encodeShape(x, y, width, height, 2);
		debug(9, "Sprite %i is at (%i, %i), width %i, height %i", spriteNum, x, y, width, height);
	}
	_screen->_curPage = bakPage;
}

void Sprites::refreshSceneAnimObject(uint8 animNum, uint8 shapeNum, uint16 x, uint16 y, bool flipX, bool unkFlag) {
	debug(9, "Sprites::refreshSceneAnimObject(%i, %i, %i, %i, %i, %i", animNum, shapeNum, x, y, flipX, unkFlag);
	AnimObject &anim = _engine->animator()->sprites()[animNum];
	anim.refreshFlag = 1;
	anim.bkgdChangeFlag = 1;

	if (unkFlag)
		anim.flags |= 0x0200;
	else
		anim.flags &= 0xFD00;

	if (flipX)
		anim.flags |= 1;
	else
		anim.flags &= 0xFE;

	anim.sceneAnimPtr = _sceneShapes[shapeNum];
	anim.animFrameNumber = -1;
	anim.x1 = x;
	anim.y1 = y;
}

int Sprites::getDrawLayer(int y) {
	debug(9, "getDrawLayer(%d)", y);
	uint8 returnValue = 0;
	for (int i = 0; i < ARRAYSIZE(_drawLayerTable); ++i) {
		uint8 temp = _drawLayerTable[i];
		if (temp) {
			if (temp <= y) {
				returnValue = i;
			}
		}
	}
	if (returnValue <= 0) {
		returnValue = 1;
	} else if (returnValue >= 7) {
		returnValue = 6;
	}
	return returnValue;
}
} // end of namespace Kyra
