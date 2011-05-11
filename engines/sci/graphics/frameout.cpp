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
#include "sci/graphics/font.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/frameout.h"
#include "sci/video/robot_decoder.h"

namespace Sci {

// TODO/FIXME: This is all guesswork

GfxFrameout::GfxFrameout(SegManager *segMan, ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxCache *cache, GfxScreen *screen, GfxPalette *palette, GfxPaint32 *paint32)
	: _segMan(segMan), _resMan(resMan), _cache(cache), _screen(screen), _palette(palette), _paint32(paint32) {

	_coordAdjuster = (GfxCoordAdjuster32 *)coordAdjuster;
	scriptsRunningWidth = 320;
	scriptsRunningHeight = 200;
}

GfxFrameout::~GfxFrameout() {
}

void GfxFrameout::clear() {
	_screenItems.clear();
	_planes.clear();
	_planePictures.clear();
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
			scriptsRunningWidth = tmpRunningWidth;
			scriptsRunningHeight = tmpRunningHeight;
		}

		_coordAdjuster->setScriptsResolution(scriptsRunningWidth, scriptsRunningHeight);
	}

	newPlane.object = object;
	newPlane.priority = readSelectorValue(_segMan, object, SELECTOR(priority));
	newPlane.lastPriority = 0xFFFF; // hidden
	newPlane.planeOffsetX = 0;
	newPlane.pictureId = 0xFFFF;
	newPlane.planePictureMirrored = false;
	newPlane.planeBack = 0;
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
				deletePlanePictures(object);
				if ((it->pictureId != 0xFFFF) && (it->pictureId != 0xFFFE)) {
					// SQ6 gives us a bad picture number for the control menu
					if (_resMan->testResource(ResourceId(kResourceTypePic, it->pictureId)))
						addPlanePicture(object, it->pictureId, 0);
				}
			}
			it->planeRect.top = readSelectorValue(_segMan, object, SELECTOR(top));
			it->planeRect.left = readSelectorValue(_segMan, object, SELECTOR(left));
			it->planeRect.bottom = readSelectorValue(_segMan, object, SELECTOR(bottom)) + 1;
			it->planeRect.right = readSelectorValue(_segMan, object, SELECTOR(right)) + 1;

			Common::Rect screenRect(_screen->getWidth(), _screen->getHeight());
			it->planeRect.top = (it->planeRect.top * screenRect.height()) / scriptsRunningHeight;
			it->planeRect.left = (it->planeRect.left * screenRect.width()) / scriptsRunningWidth;
			it->planeRect.bottom = (it->planeRect.bottom * screenRect.height()) / scriptsRunningHeight;
			it->planeRect.right = (it->planeRect.right * screenRect.width()) / scriptsRunningWidth;

			// We get negative left in kq7 in scrolling rooms
			if (it->planeRect.left < 0) {
				it->planeOffsetX = -it->planeRect.left;
				it->planeRect.left = 0;
			} else {
				it->planeOffsetX = 0;
			}

			if (it->planeRect.top < 0)
				it->planeRect.top = 0;
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

void GfxFrameout::kernelRepaintPlane(reg_t object) {
	// TODO
}

void GfxFrameout::kernelDeletePlane(reg_t object) {
	deletePlanePictures(object);
	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); it++) {
		if (it->object == object) {
			_planes.erase(it);
			Common::Rect planeRect;
			planeRect.top = readSelectorValue(_segMan, object, SELECTOR(top));
			planeRect.left = readSelectorValue(_segMan, object, SELECTOR(left));
			planeRect.bottom = readSelectorValue(_segMan, object, SELECTOR(bottom)) + 1;
			planeRect.right = readSelectorValue(_segMan, object, SELECTOR(right)) + 1;

			Common::Rect screenRect(_screen->getWidth(), _screen->getHeight());
			planeRect.top = (planeRect.top * screenRect.height()) / scriptsRunningHeight;
			planeRect.left = (planeRect.left * screenRect.width()) / scriptsRunningWidth;
			planeRect.bottom = (planeRect.bottom * screenRect.height()) / scriptsRunningHeight;
			planeRect.right = (planeRect.right * screenRect.width()) / scriptsRunningWidth;
			planeRect.clip(screenRect); // we need to do this, at least in gk1 on cemetary we get bottom right -> 201, 321
			// Blackout removed plane rect
			_paint32->fillRect(planeRect, 0);
			return;
		}
	}
}

void GfxFrameout::addPlanePicture(reg_t object, GuiResourceId pictureId, uint16 startX) {
	PlanePictureEntry newPicture;
	newPicture.object = object;
	newPicture.pictureId = pictureId;
	newPicture.picture = new GfxPicture(_resMan, _coordAdjuster, 0, _screen, _palette, pictureId, false);
	newPicture.startX = startX;
	newPicture.pictureCels = 0;
	_planePictures.push_back(newPicture);
}

void GfxFrameout::deletePlanePictures(reg_t object) {
	for (PlanePictureList::iterator it = _planePictures.begin(); it != _planePictures.end(); it++) {
		if (it->object == object) {
			delete it->picture;
			_planePictures.erase(it);
			deletePlanePictures(object);
			return;
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

	for (FrameoutList::iterator listIterator = _screenItems.begin(); listIterator != _screenItems.end(); listIterator++) {
		FrameoutEntry *itemEntry = *listIterator;

		if (itemEntry->object == object) {
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
			return;
		}
	}
}

void GfxFrameout::kernelDeleteScreenItem(reg_t object) {
	for (FrameoutList::iterator listIterator = _screenItems.begin(); listIterator != _screenItems.end(); listIterator++) {
		FrameoutEntry *itemEntry = *listIterator;
		if (itemEntry->object == object) {
			_screenItems.remove(itemEntry);
			return;
		}
	}
}

int16 GfxFrameout::kernelGetHighPlanePri() {
	sortPlanes();
	return readSelectorValue(g_sci->getEngineState()->_segMan, _planes.back().object, SELECTOR(priority));
}

// TODO: No idea yet how to implement this
void GfxFrameout::kernelAddPicAt(reg_t planeObj, int16 forWidth, GuiResourceId pictureId) {
	addPlanePicture(planeObj, pictureId, forWidth);
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

static int16 GetLongest(const char *text, int16 maxWidth, GfxFont *font) {
	uint16 curChar = 0;
	int16 maxChars = 0, curCharCount = 0;
	uint16 width = 0;

	while (width <= maxWidth) {
		curChar = (*(const byte *)text++);

		switch (curChar) {
		// We need to add 0xD, 0xA and 0xD 0xA to curCharCount and then exit
		//  which means, we split text like
		//  'Mature, experienced software analyst available.' 0xD 0xA
		//  'Bug installation a proven speciality. "No version too clean."' (normal game text, this is from lsl2)
		//   and 0xA '-------' 0xA (which is the official sierra subtitle separator)
		//  Sierra did it the same way.
		case 0xD:
			// Check, if 0xA is following, if so include it as well
			if ((*(const unsigned char *)text) == 0xA)
				curCharCount++;
			// it's meant to pass through here
		case 0xA:
			curCharCount++;
			// and it's also meant to pass through here
		case 0:
			return curCharCount;
		case ' ':
			maxChars = curCharCount; // return count up to (but not including) breaking space
			break;
		}
		if (width + font->getCharWidth(curChar) > maxWidth)
			break;
		width += font->getCharWidth(curChar);
		curCharCount++;
	}

	return maxChars;
}

void GfxFrameout::kernelFrameout() {
	if (g_sci->_robotDecoder->isVideoLoaded()) {
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

		if (it->planeBack)
			_paint32->fillRect(it->planeRect, it->planeBack);

		GuiResourceId planeMainPictureId = it->pictureId;

		_coordAdjuster->pictureSetDisplayArea(it->planeRect);
		_palette->drewPicture(planeMainPictureId);

		FrameoutList itemList;

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

					picEntry->priority = planePicture->getSci32celPriority(pictureCelNr);

					itemList.push_back(picEntry);
					picEntry++;
				}
			}
		}

		// Now sort our itemlist
		Common::sort(itemList.begin(), itemList.end(), sortHelper);

//		warning("Plane %s", _segMan->getObjectName(planeObject));

		for (FrameoutList::iterator listIterator = itemList.begin(); listIterator != itemList.end(); listIterator++) {
			FrameoutEntry *itemEntry = *listIterator;

			if (itemEntry->object.isNull()) {
				// Picture cel data
				itemEntry->y = ((itemEntry->y * _screen->getHeight()) / scriptsRunningHeight);
				itemEntry->x = ((itemEntry->x * _screen->getWidth()) / scriptsRunningWidth);
				itemEntry->picStartX = ((itemEntry->picStartX * _screen->getWidth()) / scriptsRunningWidth);

				// Out of view
				int16 pictureCelStartX = itemEntry->picStartX + itemEntry->x;
				int16 pictureCelEndX = pictureCelStartX + itemEntry->picture->getSci32celWidth(itemEntry->celNo);
				int16 planeStartX = it->planeOffsetX;
				int16 planeEndX = planeStartX + it->planeRect.width();
				if (pictureCelEndX < planeStartX)
					continue;
				if (pictureCelStartX > planeEndX)
					continue;

				int16 pictureOffsetX = it->planeOffsetX;
				int16 pictureX = itemEntry->x;
				if ((it->planeOffsetX) || (itemEntry->picStartX)) {
					if (it->planeOffsetX <= itemEntry->picStartX) {
						pictureX += itemEntry->picStartX - it->planeOffsetX;
						pictureOffsetX = 0;
					} else {
						pictureOffsetX = it->planeOffsetX - itemEntry->picStartX;
					}
				}

				itemEntry->picture->drawSci32Vga(itemEntry->celNo, pictureX, itemEntry->y, pictureOffsetX, it->planePictureMirrored);
//				warning("picture cel %d %d", itemEntry->celNo, itemEntry->priority);

			} else if (itemEntry->viewId != 0xFFFF) {
				GfxView *view = _cache->getView(itemEntry->viewId);

//				warning("view %s %04x:%04x", _segMan->getObjectName(itemEntry->object), PRINT_REG(itemEntry->object));


				if (view->isSci2Hires()) {
					int16 dummyX = 0;
					view->adjustToUpscaledCoordinates(itemEntry->y, itemEntry->x);
					view->adjustToUpscaledCoordinates(itemEntry->z, dummyX);
				} else if (getSciVersion() == SCI_VERSION_2_1) {
					itemEntry->y = (itemEntry->y * _screen->getHeight()) / scriptsRunningHeight;
					itemEntry->x = (itemEntry->x * _screen->getWidth()) / scriptsRunningWidth;
					itemEntry->z = (itemEntry->z * _screen->getHeight()) / scriptsRunningHeight;
				}

				// Adjust according to current scroll position
				itemEntry->x -= it->planeOffsetX;

				uint16 useInsetRect = readSelectorValue(_segMan, itemEntry->object, SELECTOR(useInsetRect));
				if (useInsetRect) {
					itemEntry->celRect.top = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inTop));
					itemEntry->celRect.left = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inLeft));
					itemEntry->celRect.bottom = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inBottom)) + 1;
					itemEntry->celRect.right = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inRight)) + 1;
					if (view->isSci2Hires()) {
						view->adjustToUpscaledCoordinates(itemEntry->celRect.top, itemEntry->celRect.left);
						view->adjustToUpscaledCoordinates(itemEntry->celRect.bottom, itemEntry->celRect.right);
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
					// Translate back to actual coordinate within scrollable plane
					nsRect.translate(it->planeOffsetX, 0);

					if (view->isSci2Hires()) {
						view->adjustBackUpscaledCoordinates(nsRect.top, nsRect.left);
						view->adjustBackUpscaledCoordinates(nsRect.bottom, nsRect.right);
					} else if (getSciVersion() == SCI_VERSION_2_1) {
						nsRect.top = (nsRect.top * scriptsRunningHeight) / _screen->getHeight();
						nsRect.left = (nsRect.left * scriptsRunningWidth) / _screen->getWidth();
						nsRect.bottom = (nsRect.bottom * scriptsRunningHeight) / _screen->getHeight();
						nsRect.right = (nsRect.right * scriptsRunningWidth) / _screen->getWidth();
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
					clipRect.clip(it->upscaledPlaneClipRect);
					translatedClipRect = clipRect;
					translatedClipRect.translate(it->upscaledPlaneRect.left, it->upscaledPlaneRect.top);
				} else {
					clipRect.clip(it->planeClipRect);
					translatedClipRect = clipRect;
					translatedClipRect.translate(it->planeRect.left, it->planeRect.top);
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
					reg_t stringObject = readSelector(_segMan, itemEntry->object, SELECTOR(text));

					// The object in the text selector of the item can be either a raw string
					// or a Str object. In the latter case, we need to access the object's data
					// selector to get the raw string.
					if (_segMan->isHeapObject(stringObject))
						stringObject = readSelector(_segMan, stringObject, SELECTOR(data));

					Common::String text = _segMan->getString(stringObject);
					GfxFont *font = _cache->getFont(readSelectorValue(_segMan, itemEntry->object, SELECTOR(font)));
					bool dimmed = readSelectorValue(_segMan, itemEntry->object, SELECTOR(dimmed));
					uint16 foreColor = readSelectorValue(_segMan, itemEntry->object, SELECTOR(fore));

					itemEntry->y = ((itemEntry->y * _screen->getHeight()) / scriptsRunningHeight);
					itemEntry->x = ((itemEntry->x * _screen->getWidth()) / scriptsRunningWidth);

					uint16 startX = itemEntry->x + it->planeRect.left;
					uint16 curY = itemEntry->y + it->planeRect.top;
					const char *txt = text.c_str();
					// HACK. The plane sometimes doesn't contain the correct width. This
					// hack breaks the dialog options when speaking with Grace, but it's
					// the best we got up to now. This happens because of the unimplemented
					// kTextWidth function in SCI32.
					// TODO: Remove this once kTextWidth has been implemented.
					uint16 w = it->planeRect.width() >= 20 ? it->planeRect.width() : _screen->getWidth() - 10;
					int16 charCount;

					// Upscale the coordinates/width if the fonts are already upscaled
					if (_screen->fontIsUpscaled()) {
						startX = startX * _screen->getDisplayWidth() / _screen->getWidth();
						curY = curY * _screen->getDisplayHeight() / _screen->getHeight();
						w  = w * _screen->getDisplayWidth() / _screen->getWidth();
					}

					while (*txt) {
						charCount = GetLongest(txt, w, font);
						if (charCount == 0)
							break;

						uint16 curX = startX;

						for (int i = 0; i < charCount; i++) {
							unsigned char curChar = txt[i];
							font->draw(curChar, curY, curX, foreColor, dimmed);
							curX += font->getCharWidth(curChar);
						}

						curY += font->getHeight();
						txt += charCount;
						while (*txt == ' ')
							txt++; // skip over breaking spaces
					}

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
