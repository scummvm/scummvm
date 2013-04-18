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

#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/decfile.h"
#include "gob/anifile.h"

#include "gob/minigames/geisha/penetration.h"

namespace Gob {

namespace Geisha {

static const byte kPalette[48] = {
	0x16,  0x16,  0x16,
	0x12,  0x14,  0x16,
	0x34,  0x00,  0x25,
	0x1D,  0x1F,  0x22,
	0x24,  0x27,  0x2A,
	0x2C,  0x0D,  0x22,
	0x2B,  0x2E,  0x32,
	0x12,  0x09,  0x20,
	0x3D,  0x3F,  0x00,
	0x3F,  0x3F,  0x3F,
	0x00,  0x00,  0x00,
	0x15,  0x15,  0x3F,
	0x25,  0x22,  0x2F,
	0x1A,  0x14,  0x28,
	0x3F,  0x00,  0x00,
	0x15,  0x3F,  0x15
};

Penetration::Penetration(GobEngine *vm) : _vm(vm), _background(0), _objects(0) {
	_background = new Surface(320, 200, 1);
}

Penetration::~Penetration() {
	deinit();

	delete _background;
}

bool Penetration::play(uint16 var1, uint16 var2, uint16 var3) {
	init();
	initScreen();

	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();
	while (!_vm->_util->keyPressed() && !_vm->shouldQuit())
		_vm->_util->longDelay(1);

	deinit();
	return true;
}

void Penetration::init() {
	_background->clear();

	_vm->_video->drawPackedSprite("hyprmef2.cmp", *_background);

	_objects = new ANIFile(_vm, "tcite.ani", 320);
}

void Penetration::deinit() {
	delete _objects;

	_objects = 0;
}

void Penetration::initScreen() {
	_vm->_util->setFrameRate(15);

	memcpy(_vm->_draw->_vgaPalette     , kPalette, 48);
	memcpy(_vm->_draw->_vgaSmallPalette, kPalette, 48);

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_draw->_backSurface->blit(*_background);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);
}

} // End of namespace Geisha

} // End of namespace Gob
