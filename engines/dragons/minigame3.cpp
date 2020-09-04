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
#include "common/scummsys.h"
#include "common/rect.h"
#include "dragons/actor.h"
#include "dragons/font.h"
#include "dragons/minigame3.h"
#include "dragons/dragonini.h"
#include "dragons/inventory.h"
#include "dragons/scene.h"
#include "dragons/screen.h"
#include "dragons/sound.h"
#include "dragons/talk.h"
#include "dragons/dragons.h"

namespace Dragons {

struct BunnyStruct {
	uint32 positionIdx;
	uint32 x;
	uint32 y;
	uint32 field_0xc;
	uint32 field_0x10;
};

struct TearInfo {
	uint16 x;
	uint16 y;
	uint16 unk;
	uint16 yRelated;
};

struct UnkStruct {
	uint16 position1;
	uint16 position2;
	uint32 unk4;
	uint32 unk8;
	uint32 unk12;
	uint32 unk16;
	uint32 field_0x14;
};

Minigame3::Minigame3(DragonsEngine *vm) : _vm(vm) {}

void Minigame3::run() {
	bool bVar1;
	Actor *handActorId;
	//uint16 actorId;
	int16 sVar2;
	//int iVar3;
	uint tmpValue;
	int iVar4;
	DragonINI *flicker;
	uint16 origSceneId;
	//byte auStack1584_palette[512]; //[126];
	//uint16 local_5b2;
	//byte auStack1072_palette[512];
	Actor *bunnyActorTbl[4];
	uint16 local_228 = 0;
	//uint16 local_226;
	int16 local_224;
	Actor *tearActorTbl[8];
	uint16 local_210;
	int16 local_208[16];
	uint local_1e8;
	uint oldEngineFlags;
	uint16 local_1e0;
	uint16 local_1de;
	Actor *tearBlinkActorTbl2[4];
	Actor *tearBlinkActorTbl[4];
	int16 local_1c8 = 0;
	int16 local_1c6 = 0;
	uint16 local_1c2;
	int16 local_1c0;
	int16 local_1be;
	int16 local_1bc;
	int16 local_1ba;
	uint16 local_1b8;
	int16 eyeBgYOffsetTbl[21];
	TearInfo tearInfo[30];
	Common::Point bunnyPositionsTbl[4];
	Common::Point handPositionsTbl[4];
	uint16 goodRabbitPositionTbl[4];
	uint16 bunnyPositionTbl[4];
	int16 currentState;
	uint16 flags;
	int16 local_5c;
	int16 local_5a;
	int16 local_58;
	int16 local_56;
	int16 hopCounter;
	uint16 local_50 = 0;
	BunnyStruct bunnyInfo[2];
	uint16 local_20 = 0;
	uint16 local_1e = 0;
	uint16 local_1c = 0;
	uint16 local_1a = 0;
	int16 local_14;
	InventoryState origInventoryType;
	int16 local_10;
	int16 local_e;
	UnkStruct UnkStruct_ARRAY_800931a0[4];
	int16 unkXPosTbl[20];


	Common::File *fd = new Common::File();
	if (!fd->open("arc3.bin")) {
		error("Failed to open arc3.bin");
	}

	for (int i = 0; i < 21; i++) {
		eyeBgYOffsetTbl[i] = fd->readUint16LE();
	}

	fd->seek(0x2c);
	for (int i = 0; i < 30; i++) {
		tearInfo[i].x = fd->readUint16LE();
		tearInfo[i].y = fd->readUint16LE();
		tearInfo[i].unk = fd->readUint16LE();
		tearInfo[i].yRelated = fd->readUint16LE();
	}

	for (int i = 0; i < 4; i++) {
		bunnyPositionsTbl[i].x = fd->readUint16LE();
		bunnyPositionsTbl[i].y = fd->readUint16LE();
	}

	for (int i = 0; i < 4; i++) {
		handPositionsTbl[i].x = fd->readUint16LE();
		handPositionsTbl[i].y = fd->readUint16LE();
	}

	fd->seek(_vm->getMiniGame3DataOffset());

	for (int i = 0; i < 4; i++) {
		UnkStruct_ARRAY_800931a0[i].position1 = fd->readUint16LE();
		UnkStruct_ARRAY_800931a0[i].position2 = fd->readUint16LE();
		UnkStruct_ARRAY_800931a0[i].unk4 = fd->readUint32LE();
		UnkStruct_ARRAY_800931a0[i].unk8 = fd->readUint32LE();
		UnkStruct_ARRAY_800931a0[i].unk12 = fd->readUint32LE();
		UnkStruct_ARRAY_800931a0[i].unk16 = fd->readUint32LE();
		UnkStruct_ARRAY_800931a0[i].field_0x14 = fd->readUint32LE();
	}

	for (int i = 0; i < 20; i++) {
		unkXPosTbl[i] = fd->readSint16LE();
	}

	fd->close();
	delete fd;

	origInventoryType = _vm->_inventory->getState();
	_vm->fadeToBlack();
	_vm->_inventory->setState(Closed);
	_vm->reset_screen_maybe();
	flicker = _vm->_dragonINIResource->getFlickerRecord();
	flicker->sceneId = 0;
	_vm->_dragonINIResource->setFlickerRecord(nullptr);
	origSceneId = _vm->getCurrentSceneId();
	_vm->_scene->setSceneId(6);
	_vm->_scene->loadScene(6 | 0x8000, 0);
	_vm->_scene->setFgLayerPriority(4);

// TODO
	_vm->_screen->loadPalette(1, _vm->_scene->getPalette());
	_vm->_screen->loadPalette(4, _vm->_scene->getPalette());
	_vm->_screen->updatePaletteTransparency(4, 1, 0xff, true);
	fun_80017ef0();
	oldEngineFlags = _vm->getAllFlags();
	_vm->clearFlags(ENGINE_FLAG_80);
	_vm->clearFlags(ENGINE_FLAG_20);
	_vm->clearFlags(ENGINE_FLAG_10);
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_1);
	_vm->setFlags(ENGINE_FLAG_100);
	_vm->setFlags(ENGINE_FLAG_1000_SUBTITLES_DISABLED);
// TODO
//	memcpy2(auStack1584_palette, scrFileData_maybe, 0x200);
//	memcpy2(auStack1072_palette, scrFileData_maybe, 0x200);
//	local_5b2 = 0x7fff;
//	DisableVSyncEvent();
	int i = 0;
	while ((int16)i < 4) {
		bunnyActorTbl[(int16)i] = _vm->_actorManager->loadActor(0x15, 4, 0, 0);
		if (bunnyActorTbl[(int16)i] == nullptr) {
			error("Couldn't_alloc_bunny");
		}
		bunnyActorTbl[(int16)i]->setFlag(ACTOR_FLAG_80);
		bunnyActorTbl[(int16)i]->setFlag(ACTOR_FLAG_100);
		bunnyActorTbl[(int16)i]->setFlag(ACTOR_FLAG_200);
		bunnyActorTbl[(int16)i]->setFlag(ACTOR_FLAG_4000);

		bunnyActorTbl[(int16)i]->_priorityLayer = 0;
		bunnyActorTbl[(int16)i]->_x_pos = bunnyPositionsTbl[(int16)i].x;
		bunnyActorTbl[(int16)i]->_y_pos = bunnyPositionsTbl[(int16)i].y;
		i = i + 1;
	}
	i = 0;
	while ((int16)i < 8) {
		tearActorTbl[(int16)i] = _vm->_actorManager->loadActor(0x15, 0x13, 0, 0);
		if (tearActorTbl[(int16)i] == nullptr) {
			error("Couldn't alloc tear");
		}
		tearActorTbl[(int16)i]->_flags = tearActorTbl[(int16)i]->_flags | 0x380;
		tearActorTbl[(int16)i]->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
		tearActorTbl[(int16)i]->_priorityLayer = 0;
		local_208[(int16)i] = -1;
		local_208[(int)(int16)i + 8] = 0;
		i = i + 1;
	}
	local_1e0 = 0;
	local_1e8 = 0;
	handActorId = _vm->_actorManager->loadActor(0x19, 0, 0, 0);
	if (handActorId == nullptr) {
		error("Couldn't alloc hand");
	}
	handActorId->setFlag(ACTOR_FLAG_80);
	handActorId->setFlag(ACTOR_FLAG_100);
	handActorId->setFlag(ACTOR_FLAG_200);
	handActorId->setFlag(ACTOR_FLAG_800);
	handActorId->setFlag(ACTOR_FLAG_2000);
	handActorId->setFlag(ACTOR_FLAG_4000);
	handActorId->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	handActorId->_priorityLayer = 0;
	handActorId->_walkSpeed = 0x40000;
	i = 0;
	while ((int16)i < 2) {
		tearBlinkActorTbl[(int16)i] = _vm->_actorManager->loadActor(0x34, (uint)i, 0, 0);
		if (tearBlinkActorTbl[(int16)i] == nullptr) {
			error("Couldn't alloc tear blink");
		}
		tearBlinkActorTbl[(int16)i]->_flags = tearBlinkActorTbl[(int16)i]->_flags | 0x4384;
		tearBlinkActorTbl[(int16)i]->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
		tearBlinkActorTbl[(int16)i]->_priorityLayer = 0;
		i = i + 1;
	}
	i = 0;
	while ((int16)i < 2) {
		tearBlinkActorTbl2[(int16)i] = _vm->_actorManager->loadActor(0x16, (uint)i, 0, 0);
		if (tearBlinkActorTbl2[(int16)i] == nullptr) {
			error("Couldn't alloc tear blink");
		}
		tearBlinkActorTbl2[(int16)i]->setFlag(ACTOR_FLAG_100);
		tearBlinkActorTbl2[(int16)i]->setFlag(ACTOR_FLAG_800);
		tearBlinkActorTbl2[(int16)i]->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
		tearBlinkActorTbl2[(int16)i]->_priorityLayer = 0;
		i = i + 1;
	}
	tearBlinkActorTbl2[0]->_x_pos = 0x56;
	tearBlinkActorTbl2[0]->_y_pos = 0x8c;
	tearBlinkActorTbl2[1]->_x_pos = 0xf0;
	tearBlinkActorTbl2[1]->_y_pos = 0x8c;
	tearBlinkActorTbl[0]->_x_pos = 0x23; //DAT_80093228_23;
	tearBlinkActorTbl[1]->_x_pos = 0xbf; //DAT_8009322a_bf;
	tearBlinkActorTbl[0]->_y_pos = 0xcc;
	tearBlinkActorTbl[1]->_y_pos = 0xcc;
//	EnableVSyncEvent();
	i = 0;
	while ((int16)i < 4) {
		goodRabbitPositionTbl[(int16)i] = 0;
		i = i + 1;
	}
	int16 goodRabbitStartingPosition = _vm->getRand(4);
	goodRabbitPositionTbl[goodRabbitStartingPosition] = 1;
	i = 0;
	while ((int16)i < 4) {
		bunnyPositionTbl[(int16)i] = i;
		bunnyActorTbl[(int16)i]->_priorityLayer = 2;
		bunnyActorTbl[(int16)i]->updateSequence(4);
		i = i + 1;
	}
	_vm->waitForFrames(1);
	updateBackgroundLayerOffset(2, 0x280, 0);
	updateBackgroundLayerOffset(1, 0, 0);
	updateBackgroundLayerOffset(0, 0, 0);
	_vm->fadeFromBlack();
	_vm->waitForFrames(0xf);
	_vm->_talk->loadAndDisplayDialogAroundPoint(_vm->getMiniGame3StartingDialog(), 0x14, 3, 0x1e01, 0);
	_vm->waitForFrames(0x1e);
	// TODO clearTextDialog((uint)DAT_8008e7e8, (uint)DAT_8008e844, (uint)DAT_8008e848, (uint)DAT_8008e874);
	i = 0;
	while ((int16)i < 4) {
		if (goodRabbitPositionTbl[(int16)i] == 0) {
			bunnyActorTbl[(int16)i]->updateSequence(1);
		}
		i = i + 1;
	}

	bunnyActorTbl[(goodRabbitStartingPosition + 1) % 4]->waitUntilFlag4IsSet(); // wait for a rabid rabbit to complete its display sequence.

	bunnyActorTbl[goodRabbitStartingPosition]->updateSequence(0);
	bunnyActorTbl[goodRabbitStartingPosition]->waitUntilFlag4IsSet();
	i = 0;
	while ((int16)i < 4) {
		bunnyActorTbl[(int16)i]->updateSequence(4);
		i = i + 1;
	}
	_vm->waitForFrames(0x1e);
	_vm->waitForFrames(0x1e);
	// TODO clearTextDialog((uint)DAT_8008e7e8, (uint)DAT_8008e844, (uint)DAT_8008e848, (uint)DAT_8008e874);
	local_56 = 0;
	local_1c0 = 0;
	currentState = 2;
	flags = 0;
	local_58 = 0x1e;
	hopCounter = 0;
	local_210 = 0;
	local_1de = 0;
	local_5a = 0;
	local_5c = _vm->getRand(2);
	local_5c = local_5c + 5;
	local_1be = 1;
	local_1bc = _vm->getRand(3);
	local_1bc = local_1bc + 3;
	local_1ba = 10;
	local_1b8 = 0;
	local_10 = 0;
	local_e = 0;
	tearBlinkActorTbl[0]->_priorityLayer = 3;
	tearBlinkActorTbl[1]->_priorityLayer = 3;
	tearBlinkActorTbl[0]->updateSequence(0);
	tearBlinkActorTbl[1]->updateSequence(1);
	local_1c2 = 0;
	while (true) {
		_vm->waitForFrames(1);
		switch (currentState) {
		case 1:
			if ((bunnyActorTbl[local_1a]->_sequenceID != 5) || ((bunnyActorTbl[local_1a]->_flags & 4) != 0)) {
				if ((local_56 < 1) ||
						(((int)bunnyInfo[local_20].x >> 9 <= (int)(uint)(uint16)bunnyPositionsTbl[bunnyInfo[local_1e].positionIdx].x ||
						((int)(uint)(uint16)bunnyPositionsTbl[bunnyInfo[local_20].positionIdx].x <= (int)bunnyInfo[local_1e].x >> 9)))) {
					bunnyActorTbl[local_1c]->_x_pos = bunnyPositionsTbl[bunnyInfo[local_1e].positionIdx].x;
					bunnyActorTbl[local_1a]->_x_pos = bunnyPositionsTbl[bunnyInfo[local_20].positionIdx].x;
					bunnyActorTbl[local_1c]->_y_pos = bunnyPositionsTbl[bunnyInfo[local_1e].positionIdx].y;
					bunnyActorTbl[local_1a]->_y_pos = bunnyPositionsTbl[bunnyInfo[local_20].positionIdx].y;
					currentState = 5;
				} else {
					if (bunnyActorTbl[local_1a]->_sequenceID == 5) {
						bunnyActorTbl[local_1a]->updateSequence(6);
						bunnyActorTbl[local_1c]->updateSequence(0xd);
					} else {
						local_56 = local_56 - *(int16 *)&UnkStruct_ARRAY_800931a0[local_50].field_0x14;
						bunnyInfo[local_20].x = bunnyInfo[local_20].x + UnkStruct_ARRAY_800931a0[local_50].field_0x14 * -0x200;
						bunnyInfo[local_20].y = bunnyInfo[local_20].y - bunnyInfo[local_20].field_0xc;
						bunnyInfo[local_20].field_0xc = bunnyInfo[local_20].field_0xc + bunnyInfo[local_20].field_0x10;
						bunnyActorTbl[local_1c]->_x_pos = (int16)((int)bunnyInfo[local_20].x >> 9);
						bunnyActorTbl[local_1c]->_y_pos = (int16)((int)bunnyInfo[local_20].y >> 9);
						bunnyInfo[local_1e].x = bunnyInfo[local_1e].x + UnkStruct_ARRAY_800931a0[local_50].field_0x14 * 0x200;
						bunnyInfo[local_1e].y = bunnyInfo[local_1e].y - bunnyInfo[local_1e].field_0xc;
						bunnyInfo[local_1e].field_0xc = bunnyInfo[local_1e].field_0xc + bunnyInfo[local_1e].field_0x10;
						bunnyActorTbl[local_1a]->_x_pos = (int16)((int)bunnyInfo[local_1e].x >> 9);
						bunnyActorTbl[local_1a]->_y_pos = (int16)((int)bunnyInfo[local_1e].y >> 9);
						if ((local_228 < 4) && unkXPosTbl[local_50 * 4 + local_228] < bunnyActorTbl[local_1a]->_x_pos) {
							local_228 = local_228 + 1;
							bunnyActorTbl[local_1a]->updateSequence(((uint)local_228 + 6) & 0xffff);
							bunnyActorTbl[local_1c]->updateSequence(((uint)local_228 + 0xd) & 0xffff);
						}
					}
				}
			}
			break;
		case 2:
			if (local_58 == 0) {
				currentState = 4;
			}
			break;
		case 3:
			if (local_58 == 0) {
				currentState = 4;
			}
			break;
		case 4:
			local_50 = _vm->getRand(4);
			if (local_50 < 2) {
				i = _vm->getRand(2);
			} else {
				i = 0;
			}
			bunnyInfo[(int16)i].positionIdx = (uint)UnkStruct_ARRAY_800931a0[local_50].position1;
			bunnyInfo[(int16)i].x = (uint)(uint16)bunnyPositionsTbl[bunnyInfo[(int16)i].positionIdx].x << 9;
			bunnyInfo[(int16)i].y = (uint)(uint16)bunnyPositionsTbl[bunnyInfo[(int16)i].positionIdx].y << 9;
			bunnyInfo[(int16)i].field_0xc = UnkStruct_ARRAY_800931a0[local_50].unk4;
			bunnyInfo[(int16)i].field_0x10 = UnkStruct_ARRAY_800931a0[local_50].unk8;
			i = i ^ 1;
			bunnyInfo[(int16)i].positionIdx = (uint)UnkStruct_ARRAY_800931a0[local_50].position2;
			bunnyInfo[(int16)i].x = (uint)(uint16)bunnyPositionsTbl[bunnyInfo[(int16)i].positionIdx].x << 9;
			bunnyInfo[(int16)i].y = (uint)(uint16)bunnyPositionsTbl[bunnyInfo[(int16)i].positionIdx].y << 9;
			bunnyInfo[(int16)i].field_0xc = UnkStruct_ARRAY_800931a0[local_50].unk12;
			bunnyInfo[(int16)i].field_0x10 = UnkStruct_ARRAY_800931a0[local_50].unk16;
			local_56 = bunnyPositionsTbl[UnkStruct_ARRAY_800931a0[local_50].position2].x - bunnyPositionsTbl[UnkStruct_ARRAY_800931a0[local_50].position1].x;
			local_1e = (uint16)((int)bunnyInfo[1].x <= (int)bunnyInfo[0].x);
			local_20 = (int16)((uint)local_1e + 1) + (int16)((int)((uint)local_1e + 1) / 2) * -2;
			local_1a = bunnyPositionTbl[bunnyInfo[local_1e].positionIdx];
			local_1c = bunnyPositionTbl[bunnyInfo[local_20].positionIdx];
			bunnyActorTbl[local_1a]->updateSequence(5);
			bunnyActorTbl[local_1c]->updateSequence(0xc);
			if (hopCounter == 0x1d) {
				_vm->playOrStopSound(2);
			} else {
				_vm->playOrStopSound((uint)local_1c2);
				local_1c2 = 1 - local_1c2;
			}
			local_228 = 0;
			i = goodRabbitPositionTbl[bunnyInfo[local_1e].positionIdx];
			goodRabbitPositionTbl[bunnyInfo[local_1e].positionIdx] = goodRabbitPositionTbl[bunnyInfo[local_20].positionIdx];
			goodRabbitPositionTbl[bunnyInfo[local_20].positionIdx] = i;
			i = bunnyPositionTbl[bunnyInfo[local_1e].positionIdx];
			bunnyPositionTbl[bunnyInfo[local_1e].positionIdx] = bunnyPositionTbl[bunnyInfo[local_20].positionIdx];
			bunnyPositionTbl[bunnyInfo[local_20].positionIdx] = i;
			currentState = 1;
			break;
		case 5:
			hopCounter = hopCounter + 1;
			local_5a = local_5a + 1;
			bunnyActorTbl[local_1a]->updateSequence(0xb);
			bunnyActorTbl[local_1c]->updateSequence(0x12);
			if (local_5a == local_5c) {
				local_5a = 0;
				local_5c = _vm->getRand(2);
				local_5c = local_5c + 5;
				currentState = 3;
				local_58 = 0x3c;
			} else {
				currentState = 2;
				local_58 = 2;
			}
			break;
		case 6:
			local_10 = 0;
			if (local_1e8 == 0) {
				flags = flags | 8;
			}
			break;
		default:
			error("Unknown state");
		}
		i = 0;
		while ((int16)i < 8) {
			if (local_208[(int16)i] != -1) {
				tearActorTbl[(int16)i]->_y_pos = tearActorTbl[(int16)i]->_y_pos + ((uint16)local_208[(int)(int16)i + 8] >> 6);
				if (tearActorTbl[(int16)i]->_y_pos < (int16)tearInfo[local_208[(int16)i]].yRelated) {
					local_208[(int)(int16)i + 8] = local_208[(int)(int16)i + 8] + 8;
				} else {
					tearActorTbl[(int16)i]->_priorityLayer = 0;
					local_1e8 = local_1e8 & ~(1 << ((int)local_208[(int16)i] & 0x1fU));
					local_208[(int16)i] = -1;
					local_1e0 = local_1e0 - 1;
				}
			}
			i = i + 1;
		}
		if (((local_1e0 < local_1de) && (currentState != 6)) && (sVar2 = _vm->getRand(2), sVar2 == 0)) {
			if ((local_1e8 & 0x7fff) < local_1e8 >> 0xf) {
				local_14 = 0;
			} else {
				local_14 = 0xf;
			}
			do {
				sVar2 = _vm->getRand(0xf);
				sVar2 = sVar2 + local_14;
			} while ((1 << ((int)sVar2 & 0x1fU) & local_1e8) != 0);
			local_1e8 = local_1e8 | 1 << ((int)sVar2 & 0x1fU);
			local_14 = 0;
			while ((local_14 < 8 && (local_208[local_14] != -1))) {
				local_14 = local_14 + 1;
			}
			local_208[local_14] = sVar2;
			tearActorTbl[local_14]->_x_pos = tearInfo[sVar2].x;
			tearActorTbl[local_14]->_y_pos = tearInfo[sVar2].y;
			local_208[(int)local_14 + 8] = 0x20;
			tearActorTbl[local_14]->updateSequence(0x13);
			tearActorTbl[local_14]->_priorityLayer = 3;
			local_1e0 = local_1e0 + 1;
		}
		if ((flags & 1) == 0) {
			if ((flags & 2) == 0) {
				if ((flags & 4) == 0) {
					if (_vm->isActionButtonPressed()) {
						local_1c8 = 1;
						flags = flags | 1;
						local_1c6 = 3;
						updateBackgroundLayerOffset(2, 0x640, 0);
						if (((currentState != 3) && (currentState != 6)) && (local_e == 0)) {
							local_10 = 0x5a;
							local_e = 1;
						}
					}
				} else {
					if (local_1c6 == 0) {
						if (local_1c8 == 2) {
							local_1c6 = 3;
							local_1c8 = 1;
							updateBackgroundLayerOffset(2, 0x640, 0);
						} else {
							if (local_1c8 == 1) {
								local_1c8 = 0;
								local_1ba = 0;
								flags = flags & 0xfffb;
								tearBlinkActorTbl2[0]->_priorityLayer = 0;
								tearBlinkActorTbl2[1]->_priorityLayer = 0;
								tearBlinkActorTbl2[0]->updateSequence(0);
								tearBlinkActorTbl2[1]->updateSequence(1);
							}
						}
					} else {
						local_1c6 = local_1c6 + -1;
					}
				}
			} else {
				if (local_1c6 == 0) {
					i = 0;
					while ((int16)i < 8) {
						tearActorTbl[(int16)i]->_priorityLayer = 0;
						local_208[(int16)i] = -1;
						i = i + 1;
					}
					local_210 = 0;
					local_1e0 = 0;
					local_1de = 0;
					local_1e8 = 0;
					fun_80017f70_paletteRelated(0);
					local_1b8 = 0;
					flags = (flags & 0xfffd) | 4;
					local_1c8 = 2;
					local_1c6 = 3;
					updateBackgroundLayerOffset(2, 0x780, 0);
					tearBlinkActorTbl[0]->_y_pos = 0xcc;
					tearBlinkActorTbl[1]->_y_pos = 0xcc;
				} else {
					local_1c6 = local_1c6 + -1;
				}
			}
		} else {
			if (local_1c6 == 0) {
				if (local_1c8 == 1) {
					local_1c6 = 3;
					local_1c8 = 2;
					updateBackgroundLayerOffset(2, 0x780, 0);
					tearBlinkActorTbl2[0]->updateSequence(0);
					tearBlinkActorTbl2[1]->updateSequence(1);
					tearBlinkActorTbl2[0]->_priorityLayer = 4;
					tearBlinkActorTbl2[1]->_priorityLayer = 4;
				} else {
					if (local_1c8 == 2) {
						local_1c6 = 0x14;
						local_1c8 = 3;
						updateBackgroundLayerOffset(2, 0x8c0, 0);
						tearBlinkActorTbl2[0]->updateSequence(2);
						tearBlinkActorTbl2[1]->updateSequence(3);
						flags = (flags & 0xfffe) | 2;
					}
				}
			} else {
				local_1c6 = local_1c6 + -1;
			}
		}
		if ((local_e != 0) && (local_10 == 0)) {
			//TODO implement this. clearTextDialog((uint)DAT_8008e7e8, (uint)DAT_8008e844, (uint)DAT_8008e848, (uint)DAT_8008e874);
			local_e = 0;
		}
		if ((local_e != 0) && (local_10 != 0)) {
			local_10 = local_10 + -1;
		}
		if (local_58 != 0) {
			local_58 = local_58 + -1;
		}
		local_210 = local_210 + 1;
		if (((uint)local_210 % 0x14 == 0) && (local_1de < 8)) {
			local_1de = local_1de + 1;
		}
		if (((uint)local_210 % 10 == 0) && (0x1d < local_210)) {
			if ((int16)local_1b8 < 0x1e) {
				local_1b8 = local_1b8 + 1;
				fun_80017f70_paletteRelated((uint)local_1b8);
			}
			if (100 < tearBlinkActorTbl[0]->_y_pos) {
				tearBlinkActorTbl[0]->_y_pos = tearBlinkActorTbl[0]->_y_pos + -3;
				tearBlinkActorTbl[1]->_y_pos = tearBlinkActorTbl[1]->_y_pos + -3;
			}
		}
		if (hopCounter == 0x1e) {
			currentState = 6;
		}
		if ((flags & 8) != 0) break;
		if (local_1c8 == 0) {
			if (local_1ba == 0) {
				if (local_1bc == 0) {
					tmpValue = _vm->getRand(2);
					if ((tmpValue & 0xffff) == 0) {
						local_1be = 1;
					} else {
						local_1be = -1;
					}
					local_1bc = _vm->getRand(3);
					local_1bc = local_1bc + 3;
					local_1ba = _vm->getRand(10);
					local_1ba = local_1ba + 10;
				} else {
					if (((local_1be < 0) && (-10 < local_1c0)) || ((0 < local_1be && (local_1c0 < 10)))) {
						local_1c0 = local_1c0 + local_1be;
					}
					local_1ba = 2;
					local_1bc = local_1bc + -1;
					iVar4 = (int)local_1c0 + 10;
					if (iVar4 < 0) {
						iVar4 = (int)local_1c0 + 0xd;
					}
					local_14 = (int16)(iVar4 >> 2);
					if (local_14 == 5) {
						local_14 = 4;
					}
					updateBackgroundLayerOffset(2, (int) local_14 * 0x140, 0);
					updateBackgroundLayerOffset(1, (int) -local_1c0, (int) eyeBgYOffsetTbl[local_1c0 + 10]);
					updateBackgroundLayerOffset(0, (int) -local_1c0, (int) eyeBgYOffsetTbl[local_1c0 + 10]);
				}
			} else {
				local_1ba = local_1ba + -1;
			}
		}
	}
	_vm->waitForFrames(1);
	i = 0;
	while ((int16)i < 3) {
		int16 local_16 = 0;
		while (local_16 < 3) {
			updateBackgroundLayerOffset(2, ((int) local_16 * 0x140 + 0x640) * 0x10000 >> 0x10, 0);
			_vm->waitForFrames(5);
			local_16 = local_16 + 1;
		}
		local_1b8 = local_1b8 - 0xb;
		if ((int16)local_1b8 < 0) {
			local_1b8 = 0;
		}
		fun_80017f70_paletteRelated((uint)local_1b8);
		tearBlinkActorTbl[0]->_y_pos = tearBlinkActorTbl[0]->_y_pos + 0x1e;
		if (199 < tearBlinkActorTbl[0]->_y_pos) {
			tearBlinkActorTbl[0]->_y_pos = 199;
		}
		tearBlinkActorTbl[1]->_y_pos = tearBlinkActorTbl[1]->_y_pos + 0x1e;
		if (199 < tearBlinkActorTbl[1]->_y_pos) {
			tearBlinkActorTbl[1]->_y_pos = 199;
		}
		_vm->waitForFrames(0xf);
		local_16 = 1;
		while (-1 < local_16) {
			updateBackgroundLayerOffset(2, ((int) local_16 * 0x140 + 0x640) * 0x10000 >> 0x10, 0);
			_vm->waitForFrames(5);
			local_16 = local_16 + -1;
		}
		i = i + 1;
	}
	tearBlinkActorTbl[0]->_flags = tearBlinkActorTbl[0]->_flags | 4;
	tearBlinkActorTbl[1]->_flags = tearBlinkActorTbl[1]->_flags | 4;
	updateBackgroundLayerOffset(2, 0x280, 0);
	updateBackgroundLayerOffset(1, 0, 0);
	updateBackgroundLayerOffset(0, 0, 0);
	local_224 = _vm->getRand(2);
	handActorId->updateSequence(0);
	handActorId->_x_pos = handPositionsTbl[local_224].x;
	handActorId->_y_pos = handPositionsTbl[local_224].y;
	handActorId->_priorityLayer = 2;
	bVar1 = false;
	//pick a hat flicker.
	_vm->_talk->loadAndDisplayDialogAroundPoint(_vm->getMiniGame3PickAHatDialog(), 0x14, 3, 0x1e01, 0);
	while (_vm->isFlagSet(ENGINE_FLAG_8000)) {
		_vm->waitForFrames(1);
	}
	while (_vm->waitForFrames(1), handActorId->isFlagSet(ACTOR_FLAG_10) || !_vm->isActionButtonPressed()) {
		if (_vm->isLeftKeyPressed() && ((local_224 == 1 || (local_224 == 3)))) {
			local_224 = local_224 + -1;
			bVar1 = true;
		}
		if (_vm->isRightKeyPressed() && ((local_224 == 0 || (local_224 == 2)))) {
			local_224 = local_224 + 1;
			bVar1 = true;
		}
		if (_vm->isUpKeyPressed() && ((local_224 == 2 || (local_224 == 3)))) {
			local_224 = local_224 + -2;
			bVar1 = true;
		}
		if (_vm->isDownKeyPressed() && ((local_224 == 0 || (local_224 == 1)))) {
			local_224 = local_224 + 2;
			bVar1 = true;
		}
		if (bVar1) {
			handActorId->startWalk((int)handPositionsTbl[local_224].x, (int)handPositionsTbl[local_224].y, 2);
			bVar1 = false;
		}
	}
	handActorId->updateSequence(1);
	handActorId->waitUntilFlag4IsSet();
	_vm->_dragonINIResource->getRecord(0x178)->objectState = local_224 + 1;
	if (goodRabbitPositionTbl[local_224] == 1) {
		bunnyActorTbl[bunnyPositionTbl[local_224]]->updateSequence(2);
		_vm->_dragonINIResource->getRecord(0x178)->objectState2 = 1;
	} else {
		bunnyActorTbl[bunnyPositionTbl[local_224]]->updateSequence(3);
		i = 0;
		while (((int16)i < 4 && (goodRabbitPositionTbl[(int16)i] != 1))) {
			i = i + 1;
		}
		_vm->waitForFrames(0x1e);
		bunnyActorTbl[bunnyPositionTbl[i]]->updateSequence(2);
		_vm->_dragonINIResource->getRecord(0x178)->objectState2 = 0;
	}
	_vm->waitForFrames(0x3c * 2);
	_vm->_sound->resumeMusic();
	_vm->fadeToBlack();
//	fun_80017f28_noop();
//	DAT_80093234 = DAT_80093234 + 1;
	_vm->_dragonINIResource->setFlickerRecord(flicker);
	flicker->sceneId = 1;
	_vm->setAllFlags(oldEngineFlags);
	_vm->setFlags(ENGINE_FLAG_40);
//	_vm->_screen->loadPalette(1, (uint)*(uint16 *)(*(int *)(&DAT_80071c30 + (uint)actors[0].actorFileDictionaryIndex * 8) + 10) +
//			   *(int *)(&DAT_80071c30 + (uint)actors[0].actorFileDictionaryIndex * 8));
	_vm->setupPalette1();
//	_vm->_screen->loadPalette(4, (uint)*(uint16 *)(*(int *)(&DAT_80071c30 + (uint)actors[0].actorFileDictionaryIndex * 8) + 10) +
//			   *(int *)(&DAT_80071c30 + (uint)actors[0].actorFileDictionaryIndex * 8));
	_vm->_screen->updatePaletteTransparency(4, 1, 0xff, true);
	_vm->_inventory->setState(origInventoryType);
	_vm->_scene->setSceneId(origSceneId);
	_vm->_scene->loadScene(origSceneId, 0);
}

void Minigame3::updateBackgroundLayerOffset(uint32 layerNumber, int16 xOffset, int16 yOffset) {
	_vm->_scene->setLayerOffset(layerNumber, Common::Point(xOffset, yOffset));
}

void Minigame3::fun_80017f70_paletteRelated(uint16 param_1) {
	if (param_1 > 0x1f) {
		param_1 = 0x1f;
	}
//	uVar1 = IsVSyncEventEnabled & 1;
//	DisableVSyncEvent();
	_vm->_screen->loadPalette(0, _vm->_scene->getPalette());
	_vm->_screen->setPaletteRecord(0, 0x3f, param_1 * 0x421);
//	load_palette_into_frame_buffer(0, abStack528);
	_vm->_screen->updatePaletteTransparency(0, 0x3f, 0x3f, param_1 == 0x1e ? false : true);
//	if (uVar1 != 0) {
//		EnableVSyncEvent();
//	}
}

void Minigame3::fun_80017ef0() {
	//TODO BgLayerGsSprite[2].attribute = BgLayerGsSprite[2].attribute | 0x50000000;
	// this sets the FG layer to additive colour blending (100% x Back + 100% x Sprite)
	fun_80017f70_paletteRelated(0);
}

} // End of namespace Dragons
