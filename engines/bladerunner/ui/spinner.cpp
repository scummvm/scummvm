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

#include "bladerunner/ui/spinner.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/actor.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/game_info.h"
#include "bladerunner/subtitles.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/mouse.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/shape.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/vqa_player.h"

#include "common/rect.h"
#include "common/system.h"

namespace BladeRunner {

Spinner::Spinner(BladeRunnerEngine *vm) {
	_vm = vm;
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

int Spinner::chooseDestination(int loopId, bool immediately) {
	if (_vm->_cutContent) {
		resetDescription();
	}
	_selectedDestination = 0;
	if (!_vm->openArchive("MODE.MIX")) {
		return 0;
	}

	if (loopId < 0) {
		// call Spinner:open()
		open();
	} else {
		_vm->playerLosesControl();
		_vm->_scene->loopStartSpecial(kSceneLoopModeSpinner, loopId, immediately);
		while (_vm->_gameIsRunning && !_isOpen) {
			_vm->gameTick();
		}
		_vm->playerGainsControl();
	}

	_vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceBack, "SPINNER.VQA");
	if (!_vqaPlayer->open()) {
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

	if (mapmask & 4) {
		_destinations = getDestinationsFar();
		firstShapeId = 26;
		shapeCount = 20;
		spinnerLoopId = 4;
	} else if (mapmask & 2) {
		_destinations = getDestinationsMedium();
		firstShapeId = 10;
		shapeCount = 16;
		spinnerLoopId = 2;
	} else if (mapmask & 1) {
		_destinations = getDestinationsNear();
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
		_shapes[j]->open("SPINNER.SHP", firstShapeId + j);
	}

	_imagePicker->resetImages();

	for (const Destination *dest = _destinations; dest->id != -1; ++dest) {
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

	if (_vm->_cutContent) {
		_imagePicker->activate(
			mouseInCallback,
			mouseOutCallback,
			mouseDownCallback,
			mouseUpCallback,
			this
		);
		_vm->_actors[kActorAnsweringMachine]->speechPlay(480, false);
		_vm->_ambientSounds->addSound(kSfxSPINAMB2,  5, 30, 30,  45,    0,   0, -101, -101, 0, 0);
	} else {
		_imagePicker->activate(
			nullptr,
			nullptr,
			nullptr,
			mouseUpCallback,
			this
		);
	}

	_vm->_time->pause();

	_selectedDestination = -1;
	do {
		_vm->gameTick();
	} while (_vm->_gameIsRunning && _selectedDestination == -1);

	_imagePicker->deactivate();

	for (int i = 0; i != (int)_shapes.size(); ++i) {
		delete _shapes[i];
	}
	_shapes.clear();

	_vqaPlayer->close();
	delete _vqaPlayer;
	_vqaPlayer = nullptr;

	_vm->closeArchive("MODE.MIX");

	_isOpen = false;

	_vm->_time->resume();
	_vm->_scene->resume();

	if (_vm->_cutContent) {
		_vm->_ambientSounds->removeNonLoopingSound(kSfxSPINAMB2, true);
	}
	return _selectedDestination;
}

// cut content
void Spinner::mouseInCallback(int destinationImage, void *self) {
	((Spinner *)self)->destinationFocus(destinationImage);
}

// cut content
void Spinner::mouseOutCallback(int, void *self) {
	((Spinner *)self)->destinationFocus(-1);
}

// cut content
void Spinner::mouseDownCallback(int, void *self) {
	((Spinner *)self)->_vm->_audioPlayer->playAud(((Spinner *)self)->_vm->_gameInfo->getSfxTrack(kSfxSPNBEEP9), 100, 0, 0, 50, 0);
}

void Spinner::mouseUpCallback(int destinationImage, void *self) {
	if (destinationImage >= 0 && destinationImage < 10) {
		((Spinner *)self)->setSelectedDestination(destinationImage);
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
	if (!_vm->_windowIsActive) {
		return;
	}

	int frame = _vqaPlayer->update();
	assert(frame >= -1);

	// vqaPlayer renders to _surfaceBack
	blit(_vm->_surfaceBack, _vm->_surfaceFront);

	Common::Point p = _vm->getMousePos();
	_imagePicker->handleMouseAction(p.x, p.y, false, false, false);
	if (_imagePicker->hasHoveredImage()) {
		_vm->_mouse->setCursor(1);
	} else {
		_vm->_mouse->setCursor(0);
	}
	_imagePicker->draw(_vm->_surfaceFront);
	_vm->_mouse->draw(_vm->_surfaceFront, p.x, p.y);
	_imagePicker->drawTooltip(_vm->_surfaceFront, p.x, p.y);

	if (_vm->_cutContent) {
		_vm->_subtitles->tick(_vm->_surfaceFront);
	}
	_vm->blitToScreen(_vm->_surfaceFront);

	if (_vm->_cutContent) {
		tickDescription();
	}
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

	_actorId = -1;
	_sentenceId = -1;
	_timeSpeakDescriptionStart = 0u;

	for (int i = 0; i != (int)_shapes.size(); ++i) {
		delete _shapes[i];
	}
	_shapes.clear();
}

void Spinner::resume() {
	if (_vqaPlayer == nullptr) {
		return;
	}

	//_vqa->clear();
	_vqaPlayer->setLoop(0, -1, kLoopSetModeImmediate, nullptr, nullptr);
	tick();
	_vqaPlayer->setLoop(1, -1, kLoopSetModeJustStart, nullptr, nullptr);
}

void Spinner::save(SaveFileWriteStream &f) {
	assert(!_isOpen);

	for (int i = 0; i != kSpinnerDestinations; ++i) {
		f.writeBool(_isDestinationSelectable[i]);
	}
}

void Spinner::load(SaveFileReadStream &f) {
	for (int i = 0; i != kSpinnerDestinations; ++i) {
		_isDestinationSelectable[i] = f.readBool();
	}
}

const Spinner::Destination *Spinner::getDestinationsFar() {
	static const Destination destinations[] = {
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
	return destinations;
}

const Spinner::Destination *Spinner::getDestinationsMedium() {
	static const Destination destinations[] = {
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
	return destinations;
}

const Spinner::Destination *Spinner::getDestinationsNear() {
	static const Destination destinations[] = {
		{  0, Common::Rect(210, 263, 263, 332) },
		{  1, Common::Rect(307, 330, 361, 381) },
		{  2, Common::Rect(338, 137, 362, 169) },
		{  3, Common::Rect(248, 135, 289, 168) },
		{  4, Common::Rect(352, 222, 379, 238) },
		{ -1, Common::Rect(-1,-1,-1,-1) }
	};
	return destinations;
}

void Spinner::destinationFocus(int destinationImage) {
	// TODO 590, 600 are "Third Sector", "Fourth Sector" maybe restore those too?
	switch (destinationImage) {
	case kSpinnerDestinationPoliceStation:
		setupDescription(kActorAnsweringMachine, 500);
		break;
	case kSpinnerDestinationMcCoysApartment:
		setupDescription(kActorAnsweringMachine, 510);
		break;
	case kSpinnerDestinationRuncitersAnimals:
		setupDescription(kActorAnsweringMachine, 490);
		break;
	case kSpinnerDestinationChinatown:
		setupDescription(kActorAnsweringMachine, 520);
		break;
	case kSpinnerDestinationAnimoidRow:
		setupDescription(kActorAnsweringMachine, 550);
		break;
	case kSpinnerDestinationTyrellBuilding:
		setupDescription(kActorAnsweringMachine, 560);
		break;
	case kSpinnerDestinationDNARow:
		setupDescription(kActorAnsweringMachine, 530);
		break;
	case kSpinnerDestinationBradburyBuilding:
		setupDescription(kActorAnsweringMachine, 540);
		break;
	case kSpinnerDestinationNightclubRow:
		setupDescription(kActorAnsweringMachine, 570);
		break;
	case kSpinnerDestinationHysteriaHall:
		setupDescription(kActorAnsweringMachine, 580);
		break;
	default:
		resetDescription();
		break;
	}
}

// copied from elevator.cpp code
void Spinner::setupDescription(int actorId, int sentenceId) {
	_actorId = actorId;
	_sentenceId = sentenceId;
	_timeSpeakDescriptionStart = _vm->_time->current();
}

// copied from elevator.cpp code
void Spinner::resetDescription() {
	_actorId = -1;
	_sentenceId = -1;
	_timeSpeakDescriptionStart = 0u;
}

// copied from elevator.cpp code
void Spinner::tickDescription() {
	uint32 now = _vm->_time->current();
	// unsigned difference is intentional
	if (_actorId <= 0 || (now - _timeSpeakDescriptionStart < 600u)) {
		return;
	}

	if (!_vm->_mouse->isDisabled()) {
		// mouse can still move when "disabled", so hover callbacks will work while the cursor is invisible,
		// so postpone the speech until mouse is visible again
		_vm->_actors[_actorId]->speechPlay(_sentenceId, false);
		_actorId = -1;
		_sentenceId = -1;
	}
}

} // End of namespace BladeRunner
