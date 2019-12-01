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
#include "minigame2.h"
#include "actor.h"
#include "dragons/actorresource.h"
#include "dragons.h"
#include "dragons/dragonini.h"
#include "dragons/talk.h"
#include "dragons/inventory.h"
#include "dragons/scene.h"
#include "dragons/screen.h"
#include "dragons/cursor.h"
#include "dragons/sound.h"

namespace Dragons {

Minigame2::Minigame2(DragonsEngine *vm) : _vm(vm) {}

static const uint16 unkArray[5] = {
		0xC, 0xA, 0x8, 0x6, 0x4
};

void Minigame2::run(int16 param_1, uint16 param_2, int16 param_3) {
	uint16_t uVar1;
	short sVar2;
	short sVar3;
	bool shouldExit;
	bool bVar4;
	DragonINI *flicker;
	uint32 origEngineFlags;
	Actor *uVar5;
	Actor *uVar6;
	Actor *uVar7;
	Actor *uVar8;
	Actor *uVar9;
	Actor *uVar10;
	Actor *uVar11;
	Actor *uVar12;
	Actor *uVar13;
	Actor *uVar14;
	Actor *uVar15;
	ushort uVar16;
	ushort uVar17;
	uint uVar18;
	uint uVar19;
	int iVar20;
	bool shouldShakeScreen;
	byte *actorFrameData;
	ushort local_2e6;
	int16_t actorSequenceIdTbl [15];
	uint32_t textIdTbl [4];
	uint16_t local_2b0 [8];
	ushort local_288;
	ushort local_286;
	ushort local_284;
	ushort local_282;
	ushort local_27a;
	ushort local_278;
	short local_272;
	short local_26c;
	ushort local_268;
	ushort local_264;
	ushort local_262;
	short local_260;
	short local_258;
	short local_256;
	int16_t screenShakeTbl [10];
	ushort screenShakeCounter;
	uint8 paletteData [0x200];
	ushort local_28;
	int16 originalInventoryType;

	origEngineFlags = _vm->getAllFlags();
	originalInventoryType = _vm->_inventory->getType();
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

	textIdTbl[0] = 0x4500;
	textIdTbl[1] = 0x454A;
	textIdTbl[2] = 0x4576;

	bVar4 = false;
	shouldExit = false;
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
	if (4 < param_2) {
		local_2e6 = 4;
	}
	DAT_80093ca4 = unkArray[local_2e6];
	DAT_80093c90 = unkArray[local_2e6];
	DAT_80093c94 = 0;
	_vm->_inventory->setType(0);
	DAT_80093cb4 = 2;
	DAT_80093cbc = 0;
	DAT_80093cb8 = 2;
	DAT_80093cc0 = 0;
	DAT_80093cac = 0;
	DAT_80093cb0 = 0;
	DAT_80093c9c = 0;
	_vm->_talk->DAT_8008e874_dialogBox_y2 = 0;
	_vm->_talk->DAT_8008e844_dialogBox_y1 = 0;
	_vm->_talk->DAT_8008e848_dialogBox_x2 = 0;
	_vm->_talk->DAT_8008e7e8_dialogBox_x1 = 0;
	DAT_80093c98 = DAT_80093c90;
	DAT_80093ca0 = param_1;
	DAT_80093cc4 = DAT_80093ca4;
	DAT_80093cc8 = DAT_80093ca4;
	//fade_related_calls_with_1f();
	_vm->reset_screen_maybe();
	_vm->_inventory->setType(0);
	flicker->sceneId = 0;
	_vm->_dragonINIResource->setFlickerRecord(NULL);
	_vm->setFlags(ENGINE_FLAG_800);
	_vm->_scene->setSceneId(5);
	_vm->_scene->loadSceneData(5 | 0x8000,0);
	//DisableVSyncEvent();
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_10);
	_vm->clearFlags(ENGINE_FLAG_20);
	_vm->clearFlags(ENGINE_FLAG_80);

	memcpy(paletteData, _vm->_scene->getPalette() + 0x180, 0x80);
	memcpy(paletteData + 0x80, _vm->_scene->getPalette() + 0x180, 0x80);
	_vm->_screen->loadPalette(1, paletteData);
	_vm->_screen->updatePaletteTransparency(1,0x40,0x7f,true);

	uVar5 = _vm->_actorManager->loadActor(0x11,0,0,0,6);
	uVar6 = _vm->_actorManager->loadActor(0xd,0,0x7d,199,4);
	uVar7 = _vm->_actorManager->loadActor(0xb,2,0x7d,199,4);
	uVar8 = _vm->_actorManager->loadActor(0xf,0,0x7d,199,4);
	uVar9 = _vm->_actorManager->loadActor(0x10,0,0x7d,199,4);
	actorFrameData = uVar7->frame->frameDataOffset;
	uVar10 = _vm->_actorManager->loadActor(9,(uint)(ushort)actorSequenceIdTbl[(uint)DAT_80093cb4 * 3 + (uint)DAT_80093cbc],
								uVar7->x_pos - READ_LE_INT16(actorFrameData + 0xe),
								uVar7->y_pos - READ_LE_INT16(actorFrameData + 0x10),4);
	uVar11 = _vm->_actorManager->loadActor(0x12,(uint)(ushort)actorSequenceIdTbl[(uint)DAT_80093cb4 * 3 + (uint)DAT_80093cbc],
								uVar7->x_pos - READ_LE_INT16(actorFrameData + 0xe),
								uVar7->y_pos - READ_LE_INT16(actorFrameData + 0x10),4);
	actorFrameData = uVar10->frame->frameDataOffset;
	uVar12 = _vm->_actorManager->loadActor(10,(uint)(ushort)actorSequenceIdTbl[(uint)DAT_80093cb8 * 3 + (uint)DAT_80093cc0],
			uVar10->x_pos - READ_LE_INT16(actorFrameData + 0xe),
			uVar10->y_pos - READ_LE_INT16(actorFrameData + 0x10),4);
	uVar13 = _vm->_actorManager->loadActor(0x13,(uint)(ushort)actorSequenceIdTbl[(uint)DAT_80093cb8 * 3 + (uint)DAT_80093cc0],
			uVar10->x_pos - READ_LE_INT16(actorFrameData + 0xe),
			uVar10->y_pos - READ_LE_INT16(actorFrameData + 0x10),4);
	uVar14 = _vm->_actorManager->loadActor(0x27,0,0x10,0xac,4);
	uVar15 = _vm->_actorManager->loadActor(0x27,1,0x10,0x8c,4);
	uVar5->setFlag(ACTOR_FLAG_100);
	uVar6->setFlag(ACTOR_FLAG_100);
	uVar7->setFlag(ACTOR_FLAG_100);
	uVar8->setFlag(ACTOR_FLAG_100);
	uVar9->setFlag(ACTOR_FLAG_100);
	uVar10->setFlag(ACTOR_FLAG_100);
	uVar11->setFlag(ACTOR_FLAG_100);
	uVar12->setFlag(ACTOR_FLAG_100);
	uVar13->setFlag(ACTOR_FLAG_100);
	uVar14->setFlag(ACTOR_FLAG_100);
	uVar15->setFlag(ACTOR_FLAG_100);

	uVar5->priorityLayer = 6;
	uVar10->priorityLayer = 5;
	uVar12->priorityLayer = 5;
	uVar11->priorityLayer = 4;
	uVar13->priorityLayer = 3;
	uVar9->priorityLayer = 3;
	uVar7->priorityLayer = 2;
	uVar6->priorityLayer = 2;
	uVar8->priorityLayer = 1;
	uVar14->priorityLayer = 0;
	uVar15->priorityLayer = 0;
	//TODO
//	uVar16 = AddFlatShadedQuad(0x28,0xa8,0x67,0xa8,0x67,0xaf,0x28,0xaf,0x1f,7,0);
//	uVar17 = AddFlatShadedQuad(0x28,0x88,0x67,0x88,0x67,0x8f,0x28,0x8f,0x3e0,7,0);
//	(&DAT_80083178)[(uint)uVar16 * 0xb] = (&DAT_80083178)[(uint)uVar16 * 0xb] & 0xfffe;
//	(&DAT_80083178)[(uint)uVar17 * 0xb] = (&DAT_80083178)[(uint)uVar17 * 0xb] & 0xfffe;
//	EnableVSyncEvent();
	uVar6->setFlag(ACTOR_FLAG_1);
	uVar7->setFlag(ACTOR_FLAG_1);
	uVar8->setFlag(ACTOR_FLAG_1);
	uVar9->setFlag(ACTOR_FLAG_1);
	uVar10->setFlag(ACTOR_FLAG_1);
	uVar11->setFlag(ACTOR_FLAG_1);
	uVar12->setFlag(ACTOR_FLAG_1);
	uVar13->setFlag(ACTOR_FLAG_1);
	uVar10->waitUntilFlag8And4AreSet();
	uVar12->waitUntilFlag8And4AreSet();
	uVar11->waitUntilFlag8And4AreSet();
	uVar13->waitUntilFlag8And4AreSet();
	uVar7->waitUntilFlag8And4AreSet();
	uVar9->waitUntilFlag8And4AreSet();
	uVar6->waitUntilFlag8And4AreSet();
	uVar8->waitUntilFlag8And4AreSet();

	uVar5->setFlag(ACTOR_FLAG_400);
	actorFrameData = uVar7->frame->frameDataOffset;
	uVar10->x_pos = uVar7->x_pos - READ_LE_INT16(actorFrameData + 0xe);
	uVar10->y_pos = uVar7->y_pos - READ_LE_INT16(actorFrameData + 0x10);
	uVar11->x_pos = uVar7->x_pos - READ_LE_INT16(actorFrameData + 0xe);
	uVar11->y_pos = uVar7->y_pos - READ_LE_INT16(actorFrameData + 0x10);
	actorFrameData = uVar10->frame->frameDataOffset;
	uVar12->x_pos = uVar7->x_pos - READ_LE_INT16(actorFrameData + 0xe);
	uVar12->y_pos = uVar7->y_pos - READ_LE_INT16(actorFrameData + 0x10);
	uVar13->x_pos = uVar7->x_pos - READ_LE_INT16(actorFrameData + 0xe);
	uVar13->y_pos = uVar7->y_pos - READ_LE_INT16(actorFrameData + 0x10);
	_vm->waitForFrames(200);
	// call_fade_related_1f();

	LAB_80090188:
	do {
		if (shouldExit) {
//			fade_related_calls_with_1f();
			_vm->_sound->PauseCDMusic();
//			DisableVSyncEvent();
			_vm->_dragonINIResource->getRecord(0)->x = 0x91;
			_vm->_dragonINIResource->getRecord(0)->y = 0x9b;
			_vm->_dragonINIResource->getRecord(0x123)->x = 0xc3;
			_vm->_dragonINIResource->getRecord(0x123)->y = 0x9b;
			_vm->_screen->loadPalette(1, _vm->_cursor->getPalette());
			_vm->setupPalette1();
			//TODO FUN_80035e74((uint)uVar17);
			//TODO FUN_80035e74((uint)uVar16);
			_vm->_dragonINIResource->setFlickerRecord(flicker);
			_vm->_inventory->setType(originalInventoryType);
			flicker->field_12 = local_258 + -1;
			if (flicker->field_12 == 0) {
				_vm->setVar(0xb, 1);
				flicker->actorResourceId = 0xd2; //TODO is this correct?
				_vm->_dragonINIResource->getRecord(0x120)->sceneId = 0x17;
			}
			uVar6->clearFlag(ACTOR_FLAG_40);
			uVar7->clearFlag(ACTOR_FLAG_40);
			uVar8->clearFlag(ACTOR_FLAG_40);
			uVar9->clearFlag(ACTOR_FLAG_40);
			uVar10->clearFlag(ACTOR_FLAG_40);
			uVar11->clearFlag(ACTOR_FLAG_40);
			uVar12->clearFlag(ACTOR_FLAG_40);
			uVar13->clearFlag(ACTOR_FLAG_40);
//			EnableVSyncEvent();
			if (param_3 == 0) {
//				fade_related_calls_with_1f();
//				TODO ClearFlatShadedQuads(0);
				uVar14->clearFlag(ACTOR_FLAG_40);
				uVar15->clearFlag(ACTOR_FLAG_40);
				_vm->reset_screen_maybe();
			}
			else {
				_vm->reset_screen_maybe();
				_vm->_scene->setSceneId(0x17);
				flicker->sceneId = 0x17;
				_vm->_scene->loadSceneData((uint)(0x17 | 0x8000),0);
				_vm->setAllFlags(origEngineFlags & 0xfefdffff | _vm->getAllFlags() & 0x1000000 | 0x40);
//				call_fade_related_1f();
			}
			return;
		}
		_vm->waitForFrames(1);
		shouldExit = true;
		/*
		if ((DAT_80093c9c != 0) && ((engine_flags_maybe & 0x8000) == 0)) {
			actor_update_sequenceID((uint)uVar6,0);
			FUN_8001a7c4((uint)DAT_8008e7e8,(uint)DAT_8008e844,(uint)DAT_8008e848,(uint)DAT_8008e874);
			DAT_80093c9c = 0;
		}
		if (DAT_80093c94 != 0) {
			local_264 = 0;
		}
		if (*(short *)&Actor[uVar8].field_0x7a == 1) {
			shouldShakeScreen = true;
			*(undefined2 *)&Actor[uVar8].field_0x7a = 0;
			screenShakeCounter = 0;
		}
		if (shouldShakeScreen) {
			uVar18 = (uint)screenShakeCounter;
			screenShakeCounter = screenShakeCounter + 1;
			screenShakeOffset = screenShakeTbl[uVar18];
			if (screenShakeOffset == 0) {
				shouldShakeScreen = false;
			}
		}
		if (DAT_80093ca4 == 0) {
			DAT_80093ca4 = DAT_80093cc8;
		}
		else {
			DAT_80093ca4 = DAT_80093ca4 - 1;
		}
		if (DAT_80093c90 == 0) {
			DAT_80093c90 = DAT_80093c98;
		}
		else {
			DAT_80093c90 = DAT_80093c90 - 1;
		}
		if (DAT_80093cc4 == 0) {
			DAT_80093cc4 = DAT_80093cc8;
		}
		else {
			DAT_80093cc4 = DAT_80093cc4 - 1;
		}
		if (local_27a == 0) {
			(&DAT_80083178)[(uint)uVar16 * 0xb] = (&DAT_80083178)[(uint)uVar16 * 0xb] & 0xfffe;
		}
		else {
			(&DAT_8008316c)[(uint)uVar16 * 0xb] = local_27a + 0x27;
			(&DAT_80083168)[(uint)uVar16 * 0xb] = local_27a + 0x27;
			(&DAT_80083176)[(uint)uVar16 * 0xb] =
					(ushort)(((int)((uint)local_27a - 1) >> 1) << 5) |
					(ushort)(((int)(0x40 - (uint)local_27a) >> 1) << 10);
			(&DAT_80083178)[(uint)uVar16 * 0xb] = (&DAT_80083178)[(uint)uVar16 * 0xb] | 1;
		}
		if (local_278 == 0) {
			(&DAT_80083178)[(uint)uVar17 * 0xb] = (&DAT_80083178)[(uint)uVar17 * 0xb] & 0xfffe;
			if ((local_27a != 0) || (local_258 != 0)) goto LAB_800907c4;
		}
		else {
			(&DAT_8008316c)[(uint)uVar17 * 0xb] = local_278 + 0x27;
			(&DAT_80083168)[(uint)uVar17 * 0xb] = local_278 + 0x27;
			(&DAT_80083176)[(uint)uVar17 * 0xb] =
					(ushort)(((int)((uint)local_278 - 1) >> 1) << 5) |
					(ushort)(((int)(0x40 - (uint)local_278) >> 1) << 10);
			(&DAT_80083178)[(uint)uVar17 * 0xb] = (&DAT_80083178)[(uint)uVar17 * 0xb] | 1;
			LAB_800907c4:
			if (!bVar4) {
				FUN_8001a4e4_draw_dialogbox(4,0x14,0xd,0x16,1);
				FUN_8001a4e4_draw_dialogbox(4,0x10,0xd,0x12,1);
				uVar14->priorityLayer = 6;
				uVar15->priorityLayer = 6;
				bVar4 = true;
			}
		}
		if ((((local_278 == 0) && (local_27a == 0)) && (local_258 == 0)) && (bVar4)) {
			FUN_8001a7c4(4,0x14,0xd,0x16);
			FUN_8001a7c4(4,0x10,0xd,0x12);
			uVar14->priorityLayer = 0;
			uVar15->priorityLayer = 0;
			bVar4 = false;
		}
		DisableVSyncEvent();
		actorFrameData = Actor[uVar7].frame_pointer_maybe;
		Actor[uVar11].x_pos = Actor[uVar7].x_pos - *(short *)((int)actorFrameData + 0xe);
		Actor[uVar10].x_pos = Actor[uVar11].x_pos;
		sVar2 = Actor[uVar10].x_pos;
		Actor[uVar11].y_pos = Actor[uVar7].y_pos - *(short *)((int)actorFrameData + 0x10);
		Actor[uVar10].y_pos = Actor[uVar11].y_pos;
		sVar3 = Actor[uVar10].y_pos;
		actorFrameData = Actor[uVar10].frame_pointer_maybe;
		Actor[uVar13].x_pos = sVar2 - *(short *)((int)actorFrameData + 0xe);
		Actor[uVar12].x_pos = Actor[uVar13].x_pos;
		Actor[uVar13].y_pos = sVar3 - *(short *)((int)actorFrameData + 0x10);
		Actor[uVar12].y_pos = Actor[uVar13].y_pos;
		EnableVSyncEvent();
		local_282 = DAT_80093cc0;
		local_286 = DAT_80093cbc;
		local_284 = DAT_80093cb8;
		local_288 = DAT_80093cb4;
		if (DAT_80093c94 != 1) {
			if (DAT_80093c94 < 2) {
				if (DAT_80093c94 == 0) {
					if (((local_264 < 300) || (DAT_80093ca0 != 0)) || (DAT_80093cbc == 2)) {
						if ((local_260 != 0) && (local_260 = local_260 + -1, local_260 == 0)) {
							if (local_262 != 0) {
								local_262 = local_262 - 1;
							}
							local_260 = 300;
						}
					}
					else {
						local_264 = 0;
						local_260 = 300;
						playSoundFromTxtIndex(textIdTbl[local_262]);
						actor_update_sequenceID((uint)uVar6,(uint)local_2b0[(uint)local_262 * 2]);
						uVar18 = (uint)local_262;
						local_262 = local_262 + 1;
						FUN_80093aec_dialog(textIdTbl[uVar18],0x14,1);
						if (local_262 == 3) {
							while (((DAT_80093cb4 != 2 || (DAT_80093cbc != 0)) ||
									((DAT_80093cb8 != 2 || (DAT_80093cc0 != 0))))) {
								ContinueGame?();
								if ((Actor[uVar10].flags & 4) != 0) {
									if (DAT_80093cbc != 0) {
										DAT_80093cbc = DAT_80093cbc - 1;
									}
									if (2 < DAT_80093cb4) {
										DAT_80093cb4 = DAT_80093cb4 - 1;
									}
									if (DAT_80093cb4 < 2) {
										DAT_80093cb4 = DAT_80093cb4 + 1;
									}
								}
								if ((Actor[uVar12].flags & 4) != 0) {
									if (DAT_80093cc0 != 0) {
										DAT_80093cc0 = DAT_80093cc0 - 1;
									}
									if (2 < DAT_80093cb8) {
										DAT_80093cb8 = DAT_80093cb8 - 1;
									}
									if (DAT_80093cb8 < 2) {
										DAT_80093cb8 = DAT_80093cb8 + 1;
									}
								}
								if (Actor[uVar10]._sequenceID !=
									actorSequenceIdTbl[(uint)DAT_80093cb4 * 3 + (uint)DAT_80093cbc]) {
									actor_update_sequenceID
											((uint)uVar10,
											 (uint)(ushort)actorSequenceIdTbl
											 [(uint)DAT_80093cb4 * 3 + (uint)DAT_80093cbc]);
									actor_update_sequenceID
											((uint)uVar11,
											 (uint)(ushort)actorSequenceIdTbl
											 [(uint)DAT_80093cb4 * 3 + (uint)DAT_80093cbc]);
								}
								if (Actor[uVar12]._sequenceID !=
									actorSequenceIdTbl[(uint)DAT_80093cb8 * 3 + (uint)DAT_80093cc0]) {
									actor_update_sequenceID
											((uint)uVar12,
											 (uint)(ushort)actorSequenceIdTbl
											 [(uint)DAT_80093cb8 * 3 + (uint)DAT_80093cc0]);
									actor_update_sequenceID
											((uint)uVar13,
											 (uint)(ushort)actorSequenceIdTbl
											 [(uint)DAT_80093cb8 * 3 + (uint)DAT_80093cc0]);
								}
							}
							waitForFrames_times0x3c(2);
							DisableVSyncEvent();
							memset(paletteData,0,0x200);
							Actor[uVar5].flags = Actor[uVar5].flags & 0xfbff;
							Actor[uVar6].flags = Actor[uVar6].flags | 0x400;
							Actor[uVar7].flags = Actor[uVar7].flags | 0x400;
							Actor[uVar9].flags = Actor[uVar9].flags | 0x400;
							Actor[uVar8].flags = Actor[uVar8].flags | 0x400;
							Actor[uVar10].flags = Actor[uVar10].flags | 0x400;
							Actor[uVar11].flags = Actor[uVar11].flags | 0x400;
							Actor[uVar12].flags = Actor[uVar12].flags | 0x400;
							Actor[uVar13].flags = Actor[uVar13].flags | 0x400;
							EnableVSyncEvent();
							_vm->waitForFrames(6);
							actor_update_sequenceID((uint)uVar5,1);
							do {
							} while ((Actor[uVar5].flags & 4) == 0);
							ContinueGame?();
							load_palette_into_frame_buffer(0,paletteData);
							load_palette_into_frame_buffer(1,paletteData);
							FUN_8001a7c4((uint)DAT_8008e7e8,(uint)DAT_8008e844,(uint)DAT_8008e848,
										 (uint)DAT_8008e874);
							shouldExit = true;
							goto LAB_80090188;
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
					uVar18 = IsPressedLeft(0);
					if ((uVar18 & 0xffff) != 0) {
						local_268 = 1;
					}
					uVar18 = FUN_80093520();
					if ((uVar18 & 0xffff) != 0) {
						local_268 = local_268 - 1;
					}
					uVar18 = IsPressedRight(0);
					if ((uVar18 & 0xffff) != 0) {
						local_268 = local_268 + 1;
					}
					uVar18 = FUN_80093248();
					if ((uVar18 & 0xffff) != 0) {
						local_268 = local_268 + 1;
					}
					if (Actor[uVar7]._sequenceID != local_268) {
						actor_update_sequenceID((uint)uVar7,(uint)local_268);
					}
					if ((Actor[uVar10].flags & 4) != 0) {
						uVar18 = IsPressedRight(0);
						if (((uVar18 & 0xffff) != 0) && (local_288 != 0)) {
							local_288 = local_288 - 1;
						}
						uVar18 = IsPressedLeft(0);
						if (((uVar18 & 0xffff) != 0) && (local_288 < 4)) {
							local_288 = local_288 + 1;
						}
						uVar18 = IsPressedLeft(0);
						if ((((uVar18 & 0xffff) == 0) && (uVar18 = IsPressedRight(0), (uVar18 & 0xffff) == 0))
							&& (local_288 != 2)) {
							if (local_288 < 2) {
								local_288 = local_288 + 1;
							}
							else {
								local_288 = local_288 - 1;
							}
						}
						iVar20 = IsButtonBeingPressed((uint)buttonMap_Action,0);
						if ((iVar20 == 0) || (local_26c != 0)) {
							if (local_286 != 0) {
								local_286 = local_286 - 1;
							}
						}
						else {
							if (local_286 < 2) {
								local_286 = local_286 + 1;
							}
						}
					}
					if (local_286 == 2) {
						if (local_256 < 0x14) {
							local_256 = local_256 + 1;
						}
						else {
							local_256 = 0;
						}
					}
					if (local_256 < 0x14) {
						local_264 = local_264 + 1;
					}
					else {
						local_264 = 0;
					}
					if ((Actor[uVar12].flags & 4) != 0) {
						uVar18 = FUN_80093248();
						if (((uVar18 & 0xffff) != 0) && (local_284 != 0)) {
							local_284 = local_284 - 1;
						}
						uVar18 = FUN_80093520();
						if (((uVar18 & 0xffff) != 0) && (local_284 < 4)) {
							local_284 = local_284 + 1;
						}
						uVar18 = FUN_80093520();
						if ((((uVar18 & 0xffff) == 0) && (uVar18 = FUN_80093248(), (uVar18 & 0xffff) == 0)) &&
							(local_284 != 2)) {
							if (local_284 < 2) {
								local_284 = local_284 + 1;
							}
							else {
								local_284 = local_284 - 1;
							}
						}
						uVar18 = FUN_80093800();
						if (((uVar18 & 0xffff) == 0) || (local_26c != 0)) {
							if (local_282 != 0) {
								local_282 = local_282 - 1;
							}
						}
						else {
							if (local_282 < 2) {
								local_282 = local_282 + 1;
							}
						}
					}
					if (((local_286 == 2) && (local_282 == 2)) && (local_288 == local_284)) {
						if ((DAT_80093cbc == 2) && (DAT_80093cc0 != 2)) {
							local_258 = 2;
						}
						else {
							if ((DAT_80093cbc == 2) || (DAT_80093cc0 != 2)) {
								local_288 = DAT_80093cb4;
								local_286 = DAT_80093cbc;
								local_284 = DAT_80093cb8;
								local_282 = DAT_80093cc0;
							}
							else {
								local_258 = 1;
							}
						}
					}
					if (local_258 == 0) {
						if ((local_286 != DAT_80093cbc) || (local_288 != DAT_80093cb4)) {
							DAT_80093cb4 = local_288;
							DAT_80093cbc = local_286;
							actor_update_sequenceID
									((uint)uVar10,
									 (uint)(ushort)actorSequenceIdTbl[(uint)local_288 * 3 + (uint)local_286]);
							actor_update_sequenceID
									((uint)uVar11,
									 (uint)(ushort)actorSequenceIdTbl
									 [(uint)DAT_80093cb4 * 3 + (uint)DAT_80093cbc]);
						}
						if ((local_282 != DAT_80093cc0) || (local_284 != DAT_80093cb8)) {
							DAT_80093cb8 = local_284;
							DAT_80093cc0 = local_282;
							actor_update_sequenceID
									((uint)uVar12,
									 (uint)(ushort)actorSequenceIdTbl[(uint)local_284 * 3 + (uint)local_282]);
							actor_update_sequenceID
									((uint)uVar13,
									 (uint)(ushort)actorSequenceIdTbl
									 [(uint)DAT_80093cb8 * 3 + (uint)DAT_80093cc0]);
						}
					}
					else {
						if (local_258 == 1) {
							actor_update_sequenceID((uint)uVar6,1);
							actor_update_sequenceID((uint)uVar8,1);
							if (local_288 == 2) {
								local_28 = 1;
							}
							else {
								if (local_288 < 3) {
									if (local_288 == 0) {
										local_28 = 0;
									}
									else {
										LAB_800926a4:
										local_28 = 2;
									}
								}
								else {
									if (local_288 != 4) goto LAB_800926a4;
									local_28 = 2;
								}
							}
						}
						else {
							actor_update_sequenceID((uint)uVar6,3);
							actor_update_sequenceID((uint)uVar9,1);
							if (local_284 == 2) {
								local_28 = 4;
							}
							else {
								if (local_284 < 3) {
									if (local_284 == 0) {
										local_28 = 3;
									}
									else {
										LAB_80092754:
										local_28 = 4;
									}
								}
								else {
									if (local_284 != 4) goto LAB_80092754;
									local_28 = 5;
								}
							}
						}
						Actor[uVar12].flags = Actor[uVar12].flags | 0x400;
						Actor[uVar13].flags = Actor[uVar13].flags | 0x400;
						actor_update_sequenceID((uint)uVar10,(uint)local_28 + 0xf);
						actor_update_sequenceID((uint)uVar11,(uint)local_28 + 0xf);
						DAT_80093c94 = 1;
						DAT_80093c90 = 0x1e;
					}
				}
			}
			else {
				if (DAT_80093c94 == 2) {
					Actor[uVar12].flags = Actor[uVar12].flags | 0x1000;
					Actor[uVar13].flags = Actor[uVar13].flags | 0x1000;
					Actor[uVar10].flags = Actor[uVar10].flags | 0x1000;
					Actor[uVar11].flags = Actor[uVar11].flags | 0x1000;
					screenShakeOffset = 0;
					if (local_258 == 1) {
						playSoundFromTxtIndex(DAT_80063ad0);
						actor_update_sequenceID((uint)uVar6,9);
						FUN_80093aec_dialog(DAT_80063ad0,0x14,1);
						do {
						} while ((engine_flags_maybe & 0x8000) != 0);
						actor_update_sequenceID((uint)uVar6,2);
						actor_update_sequenceID((uint)uVar9,3);
					}
					else {
						playSoundFromTxtIndex(DAT_80063ad4);
						actor_update_sequenceID((uint)uVar6,10);
						FUN_80093aec_dialog(DAT_80063ad4,0x14,1);
						actor_update_sequenceID((uint)uVar9,2);
						do {
						} while ((engine_flags_maybe & 0x8000) != 0);
						actor_update_sequenceID((uint)uVar6,4);
					}
					waitForFrames_times0x3c(2);
					shouldExit = true;
				}
			}
			goto LAB_80090188;
		}
		if (local_272 == 0) {
			local_272 = 1;
			if (local_27a != 0) {
				local_27a = local_27a - 1;
			}
			if (local_278 != 0) {
				local_278 = local_278 - 1;
			}
		}
		else {
			local_272 = local_272 + -1;
		}
		uVar18 = PressedThisFrameR2(0);
		uVar19 = PressedThisFrameR1(0);
		if (((uVar18 | uVar19) & 0xffff) != 0) {
			local_27a = local_27a + 4;
		}
		uVar18 = PressedThisFrameL2(0);
		uVar19 = PressedThisFrameL1(0);
		if (((uVar18 | uVar19) & 0xffff) != 0) {
			local_27a = local_27a + 4;
		}
		uVar18 = FUN_80093990();
		if ((uVar18 & 0xffff) != 0) {
			if (param_1 == 1) {
				local_278 = local_278 + 4;
			}
			else {
				local_278 = local_278 + 6;
			}
		}
		uVar18 = FUN_80093a30();
		if ((uVar18 & 0xffff) != 0) {
			if (param_1 == 1) {
				local_278 = local_278 + 4;
			}
			else {
				local_278 = local_278 + 6;
			}
		}
		if ((0x3f < local_278) || (0x3f < local_27a)) {
			Actor[uVar6].flags = Actor[uVar6].flags | 0x1000;
			Actor[uVar8].flags = Actor[uVar8].flags | 0x1000;
			Actor[uVar9].flags = Actor[uVar9].flags | 0x1000;
			if (0x40 < local_27a) {
				local_27a = 0x40;
			}
			if (0x40 < local_278) {
				local_278 = 0x40;
			}
			if (((local_278 == local_27a) || ((local_258 == 2 && (local_278 < local_27a)))) ||
				((local_258 == 1 && (local_27a < local_278)))) {
				if (local_258 == 1) {
					playSoundFromTxtIndex(DAT_80063ad8);
					actor_update_sequenceID((uint)uVar6,0xb);
					FUN_80093aec_dialog(DAT_80063ad8,0x14,1);
				}
				else {
					playSoundFromTxtIndex(DAT_80063adc);
					actor_update_sequenceID((uint)uVar6,0xc);
					FUN_80093aec_dialog(DAT_80063adc,0x14,1);
				}
				local_258 = 0;
				DAT_80093cb4 = 2;
				DAT_80093cbc = 1;
				DAT_80093cb8 = 0;
				DAT_80093cc0 = 1;
				Actor[uVar12].flags = Actor[uVar12].flags & 0xfbff;
				Actor[uVar13].flags = Actor[uVar13].flags & 0xfbff;
				actor_update_sequenceID
						((uint)uVar10,
						 (uint)(ushort)actorSequenceIdTbl[(uint)DAT_80093cb4 * 3 + (uint)DAT_80093cbc]);
				actor_update_sequenceID
						((uint)uVar11,
						 (uint)(ushort)actorSequenceIdTbl[(uint)DAT_80093cb4 * 3 + (uint)DAT_80093cbc]);
				actor_update_sequenceID
						((uint)uVar12,
						 (uint)(ushort)actorSequenceIdTbl[(uint)DAT_80093cb8 * 3 + (uint)DAT_80093cc0]);
				actor_update_sequenceID
						((uint)uVar13,
						 (uint)(ushort)actorSequenceIdTbl[(uint)DAT_80093cb8 * 3 + (uint)DAT_80093cc0]);
				DAT_80093c94 = 0;
				local_26c = 0x1e;
			}
			else {
				DAT_80093c94 = 2;
			}
		}
		 */
	} while( true );

}

} // End of namespace Dragons
