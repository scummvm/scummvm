/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/video.h"
#include "gob/util.h"
#include "gob/global.h"
#include "gob/palanim.h"

namespace Gob {

PalAnim::PalAnim(GobEngine *vm) : _vm(vm) {
	fadeValue = 1;
	for (int i = 0; i < 256; i++) {
		toFadeRed[i] = 0;
		toFadeGreen[i] = 0;
		toFadeBlue[i] = 0;
	}
}

char PalAnim::fadeColor(char from, char to) {
	if ((int16)from - fadeValue > (int16)to)
		return from - fadeValue;
	else if ((int16)from + fadeValue < (int16)to)
		return from + fadeValue;
	else
		return to;
}

char PalAnim::fadeStep(int16 oper) {
	byte newRed;
	byte newGreen;
	byte newBlue;
	char stop;
	int16 i;

	if (_vm->_global->colorCount != 256)
		error("fadeStep: Only 256 color mode is supported!");

	if (oper == 0) {
		stop = 1;
		if (_vm->_global->setAllPalette) {
			if (_vm->_global->inVM != 0)
				error("fade: _vm->_global->inVM != 0 not supported.");

			for (i = 0; i < 256; i++) {
				newRed =
				    fadeColor(_vm->_global->redPalette[i],
				    toFadeRed[i]);
				newGreen =
				    fadeColor(_vm->_global->greenPalette[i],
				    toFadeGreen[i]);
				newBlue =
				    fadeColor(_vm->_global->bluePalette[i],
				    toFadeBlue[i]);

				if (_vm->_global->redPalette[i] != newRed
				    || _vm->_global->greenPalette[i] != newGreen
				    || _vm->_global->bluePalette[i] != newBlue) {

					_vm->_video->setPalElem(i, newRed, newGreen, newBlue, 0, 0x13);

					_vm->_global->redPalette[i] = newRed;
					_vm->_global->greenPalette[i] = newGreen;
					_vm->_global->bluePalette[i] = newBlue;
					stop = 0;
				}
			}
		} else {
			for (i = 0; i < 16; i++) {

				_vm->_video->setPalElem(i,
				    fadeColor(_vm->_global->redPalette[i],
					toFadeRed[i]),
				    fadeColor(_vm->_global->greenPalette[i],
					toFadeGreen[i]),
				    fadeColor(_vm->_global->bluePalette[i],
					toFadeBlue[i]), -1, _vm->_global->videoMode);

				if (_vm->_global->redPalette[i] != toFadeRed[i] ||
				    _vm->_global->greenPalette[i] != toFadeGreen[i] ||
				    _vm->_global->bluePalette[i] != toFadeBlue[i])
					stop = 0;
			}
		}
		return stop;
	} else if (oper == 1) {
		stop = 1;
		for (i = 0; i < 16; i++) {
			_vm->_video->setPalElem(i,
			    fadeColor(_vm->_global->redPalette[i], toFadeRed[i]),
			    _vm->_global->greenPalette[i], _vm->_global->bluePalette[i], -1, _vm->_global->videoMode);

			if (_vm->_global->redPalette[i] != toFadeRed[i])
				stop = 0;
		}
		return stop;
	} else if (oper == 2) {
		stop = 1;
		for (i = 0; i < 16; i++) {
			_vm->_video->setPalElem(i,
			    _vm->_global->redPalette[i],
			    fadeColor(_vm->_global->greenPalette[i], toFadeGreen[i]),
			    _vm->_global->bluePalette[i], -1, _vm->_global->videoMode);

			if (_vm->_global->greenPalette[i] != toFadeGreen[i])
				stop = 0;
		}
		return stop;
	} else if (oper == 3) {
		stop = 1;
		for (i = 0; i < 16; i++) {
			_vm->_video->setPalElem(i,
			    _vm->_global->redPalette[i],
			    _vm->_global->greenPalette[i],
			    fadeColor(_vm->_global->bluePalette[i], toFadeBlue[i]),
			    -1, _vm->_global->videoMode);

			if (_vm->_global->bluePalette[i] != toFadeBlue[i])
				stop = 0;
		}
		return stop;
	}
	return 1;
}

void PalAnim::fade(Video::PalDesc *palDesc, int16 fadeV, int16 allColors) {
	char stop;
	int16 i;

	if (fadeV < 0)
		fadeValue = -fadeV;
	else
		fadeValue = 2;

	if (_vm->_global->colorCount < 256) {
		if (palDesc != 0)
			_vm->_video->setFullPalette(palDesc);
		return;
	}

	if (_vm->_global->setAllPalette == 0) {
		if (palDesc == 0) {
			for (i = 0; i < 16; i++) {
				toFadeRed[i] = 0;
				toFadeGreen[i] = 0;
				toFadeBlue[i] = 0;
			}
		} else {
			for (i = 0; i < 16; i++) {
				toFadeRed[i] = palDesc->vgaPal[i].red;
				toFadeGreen[i] = palDesc->vgaPal[i].green;
				toFadeBlue[i] = palDesc->vgaPal[i].blue;
			}
		}
	} else {
		if (_vm->_global->inVM != 0)
			error("fade: _vm->_global->inVM != 0 is not supported");

		if (palDesc == 0) {
			for (i = 0; i < 256; i++) {
				toFadeRed[i] = 0;
				toFadeGreen[i] = 0;
				toFadeBlue[i] = 0;
			}
		} else {
			for (i = 0; i < 256; i++) {
				toFadeRed[i] = palDesc->vgaPal[i].red;
				toFadeGreen[i] = palDesc->vgaPal[i].green;
				toFadeBlue[i] = palDesc->vgaPal[i].blue;
			}
		}
	}

	if (allColors == 0) {
		do {
			if (_vm->_global->tmpPalBuffer == 0)
				_vm->_video->waitRetrace(_vm->_global->videoMode);

			stop = fadeStep(0);

			if (fadeV > 0)
				_vm->_util->delay(fadeV);
		} while (stop == 0);

		if (palDesc != 0)
			_vm->_video->setFullPalette(palDesc);
		else
			_vm->_util->clearPalette();
	}

	if (allColors == 1) {
		do {
			_vm->_video->waitRetrace(_vm->_global->videoMode);
			stop = fadeStep(1);
		} while (stop == 0);

		do {
			_vm->_video->waitRetrace(_vm->_global->videoMode);
			stop = fadeStep(2);
		} while (stop == 0);

		do {
			_vm->_video->waitRetrace(_vm->_global->videoMode);
			stop = fadeStep(3);
		} while (stop == 0);

		if (palDesc != 0)
			_vm->_video->setFullPalette(palDesc);
		else
			_vm->_util->clearPalette();
	}

	free(_vm->_global->tmpPalBuffer);
	_vm->_global->tmpPalBuffer = 0;
}

}				// End of namespace Gob
