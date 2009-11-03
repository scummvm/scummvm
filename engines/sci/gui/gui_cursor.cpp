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

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_view.h"
#include "sci/gui/gui_cursor.h"

namespace Sci {

SciGuiCursor::SciGuiCursor(ResourceManager *resMan, SciGuiPalette *palette, SciGuiScreen *screen)
	: _resMan(resMan), _palette(palette), _screen(screen) {

	_upscaledHires = _screen->getUpscaledHires();
	// center mouse cursor
	setPosition(Common::Point(_screen->_displayWidth / 2, _screen->_displayHeight / 2));
	setMoveZone(Common::Rect(0, 0, _screen->_displayWidth, _screen->_displayHeight));
}

SciGuiCursor::~SciGuiCursor() {
	purgeCache();
}

void SciGuiCursor::show() {
	CursorMan.showMouse(true);
}

void SciGuiCursor::hide() {
	CursorMan.showMouse(false);
}

void SciGuiCursor::purgeCache() {
	for (CursorCache::iterator iter = _cachedCursors.begin(); iter != _cachedCursors.end(); ++iter) {
		delete iter->_value;
		iter->_value = 0;
	}

	_cachedCursors.clear();
}

void SciGuiCursor::setShape(GuiResourceId resourceId) {
	Resource *resource;
	byte *resourceData;
	Common::Point hotspot = Common::Point(0, 0);
	byte colorMapping[4];
	int16 x, y;
	byte color;
	int16 maskA, maskB;
	byte *pOut;
	byte *rawBitmap = new byte[SCI_CURSOR_SCI0_HEIGHTWIDTH * SCI_CURSOR_SCI0_HEIGHTWIDTH];

	if (resourceId == -1) {
		// no resourceId given, so we actually hide the cursor
		hide();
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
	colorMapping[1] = _screen->_colorWhite; // White is also hardcoded
	colorMapping[2] = SCI_CURSOR_SCI0_TRANSPARENCYCOLOR;
	colorMapping[3] = _palette->matchColor(&_palette->_sysPalette, 170, 170, 170); // Grey
	
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

	CursorMan.replaceCursor(rawBitmap, SCI_CURSOR_SCI0_HEIGHTWIDTH, SCI_CURSOR_SCI0_HEIGHTWIDTH, hotspot.x, hotspot.y, SCI_CURSOR_SCI0_TRANSPARENCYCOLOR);
	CursorMan.showMouse(true);

	delete[] rawBitmap;
}

void SciGuiCursor::setView(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot) {
	if (_cachedCursors.size() >= MAX_CACHED_CURSORS)
		purgeCache();

	if (!_cachedCursors.contains(viewNum))
		_cachedCursors[viewNum] = new SciGuiView(_resMan, _screen, _palette, viewNum);

	SciGuiView *cursorView = _cachedCursors[viewNum];

	sciViewCelInfo *celInfo = cursorView->getCelInfo(loopNum, celNum);
	int16 width = celInfo->width;
	int16 height = celInfo->height;
	byte clearKey = celInfo->clearKey;
	Common::Point *cursorHotspot = hotspot;
	if (!cursorHotspot)
		// Compute hotspot from xoffset/yoffset
		cursorHotspot = new Common::Point((celInfo->width >> 1) - celInfo->displaceX, celInfo->height - celInfo->displaceY - 1);

	// Eco Quest 1 uses a 1x1 transparent cursor to hide the cursor from the user. Some scalers don't seem to support this
	if (width < 2 || height < 2) {
		hide();
		delete cursorHotspot;
		return;
	}

	byte *cursorBitmap = cursorView->getBitmap(loopNum, celNum);

	if (_upscaledHires) {
		// Scale cursor by 2x
		width *= 2;
		height *= 2;
		cursorHotspot->x *= 2;
		cursorHotspot->y *= 2;
		cursorBitmap = new byte[width * height];
		_screen->scale2x(celInfo->rawBitmap, cursorBitmap, celInfo->width, celInfo->height);
	}

	CursorMan.replaceCursor(cursorBitmap, width, height, cursorHotspot->x, cursorHotspot->y, clearKey);

	if (_upscaledHires)
		delete[] cursorBitmap;

	show();

	delete cursorHotspot;
}

void SciGuiCursor::setPosition(Common::Point pos) {
	if (!_upscaledHires) {
		g_system->warpMouse(pos.x, pos.y);
	} else {
		g_system->warpMouse(pos.x * 2, pos.y * 2);
	}
}

Common::Point SciGuiCursor::getPosition() {
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	
	if (_upscaledHires) {
		mousePos.x /= 2;
		mousePos.y /= 2;
	}

	return mousePos;
}

void SciGuiCursor::refreshPosition() {
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

} // End of namespace Sci
