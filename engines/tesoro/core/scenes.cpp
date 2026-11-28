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


#include "tesoro/core/scenes.h"
#include "tesoro/scripts.h"

namespace Tesoro {

namespace {

// The village and the beach scenes share the guard and the beach floor.
void beachOrVillage(SceneAdapter &h, bool toBeach) {
	if (toBeach)
		h.enterLevel(7, 40, -147);
	else
		h.enterLevel(9, 260, -71);
	h.rebindCharacters();
	h.setObjectActive(7, "ORZOGUEI", toBeach);
	if (toBeach)
		h.selectBank(7, "SUELO", 0, 1, 1, false, false);
	else
		h.selectBank(7, "SUELO", 0, 0, 0, false, false);
}

void opening(SceneAdapter &h, uint32 ordinal) {
	switch (ordinal) {
	case 0:
		h.setJimVisible(false);
		h.setJimActive(true);
		h.cameraToJim(false);
		break;
	case 3:
		h.setObjectVisible(1, "BARCO", true);
		h.selectBank(1, "BARCO", 0, 0, -1, true, false);
		break;
	case 4:
		h.setJimVisible(true);
		break;
	case 5:
		h.setObjectActive(1, "PUERTA1", true);
		h.setObjectVisible(1, "PUERTA1", false);
		h.enterLevel(0, 550, 100);
		break;
	default:
		break;
	}
}

void cabin(SceneAdapter &h, uint32 ordinal) {
	if (ordinal <= 5) {
		h.runDoorAction(4, "PUERTA1");
		return;
	}
	switch (ordinal) {
	case 6:
		h.setSortValue(4, "CAPITAN", 10);
		break;
	case 7:
		h.setObjectActive(5, "PUERTA2", false);
		h.selectBank(3, "CIELO", 1, 0, -1, true, false);
		h.setObjectActive(6, "DICK", false);
		h.setObjectActive(6, "LOROPOCHO", false);
		h.setObjectActive(6, "CUBO", false);
		h.fadeOut(4, 100);
		h.enterLevel(3, 435, 45);
		h.setJimVisible(false);
		h.cameraToJim(true);
		h.rebindCharacters();
		break;
	case 8:
		h.setJimVisible(true);
		break;
	case 9:
		h.fadeOut(4, 100);
		h.enterLevel(4, -250, 130);
		h.setJimPhase(1);
		h.setJimStep(2);
		break;
	default:
		break;
	}
}

} // End of anonymous namespace

bool SceneRoutines::run(SceneAdapter &h, uint32 type, uint32 ordinal) {
	switch (type) {
	case 1:
		opening(h, ordinal);
		return true;
	case 2:
		cabin(h, ordinal);
		return true;
	case 3:
		if (ordinal == 1) {
			h.enterLevel(3, 30, 125);
			h.cameraToJim(true);
			h.setJimVisible(false);
			h.rebindCharacters();
		} else if (ordinal == 2) {
			h.setJimPosition(435, 45);
			h.setJimVisible(true);
		}
		return true;
	case 4:
		if (ordinal <= 2) {
			h.fadeOut(4, 100);
			if (ordinal == 0)
				h.enterLevel(5, 135, 135);
			else
				h.enterLevel(4, -250, 130);
			h.setJimPhase(1);
		}
		return true;
	case 5:
		if (ordinal == 0) {
			h.enterLevelAtFloor(7);
			h.cameraToJim(true);
			h.setJimVisible(true);
			h.setJimPhase(0);
		}
		return true;
	case 6:
		if (ordinal == 0) {
			h.fadeOut(4, 100);
			h.enterLevel(9, 72, -151);
			h.setJimVisible(true);
			h.cameraToJim(true);
			h.rebindCharacters();
			h.resetRenderQueue();
		}
		return true;
	case 7:
		if (ordinal == 0) {
			h.enterLevelAtObject(9, "SALIDA");
			h.cameraToJim(true);
		}
		return true;
	case 9:
		if (ordinal == 1) {
			h.clearPointerItem();
			h.enterLevel(13, 0, 0);
			h.setJimVisible(false);
		}
		return true;
	case 10:
		if (ordinal == 0)
			beachOrVillage(h, true);
		else if (ordinal == 1 || ordinal == 2)
			h.fadeOut(4, 100);
		else if (ordinal == 3)
			beachOrVillage(h, false);
		return true;
	case 12:
		if (ordinal == 0)
			beachOrVillage(h, true);
		else if (ordinal == 1)
			beachOrVillage(h, false);
		return true;
	case 15:
		if (ordinal == 0) {
			h.fadeOut(4, 100);
			h.wait(300);
			h.enterLevel(23, -90, -127);
		}
		return true;
	case 16:
		if (ordinal == 0) {
			h.fadeOut(4, 100);
			h.wait(300);
			h.enterLevel(18, 130, 130);
		}
		return true;
	case 17:
		if (ordinal < 4)
			h.fadeOut(4, 100);
		return true;
	case 18:
		if (ordinal == 0) {
			h.fadeOut(4, 100);
			h.enterLevel(44, 138, -103);
			h.rebindCharacters();
		} else if (ordinal == 1) {
			h.fadeOut(4, 100);
			h.enterLevel(21, 1000, -95);
			h.setJimVisible(false);
			h.rebindCharacters();
		}
		return true;
	case 19:
		if (ordinal == 0) {
			h.enterLevel(44, -173, -63);
			h.rebindCharacters();
		} else if (ordinal == 1 || ordinal == 2) {
			h.fadeOut(4, 100);
			h.enterLevel(ordinal == 1 ? 45 : 46, 0, 0);
			h.setJimVisible(false);
			h.rebindCharacters();
		} else if (ordinal == 3) {
			h.fadeOut(10, 100);
		} else if (ordinal == 4) {
			h.fadeOut(4, 100);
			h.clearMainList();
		}
		return true;
	case 20:
		// The throne-room trips: to the throne room with Jim hidden, or back to the village with him shown.
		if (ordinal <= 1) {
			h.fadeOut(4, 100);
			h.enterLevelAtJim(ordinal == 0 ? 22 : 9);
			h.setJimVisible(ordinal != 0);
			h.cameraToJim(true);
			h.rebindCharacters();
			h.resetRenderQueue();
		}
		return true;
	case 21:
		if (ordinal == 0)
			h.runDoorAction(0, "PUERTA1");
		return true;
	default:
		return false;
	}
}

} // End of namespace Tesoro
