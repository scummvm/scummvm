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
	_imagePicker = new UIImagePicker(vm, kSpinnerDestinations);
	_vqaPlayer = nullptr;
}

Spinner::~Spinner() {
	delete _imagePicker;

	reset();

	if (_vqaPlayer != nullptr) {
		_vqaPlayer->close();
		delete _vqaPlayer;
	}
}

void Spinner::setSelectableDestinationFlag(int destination, bool selectable) {
	_isDestinationSelectable[destination] = selectable;
}

bool Spinner::querySelectableDestinationFlag(int destination) const {
	return _isDestinationSelectable[destination];
}

SpinnerDestination SpinnerDestinationsNear[] = {
	{  0, Common::Rect(210, 263, 263, 332) },
	{  1, Common::Rect(307, 330, 361, 381) },
	{  2, Common::Rect(338, 137, 362, 169) },
	{  3, Common::Rect(248, 135, 289, 168) },
	{  4, Common::Rect(352, 222, 379, 238) },
	{ -1, Common::Rect(-1,-1,-1,-1) }

};

SpinnerDestination SpinnerDestinationsMedium[] = {
	{  0, Common::Rect(252, 242, 279, 283) },
	{  1, Common::Rect(301, 273, 328, 304) },
	{  2, Common::Rect(319, 182, 336, 200) },
	{  3, Common::Rect(269, 181, 293, 200) },
	{  4, Common::Rect(325, 227, 345, 240) },
	{  5, Common::Rect(259,  74, 380, 119) },
	{  6, Common::Rect(203, 124, 224, 136) },
	{  7, Common::Rect(200, 147, 222, 170) },
	{ -1, Common::Rect(-1,-1,-1,-1) }
};

SpinnerDestination SpinnerDestinationsFar[] = {
	{  0, Common::Rect(220, 227, 246, 262) },
	{  1, Common::Rect(260, 252, 286, 279) },
	{  2, Common::Rect(286, 178, 302, 196) },
	{  3, Common::Rect(244, 178, 263, 195) },
	{  4, Common::Rect(288, 216, 306, 228) },
	{  5, Common::Rect(249,  77, 353, 124) },
	{  6, Common::Rect(190, 127, 208, 138) },
	{  7, Common::Rect(185, 149, 206, 170) },
	{  8, Common::Rect(398, 249, 419, 268) },
	{  9, Common::Rect(390, 218, 419, 236) },
	{ -1, Common::Rect(-1, -1, -1, -1) }
};

static void spinner_mouseInCallback(int, void*);
static void spinner_mouseOutCallback(int, void*);
static void spinner_mouseDownCallback(int, void*);
static void spinner_mouseUpCallback(int, void*);

int Spinner::chooseDestination(int loopId, bool immediately) {
	_selectedDestination = 0;
	if (!_vm->openArchive("MODE.MIX")) {
		return 0;
	}

	if (loopId < 0) {
		_isOpen = true;
	} else {
		_vm->playerLosesControl();
		_vm->_scene->loopStartSpecial(kSceneLoopModeSpinner, loopId, immediately);
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
	uint8 mapmaskv[kSpinnerDestinations] = { 1, 1, 1, 1, 1, 3, 3, 3, 7, 7 };
	for (int i = 0; i != kSpinnerDestinations; ++i) {
		if (_isDestinationSelectable[i]) {
			mapmask |= mapmaskv[i];
		}
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

	_vqaPlayer->setLoop(spinnerLoopId,     -1, kLoopSetModeImmediate, nullptr, nullptr);
	_vqaPlayer->setLoop(spinnerLoopId + 1, -1, kLoopSetModeJustStart, nullptr, nullptr);

	for (int j = 0; j != shapeCount; ++j) {
		_shapes.push_back(new Shape(_vm));
		_shapes[j]->readFromContainer("SPINNER.SHP", firstShapeId + j);
	}

	_imagePicker->resetImages();

	for (SpinnerDestination *dest = _destinations; dest->id != -1; ++dest) {
		if (!_isDestinationSelectable[dest->id]) {
			continue;
		}

		const char *tooltip = _vm->_textSpinnerDestinations->getText(dest->id);

		_imagePicker->defineImage(
			dest->id,
			dest->rect,
			_shapes[dest->id],
			_shapes[dest->id + _shapes.size() / 2],
			_shapes[dest->id + _shapes.size() / 2],
			tooltip
		);
	}

	_imagePicker->activate(
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

	_imagePicker->deactivate();

	for (int i = 0; i != (int)_shapes.size(); ++i) {
		delete _shapes[i];
	}
	_shapes.clear();

	if (_vqaPlayer != nullptr) {
		_vqaPlayer->close();
		delete _vqaPlayer;
		_vqaPlayer = nullptr;
	}

	_vm->closeArchive("MODE.MIX");

	_isOpen = false;

	// TODO: Unfreeze game time
	// TODO: _vm->_scene->resume();

	return _selectedDestination;
}

static void spinner_mouseInCallback(int, void*) {
}

static void spinner_mouseOutCallback(int, void*) {
}

static void spinner_mouseDownCallback(int, void*) {
}

static void spinner_mouseUpCallback(int image, void *self) {
	if (image >= 0 && image < 10) {
		((Spinner *)self)->setSelectedDestination(image);
	}
}

void Spinner::open() {
	_isOpen = true;
}

bool Spinner::isOpen() const {
	return _isOpen;
}

int Spinner::handleMouseUp(int x, int y) {
	_imagePicker->handleMouseAction(x, y, false, true, false);
	return false;
}

int Spinner::handleMouseDown(int x, int y) {
	_imagePicker->handleMouseAction(x, y, true, false, false);
	return false;
}

void Spinner::tick() {
	if (!_vm->_gameIsRunning) {
		return;
	}

	int frame = _vqaPlayer->update();
	assert(frame >= -1);

	// vqaPlayer renders to _surfaceInterface
	blit(_vm->_surfaceInterface, _vm->_surfaceGame);

	Common::Point p = _vm->getMousePos();
	_imagePicker->handleMouseAction(p.x, p.y, false, false, false);
	if (_imagePicker->hasHoveredImage()) {
		_vm->_mouse->setCursor(1);
	} else {
		_vm->_mouse->setCursor(0);
	}
	_imagePicker->draw(_vm->_surfaceGame);
	_vm->_mouse->draw(_vm->_surfaceGame, p.x, p.y);
	_imagePicker->drawTooltip(_vm->_surfaceGame, p.x, p.y);

	_vm->blitToScreen(_vm->_surfaceGame);
	_vm->_system->delayMillis(10);
}

void Spinner::setSelectedDestination(int destination) {
	_selectedDestination = destination;
}

void Spinner::reset() {
	for (int i = 0; i != kSpinnerDestinations; ++i) {
		_isDestinationSelectable[i] = false;
	}

	_isOpen = false;
	_destinations = nullptr;
	_selectedDestination = -1;
	_imagePicker = nullptr;

	for (int i = 0; i != (int)_shapes.size(); ++i) {
		delete _shapes[i];
	}
	_shapes.clear();
}

void Spinner::resume() {
	if(_vqaPlayer == nullptr) {
		return;
	}

	//_vqa->clear();
	_vqaPlayer->setLoop(0, -1, kLoopSetModeImmediate, nullptr, nullptr);
	tick();
	_vqaPlayer->setLoop(1, -1, kLoopSetModeJustStart, nullptr, nullptr);
}

} // End of namespace BladeRunner
