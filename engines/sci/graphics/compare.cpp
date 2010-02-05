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
#include "sci/graphics/compare.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/view.h"

namespace Sci {

GfxCompare::GfxCompare(SegManager *segMan, Kernel *kernel, GfxCache *cache, GfxScreen *screen, GfxCoordAdjuster *coordAdjuster)
	: _segMan(segMan), _kernel(kernel), _cache(cache), _screen(screen), _coordAdjuster(coordAdjuster) {
}

GfxCompare::~GfxCompare() {
}

uint16 GfxCompare::isOnControl(uint16 screenMask, Common::Rect rect) {
	int16 x, y;
	uint16 result = 0;

	if (rect.isEmpty())
		return 0;

	if (screenMask & SCI_SCREEN_MASK_PRIORITY) {
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

static inline int sign_extend_byte(int value) {
	if (value & 0x80)
		return value - 256;
	else
		return value;
}

bool GfxCompare::canBeHereCheckRectList(reg_t checkObject, Common::Rect checkRect, List *list) {
	reg_t curAddress = list->first;
	Node *curNode = _segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 signal;
	Common::Rect curRect;

	while (curNode) {
		curObject = curNode->value;
		if (curObject != checkObject) {
			signal = GET_SEL32V(_segMan, curObject, SELECTOR(signal));
			if ((signal & (kSignalIgnoreActor | kSignalRemoveView | kSignalNoUpdate)) == 0) {
				curRect.left = GET_SEL32V(_segMan, curObject, SELECTOR(brLeft));
				curRect.top = GET_SEL32V(_segMan, curObject, SELECTOR(brTop));
				curRect.right = GET_SEL32V(_segMan, curObject, SELECTOR(brRight));
				curRect.bottom = GET_SEL32V(_segMan, curObject, SELECTOR(brBottom));
				// Check if curRect is within checkRect
				// TODO: This check is slightly odd, because it means that a rect is not contained
				// in itself. It may very well be that the original SCI engine did it just
				// this way, so it should not be changed lightly. However, somebody should
				// confirm whether the original engine really did it this way. Then, update
				// this comment accordingly, and, if necessary, fix the code.
				if (curRect.right > checkRect.left &&
				    curRect.left < checkRect.right &&
				    curRect.bottom > checkRect.top &&
				    curRect.top < checkRect.bottom) {
					return false;
				}
			}
		}
		curAddress = curNode->succ;
		curNode = _segMan->lookupNode(curAddress);
	}
	return true;
}

uint16 GfxCompare::kernelOnControl(byte screenMask, Common::Rect rect) {
	Common::Rect adjustedRect = _coordAdjuster->onControl(rect);

	uint16 result = isOnControl(screenMask, adjustedRect);
	return result;
}

void GfxCompare::kernelSetNowSeen(reg_t objectReference) {
	GfxView *view = NULL;
	Common::Rect celRect(0, 0);
	GuiResourceId viewId = (GuiResourceId)GET_SEL32V(_segMan, objectReference, SELECTOR(view));
	int16 loopNo = sign_extend_byte((int16)GET_SEL32V(_segMan, objectReference, SELECTOR(loop)));
	int16 celNo = sign_extend_byte((int16)GET_SEL32V(_segMan, objectReference, SELECTOR(cel)));
	int16 x = (int16)GET_SEL32V(_segMan, objectReference, SELECTOR(x));
	int16 y = (int16)GET_SEL32V(_segMan, objectReference, SELECTOR(y));
	int16 z = 0;
	if (_kernel->_selectorCache.z > -1)
		z = (int16)GET_SEL32V(_segMan, objectReference, SELECTOR(z));

	// now get cel rectangle
	view = _cache->getView(viewId);
	view->getCelRect(loopNo, celNo, x, y, z, &celRect);

	// TODO: sometimes loop is negative. Check what it means
	if (lookup_selector(_segMan, objectReference, _kernel->_selectorCache.nsTop, NULL, NULL) == kSelectorVariable) {
		PUT_SEL32V(_segMan, objectReference, SELECTOR(nsLeft), celRect.left);
		PUT_SEL32V(_segMan, objectReference, SELECTOR(nsRight), celRect.right);
		PUT_SEL32V(_segMan, objectReference, SELECTOR(nsTop), celRect.top);
		PUT_SEL32V(_segMan, objectReference, SELECTOR(nsBottom), celRect.bottom);
	}
}

bool GfxCompare::kernelCanBeHere(reg_t curObject, reg_t listReference) {
	Common::Rect checkRect;
	Common::Rect adjustedRect;
	uint16 signal, controlMask;
	bool result;

	checkRect.left = GET_SEL32V(_segMan, curObject, SELECTOR(brLeft));
	checkRect.top = GET_SEL32V(_segMan, curObject, SELECTOR(brTop));
	checkRect.right = GET_SEL32V(_segMan, curObject, SELECTOR(brRight));
	checkRect.bottom = GET_SEL32V(_segMan, curObject, SELECTOR(brBottom));

	adjustedRect = _coordAdjuster->onControl(checkRect);

	signal = GET_SEL32V(_segMan, curObject, SELECTOR(signal));
	controlMask = GET_SEL32V(_segMan, curObject, SELECTOR(illegalBits));
	result = (isOnControl(SCI_SCREEN_MASK_CONTROL, adjustedRect) & controlMask) ? false : true;
	if ((result) && (signal & (kSignalIgnoreActor | kSignalRemoveView)) == 0) {
		List *list = _segMan->lookupList(listReference);
		if (!list)
			error("kCanBeHere called with non-list as parameter");

		result = canBeHereCheckRectList(curObject, checkRect, list);
	}
	return result;
}

bool GfxCompare::kernelIsItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position) {
	GfxView *tmpView = _cache->getView(viewId);
	CelInfo *celInfo = tmpView->getCelInfo(loopNo, celNo);
	position.x = CLIP<int>(position.x, 0, celInfo->width - 1);
	position.y = CLIP<int>(position.y, 0, celInfo->height - 1);
	byte *celData = tmpView->getBitmap(loopNo, celNo);
	bool result = (celData[position.y * celInfo->width + position.x] == celInfo->clearKey);
	return result;
}

void GfxCompare::kernelBaseSetter(reg_t object) {
	if (lookup_selector(_segMan, object, _kernel->_selectorCache.brLeft, NULL, NULL) == kSelectorVariable) {
		int16 x = GET_SEL32V(_segMan, object, SELECTOR(x));
		int16 y = GET_SEL32V(_segMan, object, SELECTOR(y));
		int16 z = (_kernel->_selectorCache.z > -1) ? GET_SEL32V(_segMan, object, SELECTOR(z)) : 0;
		int16 yStep = GET_SEL32V(_segMan, object, SELECTOR(yStep));
		GuiResourceId viewId = GET_SEL32V(_segMan, object, SELECTOR(view));
		int16 loopNo = GET_SEL32V(_segMan, object, SELECTOR(loop));
		int16 celNo = GET_SEL32V(_segMan, object, SELECTOR(cel));

		GfxView *tmpView = _cache->getView(viewId);
		Common::Rect celRect;

		tmpView->getCelRect(loopNo, celNo, x, y, z, &celRect);
		celRect.bottom = y + 1;
		celRect.top = celRect.bottom - yStep;

		PUT_SEL32V(_segMan, object, SELECTOR(brLeft), celRect.left);
		PUT_SEL32V(_segMan, object, SELECTOR(brRight), celRect.right);
		PUT_SEL32V(_segMan, object, SELECTOR(brTop), celRect.top);
		PUT_SEL32V(_segMan, object, SELECTOR(brBottom), celRect.bottom);
	}
}

} // End of namespace Sci
