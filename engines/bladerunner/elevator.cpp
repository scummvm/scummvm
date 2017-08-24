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

#include "bladerunner/elevator.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/actor.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/mouse.h"
#include "bladerunner/shape.h"
#include "bladerunner/ui_image_picker.h"
#include "bladerunner/vqa_player.h"

#include "common/rect.h"
#include "common/system.h"

namespace BladeRunner {

Elevator::Elevator(BladeRunnerEngine *vm) : _vm(vm) {
	reset();
	_imagePicker = new UIImagePicker(vm, 8);
}

Elevator::~Elevator() {
	delete _imagePicker;
	reset();
}

int Elevator::activate(int elevatorId) {
	const char *vqaName;

	if (elevatorId == 1) {
		_buttonClicked = 3;
		vqaName = "MA06ELEV.VQA";
	} else if (elevatorId == 2) {
		_buttonClicked = 1;
		vqaName = "PS02ELEV.VQA";
	} else {
		error("Invalid elevator id");
	}

	if (!_vm->openArchive("MODE.MIX"))
		return 0;

	_vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceInterface);
	if (!_vqaPlayer->open(vqaName)) {
		return 0;
	}

	_vqaPlayer->setLoop(1, -1, 0, nullptr, nullptr);
	_vm->_mouse->setCursor(0);

	for (int i = 0; i != 16; ++i) {
		_shapes.push_back(new Shape(_vm));
		_shapes[i]->readFromContainer("ELEVATOR.SHP", i);
	}

	_imagePicker->resetImages();

	if (elevatorId == 1) {
		_imagePicker->defineImage(
			0,
			220, 298, 308, 392,
			nullptr,
			_shapes[11],
			_shapes[14],
			nullptr);
		_imagePicker->defineImage(
			1,
			259, 259, 302, 292,
			nullptr,
			_shapes[10],
			_shapes[13],
			nullptr);
		_imagePicker->defineImage(
			2,
			227, 398, 301, 434,
			nullptr,
			_shapes[12],
			_shapes[15],
			nullptr);
	} else {
		_imagePicker->defineImage(
			4,
			395, 131, 448, 164,
			nullptr,
			_shapes[0],
			_shapes[5],
			nullptr
		);
		_imagePicker->defineImage(
			3,
			395, 165, 448, 198,
			nullptr,
			_shapes[1],
			_shapes[6],
			nullptr
		);
		_imagePicker->defineImage(
			5,
			395, 199, 448, 232,
			nullptr,
			_shapes[2],
			_shapes[7],
			nullptr
		);
		_imagePicker->defineImage(
			6,
			395, 233, 448, 264,
			nullptr,
			_shapes[3],
			_shapes[8],
			nullptr
		);
		_imagePicker->defineImage(
			7,
			395, 265, 448, 295,
			nullptr,
			_shapes[4],
			_shapes[9],
			nullptr
		);
	}

	_imagePicker->setCallbacks(
		elevator_mouseInCallback,
		elevator_mouseOutCallback,
		elevator_mouseDownCallback,
		elevator_mouseUpCallback,
		this
	);

	open();

	// TODO: time->lock();

	_buttonClicked = -1;
	do {
		_vm->gameTick();
	} while (_buttonClicked == -1);

	_vqaPlayer->close();
	delete _vqaPlayer;

	for (int i = 0; i != (int)_shapes.size(); ++i)
		delete _shapes[i];
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
	_imagePicker->handleMouseAction(x, y, false, true, 0);
	return false;
}

int Elevator::handleMouseDown(int x, int y) {
	_imagePicker->handleMouseAction(x, y, true, false, 0);
	return false;
}

void Elevator::tick() {
	if (!_vm->_gameIsRunning)
		return;

	int frame = _vqaPlayer->update();
	assert(frame >= -1);

	// vqaPlayer renders to _surfaceInterface
	blit(_vm->_surfaceInterface, _vm->_surfaceGame);

	Common::Point p = _vm->getMousePos();

	// TODO(madmoose): BLADE.EXE has hasHoveredImage before handleMouseAction?
	_imagePicker->handleMouseAction(p.x, p.y, false, false, false);
	if (_imagePicker->hasHoveredImage()) {
		_vm->_mouse->setCursor(1);
	} else {
		_vm->_mouse->setCursor(0);
	}

	_imagePicker->draw(_vm->_surfaceGame);
	_vm->_mouse->draw(_vm->_surfaceGame, p.x, p.y);

	_vm->blitToScreen(_vm->_surfaceGame);
	tickDescription();
	_vm->_system->delayMillis(10);
}

void Elevator::buttonClick(int buttonId) {
	_buttonClicked = buttonId;
}

void Elevator::reset() {
	_isOpen = false;
	_vqaPlayer = 0;
	_imagePicker = 0;
	_actorId = -1;
	_sentenceId = -1;
	_timeSpeakDescription = 0;
}

void Elevator::buttonFocus(int buttonId) {
	switch (buttonId) {
		case 7:
			setupDescription(39, 140);
			break;
		case 6:
			setupDescription(39, 130);
			break;
		case 5:
			setupDescription(39, 120);
			break;
		case 4:
			setupDescription(39, 100);
			break;
		case 3:
			setupDescription(39, 110);
			break;
		case 2:
			setupDescription(39, 130);
			break;
		case 1:
			setupDescription(39, 100);
			break;
		case 0:
			setupDescription(39, 150);
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
	if (_actorId <= 0 || now < _timeSpeakDescription)
		return;

	_vm->_actors[_actorId]->speechPlay(_sentenceId, false);
	_actorId = -1;
	_sentenceId = -1;
}

void Elevator::resume() {
	// TODO
}

void elevator_mouseInCallback(int buttonId, void *self) {
	((Elevator*)self)->buttonFocus(buttonId);
}

void elevator_mouseOutCallback(int, void *self) {
	((Elevator*)self)->buttonFocus(-1);
}

void elevator_mouseDownCallback(int, void *self) {
	Elevator *elevator = ((Elevator*)self);
	const char *name = elevator->_vm->_gameInfo->getSfxTrack(515);
	elevator->_vm->_audioPlayer->playAud(name, 100, 0, 0, 50, 0);
}

void elevator_mouseUpCallback(int buttonId, void *self) {
	((Elevator*)self)->buttonClick(buttonId);
}

} // End of namespace BladeRunner
