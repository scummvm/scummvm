/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#include "base/engine.h"

#include "scumm/scumm.h"

#include "scumm/smush/smush_player.h"
#include "scumm/smush/chunk_type.h"
#include "scumm/smush/chunk.h"

#include "scumm/insane/insane.h"

namespace Scumm {

void Insane::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Chunk &b, int32 size, int32 flags) {
	if (_keyboardDisable)
		return;

	switch (_currSceneId) {
	case 1:
		iactScene1(renderBitmap, codecparam, setupsan12, setupsan13, b, size, flags);
		break;
	case 3:
	case 13:
		iactScene3(renderBitmap, codecparam, setupsan12, setupsan13, b, size, flags);
		break;
	case 4:
	case 5:
		iactScene4(renderBitmap, codecparam, setupsan12, setupsan13, b, size, flags);
		break;
	case 6:
		iactScene6(renderBitmap, codecparam, setupsan12, setupsan13, b, size, flags);
		break;
	case 17:
		iactScene17(renderBitmap, codecparam, setupsan12, setupsan13, b, size, flags);
		break;
	case 21:
		iactScene21(renderBitmap, codecparam, setupsan12, setupsan13, b, size, flags);
		break;
	}
}

void Insane::iactScene1(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Chunk &b, int32 size, int32 flags) {
	_player->checkBlock(b, TYPE_IACT, 8);

	int16 par1, par2, par3, par4, par5, par6, par7, par9, par11, par13, tmp;

	par1 = b.getWord(); // cx
	par2 = b.getWord(); // dx
	par3 = b.getWord(); // si
	par4 = b.getWord(); // bx

	switch (par1) {
	case 2: // PATCH
		if (par3 != 1)
			break;

		par5 = b.getWord(); // si
		if (_actor[0].field_8 == 112) {
			setBit(par5);
			break;
		}

		if (_approachAnim == -1) {
			chooseEnemy(); //PATCH
			_approachAnim = _enemy[_currEnemy].apprAnim;
		}

		if (_approachAnim == par4)
			clearBit(par5);
		else
			setBit(par5);
		break;
	case 3:
		if (par3 == 1) {
			setBit(b.getWord());
			_approachAnim = -1;
		}
		break;
	case 4:
		if (par3 == 1 && (_approachAnim < 0 || _approachAnim > 4))
			setBit(b.getWord());
		break;
	case 5:
		if (par2 != 13)
			break;

		tmp = b.getWord();   // +8
		tmp = b.getWord();   // +10
		par7 = b.getWord();  // +12 dx
		tmp = b.getWord();   // +14
		par9 = b.getWord();  // +16 bx
		tmp = b.getWord();   // +18
		par11 = b.getWord(); // +20 cx
		tmp = b.getWord();   // +22
		par13 = b.getWord(); // +24 ax
		
		if (par13 > _actor[0].x || par11 < _actor[0].x) {
			_tiresRustle = true;
			_actor[0].x1 = -_actor[0].x1;
			_actor[0].damage++; // PATCH
		}

		if (par9 < _actor[0].x || par7 > _actor[0].x) {
			_tiresRustle = true;
			_actor[0].damage += 4; // PATCH
		}
		break;
	case 6:
		switch (par2) {
		case 38:
			smlayer_drawSomething(renderBitmap, codecparam, 50-19, 20-13, 3, 
								  _smush_iconsNut, 7, 0, 0);
			_roadBranch = true;
			_iactSceneId = par4;
			break;
		case 25:				
			_roadBumps = true;
			_actor[0].y1 = -_actor[0].y1;
			break;
		case 11:
			if (_approachAnim >= 1 && _approachAnim <= 4 && !_needSceneSwitch)
				queueSceneSwitch(13, _smush_minefiteFlu, "minefite.san", 64, 0,
								 _continueFrame1, 1300);
			break;
		case 9:
			par5 = b.getWord(); // si
			par6 = b.getWord(); // bx
			smlayer_setFluPalette(_smush_roadrsh3Rip, 0);
			if (par5 == par6 - 1)
				smlayer_setFluPalette(_smush_roadrashRip, 0);
		}
		break;
	case 7:
		switch (par4) {
		case 1:
			_actor[0].x -= (b.getWord() - 160) / 10;
			break;
		case 2:
			par5 = b.getWord();

			if (par5 - 8 > _actor[0].x || par5 + 8 < _actor[0].x) {
				if (smlayer_isSoundRunning(86))
					smlayer_stopSound(86);
			} else {
				if (!smlayer_isSoundRunning(86))
					smlayer_startSfx(86);
			}
			break;
		}
		break;
	}

	if (_approachAnim < 0 || _approachAnim > 4)
		if (readArray(8)) {
			smlayer_drawSomething(renderBitmap, codecparam, 270-19, 20-18, 3, 
								  _smush_iconsNut, 20, 0, 0);
			_benHasGoggles = true;
		}
}

void Insane::chooseEnemy(void) {
	if (readArray(58) != 0)
		_enemy[EN_TORQUE].isEmpty = 1;

	if (_enemy[EN_TORQUE].occurences == 0) {
		_currEnemy = EN_TORQUE;
		_metEnemiesListTail++;
		_metEnemiesList[_metEnemiesListTail] = EN_TORQUE;
		return;
	}

	removeEmptyEnemies();

	int32 count, i, j, en, en2;
	bool notfound;

	en = 0;
	for (i = 0; i < 9; i++)
		if (_enemy[i].isEmpty == 0)
			++en;

	en -= 4;
	assert(en >= 0);

	count = 0;
	while (1) {
		count++;
		if (count < 14) {
			en2 = _vm->_rnd.getRandomNumber(10);
			if (en2 == 9)
				en2 = 6;
			else if (en2 > 9)
				en2 = 7;

			notfound = true;

			if (_enemy[en2].isEmpty != 0)
				continue;

			if (0 < _metEnemiesListTail) {
				i = 0;
				do {
					if (en2 == _metEnemiesList[i + 1])
						notfound = false;
					i++;
				} while (i < _metEnemiesListTail && notfound);
			}
			if (!notfound) {
				continue;
			}			
		} else {
			j = 0;
			do {
				notfound = true;
				en2 = j;
				if (0 < _metEnemiesListTail) {
					i = 0;
					do {
						if (en2 == _metEnemiesList[i + 1])
							notfound = false;
						i++;
					} while (i < _metEnemiesListTail && notfound);
				}
				j++;
			} while (j < 9 && !notfound);
			if (!notfound) {
				_metEnemiesListTail = 0;
				count = 0;				
				continue;
			}
		}
	
		++_metEnemiesListTail;
		assert(_metEnemiesListTail < ARRAYSIZE(_metEnemiesList));
		_metEnemiesList[_metEnemiesListTail] = en2;

		if (_metEnemiesListTail >= en) {
			removeEnemyFromMetList(0);
		}

		if (notfound)
			break;
	}

	_currEnemy = en2;
}

void Insane::removeEmptyEnemies(void) {
	if (_metEnemiesListTail > 0) {
		for (int i = 0; i < _metEnemiesListTail; i++)
			if (_enemy[i].isEmpty == 1)
				removeEnemyFromMetList(i);
	}
}

void Insane::removeEnemyFromMetList(int32 enemy1) {
	if (enemy1 >= _metEnemiesListTail)
		return;

	int en = enemy1;
	do {
		++en;
		assert(en + 1 < ARRAYSIZE(_metEnemiesList));
		_metEnemiesList[en] = _metEnemiesList[en + 1];
	} while (en < _metEnemiesListTail);
	_metEnemiesListTail--;
}

void Insane::iactScene3(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Chunk &b, int32 size, int32 flags) {
	_player->checkBlock(b, TYPE_IACT, 8);

	int command, par1, par2, par3, tmp;
	command = b.getWord();
	par1 = b.getWord();
	if (command == 6) {
		if (par1 == 9) {
			tmp = b.getWord();  // ptr + 4
			tmp = b.getWord();  // ptr + 6
			par2 = b.getWord(); // ptr + 8
			par3 = b.getWord(); // ptr + 10

			if (!par2)
				smlayer_setFluPalette(_smush_roadrsh3Rip, 0);
			else {
				if (par2 == par3 - 1)
					smlayer_setFluPalette(_smush_roadrashRip, 0);
			}
		} else if (par1 == 25) {
			_roadBumps = true;
			_actor[0].y1 = -_actor[0].y1;
			_actor[1].y1 = -_actor[1].y1;
		}
	}
}

void Insane::iactScene4(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Chunk &b, int32 size, int32 flags) {
	_player->checkBlock(b, TYPE_IACT, 8);

	if ((_vm->_features & GF_DEMO) && (_vm->_platform == Common::kPlatformPC))
		return;

	int16 par1, par2, par3, par4, par5;

	par1 = b.getWord(); // edx
	par2 = b.getWord(); // bx
	par3 = b.getWord();
	par4 = b.getWord(); // cx

	switch (par1) {
	case 2:
	case 4:
		par5 = b.getWord(); // si
		switch (par3) {
		case 1:
			if (par4 == 1) {
				if (readArray(6))
					setBit(par5);
				else
					clearBit(par5);
			} else {
				if (readArray(6))
					clearBit(par5);
				else
					setBit(par5);
			}
			break;
		case 2:
			if (readArray(5))
				clearBit(par5);
			else
				setBit(par5);
			break;
		}
		break;
	case 6:
		switch (par2) {
		case 38:
			
			smlayer_drawSomething(renderBitmap, codecparam, 270-19, 20-13, 3, 
								  _smush_icons2Nut, 10, 0, 0);
			_roadBranch = true;
			_iactSceneId = par4;
			break;
		case 7:
			if (readArray(4) != 0)
				return;

			smlayer_drawSomething(renderBitmap, codecparam, 160-13, 20-10, 3, // QW
								  _smush_icons2Nut, 8, 0, 0);
			_roadStop = true;
			break;
		case 8:
			if (readArray(4) == 0 || readArray(6) == 0)
				return;

			writeArray(1, _posBrokenTruck);
			writeArray(3, _val57d);
			smush_setToFinish();

			break;
		case 25:				
			if (readArray(5) == 0)
				return;
			
			_carIsBroken = true;
			smlayer_drawSomething(renderBitmap, codecparam, 160-13, 20-10, 3, // QW
								  _smush_icons2Nut, 8, 0, 0);
			break;
		case 11:
			smlayer_drawSomething(renderBitmap, codecparam, 50-19, 20-13, 3, 
								  _smush_icons2Nut, 9, 0, 0);
			_roadBranch = true;
			_iactSceneId = par4;
			break;
		}
		break;
	}
}

void Insane::iactScene6(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Chunk &b, int32 size, int32 flags) {
	_player->checkBlock(b, TYPE_IACT, 8);

	int16 par1, par2, par3, par4, par5;

	par1 = b.getWord();
	par2 = b.getWord(); // bx
	par3 = b.getWord();
	par4 = b.getWord();

	switch (par1) {
	case 7:
		par5 = b.getWord();
		if (par4 != 3)
			break;

		if (par5 >= _actor[0].x)
			break;

		_actor[0].x = par5;
		break;
	case 2:
	case 4:
		par5 = b.getWord();
		switch (par3) {
		case 1:
			if (par4 == 1) {
				if (readArray(6))
					setBit(par5);
				else
					clearBit(par5);
			} else {
				if (readArray(6))
					clearBit(par5);
				else
					setBit(par5);
			}
			break;
		case 2:
			if (readArray(5))
				clearBit(par5);
			else
				setBit(par5);
			break;
		}
		break;
	case 6:
		switch (par2) {
		case 38:
			smlayer_drawSomething(renderBitmap, codecparam, 270-19, 20-13, 3, 
								  _smush_icons2Nut, 10, 0, 0);
			_roadBranch = true;
			_iactSceneId = par4;
			break;
		case 7:
			if (readArray(4) != 0)
				return;

			_roadStop = true;
			smlayer_drawSomething(renderBitmap, codecparam, 160-13, 20-10, 3, //QW
								  _smush_icons2Nut, 8, 0, 0);
			break;
		case 8:
			if (readArray(4) == 0 || readArray(6) == 0)
				return;

			writeArray(1, _posBrokenTruck);
			writeArray(3, _posVista);
			smush_setToFinish();

			break;
		case 25:				
			if (readArray(5) == 0)
				return;
			
			_carIsBroken = true;
			smlayer_drawSomething(renderBitmap, codecparam, 160-13, 20-10, 3, //QW
								  _smush_icons2Nut, 8, 0, 0);
			break;
		case 11:
			smlayer_drawSomething(renderBitmap, codecparam, 50-19, 20-13, 3, 
								  _smush_icons2Nut, 9, 0, 0);
			_roadBranch = true;
			_iactSceneId = par4;
			break;
		}
		break;
	}
}

void Insane::iactScene17(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Chunk &b, int32 size, int32 flags) {
	_player->checkBlock(b, TYPE_IACT, 8);
	int16 par1, par2, par3, par4;

	par1 = b.getWord(); // dx
	par2 = b.getWord(); // cx
	par3 = b.getWord(); // di
	par4 = b.getWord();

	switch (par1) {
	case 2:
	case 3:
	case 4:
		if (par3 == 1) {
			setBit(b.getWord());
			_approachAnim = -1;
		}
		break;
	case 6:
		switch (par2) {
		case 38:
			smlayer_drawSomething(renderBitmap, codecparam, 28, 48, 1, 
								  _smush_iconsNut, 6, 0, 0);
			_roadBranch = true;
			_iactSceneId = par4;
			if (_counter1 <= 4) {
				if (_counter1 == 4)
					smlayer_startSfx(94);

				smlayer_showStatusMsg(-1, renderBitmap, codecparam, 24, 167, 1,
									  2, 0, "%s", handleTrsTag(5000));
			}
			_objectDetected = true;
			break;
		case 10:
			smlayer_drawSomething(renderBitmap, codecparam, 28, 48, 1, 
								  _smush_iconsNut, 6, 0, 0);
			if (_counter1 <= 4) {
				if (_counter1 == 4)
					smlayer_startSfx(94);

				smlayer_showStatusMsg(-1, renderBitmap, codecparam, 24, 167, 1,
									  2, 0, "%s", handleTrsTag(5001));
			}
			_objectDetected = true;
			_mineCaveIsNear = true;
			break;
		}
		break;
	}
}

void Insane::iactScene21(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Chunk &b, int32 size, int32 flags) {
	// void implementation
}

}

