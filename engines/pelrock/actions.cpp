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

#include "pelrock/actions.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"
#include "pelrock.h"

namespace Pelrock {

const ActionEntry actionTable[] = {
	// Room 0
	{261, OPEN, &PelrockEngine::openDrawer},
	{261, CLOSE, &PelrockEngine::closeDrawer},
	{268, OPEN, &PelrockEngine::openDoor},
	{268, CLOSE, &PelrockEngine::closeDoor},
	{3, PICKUP, &PelrockEngine::pickUpPhoto},
	{0, PICKUP, &PelrockEngine::pickYellowBook}, // Generic pickup for other items
    {4, PICKUP, &PelrockEngine::pickUpBrick}, // Brick

	// Generic handlers
	{WILDCARD, PICKUP, &PelrockEngine::noOp}, // Generic pickup action
	{WILDCARD, TALK, &PelrockEngine::noOp},   // Generic talk action
	{WILDCARD, WALK, &PelrockEngine::noOp},   // Generic walk action
	{WILDCARD, LOOK, &PelrockEngine::noOp},   // Generic look action
	{WILDCARD, PUSH, &PelrockEngine::noOp},   // Generic push action
	{WILDCARD, PULL, &PelrockEngine::noOp},   // Generic pull action
	{WILDCARD, OPEN, &PelrockEngine::noOp},   // Generic open action
	{WILDCARD, CLOSE, &PelrockEngine::noOp},  // Generic close action

	// End marker
	{WILDCARD, NO_ACTION, nullptr}};

// Handler implementations
void PelrockEngine::openDrawer(HotSpot *hotspot) {
	if (_room->hasSticker(91)) {
		_dialog->say(_res->_ingameTexts[ALREADY_OPENED_M]);
		return;
	}
	_room->addSticker(_res->getSticker(91));
	hotspot->isEnabled = false;
}

void PelrockEngine::closeDrawer(HotSpot *hotspot) {
	_room->removeSticker(91);
	hotspot->isEnabled = true;
}

void PelrockEngine::openDoor(HotSpot *hotspot) {
	_room->addSticker(_res->getSticker(93));
	_room->_currentRoomExits[0].isEnabled = true;
}

void PelrockEngine::closeDoor(HotSpot *hotspot) {
	_room->removeSticker(93);
	_room->_currentRoomExits[0].isEnabled = false;
}

void PelrockEngine::pickUpAndDisable(HotSpot *hotspot) {
	if (_inventoryItems.size() == 0) {
		_selectedInventoryItem = hotspot->extra;
	}
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
	_inventoryItems.push_back(hotspot->extra);
	hotspot->isEnabled = false;
}

void PelrockEngine::pickUpPhoto(HotSpot *hotspot) {
	_room->findHotspotByExtra(261)->isEnabled = true;
}

void PelrockEngine::pickYellowBook(HotSpot *hotspot) {
	_room->addSticker(_res->getSticker(95));
}

void PelrockEngine::pickUpBrick(HotSpot *hotspot) {
    _room->addSticker(_res->getSticker(133));
}

void PelrockEngine::noOp(HotSpot *hotspot) {
	// Do nothing
}

} // End of namespace Pelrock
