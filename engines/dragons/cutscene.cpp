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
#include "cutscene.h"
#include "dragons.h"
#include "dragons/actor.h"
#include "dragons/scene.h"
#include "dragons/talk.h"

namespace Dragons {

CutScene::CutScene(DragonsEngine *vm): _vm(vm) {

}

void CutScene::scene1() {
	// TODO spcLoadScene1 knights around the table.
/* WIP
	bool bVar1;
	undefined2 uVar2;
	undefined2 *puVar3;
	uint actorId_arg;
	uint8_t *dialogText;
	undefined2 *puVar4;
	uint uVar5;
	uint uVar6;
	uint uVar7;
	uint uVar8;
	short sVar9;
	uint uVar10;
	char acStack4024 [4000];

	uVar2 = dragon_ini_maybe_flicker_control;
	DAT_80063514 = 0xb00;
	dragon_ini_maybe_flicker_control = 0xffff;
	_vm->setUnkFlags(ENGINE_UNK1_FLAG_2);
	uVar10 = engine_flags_maybe & 0x10;
	//fade_related_calls_with_1f();
	_vm->clearFlags(ENGINE_FLAG_10);
	actors[0].flags = actors[0].flags | 0x400;
	scr_tilemap1_w = 0x28;
	actors[1].flags = actors[1].flags | 0x400;
	DAT_8006a3f0 = DAT_8006a3ec;
	load_actor_file(0x81);
	load_actor_file(0x7d);
	load_actor_file(0x7e);
	load_actor_file(0x8f);
	load_actor_file(0xaa);
	_vm->setFlags(ENGINE_FLAG_20000);
	FUN_8003d7d4();
	_vm->waitForFrames(0x5a);
	FUN_8003d97c(0xd3,0,0x233,0x17a);
	//playSoundFromTxtIndex(0x4e26);
	if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,4,0,0x4e26,0x2e01) != 2) {
		FUN_8003c108(DAT_80072de8);
		FUN_8003d97c(0xd8,0,0xfd,0x60);
		//playSoundFromTxtIndex(0x4ea2);
		if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,2,0,0x4ea2,0x701) != 2) {
			FUN_8003c108(DAT_80072de8);
			FUN_8003d97c(0xd3,0,0x233,0x17a);
			//playSoundFromTxtIndex(0x4eec);
			if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,5,0,0x4eec,0x2e01) != 2) {
				FUN_8003c108(DAT_80072de8);
				FUN_8003d7d4();
				if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072dec,4,0,0x5000,0x2e01) != 2 &&
					_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_800830c0,0x1d,0x1c,0x5074,0x501) != 2 &&
					_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072df0,9,5,0x511c,0xc01) != 2 &&
					_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_800830c0,0x1d,0x1c,0x5138,0x501) != 2) {
					FUN_8003d97c(0xd7,0,0x312,0x260);
					//playSoundFromTxtIndex(0x5152);
					if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,2,0,0x5152,0x3e01) != 2) {
						FUN_8003c108(DAT_80072de8);
						FUN_8003d97c(0xd8,0,0xfd,0x60);
						//playSoundFromTxtIndex(0x51fc);
						if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,3,0,0x51fc,0x701) != 2) {
							_vm->playSound(0x8004);
							_vm->waitForFrames(0x28);
							_vm->playSound(0x8003);
							FUN_8003c108(DAT_80072de8);
							DAT_80063514 = DAT_80063514 | 0x40;
							FUN_8003d8e8(0xd6,0,0x37a,0x280);
							DAT_80072dec = _vm->_actorManager->loadActor(0xd5,0,0x2d6,0xc6,3);
							DAT_80072df0 = _vm->_actorManager->loadActor(0xd3,2,0x87,199,3);
							_vm->waitForFramesAllowSkip(4);
							// call_fade_related_1f();
							if (FUN_8003dab8(0x52d6,0,0,0x701,1) != 2) {
								callMaybeResetData();
								//playSoundFromTxtIndex(0x530c);
								if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072dec,2,0,0x530c,0x3c01) != 2) {
									while (0x10 < _vm->_scene->_camera.x) {
										_vm->_scene->_camera.x = _vm->_scene->_camera.x + -0x10;
										_vm->waitForFrames(1);
									}
									_vm->_scene->_camera.x = 0;
									//playSoundFromTxtIndex(0x54dc);
									if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072df0,6,2,0x54dc,0x2e01) != 2) {
										FUN_8003c108(DAT_80072de8);
										FUN_8003c108(DAT_80072dec);
										FUN_8003c108(DAT_80072df0);
										FUN_8003d7d4();
										if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072e08,3,2,0x55d4,0xc01) != 2) {
											FUN_8003d97c(0xd4,0,0x8a,0);
											//playSoundFromTxtIndex(0x562c);
											if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,2,0,0x562c,0xc01) != 2) {
												FUN_8003c108(DAT_80072de8);
												FUN_8003d8e8(0xd6,0,0x37a,0x280);
												DAT_80072dec = _vm->_actorManager->loadActor(0xd5,4,0x2d6,0xc6,3);
												_vm->waitForFramesAllowSkip(4);
												// call_fade_related_1f();
												if (FUN_8003dab8(0x5780,0x14,0,0xc01,1) != 2) {
													DAT_80063514 = DAT_80063514 | 0x40;
													FUN_8003c108(DAT_80072de8);
													FUN_8003c108(DAT_80072dec);
													FUN_8003d97c(0xd7,0,0x312,0x260);
													//playSoundFromTxtIndex(0x581c);
													if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,3,0,0x581c,0x3e01) != 2) {
														FUN_8003c108(DAT_80072de8);
														FUN_8003d97c(0xd4,0,0x8a,0);
														//playSoundFromTxtIndex(0x5942);
														if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,4,0,0x5942,0xc01) != 2) {
															FUN_8003c108(DAT_80072de8);
															FUN_8003d97c(0xd3,2,0x87,0);
															//playSoundFromTxtIndex(0x5aaa);
															if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,7,2,0x5aaa,0x2e01) != 2) {
																FUN_8003c108(DAT_80072de8);
																FUN_8003d8e8(0xd6,0,0x37a,0x280);
																DAT_80072dec = _vm->_actorManager->loadActor(0xd5,0,0x2d6,0xc6,3);
																_vm->waitForFramesAllowSkip(4);
																// call_fade_related_1f();
																//playSoundFromTxtIndex(0x5afc);
																if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072dec,3,0,0x5afc,0x3c01) != 2) {
																	FUN_8003c108(DAT_80072de8);
																	FUN_8003c108(DAT_80072dec);
																	DAT_80063514 = DAT_80063514 & 0xffbf;
																	FUN_8003d7fc();
																	_vm->playSound(0x8003);
																	//playSoundFromTxtIndex(0x5b96);
																	if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072dec,8,4,0x5b96,0xc01) != 2) {
																		//playSoundFromTxtIndex(0x5c4a);
																		if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,2,0,0x5c4a,0x2e01) != 2) {
																			//playSoundFromTxtIndex(0x5dc8);
																			DAT_80072df0->updateSequence(0xf);
																			DAT_80072df4->updateSequence(0xd);
																			DAT_80072df8->updateSequence(0xe);
																			if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072dec,0xc,4,0x5dc8,0xc01) != 2) {
																				DAT_80072df0->updateSequence(6);
																				DAT_80072df4->updateSequence(0);
																				DAT_80072df8->updateSequence(2);
																				FUN_8003d8e8(0xd3,2,0x28d,0x250);
																				DAT_80072dec = _vm->_actorManager->loadActor(0xd7,0,0x348,199,3);
																				_vm->waitForFramesAllowSkip(4);
																				//call_fade_related_1f();
																				//playSoundFromTxtIndex(0x5ef2);
																				if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072dec,4,0,0x5ef2,0x3e01) != 2) {
																					FUN_8003c108(DAT_80072de8);
																					FUN_8003c108(DAT_80072dec);
																					FUN_8003d97c(0xd3,0,0x233,0x17a);
																					//playSoundFromTxtIndex(0x6000);
																					if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,8,0,0x6000,0x2e01) != 2) {
																						FUN_8003c108(DAT_80072de8);
																						FUN_8003d7fc();
																						//playSoundFromTxtIndex(0x7dcc);
																						DAT_80072df0->updateSequence(0x13);
																						DAT_80072df4->updateSequence(0x10);
																						DAT_80072df8->updateSequence(0x11);
																						if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072dec,0x12,6,0x7dcc,0xc01) != 2) {
																							FUN_8003d7d4();
																							if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072de8,0x17,0x16,0x60ee,0x701) != 2) {
																								FUN_8003d7fc();
																								DAT_80072df0->updateSequence(6);
																								DAT_80072df4->updateSequence(0);
																								DAT_80072df8->updateSequence(2);
																								DAT_80072dec->updateSequence(4);
																								//playSoundFromTxtIndex(0x5de8);
																								if (_vm->_talk->somethingTextAndSpeechAndAnimRelated(DAT_80072df8,0xb,2,0x5de8,0xc01) != 2) {
																									FUN_8003d7d4();
																									_vm->playSound(1);
																									DAT_80072df0->updateSequence(8);
																									DAT_80072df4->updateSequence(0xd);
																									DAT_80072df8->updateSequence(0x11);
																									DAT_80072dfc->updateSequence(0x15);
																									DAT_80072df0->waitUntilFlag8And4AreSet();
																									DAT_80072df4->waitUntilFlag8And4AreSet();
																									DAT_80072df8->waitUntilFlag8And4AreSet();
																									DAT_80072dfc->waitUntilFlag8And4AreSet();
																									// DisableVSyncEvent();
																									load_actor_file(0x82);
																									// EnableVSyncEvent();
																									_vm->waitForFramesAllowSkip(0x3b);
																									_vm->clearFlags(ENGINE_FLAG_20000);
																									// fade_related_calls_with_1f();
																									// DisableVSyncEvent();
																									actorId_arg = 2;
																									puVar4 = scrData_offset_200 + 4;
																									do {
																										*puVar4 = 0xffff;
																										puVar4[1] = 0xffff;
																										puVar3 = scrData_offset_200;
																										actorId_arg = actorId_arg + 1;
																										puVar4 = puVar4 + 2;
																									} while (actorId_arg < 0x20);
																									sVar9 = 0xe;
																									scrData_offset_200[1] = 0x11;
																									puVar3[2] = 199;
																									*puVar3 = 0;
																									puVar3[3] = 1;
																									// EnableVSyncEvent();
																									system_palette_related(2,0);
																									// clearActorFlags(2);
																									DAT_80072de8 = _vm->_actorManager->loadActor(0x82,0,0x60,0x114,1);
																									DAT_80072dec = _vm->_actorManager->loadActor(0x82,2,0x91,0x113,1);
																									DAT_80072df0 = _vm->_actorManager->loadActor(0x82,1,0xd0,199,1);
																									DAT_80072df4 = _vm->_actorManager->loadActor(0x82,3,0xb6,0x113,1);
																									DAT_80072de8 = _vm->_actorManager->loadActor(0x82,4,0x98,0x40,1);
																									uVar5->setFlags()
																									actors[uVar5].flags = actors[uVar5].flags | 0x8900
																											;
																									actorId_arg = (uint)DAT_80072dec;
																									actors[uVar5].field_0x7c = 0x20000;
																									uVar6 = (uint)DAT_80072df0;
																									actors[actorId_arg].flags =
																											actors[actorId_arg].flags | 0x8900;
																									actors[actorId_arg].field_0x7c = 0x18000;
																									uVar7 = (uint)DAT_80072df4;
																									actors[uVar6].flags = actors[uVar6].flags | 0x8900
																											;
																									actors[uVar6].field_0x7c = 0x14000;
																									uVar8 = (uint)DAT_80072df8;
																									actors[uVar7].flags = actors[uVar7].flags | 0x8900
																											;
																									actors[uVar7].field_0x7c = 0x1c000;
																									actors[uVar8].flags = actors[uVar8].flags | 0x8980
																											;
																									actors[uVar5].﻿priorityLayer_maybe = 3;
																									actors[actorId_arg].﻿priorityLayer_maybe = 3;
																									actors[uVar6].﻿priorityLayer_maybe = 3;
																									actors[uVar7].﻿priorityLayer_maybe = 3;
																									actors[uVar8].﻿priorityLayer_maybe = 3;
																									do {
																										_vm->waitForFrames(1);
																										actorId_arg = PressedThisFrameStart(0);
																										if ((actorId_arg & 0xffff) != 0) break;
																										bVar1 = sVar9 != 0;
																										sVar9 = sVar9 + -1;
																									} while (bVar1);
																									call_fade_related_1f();
																									actor_path_finding_maybe
																											((uint)DAT_80072df0,0xe8,0xa8,2);
																									while ((actors[(uint)DAT_80072df0].flags & 0x10)
																										   != 0) {
																										_vm->waitForFrames(1);
																									}
																									actor_path_finding_maybe((uint)DAT_80072de8,0x97,0x37,2);
																									actor_path_finding_maybe((uint)DAT_80072dec,0x97,0x37,2);
																									actor_path_finding_maybe((uint)DAT_80072df4,0x97,0x37,2);
																									actor_wait_for_8_set_1000_wait_for_4((uint)DAT_80072df0);
																									DAT_80072df0->updateSequence(6);
																									dialogText = (uint8_t *)load_string_from_dragon_txt(0x5ea2,acStack4024);
																									displayDialogAroundPoint(dialogText,0x27,0xc,0xc01,0,0x5ea2);
																									waitUntilFlag8And4AreSet((uint)DAT_80072df0);
																									actorId_arg = (uint)DAT_80072df0;
																									actors[actorId_arg].x_pos = 0xcf;
																									actors[actorId_arg].y_pos = 0x90;
																									actor_path_finding_maybe(actorId_arg,0x97,0x37,2);
																									DAT_80072df0->updateSequence(7);
																									if (((DAT_8008e7e8 != 0) || (DAT_8008e848 != 0))
																										|| ((DAT_8008e844 != 0 || (DAT_8008e874 != 0)))
																											) {
																										FUN_8001a7c4((uint)DAT_8008e7e8,
																													 (uint)DAT_8008e844,
																													 (uint)DAT_8008e848,
																													 (uint)DAT_8008e874);
																									}
																									dialogText = (uint8_t *)
																											load_string_from_dragon_txt
																													(0x5ecc,acStack4024);
																									_vm->_talk->displayDialogAroundPoint(dialogText,0x14,6,0xc01,0,0x5ecc);
																									FUN_80023794(1);
																								}
																							}
																						}
																					}
																				}
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	// fade_related_calls_with_1f();
	_vm->clearFlags(ENGINE_FLAG_20000);
	// DisableVSyncEvent();
	file_read_to_buffer(s_cursor.act_80011c44,actor_dictionary);
	// EnableVSyncEvent();
	engine_flags_maybe = uVar10 | engine_flags_maybe;
	dragon_ini_maybe_flicker_control = uVar2;
	FUN_80021fc4();
*/
	_vm->clearUnkFlags(ENGINE_UNK1_FLAG_2);

}

void CutScene::FUN_8003c108(Actor *actor) {
	// TODO fade_related_calls_with_1f();
	//DisableVSyncEvent();
	actor->reset_maybe();
	//EnableVSyncEvent();

}

void CutScene::FUN_8003d97c(uint16 resourceId, uint16 sequenceId, int16 x, uint32 param_4) {
	FUN_8003d8e8(resourceId, sequenceId, x, param_4);
	_vm->waitForFrames(5);
	// TODO call_fade_related_1f();
}


void CutScene::FUN_8003d8e8(uint16 resourceId, uint16 sequenceId, int16 x, uint32 param_4) {
	// fade_related_calls_with_1f();
	_vm->_actorManager->clearActorFlags(2);
	//DisableVSyncEvent();
	Actor *DAT_80072de8 = _vm->_actorManager->loadActor(resourceId, sequenceId, x, 199, 3);
	//EnableVSyncEvent();
	//TODO system_palette_related(3,param_4 & 0xffff);

}

void CutScene::FUN_8003d7d4() {
	FUN_8003d388();
	// call_fade_related_1f();

}

void CutScene::FUN_8003d388() {
	//TODO implement me.
}


void CutScene::FUN_8003d7fc() {
	//fade_related_calls_with_1f();
	_vm->_actorManager->clearActorFlags(2);
	DAT_80072de8 = _vm->_actorManager->loadActor(0x7d,0,2,199,1);
	DAT_80072dec = _vm->_actorManager->loadActor(0x81,4,2,199,1);
	DAT_80072df0 = _vm->_actorManager->loadActor(0x81,6,2,199,1);
	DAT_80072df4 = _vm->_actorManager->loadActor(0x81,0,2,199,1);
	DAT_80072df8 = _vm->_actorManager->loadActor(0x81,2,2,199,1);
	//TODO system_palette_related(1,0);
	_vm->waitForFrames(0xf);
	//call_fade_related_1f();

}

} // End of namespace Dragons
