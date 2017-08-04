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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/spinner.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/scene.h"
#include "bladerunner/shape.h"
#include "bladerunner/mouse.h"
#include "bladerunner/vqa_player.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/ui_image_picker.h"

#include "common/rect.h"
#include "common/system.h"

namespace BladeRunner {

Spinner::Spinner(BladeRunnerEngine *vm) : _vm(vm) {
	reset();
	_imagePicker = new UIImagePicker(vm, SPINNER_DESTINATIONS);
}

Spinner::~Spinner() {
	delete _imagePicker;
	reset();
}

void Spinner::setSelectableDestinationFlag(int destination, bool selectable) {
	_isDestinationSelectable[destination] = selectable;
}

bool Spinner::querySelectableDestinationFlag(int destination) const {
	return _isDestinationSelectable[destination];
}

SpinnerDestination SpinnerDestinationsNear[] = {
	{ 0, 0x0D2, 0x107, 0x107, 0x14C },
	{ 1, 0x133, 0x14A, 0x169, 0x17D },
	{ 2, 0x152, 0x089, 0x16A, 0x0A9 },
	{ 3, 0x0F8, 0x087, 0x121, 0x0A8 },
	{ 4, 0x160, 0x0DE, 0x17B, 0x0EE },
	{ -1, -1, -1, -1, -1 }
};

SpinnerDestination SpinnerDestinationsMedium[] = {
	{ 0, 0x0FC, 0x0F2, 0x117, 0x11B },
	{ 1, 0x12D, 0x111, 0x148, 0x130 },
	{ 2, 0x13F, 0x0B6, 0x150, 0x0C8 },
	{ 3, 0x10D, 0x0B5, 0x125, 0x0C8 },
	{ 4, 0x145, 0x0E3, 0x159, 0x0F0 },
	{ 5, 0x103, 0x04A, 0x17C, 0x077 },
	{ 6, 0x0CB, 0x07C, 0x0E0, 0x088 },
	{ 7, 0x0C8, 0x093, 0x0DE, 0x0AA },
	{ -1, -1, -1, -1, -1 }
};

SpinnerDestination SpinnerDestinationsFar[] = {
	{ 0, 0x0DC, 0x0E3, 0x0F6, 0x106 },
	{ 1, 0x104, 0x0FC, 0x11E, 0x117 },
	{ 2, 0x11E, 0x0B2, 0x12E, 0x0C4 },
	{ 3, 0x0F4, 0x0B2, 0x107, 0x0C3 },
	{ 4, 0x120, 0x0D8, 0x132, 0x0E4 },
	{ 5, 0x0F9, 0x04D, 0x161, 0x07C },
	{ 6, 0x0BE, 0x07F, 0x0D0, 0x08A },
	{ 7, 0x0B9, 0x095, 0x0CE, 0x0AA },
	{ 8, 0x18E, 0x0F9, 0x1A3, 0x10C },
	{ 9, 0x186, 0x0DA, 0x1A3, 0x0EC },
	{ -1, -1, -1, -1, -1 }
};

static void spinner_mouseInCallback(int, void*);
static void spinner_mouseOutCallback(int, void*);
static void spinner_mouseDownCallback(int, void*);
static void spinner_mouseUpCallback(int, void*);

int Spinner::interfaceChooseDest(int loopId, int loopFlag) {
	_selectedDestination = 0;
	if (!_vm->openArchive("MODE.MIX"))
		return 0;

	if (loopId < 0) {
		_isOpen = true;
	} else {
		_vm->playerLosesControl();
		_vm->_scene->loopStartSpecial(3, loopId, loopFlag);
		while (!_isOpen) {
			_vm->gameTick();
		}
		_vm->playerGainsControl();
	}

	_vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceInterface);
	if (!_vqaPlayer->open("SPINNER.VQA")) {
		return 0;
	}

	_vm->_mouse->setCursor(0);

	// Determine which map we need to show to include the active destinations
	uint8 mapmask = 0;
	uint8 mapmaskv[SPINNER_DESTINATIONS] = { 1, 1, 1, 1, 1, 3, 3, 3, 7, 7 };
	for (int i = 0; i != SPINNER_DESTINATIONS; ++i) {
		if (_isDestinationSelectable[i])
			mapmask |= mapmaskv[i];
	}

	_destinations = nullptr;
	int firstShapeId = 0;
	int shapeCount = 0;
	int spinnerLoopId = 4;

	mapmask = 1;

	if (mapmask & 4) {
		_destinations = SpinnerDestinationsFar;
		firstShapeId = 26;
		shapeCount = 20;
		spinnerLoopId = 4;
	} else if (mapmask & 2) {
		_destinations = SpinnerDestinationsMedium;
		firstShapeId = 10;
		shapeCount = 16;
		spinnerLoopId = 2;
	} else if (mapmask & 1) {
		_destinations = SpinnerDestinationsNear;
		firstShapeId = 0;
		shapeCount = 10;
		spinnerLoopId = 0;
	} else {
		return -1;
	}

	_vqaPlayer->setLoop(spinnerLoopId,     -1, 2, nullptr, nullptr);
	_vqaPlayer->setLoop(spinnerLoopId + 1, -1, 0, nullptr, nullptr);

	for (int j = 0; j != shapeCount; ++j) {
		_shapes.push_back(new Shape(_vm));
		_shapes[j]->readFromContainer("SPINNER.SHP", firstShapeId + j);
	}

	_imagePicker->resetImages();

	for (SpinnerDestination *dest = _destinations; dest->id != -1; ++dest) {
		if (!_isDestinationSelectable[dest->id])
			continue;

		const char *tooltip = _vm->_textSpinnerDestinations->getText(dest->id);

		_imagePicker->defineImage(
			dest->id,
			dest->left,
			dest->top,
			dest->right,
			dest->bottom,
			_shapes[dest->id],
			_shapes[dest->id + _shapes.size() / 2],
			_shapes[dest->id + _shapes.size() / 2],
			tooltip
		);
	}

	_imagePicker->setCallbacks(
		spinner_mouseInCallback,
		spinner_mouseOutCallback,
		spinner_mouseDownCallback,
		spinner_mouseUpCallback,
		this
	);

	// TODO: Freeze game time
	_selectedDestination = -1;
	do {
		_vm->gameTick();
	} while (_selectedDestination == -1);

	// TODO: Unfreeze game time
	_isOpen = false;
	// TODO: _vm->_scene->resume();

	for (int i = 0; i != (int)_shapes.size(); ++i)
		delete _shapes[i];
	_shapes.clear();

	return _selectedDestination;
}

static void spinner_mouseInCallback(int, void*) {
}

static void spinner_mouseOutCallback(int, void*) {
}

static void spinner_mouseDownCallback(int, void*) {
}

static void spinner_mouseUpCallback(int image, void *data) {
	if (image >= 0 && image < 10) {
		Spinner *spinner = (Spinner *)data;
		spinner->setSelectedDestination(image);
	}
}

void Spinner::setIsOpen() {
	_isOpen = true;
}

bool Spinner::isOpen() const {
	return _isOpen;
}

int Spinner::handleMouseUp(int x, int y) {
	_imagePicker->handleMouseAction(x, y, false, true, 0);
	return false;
}

int Spinner::handleMouseDown(int x, int y) {
	_imagePicker->handleMouseAction(x, y, true, false, 0);
	return false;
}

void Spinner::tick() {
	if (!_vm->_gameIsRunning)
		return;

	int frame = _vqaPlayer->update();
	assert(frame >= -1);

	// vqaPlayer renders to _surfaceInterface
	blit(_vm->_surfaceInterface, _vm->_surfaceGame);

	_imagePicker->draw(_vm->_surfaceInterface);

	Common::Point p = _vm->getMousePos();
	_imagePicker->handleMouseAction(p.x, p.y, false, false, false);
	if (_imagePicker->hasHoveredImage()) {
		_vm->_mouse->setCursor(1);
	} else {
		_vm->_mouse->setCursor(0);
	}
	_vm->_mouse->draw(_vm->_surfaceGame, p.x, p.y);

	_vm->blitToScreen(_vm->_surfaceGame);
	_vm->_system->delayMillis(10);
}

void Spinner::setSelectedDestination(int destination) {
	_selectedDestination = destination;
}

void Spinner::reset() {
	for (int i = 0; i != SPINNER_DESTINATIONS; ++i) {
		_isDestinationSelectable[i] = 0;
	}

	_isOpen = false;
	_destinations = nullptr;
	_selectedDestination = -1;
	_imagePicker = nullptr;

	for (int i = 0; i != (int)_shapes.size(); ++i)
		delete _shapes[i];
	_shapes.clear();
}

void Spinner::resume() {
	// TODO
}

} // End of namespace BladeRunner
