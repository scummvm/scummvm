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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/cursorman.h"
#include "graphics/maccursor.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/graphics/drivers/gfxdriver.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/view.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/maciconbar.h"

namespace Sci {

GfxCursor::GfxCursor(ResourceManager *resMan, GfxPalette *palette, GfxScreen *screen, GfxCoordAdjuster16 *coordAdjuster, EventManager *eventMan)
	: _resMan(resMan), _palette(palette), _screen(screen), _coordAdjuster(coordAdjuster), _event(eventMan) {

	_upscaledHires = _screen->getUpscaledHires();
	_isVisible = true;

	// center mouse cursor
	setPosition(Common::Point(_screen->getScriptWidth() / 2, _screen->getScriptHeight() / 2));
	_moveZoneActive = false;

	_zoomZoneActive = false;
	_zoomZone = Common::Rect();
	_zoomCursorView = nullptr;
	_zoomCursorLoop = 0;
	_zoomCursorCel = 0;
	_zoomPicView = nullptr;
	_zoomColor = 0;
	_zoomMultiplier = 0;

	if (g_sci && g_sci->getGameId() == GID_KQ6 && g_sci->getPlatform() == Common::kPlatformWindows)
		_useOriginalKQ6WinCursors = ConfMan.getBool("windows_cursors");
	else
		_useOriginalKQ6WinCursors = false;

	if (g_sci && g_sci->getGameId() == GID_SQ4 && g_sci->getPlatform() == Common::kPlatformWindows)
		_useOriginalSQ4WinCursors = ConfMan.getBool("windows_cursors");
	else
		_useOriginalSQ4WinCursors = false;

	if (g_sci && g_sci->getGameId() == GID_SQ4 && getSciVersion() == SCI_VERSION_1_1)
		_useSilverSQ4CDCursors = ConfMan.getBool("silver_cursors");
	else
		_useSilverSQ4CDCursors = false;
}

GfxCursor::~GfxCursor() {
	purgeCache();
	kernelClearZoomZone();
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
	if (resourceId == -1) {
		// no resourceId given, so we actually hide the cursor
		kernelHide();
		return;
	}

	// Load cursor resource...
	Resource *resource = _resMan->findResource(ResourceId(kResourceTypeCursor, resourceId), false);
	if (!resource)
		error("cursor resource %d not found", resourceId);
	if (resource->size() != SCI_CURSOR_SCI0_RESOURCESIZE)
		error("cursor resource %d has invalid size", resourceId);

	Common::Point hotspot;
	bool isSci0Cursor = (getSciVersion() <= SCI_VERSION_01);
	if (isSci0Cursor) {
		// SCI0 cursors contain hotspot flags, not actual hotspot coordinates.
		// If bit 0 of resourceData[3] is set, the hotspot should be centered,
		// otherwise it's in the top left of the mouse cursor.
		hotspot.x = hotspot.y = resource->getUint8At(3) ? SCI_CURSOR_SCI0_HEIGHTWIDTH / 2 : 0;
	} else {
		// Cursors in newer SCI versions contain actual hotspot coordinates.
		hotspot.x = resource->getUint16LEAt(0);
		hotspot.y = resource->getUint16LEAt(2);
	}

	// SCI0 cursors are black and white and transparent.
	// SCI1 cursors introduce gray when both masks are set.
	// These colors are hard-coded in the video driver.
	byte colorMapping[4];
	colorMapping[0] = 0; // black
	colorMapping[1] = _screen->getColorWhite();
	colorMapping[2] = SCI_CURSOR_SCI0_TRANSPARENCYCOLOR;
	if (isSci0Cursor) {
		colorMapping[3] = _screen->getColorWhite();
	} else {
		colorMapping[3] = SCI_CURSOR_SCI1_GRAY;
	}

	Common::SpanOwner<SciSpan<byte> > rawBitmap;
	rawBitmap->allocate(SCI_CURSOR_SCI0_HEIGHTWIDTH * SCI_CURSOR_SCI0_HEIGHTWIDTH, resource->name() + " copy");

	byte *pOut = rawBitmap->getUnsafeDataAt(0, SCI_CURSOR_SCI0_HEIGHTWIDTH * SCI_CURSOR_SCI0_HEIGHTWIDTH);
	for (int16 y = 0; y < SCI_CURSOR_SCI0_HEIGHTWIDTH; y++) {
		uint16 maskA = resource->getUint16LEAt(4 + (y << 1));
		uint16 maskB = resource->getUint16LEAt(4 + 32 + (y << 1));

		for (int16 x = 0; x < SCI_CURSOR_SCI0_HEIGHTWIDTH; x++) {
			byte color = (((maskA << x) & 0x8000) | (((maskB << x) >> 1) & 0x4000)) >> 14;
			*pOut++ = colorMapping[color];
		}
	}

	int16 heightWidth = SCI_CURSOR_SCI0_HEIGHTWIDTH;

	if (_upscaledHires != GFX_SCREEN_UPSCALED_DISABLED && _upscaledHires != GFX_SCREEN_UPSCALED_480x300) {
		// Scale cursor by 2x - note: sierra didn't do this, but it looks much better
		heightWidth *= 2;
		hotspot.x *= 2;
		hotspot.y *= 2;

		Common::SpanOwner<SciSpan<byte> > upscaledBitmap;
		upscaledBitmap->allocate(heightWidth * heightWidth, "upscaled cursor bitmap");
		_screen->scale2x(*rawBitmap, *upscaledBitmap, SCI_CURSOR_SCI0_HEIGHTWIDTH, SCI_CURSOR_SCI0_HEIGHTWIDTH);
		rawBitmap.moveFrom(upscaledBitmap);
	}

	if (hotspot.x >= heightWidth || hotspot.y >= heightWidth) {
		error("cursor %d's hotspot (%d, %d) is out of range of the cursor's dimensions (%dx%d)",
				resourceId, hotspot.x, hotspot.y, heightWidth, heightWidth);
	}

	_screen->gfxDriver()->replaceCursor(rawBitmap->getUnsafeDataAt(0, heightWidth * heightWidth), heightWidth, heightWidth, hotspot.x, hotspot.y, SCI_CURSOR_SCI0_TRANSPARENCYCOLOR);

	kernelShow();
}

void GfxCursor::kernelSetView(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot) {
	if (_cachedCursors.size() >= MAX_CACHED_CURSORS)
		purgeCache();

	// Use the original Windows cursors in KQ6, if requested
	if (_useOriginalKQ6WinCursors)
		viewNum += 2000;		// Windows cursors

	// Use the alternate silver cursors in SQ4 CD, if requested
	if (_useSilverSQ4CDCursors) {
		switch(viewNum) {
		case 850:
		case 852:
		case 854:
		case 856:
			celNum = 3;
			break;
		case 851:
		case 853:
		case 855:
		case 999:
			celNum = 2;
			break;
		default:
			break;
		}
	} else if (_useOriginalSQ4WinCursors) {
		// Use the Windows black and white cursors
		celNum += 1;
	}

	if (!_cachedCursors.contains(viewNum))
		_cachedCursors[viewNum] = new GfxView(_resMan, _screen, _palette, viewNum);

	GfxView *cursorView = _cachedCursors[viewNum];

	const CelInfo *celInfo = cursorView->getCelInfo(loopNum, celNum);
	int16 width = celInfo->width;
	int16 height = celInfo->height;
	byte clearKey = celInfo->clearKey;
	Common::Point *cursorHotspot = hotspot;

	if (!cursorHotspot)
		// Compute hotspot from xoffset/yoffset
		cursorHotspot = new Common::Point((celInfo->width >> 1) - celInfo->displaceX, celInfo->height - celInfo->displaceY - 1);

	// Eco Quest 1 uses a 1x1 transparent cursor to hide the cursor from the
	// user. Some scalers don't seem to support this
	if (width < 2 || height < 2) {
		kernelHide();
		delete cursorHotspot;
		return;
	}

	const SciSpan<const byte> &rawBitmap = cursorView->getBitmap(loopNum, celNum);
	if (_upscaledHires != GFX_SCREEN_UPSCALED_DISABLED && _upscaledHires != GFX_SCREEN_UPSCALED_480x300 && !_useOriginalKQ6WinCursors) {
		// Scale cursor by 2x - note: sierra didn't do this, but it looks much better
		width *= 2;
		height *= 2;
		cursorHotspot->x *= 2;
		cursorHotspot->y *= 2;
		Common::SpanOwner<SciSpan<byte> > cursorBitmap;
		cursorBitmap->allocate(width * height, "upscaled cursor bitmap");
		_screen->scale2x(rawBitmap, *cursorBitmap, celInfo->width, celInfo->height);
		_screen->gfxDriver()->replaceCursor(cursorBitmap->getUnsafeDataAt(0, width * height), width, height, cursorHotspot->x, cursorHotspot->y, clearKey);
	} else {
		_screen->gfxDriver()->replaceCursor(rawBitmap.getUnsafeDataAt(0, width * height), width, height, cursorHotspot->x, cursorHotspot->y, clearKey);
	}

	kernelShow();

	delete cursorHotspot;
}

// This list contains all mandatory set cursor changes, that need special handling
// Refer to GfxCursor::setPosition() below
//    Game,            newPosition,  validRect
static const SciCursorSetPositionWorkarounds setPositionWorkarounds[] = {
	{ GID_ISLANDBRAIN,  84, 109,     46,  76, 174, 243 },  // Island of Dr. Brain, game menu
	{ GID_ISLANDBRAIN, 143, 135,     57, 102, 163, 218 },  // Island of Dr. Brain, pause menu within copy protection
	{ GID_LSL5,         23, 171,      0,   0,  26, 320 },  // Larry 5, skip forward helper pop-up
	{ GID_QFG1VGA,      64, 174,     40,  37,  74, 284 },  // Quest For Glory 1 VGA, run/walk/sleep sub-menu
	{ GID_QFG3,         70, 170,     40,  61,  81, 258 },  // Quest For Glory 3, run/walk/sleep sub-menu
	{ (SciGameId)0,     -1,  -1,     -1,  -1,  -1,  -1 }
};

void GfxCursor::setPosition(Common::Point pos) {
	// Don't set position, when cursor is not visible.
	// This fixes eco quest 1 (floppy) right at the start, which is setting
	// mouse cursor to (0,0) all the time during the intro. It's escapeable
	// (now) by moving to the left or top, but it's getting on your nerves. This
	// could theoretically break some things, although sierra normally sets
	// position only when showing the cursor.
	if (!_isVisible)
		return;

	if (!_upscaledHires) {
		_screen->gfxDriver()->setMousePos(pos);
	} else {
		_screen->adjustToUpscaledCoordinates(pos.y, pos.x);
		g_system->warpMouse(pos.x, pos.y);
	}
	// WORKAROUNDS for games with windows that are hidden when the mouse cursor
	// is moved outside them - also check setPositionWorkarounds above.
	//
	// Some games display a new menu, set mouse position somewhere within and
	// expect it to be in there. This is fine for a real mouse, but on platforms
	// without a mouse, such as a Wii with a Wii Remote, or touch interfaces,
	// this won't work. In these platforms, the affected menus will close
	// immediately, because the mouse cursor's position won't be what the game
	// scripts expect.
	// We identify these cases via the cursor position set. If the mouse position
	// is outside the expected rectangle, we report back to the game scripts that
	// it's actually inside it, the first time that the mouse position is polled,
	// as the scripts expect. In subsequent mouse position poll attempts, we
	// return back the actual mouse coordinates.
	// Currently this code is enabled for all platforms, as we can't differentiate
	// between ones that have normal mouse input, and platforms that have
	// alternative mouse input methods, like a touch screen. Platforms that have
	// a normal mouse for input won't be affected by this workaround.
	const SciGameId gameId = g_sci->getGameId();
	const SciCursorSetPositionWorkarounds *workaround;
	workaround = setPositionWorkarounds;
	while (workaround->newPositionX != -1) {
		if (workaround->gameId == gameId
			&& ((workaround->newPositionX == pos.x) && (workaround->newPositionY == pos.y))) {
			EngineState *s = g_sci->getEngineState();
			s->_cursorWorkaroundActive = true;
			// At least on OpenPandora it seems that the cursor is actually set, but a bit afterwards
			// touch screen controls will overwrite the position. More information see kGetEvent in kevent.cpp.
			s->_cursorWorkaroundPosCount = 5; // should be enough for OpenPandora
			s->_cursorWorkaroundPoint = pos;
			s->_cursorWorkaroundRect = Common::Rect(workaround->rectLeft, workaround->rectTop, workaround->rectRight, workaround->rectBottom);
			return;
		}
		workaround++;
	}
}

Common::Point GfxCursor::getPosition() {
	Common::Point mousePos = g_sci->_gfxScreen->gfxDriver()->getMousePos();

	if (_upscaledHires)
		_screen->adjustBackUpscaledCoordinates(mousePos.y, mousePos.x);

	return mousePos;
}

void GfxCursor::refreshPosition() {
	Common::Point mousePoint = getPosition();

	if (_moveZoneActive) {
		bool clipped = false;

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

	if (_zoomZoneActive) {
		// Cursor
		const CelInfo *cursorCelInfo = _zoomCursorView->getCelInfo(_zoomCursorLoop, _zoomCursorCel);
		const SciSpan<const byte> &cursorBitmap = _zoomCursorView->getBitmap(_zoomCursorLoop, _zoomCursorCel);
		// Pic
		const CelInfo *picCelInfo = _zoomPicView->getCelInfo(0, 0);
		const SciSpan<const byte> &rawPicBitmap = _zoomPicView->getBitmap(0, 0);

		// Compute hotspot of cursor
		Common::Point cursorHotspot = Common::Point((cursorCelInfo->width >> 1) - cursorCelInfo->displaceX, cursorCelInfo->height - cursorCelInfo->displaceY - 1);

		int16 targetX = ((mousePoint.x - _moveZone.left) * _zoomMultiplier);
		int16 targetY = ((mousePoint.y - _moveZone.top) * _zoomMultiplier);
		if (targetX < 0)
			targetX = 0;
		if (targetY < 0)
			targetY = 0;

		targetX -= cursorHotspot.x;
		targetY -= cursorHotspot.y;

		// Sierra SCI actually drew only within zoom area, thus removing the need to fill any other pixels with upmost/left
		//  color of the picture cel. This also made the cursor not appear on top of everything. They actually drew the
		//  cursor manually within kAnimate processing and used a hidden cursor for moving.
		//  TODO: we should also do this

		// Replace the special magnifier color with the associated magnified pixels
		for (int x = 0; x < cursorCelInfo->width; x++) {
			for (int y = 0; y < cursorCelInfo->height; y++) {
				int curPos = cursorCelInfo->width * y + x;
				if (cursorBitmap[curPos] == _zoomColor) {
					int16 rawY = targetY + y;
					int16 rawX = targetX + x;
					if ((rawY >= 0) && (rawY < picCelInfo->height) && (rawX >= 0) && (rawX < picCelInfo->width)) {
						int rawPos = picCelInfo->width * rawY + rawX;
						_cursorSurface[curPos] = rawPicBitmap[rawPos];
					} else {
						_cursorSurface[curPos] = rawPicBitmap[0]; // use left and upmost pixel color
					}
				}
			}
		}

		_screen->gfxDriver()->replaceCursor(_cursorSurface->getUnsafeDataAt(0, cursorCelInfo->width * cursorCelInfo->height), cursorCelInfo->width, cursorCelInfo->height, cursorHotspot.x, cursorHotspot.y, cursorCelInfo->clearKey);
		if (g_system->getScreenFormat().bytesPerPixel != 1) {
			byte buf[3*256];
			_screen->grabPalette(buf, 0, 256);
			CursorMan.replaceCursorPalette(buf, 0, 256);
		}
	}
}

void GfxCursor::kernelResetMoveZone() {
	_moveZoneActive = false;
}

void GfxCursor::kernelSetMoveZone(Common::Rect zone) {
	_moveZone = zone;
	_moveZoneActive = true;
}

void GfxCursor::kernelClearZoomZone() {
	kernelResetMoveZone();
	_zoomZone = Common::Rect();
	_zoomColor = 0;
	_zoomMultiplier = 0;
	_zoomZoneActive = false;
	delete _zoomCursorView;
	_zoomCursorView = nullptr;
	delete _zoomPicView;
	_zoomPicView = nullptr;
	_cursorSurface.clear();
}

void GfxCursor::kernelSetZoomZone(byte multiplier, Common::Rect zone, GuiResourceId viewNum, int loopNum, int celNum, GuiResourceId picNum, byte zoomColor) {
	kernelClearZoomZone();

	// This function is a stub in the Mac version of Freddy Pharkas.
	// This function was only used in two games (LB2 and Pharkas), but there
	// was no version of LB2 for the Macintosh platform.
	// CHECKME: This wasn't verified against disassembly, one might want
	// to check against it, in case there's some leftover code in the stubbed
	// function (although it does seem that this was completely removed).
	if (g_sci->getPlatform() == Common::kPlatformMacintosh)
		return;

	_zoomMultiplier = multiplier;

	if (_zoomMultiplier != 1 && _zoomMultiplier != 2 && _zoomMultiplier != 4)
		error("Unexpected zoom multiplier (expected 1, 2 or 4)");

	_zoomCursorView = new GfxView(_resMan, _screen, _palette, viewNum);
	_zoomCursorLoop = (byte)loopNum;
	_zoomCursorCel = (byte)celNum;
	_zoomPicView = new GfxView(_resMan, _screen, _palette, picNum);
	_cursorSurface->allocateFromSpan(_zoomCursorView->getBitmap(_zoomCursorLoop, _zoomCursorCel));

	_zoomZone = zone;
	kernelSetMoveZone(_zoomZone);

	_zoomColor = zoomColor;
	_zoomZoneActive = true;
}

void GfxCursor::kernelSetPos(Common::Point pos) {
	_coordAdjuster->setCursorPos(pos);
	kernelMoveCursor(pos);
}

void GfxCursor::kernelMoveCursor(Common::Point pos) {
	_coordAdjuster->moveCursor(pos);
	if (pos.x > _screen->getScriptWidth() || pos.y > _screen->getScriptHeight()) {
		warning("attempt to place cursor at invalid coordinates (%d, %d)", pos.y, pos.x);
		return;
	}

	setPosition(pos);

	// Trigger event reading to make sure the mouse coordinates will
	// actually have changed the next time we read them.
	_event->getSciEvent(kSciEventPeek);
}

void GfxCursor::kernelSetMacCursor(GuiResourceId viewNum, int loopNum, int celNum) {
	// Here we try to map the view number onto the cursor. What they did was keep the
	// kSetCursor calls the same, but perform remapping on the cursors. They also took
	// it a step further and added a new kPlatform sub-subop that handles remapping
	// automatically. The view resources may exist, but none of the games actually
	// use them.

	// QFG1/Freddy/Hoyle4 use a straight viewNum->cursor ID mapping
	// KQ6 uses this mapping for its cursors
	if (g_sci->getGameId() == GID_KQ6) {
		if (viewNum == 990)      // Inventory Cursors
			viewNum = loopNum * 16 + celNum + 2000;
		else if (viewNum == 998) // Regular Cursors
			viewNum = celNum + 1000;
		else                     // Unknown cursor, ignored
			return;
	}
	if (g_sci->hasMacIconBar())
		g_sci->_gfxMacIconBar->setInventoryIcon(viewNum);

	Resource *resource = _resMan->findResource(ResourceId(kResourceTypeCursor, viewNum), false);

	if (!resource) {
		// The cursor resources often don't exist, this is normal behavior
		debug(0, "Mac cursor %d not found", viewNum);
		return;
	}

	CursorMan.disableCursorPalette(false);

	Common::MemoryReadStream resStream(resource->toStream());
	Graphics::MacCursor *macCursor = new Graphics::MacCursor();

	// use black for mac monochrome inverted pixels so that cursor
	//  features in FPFP and KQ6 Mac are displayed, bug #7050
	byte macMonochromeInvertedPixelColor = 0;
	if (!macCursor->readFromStream(resStream, false, macMonochromeInvertedPixelColor)) {
		warning("Failed to load Mac cursor %d", viewNum);
		delete macCursor;
		return;
	}

	if (_upscaledHires == GFX_SCREEN_UPSCALED_640x400) {
		// Scale cursor by 2x
		uint16 width = macCursor->getWidth() * 2;
		uint16 height = macCursor->getHeight() * 2;
		uint16 cursorHotspotX = macCursor->getHotspotX() * 2;
		uint16 cursorHotspotY = macCursor->getHotspotY() * 2;
		Common::SpanOwner<SciSpan<byte> > cursorBitmap;
		cursorBitmap->allocate(width * height, "upscaled cursor bitmap");
		SciSpan<const byte> sourceBitmap(macCursor->getSurface(), macCursor->getWidth() * macCursor->getHeight());
		_screen->scale2x(sourceBitmap, *cursorBitmap, macCursor->getWidth(), macCursor->getHeight());
		CursorMan.replaceCursor(cursorBitmap->getUnsafeDataAt(0, width * height), width, height, cursorHotspotX, cursorHotspotY, macCursor->getKeyColor());

		// CursorMan.replaceCursor() does this when called with just a Graphics::Cursor
		if (macCursor->getPalette()) {
			CursorMan.replaceCursorPalette(macCursor->getPalette(), macCursor->getPaletteStartIndex(), macCursor->getPaletteCount());
		}
	} else {
		CursorMan.replaceCursor(macCursor);
	}

	delete macCursor;
	kernelShow();
}

} // End of namespace Sci
