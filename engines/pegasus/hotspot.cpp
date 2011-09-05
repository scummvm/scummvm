/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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
 */

#include "pegasus/hotspot.h"

namespace Pegasus {

HotspotList g_allHotspots;

Hotspot::Hotspot(const tHotSpotID id) : IDObject(id) {
	_spotFlags = kNoHotSpotFlags;
	_spotActive = false;
}

Hotspot::~Hotspot() {
}

void Hotspot::setArea(const Common::Rect &area) {
	_spotArea = area;
}

void Hotspot::setArea(const tCoordType left, const tCoordType top, const tCoordType right, const tCoordType bottom) {
	_spotArea = Common::Rect(left, top, right, bottom);
}

void Hotspot::getBoundingBox(Common::Rect &r) const {
	r = _spotArea;
}

void Hotspot::getCenter(Common::Point &pt) const {
	pt.x = (_spotArea.left + _spotArea.right) / 2;
	pt.y = (_spotArea.top + _spotArea.bottom) / 2;
}

void Hotspot::getCenter(tCoordType &h, tCoordType &v) const {
	h = (_spotArea.left + _spotArea.right) / 2;
	v = (_spotArea.top + _spotArea.bottom) / 2;
}

void Hotspot::setActive() {
	_spotActive = true;
}

void Hotspot::setInactive() {
	_spotActive = false;
}

void Hotspot::setHotspotFlags(const tHotSpotFlags flags) {
	_spotFlags = flags;
}

void Hotspot::setMaskedHotspotFlags(const tHotSpotFlags flags, const tHotSpotFlags mask) {
	_spotFlags = (_spotFlags & ~mask) | flags;
}

bool Hotspot::isSpotActive() const {
	return _spotActive;
}

void Hotspot::moveSpotTo(const tCoordType h, const tCoordType v) {
	_spotArea.moveTo(h, v);
}

void Hotspot::moveSpotTo(const Common::Point pt) {
	_spotArea.moveTo(pt);
}

void Hotspot::moveSpot(const tCoordType h, const tCoordType v) {
	_spotArea.translate(h, v);
}

void Hotspot::moveSpot(const Common::Point pt) {
	_spotArea.translate(pt.x, pt.y);
}

bool Hotspot::pointInSpot(const Common::Point where) const {
	return _spotActive && _spotArea.contains(where);
}

tHotSpotFlags Hotspot::getHotspotFlags() const {
	return _spotFlags;
}

HotspotList::HotspotList() {
}

HotspotList::~HotspotList() {
	// TODO: Should this call deleteHotspots()?
}

void HotspotList::deleteHotspots() {
	for (HotspotIterator it = begin(); it != end(); it++)
		delete *it;

	clear();
}

Hotspot *HotspotList::findHotspot(const Common::Point where) {
	for (HotspotIterator it = begin(); it != end(); it++)
		if ((*it)->pointInSpot(where))
			return *it;

	return 0;
}

tHotSpotID HotspotList::findHotspotID(const Common::Point where) {
	Hotspot *hotspot = findHotspot(where);
	return hotspot ? hotspot->getObjectID() : kNoHotSpotID;
}

Hotspot *HotspotList::findHotspotByID(const tHotSpotID id) {
	for (HotspotIterator it = begin(); it != end(); it++)
		if ((*it)->getObjectID() == id)
			return *it;

	return 0;
}

Hotspot *HotspotList::findHotspotByMask(const tHotSpotFlags flags) {
	for (HotspotIterator it = begin(); it != end(); it++)
		if (((*it)->getHotspotFlags() & flags) == flags)
			return *it;

	return 0;
}

void HotspotList::activateMaskedHotspots(const tHotSpotFlags flags) {
	for (HotspotIterator it = begin(); it != end(); it++)
		if (flags == kNoHotSpotFlags || ((*it)->getHotspotFlags() & flags) != 0)
			(*it)->setActive();
}

void HotspotList::deactivateAllHotspots() {
	for (HotspotIterator it = begin(); it != end(); it++)
		(*it)->setInactive();
}

void HotspotList::deactivateMaskedHotspots(const tHotSpotFlags flags) {
	for (HotspotIterator it = begin(); it != end(); it++)
		if (((*it)->getHotspotFlags() & flags) != 0)
			(*it)->setInactive();
}

void HotspotList::activateOneHotspot(const tHotSpotID id) {
	for (HotspotIterator it = begin(); it != end(); it++) {
		if ((*it)->getObjectID() == id) {
			(*it)->setActive();
			return;
		}
	}
}

void HotspotList::deactivateOneHotspot(const tHotSpotID id) {
	for (HotspotIterator it = begin(); it != end(); it++) {
		if ((*it)->getObjectID() == id) {
			(*it)->setInactive();
			return;
		}
	}
}

void HotspotList::removeOneHotspot(const tHotSpotID id) {
	for (HotspotIterator it = begin(); it != end(); it++) {
		if ((*it)->getObjectID() == id) {
			erase(it);
			return;
		}
	}
}

void HotspotList::removeMaskedHotspots(const tHotSpotFlags flags) {
	if (flags != kNoHotSpotFlags) {
		for (HotspotIterator it = begin(); it != end(); ) {
			if (((*it)->getHotspotFlags() & flags) != 0)
				it = erase(it);
			else
				it++;
		}
	} else {
		clear();
	}
}

void HotspotList::setHotspotRect(const tHotSpotID id, const Common::Rect &r) {
	Hotspot *hotspot = findHotspotByID(id);
	if (hotspot)
		hotspot->setArea(r);
}

void HotspotList::getHotspotRect(const tHotSpotID id, Common::Rect &r) {
	Hotspot *hotspot = findHotspotByID(id);
	if (hotspot)
		hotspot->getBoundingBox(r);
}

} // End of namespace Pegasus
