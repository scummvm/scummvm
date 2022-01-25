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

#include "common/file.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "trecision/actor.h"
#include "trecision/animmanager.h"
#include "trecision/animtype.h"
#include "trecision/defines.h"
#include "trecision/graphics.h"
#include "trecision/pathfinding3d.h"
#include "trecision/renderer3d.h"
#include "trecision/text.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {

GraphicsManager::GraphicsManager(TrecisionEngine *vm) : _vm(vm), _rgb555Format(2, 5, 5, 5, 0, 10, 5, 0, 0) {	
	for (int i = 0; i < 3; ++i)
		_bitMask[i] = 0;

	for (int i = 0; i < 256; ++i) {
		_fonts[i]._width = 0;
		_fonts[i]._data = nullptr;
	}
}

GraphicsManager::~GraphicsManager() {
	_screenBuffer.free();
	_background.free();
	_smkBackground.free();
	_leftInventoryArrow.free();
	_rightInventoryArrow.free();
	_inventoryIcons.free();
	_saveSlotThumbnails.free();
	_textureMat.free();

	for (int i = 0; i < 256; ++i)
		delete[] _fonts[i]._data;
}

bool GraphicsManager::init() {
	// Find a suitable 16-bit format, currently we don't support other color depths
	Common::List<Graphics::PixelFormat> formats = g_system->getSupportedFormats();
	for (Common::List<Graphics::PixelFormat>::iterator it = formats.begin(); it != formats.end(); ++it) {
		if (it->bytesPerPixel != 2 || it->aBits()) {
			it = formats.reverse_erase(it);
		} else if (*it == _rgb555Format) {
			formats.clear();
			formats.push_back(_rgb555Format);
			break;
		}
	}

	if (formats.empty())
		return false;

	initGraphics(MAXX, MAXY, formats);

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

void GraphicsManager::addDirtyRect(Common::Rect rect, bool translateRect) {
	if (translateRect)
		rect.translate(0, TOP);

	_dirtyRects.push_back(rect);
}

void GraphicsManager::drawObj(int index, bool mask, Common::Rect drawRect, Common::Rect drawObjRect, bool includeDirtyRect) {
	if (drawObjRect.left > MAXX || drawObjRect.top > MAXY)
		return;

	// If we have a valid object, draw it, otherwise erase it
	// by using the background buffer
	const uint16 *buf = index >= 0 ? _vm->_objectGraphics[index].buf : (uint16 *)_smkBackground.getPixels();
	if (mask && index >= 0) {
		uint8 *maskPtr = _vm->_objectGraphics[index].mask;

		for (uint16 y = drawRect.top; y < drawRect.bottom; ++y) {
			uint16 x = 0;
			bool copyBytes = false;
			while (x < drawRect.width()) {
				if (!copyBytes) { // jump
					x += *maskPtr;
					++maskPtr;

					copyBytes = true;
				} else { // copy
					const uint16 maskOffset = *maskPtr;

					if (maskOffset != 0 && y >= drawRect.top + drawObjRect.top && y < drawRect.top + drawObjRect.bottom) {
						const void *src = (x >= drawObjRect.left) ? buf : buf + drawObjRect.left - x;
						int offset = (x >= drawObjRect.left) ? x : drawObjRect.left;
						void *dst = _screenBuffer.getBasePtr(offset + drawRect.left, y);

						if (x >= drawObjRect.left && x + maskOffset < drawObjRect.right)
							memcpy(dst, src, maskOffset * 2);
						else if (x < drawObjRect.left && x + maskOffset < drawObjRect.right && x + maskOffset >= drawObjRect.left)
							memcpy(dst, src, (maskOffset + x - drawObjRect.left) * 2);
						else if (x >= drawObjRect.left && x + maskOffset >= drawObjRect.right && x < drawObjRect.right)
							memcpy(dst, src, (drawObjRect.right - x) * 2);
						else if (x < drawObjRect.left && x + maskOffset >= drawObjRect.right)
							memcpy(dst, src, (drawObjRect.right - drawObjRect.left) * 2);
					}
					x += *maskPtr;
					buf += *maskPtr++;
					copyBytes = false;
				}
			}
		}
	} else {
		const uint16 x = drawRect.left + drawObjRect.left;

		if (x + drawObjRect.width() > MAXX || drawObjRect.top + drawObjRect.height() > MAXY) {
			warning("drawObj: Invalid surface, skipping");
			return;
		}
		
		for (uint16 y = drawObjRect.top; y < drawObjRect.bottom; ++y) {
			memcpy(_screenBuffer.getBasePtr(x, drawRect.top + y),
				   buf + (y * drawRect.width()) + drawObjRect.left, drawObjRect.width() * 2);
		}
	}

	if (includeDirtyRect)
		addDirtyRect(drawObjRect, true);
}

void GraphicsManager::eraseObj(Common::Rect drawObjRect) {
	Common::Rect eraseRect = drawObjRect;
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

void GraphicsManager::readSurface(Common::SeekableReadStream *stream, Graphics::Surface *surface, uint16 width, uint16 height, uint16 count) {
	surface->create(width * count, height, _rgb555Format);

	for (uint16 i = 0; i < count; ++i) {
		for (uint16 y = 0; y < height; ++y) {
			for (uint16 x = 0; x < width; ++x) {
				surface->setPixel(width * i + x, y, stream->readUint16LE());
			}
		}
	}

	surface->convertToInPlace(_screenFormat);
}

void GraphicsManager::readTexture(Common::SeekableReadStream *stream) {
	readSurface(stream, &_textureMat, 91, 256);
}

void GraphicsManager::drawTexturePixel(uint16 textureX, uint16 textureY, uint16 screenX, uint16 screenY) {
	const uint16 texturePixel = (uint16)_textureMat.getPixel(textureX, textureY);
	_screenBuffer.setPixel(screenX, screenY, texturePixel);
}

void GraphicsManager::loadBackground(Common::SeekableReadStream *stream) {
	SObject bgInfo;
	bgInfo.readRect(stream);

	readSurface(stream, &_background, bgInfo._rect.width(), bgInfo._rect.height());
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

	loadFont();
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

uint16 GraphicsManager::convertToScreenFormat(uint16 color) const {
	uint8 r, g, b;
	_rgb555Format.colorToRGB(color, r, g, b);
	return (uint16)_screenFormat.RGBToColor(r, g, b);
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
	const uint16 shadowVal =
			((((val & _bitMask[2]) * num >> 7) & _bitMask[2]) |
			(((val & _bitMask[1]) * num >> 7) & _bitMask[1]) |
			(((val & _bitMask[0]) * num >> 7) & _bitMask[0]));
	_screenBuffer.setPixel(x, y, shadowVal);
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
		if (lastv + cv < sv + val) {
			cv = _vm->readTime();
			continue;
		}

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

	_dirtyRects.clear();
	_vm->_flagPaintCharacter = true; // always redraws the character

	// erase character
	if (_vm->_flagShowCharacter && _vm->_actor->actorRectIsValid()) { // if a description exists
		Common::Rect actorRect = _vm->_actor->getActorRect();
		actorRect.translate(0, -TOP);
		drawObj(-1, false, Common::Rect(0, TOP, MAXX, AREA + TOP), actorRect);
	} else if (_vm->_animMgr->_animRect.left != MAXX) {
		drawObj(-1, false, Common::Rect(0, TOP, MAXX, AREA + TOP), _vm->_animMgr->_animRect);
	}

	// If there's text to remove
	if (_vm->_textStatus & TEXT_DEL) {
		// remove text
		Common::Rect drawObjRect = _vm->_textMgr->getOldTextRect();
		drawObjRect.translate(0, -TOP);
		
		if (drawObjRect.top >= 0 && drawObjRect.bottom < AREA) {
			drawObj(-1, false, Common::Rect(0, TOP, MAXX, MAXY + TOP), drawObjRect);
		} else {
			eraseObj(drawObjRect);
		}
		_vm->_textMgr->clearOldText();

		if (!(_vm->_textStatus & TEXT_DRAW)) // if there's no new text
			_vm->_textStatus = TEXT_OFF;     // stop updating text
	}

	// Suppress all the objects you removed
	for (Common::List<SSortTable>::iterator it = _vm->_sortTable.begin(); it != _vm->_sortTable.end(); ++it) {
		if (it->_remove) {
			drawObj(-1, false, Common::Rect(0, TOP, MAXX, AREA + TOP), _vm->_obj[it->_objectId]._rect);
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
	for (Common::List<SSortTable>::iterator it = _vm->_sortTable.begin(); it != _vm->_sortTable.end(); ++it) {
		if (!it->_remove && _vm->_obj[it->_objectId]._nbox == curBox) {
			// the bitmap object at the desired level
			SObject obj = _vm->_obj[it->_objectId];
			Common::Rect drawRect = obj._rect;
			drawRect.translate(0, TOP);
			drawObj(_vm->getRoomObjectIndex(it->_objectId), obj.isModeMask(), drawRect, Common::Rect(drawRect.width(), drawRect.height()), false);
			_dirtyRects.push_back(drawRect);
		}
	}

	for (DirtyRectsIterator it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
		for (int i = 0; i < MAXOBJINROOM; ++i) {
			const uint16 curObject = _vm->_room[_vm->_curRoom]._object[i];
			if (!curObject)
				break;

			SObject obj = _vm->_obj[curObject];

			if ((obj.isModeFull() || obj.isModeMask()) && _vm->isObjectVisible(curObject) && (obj._nbox == curBox)) {
				Common::Rect r = *it;
				Common::Rect r2 = obj._rect;

				r2.translate(0, TOP);

				// Include the bottom right of the rect in the intersects() check
				++r2.bottom;
				++r2.right;

				if (r.intersects(r2)) {
					Common::Rect drawRect = obj._rect;
					drawRect.translate(0, TOP);

					// Restore the bottom right of the rect
					--r2.bottom;
					--r2.right;

					// TODO: Simplify this?
					const int16 xr1 = (r2.left > r.left) ? 0 : r.left - r2.left;
					const int16 yr1 = (r2.top > r.top) ? 0 : r.top - r2.top;
					const int16 xr2 = MIN<int16>(r.right, r2.right) - r2.left;
					const int16 yr2 = MIN<int16>(r.bottom, r2.bottom) - r2.top;
					drawObj(i, obj.isModeMask(), drawRect, Common::Rect(xr1, yr1, xr2, yr2), false);
				}
			}
		}
	}

	if (_vm->_pathFind->getActorPos() == curBox && _vm->_flagShowCharacter) {
		_vm->_renderer->drawCharacter(CALCPOINTS);

		if (_vm->_actor->actorRectIsValid()) {
			const Common::Rect actorRect = _vm->_actor->getActorRect();
			// enlarge the last dirty rectangle with the actor's rectangle
			if (!_dirtyRects.empty())
				_dirtyRects.back().extend(actorRect);

			_vm->_renderer->resetZBuffer(actorRect);
		}

		_vm->_renderer->drawCharacter(DRAWFACES);

	} else if (_vm->_pathFind->getActorPos() == curBox && !_vm->_flagDialogActive) {
		_vm->_animMgr->refreshActionAnimation();
	}
}

uint16 GraphicsManager::getCharWidth(byte character) {
	return _fonts[character]._width;
}

void GraphicsManager::drawChar(byte curChar, uint16 textColor, uint16 line, Common::Rect rect, Common::Rect subtitleRect, uint16 inc, Graphics::Surface *externalSurface) {
	uint16 fontDataOffset = 0;
	const uint16 charWidth = getCharWidth(curChar);

	for (uint16 y = line * CARHEI; y < (line + 1) * CARHEI; ++y) {
		uint16 curPos = 0;
		uint16 curColor = MASKCOL;

		while (curPos <= charWidth - 1) {
			if (y >= subtitleRect.top && y < subtitleRect.bottom) {
				if (curColor != MASKCOL && _fonts[curChar]._data[fontDataOffset]) {
					const uint16 charLeft = inc + curPos;
					const uint16 charRight = charLeft + _fonts[curChar]._data[fontDataOffset];
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

			curPos += _fonts[curChar]._data[fontDataOffset];
			++fontDataOffset;

			if (curColor == MASKCOL)
				curColor = 0;
			else if (curColor == 0)
				curColor = textColor;
			else if (curColor == textColor)
				curColor = MASKCOL;
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

void GraphicsManager::loadFont() {
	Common::String fileName = "nlfont.fnt";
	Common::SeekableReadStream *fontStream = _vm->_dataFile.createReadStreamForMember(fileName);
	if (fontStream == nullptr)
		error("readData(): File %s not found", fileName.c_str());

	uint16 fontDataOffset = 768;

	for (int i = 0; i < 256; ++i) {
		uint16 offset = fontStream->readSint16LE();
		_fonts[i]._width = fontStream->readByte();

		int tmpPos = fontStream->pos();
		fontStream->seek(offset + fontDataOffset);

		int cpt = 0;
		for (uint16 y = 0; y < CARHEI; ++y) {
			uint16 curPos = 0;
			while (curPos <= _fonts[i]._width - 1) {
				curPos += fontStream->readByte();
				++cpt;
			}
		}

		fontStream->seek(offset + fontDataOffset);
		_fonts[i]._data = new int8[cpt];
		fontStream->read(_fonts[i]._data, cpt);
		fontStream->seek(tmpPos);
	}

	// Fix o+e ligature character (lowercase and uppercase). Ticket #12623

	// Format is :
	// - Each line represents a line of pixels
	// - colors are in this order : none, shadow, text. Colors are looping until the total number of pixels corresponds to the character width
	// - each number correspond to a number of pixels of the corresponding color
	// So, 1, 6, 0, 2 means : 1 pixel unchanged, 6 pixels shadow, 0 pixel in text color, 2 pixels unchanged
	static const int8 fix140[67] = {
		1, 8,
		0, 2, 2, 0, 1, 3, 0, 1,
		0, 1, 1, 0, 2, 2, 0, 3,
		0, 1, 1, 0, 3, 1, 0, 2, 0, 1,
		0, 1, 1, 0, 3, 2, 0, 1, 0, 1,
		0, 1, 1, 0, 3, 1, 0, 2, 0, 1,
		0, 1, 1, 0, 2, 2, 0, 3,
		0, 2, 2, 0, 1, 3, 0, 1,
		1, 8,
		9
	};

	static const int8 fix156[54] = {
		9,
		9,
		1, 6, 0, 2,
		0, 2, 2, 0, 1, 2, 0, 1, 0, 1,
		0, 1, 1, 0, 2, 1, 0, 2, 1, 0, 1,
		0, 1, 1, 0, 2, 4, 0, 1,
		0, 1, 1, 0, 2, 1, 0, 4,
		0, 2, 2, 0, 1, 3, 0, 1,
		1, 8,
		9
	};

	delete _fonts[140]._data;
	delete _fonts[156]._data;
	_fonts[140]._width = _fonts[156]._width = 9;
	_fonts[140]._data = new int8[67];
	_fonts[156]._data = new int8[54];

	memcpy(_fonts[140]._data, fix140, 67);
	memcpy(_fonts[156]._data, fix156, 54);
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
