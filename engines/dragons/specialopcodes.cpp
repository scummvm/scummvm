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

#include "dragons/cursor.h"
#include "dragons/cutscene.h"
#include "dragons/dragons.h"
#include "dragons/dragonflg.h"
#include "dragons/dragonini.h"
#include "dragons/dragonobd.h"
#include "dragons/inventory.h"
#include "dragons/specialopcodes.h"
#include "dragons/scene.h"
#include "dragons/actor.h"
#include "dragons/minigame1.h"
#include "talk.h"
#include "specialopcodes.h"
#include "minigame1.h"
#include "minigame4.h"


namespace Dragons {

// SpecialOpcodes

SpecialOpcodes::SpecialOpcodes(DragonsEngine *vm)
	: _vm(vm) {
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
	OPCODE(3, spcClearEngineFlag10);
	OPCODE(4, spcSetEngineFlag10);

	OPCODE(7, spcDancingMiniGame);
	OPCODE(8, spcCastleGardenLogic);
	OPCODE(9, spcUnk9);
	OPCODE(0xa, spcUnkA);
	OPCODE(0xb, clearSceneUpdateFunction);
	OPCODE(0xc, spcUnkC);
	OPCODE(0xd, spcFadeScreen);

	OPCODE(0x11, spc11ShakeScreen);
	OPCODE(0x12, spcHandleInventionBookTransition);
	OPCODE(0x13, spcUnk13InventionBookCloseRelated);
	OPCODE(0x14, spcClearEngineFlag8);
	OPCODE(0x15, spcSetEngineFlag8);

	OPCODE(0x17, spcKnightPoolReflectionLogic);
	OPCODE(0x18, clearSceneUpdateFunction);

	OPCODE(0x1a, spcActivatePizzaMakerActor);
	OPCODE(0x1b, spcDeactivatePizzaMakerActor);
	OPCODE(0x1c, spcPizzaMakerActorStopWorking);

	OPCODE(0x21, spcSetEngineFlag0x20000);
	OPCODE(0x22, spcClearEngineFlag0x20000);
	OPCODE(0x23, spcSetEngineFlag0x200000);
	OPCODE(0x24, spcClearEngineFlag0x200000);

	OPCODE(0x28, spcMonksAtBarSceneLogic);
	OPCODE(0x29, spcStopMonksAtBarSceneLogic);

	OPCODE(0x2b, spcFlameBedroomEscapeSceneLogic);
	OPCODE(0x2c, spcStopFlameBedroomEscapeSceneLogic);

	OPCODE(0x34, spcUnk34);

	OPCODE(0x36, spcFlickerClearFlag0x80);

	OPCODE(0x38, spcNoop1);
	OPCODE(0x39, spcTownAngryVillagersSceneLogic);

	OPCODE(0x3b, spcSetEngineFlag0x2000000);
	OPCODE(0x3c, spcClearEngineFlag0x2000000);
	OPCODE(0x3d, clearSceneUpdateFunction);
	OPCODE(0x3e, spcZigmondFraudSceneLogic);
	OPCODE(0x3f, clearSceneUpdateFunction);
	OPCODE(0x40, spcZigmondFraudSceneLogic1);

	OPCODE(0x42, spcDodoUnderAttackSceneLogic);

	OPCODE(0x46, spcBlackDragonOnHillSceneLogic);

	OPCODE(0x49, spcLoadScene1);

	OPCODE(0x4b, spcKnightsSavedCastleCutScene);

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

	OPCODE(0x63, spcSetInventorySequenceTo5);
	OPCODE(0x64, spcResetInventorySequence);
	OPCODE(0x65, spcUnk65ScenePaletteRelated);
	OPCODE(0x66, spcUnk66);

	OPCODE(0x6a, spcCastleGateSceneLogic);
	OPCODE(0x6b, spcTransitionToMap);
	OPCODE(0x6c, spcTransitionFromMap);

	OPCODE(0x77, spcJesterInLibrarySceneLogic);

	OPCODE(0x7a, spcBlackDragonDialogForCamelhot);
	OPCODE(0x7b, spcSetCameraXToZero);
	OPCODE(0x7c, spcDiamondIntroSequenceLogic);

	OPCODE(0x82, spc82CallResetDataMaybe);

	OPCODE(0x89, spcSetUnkFlag2);
	OPCODE(0x8a, spcClearUnkFlag2);

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

void SpecialOpcodes::spcClearEngineFlag10() {
	_vm->clearFlags(Dragons::ENGINE_FLAG_10);
}

void SpecialOpcodes::spcSetEngineFlag10() {
	_vm->setFlags(Dragons::ENGINE_FLAG_10);
}

void SpecialOpcodes::spcDancingMiniGame() {
	Minigame4 minigame4(_vm);
	minigame4.run();
}

void SpecialOpcodes::spcCastleGardenLogic() {
	//TODO
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
	//TODO call_fade_related_1f();
}

void SpecialOpcodes::spc11ShakeScreen() {
	//TODO
//	iVar18 = 1;
//	local_10a0 = DAT_8001170c;
//	local_109c = DAT_80011710;
//	local_1098 = DAT_80011714;
//	local_1094 = uint32_t_80011718;
//	local_1090 = DAT_8001171c;
//	local_108c = DAT_80011720;
//	local_1088 = DAT_80011724;
//	local_1084 = DAT_80011728;
//	screenShakeOffset = (short)local_10a0;
//	while (screenShakeOffset != 0) {
//		ContinueGame?();
//		screenShakeOffset = *(short *)((int)&local_10a0 + ((iVar18 << 0x10) >> 0xf));
//		iVar18 = iVar18 + 1;
//	}
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

void SpecialOpcodes::spcUnk34() {
	Actor *flicker = _vm->_dragonINIResource->getFlickerRecord()->actor;
	flicker->setFlag(ACTOR_FLAG_80);
	flicker->field_e = 0x100;
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
void SpecialOpcodes::spcSetEngineFlag0x2000000() {
	_vm->setFlags(Dragons::ENGINE_FLAG_2000000);
}

void SpecialOpcodes::spcClearEngineFlag0x2000000() {
	_vm->clearFlags(Dragons::ENGINE_FLAG_2000000);
}

void SpecialOpcodes::spcZigmondFraudSceneLogic() {
	//TODO
	sceneUpdater.numSteps[0] = 2;
	sceneUpdater.numSteps[1] = 2;
	sceneUpdater.numSteps[2] = 1;
	sceneUpdater.numSteps[3] = 1;
	sceneUpdater.numSteps[4] = 2;
	sceneUpdater.numSteps[5] = 2;
	sceneUpdater.numSteps[6] = 2;
	sceneUpdater.numSteps[7] = 2;

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

	//TODO field4
	setupTableBasedSceneUpdateFunction(0x168,8,0xb4);
}

void SpecialOpcodes::spcZigmondFraudSceneLogic1() {
	sceneUpdater.numSteps[0] = 1;
	sceneUpdater.iniIDTbl[0][0] = 0x197;
	sceneUpdater.sequenceIDTbl[0][0] = 0x12;
	sceneUpdater.textTbl[0][0] = 0x2F422; //TODO this might change between game versions

	setupTableBasedSceneUpdateFunction(300,1,0x708);
}

void SpecialOpcodes::spcDodoUnderAttackSceneLogic() {
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

void SpecialOpcodes::spc82CallResetDataMaybe() {
	//TODO callMaybeResetData();
}

void SpecialOpcodes::spcSetUnkFlag2() {
	_vm->setUnkFlags(ENGINE_UNK1_FLAG_2);
}

void SpecialOpcodes::spcClearUnkFlag2() {
	_vm->clearUnkFlags(ENGINE_UNK1_FLAG_2);
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

void pizzaUpdateFunction() {
		static int16 DAT_800634bc = 0;
		DragonsEngine *vm = getEngine();

		Actor *actorf4 = vm->getINI(0xf4)->actor;
		Actor *actorf5 = vm->getINI(0xf5)->actor;

		if (DAT_800634bc == 0) {
			if (actorf4->isFlagSet(ACTOR_FLAG_4)) {
				if (actorf4->_sequenceID == 0) {
					actorf4->updateSequence(1);
				} else {
					if (actorf4->_sequenceID == 1) {
						actorf4->updateSequence(2);
						actorf5->x_pos = 0x115;
						actorf5->y_pos = 0x5c;
						actorf5->updateSequence(7);
						DAT_800634bc = 0x2d;
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
			DAT_800634bc--;
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
				//TODO FUN_80036a68((&DAT_800832d8)[sceneUpdateSequenceTbl[uVar3].field_0x4[(uint) spc->sceneUpdater.curSequenceIndex]]);
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

} // End of namespace Dragons
