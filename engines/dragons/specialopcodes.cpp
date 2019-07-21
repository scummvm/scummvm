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
#include "dragons/dragons.h"
#include "dragons/dragonflg.h"
#include "dragons/dragonini.h"
#include "dragons/dragonobd.h"
#include "dragons/inventory.h"
#include "dragons/specialopcodes.h"
#include "dragons/scene.h"
#include "dragons/actor.h"
#include "specialopcodes.h"


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
	// OPCODE(1, opUnk1);
	OPCODE(3, spcClearEngineFlag10);
	OPCODE(4, spcSetEngineFlag10);

	OPCODE(9, spcUnk9);
	OPCODE(0xa, spcUnkA);

	OPCODE(0xc, spcUnkC);

	OPCODE(0x12, spcHandleInventionBookTransition);
	OPCODE(0x13, spcUnk13InventionBookCloseRelated);
	OPCODE(0x14, spcClearEngineFlag8);
	OPCODE(0x15, spcSetEngineFlag8);

	OPCODE(0x1a, spcActivatePizzaMakerActor);
	OPCODE(0x1b, spcDeactivatePizzaMakerActor);
	OPCODE(0x1c, spcPizzaMakerActorStopWorking);

	OPCODE(0x21, spcSetEngineFlag0x20000);
	OPCODE(0x22, spcClearEngineFlag0x20000);
	OPCODE(0x23, spcSetEngineFlag0x200000);
	OPCODE(0x24, spcClearEngineFlag0x200000);

	OPCODE(0x36, spcFlickerClearFlag0x80);

	OPCODE(0x3b, spcSetEngineFlag0x2000000);
	OPCODE(0x3c, spcClearEngineFlag0x2000000);

	OPCODE(0x49, spcLoadScene1);

	OPCODE(0x4e, spcUnk4e);
	OPCODE(0x4f, spcUnk4f);
	OPCODE(0x50, spcCloseInventory);
	OPCODE(0x51, spcOpenInventionBook);
	OPCODE(0x52, spcCloseInventionBook);
	OPCODE(0x53, spcClearEngineFlag0x4000000);
	OPCODE(0x54, spcSetEngineFlag0x4000000);
	OPCODE(0x55, spcSetCursorSequenceIdToZero);

	OPCODE(0x5b, spcFlickerSetFlag0x80);

	OPCODE(0x5e, spcUnk5e);
	OPCODE(0x5f, spcUnk5f);

	OPCODE(0x6b, spcTransitionToMap);
	OPCODE(0x6c, spcTransitionFromMap);

	OPCODE(0x7b, spcSetCameraXToZero);
	OPCODE(0x7c, spcDiamondIntroSequenceLogic);

}

#undef OPCODE

void SpecialOpcodes::freeOpcodes() {
	for (uint i = 0; i < DRAGONS_NUM_SPECIAL_OPCODES; ++i) {
		delete _opcodes[i];
	}
}

// Opcodes

void SpecialOpcodes::spcClearEngineFlag10() {
	_vm->clearFlags(Dragons::ENGINE_FLAG_10);
}

void SpecialOpcodes::spcSetEngineFlag10() {
	_vm->setFlags(Dragons::ENGINE_FLAG_10);
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

void SpecialOpcodes::spcFlickerClearFlag0x80() {
	_vm->_dragonINIResource->getFlickerRecord()->actor->clearFlag(ACTOR_FLAG_80);
}

void SpecialOpcodes::spcSetEngineFlag0x2000000() {
	_vm->setFlags(Dragons::ENGINE_FLAG_2000000);
}

void SpecialOpcodes::spcClearEngineFlag0x2000000() {
	_vm->clearFlags(Dragons::ENGINE_FLAG_2000000);
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

void SpecialOpcodes::spcUnk5e() {
	panCamera(1);
	_vm->_dragonINIResource->setFlickerRecord(_vm->getINI(0));
}

void SpecialOpcodes::spcUnk5f() {
	_vm->getINI(0x2ab)->field_12 = 0;
	panCamera(2);
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

void SpecialOpcodes::spcSetCameraXToZero() {
	_vm->_scene->_camera.x = 0;
}

void SpecialOpcodes::spcDiamondIntroSequenceLogic() {
	/*
	bool bVar1;
	ushort uVar2;
	undefined4 uVar3;
	DragonINI *pDVar4;
	int iVar5;
	uint uVar6;
	short sVar7;
	Actor *actorId;
	Actor *actorId_00;
	Actor *actorId_01;
	Actor *actorId_02;
	Actor *actorId_03;

	pDVar4 = dragon_ini_pointer;
	_vm->setUnkFlags(ENGINE_UNK1_FLAG_2);
	actorId = _vm->getINI(0x257)->actor;
	actorId_03 = _vm->getINI(0x259)->actor;
	actorId_01 = _vm->getINI(0x258)->actor;
	actorId_03->flags = actorId_03->flags | 0x100;
	actorId_03->priorityLayer = 4;
	uVar3 = scrFileData_maybe;
	actorId_00 = _vm->getINI(0x256)->actor; //(uint)(ushort)pDVar4[iVar5 + -1].field_0x1c;
	uVar2 = *(ushort *)((&actor_dictionary)[(uint)actors[actorId_00].﻿actorFileDictionaryIndex * 2] + 10);
	_vm->setFlags(ENGINE_FLAG_20000);
	actorId_02 = _vm->getINI(0x25a)->actor; //(uint)(ushort)pDVar4[DAT_80063ed4 + -1].field_0x1c;
	iVar5 = (&actor_dictionary)[(uint)actors[actorId_00].﻿actorFileDictionaryIndex * 2];
	if ((somethingTextAndSpeechAndAnimRelated(actorId_02,1,0,DAT_80063ed8,0x2601) != 2) && (uVar6 = actorId->actorSetSequenceAndWaitAllowSkip(2), (uVar6 & 0xffff) == 0)) {
		actorId->updateSequence(3);
		uVar6 = actorId_01->actorSetSequenceAndWaitAllowSkip(0x18);
		if ((uVar6 & 0xffff) == 0) {
			sVar7 = 0x2c;
			do {
				_vm->waitForFrames(1);
				uVar6 = _vm->checkForActionButtonRelease();
				if ((uVar6 & 0xffff) != 0) break;
				bVar1 = sVar7 != 0;
				sVar7 = sVar7 + -1;
			} while (bVar1);
			//TODO fade_related_calls_with_1f();
			load_palette_into_frame_buffer(0,(uint)uVar2 + iVar5);
			camera_x = 0x140;
			//TODO call_fade_related_1f();
			uVar6 = actorId_00->actorSetSequenceAndWaitAllowSkip(0);
			if ((uVar6 & 0xffff) == 0) {
				playSoundFromTxtIndex(0x42A66);
				uVar6 = somethingTextAndSpeechAndAnimRelated(actorId_00,1,2,DAT_80063ee0,0x3c01);
				if ((uVar6 & 0xffff) != 2) {
					sVar7 = 0x13;
					do {
						_vm->waitForFrames(1);
						uVar6 = _vm->checkForActionButtonRelease();
						if ((uVar6 & 0xffff) != 0) break;
						bVar1 = sVar7 != 0;
						sVar7 = sVar7 + -1;
					} while (bVar1);
					//TODO fade_related_calls_with_1f();
					load_palette_into_frame_buffer(0,uVar3);
					camera_x = 0;
					sVar7 = 0xf;
					//TODO call_fade_related_1f();
					actorId_01->updateSequence(0x19);
					do {
						_vm->waitForFrames(1);
						uVar6 = _vm->checkForActionButtonRelease();
						if ((uVar6 & 0xffff) != 0) break;
						bVar1 = sVar7 != 0;
						sVar7 = sVar7 + -1;
					} while (bVar1);
					actorId->updateSequence(4);
					sVar7 = 0x17;
					do {
						_vm->waitForFrames(1);
						uVar6 = _vm->checkForActionButtonRelease();
						if ((uVar6 & 0xffff) != 0) break;
						bVar1 = sVar7 != 0;
						sVar7 = sVar7 + -1;
					} while (bVar1);
					actorId_03->updateSequence(9);
					actorId_03->x_pos = 0x82;
					actorId_03->y_pos = 0xc4;
					actorId_03->priorityLayer = 4;
					uVar2 = actors[actorId].flags;
					bVar1 = false;
					while ((uVar2 & 4) == 0) {
						uVar6 = _vm->checkForActionButtonRelease();
						if ((uVar6 & 0xffff) != 0) {
							bVar1 = true;
							break;
						}
						uVar2 = actors[actorId].flags;
					}
					if (!bVar1) {
						actorId->updateSequence(5);
						uVar6 = somethingTextAndSpeechAndAnimRelated(actorId_01,0x10,2,DAT_80063ee4,0x3c01);
						if (((uVar6 & 0xffff) != 2) && (uVar6 = somethingTextAndSpeechAndAnimRelated(actorId_02,1,0,DAT_80063edc,0x2601), (uVar6 & 0xffff) != 2)) {
							sVar7 = 0x3b;
							do {
								_vm->waitForFrames(1);
								uVar6 = _vm->checkForActionButtonRelease();
								if ((uVar6 & 0xffff) != 0) break;
								bVar1 = sVar7 != 0;
								sVar7 = sVar7 + -1;
							} while (bVar1);
						}
					}
				}
			}
		}
	}
	*/
	_vm->clearUnkFlags(ENGINE_UNK1_FLAG_2);
	_vm->clearFlags(ENGINE_FLAG_20000);
	return;
}

void SpecialOpcodes::spcLoadScene1() {
	// TODO spcLoadScene1 knights around the table.
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
		return;
}

} // End of namespace Dragons
