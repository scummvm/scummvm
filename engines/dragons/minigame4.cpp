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
#include "minigame4.h"
#include "actor.h"
#include "dragons.h"
#include "dragons/dragonini.h"
#include "dragons/talk.h"
#include "dragons/inventory.h"
#include "dragons/scene.h"

namespace Dragons {

Minigame4::Minigame4(DragonsEngine *vm) : _vm(vm) {}

void Minigame4::run() {
	uint16 uVar1;
	DragonINI *uVar2;
	uint uVar4;
	ushort result;
	int16_t uVar3;

	uVar4 = _vm->getAllFlags();
	uVar3 = _vm->_inventory->getType();
	uVar2 = _vm->_dragonINIResource->getFlickerRecord();
	uVar1 = _vm->getCurrentSceneId();
	// fade_related_calls_with_1f();
	_vm->reset_screen_maybe();
	_vm->_dragonINIResource->getFlickerRecord()->sceneId = 0;
	_vm->_dragonINIResource->setFlickerRecord(NULL);
	_vm->_inventory->setType(0);
	_vm->_scene->setSceneId(7);
	_vm->_scene->loadSceneData(0x8007,0);
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_10);
	_vm->clearFlags(ENGINE_FLAG_20);
	_vm->clearFlags(ENGINE_FLAG_80);
	// DisableVSyncEvent();
	_vm->clearFlags(ENGINE_FLAG_1);
	DAT_80090428_videoUpdateRelated = 0;
	//TODO vsync_updater_function = videoUpdateFunction;
	//load_palette_into_frame_buffer(4,DAT_8006a3f8);
	//load_palette_into_frame_buffer_2(4,1,0xff,1);
	_vm->videoFlags |= 4;
	_vm->_scene->setBgLayerPriority(2);
	_vm->_scene->setMgLayerPriority(1);
	flickerActor = _vm->_actorManager->loadActor(0x18,0,0xcb,0x79,1);
	bruteActor = _vm->_actorManager->loadActor(0x17,0,0x68,0x7b,1);
	ps1ControllerActor = _vm->_actorManager->loadActor(0x17,0x16,0x9f,0x19,1);
	DAT_80090438 = _vm->_actorManager->loadActor(0x17,0xb,400,400,1);
	DAT_8009043c = _vm->_actorManager->loadActor(0x17,0xb,400,400,1);
	//EnableVSyncEvent();
	flickerActor->setFlag(ACTOR_FLAG_80);
	flickerActor->setFlag(ACTOR_FLAG_100);
	flickerActor->setFlag(ACTOR_FLAG_200);
	flickerActor->priorityLayer = 3;
	bruteActor->setFlag(ACTOR_FLAG_80);
	bruteActor->setFlag(ACTOR_FLAG_100);
	bruteActor->setFlag(ACTOR_FLAG_200);
	bruteActor->priorityLayer = 3;
	//DAT_800830e0_soundRelated = 0xf;
	//UnkSoundFunc5(0xf);
	//call_fade_related_1f();
	if (_vm->_dragonINIResource->getRecord(0x1f5)->field_12 == 3) {
		actorTalk(bruteActor,0x3321,0x4A84);
	}
	else {
		actorTalk(bruteActor,0x3321,0x49A2);
		actorTalk(flickerActor,0,0x4A56);
	}
	result = runDanceBattle();
	/* field_0x12 */
	_vm->_dragonINIResource->getRecord(0)->field_12 = result ^ 1;
	if (_vm->_dragonINIResource->getRecord(0)->field_12 == 1) {
		_vm->_dragonINIResource->getRecord(0x1f5)->sceneId = 0;
	}

	_vm->waitForFrames(2 * 0x3c);
//	fade_related_calls_with_1f();
	//DisableVSyncEvent();
	//vsync_updater_function = (code *)0x0;
	_vm->setFlags(ENGINE_FLAG_1);
	_vm->videoFlags &= ~(uint16)4;
	// EnableVSyncEvent();
	_vm->_dragonINIResource->setFlickerRecord(uVar2);
	_vm->_inventory->setType(uVar3);
//	load_palette_into_frame_buffer
//			(4,(uint)*(ushort *)
//					(*(int *)(&DAT_80071c30 + (uint)actors[0].actorFileDictionaryIndex * 8) + 10)
//			   + *(int *)(&DAT_80071c30 + (uint)actors[0].actorFileDictionaryIndex * 8));
//	load_palette_into_frame_buffer_2(4,1,0xff,1);
	_vm->_scene->setSceneId(uVar1);
	_vm->setAllFlags(uVar4);
	uVar2->sceneId = uVar1;
	_vm->_scene->loadScene(uVar1,0x1e);
}

void Minigame4::actorTalk(Actor *actorId,ushort param_2,uint32 textIndex)
{
	actorId->waitUntilFlag8SetThenSet1000AndWaitFor4();
	if (actorId == bruteActor) {
		bruteActor->updateSequence(9);
	}
	else {
		flickerActor->updateSequence(9);
	}

	actorDialog(actorId, (uint)param_2, textIndex);
	actorId->waitUntilFlag8SetThenSet1000AndWaitFor4();
	if (actorId == bruteActor) {
		bruteActor->updateSequence(0);
	}
	else {
		flickerActor->updateSequence(0);
	}
}

void Minigame4::actorDialog(Actor *actorId, ushort param_2, uint32 textIndex) {
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
			actorTalk(bruteActor,0x3321, 0x4D50);
			return 1;
		}
		currentStep = currentStep + 1;
	}
	resetActors();
	actorTalk(bruteActor,0x3321, 0x4ADE);
	currentStep = 0;
	while (currentStep < 0xc) {
		if (singleDanceRound(round2StepPositionTbl[(uint)currentStep], round2DurationTbl[(uint)currentStep])) {
			actorTalk(bruteActor,0x3321,0x4DD4);
			return 1;
		}
		currentStep = currentStep + 1;
	}
	resetActors();
	actorTalk(bruteActor,0x3321, 0x4B6A);
	currentStep = 0;
	while( true ) {
		if (0x11 < currentStep) {
			_vm->_talk->loadText(0x4C0C, auStack2192, 1000);
			_vm->_talk->displayDialogAroundPoint(auStack2192, 0x27,0xc,0x3321,0,0x4C0C);
			_vm->waitForFrames(0x10a);
			bruteActor->updateSequence(8);
			//TODO
//			if ((((DAT_8008e7e8 != 0) || (DAT_8008e848 != 0)) || (DAT_8008e844 != 0)) ||
//				(DAT_8008e874 != 0)) {
//				FUN_8001a7c4((uint)DAT_8008e7e8,(uint)DAT_8008e844,(uint)DAT_8008e848,(uint)DAT_8008e874);
//			}
			flickerActor->waitUntilFlag8SetThenSet1000AndWaitFor4();
			flickerActor->updateSequence(7);
			actorTalk(flickerActor,0, 0x4CC8);
			return 0;
		}

		if (singleDanceRound(round3StepPositionTbl[(uint)currentStep], round3DurationTbl[(uint)currentStep])) {
			break;
		}
		currentStep = currentStep + 1;
	}
	actorTalk(bruteActor,0x3321, 0x4DEE);
	return 1;
}

const static uint16 xDancePosTbl[] = { 0xAC, 0xB5, 0xBC, 0xB3, 0xB4, 0xAF };
const static uint16 yDancePosTbl[] = { 0x1C, 0x23, 0x1A, 0x14, 0x12, 0xF };

const static uint16 uint16_t_ARRAY_80090400[] = { 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
const static uint16 uint16_t_ARRAY_800903e8[] = { 1, 2, 3, 4, 5, 6 };

uint16 Minigame4::singleDanceRound(uint16 currentDancePosition, uint16 duration) {
	DAT_80090438->x_pos = xDancePosTbl[(uint)currentDancePosition];
	DAT_80090438->y_pos = yDancePosTbl[(uint)currentDancePosition];
	DAT_80090438->updateSequence(10);
	ps1ControllerActor->updateSequence(uint16_t_ARRAY_80090400[(uint)currentDancePosition]);
	bruteActor->updateSequence(uint16_t_ARRAY_800903e8[(uint)currentDancePosition]);
	while ((flickerActor->_sequenceID != uint16_t_ARRAY_800903e8[(uint)currentDancePosition] &&
			(duration = duration + -1, duration != 0))) {
		_vm->waitForFrames(1);
		updateFlickerFromInput();
	}
	if (flickerActor->_sequenceID == uint16_t_ARRAY_800903e8[(uint)currentDancePosition]) {
		while (duration = duration + -1, duration != 0) {
			_vm->waitForFrames(1);
		}
		DAT_80090438->x_pos = xDancePosTbl[(uint)currentDancePosition];
		DAT_8009043c->x_pos = DAT_80090438->x_pos;
		DAT_80090438->y_pos = yDancePosTbl[(uint)currentDancePosition];
		DAT_8009043c->y_pos = DAT_80090438->y_pos;
		DAT_8009043c->updateSequence(0xb);
		ps1ControllerActor->updateSequence(0x16);
		return 0;
	}

	return FUN_8009009c(1);
}

void Minigame4::resetActors() {
	bruteActor->waitUntilFlag8SetThenSet1000();
	flickerActor->waitUntilFlag8SetThenSet1000();
	while (bruteActor->_sequenceID != 0 || flickerActor->_sequenceID != 0) {
		_vm->waitForFrames(1);
		if (bruteActor->_sequenceID != 0 &&
			bruteActor->isFlagSet(ACTOR_FLAG_4) &&
			bruteActor->isFlagSet(ACTOR_FLAG_8)) {
			bruteActor->updateSequence(0);
		}
		if (flickerActor->_sequenceID != 0 &&
			flickerActor->isFlagSet(ACTOR_FLAG_4) &&
			flickerActor->isFlagSet(ACTOR_FLAG_8)) {
			flickerActor->updateSequence(0);
		}
	}
}

void Minigame4::updateFlickerFromInput() {
	if (_vm->isSquareButtonPressed() && flickerActor->_sequenceID != 1) {
		flickerActor->updateSequence(1);
	}

	if (_vm->isCrossButtonPressed() && flickerActor->_sequenceID != 2) {
		flickerActor->updateSequence(2);
	}

	if (_vm->isCircleButtonPressed() && flickerActor->_sequenceID != 3) {
		flickerActor->updateSequence(3);
	}

	if (_vm->isTriangleButtonPressed() && flickerActor->_sequenceID != 4) {
		flickerActor->updateSequence(4);
	}

	if (_vm->isR1ButtonPressed() && flickerActor->_sequenceID != 5) {
		flickerActor->updateSequence(5);
	}

	if (_vm->isL1ButtonPressed() && flickerActor->_sequenceID != 6) {
		flickerActor->updateSequence(6);
	}
}

uint16 Minigame4::FUN_8009009c(uint16 unk) {
	resetActors();
	if (unk == 0) {
		bruteActor->updateSequence(8);
		flickerActor->updateSequence(7);
	}
	else {
		bruteActor->updateSequence(7);
		flickerActor->updateSequence(8);
	}
	do {
		do {
		} while (bruteActor->isFlagSet(ACTOR_FLAG_4));

	} while (flickerActor->isFlagSet(ACTOR_FLAG_4));
	return (uint)unk;
}


} // End of namespace Dragons
