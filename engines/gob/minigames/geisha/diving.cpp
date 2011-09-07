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

#include "common/list.h"

#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/decfile.h"
#include "gob/anifile.h"

#include "gob/minigames/geisha/evilfish.h"
#include "gob/minigames/geisha/diving.h"

namespace Gob {

namespace Geisha {

Diving::Diving(GobEngine *vm) : _vm(vm), _background(0),
	_objects(0), _gui(0), _oko(0), _lungs(0), _heart(0) {

}

Diving::~Diving() {
	deinit();
}

bool Diving::play(uint16 playerCount, bool hasPearlLocation) {
	init();
	initScreen();

	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();

	EvilFish shark(*_objects, 320, 0, 14, 8, 9, 3);

	Common::List<ANIObject *> objects;

	objects.push_back(_water);
	objects.push_back(&shark);

	shark.enter(EvilFish::kDirectionLeft, 90);

	while (!_vm->_util->keyPressed() && !_vm->shouldQuit()) {
		int16 left, top, right, bottom;

		// Clear the previous animation frames
		for (Common::List<ANIObject *>::iterator o = objects.reverse_begin();
		     o != objects.end(); --o) {

			(*o)->clear(*_vm->_draw->_backSurface, left, top, right, bottom);
			_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
		}

		// Draw the current animation frames
		for (Common::List<ANIObject *>::iterator o = objects.begin();
		     o != objects.end(); ++o) {

			(*o)->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
			_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

			(*o)->advance();
		}

		_vm->_draw->blitInvalidated();

		_vm->_util->waitEndFrame();
		_vm->_util->processInput();
	}

	deinit();
	return true;
}

void Diving::init() {
	_background = new DECFile(_vm, "tperle.dec"  , 320, 200);
	_objects    = new ANIFile(_vm, "tperle.ani"  , 320);
	_gui        = new ANIFile(_vm, "tperlcpt.ani", 320);
	_oko        = new ANIFile(_vm, "tplonge.ani" , 320);

	_water = new ANIObject(*_objects);
	_lungs = new ANIObject(*_gui);
	_heart = new ANIObject(*_gui);

	_water->setAnimation(7);
	_water->setPosition();
	_water->setVisible(true);

	_lungs->setAnimation(0);
	_lungs->setPosition();
	_lungs->setVisible(true);

	_heart->setAnimation(1);
	_heart->setPosition();
	_heart->setVisible(true);
}

void Diving::deinit() {
	delete _heart;
	delete _lungs;
	delete _water;

	delete _oko;
	delete _gui;
	delete _objects;
	delete _background;

	_water = 0;
	_heart = 0;
	_lungs = 0;

	_oko        = 0;
	_gui        = 0;
	_objects    = 0;
	_background = 0;
}

void Diving::initScreen() {
	_vm->_util->setFrameRate(15);

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_draw->_backSurface->clear();
	_background->draw(*_vm->_draw->_backSurface);

	int16 left, top, right, bottom;
	_lungs->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
	_heart->draw(*_vm->_draw->_backSurface, left, top, right, bottom);

	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);
}

} // End of namespace Geisha

} // End of namespace Gob
