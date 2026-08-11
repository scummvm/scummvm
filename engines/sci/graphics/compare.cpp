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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/view.h"

namespace Sci {

GfxCompare::GfxCompare(SegManager *segMan, GfxCache *cache, GfxScreen *screen, GfxCoordAdjuster16 *coordAdjuster)
	: _segMan(segMan), _cache(cache), _screen(screen), _coordAdjuster(coordAdjuster) {
}

GfxCompare::~GfxCompare() {
}

uint16 GfxCompare::isOnControl(uint16 screenMask, const Common::Rect &rect) {
	int16 x, y;
	uint16 result = 0;

	if (rect.isEmpty())
		return 0;

	if (screenMask & GFX_SCREEN_MASK_PRIORITY) {
		for (y = rect.top; y < rect.bottom; y++) {
			for (x = rect.left; x < rect.right; x++) {
				result |= 1 << _screen->getPriority(x, y);
			}
		}
	} else {
		for (y = rect.top; y < rect.bottom; y++) {
			for (x = rect.left; x < rect.right; x++) {
				result |= 1 << _screen->getControl(x, y);
			}
		}
	}
	return result;
}

reg_t GfxCompare::canBeHereCheckRectList(const reg_t checkObject, const Common::Rect &checkRect, const List *list, const uint16 signalFlags) const {
	reg_t curAddress = list->first;
	Node *curNode = _segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 signal;
	Common::Rect curRect;

	while (curNode) {
		curObject = curNode->value;
		if (curObject != checkObject) {
			signal = readSelectorValue(_segMan, curObject, SELECTOR(signal));
			if (!(signal & signalFlags)) {
				curRect.left = readSelectorValue(_segMan, curObject, SELECTOR(brLeft));
				curRect.top = readSelectorValue(_segMan, curObject, SELECTOR(brTop));
				curRect.right = readSelectorValue(_segMan, curObject, SELECTOR(brRight));
				curRect.bottom = readSelectorValue(_segMan, curObject, SELECTOR(brBottom));
				// Check if curRect is within checkRect
				// This behavior is slightly odd, but it's how the original SCI
				// engine did it: a rect cannot be contained within itself
				// (there is no equality). Do NOT change this to contains(), as
				// it breaks KQ4 early (bug #5755).
				if (curRect.right > checkRect.left &&
					curRect.left < checkRect.right &&
					curRect.bottom > checkRect.top &&
					curRect.top < checkRect.bottom)
					return curObject;
			}
		}
		curAddress = curNode->succ;
		curNode = _segMan->lookupNode(curAddress);
	}
	return NULL_REG;
}

uint16 GfxCompare::kernelOnControl(byte screenMask, const Common::Rect &rect) {
	Common::Rect adjustedRect = _coordAdjuster->onControl(rect);

	uint16 result = isOnControl(screenMask, adjustedRect);
	return result;
}

void GfxCompare::kernelSetNowSeen(reg_t objectReference) {
	GfxView *view = nullptr;
	Common::Rect celRect(0, 0);
	GuiResourceId viewId = (GuiResourceId)readSelectorValue(_segMan, objectReference, SELECTOR(view));
	int16 loopNo = readSelectorValue(_segMan, objectReference, SELECTOR(loop));
	int16 celNo = readSelectorValue(_segMan, objectReference, SELECTOR(cel));
	int16 x = (int16)readSelectorValue(_segMan, objectReference, SELECTOR(x));
	int16 y = (int16)readSelectorValue(_segMan, objectReference, SELECTOR(y));
	int16 z = 0;
	if (SELECTOR(z) > -1)
		z = (int16)readSelectorValue(_segMan, objectReference, SELECTOR(z));

	view = _cache->getView(viewId);
	view->getCelRect(loopNo, celNo, x, y, z, celRect);

	if (lookupSelector(_segMan, objectReference, SELECTOR(nsTop), nullptr, nullptr) == kSelectorVariable) {
		setNSRect(objectReference, celRect);
	}
}

reg_t GfxCompare::kernelCanBeHere(reg_t curObject, reg_t listReference) {
	Common::Rect checkRect;
	uint16 result;

	checkRect.left = readSelectorValue(_segMan, curObject, SELECTOR(brLeft));
	checkRect.top = readSelectorValue(_segMan, curObject, SELECTOR(brTop));
	checkRect.right = readSelectorValue(_segMan, curObject, SELECTOR(brRight));
	checkRect.bottom = readSelectorValue(_segMan, curObject, SELECTOR(brBottom));
	uint16 signal = readSelectorValue(_segMan, curObject, SELECTOR(signal));

	if (!checkRect.isValidRect()) {	// can occur in Iceman and Mother Goose - HACK? TODO: is this really occurring in sierra sci? check this
		warning("kCan(t)BeHere - invalid rect %d, %d -> %d, %d", checkRect.left, checkRect.top, checkRect.right, checkRect.bottom);
		return NULL_REG; // this means "can be here"
	}

	Common::Rect adjustedRect = _coordAdjuster->onControl(checkRect);
	uint16 controlMask = readSelectorValue(_segMan, curObject, SELECTOR(illegalBits));
	result = isOnControl(GFX_SCREEN_MASK_CONTROL, adjustedRect) & controlMask;
	if ((!result) && (signal & (kSignalIgnoreActor | kSignalRemoveView)) == 0) {
		List *list = _segMan->lookupList(listReference);
		if (!list)
			error("kCanBeHere called with non-list as parameter");

		return canBeHereCheckRectList(curObject, checkRect, list, kSignalIgnoreActor | kSignalRemoveView | kSignalNoUpdate);
	}

	return make_reg(0, result);
}

reg_t GfxCompare::kernelCantBeHere32(const reg_t curObject, const reg_t listReference) const {
	// Most of SCI32 graphics code converts rects from the VM to exclusive
	// rects before operating on them, but this call leverages SCI16 engine
	// code that operates on inclusive rects, so the rect's bottom-right
	// point is not modified like in other SCI32 kernel calls
	Common::Rect checkRect;

	// At least LSL6 hires passes invalid rectangles which trigger the
	// isValidRect assertion in the Rect constructor; this is avoided by
	// assigning the properties after construction and then testing the
	// rect for validity ourselves here. SSCI does not care about whether
	// or not the rects are valid
	checkRect.left = readSelectorValue(_segMan, curObject, SELECTOR(brLeft));
	checkRect.top = readSelectorValue(_segMan, curObject, SELECTOR(brTop));
	checkRect.right = readSelectorValue(_segMan, curObject, SELECTOR(brRight));
	checkRect.bottom = readSelectorValue(_segMan, curObject, SELECTOR(brBottom));
	if (!checkRect.isValidRect()) {
		return make_reg(0, 0);
	}

	uint16 result = 0;
	uint16 signal = readSelectorValue(_segMan, curObject, SELECTOR(signal));
	const uint16 signalFlags = kSignalIgnoreActor | kSignalHidden;

	if ((signal & signalFlags) == 0) {
		List *list = _segMan->lookupList(listReference);
		if (!list) {
			error("kCantBeHere called with non-list as parameter");
		}
		result = !canBeHereCheckRectList(curObject, checkRect, list, signalFlags).isNull();
	}

	return make_reg(0, result);
}

bool GfxCompare::kernelIsItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position) {
	GfxView *tmpView = _cache->getView(viewId);
	const CelInfo *celInfo = tmpView->getCelInfo(loopNo, celNo);
	position.x = CLIP<int>(position.x, 0, celInfo->width - 1);
	position.y = CLIP<int>(position.y, 0, celInfo->height - 1);
	const SciSpan<const byte> &celData = tmpView->getBitmap(loopNo, celNo);
	bool result = (celData[position.y * celInfo->width + position.x] == celInfo->clearKey);
	return result;
}

void GfxCompare::kernelBaseSetter(reg_t object) {
	if (lookupSelector(_segMan, object, SELECTOR(brLeft), nullptr, nullptr) == kSelectorVariable) {
		int16 x = readSelectorValue(_segMan, object, SELECTOR(x));
		int16 y = readSelectorValue(_segMan, object, SELECTOR(y));
		int16 z = (SELECTOR(z) > -1) ? readSelectorValue(_segMan, object, SELECTOR(z)) : 0;
		int16 yStep = readSelectorValue(_segMan, object, SELECTOR(yStep));
		GuiResourceId viewId = readSelectorValue(_segMan, object, SELECTOR(view));
		int16 loopNo = readSelectorValue(_segMan, object, SELECTOR(loop));
		int16 celNo = readSelectorValue(_segMan, object, SELECTOR(cel));
		uint16 scaleSignal = 0;
		if (getSciVersion() >= SCI_VERSION_1_1)
			scaleSignal = readSelectorValue(_segMan, object, SELECTOR(scaleSignal));

		Common::Rect celRect;

		GfxView *tmpView = _cache->getView(viewId);
		if (!tmpView->isScaleable())
			scaleSignal = 0;

		if (scaleSignal & kScaleSignalDoScaling) {
			celRect = getNSRect(object);
		} else {
			tmpView->getCelRect(loopNo, celNo, x, y, z, celRect);
		}

		celRect.bottom = y + 1;
		celRect.top = celRect.bottom - yStep;

		writeSelectorValue(_segMan, object, SELECTOR(brLeft), celRect.left);
		writeSelectorValue(_segMan, object, SELECTOR(brRight), celRect.right);
		writeSelectorValue(_segMan, object, SELECTOR(brTop), celRect.top);
		writeSelectorValue(_segMan, object, SELECTOR(brBottom), celRect.bottom);
	}
}

Common::Rect GfxCompare::getNSRect(reg_t object) {
	Common::Rect nsRect;
	nsRect.top = readSelectorValue(_segMan, object, SELECTOR(nsTop));
	nsRect.left = readSelectorValue(_segMan, object, SELECTOR(nsLeft));
	nsRect.bottom = readSelectorValue(_segMan, object, SELECTOR(nsBottom));
	nsRect.right = readSelectorValue(_segMan, object, SELECTOR(nsRight));

	return nsRect;
}

void GfxCompare::setNSRect(reg_t object, Common::Rect nsRect) {
	writeSelectorValue(_segMan, object, SELECTOR(nsLeft), nsRect.left);
	writeSelectorValue(_segMan, object, SELECTOR(nsTop), nsRect.top);
	writeSelectorValue(_segMan, object, SELECTOR(nsRight), nsRect.right);
	writeSelectorValue(_segMan, object, SELECTOR(nsBottom), nsRect.bottom);
}

} // End of namespace Sci
