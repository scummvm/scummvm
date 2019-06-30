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
#ifndef SLUDGE_REGION_H
#define SLUDGE_REGION_H

#include "sludge/objtypes.h"
#include "sludge/freeze.h"

namespace Sludge {

struct ScreenRegion {
	int x1, y1, x2, y2, sX, sY, di;
	ObjectType *thisType;
};
typedef Common::List<ScreenRegion *> ScreenRegionList;

class RegionManager {
public:
	RegionManager(SludgeEngine *vm);
	~RegionManager();

	// Kill
	void kill();

	// Add & remove region
	bool addScreenRegion(int x1, int y1, int x2, int y2, int, int, int, int objectNum);
	void removeScreenRegion(int objectNum);

	// Save & load
	void loadRegions(Common::SeekableReadStream *stream);
	void saveRegions(Common::WriteStream *stream);

	// Draw
	void showBoxes();

	// Setter & getter
	ScreenRegion *getRegionForObject(int obj);
	ScreenRegion *getOverRegion() const { return _overRegion; }
	void setOverRegion(ScreenRegion *newRegion) { _overRegion = newRegion; }
	void updateOverRegion();
	bool isRegionChanged() const { return _lastRegion != _overRegion; }
	void updateLastRegion() { _lastRegion = _overRegion; }
	void resetOverRegion() { _overRegion = nullptr; }
	void resetLastRegion() { _lastRegion = nullptr; }

	// Freeze
	void freeze(FrozenStuffStruct *frozenStuff);
	void resotre(FrozenStuffStruct *frozenStuff);

private:
	SludgeEngine *_vm;

	ScreenRegionList *_allScreenRegions;
	ScreenRegion *_overRegion;
	ScreenRegion *_lastRegion;
};

} // End of namespace Sludge

#endif
