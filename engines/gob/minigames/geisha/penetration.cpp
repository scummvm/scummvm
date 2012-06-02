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
#include "gob/cmpfile.h"
#include "gob/anifile.h"
#include "gob/aniobject.h"

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

Penetration::Penetration(GobEngine *vm) : _vm(vm), _background(0), _sprites(0), _objects(0) {
	_background = new Surface(320, 200, 1);
}

Penetration::~Penetration() {
	deinit();

	delete _background;
}

bool Penetration::play(bool hasAccessPass, bool hasMaxEnergy, bool testMode) {
	init();
	initScreen();

	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();

	while (!_vm->shouldQuit()) {
		updateAnims();

		// Draw and wait for the end of the frame
		_vm->_draw->blitInvalidated();
		_vm->_util->waitEndFrame();

		// Handle input
		_vm->_util->processInput();

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);
		// Aborting the game
		if (key == kKeyEscape)
			break;
	}

	deinit();
	return false;
}

void Penetration::init() {
	_background->clear();

	_vm->_video->drawPackedSprite("hyprmef2.cmp", *_background);

	_sprites = new CMPFile(_vm, "tcifplai.cmp", 320, 200);
	_objects = new ANIFile(_vm, "tcite.ani", 320);
}

void Penetration::deinit() {
	_anims.clear();

	delete _objects;
	delete _sprites;

	_objects = 0;
	_sprites = 0;
}

void Penetration::initScreen() {
	_vm->_util->setFrameRate(15);

	memcpy(_vm->_draw->_vgaPalette     , kPalette, 48);
	memcpy(_vm->_draw->_vgaSmallPalette, kPalette, 48);

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_draw->_backSurface->blit(*_background);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);
}

int16 Penetration::checkInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons) {
	_vm->_util->getMouseState(&mouseX, &mouseY, &mouseButtons);

	return _vm->_util->checkKey();
}

void Penetration::updateAnims() {
	int16 left, top, right, bottom;

	// Clear the previous animation frames
	for (Common::List<ANIObject *>::iterator a = _anims.reverse_begin();
			 a != _anims.end(); --a) {

		(*a)->clear(*_vm->_draw->_backSurface, left, top, right, bottom);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
	}

	// Draw the current animation frames
	for (Common::List<ANIObject *>::iterator a = _anims.begin();
			 a != _anims.end(); ++a) {

		(*a)->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

		(*a)->advance();
	}
}

} // End of namespace Geisha

} // End of namespace Gob
