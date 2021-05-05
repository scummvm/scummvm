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

#include <common/file.h>

#include "common/system.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "trecision/dialog.h"
#include "trecision/trecision.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/defines.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/graphics.h"
#include "trecision/sound.h"
#include "trecision/video.h"

namespace Trecision {

// GAME POINTER
uint16 _actionPosition[MAXACTIONINROOM];			// Starting position of each action in the room
// DATA POINTER
char *TextArea;
// DTEXT
char DTextLines[MAXDTEXTLINES][MAXDTEXTCHARS];
// MOUSE
SDText curString;
SDText oldString;
uint8  TextStatus;

int actionInRoom(int curA) {
	for (int b = 0; b < MAXACTIONINROOM; b++) {
		if (g_vm->_room[g_vm->_curRoom]._actions[b] == curA)
			return b;
	}

	warning("Action %d not found in room %d", curA, g_vm->_curRoom);
	return 0;
}

void ReadLoc() {
	if (g_vm->_curRoom == kRoom11 && !(g_vm->_room[kRoom11]._flag & kObjFlagDone))
		g_vm->_flagShowCharacter = true;

	g_vm->_soundMgr->fadeOut();

	g_vm->_graphicsMgr->clearScreenBufferTop();

	Common::String filename = Common::String::format("%s.cr", g_vm->_room[g_vm->_curRoom]._baseName);
	Common::SeekableReadStream *picFile = g_vm->_dataFile.createReadStreamForCompressedMember(filename);

	SObject bgInfo;
	bgInfo.readRect(picFile);

	g_vm->_graphicsMgr->loadBackground(picFile, bgInfo._rect.width(), bgInfo._rect.height());
	g_vm->_sortTable.clear();
	ReadObj(picFile);

	g_vm->_soundMgr->stopAll();

	if (g_vm->_room[g_vm->_curRoom]._sounds[0] != 0)
		g_vm->_soundMgr->loadRoomSounds();

	Common::String fname = Common::String::format("%s.3d", g_vm->_room[g_vm->_curRoom]._baseName);
	read3D(fname);

	g_vm->_graphicsMgr->resetScreenBuffer();

	if (g_vm->_room[g_vm->_curRoom]._bkgAnim) {
		g_vm->_animMgr->startSmkAnim(g_vm->_room[g_vm->_curRoom]._bkgAnim);
	} else
		g_vm->_animMgr->smkStop(kSmackerBackground);

	InitAtFrameHandler(g_vm->_room[g_vm->_curRoom]._bkgAnim, 0);
}

void TendIn() {
	TextStatus = TEXT_OFF;

	if (g_vm->_curRoom == kRoomIntro) {
		g_vm->_dialogMgr->playDialog(dFLOG);
		return;
	}

	g_vm->_flagPaintCharacter = true;
	g_vm->_soundMgr->waitEndFading();
	PaintScreen(true);

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, MAXY);
}

void readObject(Common::SeekableReadStream *stream, uint16 objIndex, uint16 objectId) {
	SObject *obj = &g_vm->_obj[objectId];

	if (obj->_mode & OBJMODE_FULL) {
		obj->readRect(stream);

		uint32 size = obj->_rect.width() * obj->_rect.height();
		delete[] g_vm->_objPointers[objIndex];
		g_vm->_objPointers[objIndex] = new uint16[size];
		for (uint32 i = 0; i < size; ++i)
			g_vm->_objPointers[objIndex][i] = stream->readUint16LE();

		g_vm->_graphicsMgr->updatePixelFormat(g_vm->_objPointers[objIndex], size);
	}

	if (obj->_mode & OBJMODE_MASK) {
		obj->readRect(stream);

		uint32 size = stream->readUint32LE();
		delete[] g_vm->_objPointers[objIndex];
		g_vm->_objPointers[objIndex] = new uint16[size];
		for (uint32 i = 0; i < size; ++i)
			g_vm->_objPointers[objIndex][i] = stream->readUint16LE();

		g_vm->_graphicsMgr->updatePixelFormat(g_vm->_objPointers[objIndex], size);

		size = stream->readUint32LE();
		delete[] g_vm->_maskPointers[objIndex];
		g_vm->_maskPointers[objIndex] = new uint8[size];
		for (uint32 i = 0; i < size; ++i)
			g_vm->_maskPointers[objIndex][i] = (uint8)stream->readByte();
	}

	if (obj->_mode & (OBJMODE_MASK | OBJMODE_FULL)) {
		SSortTable entry;
		entry._objectId = objectId;
		entry._remove = !g_vm->isObjectVisible(objectId);
		g_vm->_sortTable.push_back(entry);		
	}
}

void ReadObj(Common::SeekableReadStream *stream) {
	if (!g_vm->_room[g_vm->_curRoom]._object[0])
		return;

	for (uint16 objIndex = 0; objIndex < MAXOBJINROOM; objIndex++) {
		const uint16 objectId = g_vm->_room[g_vm->_curRoom]._object[objIndex];
		if (!objectId)
			break;

		if (g_vm->_curRoom == kRoom41D && objIndex == PATCHOBJ_ROOM41D)
			break;

		if (g_vm->_curRoom == kRoom2C && objIndex == PATCHOBJ_ROOM2C)
			break;

		readObject(stream, objIndex, objectId);
	}
}

void ReadExtraObj2C() {
	if (!g_vm->_room[g_vm->_curRoom]._object[32])
		return;

	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember("2c2.bm");

	for (uint16 objIndex = PATCHOBJ_ROOM2C; objIndex < MAXOBJINROOM; objIndex++) {
		const uint16 objectId = g_vm->_room[g_vm->_curRoom]._object[objIndex];
		if (!objectId)
			break;

		readObject(ff, objIndex, objectId);
	}

	delete ff;
}

void ReadExtraObj41D() {
	if (!g_vm->_room[g_vm->_curRoom]._object[32])
		return;

	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember("41d2.bm");
	for (uint16 objIndex = PATCHOBJ_ROOM41D; objIndex < MAXOBJINROOM; objIndex++) {
		const uint16 objectId = g_vm->_room[g_vm->_curRoom]._object[objIndex];
		if (!objectId)
			break;

		readObject(ff, objIndex, objectId);
	}
	delete ff;
}

struct ElevatorAction {
	uint16 dialog;
	uint16 choice;
	uint16 action;
	uint16 newRoom;
};

void RedrawRoom() {
	const uint16 curDialog = g_vm->_dialogMgr->_curDialog;
	const uint16 curChoice = g_vm->_dialogMgr->_curChoice;
	const uint16 bgAnim = g_vm->_room[g_vm->_curRoom]._bkgAnim;
	const ElevatorAction elevatorActions[6] = {
		{dASCENSORE12, 3, a129PARLACOMPUTERESCENDE, kRoom13},
		{dASCENSORE12, 4, a129PARLACOMPUTERESCENDE, kRoom16},
		{dASCENSORE13, 17, a139CHIUDONOPORTESU, kRoom12},
		{dASCENSORE13, 18, a1316CHIUDONOPORTEGIU, kRoom16},
		{dASCENSORE16, 32, a1616SALECONASCENSORE, kRoom12},
		{dASCENSORE16, 33, a1616SALECONASCENSORE, kRoom13},
	};

	g_vm->_flagShowCharacter = g_vm->_dialogMgr->showCharacterAfterDialog();

	for (int i = 0; i < 6; ++i) {
		if (curDialog == elevatorActions[i].dialog && curChoice == elevatorActions[i].choice) {
			g_vm->StartCharacterAction(elevatorActions[i].action, elevatorActions[i].newRoom, 20, 0);
			break;
		}
	}

	g_vm->_sortTable.clear();
	g_vm->_graphicsMgr->resetScreenBuffer();

	if (bgAnim)
		g_vm->_animMgr->startSmkAnim(bgAnim);

	if (g_vm->_curRoom == kRoom4P && curDialog == dF4PI)
		g_vm->_animMgr->smkGoto(kSmackerBackground, 21);

	TextStatus = TEXT_OFF;
	g_vm->_flagPaintCharacter = true;
	PaintScreen(true);
	g_vm->_graphicsMgr->copyToScreen(0, 0, 640, 480);
}

} // End of namespace Trecision
