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
#include "dragons/minigame2.h"
#include "dragons/actor.h"
#include "dragons/actorresource.h"
#include "dragons/dragons.h"
#include "dragons/dragonini.h"
#include "dragons/font.h"
#include "dragons/talk.h"
#include "dragons/inventory.h"
#include "dragons/scene.h"
#include "dragons/screen.h"
#include "dragons/cursor.h"
#include "dragons/sound.h"

namespace Dragons {

Minigame2::Minigame2(DragonsEngine *vm) : _vm(vm), _dat_80093c70(false), _dat_80093c72(false), _dat_80093c74(0), _dat_80093ca8(false) {
	_dat_80093cb4 = 0;
	_dat_80093cbc = 0;
	_dat_80093cb8 = 0;
	_dat_80093cc0 = 0;
	_dat_80093ca4 = 0;
	_dat_80093c90 = 0;
	_dat_80093c94 = 0;
	_dat_80093cac = 0;
	_dat_80093cb0 = false;
	_dat_80093c9c = 0;
	_dat_80093c98 = 0;
	_dat_80093ca0 = 0;
	_dat_80093cc4 = 0;
	_dat_80093cc8 = 0;
}

static const uint16 unkArray[5] = {
	0xC, 0xA, 0x8, 0x6, 0x4
};

void Minigame2::run(int16 param_1, uint16 param_2, int16 param_3) {
	short sVar2;
	short sVar3;
	bool bVar4;
	DragonINI *flicker;
	uint32 origEngineFlags;
	Actor *loungealotBigPunchActor;
	Actor *loungealotHeadActor;
	Actor *loungealotLeftUpperArm;
	Actor *loungealotLegActor;
	Actor *loungealotRightArm;
	Actor *flickerArm;
	Actor *loungealotThumb;
	Actor *uVar12;
	Actor *uVar13;
	Actor *flickerPortrait;
	Actor *loungealotPortrait;
	uint uVar18;
	//uint uVar19;
	//int iVar20;
	bool shouldShakeScreen;
	uint16 local_2e6;
	int16 actorSequenceIdTbl [15];
	uint32 textIdTbl [4];
	uint16 local_2b0 [8];
	uint16 local_288;
	uint16 local_286;
	uint16 local_284;
	uint16 local_282;
	uint16 local_27a;
	uint16 local_278;
	short local_272;
	short local_26c;
	uint16 local_268;
	uint16 local_264;
	uint16 local_262;
	short local_260;
	short local_258;
	short local_256;
	int16 screenShakeTbl [10];
	uint16 screenShakeCounter;
	uint8 paletteData [0x200];
	uint16 local_28;
	InventoryState originalInventoryType;

	origEngineFlags = _vm->getAllFlags();
	originalInventoryType = _vm->_inventory->getState();
	flicker = _vm->_dragonINIResource->getFlickerRecord();

	Common::File *fd = new Common::File();
	if (!fd->open("arc2.bin")) {
		error("Failed to open arc2.bin");
	}

	for (int i = 0; i < 15; i++) {
		actorSequenceIdTbl[i] = fd->readSint16LE();
	}

	fd->skip(2);

	for (int i = 0; i < 6; i++) {
		local_2b0[i] = fd->readSint16LE();
	}

	for (int i = 0; i < 9; i++) {
		screenShakeTbl[i] = fd->readSint16LE();
	}

	fd->close();

	textIdTbl[0] = _vm->getDialogTextId(0x4500);
	textIdTbl[1] = _vm->getDialogTextId(0x454A);
	textIdTbl[2] = _vm->getDialogTextId(0x4576);

	bVar4 = false;
	local_27a = 0;
	local_278 = 0;
	local_272 = 0;
	local_26c = 0x1e;
	shouldShakeScreen = false;
	local_264 = 0;
	local_262 = 0;
	local_260 = 300;
	local_258 = 0;
	local_256 = 0;
	screenShakeCounter = 0;
	local_2e6 = param_2;
	if (param_2 > 4) {
		local_2e6 = 4;
	}
	_dat_80093ca4 = unkArray[local_2e6];
	_dat_80093c90 = unkArray[local_2e6];
	_dat_80093c94 = 0;
	_vm->_inventory->setState(Closed);
	_dat_80093cb4 = 2;
	_dat_80093cbc = 0;
	_dat_80093cb8 = 2;
	_dat_80093cc0 = 0;
	_dat_80093cac = 0;
	_dat_80093cb0 = false;
	_dat_80093c9c = 0;
	_vm->_talk->_dat_8008e874_dialogBox_y2 = 0;
	_vm->_talk->_dat_8008e844_dialogBox_y1 = 0;
	_vm->_talk->_dat_8008e848_dialogBox_x2 = 0;
	_vm->_talk->_dat_8008e7e8_dialogBox_x1 = 0;
	_dat_80093c98 = _dat_80093c90;
	_dat_80093ca0 = param_1;
	_dat_80093cc4 = _dat_80093ca4;
	_dat_80093cc8 = _dat_80093ca4;
	_vm->fadeToBlack();
	_vm->reset_screen_maybe();
	_vm->_inventory->setState(Closed);
	flicker->sceneId = 0;
	_vm->_dragonINIResource->setFlickerRecord(nullptr);
	_vm->setFlags(ENGINE_FLAG_800);
	_vm->_scene->setSceneId(5);
	_vm->_scene->loadSceneData(5 | 0x8000, 0);
	//DisableVSyncEvent();
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_10);
	_vm->clearFlags(ENGINE_FLAG_20);
	_vm->clearFlags(ENGINE_FLAG_80);

	memcpy(paletteData, _vm->_scene->getPalette() + 0x180, 0x80);
	memcpy(paletteData + 0x80, _vm->_scene->getPalette() + 0x180, 0x80);
	_vm->_screen->loadPalette(1, paletteData);
	_vm->_screen->updatePaletteTransparency(1, 0x40, 0x7f, true);

	loungealotBigPunchActor = _vm->_actorManager->loadActor(0x11, 0, 0, 0, 6);
	loungealotHeadActor = _vm->_actorManager->loadActor(0xd, 0, 0x7d, 199, 4);
	loungealotLeftUpperArm = _vm->_actorManager->loadActor(0xb, 2, 0x7d, 199, 4);
	loungealotLegActor = _vm->_actorManager->loadActor(0xf, 0, 0x7d, 199, 4);
	loungealotRightArm = _vm->_actorManager->loadActor(0x10, 0, 0x7d, 199, 4);
	flickerArm = _vm->_actorManager->loadActor(9, (uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb4 * 3 + (uint)_dat_80093cbc],
											   loungealotLeftUpperArm->_x_pos,
											   loungealotLeftUpperArm->_y_pos, 4);
	loungealotThumb = _vm->_actorManager->loadActor(0x12, (uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb4 * 3 + (uint)_dat_80093cbc],
													loungealotLeftUpperArm->_x_pos,
													loungealotLeftUpperArm->_y_pos, 4);
	uVar12 = _vm->_actorManager->loadActor(10, (uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb8 * 3 + (uint)_dat_80093cc0],
										   flickerArm->_x_pos,
										   flickerArm->_y_pos, 4);
	uVar13 = _vm->_actorManager->loadActor(0x13, (uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb8 * 3 + (uint)_dat_80093cc0],
										   flickerArm->_x_pos,
										   flickerArm->_y_pos, 4);
	flickerPortrait = _vm->_actorManager->loadActor(0x27, 0, 0x10, 0xac, 4);
	loungealotPortrait = _vm->_actorManager->loadActor(0x27, 1, 0x10, 0x8c, 4);
	loungealotBigPunchActor->setFlag(ACTOR_FLAG_100);
	loungealotHeadActor->setFlag(ACTOR_FLAG_100);
	loungealotLeftUpperArm->setFlag(ACTOR_FLAG_100);
	loungealotLegActor->setFlag(ACTOR_FLAG_100);
	loungealotRightArm->setFlag(ACTOR_FLAG_100);
	flickerArm->setFlag(ACTOR_FLAG_100);
	loungealotThumb->setFlag(ACTOR_FLAG_100);
	uVar12->setFlag(ACTOR_FLAG_100);
	uVar13->setFlag(ACTOR_FLAG_100);
	flickerPortrait->setFlag(ACTOR_FLAG_100);
	loungealotPortrait->setFlag(ACTOR_FLAG_100);

	loungealotBigPunchActor->_priorityLayer = 6;
	flickerArm->_priorityLayer = 5;
	uVar12->_priorityLayer = 5;
	loungealotThumb->_priorityLayer = 4;
	uVar13->_priorityLayer = 3;
	loungealotRightArm->_priorityLayer = 3;
	loungealotLeftUpperArm->_priorityLayer = 2;
	loungealotHeadActor->_priorityLayer = 2;
	loungealotLegActor->_priorityLayer = 1;
	flickerPortrait->_priorityLayer = 0;
	loungealotPortrait->_priorityLayer = 0;
	FlatQuad *flickerPowerMeter = _vm->_screen->getFlatQuad(_vm->_screen->addFlatQuad(0x28, 0xa8, 0x67, 0xa8, 0x67, 0xaf, 0x28, 0xaf, 0x1f, 7, 0));
	FlatQuad *loungealotPowerMeter = _vm->_screen->getFlatQuad(_vm->_screen->addFlatQuad(0x28, 0x88, 0x67, 0x88, 0x67, 0x8f, 0x28, 0x8f, 0x3e0, 7, 0));
	flickerPowerMeter->flags &= ~1u;
	loungealotPowerMeter->flags &= ~1u;

//	EnableVSyncEvent();
	loungealotHeadActor->setFlag(ACTOR_FLAG_1);
	loungealotLeftUpperArm->setFlag(ACTOR_FLAG_1);
	loungealotLegActor->setFlag(ACTOR_FLAG_1);
	loungealotRightArm->setFlag(ACTOR_FLAG_1);
	flickerArm->setFlag(ACTOR_FLAG_1);
	loungealotThumb->setFlag(ACTOR_FLAG_1);
	uVar12->setFlag(ACTOR_FLAG_1);
	uVar13->setFlag(ACTOR_FLAG_1);
	flickerArm->waitUntilFlag8And4AreSet();
	uVar12->waitUntilFlag8And4AreSet();
	loungealotThumb->waitUntilFlag8And4AreSet();
	uVar13->waitUntilFlag8And4AreSet();
	loungealotLeftUpperArm->waitUntilFlag8And4AreSet();
	loungealotRightArm->waitUntilFlag8And4AreSet();
	loungealotHeadActor->waitUntilFlag8And4AreSet();
	loungealotLegActor->waitUntilFlag8And4AreSet();

	loungealotBigPunchActor->setFlag(ACTOR_FLAG_400);
	flickerArm->_x_pos = loungealotLeftUpperArm->_x_pos - loungealotLeftUpperArm->_frame->field_e;
	flickerArm->_y_pos = loungealotLeftUpperArm->_y_pos - loungealotLeftUpperArm->_frame->field_10;
	loungealotThumb->_x_pos = loungealotLeftUpperArm->_x_pos - loungealotLeftUpperArm->_frame->field_e;
	loungealotThumb->_y_pos = loungealotLeftUpperArm->_y_pos - loungealotLeftUpperArm->_frame->field_10;
	uVar12->_x_pos = loungealotLeftUpperArm->_x_pos - flickerArm->_frame->field_e;
	uVar12->_y_pos = loungealotLeftUpperArm->_y_pos - flickerArm->_frame->field_10;
	uVar13->_x_pos = loungealotLeftUpperArm->_x_pos - flickerArm->_frame->field_e;
	uVar13->_y_pos = loungealotLeftUpperArm->_y_pos - flickerArm->_frame->field_10;
	_vm->waitForFrames(2);
	_vm->fadeFromBlack();

	do {
		_vm->waitForFrames(1);

		if ((_dat_80093c9c != 0) && !_vm->isFlagSet(ENGINE_FLAG_8000)) {
			loungealotHeadActor->updateSequence(0);
			_vm->_talk->FUN_8001a7c4_clearDialogBoxMaybe();
			_dat_80093c9c = 0;
		}
		if (_dat_80093c94 != 0) {
			local_264 = 0;
		}
		if (loungealotLegActor->_field_7a == 1) {
			shouldShakeScreen = true;
			loungealotLegActor->_field_7a = 0;
			screenShakeCounter = 0;
		}
		if (shouldShakeScreen) {
			_vm->_screen->setScreenShakeOffset(0, screenShakeTbl[screenShakeCounter]);
			if (screenShakeTbl[screenShakeCounter] == 0) {
				shouldShakeScreen = false;
			}
			screenShakeCounter = screenShakeCounter + 1;
		}
		if (_dat_80093ca4 == 0) {
			_dat_80093ca4 = _dat_80093cc8;
		} else {
			_dat_80093ca4 = _dat_80093ca4 - 1;
		}
		if (_dat_80093c90 == 0) {
			_dat_80093c90 = _dat_80093c98;
		} else {
			_dat_80093c90 = _dat_80093c90 - 1;
		}
		if (_dat_80093cc4 == 0) {
			_dat_80093cc4 = _dat_80093cc8;
		} else {
			_dat_80093cc4 = _dat_80093cc4 - 1;
		}

		if (local_27a == 0) {
			flickerPowerMeter->flags &= ~1u;
		} else {
			flickerPowerMeter->points[1].x = local_27a + 0x27;
			flickerPowerMeter->points[3].x = local_27a + 0x27;
			flickerPowerMeter->colour = (uint16)(((int)((uint)local_27a - 1) >> 1) << 5) |
										(uint16)(((int)(0x40 - (uint)local_27a) >> 1) << 10);
			flickerPowerMeter->flags |= 1;
		}
		if (local_278 == 0) {
			loungealotPowerMeter->flags &= ~1u;
			if ((local_27a != 0) || (local_258 != 0)) goto LAB_800907c4;
		} else {
			loungealotPowerMeter->points[1].x = local_278 + 0x27;
			loungealotPowerMeter->points[3].x = local_278 + 0x27;
			loungealotPowerMeter->colour = (uint16)(((int)((uint)local_278 - 1) >> 1) << 5) |
										   (uint16)(((int)(0x40 - (uint)local_278) >> 1) << 10);
			loungealotPowerMeter->flags |= 1;

			LAB_800907c4:
			if (!bVar4) {
				_vm->_fontManager->drawTextDialogBox(4, 0x14, 0xd, 0x16);
				_vm->_fontManager->drawTextDialogBox(4, 0x10, 0xd, 0x12);
				flickerPortrait->_priorityLayer = 6;
				loungealotPortrait->_priorityLayer = 6;
				bVar4 = true;
			}
		}
		if ((((local_278 == 0) && (local_27a == 0)) && (local_258 == 0)) && (bVar4)) {
			_vm->_fontManager->clearTextDialog(4, 0x14, 0xd, 0x16);
			_vm->_fontManager->clearTextDialog(4, 0x10, 0xd, 0x12);
			flickerPortrait->_priorityLayer = 0;
			loungealotPortrait->_priorityLayer = 0;
			bVar4 = false;
		}

		//DisableVSyncEvent();
		loungealotThumb->_x_pos = loungealotLeftUpperArm->_x_pos - loungealotLeftUpperArm->_frame->field_e;
		flickerArm->_x_pos = loungealotThumb->_x_pos;
		sVar2 = flickerArm->_x_pos;
		loungealotThumb->_y_pos = loungealotLeftUpperArm->_y_pos - loungealotLeftUpperArm->_frame->field_10;
		flickerArm->_y_pos = loungealotThumb->_y_pos;
		sVar3 = flickerArm->_y_pos;
		uVar13->_x_pos = sVar2 - flickerArm->_frame->field_e;
		uVar12->_x_pos = uVar13->_x_pos;
		uVar13->_y_pos = sVar3 - flickerArm->_frame->field_10;
		uVar12->_y_pos = uVar13->_y_pos;
//		EnableVSyncEvent();
		local_282 = _dat_80093cc0;
		local_286 = _dat_80093cbc;
		local_284 = _dat_80093cb8;
		local_288 = _dat_80093cb4;
		if (_dat_80093c94 != 1) {
			if (_dat_80093c94 < 2) {
				if (_dat_80093c94 == 0) {
					if (((local_264 < 300) || (_dat_80093ca0 != 0)) || (_dat_80093cbc == 2)) {
						if ((local_260 != 0) && (local_260 = local_260 + -1, local_260 == 0)) {
							if (local_262 != 0) {
								local_262 = local_262 - 1;
							}
							local_260 = 300;
						}
					} else {
						local_264 = 0;
						local_260 = 300;
//						playSoundFromTxtIndex(textIdTbl[local_262]);
						loungealotHeadActor->updateSequence((uint)local_2b0[(uint)local_262 * 2]);
						uVar18 = (uint)local_262;
						local_262 = local_262 + 1;
						fun_80093aec_dialog(textIdTbl[uVar18], 0x14, 1);
						if (local_262 == 3) {
							while (((_dat_80093cb4 != 2 || (_dat_80093cbc != 0)) ||
									((_dat_80093cb8 != 2 || (_dat_80093cc0 != 0))))) {
								_vm->waitForFrames(1);
								if (flickerArm->isFlagSet(ACTOR_FLAG_4)) {
									if (_dat_80093cbc != 0) {
										_dat_80093cbc = _dat_80093cbc - 1;
									}
									if (2 < _dat_80093cb4) {
										_dat_80093cb4 = _dat_80093cb4 - 1;
									}
									if (_dat_80093cb4 < 2) {
										_dat_80093cb4 = _dat_80093cb4 + 1;
									}
								}
								if ((uVar12->_flags & 4) != 0) {
									if (_dat_80093cc0 != 0) {
										_dat_80093cc0 = _dat_80093cc0 - 1;
									}
									if (2 < _dat_80093cb8) {
										_dat_80093cb8 = _dat_80093cb8 - 1;
									}
									if (_dat_80093cb8 < 2) {
										_dat_80093cb8 = _dat_80093cb8 + 1;
									}
								}
								if (flickerArm->_sequenceID != actorSequenceIdTbl[(uint)_dat_80093cb4 * 3 + (uint)_dat_80093cbc]) {
									flickerArm->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb4 * 3 + (uint)_dat_80093cbc]);
									loungealotThumb->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb4 * 3 + (uint)_dat_80093cbc]);
								}
								if (uVar12->_sequenceID !=
									actorSequenceIdTbl[(uint)_dat_80093cb8 * 3 + (uint)_dat_80093cc0]) {
									uVar12->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb8 * 3 + (uint)_dat_80093cc0]);
									uVar13->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb8 * 3 + (uint)_dat_80093cc0]);
								}
							}
							_vm->waitForFrames(2 * 0x3c);
//							DisableVSyncEvent();
							memset(paletteData, 0, 0x200);
							loungealotBigPunchActor->_flags = loungealotBigPunchActor->_flags & 0xfbff;
							loungealotHeadActor->setFlag(ACTOR_FLAG_400);
							loungealotLeftUpperArm->setFlag(ACTOR_FLAG_400);
							loungealotRightArm->setFlag(ACTOR_FLAG_400);
							loungealotLegActor->_flags = loungealotLegActor->_flags | 0x400;
							flickerArm->setFlag(ACTOR_FLAG_400);
							loungealotThumb->setFlag(ACTOR_FLAG_400);
							uVar12->setFlag(ACTOR_FLAG_400);
							uVar13->setFlag(ACTOR_FLAG_400);
//							EnableVSyncEvent();
							_vm->waitForFrames(6);
							loungealotBigPunchActor->updateSequence(1);
							loungealotBigPunchActor->waitUntilFlag4IsSet();
							_vm->waitForFrames(1);
							_vm->_screen->loadPalette(0, paletteData);
							_vm->_screen->loadPalette(1, paletteData);
							_vm->_talk->FUN_8001a7c4_clearDialogBoxMaybe();
							//punched in the face.
							break;
						}
					}
					if (local_26c != 0) {
						local_26c = local_26c + -1;
					}
					if (local_278 != 0) {
						local_278 = local_278 - 1;
					}
					if (local_27a != 0) {
						local_27a = local_27a - 1;
					}
					local_268 = 2;
					if (_vm->isLeftKeyPressed()) {
						local_268 = 1;
					}
					if (fun_80093520()) {
						local_268 = local_268 - 1;
					}
					if (_vm->isRightKeyPressed()) {
						local_268 = local_268 + 1;
					}
					if (fun_80093248()) {
						local_268 = local_268 + 1;
					}
					if (loungealotLeftUpperArm->_sequenceID != local_268) {
						loungealotLeftUpperArm->updateSequence(local_268);
					}
					if ((flickerArm->_flags & 4) != 0) {
						if (_vm->isRightKeyPressed() && (local_288 != 0)) {
							local_288 = local_288 - 1;
						}
						if (_vm->isLeftKeyPressed() && (local_288 < 4)) {
							local_288 = local_288 + 1;
						}
						if ((!_vm->isLeftKeyPressed() && !_vm->isRightKeyPressed()) && (local_288 != 2)) {
							if (local_288 < 2) {
								local_288 = local_288 + 1;
							} else {
								local_288 = local_288 - 1;
							}
						}
						if (!_vm->isActionButtonPressed() || (local_26c != 0)) {
							if (local_286 != 0) {
								local_286 = local_286 - 1;
							}
						} else {
							if (local_286 < 2) {
								local_286 = local_286 + 1;
							}
						}
					}
					if (local_286 == 2) {
						if (local_256 < 0x14) {
							local_256 = local_256 + 1;
						} else {
							local_256 = 0;
						}
					}
					if (local_256 < 0x14) {
						local_264 = local_264 + 1;
					} else {
						local_264 = 0;
					}
					if ((uVar12->_flags & 4) != 0) {
						if (fun_80093248() && (local_284 != 0)) {
							local_284 = local_284 - 1;
						}
						if (fun_80093520() && (local_284 < 4)) {
							local_284 = local_284 + 1;
						}
						if ((!fun_80093520() && !fun_80093248()) && (local_284 != 2)) {
							if (local_284 < 2) {
								local_284 = local_284 + 1;
							} else {
								local_284 = local_284 - 1;
							}
						}
						if (!fun_80093800() || (local_26c != 0)) {
							if (local_282 != 0) {
								local_282 = local_282 - 1;
							}
						} else {
							if (local_282 < 2) {
								local_282 = local_282 + 1;
							}
						}
					}
					if (((local_286 == 2) && (local_282 == 2)) && (local_288 == local_284)) {
						if ((_dat_80093cbc == 2) && (_dat_80093cc0 != 2)) {
							local_258 = 2;
						} else {
							if ((_dat_80093cbc == 2) || (_dat_80093cc0 != 2)) {
								local_288 = _dat_80093cb4;
								local_286 = _dat_80093cbc;
								local_284 = _dat_80093cb8;
								local_282 = _dat_80093cc0;
							} else {
								local_258 = 1;
							}
						}
					}
					if (local_258 == 0) {
						if ((local_286 != _dat_80093cbc) || (local_288 != _dat_80093cb4)) {
							_dat_80093cb4 = local_288;
							_dat_80093cbc = local_286;
							flickerArm->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)local_288 * 3 + (uint)local_286]);
							loungealotThumb->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb4 * 3 + (uint)_dat_80093cbc]);
						}
						if ((local_282 != _dat_80093cc0) || (local_284 != _dat_80093cb8)) {
							_dat_80093cb8 = local_284;
							_dat_80093cc0 = local_282;
							uVar12->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)local_284 * 3 + (uint)local_282]);
							uVar13->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb8 * 3 + (uint)_dat_80093cc0]);
						}
					} else {
						if (local_258 == 1) {
							loungealotHeadActor->updateSequence(1);
							loungealotLegActor->updateSequence(1);
							if (local_288 == 2) {
								local_28 = 1;
							} else {
								if (local_288 < 3) {
									if (local_288 == 0) {
										local_28 = 0;
									} else {
										LAB_800926a4:
										local_28 = 2;
									}
								} else {
									if (local_288 != 4) goto LAB_800926a4;
									local_28 = 2;
								}
							}
						} else {
							loungealotHeadActor->updateSequence(3);
							loungealotRightArm->updateSequence(1);
							if (local_284 == 2) {
								local_28 = 4;
							} else {
								if (local_284 < 3) {
									if (local_284 == 0) {
										local_28 = 3;
									} else {
										LAB_80092754:
										local_28 = 4;
									}
								} else {
									if (local_284 != 4) goto LAB_80092754;
									local_28 = 5;
								}
							}
						}
						uVar12->_flags = uVar12->_flags | 0x400;
						uVar13->_flags = uVar13->_flags | 0x400;
						flickerArm->updateSequence((uint)local_28 + 0xf);
						loungealotThumb->updateSequence((uint)local_28 + 0xf);
						_dat_80093c94 = 1;
						_dat_80093c90 = 0x1e;
					}
				}
			} else {
				if (_dat_80093c94 == 2) { // Flicker loses by being pinned
					uVar12->_flags = uVar12->_flags | 0x1000;
					uVar13->_flags = uVar13->_flags | 0x1000;
					flickerArm->_flags = flickerArm->_flags | 0x1000;
					loungealotThumb->_flags = loungealotThumb->_flags | 0x1000;
					_vm->_screen->setScreenShakeOffset(0, 0);
					if (local_258 == 1) {
//						playSoundFromTxtIndex(DAT_80063ad0);
						loungealotHeadActor->updateSequence(9);
						fun_80093aec_dialog(_vm->getDialogTextId(0x46BC), 0x14, 1);
						do {
							_vm->waitForFrames(1);
						} while (_vm->isFlagSet(ENGINE_FLAG_8000));
						loungealotHeadActor->updateSequence(2);
						loungealotRightArm->updateSequence(3);
					} else {
//						playSoundFromTxtIndex(DAT_80063ad4);
						loungealotHeadActor->updateSequence(10);
						fun_80093aec_dialog(_vm->getDialogTextId(0x4718), 0x14, 1);
						loungealotRightArm->updateSequence(2);
						do {
							_vm->waitForFrames(1);
						} while (_vm->isFlagSet(ENGINE_FLAG_8000));
						loungealotHeadActor->updateSequence(4);
					}
					_vm->waitForFrames(2 * 0x3c);
					break;
				}
			}
			continue;
		}
		if (local_272 == 0) {
			local_272 = 1;
			if (local_27a != 0) {
				local_27a = local_27a - 1;
			}
			if (local_278 != 0) {
				local_278 = local_278 - 1;
			}
		} else {
			local_272 = local_272 + -1;
		}

		if (_vm->isR1ButtonPressed()) { //TODO || _vm->isR2ButtonPressed()) {
			local_27a = local_27a + 4;
		}
		if (_vm->isL1ButtonPressed()) { //TODO || _vm->isL2ButtonPressed()) {
			local_27a = local_27a + 4;
		}
		if (fun_80093990()) {
			if (param_1 == 1) {
				local_278 = local_278 + 4;
			} else {
				local_278 = local_278 + 6;
			}
		}
		if (fun_80093a30()) {
			if (param_1 == 1) {
				local_278 = local_278 + 4;
			} else {
				local_278 = local_278 + 6;
			}
		}
		if ((0x3f < local_278) || (0x3f < local_27a)) {
			loungealotHeadActor->setFlag(ACTOR_FLAG_1000);
			loungealotLegActor->setFlag(ACTOR_FLAG_1000);
			loungealotRightArm->setFlag(ACTOR_FLAG_1000);
			if (0x40 < local_27a) {
				local_27a = 0x40;
			}
			if (0x40 < local_278) {
				local_278 = 0x40;
			}
			if (((local_278 == local_27a) || ((local_258 == 2 && (local_278 < local_27a)))) ||
				((local_258 == 1 && (local_27a < local_278)))) {
				if (local_258 == 1) {
//					playSoundFromTxtIndex(DAT_80063ad8);
					loungealotHeadActor->updateSequence(0xb);
					fun_80093aec_dialog(_vm->getDialogTextId(0x475E), 0x14, 1);
				} else {
//					playSoundFromTxtIndex(DAT_80063adc);
					loungealotHeadActor->updateSequence(0xc);
					fun_80093aec_dialog(_vm->getDialogTextId(0x4774), 0x14, 1);
				}
				local_258 = 0;
				_dat_80093cb4 = 2;
				_dat_80093cbc = 1;
				_dat_80093cb8 = 0;
				_dat_80093cc0 = 1;
				uVar12->_flags = uVar12->_flags & 0xfbff;
				uVar13->_flags = uVar13->_flags & 0xfbff;
				flickerArm->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb4 * 3 + (uint)_dat_80093cbc]);
				loungealotThumb->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb4 * 3 + (uint)_dat_80093cbc]);
				uVar12->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb8 * 3 + (uint)_dat_80093cc0]);
				uVar13->updateSequence((uint)(uint16)actorSequenceIdTbl[(uint)_dat_80093cb8 * 3 + (uint)_dat_80093cc0]);
				_dat_80093c94 = 0;
				local_26c = 0x1e;
			} else {
				_dat_80093c94 = 2;
			}
		}
	} while (true);

	_vm->fadeToBlack();
	_vm->_fontManager->clearText();
	_vm->_sound->resumeMusic();
//	DisableVSyncEvent();
	_vm->_dragonINIResource->getRecord(0)->x = 0x91;
	_vm->_dragonINIResource->getRecord(0)->y = 0x9b;
	_vm->_dragonINIResource->getRecord(0x123)->x = 0xc3;
	_vm->_dragonINIResource->getRecord(0x123)->y = 0x9b;
	_vm->_screen->loadPalette(1, _vm->_cursor->getPalette());
	_vm->setupPalette1();
	_vm->_screen->clearAllFlatQuads();
	_vm->_dragonINIResource->setFlickerRecord(flicker);
	_vm->_inventory->setState(originalInventoryType);
	flicker->objectState = local_258 + -1;
	if (flicker->objectState == 0) {
		_vm->setVar(0xb, 1);
		flicker->actorResourceId = 0xd2; //TODO is this correct?
		_vm->_actorManager->loadActor(0xd2, flicker->actor->_actorID);
		_vm->_dragonINIResource->getRecord(0x120)->sceneId = 0x17;
	}
	loungealotHeadActor->clearFlag(ACTOR_FLAG_40);
	loungealotLeftUpperArm->clearFlag(ACTOR_FLAG_40);
	loungealotLegActor->clearFlag(ACTOR_FLAG_40);
	loungealotRightArm->clearFlag(ACTOR_FLAG_40);
	flickerArm->clearFlag(ACTOR_FLAG_40);
	loungealotThumb->clearFlag(ACTOR_FLAG_40);
	uVar12->clearFlag(ACTOR_FLAG_40);
	uVar13->clearFlag(ACTOR_FLAG_40);
//	EnableVSyncEvent();
	if (param_3 == 0) {
		_vm->fadeToBlack();
		_vm->_screen->clearAllFlatQuads();
		flickerPortrait->clearFlag(ACTOR_FLAG_40);
		loungealotPortrait->clearFlag(ACTOR_FLAG_40);
		_vm->reset_screen_maybe();
	} else {
		_vm->reset_screen_maybe();
		_vm->_scene->setSceneId(0x17);
		flicker->sceneId = 0x17;
		_vm->_scene->loadSceneData((uint)(0x17 | 0x8000), 0);
		_vm->setAllFlags((origEngineFlags & 0xfefdffff) | (_vm->getAllFlags() & 0x1000000) | 0x40);
		_vm->fadeFromBlack();
	}
	_vm->_fontManager->clearText();
}

void Minigame2::fun_80093aec_dialog(uint32 textId, int16 x, int16 y) {
	uint16 auStack4024 [2000];
	_vm->_talk->FUN_8001a7c4_clearDialogBoxMaybe();
	_vm->_talk->loadText(textId, auStack4024, 2000);
	_vm->_talk->displayDialogAroundPoint(auStack4024, x, y, 0, 0, textId);
	_dat_80093c9c = 1;
}

bool Minigame2::fun_80093520() {
	bool uVar2;

	if (_dat_80093ca0 == 0) {
		if (!_dat_80093c70) {
			if (!_dat_80093c72 || (3 < _dat_80093cb8)) {
				_dat_80093c72 = false;
				_dat_80093c74 = _dat_80093c74 + 1;
				if (_dat_80093c74 < 2) {
					_dat_80093c70 = true;
					_dat_80093cac = 0;
					uVar2 = false;
				} else {
					if (_dat_80093ca4 == 0) {
						_dat_80093cac = 0;
						if (_dat_80093cb8 == _dat_80093cb4) {
							if (_dat_80093cbc < _dat_80093cc0) {
								_dat_80093cac = (uint16)(_dat_80093cb8 < 4);
							} else {
								if (!_dat_80093cb0 && _vm->getRand(8) < 3) {
									if (_vm->getRand(8) < 3) {
										_dat_80093c72 = 1;
										_dat_80093c74 = 0;
										_dat_80093cac = 1;
									} else {
										_dat_80093ca4 = _dat_80093cc8 + _vm->getRand(10);
										_dat_80093cac = 1;
									}
									return true;
								}
								if (_dat_80093cb4 == 4) {
									_dat_80093cac = 1;
								}
							}
						} else {
							if ((_dat_80093cb8 < _dat_80093cb4) && (_dat_80093cb4 != 2)) {
								_dat_80093cac = 1;
							}
						}
					}
					uVar2 = (uint)_dat_80093cac;
				}
			} else {
				_dat_80093cac = 1;
				uVar2 = true;
			}
		} else {
			_dat_80093cac = 0;
			uVar2 = false;
		}
	} else {
		uVar2 = _vm->isLeftKeyPressed();
	}
	return uVar2;
}

bool Minigame2::fun_80093248() {
	bool uVar2;

	if (_dat_80093ca0 == 0) {
		if (!_dat_80093c72) {
			if (!_dat_80093c70 || (_dat_80093cb8 == 0)) {
				_dat_80093c70 = false;
				_dat_80093c74 = _dat_80093c74 + 1;
				if (_dat_80093c74 < 2) {
					_dat_80093c72 = true;
					_dat_80093cb0 = false;
					uVar2 = false;
				} else {
					if (_dat_80093ca4 == 0) {
						_dat_80093cb0 = false;
						if (_dat_80093cb8 == _dat_80093cb4) {
							if (_dat_80093cbc < _dat_80093cc0) {
								_dat_80093cb0 = (bool)(_dat_80093cb8 != 0);
							} else {
								if ((_dat_80093cac == 0) &&  _vm->getRand(8) < 3) {
									if (_vm->getRand(8) < 3) {
										_dat_80093c70 = true;
										_dat_80093c74 = 0;
										_dat_80093cb0 = true;
									} else {
										_dat_80093ca4 = _dat_80093cc8 + _vm->getRand(10);
										_dat_80093cb0 = true;
									}

									return true;
								}
								if (_dat_80093cb4 == 0) {
									_dat_80093cb0 = true;
								}
							}
						} else {
							if ((_dat_80093cb4 < _dat_80093cb8) && (_dat_80093cb4 != 2)) {
								_dat_80093cb0 = true;
							}
						}
					}
					uVar2 = _dat_80093cb0;
				}
			} else {
				_dat_80093cb0 = true;
				uVar2 = true;
			}
		} else {
			_dat_80093cb0 = false;
			uVar2 = false;
		}
	} else {
		uVar2 = _vm->isRightKeyPressed();
	}
	return uVar2;
}

bool Minigame2::fun_80093800() {
	bool uVar2;

	if (_dat_80093ca0 == 0) {
		if (_dat_80093ca4 == 0) {
			_dat_80093ca8 = false;
			if ((_dat_80093cb8 == _dat_80093cb4) && (_dat_80093cc0 < _dat_80093cbc)) {
				_dat_80093ca8 = true;
			} else {
				if (_vm->getRand(8) < 3) {
					_dat_80093ca4 = _vm->getRand(10);
					_dat_80093ca4 = _dat_80093cc8 + _dat_80093ca4;
					_dat_80093ca8 = true;
				}
			}
		}
		uVar2 = _dat_80093ca8;
	} else {
		uVar2 = false;
		if (_vm->isSquareButtonPressed() || _vm->isCrossButtonPressed() ||
			 _vm->isCircleButtonPressed() ||
			_vm->isTriangleButtonPressed()) {
			uVar2 = true;
		}
	}
	return uVar2;
}

bool Minigame2::fun_80093a30() {
	bool uVar1;

	if (_dat_80093ca0 == 0) {
		uVar1 = (uint) _dat_80093c90 == (uint) _dat_80093cc8 / 3;
	} else {
		uVar1 = _vm->isL1ButtonPressed(); // TODO || _vm->isL2ButtonPressed();
	}
	return uVar1;
}

bool Minigame2::fun_80093990() {
	bool uVar1;

	if (_dat_80093ca0 == 0) {
		uVar1 = _dat_80093c90 == 0;
	} else {
		uVar1 = _vm->isR1ButtonPressed(); // TODO || _vm->isR2ButtonPressed();
	}
	return uVar1;
}

} // End of namespace Dragons
