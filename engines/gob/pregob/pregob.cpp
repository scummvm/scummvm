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

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/palanim.h"
#include "gob/draw.h"
#include "gob/video.h"

#include "gob/pregob/pregob.h"

namespace Gob {

PreGob::PreGob(GobEngine *vm) : _vm(vm), _fadedOut(false) {
}

PreGob::~PreGob() {
}

void PreGob::fadeOut() {
	if (_fadedOut || _vm->shouldQuit())
		return;

	// Fade to black
	_vm->_palAnim->fade(0, 0, 0);

	_fadedOut = true;
}

void PreGob::fadeIn() {
	if (!_fadedOut || _vm->shouldQuit())
		return;

	// Fade to palette
	_vm->_draw->blitInvalidated();
	_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);

	_fadedOut = false;
}

void PreGob::clearScreen() {
	_vm->_draw->_backSurface->clear();
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);
	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();
}

void PreGob::initScreen() {
	_vm->_util->setFrameRate(15);

	_fadedOut = true;

	_vm->_draw->initScreen();

	_vm->_draw->_backSurface->clear();
	_vm->_util->clearPalette();

	_vm->_draw->forceBlit();
	_vm->_video->retrace();

	_vm->_util->processInput();
}

void PreGob::setPalette(const byte *palette, uint16 size) {
	memcpy(_vm->_draw->_vgaPalette, palette, 3 * size);

	// If we didn't fade out prior, immediately set the palette
	if (!_fadedOut)
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

} // End of namespace Gob
