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

#include "gob/gob.h"
#include "gob/mult.h"
#include "gob/game.h"
#include "gob/scenery.h"
#include "gob/global.h"
#include "gob/inter.h"
#include "gob/anim.h"
#include "gob/draw.h"
#include "gob/palanim.h"

namespace Gob {

Mult_v1::Mult_v1(GobEngine *vm) : Mult(vm) {
}

void Mult_v1::loadMult(int16 resId) {
	char animCount;
	char staticCount;
	int16 palIndex;
	int16 i, j;

	_sndSlotsCount = 0;
	_frameStart = 0;
	_multData = _vm->_game->loadExtData(resId, 0, 0);
	_dataPtr = _multData;

	staticCount = _dataPtr[0];
	animCount = _dataPtr[1];
	_dataPtr += 2;
	staticCount++;
	animCount++;

	for (i = 0; i < staticCount; i++, _dataPtr += 14) {
		_staticIndices[i] = _vm->_scenery->loadStatic(1);

		if (_staticIndices[i] >= 100) {
			_staticIndices[i] -= 100;
			_staticLoaded[i] = 1;
		} else {
			_staticLoaded[i] = 0;
		}
	}

	for (i = 0; i < animCount; i++, _dataPtr += 14) {
		_animIndices[i] = _vm->_scenery->loadAnim(1);

		if (_animIndices[i] >= 100) {
			_animIndices[i] -= 100;
			_animLoaded[i] = 1;
		} else {
			_animLoaded[i] = 0;
		}
	}

	_frameRate = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_staticKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_staticKeys = new Mult_StaticKey[_staticKeysCount];
	for (i = 0; i < _staticKeysCount; i++, _dataPtr += 4) {
		_staticKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_staticKeys[i].layer = (int16)READ_LE_UINT16(_dataPtr + 2);
	}

	for (j = 0; j < 4; j++) {
		_animKeysCount[j] = READ_LE_UINT16(_dataPtr);
		_dataPtr += 2;

		_animKeys[j] = new Mult_AnimKey[_animKeysCount[j]];
		for (i = 0; i < _animKeysCount[j]; i++, _dataPtr += 10) {
			_animKeys[j][i].frame = (int16)READ_LE_UINT16(_dataPtr);
			_animKeys[j][i].layer = (int16)READ_LE_UINT16(_dataPtr + 2);
			_animKeys[j][i].posX = (int16)READ_LE_UINT16(_dataPtr + 4);
			_animKeys[j][i].posY = (int16)READ_LE_UINT16(_dataPtr + 6);
			_animKeys[j][i].order = (int16)READ_LE_UINT16(_dataPtr + 8);
		}
	}

	for (palIndex = 0; palIndex < 5; palIndex++) {
		for (i = 0; i < 16; i++) {
			_fadePal[palIndex][i].red = _dataPtr[0];
			_fadePal[palIndex][i].green = _dataPtr[1];
			_fadePal[palIndex][i].blue = _dataPtr[2];
			_dataPtr += 3;
		}
	}

	_palFadeKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;
	_palFadeKeys = new Mult_PalFadeKey[_palFadeKeysCount];

	for (i = 0; i < _palFadeKeysCount; i++, _dataPtr += 7) {
		_palFadeKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_palFadeKeys[i].fade = (int16)READ_LE_UINT16(_dataPtr + 2);
		_palFadeKeys[i].palIndex = (int16)READ_LE_UINT16(_dataPtr + 4);
		_palFadeKeys[i].flag = *(_dataPtr + 6);
	}

	_palKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_palKeys = new Mult_PalKey[_palKeysCount];
	for (i = 0; i < _palKeysCount; i++, _dataPtr += 80) {
		_palKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_palKeys[i].cmd = (int16)READ_LE_UINT16(_dataPtr + 2);
		_palKeys[i].rates[0] = (int16)READ_LE_UINT16(_dataPtr + 4);
		_palKeys[i].rates[1] = (int16)READ_LE_UINT16(_dataPtr + 6);
		_palKeys[i].rates[2] = (int16)READ_LE_UINT16(_dataPtr + 8);
		_palKeys[i].rates[3] = (int16)READ_LE_UINT16(_dataPtr + 10);
		_palKeys[i].unknown0 = (int16)READ_LE_UINT16(_dataPtr + 12);
		_palKeys[i].unknown1 = (int16)READ_LE_UINT16(_dataPtr + 14);
		memcpy(_palKeys[i].subst, _dataPtr + 16, 64);
	}

	_textKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;
	_textKeys = new Mult_TextKey[_textKeysCount];

	for (i = 0; i < _textKeysCount; i++, _dataPtr += 28) {
		_textKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_textKeys[i].cmd = (int16)READ_LE_UINT16(_dataPtr + 2);
		for (int k = 0; k < 9; ++k)
			_textKeys[i].unknown0[k] = (int16)READ_LE_UINT16(_dataPtr + 4 + (k * 2));
		_textKeys[i].index = (int16)READ_LE_UINT16(_dataPtr + 22);
		_textKeys[i].unknown1[0] = (int16)READ_LE_UINT16(_dataPtr + 24);
		_textKeys[i].unknown1[1] = (int16)READ_LE_UINT16(_dataPtr + 26);
	}

	_sndKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_sndKeys = new Mult_SndKey[_sndKeysCount];
	for (i = 0; i < _sndKeysCount; i++) {
		_sndKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_sndKeys[i].cmd = (int16)READ_LE_UINT16(_dataPtr + 2);
		_sndKeys[i].freq = (int16)READ_LE_UINT16(_dataPtr + 4);
		_sndKeys[i].channel = (int16)READ_LE_UINT16(_dataPtr + 6);
		_sndKeys[i].repCount = (int16)READ_LE_UINT16(_dataPtr + 8);
		_sndKeys[i].resId = (int16)READ_LE_UINT16(_dataPtr + 10);
		_sndKeys[i].soundIndex = (int16)READ_LE_UINT16(_dataPtr + 12);

		_sndKeys[i].soundIndex = -1;
		_sndKeys[i].resId = -1;
		_dataPtr += 36;
		switch (_sndKeys[i].cmd) {
		case 1:
		case 4:
			_sndKeys[i].resId = READ_LE_UINT16(_vm->_global->_inter_execPtr);

			for (j = 0; j < i; j++) {
				if (_sndKeys[i].resId ==
				    _sndKeys[j].resId) {
					_sndKeys[i].soundIndex =
					    _sndKeys[j].soundIndex;
					_vm->_global->_inter_execPtr += 2;
					break;
				}
			}
			if (i == j) {
				_vm->_inter->loadSound(19 - _sndSlotsCount);
				_sndKeys[i].soundIndex =
				    19 - _sndSlotsCount;
				_sndSlotsCount++;
			}
			break;

		case 3:
			_vm->_global->_inter_execPtr += 6;
			break;

		case 5:
			_vm->_global->_inter_execPtr += _sndKeys[i].freq * 2;
			break;
		}
	}
}

void Mult_v1::setMultData(uint16 multindex) {
	error("Switching mults not supported for Gob1");
}

void Mult_v1::multSub(uint16 multindex) {
	error("Switching mults not supported for Gob1");
}

void Mult_v1::playMult(int16 startFrame, int16 endFrame, char checkEscape,
	    char handleMouse) {
	char stopNoClear;
	char stop;
	Mult_Object *multObj;
	Mult_AnimData *animData;

	if (_multData == 0)
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
		memcpy((char *)_palAnimPalette, (char *)_vm->_global->_pPaletteDesc->vgaPal, 768);

		if (_vm->_anim->_animSurf == 0) {
			_vm->_util->setFrameRate(_frameRate);
			_vm->_anim->_areaTop = 0;
			_vm->_anim->_areaLeft = 0;
			_vm->_anim->_areaWidth = 320;
			_vm->_anim->_areaHeight = 200;
			_objCount = 4;

			_objects = new Mult_Object[_objCount];
			memset(_objects, 0, _objCount * sizeof(Mult_Object));
			_renderData = new int16[9 * _objCount];
			memset(_renderData, 0, _objCount * 9 * sizeof(int16));

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

			_vm->_anim->_animSurf =
			    _vm->_video->initSurfDesc(_vm->_global->_videoMode, 320, 200, 0);
			_vm->_draw->_spritesArray[22] = _vm->_anim->_animSurf;

			_vm->_video->drawSprite(_vm->_draw->_backSurface, _vm->_anim->_animSurf,
			    0, 0, 319, 199, 0, 0, 0);

			_animDataAllocated = 1;
		} else
			_animDataAllocated = 0;
		_frame = 0;
	}

	do {
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

			delete[] _renderData;
			_renderData = 0;

			delete[] _animArrayX;
			_animArrayX = 0;

			delete[] _animArrayY;
			_animArrayY = 0;

			delete[] _animArrayData;
			_animArrayData = 0;

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

char Mult_v1::drawStatics(char stop) {
	if (_staticKeys[_staticKeysCount - 1].frame > _frame)
		stop = 0;

	for (_counter = 0; _counter < _staticKeysCount;
	    _counter++) {
		if (_staticKeys[_counter].frame != _frame
		    || _staticKeys[_counter].layer == -1)
			continue;

		for (_vm->_scenery->_curStatic = 0, _vm->_scenery->_curStaticLayer = _staticKeys[_counter].layer;
			 _vm->_scenery->_curStaticLayer >= _vm->_scenery->_statics[_staticIndices[_vm->_scenery->_curStatic]].layersCount;
			 _vm->_scenery->_curStatic++) {
			_vm->_scenery->_curStaticLayer -=
			    _vm->_scenery->_statics[_staticIndices[_vm->_scenery->_curStatic]].layersCount;
		}

		_vm->_scenery->_curStatic = _staticIndices[_vm->_scenery->_curStatic];
		_vm->_scenery->renderStatic(_vm->_scenery->_curStatic, _vm->_scenery->_curStaticLayer);
		_vm->_video->drawSprite(_vm->_draw->_backSurface, _vm->_anim->_animSurf,
		    0, 0, 319, 199, 0, 0, 0);
	}
	return stop;
}

char Mult_v1::drawAnims(char stop) {
	Mult_AnimKey *key;
	Mult_Object *animObj;
	int16 i;
	int16 count;

	for (_index = 0; _index < 4; _index++) {
		for (_counter = 0; _counter < _animKeysCount[_index]; _counter++) {
			key = &_animKeys[_index][_counter];
			animObj = &_objects[_index];
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

				count = _vm->_scenery->_animations[_animIndices[0]].layersCount;
				i = 0;
				while (animObj->pAnimData->layer >= count) {
					animObj->pAnimData->layer -= count;
					i++;

					count = _vm->_scenery->_animations[_animIndices[i]].layersCount;
				}
				animObj->pAnimData->animation = _animIndices[i];
			} else {
				animObj->pAnimData->isStatic = 1;
			}
		}
	}
	return stop;
}

void Mult_v1::drawText(char *pStop, char *pStopNoClear) {
	char *savedIP;

	int16 cmd;
	for (_index = 0; _index < _textKeysCount; _index++) {
		if (_textKeys[_index].frame != _frame)
			continue;

		cmd = _textKeys[_index].cmd;
		if (cmd == 0) {
			*pStop = 0;
		} else if (cmd == 1) {
			*pStopNoClear = 1;
			_frameStart = 0;
		} else if (cmd == 3) {
			*pStop = 0;
			savedIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr = (char *)(&_textKeys[_index].index);
			_vm->_global->_inter_execPtr = savedIP;
		}
	}
}

char Mult_v1::prepPalAnim(char stop) {
	_palKeyIndex = -1;
	do {
		_palKeyIndex++;
		if (_palKeyIndex >= _palKeysCount)
			return stop;
	} while (_palKeys[_palKeyIndex].frame != _frame);

	if (_palKeys[_palKeyIndex].cmd == -1) {
		stop = 0;
		_doPalSubst = 0;
		_vm->_global->_pPaletteDesc->vgaPal = _oldPalette;

		memcpy((char *)_palAnimPalette, (char *)_vm->_global->_pPaletteDesc->vgaPal, 768);

		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	} else {
		stop = 0;
		_doPalSubst = 1;
		_palAnimKey = _palKeyIndex;

		_palAnimIndices[0] = 0;
		_palAnimIndices[1] = 0;
		_palAnimIndices[2] = 0;
		_palAnimIndices[3] = 0;

		_vm->_global->_pPaletteDesc->vgaPal = _palAnimPalette;
	}
	return stop;
}

void Mult_v1::doPalAnim(void) {
	int16 off;
	int16 off2;
	Video::Color *palPtr;
	Mult_PalKey *palKey;

	if (_doPalSubst == 0)
		return;

	for (_index = 0; _index < 4; _index++) {
		palKey = &_palKeys[_palAnimKey];

		if ((_frame % palKey->rates[_index]) != 0)
			continue;

		_palAnimRed[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].red;
		_palAnimGreen[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].green;
		_palAnimBlue[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].blue;

		while (1) {
			off = palKey->subst[(_palAnimIndices[_index] + 1) % 16][_index];
			if (off == 0) {
				off = palKey->subst[_palAnimIndices[_index]][_index] - 1;

				_vm->_global->_pPaletteDesc->vgaPal[off].red = _palAnimRed[_index];
				_vm->_global->_pPaletteDesc->vgaPal[off].green = _palAnimGreen[_index];
				_vm->_global->_pPaletteDesc->vgaPal[off].blue = _palAnimBlue[_index];
			} else {
				off = palKey->subst[(_palAnimIndices[_index] + 1) % 16][_index] - 1;
				off2 = palKey->subst[_palAnimIndices[_index]][_index] - 1;

				_vm->_global->_pPaletteDesc->vgaPal[off2].red = _vm->_global->_pPaletteDesc->vgaPal[off].red;
				_vm->_global->_pPaletteDesc->vgaPal[off2].green = _vm->_global->_pPaletteDesc->vgaPal[off].green;
				_vm->_global->_pPaletteDesc->vgaPal[off2].blue = _vm->_global->_pPaletteDesc->vgaPal[off].blue;
			}

			_palAnimIndices[_index] = (_palAnimIndices[_index] + 1) % 16;

			off = palKey->subst[_palAnimIndices[_index]][_index];

			if (off == 0) {
				_palAnimIndices[_index] = 0;
				off = palKey->subst[0][_index] - 1;

				_palAnimRed[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].red;
				_palAnimGreen[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].green;
				_palAnimBlue[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].blue;
			}
			if (_palAnimIndices[_index] == 0)
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

char Mult_v1::doFadeAnim(char stop) {
	Mult_PalFadeKey *fadeKey;

	for (_index = 0; _index < _palFadeKeysCount; _index++) {
		fadeKey = &_palFadeKeys[_index];

		if (fadeKey->frame != _frame)
			continue;

		stop = 0;
		if ((fadeKey->flag & 1) == 0) {
			if (fadeKey->fade == 0) {
				_vm->_global->_pPaletteDesc->vgaPal = _fadePal[fadeKey->palIndex];
				_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			} else {
				_vm->_global->_pPaletteDesc->vgaPal = _fadePal[fadeKey->palIndex];
				_vm->_palanim->fade(_vm->_global->_pPaletteDesc, fadeKey->fade, 0);
			}
		} else {
			_vm->_global->_pPaletteDesc->vgaPal = _fadePal[fadeKey->palIndex];
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

char Mult_v1::doSoundAnim(char stop, int16 frame) {
	Mult_SndKey *sndKey;
	for (_index = 0; _index < _sndKeysCount; _index++) {
		sndKey = &_sndKeys[_index];
		if (sndKey->frame != frame)
			continue;

		if (sndKey->cmd != -1) {
			if (sndKey->cmd == 1) {
				_vm->_snd->stopSound(0);
				stop = 0;
				playSound(_vm->_game->_soundSamples[sndKey->soundIndex], sndKey->repCount,
				    sndKey->freq, sndKey->channel);

			} else if (sndKey->cmd == 4) {
				_vm->_snd->stopSound(0);
				stop = 0;
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

void Mult_v1::animate(void) {
	int16 minOrder;
	int16 maxOrder;
	int16 *pCurLefts;
	int16 *pCurRights;
	int16 *pCurTops;
	int16 *pCurBottoms;
	int16 *pDirtyLefts;
	int16 *pDirtyRights;
	int16 *pDirtyTops;
	int16 *pDirtyBottoms;
	int16 *pNeedRedraw;
	Mult_AnimData *pAnimData;
	int16 i, j;
	int16 order;

	if (_renderData == 0)
		return;

	pDirtyLefts = _renderData;
	pDirtyRights = pDirtyLefts + _objCount;
	pDirtyTops = pDirtyRights + _objCount;
	pDirtyBottoms = pDirtyTops + _objCount;
	pNeedRedraw = pDirtyBottoms + _objCount;
	pCurLefts = pNeedRedraw + _objCount;
	pCurRights = pCurLefts + _objCount;
	pCurTops = pCurRights + _objCount;
	pCurBottoms = pCurTops + _objCount;
	minOrder = 100;
	maxOrder = 0;

	//Find dirty areas
	for (i = 0; i < _objCount; i++) {
		pNeedRedraw[i] = 0;
		pDirtyTops[i] = 1000;
		pDirtyLefts[i] = 1000;
		pDirtyBottoms[i] = 1000;
		pDirtyRights[i] = 1000;
		pAnimData = _objects[i].pAnimData;

		if (pAnimData->isStatic == 0 && pAnimData->isPaused == 0 &&
		    _objects[i].tick == pAnimData->maxTick) {
			if (pAnimData->order < minOrder)
				minOrder = pAnimData->order;

			if (pAnimData->order > maxOrder)
				maxOrder = pAnimData->order;

			pNeedRedraw[i] = 1;
			_vm->_scenery->updateAnim(pAnimData->layer, pAnimData->frame,
			    pAnimData->animation, 0,
			    *(_objects[i].pPosX), *(_objects[i].pPosY),
			    0);

			if (_objects[i].lastLeft != -1) {
				pDirtyLefts[i] =
				    MIN(_objects[i].lastLeft,
				    _vm->_scenery->_toRedrawLeft);
				pDirtyTops[i] =
				    MIN(_objects[i].lastTop,
				    _vm->_scenery->_toRedrawTop);
				pDirtyRights[i] =
				    MAX(_objects[i].lastRight,
				    _vm->_scenery->_toRedrawRight);
				pDirtyBottoms[i] =
				    MAX(_objects[i].lastBottom,
				    _vm->_scenery->_toRedrawBottom);
			} else {
				pDirtyLefts[i] = _vm->_scenery->_toRedrawLeft;
				pDirtyTops[i] = _vm->_scenery->_toRedrawTop;
				pDirtyRights[i] = _vm->_scenery->_toRedrawRight;
				pDirtyBottoms[i] = _vm->_scenery->_toRedrawBottom;
			}
			pCurLefts[i] = _vm->_scenery->_toRedrawLeft;
			pCurRights[i] = _vm->_scenery->_toRedrawRight;
			pCurTops[i] = _vm->_scenery->_toRedrawTop;
			pCurBottoms[i] = _vm->_scenery->_toRedrawBottom;
		} else {
			if (_objects[i].lastLeft != -1) {
				if (pAnimData->order < minOrder)
					minOrder = pAnimData->order;

				if (pAnimData->order > maxOrder)
					maxOrder = pAnimData->order;

				if (pAnimData->isStatic)
					*pNeedRedraw = 1;

				pCurLefts[i] = _objects[i].lastLeft;
				pDirtyLefts[i] = _objects[i].lastLeft;

				pCurTops[i] = _objects[i].lastTop;
				pDirtyTops[i] = _objects[i].lastTop;

				pCurRights[i] = _objects[i].lastRight;
				pDirtyRights[i] = _objects[i].lastRight;

				pCurBottoms[i] = _objects[i].lastBottom;
				pDirtyBottoms[i] = _objects[i].lastBottom;
			}
		}
	}

	// Find intersections
	for (i = 0; i < _objCount; i++) {
		pAnimData = _objects[i].pAnimData;
		pAnimData->intersected = 200;

		if (pAnimData->isStatic)
			continue;

		for (j = 0; j < _objCount; j++) {
			if (i == j)
				continue;

			if (_objects[j].pAnimData->isStatic)
				continue;

			if (pCurRights[i] < pCurLefts[j])
				continue;

			if (pCurRights[j] < pCurLefts[i])
				continue;

			if (pCurBottoms[i] < pCurTops[j])
				continue;

			if (pCurBottoms[j] < pCurTops[i])
				continue;

			pAnimData->intersected = j;
			break;
		}
	}

	// Restore dirty areas
	for (i = 0; i < _objCount; i++) {

		if (pNeedRedraw[i] == 0 || _objects[i].lastLeft == -1)
			continue;

		_vm->_draw->_sourceSurface = 22;
		_vm->_draw->_destSurface = 21;
		_vm->_draw->_spriteLeft = pDirtyLefts[i] - _vm->_anim->_areaLeft;
		_vm->_draw->_spriteTop = pDirtyTops[i] - _vm->_anim->_areaTop;
		_vm->_draw->_spriteRight = pDirtyRights[i] - pDirtyLefts[i] + 1;
		_vm->_draw->_spriteBottom = pDirtyBottoms[i] - pDirtyTops[i] + 1;
		_vm->_draw->_destSpriteX = pDirtyLefts[i];
		_vm->_draw->_destSpriteY = pDirtyTops[i];
		_vm->_draw->_transparency = 0;
		_vm->_draw->spriteOperation(DRAW_BLITSURF);
		_objects[i].lastLeft = -1;
	}

	// Update view
	for (order = minOrder; order <= maxOrder; order++) {
		for (i = 0; i < _objCount; i++) {
			pAnimData = _objects[i].pAnimData;
			if (pAnimData->order != order)
				continue;

			if (pNeedRedraw[i]) {
				if (pAnimData->isStatic == 0) {

					_vm->_scenery->updateAnim(pAnimData->layer,
					    pAnimData->frame,
					    pAnimData->animation, 2,
					    *(_objects[i].pPosX),
					    *(_objects[i].pPosY), 1);

					if (_vm->_scenery->_toRedrawLeft != -12345) {
						_objects[i].lastLeft =
						    _vm->_scenery->_toRedrawLeft;
						_objects[i].lastTop =
						    _vm->_scenery->_toRedrawTop;
						_objects[i].lastRight =
						    _vm->_scenery->_toRedrawRight;
						_objects[i].lastBottom =
						    _vm->_scenery->_toRedrawBottom;
					} else {
						_objects[i].lastLeft = -1;
					}
				}
				_vm->_scenery->updateStatic(order + 1);
			} else if (pAnimData->isStatic == 0) {
				for (j = 0; j < _objCount; j++) {
					if (pNeedRedraw[j] == 0)
						continue;

					if (pDirtyRights[i] < pDirtyLefts[j])
						continue;

					if (pDirtyRights[j] < pDirtyLefts[i])
						continue;

					if (pDirtyBottoms[i] < pDirtyTops[j])
						continue;

					if (pDirtyBottoms[j] < pDirtyTops[i])
						continue;

					_vm->_scenery->_toRedrawLeft = pDirtyLefts[j];
					_vm->_scenery->_toRedrawRight = pDirtyRights[j];
					_vm->_scenery->_toRedrawTop = pDirtyTops[j];
					_vm->_scenery->_toRedrawBottom = pDirtyBottoms[j];

					_vm->_scenery->updateAnim(pAnimData->layer,
					    pAnimData->frame,
					    pAnimData->animation, 4,
					    *(_objects[i].pPosX),
					    *(_objects[i].pPosY), 1);

					_vm->_scenery->updateStatic(order + 1);
				}
			}
		}
	}

	// Advance animations
	for (i = 0; i < _objCount; i++) {
		pAnimData = _objects[i].pAnimData;
		if (pAnimData->isStatic || pAnimData->isPaused)
			continue;

		if (_objects[i].tick == pAnimData->maxTick) {
			_objects[i].tick = 0;
			if (pAnimData->animType == 4) {
				pAnimData->isPaused = 1;
				pAnimData->frame = 0;
			} else {
				pAnimData->frame++;
				if (pAnimData->frame >=
				    _vm->_scenery->_animations[(int)pAnimData->animation].layers[pAnimData->layer]->framesCount) {
					switch (pAnimData->animType) {
					case 0:
						pAnimData->frame = 0;
						break;

					case 1:
						pAnimData->frame = 0;

						*(_objects[i].pPosX) =
						    *(_objects[i].pPosX) +
						    _vm->_scenery->_animations[(int)pAnimData->animation].layers[pAnimData->layer]->animDeltaX;

						*(_objects[i].pPosY) =
						    *(_objects[i].pPosY) +
						    _vm->_scenery->_animations[(int)pAnimData->animation].layers[pAnimData->layer]->animDeltaY;
						break;

					case 2:
						pAnimData->frame = 0;
						pAnimData->animation =
						    pAnimData->newAnimation;
						pAnimData->layer =
						    pAnimData->newLayer;
						break;

					case 3:
						pAnimData->animType = 4;
						pAnimData->frame = 0;
						break;

					case 5:
						pAnimData->isStatic = 1;
						pAnimData->frame = 0;
						break;

					case 6:
						pAnimData->frame--;
						pAnimData->isPaused = 1;
						break;
					}
					pAnimData->newCycle = 1;
				} else {
					pAnimData->newCycle = 0;
				}
			}
		} else {
			_objects[i].tick++;
		}
	}
}

void Mult_v1::freeMult(void) {
	if (_vm->_anim->_animSurf != 0)
		_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);

	delete[] _objects;
	delete[] _renderData;
	delete[] _orderArray;

	_objects = 0;
	_renderData = 0;
	_orderArray = 0;
	_vm->_anim->_animSurf = 0;
}

void Mult_v1::playSound(Snd::SoundDesc * soundDesc, int16 repCount, int16 freq,
	    int16 channel) {
	_vm->_snd->playSample(soundDesc, repCount, freq);
}

void Mult_v1::freeMultKeys(void) {
	int i;
	char animCount;
	char staticCount;

	_dataPtr = _multData;
	staticCount = _dataPtr[0];
	animCount = _dataPtr[1];

	delete[] _dataPtr;

	staticCount++;
	animCount++;
	for (i = 0; i < staticCount; i++) {

		if (_staticLoaded[i] != 0)
			_vm->_scenery->freeStatic(_staticIndices[i]);
	}

	for (i = 0; i < animCount; i++) {
		if (_animLoaded[i] != 0)
			_vm->_scenery->freeAnim(_animIndices[i]);
	}

	delete[] _staticKeys;

	for (i = 0; i < 4; i++)
		delete[] _animKeys[i];

	delete[] _palFadeKeys;
	delete[] _palKeys;
	delete[] _textKeys;

	for (i = 0; i < _sndSlotsCount; i++) {
		_vm->_game->freeSoundSlot(19 - i);
	}

	delete[] _sndKeys;

	_multData = 0;

	if (_animDataAllocated != 0) {
		delete[] _objects;
		_objects = 0;

		delete[] _renderData;
		_renderData = 0;

		delete[] _animArrayX;
		_animArrayX = 0;

		delete[] _animArrayY;
		_animArrayY = 0;

		delete[] _animArrayData;
		_animArrayData = 0;

		if (_vm->_anim->_animSurf)
			_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);
		_vm->_anim->_animSurf = 0;

		_animDataAllocated = 0;
	}
}

} // End of namespace Gob
