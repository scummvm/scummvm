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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/frameout.h"

namespace Sci {

GfxFrameout::GfxFrameout(SegManager *segMan, ResourceManager *resMan, GfxCache *cache, GfxScreen *screen, GfxPalette *palette)
	: _segMan(segMan), _resMan(resMan), _cache(cache), _screen(screen), _palette(palette) {
}

GfxFrameout::~GfxFrameout() {
}

void GfxFrameout::kernelAddPlane(reg_t object) {
	_planes.push_back(object);
	int16 planePri = GET_SEL32V(_segMan, object, priority) & 0xFFFF;
	if (planePri > _highPlanePri)
		_highPlanePri = planePri;
}

void GfxFrameout::kernelUpdatePlane(reg_t object) {
}

void GfxFrameout::kernelDeletePlane(reg_t object) {
	for (uint32 planeNr = 0; planeNr < _planes.size(); planeNr++) {
		if (_planes[planeNr] == object) {
			_planes.remove_at(planeNr);
			break;
		}
	}

	// Recalculate highPlanePri
	_highPlanePri = 0;

	for (uint32 planeNr = 0; planeNr < _planes.size(); planeNr++) {
		int16 planePri = GET_SEL32V(_segMan, _planes[planeNr], priority) & 0xFFFF;
		if (planePri > _highPlanePri)
			_highPlanePri = planePri;
	}
}

void GfxFrameout::kernelAddScreenItem(reg_t object) {
	_screenItems.push_back(object);
	warning("addScreenItem %X:%X (%s)", object.segment, object.offset, _segMan->getObjectName(object));
}

void GfxFrameout::kernelDeleteScreenItem(reg_t object) {
	for (uint32 itemNr = 0; itemNr < _screenItems.size(); itemNr++) {
		if (_screenItems[itemNr] == object) {
			_screenItems.remove_at(itemNr);
			return;
		}
	}
}

int16 GfxFrameout::kernelGetHighPlanePri() {
	return _highPlanePri;
}

bool sortHelper(const FrameoutEntry* entry1, const FrameoutEntry* entry2) {
	return (entry1->priority == entry2->priority) ? (entry1->y < entry2->y) : (entry1->priority < entry2->priority);
}

void GfxFrameout::kernelFrameout() {
	int16 itemCount = 0;
	reg_t planeObject;
	GuiResourceId planePictureNr;
	SciGuiPicture *planePicture = 0;
	int16 planePictureCels = 0;
	int16 planePictureCel;
	int16 planePriority;
	int16 planeTop, planeLeft;

	reg_t itemObject;
	reg_t itemPlane;

	FrameoutEntry *itemData;
	FrameoutList itemList;
	FrameoutEntry *itemEntry;

	// Allocate enough space for all screen items
	itemData = (FrameoutEntry *)malloc(_screenItems.size() * sizeof(FrameoutEntry));

	for (uint32 planeNr = 0; planeNr < _planes.size(); planeNr++) {
		planeObject = _planes[planeNr];
		planePriority = GET_SEL32V(_segMan, planeObject, priority);

		if (planePriority == -1) // Plane currently not meant to be shown
			continue;

		planePictureNr = GET_SEL32V(_segMan, planeObject, picture);
		if ((planePictureNr != 0xFFFF) && (planePictureNr != 0xFFFE)) {
			planePicture = new SciGuiPicture(_resMan, 0, _screen, _palette, planePictureNr, false);
			planePictureCels = planePicture->getSci32celCount();
		}

		planeTop = GET_SEL32V(_segMan, planeObject, top);
		planeLeft = GET_SEL32V(_segMan, planeObject, left);

		// Fill our itemlist for this plane
		itemCount = 0;
		itemEntry = itemData;
		for (uint32 itemNr = 0; itemNr < _screenItems.size(); itemNr++) {
			itemObject = _screenItems[itemNr];
			itemPlane = GET_SEL32(_segMan, itemObject, plane);
			if (planeObject == itemPlane) {
				// Found an item on current plane
				itemEntry->viewId = GET_SEL32V(_segMan, itemObject, view);
				itemEntry->loopNo = GET_SEL32V(_segMan, itemObject, loop);
				itemEntry->celNo = GET_SEL32V(_segMan, itemObject, cel);
				itemEntry->x = GET_SEL32V(_segMan, itemObject, x);
				itemEntry->y = GET_SEL32V(_segMan, itemObject, y);
				itemEntry->z = GET_SEL32V(_segMan, itemObject, z);
				itemEntry->priority = GET_SEL32V(_segMan, itemObject, priority);
				itemEntry->scaleX = GET_SEL32V(_segMan, itemObject, scaleX);
				itemEntry->scaleY = GET_SEL32V(_segMan, itemObject, scaleY);

				itemEntry->x += planeLeft;
				itemEntry->y += planeTop;

				itemList.push_back(itemEntry);
				itemEntry++;
				itemCount++;
			}
		}

		// Now sort our itemlist
		Common::sort(itemList.begin(), itemList.end(), sortHelper);

		// Now display itemlist
		planePictureCel = 0;

		itemEntry = itemData;
		FrameoutList::iterator listIterator = itemList.begin();
		FrameoutList::iterator listEnd = itemList.end();
		while (listIterator != listEnd) {
			itemEntry = *listIterator;
			if (planePicture) {
				while ((planePictureCel <= itemEntry->priority) && (planePictureCel < planePictureCels)) {
					planePicture->drawSci32Vga(planePictureCel);
					planePictureCel++;
				}
			}
			if (itemEntry->viewId != 0xFFFF) {
				View *view = _cache->getView(itemEntry->viewId);

				if ((itemEntry->scaleX == 128) && (itemEntry->scaleY == 128))
					view->getCelRect(itemEntry->loopNo, itemEntry->celNo, itemEntry->x, itemEntry->y, itemEntry->z, &itemEntry->celRect);
				else
					view->getCelScaledRect(itemEntry->loopNo, itemEntry->celNo, itemEntry->x, itemEntry->y, itemEntry->z, itemEntry->scaleX, itemEntry->scaleY, &itemEntry->celRect);

				if (itemEntry->celRect.top < 0 || itemEntry->celRect.top >= _screen->getHeight()) {
					listIterator++;
					continue;
				}

				if (itemEntry->celRect.left < 0 || itemEntry->celRect.left >= _screen->getWidth()) {
					listIterator++;
					continue;
				}

				if ((itemEntry->scaleX == 128) && (itemEntry->scaleY == 128))
					view->draw(itemEntry->celRect, itemEntry->celRect, itemEntry->celRect, itemEntry->loopNo, itemEntry->celNo, 255, 0, false);
				else
					view->drawScaled(itemEntry->celRect, itemEntry->celRect, itemEntry->celRect, itemEntry->loopNo, itemEntry->celNo, 255, itemEntry->scaleX, itemEntry->scaleY);
			}
			listIterator++;
		}
		if (planePicture) {
			while (planePictureCel < planePictureCels) {
				planePicture->drawSci32Vga(planePictureCel);
				planePictureCel++;
			}
			delete planePicture;
			planePicture = 0;
		}
	}
	free(itemData);
}

} // End of namespace Sci
