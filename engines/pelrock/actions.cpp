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

#include "pelrock/actions.h"
#include "pelrock.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

const ActionEntry actionTable[] = {
	// Room 0
	{261, OPEN, &PelrockEngine::openDrawer},
	{261, CLOSE, &PelrockEngine::closeDrawer},
	{268, OPEN, &PelrockEngine::openDoor},
	{268, CLOSE, &PelrockEngine::closeDoor},
	{3, PICKUP, &PelrockEngine::pickUpPhoto},
	{0, PICKUP, &PelrockEngine::pickYellowBook}, // Generic pickup for other items
	{4, PICKUP, &PelrockEngine::pickUpBrick},    // Brick

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
	{2, 281, &PelrockEngine::useCardWithATM}, // Use ATM Card with ATM
	// End marker
	{WILDCARD, WILDCARD, nullptr}
};

	// Handler implementations
void PelrockEngine::openDoor(HotSpot *hotspot) {
	_room->addSticker(_res->getSticker(93), false);
	_room->enableExit(0, false);
}

void PelrockEngine::openDrawer(HotSpot *hotspot) {
	if (_room->hasSticker(91)) {
		_dialog->say(_res->_ingameTexts[YA_ABIERTO_M]);
		return;
	}
	_room->addSticker(_res->getSticker(91));
	// TODO: Check if we need to disable the hotspot
	if(_room->findHotspotByExtra(1)->isEnabled &&
	   _room->findHotspotByExtra(2)->isEnabled &&
	   _room->findHotspotByExtra(3)->isEnabled) {
	   _room->disableHotspot(hotspot);
	}
}

void PelrockEngine::closeDoor(HotSpot *hotspot) {
	_room->removeSticker(93);
	_room->disableExit(0, false);
}

void PelrockEngine::pickUpPhoto(HotSpot *hotspot) {
	_room->enableHotspot(_room->findHotspotByExtra(261));
}

void PelrockEngine::pickYellowBook(HotSpot *hotspot) {
	_room->addSticker(_res->getSticker(95));
}

void PelrockEngine::pickUpBrick(HotSpot *hotspot) {
	_room->addSticker(_res->getSticker(133));
}

void PelrockEngine::closeDrawer(HotSpot *hotspot) {
	_room->removeSticker(91);
	_room->enableHotspot(hotspot);
}


void PelrockEngine::useCardWithATM(int inventoryObject, HotSpot *hotspot) {
	debug("Withdrawing money from ATM using card (inv obj %d)", inventoryObject);
	if(_state.JEFE_INGRESA_PASTA) {
		_state.JEFE_INGRESA_PASTA = 0;
		addInventoryItem(75);
	}
	else {
		int billCount = 0;
		for(int i = 0; i < _state.inventoryItems.size(); i++) {
			if(_state.inventoryItems[i] == 5) {
				billCount++;
			}
		}
		if(billCount < 13) {
			addInventoryItem(5); // 1000 pesetas bill
			_dialog->say(_res->_ingameTexts[109]);
		}
		else {
			_dialog->say(_res->_ingameTexts[NOMONEY_LEFT]);
		}

	}
}

void PelrockEngine::pickUpAndDisable(HotSpot *hotspot) {
	addInventoryItem(hotspot->extra);
	_room->disableHotspot(hotspot);
}

void PelrockEngine::addInventoryItem(int item) {
	if (_state.inventoryItems.size() == 0) {
		_state.selectedInventoryItem = item;
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
	_state.addInventoryItem(item);
}


void PelrockEngine::dialogActionTrigger(uint16 actionTrigger, byte room, byte rootIndex) {
	if(actionTrigger == 328) {
		debug("Disabling root %d in room %d", rootIndex, room);
		_state.setRootDisabledState(room, rootIndex, true);
	}
}

void PelrockEngine::performActionTrigger(uint16 actionTrigger) {
	debug("Performing action trigger: %d", actionTrigger);
	switch (actionTrigger) {
	case 257:
		byte *palette = new byte[768];
		if (_extraScreen == nullptr) {
			_extraScreen = new byte[640 * 400];
		}
		_res->getExtraScreen(9, _extraScreen, palette);

		g_system->getPaletteManager()->setPalette(palette, 0, 256);
		extraScreenLoop();

		_dialog->say(_res->_ingameTexts[SOHOT]);
		_screen->markAllDirty();
		_screen->update();

		delete[] palette;
		break;
	}
}


void PelrockEngine::noOpAction(HotSpot *hotspot) {
	// Do nothing
}


void PelrockEngine::noOpItem(int item, HotSpot *hotspot) {
	// Do nothing
}

} // End of namespace Pelrock
