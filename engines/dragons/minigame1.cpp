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
#include "dragons/minigame1.h"
#include "dragons/actor.h"
#include "dragons/dragons.h"
#include "dragons/dragonini.h"
#include "dragons/talk.h"
#include "dragons/screen.h"

namespace Dragons {

#define DAT_80063a48 0x12d
#define DAT_80063a40 0x12f

Minigame1::Minigame1(DragonsEngine *vm) : _vm(vm) {}

void Minigame1::run() {

	const uint32 dialogIdTbl[17] = {
		0x21312, 0x2134C, 0x21386, 0x213C0,
		0x213E2, 0x21428, 0x2146C, 0x214B4,
		0x214E4, 0x21514, 0x21540, 0x21590,
		0x215E2, 0x2164E, 0x216AA, 0x216D2,
		0x217D8
	};
/* WARNING: Could not reconcile some variable overlaps */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

	Actor *uVar1;
	short sVar2;
	bool bVar3;
	uint16 uVar5;
	uint savedEngineFlags;
	uint16 flickerXPos;
	uint16 auStack1008 [200];
	DragonINI *originalFlickerIniID;
	uint16 local_25c;
	uint16 local_25a = 0;
	uint16 catFieldE_scaleMaybe;
	uint16 hitCounter;
	uint16 local_254;
	short local_252;
	short local_250;
	uint16 local_246;
	uint16 local_242;
	uint16 local_240;
	uint16 local_23e;
	short local_23c = 0;
	short local_23a;
	short local_238;
	short gameState;
	uint16 local_234 = 0;
	short local_232 = 0;
	uint16 local_22e;
	uint16 local_22c;
	Actor *targetActorIdTbl [5];
	short local_21e;
	uint16 auStack536 [72];
	short local_188 [8];
	uint16 auStack378 [4];
	uint16 i;
	uint16 local_16e;
	short local_16a;
	uint32 local_168;
	uint16 auStack352 [24];
	uint16 local_130 [10];
	int32 local_118 [22];
	uint32 local_c0 [22];
	uint16 actorSequenceTimerStartValue;

	Actor *catActor;
	Actor *pusherActor;
	Actor *flickerActor;
	Actor *wheelsActor;
	Actor *dustSpriteActor;

	Common::File *fd = new Common::File();
	if (!fd->open("arc1.bin")) {
		error("Failed to open arc1.bin");
	}

	for (i = 0; i < 0x42; i++) {
		auStack536[i] = fd->readUint16LE();
	}

	fd->seek(132);
	for (i = 0; i < 0x18; i++) {
		auStack352[i] = fd->readUint16LE();
	}

	for (i = 0; i < 10; i++) {
		local_130[i] = fd->readUint16LE();
	}
	fd->close();
	delete fd;

//	local_130._0_4_ = DAT_8008e940;
//	local_130._4_4_ = DAT_8008e944;
//	local_130._8_4_ = DAT_8008e948;
//	local_130._12_4_ = DAT_8008e94c;
//	local_130[8] = DAT_8008e950;

	local_c0[0] = dialogIdTbl[0];
	local_c0[1] = 0x1e;
	local_c0[2] = dialogIdTbl[1];
	local_c0[3] = 0x23;
	local_c0[4] = dialogIdTbl[2];
	local_c0[5] = 0x1e;
	local_c0[6] = dialogIdTbl[3];
	local_c0[7] = 0x1e;
	local_c0[8] = dialogIdTbl[4];
	local_c0[9] = 0x28;
	local_c0[10] = dialogIdTbl[5];
	local_c0[11] = 0x1e;
	local_c0[12] = dialogIdTbl[6];
	local_c0[13] = 0x28;
	local_c0[14] = dialogIdTbl[7];
	local_c0[15] = 0x23;
	local_c0[16] = dialogIdTbl[8];
	local_c0[17] = 0x23;
	local_c0[18] = dialogIdTbl[9];
	local_c0[19] = 0x1e;
	local_c0[20] = dialogIdTbl[10];
	local_c0[21] = 0x32;

	memcpy(local_118, local_c0, 0x58);

	local_c0[0] = dialogIdTbl[11];
	local_c0[1] = 0x3c;
	local_c0[2] = dialogIdTbl[12];
	local_c0[3] = 0x3c;
	local_c0[12] = dialogIdTbl[13];
	local_c0[13] = 0x3c;
	local_c0[8] = dialogIdTbl[13];
	local_c0[9] = 0x3c;
	local_c0[14] = dialogIdTbl[14];
	local_c0[15] = 0x3c;
	local_c0[10] = dialogIdTbl[14];
	local_c0[11] = 0x3c;
	local_c0[4] = dialogIdTbl[13];
	local_c0[5] = 0x3c;
	local_c0[6] = dialogIdTbl[14];
	local_c0[7] = 0x3c;

	originalFlickerIniID = _vm->_dragonINIResource->getFlickerRecord();
	originalFlickerIniID->actor->setFlag(ACTOR_FLAG_100);
	originalFlickerIniID->actor->_priorityLayer = 0;

	savedEngineFlags = _vm->getMultipleFlags(ENGINE_FLAG_8 | ENGINE_FLAG_10 | ENGINE_FLAG_20 | ENGINE_FLAG_80);
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_10);
	_vm->clearFlags(ENGINE_FLAG_20);
	_vm->clearFlags(ENGINE_FLAG_80);
	_vm->_dragonINIResource->setFlickerRecord(_vm->getINI(DAT_80063a40 - 1));
	flickerActor = _vm->getINI(DAT_80063a40 - 1)->actor;
	flickerActor->_flags = flickerActor->_flags | 0x380;
	flickerActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	flickerActor->_priorityLayer = 4;
	flickerActor->_direction = -1;
	flickerActor->updateSequence(0x15);
	hitCounter = 0;
	local_254 = 0;
	local_252 = 0;
	flickerXPos = flickerActor->_x_pos;
	local_25c = 0;
	pusherActor = _vm->_actorManager->loadActor(0x26, 1, flickerXPos,
								  (int)(((uint)(uint16)flickerActor->_y_pos + 5) * 0x10000) >> 0x10);
//	if (pusherActorId == -1) {
//		ProbablyShowASCIIMessage(s_couldn't_alloc_pusher_8008e954, 2, 4, 0, 0xffffffff);
//	}
	pusherActor->_flags = pusherActor->_flags | 0x380;
	pusherActor->_x_pos = flickerActor->_x_pos + -0xe;
	pusherActor->_y_pos = flickerActor->_y_pos + 7;
	pusherActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	pusherActor->_priorityLayer = 6;
	wheelsActor = _vm->_actorManager->loadActor(7, 0x11, 0, 0);
//	if (wheelsActorId == -1) {
//		ProbablyShowASCIIMessage(s_couldn't_alloc_wheels_8008e96c, 2, 4, 0, 0xffffffff);
//	}
	wheelsActor->_flags = wheelsActor->_flags | 0x380;
	wheelsActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	wheelsActor->_x_pos = flickerActor->_x_pos;
	wheelsActor->_y_pos = flickerActor->_y_pos;
	wheelsActor->_priorityLayer = 5;
	wheelsActor->updateSequence(0x11);
	local_242 = 0;
	catActor = _vm->_actorManager->loadActor(7, 9, 0, 0);
//	if (catActorId == -1) {
//		ProbablyShowASCIIMessage(s_couldn't_alloc-cat_8008e984, 2, 4, 0, 0xffffffff);
//	}
	catActor->_flags = catActor->_flags | 0x380;
	catActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	catActor->_priorityLayer = 0;
	i = 0;
	while (i < 3) {
		targetActorIdTbl[(uint)i + 1] = _vm->_actorManager->loadActor(8, 1, 0, 0, 0);
//		if (targetActorIdTbl[(uint)i + 1] == -1) {
//			ProbablyShowASCIIMessage(s_couldn't_alloc_target!_8008e998, 2, 4, 0, 0xffffffff);
//		}
		targetActorIdTbl[(uint)i + 1]->_flags = targetActorIdTbl[(uint)i + 1]->_flags | 0x380;
		targetActorIdTbl[(uint)i + 1]->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
		auStack378[(uint)i] = 0;
		//TODO FUN_80017010_update_actor_texture_maybe(1);
		i = i + 1;
	}
	for (i = 0; i < 8; i++) {
		local_188[i] = 0;
	}
	dustSpriteActor = _vm->_actorManager->loadActor(8, 8, 100, 100, 0);
//	if (dustSpriteActorId == 0xffff) {
//		ProbablyShowASCIIMessage(s_couldn't_alloc_dust_sprite!_8008e9b0, 2, 5, 0, 0xffffffff);
//	}
	dustSpriteActor->_flags = dustSpriteActor->_flags | 0x380;
	dustSpriteActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	uVar1 = _vm->getINI(DAT_80063a48 - 1)->actor; //dragon_ini_pointer[DAT_80063a48 + -1].actorId;
	local_21e = 0;
	actorSequenceTimerStartValue = uVar1->_sequenceTimerMaxValue;
	_vm->setFlags(ENGINE_FLAG_4000000);
	local_23e = 0x3700;
	local_23a = 0x100;
	local_240 = 0x4a80;
	local_238 = 0;
	catFieldE_scaleMaybe = 0x30;
	catActor->_y_pos = 0x6e;
	catActor->_x_pos = 0x95;
	catActor->_scale = 0x30;
	catActor->_priorityLayer = 2;
	catActor->updateSequence(0xb);
	gameState = 5;
	local_246 = 1;
	bVar3 = false;
	local_22e = 0;
	local_22c = 0;
	local_250 = 0;
	LAB_8008fa78:
	do {
		do {
			_vm->waitForFrames(1);
			if ((local_250 != 0) && (local_250 = local_250 + -1, local_250 == 0)) {
				_vm->_talk->FUN_8001a7c4_clearDialogBoxMaybe();
			}
			if ((local_21e == 1) && (local_252 == 0)) {
				uVar1->_sequenceTimerMaxValue = actorSequenceTimerStartValue;
			}
			if (local_21e != 0) {
				local_21e = local_21e + -1;
			}
			switch (gameState) {
			case 0:
				break;
			case 1: //cat in the catapult ready to fire.
				if (local_252 == 0) {
					if (local_246 != 8) {
						local_246 = 0;
					}
//						iVar6 = IsButtonBeingPressed((uint)DAT_800728ac, 0);
					if (!_vm->isActionButtonPressed()) {
						if (local_25c == 0) {
							if ((((flickerActor->_sequenceID != 0) &&
								  (flickerActor->_sequenceID != 5)) &&
								 (flickerActor->_sequenceID != 6)) ||
								((flickerActor->_flags & 4) != 0)) {
								flickerActor->updateSequence(0);
							}
						} else {
							local_246 = 1;
							bVar3 = false;
							pusherActor->updateSequence(1);
							gameState = 2;
							if (local_25c < 0x14) {
								local_25a = 1;
							} else {
								if (local_25c < 0x2d) {
									local_25a = 2;
								} else {
									if (local_25c < 0x169) {
										local_25a = 3;
									}
								}
							}
						}
						local_25c = 0;
					} else {
						pusherActor->_x_pos = flickerActor->_x_pos + -0xe;
						pusherActor->_y_pos = flickerActor->_y_pos + 7;
						if (local_25c < 0x168) {
							local_25c = local_25c + 1;
							if (local_25c < 0x14) {
								if (((pusherActor->_sequenceID != 4) &&
								(pusherActor->_sequenceID != 2)) &&
								(pusherActor->_sequenceID != 3)) {
									pusherActor->updateSequence(4);
								}
								if (flickerActor->_sequenceID != 1) {
									flickerActor->updateSequence(1);
									_vm->playOrStopSound(2);
								}
							} else {
								if (local_25c < 0x2d) {
									if (((pusherActor->_sequenceID != 5) &&
									(pusherActor->_sequenceID != 2)) &&
									(pusherActor->_sequenceID != 3)) {
										pusherActor->updateSequence(5);
									}
									if (flickerActor->_sequenceID != 2) {
										flickerActor->updateSequence(2);
										_vm->playOrStopSound(3);
									}
								} else {
									if (local_25c < 0x169) {
										if (((pusherActor->_sequenceID != 6) &&
										(pusherActor->_sequenceID != 2)) &&
										(pusherActor->_sequenceID != 3)) {
											pusherActor->updateSequence(6);
										}
										if (flickerActor->_sequenceID != 3) {
											flickerActor->updateSequence(3);
											_vm->playOrStopSound(4);
										}
									}
								}
							}
						} else {
							if (pusherActor->_sequenceID != 6) {
								pusherActor->updateSequence(6);
							}
							if (flickerActor->_sequenceID != 3) {
								flickerActor->updateSequence(3);
							}
						}
					}
				}
				break;
			case 2: // initial release of cat.
				if (flickerActor->_sequenceID == 7) {
					if ((flickerActor->_flags & 4) != 0) {
						i = 1;
						while ((i < 8 && ((((int)(uint)flickerXPos < (int)((uint)auStack352[(uint)i * 3] - 6) ||
											((uint)auStack352[(uint)i * 3 + 1] + 6 < (uint)flickerXPos)) ||
											(local_25a != auStack352[(uint)i * 3 + 2]))))) {
							i = i + 1;
						}
						local_23c = 0;
						if ((i != 8) && ((flickerXPos < auStack352[(uint)i * 3] || (auStack352[(uint)i * 3] < flickerXPos)))) {
							local_23c = (short)((int)(((uint)auStack352[(uint)i * 3] + 8) * 0x80) / 0x2a) -
										(short)((int)((uint)flickerXPos << 7) / 0x2a);
						}
						local_240 = flickerXPos << 7;
						catActor->_x_pos = flickerXPos & 0x1ff;
						local_23e = 0x2d00;
						local_23a = (local_25a + 3) * 0x80;
						catActor->_y_pos = 0x5a;
						catFieldE_scaleMaybe = 0x100;
						catActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
						catActor->updateSequence(0xc);
						_vm->playOrStopSound(5);
						catActor->_priorityLayer = 3;
						flickerActor->updateSequence(8);
						gameState = 3;
					}
				} else {
					flickerActor->updateSequence(7);
					_vm->playOrStopSound(1);
				}
				break;
			case 3: // cat flying through the air
				local_240 = local_240 + local_23c;
				if ((uint)local_25a * 2 + 0xb4 < (uint)catFieldE_scaleMaybe) {
					local_23e = local_23e - local_23a;
					local_23a = local_23a - local_130[((uint)local_25a - 1) * 3];
					if (local_23a < 0) {
						local_23a = 0;
					}
				} else {
					if ((int)(uint)catFieldE_scaleMaybe < (int)((uint)local_25a * -4 + 0xba)) {
						local_23e = local_23e + local_23a;
						local_23a = local_23a + local_130[((uint)local_25a - 1) * 3 + 2];
					} else {
						local_23a = 0;
					}
				}
				catActor->_x_pos = local_240 >> 7;
				catActor->_y_pos = local_23e >> 7;
				catFieldE_scaleMaybe = catFieldE_scaleMaybe - 3;
				catActor->_scale = catFieldE_scaleMaybe;
				if (catFieldE_scaleMaybe == 0x7f) {
					i = 0;
					while ((i < 8 && (((local_240 >> 7 < auStack352[(uint)i * 3] ||
										(auStack352[(uint)i * 3 + 1] < local_240 >> 7)) ||
									   (local_25a != auStack352[(uint)i * 3 + 2]))))) {
						i = i + 1;
					}
					if ((i != 8) && (local_188[(uint)i] != 0)) {
						uVar1->_sequenceTimerMaxValue = 2;
						local_21e = 0x3c;
						if (local_250 != 0) {
							_vm->_talk->FUN_8001a7c4_clearDialogBoxMaybe();
							local_250 = 0;
						}
						hitCounter = hitCounter + 1;
						catActor->updateSequence(0xd);
						if ((i == 0) && (9 < hitCounter)) {
							local_22c = 0x16;
							local_252 = 2;
							catActor->updateSequence(0xd);
							gameState = 4;
						} else {
							catActor->updateSequence(0xd);
							_vm->playOrStopSound(6);
							gameState = 8;
							local_234 = 0;
						}
						if (local_252 == 0) { //successful hit maybe?
							uint32 textId = _vm->getDialogTextId(local_118[((uint)hitCounter - 1) * 2]);
							_vm->_talk->loadText(textId, auStack1008, 200);
							_vm->_talk->displayDialogAroundPoint(auStack1008, (int)(short)(flickerXPos >> 3), 0xc, 0, 0, textId);
							local_250 = *(short *)(local_118 + ((uint)hitCounter - 1) * 2 + 1);
						}
						targetActorIdTbl[(uint)(uint16)local_188[(uint)i]]->_priorityLayer = 3;
						if (i == 0) {
							targetActorIdTbl[(uint)local_188[0]]->updateSequence(7);
						} else {
							targetActorIdTbl[(uint)(uint16)local_188[(uint)i]]->_y_pos -= 3;
							targetActorIdTbl[(uint)(uint16)local_188[(uint)i]]->updateSequence(6);
						}
						auStack378[(uint)(uint16)local_188[(uint)i] - 1] = 0;
						local_188[(uint)i] = 0;
						break;
					}
					if ((i == 8) &&
						((((local_25a == 1 && (local_240 >> 7 < 0x10e)) ||
						   ((local_25a == 2 &&
							 ((((0x7f < local_240 >> 7 && (local_240 >> 7 < 0xad)) ||
								((0x30 < local_240 >> 7 && (local_240 >> 7 < 0x4a)))) ||
							   ((0xf8 < local_240 >> 7 && (local_240 >> 7 < 0x10f)))))))) ||
						  ((local_25a == 3 &&
							(((0x3c < local_240 >> 7 && (local_240 >> 7 < 0x46)) ||
							  ((0x101 < local_240 >> 7 && (local_240 >> 7 < 0x10a)))))))))) {
						dustSpriteActor->_x_pos = catActor->_x_pos;
						dustSpriteActor->_y_pos = catActor->_y_pos + 2;
						dustSpriteActor->updateSequence(8);
						catActor->_priorityLayer = 4;
						dustSpriteActor->_priorityLayer = 3;
						catActor->updateSequence(0xd);
						gameState = 4;
						_vm->playOrStopSound(6);
					}
				}
				if (catFieldE_scaleMaybe < 0x7f) {
					catActor->_priorityLayer = 2;
				}
				if ((0xc < catFieldE_scaleMaybe) && (catFieldE_scaleMaybe < 0x41)) {
					catActor->_priorityLayer = 0;
				}
				if ((short)catFieldE_scaleMaybe < 2) {
					local_23e = 0x3700;
					local_23a = 0x100;
					local_240 = 0x4a80;
					catFieldE_scaleMaybe = 0x30;
					catActor->_y_pos = 0x6e;
					catActor->_x_pos = 0x95;
					catActor->_scale = 0x30;
					catActor->updateSequence(0xb);
					gameState = 5;
				}
				break;
			case 4: // cat sliding down wall.
				if (((catActor->_flags & 4) != 0) &&
					((dustSpriteActor->_flags & 4) != 0)) {
					if (catActor->_sequenceID == 0xe) {
						if (local_23e < 0x4300) {
							local_23e = local_23e + local_23a;
							local_23a = local_23a + 0x18;
							catActor->_y_pos = local_23e >> 7;
						} else {
							catActor->updateSequence(0xf);
							_vm->playOrStopSound(7);
							gameState = 6;
						}
					} else {
						dustSpriteActor->_priorityLayer = 0;
						catActor->_priorityLayer = 3;
						catActor->updateSequence(0xe);
						_vm->playOrStopSound(8);
						local_23a = 0x40;
					}
				}
				break;
			case 5: // cat behind portcullis
				if (local_23e >> 7 < 0x86) {
					local_23e = local_23e + local_23a;
					catFieldE_scaleMaybe = catFieldE_scaleMaybe + 8;
					catActor->_y_pos = local_23e >> 7;
					catActor->_scale = catFieldE_scaleMaybe;
				} else {
					gameState = 6;
					catActor->_sequenceID = 0x10;
					catActor->_flags = catActor->_flags | 4;
				}
				break;
			case 6: // cat run across field
				catActor->_priorityLayer = 3;
				if (local_252 == 0) {
					if (catActor->_sequenceID == 0xf) {
						if ((catActor->_flags & 4) != 0) {
							catActor->updateSequence(0x10);
						}
					} else {
						if (catActor->_sequenceID == 0x10) {
							if ((catActor->_flags & 4) != 0) {
								catFieldE_scaleMaybe = 0x80;
								local_23e = 0x4300;
								local_23a = 0x100;
								catActor->_y_pos = 0x86;
								catActor->_scale = 0x80;
								catActor->updateSequence(0xb);
								if (flickerXPos < local_240 >> 7) {
									sVar2 = flickerXPos + 0x32;
								} else {
									sVar2 = flickerXPos - 0x32;
								}
								local_16e = sVar2 * 0x80;
								local_23c = (short)(((int)(((uint)local_16e - (uint)local_240) * 0x10000) >> 0x10) /
													0x1c);
							}
						} else {
							if (local_23e < 0x5f00) {
								if (local_238 == 0) {
									local_23e = local_23e + local_23a;
									local_240 = local_240 + local_23c;
									catFieldE_scaleMaybe = catFieldE_scaleMaybe + 6;
									if (0x100 < catFieldE_scaleMaybe) {
										catFieldE_scaleMaybe = 0x100;
									}
									catActor->_scale = catFieldE_scaleMaybe;
									catActor->_y_pos = local_23e >> 7;
									catActor->_x_pos = local_240 >> 7;
									local_238 = 1;
								} else {
									local_238 = local_238 + -1;
								}
							} else {
								if ((int)(uint)(local_240 >> 7) < (int)((uint)flickerXPos - 0x32)) {
									if (catActor->_sequenceID != 9) {
										catActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
										catActor->updateSequence(9);
									}
									local_240 = local_240 + 0x180;
									catActor->_x_pos = local_240 >> 7;
								} else {
									if ((uint)flickerXPos + 0x32 < (uint)(local_240 >> 7)) {
										if (catActor->_sequenceID != 10) {
											catActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
											catActor->updateSequence(10);
										}
										local_240 = local_240 - 0x180;
										catActor->_x_pos = local_240 >> 7;
									} else {
										gameState = 7;
									}
								}
							}
						}
					}
				}
				break;
			case 7: // cat jumping into catapult
				if (catActor->_priorityLayer == 0) {
					gameState = 1;
				} else {
					catActor->_priorityLayer = 0;
					if (local_240 >> 7 < flickerXPos) {
						flickerActor->updateSequence(5);
					} else {
						flickerActor->updateSequence(6);
					}
				}
				break;
			case 8: // cat hit target.
				if (local_234 == 1) {
					if (local_23a < 1) {
						local_234 = 2;
					} else {
						local_23a--;
						catActor->_y_pos = catActor->_y_pos + 2;
					}
				} else {
					if (local_234 < 2) {
						if ((local_234 == 0) && ((catActor->_flags & 4) != 0)) {
							local_234 = 3;
							local_232 = 0;
						}
					} else {
						if (local_234 == 2) {
							local_23e = 0x4100;
							local_240 = 0x4a80;
							catActor->_y_pos = 0x82;
							catActor->_x_pos = 0x95;
							catActor->_priorityLayer = 3;
							catActor->_scale = 0x80;
							catActor->updateSequence(0x10);
							gameState = 6;
						} else {
							if (local_234 == 3) {
								if (local_232 == 0) {
									catActor->_priorityLayer = 2;
									local_23a = 8;
									local_234 = 1;
								} else {
									local_232 = local_232 + -1;
								}
							}
						}
					}
				}
				break;
			default:
				//ProbablyShowASCIIMessage(s_undefined_state!_8008e9cc, 2, 3, 0, 0xffffffff);
				gameState = 1;
				catActor->_priorityLayer = 0;
				flickerActor->updateSequence(0);
			}
			if ((local_252 == 0) && (gameState != 2)) {
				if (!_vm->isLeftKeyPressed() || (flickerXPos < 0x37)) {
					if (!_vm->isRightKeyPressed() || (0x107 < flickerXPos)) {
						if ((pusherActor->_sequenceID != local_246) &&
						(((pusherActor->_sequenceID != 4 &&
															   (pusherActor->_sequenceID != 5)) &&
						(pusherActor->_sequenceID != 6)))) {
							pusherActor->updateSequence((uint)local_246);
							if (bVar3) {
								pusherActor->_x_pos = flickerActor->_x_pos + 2;
								pusherActor->_y_pos = flickerActor->_y_pos;
							} else {
								pusherActor->_x_pos = flickerActor->_x_pos - 0xe;
								pusherActor->_y_pos = flickerActor->_y_pos + 7;
							}
						}
					} else {
						bVar3 = true;
						local_246 = 8;
						if (pusherActor->_sequenceID != 2) {
							pusherActor->updateSequence(2);
						}
						flickerXPos = flickerXPos + 2;
						if (flickerXPos < 0x109) {
							if (flickerXPos < 0x36) {
								flickerXPos = 0x36;
							}
						} else {
							flickerXPos = 0x108;
						}
						if (local_242 == 0) {
							local_242 = 0xb;
						} else {
							local_242 = local_242 - 1;
						}
						flickerActor->_x_pos = flickerXPos;
						wheelsActor->_x_pos = flickerXPos;
						if ((uint)wheelsActor->_sequenceID != (uint)local_242 / 3 + 0x11)
						{
							wheelsActor->updateSequence((uint)local_242 / 3 + 0x11);
						}
						pusherActor->_x_pos = flickerActor->_x_pos + 2;
						pusherActor->_y_pos = flickerActor->_y_pos;
					}
				} else {
					bVar3 = false;
					local_246 = (uint16)(gameState != 1);
					if (pusherActor->_sequenceID != 3) {
						pusherActor->updateSequence(3);
					}
					flickerXPos = flickerXPos - 2;
					if (flickerXPos < 0x109) {
						if (flickerXPos < 0x36) {
							flickerXPos = 0x36;
						}
					} else {
						flickerXPos = 0x108;
					}
					local_242 = (short)((uint)local_242 + 1) +
								(short)((int)((uint)local_242 + 1) / 6 >> 1) * -0xc;
					flickerActor->_x_pos = flickerXPos;
					wheelsActor->_x_pos = flickerXPos;
					if ((uint)wheelsActor->_sequenceID != (uint)local_242 / 3 + 0x11) {
						wheelsActor->updateSequence((uint)local_242 / 3 + 0x11);
					}
					pusherActor->_x_pos = flickerActor->_x_pos + -2;
					pusherActor->_y_pos = flickerActor->_y_pos;
				}
			}
			if ((local_22c < 0x16) && (auStack536[(uint)local_22c * 3 + 2] <= local_22e)) {
				if ((local_22c == 0x14) && (hitCounter < 9)) {
					local_252 = 1;
					local_22c = 0x16;
				} else {
					if (auStack536[(uint)local_22c * 3 + 1] == 1) {
						i = 0;
						while ((i < 3 && (auStack378[(uint)i] != 0))) {
							i = i + 1;
						}
						if (i == 3) {
							debug("too many targets");
//							ProbablyShowASCIIMessage(s_too_many_targets!_8008e9e0, 2, 4, 0, 0xffffffff);
						}
						if (auStack536[(uint)local_22c * 3] == 0) {
							targetActorIdTbl[(uint)i + 1]->_x_pos = auStack352[(uint)auStack536[(uint)local_22c * 3] * 3] + 0xd;
						} else {
							targetActorIdTbl[(uint)i + 1]->_x_pos = auStack352[(uint)auStack536[(uint)local_22c * 3] * 3] + 8;
						}
						targetActorIdTbl[(uint)i + 1]->_y_pos = (4 - auStack352[(uint)auStack536[(uint)local_22c * 3] * 3 + 2]) * 0x20;
						targetActorIdTbl[(uint)i + 1]->_priorityLayer = 2;
						if (auStack536[(uint)local_22c * 3] == 0) {
							targetActorIdTbl[(uint)i + 1]->updateSequence(3);
						} else {
							targetActorIdTbl[(uint)i + 1]->updateSequence(0);
						}
						targetActorIdTbl[(uint)i + 1]->_priorityLayer = 2;
						local_188[(uint)auStack536[(uint)local_22c * 3]] = i + 1;
						auStack378[(uint)i] = auStack536[(uint)local_22c * 3] + 1;
					} else {
						if ((auStack536[(uint)local_22c * 3 + 1] == 2) &&
							(i = auStack536[(uint)local_22c * 3], local_188[(uint)i] != 0)) {
							if (auStack536[(uint)local_22c * 3] == 0) {
								targetActorIdTbl[(uint)(uint16)local_188[(uint)i]]->updateSequence(5);
							} else {
								targetActorIdTbl[(uint)(uint16)local_188[(uint)i]]->updateSequence(2);
							}
							if (local_250 != 0) {
								_vm->_talk->FUN_8001a7c4_clearDialogBoxMaybe();
								local_250 = 0;
							}
							if ((local_254 == 0) && (local_252 == 0)) {
								if (auStack536[(uint)local_22c * 3] == 0) {
									uVar5 = _vm->getRand(2);
									local_168 = local_c0[(uint)uVar5 * 2 + 4];
									local_16a = *(short *)(local_c0 + (uint)uVar5 * 2 + 5);
								} else {
									uVar5 = _vm->getRand(2);
									local_168 = local_c0[(uint)uVar5 * 2];
									local_16a = *(short *)(local_c0 + (uint)uVar5 * 2 + 1);
								}
							} else {
								uVar5 = _vm->getRand(2);
								local_168 = local_c0[(uint)uVar5 * 2 + 4];
								local_16a = *(short *)(local_c0 + (uint)uVar5 * 2 + 5);
							}
							if ((local_252 == 0) || ((auStack536[(uint)local_22c * 3] == 0 && (local_254 == 0)))) {
								uint32 textId = _vm->getDialogTextId(local_168);
								_vm->_talk->loadText(textId, auStack1008, 200);
								_vm->_talk->displayDialogAroundPoint(auStack1008, (int)(short)(flickerXPos >> 3), 0xc, 0, 0, textId);
								local_250 = local_16a;
							}
							if (local_254 < 2) {
								local_254 = local_254 + 1;
							}
							auStack378[(uint)(uint16)local_188[(uint)i] - 1] = 0;
							local_188[(uint)i] = 0;
							if ((1 < local_254) || (auStack536[(uint)local_22c * 3] == 0)) {
								local_252 = 1;
							}
						}
					}
				}
				local_22e = 0;
				local_22c = local_22c + 1;
			}
			i = 0;
			while (i < 3) {
				if ((targetActorIdTbl[(uint)i + 1]->_sequenceID == 1) ||
				(targetActorIdTbl[(uint)i + 1]->_sequenceID == 4)) {
					local_188[(uint)auStack378[(uint)i]] = 0;
					auStack378[(uint)i] = 0;
				}
				i = i + 1;
			}
			local_22e = local_22e + 1;
		} while (local_252 == 0);

		if (flickerActor->_x_pos < 0x118) {
			flickerActor->_x_pos = flickerActor->_x_pos + 2;
			if (pusherActor->_sequenceID != 2) {
				pusherActor->updateSequence(2);
			}
			pusherActor->_x_pos = flickerActor->_x_pos + 2;
			pusherActor->_y_pos = flickerActor->_y_pos;
			wheelsActor->_x_pos = wheelsActor->_x_pos + 2;
			if (local_242 == 0) {
				local_242 = 0xb;
			} else {
				local_242--;
			}
			if ((uint)wheelsActor->_sequenceID != (uint)local_242 / 3 + 0x11) {
				wheelsActor->updateSequence((uint)local_242 / 3 + 0x11);
			}
			goto LAB_8008fa78;
		}

		if ((local_252 == 1) && ((gameState == 6 || (gameState == 1)))) {
			pusherActor->updateSequence(9);
			_vm->waitForFrames(0xf);
			if (local_250 != 0) {
				_vm->_talk->FUN_8001a7c4_clearDialogBoxMaybe();
			}
			_vm->waitForFrames(0x3c);
			_vm->getINI(DAT_80063a40 - 1)->actor->clearFlag(ACTOR_FLAG_100);
			break;
		}

		if ((local_252 == 2) && (gameState == 6)) {
			_vm->getINI(DAT_80063a40 - 1)->objectState2 = 2;
			if (local_250 != 0) {
				_vm->_talk->FUN_8001a7c4_clearDialogBoxMaybe();
			}
			pusherActor->updateSequence(7);
			_vm->_talk->loadText(_vm->getDialogTextId(0x216D2), auStack1008, 200);
			_vm->_talk->displayDialogAroundPoint(auStack1008, 0x19, 0xc, 0, 1, _vm->getDialogTextId(0x216D2));

			pusherActor->updateSequence(1);
			_vm->waitForFrames(0x1e);
			_vm->getINI(DAT_80063a40 - 1)->actor->clearFlag(ACTOR_FLAG_100);
			break;
		}
	} while (true);

	_vm->clearAllText();
	flickerActor->updateSequence(0x15);
	// DisableVSyncEvent();
	catActor->reset_maybe();
	wheelsActor->reset_maybe();
	i = 0;
	while (i < 3) {
		targetActorIdTbl[(uint)i + 1]->reset_maybe();
		i = i + 1;
	}
	pusherActor->reset_maybe();
	dustSpriteActor->reset_maybe();
	// EnableVSyncEvent();
	_vm->_dragonINIResource->getFlickerRecord()->actor->clearFlag(ACTOR_FLAG_100);
	_vm->_dragonINIResource->setFlickerRecord(originalFlickerIniID);
	flickerActor = originalFlickerIniID->actor;
	flickerActor->clearFlag(ACTOR_FLAG_100);
	flickerActor->_priorityLayer = 6; //TODO this is 2 in the original but that leave flicker invisible.
	_vm->clearFlags(ENGINE_FLAG_4000000);
	_vm->setFlags(savedEngineFlags);
	uVar1->_sequenceTimerMaxValue = actorSequenceTimerStartValue;
	return;
}

} // End of namespace Dragons
