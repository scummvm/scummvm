/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "igor/igor.h"

namespace Igor {

// A 26x52 area of the picture is redrawn from the animation resource, whose frames are 26x53 each.
static const int kPart73PatchOffset = 55 * 320 + 157;
static const int kPart73PatchWidth = 26;
static const int kPart73PatchHeight = 52;
static const int kPart73PatchFrameSize = 26 * 53;

// Where the lines of the person of the picture are shown.
static const int kPart73TextX = 170;
static const int kPart73TextY = 50;

// The object that goes to the end of the inventory while the scene lasts.
static const int kPart73InventoryObject = 25;
static const int kPart73InventoryIndex = 60;

void IgorEngine::PART_73_DRAW_PATCH(int frame) {
	copyArea(_screenVGA, kPart73PatchOffset, 320, _animFramesBuffer + (frame - 1) * kPart73PatchFrameSize + kPart73PatchWidth,
			 kPart73PatchWidth, kPart73PatchWidth, kPart73PatchHeight);
}

void IgorEngine::PART_73_UPDATE_DIALOGUE(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_73_DRAW_PATCH(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_73_DRAW_PATCH(getRandomNumber(4) + 1);
		break;
	}
}

void IgorEngine::PART_73() {
	playMusic(8);
	_gameState.enableLight = 1;
	loadRoomData(PAL_Part73, IMG_Part73, BOX_Part73, MSK_Part73, TXT_Part73);
	// the colors of Igor
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	static const int anim[] = { ANM_Part73, 0 };
	loadAnimData(anim);
	memcpy(_screenVGA, _screenLayer1, 46080);
	memset(_screenVGA + 46080, 0, 17920);
	PART_73_DRAW_PATCH(1);
	WalkData *wd = &_walkData[0];
	wd->setPos(195, 107, 4, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 30;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 0;
	moveIgor(4, 0);
	fadeIn(768);
	_gameState.igorMoving = false;

	cutsceneSayWithCallback(kPart73TextX, kPart73TextY, 63, 14, 19, { { 201, 2, 928 } }, &IgorEngine::PART_73_UPDATE_DIALOGUE);
	igorSayAndWait({ { 203, 1, 929 }, { 204, 2, 930 } });
	cutsceneSayWithCallback(kPart73TextX, kPart73TextY, 63, 14, 19, { { 206, 2, 931 }, { 208, 2, 932 } },
							&IgorEngine::PART_73_UPDATE_DIALOGUE);
	igorSayAndWait({ { 210, 1, 933 }, { 211, 1, 934 } });

	--_walkDataLastIndex;
	buildWalkPath(195, 107, 319, 123);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();

	_objectsState[7] = 1;
	UPDATE_OBJECT_STATE(8);
	// the object is taken out of the inventory and put at its end
	removeInventoryEntry(kPart73InventoryIndex);
	++_inventoryInfo[73];
	_inventoryInfo[_inventoryInfo[73] - 1] = kPart73InventoryObject;
	_inventoryInfo[kPart73InventoryIndex] = _inventoryInfo[73];
	_inventoryInfo[72] = _inventoryOffsetTable[(_inventoryInfo[73] - 1) / 7];
	fadeOut(624);

	// the card with the time that has passed
	PART_MEANWHILE(IMG_Part73Card);
	PART_80();

	setupDefaultPalette();
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	_currentAction.verb = kVerbWalk;
	_objectsState[106] = 0;
	_currentPart = 40;
}

} // End of namespace Igor
