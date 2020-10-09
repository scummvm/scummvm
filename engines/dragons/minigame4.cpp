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
#include "dragons/minigame4.h"
#include "dragons/actor.h"
#include "dragons/dragons.h"
#include "dragons/dragonini.h"
#include "dragons/font.h"
#include "dragons/talk.h"
#include "dragons/inventory.h"
#include "dragons/scene.h"
#include "dragons/screen.h"
#include "dragons/sound.h"

namespace Dragons {

static void videoUpdaterFunction();

Minigame4::Minigame4(DragonsEngine *vm) : _vm(vm) {
	_flickerActor = nullptr;
	_bruteActor = nullptr;
	_ps1ControllerActor = nullptr;
	_dat_80090438 = nullptr;
	_dat_8009043c = nullptr;
	_layer2XOffset = 0;
}

void Minigame4::run() {
	uint16 uVar1;
	DragonINI *flicker;
	uint uVar4;
	uint16 result;
	InventoryState uVar3;

	uVar4 = _vm->getAllFlags();
	uVar3 = _vm->_inventory->getState();
	flicker = _vm->_dragonINIResource->getFlickerRecord();
	uVar1 = _vm->getCurrentSceneId();
	_vm->fadeToBlack();
	_vm->reset_screen_maybe();
	_vm->_dragonINIResource->getFlickerRecord()->sceneId = 0;
	_vm->_dragonINIResource->setFlickerRecord(nullptr);
	_vm->_inventory->setState(Closed);
	_vm->_scene->setSceneId(7);
	_vm->_scene->loadSceneData(0x8007, 0);
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_10);
	_vm->clearFlags(ENGINE_FLAG_20);
	_vm->clearFlags(ENGINE_FLAG_80);
	// DisableVSyncEvent();
	_vm->clearFlags(ENGINE_FLAG_1);
	_layer2XOffset = 0;
	_vm->setVsyncUpdateFunction(videoUpdaterFunction);
	_vm->_screen->loadPalette(4, _vm->_scene->getPalette());
	_vm->_screen->updatePaletteTransparency(4, 1, 0xff, true);
	_vm->_videoFlags |= 4;
	_vm->_scene->setBgLayerPriority(2);
	_vm->_scene->setMgLayerPriority(1);
	_flickerActor = _vm->_actorManager->loadActor(0x18, 0, 0xcb, 0x79, 1);
	_bruteActor = _vm->_actorManager->loadActor(0x17, 0, 0x68, 0x7b, 1);
	_ps1ControllerActor = _vm->_actorManager->loadActor(0x17, 0x16, 0x9f, 0x19, 1);
	_dat_80090438 = _vm->_actorManager->loadActor(0x17, 0xb, 400, 400, 1);
	_dat_8009043c = _vm->_actorManager->loadActor(0x17, 0xb, 400, 400, 1);
	//EnableVSyncEvent();
	_flickerActor->setFlag(ACTOR_FLAG_80);
	_flickerActor->setFlag(ACTOR_FLAG_100);
	_flickerActor->setFlag(ACTOR_FLAG_200);
	_flickerActor->_priorityLayer = 3;
	_bruteActor->setFlag(ACTOR_FLAG_80);
	_bruteActor->setFlag(ACTOR_FLAG_100);
	_bruteActor->setFlag(ACTOR_FLAG_200);
	_bruteActor->_priorityLayer = 3;
	_vm->_sound->playMusic(0xf);
	_vm->fadeFromBlack();
	if (_vm->_dragonINIResource->getRecord(0x1f5)->objectState == 3) {
		actorTalk(_bruteActor, 0x3321, 0x4A84);
	} else {
		actorTalk(_bruteActor, 0x3321, 0x49A2);
		actorTalk(_flickerActor, 0, 0x4A56);
	}
	result = runDanceBattle();
	/* field_0x12 */
	_vm->_dragonINIResource->getRecord(0)->objectState = result ^ 1;
	if (_vm->_dragonINIResource->getRecord(0)->objectState == 1) {
		_vm->_dragonINIResource->getRecord(0x1f5)->sceneId = 0;
	}

	_vm->waitForFrames(2 * 0x3c);
	_vm->fadeToBlack();
	//DisableVSyncEvent();
	_vm->setVsyncUpdateFunction(nullptr);
	_vm->setFlags(ENGINE_FLAG_1);
	_vm->_videoFlags &= ~(uint16)4;
	// EnableVSyncEvent();
	_vm->_dragonINIResource->setFlickerRecord(flicker);
	_vm->_inventory->setState(uVar3);
//	_vm->_screen->loadPalette(4, (uint)*(uint16 *)
//					(*(int *)(&DAT_80071c30 + (uint)actors[0].actorFileDictionaryIndex * 8) + 10)
//			   + *(int *)(&DAT_80071c30 + (uint)actors[0].actorFileDictionaryIndex * 8));
	_vm->_screen->updatePaletteTransparency(4, 1, 0xff, true);
	_vm->_scene->setSceneId(uVar1);
	_vm->setAllFlags(uVar4);
	flicker->sceneId = uVar1;
	_vm->_scene->loadScene(uVar1, 0x1e);
}

void Minigame4::actorTalk(Actor *actorId, uint16 param_2, uint32 textIndex) {
	actorId->waitUntilFlag8SetThenSet1000AndWaitFor4();
	if (actorId == _bruteActor) {
		_bruteActor->updateSequence(9);
	} else {
		_flickerActor->updateSequence(9);
	}

	actorDialog(actorId, (uint)param_2, _vm->getDialogTextId(textIndex));
	actorId->waitUntilFlag8SetThenSet1000AndWaitFor4();
	if (actorId == _bruteActor) {
		_bruteActor->updateSequence(0);
	} else {
		_flickerActor->updateSequence(0);
	}
}

void Minigame4::actorDialog(Actor *actorId, uint16 param_2, uint32 textIndex) {
	uint16 buf[1000];
	_vm->_talk->loadText(textIndex, buf, 1000);
	_vm->_talk->displayDialogAroundActor(actorId, param_2, buf, textIndex);
}

uint16 Minigame4::runDanceBattle() {
	uint16 auStack2192 [1000];
	uint16 currentStep;
	uint16 round1StepPositionTbl [12];
	uint16 round1DurationTbl [12];
	uint16 round2StepPositionTbl [12];
	uint16 round2DurationTbl [12];
	uint16 round3StepPositionTbl [20];
	uint16 round3DurationTbl [20];

	Common::File *fd = new Common::File();
	if (!fd->open("arc4.bin")) {
		error("Failed to open arc4.bin");
	}

	for (int i = 0; i < 0xc; i++) {
		round1StepPositionTbl[i] = fd->readUint16LE();
	}
	for (int i = 0; i < 0xc; i++) {
		round1DurationTbl[i] = fd->readUint16LE();
	}
	for (int i = 0; i < 0xc; i++) {
		round2StepPositionTbl[i] = fd->readUint16LE();
	}
	for (int i = 0; i < 0xc; i++) {
		round2DurationTbl[i] = fd->readUint16LE();
	}
	for (int i = 0; i < 0x12; i++) {
		round3StepPositionTbl[i] = fd->readUint16LE();
	}
	for (int i = 0; i < 0x12; i++) {
		round3DurationTbl[i] = fd->readUint16LE();
	}
	fd->close();
	delete fd;

	currentStep = 0;
	while (currentStep < 0xc) {
		if (singleDanceRound(round1StepPositionTbl[(uint)currentStep], round1DurationTbl[(uint)currentStep])) {
			actorTalk(_bruteActor, 0x3321, 0x4D50);
			return 1;
		}
		currentStep = currentStep + 1;
	}
	resetActors();
	actorTalk(_bruteActor, 0x3321, 0x4ADE);
	currentStep = 0;
	while (currentStep < 0xc) {
		if (singleDanceRound(round2StepPositionTbl[(uint)currentStep], round2DurationTbl[(uint)currentStep])) {
			actorTalk(_bruteActor, 0x3321, 0x4DD4);
			return 1;
		}
		currentStep = currentStep + 1;
	}
	resetActors();
	actorTalk(_bruteActor, 0x3321, 0x4B6A);
	currentStep = 0;
	while (true) {
		if (0x11 < currentStep) {
			uint32 textId = _vm->getDialogTextId(0x4C0C);
			_vm->_talk->loadText(textId, auStack2192, 1000);
			_vm->_talk->displayDialogAroundPoint(auStack2192, 0x27, 0xc, 0x3321, 0, textId);
			_vm->waitForFrames(0x10a);
			_bruteActor->updateSequence(8);
			_vm->_fontManager->clearText();
			_flickerActor->waitUntilFlag8SetThenSet1000AndWaitFor4();
			_flickerActor->updateSequence(7);
			actorTalk(_flickerActor, 0, 0x4CC8);
			return 0;
		}

		if (singleDanceRound(round3StepPositionTbl[(uint)currentStep], round3DurationTbl[(uint)currentStep])) {
			break;
		}
		currentStep = currentStep + 1;
	}
	actorTalk(_bruteActor, 0x3321, 0x4DEE);
	return 1;
}

const static uint16 xDancePosTbl[6] = { 0xAC, 0xB5, 0xBC, 0xB3, 0xB4, 0xAF };
const static uint16 yDancePosTbl[6] = { 0x1C, 0x23, 0x1A, 0x14, 0x12, 0xF };

const static uint16 uint16_ARRAY_80090400[6] = { 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
const static uint16 uint16_ARRAY_800903e8[6] = { 1, 2, 3, 4, 5, 6 };

uint16 Minigame4::singleDanceRound(uint16 currentDancePosition, uint16 duration) {
	_dat_80090438->_x_pos = xDancePosTbl[(uint)currentDancePosition];
	_dat_80090438->_y_pos = yDancePosTbl[(uint)currentDancePosition];
	_dat_80090438->updateSequence(10);
	_ps1ControllerActor->updateSequence(uint16_ARRAY_80090400[(uint)currentDancePosition]);
	_bruteActor->updateSequence(uint16_ARRAY_800903e8[(uint)currentDancePosition]);
	while ((_flickerActor->_sequenceID != uint16_ARRAY_800903e8[(uint)currentDancePosition] &&
			(duration = duration + -1, duration != 0))) {
		_vm->waitForFrames(1);
		updateFlickerFromInput();
	}
	if (_flickerActor->_sequenceID == uint16_ARRAY_800903e8[(uint)currentDancePosition]) {
		while (duration = duration + -1, duration != 0) {
			_vm->waitForFrames(1);
		}
		_dat_80090438->_x_pos = xDancePosTbl[(uint)currentDancePosition];
		_dat_8009043c->_x_pos = _dat_80090438->_x_pos;
		_dat_80090438->_y_pos = yDancePosTbl[(uint)currentDancePosition];
		_dat_8009043c->_y_pos = _dat_80090438->_y_pos;
		_dat_8009043c->updateSequence(0xb);
		_ps1ControllerActor->updateSequence(0x16);
		return 0;
	}

	return fun_8009009c(1);
}

void Minigame4::resetActors() {
	_bruteActor->waitUntilFlag8SetThenSet1000();
	_flickerActor->waitUntilFlag8SetThenSet1000();
	while (_bruteActor->_sequenceID != 0 || _flickerActor->_sequenceID != 0) {
		_vm->waitForFrames(1);
		if (_bruteActor->_sequenceID != 0 &&
			_bruteActor->isFlagSet(ACTOR_FLAG_4) &&
			_bruteActor->isFlagSet(ACTOR_FLAG_8)) {
			_bruteActor->updateSequence(0);
		}
		if (_flickerActor->_sequenceID != 0 &&
			_flickerActor->isFlagSet(ACTOR_FLAG_4) &&
			_flickerActor->isFlagSet(ACTOR_FLAG_8)) {
			_flickerActor->updateSequence(0);
		}
	}
}

void Minigame4::updateFlickerFromInput() {
	if (_vm->isSquareButtonPressed() && _flickerActor->_sequenceID != 1) {
		_flickerActor->updateSequence(1);
	}

	if (_vm->isCrossButtonPressed() && _flickerActor->_sequenceID != 2) {
		_flickerActor->updateSequence(2);
	}

	if (_vm->isCircleButtonPressed() && _flickerActor->_sequenceID != 3) {
		_flickerActor->updateSequence(3);
	}

	if (_vm->isTriangleButtonPressed() && _flickerActor->_sequenceID != 4) {
		_flickerActor->updateSequence(4);
	}

	if (_vm->isR1ButtonPressed() && _flickerActor->_sequenceID != 5) {
		_flickerActor->updateSequence(5);
	}

	if (_vm->isL1ButtonPressed() && _flickerActor->_sequenceID != 6) {
		_flickerActor->updateSequence(6);
	}
}

uint16 Minigame4::fun_8009009c(uint16 unk) {
	resetActors();
	if (unk == 0) {
		_bruteActor->updateSequence(8);
		_flickerActor->updateSequence(7);
	} else {
		_bruteActor->updateSequence(7);
		_flickerActor->updateSequence(8);
	}
	do {
		do {
		} while (_bruteActor->isFlagSet(ACTOR_FLAG_4));

	} while (_flickerActor->isFlagSet(ACTOR_FLAG_4));
	return (uint)unk;
}

void videoUpdaterFunction() {
	static uint16 layer2XOffset = 0;
	static uint16 layer0XOffset = 0;
	DragonsEngine *vm = getEngine();
	vm->_scene->setLayerOffset(2, Common::Point(layer2XOffset, 0));
	layer2XOffset = (layer2XOffset + 3) % 512;
	vm->_scene->setLayerOffset(0, Common::Point(layer0XOffset, 0));
	layer0XOffset = (layer0XOffset + 4) % 512;
}


} // End of namespace Dragons
