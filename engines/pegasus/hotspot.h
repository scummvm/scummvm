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

#ifndef PEGASUS_HOTSPOT_H
#define PEGASUS_HOTSPOT_H

#include "common/list.h"
#include "common/rect.h"

#include "pegasus/constants.h"
#include "pegasus/types.h"
#include "pegasus/util.h"

/*

	Hot spots combine a pixel area, an ID value and an active flag.
	
	A point is considered in a hot spot if the point is in the hot spot's pixel area and
	the active flag is set.
	
	In addition, hot spots have a 32 bit word of bit flags for filtering use.

*/

namespace Pegasus {

class Hotspot : public IDObject {
public:
	Hotspot(const tHotSpotID);
	virtual ~Hotspot();
	
	void setArea(const Common::Rect &);
	void setArea(const tCoordType, const tCoordType, const tCoordType, const tCoordType);
	void getBoundingBox(Common::Rect &) const;
	void getCenter(Common::Point&) const;
	void getCenter(tCoordType&, tCoordType&) const;
	
	void moveSpotTo(const tCoordType, const tCoordType);
	void moveSpotTo(const Common::Point);
	void moveSpot(const tCoordType, const tCoordType);
	void moveSpot(const Common::Point);
	
	bool pointInSpot(const Common::Point) const;
	
	void setActive();
	void setInactive();
	bool isSpotActive() const;
	
	tHotSpotFlags getHotspotFlags() const;
	void setHotspotFlags(const tHotSpotFlags);
	void setMaskedHotspotFlags(const tHotSpotFlags flags, const tHotSpotFlags mask);

protected:
	Common::Rect _spotArea;
	tHotSpotFlags _spotFlags;
	bool _spotActive;
};

class HotspotList : public Common::List<Hotspot *> {
public:
	HotspotList();
	virtual ~HotspotList();

	void deleteHotspots();

	Hotspot *findHotspot(const Common::Point);
	tHotSpotID findHotspotID(const Common::Point);
	Hotspot * findHotspotByID(const tHotSpotID);
	Hotspot * findHotspotByMask(const tHotSpotFlags);

	void activateMaskedHotspots(const tHotSpotFlags = kNoHotSpotFlags);
	void deactivateAllHotspots();
	void deactivateMaskedHotspots(const tHotSpotFlags);

	void activateOneHotspot(const tHotSpotID);
	void deactivateOneHotspot(const tHotSpotID);
	
	void removeOneHotspot(const tHotSpotID);
	void removeMaskedHotspots(const tHotSpotFlags = kNoHotSpotFlags);

	void setHotspotRect(const tHotSpotID, const Common::Rect&);
	void getHotspotRect(const tHotSpotID, Common::Rect&);
};

typedef HotspotList::iterator HotspotIterator;

// FIXME: Remove global construction
extern HotspotList g_allHotspots;

} // End of namespace Pegasus

#endif
