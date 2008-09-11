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
 * $URL$
 * $Id$
 *
 */

#include "drascula/drascula.h"

namespace Drascula {

void DrasculaEngine::pickObject(int object) {
	if (currentChapter == 6)
		loadPic("iconsp.alg", backSurface);
	else if (currentChapter == 4)
		loadPic("icons2.alg", backSurface);
	else if (currentChapter == 5)
		loadPic("icons3.alg", backSurface);
	else
		loadPic("icons.alg", backSurface);
	chooseObject(object);
	if (currentChapter == 2)
		loadPic(menuBackground, backSurface);
	else
		loadPic(99, backSurface);
}

void DrasculaEngine::chooseObject(int object) {
	if (currentChapter == 5) {
		if (takeObject == 1 && menuScreen == 0 && pickedObject != 16)
			addObject(pickedObject);
	} else {
		if (takeObject == 1 && menuScreen == 0)
			addObject(pickedObject);
	}
	for (int i = 0; i < OBJHEIGHT; i++)
		memcpy(mouseCursor + i * OBJWIDTH, backSurface + _x1d_menu[object] + (_y1d_menu[object] + i) * 320, OBJWIDTH);
	setCursor(kCursorCurrentItem);
	takeObject = 1;
	pickedObject = object;
}

int DrasculaEngine::removeObject(int obj) {
	int result = 1;

	for (int h = 1; h < 43; h++) {
		if (inventoryObjects[h] == obj) {
			inventoryObjects[h] = 0;
			result = 0;
			break;
		}
	}

	return result;
}

void DrasculaEngine::gotoObject(int pointX, int pointY) {
	if (currentChapter == 5 || currentChapter == 6) {
		if (hare_se_ve == 0) {
			curX = roomX;
			curY = roomY;
			updateRoom();
			updateScreen();
			return;
		}
	}
	roomX = pointX;
	roomY = pointY;
	startWalking();

	for (;;) {
		updateRoom();
		updateScreen();
		if (characterMoved == 0)
			break;
	}

	if (walkToObject == 1) {
		walkToObject = 0;
		trackProtagonist = trackFinal;
	}
	updateRoom();
	updateScreen();
}

void DrasculaEngine::checkObjects() {
	int l, veo = 0;

	for (l = 0; l < numRoomObjs; l++) {
		if (mouseX > x1[l] && mouseY > y1[l]
				&& mouseX < x2[l] && mouseY < y2[l]
				&& visible[l] == 1 && isDoor[l] == 0) {
			strcpy(textName, objName[l]);
			hasName = 1;
			veo = 1;
		}
	}

	if (mouseX > curX + 2 && mouseY > curY + 2
			&& mouseX < curX + curWidth - 2 && mouseY < curY + curHeight - 2) {
		if (currentChapter == 2 || veo == 0) {
			strcpy(textName, "hacker");
			hasName = 1;
			veo = 1;
		}
	}

	if (veo == 0)
		hasName = 0;
}

void DrasculaEngine::removeObject() {
	int h = 0, n;

	updateRoom();

	for (n = 1; n < 43; n++){
		if (whichObject() == n) {
			h = inventoryObjects[n];
			inventoryObjects[n] = 0;
			if (h != 0)
				takeObject = 1;
		}
	}

	updateEvents();

	if (takeObject == 1)
		chooseObject(h);
}

bool DrasculaEngine::pickupObject() {
	int h = pickedObject;
	checkFlags = 1;

	updateRoom();
	showMenu();
	updateScreen();

	// Objects with an ID smaller than 7 are the inventory verbs
	if (pickedObject >= 7) {
		for (int n = 1; n < 43; n++) {
			if (whichObject() == n && inventoryObjects[n] == 0) {
				inventoryObjects[n] = h;
				takeObject = 0;
				checkFlags = 0;
			}
		}
	}

	if (checkFlags == 1) {
		if (checkMenuFlags())
			return true;
	}
	updateEvents();
	if (takeObject == 0)
		selectVerb(0);

	return false;
}

void DrasculaEngine::addObject(int obj) {
	int h, position = 0;

	for (h = 1; h < 43; h++) {
		if (inventoryObjects[h] == obj)
			position = 1;
	}

	if (position == 0) {
		for (h = 1; h < 43; h++) {
			if (inventoryObjects[h] == 0) {
				inventoryObjects[h] = obj;
				position = 1;
				break;
			}
		}
	}
}

int DrasculaEngine::whichObject() {
	int n = 0;

	for (n = 1; n < 43; n++) {
		if (mouseX > _itemLocations[n].x && mouseY > _itemLocations[n].y &&
			mouseX < _itemLocations[n].x + OBJWIDTH && 
			mouseY < _itemLocations[n].y + OBJHEIGHT)
			break;
	}

	return n;
}

void DrasculaEngine::updateVisible() {
	if (currentChapter == 1) {
		// nothing
	} else if (currentChapter == 2) {
		if (roomNumber == 2 && flags[40] == 0)
			visible[3] = 0;
		else if (roomNumber == 3 && flags[3] == 1)
			visible[8] = 0;
		else if (roomNumber == 6 && flags[1] == 1 && flags[10] == 0) {
			visible[2] = 0;
			visible[4] = 1;
		} else if (roomNumber == 7 && flags[35] == 1)
			visible[3] = 0;
		else if (roomNumber == 14 && flags[5] == 1)
			visible[4] = 0;
		else if (roomNumber == 18 && flags[28] == 1)
			visible[2] = 0;
	} else if (currentChapter == 3) {
		// nothing
	} else if (currentChapter == 4) {
		if (roomNumber == 23 && flags[0] == 0 && flags[11] == 0)
			visible[2] = 1;
		if (roomNumber == 23 && flags[0] == 1 && flags[11] == 0)
			visible[2] = 0;
		if (roomNumber == 21 && flags[10] == 1)
			visible[2] = 0;
		if (roomNumber == 22 && flags[26] == 1) {
			visible[2] = 0;
			visible[1] = 1;
		}
		if (roomNumber == 22 && flags[27] == 1)
			visible[3] = 0;
		if (roomNumber == 26 && flags[21] == 0)
			strcpy(objName[2], _textmisc[0]);
		if (roomNumber == 26 && flags[18] == 1)
			visible[2] = 0;
		if (roomNumber == 26 && flags[12] == 1)
			visible[1] = 0;
		if (roomNumber == 35 && flags[14] == 1)
			visible[2] = 0;
		if (roomNumber == 35 && flags[17] == 1)
			visible[3] = 1;
		if (roomNumber == 35 && flags[15] == 1)
			visible[1] = 0;
	} else if (currentChapter == 5) {
		if (roomNumber == 49 && flags[6] == 1)
			visible[2] = 0;
		if (roomNumber == 49 && flags[6] == 0)
			visible[1] = 0;
		if (roomNumber == 49 && flags[6] == 1)
			visible[1] = 1;
		if (roomNumber == 45 && flags[6] == 1)
			visible[3] = 1;
		if (roomNumber == 53 && flags[2] == 1)
			visible[3] = 0;
		if (roomNumber == 54 && flags[13] == 1)
			visible[3] = 0;
		if (roomNumber == 55 && flags[8] == 1)
			visible[1] = 0;
	} else if (currentChapter == 6) {
		if (roomNumber == 58 && flags[8] == 0)
			isDoor[1] = 0;
		if (roomNumber == 58 && flags[8] == 1)
			isDoor[1] = 1;
		if (roomNumber == 59)
			isDoor[1] = 0;
		if (roomNumber == 60) {
			trackDrascula = 0;
			drasculaX = 155;
			drasculaY = 69;
		}
	}
}

} // End of namespace Drascula
