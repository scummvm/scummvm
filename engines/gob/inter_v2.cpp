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
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "sound/mixer.h"
#include "sound/mods/infogrames.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/goblin.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/parse.h"
#include "gob/scenery.h"
#include "gob/video.h"
#include "gob/save/saveload.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_v2
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_v2::Inter_v2(GobEngine *vm) : Inter_v1(vm) {
}

void Inter_v2::setupOpcodesDraw() {
	Inter_v1::setupOpcodesDraw();

	OPCODEDRAW(0x01, o2_playMult);
	OPCODEDRAW(0x02, o2_freeMultKeys);

	OPCODEDRAW(0x0A, o2_setRenderFlags);

	OPCODEDRAW(0x13, o2_multSub);

	OPCODEDRAW(0x14, o2_initMult);

	OPCODEDRAW(0x17, o2_loadMultObject);

	OPCODEDRAW(0x1C, o2_renderStatic);
	OPCODEDRAW(0x1D, o2_loadCurLayer);

	OPCODEDRAW(0x20, o2_playCDTrack);
	OPCODEDRAW(0x21, o2_waitCDTrackEnd);
	OPCODEDRAW(0x22, o2_stopCD);
	OPCODEDRAW(0x23, o2_readLIC);

	OPCODEDRAW(0x24, o2_freeLIC);
	OPCODEDRAW(0x25, o2_getCDTrackPos);

	OPCODEDRAW(0x30, o2_loadFontToSprite);

	OPCODEDRAW(0x40, o2_totSub);
	OPCODEDRAW(0x41, o2_switchTotSub);
	OPCODEDRAW(0x42, o2_pushVars);
	OPCODEDRAW(0x43, o2_popVars);

	OPCODEDRAW(0x50, o2_loadMapObjects);
	OPCODEDRAW(0x51, o2_freeGoblins);
	OPCODEDRAW(0x52, o2_moveGoblin);
	OPCODEDRAW(0x53, o2_writeGoblinPos);

	OPCODEDRAW(0x54, o2_stopGoblin);
	OPCODEDRAW(0x55, o2_setGoblinState);
	OPCODEDRAW(0x56, o2_placeGoblin);

	OPCODEDRAW(0x80, o2_initScreen);
	OPCODEDRAW(0x81, o2_scroll);
	OPCODEDRAW(0x82, o2_setScrollOffset);
	OPCODEDRAW(0x83, o2_playImd);

	OPCODEDRAW(0x84, o2_getImdInfo);
	OPCODEDRAW(0x85, o2_openItk);
	OPCODEDRAW(0x86, o2_closeItk);
	OPCODEDRAW(0x87, o2_setImdFrontSurf);

	OPCODEDRAW(0x88, o2_resetImdFrontSurf);
}

void Inter_v2::setupOpcodesFunc() {
	Inter_v1::setupOpcodesFunc();

	OPCODEFUNC(0x09, o2_assign);

	OPCODEFUNC(0x11, o2_printText);

	OPCODEFUNC(0x17, o2_animPalInit);

	OPCODEFUNC(0x18, o2_addCollision);
	OPCODEFUNC(0x19, o2_freeCollision);

	OPCODEFUNC(0x25, o2_goblinFunc);

	OPCODEFUNC(0x39, o2_stopSound);
	OPCODEFUNC(0x3A, o2_loadSound);

	OPCODEFUNC(0x3E, o2_getFreeMem);
	OPCODEFUNC(0x3F, o2_checkData);

	OPCODEFUNC(0x4D, o2_readData);
	OPCODEFUNC(0x4E, o2_writeData);
}

void Inter_v2::setupOpcodesGob() {
	OPCODEGOB(  0, o2_loadInfogramesIns);
	OPCODEGOB(  1, o2_startInfogrames);
	OPCODEGOB(  2, o2_stopInfogrames);

	OPCODEGOB( 10, o2_playInfogrames);

	OPCODEGOB(100, o2_handleGoblins);

	OPCODEGOB(500, o2_playProtracker);
	OPCODEGOB(501, o2_stopProtracker);
}

void Inter_v2::checkSwitchTable(byte **ppExec) {
	byte cmd;
	int16 len;
	int32 value;
	bool found;

	found = false;
	*ppExec = 0;

	cmd = *_vm->_global->_inter_execPtr;

	value = _vm->_parse->parseVarIndex();

	switch (cmd) {
	case TYPE_VAR_INT8:
	case TYPE_ARRAY_INT8:
		value = (int8) READ_VARO_UINT8(value);
		break;

	case TYPE_VAR_INT32:
	case TYPE_ARRAY_INT32:
		value = READ_VARO_UINT32(value);
		break;

	default:
		value = (int16) READ_VARO_UINT16(value);
		break;
	}

	if (_terminate)
		return;

	len = (int8) *_vm->_global->_inter_execPtr++;
	while (len != -5) {
		for (int i = 0; i < len; i++) {
			cmd = *_vm->_global->_inter_execPtr;

			switch (cmd) {
			case TYPE_IMM_INT32:
				_vm->_global->_inter_execPtr++;
				if (!found &&
						(value ==
						(int32) (READ_LE_UINT32(_vm->_global->_inter_execPtr))))
					found = true;
				_vm->_global->_inter_execPtr += 5;
				break;

			case TYPE_IMM_INT16:
				_vm->_global->_inter_execPtr++;
				if (!found &&
						(value ==
						(int16) (READ_LE_UINT16(_vm->_global->_inter_execPtr))))
					found = true;
				_vm->_global->_inter_execPtr += 3;
				break;

			case TYPE_IMM_INT8:
				_vm->_global->_inter_execPtr++;
				if (!found && (value == (int8) *_vm->_global->_inter_execPtr))
					found = true;
				_vm->_global->_inter_execPtr += 2;
				break;

			default:
				if (!found) {
					evalExpr(0);
					if (value == _vm->_parse->getResultInt())
						found = true;
				} else
					_vm->_parse->skipExpr(99);
				break;
			}
		}

		if (found && !*ppExec)
			*ppExec = _vm->_global->_inter_execPtr;

		_vm->_global->_inter_execPtr +=
		  READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
		len = (int8) *_vm->_global->_inter_execPtr++;
	}

	if ((*_vm->_global->_inter_execPtr >> 4) != 4)
		return;

	_vm->_global->_inter_execPtr++;
	if (!*ppExec)
		*ppExec = _vm->_global->_inter_execPtr;

	_vm->_global->_inter_execPtr +=
	  READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
}

void Inter_v2::o2_playMult() {
	int16 checkEscape;

	checkEscape = load16();

	_vm->_mult->setMultData(checkEscape >> 1);
	_vm->_mult->playMult(VAR(57), -1, checkEscape & 0x1, 0);
}

void Inter_v2::o2_freeMultKeys() {
	uint16 index = load16();

	if (!_vm->_mult->hasMultData(index))
		return;

	_vm->_mult->setMultData(index);
	_vm->_mult->freeMultKeys();
	_vm->_mult->zeroMultData(index);
}

void Inter_v2::o2_setRenderFlags() {
	int16 expr;

	expr = _vm->_parse->parseValExpr();

	if (expr & 0x8000) {
		_vm->_draw->_renderFlags |= expr & 0x3FFF;
	} else {
		if (expr & 0x4000)
			_vm->_draw->_renderFlags &= expr & 0x3FFF;
		else
			_vm->_draw->_renderFlags = expr;
	}
}

void Inter_v2::o2_multSub() {
	_vm->_mult->multSub(_vm->_parse->parseValExpr());
}

void Inter_v2::o2_initMult() {
	int16 oldAnimHeight;
	int16 oldAnimWidth;
	int16 oldObjCount;
	int16 posXVar;
	int16 posYVar;
	int16 animDataVar;

	oldAnimWidth = _vm->_mult->_animWidth;
	oldAnimHeight = _vm->_mult->_animHeight;
	oldObjCount = _vm->_mult->_objCount;

	_vm->_mult->_animLeft = load16();
	_vm->_mult->_animTop = load16();
	_vm->_mult->_animWidth = load16();
	_vm->_mult->_animHeight = load16();
	_vm->_mult->_objCount = load16();
	posXVar = _vm->_parse->parseVarIndex();
	posYVar = _vm->_parse->parseVarIndex();
	animDataVar = _vm->_parse->parseVarIndex();

	if (_vm->_mult->_objects && (oldObjCount != _vm->_mult->_objCount)) {
		warning("Initializing new objects without having "
				"cleaned up the old ones at first");

		_vm->_mult->clearObjectVideos();

		for (int i = 0; i < _vm->_mult->_objCount; i++) {
			delete _vm->_mult->_objects[i].pPosX;
			delete _vm->_mult->_objects[i].pPosY;
		}

		delete[] _vm->_mult->_objects;
		delete[] _vm->_mult->_renderObjs;
		delete[] _vm->_mult->_orderArray;

		_vm->_mult->_objects = 0;
		_vm->_mult->_renderObjs = 0;
		_vm->_mult->_orderArray = 0;
	}

	if (_vm->_mult->_objects == 0) {
		_vm->_mult->_renderObjs = new Mult::Mult_Object*[_vm->_mult->_objCount];
		memset(_vm->_mult->_renderObjs, 0,
				_vm->_mult->_objCount * sizeof(Mult::Mult_Object*));

		if (_terminate)
			return;

		_vm->_mult->_orderArray = new int8[_vm->_mult->_objCount];
		memset(_vm->_mult->_orderArray, 0, _vm->_mult->_objCount * sizeof(int8));
		_vm->_mult->_objects = new Mult::Mult_Object[_vm->_mult->_objCount];
		memset(_vm->_mult->_objects, 0,
				_vm->_mult->_objCount * sizeof(Mult::Mult_Object));

		for (int i = 0; i < _vm->_mult->_objCount; i++) {
			uint32 offPosX = i * 4 + (posXVar / 4) * 4;
			uint32 offPosY = i * 4 + (posYVar / 4) * 4;
			uint32 offAnim = animDataVar + i * 4 * _vm->_global->_inter_animDataSize;

			_vm->_mult->_objects[i].pPosX = new VariableReference(*_vm->_inter->_variables, offPosX);
			_vm->_mult->_objects[i].pPosY = new VariableReference(*_vm->_inter->_variables, offPosY);

			_vm->_mult->_objects[i].pAnimData =
				(Mult::Mult_AnimData *) _variables->getAddressOff8(offAnim);

			_vm->_mult->_objects[i].pAnimData->isStatic = 1;
			_vm->_mult->_objects[i].tick = 0;
			_vm->_mult->_objects[i].lastLeft = -1;
			_vm->_mult->_objects[i].lastRight = -1;
			_vm->_mult->_objects[i].lastTop = -1;
			_vm->_mult->_objects[i].lastBottom = -1;
			_vm->_mult->_objects[i].goblinX = 1;
			_vm->_mult->_objects[i].goblinY = 1;
		}
	}

	if (_vm->_mult->_animSurf &&
	    ((oldAnimWidth != _vm->_mult->_animWidth) ||
			 (oldAnimHeight != _vm->_mult->_animHeight))) {
		_vm->_draw->freeSprite(22);
		_vm->_mult->_animSurf.reset();
	}

	_vm->_draw->adjustCoords(0,
			&_vm->_mult->_animWidth, &_vm->_mult->_animHeight);
	if (!_vm->_mult->_animSurf) {
		_vm->_draw->initSpriteSurf(22, _vm->_mult->_animWidth,
				_vm->_mult->_animHeight, 0);
		_vm->_mult->_animSurf = _vm->_draw->_spritesArray[22];
		if (_terminate)
			return;
	}

	_vm->_draw->adjustCoords(1,
			&_vm->_mult->_animWidth, &_vm->_mult->_animHeight);
	_vm->_draw->_sourceSurface = 21;
	_vm->_draw->_destSurface = 22;
	_vm->_draw->_spriteLeft = _vm->_mult->_animLeft;
	_vm->_draw->_spriteTop = _vm->_mult->_animTop;
	_vm->_draw->_spriteRight = _vm->_mult->_animWidth;
	_vm->_draw->_spriteBottom = _vm->_mult->_animHeight;
	_vm->_draw->_destSpriteX = 0;
	_vm->_draw->_destSpriteY = 0;
	_vm->_draw->spriteOperation(0);

	debugC(4, kDebugGraphics, "o2_initMult: x = %d, y = %d, w = %d, h = %d",
		  _vm->_mult->_animLeft, _vm->_mult->_animTop,
			_vm->_mult->_animWidth, _vm->_mult->_animHeight);
	debugC(4, kDebugGraphics, "    _vm->_mult->_objCount = %d, "
			"animation data size = %d", _vm->_mult->_objCount,
			_vm->_global->_inter_animDataSize);
}

void Inter_v2::o2_loadMultObject() {
	int16 val;
	int16 objIndex;
	int16 animation;
	int16 layer;
	byte *multData;

	objIndex = _vm->_parse->parseValExpr();
	val = _vm->_parse->parseValExpr();
	*_vm->_mult->_objects[objIndex].pPosX = val;
	val = _vm->_parse->parseValExpr();
	*_vm->_mult->_objects[objIndex].pPosY = val;

	debugC(4, kDebugGameFlow, "Loading mult object %d", objIndex);

	multData = (byte *) _vm->_mult->_objects[objIndex].pAnimData;
	for (int i = 0; i < 11; i++) {
		if (*_vm->_global->_inter_execPtr != 99)
			multData[i] = _vm->_parse->parseValExpr();
		else
			_vm->_global->_inter_execPtr++;
	}

	Mult::Mult_Object &obj = _vm->_mult->_objects[objIndex];
	Mult::Mult_AnimData &objAnim = *(obj.pAnimData);

	if ((objAnim.animType == 100) && (objIndex < _vm->_goblin->_gobsCount)) {

		val = *(obj.pPosX) % 256;
		obj.destX = val;
		obj.gobDestX = val;
		obj.goblinX = val;

		val = *(obj.pPosY) % 256;
		obj.destY = val;
		obj.gobDestY = val;
		obj.goblinY = val;

		*(obj.pPosX) *= _vm->_map->_tilesWidth;

		layer = objAnim.layer;
		animation = obj.goblinStates[layer][0].animation;
		objAnim.framesLeft = objAnim.maxFrame;
		objAnim.nextState = -1;
		objAnim.newState = -1;
		objAnim.pathExistence = 0;
		objAnim.isBusy = 0;
		objAnim.state = layer;
		objAnim.layer = obj.goblinStates[objAnim.state][0].layer;
		objAnim.animation = animation;
		_vm->_scenery->updateAnim(layer, 0, animation, 0,
				*(obj.pPosX), *(obj.pPosY), 0);

		if (!_vm->_map->_bigTiles)
			*(obj.pPosY) = (obj.goblinY + 1) * _vm->_map->_tilesHeight
				- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
		else
			*(obj.pPosY) = ((obj.goblinY + 1) * _vm->_map->_tilesHeight) -
				(_vm->_scenery->_animBottom - _vm->_scenery->_animTop) -
				((obj.goblinY + 1) / 2);
		*(obj.pPosX) = obj.goblinX * _vm->_map->_tilesWidth;

	} else if ((objAnim.animType == 101) && (objIndex < _vm->_goblin->_gobsCount)) {

		layer = objAnim.layer;
		animation = obj.goblinStates[layer][0].animation;
		objAnim.nextState = -1;
		objAnim.newState = -1;
		objAnim.state = layer;
		objAnim.layer = obj.goblinStates[objAnim.state][0].layer;
		objAnim.animation = animation;

		if ((*(obj.pPosX) == 1000) && (*(obj.pPosY) == 1000)) {
			Scenery::AnimLayer *animLayer =
				_vm->_scenery->getAnimLayer(animation, objAnim.layer);

			*(obj.pPosX) = animLayer->posX;
			*(obj.pPosY) = animLayer->posY;
		}
		_vm->_scenery->updateAnim(layer, 0, animation, 0,
				*(obj.pPosX), *(obj.pPosY), 0);

	} else if ((objAnim.animType != 100) && (objAnim.animType != 101)) {

		if ((((int32) *(obj.pPosX)) == -1234) && (((int32) *(obj.pPosY)) == -4321)) {

			if (obj.videoSlot > 0)
				_vm->_vidPlayer->slotClose(obj.videoSlot - 1);

			obj.videoSlot = 0;
			obj.lastLeft = -1;
			obj.lastTop = -1;
			obj.lastBottom = -1;
			obj.lastRight = -1;
		}

	}
}

void Inter_v2::o2_renderStatic() {
	int16 layer;
	int16 index;

	index = _vm->_parse->parseValExpr();
	layer = _vm->_parse->parseValExpr();
	_vm->_scenery->renderStatic(index, layer);
}

void Inter_v2::o2_loadCurLayer() {
	_vm->_scenery->_curStatic = _vm->_parse->parseValExpr();
	_vm->_scenery->_curStaticLayer = _vm->_parse->parseValExpr();
}

void Inter_v2::o2_playCDTrack() {
	if (!(_vm->_draw->_renderFlags & RENDERFLAG_NOBLITINVALIDATED))
		_vm->_draw->blitInvalidated();

	evalExpr(0);
	_vm->_sound->cdPlay(_vm->_parse->getResultStr());
}

void Inter_v2::o2_waitCDTrackEnd() {
	debugC(1, kDebugSound, "CDROM: Waiting for playback to end");

	while (_vm->_sound->cdGetTrackPos() >= 0)
		_vm->_util->longDelay(1);
}

void Inter_v2::o2_stopCD() {
	_vm->_sound->cdStop();
}

void Inter_v2::o2_readLIC() {
	char path[40];

	evalExpr(0);
	strncpy0(path, _vm->_parse->getResultStr(), 35);
	strcat(path, ".LIC");

	_vm->_sound->cdLoadLIC(path);
}

void Inter_v2::o2_freeLIC() {
	_vm->_sound->cdUnloadLIC();
}

void Inter_v2::o2_getCDTrackPos() {
	int16 varPos;
	int16 varName;

	_vm->_util->longDelay(1);

	varPos = _vm->_parse->parseVarIndex();
	varName = _vm->_parse->parseVarIndex();

	WRITE_VAR_OFFSET(varPos, _vm->_sound->cdGetTrackPos(GET_VARO_STR(varName)));
	WRITE_VARO_STR(varName, _vm->_sound->cdGetCurrentTrack());
}

void Inter_v2::o2_loadFontToSprite() {
	int16 i = load16();

	_vm->_draw->_fontToSprite[i].sprite = *_vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr += 2;
	_vm->_draw->_fontToSprite[i].base = *_vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr += 2;
	_vm->_draw->_fontToSprite[i].width = *_vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr += 2;
	_vm->_draw->_fontToSprite[i].height = *_vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr += 2;
}

void Inter_v2::o2_totSub() {
	char totFile[14];
	byte length;
	int flags;
	int i;

	length = *_vm->_global->_inter_execPtr++;
	if ((length & 0x7F) > 13)
		error("Length in o2_totSub is greater than 13 (%d)", length);

	if (length & 0x80) {
		evalExpr(0);
		strcpy(totFile, _vm->_parse->getResultStr());
	} else {
		for (i = 0; i < length; i++)
			totFile[i] = (char) *_vm->_global->_inter_execPtr++;
		totFile[i] = 0;
	}

	// WORKAROUND: There is a race condition in the script when opening the notepad
	if (!scumm_stricmp(totFile, "edit"))
		_vm->_util->forceMouseUp();

	flags = *_vm->_global->_inter_execPtr++;
	_vm->_game->totSub(flags, totFile);
}

void Inter_v2::o2_switchTotSub() {
	int16 index;
	int16 skipPlay;

	index = load16();
	skipPlay = load16();

	_vm->_game->switchTotSub(index, skipPlay);
}

void Inter_v2::o2_pushVars() {
	byte count;
	int16 varOff;

	count = *_vm->_global->_inter_execPtr++;
	for (int i = 0; i < count; i++, _varStackPos++) {
		if ((*_vm->_global->_inter_execPtr == 25) ||
				(*_vm->_global->_inter_execPtr == 28)) {

			varOff = _vm->_parse->parseVarIndex();
			_vm->_global->_inter_execPtr++;

			_variables->copyTo(varOff, _varStack + _varStackPos, _vm->_global->_inter_animDataSize * 4);

			_varStackPos += _vm->_global->_inter_animDataSize * 4;
			_varStack[_varStackPos] = _vm->_global->_inter_animDataSize * 4;

		} else {
			int32 n = _vm->_parse->getResultInt();

			if (evalExpr(&varOff) != 20)
				n = 0;

			memcpy(_varStack + _varStackPos, &n, 4);
			_varStackPos += 4;
			_varStack[_varStackPos] = 4;
		}
	}
}

void Inter_v2::o2_popVars() {
	byte count;
	int16 varOff;
	int16 size;

	count = *_vm->_global->_inter_execPtr++;
	for (int i = 0; i < count; i++) {
		varOff = _vm->_parse->parseVarIndex();
		size = _varStack[--_varStackPos];

		_varStackPos -= size;
		_variables->copyFrom(varOff, _varStack + _varStackPos, size);
	}
}

void Inter_v2::o2_loadMapObjects() {
	_vm->_map->loadMapObjects(0);
}

void Inter_v2::o2_freeGoblins() {
	_vm->_goblin->freeObjects();
}

void Inter_v2::o2_moveGoblin() {
	int16 destX, destY;
	int16 index;

	destX = _vm->_parse->parseValExpr();
	destY = _vm->_parse->parseValExpr();
	index = _vm->_parse->parseValExpr();
	_vm->_goblin->move(destX, destY, index);
}

void Inter_v2::o2_writeGoblinPos() {
	int16 varX, varY;
	int16 index;

	varX = _vm->_parse->parseVarIndex();
	varY = _vm->_parse->parseVarIndex();
	index = _vm->_parse->parseValExpr();
	WRITE_VAR_OFFSET(varX, _vm->_mult->_objects[index].goblinX);
	WRITE_VAR_OFFSET(varY, _vm->_mult->_objects[index].goblinY);
}

void Inter_v2::o2_stopGoblin() {
	int16 index = _vm->_parse->parseValExpr();

	_vm->_mult->_objects[index].pAnimData->pathExistence = 4;
}

void Inter_v2::o2_setGoblinState() {
	int16 index;
	int16 state;
	int16 type;
	int16 layer;
	int16 animation;
	int16 deltaX, deltaY;
	int16 deltaWidth, deltaHeight;

	index = _vm->_parse->parseValExpr();
	state = _vm->_parse->parseValExpr();
	type = _vm->_parse->parseValExpr();

	Mult::Mult_Object &obj = _vm->_mult->_objects[index];
	Mult::Mult_AnimData &objAnim = *(obj.pAnimData);

	objAnim.stateType = type;
	if (!obj.goblinStates[state])
		return;

	Scenery::AnimLayer *animLayer;
	switch (type) {
	case 0:
		objAnim.frame = 0;
		layer = obj.goblinStates[state][0].layer;
		animation = obj.goblinStates[state][0].animation;
		objAnim.state = state;
		objAnim.layer = layer;
		objAnim.animation = animation;

		animLayer = _vm->_scenery->getAnimLayer(animation, layer);
		*(obj.pPosX) = animLayer->posX;
		*(obj.pPosY) = animLayer->posY;
		objAnim.isPaused = 0;
		objAnim.isStatic = 0;
		objAnim.newCycle = animLayer->framesCount;
		break;

	case 1:
	case 4:
	case 6:
		layer = obj.goblinStates[objAnim.state][0].layer;
		animation = obj.goblinStates[objAnim.state][0].animation;
		_vm->_scenery->updateAnim(layer, 0, animation, 0,
				*(obj.pPosX), *(obj.pPosY), 0);

		deltaHeight = _vm->_scenery->_animBottom - _vm->_scenery->_animTop;
		deltaWidth = _vm->_scenery->_animRight - _vm->_scenery->_animLeft;

		animLayer =
			_vm->_scenery->getAnimLayer(objAnim.animation, objAnim.layer);
		deltaX = animLayer->animDeltaX;
		deltaY = animLayer->animDeltaY;

		layer = obj.goblinStates[state][0].layer;
		animation = obj.goblinStates[state][0].animation;
		objAnim.state = state;
		objAnim.layer = layer;
		objAnim.animation = animation;
		objAnim.frame = 0;
		objAnim.isPaused = 0;
		objAnim.isStatic = 0;

		animLayer = _vm->_scenery->getAnimLayer(animation, layer);
		objAnim.newCycle = animLayer->framesCount;

		_vm->_scenery->updateAnim(layer, 0, animation, 0,
				*(obj.pPosX), *(obj.pPosY), 0);

		deltaHeight -= _vm->_scenery->_animBottom - _vm->_scenery->_animTop;
		deltaWidth -= _vm->_scenery->_animRight - _vm->_scenery->_animLeft;
		*(obj.pPosX) += deltaWidth + deltaX;
		*(obj.pPosY) += deltaHeight + deltaY;
		break;

	case 11:
		layer = obj.goblinStates[state][0].layer;
		animation = obj.goblinStates[state][0].animation;
		objAnim.state = state;
		objAnim.layer = layer;
		objAnim.animation = animation;
		objAnim.frame = 0;
		objAnim.isPaused = 0;
		objAnim.isStatic = 0;

		animLayer = _vm->_scenery->getAnimLayer(animation, layer);
		objAnim.newCycle = animLayer->framesCount;
		_vm->_scenery->updateAnim(layer, 0, animation, 0,
				*(obj.pPosX), *(obj.pPosY), 0);

		if (_vm->_map->_bigTiles)
			*(obj.pPosY) = ((obj.goblinY + 1) * _vm->_map->_tilesHeight) -
				(_vm->_scenery->_animBottom - _vm->_scenery->_animTop) -
				((obj.goblinY + 1) / 2);
		else
			*(obj.pPosY) = ((obj.goblinY + 1) * _vm->_map->_tilesHeight) -
				(_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
		*(obj.pPosX) = obj.goblinX * _vm->_map->_tilesWidth;
		break;
	}
}

void Inter_v2::o2_placeGoblin() {
	int16 index;
	int16 x, y;
	int16 state;

	index = _vm->_parse->parseValExpr();
	x = _vm->_parse->parseValExpr();
	y = _vm->_parse->parseValExpr();
	state = _vm->_parse->parseValExpr();

	_vm->_goblin->placeObject(0, 0, index, x, y, state);
}

void Inter_v2::o2_initScreen() {
	int16 offY;
	int16 videoMode;
	int16 width, height;

	offY = load16();

	videoMode = offY & 0xFF;
	offY = (offY >> 8) & 0xFF;

	width = _vm->_parse->parseValExpr();
	height = _vm->_parse->parseValExpr();

	_vm->_video->clearScreen();

	// Lost in Time switches to 640x400x16 when showing the title screen
	if (_vm->getGameType() == kGameTypeLostInTime) {

		if (videoMode == 0x10) {

			width = _vm->_width = 640;
			height = _vm->_height = 400;
			_vm->_global->_colorCount = 16;

			_vm->_video->setSize(true);

		} else if (_vm->_global->_videoMode == 0x10) {

			if (width == -1)
				width = 320;
			if (height == -1)
				height = 200;

			_vm->_width = 320;
			_vm->_height = 200;
			_vm->_global->_colorCount = 256;

			_vm->_video->setSize(false);

		}
	}

	_vm->_global->_fakeVideoMode = videoMode;

	// Some versions require this
	if (videoMode == 0xD)
		videoMode = _vm->_mode;

	if ((videoMode == _vm->_global->_videoMode) && (width == -1))
		return;

	if (width > 0)
		_vm->_video->_surfWidth = width;
	if (height > 0)
		_vm->_video->_surfHeight = height;

	_vm->_video->_splitHeight1 = MIN<int16>(_vm->_height, _vm->_video->_surfHeight - offY);
	_vm->_video->_splitHeight2 = offY;
	_vm->_video->_splitStart = _vm->_video->_surfHeight - offY;

	_vm->_video->_screenDeltaX = 0;
	_vm->_video->_screenDeltaY = 0;

	_vm->_global->_mouseMinX = 0;
	_vm->_global->_mouseMinY = 0;
	_vm->_global->_mouseMaxX = _vm->_width;
	_vm->_global->_mouseMaxY = _vm->_height - _vm->_video->_splitHeight2 - 1;

	_vm->_draw->closeScreen();
	_vm->_util->clearPalette();
	memset(_vm->_global->_redPalette, 0, 256);
	memset(_vm->_global->_greenPalette, 0, 256);
	memset(_vm->_global->_bluePalette, 0, 256);

	_vm->_global->_videoMode = videoMode;
	_vm->_video->initPrimary(videoMode);
	WRITE_VAR(15, _vm->_global->_fakeVideoMode);

	_vm->_global->_setAllPalette = true;

	_vm->_util->setMousePos(_vm->_global->_inter_mouseX,
			_vm->_global->_inter_mouseY);
	_vm->_util->clearPalette();

	_vm->_draw->initScreen();

	_vm->_util->setScrollOffset();
}

void Inter_v2::o2_scroll() {
	int16 startX;
	int16 startY;
	int16 endX;
	int16 endY;
	int16 stepX;
	int16 stepY;
	int16 curX;
	int16 curY;

	startX = CLIP((int) _vm->_parse->parseValExpr(), 0,
			_vm->_video->_surfWidth - _vm->_width);
	startY = CLIP((int) _vm->_parse->parseValExpr(), 0,
			_vm->_video->_surfHeight - _vm->_height);
	endX = CLIP((int) _vm->_parse->parseValExpr(), 0,
			_vm->_video->_surfWidth - _vm->_width);
	endY = CLIP((int) _vm->_parse->parseValExpr(), 0,
			_vm->_video->_surfHeight - _vm->_height);
	stepX = _vm->_parse->parseValExpr();
	stepY = _vm->_parse->parseValExpr();

	curX = startX;
	curY = startY;
	while (!_vm->shouldQuit() && ((curX != endX) || (curY != endY))) {
		curX = stepX > 0 ? MIN(curX + stepX, (int) endX) :
			MAX(curX + stepX, (int) endX);
		curY = stepY > 0 ? MIN(curY + stepY, (int) endY) :
			MAX(curY + stepY, (int) endY);

		_vm->_draw->_scrollOffsetX = curX;
		_vm->_draw->_scrollOffsetY = curY;
		_vm->_util->setScrollOffset();
		_vm->_video->dirtyRectsAll();
	}
}

void Inter_v2::o2_setScrollOffset() {
	int16 offsetX, offsetY;

	offsetX = _vm->_parse->parseValExpr();
	offsetY = _vm->_parse->parseValExpr();

	if (offsetX == -1) {
		WRITE_VAR(2, _vm->_draw->_scrollOffsetX);
		WRITE_VAR(3, _vm->_draw->_scrollOffsetY);
	} else {
		int16 screenW = _vm->_video->_surfWidth;
		int16 screenH = _vm->_video->_surfHeight;

		if (screenW > _vm->_width)
			screenW -= _vm->_width;
		if (screenH > _vm->_height)
			screenH -= _vm->_height;

		_vm->_draw->_scrollOffsetX = CLIP<int16>(offsetX, 0, screenW);
		_vm->_draw->_scrollOffsetY = CLIP<int16>(offsetY, 0, screenH);
		_vm->_video->dirtyRectsAll();
	}

	_vm->_util->setScrollOffset();
	_noBusyWait = true;
}

void Inter_v2::o2_playImd() {
	char imd[128];
	int16 x, y;
	int16 startFrame;
	int16 lastFrame;
	int16 breakKey;
	int16 flags;
	int16 palStart;
	int16 palEnd;
	uint16 palCmd;
	bool close;

	evalExpr(0);
	_vm->_parse->getResultStr()[8] = 0;
	strncpy0(imd, _vm->_parse->getResultStr(), 127);

	x = _vm->_parse->parseValExpr();
	y = _vm->_parse->parseValExpr();
	startFrame = _vm->_parse->parseValExpr();
	lastFrame = _vm->_parse->parseValExpr();
	breakKey = _vm->_parse->parseValExpr();
	flags = _vm->_parse->parseValExpr();
	palStart = _vm->_parse->parseValExpr();
	palEnd = _vm->_parse->parseValExpr();
	palCmd = 1 << (flags & 0x3F);

	debugC(1, kDebugVideo, "Playing video \"%s\" @ %d+%d, frames %d - %d, "
			"paletteCmd %d (%d - %d), flags %X", _vm->_parse->getResultStr(), x, y,
			startFrame, lastFrame, palCmd, palStart, palEnd, flags);

	if ((imd[0] != 0) && !_vm->_vidPlayer->primaryOpen(imd, x, y, flags)) {
		WRITE_VAR(11, (uint32) -1);
		return;
	}

	close = (lastFrame == -1);
	if (startFrame == -2) {
		startFrame = lastFrame = 0;
		close = false;
	}

	if (startFrame >= 0) {
		_vm->_game->_preventScroll = true;
		_vm->_vidPlayer->primaryPlay(startFrame, lastFrame, breakKey, palCmd, palStart, palEnd, 0);
		_vm->_game->_preventScroll = false;
	}

	if (close)
		_vm->_vidPlayer->primaryClose();
}

void Inter_v2::o2_getImdInfo() {
	int16 varX, varY;
	int16 varFrames;
	int16 varWidth, varHeight;

	evalExpr(0);
	varX = _vm->_parse->parseVarIndex();
	varY = _vm->_parse->parseVarIndex();
	varFrames = _vm->_parse->parseVarIndex();
	varWidth = _vm->_parse->parseVarIndex();
	varHeight = _vm->_parse->parseVarIndex();

	// WORKAROUND: The nut rolling animation in the administration center
	// in Woodruff is called "noixroul", but the scripts think it's "noixroule".
	if ((_vm->getGameType() == kGameTypeWoodruff) &&
			(!scumm_stricmp(_vm->_parse->getResultStr(), "noixroule")))
		strcpy(_vm->_parse->getResultStr(), "noixroul");

	_vm->_vidPlayer->writeVideoInfo(_vm->_parse->getResultStr(), varX, varY,
			varFrames, varWidth, varHeight);
}

void Inter_v2::o2_openItk() {
	char fileName[32];

	evalExpr(0);
	strncpy0(fileName, _vm->_parse->getResultStr(), 27);
	if (!strchr(fileName, '.'))
		strcat(fileName, ".ITK");

	_vm->_dataIO->openDataFile(fileName, true);
}

void Inter_v2::o2_closeItk() {
	_vm->_dataIO->closeDataFile(true);
}

void Inter_v2::o2_setImdFrontSurf() {
}

void Inter_v2::o2_resetImdFrontSurf() {
}

bool Inter_v2::o2_assign(OpFuncParams &params) {
	byte destType = *_vm->_global->_inter_execPtr;
	int16 dest = _vm->_parse->parseVarIndex();

	byte loopCount;
	if (*_vm->_global->_inter_execPtr == 99) {
		_vm->_global->_inter_execPtr++;
		loopCount = *_vm->_global->_inter_execPtr++;
	} else
		loopCount = 1;

	for (int i = 0; i < loopCount; i++) {
		int16 result;
		int16 srcType = evalExpr(&result);

		switch (destType) {
		case TYPE_VAR_INT8:
		case TYPE_ARRAY_INT8:
			WRITE_VARO_UINT8(dest + i, _vm->_parse->getResultInt());
			break;

		case TYPE_VAR_INT16:
		case TYPE_ARRAY_INT16:
			WRITE_VARO_UINT16(dest + i * 2, _vm->_parse->getResultInt());
			break;

		case TYPE_VAR_INT32:
		case TYPE_ARRAY_INT32:
			WRITE_VAR_OFFSET(dest + i * 4, _vm->_parse->getResultInt());
			break;

		case TYPE_VAR_INT32_AS_INT16:
			WRITE_VARO_UINT16(dest + i * 4, _vm->_parse->getResultInt());
			break;

		case TYPE_VAR_STR:
		case TYPE_ARRAY_STR:
			if (srcType == TYPE_IMM_INT16)
				WRITE_VARO_UINT8(dest, result);
			else
				WRITE_VARO_STR(dest, _vm->_parse->getResultStr());
			break;
		}
	}

	return false;
}

bool Inter_v2::o2_printText(OpFuncParams &params) {
	char buf[60];
	int i;

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->_backColor = _vm->_parse->parseValExpr();
	_vm->_draw->_frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->_fontIndex = _vm->_parse->parseValExpr();
	_vm->_draw->_destSurface = 21;
	_vm->_draw->_textToPrint = buf;
	_vm->_draw->_transparency = 0;

	if (_vm->_draw->_backColor == 16) {
		_vm->_draw->_backColor = 0;
		_vm->_draw->_transparency = 1;
	}

	do {
		for (i = 0; (((char) *_vm->_global->_inter_execPtr) != '.') &&
				(*_vm->_global->_inter_execPtr != 200);
				i++, _vm->_global->_inter_execPtr++) {
			buf[i] = (char) *_vm->_global->_inter_execPtr;
		}

		if (*_vm->_global->_inter_execPtr != 200) {
			_vm->_global->_inter_execPtr++;
			switch (*_vm->_global->_inter_execPtr) {
			case TYPE_VAR_INT8:
			case TYPE_ARRAY_INT8:
				sprintf(buf + i, "%d",
						(int8) READ_VARO_UINT8(_vm->_parse->parseVarIndex()));
				break;

			case TYPE_VAR_INT16:
			case TYPE_VAR_INT32_AS_INT16:
			case TYPE_ARRAY_INT16:
				sprintf(buf + i, "%d",
						(int16) READ_VARO_UINT16(_vm->_parse->parseVarIndex()));
				break;

			case TYPE_VAR_INT32:
			case TYPE_ARRAY_INT32:
				sprintf(buf + i, "%d",
						VAR_OFFSET(_vm->_parse->parseVarIndex()));
				break;

			case TYPE_VAR_STR:
			case TYPE_ARRAY_STR:
				sprintf(buf + i, "%s",
						GET_VARO_STR(_vm->_parse->parseVarIndex()));
				break;
			}
			_vm->_global->_inter_execPtr++;
		} else
			buf[i] = 0;

		_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
	} while (*_vm->_global->_inter_execPtr != 200);

	_vm->_global->_inter_execPtr++;

	return false;
}

bool Inter_v2::o2_animPalInit(OpFuncParams &params) {
	int16 index;

	index = load16();
	if (index > 0) {
		index--;
		_animPalLowIndex[index] = _vm->_parse->parseValExpr();
		_animPalHighIndex[index] = _vm->_parse->parseValExpr();
		_animPalDir[index] = 1;
	} else if (index == 0) {
		memset(_animPalDir, 0, 8 * sizeof(int16));
		_vm->_parse->parseValExpr();
		_vm->_parse->parseValExpr();
	} else {
		index = -index - 1;
		_animPalLowIndex[index] = _vm->_parse->parseValExpr();
		_animPalHighIndex[index] = _vm->_parse->parseValExpr();
		_animPalDir[index] = -1;
	}
	return false;
}

bool Inter_v2::o2_addCollision(OpFuncParams &params) {
	int16 id;
	int16 left, top, width, height;
	int16 flags;
	int16 key;
	int16 funcSub;

	id = _vm->_parse->parseValExpr();
	funcSub = _vm->_global->_inter_execPtr - _vm->_game->_totFileData;
	left = _vm->_parse->parseValExpr();
	top = _vm->_parse->parseValExpr();
	width = _vm->_parse->parseValExpr();
	height = _vm->_parse->parseValExpr();
	flags = _vm->_parse->parseValExpr();
	key = load16();

	if (key == 0)
		key = ABS(id) + 41960;

	_vm->_draw->adjustCoords(0, &left, &top);
	_vm->_draw->adjustCoords(2, &width, &height);

	if (left < 0) {
		width += left;
		left = 0;
	}

	if (top < 0) {
		height += top;
		top = 0;
	}

	int16 index;
	if (id < 0)
		index = _vm->_game->addNewCollision(0xD000 - id, left & 0xFFFC, top & 0xFFFC,
				left + width + 3, top + height + 3, flags, key, 0, 0);
	else
		index = _vm->_game->addNewCollision(0xE000 + id, left, top,
				left + width - 1, top + height - 1, flags, key, 0, 0);

	_vm->_game->_collisionAreas[index].funcSub = funcSub;

	return false;
}

bool Inter_v2::o2_freeCollision(OpFuncParams &params) {
	int16 id;

	id = _vm->_parse->parseValExpr();
	if (id == -2) {
		for (int i = 0; i < 150; i++) {
			if ((_vm->_game->_collisionAreas[i].id & 0xF000) == 0xD000)
				_vm->_game->_collisionAreas[i].left = 0xFFFF;
		}
	} else if (id == -1) {
		for (int i = 0; i < 150; i++) {
			if ((_vm->_game->_collisionAreas[i].id & 0xF000) == 0xE000)
				_vm->_game->_collisionAreas[i].left = 0xFFFF;
		}
	} else
		_vm->_game->freeCollision(0xE000 + id);

	return false;
}

bool Inter_v2::o2_goblinFunc(OpFuncParams &params) {
	// TODO: In Inca 2, this is the big SpaceShoot0rz()-Opcode.
	// It's not yet implemented, so we fudge our way through
	// and pretend we've won.
	if (_vm->getGameType() == kGameTypeInca2) {
		_vm->_global->_inter_execPtr += 4;
		uint16 resVar = (uint16) load16();
		_vm->_global->_inter_execPtr += 4;

		WRITE_VAR(resVar, 1);
		return false;
	}

	OpGobParams gobParams;
	int16 cmd;

	cmd = load16();

	gobParams.paramCount = load16();
	gobParams.extraData = cmd;

	if (cmd != 101)
		executeOpcodeGob(cmd, gobParams);
	return false;
}

bool Inter_v2::o2_stopSound(OpFuncParams &params) {
	int16 expr;

	expr = _vm->_parse->parseValExpr();

	if (expr < 0) {
		_vm->_sound->adlibStop();
	} else
		_vm->_sound->blasterStop(expr);

	_soundEndTimeKey = 0;
	return false;
}

bool Inter_v2::o2_loadSound(OpFuncParams &params) {
	loadSound(0);
	return false;
}

bool Inter_v2::o2_getFreeMem(OpFuncParams &params) {
	int16 freeVar;
	int16 maxFreeVar;

	freeVar = _vm->_parse->parseVarIndex();
	maxFreeVar = _vm->_parse->parseVarIndex();

	// HACK
	WRITE_VAR_OFFSET(freeVar, 1000000);
	WRITE_VAR_OFFSET(maxFreeVar, 1000000);
	WRITE_VAR(16, READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4);
	return false;
}

bool Inter_v2::o2_checkData(OpFuncParams &params) {
	int16 handle;
	int16 varOff;
	int32 size;
	SaveLoad::SaveMode mode;

	evalExpr(0);
	varOff = _vm->_parse->parseVarIndex();

	size = -1;
	handle = 1;

	mode = _vm->_saveLoad->getSaveMode(_vm->_parse->getResultStr());
	if (mode == SaveLoad::kSaveModeNone) {

		if (_vm->_dataIO->existData(_vm->_parse->getResultStr()))
			size = _vm->_dataIO->getDataSize(_vm->_parse->getResultStr());
		else
			warning("File \"%s\" not found", _vm->_parse->getResultStr());

	} else if (mode == SaveLoad::kSaveModeSave)
		size = _vm->_saveLoad->getSize(_vm->_parse->getResultStr());
	else if (mode == SaveLoad::kSaveModeExists)
		size = 23;

	if (size == -1)
		handle = -1;

	debugC(2, kDebugFileIO, "Requested size of file \"%s\": %d",
			_vm->_parse->getResultStr(), size);

	WRITE_VAR_OFFSET(varOff, handle);
	WRITE_VAR(16, (uint32) size);

	return false;
}

bool Inter_v2::o2_readData(OpFuncParams &params) {
	int32 retSize;
	int32 size;
	int32 offset;
	int16 dataVar;
	int16 handle;
	byte *buf;
	SaveLoad::SaveMode mode;

	evalExpr(0);
	dataVar = _vm->_parse->parseVarIndex();
	size = _vm->_parse->parseValExpr();
	evalExpr(0);
	offset = _vm->_parse->getResultInt();
	retSize = 0;

	debugC(2, kDebugFileIO, "Read from file \"%s\" (%d, %d bytes at %d)",
			_vm->_parse->getResultStr(), dataVar, size, offset);

	mode = _vm->_saveLoad->getSaveMode(_vm->_parse->getResultStr());
	if (mode == SaveLoad::kSaveModeSave) {
		WRITE_VAR(1, 1);
		if (_vm->_saveLoad->load(_vm->_parse->getResultStr(), dataVar, size, offset))
			WRITE_VAR(1, 0);
		return false;
	} else if (mode == SaveLoad::kSaveModeIgnore)
		return false;

	if (size < 0) {
		warning("Attempted to read a raw sprite from file \"%s\"",
				_vm->_parse->getResultStr());
		return false ;
	} else if (size == 0) {
		dataVar = 0;
		size = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;
	}

	buf = _variables->getAddressOff8(dataVar);

	if (_vm->_parse->getResultStr()[0] == 0) {
		WRITE_VAR(1, size);
		return false;
	}

	WRITE_VAR(1, 1);
	handle = _vm->_dataIO->openData(_vm->_parse->getResultStr());

	if (handle < 0)
		return false;

	DataStream *stream = _vm->_dataIO->openAsStream(handle, true);

	_vm->_draw->animateCursor(4);
	if (offset < 0)
		stream->seek(offset + 1, SEEK_END);
	else
		stream->seek(offset);

	if (((dataVar >> 2) == 59) && (size == 4)) {
		WRITE_VAR(59, stream->readUint32LE());
		// The scripts in some versions divide through 256^3 then,
		// effectively doing a LE->BE conversion
		if ((_vm->getPlatform() != Common::kPlatformPC) && (VAR(59) < 256))
			WRITE_VAR(59, SWAP_BYTES_32(VAR(59)));
	} else
		retSize = stream->read(buf, size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	delete stream;
	return false;
}

bool Inter_v2::o2_writeData(OpFuncParams &params) {
	int32 offset;
	int32 size;
	int16 dataVar;
	SaveLoad::SaveMode mode;

	evalExpr(0);
	dataVar = _vm->_parse->parseVarIndex();
	size = _vm->_parse->parseValExpr();
	evalExpr(0);
	offset = _vm->_parse->getResultInt();

	debugC(2, kDebugFileIO, "Write to file \"%s\" (%d, %d bytes at %d)",
			_vm->_parse->getResultStr(), dataVar, size, offset);

	WRITE_VAR(1, 1);

	mode = _vm->_saveLoad->getSaveMode(_vm->_parse->getResultStr());
	if (mode == SaveLoad::kSaveModeSave) {
		if (_vm->_saveLoad->save(_vm->_parse->getResultStr(), dataVar, size, offset))
			WRITE_VAR(1, 0);
	} else if (mode == SaveLoad::kSaveModeNone)
		warning("Attempted to write to file \"%s\"", _vm->_parse->getResultStr());

	return false;
}

void Inter_v2::o2_loadInfogramesIns(OpGobParams &params) {
	int16 varName;
	char fileName[20];

	varName = load16();

	strncpy0(fileName, GET_VAR_STR(varName), 15);
	strcat(fileName, ".INS");

	_vm->_sound->infogramesLoadInstruments(fileName);
}

void Inter_v2::o2_playInfogrames(OpGobParams &params) {
	int16 varName;
	char fileName[20];

	varName = load16();

	strncpy0(fileName, GET_VAR_STR(varName), 15);
	strcat(fileName, ".DUM");

	_vm->_sound->infogramesLoadSong(fileName);
	_vm->_sound->infogramesPlay();
}

void Inter_v2::o2_startInfogrames(OpGobParams &params) {
	load16();

	_vm->_sound->infogramesPlay();
}

void Inter_v2::o2_stopInfogrames(OpGobParams &params) {
	load16();

	_vm->_sound->infogramesStop();
}

void Inter_v2::o2_playProtracker(OpGobParams &params) {
	_vm->_sound->protrackerPlay("mod.babayaga");
}

void Inter_v2::o2_stopProtracker(OpGobParams &params) {
	_vm->_sound->protrackerStop();
}

void Inter_v2::o2_handleGoblins(OpGobParams &params) {
	_vm->_goblin->_gob1NoTurn = VAR(load16()) != 0;
	_vm->_goblin->_gob2NoTurn = VAR(load16()) != 0;
	_vm->_goblin->_gob1RelaxTimeVar = load16();
	_vm->_goblin->_gob2RelaxTimeVar = load16();
	_vm->_goblin->_gob1Busy = VAR(load16()) != 0;
	_vm->_goblin->_gob2Busy = VAR(load16()) != 0;
	_vm->_goblin->handleGoblins();
}

int16 Inter_v2::loadSound(int16 search) {
	byte *dataPtr;
	int16 id;
	int16 slot;
	uint16 slotIdMask;
	uint32 dataSize;
	SoundType type;
	SoundSource source;

	type = SOUND_SND;
	slotIdMask = 0;
	dataSize = 0;

	if (!search) {
		slot = _vm->_parse->parseValExpr();
		if (slot < 0) {
			type = SOUND_ADL;
			slot = -slot;
		}
		id = load16();
	} else {
		id = load16();

		for (slot = 0; slot < Sound::kSoundsCount; slot++)
			if (_vm->_sound->sampleGetBySlot(slot)->isId(id)) {
				slotIdMask = 0x8000;
				break;
			}

		if (slot == Sound::kSoundsCount) {
			for (slot = (Sound::kSoundsCount - 1); slot >= 0; slot--) {
				if (_vm->_sound->sampleGetBySlot(slot)->empty())
					break;
			}

			if (slot == -1) {
				warning("Inter_v2::loadSound(): No free slot to load sound "
						"(id = %d)", id);
				return 0;
			}
		}
	}

	SoundDesc *sample = _vm->_sound->sampleGetBySlot(slot);

	_vm->_sound->sampleFree(sample, true, slot);

	if (id == -1) {
		char sndfile[14];

		source = SOUND_FILE;

		strncpy0(sndfile, (const char *) _vm->_global->_inter_execPtr, 9);
		_vm->_global->_inter_execPtr += 9;

		if (type == SOUND_ADL)
			strcat(sndfile, ".ADL");
		else
			strcat(sndfile, ".SND");

		dataPtr = (byte *) _vm->_dataIO->getData(sndfile);
		if (dataPtr)
			dataSize = _vm->_dataIO->getDataSize(sndfile);
	} else if (id >= 30000) {
		source = SOUND_EXT;

		dataPtr = (byte *) _vm->_game->loadExtData(id, 0, 0, &dataSize);
	} else {
		int16 totSize;

		source = SOUND_TOT;

		dataPtr = (byte *) _vm->_game->loadTotResource(id, &totSize);
		dataSize = (uint32) ((int32) totSize);
	}

	if (dataPtr) {
		sample->load(type, source, dataPtr, dataSize);
		sample->_id = id;
	}

	return slot | slotIdMask;
}

void Inter_v2::animPalette() {
	int16 i;
	int16 j;
	Video::Color col;
	bool first;

	first = true;
	for (j = 0; j < 8; j ++) {
		if (_animPalDir[j] == 0)
			continue;

		if (first) {
			_vm->_video->waitRetrace();
			first = false;
		}

		if (_animPalDir[j] == -1) {
			col = _vm->_global->_pPaletteDesc->vgaPal[_animPalLowIndex[j]];

			for (i = _animPalLowIndex[j]; i < _animPalHighIndex[j]; i++)
				_vm->_draw->_vgaPalette[i] = _vm->_draw->_vgaPalette[i + 1];

			_vm->_global->_pPaletteDesc->vgaPal[_animPalHighIndex[j]] = col;
		} else {
			col = _vm->_global->_pPaletteDesc->vgaPal[_animPalHighIndex[j]];
			for (i = _animPalHighIndex[j]; i > _animPalLowIndex[j]; i--)
				_vm->_draw->_vgaPalette[i] = _vm->_draw->_vgaPalette[i - 1];

			_vm->_global->_pPaletteDesc->vgaPal[_animPalLowIndex[j]] = col;
		}
		_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;
	}
	if (!first)
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

} // End of namespace Gob
