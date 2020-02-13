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
	_allScreenRegions = new ScreenRegionList;
	_allScreenRegions->clear();
	_lastRegion = nullptr;
	_overRegion = nullptr;
}

RegionManager::~RegionManager() {
	kill();

	delete _allScreenRegions;
	_allScreenRegions = nullptr;
}

void RegionManager::showBoxes() {
	for (ScreenRegionList::iterator it = _allScreenRegions->begin(); it != _allScreenRegions->end(); ++it) {
		g_sludge->_gfxMan->drawVerticalLine((*it)->x1, (*it)->y1, (*it)->y2);
		g_sludge->_gfxMan->drawVerticalLine((*it)->x2, (*it)->y1, (*it)->y2);
		g_sludge->_gfxMan->drawHorizontalLine((*it)->x1, (*it)->y1, (*it)->x2);
		g_sludge->_gfxMan->drawHorizontalLine((*it)->x1, (*it)->y2, (*it)->x2);
	}
}

void RegionManager::removeScreenRegion(int objectNum) {
	for (ScreenRegionList::iterator it = _allScreenRegions->begin(); it != _allScreenRegions->end(); ++it) {
		if ((*it)->thisType->objectNum == objectNum) {
			ScreenRegion *killMe = *it;
			g_sludge->_objMan->removeObjectType(killMe->thisType);
			if (killMe == _overRegion)
				_overRegion = nullptr;
			delete killMe;
			killMe = nullptr;
			it = _allScreenRegions->reverse_erase(it);
		}
	}
}

void RegionManager::saveRegions(Common::WriteStream *stream) {
	uint numRegions = _allScreenRegions->size();
	stream->writeUint16BE(numRegions);
	for (ScreenRegionList::iterator it = _allScreenRegions->begin(); it != _allScreenRegions->end(); ++it) {
		stream->writeUint16BE((*it)->x1);
		stream->writeUint16BE((*it)->y1);
		stream->writeUint16BE((*it)->x2);
		stream->writeUint16BE((*it)->y2);
		stream->writeUint16BE((*it)->sX);
		stream->writeUint16BE((*it)->sY);
		stream->writeUint16BE((*it)->di);
		g_sludge->_objMan->saveObjectRef((*it)->thisType, stream);
	}
}

void RegionManager::loadRegions(Common::SeekableReadStream *stream) {
	int numRegions = stream->readUint16BE();
	while (numRegions--) {
		ScreenRegion *newRegion = new ScreenRegion;
		_allScreenRegions->push_back(newRegion);
		newRegion->x1 = stream->readUint16BE();
		newRegion->y1 = stream->readUint16BE();
		newRegion->x2 = stream->readUint16BE();
		newRegion->y2 = stream->readUint16BE();
		newRegion->sX = stream->readUint16BE();
		newRegion->sY = stream->readUint16BE();
		newRegion->di = stream->readUint16BE();
		newRegion->thisType = g_sludge->_objMan->loadObjectRef(stream);
	}
}

void RegionManager::kill() {
	for (ScreenRegionList::iterator it = _allScreenRegions->begin(); it != _allScreenRegions->end(); ++it) {
		ScreenRegion *killRegion = (*it);
		g_sludge->_objMan->removeObjectType(killRegion->thisType);
		delete killRegion;
	}
	_allScreenRegions->clear();
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
	_allScreenRegions->push_front(newRegion);
	return (bool) (newRegion->thisType != nullptr);
}

void RegionManager::updateOverRegion() {
	int cameraX = g_sludge->_gfxMan->getCamX();
	int cameraY = g_sludge->_gfxMan->getCamY();
	for (ScreenRegionList::iterator it = _allScreenRegions->begin(); it != _allScreenRegions->end(); ++it) {
		if ((g_sludge->_evtMan->mouseX() >= (*it)->x1 - cameraX)
				&& (g_sludge->_evtMan->mouseY() >= (*it)->y1 - cameraY)
				&& (g_sludge->_evtMan->mouseX() <= (*it)->x2 - cameraX)
				&& (g_sludge->_evtMan->mouseY() <= (*it)->y2 - cameraY)) {
			_overRegion = (*it);
			return;
		}
	}
	_overRegion = nullptr;
	return;
}

ScreenRegion *RegionManager::getRegionForObject(int obj) {
	for (ScreenRegionList::iterator it = _allScreenRegions->begin(); it != _allScreenRegions->end(); ++it) {
		if (obj == (*it)->thisType->objectNum) {
			return (*it);
		}
	}

	return nullptr;
}

void RegionManager::freeze(FrozenStuffStruct *frozenStuff) {
	frozenStuff->allScreenRegions = _allScreenRegions;
	_allScreenRegions = new ScreenRegionList;
	_overRegion = nullptr;
}

void RegionManager::resotre(FrozenStuffStruct *frozenStuff) {
	// kill
	kill();
	delete _allScreenRegions;
	_allScreenRegions = nullptr;

	// restore
	_allScreenRegions = frozenStuff->allScreenRegions;
	_overRegion = nullptr;
}

} // End of namespace Sludge
