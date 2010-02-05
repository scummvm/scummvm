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
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/font.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/transitions.h"

namespace Sci {

GfxPaint16::GfxPaint16(ResourceManager *resMan, SegManager *segMan, Kernel *kernel, GfxCache *cache, GfxPorts *ports, GfxCoordAdjuster *coordAdjuster, GfxScreen *screen, GfxPalette *palette, GfxTransitions *transitions)
	: _resMan(resMan), _segMan(segMan), _kernel(kernel), _cache(cache), _ports(ports), _coordAdjuster(coordAdjuster), _screen(screen), _palette(palette), _transitions(transitions) {
}

GfxPaint16::~GfxPaint16() {
}

void GfxPaint16::init(GfxAnimate *animate, GfxText16 *text16) {
	_animate = animate;
	_text16 = text16;

	_EGAdrawingVisualize = false;
}

void GfxPaint16::setEGAdrawingVisualize(bool state) {
	_EGAdrawingVisualize = state;
}

void GfxPaint16::drawPicture(GuiResourceId pictureId, int16 animationNr, bool mirroredFlag, bool addToFlag, GuiResourceId paletteId) {
	GfxPicture *picture = new GfxPicture(_resMan, _ports, _screen, _palette, pictureId, _EGAdrawingVisualize);

	// do we add to a picture? if not -> clear screen with white
	if (!addToFlag)
		clearScreen(_screen->getColorWhite());

	picture->draw(animationNr, mirroredFlag, addToFlag, paletteId);
	delete picture;
}

// This one is the only one that updates screen!
void GfxPaint16::drawCelAndShow(GuiResourceId viewId, int16 loopNo, int16 celNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo, uint16 scaleX, uint16 scaleY) {
	GfxView *view = _cache->getView(viewId);
	Common::Rect celRect;

	if (view) {
		celRect.left = leftPos;
		celRect.top = topPos;
		celRect.right = celRect.left + view->getWidth(loopNo, celNo);
		celRect.bottom = celRect.top + view->getHeight(loopNo, celNo);

		drawCel(view, loopNo, celNo, celRect, priority, paletteNo, scaleX, scaleY);

		if (getSciVersion() >= SCI_VERSION_1_1) {
			if (!_screen->_picNotValidSci11) {
				bitsShow(celRect);
			}
		} else {
			if (!_screen->_picNotValid)
				bitsShow(celRect);
		}
	}
}

// This version of drawCel is not supposed to call BitsShow()!
void GfxPaint16::drawCel(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Rect celRect, byte priority, uint16 paletteNo, uint16 scaleX, uint16 scaleY) {
	drawCel(_cache->getView(viewId), loopNo, celNo, celRect, priority, paletteNo, scaleX, scaleY);
}

// This version of drawCel is not supposed to call BitsShow()!
void GfxPaint16::drawCel(GfxView *view, int16 loopNo, int16 celNo, Common::Rect celRect, byte priority, uint16 paletteNo, uint16 scaleX, uint16 scaleY) {
	Common::Rect clipRect = celRect;
	clipRect.clip(_ports->_curPort->rect);
	if (clipRect.isEmpty()) // nothing to draw
		return;

	Common::Rect clipRectTranslated = clipRect;
	_ports->offsetRect(clipRectTranslated);
	if (scaleX == 128 && scaleY == 128) {
		view->draw(celRect, clipRect, clipRectTranslated, loopNo, celNo, priority, paletteNo, false);
	} else {
		view->drawScaled(celRect, clipRect, clipRectTranslated, loopNo, celNo, priority, scaleX, scaleY);
	}
}

// This is used as replacement for drawCelAndShow() when hires-cels are drawn to screen
//  Hires-cels are available only SCI 1.1+
void GfxPaint16::drawHiresCelAndShow(GuiResourceId viewId, int16 loopNo, int16 celNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo, reg_t upscaledHiresHandle, uint16 scaleX, uint16 scaleY) {
	GfxView *view = _cache->getView(viewId);
	Common::Rect celRect, curPortRect, clipRect, clipRectTranslated;
	Common::Point curPortPos;
	bool upscaledHiresHack = false;

	if (view) {
		if ((leftPos == 0) && (topPos == 0)) {
			// HACK: in kq6, we get leftPos&topPos == 0 SOMETIMES, that's why we need to get coordinates from upscaledHiresHandle
			//  I'm not sure if this is what we are supposed to do or if there is some other bug that actually makes
			//  coordinates to be 0 in the first place
			byte *memoryPtr = NULL;
			memoryPtr = kmem(_segMan, upscaledHiresHandle);
			if (memoryPtr) {
				Common::Rect upscaledHiresRect;
				_screen->bitsGetRect(memoryPtr, &upscaledHiresRect);
				leftPos = upscaledHiresRect.left;
				topPos = upscaledHiresRect.top;
				upscaledHiresHack = true;
			}
		}

		celRect.left = leftPos;
		celRect.top = topPos;
		celRect.right = celRect.left + view->getWidth(loopNo, celNo);
		celRect.bottom = celRect.top + view->getHeight(loopNo, celNo);
		// adjust curPort to upscaled hires
		clipRect = celRect;
		curPortRect = _ports->_curPort->rect;
		curPortRect.top *= 2; curPortRect.bottom *= 2; curPortRect.bottom++;
		curPortRect.left *= 2; curPortRect.right *= 2; curPortRect.right++;
		clipRect.clip(curPortRect);
		if (clipRect.isEmpty()) // nothing to draw
			return;

		clipRectTranslated = clipRect;
		if (!upscaledHiresHack) {
			curPortPos.x = _ports->_curPort->left * 2; curPortPos.y = _ports->_curPort->top * 2;
			clipRectTranslated.top += curPortPos.y; clipRectTranslated.bottom += curPortPos.y;
			clipRectTranslated.left += curPortPos.x; clipRectTranslated.right += curPortPos.x;
		}

		view->draw(celRect, clipRect, clipRectTranslated, loopNo, celNo, priority, paletteNo, true);
		if (!_screen->_picNotValidSci11) {
			_screen->copyDisplayRectToScreen(clipRectTranslated);
		}
	}
}

void GfxPaint16::clearScreen(byte color) {
	fillRect(_ports->_curPort->rect, SCI_SCREEN_MASK_ALL, color, 0, 0);
}

void GfxPaint16::invertRect(const Common::Rect &rect) {
	int16 oldpenmode = _ports->_curPort->penMode;
	_ports->_curPort->penMode = 2;
	fillRect(rect, 1, _ports->_curPort->penClr, _ports->_curPort->backClr);
	_ports->_curPort->penMode = oldpenmode;
}

void GfxPaint16::eraseRect(const Common::Rect &rect) {
	fillRect(rect, 1, _ports->_curPort->backClr);
}

void GfxPaint16::paintRect(const Common::Rect &rect) {
	fillRect(rect, 1, _ports->_curPort->penClr);
}

void GfxPaint16::fillRect(const Common::Rect &rect, int16 drawFlags, byte clrPen, byte clrBack, byte bControl) {
	Common::Rect r = rect;
	r.clip(_ports->_curPort->rect);
	if (r.isEmpty()) // nothing to fill
		return;

	int16 oldPenMode = _ports->_curPort->penMode;
	_ports->offsetRect(r);
	int16 x, y;
	byte curVisual;

	// Doing visual first
	if (drawFlags & SCI_SCREEN_MASK_VISUAL) {
		if (oldPenMode == 2) { // invert mode
			for (y = r.top; y < r.bottom; y++) {
				for (x = r.left; x < r.right; x++) {
					curVisual = _screen->getVisual(x, y);
					if (curVisual == clrPen) {
						_screen->putPixel(x, y, SCI_SCREEN_MASK_VISUAL, clrBack, 0, 0);
					} else if (curVisual == clrBack) {
						_screen->putPixel(x, y, SCI_SCREEN_MASK_VISUAL, clrPen, 0, 0);
					}
				}
			}
		} else { // just fill rect with ClrPen
			for (y = r.top; y < r.bottom; y++) {
				for (x = r.left; x < r.right; x++) {
					_screen->putPixel(x, y, SCI_SCREEN_MASK_VISUAL, clrPen, 0, 0);
				}
			}
		}
	}

	if (drawFlags < 2)
		return;
	drawFlags &= SCI_SCREEN_MASK_PRIORITY|SCI_SCREEN_MASK_CONTROL;

	if (oldPenMode != 2) {
		for (y = r.top; y < r.bottom; y++) {
			for (x = r.left; x < r.right; x++) {
				_screen->putPixel(x, y, drawFlags, 0, clrBack, bControl);
			}
		}
	} else {
		for (y = r.top; y < r.bottom; y++) {
			for (x = r.left; x < r.right; x++) {
				_screen->putPixel(x, y, drawFlags, 0, !_screen->getPriority(x, y), !_screen->getControl(x, y));
			}
		}
	}
}

void GfxPaint16::frameRect(const Common::Rect &rect) {
	Common::Rect r;
	// left
	r = rect;
	r.right = rect.left + 1;
	paintRect(r);
	// right
	r.right = rect.right;
	r.left = rect.right - 1;
	paintRect(r);
	//top
	r.left = rect.left;
	r.bottom = rect.top + 1;
	paintRect(r);
	//bottom
	r.bottom = rect.bottom;
	r.top = rect.bottom - 1;
	paintRect(r);
}

void GfxPaint16::bitsShow(const Common::Rect &rect) {
	Common::Rect workerRect(rect.left, rect.top, rect.right, rect.bottom);
	workerRect.clip(_ports->_curPort->rect);
	if (workerRect.isEmpty()) // nothing to show
		return;

	_ports->offsetRect(workerRect);
	_screen->copyRectToScreen(workerRect);
}

void GfxPaint16::bitsShowHires(const Common::Rect &rect) {
	_screen->copyDisplayRectToScreen(rect);
}

reg_t GfxPaint16::bitsSave(const Common::Rect &rect, byte screenMask) {
	reg_t memoryId;
	byte *memoryPtr;
	int size;

	Common::Rect workerRect(rect.left, rect.top, rect.right, rect.bottom);
	workerRect.clip(_ports->_curPort->rect);
	if (workerRect.isEmpty()) // nothing to save
		return NULL_REG;

	if (screenMask == SCI_SCREEN_MASK_DISPLAY) {
		// Adjust rect to upscaled hires, but dont adjust according to port
		workerRect.top *= 2; workerRect.bottom *= 2; workerRect.bottom++;
		workerRect.left *= 2; workerRect.right *= 2; workerRect.right++;
	} else {
		_ports->offsetRect(workerRect);
	}

	// now actually ask _screen how much space it will need for saving
	size = _screen->bitsGetDataSize(workerRect, screenMask);

	memoryId = kalloc(_segMan, "SaveBits()", size);
	memoryPtr = kmem(_segMan, memoryId);
	_screen->bitsSave(workerRect, screenMask, memoryPtr);
	return memoryId;
}

void GfxPaint16::bitsGetRect(reg_t memoryHandle, Common::Rect *destRect) {
	byte *memoryPtr = NULL;

	if (!memoryHandle.isNull()) {
		memoryPtr = kmem(_segMan, memoryHandle);

		if (memoryPtr) {
			_screen->bitsGetRect(memoryPtr, destRect);
		}
	}
}

void GfxPaint16::bitsRestore(reg_t memoryHandle) {
	byte *memoryPtr = NULL;

	if (!memoryHandle.isNull()) {
		memoryPtr = kmem(_segMan, memoryHandle);

		if (memoryPtr) {
			_screen->bitsRestore(memoryPtr);
			kfree(_segMan, memoryHandle);
		}
	}
}

void GfxPaint16::bitsFree(reg_t memoryHandle) {
	if (!memoryHandle.isNull()) {
		kfree(_segMan, memoryHandle);
	}
}

void GfxPaint16::kernelDrawPicture(GuiResourceId pictureId, int16 animationNr, bool animationBlackoutFlag, bool mirroredFlag, bool addToFlag, int16 EGApaletteNo) {
	Port *oldPort = _ports->setPort((Port *)_ports->_picWind);

	if (_ports->isFrontWindow(_ports->_picWind)) {
		_screen->_picNotValid = 1;
		drawPicture(pictureId, animationNr, mirroredFlag, addToFlag, EGApaletteNo);
		_transitions->setup(animationNr, animationBlackoutFlag);
	} else {
		_ports->beginUpdate(_ports->_picWind);
		drawPicture(pictureId, animationNr, mirroredFlag, addToFlag, EGApaletteNo);
		_ports->endUpdate(_ports->_picWind);
	}
	_ports->setPort(oldPort);
}

void GfxPaint16::kernelDrawCel(GuiResourceId viewId, int16 loopNo, int16 celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo, bool hiresMode, reg_t upscaledHiresHandle) {
	// some calls are hiresMode even under kq6 DOS, that's why we check for upscaled hires here
	if ((!hiresMode) || (!_screen->getUpscaledHires())) {
		drawCelAndShow(viewId, loopNo, celNo, leftPos, topPos, priority, paletteNo);
	} else {
		drawHiresCelAndShow(viewId, loopNo, celNo, leftPos, topPos, priority, paletteNo, upscaledHiresHandle);
	}
	_palette->setOnScreen();
}

void GfxPaint16::kernelGraphFillBoxForeground(Common::Rect rect) {
	paintRect(rect);
}

void GfxPaint16::kernelGraphFillBoxBackground(Common::Rect rect) {
	eraseRect(rect);
}

void GfxPaint16::kernelGraphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control) {
	fillRect(rect, colorMask, color, priority, control);
}

void GfxPaint16::kernelGraphFrameBox(Common::Rect rect, int16 color) {
	int16 oldColor = _ports->getPort()->penClr;
	_ports->penColor(color);
	frameRect(rect);
	_ports->penColor(oldColor);
}

void GfxPaint16::kernelGraphDrawLine(Common::Point startPoint, Common::Point endPoint, int16 color, int16 priority, int16 control) {
	_ports->offsetLine(startPoint, endPoint);
	_screen->drawLine(startPoint.x, startPoint.y, endPoint.x, endPoint.y, color, priority, control);
}

reg_t GfxPaint16::kernelGraphSaveBox(Common::Rect rect, uint16 screenMask) {
	return bitsSave(rect, screenMask);
}

reg_t GfxPaint16::kernelGraphSaveUpscaledHiresBox(Common::Rect rect) {
	return bitsSave(rect, SCI_SCREEN_MASK_DISPLAY);
}

void GfxPaint16::kernelGraphRestoreBox(reg_t handle) {
	bitsRestore(handle);
}

void GfxPaint16::kernelGraphUpdateBox(Common::Rect rect, bool hiresMode) {
	// some calls are hiresMode even under kq6 DOS, that's why we check for upscaled hires here
	if ((!hiresMode) || (!_screen->getUpscaledHires()))
		bitsShow(rect);
	else
		bitsShowHires(rect);
}

void GfxPaint16::kernelGraphRedrawBox(Common::Rect rect) {
	_coordAdjuster->kernelLocalToGlobal(rect.left, rect.top);
	_coordAdjuster->kernelLocalToGlobal(rect.right, rect.bottom);
	Port *oldPort = _ports->setPort((Port *)_ports->_picWind);
	_coordAdjuster->kernelGlobalToLocal(rect.left, rect.top);
	_coordAdjuster->kernelGlobalToLocal(rect.right, rect.bottom);

	_animate->reAnimate(rect);

	_ports->setPort(oldPort);
}

} // End of namespace Sci
