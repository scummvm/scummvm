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
#include "dragons/specialopcodes.h"
#include "dragons/screen.h"
#include "dragons/minigame1.h"
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
	OPCODE(0x82, spc82CallResetDataMaybe);
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
	minigame2.run(0,1,1);
}

void SpecialOpcodes::spcClearEngineFlag10() {
	_vm->clearFlags(Dragons::ENGINE_FLAG_10);
}

void SpecialOpcodes::spcSetEngineFlag10() {
	_vm->setFlags(Dragons::ENGINE_FLAG_10);
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
	sceneUpdater.textTbl[0][0] = 0x22660;
	sceneUpdater.textTbl[0][1] = 0x226CA;
	sceneUpdater.textTbl[1][0] = 0x22738;
	sceneUpdater.textTbl[1][1] = 0x22790;
	sceneUpdater.textTbl[2][0] = 0x227E8;
	sceneUpdater.textTbl[2][1] = 0x2283C;
	sceneUpdater.textTbl[3][0] = 0x228A0;
	sceneUpdater.textTbl[3][1] = 0x228EC;
	if (_vm->getCurrentSceneId() == 0x1a) {
		sceneUpdater.sequenceIDTbl[0][0] = 1;
		sceneUpdater.sequenceIDTbl[1][0] = 1;
		sceneUpdater.sequenceIDTbl[2][0] = 1;
		sceneUpdater.sequenceIDTbl[3][0] = 1;
		sceneUpdater.iniIDTbl[3][0] = 0x145;
		sceneUpdater.iniIDTbl[4][0] = 0x144;
	}
	else {
		sceneUpdater.sequenceIDTbl[0][0] = -1;
		sceneUpdater.sequenceIDTbl[1][0] = -1;
		sceneUpdater.sequenceIDTbl[2][0] = -1;
		sceneUpdater.sequenceIDTbl[3][0] = -1;
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
	setupTableBasedSceneUpdateFunction(1,4,0xb4);
}

void SpecialOpcodes::spcUnk9() {
	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	assert(flicker);
	flicker->field_1a_flags_maybe |= Dragons::INI_FLAG_20;
	assert(flicker->actor);
	flicker->actor->flags |= Dragons::ACTOR_FLAG_100;
	flicker->actor->priorityLayer = 0;
	_vm->getINI(1)->field_1a_flags_maybe |= Dragons::INI_FLAG_20;
}


void SpecialOpcodes::spcUnkA() {
	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	flicker->field_1a_flags_maybe &= ~Dragons::INI_FLAG_20;
	flicker->actor->flags &= ~Dragons::ACTOR_FLAG_100;
	_vm->getINI(1)->field_1a_flags_maybe &= ~Dragons::INI_FLAG_20;
}

void SpecialOpcodes::spcUnkC() {
	//TODO fade_related_calls_with_1f();
}

void SpecialOpcodes::spcFadeScreen() {
	_vm->call_fade_related_1f();
}

void SpecialOpcodes::spcLadyOfTheLakeCapturedSceneLogic() {
	//TODO
	//DAT_80083148 = 0;
	//DAT_80083154 = 0;
	_vm->setSceneUpdateFunction(ladyOfTheLakeCapturedUpdateFunction);
}

void SpecialOpcodes::spcStopLadyOfTheLakeCapturedSceneLogic() {
	_vm->setSceneUpdateFunction(NULL);
// TODO
	_vm->_sound->PauseCDMusic();
//	if ((DAT_80083148 != 0) || (DAT_80083154 != 0)) {
//		FUN_8001ac5c((uint)DAT_80083148,(uint)DAT_80083150,(uint)DAT_80083154,(uint)DAT_80083158);
//	}
//	DAT_80083148 = 0;
//	DAT_80083154 = 0;
}

void SpecialOpcodes::spc11ShakeScreen() {
	for (int i = 0; i < 16; i++ ) {
		_vm->_screen->setScreenShakeOffset(0, shakeTbl[i]);
		_vm->waitForFrames(1);
	}
}

void SpecialOpcodes::spcHandleInventionBookTransition() {
	int16 invType =_vm->_inventory->getType();
	if (invType == 1) {
		_vm->_inventory->closeInventory();
		_vm->_inventory->setType(0);
	}
	if (invType == 2) {
		_vm->_inventory->closeInventionBook();
		_vm->_inventory->setType(0);
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
	_vm->clearFlags(Dragons::ENGINE_FLAG_8);
}

void SpecialOpcodes::spcSetEngineFlag8() {
	_vm->setFlags(Dragons::ENGINE_FLAG_8);
}

void SpecialOpcodes::spcKnightPoolReflectionLogic() {
	//TODO
	/*
	sceneUpdateTblTextId0[0] = DAT_80063c10;
	sceneUpdateTblTextId0[1] = DAT_80063c14;
	sceneUpdateTblTextId0[2] = DAT_80063c18;
	sceneUpdateTblTextId0[3] = DAT_80063c1c;
	sceneUpdateTblTextId1[0] = DAT_80063c20;
	sceneUpdateTblTextId1[1] = DAT_80063c24;
	sceneUpdateTblTextId2[0] = DAT_80063c28;
	sceneUpdateTblTextId2[1] = DAT_80063c2c;
	sceneUpdateTblTextId2[2] = DAT_80063c30;
	sceneUpdateTblTextId2[3] = DAT_80063c34;
	sceneUpdateTblTextId2[4] = DAT_80063c38;
	sceneUpdateTblTextId3[0] = DAT_80063c3c;
	sceneUpdateTblTextId3[1] = DAT_80063c40;
	sceneUpdateTblTextId3[2] = DAT_80063c44;
	sceneUpdateTblTextId3[3] = DAT_80063c48;
	sceneUpdateTblTextId4[0] = DAT_80063c4c;
	sceneUpdateTblTextId4[1] = DAT_80063c50;
	sceneUpdateTblTextId4[2] = DAT_80063c54;
	sceneUpdateTblTextId4[3] = DAT_80063c58;
	sceneUpdateTblTextId5[0] = DAT_80063c5c;
	sceneUpdateTblTextId5[1] = DAT_80063c60;
	sceneUpdateTblTextId5[2] = DAT_80063c64;
	sceneUpdateTblIniId0[0] = DAT_80063bb0;
	sceneUpdateTblIniId0[1] = DAT_80063bac;
	sceneUpdateTblIniId0[2] = DAT_80063bb0;
	sceneUpdateTblIniId0[3] = DAT_80063bac;
	sceneUpdateTblIniId1[0] = DAT_80063bb0;
	sceneUpdateTblIniId1[1] = DAT_80063bac;
	sceneUpdateTblIniId2[0] = DAT_80063bb0;
	sceneUpdateTblIniId2[1] = DAT_80063bac;
	sceneUpdateTblIniId2[2] = DAT_80063bb0;
	sceneUpdateTblIniId2[3] = DAT_80063bac;
	sceneUpdateTblIniId2[4] = DAT_80063bb0;
	sceneUpdateTblIniId3[0] = DAT_80063bb0;
	sceneUpdateTblIniId3[1] = DAT_80063bac;
	sceneUpdateTblIniId3[2] = DAT_80063bb0;
	sceneUpdateTblIniId3[3] = DAT_80063bac;
	sceneUpdateTblIniId4[0] = DAT_80063bac;
	sceneUpdateTblIniId4[1] = DAT_80063bb0;
	sceneUpdateTblIniId4[2] = DAT_80063bac;
	sceneUpdateTblIniId4[3] = DAT_80063bb0;
	sceneUpdateTblIniId5[0] = DAT_80063bb0;
	sceneUpdateTblIniId5[1] = DAT_80063bac;
	sceneUpdateTblIniId5[2] = DAT_80063bb0;
	sceneUpdateTblSeqId0[0] = 10;
	sceneUpdateTblSeqId0[1] = 2;
	sceneUpdateTblSeqId0[2] = 10;
	sceneUpdateTblSeqId0[3] = 2;
	sceneUpdateTblSeqId1[0] = 10;
	sceneUpdateTblSeqId1[1] = 2;
	sceneUpdateTblSeqId2[0] = 10;
	sceneUpdateTblSeqId2[1] = 2;
	sceneUpdateTblSeqId2[2] = 10;
	sceneUpdateTblSeqId2[3] = 2;
	sceneUpdateTblSeqId2[4] = 10;
	sceneUpdateTblSeqId3[0] = 10;
	sceneUpdateTblSeqId3[1] = 2;
	sceneUpdateTblSeqId3[2] = 10;
	sceneUpdateTblSeqId3[3] = 2;
	sceneUpdateTblSeqId4[0] = 2;
	sceneUpdateTblSeqId4[1] = 10;
	sceneUpdateTblSeqId4[2] = 2;
	sceneUpdateTblSeqId4[3] = 10;
	sceneUpdateTblSeqId5[0] = 10;
	sceneUpdateTblSeqId5[2] = 10;
	sceneUpdateSequenceTbl[2].numSteps = 5;
	sceneUpdateTblSeqId5[1] = 2;
	sceneUpdateSequenceTbl[1].numSteps = 2;
	sceneUpdateSequenceTbl[0].numSteps = 4;
	sceneUpdateSequenceTbl[3].numSteps = 4;
	sceneUpdateSequenceTbl[4].numSteps = 4;
	sceneUpdateSequenceTbl[5].numSteps = 3;
	sceneUpdateSequenceTbl[0].textTbl = sceneUpdateTblTextId0;
	sceneUpdateSequenceTbl[1].textTbl = sceneUpdateTblTextId1;
	sceneUpdateSequenceTbl[2].textTbl = sceneUpdateTblTextId2;
	sceneUpdateSequenceTbl[3].textTbl = sceneUpdateTblTextId3;
	sceneUpdateSequenceTbl[4].textTbl = sceneUpdateTblTextId4;
	sceneUpdateSequenceTbl[5].textTbl = sceneUpdateTblTextId5;
	sceneUpdateSequenceTbl[5].sequenceIdPtr = sceneUpdateTblSeqId5;
	sceneUpdateSequenceTbl[4].sequenceIdPtr = sceneUpdateTblSeqId4;
	sceneUpdateSequenceTbl[0].iniIdPtr = sceneUpdateTblIniId0;
	sceneUpdateSequenceTbl[1].iniIdPtr = sceneUpdateTblIniId1;
	sceneUpdateSequenceTbl[2].iniIdPtr = sceneUpdateTblIniId2;
	sceneUpdateSequenceTbl[3].iniIdPtr = sceneUpdateTblIniId3;
	sceneUpdateSequenceTbl[4].iniIdPtr = sceneUpdateTblIniId4;
	sceneUpdateSequenceTbl[5].iniIdPtr = sceneUpdateTblIniId5;
	sceneUpdateSequenceTbl[0].sequenceIdPtr = sceneUpdateTblSeqId0;
	sceneUpdateSequenceTbl[1].sequenceIdPtr = sceneUpdateTblSeqId1;
	sceneUpdateSequenceTbl[2].sequenceIdPtr = sceneUpdateTblSeqId2;
	sceneUpdateSequenceTbl[3].sequenceIdPtr = sceneUpdateTblSeqId3;
	setupTableBasedSceneUpdateFunction(0x168,6,300);
	 */
}

void SpecialOpcodes::spcWalkOnStilts() {
	//TODO
	ushort uVar1;
	bool bVar2;
	void *pvVar3;
	uint actorId;
	uint uVar4;

	Actor *actor = _vm->_dragonINIResource->getRecord(0x2a0)->actor;
	Actor *actor1 = _vm->_dragonINIResource->getRecord(0x2a9)->actor;

	actor->_walkSlopeX = actor->_walkSlopeX / 3;
	actor->_walkSlopeY = actor->_walkSlopeY / 3;
	bVar2 = false;
	while (actor->isFlagSet(ACTOR_FLAG_10)) {
		//pvVar3 = actor->frame_pointer_maybe;
		if (actor->frame->field_c == 0) { //*(short *)((int)pvVar3 + 0xc) == 0) {
			bVar2 = false;
		}
		else {
			if (((!bVar2) && (0x6a < actor->y_pos)) && (actor->y_pos < 0x96)) {
				bVar2 = true;
				actor1->x_pos = actor->x_pos - READ_LE_INT16(actor->frame->frameDataOffset); //*(short *)((int)pvVar3 + 0xe);
				actor1->y_pos = actor->y_pos - READ_LE_INT16(actor->frame->frameDataOffset + 2); //*(short *)((int)pvVar3 + 0x10);
				actor1->updateSequence(9);
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
		_vm->setSceneUpdateFunction(NULL);
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
	ini121->actor = _vm->_actorManager->loadActor(0x48, 4, ini121->actor->x_pos, ini121->actor->y_pos);
	origActor->reset_maybe();
//	reset_actor_maybe();
	ini121->actor->setFlag(ACTOR_FLAG_80);
	ini121->actor->scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	ini121->actor->priorityLayer = 2;
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
	_vm->_dragonINIResource->getFlickerRecord()->actor->priorityLayer = 2;
}

void SpecialOpcodes::spcMenInMinesSceneLogic() {
	//TODO
}

void SpecialOpcodes::spcStopMenInMinesSceneLogic() {
	//TODO
}

void SpecialOpcodes::spcMonksAtBarSceneLogic() {
	//TODO
}

void SpecialOpcodes::spcStopMonksAtBarSceneLogic() {
	//TODO
}

void SpecialOpcodes::spcFlameBedroomEscapeSceneLogic() {
	//TODO
//	local_fc8 = DAT_80063cb8;
//	local_fc4 = DAT_80063cbc;
//	local_fc0 = DAT_80063cc0;
//	local_fbc = DAT_80063cc0;
//	local_fb8 = DAT_80063cc8;
//	local_fb4 = DAT_80063ccc;
//	local_ff0[0] = DAT_80063cb8;
//	local_ff0[1] = DAT_80063cbc;
//	local_ff0[2] = DAT_80063cc0;
//	local_ff0[3] = DAT_80063cc0;
//	local_fe0 = DAT_80063cc8;
//	local_fdc = DAT_80063ccc;
//	DisableVSyncEvent();
//	uVar13 = 0;
//	uVar7 = 0;
//	do {
//		uVar13 = uVar13 + 1;
//		(&DAT_800832d8)[uVar7] = local_ff0[uVar7];
//		uVar7 = (uint)uVar13;
//	} while (uVar13 < 6);
//	EnableVSyncEvent();
//	DAT_80072898 = 0xffff;
//	if ((DAT_80083148 != 0) && (DAT_80083154 != 0)) {
//		FUN_8001ac5c((uint)DAT_80083148,(uint)DAT_80083150,(uint)DAT_80083154,(uint)DAT_80083158);
//	}
//	func_ptr_unk = FUN_80038164;
}

void SpecialOpcodes::spcStopFlameBedroomEscapeSceneLogic() {
	//TODO
//	DAT_80072898 = 0;
//	FUN_8001ac5c((uint)DAT_80083148,(uint)DAT_80083150,(uint)DAT_80083154,(uint)DAT_80083158);
//	DAT_80083154 = 0;
//	DAT_80083148 = 0;
//	actor_update_sequenceID((uint)(ushort)dragon_ini_pointer[DAT_80063a3c + -1].field_0x1c,0);
//	if (func_ptr_unk == FUN_80038164) {
//		func_ptr_unk = 0;
//	}
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
	// TODO spcCastleGateMoatDrainedSceneLogic
}
void SpecialOpcodes::spcUnk34() {
	Actor *flicker = _vm->_dragonINIResource->getFlickerRecord()->actor;
	flicker->setFlag(ACTOR_FLAG_80);
	flicker->scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
}

void SpecialOpcodes::spcFlickerClearFlag0x80() {
	_vm->_dragonINIResource->getFlickerRecord()->actor->clearFlag(ACTOR_FLAG_80);
}

void SpecialOpcodes::spcNoop1() {
	//No op
}

void SpecialOpcodes::spcTownAngryVillagersSceneLogic() {
	//TODO
}

void SpecialOpcodes::spcBlackDragonCrashThroughGate() {
	for (int i = 0; i < 16; i++ ) {
		_vm->_screen->setScreenShakeOffset(shakeTbl[i], shakeTbl[i]);
		_vm->waitForFrames(1);
	}
}

void SpecialOpcodes::spcSetEngineFlag0x2000000() {
	_vm->setFlags(Dragons::ENGINE_FLAG_2000000);
}

void SpecialOpcodes::spcClearEngineFlag0x2000000() {
	_vm->clearFlags(Dragons::ENGINE_FLAG_2000000);
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

	sceneUpdater.textTbl[0][0] = 0x2D000;
	sceneUpdater.textTbl[0][1] = 0x2D044;
	sceneUpdater.textTbl[1][0] = 0x2D0B2;
	sceneUpdater.textTbl[1][1] = 0x2D0D6;
	sceneUpdater.textTbl[2][0] = 0x2D152;
	sceneUpdater.textTbl[3][0] = 0x2D1A4;
	sceneUpdater.textTbl[4][0] = 0x2D20A;
	sceneUpdater.textTbl[4][1] = 0x2D27C;
	sceneUpdater.textTbl[5][0] = 0x2D2EC;
	sceneUpdater.textTbl[5][1] = 0x2D336;
	sceneUpdater.textTbl[6][0] = 0x2D3E0;
	sceneUpdater.textTbl[6][1] = 0x2D456;
	sceneUpdater.textTbl[7][0] = 0x2D4A8;
	sceneUpdater.textTbl[7][1] = 0x2D504;

	setupTableBasedSceneUpdateFunction(0x168,8,0xb4);
}

void SpecialOpcodes::spcZigmondFraudSceneLogic1() {
	sceneUpdater.numSteps[0] = 1;
	sceneUpdater.iniIDTbl[0][0] = 0x197;
	sceneUpdater.sequenceIDTbl[0][0] = 0x12;
	sceneUpdater.textTbl[0][0] = 0x2F422; //TODO this might change between game versions

	setupTableBasedSceneUpdateFunction(300,1,0x708);
}

void SpecialOpcodes::spcBrokenBlackDragonSceneLogic() {
	//TODO spcBrokenBlackDragonSceneLogic
}

void SpecialOpcodes::spcDodoUnderAttackSceneLogic() {
	//TODO
	sceneUpdater.sequenceID = -1;
}

void SpecialOpcodes::spcForestWithoutDodoSceneLogic() {
	//TODO
	sceneUpdater.sequenceID = -1;
}

void SpecialOpcodes::spcBlackDragonOnHillSceneLogic() {
	//TODO
	sceneUpdater.sequenceID = -1;
}

void SpecialOpcodes::spcUnk4e() {
	panCamera(1);
}

void SpecialOpcodes::spcUnk4f() {
	panCamera(2);
}

void SpecialOpcodes::spcCloseInventory() {
	if (_vm->_inventory->getType() == 1) {
		_vm->_inventory->closeInventory();
		_vm->_inventory->setType(0);
	}
}

void SpecialOpcodes::spcOpenInventionBook() {
	if (_vm->_inventory->getType() == 1) {
		_vm->_inventory->closeInventory();
	}
	_vm->_inventory->openInventionBook();
	_vm->_inventory->setType(2);
}

void SpecialOpcodes::spcCloseInventionBook() {
	_vm->_inventory->closeInventionBook();
	_vm->_inventory->setType(0);
}

void SpecialOpcodes::spcSetEngineFlag0x4000000() {
	_vm->setFlags(Dragons::ENGINE_FLAG_4000000);
}

void SpecialOpcodes::spcClearEngineFlag0x4000000() {
	_vm->clearFlags(Dragons::ENGINE_FLAG_4000000);
}

void SpecialOpcodes::spcSetCursorSequenceIdToZero() {
	_vm->_cursor->_sequenceID = 0;
}

void SpecialOpcodes::spcFlickerSetFlag0x80() {
	_vm->_dragonINIResource->getFlickerRecord()->actor->setFlag(ACTOR_FLAG_80);
}

//used in castle garden scene with knights
void SpecialOpcodes::spcUnk5d() {
	_vm->getINI(_vm->getINI(0x13b)->field_12 + 0x13c)->actor->updateSequence(_vm->getINI(0x13b)->field_14);
}

void SpecialOpcodes::spcUnk5e() {
	panCamera(1);
	_vm->_dragonINIResource->setFlickerRecord(_vm->getINI(0));
}

void SpecialOpcodes::spcUnk5f() {
	_vm->getINI(0x2ab)->field_12 = 0;
	panCamera(2);
}

void SpecialOpcodes::spcCastleBuildBlackDragonSceneLogic() {
	_vm->setSceneUpdateFunction(castleBuildingBlackDragon2UpdateFunction);
}

void SpecialOpcodes::spcStopSceneUpdateFunction() {
	_vm->setSceneUpdateFunction(NULL);
}

void SpecialOpcodes::spcSetInventorySequenceTo5() {
	_vm->_inventory->updateActorSequenceId(5);
}

void SpecialOpcodes::spcResetInventorySequence() {
	_vm->_inventory->resetSequenceId();
}

void SpecialOpcodes::spcUnk65ScenePaletteRelated() {
	//TODO
//	uint uVar1;
//	ushort uVar2;
//	RECT local_10;
//
//	uVar2 = 0xb1;
//	local_10.y = 0xf1;
//	local_10.w = 0x100;
//	local_10.x = 0;
//	local_10.h = 1;
//	uVar1 = 0xb1;
//	do {
//		uVar2 = uVar2 + 1;
//		*(undefined2 *)(uVar1 * 2 + scrFileData_maybe) = 0;
//		uVar1 = (uint)uVar2;
//	} while (uVar2 < 0xc0);
//	LoadImage(&local_10,&palette_data);
//	DrawSync(0);
}

void SpecialOpcodes::spcUnk66() {
	uint16 var =_vm->getVar(2);

	uint16 bVar1 = (var & 1) == 0;
	uint16 uVar9 = bVar1;
	if ((var & 4) == 0) {
		uVar9 = (ushort)bVar1 + 1;
	}
	if ((var & 2) == 0) {
		uVar9 = uVar9 + 1;
	}
	_vm->getINI(1)->field_12 = uVar9;
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
//TODO spcCastleGateSceneLogic
}

// 0x80038c1c
void SpecialOpcodes::panCamera(int16 mode) {
	int iVar1;
	int iVar2;

	if (mode == 1) {
		_vm->getINI(0x2ab)->field_12 = _vm->_scene->_camera.x;
		_vm->_dragonINIResource->setFlickerRecord(NULL);
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
	_vm->_talk->loadText(0x30DD8, buffer, 1024); //TODO might need to check dialog in other game versions
	_vm->_talk->displayDialogAroundPoint(buffer,0x27,0xc,0xc01,0,0x30DD8);
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
	//TODO here.....
}

void SpecialOpcodes::spc82CallResetDataMaybe() {
	//TODO callMaybeResetData();
}

void SpecialOpcodes::spcStopScreenShakeUpdater() {
	_vm->setSceneUpdateFunction(NULL);
	_vm->_screen->setScreenShakeOffset(0, 0);
}

void SpecialOpcodes::spcInsideBlackDragonScreenShake() {
	for (int i = 0; i < 5; i ++) {
		_vm->_screen->setScreenShakeOffset(0, shakeTbl[i]);
		_vm->waitForFrames(1);
	}
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
		_vm->_dragonINIResource->getRecord(0x169)->field_12 = 1;
	} else {
		_vm->_dragonINIResource->getRecord(0x169)->field_12 = 0;
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
	//TODO map transition
//	DAT_8006a422 = 0;
//	DAT_8006a424 = 0;
//	cursorSequenceId = 0;
//	_vm->waitForFrames();
//	engine_flags_maybe = engine_flags_maybe | 0x20000000;
//	FUN_80023b34(0,0,1);
}

void SpecialOpcodes::spcTransitionFromMap() {
	//TODO map transition
}

void SpecialOpcodes::spcCaveOfDilemmaSceneLogic() {
	_vm->setSceneUpdateFunction(caveOfDilemmaUpdateFunction);
}

void SpecialOpcodes::spcFadeCreditsToBackStageScene() {
//	call_fade_related();
	_vm->_screen->loadPalette(0, _vm->_scene->getPalette());
	_vm->_scene->_camera.x = 0x140;
	_vm->waitForFrames(0x3c);
//	Call_SomeShadeTexThingB1E();
//TODO
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
//	buf2048bytes = (int32_t *)((iVar1 + 3U & 0xfffffffc) + (int)buf2048bytes);
//	memcpy2((byte *)buf2048bytes,scrFileData_maybe,0x200);
//	buf2048bytes = buf2048bytes + 0x80;
	_vm->_screen->loadPalette(0, _vm->_dragonINIResource->getRecord(0x2C8)->actor->_actorResource->getPalette());
	_vm->_scene->setMgLayerPriority(0);
	_vm->_scene->setFgLayerPriority(0);
//	vsync_updater_function = creditsUpdateFunction;
	_vm->_credits->start();
}

void SpecialOpcodes::spcEndCreditsAndRestartGame() {
//	call_fade_related();
//	_volumeSFX = 0;
//	setCDAVolumes();
	while (_vm->_credits->isRunning()) {
		_vm->waitForFrames(1);
	}
//	ReloadGameFlag = 2;
//	Exec_FMV_RELOADTT();
//TODO need to return to main menu here.
}

void SpecialOpcodes::spcLoadLadyOfTheLakeActor() {
	//TODO
	DragonINI *ini = _vm->_dragonINIResource->getRecord(0x2a7);
	_vm->_actorManager->loadActor(0xcd, ini->actor->_actorID);
	ini->actor->setFlag(ACTOR_FLAG_4);
	ini->actorResourceId = 0xcd;
	//DisableVSyncEvent();
//	uVar17 = (uint)(ushort)dragon_ini_pointer[DAT_8006398c + -1].field_0x1c;
//	uVar7 = load_actor_file(0xcc);
//	file_read_to_buffer(s_s12a6.act_80011740,(&actor_dictionary)[(uVar7 & 0xffff) * 2]);
//	actors[uVar17].﻿actorFileDictionaryIndex = (uint16_t)uVar7;
//	actors[uVar17].﻿resourceID = 0xcd;
//	iVar18 = DAT_8006398c;
//	actors[uVar17].flags = actors[uVar17].flags | 4;
//	dragon_ini_pointer[iVar18 + -1].x = 0xcd;
//	LAB_8002ad94:
	//EnableVSyncEvent();
}

void SpecialOpcodes::spcUseClickerOnLever() {
	if (_vm->_inventory->getType() != 0) {
		_vm->_talk->flickerRandomDefaultResponse();
		_vm->_dragonINIResource->getRecord(0)->field_12 = 1;
	} else {
		_vm->_dragonINIResource->getRecord(0)->field_12 = 0;
	}
}

void SpecialOpcodes::spcJesterInLibrarySceneLogic() {
	//TODO
}

void SpecialOpcodes::pizzaMakerStopWorking() {
	Actor *actorf4 = _vm->getINI(0xf4)->actor;
	Actor *actorf5 = _vm->getINI(0xf5)->actor;

	if (actorf4->_sequenceID != 0) {
		if (actorf4->_sequenceID == 1) {
			actorf4->waitUntilFlag8And4AreSet();
			actorf4->updateSequence(2);
			actorf5->x_pos = 0x115;
			actorf5->y_pos = 0x5c;
			actorf5->updateSequence(7);
			_vm->waitForFrames(0x78);
			actorf5->updateSequence(8);
			actorf5->waitUntilFlag8And4AreSet();
			actorf5->x_pos = 0xff9c;
			actorf5->y_pos = 100;
			actorf4->updateSequence(3);
		}
		else {
			if (actorf4->_sequenceID == 2) {
				_vm->waitForFrames(0x78);
				actorf5->updateSequence(8);
				actorf5->waitUntilFlag8And4AreSet();
				actorf5->x_pos = 0xff9c;
				actorf5->y_pos = 100;
				actorf4->updateSequence(3);
			}
			else {
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
//TODO
//	if (DAT_80083148 != DAT_80083154) {
//		FUN_8001ac5c((uint)DAT_80083148,(uint)DAT_80083150,(uint)DAT_80083154,(uint)DAT_80083158);
//	}
	if (sceneUpdater.sequenceID != -1) {
		_vm->getINI(sceneUpdater.iniID)->actor->updateSequence(sceneUpdater.sequenceID);
	}
	_vm->setSceneUpdateFunction(NULL);
}

void SpecialOpcodes::setupTableBasedSceneUpdateFunction(uint16 initialCounter, uint16 numSequences,
														uint16 sequenceDuration) {
	sceneUpdater.sequenceID = -1;
//TODO
//	DAT_80083154 = 0;
//	DAT_80083148 = 0;
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
	actor->priorityLayer = 2;
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
						actorf5->x_pos = 0x115;
						actorf5->y_pos = 0x5c;
						actorf5->updateSequence(7);
						counter = 0x2d;
						return;
					}
					if (actorf4->_sequenceID == 2) {
						if ((actorf5->_sequenceID == 8) &&
							(actorf5->isFlagSet(ACTOR_FLAG_4))) {
							actorf5->x_pos = -100;
							actorf5->y_pos = 100;
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
		}
		else {
			counter--;
		}
}

void tableBasedSceneUpdateFunction() {
	uint uVar3;
	DragonsEngine *vm = getEngine();
	SpecialOpcodes *spc = vm->_scriptOpcodes->_specialOpCodes;

	uVar3 = (uint)spc->sceneUpdater.curSequence;
	if (!vm->isFlagSet(ENGINE_FLAG_8000) || vm->data_800633fc == 1) {
		if (spc->sceneUpdater.sequenceID != -1) {
			vm->getINI(spc->sceneUpdater.iniID)->actor->updateSequence(spc->sceneUpdater.sequenceID);
			spc->sceneUpdater.sequenceID = -1;
		}
		if (vm->data_800633fc == 0) {
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
	if (ini->field_10 <= 0) {
		if (ini->field_12 == 0) {
			ini->actor->updateSequence(0xb);
			ini->field_10 = 0x68;
			ini->field_12 = 1;
		}
		else if (ini->field_12 == 1) {
			ini->actor->updateSequence(4);
			ini->field_10 = vm->getRand(0xb4);
			ini->field_12 = 0;
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
	}
	else {
		counter--;
	}
	vm->_scriptOpcodes->_specialOpCodes->setSpecialOpCounter(counter);
}

void ladyOfTheLakeCapturedUpdateFunction() {
	const uint32 dialogTbl[] = {
			0x490C8, 0x490FC, 0x4913A
	};
	static int ladyofLakeCountdownTimer = 0x12c;
	static uint8 ladyOfLakeDialogIndex = 0;
	DragonsEngine *vm = getEngine();

	if (!vm->isFlagSet(ENGINE_FLAG_8000)) {
		ladyofLakeCountdownTimer--;
		if (ladyofLakeCountdownTimer == 0) {
			vm->_talk->playDialogAudioDontWait(dialogTbl[ladyOfLakeDialogIndex]);
			if (ladyOfLakeDialogIndex == 2) {
				ladyOfLakeDialogIndex = 0;
			} else {
				ladyOfLakeDialogIndex++;
			}
			ladyofLakeCountdownTimer = 0x708;
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
		if (oldManActor->y_pos < 0x53) {
			oldManActor->y_pos = 0x52;
			cloudChairActor->y_pos = 0x52;
			direction = 1;
		}
		if (0x5b < oldManActor->y_pos) {
			oldManActor->y_pos = 0x5c;
			cloudChairActor->y_pos = 0x5c;
			direction = -1;
		}
		yOffset = direction * 2;
		oldManActor->y_pos = oldManActor->y_pos + yOffset;
		cloudChairActor->y_pos = cloudChairActor->y_pos + yOffset;
		counter = 10;
	}
	else {
		counter--;
	}
}
} // End of namespace Dragons
