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

#ifndef SCI_GRAPHICS_COMPARE_H
#define SCI_GRAPHICS_COMPARE_H

#include "common/hashmap.h"

namespace Sci {

class Screen;

/**
 * Compare class, handles compare operations graphic-wise (like when checking control screen for a pattern etc.)
 */
class GfxCompare {
public:
	GfxCompare(SegManager *segMan, GfxCache *cache, GfxScreen *screen, GfxCoordAdjuster16 *coordAdjuster);
	~GfxCompare();

	uint16 kernelOnControl(byte screenMask, const Common::Rect &rect);
	void kernelSetNowSeen(reg_t objectReference);
	reg_t kernelCanBeHere(reg_t curObject, reg_t listReference);
	reg_t kernelCantBeHere32(const reg_t curObject, const reg_t listReference) const;
	bool kernelIsItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position);
	void kernelBaseSetter(reg_t object);
	Common::Rect getNSRect(reg_t object);
	void setNSRect(reg_t object, Common::Rect nsRect);

private:
	SegManager *_segMan;
	GfxCache *_cache;
	GfxScreen *_screen;
	GfxCoordAdjuster16 *_coordAdjuster;

	uint16 isOnControl(uint16 screenMask, const Common::Rect &rect);

	/**
	 * This function checks whether any of the objects in the given list,
	 * *different* from checkObject, has a brRect which is contained inside
	 * checkRect.
	 */
	reg_t canBeHereCheckRectList(const reg_t checkObject, const Common::Rect &checkRect, const List *list, const uint16 signalFlags) const;
};

} // End of namespace Sci

#endif // SCI_GRAPHICS_COMPARE_H
