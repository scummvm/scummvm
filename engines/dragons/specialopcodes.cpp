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

#include "dragons/actorresource.h"
#include "dragons/background.h"
#include "dragons/cursor.h"
#include "dragons/cutscene.h"
#include "dragons/credits.h"
#include "dragons/dragons.h"
#include "dragons/dragonflg.h"
#include "dragons/dragonini.h"
#include "dragons/dragonobd.h"
#include "dragons/inventory.h"
#include "dragons/specialopcodes.h"
#include "dragons/scene.h"
#include "dragons/sound.h"
#include "dragons/actor.h"
#include "dragons/minigame1.h"
#include "dragons/talk.h"
#include "dragons/screen.h"
#include "dragons/minigame2.h"
#include "dragons/minigame3.h"
#include "dragons/minigame4.h"
#include "dragons/minigame5.h"


namespace Dragons {

const int16 shakeTbl[16] = {
	5,      2,     -2,     -5,
	5,      2,     -2,     -5,
	-1,      2,     -1,     -1,
	2,     -1,      1,      0
};

// SpecialOpcodes

SpecialOpcodes::SpecialOpcodes(DragonsEngine *vm)
	: _vm(vm), _specialOpCounter(0) {
	_dat_80083148 = 0;
	_uint16_t_80083154 = 0;
	initOpcodes();
}

SpecialOpcodes::~SpecialOpcodes() {
	freeOpcodes();
}

void SpecialOpcodes::run(int16 op) {
	if (!_opcodes[op])
		error("SpecialOpcodes::execOpcode() Unimplemented opcode %d (0x%X)", op, op);
	debug(3, "run(%d) %s", op, _opcodeNames[op].c_str());
	(*_opcodes[op])();
}

typedef Common::Functor0Mem<void, SpecialOpcodes> SpecialOpcodeI;
#define OPCODE(op, func) \
	_opcodes[op] = new SpecialOpcodeI(this, &SpecialOpcodes::func); \
	_opcodeNames[op] = #func;

void SpecialOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < DRAGONS_NUM_SPECIAL_OPCODES; ++i) {
		_opcodes[i] = 0;
	}
	// Register opcodes
	OPCODE(1, spcCatapultMiniGame);
	OPCODE(2, spcThumbWrestlingMiniGame);
	OPCODE(3, spcClearEngineFlag10);
	OPCODE(4, spcSetEngineFlag10);

	OPCODE(6, spcRabbitsMiniGame);
	OPCODE(7, spcDancingMiniGame);
	OPCODE(8, spcCastleGardenLogic);
	OPCODE(9, spcUnk9);
	OPCODE(0xa, spcUnkA);
	OPCODE(0xb, clearSceneUpdateFunction);
	OPCODE(0xc, spcUnkC);
	OPCODE(0xd, spcFadeScreen);
	OPCODE(0xe, spcLadyOfTheLakeCapturedSceneLogic);
	OPCODE(0xf, spcLadyOfTheLakeCapturedSceneLogic);
	OPCODE(0x10, spcStopLadyOfTheLakeCapturedSceneLogic);
	OPCODE(0x11, spc11ShakeScreen);
	OPCODE(0x12, spcHandleInventionBookTransition);
	OPCODE(0x13, spcUnk13InventionBookCloseRelated);
	OPCODE(0x14, spcClearEngineFlag8);
	OPCODE(0x15, spcSetEngineFlag8);
	OPCODE(0x16, spcNoop1);
	OPCODE(0x17, spcKnightPoolReflectionLogic);
	OPCODE(0x18, clearSceneUpdateFunction);
	OPCODE(0x19, spcWalkOnStilts);
	OPCODE(0x1a, spcActivatePizzaMakerActor);
	OPCODE(0x1b, spcDeactivatePizzaMakerActor);
	OPCODE(0x1c, spcPizzaMakerActorStopWorking);
	OPCODE(0x1d, spcDragonArrivesAtTournament);
	OPCODE(0x1e, spcDragonCatapultMiniGame);
	OPCODE(0x1f, spcStGeorgeDragonLanded);

	OPCODE(0x21, spcSetEngineFlag0x20000);
	OPCODE(0x22, spcClearEngineFlag0x20000);
	OPCODE(0x23, spcSetEngineFlag0x200000);
	OPCODE(0x24, spcClearEngineFlag0x200000);
	OPCODE(0x25, spcFlickerSetPriority2);
	OPCODE(0x26, spcMenInMinesSceneLogic);
	OPCODE(0x27, spcStopMenInMinesSceneLogic);
	OPCODE(0x28, spcMonksAtBarSceneLogic);
	OPCODE(0x29, spcStopMonksAtBarSceneLogic);

	OPCODE(0x2b, spcFlameBedroomEscapeSceneLogic);
	OPCODE(0x2c, spcStopFlameBedroomEscapeSceneLogic);

	OPCODE(0x2e, spcCastleMoatFull);
	OPCODE(0x2f, spcCastleRestoreScalePoints);
	OPCODE(0x30, spcCastleMoatUpdateActorSceneScalePoints);
	OPCODE(0x31, spcCastleGateMoatDrainedSceneLogic);

	OPCODE(0x34, spcUnk34);

	OPCODE(0x36, spcFlickerClearFlag0x80);

	OPCODE(0x38, spcNoop1);
	OPCODE(0x39, spcTownAngryVillagersSceneLogic);
	OPCODE(0x3a, spcBlackDragonCrashThroughGate);
	OPCODE(0x3b, spcSetEngineFlag0x2000000);
	OPCODE(0x3c, spcClearEngineFlag0x2000000);
	OPCODE(0x3d, clearSceneUpdateFunction);
	OPCODE(0x3e, spcZigmondFraudSceneLogic);
	OPCODE(0x3f, clearSceneUpdateFunction);
	OPCODE(0x40, spcZigmondFraudSceneLogic1);
	OPCODE(0x41, spcBrokenBlackDragonSceneLogic);
	OPCODE(0x42, spcDodoUnderAttackSceneLogic);
	OPCODE(0x43, spcForestWithoutDodoSceneLogic);

	OPCODE(0x46, spcBlackDragonOnHillSceneLogic);

	OPCODE(0x48, spcHedgehogTest);
	OPCODE(0x49, spcLoadScene1);

	OPCODE(0x4b, spcKnightsSavedCastleCutScene);
	OPCODE(0x4c, spcFlickerReturnsCutScene);
	OPCODE(0x4d, spcKnightsSavedAgainCutScene);
	OPCODE(0x4e, spcUnk4e);
	OPCODE(0x4f, spcUnk4f);
	OPCODE(0x50, spcCloseInventory);
	OPCODE(0x51, spcOpenInventionBook);
	OPCODE(0x52, spcCloseInventionBook);
	OPCODE(0x53, spcClearEngineFlag0x4000000);
	OPCODE(0x54, spcSetEngineFlag0x4000000);
	OPCODE(0x55, spcSetCursorSequenceIdToZero);

	OPCODE(0x5b, spcFlickerSetFlag0x80);

	OPCODE(0x5d, spcUnk5d); //knights in castle garden
	OPCODE(0x5e, spcUnk5e);
	OPCODE(0x5f, spcUnk5f);

	OPCODE(0x61, spcCastleBuildBlackDragonSceneLogic);
	OPCODE(0x62, spcStopSceneUpdateFunction)
	OPCODE(0x63, spcSetInventorySequenceTo5);
	OPCODE(0x64, spcResetInventorySequence);
	OPCODE(0x65, spcUnk65ScenePaletteRelated);
	OPCODE(0x66, spcUnk66);
	OPCODE(0x67, spcTournamentSetCamera);
	OPCODE(0x68, spcTournamentCutScene);
	OPCODE(0x69, spcInsideBlackDragonUpdatePalette);
	OPCODE(0x6a, spcCastleGateSceneLogic);
	OPCODE(0x6b, spcTransitionToMap);
	OPCODE(0x6c, spcTransitionFromMap);
	OPCODE(0x6d, spcCaveOfDilemmaSceneLogic);

	OPCODE(0x70, spcLoadLadyOfTheLakeActor);
	OPCODE(0x71, spcFadeCreditsToBackStageScene);
	OPCODE(0x72, spcRunCredits);
	OPCODE(0x73, spcEndCreditsAndRestartGame);
	OPCODE(0x74, spcUseClickerOnLever);

	OPCODE(0x77, spcJesterInLibrarySceneLogic);

	OPCODE(0x79, spcSetUnkFlag2);
	OPCODE(0x7a, spcBlackDragonDialogForCamelhot);
	OPCODE(0x7b, spcSetCameraXToZero);
	OPCODE(0x7c, spcDiamondIntroSequenceLogic);
	OPCODE(0x7d, spcLoadFileS10a6act);
	OPCODE(0x7e, spcLoadFileS10a7act);
	OPCODE(0x7f, spcFlickerPutOnStGeorgeArmor);
	OPCODE(0x80, spcUnk80FlickerArmorOn);
	OPCODE(0x81, spcShakeScreenSceneLogic);
	OPCODE(0x82, spcClearTextFromScreen);
	OPCODE(0x83, spcStopScreenShakeUpdater);
	OPCODE(0x84, spcInsideBlackDragonScreenShake);
	OPCODE(0x85, spc85SetScene1To0x35);
	OPCODE(0x86, spc86SetScene1To0x33);
	OPCODE(0x87, spc87SetScene1To0x17);
	OPCODE(0x88, spc88SetScene1To0x16);
	OPCODE(0x89, spcSetUnkFlag2);
	OPCODE(0x8a, spcClearUnkFlag2);
	OPCODE(0x8b, spcUnk8b);
}

#undef OPCODE

void SpecialOpcodes::freeOpcodes() {
	for (uint i = 0; i < DRAGONS_NUM_SPECIAL_OPCODES; ++i) {
		delete _opcodes[i];
	}
}

// Opcodes

void SpecialOpcodes::spcCatapultMiniGame() {
	Minigame1 minigame1(_vm);
	minigame1.run();
}

void SpecialOpcodes::spcThumbWrestlingMiniGame() {
	Minigame2 minigame2(_vm);
	minigame2.run(0, 1, 1);
}

void SpecialOpcodes::spcClearEngineFlag10() {
	_vm->clearFlags(ENGINE_FLAG_10);
}

void SpecialOpcodes::spcSetEngineFlag10() {
	_vm->setFlags(ENGINE_FLAG_10);
}

void SpecialOpcodes::spcRabbitsMiniGame() {
	Minigame3 minigame3(_vm);
	minigame3.run();
}

void SpecialOpcodes::spcDancingMiniGame() {
	Minigame4 minigame4(_vm);
	minigame4.run();
}

void SpecialOpcodes::spcCastleGardenLogic() {
	sceneUpdater.textTbl[0][0] = _vm->getDialogTextId(0x22660);
	sceneUpdater.textTbl[0][1] = _vm->getDialogTextId(0x226CA);
	sceneUpdater.textTbl[1][0] = _vm->getDialogTextId(0x22738);
	sceneUpdater.textTbl[1][1] = _vm->getDialogTextId(0x22790);
	sceneUpdater.textTbl[2][0] = _vm->getDialogTextId(0x227E8);
	sceneUpdater.textTbl[2][1] = _vm->getDialogTextId(0x2283C);
	sceneUpdater.textTbl[3][0] = _vm->getDialogTextId(0x228A0);
	sceneUpdater.textTbl[3][1] = _vm->getDialogTextId(0x228EC);
	if (_vm->getCurrentSceneId() == 0x1a) {
		sceneUpdater.sequenceIDTbl[0][0] = 1;
		sceneUpdater.sequenceIDTbl[1][0] = 1;
		sceneUpdater.sequenceIDTbl[2][0] = 1;
		sceneUpdater.sequenceIDTbl[3][0] = 1;
		sceneUpdater.iniIDTbl[3][0] = 0x145;
		sceneUpdater.iniIDTbl[4][0] = 0x144;
	} else {
		sceneUpdater.sequenceIDTbl[0][0] = 0xffff;
		sceneUpdater.sequenceIDTbl[1][0] = 0xffff;
		sceneUpdater.sequenceIDTbl[2][0] = 0xffff;
		sceneUpdater.sequenceIDTbl[3][0] = 0xffff;
		sceneUpdater.iniIDTbl[3][0] = 0x83;
		sceneUpdater.iniIDTbl[4][0] = 0x74;
	}
	sceneUpdater.sequenceIDTbl[3][1] = 1;
	sceneUpdater.sequenceIDTbl[2][1] = 1;
	sceneUpdater.sequenceIDTbl[1][1] = 1;
	sceneUpdater.sequenceIDTbl[0][1] = 1;
	sceneUpdater.numSteps[0] = 2;
	sceneUpdater.numSteps[1] = 2;
	sceneUpdater.numSteps[2] = 2;
	sceneUpdater.numSteps[3] = 2;
	sceneUpdater.iniIDTbl[0][0] = sceneUpdater.iniIDTbl[3][0];
	sceneUpdater.iniIDTbl[0][1] = sceneUpdater.iniIDTbl[4][0];
	sceneUpdater.iniIDTbl[1][0] = sceneUpdater.iniIDTbl[3][0];
	sceneUpdater.iniIDTbl[1][1] = sceneUpdater.iniIDTbl[4][0];
	sceneUpdater.iniIDTbl[2][0] = sceneUpdater.iniIDTbl[3][0];
	sceneUpdater.iniIDTbl[2][1] = sceneUpdater.iniIDTbl[4][0];
	sceneUpdater.iniIDTbl[3][1] = sceneUpdater.iniIDTbl[4][0];
	setupTableBasedSceneUpdateFunction(1, 4, 0xb4);
}

void SpecialOpcodes::spcUnk9() {
	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	assert(flicker);
	flicker->flags |= INI_FLAG_20;
	assert(flicker->actor);
	flicker->actor->setFlag(ACTOR_FLAG_100);
	flicker->actor->_priorityLayer = 0;
	_vm->getINI(1)->flags |= INI_FLAG_20;
}


void SpecialOpcodes::spcUnkA() {
	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	flicker->flags &= ~INI_FLAG_20;
	flicker->actor->clearFlag(ACTOR_FLAG_100);
	_vm->getINI(1)->flags &= ~INI_FLAG_20;
}

void SpecialOpcodes::spcUnkC() {
	_vm->fadeToBlack();
}

void SpecialOpcodes::spcFadeScreen() {
	_vm->fadeFromBlack();
}

void SpecialOpcodes::spcLadyOfTheLakeCapturedSceneLogic() {
	_dat_80083148 = 0;
	_uint16_t_80083154 = 0;
	_vm->setSceneUpdateFunction(ladyOfTheLakeCapturedUpdateFunction);
}

void SpecialOpcodes::spcStopLadyOfTheLakeCapturedSceneLogic() {
	_vm->clearSceneUpdateFunction();
	_vm->_sound->resumeMusic();
	if ((_dat_80083148 != 0) || (_uint16_t_80083154 != 0)) {
		//TODO FUN_8001ac5c((uint)_dat_80083148, (uint)DAT_80083150, (uint)_uint16_t_80083154, (uint)DAT_80083158);
	}
	_dat_80083148 = 0;
	_uint16_t_80083154 = 0;
}

void SpecialOpcodes::spc11ShakeScreen() {
	for (int i = 0; i < 16; i++) {
		_vm->_screen->setScreenShakeOffset(0, shakeTbl[i]);
		_vm->waitForFrames(1);
	}
}

void SpecialOpcodes::spcHandleInventionBookTransition() {
	int16 invType = _vm->_inventory->getState();
	if (invType == InventoryOpen) {
		_vm->_inventory->closeInventory();
		_vm->_inventory->setState(Closed);
	}
	if (invType == InventionBookOpen) {
		_vm->_inventory->closeInventionBook();
		_vm->_inventory->setState(Closed);
	}
	_vm->_cursor->updateSequenceID(1);
	_vm->setFlags(ENGINE_FLAG_400);
	_vm->clearFlags(ENGINE_FLAG_10);
}

void SpecialOpcodes::spcUnk13InventionBookCloseRelated() {
	_vm->clearFlags(ENGINE_FLAG_400);
	_vm->setFlags(ENGINE_FLAG_10);
}

void SpecialOpcodes::spcClearEngineFlag8() {
	_vm->clearFlags(ENGINE_FLAG_8);
}

void SpecialOpcodes::spcSetEngineFlag8() {
	_vm->setFlags(ENGINE_FLAG_8);
}

void SpecialOpcodes::spcKnightPoolReflectionLogic() {
	sceneUpdater.textTbl[0][0] = _vm->getDialogTextId(0x23E90);
	sceneUpdater.textTbl[0][1] = _vm->getDialogTextId(0x23EE6);
	sceneUpdater.textTbl[0][2] = _vm->getDialogTextId(0x23F0C);
	sceneUpdater.textTbl[0][3] = _vm->getDialogTextId(0x23F86);
	sceneUpdater.textTbl[1][0] = _vm->getDialogTextId(0x24000);
	sceneUpdater.textTbl[1][1] = _vm->getDialogTextId(0x2406A);
	sceneUpdater.textTbl[2][0] = _vm->getDialogTextId(0x240C2);
	sceneUpdater.textTbl[2][1] = _vm->getDialogTextId(0x2411E);
	sceneUpdater.textTbl[2][2] = _vm->getDialogTextId(0x24158);
	sceneUpdater.textTbl[2][3] = _vm->getDialogTextId(0x241BC);
	sceneUpdater.textTbl[2][4] = _vm->getDialogTextId(0x241EE);
	sceneUpdater.textTbl[3][0] = _vm->getDialogTextId(0x24240);
	sceneUpdater.textTbl[3][1] = _vm->getDialogTextId(0x24286);
	sceneUpdater.textTbl[3][2] = _vm->getDialogTextId(0x242B0);
	sceneUpdater.textTbl[3][3] = _vm->getDialogTextId(0x2431C);
	sceneUpdater.textTbl[4][0] = _vm->getDialogTextId(0x2437C);
	sceneUpdater.textTbl[4][1] = _vm->getDialogTextId(0x243B2);
	sceneUpdater.textTbl[4][2] = _vm->getDialogTextId(0x2440A);
	sceneUpdater.textTbl[4][3] = _vm->getDialogTextId(0x24432);
	sceneUpdater.textTbl[5][0] = _vm->getDialogTextId(0x24480);
	sceneUpdater.textTbl[5][1] = _vm->getDialogTextId(0x244EE);
	sceneUpdater.textTbl[5][2] = _vm->getDialogTextId(0x2453C);

	sceneUpdater.iniIDTbl[0][0] = 0x155;
	sceneUpdater.iniIDTbl[0][1] = 0x14B;
	sceneUpdater.iniIDTbl[0][2] = 0x155;
	sceneUpdater.iniIDTbl[0][3] = 0x14B;
	sceneUpdater.iniIDTbl[1][0] = 0x155;
	sceneUpdater.iniIDTbl[1][1] = 0x14B;
	sceneUpdater.iniIDTbl[2][0] = 0x155;
	sceneUpdater.iniIDTbl[2][1] = 0x14B;
	sceneUpdater.iniIDTbl[2][2] = 0x155;
	sceneUpdater.iniIDTbl[2][3] = 0x14B;
	sceneUpdater.iniIDTbl[2][4] = 0x155;
	sceneUpdater.iniIDTbl[3][0] = 0x155;
	sceneUpdater.iniIDTbl[3][1] = 0x14B;
	sceneUpdater.iniIDTbl[3][2] = 0x155;
	sceneUpdater.iniIDTbl[3][3] = 0x14B;
	sceneUpdater.iniIDTbl[4][0] = 0x14B;
	sceneUpdater.iniIDTbl[4][1] = 0x155;
	sceneUpdater.iniIDTbl[4][2] = 0x14B;
	sceneUpdater.iniIDTbl[4][3] = 0x155;
	sceneUpdater.iniIDTbl[5][0] = 0x155;
	sceneUpdater.iniIDTbl[5][1] = 0x14B;
	sceneUpdater.iniIDTbl[5][2] = 0x155;
	sceneUpdater.sequenceIDTbl[0][0] = 10;
	sceneUpdater.sequenceIDTbl[0][1] = 2;
	sceneUpdater.sequenceIDTbl[0][2] = 10;
	sceneUpdater.sequenceIDTbl[0][3] = 2;
	sceneUpdater.sequenceIDTbl[1][0] = 10;
	sceneUpdater.sequenceIDTbl[1][1] = 2;
	sceneUpdater.sequenceIDTbl[2][0] = 10;
	sceneUpdater.sequenceIDTbl[2][1] = 2;
	sceneUpdater.sequenceIDTbl[2][2] = 10;
	sceneUpdater.sequenceIDTbl[2][3] = 2;
	sceneUpdater.sequenceIDTbl[2][4] = 10;
	sceneUpdater.sequenceIDTbl[3][0] = 10;
	sceneUpdater.sequenceIDTbl[3][1] = 2;
	sceneUpdater.sequenceIDTbl[3][2] = 10;
	sceneUpdater.sequenceIDTbl[3][3] = 2;
	sceneUpdater.sequenceIDTbl[4][0] = 2;
	sceneUpdater.sequenceIDTbl[4][1] = 10;
	sceneUpdater.sequenceIDTbl[4][2] = 2;
	sceneUpdater.sequenceIDTbl[4][3] = 10;
	sceneUpdater.sequenceIDTbl[5][0] = 10;
	sceneUpdater.sequenceIDTbl[5][2] = 10;
	sceneUpdater.numSteps[2] = 5;
	sceneUpdater.sequenceIDTbl[5][1] = 2;
	sceneUpdater.numSteps[1] = 2;
	sceneUpdater.numSteps[0] = 4;
	sceneUpdater.numSteps[3] = 4;
	sceneUpdater.numSteps[4] = 4;
	sceneUpdater.numSteps[5] = 3;
	setupTableBasedSceneUpdateFunction(0x168, 6, 300);
}

void SpecialOpcodes::spcWalkOnStilts() {
	bool isInWater = false;

	Actor *flickerOnStilts = _vm->_dragonINIResource->getRecord(0x2a0)->actor;
	Actor *waterRipples = _vm->_dragonINIResource->getRecord(0x2a9)->actor;

	flickerOnStilts->_walkSlopeX = flickerOnStilts->_walkSlopeX / 3;
	flickerOnStilts->_walkSlopeY = flickerOnStilts->_walkSlopeY / 3;

	while (flickerOnStilts->isFlagSet(ACTOR_FLAG_10)) {
		if (flickerOnStilts->_frame->field_c == 0) {
			isInWater = false;
		} else {
			if (!isInWater && flickerOnStilts->_y_pos >= 0x6a && flickerOnStilts->_y_pos < 0x96) {
				isInWater = true;
				waterRipples->_x_pos = flickerOnStilts->_x_pos - flickerOnStilts->_frame->field_e;
				waterRipples->_y_pos = flickerOnStilts->_y_pos - flickerOnStilts->_frame->field_10;
				waterRipples->updateSequence(9);
			}
		}
		_vm->waitForFrames(1);
	}
}

void SpecialOpcodes::spcActivatePizzaMakerActor() {
	_vm->setSceneUpdateFunction(pizzaUpdateFunction);
}

void SpecialOpcodes::spcDeactivatePizzaMakerActor() {
	if (_vm->getSceneUpdateFunction() == pizzaUpdateFunction) {
		_vm->clearSceneUpdateFunction();
	}
}

void SpecialOpcodes::spcPizzaMakerActorStopWorking() {
	spcDeactivatePizzaMakerActor();
	pizzaMakerStopWorking();
}

void SpecialOpcodes::spcDragonArrivesAtTournament() {
	_vm->_dragonINIResource->getRecord(0x123)->actor->setFlag(ACTOR_FLAG_400);
	_vm->_dragonINIResource->getRecord(0x124)->actor->setFlag(ACTOR_FLAG_400);
	_vm->_dragonINIResource->getRecord(0)->actor->setFlag(ACTOR_FLAG_400);
}

void SpecialOpcodes::spcDragonCatapultMiniGame() {
	Minigame5 minigame5(_vm);
	minigame5.run();
}

void SpecialOpcodes::spcStGeorgeDragonLanded() {
//	DisableVSyncEvent();
	DragonINI *ini121 = _vm->_dragonINIResource->getRecord(0x121);
	Actor *origActor = ini121->actor;
	ini121->actor = _vm->_actorManager->loadActor(0x48, 4, ini121->actor->_x_pos, ini121->actor->_y_pos);
	origActor->reset_maybe();
//TODO	reset_actor_maybe();
	ini121->actor->setFlag(ACTOR_FLAG_80);
	ini121->actor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	ini121->actor->_priorityLayer = 2;
	ini121->actorResourceId = 0x48;

	_vm->updateActorSequences();
	_vm->_scene->draw();
}

void SpecialOpcodes::spcSetEngineFlag0x20000() {
	_vm->setFlags(ENGINE_FLAG_20000);
}

void SpecialOpcodes::spcClearEngineFlag0x20000() {
	_vm->clearFlags(ENGINE_FLAG_20000);
}

void SpecialOpcodes::spcSetEngineFlag0x200000() {
	_vm->setFlags(ENGINE_FLAG_200000);
}

void SpecialOpcodes::spcClearEngineFlag0x200000() {
	_vm->clearFlags(ENGINE_FLAG_200000);
}

void SpecialOpcodes::spcFlickerSetPriority2() {
	_vm->_dragonINIResource->getFlickerRecord()->actor->_priorityLayer = 2;
}

void SpecialOpcodes::spcMenInMinesSceneLogic() {
	_vm->setSceneUpdateFunction(menInMinesSceneUpdateFunction);
	_uint16_t_80083154 = 0;
	_dat_80083148 = 0;
	setSpecialOpCounter(0);
}

void SpecialOpcodes::spcStopMenInMinesSceneLogic() {
	if (_vm->getSceneUpdateFunction() == menInMinesSceneUpdateFunction) {
		_vm->clearSceneUpdateFunction();
		if (0x3c < _specialOpCounter) {
			_specialOpCounter = 0x3c;
		}
		for (; _specialOpCounter > 0; _specialOpCounter--) {
			_vm->waitForFrames(1);
		}
		//TODO
		//FUN_8001ac5c((uint)_dat_80083148, (uint)DAT_80083150, (uint)_uint16_t_80083154, (uint)DAT_80083158);
	}
}

void SpecialOpcodes::spcMonksAtBarSceneLogic() {
	setSpecialOpCounter(-1);
	_vm->setSceneUpdateFunction(monksAtBarSceneUpdateFunction);
	_uint16_t_80083154 = 0;
	_dat_80083148 = 0;
}

void SpecialOpcodes::spcStopMonksAtBarSceneLogic() {
	if (_vm->getSceneUpdateFunction() == monksAtBarSceneUpdateFunction) {
		_vm->clearSceneUpdateFunction();
		if ((_dat_80083148 != 0) && (_uint16_t_80083154 != 0)) {
			//TODO FUN_8001ac5c((uint)_dat_80083148, (uint)DAT_80083150, (uint)_uint16_t_80083154, (uint)DAT_80083158);
		}
		setSpecialOpCounter(0);
		_uint16_t_80083154 = 0;
		_dat_80083148 = 0;
	}

}

void SpecialOpcodes::spcFlameBedroomEscapeSceneLogic() {
	setSpecialOpCounter(-1);
	if ((_dat_80083148 != 0) && (_uint16_t_80083154 != 0)) {
		//TODO FUN_8001ac5c((uint)_dat_80083148, (uint)DAT_80083150, (uint)_uint16_t_80083154, (uint)DAT_80083158);
	}
	_vm->setSceneUpdateFunction(flameEscapeSceneUpdateFunction);
	_uint16_t_80083154 = 0;
	_dat_80083148 = 0;
}

void SpecialOpcodes::spcStopFlameBedroomEscapeSceneLogic() {
	setSpecialOpCounter(0);
//	TODO FUN_8001ac5c((uint)_dat_80083148, (uint)DAT_80083150, (uint)_uint16_t_80083154, (uint)DAT_80083158);
	_uint16_t_80083154 = 0;
	_dat_80083148 = 0;
	_vm->_dragonINIResource->getRecord(0x96)->actor->updateSequence(0);
	if (_vm->getSceneUpdateFunction() == flameEscapeSceneUpdateFunction) {
		_vm->clearSceneUpdateFunction();
	}
}

void SpecialOpcodes::spcCastleMoatFull() {
	_vm->_scene->getScaleLayer()->backup();
}

void SpecialOpcodes::spcCastleRestoreScalePoints() {
	_vm->_scene->getScaleLayer()->restore();
}

void SpecialOpcodes::spcCastleMoatUpdateActorSceneScalePoints() {
	_vm->_scene->getScaleLayer()->clearAll();
	_vm->_scene->getScaleLayer()->setValue(0, 199, 7);
}

void SpecialOpcodes::spcCastleGateMoatDrainedSceneLogic() {
	setSpecialOpCounter(-1);
	if ((_dat_80083148 != 0) && (_uint16_t_80083154 != 0)) {
		//TODO FUN_8001ac5c((uint)_dat_80083148, (uint)DAT_80083150, (uint)_uint16_t_80083154, (uint)DAT_80083158);
	}
	_vm->setSceneUpdateFunction(moatDrainedSceneUpdateFunction);
	_uint16_t_80083154 = 0;
	_dat_80083148 = 0;
}
void SpecialOpcodes::spcUnk34() {
	Actor *flicker = _vm->_dragonINIResource->getFlickerRecord()->actor;
	flicker->setFlag(ACTOR_FLAG_80);
	flicker->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
}

void SpecialOpcodes::spcFlickerClearFlag0x80() {
	_vm->_dragonINIResource->getFlickerRecord()->actor->clearFlag(ACTOR_FLAG_80);
}

void SpecialOpcodes::spcNoop1() {
	//No op
}

void SpecialOpcodes::spcTownAngryVillagersSceneLogic() {
	sceneUpdater.textTbl[0][0] = _vm->getDialogTextId(0x35946);
	sceneUpdater.textTbl[0][1] = _vm->getDialogTextId(0x359BC);
	sceneUpdater.textTbl[1][0] = _vm->getDialogTextId(0x35A38);
	sceneUpdater.textTbl[1][1] = _vm->getDialogTextId(0x35ABC);
	sceneUpdater.textTbl[2][0] = _vm->getDialogTextId(0x35B28);
	sceneUpdater.textTbl[2][1] = _vm->getDialogTextId(0x35B9C);
	sceneUpdater.textTbl[3][0] = _vm->getDialogTextId(0x35C10);
	sceneUpdater.textTbl[3][1] = _vm->getDialogTextId(0x35C80);
	sceneUpdater.textTbl[4][0] = _vm->getDialogTextId(0x35CFA);
	sceneUpdater.textTbl[4][1] = _vm->getDialogTextId(0x35D64);
	sceneUpdater.iniIDTbl[0][0] = 0x1DC;
	sceneUpdater.iniIDTbl[0][1] = 0x1DD;
	sceneUpdater.iniIDTbl[1][0] = 0x1DC;
	sceneUpdater.iniIDTbl[1][1] = 0x1DD;
	sceneUpdater.iniIDTbl[2][0] = 0x1DC;
	sceneUpdater.iniIDTbl[2][1] = 0x1DD;
	sceneUpdater.iniIDTbl[3][0] = 0x1DC;
	sceneUpdater.iniIDTbl[3][1] = 0x1DD;
	sceneUpdater.iniIDTbl[4][0] = 0x1DC;
	sceneUpdater.iniIDTbl[4][1] = 0x1DD;
	sceneUpdater.sequenceIDTbl[0][0] = 0x11;
	sceneUpdater.sequenceIDTbl[0][1] = 6;
	sceneUpdater.sequenceIDTbl[1][0] = 0x11;
	sceneUpdater.sequenceIDTbl[1][1] = 6;
	sceneUpdater.sequenceIDTbl[2][0] = 0x11;
	sceneUpdater.sequenceIDTbl[2][1] = 6;
	sceneUpdater.sequenceIDTbl[3][0] = 0x11;
	sceneUpdater.sequenceIDTbl[3][1] = 6;
	sceneUpdater.sequenceIDTbl[4][0] = 0x11;
	sceneUpdater.sequenceIDTbl[4][1] = 6;
	sceneUpdater.numSteps[0] = 2;
	sceneUpdater.numSteps[1] = 2;
	sceneUpdater.numSteps[2] = 2;
	sceneUpdater.numSteps[3] = 2;
	sceneUpdater.numSteps[4] = 2;
	setupTableBasedSceneUpdateFunction(0xf0, 5, 0x708);
}

void SpecialOpcodes::spcBlackDragonCrashThroughGate() {
	for (int i = 0; i < 16; i++) {
		_vm->_screen->setScreenShakeOffset(shakeTbl[i], shakeTbl[i]);
		_vm->waitForFrames(1);
	}
}

void SpecialOpcodes::spcSetEngineFlag0x2000000() {
	_vm->setFlags(ENGINE_FLAG_2000000);
}

void SpecialOpcodes::spcClearEngineFlag0x2000000() {
	_vm->clearFlags(ENGINE_FLAG_2000000);
}

void SpecialOpcodes::spcZigmondFraudSceneLogic() {
	sceneUpdater.numSteps[0] = 2;
	sceneUpdater.numSteps[1] = 2;
	sceneUpdater.numSteps[2] = 1;
	sceneUpdater.numSteps[3] = 1;
	sceneUpdater.numSteps[4] = 2;
	sceneUpdater.numSteps[5] = 2;
	sceneUpdater.numSteps[6] = 2;
	sceneUpdater.numSteps[7] = 2;
//TODO do we need this -> DAT_80072dc0 = 2;
	sceneUpdater.iniIDTbl[0][0] = 0x19E;
	sceneUpdater.iniIDTbl[0][1] = 0x197;
	sceneUpdater.iniIDTbl[1][0] = 0x19E;
	sceneUpdater.iniIDTbl[1][1] = 0x197;
	sceneUpdater.iniIDTbl[2][0] = 0x197;
	sceneUpdater.iniIDTbl[3][0] = 0x19e;
	sceneUpdater.iniIDTbl[4][0] = 0x197;
	sceneUpdater.iniIDTbl[4][1] = 0x19e;
	sceneUpdater.iniIDTbl[5][0] = 0x19e;
	sceneUpdater.iniIDTbl[5][1] = 0x197;
	sceneUpdater.iniIDTbl[6][0] = 0x197;
	sceneUpdater.iniIDTbl[6][1] = 0x19e;
	sceneUpdater.iniIDTbl[7][0] = 0x19e;
	sceneUpdater.iniIDTbl[7][1] = 0x197;

	sceneUpdater.sequenceIDTbl[0][0] = 2;
	sceneUpdater.sequenceIDTbl[0][1] = 0xe;
	sceneUpdater.sequenceIDTbl[1][0] = 2;
	sceneUpdater.sequenceIDTbl[1][1] = 0xe;
	sceneUpdater.sequenceIDTbl[2][0] = 0xe;
	sceneUpdater.sequenceIDTbl[3][0] = 2;
	sceneUpdater.sequenceIDTbl[4][0] = 0xe;
	sceneUpdater.sequenceIDTbl[4][1] = 2;
	sceneUpdater.sequenceIDTbl[5][0] = 2;
	sceneUpdater.sequenceIDTbl[5][1] = 0xe;
	sceneUpdater.sequenceIDTbl[6][0] = 0xe;
	sceneUpdater.sequenceIDTbl[6][1] = 2;
	sceneUpdater.sequenceIDTbl[7][0] = 2;
	sceneUpdater.sequenceIDTbl[7][1] = 0xe;

	sceneUpdater.textTbl[0][0] = _vm->getDialogTextId(0x2D000);
	sceneUpdater.textTbl[0][1] = _vm->getDialogTextId(0x2D044);
	sceneUpdater.textTbl[1][0] = _vm->getDialogTextId(0x2D0B2);
	sceneUpdater.textTbl[1][1] = _vm->getDialogTextId(0x2D0D6);
	sceneUpdater.textTbl[2][0] = _vm->getDialogTextId(0x2D152);
	sceneUpdater.textTbl[3][0] = _vm->getDialogTextId(0x2D1A4);
	sceneUpdater.textTbl[4][0] = _vm->getDialogTextId(0x2D20A);
	sceneUpdater.textTbl[4][1] = _vm->getDialogTextId(0x2D27C);
	sceneUpdater.textTbl[5][0] = _vm->getDialogTextId(0x2D2EC);
	sceneUpdater.textTbl[5][1] = _vm->getDialogTextId(0x2D336);
	sceneUpdater.textTbl[6][0] = _vm->getDialogTextId(0x2D3E0);
	sceneUpdater.textTbl[6][1] = _vm->getDialogTextId(0x2D456);
	sceneUpdater.textTbl[7][0] = _vm->getDialogTextId(0x2D4A8);
	sceneUpdater.textTbl[7][1] = _vm->getDialogTextId(0x2D504);

	setupTableBasedSceneUpdateFunction(0x168, 8, 0xb4);
}

void SpecialOpcodes::spcZigmondFraudSceneLogic1() {
	sceneUpdater.numSteps[0] = 1;
	sceneUpdater.iniIDTbl[0][0] = 0x197;
	sceneUpdater.sequenceIDTbl[0][0] = 0x12;
	sceneUpdater.textTbl[0][0] = _vm->getDialogTextId(0x2F422);

	setupTableBasedSceneUpdateFunction(300, 1, 0x708);
}

void SpecialOpcodes::spcBrokenBlackDragonSceneLogic() {
	sceneUpdater.sequenceIDTbl[0][0] = 5;
	sceneUpdater.sequenceIDTbl[1][0] = 5;
	sceneUpdater.sequenceIDTbl[1][1] = 2;
	sceneUpdater.sequenceIDTbl[2][0] = 5;
	sceneUpdater.numSteps[0] = 1;
	sceneUpdater.sequenceIDTbl[2][1] = 2;
	sceneUpdater.numSteps[1] = 2;
	sceneUpdater.numSteps[2] = 2;
	sceneUpdater.textTbl[0][0] = _vm->getDialogTextId(0x40802);
	sceneUpdater.textTbl[1][0] = _vm->getDialogTextId(0x40852);
	sceneUpdater.textTbl[1][1] = _vm->getDialogTextId(0x40896);
	sceneUpdater.textTbl[2][0] = _vm->getDialogTextId(0x408C0);
	sceneUpdater.textTbl[2][1] = _vm->getDialogTextId(0x4092A);
	sceneUpdater.iniIDTbl[0][0] = 0x23B;
	sceneUpdater.iniIDTbl[1][0] = 0x23B;
	sceneUpdater.iniIDTbl[1][1] = 0x231;
	sceneUpdater.iniIDTbl[2][0] = 0x23B;
	sceneUpdater.iniIDTbl[2][1] = 0x231;
	setupTableBasedSceneUpdateFunction(300, 3, 600);
}

void SpecialOpcodes::spcDodoUnderAttackSceneLogic() {
	sceneUpdater.sequenceIDTbl[0][0] = 8;
	sceneUpdater.sequenceIDTbl[1][0] = 8;
	sceneUpdater.sequenceIDTbl[2][0] = 8;
	sceneUpdater.sequenceIDTbl[3][0] = 8;
	sceneUpdater.sequenceIDTbl[4][0] = 8;
	sceneUpdater.sequenceIDTbl[5][0] = 8;
	sceneUpdater.numSteps[0] = 1;
	sceneUpdater.numSteps[1] = 1;
	sceneUpdater.textTbl[0][0] = _vm->getDialogTextId(0x3353A);
	sceneUpdater.iniIDTbl[0][0] = 0x1C4;
	sceneUpdater.textTbl[1][0] = _vm->getDialogTextId(0x335AC);
	sceneUpdater.iniIDTbl[1][0] = 0x1C4;
	sceneUpdater.textTbl[2][0] = _vm->getDialogTextId(0x335F8);
	sceneUpdater.iniIDTbl[2][0] = 0x1C4;
	sceneUpdater.textTbl[3][0] = _vm->getDialogTextId(0x33660);
	sceneUpdater.iniIDTbl[3][0] = 0x1C4;
	sceneUpdater.textTbl[4][0] = _vm->getDialogTextId(0x336DE);
	sceneUpdater.iniIDTbl[4][0] = 0x1C4;
	sceneUpdater.textTbl[5][0] = _vm->getDialogTextId(0x3375C);
	sceneUpdater.iniIDTbl[5][0] = 0x1C4;
	sceneUpdater.numSteps[2] = 1;
	sceneUpdater.numSteps[3] = 1;
	sceneUpdater.numSteps[4] = 1;
	sceneUpdater.numSteps[5] = 1;
	setupTableBasedSceneUpdateFunction(0x1e0, 6, 0x1e0);
}

void SpecialOpcodes::spcForestWithoutDodoSceneLogic() {
	sceneUpdater.sequenceIDTbl[0][0] = 5;
	sceneUpdater.sequenceIDTbl[0][1] = 0xb;
	sceneUpdater.sequenceIDTbl[1][0] = 0xb;
	sceneUpdater.sequenceIDTbl[1][2] = 0xb;
	sceneUpdater.numSteps[0] = 2;
	sceneUpdater.numSteps[1] = 3;
	sceneUpdater.sequenceIDTbl[1][1] = 5;
	sceneUpdater.textTbl[0][0] = _vm->getDialogTextId(0x33EA0);
	sceneUpdater.iniIDTbl[0][0] = 0x1C5;
	sceneUpdater.textTbl[0][1] = _vm->getDialogTextId(0x33EFC);
	sceneUpdater.iniIDTbl[0][1] = 0x1C4;
	sceneUpdater.textTbl[1][0] = _vm->getDialogTextId(0x33F34);
	sceneUpdater.iniIDTbl[1][0] = 0x1C4;
	sceneUpdater.textTbl[1][1] = _vm->getDialogTextId(0x34000);
	sceneUpdater.iniIDTbl[1][1] = 0x1C5;
	sceneUpdater.textTbl[1][2] = _vm->getDialogTextId(0x34074);
	sceneUpdater.iniIDTbl[1][2] = 0x1C4;
	setupTableBasedSceneUpdateFunction(300, 2, 600);
}

void SpecialOpcodes::spcBlackDragonOnHillSceneLogic() {
	sceneUpdater.sequenceIDTbl[0][0] = 3;
	sceneUpdater.sequenceIDTbl[0][1] = 7;
	sceneUpdater.sequenceIDTbl[1][0] = 3;
	sceneUpdater.sequenceIDTbl[1][1] = 7;
	sceneUpdater.sequenceIDTbl[2][0] = 3;
	sceneUpdater.sequenceIDTbl[2][1] = 7;
	sceneUpdater.sequenceIDTbl[3][0] = 3;
	sceneUpdater.numSteps[0] = 2;
	sceneUpdater.textTbl[0][0] = _vm->getDialogTextId(0x325EA);
	sceneUpdater.textTbl[0][1] = _vm->getDialogTextId(0x3262A);
	sceneUpdater.textTbl[1][0] = _vm->getDialogTextId(0x32686);
	sceneUpdater.textTbl[1][1] = _vm->getDialogTextId(0x326D8);
	sceneUpdater.textTbl[2][0] = _vm->getDialogTextId(0x3270E);
	sceneUpdater.textTbl[2][1] = _vm->getDialogTextId(0x32774);
	sceneUpdater.textTbl[3][0] = _vm->getDialogTextId(0x32D72);
	sceneUpdater.iniIDTbl[0][0] = 0x1B6;
	sceneUpdater.iniIDTbl[0][1] = 0x1B7;
	sceneUpdater.iniIDTbl[1][0] = 0x1B6;
	sceneUpdater.iniIDTbl[1][1] = 0x1B7;
	sceneUpdater.iniIDTbl[2][0] = 0x1B6;
	sceneUpdater.iniIDTbl[2][1] = 0x1B7;
	sceneUpdater.iniIDTbl[3][0] = 0x1B6;
	sceneUpdater.numSteps[1] = 2;
	sceneUpdater.numSteps[2] = 2;
	sceneUpdater.numSteps[3] = 1;
	setupTableBasedSceneUpdateFunction(300, 4, 300);
}

void SpecialOpcodes::spcUnk4e() {
	panCamera(1);
}

void SpecialOpcodes::spcUnk4f() {
	panCamera(2);
}

void SpecialOpcodes::spcCloseInventory() {
	if (_vm->_inventory->getState() == InventoryOpen) {
		_vm->_inventory->closeInventory();
		_vm->_inventory->setState(Closed);
	}
}

void SpecialOpcodes::spcOpenInventionBook() {
	if (_vm->_inventory->getState() == InventoryOpen) {
		_vm->_inventory->closeInventory();
	}
	_vm->_inventory->openInventionBook();
	_vm->_inventory->setState(InventionBookOpen);
}

void SpecialOpcodes::spcCloseInventionBook() {
	_vm->_inventory->closeInventionBook();
	_vm->_inventory->setState(Closed);
}

void SpecialOpcodes::spcSetEngineFlag0x4000000() {
	_vm->setFlags(ENGINE_FLAG_4000000);
}

void SpecialOpcodes::spcClearEngineFlag0x4000000() {
	_vm->clearFlags(ENGINE_FLAG_4000000);
}

void SpecialOpcodes::spcSetCursorSequenceIdToZero() {
	_vm->_cursor->_sequenceID = 0;
}

void SpecialOpcodes::spcFlickerSetFlag0x80() {
	_vm->_dragonINIResource->getFlickerRecord()->actor->setFlag(ACTOR_FLAG_80);
}

//used in castle garden scene with knights
void SpecialOpcodes::spcUnk5d() {
	_vm->getINI(_vm->getINI(0x13b)->objectState + 0x13c)->actor->updateSequence(_vm->getINI(0x13b)->objectState2);
}

void SpecialOpcodes::spcUnk5e() {
	panCamera(1);
	_vm->_dragonINIResource->setFlickerRecord(_vm->getINI(0));
}

void SpecialOpcodes::spcUnk5f() {
	_vm->getINI(0x2ab)->objectState = 0;
	panCamera(2);
}

void SpecialOpcodes::spcCastleBuildBlackDragonSceneLogic() {
	_vm->setSceneUpdateFunction(castleBuildingBlackDragon2UpdateFunction);
}

void SpecialOpcodes::spcStopSceneUpdateFunction() {
	_vm->clearSceneUpdateFunction();
}

void SpecialOpcodes::spcSetInventorySequenceTo5() {
	_vm->_inventory->updateActorSequenceId(5);
}

void SpecialOpcodes::spcResetInventorySequence() {
	_vm->_inventory->resetSequenceId();
}

void SpecialOpcodes::spcUnk65ScenePaletteRelated() {
	byte *palette = _vm->_screen->getPalette(0);
	memset(palette + 0xb1 * 2, 0, 32); //zero out 16 palette records from index 0xb1 to 0xc0
}

void SpecialOpcodes::spcUnk66() {
	uint16 var =_vm->getVar(2);

	uint16 bVar1 = (var & 1) == 0;
	uint16 uVar9 = bVar1;
	if ((var & 4) == 0) {
		uVar9 = (uint16)bVar1 + 1;
	}
	if ((var & 2) == 0) {
		uVar9 = uVar9 + 1;
	}
	_vm->getINI(1)->objectState = uVar9;
}

void SpecialOpcodes::spcTournamentSetCamera() {
	_vm->_scene->_camera.x = 0x140; // TODO should have method for updating camera.
}

void SpecialOpcodes::spcTournamentCutScene() {
	CutScene *cutScene = new CutScene(_vm);
	cutScene->tournamentCutScene();
	delete cutScene;
}

void SpecialOpcodes::spcInsideBlackDragonUpdatePalette() {
	memcpy(_vm->_scene->getPalette() + 0x180,
			_vm->_dragonINIResource->getRecord(0x2b2)->actor->_actorResource->getPalette() + 0x180,
			0x80);
	_vm->_screen->loadPalette(0, _vm->_scene->getPalette());
}

void SpecialOpcodes::spcCastleGateSceneLogic() {
	_vm->_screen->updatePaletteTransparency(0, 0xc0, 0xff, true);
	//TODO FUN_80017d68(3, 0); sets sprite layer attribute from layers 0 and 1. Doesn't seem to be needed.
	setSpecialOpCounter(-1);
	_vm->clearFlags(ENGINE_FLAG_1);
	_vm->setSceneUpdateFunction(castleFogUpdateFunction);
}

// 0x80038c1c
void SpecialOpcodes::panCamera(int16 mode) {
	int iVar1;
	int iVar2;

	if (mode == 1) {
		_vm->getINI(0x2ab)->objectState = _vm->_scene->_camera.x;
		_vm->_dragonINIResource->setFlickerRecord(nullptr);
		iVar2 = (int) _vm->_scene->_camera.x;
		iVar1 = iVar2;
		while (iVar1 <= (_vm->_scene->getStageWidth() - 320)) {
			_vm->_scene->_camera.x = (short) iVar2;
			_vm->waitForFrames(1);
			iVar2 = iVar2 + 4;
			iVar1 = iVar2 * 0x10000 >> 0x10;
		}
		_vm->_scene->_camera.x = _vm->_scene->getStageWidth() - 320;
	}
	if (mode == 2) {
		iVar2 = (int) _vm->_scene->_camera.x;
		iVar1 = iVar2;
		while (-1 < iVar1) {
			_vm->_scene->_camera.x = (short) iVar2;
			_vm->waitForFrames(1);
			iVar2 = iVar2 + -3;
			iVar1 = iVar2 * 0x10000;
		}
		_vm->_scene->_camera.x = 0;
		_vm->_dragonINIResource->setFlickerRecord(_vm->getINI(0));
	}
	return;
}

void SpecialOpcodes::spcBlackDragonDialogForCamelhot() {
	uint16 buffer[1024];
	uint32 textId = _vm->getDialogTextId(0x30DD8);
	_vm->_talk->loadText(textId, buffer, 1024);
	_vm->_talk->displayDialogAroundPoint(buffer, 0x27, 0xc, 0xc01, 0, textId);
	//TODO this isn't quite right. The audio isn't played and it's not waiting long enough.
}

void SpecialOpcodes::spcSetCameraXToZero() {
	_vm->_scene->_camera.x = 0;
}

void SpecialOpcodes::spcDiamondIntroSequenceLogic() {
	CutScene *cutScene = new CutScene(_vm);
	cutScene->diamondScene();
	delete cutScene;
}

void SpecialOpcodes::spcLoadFileS10a6act() {
	//TODO is this needed?
}

void SpecialOpcodes::spcLoadFileS10a7act() {
	//TODO is this needed?
}

void SpecialOpcodes::spcFlickerPutOnStGeorgeArmor() {
	Actor *actor = _vm->_dragonINIResource->getRecord(0x21f)->actor;
	actor->setFlag(ACTOR_FLAG_100);
	actor->_priorityLayer = 1;
}

void SpecialOpcodes::spcClearTextFromScreen() {
	_vm->clearAllText();
}

void SpecialOpcodes::spcStopScreenShakeUpdater() {
	_vm->clearSceneUpdateFunction();
	_vm->_screen->setScreenShakeOffset(0, 0);
}

void SpecialOpcodes::spcInsideBlackDragonScreenShake() {
	for (int i = 0; i < 5; i ++) {
		_vm->_screen->setScreenShakeOffset(0, shakeTbl[i]);
		_vm->waitForFrames(1);
	}
	_vm->_screen->setScreenShakeOffset(0, 0);
}

void SpecialOpcodes::spc85SetScene1To0x35() {
	_vm->_sceneId1 = 0x35;
}

void SpecialOpcodes::spc86SetScene1To0x33() {
	_vm->_sceneId1 = 0x33;
}

void SpecialOpcodes::spc87SetScene1To0x17() {
	_vm->_sceneId1 = 0x17;
}

void SpecialOpcodes::spc88SetScene1To0x16() {
	_vm->_sceneId1 = 0x16;
}

void SpecialOpcodes::spcSetUnkFlag2() {
	_vm->setUnkFlags(ENGINE_UNK1_FLAG_2);
}

void SpecialOpcodes::spcClearUnkFlag2() {
	_vm->clearUnkFlags(ENGINE_UNK1_FLAG_2);
}

void SpecialOpcodes::spcUnk8b() {
	//TODO sceneId_1 = DAT_80063e20; //0xA
}

void SpecialOpcodes::spcHedgehogTest() {
	if (_vm->_dragonINIResource->getRecord(0x168)->actor->_sequenceID == 4 &&
			_vm->_dragonINIResource->getRecord(0x169)->actor->_sequenceID == 4 &&
			_vm->_dragonINIResource->getRecord(0x16a)->actor->_sequenceID == 4) {
		_vm->_dragonINIResource->getRecord(0x169)->objectState = 1;
	} else {
		_vm->_dragonINIResource->getRecord(0x169)->objectState = 0;
	}
}

void SpecialOpcodes::spcLoadScene1() {
	CutScene *cutScene = new CutScene(_vm);
	cutScene->scene1();
	delete cutScene;
}

void SpecialOpcodes::spcKnightsSavedCastleCutScene() {
	CutScene *cutScene = new CutScene(_vm);
	cutScene->knightsSavedBackAtCastle();
	delete cutScene;
}

void SpecialOpcodes::spcFlickerReturnsCutScene() {
	CutScene *cutScene = new CutScene(_vm);
	cutScene->flameReturnsCutScene();
	delete cutScene;
}

void SpecialOpcodes::spcKnightsSavedAgainCutScene() {
	CutScene *cutScene = new CutScene(_vm);
	cutScene->knightsSavedAgain();
	delete cutScene;
}

void SpecialOpcodes::spcTransitionToMap() {
	_vm->_scene->setMgLayerPriority(0);
	_vm->_scene->setFgLayerPriority(0);
	_vm->_cursor->updateSequenceID(0);
	_vm->waitForFrames(1);
	_vm->setFlags(ENGINE_FLAG_20000000);
	mapTransition(1);
}

void SpecialOpcodes::spcTransitionFromMap() {
	mapTransition(0);
	_vm->_scene->setMgLayerPriority(2);
	_vm->_scene->setFgLayerPriority(3);
	_vm->clearFlags(ENGINE_FLAG_20000000);
}

void SpecialOpcodes::spcCaveOfDilemmaSceneLogic() {
	_vm->setSceneUpdateFunction(caveOfDilemmaUpdateFunction);
}

void SpecialOpcodes::spcFadeCreditsToBackStageScene() {
	_vm->fadeToBlackExcludingFont();
	_vm->_screen->loadPalette(0, _vm->_scene->getPalette());
	_vm->_scene->_camera.x = 0x140;
	_vm->waitForFrames(0x3c);
	_vm->fadeFromBlackExcludingFont();
}

void SpecialOpcodes::spcRunCredits() {
//	int iVar1;
//	DAT_8006a440 = 0x1a;
//	DAT_80087270 = 0x78;
	_vm->setUnkFlags(ENGINE_UNK1_FLAG_1);
	_vm->setUnkFlags(ENGINE_UNK1_FLAG_2);
//	open_files_slot_number_tbl = 0;
//	DAT_80072de4 = 0;
//	buf2048bytes = actor_dictionary;
//	DAT_8007273c = actor_dictionary;
	_vm->_cursor->setActorFlag400();
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->setFlags(ENGINE_FLAG_8000000);
//	iVar1 = file_read_to_buffer(strCredits_txt);
//	DAT_800728ec = iVar1 + (int)DAT_8007273c;
//	buf2048bytes = (int32 *)((iVar1 + 3U & 0xfffffffc) + (int)buf2048bytes);
//	memcpy2((byte *)buf2048bytes, scrFileData_maybe, 0x200);
//	buf2048bytes = buf2048bytes + 0x80;
	_vm->_screen->loadPalette(0, _vm->_dragonINIResource->getRecord(0x2C8)->actor->_actorResource->getPalette());
	_vm->_scene->setMgLayerPriority(0);
	_vm->_scene->setFgLayerPriority(0);
//	vsync_updater_function = creditsUpdateFunction;
	_vm->_credits->start();
}

void SpecialOpcodes::spcEndCreditsAndRestartGame() {
	_vm->fadeToBlackExcludingFont();
//	_volumeSFX = 0;
//	setCDAVolumes();
	while (_vm->_credits->isRunning()) {
		_vm->waitForFrames(1);
	}
//	ReloadGameFlag = 2;
//	Exec_FMV_RELOADTT();

	_vm->waitForFrames(100);

	Dragons::DragonsEngine::quitGame();
}

void SpecialOpcodes::spcLoadLadyOfTheLakeActor() {
	//TODO
	DragonINI *ini = _vm->_dragonINIResource->getRecord(0x2a7);
	_vm->_actorManager->loadActor(0xcd, ini->actor->_actorID);
	ini->actor->setFlag(ACTOR_FLAG_4);
	ini->actorResourceId = 0xcd;
	//DisableVSyncEvent();
//	uVar17 = (uint)(uint16)dragon_ini_pointer[DAT_8006398c + -1].field_0x1c;
//	uVar7 = load_actor_file(0xcc);
//	file_read_to_buffer(s_s12a6.act_80011740, (&actor_dictionary)[(uVar7 & 0xffff) * 2]);
//	actors[uVar17].﻿actorFileDictionaryIndex = (uint16_t)uVar7;
//	actors[uVar17].﻿resourceID = 0xcd;
//	iVar18 = DAT_8006398c;
//	actors[uVar17]._flags = actors[uVar17]._flags | 4;
//	dragon_ini_pointer[iVar18 + -1].x = 0xcd;
//	LAB_8002ad94:
	//EnableVSyncEvent();
}

void SpecialOpcodes::spcUseClickerOnLever() {
	if (_vm->_inventory->isOpen()) {
		_vm->_talk->flickerRandomDefaultResponse();
		_vm->_dragonINIResource->getRecord(0)->objectState = 1;
	} else {
		_vm->_dragonINIResource->getRecord(0)->objectState = 0;
	}
}

void SpecialOpcodes::spcJesterInLibrarySceneLogic() {
	sceneUpdater.sequenceIDTbl[0][0] = 0xffff;
	sceneUpdater.sequenceIDTbl[1][0] = 0xffff;
	sceneUpdater.sequenceIDTbl[2][0] = 0xffff;
	sceneUpdater.numSteps[0] = 1;
	sceneUpdater.numSteps[1] = 1;
	sceneUpdater.numSteps[2] = 1;
	sceneUpdater.textTbl[0][0] = _vm->getDialogTextId(0x18502);
	sceneUpdater.textTbl[1][0] = _vm->getDialogTextId(0x185E0);
	sceneUpdater.textTbl[2][0] = _vm->getDialogTextId(0x18596);
	sceneUpdater.iniIDTbl[0][0] = 0xD7;
	sceneUpdater.iniIDTbl[1][0] = 0xD7;
	sceneUpdater.iniIDTbl[2][0] = 0xD7;
	setupTableBasedSceneUpdateFunction(300, 3, 0x708);
}

void SpecialOpcodes::pizzaMakerStopWorking() {
	Actor *actorf4 = _vm->getINI(0xf4)->actor;
	Actor *actorf5 = _vm->getINI(0xf5)->actor;

	if (actorf4->_sequenceID != 0) {
		if (actorf4->_sequenceID == 1) {
			actorf4->waitUntilFlag8And4AreSet();
			actorf4->updateSequence(2);
			actorf5->_x_pos = 277;
			actorf5->_y_pos = 92;
			actorf5->updateSequence(7);
			_vm->waitForFrames(0x78);
			actorf5->updateSequence(8);
			actorf5->waitUntilFlag8And4AreSet();
			actorf5->_x_pos = -100;
			actorf5->_y_pos = 100;
			actorf4->updateSequence(3);
		} else {
			if (actorf4->_sequenceID == 2) {
				_vm->waitForFrames(0x78);
				actorf5->updateSequence(8);
				actorf5->waitUntilFlag8And4AreSet();
				actorf5->_x_pos = -100;
				actorf5->_y_pos = 100;
				actorf4->updateSequence(3);
			} else {
				if (actorf4->_sequenceID != 3) {
					return;
				}
			}
		}
		actorf4->waitUntilFlag8And4AreSet();
		actorf4->updateSequence(0);
	}
}

void SpecialOpcodes::clearSceneUpdateFunction() {
	if (_dat_80083148 != _uint16_t_80083154) {
		//TODO FUN_8001ac5c((uint)_dat_80083148, (uint)DAT_80083150, (uint)_uint16_t_80083154, (uint)DAT_80083158);
	}
	if (sceneUpdater.sequenceID != -1) {
		_vm->getINI(sceneUpdater.iniID)->actor->updateSequence(sceneUpdater.sequenceID);
	}
	_vm->clearSceneUpdateFunction();
}

void SpecialOpcodes::setupTableBasedSceneUpdateFunction(uint16 initialCounter, uint16 numSequences,
														uint16 sequenceDuration) {
	sceneUpdater.sequenceID = -1;
	_uint16_t_80083154 = 0;
	_dat_80083148 = 0;
//TODO
//	DAT_80072858 = 0;
	sceneUpdater.curSequenceIndex = 0;
	sceneUpdater.numTotalSequences = numSequences;
	sceneUpdater.curSequence = _vm->getRand(numSequences);
	sceneUpdater.sequenceDuration = sequenceDuration;
	sceneUpdater.counter = initialCounter;

	//TODO

	_vm->setSceneUpdateFunction(tableBasedSceneUpdateFunction);
}

void SpecialOpcodes::spcUnk80FlickerArmorOn() {
	Actor *actor = _vm->_dragonINIResource->getRecord(0x21f)->actor;
	actor->_priorityLayer = 2;
	actor->clearFlag(ACTOR_FLAG_100);
}

void SpecialOpcodes::spcShakeScreenSceneLogic() {
	setSpecialOpCounter(0x1e);
	_vm->setSceneUpdateFunction(shakeScreenUpdateFunction);
}

int16 SpecialOpcodes::getSpecialOpCounter() {
	return _specialOpCounter;
}

void SpecialOpcodes::setSpecialOpCounter(int16 newValue) {
	_specialOpCounter = newValue;
}

void SpecialOpcodes::mapTransition(uint16 mode) {
	const uint16 mapLookupTbl[26] = {
			160,    100,     45,    100,
			105,    170,     14,     87,
			 83,     33,     86,    100,
			 25,    180,    161,    156,
			195,     47,    287,     35,
			292,     80,    202,    182,
			127,     78
	};

	const uint16 mapSceneIdTbl[13] = {
			0,
			0xa, 0x15, 0x1a,
			0x1b, 0x1c, 0x1e,
			0x20, 0x23, 0x25,
			0x2d, 0x30, 0x31
	};

	uint16 targetLocation = _vm->getINI(0x1e)->objectState;
	if (targetLocation == 0) {
		targetLocation = 0xc;
		for (int i = 1; i < 0xd; i++) {
			if (_vm->_scene->_mapTransitionEffectSceneID < mapSceneIdTbl[i]) {
				targetLocation = i - 1;
				break;
			}
		}
	}

	_vm->_cursor->updatePosition(mapLookupTbl[targetLocation * 2], mapLookupTbl[targetLocation * 2 + 1]);
	_vm->setFlags(ENGINE_FLAG_20);

	int16 cursorX = _vm->_cursor->_x;
	int16 cursorY = _vm->_cursor->_y;

	int32 topIncrement = (cursorY << 0x10) >> 4;
	int32 bottomIncrement = (DRAGONS_SCREEN_HEIGHT - cursorY) * 0x10000 >> 4;
	int32 rightIncrement = (DRAGONS_SCREEN_WIDTH - cursorX) * 0x10000 >> 4;

	if (mode == 0) { //Close map
		FlatQuad *topQuad = _vm->_screen->getFlatQuad(_vm->_screen->addFlatQuad(0,0,0x140,0,0x140,0,0,0,1,4,0));
		FlatQuad *bottomQuad = _vm->_screen->getFlatQuad(_vm->_screen->addFlatQuad(0,200,0x140,200,0x140,200,0,200,1,4,0));
		FlatQuad *leftQuad = _vm->_screen->getFlatQuad(_vm->_screen->addFlatQuad(0,0,0,0,0,200,0,200,1,4,0));
		FlatQuad *rightQuad = _vm->_screen->getFlatQuad(_vm->_screen->addFlatQuad(0x140,0,0x140,0,0x140,200,0x140,200,1,4,0));
		int32 topY = topQuad->points[3].y << 0x10;
		int32 bottomY = bottomQuad->points[0].y << 0x10;
		int32 rightX = rightQuad->points[0].x << 0x10;
		int32 leftX = leftQuad->points[1].x << 0x10;

		while (topY < (cursorY << 0x10)) {
			topY = topY + topIncrement;
			bottomY = bottomY - bottomIncrement;
			leftX = leftX + cursorX * 0x1000;
			topQuad->points[3].y = topY >> 0x10;
			topQuad->points[2].y = topY >> 0x10;
			bottomQuad->points[0].y = bottomY >> 0x10;
			bottomQuad->points[1].y = bottomY >> 0x10;
			leftQuad->points[1].x = leftX >> 0x10;
			leftQuad->points[3].x = leftX >> 0x10;
			rightX = rightX - rightIncrement;
			rightQuad->points[0].x = rightX >> 0x10;
			rightQuad->points[2].x = rightX >> 0x10;
			_vm->waitForFrames(1);
		}

		_vm->fadeToBlack();
	} else if (mode == 1) { // Open map
		FlatQuad *topQuad = _vm->_screen->getFlatQuad(_vm->_screen->addFlatQuad(0,0,0x140,0,0x140,cursorY,0,cursorY,1,4,0));
		FlatQuad *bottomQuad = _vm->_screen->getFlatQuad(_vm->_screen->addFlatQuad(0, cursorY, 0x140, cursorY, 0x140, 200, 0, 200, 1, 4, 0));
		FlatQuad *leftQuad = _vm->_screen->getFlatQuad(_vm->_screen->addFlatQuad(0, 0, cursorX, 0, cursorX, 200, 0, 200, 1, 4, 0));
		FlatQuad *rightQuad = _vm->_screen->getFlatQuad(_vm->_screen->addFlatQuad(cursorX,0,0x140,0,0x140,200,cursorX,200,1,4,0));
		int32 topY = topQuad->points[3].y << 0x10;
		int32 bottomY = bottomQuad->points[0].y << 0x10;
		int32 leftX = leftQuad->points[1].x << 0x10;
		int32 rightX = rightQuad->points[0].x << 0x10;
		_vm->waitForFrames(2);
		bool hasDoneFade = false;
		while (0 < topY) {
			topY = topY - topIncrement;
			bottomY = bottomY + bottomIncrement;
			leftX = leftX + cursorX * -0x1000;
			topQuad->points[3].y = topY >> 0x10;
			topQuad->points[2].y = topY >> 0x10;
			bottomQuad->points[0].y = bottomY >> 0x10;
			bottomQuad->points[1].y = bottomY >> 0x10;
			leftQuad->points[1].x = leftX >> 0x10;
			leftQuad->points[3].x = leftX >> 0x10;
			rightX = rightX + rightIncrement;
			rightQuad->points[0].x = rightX >> 0x10;
			rightQuad->points[2].x = rightX >> 0x10;
			_vm->waitForFrames(1);
			if (!hasDoneFade) {
				_vm->fadeFromBlack();
				hasDoneFade = true;
			}
		}
	}
	_vm->_screen->clearAllFlatQuads();
}

void pizzaUpdateFunction() {
	static int16 counter = 0;
	DragonsEngine *vm = getEngine();

	Actor *actorf4 = vm->getINI(0xf4)->actor;
	Actor *actorf5 = vm->getINI(0xf5)->actor;

	if (counter == 0) {
		if (actorf4->isFlagSet(ACTOR_FLAG_4)) {
			if (actorf4->_sequenceID == 0) {
				actorf4->updateSequence(1);
			} else {
				if (actorf4->_sequenceID == 1) {
					actorf4->updateSequence(2);
					actorf5->_x_pos = 0x115;
					actorf5->_y_pos = 0x5c;
					actorf5->updateSequence(7);
					counter = 0x2d;
					return;
				}
				if (actorf4->_sequenceID == 2) {
					if ((actorf5->_sequenceID == 8) &&
						(actorf5->isFlagSet(ACTOR_FLAG_4))) {
						actorf5->_x_pos = -100;
						actorf5->_y_pos = 100;
						actorf4->updateSequence(3);
					} else {
						if (actorf5->_sequenceID == 8) {
							return;
						}
						actorf5->updateSequence(8);
					}
				} else {
					if (actorf4->_sequenceID != 3) {
						return;
					}
					actorf4->updateSequence(0);
				}
			}
		}
	} else {
		counter--;
	}
}

void tableBasedSceneUpdateFunction() {
	uint uVar3;
	DragonsEngine *vm = getEngine();
	SpecialOpcodes *spc = vm->_scriptOpcodes->_specialOpCodes;

	uVar3 = (uint)spc->sceneUpdater.curSequence;
	if (!vm->isFlagSet(ENGINE_FLAG_8000) || vm->_isLoadingDialogAudio) {
		if (spc->sceneUpdater.sequenceID != -1) {
			vm->getINI(spc->sceneUpdater.iniID)->actor->updateSequence(spc->sceneUpdater.sequenceID);
			spc->sceneUpdater.sequenceID = -1;
		}
		if (!vm->_isLoadingDialogAudio) {
			spc->sceneUpdater.counter--;
			if (spc->sceneUpdater.counter == 0) {
				spc->sceneUpdater.sequenceID = spc->sceneUpdater.sequenceIDTbl[uVar3][spc->sceneUpdater.curSequenceIndex]; //*(int16_t *) (sceneUpdateSequenceTbl[uVar3].sequenceIdPtr + (uint) spc->sceneUpdater.curSequenceIndex);
				spc->sceneUpdater.iniID = spc->sceneUpdater.iniIDTbl[uVar3][spc->sceneUpdater.curSequenceIndex] - 1;
//						*(short *) (sceneUpdateSequenceTbl[uVar3].iniIdPtr + (uint) spc->sceneUpdater.curSequenceIndex) -
//						1;
				if (spc->sceneUpdater.sequenceID != -1) {
					Actor *actor = vm->getINI(spc->sceneUpdater.iniID)->actor;
					uint16 originalSequenceID = actor->_sequenceID;
					actor->updateSequence(spc->sceneUpdater.sequenceID);
					spc->sceneUpdater.sequenceID = originalSequenceID;
				}
				vm->_talk->playDialogAudioDontWait(spc->sceneUpdater.textTbl[uVar3][spc->sceneUpdater.curSequenceIndex]);
				spc->sceneUpdater.curSequenceIndex++;
				spc->sceneUpdater.counter = 0x1e;
				if (spc->sceneUpdater.numSteps[uVar3] <= (uint) spc->sceneUpdater.curSequenceIndex) {
					spc->sceneUpdater.curSequenceIndex = 0;
					spc->sceneUpdater.curSequence = vm->getRand((uint) spc->sceneUpdater.numTotalSequences);
					spc->sceneUpdater.counter = spc->sceneUpdater.sequenceDuration;
				}
			}
		}
	}
}

void castleBuildingBlackDragon2UpdateFunction() {
	DragonINI *ini;
	DragonsEngine *vm = getEngine();

	ini = vm->_dragonINIResource->getRecord(0x231);
	if (ini->counter <= 0) {
		if (ini->objectState == 0) {
			ini->actor->updateSequence(0xb);
			ini->counter = 0x68;
			ini->objectState = 1;
		} else if (ini->objectState == 1) {
			vm->_dragonINIResource->getRecord(0x234)->actor->updateSequence(4);
			ini->counter = vm->getRand(0xb4) + 900;
			ini->objectState = 0;
		}
	}
}

void shakeScreenUpdateFunction() {
	static uint8 shakeDirection = 0;
	DragonsEngine *vm = getEngine();
	int16 counter = vm->_scriptOpcodes->_specialOpCodes->getSpecialOpCounter();

	if (counter == 0) {
		shakeDirection = shakeDirection ^ 1u;
		int16 shakeValue = shakeDirection != 0 ? 1 : -1;
		vm->_screen->setScreenShakeOffset(shakeValue, shakeValue);
	} else {
		counter--;
	}
	vm->_scriptOpcodes->_specialOpCodes->setSpecialOpCounter(counter);
}

void ladyOfTheLakeCapturedUpdateFunction() {
	const uint32 dialogTbl[3] = {
			0x490C8, 0x490FC, 0x4913A
	};
	static int ladyofLakeCountdownTimer = 0x12c;
	static uint8 ladyOfLakeDialogIndex = 0;
	DragonsEngine *vm = getEngine();

	if (!vm->isFlagSet(ENGINE_FLAG_8000)) {
		ladyofLakeCountdownTimer--;
		if (ladyofLakeCountdownTimer == 0) {
			vm->_talk->playDialogAudioDontWait(vm->getDialogTextId(dialogTbl[ladyOfLakeDialogIndex]));
			if (ladyOfLakeDialogIndex == 2) {
				ladyOfLakeDialogIndex = 0;
			} else {
				ladyOfLakeDialogIndex++;
			}
			ladyofLakeCountdownTimer = 0x708;
		}
	}
}

void castleFogUpdateFunction() {
	static int16 castleFogXOffset = 0;
	DragonsEngine *vm = getEngine();
	int16 specialOpCounter = vm->_scriptOpcodes->_specialOpCodes->getSpecialOpCounter();
	if (specialOpCounter == -1) {
		castleFogXOffset = 0;
		specialOpCounter = 6;
	}
	if (specialOpCounter == 0) {
		castleFogXOffset--;
		if (castleFogXOffset < 0) {
			castleFogXOffset = 0x13f;
		}
		vm->_scene->setLayerOffset(2, Common::Point(castleFogXOffset, 0));
		specialOpCounter = 6;
	} else {
		specialOpCounter--;
	}
	vm->_scriptOpcodes->_specialOpCodes->setSpecialOpCounter(specialOpCounter);
}

void menInMinesSceneUpdateFunction() {
	const uint32 sceneUpdateFuncDialogTbl[4] = {
		0x4590A, 0x45994, 0x459F4, 0x45A60
	};
	DragonsEngine *vm = getEngine();
	uint16 sequenceId;
	Actor *actor = vm->_dragonINIResource->getRecord(0x293)->actor;

	if (!vm->isFlagSet(ENGINE_FLAG_8000)) {
		uint16 specialOpCounter = vm->_scriptOpcodes->_specialOpCodes->getSpecialOpCounter();
		if (specialOpCounter != 0) {
			if (actor->_sequenceID != 0) {
				actor->updateSequence(0);
			}
			vm->_scriptOpcodes->_specialOpCodes->setSpecialOpCounter(specialOpCounter - 1);
			return;
		}
		vm->_scriptOpcodes->_specialOpCodes->setSpecialOpCounter(vm->getRand(5) * 0x3c + 0x708);
		vm->_talk->playDialogAudioDontWait(vm->getDialogTextId(sceneUpdateFuncDialogTbl[vm->getRand(4)]));
		sequenceId = 2;
	} else {
		if (!vm->_isLoadingDialogAudio) {
			return;
		}
		sequenceId = 0;
	}
	actor->updateSequence(sequenceId);
}

void monksAtBarSceneUpdateFunction() {
	static uint8 monksAtBarCurrentState = 0;
	static const uint32 sceneUpdateFuncDialogTbl[6] = {
		0x37800, 0x37854, 0x378CA,
		0x39152, 0x3919A, 0x3922C
	};
	static const uint32 barKeeperTextIdTbl[10] = {
		0x38C68, 0x38CE2, 0x38D4E, 0x38CE2,
		0x38DC2, 0x38E0C, 0x38C68, 0x38E5C,
		0x38ED0, 0x38CE2
	};
	static const uint32 DAT_800832f0[4] = {0x38F2A, 0x39000, 0x39084, 0x390E8};

	DragonsEngine *vm = getEngine();
	Actor *barKeeper = vm->_dragonINIResource->getRecord(0x1e7)->actor;
	Actor *monk1 = vm->_dragonINIResource->getRecord(0x1ec)->actor;
	Actor *monk2 = vm->_dragonINIResource->getRecord(0x1ed)->actor;
	DragonINI *ini = vm->_dragonINIResource->getRecord(0x1e6);
	bool bVar1;
	short sVar2;
	uint32 textIndex;
	uint16 sequenceId;
	int16 specialOpCounter = vm->_scriptOpcodes->_specialOpCodes->getSpecialOpCounter();
	if (specialOpCounter == -1) {
		monksAtBarCurrentState = vm->getRand(2) * 2;
		specialOpCounter = 600;
	}
	if (vm->isFlagSet(ENGINE_FLAG_8000)) {
		if (!vm->_isLoadingDialogAudio) {
			return;
		}
		monk1->updateSequence(0);
		monk2->updateSequence(8);
		return;
	}
	bVar1 = ini->sceneId != 0;
	if (specialOpCounter != 0) {
		specialOpCounter = specialOpCounter + -1;
	}
	if (specialOpCounter == 0) {
		switch (monksAtBarCurrentState) {
		case 0:
			if (0x31 < vm->getRand(100)) {
				sVar2 = vm->getRand(3);
				specialOpCounter = (sVar2 + 3) * 0x3c;
				monksAtBarCurrentState = 2;
				break;
			}
			if (bVar1) {
				barKeeper->updateSequence(0xc);
				textIndex = sceneUpdateFuncDialogTbl[vm->getRand(3) + 3];
			} else {
				barKeeper->updateSequence(2);
				textIndex = sceneUpdateFuncDialogTbl[vm->getRand(3)];
			}
			vm->_talk->playDialogAudioDontWait(vm->getDialogTextId(textIndex));
			monksAtBarCurrentState = 1;
			specialOpCounter = 0;
			break;
		case 1:
			if (bVar1) {
				sequenceId = 8;
			} else {
				sequenceId = 0;
			}
			barKeeper->updateSequence(sequenceId);
			specialOpCounter = 0x168;
			monksAtBarCurrentState = 2;
			break;
		case 2: {
			uint16 randTextId = vm->getRand(10);
			if ((randTextId & 1) == 0) {
				monk1->updateSequence(2);
			} else {
				monk2->updateSequence(10);
			}
			vm->_talk->playDialogAudioDontWait(vm->getDialogTextId(barKeeperTextIdTbl[randTextId]));
			monksAtBarCurrentState = 3;
			specialOpCounter = 0;
			break;
		}
		case 3:
			monk1->updateSequence(0);
			monk2->updateSequence(8);
			specialOpCounter = 0x3c;
			monksAtBarCurrentState = 4;
			break;
		case 4:
			if (bVar1) {
				barKeeper->updateSequence(0xc);
				textIndex = sceneUpdateFuncDialogTbl[vm->getRand(3) + 3];
			} else {
				barKeeper->updateSequence(2);
				textIndex = DAT_800832f0[vm->getRand(4)];
			}
			vm->_talk->playDialogAudioDontWait(vm->getDialogTextId(textIndex));
			monksAtBarCurrentState = 5;
			specialOpCounter = 0;
			break;
		case 5:
			if (bVar1) {
				sequenceId = 8;
			} else {
				sequenceId = 0;
			}
			barKeeper->updateSequence(sequenceId);
			specialOpCounter = 0x78;
			if (!bVar1) {
				monksAtBarCurrentState = 0x37;
				break;
			}
			monksAtBarCurrentState = 6;
			break;
		case 6:
			if (bVar1) {
				barKeeper->updateSequence(0xb);
				ini->actor->updateSequence(7);
				monksAtBarCurrentState = 10;
			} else {
				barKeeper->updateSequence(0xd);
				monk1->updateSequence(0x14);
				monksAtBarCurrentState = 0x41;
			}
			specialOpCounter = 0;
			break;
		case 7:
			if (!barKeeper->isFlagSet(ACTOR_FLAG_4)) {
				break;
			}
			barKeeper->updateSequence(0xe);
			monk2->updateSequence(0x15);
			monksAtBarCurrentState = 8;
			specialOpCounter = 0;
			break;
		case 8:
			if (barKeeper->isFlagSet(ACTOR_FLAG_4)) {
				monk2->updateSequence(8);
				barKeeper->updateSequence(0);
				monksAtBarCurrentState = 9;
				specialOpCounter = 300;
			}
			break;
		case 9:
			sequenceId = 0x10;
			if (0x31 < vm->getRand(100)) {
				sequenceId = 0x16;
			}
			monk1->updateSequence(sequenceId);
			sequenceId = 0x11;
			if (0x31 < vm->getRand(100)) {
				sequenceId = 0x17;
			}
			monk2->updateSequence(sequenceId);
			monksAtBarCurrentState = 0;
			sVar2 = vm->getRand(10);
			specialOpCounter = (sVar2 + 10) * 0x3c;
			break;
		case 10:
			if (!barKeeper->isFlagSet(ACTOR_FLAG_4)) {
				break;
			}
			ini->actor->updateSequence(0);
			sequenceId = 8;
			monk2 = barKeeper;
			monk2->updateSequence(sequenceId);
			monksAtBarCurrentState = 0;
			sVar2 = vm->getRand(10);
			specialOpCounter = (sVar2 + 10) * 0x3c;
			break;
		case 0x37:
			barKeeper->updateSequence(0x11);
			specialOpCounter = 0x14;
			monksAtBarCurrentState = 6;
			break;
		case 0x41:
			if (!barKeeper->isFlagSet(ACTOR_FLAG_4)) {
				break;
			}
			barKeeper->updateSequence(0xf);
			monk1->updateSequence(0);
			monksAtBarCurrentState = 7;
			specialOpCounter = 0;
			break;
		}
	}
	vm->_scriptOpcodes->_specialOpCodes->setSpecialOpCounter(specialOpCounter);
	vm->_dragonINIResource->getRecord(0)->objectState = 0;
}

void flameEscapeSceneUpdateFunction() {
	static const uint32 dialogTbl[6] = {
		0x10458, 0x104A0, 0x10500, 0x10500, 0x10550, 0x10578
	};
	static bool DAT_800634c0 = false;
	DragonsEngine *vm = getEngine();
	Actor *flame = vm->_dragonINIResource->getRecord(0x96)->actor;

	if (!vm->isFlagSet(ENGINE_FLAG_8000)) {
		int16 specialOpCounter = vm->_scriptOpcodes->_specialOpCodes->getSpecialOpCounter();
		if (specialOpCounter == -1) {
			DAT_800634c0 = false;
			specialOpCounter = 300;
		}
		if (specialOpCounter != 0) {
			specialOpCounter = specialOpCounter + -1;
		}
		if (specialOpCounter == 0) {
			if (DAT_800634c0 == 0) {
				flame->updateSequence(0x12);
				vm->_talk->playDialogAudioDontWait(vm->getDialogTextId(dialogTbl[vm->getRand(6)]));
				specialOpCounter = 0;
				DAT_800634c0 = 1;
			} else {
				if (DAT_800634c0 == 1) {
					flame->updateSequence(0x10);
					specialOpCounter = (vm->getRand(0x14) + 10) * 0x3c;
					DAT_800634c0 = 0;
				}
			}
		}
		vm->_scriptOpcodes->_specialOpCodes->setSpecialOpCounter(specialOpCounter);
	} else {
		if (vm->_isLoadingDialogAudio && flame->_sequenceID != 0x10) {
			flame->updateSequence(0x10);
		}
	}
}

void caveOfDilemmaUpdateFunction() {
	static int32 counter = 0;
	static int16 direction = 1;
	int16 yOffset;
	Actor *oldManActor;
	Actor *cloudChairActor;
	DragonsEngine *vm = getEngine();

	if (counter == 0) {
		oldManActor = vm->_dragonINIResource->getRecord(0x161)->actor;
		cloudChairActor = vm->_dragonINIResource->getRecord(0x160)->actor;
		if (oldManActor->_y_pos < 0x53) {
			oldManActor->_y_pos = 0x52;
			cloudChairActor->_y_pos = 0x52;
			direction = 1;
		}
		if (0x5b < oldManActor->_y_pos) {
			oldManActor->_y_pos = 0x5c;
			cloudChairActor->_y_pos = 0x5c;
			direction = -1;
		}
		yOffset = direction * 2;
		oldManActor->_y_pos = oldManActor->_y_pos + yOffset;
		cloudChairActor->_y_pos = cloudChairActor->_y_pos + yOffset;
		counter = 10;
	} else {
		counter--;
	}
}

void moatDrainedSceneUpdateFunction() {
	static const uint32 moatDrainedTextIdTbl[4] = {
		0x3C97A, 0x3C9AC, 0x3C9F8, 0x3CA48
	};
	static uint16 moatDrainedUpdateCounter = 0;
	static bool moatDrainedStatus = false;
	DragonsEngine *vm = getEngine();

	if (vm->_scriptOpcodes->_specialOpCodes->getSpecialOpCounter() == -1) {
		moatDrainedUpdateCounter = 600;
	}
	castleFogUpdateFunction();
	if (((vm->_dragonINIResource->getRecord(0x208)->objectState2 == 2) &&
		 !vm->isFlagSet(ENGINE_FLAG_8000))) {
		if (moatDrainedUpdateCounter != 0) {
			moatDrainedUpdateCounter--;
		}
		if (moatDrainedUpdateCounter <= 0) {
			if (!moatDrainedStatus) {
				vm->_talk->playDialogAudioDontWait(vm->getDialogTextId(moatDrainedTextIdTbl[vm->getRand(4)]));
				moatDrainedStatus = true;
				moatDrainedUpdateCounter = 0x1e;
			} else {
				if (moatDrainedStatus) {
					vm->_dragonINIResource->getRecord(0x1fa)->actor->updateSequence(7);
					moatDrainedUpdateCounter = vm->getRand(300) + 0x4b0;
					moatDrainedStatus = false;
				}
			}
		}
	}
}

} // End of namespace Dragons
