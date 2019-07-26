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
#include "dragons/inventory.h"
#include "dragons/cursor.h"
#include "dragons/dragonini.h"
#include "dragons/scene.h"
#include "dragons/talk.h"

namespace Dragons {

CutScene::CutScene(DragonsEngine *vm): _vm(vm) {

}

void CutScene::scene1() {
	// TODO spcLoadScene1 knights around the table.
	bool isFlag10Set = _vm->isFlagSet(ENGINE_FLAG_10);

	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();

	DAT_80063514 = 0xb00;
	_vm->_dragonINIResource->setFlickerRecord(NULL);

	_vm->setUnkFlags(ENGINE_UNK1_FLAG_2);
	//fade_related_calls_with_1f();
	_vm->clearFlags(ENGINE_FLAG_10);

	_vm->_inventory->setActorFlag400();
	_vm->_cursor->setActorFlag400();

//	scr_tilemap1_w = 0x28;
//	DAT_8006a3f0 = DAT_8006a3ec;

//	load_actor_file(0x81);
//	load_actor_file(0x7d);
//	load_actor_file(0x7e);
//	load_actor_file(0x8f);
//	load_actor_file(0xaa);
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
								// TODO callMaybeResetData();
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
																									// TODO load_actor_file(0x82);
																									// EnableVSyncEvent();
																									_vm->waitForFramesAllowSkip(0x3b);
																									_vm->clearFlags(ENGINE_FLAG_20000);
																									// fade_related_calls_with_1f();
																									// DisableVSyncEvent();
//TODO
//																									actorId_arg = 2;
//																									puVar4 = scrData_offset_200 + 4;
//																									do {
//																										*puVar4 = 0xffff;
//																										puVar4[1] = 0xffff;
//																										puVar3 = scrData_offset_200;
//																										actorId_arg = actorId_arg + 1;
//																										puVar4 = puVar4 + 2;
//																									} while (actorId_arg < 0x20);
//																									sVar9 = 0xe;
//																									scrData_offset_200[1] = 0x11;
//																									puVar3[2] = 199;
//																									*puVar3 = 0;
//																									puVar3[3] = 1;

																									// EnableVSyncEvent();
																									// system_palette_related(2,0);
																									_vm->_actorManager->clearActorFlags(2);
																									DAT_80072de8 = _vm->_actorManager->loadActor(0x82,0,0x60,0x114,1);
																									DAT_80072dec = _vm->_actorManager->loadActor(0x82,2,0x91,0x113,1);
																									DAT_80072df0 = _vm->_actorManager->loadActor(0x82,1,0xd0,199,1);
																									DAT_80072df4 = _vm->_actorManager->loadActor(0x82,3,0xb6,0x113,1);
																									DAT_80072de8 = _vm->_actorManager->loadActor(0x82,4,0x98,0x40,1);

																									DAT_80072de8->setFlag(ACTOR_FLAG_100);
																									DAT_80072de8->setFlag(ACTOR_FLAG_800);
																									DAT_80072de8->setFlag(ACTOR_FLAG_8000);
																									DAT_80072de8->field_7c = 0x20000;
																									DAT_80072de8->priorityLayer = 3;

																									DAT_80072dec->setFlag(ACTOR_FLAG_100);
																									DAT_80072dec->setFlag(ACTOR_FLAG_800);
																									DAT_80072dec->setFlag(ACTOR_FLAG_8000);
																									DAT_80072dec->field_7c = 0x18000;
																									DAT_80072dec->priorityLayer = 3;

																									DAT_80072df0->setFlag(ACTOR_FLAG_100);
																									DAT_80072df0->setFlag(ACTOR_FLAG_800);
																									DAT_80072df0->setFlag(ACTOR_FLAG_8000);
																									DAT_80072df0->field_7c = 0x14000;
																									DAT_80072df0->priorityLayer = 3;

																									DAT_80072df4->setFlag(ACTOR_FLAG_100);
																									DAT_80072df4->setFlag(ACTOR_FLAG_800);
																									DAT_80072df4->setFlag(ACTOR_FLAG_8000);
																									DAT_80072df4->field_7c = 0x1c000;
																									DAT_80072df4->priorityLayer = 3;

																									DAT_80072df8->setFlag(ACTOR_FLAG_100);
																									DAT_80072df8->setFlag(ACTOR_FLAG_800);
																									DAT_80072df8->setFlag(ACTOR_FLAG_8000);
																									DAT_80072df8->priorityLayer = 3;

																									_vm->waitForFramesAllowSkip(0xe);
																									// call_fade_related_1f();
																									DAT_80072df0->pathfinding_maybe(0xe8,0xa8,2);
																									while (DAT_80072df0->isFlagSet(ACTOR_FLAG_10)) {
																										_vm->waitForFrames(1);
																									}
																									DAT_80072de8->pathfinding_maybe(0x97,0x37,2);
																									DAT_80072dec->pathfinding_maybe(0x97,0x37,2);
																									DAT_80072df4->pathfinding_maybe(0x97,0x37,2);
																									DAT_80072df0->waitUntilFlag8SetThenSet1000AndWaitFor4();
																									DAT_80072df0->updateSequence(6);
																									uint16 dialog[2000];
																									dialog[0] = 0;
																									_vm->_talk->loadText(0x5ea2, dialog, 2000);

																									_vm->_talk->displayDialogAroundPoint(dialog,0x27,0xc,0xc01,0,0x5ea2);
																									DAT_80072df0->waitUntilFlag8And4AreSet();
																									DAT_80072df0->x_pos = 0xcf;
																									DAT_80072df0->y_pos = 0x90;
																									DAT_80072df0->pathfinding_maybe(0x97, 0x37, 2);
																									DAT_80072df0->updateSequence(7);
//TODO
//																									if (((DAT_8008e7e8 != 0) || (DAT_8008e848 != 0))
//																										|| ((DAT_8008e844 != 0 || (DAT_8008e874 != 0)))
//																											) {
//																										FUN_8001a7c4((uint)DAT_8008e7e8,
//																													 (uint)DAT_8008e844,
//																													 (uint)DAT_8008e848,
//																													 (uint)DAT_8008e874);
//																									}

																									dialog[0] = 0;
																									_vm->_talk->loadText(0x5ecc, dialog, 2000);
																									_vm->_talk->displayDialogAroundPoint(dialog,0x14,6,0xc01,0,0x5ecc);
																									_vm->waitForFrames(0x3c);
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
	//file_read_to_buffer(s_cursor.act_80011c44,actor_dictionary);
	// EnableVSyncEvent();
	if (isFlag10Set) {
		_vm->setFlags(ENGINE_FLAG_10);
	} else {
		_vm->clearFlags(ENGINE_FLAG_10);
	}

	_vm->_dragonINIResource->setFlickerRecord(flicker);
	cursorInventoryClearFlag400();
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
	DAT_80072de8 = _vm->_actorManager->loadActor(resourceId, sequenceId, x, 199, 3);
	//EnableVSyncEvent();
	//TODO system_palette_related(3,param_4 & 0xffff);

}

void CutScene::FUN_8003d7d4() {
	FUN_8003d388();
	// call_fade_related_1f();

}

void CutScene::FUN_8003d388() {
	uint sequenceId;

	// fade_related_calls_with_1f();
	_vm->_actorManager->clearActorFlags(2);
	if ((DAT_80063514 & 0x80) == 0) {
		DAT_80072de8 = _vm->_actorManager->loadActor(0x7e,0x16,0x40,0xa0,1);
	}
	DAT_80072dec = _vm->_actorManager->loadActor(0x7e,0,0xbf,0xba,1);
	if ((DAT_80063514 & 8) == 0) {
		DAT_80072df0 = _vm->_actorManager->loadActor(0x7e,5,0x94,0x82,1);
	}
	if ((DAT_80063514 & 0x10) == 0) {
		DAT_80072df4 = _vm->_actorManager->loadActor(0x7e,10,0x6f,0x95,1);
	}
	if ((DAT_80063514 & 4) == 0) {
		DAT_80072df8 = _vm->_actorManager->loadActor(0x7e,0xe,0xa9,0x87,1);
	}
	if ((DAT_80063514 & 0x20) == 0) {
		DAT_80072dfc = _vm->_actorManager->loadActor(0x7e,0x12,0xcd,0x8e,1);
	}
	if ((DAT_80063514 & 1) == 0) {
		DAT_80072e04 = _vm->_actorManager->loadActor(0x7e,0x19,0x10e,0x89,1);
	}
	if ((DAT_80063514 & 2) == 0) {
		DAT_80072e08 = _vm->_actorManager->loadActor(0x8f,2,100,0xbc,1);
	}
	if ((DAT_80063514 & 0x40) != 0) {
		DAT_80072e0c = _vm->_actorManager->loadActor(0x8f,0,0xd2,100,1);
		DAT_800830a0 = _vm->_actorManager->loadActor(0x8f,1,0xe6,0x6e,1);
	}
	DAT_800830b8 = _vm->_actorManager->loadActor(0xaa,0,0x2e,0x2d,1);
	DAT_800830b8->setFlag(ACTOR_FLAG_8000);
	DAT_800830bc = _vm->_actorManager->loadActor(0xaa,1,0x115,0x22,1);
	DAT_800830bc->setFlag(ACTOR_FLAG_100);
	DAT_800830bc->setFlag(ACTOR_FLAG_8000);
	DAT_800830bc->priorityLayer = 4;
	if ((DAT_80063514 & 0x100) != 0) {
		DAT_800830c0 = _vm->_actorManager->loadActor(0x7e,0x1c,0x21,0x87,1);
	}
	if ((DAT_80063514 & 0x200) != 0) {
		if ((DAT_80063514 & 0x800) == 0) {
			sequenceId = 2;
		}
		else {
			sequenceId = 4;
		}
		DAT_800830d4 = _vm->_actorManager->loadActor(0xaa,sequenceId,0xf4,199,1);
		DAT_800830d4->setFlag(ACTOR_FLAG_8000);
	}
	if ((DAT_80063514 & 0x400) != 0) {
		DAT_800830dc = _vm->_actorManager->loadActor(0xaa,3,0xf4,199,1);
		DAT_800830dc->setFlag(ACTOR_FLAG_8000);
	}
	// system_palette_related(0,0);
	_vm->waitForFramesAllowSkip(0xe);
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

uint16 CutScene::FUN_8003dab8(uint32 textId,uint16 x,uint16 y,uint16 param_4,int16 param_5)
{
	uint16 dialog[2000];
	dialog[0] = 0;
	_vm->_talk->loadText(textId, dialog, 2000);

	_vm->_talk->displayDialogAroundPoint(dialog,x,y,param_4,param_5,textId);
	return 1; //TODO this should return (uint)dialogText & 0xffff;
}

void CutScene::cursorInventoryClearFlag400() {
	_vm->_cursor->clearActorFlag400();
	_vm->_inventory->clearActorFlag400();
}

} // End of namespace Dragons
