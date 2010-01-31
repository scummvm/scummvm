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
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/view.h"

namespace Sci {

GfxCompare::GfxCompare(SegManager *segMan, Kernel *kernel, GfxCache *cache, GfxScreen *screen)
	: _segMan(segMan), _kernel(kernel), _cache(cache), _screen(screen) {
}

GfxCompare::~GfxCompare() {
}

uint16 GfxCompare::onControl(uint16 screenMask, Common::Rect rect) {
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

bool GfxCompare::CanBeHereCheckRectList(reg_t checkObject, Common::Rect checkRect, List *list) {
	reg_t curAddress = list->first;
	Node *curNode = _segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 signal;
	Common::Rect curRect;

	while (curNode) {
		curObject = curNode->value;
		if (curObject != checkObject) {
			signal = GET_SEL32V(_segMan, curObject, signal);
			if ((signal & (kSignalIgnoreActor | kSignalRemoveView | kSignalNoUpdate)) == 0) {
				curRect.left = GET_SEL32V(_segMan, curObject, brLeft);
				curRect.top = GET_SEL32V(_segMan, curObject, brTop);
				curRect.right = GET_SEL32V(_segMan, curObject, brRight);
				curRect.bottom = GET_SEL32V(_segMan, curObject, brBottom);
				// Check if curRect is within checkRect
				if (curRect.right > checkRect.left && curRect.left < checkRect.right && curRect.bottom > checkRect.top && curRect.top < checkRect.bottom) {
					return false;
				}
			}
		}
		curAddress = curNode->succ;
		curNode = _segMan->lookupNode(curAddress);
	}
	return true;
}

void GfxCompare::SetNowSeen(reg_t objectReference) {
	View *view = NULL;
	Common::Rect celRect(0, 0);
	GuiResourceId viewId = (GuiResourceId)GET_SEL32V(_segMan, objectReference, view);
	int16 loopNo = sign_extend_byte((int16)GET_SEL32V(_segMan, objectReference, loop));
	int16 celNo = sign_extend_byte((int16)GET_SEL32V(_segMan, objectReference, cel));
	int16 x = (int16)GET_SEL32V(_segMan, objectReference, x);
	int16 y = (int16)GET_SEL32V(_segMan, objectReference, y);
	int16 z = 0;
	if (_kernel->_selectorCache.z > -1)
		z = (int16)GET_SEL32V(_segMan, objectReference, z);

	// now get cel rectangle
	view = _cache->getView(viewId);
	view->getCelRect(loopNo, celNo, x, y, z, &celRect);

	// TODO: sometimes loop is negative. Check what it means
	if (lookup_selector(_segMan, objectReference, _kernel->_selectorCache.nsTop, NULL, NULL) == kSelectorVariable) {
		PUT_SEL32V(_segMan, objectReference, nsLeft, celRect.left);
		PUT_SEL32V(_segMan, objectReference, nsRight, celRect.right);
		PUT_SEL32V(_segMan, objectReference, nsTop, celRect.top);
		PUT_SEL32V(_segMan, objectReference, nsBottom, celRect.bottom);
	}
}

} // End of namespace Sci
