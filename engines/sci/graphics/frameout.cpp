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
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/font.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/frameout.h"

namespace Sci {

GfxFrameout::GfxFrameout(SegManager *segMan, ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxCache *cache, GfxScreen *screen, GfxPalette *palette, GfxPaint32 *paint32)
	: _segMan(segMan), _resMan(resMan), _cache(cache), _screen(screen), _palette(palette), _paint32(paint32) {

	_coordAdjuster = (GfxCoordAdjuster32 *)coordAdjuster;
	scriptsRunningWidth = 320;
	scriptsRunningHeight = 200;
}

GfxFrameout::~GfxFrameout() {
}

void GfxFrameout::kernelAddPlane(reg_t object) {
	PlaneEntry newPlane;

	if (_planes.empty()) {
		// There has to be another way for sierra sci to do this or maybe script resolution is compiled into
		//  interpreter (TODO)
		scriptsRunningHeight = readSelectorValue(_segMan, object, SELECTOR(resY));
		scriptsRunningWidth = readSelectorValue(_segMan, object, SELECTOR(resX));
		_coordAdjuster->setScriptsResolution(scriptsRunningWidth, scriptsRunningHeight);
	}

	newPlane.object = object;
	newPlane.pictureId = 0xFFFF;
	newPlane.picture = NULL;
	newPlane.lastPriority = 0xFFFF; // hidden
	_planes.push_back(newPlane);

	kernelUpdatePlane(object);
}

void GfxFrameout::kernelUpdatePlane(reg_t object) {
	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); it++) {
		if (it->object == object) {
			// Read some information
			it->priority = readSelectorValue(_segMan, object, SELECTOR(priority));
			GuiResourceId lastPictureId = it->pictureId;
			it->pictureId = readSelectorValue(_segMan, object, SELECTOR(picture));
			if (lastPictureId != it->pictureId) {
				// picture got changed, load new picture
				if ((it->pictureId != 0xFFFF) && (it->pictureId != 0xFFFE)) {
					// SQ6 gives us a bad picture number for the control menu
					if (_resMan->testResource(ResourceId(kResourceTypePic, it->pictureId)))
						it->picture = new GfxPicture(_resMan, _coordAdjuster, 0, _screen, _palette, it->pictureId, false);
				} else {
					delete it->picture;
					it->picture = NULL;
				}
			}
			sortPlanes();
			return;
		}
	}
	error("kUpdatePlane called on plane that wasn't added before");
}

void GfxFrameout::kernelDeletePlane(reg_t object) {
	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); it++) {
		if (it->object == object) {
			_planes.erase(it);
			Common::Rect planeRect;
			planeRect.top = readSelectorValue(_segMan, object, SELECTOR(top));
			planeRect.left = readSelectorValue(_segMan, object, SELECTOR(left));
			planeRect.bottom = readSelectorValue(_segMan, object, SELECTOR(bottom)) + 1;
			planeRect.right = readSelectorValue(_segMan, object, SELECTOR(right)) + 1;

			planeRect.top = (planeRect.top * _screen->getHeight()) / scriptsRunningHeight;
			planeRect.left = (planeRect.left * _screen->getWidth()) / scriptsRunningWidth;
			planeRect.bottom = (planeRect.bottom * _screen->getHeight()) / scriptsRunningHeight;
			planeRect.right = (planeRect.right * _screen->getWidth()) / scriptsRunningWidth;
			// Blackout removed plane rect
			_paint32->fillRect(planeRect, 0);
			return;
		}
	}
}

void GfxFrameout::kernelAddScreenItem(reg_t object) {
	_screenItems.push_back(object);
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
	sortPlanes();
	return readSelectorValue(g_sci->getEngineState()->_segMan, _planes.back().object, SELECTOR(priority));
}

bool sortHelper(const FrameoutEntry* entry1, const FrameoutEntry* entry2) {
	if (entry1->priority == entry2->priority) {
		if (entry1->y == entry2->y)
			return (entry1->givenOrderNr < entry2->givenOrderNr);
		return (entry1->y < entry2->y);
	}
	return (entry1->priority < entry2->priority);
}

bool planeSortHelper(const PlaneEntry &entry1, const PlaneEntry &entry2) {
//	SegManager *segMan = g_sci->getEngineState()->_segMan;

//	uint16 plane1Priority = readSelectorValue(segMan, entry1, SELECTOR(priority));
//	uint16 plane2Priority = readSelectorValue(segMan, entry2, SELECTOR(priority));

	if (entry1.priority == 0xffff)
		return true;

	if (entry2.priority == 0xffff)
		return false;

	return entry1.priority < entry2.priority;
}

void GfxFrameout::sortPlanes() {
	// First, remove any invalid planes
	for (PlaneList::iterator it = _planes.begin(); it != _planes.end();) {
		if (!_segMan->isObject(it->object))
			it = _planes.erase(it);
		else
			it++;
	}

	// Sort the rest of them
	Common::sort(_planes.begin(), _planes.end(), planeSortHelper);
}

void GfxFrameout::kernelFrameout() {
	_palette->palVaryUpdate();

	// Allocate enough space for all screen items
	FrameoutEntry *itemData = new FrameoutEntry[_screenItems.size()];

	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); it++) {
		reg_t planeObject = it->object;
		uint16 planeLastPriority = it->lastPriority;

		Common::Rect planeRect;
		planeRect.top = readSelectorValue(_segMan, planeObject, SELECTOR(top));
		planeRect.left = readSelectorValue(_segMan, planeObject, SELECTOR(left));
		planeRect.bottom = readSelectorValue(_segMan, planeObject, SELECTOR(bottom)) + 1;
		planeRect.right = readSelectorValue(_segMan, planeObject, SELECTOR(right)) + 1;

		// Update priority here, sq6 sets it w/o UpdatePlane
		uint16 planePriority = it->priority = readSelectorValue(_segMan, planeObject, SELECTOR(priority));

		planeRect.top = (planeRect.top * _screen->getHeight()) / scriptsRunningHeight;
		planeRect.left = (planeRect.left * _screen->getWidth()) / scriptsRunningWidth;
		planeRect.bottom = (planeRect.bottom * _screen->getHeight()) / scriptsRunningHeight;
		planeRect.right = (planeRect.right * _screen->getWidth()) / scriptsRunningWidth;

		// We get negative left in kq7 in scrolling rooms
		if (planeRect.left < 0)
			planeRect.left = 0;
		if (planeRect.top < 0)
			planeRect.top = 0;
		// We get bad plane-bottom in sq6
		if (planeRect.right > _screen->getWidth())
			planeRect.right = _screen->getWidth();
		if (planeRect.bottom > _screen->getHeight())
			planeRect.bottom = _screen->getHeight();

		it->lastPriority = planePriority;
		if (planePriority == 0xffff) { // Plane currently not meant to be shown
			// If plane was shown before, delete plane rect
			if (planePriority != planeLastPriority)
				_paint32->fillRect(planeRect, 0);
			continue;
		}

		Common::Rect planeClipRect(planeRect.width(), planeRect.height());

		Common::Rect upscaledPlaneRect = planeRect;
		Common::Rect upscaledPlaneClipRect = planeClipRect;
		if (_screen->getUpscaledHires()) {
			_screen->adjustToUpscaledCoordinates(upscaledPlaneRect.top, upscaledPlaneRect.left);
			_screen->adjustToUpscaledCoordinates(upscaledPlaneRect.bottom, upscaledPlaneRect.right);
			_screen->adjustToUpscaledCoordinates(upscaledPlaneClipRect.top, upscaledPlaneClipRect.left);
			_screen->adjustToUpscaledCoordinates(upscaledPlaneClipRect.bottom, upscaledPlaneClipRect.right);
		}

		byte planeBack = readSelectorValue(_segMan, planeObject, SELECTOR(back));
		if (planeBack)
			_paint32->fillRect(planeRect, planeBack);

		GuiResourceId planePictureNr = it->pictureId;
		GfxPicture *planePicture = it->picture;
		int16 planePictureCels = 0;
		bool planePictureMirrored = false;

		if (planePicture) {
			planePictureCels = planePicture->getSci32celCount();

			_coordAdjuster->pictureSetDisplayArea(planeRect);
			_palette->drewPicture(planePictureNr);

			if (readSelectorValue(_segMan, planeObject, SELECTOR(mirrored)))
				planePictureMirrored = true;
		}

		// Fill our itemlist for this plane
		int16 itemCount = 0;
		FrameoutEntry *itemEntry = itemData;
		FrameoutList itemList;

		for (uint32 itemNr = 0; itemNr < _screenItems.size(); itemNr++) {
			reg_t itemObject = _screenItems[itemNr];

			// Remove any invalid items
			if (!_segMan->isObject(itemObject)) {
				_screenItems.remove_at(itemNr);
				itemNr--;
				continue;
			}

			reg_t itemPlane = readSelector(_segMan, itemObject, SELECTOR(plane));
			if (planeObject == itemPlane) {
				// Found an item on current plane
				itemEntry->viewId = readSelectorValue(_segMan, itemObject, SELECTOR(view));
				itemEntry->loopNo = readSelectorValue(_segMan, itemObject, SELECTOR(loop));
				itemEntry->celNo = readSelectorValue(_segMan, itemObject, SELECTOR(cel));
				itemEntry->x = readSelectorValue(_segMan, itemObject, SELECTOR(x));
				itemEntry->y = readSelectorValue(_segMan, itemObject, SELECTOR(y));
				itemEntry->z = readSelectorValue(_segMan, itemObject, SELECTOR(z));
				itemEntry->priority = readSelectorValue(_segMan, itemObject, SELECTOR(priority));
				if (readSelectorValue(_segMan, itemObject, SELECTOR(fixPriority)) == 0)
					itemEntry->priority = itemEntry->y;

				itemEntry->signal = readSelectorValue(_segMan, itemObject, SELECTOR(signal));
				itemEntry->scaleX = readSelectorValue(_segMan, itemObject, SELECTOR(scaleX));
				itemEntry->scaleY = readSelectorValue(_segMan, itemObject, SELECTOR(scaleY));
				itemEntry->object = itemObject;
				itemEntry->givenOrderNr = itemNr;

				itemList.push_back(itemEntry);
				itemEntry++;
				itemCount++;
			}
		}

		FrameoutEntry *pictureCels = NULL;

		if (planePicture) {
			// Show base picture
//			planePicture->drawSci32Vga(0, planePicture->getSci32celX(0), planePicture->getSci32celY(0), planePictureMirrored);
			// Allocate memory for picture cels
			pictureCels = new FrameoutEntry[planePicture->getSci32celCount()];
			// Add following cels to the itemlist
			FrameoutEntry *picEntry = pictureCels;
			for (int pictureCelNr = 0; pictureCelNr < planePictureCels; pictureCelNr++) {
				picEntry->celNo = pictureCelNr;
				picEntry->object = NULL_REG;
				picEntry->y = planePicture->getSci32celY(pictureCelNr);
				picEntry->x = planePicture->getSci32celX(pictureCelNr);

				picEntry->priority = planePicture->getSci32celPriority(pictureCelNr);

				itemList.push_back(picEntry);
				picEntry++;
			}
		}

		// Now sort our itemlist
		Common::sort(itemList.begin(), itemList.end(), sortHelper);

		// Now display itemlist
		itemEntry = itemData;

//		warning("Plane %s", _segMan->getObjectName(planeObject));

		for (FrameoutList::iterator listIterator = itemList.begin(); listIterator != itemList.end(); listIterator++) {
			itemEntry = *listIterator;

			if (itemEntry->object.isNull()) {
				// Picture cel data
				itemEntry->y = ((itemEntry->y * _screen->getHeight()) / scriptsRunningHeight);
				itemEntry->x = ((itemEntry->x * _screen->getWidth()) / scriptsRunningWidth);

				planePicture->drawSci32Vga(itemEntry->celNo, itemEntry->x, itemEntry->y, planePictureMirrored);
//				warning("picture cel %d %d", itemEntry->celNo, itemEntry->priority);

			} else if (itemEntry->viewId != 0xFFFF) {
				GfxView *view = _cache->getView(itemEntry->viewId);

//				warning("view %s %04x:%04x", _segMan->getObjectName(itemEntry->object), PRINT_REG(itemEntry->object));

				switch (getSciVersion()) {
				case SCI_VERSION_2:
					if (view->isSci2Hires()) {
						int16 dummyX = 0;
						_screen->adjustToUpscaledCoordinates(itemEntry->y, itemEntry->x);
						_screen->adjustToUpscaledCoordinates(itemEntry->z, dummyX);
					}
					break;
				case SCI_VERSION_2_1:
					itemEntry->y = (itemEntry->y * _screen->getHeight()) / scriptsRunningHeight;
					itemEntry->x = (itemEntry->x * _screen->getWidth()) / scriptsRunningWidth;
					itemEntry->z = (itemEntry->z * _screen->getHeight()) / scriptsRunningHeight;
					break;
				default:
					break;
				}

				uint16 useInsetRect = readSelectorValue(_segMan, itemEntry->object, SELECTOR(useInsetRect));
				if (useInsetRect) {
					itemEntry->celRect.top = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inTop));
					itemEntry->celRect.left = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inLeft));
					itemEntry->celRect.bottom = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inBottom)) + 1;
					itemEntry->celRect.right = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inRight)) + 1;
					if (view->isSci2Hires()) {
						_screen->adjustToUpscaledCoordinates(itemEntry->celRect.top, itemEntry->celRect.left);
						_screen->adjustToUpscaledCoordinates(itemEntry->celRect.bottom, itemEntry->celRect.right);
					}
					itemEntry->celRect.translate(itemEntry->x, itemEntry->y);
					// TODO: maybe we should clip the cels rect with this, i'm not sure
					//  the only currently known usage is game menu of gk1
				} else {
					if ((itemEntry->scaleX == 128) && (itemEntry->scaleY == 128))
						view->getCelRect(itemEntry->loopNo, itemEntry->celNo, itemEntry->x, itemEntry->y, itemEntry->z, itemEntry->celRect);
					else
						view->getCelScaledRect(itemEntry->loopNo, itemEntry->celNo, itemEntry->x, itemEntry->y, itemEntry->z, itemEntry->scaleX, itemEntry->scaleY, itemEntry->celRect);

					Common::Rect nsRect = itemEntry->celRect;
					switch (getSciVersion()) {
					case SCI_VERSION_2:
						if (view->isSci2Hires()) {
							_screen->adjustBackUpscaledCoordinates(nsRect.top, nsRect.left);
							_screen->adjustBackUpscaledCoordinates(nsRect.bottom, nsRect.right);
						}
						break;
					case SCI_VERSION_2_1:
						nsRect.top = (nsRect.top * scriptsRunningHeight) / _screen->getHeight();
						nsRect.left = (nsRect.left * scriptsRunningWidth) / _screen->getWidth();
						nsRect.bottom = (nsRect.bottom * scriptsRunningHeight) / _screen->getHeight();
						nsRect.right = (nsRect.right * scriptsRunningWidth) / _screen->getWidth();
						break;
					default:
						break;
					}
					writeSelectorValue(_segMan, itemEntry->object, SELECTOR(nsLeft), nsRect.left);
					writeSelectorValue(_segMan, itemEntry->object, SELECTOR(nsTop), nsRect.top);
					writeSelectorValue(_segMan, itemEntry->object, SELECTOR(nsRight), nsRect.right);
					writeSelectorValue(_segMan, itemEntry->object, SELECTOR(nsBottom), nsRect.bottom);
				}

				int16 screenHeight = _screen->getHeight();
				int16 screenWidth = _screen->getWidth();
				if (view->isSci2Hires()) {
					screenHeight = _screen->getDisplayHeight();
					screenWidth = _screen->getDisplayWidth();
				}

				if (itemEntry->celRect.bottom < 0 || itemEntry->celRect.top >= screenHeight)
					continue;

				if (itemEntry->celRect.right < 0 || itemEntry->celRect.left >= screenWidth)
					continue;

				Common::Rect clipRect, translatedClipRect;
				clipRect = itemEntry->celRect;
				if (view->isSci2Hires()) {
					clipRect.clip(upscaledPlaneClipRect);
					translatedClipRect = clipRect;
					translatedClipRect.translate(upscaledPlaneRect.left, upscaledPlaneRect.top);
				} else {
					clipRect.clip(planeClipRect);
					translatedClipRect = clipRect;
					translatedClipRect.translate(planeRect.left, planeRect.top);
				}

				if (!clipRect.isEmpty()) {
					if ((itemEntry->scaleX == 128) && (itemEntry->scaleY == 128))
						view->draw(itemEntry->celRect, clipRect, translatedClipRect, itemEntry->loopNo, itemEntry->celNo, 255, 0, view->isSci2Hires());
					else
						view->drawScaled(itemEntry->celRect, clipRect, translatedClipRect, itemEntry->loopNo, itemEntry->celNo, 255, itemEntry->scaleX, itemEntry->scaleY);
				}
			} else {
				// Most likely a text entry
				// This draws text the "SCI0-SCI11" way. In SCI2, text is prerendered in kCreateTextBitmap
				// TODO: rewrite this the "SCI2" way (i.e. implement the text buffer to draw inside kCreateTextBitmap)
				if (lookupSelector(_segMan, itemEntry->object, SELECTOR(text), NULL, NULL) == kSelectorVariable) {
					Common::String text = _segMan->getString(readSelector(_segMan, itemEntry->object, SELECTOR(text)));
					GfxFont *font = _cache->getFont(readSelectorValue(_segMan, itemEntry->object, SELECTOR(font)));
					bool dimmed = readSelectorValue(_segMan, itemEntry->object, SELECTOR(dimmed));
					uint16 foreColor = readSelectorValue(_segMan, itemEntry->object, SELECTOR(fore));

					itemEntry->y = ((itemEntry->y * _screen->getHeight()) / scriptsRunningHeight);
					itemEntry->x = ((itemEntry->x * _screen->getWidth()) / scriptsRunningWidth);

					uint16 curX = itemEntry->x + planeRect.left;
					uint16 curY = itemEntry->y + planeRect.top;
					for (uint32 i = 0; i < text.size(); i++) {
						unsigned char curChar = text[i];
						// TODO: proper text splitting... this is a hack
						if ((curChar == ' ' && i > 0 && text[i - i] == ' ') || curChar == '\n' || 
							(curX + font->getCharWidth(curChar) > _screen->getWidth())) {
							curY += font->getHeight();
							curX = itemEntry->x;
						}
						font->draw(curChar, curY, curX, foreColor, dimmed);
						curX += font->getCharWidth(curChar);
					}
				}
			}
		}

		if (planePicture) {
			delete[] pictureCels;
		}
	}

	delete[] itemData;
	_screen->copyToScreen();

	//g_sci->getEngineState()->_throttleTrigger = true;
	// currently disabled because clones computer is too slow :P
}

} // End of namespace Sci
