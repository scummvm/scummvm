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

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"

namespace Prisoner {

/* Zones */

void PrisonerEngine::clearActorZone(int16 actorIndex) {
	for (int16 zoneIndex = 0; zoneIndex < kMaxZones; zoneIndex++) {
		Zone *zone = &_zones[zoneIndex];
		if (zone->used == 1 && zone->actorIndex == actorIndex) {
			clearZone(zoneIndex);
			clearZoneZoneActions(zoneIndex);
		}
	}
}

void PrisonerEngine::clearZone(int16 zoneIndex) {
	Zone *zone = &_zones[zoneIndex];
	zone->used = 0;
	if (zone->resourceCacheSlot != -1) {
		_res->unload(zone->resourceCacheSlot);
		zone->resourceCacheSlot = -1;
	}
}

void PrisonerEngine::removeZone(int16 zoneIndex) {
	clearZoneZoneActions(zoneIndex);
	clearZone(zoneIndex);
}

void PrisonerEngine::clearZones() {
	for (int16 zoneIndex = 0; zoneIndex < kMaxZones; zoneIndex++) {
		if (_zones[zoneIndex].used)
			clearZone(zoneIndex);
	}
}

int16 PrisonerEngine::addZone(int16 x1, int16 y1, int16 x2, int16 y2, int16 mouseCursor, Common::String &pakName,
	int16 pakSlot, Common::String &identifier) {

	int16 zoneIndex = _zones.getFreeSlot();
	Zone *zone = &_zones[zoneIndex];

	if (pakName.size() > 0) {
		zone->resourceCacheSlot = loadTextResource(pakName, pakSlot);
		TextResource *textResource = _res->get<TextResource>(zone->resourceCacheSlot);
		zone->textIndex = textResource->getIndex(identifier);
		zone->hasText = true;
	} else {
		zone->textIndex = 0;
		zone->hasText = false;
		zone->resourceCacheSlot = -1;
	}

	zone->used = 1;
	zone->x1 = x1;
	zone->y1 = y1;
	zone->x2 = x2;
	zone->y2 = y2;
	zone->type = -1;
	zone->actorIndex = -1;
	zone->mouseCursor = mouseCursor;
	zone->identifier = identifier;

	return zoneIndex;
}

int16 PrisonerEngine::addActorZone(int16 actorIndex, int16 mouseCursor, Common::String &pakName,
	int16 pakSlot, Common::String &identifier) {

	int16 zoneIndex = _zones.getFreeSlot();
	Zone *zone = &_zones[zoneIndex];
	ActorSprite *actorSprite = _actors[actorIndex].actorSprite;

	zone->resourceCacheSlot = loadTextResource(pakName, pakSlot);
	TextResource *textResource = _res->get<TextResource>(zone->resourceCacheSlot);
	zone->textIndex = textResource->getIndex(identifier);
	zone->hasText = true;

	zone->used = 1;
	zone->type = 1;
	zone->actorIndex = actorIndex;
	zone->x1 = actorSprite->x + actorSprite->boundsX1;
	zone->y1 = actorSprite->y + actorSprite->boundsY1;
	zone->x2 = actorSprite->x + actorSprite->boundsX2;
	zone->y2 = actorSprite->y + actorSprite->boundsY2;
	zone->mouseCursor = mouseCursor;
	zone->identifier = identifier;

	return zoneIndex;
}

int16 PrisonerEngine::addItemZone(int16 sceneItemIndex, int16 mouseCursor, Common::String &pakName,
	int16 pakSlot, Common::String &identifier) {

	int16 zoneIndex = _zones.getFreeSlot();
	Zone *zone = &_zones[zoneIndex];
	SceneItem *sceneItem = &_sceneItems[sceneItemIndex];
	ActorSprite *actorSprite = _actors[sceneItem->actorIndex].actorSprite;

	zone->resourceCacheSlot = loadTextResource(pakName, pakSlot);
	TextResource *textResource = _res->get<TextResource>(zone->resourceCacheSlot);
	zone->textIndex = textResource->getIndex(identifier);
	zone->hasText = true;

	zone->used = 1;
	zone->type = 2;
	zone->actorIndex = sceneItem->actorIndex;
	zone->x1 = actorSprite->x + actorSprite->boundsX1;
	zone->y1 = actorSprite->y + actorSprite->boundsY1;
	zone->x2 = actorSprite->x + actorSprite->boundsX2;
	zone->y2 = actorSprite->y + actorSprite->boundsY2;
	zone->mouseCursor = mouseCursor;
	zone->identifier = identifier;

	return zoneIndex;
}

void PrisonerEngine::drawZoneDescription(int16 zoneIndex) {
	Zone *zone = &_zones[zoneIndex];
	if (zone->hasText) {
		TextResource *textResource = _res->get<TextResource>(zone->resourceCacheSlot);
		// TODO: setFontColors(_textFont, fontColor5.outline, fontColor5.ink);
		setActiveFont(_textFont);
 		_screen->fillRect(0, 398, 639, 479, 0);
		drawTextEx(0, 639, 398, 479, textResource->getText(zone->textIndex)->getChunkLineString(0, 0));
		addDirtyRect(0, 398, 640, 82, 1);
		_zoneTextActive = true;
	} else {
		_screen->fillRect(0, 398, 639, 479, 0);
		addDirtyRect(0, 398, 640, 82, 1);
	}
}

void PrisonerEngine::updateZones(int16 x, int16 y) {

	bool zoneFound = false;
	bool zoneCursorSet = false;
	int16 newZoneIndex = -1;

	_zoneIndexAtMouse = -1;

	for (int16 zoneIndex = 0; zoneIndex < _zones.count(); zoneIndex++) {
		Zone *zone = &_zones[zoneIndex];
		if (zone->used == 1) {
			if (zone->type == 1) {
				ActorSprite *actorSprite = _actors[zone->actorIndex].actorSprite;
				zone->x1 = actorSprite->zoneX1;
				zone->y1 = actorSprite->zoneY1;
				zone->x2 = actorSprite->zoneX2;
				zone->y2 = actorSprite->zoneY2;
			}
			if (x >= zone->x1 && x <= zone->x2 && y >= zone->y1 && y <= zone->y2) {
				if (zone->actorIndex != _mainActorIndex || zone->actorIndex == -1) {
					if (!zoneFound) {
						zoneFound = true;
						drawZoneDescription(zoneIndex);
						_zoneIndexAtMouse = zoneIndex;
						if (zone->mouseCursor != -1) {
							zoneCursorSet = true;
							if (zone->mouseCursor != _currMouseCursor) {
								_zoneMouseCursorActive = true;
								_zoneMouseCursor = zone->mouseCursor;
							}
						}
					}
				} else {
					newZoneIndex = zoneIndex;
				}
			}
		}
	}

	if (_zoneIndexAtMouse == -1 && newZoneIndex != -1) {
		drawZoneDescription(newZoneIndex);
		_zoneIndexAtMouse = newZoneIndex;
	}

	if (_zoneIndexAtMouse == -1 && _zoneTextActive) {
		_zoneTextActive = false;
		_screen->fillRect(0, 398, 639, 479, 0);
		addDirtyRect(0, 398, 640, 82, 1);
	}

	if (!zoneCursorSet) {
		_zoneMouseCursorActive = false;
		_zoneMouseCursor = -1;
	}

}

void PrisonerEngine::checkQueuedZoneAction() {

	if (_queuedZoneAction.pathNodeIndex == -1 || _mainActorIndex == -1 ||
		(_actors[_mainActorIndex].pathNodeIndex == _queuedZoneAction.pathNodeIndex &&
		_actors[_mainActorIndex].status == 0)) {

		_newSceneIndex = _queuedZoneAction.sceneIndex;
		_newModuleIndex = _queuedZoneAction.moduleIndex;

		if (_queuedZoneAction.scriptIndex1 == -1) {
			_queuedZoneAction.used = 0;
			_queuedZoneAction.zoneActionIndex = -1;
			_exitZoneActionFlag = true;
		} else {
			debug("checkQueuedZoneAction() startScript(%d, %d)",
				_queuedZoneAction.scriptProgIndex, _queuedZoneAction.scriptIndex1);
			startScript(_queuedZoneAction.scriptProgIndex, _queuedZoneAction.scriptIndex1);
			_queuedZoneAction.used = 2;
			if (_newSceneIndex != -1 &&  _newModuleIndex != -1)
				setLeaveSceneScript(_queuedZoneAction.scriptProgIndex, _queuedZoneAction.scriptIndex1);
		}

		_queuedZoneAction.pathNodeIndex = -1;
	}

}

bool PrisonerEngine::isPointInZone(int16 x, int16 y, int16 zoneIndex) {
	Zone *zone = &_zones[zoneIndex];
	return x > zone->x1 && x < zone->x2 && y > zone->y1 && y < zone->y2;
}

/* ZoneActions */

int16 PrisonerEngine::updateZoneActions(int16 zoneActionType) {
	_currZoneActionIndex = -1;
	_inventoryItemIndex2 = -1;
	if (_zoneIndexAtMouse != -1) {
		for (int16 zoneActionIndex = 0; zoneActionIndex < kMaxZoneActions; zoneActionIndex++) {
			ZoneAction *zoneAction = &_zoneActions[zoneActionIndex];
			if (zoneAction->used && zoneAction->zoneIndex == _zoneIndexAtMouse) {
				if (zoneAction->type == zoneActionType) {
					if (zoneActionType != 9 || zoneAction->inventoryItemIndex == _inventoryItemCursor)
						return zoneActionIndex;
				} else if (zoneAction->scriptIndex2 != 0) {
					_currZoneActionIndex = zoneActionIndex;
					_inventoryItemIndex2 = zoneActionType;
				}
			}
		}
	}
	return -1;
}

int16 PrisonerEngine::addZoneAction(int16 zoneIndex, int16 type, int16 pathNodeIndex, int16 scriptIndex,
	int16 scriptProgIndex, int16 moduleIndex, int16 sceneIndex, int16 inventoryItemIndex) {

	int16 zoneActionIndex = _zoneActions.getFreeSlot();
	ZoneAction *zoneAction = &_zoneActions[zoneActionIndex];

	zoneAction->used = 1;
	zoneAction->zoneIndex = zoneIndex;
	zoneAction->type = type;
	zoneAction->inventoryItemIndex = inventoryItemIndex;
	zoneAction->pathNodeIndex = pathNodeIndex;
	zoneAction->scriptIndex1 = scriptIndex;
	zoneAction->scriptProgIndex = scriptProgIndex;
	zoneAction->moduleIndex = moduleIndex;
	zoneAction->sceneIndex = sceneIndex;
	zoneAction->scriptIndex2 = 0;
	zoneAction->unk2 = 0;

	return zoneActionIndex;
}

void PrisonerEngine::clearZoneZoneActions(int16 zoneIndex) {
	for (int16 zoneActionIndex = 0; zoneActionIndex < kMaxZoneActions; zoneActionIndex++) {
		ZoneAction *zoneAction = &_zoneActions[zoneActionIndex];
		if (zoneAction->used == 1 && zoneAction->zoneIndex == zoneIndex) {
			zoneAction->used = 0;
		}
	}
}

void PrisonerEngine::clearZoneAction(int16 zoneActionIndex) {
	_zoneActions[zoneActionIndex].used = 0;
}

void PrisonerEngine::clearZoneActions() {
	for (int16 zoneActionIndex = 0; zoneActionIndex < kMaxZoneActions; zoneActionIndex++) {
		clearZoneAction(zoneActionIndex);
	}
	_queuedZoneAction.used = 0;
}

void PrisonerEngine::setZoneActionScript(int16 zoneActionIndex, int16 scriptIndex) {
	_zoneActions[zoneActionIndex].scriptIndex2 = scriptIndex;
}

void PrisonerEngine::initZonesAndZoneActions() {
	_zones.clear();
	_zoneActions.clear();
}

int16 PrisonerEngine::checkZoneAction(int16 zoneActionType) {

	if (_queuedZoneAction.used == 2)
		return -1;

	int16 zoneActionIndex = updateZoneActions(zoneActionType);

	debug("zoneActionIndex = %d; zoneActionType = %d", zoneActionIndex, zoneActionType);

	_queuedZoneAction.used = 0;
	_queuedZoneAction.scriptIndex1 = -1;

	if (zoneActionIndex != -1) {
		ZoneAction *zoneAction = &_zoneActions[zoneActionIndex];
		_queuedZoneAction.zoneActionIndex = zoneActionIndex;
		_exitZoneActionFlag = 0;
		_queuedZoneAction.used = 1;
		_queuedZoneAction.zoneIndex = zoneAction->zoneIndex;
		_queuedZoneAction.type = zoneAction->type;
		_queuedZoneAction.inventoryItemIndex = zoneAction->inventoryItemIndex;
		_queuedZoneAction.pathNodeIndex = zoneAction->pathNodeIndex;
		_queuedZoneAction.scriptIndex1 = zoneAction->scriptIndex1;
		_queuedZoneAction.scriptProgIndex = zoneAction->scriptProgIndex;
		_queuedZoneAction.sceneIndex = zoneAction->sceneIndex;
		_queuedZoneAction.moduleIndex = zoneAction->moduleIndex;
		if (zoneAction->pathNodeIndex != -1 && _mainActorIndex != -1) {
			actorWalkToPathNode(_mainActorIndex, zoneAction->pathNodeIndex);
		}

		debug(
			"_queuedZoneAction: zoneIndex = %d; type = %d; pathNodeIndex = %d; scriptIndex1 = %d\n"
			"scriptProgIndex = %d; sceneIndex = %d; moduleIndex = %d",
			_queuedZoneAction.zoneIndex, _queuedZoneAction.type, _queuedZoneAction.pathNodeIndex,
			_queuedZoneAction.scriptIndex1, _queuedZoneAction.scriptProgIndex,
			_queuedZoneAction.sceneIndex, _queuedZoneAction.moduleIndex);

	} else if (_currZoneActionIndex != -1) {
		ZoneAction *zoneAction = &_zoneActions[_currZoneActionIndex];
		zoneActionIndex = _currZoneActionIndex;
		_queuedZoneAction.sceneIndex = -1;
		_queuedZoneAction.moduleIndex = -1;
		if (zoneAction->scriptIndex2 != 0) {
			_queuedZoneAction.zoneActionIndex = _currZoneActionIndex;
			if (zoneActionType != 9) {
				_inventoryItemIndex2 = -zoneActionType;
			} else {
				_inventoryItemIndex2 = _inventoryItemCursor;
			}
			_exitZoneActionFlag = 0;
			_queuedZoneAction.used = 1;
			_queuedZoneAction.zoneIndex = zoneAction->zoneIndex;
			_queuedZoneAction.type = zoneAction->type;
			_queuedZoneAction.inventoryItemIndex = zoneAction->inventoryItemIndex;
			_queuedZoneAction.pathNodeIndex = zoneAction->pathNodeIndex;
			_queuedZoneAction.scriptIndex1 = zoneAction->scriptIndex2;
			_queuedZoneAction.scriptProgIndex = zoneAction->scriptProgIndex;
			_queuedZoneAction.scriptIndex2 = 0;
			if (zoneAction->pathNodeIndex != -1 && _mainActorIndex != -1) {
				actorWalkToPathNode(_mainActorIndex, zoneAction->pathNodeIndex);
			}
		}
	}

	return zoneActionIndex;
}

} // End of namespace Prisoner
