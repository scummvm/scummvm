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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sludge/allfiles.h"
#include "sludge/backdrop.h"
#include "sludge/event.h"
#include "sludge/graphics.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/region.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"

namespace Sludge {

RegionManager::RegionManager(SludgeEngine *vm)
{
	_vm = vm;
	_allScreenRegions = nullptr;
	_lastRegion = nullptr;
	_overRegion = nullptr;
}

RegionManager::~RegionManager() {
	kill();
}

void RegionManager::showBoxes() {
	ScreenRegion*huntRegion = _allScreenRegions;

	while (huntRegion) {
		g_sludge->_gfxMan->drawVerticalLine(huntRegion->x1, huntRegion->y1, huntRegion->y2);
		g_sludge->_gfxMan->drawVerticalLine(huntRegion->x2, huntRegion->y1, huntRegion->y2);
		g_sludge->_gfxMan->drawHorizontalLine(huntRegion->x1, huntRegion->y1, huntRegion->x2);
		g_sludge->_gfxMan->drawHorizontalLine(huntRegion->x1, huntRegion->y2, huntRegion->x2);
		huntRegion = huntRegion->next;
	}
}

void RegionManager::removeScreenRegion(int objectNum) {
	ScreenRegion **huntRegion = &_allScreenRegions;
	ScreenRegion *killMe;

	while (*huntRegion) {
		if ((*huntRegion)->thisType->objectNum == objectNum) {
			killMe = *huntRegion;
			*huntRegion = killMe->next;
			g_sludge->_objMan->removeObjectType(killMe->thisType);
			if (killMe == _overRegion)
				_overRegion = NULL;
			delete killMe;
			killMe = NULL;
		} else {
			huntRegion = &((*huntRegion)->next);
		}
	}
}

void RegionManager::saveRegions(Common::WriteStream *stream) {
	int numRegions = 0;
	ScreenRegion *thisRegion = _allScreenRegions;
	while (thisRegion) {
		thisRegion = thisRegion->next;
		numRegions++;
	}
	stream->writeUint16BE(numRegions);
	thisRegion = _allScreenRegions;
	while (thisRegion) {
		stream->writeUint16BE(thisRegion->x1);
		stream->writeUint16BE(thisRegion->y1);
		stream->writeUint16BE(thisRegion->x2);
		stream->writeUint16BE(thisRegion->y2);
		stream->writeUint16BE(thisRegion->sX);
		stream->writeUint16BE(thisRegion->sY);
		stream->writeUint16BE(thisRegion->di);
		g_sludge->_objMan->saveObjectRef(thisRegion->thisType, stream);

		thisRegion = thisRegion->next;
	}
}

void RegionManager::loadRegions(Common::SeekableReadStream *stream) {
	int numRegions = stream->readUint16BE();

	ScreenRegion *newRegion;
	ScreenRegion **pointy = &_allScreenRegions;

	while (numRegions--) {
		newRegion = new ScreenRegion;
		*pointy = newRegion;
		pointy = &(newRegion->next);

		newRegion->x1 = stream->readUint16BE();
		newRegion->y1 = stream->readUint16BE();
		newRegion->x2 = stream->readUint16BE();
		newRegion->y2 = stream->readUint16BE();
		newRegion->sX = stream->readUint16BE();
		newRegion->sY = stream->readUint16BE();
		newRegion->di = stream->readUint16BE();
		newRegion->thisType = g_sludge->_objMan->loadObjectRef(stream);
	}
	*pointy = NULL;
}

void RegionManager::kill() {
	ScreenRegion *killRegion;
	while (_allScreenRegions) {
		killRegion = _allScreenRegions;
		_allScreenRegions = _allScreenRegions->next;
		g_sludge->_objMan->removeObjectType(killRegion->thisType);
		delete killRegion;
	}
	_overRegion = nullptr;
	_lastRegion = nullptr;
}

bool RegionManager::addScreenRegion(int x1, int y1, int x2, int y2, int sX, int sY, int di,
		int objectNum) {
	ScreenRegion *newRegion = new ScreenRegion;
	if (!checkNew(newRegion))
		return false;
	newRegion->di = di;
	newRegion->x1 = x1;
	newRegion->y1 = y1;
	newRegion->x2 = x2;
	newRegion->y2 = y2;
	newRegion->sX = sX;
	newRegion->sY = sY;
	newRegion->thisType = g_sludge->_objMan->loadObjectType(objectNum);
	newRegion->next = _allScreenRegions;
	_allScreenRegions = newRegion;
	return (bool) (newRegion->thisType != NULL);
}

void RegionManager::updateOverRegion() {
	int cameraX = g_sludge->_gfxMan->getCamX();
	int cameraY = g_sludge->_gfxMan->getCamY();
	ScreenRegion *thisRegion = _allScreenRegions;
	while (thisRegion) {
		if ((g_sludge->_evtMan->mouseX() >= thisRegion->x1 - cameraX)
				&& (g_sludge->_evtMan->mouseY() >= thisRegion->y1 - cameraY)
				&& (g_sludge->_evtMan->mouseX() <= thisRegion->x2 - cameraX)
				&& (g_sludge->_evtMan->mouseY() <= thisRegion->y2 - cameraY)) {
			_overRegion = thisRegion;
			return;
		}
		thisRegion = thisRegion->next;
	}
	_overRegion = NULL;
	return;
}

ScreenRegion *RegionManager::getRegionForObject(int obj) {
	ScreenRegion *thisRegion = _allScreenRegions;

	while (thisRegion) {
		if (obj == thisRegion->thisType->objectNum) {
			return thisRegion;
		}
		thisRegion = thisRegion->next;
	}

	return NULL;
}

void RegionManager::freeze(FrozenStuffStruct *frozenStuff) {
	frozenStuff->allScreenRegions = _allScreenRegions;
	_allScreenRegions = nullptr;
	_overRegion = nullptr;
}

void RegionManager::resotre(FrozenStuffStruct *frozenStuff) {
	_allScreenRegions = frozenStuff->allScreenRegions;
	_overRegion = nullptr;
}

} // End of namespace Sludge
