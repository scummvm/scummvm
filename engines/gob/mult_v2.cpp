/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
#include "common/endian.h"
#include "common/stream.h"

#include "gob/gob.h"
#include "gob/mult.h"
#include "gob/game.h"
#include "gob/scenery.h"
#include "gob/global.h"
#include "gob/inter.h"
#include "gob/anim.h"
#include "gob/draw.h"
#include "gob/palanim.h"
#include "gob/parse.h"
#include "gob/music.h"
#include "gob/map.h"

namespace Gob {

Mult_v2::Mult_v2(GobEngine *vm) : Mult_v1(vm) {
	int i;

	_renderData2 = 0;
	_multData2 = 0;
	for (i = 0; i < 8; i++) _multDatas[i] = 0;
}

Mult_v2::~Mult_v2() {
	int i;

	freeMultKeys();
	for (i = 0; i < 8; i++) {
		_multData2 = _multDatas[i];
		freeMultKeys();
	}

	if (_orderArray)
		delete[] _orderArray;
	if (_renderData2)
		delete[] _renderData2;
}

void Mult_v2::loadMult(int16 resId) {
	int16 i, j;
	int8 index;
	char staticCount;
	char animCount;
	char *extData;
	bool hbstaticCount;
	int16 palIndex;
	int16 size;
	
	index = (resId & 0x8000) ? *_vm->_global->_inter_execPtr++ : 0;

	_multData2 = new Mult_Data;
	memset(_multData2, 0, sizeof(Mult_Data));

	_multDatas[index] = _multData2;

	for (i = 0; i < 10; i++) {
		_multData2->staticLoaded[i] = 0;
		_multData2->animLoaded[i] = 0;
	}

	for (i = 0; i < 4; i++)
		_multData2->field_124[0][i] = i;

	_multData2->sndSlotsCount = 0;
	_multData2->frameStart = 0;

	extData = _vm->_game->loadExtData(resId, 0, 0);
	Common::MemoryReadStream data((byte *) extData, 4294967295U);

	_multData2->staticCount = staticCount = data.readSByte();
	_multData2->animCount = animCount = data.readSByte();
	staticCount++;
	animCount++;

	hbstaticCount = (staticCount & 0x80) != 0;
	staticCount &= 0x7F;

	debugC(7, DEBUG_GRAPHICS, "statics: %u, anims: %u, hb: %u", staticCount, animCount, hbstaticCount);
	for (i = 0; i < staticCount; i++, data.seek(14, SEEK_CUR)) {
		_multData2->staticIndices[i] = _vm->_scenery->loadStatic(1);

		if (_multData2->staticIndices[i] >= 100) {
			_multData2->staticIndices[i] -= 100;
			_multData2->staticLoaded[i] = 1;
		} else
			_multData2->staticLoaded[i] = 0;
	}

	for (i = 0; i < animCount; i++, data.seek(14, SEEK_CUR)) {
		_multData2->animIndices[i] = _vm->_scenery->loadAnim(1);

		if (_multData2->animIndices[i] >= 100) {
			_multData2->animIndices[i] -= 100;
			_multData2->animLoaded[i] = 1;
		} else
			_multData2->animLoaded[i] = 0;
	}

	_multData2->frameRate = data.readSint16LE();
	_multData2->staticKeysCount = data.readSint16LE();
	_multData2->staticKeys = new Mult_StaticKey[_multData2->staticKeysCount];
	for (i = 0; i < _multData2->staticKeysCount; i++) {
		_multData2->staticKeys[i].frame = data.readSint16LE();
		_multData2->staticKeys[i].layer = data.readSint16LE();
	}

	for (i = 0; i < 4; i++) {
		_multData2->someKeysCount[i] = 0;
		_multData2->someKeys[i] = 0;
		_multData2->someKeysIndices[i] = -1;

		for (j = 0; j < 4; j++) {
			_multData2->field_15F[i][j] = 0;
			_multData2->field_17F[i][j] = 0;
		}

		_multData2->animKeysFrames[i] = -1;
		_multData2->animKeysCount[i] = data.readSint16LE();
		_multData2->animKeys[i] = new Mult_AnimKey[_multData2->animKeysCount[i]];
		for (j = 0; j < _multData2->animKeysCount[i]; j++) {
			_multData2->animKeys[i][j].frame = data.readSint16LE();
			_multData2->animKeys[i][j].layer = data.readSint16LE();
			_multData2->animKeys[i][j].posX = data.readSint16LE();
			_multData2->animKeys[i][j].posY = data.readSint16LE();
			_multData2->animKeys[i][j].order = data.readSint16LE();
		}
	}

	for (palIndex = 0; palIndex < 5; palIndex++) {
		for (i = 0; i < 16; i++) {
			_multData2->fadePal[palIndex][i].red = data.readByte();
			_multData2->fadePal[palIndex][i].green = data.readByte();
			_multData2->fadePal[palIndex][i].blue = data.readByte();
		}
	}

	_multData2->palFadeKeysCount = data.readSint16LE();
	_multData2->palFadeKeys = new Mult_PalFadeKey[_multData2->palFadeKeysCount];
	for (i = 0; i < _multData2->palFadeKeysCount; i++) {
		_multData2->palFadeKeys[i].frame = data.readSint16LE();
		_multData2->palFadeKeys[i].fade = data.readSint16LE();
		_multData2->palFadeKeys[i].palIndex = data.readSint16LE();
		_multData2->palFadeKeys[i].flag = data.readSByte();
	}

	_multData2->palKeysCount = data.readSint16LE();
	_multData2->palKeys = new Mult_PalKey[_multData2->palKeysCount];
	for (i = 0; i < _multData2->palKeysCount; i++) {
		_multData2->palKeys[i].frame = data.readSint16LE();
		_multData2->palKeys[i].cmd = data.readSint16LE();
		_multData2->palKeys[i].rates[0] = data.readSint16LE();
		_multData2->palKeys[i].rates[1] = data.readSint16LE();
		_multData2->palKeys[i].rates[2] = data.readSint16LE();
		_multData2->palKeys[i].rates[3] = data.readSint16LE();
		_multData2->palKeys[i].unknown0 = data.readSint16LE();
		_multData2->palKeys[i].unknown1 = data.readSint16LE();
		data.read(_multData2->palKeys[i].subst, 64);
	}

	_multData2->textKeysCount = data.readSint16LE();
	_multData2->textKeys = new Mult_TextKey[_multData2->textKeysCount];
	for (i = 0; i < _multData2->textKeysCount; i++) {
		_multData2->textKeys[i].frame = data.readSint16LE();
		_multData2->textKeys[i].cmd = data.readSint16LE();
		if (!hbstaticCount)
			data.seek(24, SEEK_CUR);
	}

	_multData2->sndKeysCount = data.readSint16LE();
	warning("SoundKeyCount: %d", _multData2->sndKeysCount);
	_multData2->sndKeys = new Mult_SndKey[_multData2->sndKeysCount];
	for (i = 0; i < _multData2->sndKeysCount; i++) {
		_multData2->sndKeys[i].frame = data.readSint16LE();
		_multData2->sndKeys[i].cmd = data.readSint16LE();
		_multData2->sndKeys[i].freq = data.readSint16LE();
		_multData2->sndKeys[i].channel = data.readSint16LE();
		_multData2->sndKeys[i].repCount = data.readSint16LE();
		_multData2->sndKeys[i].soundIndex = -1;
		_multData2->sndKeys[i].resId = -1;
		data.seek(2, SEEK_CUR);
		if (!hbstaticCount)
			data.seek(24, SEEK_CUR);

		switch (_multData2->sndKeys[i].cmd) {
		case 1:
		case 4:
			_multData2->sndKeys[i].resId = READ_LE_UINT16(_vm->_global->_inter_execPtr);
			for (j = 0; j < i; j++) { // loc_7071
				if (_multData2->sndKeys[j].resId == _multData2->sndKeys[i].resId) {
					_multData2->sndKeys[i].soundIndex = _multData2->sndKeys[j].soundIndex;
					_vm->_global->_inter_execPtr += 2;
					break;
				}
			}
			if (i == j) {
				_multData2->sndSlot[_multData2->sndSlotsCount] = _vm->_inter->loadSound(1);
				_multData2->sndKeys[i].soundIndex = _multData2->sndSlot[_multData2->sndSlotsCount] & 0x7FFF;
				_multData2->sndSlotsCount++;
			}
			break;
		case 3:
			_vm->_global->_inter_execPtr += 4;
			break;
		}
	}
	
	_multData2->somepointer09 = 0;
	_multData2->somepointer10 = 0;

	if (hbstaticCount) {
//		warning("GOB2 Stub! Mult_Data.somepointer09, Mult_Data.somepointer10");
		size = _vm->_inter->load16();
		_multData2->execPtr = _vm->_global->_inter_execPtr;
		_vm->_global->_inter_execPtr += size * 2;
		if (_vm->_game->_totFileData[0x29] >= 51) {
			size = data.readSint16LE();
			_multData2->somepointer10 = new char[size * 20];
//			data.read(_multData2->somepointer09 /*???*/, size * 20);
			data.read(_multData2->somepointer10, size * 20);
			size = _vm->_inter->load16();
			if (size > 0) {
				_multData2->somepointer09 = new char[size * 14];
				memcpy(_multData2->somepointer09, _vm->_global->_inter_execPtr, size * 14);
				_vm->_global->_inter_execPtr += size * 14;
				data.seek(2, SEEK_CUR);
				for (i = 0; i < 4; i++) {
					_multData2->someKeysCount[i] = data.readSint16LE();
					_multData2->someKeys[i] = new Mult_SomeKey[_multData2->someKeysCount[i]];
					for (j = 0; j < _multData2->someKeysCount[i]; j++) {
						_multData2->someKeys[i][j].frame = data.readSint16LE();
						_multData2->someKeys[i][j].field_2 = data.readSint16LE();
						_multData2->someKeys[i][j].field_4 = data.readSint16LE();
						_multData2->someKeys[i][j].field_6 = data.readSint16LE();
						_multData2->someKeys[i][j].field_8 = data.readSint16LE();
						_multData2->someKeys[i][j].field_A = data.readSint16LE();
						_multData2->someKeys[i][j].field_C = data.readSint16LE();
						_multData2->someKeys[i][j].field_E = data.readSint16LE();
					}
				}
			}
		}
	}

	delete[] extData;
}

void Mult_v2::setMultData(uint16 multindex) {
	if (multindex > 7)
		error("Multindex out of range");

	debugC(4, DEBUG_GAMEFLOW, "Switching to mult %d", multindex);
	_multData2 = _multDatas[multindex];
}

void Mult_v2::multSub(uint16 multindex) {
	uint16 flags;
	int16 expr;
	int16 textFrame;
	int16 index; // di
	int i;
	int j;

	flags = multindex;
	multindex = (multindex >> 12) & 0xF;

	if (multindex > 7)
		error("Multindex out of range");

	debugC(4, DEBUG_GAMEFLOW, "Sub mult %d", multindex);
	_multData2 = _multDatas[multindex];

	if (_multData2 == 0) {
		_vm->_parse->parseValExpr();
		_vm->_parse->parseValExpr();
		_vm->_parse->parseValExpr();
		_vm->_parse->parseValExpr();
		return;
	}

	if (flags & 0x200)
		index = 3;
	else if (flags & 0x100)
		index = 2;
	else if (flags & 0x80)
		index = 1;
	else
		index = 0;

	if (flags & 0x400) {
		flags = 0x400;
		_multData2->field_156 = -1;
	} else {
		_multData2->field_156 = 1;
		flags &= 0x7F;
	}

	_multData2->field_124[index][0] = flags;
	for (i = 1; i < 4; i++) {
		_multData2->field_124[index][i] = _vm->_parse->parseValExpr();
	}
	expr = _vm->_parse->parseValExpr();
	_multData2->animKeysFrames[index] = expr;
	_multData2->someKeysFrames[index] = expr;
	
	WRITE_VAR(18 + index, expr);
	if (expr == -1) {
		if (_objects)
			for (i = 0; i < 4; i++)
				if ((_multData2->field_124[index][i] != -1) && (_multData2->field_124[index][i] != 1024))
					_objects[_multData2->field_124[index][i]].pAnimData->animType =
						_objects[_multData2->field_124[index][i]].pAnimData->field_17;
	} else {
		if (_multData2->field_156 == 1) {
			_multData2->field_157[index] = 32000;
			for (i = 0; i < _multData2->textKeysCount; i++) {
				textFrame = _multData2->textKeys[i].frame;
				if ((textFrame > _multData2->someKeysFrames[index]) &&
						(textFrame < _multData2->field_157[index])) {
					_multData2->field_157[index] = textFrame;
				}
			}
		} else {
			_multData2->field_157[index] = 0;
			for (i = 0; i < _multData2->textKeysCount; i++) {
				textFrame = _multData2->textKeys[i].frame;
				if ((textFrame < _multData2->someKeysFrames[index]) &&
						(textFrame > _multData2->field_157[index])) {
					_multData2->field_157[index] = textFrame;
				}
			}
		}
		if (_objects) {
			for (i = 0; i < 4; i++) {
				if ((_multData2->field_124[index][i] != -1) && (_multData2->field_124[index][i] != 1024))
					_objects[_multData2->field_124[index][i]].pAnimData->field_17 =
						_objects[_multData2->field_124[index][i]].pAnimData->animType;
			}
		}

		for (i = 0; i < 4; i++) {
			_multData2->field_15F[index][i] = 0;
			for (j = 0; j < _multData2->animKeysCount[i]; j++) {
				if (_multData2->animKeys[i][j].frame >= _multData2->someKeysFrames[index])
					_multData2->field_15F[index][i] = j;
			}
		}

		if (_multData2->field_156 == -1) { // loc_60CF
			warning("Mult_v2::multSub(), someKeys and someKeysIndices");
		}

		for (i = 0; i < 4; i++) {
			_multData2->field_17F[index][i] = 0;
			for (j = 0; j < _multData2->someKeysCount[i]; j++) {
				if (_multData2->field_156 == 1) {
					if (_multData2->someKeys[i][j].frame >= _multData2->someKeysFrames[index]) {
						_multData2->field_17F[index][i] = j;
						break;
					}
				} else {
					if (_multData2->someKeys[i][j].frame >= _multData2->field_157[index]) {
						_multData2->field_17F[index][i] = j;
						break;
					}
				}
			}
		}
	}
}

void Mult_v2::playMult(int16 startFrame, int16 endFrame, char checkEscape,
	    char handleMouse) {
	char stopNoClear;
	char stop;
	Mult_Object *multObj;
	Mult_AnimData *animData;

	if (_multData2 == 0)
		return;

	stopNoClear = 0;
	_frame = startFrame;
	if (endFrame == -1)
		endFrame = 32767;

	if (_frame == -1) {
		_doPalSubst = 0;
		_palFadingRed = 0;
		_palFadingGreen = 0;
		_palFadingBlue = 0;

		_oldPalette = _vm->_global->_pPaletteDesc->vgaPal;
//		memcpy((char *)_palAnimPalette, (char *)_vm->_global->_pPaletteDesc->vgaPal, 768);

		if (_vm->_anim->_animSurf == 0) {
			_vm->_util->setFrameRate(_multData2->frameRate);
			_vm->_anim->_areaTop = 0;
			_vm->_anim->_areaLeft = 0;
			_vm->_anim->_areaWidth = 320;
			_vm->_anim->_areaHeight = 200;
			_objCount = 4;

			if (_objects)
				delete[] _objects;
			if (_orderArray)
				delete[] _orderArray;
/*			if (_renderData)
				delete[] _renderData;*/
			if (_renderData2)
				delete[] _renderData2;

			_objects = new Mult_Object[_objCount];
			memset(_objects, 0, _objCount * sizeof(Mult_Object));

			_orderArray = new int8[_objCount];
			memset(_orderArray, 0, _objCount * sizeof(int8));
/*			_renderData = new int16[9 * _objCount];
			memset(_renderData, 0, _objCount * 9 * sizeof(int16));*/
			_renderData2 = new Mult_Object*[_objCount];
			memset(_renderData2, 0, _objCount * sizeof(Mult_Object*));

			_animArrayX = new int32[_objCount];
			_animArrayY = new int32[_objCount];

			_animArrayData = new Mult_AnimData[_objCount];
			memset(_animArrayData, 0, _objCount * sizeof(Mult_AnimData));

			for (_counter = 0; _counter < _objCount; _counter++) {
				multObj = &_objects[_counter];

				multObj->pPosX = (int32 *)&_animArrayX[_counter];
				multObj->pPosY = (int32 *)&_animArrayY[_counter];

				multObj->pAnimData = &_animArrayData[_counter];

				animData = multObj->pAnimData;
				animData->isStatic = 1;

				multObj->tick = 0;
				multObj->lastLeft = -1;
				multObj->lastTop = -1;
				multObj->lastRight = -1;
				multObj->lastBottom = -1;
			}

			_vm->_draw->adjustCoords(0, &_vm->_anim->_areaWidth, &_vm->_anim->_areaHeight);

			if ((_vm->_global->_videoMode == 0x14) &&
				((_vm->_draw->_backSurface->width * _vm->_draw->_backSurface->height) / 2
					+ (_vm->_anim->_areaWidth * _vm->_anim->_areaHeight) / 4) < 64000) {
				_vm->_anim->_animSurf = new Video::SurfaceDesc;
				memcpy(_vm->_anim->_animSurf, _vm->_draw->_frontSurface, sizeof(Video::SurfaceDesc));
				_vm->_anim->_animSurf->width = (_vm->_anim->_areaLeft + _vm->_anim->_areaWidth - 1) | 7;
				_vm->_anim->_animSurf->width -= (_vm->_anim->_areaLeft & 0x0FFF8) - 1;
				_vm->_anim->_animSurf->height = _vm->_anim->_areaHeight;
				_vm->_anim->_animSurf->vidPtr +=
					(_vm->_draw->_backSurface->width * _vm->_draw->_backSurface->height) / 2;
			} else
				_vm->_draw->initBigSprite(22, _vm->_anim->_areaWidth, _vm->_anim->_areaHeight, 0);
		
			_vm->_draw->adjustCoords(1, &_vm->_anim->_areaWidth, &_vm->_anim->_areaHeight);
			_vm->_draw->_sourceSurface = 21;
			_vm->_draw->_destSurface = 22;
			_vm->_draw->_destSpriteX = 0;
			_vm->_draw->_destSpriteY = 0;
			_vm->_draw->_spriteLeft = 0;
			_vm->_draw->_spriteTop = 0;
			_vm->_draw->_spriteRight = 320;
			_vm->_draw->_spriteBottom = 200;
			_vm->_draw->_transparency = 0;
			_vm->_draw->spriteOperation(0);
			_animDataAllocated = 1;

			for (_counter = 0; _counter < _objCount; _counter++) {
				_multData2->palAnimIndices[_counter] = _counter;
			}

			_animDataAllocated = 1;
		} else
			_animDataAllocated = 0;
		_frame = 0;
	}

	do {
		_vm->_snd->loopSounds();
		stop = 1;

		if (VAR(58) == 0) {
			stop = drawStatics(stop);
			stop = drawAnims(stop);
		}

		animate();
		if (handleMouse) {
			_vm->_draw->animateCursor(-1);
		} else {
			_vm->_draw->blitInvalidated();
		}

		if (VAR(58) == 0) {
			drawText(&stop, &stopNoClear);
		}

		stop = prepPalAnim(stop);
		doPalAnim();

		stop = doFadeAnim(stop);
		stop = doSoundAnim(stop, _frame);

		if (_frame >= endFrame)
			stopNoClear = 1;

		if (_vm->_snd->_playingSound)
			stop = 0;

		_vm->_util->processInput();
		if (checkEscape && _vm->_util->checkKey() == 0x11b)	// Esc
			stop = 1;

		_frame++;
		_vm->_util->waitEndFrame();
	} while (stop == 0 && stopNoClear == 0 && !_vm->_quitRequested);

	if (stopNoClear == 0) {
		if (_animDataAllocated) {
			delete[] _objects;
			_objects = 0;

			delete[] _renderData2;
			_renderData2 = 0;

			delete[] _animArrayX;
			_animArrayX = 0;

			delete[] _animArrayY;
			_animArrayY = 0;

			delete[] _animArrayData;
			_animArrayData = 0;

			delete[] _orderArray;
			_orderArray = 0;

			if (_vm->_anim->_animSurf)
				_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);
			_vm->_anim->_animSurf = 0;

			_animDataAllocated = 0;
		}

		if (_vm->_snd->_playingSound != 0)
			_vm->_snd->stopSound(10);

		WRITE_VAR(57, (uint32)-1);
	} else {
		WRITE_VAR(57, _frame - 1 - _frameStart);
	}
}

char Mult_v2::drawStatics(char stop) {
	int i;

	if (_multData2->staticKeys[_multData2->staticKeysCount - 1].frame > _frame)
		stop = 0;

	for (_counter = 0; _counter < _multData2->staticKeysCount; _counter++) {
		if (_multData2->staticKeys[_counter].frame != _frame
		    || _multData2->staticKeys[_counter].layer == -1)
			continue;

		// loc_4FA8
		if (_multData2->staticKeys[_counter].layer >= 0) {
			_vm->_scenery->_curStatic = 0;
			_vm->_scenery->_curStaticLayer = _multData2->staticKeys[_counter].layer;
			
			i = 0;
			while (_vm->_scenery->_statics[_multData2->staticIndices[i]].layersCount <= _vm->_scenery->_curStaticLayer) {
				_vm->_scenery->_curStaticLayer -=
					_vm->_scenery->_statics[_multData2->staticIndices[i]].layersCount;
				i++;
				_vm->_scenery->_curStatic++;
			}
			_vm->_scenery->_curStatic = _multData2->staticIndices[_vm->_scenery->_curStatic];
			_vm->_scenery->renderStatic(_vm->_scenery->_curStatic, _vm->_scenery->_curStaticLayer);
		} else {
			_vm->_draw->_spriteLeft =
				READ_LE_UINT16(_multData2->execPtr + ((-_multData2->staticKeys[_counter].layer - 2) * 2));
			_vm->_draw->_destSpriteX = 0;
			_vm->_draw->_destSpriteY = 0;
			_vm->_draw->_destSurface = 21;
			_vm->_draw->_transparency = 0;
			_vm->_draw->spriteOperation(5);
			_vm->_scenery->_curStatic = -1;
		}
		_vm->_draw->_sourceSurface = 21;
		_vm->_draw->_destSurface = 22;
		_vm->_draw->_destSpriteX = 0;
		_vm->_draw->_destSpriteY = 0;
		_vm->_draw->_spriteLeft = 0;
		_vm->_draw->_spriteTop = 0;
		_vm->_draw->_spriteRight = 320;
		_vm->_draw->_spriteBottom = 200;
		_vm->_draw->_transparency = 0;
		_vm->_draw->spriteOperation(0);
	}
	return stop;
}

char Mult_v2::drawAnims(char stop) { // loc_50D5
	Mult_AnimKey *key;
	Mult_Object *animObj;
	int16 i;
	int16 count;
	
	for (i = 0; i < 4; i++) {
		if (_multData2->animKeys[i][_multData2->animKeysCount[i] - 1].frame > _frame)
			stop = 0;
	}

	for (_index = 0; _index < 4; _index++) {
		for (_counter = 0; _counter < _multData2->animKeysCount[_index]; _counter++) {
			key = &_multData2->animKeys[_index][_counter];
			animObj = &_objects[_multData2->field_124[0][_index]];
			if (key->frame != _frame)
				continue;
			
			if (key->layer != -1) {
				(*animObj->pPosX) = key->posX;
				(*animObj->pPosY) = key->posY;

				animObj->pAnimData->frame = 0;
				animObj->pAnimData->order = key->order;
				animObj->pAnimData->animType = 1;

				animObj->pAnimData->isPaused = 0;
				animObj->pAnimData->isStatic = 0;
				animObj->pAnimData->maxTick = 0;
				animObj->tick = 0;
				animObj->pAnimData->layer = key->layer;

				count = _vm->_scenery->_animations[_multData2->animIndices[0]].layersCount;
				i = 0;
				while (animObj->pAnimData->layer >= count) {
					animObj->pAnimData->layer -= count;
					i++;

					count = _vm->_scenery->_animations[_multData2->animIndices[i]].layersCount;
				}
				animObj->pAnimData->animation = _multData2->animIndices[i];
			} else {
				animObj->pAnimData->isStatic = 1;
			}
		}
	}

	return stop;
}

void Mult_v2::drawText(char *pStop, char *pStopNoClear) {
	char *savedIP;

	int16 cmd;
	for (_index = 0; _index < _multData2->textKeysCount; _index++) {
		if (_multData2->textKeys[_index].frame != _frame)
			continue;

		cmd = _multData2->textKeys[_index].cmd;
		if (cmd == 0) {
			*pStop = 0;
		} else if (cmd == 1) {
			*pStopNoClear = 1;
			_frameStart = 0;
		} else if (cmd == 3) {
			*pStop = 0;
			savedIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr = (char *)(&_multData2->textKeys[_index].index);
			_vm->_global->_inter_execPtr = savedIP;
		}
	}
}

char Mult_v2::prepPalAnim(char stop) {
	_palKeyIndex = -1;
	do {
		_palKeyIndex++;
		if (_palKeyIndex >= _multData2->palKeysCount)
			return stop;
	} while (_multData2->palKeys[_palKeyIndex].frame != _frame);

	if (_multData2->palKeys[_palKeyIndex].cmd == -1) {
		stop = 0;
		_doPalSubst = 0;
		_vm->_global->_pPaletteDesc->vgaPal = _oldPalette;

		memcpy((char *)_palAnimPalette, (char *)_vm->_global->_pPaletteDesc->vgaPal, 768);

		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	} else {
		stop = 0;
		_doPalSubst = 1;
		_palAnimKey = _palKeyIndex;

		_multData2->palAnimIndices[0] = 0;
		_multData2->palAnimIndices[1] = 0;
		_multData2->palAnimIndices[2] = 0;
		_multData2->palAnimIndices[3] = 0;

		_vm->_global->_pPaletteDesc->vgaPal = _palAnimPalette;
	}
	return stop;
}

void Mult_v2::doPalAnim(void) {
	int16 off;
	int16 off2;
	Video::Color *palPtr;
	Mult_PalKey *palKey;

	if (_doPalSubst == 0)
		return;

	for (_index = 0; _index < 4; _index++) {
		palKey = &_multData2->palKeys[_palAnimKey];

		if ((_frame % palKey->rates[_index]) != 0)
			continue;

		_palAnimRed[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].red;
		_palAnimGreen[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].green;
		_palAnimBlue[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].blue;

		while (1) {
			off = palKey->subst[(_multData2->palAnimIndices[_index] + 1) % 16][_index];
			if (off == 0) {
				off = palKey->subst[_multData2->palAnimIndices[_index]][_index] - 1;

				_vm->_global->_pPaletteDesc->vgaPal[off].red = _palAnimRed[_index];
				_vm->_global->_pPaletteDesc->vgaPal[off].green = _palAnimGreen[_index];
				_vm->_global->_pPaletteDesc->vgaPal[off].blue = _palAnimBlue[_index];
			} else {
				off = palKey->subst[(_multData2->palAnimIndices[_index] + 1) % 16][_index] - 1;
				off2 = palKey->subst[_multData2->palAnimIndices[_index]][_index] - 1;

				_vm->_global->_pPaletteDesc->vgaPal[off2].red =
					_vm->_global->_pPaletteDesc->vgaPal[off].red;
				_vm->_global->_pPaletteDesc->vgaPal[off2].green =
					_vm->_global->_pPaletteDesc->vgaPal[off].green;
				_vm->_global->_pPaletteDesc->vgaPal[off2].blue =
					_vm->_global->_pPaletteDesc->vgaPal[off].blue;
			}

			_multData2->palAnimIndices[_index] = (_multData2->palAnimIndices[_index] + 1) % 16;

			off = palKey->subst[_multData2->palAnimIndices[_index]][_index];

			if (off == 0) {
				_multData2->palAnimIndices[_index] = 0;
				off = palKey->subst[0][_index] - 1;

				_palAnimRed[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].red;
				_palAnimGreen[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].green;
				_palAnimBlue[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].blue;
			}
			if (_multData2->palAnimIndices[_index] == 0)
				break;
		}
	}

	if (_vm->_global->_colorCount == 256) {
		_vm->_video->waitRetrace(_vm->_global->_videoMode);

		palPtr = _vm->_global->_pPaletteDesc->vgaPal;
		for (_counter = 0; _counter < 16; _counter++) {
			_vm->_video->setPalElem(_counter, palPtr->red, palPtr->green, palPtr->blue, 0, 0x13);
			palPtr++;
		}

		palPtr = _vm->_global->_pPaletteDesc->vgaPal;
		for (_counter = 0; _counter < 16; _counter++) {
			_vm->_global->_redPalette[_counter] = palPtr->red;
			_vm->_global->_greenPalette[_counter] = palPtr->green;
			_vm->_global->_bluePalette[_counter] = palPtr->blue;
			palPtr++;
		}
	} else {
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	}
}

char Mult_v2::doFadeAnim(char stop) {
	Mult_PalFadeKey *fadeKey;

	for (_index = 0; _index < _multData2->palFadeKeysCount; _index++) {
		fadeKey = &_multData2->palFadeKeys[_index];

		if (fadeKey->frame != _frame)
			continue;

		stop = 0;
		if ((fadeKey->flag & 1) == 0) {
			if (fadeKey->fade == 0) {
				_vm->_global->_pPaletteDesc->vgaPal = _multData2->fadePal[fadeKey->palIndex];
				_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			} else {
				_vm->_global->_pPaletteDesc->vgaPal = _multData2->fadePal[fadeKey->palIndex];
				_vm->_palanim->fade(_vm->_global->_pPaletteDesc, fadeKey->fade, 0);
			}
		} else {
			_vm->_global->_pPaletteDesc->vgaPal = _multData2->fadePal[fadeKey->palIndex];
			_vm->_palanim->fade(_vm->_global->_pPaletteDesc, fadeKey->fade, -1);

			_palFadingRed = (fadeKey->flag >> 1) & 1;
			_palFadingGreen = (fadeKey->flag >> 2) & 1;
			_palFadingBlue = (fadeKey->flag >> 3) & 1;
		}
	}

	if (_palFadingRed) {
		_palFadingRed = !_vm->_palanim->fadeStep(1);
		stop = 0;
	}
	if (_palFadingGreen) {
		_palFadingGreen = !_vm->_palanim->fadeStep(2);
		stop = 0;
	}
	if (_palFadingBlue) {
		_palFadingBlue = !_vm->_palanim->fadeStep(3);
		stop = 0;
	}
	return stop;
}

char Mult_v2::doSoundAnim(char stop, int16 frame) {
	Mult_SndKey *sndKey;
	for (_index = 0; _index < _multData2->sndKeysCount; _index++) {
		sndKey = &_multData2->sndKeys[_index];
		if (sndKey->frame != frame)
			continue;

		if (sndKey->cmd != -1) {
			if ((sndKey->cmd == 1) || (sndKey->cmd == 4)) {
				_vm->_snd->stopSound(0);
				if (_vm->_game->_soundSamples[sndKey->soundIndex] == 0)
					continue;
				playSound(_vm->_game->_soundSamples[sndKey->soundIndex], sndKey->repCount,
				    sndKey->freq, sndKey->channel);
			}
		} else {
			if (_vm->_snd->_playingSound)
				_vm->_snd->stopSound(sndKey->channel);
		}
	}
	return stop;
}

// "deplaceheros"
void Mult_v2::sub_62DD(int16 index) {
	Mult_Object *animObj;
	Mult_AnimKey *animKey;
	Mult_SomeKey *someKey1, *someKey2;
	int16 frame;
	int16 layer;
	int16 layers;
	int16 curAnim;
	int i, j;
	
	frame = _multData2->animKeysFrames[index];
	if (frame == -1)
		return;

	for (i = 0; i < 4; i++) {
		if (_multData2->field_124[index][i] != -1) {
			for (j = _multData2->field_15F[index][i]; j < _multData2->animKeysCount[i]; j++) {
				if ((i >= 4) || (j >= _multData2->animKeysCount[i]))
					continue;
				animKey = &_multData2->animKeys[i][j];
				if (animKey->frame == frame) {
					animObj = &_objects[_multData2->field_124[index][i]];
					if (animKey->layer > -1) {
						_multData2->field_15F[index][i] = j;
						(*animObj->pPosX) = animKey->posX;
						(*animObj->pPosY) = animKey->posY;
						animObj->pAnimData->frame = 0;
						animObj->pAnimData->animType = 1;
						animObj->pAnimData->isStatic = 0;
						animObj->pAnimData->isPaused = 0;
						animObj->pAnimData->maxTick = 0;
						animObj->pAnimData->animation = 0;
						animObj->tick = 0;
						curAnim = _multData2->animIndices[0];
						layer = animKey->layer;
						layers = _vm->_scenery->_animations[curAnim].layersCount;
						while (layer >= layers) {
							layer -= layers;
							animObj->pAnimData->animation++;
							curAnim = _multData2->animIndices[animObj->pAnimData->animation];
							layers = _vm->_scenery->_animations[curAnim].layersCount;
						}
						animObj->pAnimData->layer = layer;
						animObj->pAnimData->animation =
							_multData2->animIndices[animObj->pAnimData->animation];
						break;
					} else
						animObj->pAnimData->isStatic = 1;
				} else if (animKey->frame > frame)
					break;
			}
		}

		if (_multData2->field_124[index][i] != -1) {
			for (j = _multData2->field_17F[index][i]; j < _multData2->someKeysCount[i]; j++) {
				someKey1 = &_multData2->someKeys[i][j];
				someKey2 = &_multData2->someKeys[i][j-1];
				if (someKey1->frame == frame) {
					if (someKey1->field_2 != -1) {
						_multData2->someKeysIndices[0] = -1;
						_multData2->someKeysIndices[1] = -1;
						_multData2->someKeysIndices[2] = -1;
						_multData2->someKeysIndices[3] = -1;
						if ((_multData2->field_156 == 1) || (someKey2->field_2 == 1))
							_multData2->someKeysIndices[i] = j;
						else if (_multData2->field_157[index] == frame)
							_multData2->someKeysIndices[i] = -1;
						else
							_multData2->someKeysIndices[i] = j - 1;
					} else
						_multData2->someKeysIndices[i] = -1;
				} else if(someKey1->frame > frame)
					break;
			}
		}
		if (_multData2->someKeysIndices[i] != -1) {
/*
			int arg3 = frame - _multData2->someKeys[i][_multData2->someKeysIndices[i]].field_0;
			int arg2 =  _multData2->field_156;
			if ((arg2 != 1) && (--arg3 > 0))
			arg3 = 0;
			int arg1 = _multData2->someKeys[i][_multData2->someKeysIndices[i]];
			// somepointer09 is 14 bytes wide (surely a struct)
			int arg0 = _multData2->somepointer09[-_multData2->someKeys[i][_multData2->someKeysIndices[i]].field_2 - 2];
*/
//			warning("GOB2 Stub! sub_1CBF8(arg0, arg1, arg2, arg3);");
		}
	}
	
	doSoundAnim(0, frame);
	WRITE_VAR(22, frame);

	if (_multData2->field_157[index] == frame) {
		_multData2->someKeysIndices[0] = -1;
		_multData2->someKeysIndices[1] = -1;
		_multData2->someKeysIndices[2] = -1;
		_multData2->someKeysIndices[3] = -1;
		frame = -1;
		for (i = 0; i < 4; i++)
			if ((_multData2->field_124[index][i] != -1) && (_multData2->field_124[index][i] != 1024))
				_objects[_multData2->field_124[index][i]].pAnimData->animType =
					_objects[_multData2->field_124[index][i]].pAnimData->field_17;
	} else if(_multData2->field_156 == 1)
		frame++;
	else
		frame--;

	// loc_6A06
	_multData2->animKeysFrames[index] = frame;
	WRITE_VAR(18 + index, frame);
}

// "avancerperso"
void Mult_v2::sub_6A35(void) {
	int i;
	int j;
	
	for (i = 0; i < 8; i++)
		if (_multDatas[i] != 0) {
			_multData2 = _multDatas[i];
			for (j = 0; j < 4; j++)
				sub_62DD(j);
		}
}

void Mult_v2::animate(void) {
	Mult_Object *animObj1, *animObj2;
	Mult_AnimData *animData1, *animData2;
	int i;
	int j;
	int8 minOrder = 100;
	int8 maxOrder = 0;
	int8 *orderArray;
	int orderArrayPos = 0;
	int8 animIndices[150];
	int numAnims = 0; // di
	
	if (_objects == 0)
		return;

	if (_objCount == 0)
		orderArray = 0;
	else {
		if (_orderArray == 0)
			return;
		orderArray = _orderArray;
	}

	sub_6A35();
	
	for (i = 0; i < _objCount; i++) {
		animData1 = _objects[i].pAnimData;
		animData1->intersected = 200;
		if ((animData1->isStatic != 2) &&
				((animData1->isStatic == 0) || (_objects[i].lastLeft != -1))) {
			animIndices[numAnims] = i;
			_renderData2[numAnims] = &_objects[i];
			numAnims++;
		}
	}

	for (i = 0; i < numAnims; i++) {
		animObj1 = _renderData2[i];

		animObj1->someFlag = 0;
		animObj1->somethingTop = 1000;
		animObj1->somethingLeft = 1000;
		animObj1->somethingBottom = 0;
		animObj1->somethingRight = 0;
		
		animData1 = animObj1->pAnimData;

		if (animData1->isStatic != 2) {
			if ((animData1->isStatic == 0) && (animData1->isPaused == 0)
					&& (animData1->maxTick == animObj1->tick)) {
				animObj1->someFlag = 1;
				_vm->_scenery->updateAnim(animData1->layer, animData1->frame,
						animData1->animation, 8, *animObj1->pPosX, *animObj1->pPosY, 0);
				if (animObj1->lastLeft == -1) {
					animObj1->somethingLeft = _vm->_scenery->_toRedrawLeft;
					animObj1->somethingTop = _vm->_scenery->_toRedrawTop;
					animObj1->somethingRight = _vm->_scenery->_toRedrawRight;
					animObj1->somethingBottom = _vm->_scenery->_toRedrawBottom;
				} else {
					animObj1->somethingLeft = MIN(animObj1->lastLeft, _vm->_scenery->_toRedrawLeft);
					animObj1->somethingTop = MIN(animObj1->lastTop, _vm->_scenery->_toRedrawTop);
					animObj1->somethingRight = MAX(animObj1->lastRight, _vm->_scenery->_toRedrawRight);
					animObj1->somethingBottom = MAX(animObj1->lastBottom, _vm->_scenery->_toRedrawBottom);
					if ((_vm->_game->_totFileData[0x29] > 50) &&
							(animObj1->somethingLeft == animObj1->lastLeft) &&
							(animObj1->somethingTop == animObj1->lastTop) &&
							(animObj1->somethingRight == animObj1->lastRight) &&
							(animObj1->somethingBottom == animObj1->lastBottom) &&
							(animData1->somethingLayer == animData1->layer) &&
							(animData1->somethingFrame == animData1->frame) &&
							(animData1->somethingAnimation == animData1->animation)) {
						animObj1->someFlag = 0;
					}
				}
			} else if (animData1->isStatic == 0) {
				if (animObj1->lastLeft == -1) {
					animObj1->someFlag = 1;
					_vm->_scenery->updateAnim(animData1->layer, animData1->frame,
						animData1->animation, 8, *animObj1->pPosX, *animObj1->pPosY, 0);
					animObj1->somethingLeft = _vm->_scenery->_toRedrawLeft;
					animObj1->somethingTop = _vm->_scenery->_toRedrawTop;
					animObj1->somethingRight = _vm->_scenery->_toRedrawRight;
					animObj1->somethingBottom = _vm->_scenery->_toRedrawBottom;
				} else {
					animObj1->somethingLeft = animObj1->lastLeft;
					animObj1->somethingTop = animObj1->lastTop;
					animObj1->somethingRight = animObj1->lastRight;
					animObj1->somethingBottom = animObj1->lastBottom;
				}
			} else if (animObj1->lastLeft != -1) {
				animObj1->someFlag = 1;
				animObj1->somethingLeft = animObj1->lastLeft;
				animObj1->somethingTop = animObj1->lastTop;
				animObj1->somethingRight = animObj1->lastRight;
				animObj1->somethingBottom = animObj1->lastBottom;
			}
			animData1->somethingLayer = animData1->layer;
			animData1->somethingFrame = animData1->frame;
			animData1->somethingAnimation = animData1->animation;
			if ((animObj1->someFlag != 0) || (animData1->isStatic == 0)) {
				minOrder = MIN(minOrder, animData1->order);
				maxOrder = MAX(maxOrder, animData1->order);
			}
		}
	}

	for (i = 0; i < numAnims; i++) {
		if ((_renderData2[i]->someFlag != 0) && (_renderData2[i]->lastLeft != -1)) {
			int maxleft = MAX(_renderData2[i]->somethingLeft, _vm->_anim->_areaLeft);
			int maxtop = MAX(_renderData2[i]->somethingTop, _vm->_anim->_areaTop);

			_vm->_draw->_sourceSurface = 22;
			_vm->_draw->_destSurface = 21;
			_vm->_draw->_spriteLeft = maxleft - _vm->_anim->_areaLeft;
			_vm->_draw->_spriteTop = maxtop - _vm->_anim->_areaTop;
			_vm->_draw->_spriteRight = _renderData2[i]->somethingRight - maxleft + 1;
			_vm->_draw->_spriteBottom = _renderData2[i]->somethingBottom - maxtop + 1;
			if ((_vm->_draw->_spriteRight > 0) && (_vm->_draw->_spriteBottom > 0)) {
				_vm->_draw->_destSpriteX = maxleft;
				_vm->_draw->_destSpriteY = maxtop;
				_vm->_draw->_transparency = 0;
				_vm->_draw->spriteOperation(10);
			}
			_renderData2[i]->lastLeft = -1;
		}
	}

	for (j = minOrder; j <= maxOrder; j++) {
		for (i = 0; i < numAnims; i++) {
			animData1 = _renderData2[i]->pAnimData;
			if (((animData1->isStatic == 0) || (_renderData2[i]->someFlag != 0))
					&& (animData1->order == j))
				orderArray[orderArrayPos++] = i;
		}
	}

	if (_vm->_goblin->_gobsCount >= 0) {
		for (i = 0; i < orderArrayPos; i++) {
			animObj1 = _renderData2[orderArray[i]];
			for (j = i+1; j < orderArrayPos; j++) {
				animObj2 = _renderData2[orderArray[j]];
				if ((animObj1->pAnimData->order == animObj2->pAnimData->order) &&
						((animObj1->somethingBottom > animObj2->somethingBottom) ||
						((animObj1->somethingBottom == animObj2->somethingBottom) &&
						 (animObj1->pAnimData->isBusy == 1))))
						SWAP(orderArray[i], orderArray[j]);
			}
		}
	}

	for (i = 0; i < orderArrayPos; i++) {
		animObj1 = _renderData2[orderArray[i]];
		animData1 = animObj1->pAnimData;
		if ((animObj1->someFlag == 0) && (animData1->isStatic == 0)) {
			for (j = 0; j < orderArrayPos; j++) {
				animObj2 = _renderData2[orderArray[j]];
				if ((animObj2->someFlag != 0) &&
						(animObj1->somethingRight >= animObj2->somethingLeft) &&
						(animObj2->somethingRight >= animObj1->somethingLeft) &&
						(animObj1->somethingBottom >= animObj2->somethingTop) &&
						(animObj2->somethingBottom >= animObj1->somethingTop))
				{
					_vm->_scenery->_toRedrawLeft = animObj2->somethingLeft;
					_vm->_scenery->_toRedrawRight = animObj2->somethingRight;
					_vm->_scenery->_toRedrawTop = animObj2->somethingTop;
					_vm->_scenery->_toRedrawBottom = animObj2->somethingBottom;
					_vm->_scenery->updateAnim(animData1->layer, animData1->frame,
							animData1->animation, 12, *animObj1->pPosX, *animObj1->pPosY, 1);
					_vm->_scenery->updateStatic(animObj1->pAnimData->order + 1);
				}
			}
		} else if (animData1->isStatic == 0) {
			_vm->_scenery->updateAnim(animData1->layer, animData1->frame,
					animData1->animation, 10, *animObj1->pPosX, *animObj1->pPosY, 1);
			if (_vm->_scenery->_toRedrawLeft != -12345) {
				if (_vm->_global->_pressedKeys[0x36]) {
//					warning("GOB2 Stub! word_2F3BF & word_2F3C1; someValueToAddToY & someValueToAddToX, respectively");
					// draws a rectangle around the region to redraw, why?
					_vm->_video->drawLine(_vm->_draw->_frontSurface,
							_vm->_scenery->_toRedrawLeft, _vm->_scenery->_toRedrawTop,
							_vm->_scenery->_toRedrawRight, _vm->_scenery->_toRedrawTop, 15);
					_vm->_video->drawLine(_vm->_draw->_frontSurface,
							_vm->_scenery->_toRedrawLeft, _vm->_scenery->_toRedrawBottom,
							_vm->_scenery->_toRedrawRight, _vm->_scenery->_toRedrawBottom, 15);
					_vm->_video->drawLine(_vm->_draw->_frontSurface,
							_vm->_scenery->_toRedrawLeft, _vm->_scenery->_toRedrawTop,
							_vm->_scenery->_toRedrawLeft, _vm->_scenery->_toRedrawBottom, 15);
					_vm->_video->drawLine(_vm->_draw->_frontSurface,
							_vm->_scenery->_toRedrawRight, _vm->_scenery->_toRedrawTop,
							_vm->_scenery->_toRedrawRight, _vm->_scenery->_toRedrawBottom, 15);
				}
				animObj1->lastLeft = _vm->_scenery->_toRedrawLeft;
				animObj1->lastRight = _vm->_scenery->_toRedrawRight;
				animObj1->lastTop = _vm->_scenery->_toRedrawTop;
				animObj1->lastBottom = _vm->_scenery->_toRedrawBottom;
			} else
				animObj1->lastLeft = -1;
		} else {
			_vm->_scenery->_toRedrawLeft = animObj1->somethingLeft;
			_vm->_scenery->_toRedrawRight = animObj1->somethingRight;
			_vm->_scenery->_toRedrawTop = animObj1->somethingTop;
			_vm->_scenery->_toRedrawBottom = animObj1->somethingBottom;
		}
		_vm->_scenery->updateStatic(animObj1->pAnimData->order + 1);
	}

	for (i = 0; i < numAnims; i++) {
		animObj1 = _renderData2[i];
		animData1 = animObj1->pAnimData;
		if (animData1->isStatic != 0)
			continue;

		if ((animData1->animType == 7) && (animData1->field_F != -1)) {
			animData1->layer = animData1->field_F;
			animData1->frame = 0;
			animData1->field_F = -1;
			animData1->isPaused = 0;
		}
		if (animData1->isPaused != 0)
			continue;

		if (animData1->maxTick == animObj1->tick) {
			animObj1->tick = 0;
			if ((animData1->animType < 100) || (_vm->_goblin->_gobsCount < 0)) {
				if (animData1->animType == 4) {
					animData1->frame = 0;
					animData1->isPaused = 1;
				}
				else {
					if (animData1->animType != 8)
						animData1->frame++;
					if (animData1->frame >=
							_vm->_scenery->_animations[(int)animData1->animation].layers[animData1->layer].framesCount) {
						switch (animData1->animType) {
						case 0:
							animData1->frame = 0;
							break;

						case 1:
							animData1->frame = 0;
							*(_objects[i].pPosX) +=
								_vm->_scenery->_animations[(int)animData1->animation].layers[animData1->layer].animDeltaX;
							*(_objects[i].pPosY) +=
								_vm->_scenery->_animations[(int)animData1->animation].layers[animData1->layer].animDeltaY;
							break;

						case 2:
							animData1->frame = 0;
							animData1->animation = animData1->newAnimation;
							animData1->layer = animData1->newLayer;
							break;

						case 3:
							animData1->animType = 4;
							animData1->frame = 0;
							break;

						case 5:
							animData1->isStatic = 1;
							animData1->frame = 0;
							break;

						case 6:
						case 7:
							animData1->frame--;
							animData1->isPaused = 1;
							break;
						}
						animData1->newCycle = 1;
					} else
						animData1->newCycle = 0;
				}
			}
			else if (animData1->animType == 100)
				_vm->_goblin->moveAdvance(animObj1, 0, 0, 0);
			else if (animData1->animType == 101)
				_vm->_goblin->sub_11984(animObj1);
		} else
			animObj1->tick++;
	}

	for (i = 0; i < numAnims; i++) {
		animObj1 = _renderData2[i];
		animData1 = animObj1->pAnimData;
		if ((animData1->isStatic == 0) && (animObj1->lastLeft != -1))
			for (j = 0; j < numAnims; j++) {
				animObj2 = _renderData2[j];
				animData2 = animObj2->pAnimData;
				if ((i != j) && (animData2->isStatic == 0) && (animObj2->lastLeft != -1))
					if ((animObj2->lastRight >= animObj1->lastLeft) &&
							(animObj2->lastLeft <= animObj1->lastRight) &&
							(animObj2->lastBottom >= animObj1->lastTop) &&
							(animObj2->lastTop <= animObj1->lastBottom))
						animData2->intersected = animIndices[i];
			}
	}

}

void Mult_v2::playSound(Snd::SoundDesc * soundDesc, int16 repCount, int16 freq,
	    int16 channel) {
	if (soundDesc->frequency >= 0) {
		if (soundDesc->frequency == freq)
			_vm->_snd->playSample(soundDesc, repCount, -channel);
		else
			_vm->_snd->playSample(soundDesc, repCount, freq);
	} else {
		if (soundDesc->frequency == -freq)
			_vm->_snd->playSample(soundDesc, repCount, -channel);
		else
			_vm->_snd->playSample(soundDesc, repCount, freq);
	}
}

void Mult_v2::freeMult(void) {
	if (_vm->_anim->_animSurf != 0)
		delete _vm->_anim->_animSurf;

	delete[] _objects;
	delete[] _renderData2;
	delete[] _orderArray;

	_objects = 0;
	_renderData2 = 0;
	_orderArray = 0;
	_vm->_anim->_animSurf = 0;
}

void Mult_v2::freeMultKeys(void) {
	int i;
	char animCount;
	char staticCount;

	if (_multData2 == 0)
		return;

	staticCount = (_multData2->staticCount + 1) && 0x7F;
	animCount = _multData2->animCount + 1;

	for (i = 0; i < staticCount; i++) { // loc_7345
		if (_multData2->staticLoaded[i] != 0)
			_vm->_scenery->freeStatic(_multData2->staticIndices[i]);
	}

	for (i = 0; i < animCount; i++) { // loc_7377
		if (_multData2->animLoaded[i] != 0)
			_vm->_scenery->freeAnim(_multData2->animIndices[i]);
	}

	delete[] _multData2->staticKeys;

	for (i = 0; i < 4; i++) { // loc_73BA
		delete[] _multData2->animKeys[i];
		if (_multData2->someKeys[i] != 0)
			delete[] _multData2->someKeys[i];
	}

	delete[] _multData2->palFadeKeys;
	delete[] _multData2->palKeys;
	delete[] _multData2->textKeys;

	for (i = 0; i < _multData2->sndSlotsCount; i++) { // loc_7448
		if ((_multData2->sndSlot[i] & 0x8000) == 0)
			_vm->_game->freeSoundSlot(_multData2->sndSlot[i]);
	}
	
	delete[] _multData2->sndKeys;

	if (_multData2->somepointer09 != 0)
		delete[] _multData2->somepointer09;
	if (_multData2->somepointer10 != 0)
		delete[] _multData2->somepointer10;

	if (_animDataAllocated != 0) {
		freeMult();
		
		delete[] _animArrayX;
		_animArrayX = 0;

		delete[] _animArrayY;
		_animArrayY = 0;

		delete[] _animArrayData;
		_animArrayData = 0;

		_animDataAllocated = 0;
	}

	for (i = 0; i < 8; i++)
		if (_multDatas[i] == _multData2)
			_multDatas[i] = 0;

	delete _multData2;
	_multData2 = 0;
}

} // End of namespace Gob
