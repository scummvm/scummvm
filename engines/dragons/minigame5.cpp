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
#include "dragons/minigame5.h"
#include "dragons/actor.h"
#include "dragons/dragons.h"
#include "dragons/dragonini.h"
#include "dragons/talk.h"
#include "dragons/scene.h"
#include "dragons/screen.h"

namespace Dragons {

#define DAT_80063a48 0x12d
#define DAT_80063a40 0x12f
#define DAT_80063bd0 0x133


Minigame5::Minigame5(DragonsEngine *vm) : _vm(vm), _dat_800633e6(0) {}

void Minigame5::run() {
	uint16 uVar1;
	//int iVar2;
	//byte *local_v0_7008;
	//void *local_v0_7256;
	//byte *uVar3;
	//uint uVar4;
	uint16 local_850;
	uint16 auStack2120 [1000];
	DragonINI*local_78;
	//uint16 local_76;
	uint16 local_74;
	uint16 local_72 = 0;
	uint16 bombScale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	Actor *bombActor;
	Actor *flickerActor;
	Actor *pusherActor;
	Actor *dustActor;
	uint16 local_66;
	Actor *wheelsActor;
	uint16 local_62;
	uint16 local_60 = 0;
	uint16 local_5e = 0;
	short local_5c = 0;
	short local_5a = 0;
	uint16 currentState;
	short local_50;
	Actor *local_4e;
	//uint16 local_4c;
	//uint16 local_4a;
	//uint16 local_48;
	int16 local_46;
	//int16 local_44;
	short local_42;
	uint16 local_30 [4];
	//uint16 uStack42;
	uint16 local_28 [10];
	short local_10;
	uint32 savedEngineFlags;

	Common::File *fd = new Common::File();
	if (!fd->open("arc5.bin")) {
		error("Failed to open arc5.bin");
	}

	for (int i = 0; i < 4; i++) {
		local_30[i] = fd->readUint16LE();
	}

	for (int i = 0; i < 10; i++) {
		local_28[i] = fd->readUint16LE();
	}

	fd->close();

	//local_48 = 0;
	//local_76 = _vm->_scene->getSceneId();
	local_78 = _vm->_dragonINIResource->getFlickerRecord();
	local_78->actor->setFlag(ACTOR_FLAG_100);
	local_78->actor->_priorityLayer = 0;
	savedEngineFlags = _vm->getMultipleFlags(ENGINE_FLAG_8 | ENGINE_FLAG_10 | ENGINE_FLAG_20 | ENGINE_FLAG_80);
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_10);
	_vm->clearFlags(ENGINE_FLAG_20);
	_vm->clearFlags(ENGINE_FLAG_80);
	_vm->_dragonINIResource->setFlickerRecord(_vm->_dragonINIResource->getRecord(DAT_80063a40 - 1));
	flickerActor = _vm->_dragonINIResource->getFlickerRecord()->actor;
	flickerActor->_flags = flickerActor->_flags | 0x380;
	flickerActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	flickerActor->_priorityLayer = 4;
	flickerActor->_direction = -1;
	flickerActor->updateSequence(0x19);
	currentState = 0;
	local_10 = 0;
	local_850 = flickerActor->_x_pos;
	uVar1 = flickerActor->_y_pos;
	local_74 = 0;
//	DisableVSyncEvent();
	pusherActor = _vm->_actorManager->loadActor
			(0x26, 0, (int)(short)local_850, (int)(((uint)uVar1 + 5) * 0x10000) >> 0x10);
//	EnableVSyncEvent();
	if (pusherActor == nullptr) {
		error("Couldn't alloc pusher!");
	}
	pusherActor->_flags = pusherActor->_flags | 0x380;
	pusherActor->_x_pos = flickerActor->_x_pos + -0xe;
	pusherActor->_y_pos = flickerActor->_y_pos + 7;
	pusherActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	pusherActor->_priorityLayer = 6;
//	DisableVSyncEvent();
	wheelsActor = _vm->_actorManager->loadActor(7, 0x11, 0, 0);
//	EnableVSyncEvent();
	if (wheelsActor == nullptr) {
		error("Couldn't alloc wheels!");
	}
	wheelsActor->_flags = wheelsActor->_flags | 0x380;
	wheelsActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	wheelsActor->_x_pos = flickerActor->_x_pos;
	wheelsActor->_y_pos = flickerActor->_y_pos;
	wheelsActor->_priorityLayer = 5;
	wheelsActor->updateSequence(0x11);
	local_62 = 0;
//	DisableVSyncEvent();
	bombActor = _vm->_actorManager->loadActor(7, 0x1c, 0, 0);
//	EnableVSyncEvent();
	if (bombActor == nullptr) {
		error("Couldn't alloc bomb!");
	}
	bombActor->_flags = bombActor->_flags | 0x380;
	bombActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	bombActor->_priorityLayer = 0;
//	DisableVSyncEvent();
	dustActor = _vm->_actorManager->loadActor(8, 8, 100, 100, 0);
//	EnableVSyncEvent();
	if (dustActor == nullptr) {
		error("Couldn't alloc dust sprite!");
	}
	dustActor->_flags = dustActor->_flags | 0x380;
	dustActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	local_4e = _vm->_dragonINIResource->getRecord(DAT_80063a48 + -1)->actor;
	//local_4c = 0;
	//local_4a = local_4e->_sequenceTimerMaxValue;
	_vm->setFlags(ENGINE_FLAG_4000000);
	currentState = 1;
	local_66 = 0;
	local_50 = 0;
	while (true) {
		while (true) {
			do {
				_vm->waitForFrames(1);
				switch (currentState) {
				case 1:
					if (local_66 != 8) {
						local_66 = 0;
					}
					//iVar2 = IsButtonBeingPressed((uint)DAT_800728ac, 0);
					if (!_vm->isActionButtonPressed()) {
						if (local_74 == 0) {
							if ((((flickerActor->_sequenceID != 0) &&
								  (flickerActor->_sequenceID != 5)) &&
								 (flickerActor->_sequenceID != 6)) ||
								((flickerActor->_flags & 4) != 0)) {
								flickerActor->updateSequence(0x19);
							}
						} else {
							local_66 = 1;
							local_50 = 0;
							pusherActor->updateSequence(1);
							currentState = 2;
							if (local_74 < 0x14) {
								local_72 = 1;
							} else {
								if (local_74 < 0x2d) {
									local_72 = 2;
								} else {
									if (local_74 < 0x169) {
										local_72 = 3;
									}
								}
							}
						}
						local_74 = 0;
					} else {
						pusherActor->_x_pos = flickerActor->_x_pos + -0xe;
						pusherActor->_y_pos = flickerActor->_y_pos + 7;
						if (local_74 < 0x168) {
							local_74 = local_74 + 1;
							if (local_74 < 0x14) {
								if (((pusherActor->_sequenceID != 4) &&
									 (pusherActor->_sequenceID != 2)) &&
									(pusherActor->_sequenceID != 3)) {
									pusherActor->updateSequence(4);
								}
								if (flickerActor->_sequenceID != 0x1a) {
									flickerActor->updateSequence(0x1a);
									_vm->playOrStopSound(2);
								}
							} else {
								if (local_74 < 0x2d) {
									if (((pusherActor->_sequenceID != 5) &&
										 (pusherActor->_sequenceID != 2)) &&
										(pusherActor->_sequenceID != 3)) {
										pusherActor->updateSequence(5);
									}
									if (flickerActor->_sequenceID != 0x1e) {
										flickerActor->updateSequence(0x1e);
										_vm->playOrStopSound(3);
									}
								} else {
									if (local_74 < 0x169) {
										if (((pusherActor->_sequenceID != 6) &&
											 (pusherActor->_sequenceID != 2)) &&
											(pusherActor->_sequenceID != 3)) {
											pusherActor->updateSequence(6);
										}
										if (flickerActor->_sequenceID != 0x1f) {
											flickerActor->updateSequence(0x1f);
											_vm->playOrStopSound(4);
										}
									}
								}
							}
						} else {
							if (pusherActor->_sequenceID != 6) {
								pusherActor->updateSequence(6);
							}
							if (flickerActor->_sequenceID != 0x1f) {
								flickerActor->updateSequence(0x1f);
							}
						}
					}
					break;
				case 2:
					if (flickerActor->_sequenceID == 0x1b) {
						if ((flickerActor->_flags & 4) != 0) {
							if ((((int)(uint)local_850 < (int)((local_30[0]) - 6)) ||
								 ((uint)local_30[1] + 6 < (uint)local_850)) || (local_72 != local_30[2])) {
								local_42 = 8;
							} else {
								local_42 = 0;
							}
							local_5c = 0;
							if ((local_42 != 8) && ((local_850 < local_30[0] || (local_30[1] < local_850)))) {
								local_5c = (short)((int)(((local_30[0]) + 0x17) * 0x80) / 0x2a) -
										   (short)((int)((uint)local_850 << 7) / 0x2a);
							}
							local_60 = local_850 << 7;
							bombActor->_x_pos = local_850 & 0x1ff;
							local_5e = 0x2d00;
							local_5a = (local_72 + 3) * 0x80;
							bombActor->_y_pos = 0x5a;
							bombScale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
							bombActor->_scale = bombScale;
							_vm->playOrStopSound(10);
							bombActor->_priorityLayer = 3;
							flickerActor->updateSequence(8);
							currentState = 3;
						}
					} else {
						flickerActor->updateSequence(0x1b);
						_vm->playOrStopSound(1);
					}
					break;
				case 3:
					local_60 = local_60 + local_5c;
					if ((uint)local_72 * 2 + 0xb4 < (uint)bombScale) {
						local_5e = local_5e - local_5a;
						local_5a = local_5a - local_28[((uint)local_72 - 1) * 3];
						if (local_5a < 0) {
							local_5a = 0;
						}
					} else {
						if ((int)(uint)bombScale < (int)((uint)local_72 * -4 + 0xba)) {
							local_5e = local_5e + local_5a;
							local_5a = local_5a + local_28[((uint)local_72 - 1) * 3 + 2];
						} else {
							local_5a = 0;
						}
					}
					bombActor->_x_pos = local_60 >> 7;
					bombActor->_y_pos = local_5e >> 7;
					bombScale = bombScale - 3;
					bombActor->_scale = bombScale;
					if (bombScale == 0x7f) {
						if (((local_60 >> 7 < local_30[0]) || (local_30[1] < local_60 >> 7)) ||
							(local_72 != local_30[2])) {
							local_42 = 8;
						} else {
							local_42 = 0;
						}
						if (local_42 == 8) {
							if ((((local_72 == 1) && (local_60 >> 7 < 0x10e)) ||
								 ((local_72 == 2 &&
								   ((((0x7f < local_60 >> 7 && (local_60 >> 7 < 0xad)) ||
									  ((0x30 < local_60 >> 7 && (local_60 >> 7 < 0x4a)))) ||
									 ((0xf8 < local_60 >> 7 && (local_60 >> 7 < 0x10f)))))))) ||
								((local_72 == 3 &&
								  (((0x3c < local_60 >> 7 && (local_60 >> 7 < 0x46)) ||
									((0x101 < local_60 >> 7 && (local_60 >> 7 < 0x10a)))))))) {
								bombActor->_priorityLayer = 0;
								dustActor->_priorityLayer = 4;
								dustActor->_x_pos = bombActor->_x_pos;
								dustActor->_y_pos = bombActor->_y_pos;
								dustActor->updateSequence(9);
								currentState = 4;
							}
						} else {
							local_4e->_sequenceTimerMaxValue = 2;
							//local_4c = 0x3c;
							bombActor->_priorityLayer = 0;
							currentState = 8;
						}
					}
					if (bombScale < 0x7f) {
						bombActor->_priorityLayer = 2;
					}
					if ((0xc < bombScale) && (bombScale < 0x41)) {
						bombActor->_priorityLayer = 0;
					}
					if ((short)bombScale < 2) {
						currentState = 5;
					}
					break;
				case 4:
					pusherActor->updateSequence(9);
					_vm->waitForFrames(0x3c);
					pusherActor->updateSequence(0xb);
					if (_dat_800633e6 == 0) {
						uint32 textId = _vm->getDialogTextId(0x21BF0);
						_vm->_talk->loadText(textId, auStack2120, 1000);
						_vm->_talk->displayDialogAroundPoint(auStack2120, (int)(short)(local_850 >> 3), 0xc, 0, 1, textId);
						_dat_800633e6 = 1;
					} else {
						uint32 textId = _vm->getDialogTextId(0x21DAE);
						_vm->_talk->loadText(textId, auStack2120, 1000);
						_vm->_talk->displayDialogAroundPoint(auStack2120, (int)(short)(local_850 >> 3), 0xc, 0, 1, textId);
					}
					_vm->waitForFrames(10);
					local_10 = 1;
					currentState = 6;
					break;
				case 5:
					currentState = 4;
					break;
				case 6:
					currentState = 7;
					break;
				case 7:
					break;
				case 8:
					bombActor->_priorityLayer = 0;
					pusherActor->updateSequence(0);
					_vm->_dragonINIResource->getRecord(DAT_80063bd0 + -1)->actor->updateSequence(2);
					_vm->waitForFrames(0x12);
					_vm->_talk->loadText(_vm->getDialogTextId(0x21E3E), auStack2120, 1000);
					_vm->_talk->displayDialogAroundPoint(auStack2120, 0xf, 2, 0x501, 0, _vm->getDialogTextId(0x21E3E));
					_vm->clearAllText();
					_vm->_dragonINIResource->getRecord(DAT_80063bd0 + -1)->actor->updateSequence(3);
					_vm->_dragonINIResource->getRecord(DAT_80063bd0 + -1)->actor->waitUntilFlag8And4AreSet();
					pusherActor->updateSequence(7);
					_vm->_talk->loadText(_vm->getDialogTextId(0x475DA), auStack2120, 1000);
					_vm->_talk->displayDialogAroundPoint(auStack2120, (int)(short)(local_850 >> 3), 0xc, 0, 1, _vm->getDialogTextId(0x475DA));
					pusherActor->_flags = pusherActor->_flags | 0x1000;
					local_10 = 2;
					//local_48 = 1;
					currentState = 6;
					break;
				default:
					debug("undefined state!");
					currentState = 1;
					bombActor->_priorityLayer = 0;
					break;
				}
				if ((local_10 == 0) && (currentState != 2)) {
					if (!_vm->isLeftKeyPressed() || (local_850 < 0x37)) {
						if (!_vm->isRightKeyPressed() || (0x107 < local_850)) {
							if ((pusherActor->_sequenceID != local_66) &&
								(((pusherActor->_sequenceID != 4 && (pusherActor->_sequenceID != 5))
								  && (pusherActor->_sequenceID != 6)))) {
								pusherActor->updateSequence((uint)local_66);
								if (local_50 == 0) {
									pusherActor->_x_pos = flickerActor->_x_pos + -0xe;
									pusherActor->_y_pos = flickerActor->_y_pos + 7;
								} else {
									pusherActor->_x_pos = flickerActor->_x_pos + 2;
									pusherActor->_y_pos = flickerActor->_y_pos;
								}
							}
						} else {
							local_50 = 1;
							local_66 = 8;
							if (pusherActor->_sequenceID != 2) {
								pusherActor->updateSequence(2);
							}
							local_850 = local_850 + 2;
							if (local_850 < 0x109) {
								if (local_850 < 0x36) {
									local_850 = 0x36;
								}
							} else {
								local_850 = 0x108;
							}
							if (local_62 == 0) {
								local_62 = 0xb;
							} else {
								local_62 = local_62 - 1;
							}
							flickerActor->_x_pos = local_850;
							wheelsActor->_x_pos = local_850;
							if ((uint)wheelsActor->_sequenceID != (uint)local_62 / 3 + 0x11) {
								wheelsActor->updateSequence((uint)local_62 / 3 + 0x11);
							}
							pusherActor->_x_pos = flickerActor->_x_pos + 2;
							pusherActor->_y_pos = flickerActor->_y_pos;
						}
					} else {
						local_50 = 0;
						local_66 = (uint16)(currentState != 1);
						if (pusherActor->_sequenceID != 3) {
							pusherActor->updateSequence(3);
						}
						local_850 = local_850 - 2;
						if (local_850 < 0x109) {
							if (local_850 < 0x36) {
								local_850 = 0x36;
							}
						} else {
							local_850 = 0x108;
						}
						local_62 = (short)((uint)local_62 + 1) +
								   (short)((int)((uint)local_62 + 1) / 6 >> 1) * -0xc;
						flickerActor->_x_pos = local_850;
						wheelsActor->_x_pos = local_850;
						if ((uint)wheelsActor->_sequenceID != (uint)local_62 / 3 + 0x11) {
							wheelsActor->updateSequence((uint)local_62 / 3 + 0x11);
						}
						pusherActor->_x_pos = flickerActor->_x_pos + -2;
						pusherActor->_y_pos = flickerActor->_y_pos;
					}
				}
			} while (local_10 == 0);
			if ((local_10 == 2) || (0x117 < flickerActor->_x_pos)) break;
			flickerActor->_x_pos = flickerActor->_x_pos + 2;
			if (pusherActor->_sequenceID != 2) {
				pusherActor->updateSequence(2);
			}
			pusherActor->_x_pos = flickerActor->_x_pos + 2;
			pusherActor->_y_pos = flickerActor->_y_pos;
			wheelsActor->_x_pos = wheelsActor->_x_pos + 2;
			if (local_62 == 0) {
				local_62 = 0xb;
			} else {
				local_62 = local_62 - 1;
			}
			if ((uint)wheelsActor->_sequenceID != (uint)local_62 / 3 + 0x11) {
				wheelsActor->updateSequence((uint)local_62 / 3 + 0x11);
			}
		}
		if (local_10 == 1) break;
		if (local_10 == 2) {
			_vm->_dragonINIResource->getRecord(DAT_80063a40 + -1)->objectState2 = 2;
			pusherActor->updateSequence(7);
			_vm->waitForFrames(0x3c);
			pusherActor->updateSequence(1);
			_vm->waitForFrames(0x1e);
			_vm->_dragonINIResource->getRecord(DAT_80063a40 + -1)->actor->clearFlag(ACTOR_FLAG_100);
			LAB_8009157c:
			_vm->clearAllText();
			flickerActor->updateSequence(0x15);
			local_46 = 0;
			//local_44 = 0;
			if (local_10 == 2) {
//				DisableVSyncEvent();
				local_46 = pusherActor->_x_pos;
				//local_44 = pusherActor->_y_pos;
				pusherActor->reset_maybe();
//				EnableVSyncEvent();
			} else {
//				DisableVSyncEvent();
				dustActor->reset_maybe();
				bombActor->reset_maybe();
				wheelsActor->reset_maybe();
				pusherActor->reset_maybe();
//				EnableVSyncEvent();
			}
			_vm->_dragonINIResource->setFlickerRecord(local_78);
			if (local_10 == 2) {
				local_78->actor->_x_pos = local_46;
				local_78->actor->setFlag(ACTOR_FLAG_100);
				local_78->actor->_priorityLayer = 5;
			} else {
				local_78->actor->clearFlag(ACTOR_FLAG_100);
				local_78->actor->_priorityLayer = 2;
			}
			_vm->clearFlags(ENGINE_FLAG_4000000);
			_vm->setFlags(savedEngineFlags);
			return;
		}
	}
	_vm->_dragonINIResource->getRecord(DAT_80063a40 + -1)->actor->clearFlag(ACTOR_FLAG_100);
	_vm->_dragonINIResource->getRecord(DAT_80063a40 + -1)->objectState2 = 0;
	goto LAB_8009157c;
}

} // End of namespace Dragons
