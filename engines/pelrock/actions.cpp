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

	// Room 3
	{290, OPEN, &PelrockEngine::openShopDoor},
	{290, CLOSE, &PelrockEngine::closeShopDoor},
	{32, OPEN, &PelrockEngine::openLamppost},

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
	{4, 294, &PelrockEngine::useBrickWithWindow},       // Use Brick with Window (Room 3)
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
			_dialog->say(_res->_ingameTexts[NOTENGOMASDINERO]);
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

void PelrockEngine::openShopDoor(HotSpot *hotspot) {
	if (!_state->flagIsSet(FLAG_TIENDA_ABIERTA)) {
		_dialog->say(_res->_ingameTexts[TIENDA_CERRADA]);
		return;
	} else {
		openDoor(hotspot, 0, 13, MASCULINE, false);
	}
}

void PelrockEngine::closeShopDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 0, 13, MASCULINE, false);
}

void PelrockEngine::openLamppost(HotSpot *hotspot) {
	debug("Opening lamppost");
	_room->addSticker(14);
}

void PelrockEngine::useBrickWithWindow(int inventoryObject, HotSpot *hotspot) {
	debug("Using brick with window - sticker 11 will be added");

	// Check if window is already broken
	if (_room->hasSticker(11)) {
		// Window already broken, say something generic
		_alfredState.direction = ALFRED_UP;
		_dialog->say(_res->_ingameTexts[YA_ABIERTO_M]); // "It's already open/broken"
		return;
	}

	// TODO: Play Alfred's throwing animation
	// This would require adding a new special animation entry
	// _res->loadAlfredSpecialAnim(BRICK_THROW_ANIM);
	// _alfredState.animState = ALFRED_SPECIAL_ANIM;
	// waitForSpecialAnimation();

	// TODO: Animate sprite 8 (brick projectile) moving to window
	Sprite *brickSprite = _room->findSpriteByIndex(7);
	HotSpot *windowHotspot = _room->findHotspotByExtra(294);
	brickSprite->x = _alfredState.x - brickSprite->w / 2;
	brickSprite->y = _alfredState.y - kAlfredFrameHeight;
	brickSprite->zOrder = 20; // Make it visible
	int target = windowHotspot->y + windowHotspot->h / 2;
	while (!shouldQuit()) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		if (_chrono->_gameTick) {
			_room->findSpriteByIndex(7)->y -= 40;
			if (_room->findSpriteByIndex(7)->y < target) {
				_room->findSpriteByIndex(7)->zOrder = -1;
				break;
			}
		}
		_screen->update();
		g_system->delayMillis(10);
	}
	// This would involve loading and animating the room sprite

	// Add the broken window sticker
	_room->addSticker(11);
	_sound->playSound(_room->_roomSfx[2]); // Play glass breaking sound

	// Remove brick from inventory
	_state->removeInventoryItem(4);

	int16 x = 639; // put at the very edge of the screen
	int16 y = windowHotspot->y;
	// Play the NPC dialog sequence
	int16 dialog1y = y + 22;
	int16 dialog2y = dialog1y + 10 + _largeFont->getFontHeight();
	_dialog->say(_res->_ingameTexts[QUEHASIDOESO], x, dialog1y);
	_dialog->say(_res->_ingameTexts[QUIENANDAAHI], x, dialog2y);
	_dialog->say(_res->_ingameTexts[YOMEVOY]);

	_state->setFlag(FLAG_TIENDA_ABIERTA, true);
	_room->onlyPersistSticker(_room->_currentRoomNumber, 9);
	_room->onlyPersistSticker(_room->_currentRoomNumber, 10);
	_room->disableHotspot(_room->findHotspotByExtra(295)); // Disable storefront hotspot
	_room->disableHotspot(_room->findHotspotByExtra(294)); // Disable window hotspot
	walkTo(639, _alfredState.y);
}

void PelrockEngine::openDoor(HotSpot *hotspot, int exitIndex, int sticker, bool masculine, bool stayClosed) {
	if (_room->hasSticker(sticker)) {
		int text = masculine == MASCULINE ? YA_ABIERTO_M : YA_ABIERTA_F;
		_dialog->say(_res->_ingameTexts[text]);
		return;
	}
	_room->enableExit(exitIndex, !stayClosed);
	_room->addSticker(sticker, !stayClosed);
}

void PelrockEngine::closeDoor(HotSpot *hotspot, int exitIndex, int sticker, bool masculine, bool stayOpen) {
	if (!_room->hasSticker(sticker)) {
		int text = masculine == MASCULINE ? YA_CERRADO_M : YA_CERRADA_F;
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
		_dialog->say(_res->_ingameTexts[QUEBUENA_ESTA]);
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

		loadExtraScreenAndPresent(3);
		debug("After extra screen");
		_dialog->say(_res->_ingameTexts[QUEASCO]);
		break;
	case 0: // yellow book
		_res->loadAlfredSpecialAnim(2);
		_alfredState.animState = ALFRED_SPECIAL_ANIM;
		waitForSpecialAnimation();
		_dialog->say(_res->_ingameTexts[CUENTOPARECIDO]);
		break;
	default:
		break;
	}
}

} // End of namespace Pelrock
