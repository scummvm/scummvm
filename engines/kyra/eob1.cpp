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

#ifdef ENABLE_EOB

#include "kyra/eob1.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

namespace Kyra {

EobEngine::EobEngine(OSystem *system, const GameFlags &flags) : EobCoreEngine(system, flags) {
	_numSpells = 53;
	_menuChoiceInit = 4;
}

EobEngine::~EobEngine() {
	delete[] _itemsOverlay;
}

Common::Error EobEngine::init() {
	Common::Error err = EobCoreEngine::init();
	if (err.getCode() != Common::kNoError)
		return err;

	initStaticResource();

	_itemsOverlay = _res->fileData("ITEMRMP.VGA", 0);

	_bkgColor_1 = 132;
	_color1_1 = 135;
	_color2_1 = 130;
	_color4 = 133;
	_color5 = 133;
	_color6 = 180;
	_color7 = 177;
	_color8 = 184;

	_color14 = _color8;
	_color13 = _color7;
	_color12 = _color6;

	_screen->modifyScreenDim(7, 0x01, 0xB3, 0x22, 0x12);
	_screen->modifyScreenDim(9, 0x01, 0x7D, 0x26, 0x3F);
	_screen->modifyScreenDim(12, 0x01, 0x04, 0x14, 0xA0);

	_scriptTimersCount = 1;

	return Common::kNoError;
}

void EobEngine::startupNew() {
	_currentLevel = 1;
	_currentSub = 0;
	loadLevel(1, 0);
	_currentBlock = 490;
	_currentDirection = 0;
	setHandItem(0);
	EobCoreEngine::startupNew();
}

void EobEngine::startupLoad() {
	_sound->loadSoundFile("ADLIB");
}

void EobEngine::npcSequence(int npcIndex) {


}

void EobEngine::updateUsedCharacterHandItem(int charIndex, int slot) {
	EobItem *itm = &_items[_characters[charIndex].inventory[slot]];
	if (itm->type == 48) {
		int charges = itm->flags & 0x3f;
		if (--charges)
			--itm->flags;
		else
			deleteInventoryItem(charIndex, slot);
	} else if (itm->type == 34 || itm->type == 35) {
		deleteInventoryItem(charIndex, slot);
	}
}

void EobEngine::replaceMonster(int unit, uint16 block, int pos, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem) {
	if (_levelBlockProperties[block].flags & 7)
		return;

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].hitPointsCur <= 0) {
			initMonster(i, unit, block, pos, dir, type, shpIndex, mode, h2, randItem, fixedItem);;
			break;
		}
	}
}

void EobEngine::loadDoorShapes(int doorType1, int shapeId1, int doorType2, int shapeId2) {
	_screen->loadEobBitmap("DOOR", 5, 3);
	_screen->_curPage = 2;

	if (doorType1 != 0xff) {
		for (int i = 0; i < 3; i++) {
			const uint8 *enc = &_doorShapeEncodeDefs[(doorType1 * 3 + i) << 2];
			_doorShapes[shapeId1 + i] = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3]);
			enc = &_doorSwitchShapeEncodeDefs[(doorType1 * 3 + i) << 2];
			_doorSwitches[shapeId1 + i].shp = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3]);
			_doorSwitches[shapeId1 + i].x = _doorSwitchCoords[doorType1 << 1];
			_doorSwitches[shapeId1 + i].y = _doorSwitchCoords[(doorType1 << 1) + 1];
		}
	}

	if (doorType2 != 0xff) {
		for (int i = 0; i < 3; i++) {
			const uint8 *enc = &_doorShapeEncodeDefs[(doorType2 * 3 + i) << 2];
			_doorShapes[shapeId2 + i] = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3]);
			enc = &_doorSwitchShapeEncodeDefs[(doorType2 * 3 + i) << 2];
			_doorSwitches[shapeId2 + i].shp = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3]);
			_doorSwitches[shapeId2 + i].x = _doorSwitchCoords[doorType2 << 1];
			_doorSwitches[shapeId2 + i].y = _doorSwitchCoords[(doorType2 << 1) + 1];
		}
	}

	_screen->_curPage = 0;
}

void EobEngine::drawDoorIntern(int type, int index, int x, int y, int w, int wall, int mDim, int16 y1, int16 y2) {
	int shapeIndex = type + 2 - mDim;
	uint8 *shp = _doorShapes[shapeIndex];

	int d1 = 0;
	int d2 = 0;
	int v = 0;
	const ScreenDim *td = _screen->getScreenDim(5);

	switch (_currentLevel) {
		case 4:
		case 5:
		case 6:
			y = _dscDoorY2[mDim] - shp[1];
			d1 = _dscDoorCoordsExt[index << 1] >> 3;
			d2 = _dscDoorCoordsExt[(index << 1) + 1] >> 3;
			if (_shpDmX1 > d1)
				d1 = _shpDmX1;
			if (_shpDmX2 < d2)
				d2 = _shpDmX2;
			_screen->modifyScreenDim(5, d1, td->sy, d2 - d1, td->h);
			v = ((wall < 30) ? (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult3[mDim] : _dscDoorScaleMult4[mDim]) * -1;
			v -= (shp[2] << 3);
			drawBlockObject(0, 2, shp, x + v, y, 5);
			v += (shp[2] << 3);
			drawBlockObject(1, 2, shp, x - v, y, 5);
			if (_wllShapeMap[wall] == -1)
				drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w - v, _doorSwitches[shapeIndex].y, 5);
			break;

		case 7:
		case 8:
		case 9:
			y = _dscDoorY3[mDim] - _doorShapes[shapeIndex + 3][1];
			d1 = x - (_doorShapes[shapeIndex + 3][2] << 2);
			x -= (shp[2] << 2);
			drawBlockObject(0, 2, _doorShapes[shapeIndex + 3], d1, y, 5);
			scaleLevelShapesDim(index, y1, y2, 5);
			y = _dscDoorY3[mDim] - ((wall < 30) ? (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult1[mDim] : _dscDoorScaleMult2[mDim]);
			drawBlockObject(0, 2, shp, x, y, 5);
			if (_wllShapeMap[wall] == -1)
				drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
			break;

		case 10:
		case 11:
			v = ((wall < 30) ? (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult5[mDim] : _dscDoorScaleMult6[mDim]) * -1;
			x -= (shp[2] << 2);
			y = _dscDoorY3[mDim] + v;
			drawBlockObject(0, 2, shp, x, y + v, 5);
			v >>= 4;
			y = _dscDoorY5[mDim];
			drawBlockObject(0, 2, _doorShapes[shapeIndex + 3], x, y - v, 5);
			if (_wllShapeMap[wall] == -1)
				drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
			break;

		default:
			y = (_currentLevel == 12 ? _dscDoorY6[mDim] : _dscDoorY1[mDim]) - shp[1];
			x -= (shp[2] << 2);
			y -= (wall >= 30 ? _dscDoorScaleMult2[mDim] : (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult1[mDim]);
			drawBlockObject(0, 2, shp, x, y, 5);

			if (_wllShapeMap[wall] == -1)
				drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
			break;
	}
}

uint32 EobEngine::convertSpellFlagToEob2Format(uint32 flag, int ignoreInvisibility) {
	uint32 res = 0;
	if (flag & 0x01)
		res |= 0x20;
	if (flag & 0x02)
		res |= 0x400;
	if (flag & 0x04)
		res |= 0x80;
	if (flag & 0x08)
		res |= 0x40;
	if (ignoreInvisibility)
		res |= 0x100;
	return res;
}

uint32 EobEngine::convertCharacterEffectFlagToEob2Format(uint32 flag) {
	uint32 res = 0;
	if (flag & 0x02)
		res |= 0x08;
	if (flag & 0x04)
		res |= 0x40;
	if (flag & 0x80)
		res |= 0x2000;
	if (flag & 0x100)
		res |= 0x4000;
	return res;
}

}	// End of namespace Kyra

#endif // ENABLE_EOB
