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
 */

#include "common/algorithm.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/list_intern.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/engine.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/font.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/frameout.h"
#include "sci/video/robot_decoder.h"

namespace Sci {

// TODO/FIXME: This is all guesswork

GfxFrameout::GfxFrameout(SegManager *segMan, ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxCache *cache, GfxScreen *screen, GfxPalette *palette, GfxPaint32 *paint32)
	: _segMan(segMan), _resMan(resMan), _cache(cache), _screen(screen), _palette(palette), _paint32(paint32) {

	_coordAdjuster = (GfxCoordAdjuster32 *)coordAdjuster;
	_scriptsRunningWidth = 320;
	_scriptsRunningHeight = 200;
}

GfxFrameout::~GfxFrameout() {
	clear();
}

void GfxFrameout::clear() {
	deletePlaneItems(NULL_REG);
	_planes.clear();
	deletePlanePictures(NULL_REG);
}

void GfxFrameout::kernelAddPlane(reg_t object) {
	PlaneEntry newPlane;

	if (_planes.empty()) {
		// There has to be another way for sierra sci to do this or maybe script resolution is compiled into
		//  interpreter (TODO)
		uint16 tmpRunningWidth = readSelectorValue(_segMan, object, SELECTOR(resX));
		uint16 tmpRunningHeight = readSelectorValue(_segMan, object, SELECTOR(resY));

		// The above can be 0 in SCI3 (e.g. Phantasmagoria 2)
		if (tmpRunningWidth > 0 && tmpRunningHeight > 0) {
			_scriptsRunningWidth = tmpRunningWidth;
			_scriptsRunningHeight = tmpRunningHeight;
		}

		_coordAdjuster->setScriptsResolution(_scriptsRunningWidth, _scriptsRunningHeight);
	}

	newPlane.object = object;
	newPlane.priority = readSelectorValue(_segMan, object, SELECTOR(priority));
	newPlane.lastPriority = 0xFFFF; // hidden
	newPlane.planeOffsetX = 0;
	newPlane.planeOffsetY = 0;
	newPlane.pictureId = 0xFFFF;
	newPlane.planePictureMirrored = false;
	newPlane.planeBack = 0;
	_planes.push_back(newPlane);

	kernelUpdatePlane(object);
}

void GfxFrameout::kernelUpdatePlane(reg_t object) {
	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); ++it) {
		if (it->object == object) {
			// Read some information
			it->priority = readSelectorValue(_segMan, object, SELECTOR(priority));
			GuiResourceId lastPictureId = it->pictureId;
			it->pictureId = readSelectorValue(_segMan, object, SELECTOR(picture));
			if (lastPictureId != it->pictureId) {
				// picture got changed, load new picture
				deletePlanePictures(object);
				if ((it->pictureId != 0xFFFF) && (it->pictureId != 0xFFFE)) {
					// SQ6 gives us a bad picture number for the control menu
					if (_resMan->testResource(ResourceId(kResourceTypePic, it->pictureId)))
						addPlanePicture(object, it->pictureId, 0);
				}
			}
			it->planeRect.top = readSelectorValue(_segMan, object, SELECTOR(top));
			it->planeRect.left = readSelectorValue(_segMan, object, SELECTOR(left));
			it->planeRect.bottom = readSelectorValue(_segMan, object, SELECTOR(bottom));
			it->planeRect.right = readSelectorValue(_segMan, object, SELECTOR(right));

			Common::Rect screenRect(_screen->getWidth(), _screen->getHeight());
			it->planeRect.top = (it->planeRect.top * screenRect.height()) / _scriptsRunningHeight;
			it->planeRect.left = (it->planeRect.left * screenRect.width()) / _scriptsRunningWidth;
			it->planeRect.bottom = (it->planeRect.bottom * screenRect.height()) / _scriptsRunningHeight;
			it->planeRect.right = (it->planeRect.right * screenRect.width()) / _scriptsRunningWidth;

			// We get negative left in kq7 in scrolling rooms
			if (it->planeRect.left < 0) {
				it->planeOffsetX = -it->planeRect.left;
				it->planeRect.left = 0;
			} else {
				it->planeOffsetX = 0;
			}
			
			if (it->planeRect.top < 0) {
				it->planeOffsetY = -it->planeRect.top;
				it->planeRect.top = 0;
			} else {
				it->planeOffsetY = 0;
			}

			// We get bad plane-bottom in sq6
			if (it->planeRect.right > _screen->getWidth())
				it->planeRect.right = _screen->getWidth();
			if (it->planeRect.bottom > _screen->getHeight())
				it->planeRect.bottom = _screen->getHeight();

			it->planeClipRect = Common::Rect(it->planeRect.width(), it->planeRect.height());
			it->upscaledPlaneRect = it->planeRect;
			it->upscaledPlaneClipRect = it->planeClipRect;
			if (_screen->getUpscaledHires()) {
				_screen->adjustToUpscaledCoordinates(it->upscaledPlaneRect.top, it->upscaledPlaneRect.left);
				_screen->adjustToUpscaledCoordinates(it->upscaledPlaneRect.bottom, it->upscaledPlaneRect.right);
				_screen->adjustToUpscaledCoordinates(it->upscaledPlaneClipRect.top, it->upscaledPlaneClipRect.left);
				_screen->adjustToUpscaledCoordinates(it->upscaledPlaneClipRect.bottom, it->upscaledPlaneClipRect.right);
			}

			it->planePictureMirrored = readSelectorValue(_segMan, object, SELECTOR(mirrored));
			it->planeBack = readSelectorValue(_segMan, object, SELECTOR(back));

			sortPlanes();

			// Update the items in the plane
			for (FrameoutList::iterator listIterator = _screenItems.begin(); listIterator != _screenItems.end(); listIterator++) {
				reg_t itemPlane = readSelector(_segMan, (*listIterator)->object, SELECTOR(plane));
				if (object == itemPlane) {
					kernelUpdateScreenItem((*listIterator)->object);
				}
			}

			return;
		}
	}
	error("kUpdatePlane called on plane that wasn't added before");
}

void GfxFrameout::kernelDeletePlane(reg_t object) {
	deletePlaneItems(object);
	deletePlanePictures(object);

	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); ++it) {
		if (it->object == object) {
			_planes.erase(it);
			Common::Rect planeRect;
			planeRect.top = readSelectorValue(_segMan, object, SELECTOR(top));
			planeRect.left = readSelectorValue(_segMan, object, SELECTOR(left));
			planeRect.bottom = readSelectorValue(_segMan, object, SELECTOR(bottom));
			planeRect.right = readSelectorValue(_segMan, object, SELECTOR(right));

			Common::Rect screenRect(_screen->getWidth(), _screen->getHeight());
			planeRect.top = (planeRect.top * screenRect.height()) / _scriptsRunningHeight;
			planeRect.left = (planeRect.left * screenRect.width()) / _scriptsRunningWidth;
			planeRect.bottom = (planeRect.bottom * screenRect.height()) / _scriptsRunningHeight;
			planeRect.right = (planeRect.right * screenRect.width()) / _scriptsRunningWidth;
			// Blackout removed plane rect
			_paint32->fillRect(planeRect, 0);
			return;
		}
	}
}

void GfxFrameout::addPlanePicture(reg_t object, GuiResourceId pictureId, uint16 startX, uint16 startY) {
	PlanePictureEntry newPicture;
	newPicture.object = object;
	newPicture.pictureId = pictureId;
	newPicture.picture = new GfxPicture(_resMan, _coordAdjuster, 0, _screen, _palette, pictureId, false);
	newPicture.startX = startX;
	newPicture.startY = startY;
	newPicture.pictureCels = 0;
	_planePictures.push_back(newPicture);
}

void GfxFrameout::deletePlanePictures(reg_t object) {
	PlanePictureList::iterator it = _planePictures.begin();

	while (it != _planePictures.end()) {
		if (it->object == object || object.isNull()) {
			delete it->pictureCels;
			delete it->picture;
			it = _planePictures.erase(it);
		} else {
			++it;
		}
	}
}

void GfxFrameout::kernelAddScreenItem(reg_t object) {
	// Ignore invalid items
	if (!_segMan->isObject(object))
		return;

	FrameoutEntry *itemEntry = new FrameoutEntry();
	memset(itemEntry, 0, sizeof(FrameoutEntry));
	itemEntry->object = object;
	itemEntry->givenOrderNr = _screenItems.size();
	_screenItems.push_back(itemEntry);

	kernelUpdateScreenItem(object);
}

void GfxFrameout::kernelUpdateScreenItem(reg_t object) {
	// Ignore invalid items
	if (!_segMan->isObject(object))
		return;

	FrameoutEntry *itemEntry = findScreenItem(object);
	if (!itemEntry) {
		warning("kernelUpdateScreenItem: invalid object %04x:%04x", PRINT_REG(object));
		return;
	}

	itemEntry->viewId = readSelectorValue(_segMan, object, SELECTOR(view));
	itemEntry->loopNo = readSelectorValue(_segMan, object, SELECTOR(loop));
	itemEntry->celNo = readSelectorValue(_segMan, object, SELECTOR(cel));
	itemEntry->x = readSelectorValue(_segMan, object, SELECTOR(x));
	itemEntry->y = readSelectorValue(_segMan, object, SELECTOR(y));
	itemEntry->z = readSelectorValue(_segMan, object, SELECTOR(z));
	itemEntry->priority = readSelectorValue(_segMan, object, SELECTOR(priority));
	if (readSelectorValue(_segMan, object, SELECTOR(fixPriority)) == 0)
		itemEntry->priority = itemEntry->y;

	itemEntry->signal = readSelectorValue(_segMan, object, SELECTOR(signal));
	itemEntry->scaleX = readSelectorValue(_segMan, object, SELECTOR(scaleX));
	itemEntry->scaleY = readSelectorValue(_segMan, object, SELECTOR(scaleY));
}

void GfxFrameout::kernelDeleteScreenItem(reg_t object) {
	FrameoutEntry *itemEntry = findScreenItem(object);
	if (!itemEntry) {
		warning("kernelDeleteScreenItem: invalid object %04x:%04x", PRINT_REG(object));
		return;
	}

	_screenItems.remove(itemEntry);
	delete itemEntry;
}

void GfxFrameout::deletePlaneItems(reg_t planeObject) {
	FrameoutList::iterator listIterator = _screenItems.begin();

	while (listIterator != _screenItems.end()) {
		bool objectMatches = false;
		if (!planeObject.isNull()) {
			reg_t itemPlane = readSelector(_segMan, (*listIterator)->object, SELECTOR(plane));
			objectMatches = (planeObject == itemPlane);
		} else {
			objectMatches = true;
		}
		
		if (objectMatches) {
			FrameoutEntry *itemEntry = *listIterator;
			listIterator = _screenItems.erase(listIterator);
			delete itemEntry;
		} else {
			++listIterator;
		}
	}
}

FrameoutEntry *GfxFrameout::findScreenItem(reg_t object) {
	for (FrameoutList::iterator listIterator = _screenItems.begin(); listIterator != _screenItems.end(); listIterator++) {
		FrameoutEntry *itemEntry = *listIterator;
		if (itemEntry->object == object)
			return itemEntry;
	}

	return NULL;
}

int16 GfxFrameout::kernelGetHighPlanePri() {
	sortPlanes();
	return readSelectorValue(g_sci->getEngineState()->_segMan, _planes.back().object, SELECTOR(priority));
}

void GfxFrameout::kernelAddPicAt(reg_t planeObj, GuiResourceId pictureId, int16 pictureX, int16 pictureY) {
	addPlanePicture(planeObj, pictureId, pictureX, pictureY);
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

int16 GfxFrameout::upscaleHorizontalCoordinate(int16 coordinate) {
	return ((coordinate * _screen->getWidth()) / _scriptsRunningWidth);
}

int16 GfxFrameout::upscaleVerticalCoordinate(int16 coordinate) {
	return ((coordinate * _screen->getHeight()) / _scriptsRunningHeight);
}

Common::Rect GfxFrameout::upscaleRect(Common::Rect &rect) {
	rect.top = (rect.top * _scriptsRunningHeight) / _screen->getHeight();
	rect.left = (rect.left * _scriptsRunningWidth) / _screen->getWidth();
	rect.bottom = (rect.bottom * _scriptsRunningHeight) / _screen->getHeight();
	rect.right = (rect.right * _scriptsRunningWidth) / _screen->getWidth();

	return rect;
}

void GfxFrameout::showVideo() {
	bool skipVideo = false;
	RobotDecoder *videoDecoder = g_sci->_robotDecoder;
	uint16 x = videoDecoder->getPos().x;
	uint16 y = videoDecoder->getPos().y;

	if (videoDecoder->hasDirtyPalette())
		videoDecoder->setSystemPalette();

	while (!g_engine->shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();
			if (frame) {
				g_system->copyRectToScreen((byte *)frame->pixels, frame->pitch, x, y, frame->w, frame->h);

				if (videoDecoder->hasDirtyPalette())
					videoDecoder->setSystemPalette();

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}
}

void GfxFrameout::createPlaneItemList(reg_t planeObject, FrameoutList &itemList) {
	// Copy screen items of the current frame to the list of items to be drawn
	for (FrameoutList::iterator listIterator = _screenItems.begin(); listIterator != _screenItems.end(); listIterator++) {
		reg_t itemPlane = readSelector(_segMan, (*listIterator)->object, SELECTOR(plane));
		if (planeObject == itemPlane) {
			kernelUpdateScreenItem((*listIterator)->object);	// TODO: Why is this necessary?
			itemList.push_back(*listIterator);
		}
	}

	for (PlanePictureList::iterator pictureIt = _planePictures.begin(); pictureIt != _planePictures.end(); pictureIt++) {
		if (pictureIt->object == planeObject) {
			GfxPicture *planePicture = pictureIt->picture;
			// Allocate memory for picture cels
			pictureIt->pictureCels = new FrameoutEntry[planePicture->getSci32celCount()];

			// Add following cels to the itemlist
			FrameoutEntry *picEntry = pictureIt->pictureCels;
			int planePictureCels = planePicture->getSci32celCount();
			for (int pictureCelNr = 0; pictureCelNr < planePictureCels; pictureCelNr++) {
				picEntry->celNo = pictureCelNr;
				picEntry->object = NULL_REG;
				picEntry->picture = planePicture;
				picEntry->y = planePicture->getSci32celY(pictureCelNr);
				picEntry->x = planePicture->getSci32celX(pictureCelNr);
				picEntry->picStartX = pictureIt->startX;
				picEntry->picStartY = pictureIt->startY;

				picEntry->priority = planePicture->getSci32celPriority(pictureCelNr);

				itemList.push_back(picEntry);
				picEntry++;
			}
		}
	}

	// Now sort our itemlist
	Common::sort(itemList.begin(), itemList.end(), sortHelper);
}

bool GfxFrameout::isPictureOutOfView(FrameoutEntry *itemEntry, Common::Rect planeRect, int16 planeOffsetX, int16 planeOffsetY) {
	// Out of view horizontally (sanity checks)
	int16 pictureCelStartX = itemEntry->picStartX + itemEntry->x;
	int16 pictureCelEndX = pictureCelStartX + itemEntry->picture->getSci32celWidth(itemEntry->celNo);
	int16 planeStartX = planeOffsetX;
	int16 planeEndX = planeStartX + planeRect.width();
	if (pictureCelEndX < planeStartX)
		return true;
	if (pictureCelStartX > planeEndX)
		return true;

	// Out of view vertically (sanity checks)
	int16 pictureCelStartY = itemEntry->picStartY + itemEntry->y;
	int16 pictureCelEndY = pictureCelStartY + itemEntry->picture->getSci32celHeight(itemEntry->celNo);
	int16 planeStartY = planeOffsetY;
	int16 planeEndY = planeStartY + planeRect.height();
	if (pictureCelEndY < planeStartY)
		return true;
	if (pictureCelStartY > planeEndY)
		return true;

	return false;
}

void GfxFrameout::drawPicture(FrameoutEntry *itemEntry, int16 planeOffsetX, int16 planeOffsetY, bool planePictureMirrored) {
	int16 pictureOffsetX = planeOffsetX;
	int16 pictureX = itemEntry->x;
	if ((planeOffsetX) || (itemEntry->picStartX)) {
		if (planeOffsetX <= itemEntry->picStartX) {
			pictureX += itemEntry->picStartX - planeOffsetX;
			pictureOffsetX = 0;
		} else {
			pictureOffsetX = planeOffsetX - itemEntry->picStartX;
		}
	}

	int16 pictureOffsetY = planeOffsetY;
	int16 pictureY = itemEntry->y;
	if ((planeOffsetY) || (itemEntry->picStartY)) {
		if (planeOffsetY <= itemEntry->picStartY) {
			pictureY += itemEntry->picStartY - planeOffsetY;
			pictureOffsetY = 0;
		} else {
			pictureOffsetY = planeOffsetY - itemEntry->picStartY;
		}
	}

	itemEntry->picture->drawSci32Vga(itemEntry->celNo, pictureX, itemEntry->y, pictureOffsetX, pictureOffsetY, planePictureMirrored);
	//	warning("picture cel %d %d", itemEntry->celNo, itemEntry->priority);
}

void GfxFrameout::kernelFrameout() {
	if (g_sci->_robotDecoder->isVideoLoaded()) {
		showVideo();
		return;
	}

	_palette->palVaryUpdate();

	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); it++) {
		reg_t planeObject = it->object;
		uint16 planeLastPriority = it->lastPriority;

		// Update priority here, sq6 sets it w/o UpdatePlane
		uint16 planePriority = it->priority = readSelectorValue(_segMan, planeObject, SELECTOR(priority));

		it->lastPriority = planePriority;
		if (planePriority == 0xffff) { // Plane currently not meant to be shown
			// If plane was shown before, delete plane rect
			if (planePriority != planeLastPriority)
				_paint32->fillRect(it->planeRect, 0);
			continue;
		}

		// There is a race condition lurking in SQ6, which causes the game to hang in the intro, when teleporting to Polysorbate LX.
		// Since I first wrote the patch, the race has stopped occurring for me though.
		// I'll leave this for investigation later, when someone can reproduce.
		//if (it->pictureId == 0xffff)	// FIXME: This is what SSCI does, and fixes the intro of LSL7, but breaks the dialogs in GK1 (adds black boxes)
		if (it->planeBack)
			_paint32->fillRect(it->planeRect, it->planeBack);

		GuiResourceId planeMainPictureId = it->pictureId;

		_coordAdjuster->pictureSetDisplayArea(it->planeRect);
		_palette->drewPicture(planeMainPictureId);

		FrameoutList itemList;

		createPlaneItemList(planeObject, itemList);

//		warning("Plane %s", _segMan->getObjectName(planeObject));

		for (FrameoutList::iterator listIterator = itemList.begin(); listIterator != itemList.end(); listIterator++) {
			FrameoutEntry *itemEntry = *listIterator;

			if (itemEntry->object.isNull()) {
				// Picture cel data
				itemEntry->x = upscaleHorizontalCoordinate(itemEntry->x);
				itemEntry->y = upscaleVerticalCoordinate(itemEntry->y);
				itemEntry->picStartX = upscaleHorizontalCoordinate(itemEntry->picStartX);
				itemEntry->picStartY = upscaleVerticalCoordinate(itemEntry->picStartY);

				if (!isPictureOutOfView(itemEntry, it->planeRect, it->planeOffsetX, it->planeOffsetY))
					drawPicture(itemEntry, it->planeOffsetX, it->planeOffsetY, it->planePictureMirrored);
			} else {
				GfxView *view = (itemEntry->viewId != 0xFFFF) ? _cache->getView(itemEntry->viewId) : NULL;
				
				if (view && view->isSci2Hires()) {
					int16 dummyX = 0;
					view->adjustToUpscaledCoordinates(itemEntry->y, itemEntry->x);
					view->adjustToUpscaledCoordinates(itemEntry->z, dummyX);
				} else if (getSciVersion() == SCI_VERSION_2_1) {
					itemEntry->x = upscaleHorizontalCoordinate(itemEntry->x);
					itemEntry->y = upscaleVerticalCoordinate(itemEntry->y);
					itemEntry->z = upscaleVerticalCoordinate(itemEntry->z);
				}

				// Adjust according to current scroll position
				itemEntry->x -= it->planeOffsetX;
				itemEntry->y -= it->planeOffsetY;

				uint16 useInsetRect = readSelectorValue(_segMan, itemEntry->object, SELECTOR(useInsetRect));
				if (useInsetRect) {
					itemEntry->celRect.top = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inTop));
					itemEntry->celRect.left = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inLeft));
					itemEntry->celRect.bottom = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inBottom));
					itemEntry->celRect.right = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inRight));
					if (view && view->isSci2Hires()) {
						view->adjustToUpscaledCoordinates(itemEntry->celRect.top, itemEntry->celRect.left);
						view->adjustToUpscaledCoordinates(itemEntry->celRect.bottom, itemEntry->celRect.right);
					}
					itemEntry->celRect.translate(itemEntry->x, itemEntry->y);
					// TODO: maybe we should clip the cels rect with this, i'm not sure
					//  the only currently known usage is game menu of gk1
				} else if (view) {
						if ((itemEntry->scaleX == 128) && (itemEntry->scaleY == 128))
							view->getCelRect(itemEntry->loopNo, itemEntry->celNo,
								itemEntry->x, itemEntry->y, itemEntry->z, itemEntry->celRect);
						else
							view->getCelScaledRect(itemEntry->loopNo, itemEntry->celNo, 
								itemEntry->x, itemEntry->y, itemEntry->z, itemEntry->scaleX,
								itemEntry->scaleY, itemEntry->celRect);

					Common::Rect nsRect = itemEntry->celRect;
					// Translate back to actual coordinate within scrollable plane
					nsRect.translate(it->planeOffsetX, it->planeOffsetY);

					if (view && view->isSci2Hires()) {
						view->adjustBackUpscaledCoordinates(nsRect.top, nsRect.left);
						view->adjustBackUpscaledCoordinates(nsRect.bottom, nsRect.right);
					} else if (getSciVersion() == SCI_VERSION_2_1) {
						nsRect = upscaleRect(nsRect);
					}

					if (g_sci->getGameId() == GID_PHANTASMAGORIA2) {
						// HACK: Some (?) objects in Phantasmagoria 2 have no NS rect. Skip them for now.
						// TODO: Remove once we figure out how Phantasmagoria 2 draws objects on screen.
						if (lookupSelector(_segMan, itemEntry->object, SELECTOR(nsLeft), NULL, NULL) != kSelectorVariable)
							continue;
					}

					g_sci->_gfxCompare->setNSRect(itemEntry->object, nsRect);
				}

				int16 screenHeight = _screen->getHeight();
				int16 screenWidth = _screen->getWidth();
				if (view && view->isSci2Hires()) {
					screenHeight = _screen->getDisplayHeight();
					screenWidth = _screen->getDisplayWidth();
				}

				if (itemEntry->celRect.bottom < 0 || itemEntry->celRect.top >= screenHeight)
					continue;

				if (itemEntry->celRect.right < 0 || itemEntry->celRect.left >= screenWidth)
					continue;

				Common::Rect clipRect, translatedClipRect;
				clipRect = itemEntry->celRect;

				if (view && view->isSci2Hires()) {
					clipRect.clip(it->upscaledPlaneClipRect);
					translatedClipRect = clipRect;
					translatedClipRect.translate(it->upscaledPlaneRect.left, it->upscaledPlaneRect.top);
				} else {
					clipRect.clip(it->planeClipRect);
					translatedClipRect = clipRect;
					translatedClipRect.translate(it->planeRect.left, it->planeRect.top);
				}

				if (view) {
					if (!clipRect.isEmpty()) {
						if ((itemEntry->scaleX == 128) && (itemEntry->scaleY == 128))
							view->draw(itemEntry->celRect, clipRect, translatedClipRect, 
								itemEntry->loopNo, itemEntry->celNo, 255, 0, view->isSci2Hires());
						else
							view->drawScaled(itemEntry->celRect, clipRect, translatedClipRect, 
								itemEntry->loopNo, itemEntry->celNo, 255, itemEntry->scaleX, itemEntry->scaleY);
					}
				}

				// Draw text, if it exists
				if (lookupSelector(_segMan, itemEntry->object, SELECTOR(text), NULL, NULL) == kSelectorVariable) {
					g_sci->_gfxText32->drawTextBitmap(itemEntry->x, itemEntry->y, it->planeRect, itemEntry->object);
				}
			}
		}

		for (PlanePictureList::iterator pictureIt = _planePictures.begin(); pictureIt != _planePictures.end(); pictureIt++) {
			if (pictureIt->object == planeObject) {
				delete[] pictureIt->pictureCels;
				pictureIt->pictureCels = 0;
			}
		}
	}

	_screen->copyToScreen();

	g_sci->getEngineState()->_throttleTrigger = true;
}

} // End of namespace Sci
