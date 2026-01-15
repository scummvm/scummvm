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

#include "graphics/paletteman.h"

#include "pelrock.h"
#include "pelrock/actions.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

#define MASCULINE true
#define FEMININE false

const ActionEntry actionTable[] = {
	// Room 0
	{261, OPEN, &PelrockEngine::openRoomDrawer},
	{261, CLOSE, &PelrockEngine::closeRoomDrawer},
	{268, OPEN, &PelrockEngine::openRoomDoor},
	{268, CLOSE, &PelrockEngine::closeRoomDoor},
	{3, PICKUP, &PelrockEngine::pickUpPhoto},
	{0, PICKUP, &PelrockEngine::pickYellowBook}, // Generic pickup for other items
	// Room 1
	{4, PICKUP, &PelrockEngine::pickUpBrick}, // Brick
	// Room 2
	{282, OPEN, &PelrockEngine::openMcDoor},
	{282, CLOSE, &PelrockEngine::closeMcDoor},

	// Room 12
	{60, PICKUP, &PelrockEngine::grabKetchup},
	{61, PICKUP, &PelrockEngine::grabMustard},
	{62, PICKUP, &PelrockEngine::grabSpicey},
	{370, OPEN, &PelrockEngine::openKitchenDoor},
	{370, CLOSE, &PelrockEngine::closeKitchenDoor},

	// Room 13
	{375, OPEN, &PelrockEngine::openKitchenDrawer},
	{374, OPEN, &PelrockEngine::openKitchenDoorFromInside},

	// Generic handlers
	{WILDCARD, PICKUP, &PelrockEngine::noOpAction}, // Generic pickup action
	{WILDCARD, TALK, &PelrockEngine::noOpAction},   // Generic talk action
	{WILDCARD, WALK, &PelrockEngine::noOpAction},   // Generic walk action
	{WILDCARD, LOOK, &PelrockEngine::noOpAction},   // Generic look action
	{WILDCARD, PUSH, &PelrockEngine::noOpAction},   // Generic push action
	{WILDCARD, PULL, &PelrockEngine::noOpAction},   // Generic pull action
	{WILDCARD, OPEN, &PelrockEngine::noOpAction},   // Generic open action
	{WILDCARD, CLOSE, &PelrockEngine::noOpAction},  // Generic close action

	// End marker
	{WILDCARD, NO_ACTION, nullptr}};

const CombinationEntry combinationTable[] = {
	{2, 281, &PelrockEngine::useCardWithATM},           // Use ATM Card with ATM
	{62, 117, &PelrockEngine::useSpicySauceWithBurger}, // Use Spicy Sauce with Burger
	// End marker
	{WILDCARD, WILDCARD, nullptr}};

void PelrockEngine::openRoomDoor(HotSpot *hotspot) {
	openDoor(hotspot, 0, 93, FEMININE, true);
}

void PelrockEngine::openRoomDrawer(HotSpot *hotspot) {
	if (_room->hasSticker(91)) {
		_dialog->say(_res->_ingameTexts[YA_ABIERTO_M]);
		return;
	}
	_room->addSticker(91);
	// TODO: Check if we need to disable the hotspot
	if (_room->findHotspotByExtra(1)->isEnabled &&
		_room->findHotspotByExtra(2)->isEnabled &&
		_room->findHotspotByExtra(3)->isEnabled) {
		_room->disableHotspot(hotspot);
	}
}

void PelrockEngine::closeRoomDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 0, 93, FEMININE, true);
}

void PelrockEngine::pickUpPhoto(HotSpot *hotspot) {
	_room->enableHotspot(_room->findHotspotByExtra(261));
}

void PelrockEngine::pickYellowBook(HotSpot *hotspot) {
	_room->addSticker(95);
}

void PelrockEngine::pickUpBrick(HotSpot *hotspot) {
	_room->addSticker(133);
}

void PelrockEngine::closeRoomDrawer(HotSpot *hotspot) {
	_room->removeSticker(91);
	_room->enableHotspot(hotspot);
}

void PelrockEngine::useCardWithATM(int inventoryObject, HotSpot *hotspot) {
	debug("Withdrawing money from ATM using card (inv obj %d)", inventoryObject);
	if (_state->flagIsSet(FLAG_JEFE_INGRESA_PASTA)) {
		_state->setFlag(FLAG_JEFE_INGRESA_PASTA, false);
		addInventoryItem(75);
	} else {
		int billCount = 0;
		for (int i = 0; i < _state->inventoryItems.size(); i++) {
			if (_state->inventoryItems[i] == 5) {
				billCount++;
			}
		}
		if (billCount < 13) {
			addInventoryItem(5); // 1000 pesetas bill
			_dialog->say(_res->_ingameTexts[TEAPETECE_BUENRATO]);
		} else {
			_dialog->say(_res->_ingameTexts[NOMONEY_LEFT]);
		}
	}
}

void PelrockEngine::openMcDoor(HotSpot *hotspot) {
	openDoor(hotspot, 2, 7, FEMININE, false);
}

void PelrockEngine::closeMcDoor(HotSpot *hotspot) {
	// FIXME: Impossible to close right now
	closeDoor(hotspot, 2, 7, FEMININE, false);
}

void PelrockEngine::pickUpAndDisable(HotSpot *hotspot) {
	addInventoryItem(hotspot->extra);
	_room->disableHotspot(hotspot);
}

void PelrockEngine::grabKetchup(HotSpot *hotspot) {
	_room->addSticker(70);
}

void PelrockEngine::grabMustard(HotSpot *hotspot) {
	_room->addSticker(72);
}

void PelrockEngine::grabSpicey(HotSpot *hotspot) {
	_room->addSticker(71);
}

void PelrockEngine::openKitchenDoor(HotSpot *hotspot) {
	openDoor(hotspot, 1, 32, MASCULINE, true);
}

void PelrockEngine::closeKitchenDoor(HotSpot *HotSpot) {
	closeDoor(HotSpot, 1, 32, MASCULINE, true);
}

void PelrockEngine::openKitchenDrawer(HotSpot *hotspot) {
	if (!_state->flagIsSet(FLAG_JEFE_ENCARCELADO)) {
		_dialog->say(_res->_ingameTexts[QUITA_ESAS_MANOS]);
	} else {
		_room->addSticker(36);
		addInventoryItem(63); // Add recipe
		_dialog->say(_res->_ingameTexts[QUESESTO_RECETA]);
	}
}

void PelrockEngine::openKitchenDoorFromInside(HotSpot *hotspot) {
	openDoor(hotspot, 0, 34, MASCULINE, true);
}

void PelrockEngine::useSpicySauceWithBurger(int inventoryObject, HotSpot *hotspot) {
	_state->setFlag(FLAG_PUESTA_SALSA_PICANTE, true);
	_dialog->say(_res->_ingameTexts[VAESTAR_POCOFUERTE]);
}

void PelrockEngine::openDoor(HotSpot *hotspot, int exitIndex, int sticker, bool masculine, bool stayClosed) {
	if (_room->hasSticker(sticker)) {
		int text = masculine == MASCULINE ? YA_ABIERTO_M : ALREADY_OPENED_F;
		_dialog->say(_res->_ingameTexts[text]);
		return;
	}
	_room->enableExit(exitIndex, !stayClosed);
	_room->addSticker(sticker, !stayClosed);
}

void PelrockEngine::closeDoor(HotSpot *hotspot, int exitIndex, int sticker, bool masculine, bool stayOpen) {
	if (!_room->hasSticker(sticker)) {
		int text = masculine == MASCULINE ? ALREADY_CLOSED_M : ALREADY_CLOSED_F;
		_dialog->say(_res->_ingameTexts[text]);
		return;
	}
	_room->disableExit(exitIndex, !stayOpen);
	_room->removeSticker(sticker);
}

void PelrockEngine::addInventoryItem(int item) {
	if (_state->inventoryItems.size() == 0) {
		_state->selectedInventoryItem = item;
	}
	_flashingIcon = item;
	int frameCounter = 0;
	while (frameCounter < kIconFlashDuration) {
		_events->pollEvent();

		bool didRender = renderScene(OVERLAY_PICKUP_ICON);
		_screen->update();
		if (didRender) {
			frameCounter++;
		}
		g_system->delayMillis(10);
	}
	_state->addInventoryItem(item);
}

void PelrockEngine::dialogActionTrigger(uint16 actionTrigger, byte room, byte rootIndex) {
	if (actionTrigger == 328) {
		debug("Disabling root %d in room %d", rootIndex, room);
		_state->setRootDisabledState(room, rootIndex, true);
	}
}

void PelrockEngine::performActionTrigger(uint16 actionTrigger) {
	debug("Performing action trigger: %d", actionTrigger);
	switch (actionTrigger) {
	case 257:
		_sound->playMusicTrack(25);
		loadExtraScreenAndPresent(9);
		_dialog->say(_res->_ingameTexts[SOHOT]);
		_screen->markAllDirty();
		_screen->update();
		break;
	}
}

void PelrockEngine::noOpAction(HotSpot *hotspot) {
}

void PelrockEngine::noOpItem(int item, HotSpot *hotspot) {
	// 154 to 169
	debug("No-op item %d with hotspot %d", item, hotspot->extra);
	_alfredState.direction = ALFRED_DOWN;
	byte response = (byte)getRandomNumber(12);
	_dialog->say(_res->_ingameTexts[154 + response]);
}

void PelrockEngine::useOnAlfred(int inventoryObject) {

	debug("Using item %d on Alfred", inventoryObject);
	switch (inventoryObject) {
	case 63: // Recipe book
		_res->loadAlfredSpecialAnim(1);
		_alfredState.animState = ALFRED_SPECIAL_ANIM;
		waitForSpecialAnimation();
		debug("After special anim");
		loadExtraScreenAndPresent(3);
		debug("After extra screen");
		_dialog->say(_res->_ingameTexts[QUEASCO]);
		break;

	default:
		break;
	}
}

} // End of namespace Pelrock
