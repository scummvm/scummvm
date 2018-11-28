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

#include "bladerunner/ui/elevator.h"

#include "bladerunner/actor.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/game_info.h"
#include "bladerunner/mouse.h"
#include "bladerunner/shape.h"
#include "bladerunner/script/script.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/vqa_player.h"

#include "common/rect.h"
#include "common/str.h"
#include "common/system.h"

namespace BladeRunner {

Elevator::Elevator(BladeRunnerEngine *vm) {
	_vm = vm;
	reset();
	_imagePicker = new UIImagePicker(vm, 8);
}

Elevator::~Elevator() {
	delete _imagePicker;
	_imagePicker = nullptr;
}

int Elevator::activate(int elevatorId) {
	const char *vqaName;

	if (elevatorId == kElevatorMA) {
		_buttonClicked = 3;
		vqaName = "MA06ELEV.VQA";
	} else if (elevatorId == kElevatorPS) {
		_buttonClicked = 1;
		vqaName = "PS02ELEV.VQA";
	} else {
		error("Invalid elevator id");
	}

	if (!_vm->openArchive("MODE.MIX")) {
		return 0;
	}

	_vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceBack, vqaName);
	if (!_vqaPlayer->open()) {
		return 0;
	}

	_vqaPlayer->setLoop(1, -1, kLoopSetModeJustStart, nullptr, nullptr);
	_vm->_mouse->setCursor(0);

	for (int i = 0; i != 16; ++i) {
		_shapes.push_back(new Shape(_vm));
		_shapes[i]->open("ELEVATOR.SHP", i);
	}

	_imagePicker->resetImages();

	if (elevatorId == kElevatorMA) {
		_imagePicker->defineImage(
			0,
			Common::Rect(220, 298, 308, 392),
			nullptr,
			_shapes[11],
			_shapes[14],
			nullptr);
		_imagePicker->defineImage(
			1,
			Common::Rect(259, 259, 302, 292),
			nullptr,
			_shapes[10],
			_shapes[13],
			nullptr);
		_imagePicker->defineImage(
			2,
			Common::Rect(227, 398, 301, 434),
			nullptr,
			_shapes[12],
			_shapes[15],
			nullptr);
	} else { // kElevatorPS
		_imagePicker->defineImage(
			4,
			Common::Rect(395, 131, 448, 164),
			nullptr,
			_shapes[0],
			_shapes[5],
			nullptr
		);
		_imagePicker->defineImage(
			3,
			Common::Rect(395, 165, 448, 198),
			nullptr,
			_shapes[1],
			_shapes[6],
			nullptr
		);
		_imagePicker->defineImage(
			5,
			Common::Rect(395, 199, 448, 232),
			nullptr,
			_shapes[2],
			_shapes[7],
			nullptr
		);
		_imagePicker->defineImage(
			6,
			Common::Rect(395, 233, 448, 264),
			nullptr,
			_shapes[3],
			_shapes[8],
			nullptr
		);
		_imagePicker->defineImage(
			7,
			Common::Rect(395, 265, 448, 295),
			nullptr,
			_shapes[4],
			_shapes[9],
			nullptr
		);
	}

	_imagePicker->activate(
		mouseInCallback,
		mouseOutCallback,
		mouseDownCallback,
		mouseUpCallback,
		this
	);

	open();

	// TODO: time->lock();

	_buttonClicked = -1;
	do {
		_vm->gameTick();
	} while (_vm->_gameIsRunning && _buttonClicked == -1);

	_imagePicker->deactivate();

	_vqaPlayer->close();
	delete _vqaPlayer;

	for (int i = 0; i != (int)_shapes.size(); ++i) {
		delete _shapes[i];
	}
	_shapes.clear();

	_vm->closeArchive("MODE.MIX");

	_isOpen = false;

	// TODO: time->unlock();

	return _buttonClicked;
}

void Elevator::open() {
	resetDescription();
	_isOpen = true;
}

bool Elevator::isOpen() const {
	return _isOpen;
}

int Elevator::handleMouseUp(int x, int y) {
	_imagePicker->handleMouseAction(x, y, false, true, false);
	return false;
}

int Elevator::handleMouseDown(int x, int y) {
	_imagePicker->handleMouseAction(x, y, true, false, false);
	return false;
}

void Elevator::tick() {
	if (!_vm->_gameIsRunning) {
		return;
	}

	int frame = _vqaPlayer->update();
	assert(frame >= -1);

	// vqaPlayer renders to _surfaceBack
	blit(_vm->_surfaceBack, _vm->_surfaceFront);

	Common::Point p = _vm->getMousePos();

	// TODO(madmoose): BLADE.EXE has hasHoveredImage before handleMouseAction?
	_imagePicker->handleMouseAction(p.x, p.y, false, false, false);
	if (_imagePicker->hasHoveredImage()) {
		_vm->_mouse->setCursor(1);
	} else {
		_vm->_mouse->setCursor(0);
	}

	_imagePicker->draw(_vm->_surfaceFront);
	_vm->_mouse->draw(_vm->_surfaceFront, p.x, p.y);

	_vm->blitToScreen(_vm->_surfaceFront);
	tickDescription();
	_vm->_system->delayMillis(10);
}

void Elevator::buttonClick(int buttonId) {
	_buttonClicked = buttonId;
}

void Elevator::reset() {
	_isOpen = false;
	_vqaPlayer = nullptr;
	_imagePicker = nullptr;
	_actorId = -1;
	_sentenceId = -1;
	_timeSpeakDescription = 0;
	_buttonClicked = false;
}

void Elevator::buttonFocus(int buttonId) {
	switch (buttonId) {
	case 7:
		setupDescription(kActorAnsweringMachine, 140);
		break;
	case 6:
		setupDescription(kActorAnsweringMachine, 130);
		break;
	case 5:
		setupDescription(kActorAnsweringMachine, 120);
		break;
	case 4:
		setupDescription(kActorAnsweringMachine, 100);
		break;
	case 3:
		setupDescription(kActorAnsweringMachine, 110);
		break;
	case 2:
		setupDescription(kActorAnsweringMachine, 130);
		break;
	case 1:
		setupDescription(kActorAnsweringMachine, 100);
		break;
	case 0:
		setupDescription(kActorAnsweringMachine, 150);
		break;
	default:
		resetDescription();
		break;
	}
}

void Elevator::setupDescription(int actorId, int sentenceId) {
	_actorId = actorId;
	_sentenceId = sentenceId;

	// TODO: Use proper timer
	_timeSpeakDescription = _vm->getTotalPlayTime() + 600;
}

void Elevator::resetDescription() {
	_actorId = -1;
	_sentenceId = -1;
	_timeSpeakDescription = 0;
}

void Elevator::tickDescription() {
	int now = _vm->getTotalPlayTime();
	if (_actorId <= 0 || now < _timeSpeakDescription) {
		return;
	}

	_vm->_actors[_actorId]->speechPlay(_sentenceId, false);
	_actorId = -1;
	_sentenceId = -1;
}

void Elevator::resume() {
	// TODO
}

void Elevator::mouseInCallback(int buttonId, void *self) {
	((Elevator *)self)->buttonFocus(buttonId);
}

void Elevator::mouseOutCallback(int, void *self) {
	((Elevator *)self)->buttonFocus(-1);
}

void Elevator::mouseDownCallback(int, void *self) {
	Elevator *elevator = ((Elevator *)self);
	elevator->_vm->_audioPlayer->playAud(elevator->_vm->_gameInfo->getSfxTrack(515), 100, 0, 0, 50, 0);
}

void Elevator::mouseUpCallback(int buttonId, void *self) {
	((Elevator *)self)->buttonClick(buttonId);
}

} // End of namespace BladeRunner
