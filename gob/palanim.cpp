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
	_fadeValue = 1;
	for (int i = 0; i < 256; i++) {
		_toFadeRed[i] = 0;
		_toFadeGreen[i] = 0;
		_toFadeBlue[i] = 0;
	}
}

char PalAnim::fadeColor(char from, char to) {
	if ((int16)from - _fadeValue > (int16)to)
		return from - _fadeValue;
	else if ((int16)from + _fadeValue < (int16)to)
		return from + _fadeValue;
	else
		return to;
}

char PalAnim::fadeStep(int16 oper) {
	byte newRed;
	byte newGreen;
	byte newBlue;
	char stop;
	int16 i;

	if (_vm->_global->_colorCount != 256)
		error("fadeStep: Only 256 color mode is supported!");

	if (oper == 0) {
		stop = 1;
		if (_vm->_global->_setAllPalette) {
			if (_vm->_global->_inVM != 0)
				error("fade: _vm->_global->_inVM != 0 not supported.");

			for (i = 0; i < 256; i++) {
				newRed = fadeColor(_vm->_global->_redPalette[i], _toFadeRed[i]);
				newGreen = fadeColor(_vm->_global->_greenPalette[i], _toFadeGreen[i]);
				newBlue = fadeColor(_vm->_global->_bluePalette[i], _toFadeBlue[i]);

				if (_vm->_global->_redPalette[i] != newRed
				    || _vm->_global->_greenPalette[i] != newGreen
				    || _vm->_global->_bluePalette[i] != newBlue) {

					_vm->_video->setPalElem(i, newRed, newGreen, newBlue, 0, 0x13);

					_vm->_global->_redPalette[i] = newRed;
					_vm->_global->_greenPalette[i] = newGreen;
					_vm->_global->_bluePalette[i] = newBlue;
					stop = 0;
				}
			}
		} else {
			for (i = 0; i < 16; i++) {

				_vm->_video->setPalElem(i,
				    fadeColor(_vm->_global->_redPalette[i],
					_toFadeRed[i]),
				    fadeColor(_vm->_global->_greenPalette[i],
					_toFadeGreen[i]),
				    fadeColor(_vm->_global->_bluePalette[i],
					_toFadeBlue[i]), -1, _vm->_global->_videoMode);

				if (_vm->_global->_redPalette[i] != _toFadeRed[i] ||
				    _vm->_global->_greenPalette[i] != _toFadeGreen[i] ||
				    _vm->_global->_bluePalette[i] != _toFadeBlue[i])
					stop = 0;
			}
		}
		return stop;
	} else if (oper == 1) {
		stop = 1;
		for (i = 0; i < 16; i++) {
			_vm->_video->setPalElem(i,
			    fadeColor(_vm->_global->_redPalette[i], _toFadeRed[i]),
			    _vm->_global->_greenPalette[i], _vm->_global->_bluePalette[i], -1, _vm->_global->_videoMode);

			if (_vm->_global->_redPalette[i] != _toFadeRed[i])
				stop = 0;
		}
		return stop;
	} else if (oper == 2) {
		stop = 1;
		for (i = 0; i < 16; i++) {
			_vm->_video->setPalElem(i,
			    _vm->_global->_redPalette[i],
			    fadeColor(_vm->_global->_greenPalette[i], _toFadeGreen[i]),
			    _vm->_global->_bluePalette[i], -1, _vm->_global->_videoMode);

			if (_vm->_global->_greenPalette[i] != _toFadeGreen[i])
				stop = 0;
		}
		return stop;
	} else if (oper == 3) {
		stop = 1;
		for (i = 0; i < 16; i++) {
			_vm->_video->setPalElem(i,
			    _vm->_global->_redPalette[i],
			    _vm->_global->_greenPalette[i],
			    fadeColor(_vm->_global->_bluePalette[i], _toFadeBlue[i]),
			    -1, _vm->_global->_videoMode);

			if (_vm->_global->_bluePalette[i] != _toFadeBlue[i])
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
		_fadeValue = -fadeV;
	else
		_fadeValue = 2;

	if (_vm->_global->_colorCount < 256) {
		if (palDesc != 0)
			_vm->_video->setFullPalette(palDesc);
		return;
	}

	if (_vm->_global->_setAllPalette == 0) {
		if (palDesc == 0) {
			for (i = 0; i < 16; i++) {
				_toFadeRed[i] = 0;
				_toFadeGreen[i] = 0;
				_toFadeBlue[i] = 0;
			}
		} else {
			for (i = 0; i < 16; i++) {
				_toFadeRed[i] = palDesc->vgaPal[i].red;
				_toFadeGreen[i] = palDesc->vgaPal[i].green;
				_toFadeBlue[i] = palDesc->vgaPal[i].blue;
			}
		}
	} else {
		if (_vm->_global->_inVM != 0)
			error("fade: _vm->_global->_inVM != 0 is not supported");

		if (palDesc == 0) {
			for (i = 0; i < 256; i++) {
				_toFadeRed[i] = 0;
				_toFadeGreen[i] = 0;
				_toFadeBlue[i] = 0;
			}
		} else {
			for (i = 0; i < 256; i++) {
				_toFadeRed[i] = palDesc->vgaPal[i].red;
				_toFadeGreen[i] = palDesc->vgaPal[i].green;
				_toFadeBlue[i] = palDesc->vgaPal[i].blue;
			}
		}
	}

	if (allColors == 0) {
		do {
			_vm->_video->waitRetrace(_vm->_global->_videoMode);

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
			_vm->_video->waitRetrace(_vm->_global->_videoMode);
			stop = fadeStep(1);
		} while (stop == 0);

		do {
			_vm->_video->waitRetrace(_vm->_global->_videoMode);
			stop = fadeStep(2);
		} while (stop == 0);

		do {
			_vm->_video->waitRetrace(_vm->_global->_videoMode);
			stop = fadeStep(3);
		} while (stop == 0);

		if (palDesc != 0)
			_vm->_video->setFullPalette(palDesc);
		else
			_vm->_util->clearPalette();
	}
}

}				// End of namespace Gob
