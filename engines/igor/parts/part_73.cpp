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

static const int kPart73PolicemanOffset = 55 * 320 + 157;
static const int kPart73PolicemanWidth = 26;
static const int kPart73PolicemanHeight = 52;
static const int kPart73PolicemanFrameSize = 26 * 53;

// Great dosicua
static const int kPart73InventoryIndex = 60;

void IgorEngine::part73DrawPoliceman(int frame) {
	copyArea(_screenVGA, kPart73PolicemanOffset, 320, _animFramesBuffer + (frame - 1) * kPart73PolicemanFrameSize + kPart73PolicemanWidth,
			 kPart73PolicemanWidth, kPart73PolicemanWidth, kPart73PolicemanHeight);
}

void IgorEngine::part73UpdateDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part73DrawPoliceman(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part73DrawPoliceman(getRandomNumber(4) + 1);
		break;
	}
}

void IgorEngine::part73Cutscene() {
	const bool savingWasAllowed = _canSaveGame;
	_canSaveGame = false;
	if (isCDVersion())
		playMusic(8);
	_gameState.enableLight = 1;
	loadRoomData(PAL_Part73, IMG_Part73, BOX_Part73, MSK_Part73, TXT_Part73);
	// the colors of Igor
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	static const int anim[] = { ANM_Part73, 0 };
	loadAnimData(anim);
	copyLayerToScreen(_screenLayer1Surface);
	fillScreenRows(144, 56, 0);
	part73DrawPoliceman(1);
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

	cutsceneSayWithCallback(170, 50, 63, 14, 19, { { 201, 2, 928 } }, &IgorEngine::part73UpdateDialogue);
	igorSayAndWait({ { 203, 1, 929 }, { 204, 2, 930 } });
	cutsceneSayWithCallback(170, 50, 63, 14, 19, { { 206, 2, 931 }, { 208, 2, 932 } }, &IgorEngine::part73UpdateDialogue);
	igorSayAndWait({ { 210, 1, 933 }, { 211, 1, 934 } });

	--_walkDataLastIndex;
	buildWalkPath(195, 107, 319, 123);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();

	_objectsState[kStatueDeliveredStateIndex] = 1;
	partMainUpdateObjectState(8);
	// removes the great dosicua
	removeInventoryEntry(kPart73InventoryIndex);
	++_inventoryInfo[kInventoryCount];
	_inventoryInfo[_inventoryInfo[kInventoryCount] - 1] = kInventoryObjectLizard;
	_inventoryInfo[kPart73InventoryIndex] = _inventoryInfo[kInventoryCount];
	_inventoryInfo[kInventoryFirstVisible] = _inventoryOffsetTable[(_inventoryInfo[kInventoryCount] - 1) / 7];
	fadeOut(624);

	// the card with the time that has passed
	partMainMeanwhile(IMG_Part73Card);
	// loads part80 now
	part80();

	setupDefaultPalette();
	setVerbPanelColors();
	setInventoryPanelColors();
	drawVerbsPanel();
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	_currentAction.verb = kVerbWalk;
	_objectsState[kStatuetteTaken2StateIndex] = 0;
	_currentPart = 40;
	_canSaveGame = savingWasAllowed;
}

} // End of namespace Igor
