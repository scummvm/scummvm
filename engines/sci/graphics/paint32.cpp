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
#include "sci/graphics/cache.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/font.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"

namespace Sci {

GfxPaint32::GfxPaint32(ResourceManager *resMan, SegManager *segMan, Kernel *kernel, GfxCache *cache, GfxScreen *screen, GfxPalette *palette)
	: _resMan(resMan), _segMan(segMan), _kernel(kernel), _cache(cache), _screen(screen), _palette(palette) {
}

GfxPaint32::~GfxPaint32() {
}

void GfxPaint32::fillRect(Common::Rect rect, byte color) {
	int16 y, x;
	for (y = rect.top; y < rect.bottom; y++) {
		for (x = rect.left; x < rect.right; x++) {
			_screen->putPixel(x, y, SCI_SCREEN_MASK_VISUAL, color, 0, 0);
		}
	}
}

void GfxPaint32::kernelDrawPicture(GuiResourceId pictureId, int16 animationNr, bool animationBlackoutFlag, bool mirroredFlag, bool addToFlag, int16 EGApaletteNo) {
	SciGuiPicture *picture = new SciGuiPicture(_resMan, 0, _screen, _palette, pictureId, false);

	picture->draw(animationNr, mirroredFlag, addToFlag, EGApaletteNo);
	delete picture;
}

// This is "hacked" together, because its only used by debug command
void GfxPaint32::kernelDrawCel(GuiResourceId viewId, int16 loopNo, int16 celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo, bool hiresMode, reg_t upscaledHiresHandle) {
	GfxView *view = _cache->getView(viewId);
	Common::Rect celRect(50, 50, 50, 50);
	Common::Rect translatedRect;
	celRect.bottom += view->getHeight(loopNo, celNo);
	celRect.right += view->getWidth(loopNo, celNo);
	view->draw(celRect, celRect, celRect, loopNo, celNo, 255, 0, false);
	_screen->copyRectToScreen(celRect);
}

void GfxPaint32::kernelGraphDrawLine(Common::Point startPoint, Common::Point endPoint, int16 color, int16 priority, int16 control) {
	_screen->drawLine(startPoint.x, startPoint.y, endPoint.x, endPoint.y, color, priority, control);
}

} // End of namespace Sci
