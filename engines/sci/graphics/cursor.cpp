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

#include "graphics/cursorman.h"
#include "common/util.h"
#include "common/events.h"
#include "common/system.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/view.h"
#include "sci/graphics/cursor.h"

namespace Sci {

GfxCursor::GfxCursor(ResourceManager *resMan, GfxPalette *palette, GfxScreen *screen)
	: _resMan(resMan), _palette(palette), _screen(screen) {

	_upscaledHires = _screen->getUpscaledHires();
	_isVisible = true;

	// center mouse cursor
	setPosition(Common::Point(_screen->getWidth() / 2, _screen->getHeight() / 2));
	kernelSetMoveZone(Common::Rect(0, 0, _screen->getDisplayWidth(), _screen->getDisplayHeight()));
}

GfxCursor::~GfxCursor() {
	purgeCache();
}

void GfxCursor::init(GfxCoordAdjuster *coordAdjuster, EventManager *event) {
	_coordAdjuster = coordAdjuster;
	_event = event;
}

void GfxCursor::kernelShow() {
	CursorMan.showMouse(true);
	_isVisible = true;
}

void GfxCursor::kernelHide() {
	CursorMan.showMouse(false);
	_isVisible = false;
}

bool GfxCursor::isVisible() {
	return _isVisible;
}

void GfxCursor::purgeCache() {
	for (CursorCache::iterator iter = _cachedCursors.begin(); iter != _cachedCursors.end(); ++iter) {
		delete iter->_value;
		iter->_value = 0;
	}

	_cachedCursors.clear();
}

void GfxCursor::kernelSetShape(GuiResourceId resourceId) {
	Resource *resource;
	byte *resourceData;
	Common::Point hotspot = Common::Point(0, 0);
	byte colorMapping[4];
	int16 x, y;
	byte color;
	int16 maskA, maskB;
	byte *pOut;
	byte *rawBitmap = new byte[SCI_CURSOR_SCI0_HEIGHTWIDTH * SCI_CURSOR_SCI0_HEIGHTWIDTH];
	int16 heightWidth;

	if (resourceId == -1) {
		// no resourceId given, so we actually hide the cursor
		kernelHide();
		delete[] rawBitmap;
		return;
	}

	// Load cursor resource...
	resource = _resMan->findResource(ResourceId(kResourceTypeCursor, resourceId), false);
	if (!resource)
		error("cursor resource %d not found", resourceId);
	if (resource->size != SCI_CURSOR_SCI0_RESOURCESIZE)
		error("cursor resource %d has invalid size", resourceId);

	resourceData = resource->data;
	// hotspot is specified for SCI1 cursors
	hotspot.x = READ_LE_UINT16(resourceData);
	hotspot.y = READ_LE_UINT16(resourceData + 2);
	// bit 0 of resourceData[3] is set on <SCI1 games, which means center hotspot
	if ((hotspot.x == 0) && (hotspot.y == 256))
		hotspot.x = hotspot.y = SCI_CURSOR_SCI0_HEIGHTWIDTH / 2;

	// Now find out what colors we are supposed to use
	colorMapping[0] = 0; // Black is hardcoded
	colorMapping[1] = _screen->getColorWhite(); // White is also hardcoded
	colorMapping[2] = SCI_CURSOR_SCI0_TRANSPARENCYCOLOR;
	colorMapping[3] = _palette->matchColor(170, 170, 170); // Grey

	// Seek to actual data
	resourceData += 4;

	pOut = rawBitmap;
	for (y = 0; y < SCI_CURSOR_SCI0_HEIGHTWIDTH; y++) {
		maskA = READ_LE_UINT16(resourceData + (y << 1));
		maskB = READ_LE_UINT16(resourceData + 32 + (y << 1));

		for (x = 0; x < SCI_CURSOR_SCI0_HEIGHTWIDTH; x++) {
			color = (((maskA << x) & 0x8000) | (((maskB << x) >> 1) & 0x4000)) >> 14;
			*pOut++ = colorMapping[color];
		}
	}

	heightWidth = SCI_CURSOR_SCI0_HEIGHTWIDTH;

	if (_upscaledHires) {
		// Scale cursor by 2x - note: sierra didn't do this, but it looks much better
		heightWidth *= 2;
		hotspot.x *= 2;
		hotspot.y *= 2;
		byte *upscaledBitmap = new byte[heightWidth * heightWidth];
		_screen->scale2x(rawBitmap, upscaledBitmap, SCI_CURSOR_SCI0_HEIGHTWIDTH, SCI_CURSOR_SCI0_HEIGHTWIDTH);
		delete[] rawBitmap;
		rawBitmap = upscaledBitmap;
	}

	CursorMan.replaceCursor(rawBitmap, heightWidth, heightWidth, hotspot.x, hotspot.y, SCI_CURSOR_SCI0_TRANSPARENCYCOLOR);
	kernelShow();

	delete[] rawBitmap;
}

void GfxCursor::kernelSetView(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot) {
	if (_cachedCursors.size() >= MAX_CACHED_CURSORS)
		purgeCache();

	if (!_cachedCursors.contains(viewNum))
		_cachedCursors[viewNum] = new GfxView(_resMan, _screen, _palette, viewNum);

	GfxView *cursorView = _cachedCursors[viewNum];

	CelInfo *celInfo = cursorView->getCelInfo(loopNum, celNum);
	int16 width = celInfo->width;
	int16 height = celInfo->height;
	byte clearKey = celInfo->clearKey;
	Common::Point *cursorHotspot = hotspot;
	byte *cursorBitmap;

	if (!cursorHotspot)
		// Compute hotspot from xoffset/yoffset
		cursorHotspot = new Common::Point((celInfo->width >> 1) - celInfo->displaceX, celInfo->height - celInfo->displaceY - 1);

	// Eco Quest 1 uses a 1x1 transparent cursor to hide the cursor from the user. Some scalers don't seem to support this
	if (width < 2 || height < 2) {
		kernelHide();
		delete cursorHotspot;
		return;
	}

	celInfo->rawBitmap = cursorView->getBitmap(loopNum, celNum);
	if (_upscaledHires) {
		// Scale cursor by 2x - note: sierra didn't do this, but it looks much better
		width *= 2;
		height *= 2;
		cursorHotspot->x *= 2;
		cursorHotspot->y *= 2;
		cursorBitmap = new byte[width * height];
		_screen->scale2x(celInfo->rawBitmap, cursorBitmap, celInfo->width, celInfo->height);
	} else {
		cursorBitmap = celInfo->rawBitmap;
	}

	CursorMan.replaceCursor(cursorBitmap, width, height, cursorHotspot->x, cursorHotspot->y, clearKey);

	if (_upscaledHires)
		delete[] cursorBitmap;

	kernelShow();

	delete cursorHotspot;
}

void GfxCursor::kernelSetMacCursor(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot) {
	// See http://developer.apple.com/legacy/mac/library/documentation/mac/QuickDraw/QuickDraw-402.html
	// for more information.

	// View 998 seems to be a fake resource used to call for for the Mac CURS resources
	// For other resources, they're still in the views, so use them.
	if (viewNum != 998) {
		kernelSetView(viewNum, loopNum, celNum, hotspot);
		return;
	}

	// TODO: What about the 2000 resources? Inventory items? How to handle?
	// TODO: What games does this work for? At least it does for KQ6.
	// TODO: Stop asking rhetorical questions.
	// TODO: It was fred all along!

	Resource *resource = _resMan->findResource(ResourceId(kResourceTypeCursor, 1000 + celNum), false);

	if (!resource) {
		warning("CURS %d not found", 1000 + celNum);
		return;
	}

	assert(resource);

	byte *cursorBitmap = new byte[16 * 16];
	byte *data = resource->data;

	// Get B&W data
	for (byte i = 0; i < 32; i++) {
		byte imageByte = *data++;
		for (byte b = 0; b < 8; b++)
			cursorBitmap[i * 8 + b] = (byte)((imageByte & (0x80 >> b)) > 0 ? 0x00 : 0xFF);
	}

	// Apply mask data
	for (byte i = 0; i < 32; i++) {
		byte imageByte = *data++;
		for (byte b = 0; b < 8; b++)
			if ((imageByte & (0x80 >> b)) == 0)
				cursorBitmap[i * 8 + b] = SCI_CURSOR_SCI0_TRANSPARENCYCOLOR; // Doesn't matter, just is transparent
	}

	uint16 hotspotX = READ_BE_UINT16(data);
	uint16 hotspotY = READ_BE_UINT16(data + 2);

	CursorMan.replaceCursor(cursorBitmap, 16, 16, hotspotX, hotspotY, SCI_CURSOR_SCI0_TRANSPARENCYCOLOR);

	delete[] cursorBitmap;

	kernelShow();
}

void GfxCursor::setPosition(Common::Point pos) {
	// Don't set position, when cursor is not visible
	//  This fixes eco quest 1 (floppy) right at the start, which is setting mouse cursor to 0, 0 all the time during the
	//  intro. It's escapeable (now) by moving to the left or top, but it's getting on your nerves.
	//  This could theoretically break some things, although sierra normally sets position only when showing the cursor.
	if (!_isVisible)
		return;

	if (!_upscaledHires) {
		g_system->warpMouse(pos.x, pos.y);
	} else {
		_screen->adjustToUpscaledCoordinates(pos.y, pos.x);
		g_system->warpMouse(pos.x, pos.y);
	}
}

Common::Point GfxCursor::getPosition() {
	Common::Point mousePos = g_system->getEventManager()->getMousePos();

	switch (_upscaledHires) {
	case GFX_SCREEN_UPSCALED_640x400:
		mousePos.x /= 2;
		mousePos.y /= 2;
		break;
	case GFX_SCREEN_UPSCALED_640x440:
		mousePos.x /= 2;
		mousePos.y = (mousePos.y * 5) / 11;
		break;
	case GFX_SCREEN_UPSCALED_640x480:
		mousePos.x /= 2;
		mousePos.y = (mousePos.y * 5) / 12;
	default:
		break;
	}

	return mousePos;
}

void GfxCursor::refreshPosition() {
	bool clipped = false;
	Common::Point mousePoint = getPosition();

	if (mousePoint.x < _moveZone.left) {
		mousePoint.x = _moveZone.left;
		clipped = true;
	} else if (mousePoint.x >= _moveZone.right) {
		mousePoint.x = _moveZone.right - 1;
		clipped = true;
	}

	if (mousePoint.y < _moveZone.top) {
		mousePoint.y = _moveZone.top;
		clipped = true;
	} else if (mousePoint.y >= _moveZone.bottom) {
		mousePoint.y = _moveZone.bottom - 1;
		clipped = true;
	}

	// FIXME: Do this only when mouse is grabbed?
	if (clipped)
		setPosition(mousePoint);
}

void GfxCursor::kernelSetMoveZone(Common::Rect zone) {
	 _moveZone = zone;
}

void GfxCursor::kernelSetPos(Common::Point pos) {
	_coordAdjuster->setCursorPos(pos);
	kernelMoveCursor(pos);
}

void GfxCursor::kernelMoveCursor(Common::Point pos) {
	_coordAdjuster->moveCursor(pos);
	if (pos.x > _screen->getWidth() || pos.y > _screen->getHeight()) {
		warning("attempt to place cursor at invalid coordinates (%d, %d)", pos.y, pos.x);
		return;
	}

	setPosition(pos);

	// Trigger event reading to make sure the mouse coordinates will
	// actually have changed the next time we read them.
	_event->getSciEvent(SCI_EVENT_PEEK);
}

} // End of namespace Sci
