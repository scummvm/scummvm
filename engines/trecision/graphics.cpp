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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "trecision/3d.h"
#include "trecision/actor.h"
#include "trecision/anim.h"
#include "trecision/defines.h"
#include "trecision/graphics.h"
#include "trecision/text.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {

GraphicsManager::GraphicsManager(TrecisionEngine *vm) : _vm(vm),  _font(nullptr), kImageFormat(2, 5, 5, 5, 0, 10, 5, 0, 0) {	
	// RGB555
	_drawRect = Common::Rect(0, 0, 0, 0);
	_drawObjRect = Common::Rect(0, 0, 0, 0);
	_drawObjIndex = -1;
	_drawMask = false;
	_actorRect = nullptr;
	for (int i = 0; i < 3; ++i)
		_bitMask[i] = 0;
}

GraphicsManager::~GraphicsManager() {
	_screenBuffer.free();
	_background.free();
	_smkBackground.free();
	_leftInventoryArrow.free();
	_rightInventoryArrow.free();
	_inventoryIcons.free();
	_saveSlotThumbnails.free();
	delete[] _font;
}

bool GraphicsManager::init() {
	const Graphics::PixelFormat *bestFormat = &kImageFormat;

	// Find a 16-bit format, currently we don't support other color depths
	Common::List<Graphics::PixelFormat> formats = g_system->getSupportedFormats();
	bool found = false;
	for (Common::List<Graphics::PixelFormat>::const_iterator i = formats.begin(); i != formats.end(); ++i) {
		if (i->bytesPerPixel == 2) {
			bestFormat = &*i;
			found = true;
			break;
		}
	}

	if (!found)
		return false;

	initGraphics(MAXX, MAXY, bestFormat);

	_screenFormat = g_system->getScreenFormat();
	if (_screenFormat.bytesPerPixel != 2)
		return false;
	_bitMask[0] = _screenFormat.rMax() << _screenFormat.rShift;
	_bitMask[1] = _screenFormat.gMax() << _screenFormat.gShift;
	_bitMask[2] = _screenFormat.bMax() << _screenFormat.bShift;

	clearScreen();

	_screenBuffer.create(MAXX, MAXY, _screenFormat);
	_background.create(MAXX, MAXY, _screenFormat);
	_smkBackground.create(MAXX, AREA, _screenFormat);
	_saveSlotThumbnails.create(READICON * ICONDX, ICONDY, _screenFormat);

	loadData();
	initCursor();
	hideCursor();

	return true;
}

void GraphicsManager::addDirtyRect(Common::Rect rect, bool translateRect, bool updateActorRect) {
	if (translateRect)
		rect.translate(0, TOP);

	_dirtyRects.push_back(rect);

	if (updateActorRect)
		_actorRect = &_dirtyRects.back();
}

void GraphicsManager::drawObj() {
	if (_drawObjRect.left > MAXX || _drawObjRect.top > MAXX || _drawObjRect.right > MAXX || _drawObjRect.bottom > MAXX)
		return;

	// If we have a valid object, draw it, otherwise erase it
	// by using the background buffer
	const uint16 *buf = _drawObjIndex >= 0 ? _vm->_objPointers[_drawObjIndex] : (uint16 *)_smkBackground.getPixels();
	if (_drawMask && _drawObjIndex >= 0) {
		uint8 *mask = _vm->_maskPointers[_drawObjIndex];

		for (uint16 y = _drawRect.top; y < _drawRect.bottom; ++y) {
			uint16 sco = 0;
			uint16 c = 0;
			while (sco < _drawRect.width()) {
				if (c == 0) { // jump
					sco += *mask;
					++mask;

					c = 1;
				} else { // copy
					const uint16 maskOffset = *mask;

					if (maskOffset != 0 && y >= _drawRect.top + _drawObjRect.top && y < _drawRect.top + _drawObjRect.bottom) {
						if (sco >= _drawObjRect.left && sco + maskOffset < _drawObjRect.right)
							memcpy(_screenBuffer.getBasePtr(sco + _drawRect.left, y), buf, maskOffset * 2);

						else if (sco < _drawObjRect.left && sco + maskOffset < _drawObjRect.right && sco + maskOffset >= _drawObjRect.left)
							memcpy(_screenBuffer.getBasePtr(_drawObjRect.left + _drawRect.left, y), buf + _drawObjRect.left - sco, (maskOffset + sco - _drawObjRect.left) * 2);

						else if (sco >= _drawObjRect.left && sco + maskOffset >= _drawObjRect.right && sco < _drawObjRect.right)
							memcpy(_screenBuffer.getBasePtr(sco + _drawRect.left, y), buf, (_drawObjRect.right - sco) * 2);

						else if (sco < _drawObjRect.left && sco + maskOffset >= _drawObjRect.right)
							memcpy(_screenBuffer.getBasePtr(_drawObjRect.left + _drawRect.left, y), buf + _drawObjRect.left - sco, (_drawObjRect.right - _drawObjRect.left) * 2);
					}
					sco += *mask;
					buf += *mask++;
					c = 0;
				}
			}
		}
	} else {
		const uint16 x = _drawRect.left + _drawObjRect.left;

		if (x + _drawObjRect.width() > MAXX || _drawObjRect.top + _drawObjRect.height() > MAXY) {
			warning("drawObj: Invalid surface, skipping");
			return;
		}
		
		for (uint16 y = _drawObjRect.top; y < _drawObjRect.bottom; ++y) {
			memcpy(_screenBuffer.getBasePtr(x, _drawRect.top + y),
				   buf + (y * _drawRect.width()) + _drawObjRect.left, _drawObjRect.width() * 2);
		}
	}
}

void GraphicsManager::eraseObj() {
	Common::Rect eraseRect = _drawObjRect;
	eraseRect.translate(0, TOP);
	if (eraseRect.isValidRect())
		_screenBuffer.fillRect(eraseRect, 0);
}

void GraphicsManager::clearScreen() {
	g_system->fillScreen(0);
}

void GraphicsManager::copyToScreenBuffer(const Graphics::Surface *surface, int x, int y, const byte *palette) {
	Graphics::Surface *surface16 = surface->convertTo(_screenFormat, palette);

	copyToScreenBufferInner(surface16, x, y);

	surface16->free();
	delete surface16;
}

void GraphicsManager::copyToScreenBufferInner(const Graphics::Surface *surface, int x, int y) {
	if (x + surface->w > MAXX || y + surface->h > MAXY) {
		warning("copyToScreenBufferInner: Invalid surface, skipping");
		return;
	}

	for (int curY = 0; curY < surface->h; ++curY) {
		// NOTE: We use surface width for the pitch so that memcpy works
		// correctly with surfaces from getSubArea()
		memcpy(_screenBuffer.getBasePtr(x, y + curY), surface->getBasePtr(0, curY), surface->w * 2);
	}
}

void GraphicsManager::blitToScreenBuffer(const Graphics::Surface *surface, int x, int y, const byte *palette, bool useSmkBg) {
	if (x + surface->w > MAXX || y + surface->h > MAXY) {
		warning("blitToScreenBuffer: Invalid surface, skipping");
		return;
	}

	const uint16 mask = (uint16)_screenFormat.RGBToColor(palette[0], palette[1], palette[2]);
	Graphics::Surface *surface16 = surface->convertTo(_screenFormat, palette);

	for (int curY = 0; curY < surface16->h; ++curY) {
		for (int curX = 0; curX < surface16->w; ++curX) {
			const int destX = x + curX;
			const int destY = y + curY;
			const uint16 pixel = (uint16)surface16->getPixel(curX, curY);
			if (pixel != mask) {
				_screenBuffer.setPixel(destX, destY, pixel);
				if (useSmkBg)
					_smkBackground.setPixel(destX, destY - TOP, pixel);
			} else if (useSmkBg) {
				const uint16 bgPixel = _background.getPixel(destX, destY - TOP);
				_screenBuffer.setPixel(destX, destY, bgPixel);
				_smkBackground.setPixel(destX, destY - TOP, bgPixel);
			}
		}
	}

	surface16->free();
	delete surface16;
}

void GraphicsManager::copyToScreen(int x, int y, int w, int h) {
	g_system->copyRectToScreen(
		_screenBuffer.getBasePtr(x, y),
		MAXX * 2, x, y, w, h
	);
}

uint16 *GraphicsManager::getScreenBufferPtr() {
	return (uint16 *)_screenBuffer.getPixels();
}

void GraphicsManager::readSurface(Common::SeekableReadStream *stream, Graphics::Surface *surface, uint16 width, uint16 height, uint16 count) {
	surface->create(width * count, height, kImageFormat);

	for (uint16 i = 0; i < count; ++i) {
		for (uint16 y = 0; y < height; ++y) {
			void *p = surface->getBasePtr(width * i, y);
			stream->read(p, width * kImageFormat.bytesPerPixel);
		}
	}

	surface->convertToInPlace(_screenFormat);
}

void GraphicsManager::loadBackground(Common::SeekableReadStream *stream, uint16 width, uint16 height) {
	readSurface(stream, &_background, width, height);
	_smkBackground.copyFrom(_background);
	memcpy(_screenBuffer.getBasePtr(0, TOP), _background.getPixels(), _background.pitch * _background.h);
}

void GraphicsManager::loadData() {
	Common::SeekableReadStream *arrowsDataFile = _vm->_dataFile.createReadStreamForMember("frecc.bm");
	// The data file contains images for deactivated arrows, which aren't used. Skip them.
	arrowsDataFile->skip(ICONMARGDX * ICONDY * 2 * 3);
	readSurface(arrowsDataFile, &_leftInventoryArrow, ICONMARGSX, ICONDY);
	readSurface(arrowsDataFile, &_rightInventoryArrow, ICONMARGSX, ICONDY);
	delete arrowsDataFile;

	Common::SeekableReadStream *iconsDataFile = _vm->_dataFile.createReadStreamForMember("icone.bm");
	readSurface(iconsDataFile, &_inventoryIcons, ICONDX, ICONDY, READICON);
	delete iconsDataFile;

	_font = _vm->readData("nlfont.fnt");
}

void GraphicsManager::setSaveSlotThumbnail(byte iconSlot, const Graphics::Surface *thumbnail) {
	Graphics::Surface *scaled = thumbnail->scale(ICONDX, ICONDY);
	scaled->convertToInPlace(_screenFormat);

	for (uint16 y = 0; y < ICONDY; ++y) {
		memcpy(_saveSlotThumbnails.getBasePtr(ICONDX * iconSlot, y), scaled->getBasePtr(0, y), ICONDX * 2);
	}

	scaled->free();
	delete scaled;
}

void GraphicsManager::drawLeftInventoryArrow(byte startLine) {
	Graphics::Surface arrow = _leftInventoryArrow.getSubArea(Common::Rect(
		0, startLine, _leftInventoryArrow.w, _leftInventoryArrow.h
	));
	copyToScreenBufferInner(&arrow, 0, FIRSTLINE);
}

void GraphicsManager::drawRightInventoryArrow(byte startLine) {
	Graphics::Surface arrow = _rightInventoryArrow.getSubArea(Common::Rect(
		0, startLine, _rightInventoryArrow.w, _rightInventoryArrow.h
	));
	copyToScreenBufferInner(&arrow, MAXX - ICONMARGDX, FIRSTLINE);
}

void GraphicsManager::drawInventoryIcon(byte iconIndex, byte iconSlot, byte startLine) {
	Graphics::Surface icon = _inventoryIcons.getSubArea(Common::Rect(
		iconIndex * ICONDX,
		startLine,
		iconIndex * ICONDX + ICONDX,
		_inventoryIcons.h
	));
	copyToScreenBufferInner(&icon, iconSlot * ICONDX + ICONMARGSX, FIRSTLINE);
}

void GraphicsManager::drawSaveSlotThumbnail(byte iconIndex, byte iconSlot, byte startLine) {
	Graphics::Surface icon = _saveSlotThumbnails.getSubArea(Common::Rect(
		iconIndex * ICONDX,
		startLine,
		iconIndex * ICONDX + ICONDX,
		_saveSlotThumbnails.h
	));
	copyToScreenBufferInner(&icon, iconSlot * ICONDX + ICONMARGSX, FIRSTLINE);
}

void GraphicsManager::clearScreenBuffer() {
	_screenBuffer.fillRect(Common::Rect(0, 0, MAXX, MAXY), 0);
}

void GraphicsManager::clearScreenBufferTop() {
	// Clears lines 0 - 60
	_screenBuffer.fillRect(Common::Rect(0, 0, MAXX, TOP), 0);
}

void GraphicsManager::clearScreenBufferInventory() {
	// Clears lines 420 - 480
	_screenBuffer.fillRect(Common::Rect(0, FIRSTLINE, MAXX, MAXY), 0);
}

void GraphicsManager::clearScreenBufferSaveSlotDescriptions() {
	// Clears lines 470 - 480
	_screenBuffer.fillRect(Common::Rect(0, FIRSTLINE + ICONDY + 10, MAXX, MAXY), 0);
}

void GraphicsManager::updatePixelFormat(uint16 *p, uint32 len) const {
	if (_screenFormat == kImageFormat)
		return;

	uint8 r, g, b;
	for (uint32 a = 0; a < len; ++a) {
		const uint16 t = p[a];
		kImageFormat.colorToRGB(t, r, g, b);
		p[a] = _screenFormat.RGBToColor(r, g, b);
	}
}

/**
 *					Shadow Pixel
 *				(dark) 0..8 (light)
 */
void GraphicsManager::shadow(uint16 x, uint16 y, uint8 num) {
	if (x > MAXX || y > MAXY) {
		warning("shadow: Invalid pixel, skipping");
		return;
	}

	const uint16 val = (uint16)_screenBuffer.getPixel(x, y);
	const uint16 shadow =
			((((val & _bitMask[2]) * num >> 7) & _bitMask[2]) |
			(((val & _bitMask[1]) * num >> 7) & _bitMask[1]) |
			(((val & _bitMask[0]) * num >> 7) & _bitMask[0]));
	_screenBuffer.setPixel(x, y, shadow);
}

void GraphicsManager::pixelAliasing(uint16 x, uint16 y) {
	if (x > MAXX || y > MAXY) {
		warning("pixelAliasing: Invalid pixel, skipping");
		return;
	}

	int px1 = _screenBuffer.getPixel(x - 1, y);
	int px2 = _screenBuffer.getPixel(x, y);

	_screenBuffer.setPixel(x - 1, y, aliasing(px1, px2, 6));      // 75% 25%
	_screenBuffer.setPixel(x, y, aliasing(px1, px2, 2));            // 25% 75%
}

/**
 *					Aliasing Pixel
 */
uint16 GraphicsManager::aliasing(uint32 val1, uint32 val2, uint8 num) {
	// 0:   0% val1 100% val2
	// 1:  12% val1  87% val2
	// 2:  25% val1  75% val2
	// 3:  37% val1  62% val2
	// 4:  50% val1  50% val2
	// 5:  62% val1  37% val2
	// 6:  75% val1  25% val2
	// 7:  87% val1  12% val2
	// 8: 100% val1   0% val2

	return (((((val1 & _bitMask[2]) * num + (val2 & _bitMask[2]) * (8 - num)) >> 3) & _bitMask[2]) |
			((((val1 & _bitMask[1]) * num + (val2 & _bitMask[1]) * (8 - num)) >> 3) & _bitMask[1]) |
			((((val1 & _bitMask[0]) * num + (val2 & _bitMask[0]) * (8 - num)) >> 3) & _bitMask[0]));
}

void GraphicsManager::dissolve() {
	const uint16 val = 30;
	uint16 centerX = MAXX / 2;
	uint16 centerY = MAXY / 2;

	int lastv = 9000;

	uint32 sv = _vm->readTime();
	uint32 cv = _vm->readTime();

	while (sv + val > cv) {
		_vm->checkSystem();
		if (lastv + cv < sv + val)
			continue;

		lastv = (sv - cv) + val;

		const float a = (float)(((centerX + 200) / val) * lastv);
		const float b = (float)((centerY / val) * lastv);

		float x = 0.0f;
		float y = b;

		if (centerY - (int)y > TOP)
			memset(_screenBuffer.getBasePtr(0, TOP), 0, (centerY - (int)y - TOP) * MAXX * 2);
		if (AREA + TOP > centerY + (int)y)
			memset(_screenBuffer.getBasePtr(0, centerY + (int)y), 0, (AREA + TOP - (centerY + (int)y)) * MAXX * 2);

		float d1 = b * b - a * a * b + a * a / 4.0f;
		while (_vm->floatComp(a * a * (y - 0.5f), b * b * (x + 1.0f)) == 1) {
			if (_vm->floatComp(d1, 0.0f) == -1)
				d1 += b * b * (2.0f * x + 3.0f);
			else {
				d1 += b * b * (2.0f * x + 3.0f) + a * a * (-2.0f * y + 2.0f);
				y -= 1.0f;
			}
			x += 1.0f;

			int rightX = centerX + (int)x;
			int maxY = centerY + (int)y;
			int minY = centerY - (int)y;
			if (rightX < MAXX) {
				if (maxY < MAXY)
					memset(_screenBuffer.getBasePtr(rightX, maxY), 0, (MAXX - rightX) * 2);
				if (minY >= 0)
					memset(_screenBuffer.getBasePtr(rightX, minY), 0, (MAXX - rightX) * 2);
			}
			int leftX = centerX - (int)x;
			if (leftX > 0) {
				if (maxY < MAXY)
					memset(_screenBuffer.getBasePtr(0, maxY), 0, leftX * 2);
				if (minY >= 0)
					memset(_screenBuffer.getBasePtr(0, minY), 0, leftX * 2);
			}
		}

		float d2 = b * b * (x + 0.5f) * (x + 0.5f) + a * a * (y - 1.0f) * (y - 1.0f) - a * a * b * b;
		while (_vm->floatComp(y, 0.0f) == 1) {
			if (_vm->floatComp(d2, 0.0f) == -1) {
				d2 += b * b * (2.0f * x + 2.0f) + a * a * (-2.0f * y + 3.0f);
				x += 1.0f;
			} else
				d2 += a * a * (-2.0f * y + 3.0f);
			y -= 1.0f;

			int rightX = centerX + (int)x;
			int maxY = centerY + (int)y;
			int minY = centerY - (int)y;
			if (rightX < MAXX) {
				if (maxY < MAXY)
					memset(_screenBuffer.getBasePtr(rightX, maxY), 0, (MAXX - rightX) * 2);
				if (minY >= 0)
					memset(_screenBuffer.getBasePtr(rightX, minY), 0, (MAXX - rightX) * 2);
			}
			int leftX = centerX - (int)x;
			if (leftX > 0) {
				if (maxY < MAXY)
					memset(_screenBuffer.getBasePtr(0, maxY), 0, leftX * 2);
				if (minY >= 0)
					memset(_screenBuffer.getBasePtr(0, minY), 0, leftX * 2);
			}
		}

		copyToScreen(0, 0, MAXX, MAXY);
		cv = _vm->readTime();
	}

	clearScreen();
}

void GraphicsManager::paintScreen(bool flag) {
	_vm->_animTypeMgr->next();

	_actorRect = nullptr;
	_dirtyRects.clear();
	_vm->_flagPaintCharacter = true; // always redraws the character

	int x1 = _vm->_actor->_lim[0];
	int y1 = _vm->_actor->_lim[2] - TOP;
	int x2 = _vm->_actor->_lim[1];
	int y2 = _vm->_actor->_lim[3] - TOP;

	// erase character
	if (_vm->_flagShowCharacter && x2 > x1 && y2 > y1) { // if a description exists
		_drawRect = Common::Rect(0, TOP, MAXX, AREA + TOP);
		_drawObjRect = Common::Rect(x1, y1, x2, y2);
		_drawObjIndex = -1;
		_drawMask = false;
		drawObj();

		addDirtyRect(_drawObjRect, true, true);
	} else if (_vm->_animMgr->_animRect.left != MAXX) {
		_drawRect = Common::Rect(0, TOP, MAXX, AREA + TOP);
		_drawObjRect = _vm->_animMgr->_animRect;
		_drawObjIndex = -1;
		_drawMask = false;
		drawObj();

		addDirtyRect(_drawObjRect, true, true);
	}

	// If there's text to remove
	if (_vm->_textStatus & TEXT_DEL) {
		// remove text
		_drawRect = Common::Rect(0, TOP, MAXX, MAXY + TOP);
		_drawObjRect = _vm->_textMgr->getOldTextRect();
		_drawObjRect.translate(0, -TOP);
		_drawObjIndex = -1;
		_drawMask = false;

		if (_drawObjRect.top >= 0 && _drawObjRect.bottom < AREA) {
			drawObj();
		} else {
			eraseObj();
		}
		_vm->_textMgr->clearOldText();
		addDirtyRect(_drawObjRect, true);

		if (!(_vm->_textStatus & TEXT_DRAW)) // if there's no new text
			_vm->_textStatus = TEXT_OFF;     // stop updating text
	}

	// Suppress all the objects you removed
	for (Common::List<SSortTable>::iterator i = _vm->_sortTable.begin(); i != _vm->_sortTable.end(); ++i) {
		if (i->_remove) {
			_drawRect = Common::Rect(0, TOP, MAXX, AREA + TOP);
			_drawObjRect = _vm->_obj[i->_objectId]._rect;
			_drawObjIndex = -1;
			_drawMask = false;
			drawObj();
			addDirtyRect(_drawObjRect, true);
		}
	}

	// Find the position of the character
	_vm->_pathFind->actorOrder();

	// For every box from the horizon forward...
	// Copy per level
	for (int liv = _vm->_pathFind->_numSortPanel; liv >= 0; --liv) {
		uint16 curBox = _vm->_pathFind->_sortPan[liv]._num;

		// draws all objects and animations that intersect the boundaries and refer to the current box
		paintObjAnm(curBox);
	}

	if (_vm->_textStatus & TEXT_DRAW) {
		_vm->_textMgr->drawCurString();
		_vm->_textStatus = TEXT_DRAW; // Activate text update
	}

	_vm->_actor->updateStepSound();
	
	if (!flag && !_vm->_flagDialogActive) {
		copyToScreen(0, 0, MAXX, MAXY);
	}

	_vm->_sortTable.clear();

	_vm->_flagPaintCharacter = false;
	_vm->_flagWaitRegen = false;

	// Handle papaverine delayed action
	if (_vm->_curRoom == kRoom4A && _vm->_obj[oCHOCOLATES4A].isFlagExtra()) {
		if (_vm->_animMgr->smkCurFrame(kSmackerBackground) > 480) {
			_vm->playScript(s4AHELLEN);
			_vm->_obj[oCHOCOLATES4A].setFlagExtra(false);
		}
	}
	//
}

/**
 *    Draw all objects and animations that intersect
 *        boundaries belonging to curbox
 */
void GraphicsManager::paintObjAnm(uint16 curBox) {
	_vm->_animMgr->refreshAnim(curBox);

	// draws new cards belonging to the current box
	for (Common::List<SSortTable>::iterator i = _vm->_sortTable.begin(); i != _vm->_sortTable.end(); ++i) {
		if (!i->_remove && _vm->_obj[i->_objectId]._nbox == curBox) {
			// the bitmap object at the desired level
			SObject obj = _vm->_obj[i->_objectId];
			_drawRect = obj._rect;
			_drawRect.translate(0, TOP);
			_drawObjRect = Common::Rect(_drawRect.width(), _drawRect.height());
			_drawObjIndex = _vm->getRoomObjectIndex(i->_objectId);
			_drawMask = obj.isModeMask();
			drawObj();
			_dirtyRects.push_back(_drawRect);
		}
	}

	for (DirtyRectsIterator d = _dirtyRects.begin(); d != _dirtyRects.end(); ++d) {
		for (int b = 0; b < MAXOBJINROOM; ++b) {
			const uint16 curObject = _vm->_room[_vm->_curRoom]._object[b];
			if (!curObject)
				break;

			SObject obj = _vm->_obj[curObject];

			if ((obj.isModeFull() || obj.isModeMask()) && _vm->isObjectVisible(curObject) && (obj._nbox == curBox)) {
				Common::Rect r = *d;
				Common::Rect r2 = obj._rect;

				r2.translate(0, TOP);

				// Include the bottom right of the rect in the intersects() check
				++r2.bottom;
				++r2.right;

				if (r.intersects(r2)) {
					_drawRect = obj._rect;
					_drawRect.translate(0, TOP);

					// Restore the bottom right of the rect
					--r2.bottom;
					--r2.right;

					// TODO: Simplify this?
					const int16 xr1 = (r2.left > r.left) ? 0 : r.left - r2.left;
					const int16 yr1 = (r2.top > r.top) ? 0 : r.top - r2.top;
					const int16 xr2 = MIN<int16>(r.right, r2.right) - r2.left;
					const int16 yr2 = MIN<int16>(r.bottom, r2.bottom) - r2.top;
					_drawObjRect = Common::Rect(xr1, yr1, xr2, yr2);
					_drawObjIndex = b;
					_drawMask = obj.isModeMask();
					drawObj();
				}
			}
		}
	}

	if (_vm->_actorPos == curBox && _vm->_flagShowCharacter) {
		_vm->_renderer->drawCharacter(CALCPOINTS);

		int x1 = _vm->_actor->_lim[0];
		int y1 = _vm->_actor->_lim[2];
		int x2 = _vm->_actor->_lim[1];
		int y2 = _vm->_actor->_lim[3];

		if (x2 > x1 && y2 > y1) {
			// enlarge the rectangle of the character
			Common::Rect l(x1, y1, x2, y2);
			if (_actorRect)
				_actorRect->extend(l);

			_vm->_renderer->resetZBuffer(x1, y1, x2, y2);
		}

		_vm->_renderer->drawCharacter(DRAWFACES);

	} else if (_vm->_actorPos == curBox && !_vm->_flagDialogActive) {
		_vm->_animMgr->refreshActionAnimation();
	}
}

uint16 GraphicsManager::getCharWidth(byte character) {
	return _font[character * 3 + 2];
}

void GraphicsManager::drawChar(byte curChar, uint16 shadowCol, uint16 textCol, uint16 line, Common::Rect rect, Common::Rect subtitleRect, uint16 inc, Graphics::Surface *externalSurface) {
	const uint16 charOffset = _font[curChar * 3] + (uint16)(_font[curChar * 3 + 1] << 8);
	uint16 fontDataOffset = 768;
	const uint16 charWidth = getCharWidth(curChar);

	for (uint16 y = line * CARHEI; y < (line + 1) * CARHEI; ++y) {
		uint16 curPos = 0;
		uint16 curColor = shadowCol;

		while (curPos <= charWidth - 1) {
			if (y >= subtitleRect.top && y < subtitleRect.bottom) {
				if (curColor != MASKCOL && (_font[charOffset + fontDataOffset])) {
					const uint16 charLeft = inc + curPos;
					const uint16 charRight = charLeft + _font[charOffset + fontDataOffset];
					drawCharPixel(
						y,
						charLeft,
						charRight,
						rect,
						subtitleRect,
						curColor,
						externalSurface
					);
				}
			}

			curPos += _font[charOffset + fontDataOffset];
			++fontDataOffset;

			if (curColor == shadowCol)
				curColor = 0;
			else if (curColor == 0)
				curColor = textCol;
			else if (curColor == textCol)
				curColor = shadowCol;
		}
	}
}

void GraphicsManager::drawCharPixel(uint16 y, uint16 charLeft, uint16 charRight, Common::Rect rect, Common::Rect subtitleRect, uint16 color, Graphics::Surface *externalSurface) {
	Graphics::Surface *surface = externalSurface ? externalSurface : &_screenBuffer;
	uint16 *dst1 = (uint16 *)surface->getBasePtr(rect.left + charLeft, rect.top + y);
	uint16 *dst2 = (uint16 *)surface->getBasePtr(rect.left + subtitleRect.left, rect.top + y);
	uint16 *dst = nullptr;
	uint16 size = 0;

	if (charLeft >= subtitleRect.left && charRight < subtitleRect.right) {
		dst = dst1;
		size = charRight - charLeft;
	} else if (charLeft < subtitleRect.left && charRight < subtitleRect.right && charRight > subtitleRect.left) {
		dst = dst2;
		size = charRight - subtitleRect.left;
	} else if (charLeft >= subtitleRect.left && charRight >= subtitleRect.right && subtitleRect.right > charLeft) {
		dst = dst1;
		size = subtitleRect.right - charLeft;
	} else if (charLeft < subtitleRect.left && charRight >= subtitleRect.right && subtitleRect.right > charLeft) {
		dst = dst2;
		size = subtitleRect.right - subtitleRect.left;
	}

	if (dst && size > 0) {
		uint16 *d = dst;
		for (uint32 i = 0; i < size; ++i)
			*d++ = color;
	}
}

void GraphicsManager::initCursor() {
	const int cw = 21, ch = 21;
	const int cx = 10, cy = 10;
	uint16 cursor[cw * ch];
	memset(cursor, 0, ARRAYSIZE(cursor) * 2);

	const uint16 cursorColor = (uint16)_screenFormat.RGBToColor(255, 255, 255);

	for (int i = 0; i < cw; ++i) {
		if (i >= 8 && i <= 12 && i != 10)
			continue;
		cursor[cx * cw + i] = cursorColor; // horizontal
		cursor[cx + cw * i] = cursorColor; // vertical
	}

	CursorMan.pushCursor(cursor, cw, ch, cx, cy, 0, false, &_screenFormat);
}

void GraphicsManager::showCursor() {
	CursorMan.showMouse(true);
}

void GraphicsManager::hideCursor() {
	CursorMan.showMouse(false);
}

bool GraphicsManager::isCursorVisible() {
	return CursorMan.isVisible();
}

void GraphicsManager::showDemoPic() {
	Common::File file;
	if (file.open("EndPic.bm")) {
		readSurface(&file, &_screenBuffer, MAXX, MAXY);
		copyToScreen(0, 0, MAXX, MAXY);
		g_system->updateScreen();

		_vm->freeKey();
		_vm->_mouseLeftBtn = _vm->_mouseRightBtn = false;
		_vm->waitKey();
	}
}

} // End of namespace Trecision
