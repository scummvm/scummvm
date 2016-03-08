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

#include "common/system.h"

#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "gui/message.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/controls16.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/view.h"
#ifdef ENABLE_SCI32
#include "sci/graphics/celobj32.h"
#include "sci/graphics/controls32.h"
#include "sci/graphics/font.h"	// TODO: remove once kBitmap is moved in a separate class
#include "sci/graphics/frameout.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/text32.h"
#endif

namespace Sci {
#ifdef ENABLE_SCI32

extern void showScummVMDialog(const Common::String &message);

reg_t kIsHiRes(EngineState *s, int argc, reg_t *argv) {
	// Returns 0 if the screen width or height is less than 640 or 400,
	// respectively.
	if (g_system->getWidth() < 640 || g_system->getHeight() < 400)
		return make_reg(0, 0);

	return make_reg(0, 1);
}

// SCI32 variant, can't work like sci16 variants
reg_t kCantBeHere32(EngineState *s, int argc, reg_t *argv) {
	// TODO
//	reg_t curObject = argv[0];
//	reg_t listReference = (argc > 1) ? argv[1] : NULL_REG;

	return NULL_REG;
}

reg_t kAddScreenItem(EngineState *s, int argc, reg_t *argv) {
	debugC(6, kDebugLevelGraphics, "kAddScreenItem %x:%x (%s)", argv[0].getSegment(), argv[0].getOffset(), g_sci->getEngineState()->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelAddScreenItem(argv[0]);
	return NULL_REG;
}

reg_t kUpdateScreenItem(EngineState *s, int argc, reg_t *argv) {
	debugC(7, kDebugLevelGraphics, "kUpdateScreenItem %x:%x (%s)", argv[0].getSegment(), argv[0].getOffset(), g_sci->getEngineState()->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelUpdateScreenItem(argv[0]);
	return NULL_REG;
}

reg_t kDeleteScreenItem(EngineState *s, int argc, reg_t *argv) {
	debugC(6, kDebugLevelGraphics, "kDeleteScreenItem %x:%x (%s)", argv[0].getSegment(), argv[0].getOffset(), g_sci->getEngineState()->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelDeleteScreenItem(argv[0]);
	return NULL_REG;
}

reg_t kAddPlane(EngineState *s, int argc, reg_t *argv) {
	debugC(6, kDebugLevelGraphics, "kAddPlane %x:%x (%s)", argv[0].getSegment(), argv[0].getOffset(), g_sci->getEngineState()->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelAddPlane(argv[0]);
	return s->r_acc;
}

reg_t kUpdatePlane(EngineState *s, int argc, reg_t *argv) {
	debugC(7, kDebugLevelGraphics, "kUpdatePlane %x:%x (%s)", argv[0].getSegment(), argv[0].getOffset(), g_sci->getEngineState()->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelUpdatePlane(argv[0]);
	return s->r_acc;
}

reg_t kDeletePlane(EngineState *s, int argc, reg_t *argv) {
	debugC(6, kDebugLevelGraphics, "kDeletePlane %x:%x (%s)", argv[0].getSegment(), argv[0].getOffset(), g_sci->getEngineState()->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelDeletePlane(argv[0]);
	return s->r_acc;
}

reg_t kAddPicAt(EngineState *s, int argc, reg_t *argv) {
	reg_t planeObj = argv[0];
	GuiResourceId pictureId = argv[1].toUint16();
	int16 x = argv[2].toSint16();
	int16 y = argv[3].toSint16();
	bool mirrorX = argc > 4 ? argv[4].toSint16() : false;

	g_sci->_gfxFrameout->kernelAddPicAt(planeObj, pictureId, x, y, mirrorX);
	return NULL_REG;
}

reg_t kGetHighPlanePri(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_gfxFrameout->kernelGetHighPlanePri());
}

reg_t kFrameOut(EngineState *s, int argc, reg_t *argv) {
	bool showBits = argc > 0 ? argv[0].toUint16() : true;
	g_sci->_gfxFrameout->kernelFrameOut(showBits);
	s->speedThrottler(16);
	s->_throttleTrigger = true;
	return NULL_REG;
}

reg_t kSetPalStyleRange(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxFrameout->kernelSetPalStyleRange(argv[0].toUint16(), argv[1].toUint16());
	return NULL_REG;
}

reg_t kObjectIntersect(EngineState *s, int argc, reg_t *argv) {
	Common::Rect objRect1 = g_sci->_gfxCompare->getNSRect(argv[0]);
	Common::Rect objRect2 = g_sci->_gfxCompare->getNSRect(argv[1]);
	return make_reg(0, objRect1.intersects(objRect2));
}

// Tests if the coordinate is on the passed object
reg_t kIsOnMe(EngineState *s, int argc, reg_t *argv) {
	int16 x = argv[0].toSint16();
	int16 y = argv[1].toSint16();
	reg_t object = argv[2];
	bool checkPixel = argv[3].toSint16();

	return g_sci->_gfxFrameout->kernelIsOnMe(object, Common::Point(x, y), checkPixel);
}

reg_t kCreateTextBitmap(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;

	int16 subop = argv[0].toUint16();

	int16 width = 0;
	int16 height = 0;
	reg_t object;

	if (subop == 0) {
		width = argv[1].toUint16();
		height = argv[2].toUint16();
		object = argv[3];
	} else if (subop == 1) {
		object = argv[1];
	} else {
		warning("Invalid kCreateTextBitmap subop %d", subop);
		return NULL_REG;
	}

	Common::String text = segMan->getString(readSelector(segMan, object, SELECTOR(text)));
	int16 foreColor = readSelectorValue(segMan, object, SELECTOR(fore));
	int16 backColor = readSelectorValue(segMan, object, SELECTOR(back));
	int16 skipColor = readSelectorValue(segMan, object, SELECTOR(skip));
	GuiResourceId fontId = (GuiResourceId)readSelectorValue(segMan, object, SELECTOR(font));
	int16 borderColor = readSelectorValue(segMan, object, SELECTOR(borderColor));
	int16 dimmed = readSelectorValue(segMan, object, SELECTOR(dimmed));

	Common::Rect rect(
		readSelectorValue(segMan, object, SELECTOR(textLeft)),
		readSelectorValue(segMan, object, SELECTOR(textTop)),
		readSelectorValue(segMan, object, SELECTOR(textRight)) + 1,
		readSelectorValue(segMan, object, SELECTOR(textBottom)) + 1
	);

	if (subop == 0) {
		TextAlign alignment = (TextAlign)readSelectorValue(segMan, object, SELECTOR(mode));
		return g_sci->_gfxText32->createFontBitmap(width, height, rect, text, foreColor, backColor, skipColor, fontId, alignment, borderColor, dimmed, true);
	} else {
		CelInfo32 celInfo;
		celInfo.type = kCelTypeView;
		celInfo.resourceId = readSelectorValue(segMan, object, SELECTOR(view));
		celInfo.loopNo = readSelectorValue(segMan, object, SELECTOR(loop));
		celInfo.celNo = readSelectorValue(segMan, object, SELECTOR(cel));
		return g_sci->_gfxText32->createFontBitmap(celInfo, rect, text, foreColor, backColor, fontId, skipColor, borderColor, dimmed);
	}
}

reg_t kText(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kTextSize32(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxText32->setFont(argv[2].toUint16());

	reg_t *rect = s->_segMan->derefRegPtr(argv[0], 4);

	Common::String text = s->_segMan->getString(argv[1]);
	int16 maxWidth = argc > 3 ? argv[3].toSint16() : 0;
	bool doScaling = argc > 4 ? argv[4].toSint16() : true;

	Common::Rect textRect = g_sci->_gfxText32->getTextSize(text, maxWidth, doScaling);
	rect[0] = make_reg(0, textRect.left);
	rect[1] = make_reg(0, textRect.top);
	rect[2] = make_reg(0, textRect.right - 1);
	rect[3] = make_reg(0, textRect.bottom - 1);
	return NULL_REG;
}

reg_t kTextWidth(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxText32->setFont(argv[1].toUint16());
	Common::String text = s->_segMan->getString(argv[0]);
	return make_reg(0, g_sci->_gfxText32->getStringWidth(text));
}

reg_t kWinHelp(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 1:
		// Load a help file
		// Maybe in the future we can implement this, but for now this message should suffice
		showScummVMDialog("Please use an external viewer to open the game's help file: " + s->_segMan->getString(argv[1]));
		break;
	case 2:
		// Looks like some init function
		break;
	default:
		warning("Unknown kWinHelp subop %d", argv[0].toUint16());
	}

	return s->r_acc;
}

/**
 * Causes an immediate plane transition with an optional transition
 * effect
 */
reg_t kSetShowStyle(EngineState *s, int argc, reg_t *argv) {
	ShowStyleType type = (ShowStyleType)argv[0].toUint16();
	reg_t planeObj = argv[1];
	int16 seconds = argv[2].toSint16();
	// NOTE: This value seems to indicate whether the transition is an
	// “exit” transition (0) or an “enter” transition (-1) for fade
	// transitions. For other types of transitions, it indicates a palette
	// index value to use when filling the screen.
	int16 back = argv[3].toSint16();
	int16 priority = argv[4].toSint16();
	int16 animate = argv[5].toSint16();
	// TODO: Rename to frameOutNow?
	int16 refFrame = argv[6].toSint16();
	int16 blackScreen;
	reg_t pFadeArray;
	int16 divisions;

	// SCI 2–2.1early
	if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
		blackScreen = 0;
		pFadeArray = NULL_REG;
		divisions = argc > 7 ? argv[7].toSint16() : -1;
	}
	// SCI 2.1mid–2.1late
	else if (getSciVersion() < SCI_VERSION_3) {
		blackScreen = 0;
		pFadeArray = argc > 7 ? argv[7] : NULL_REG;
		divisions = argc > 8 ? argv[8].toSint16() : -1;
	}
	// SCI 3
	else {
		blackScreen = argv[7].toSint16();
		pFadeArray = argc > 8 ? argv[8] : NULL_REG;
		divisions = argc > 9 ? argv[9].toSint16() : -1;
	}

// TODO: Reuse later for SCI2 and SCI3 implementation and then discard
//	warning("kSetShowStyle: effect %d, plane: %04x:%04x (%s), sec: %d, "
//			"dir: %d, prio: %d, animate: %d, ref frame: %d, black screen: %d, "
//			"pFadeArray: %04x:%04x (%s), divisions: %d",
//			type, PRINT_REG(planeObj), s->_segMan->getObjectName(planeObj), seconds,
//			back, priority, animate, refFrame, blackScreen,
//			PRINT_REG(pFadeArray), s->_segMan->getObjectName(pFadeArray), divisions);

	// NOTE: The order of planeObj and showStyle are reversed
	// because this is how SCI3 called the corresponding method
	// on the KernelMgr
	g_sci->_gfxFrameout->kernelSetShowStyle(argc, planeObj, type, seconds, back, priority, animate, refFrame, pFadeArray, divisions, blackScreen);

	return NULL_REG;
}

reg_t kCelInfo(EngineState *s, int argc, reg_t *argv) {
	// Used by Shivers 1, room 23601 to determine what blocks on the red door puzzle board
	// are occupied by pieces already

	switch (argv[0].toUint16()) {	// subops 0 - 4
		// 0 - return the view
		// 1 - return the loop
		// 2, 3 - nop
		case 4: {
			GuiResourceId viewId = argv[1].toSint16();
			int16 loopNo = argv[2].toSint16();
			int16 celNo = argv[3].toSint16();
			int16 x = argv[4].toUint16();
			int16 y = argv[5].toUint16();
			byte color = g_sci->_gfxCache->kernelViewGetColorAtCoordinate(viewId, loopNo, celNo, x, y);
			return make_reg(0, color);
		}
		default: {
			kStub(s, argc, argv);
			return s->r_acc;
		}
	}
}

reg_t kScrollWindow(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kScrollWindowCreate(EngineState *s, int argc, reg_t *argv) {
	debug("kScrollWindowCreate");
	kStub(s, argc, argv);
	return argv[0];
}

reg_t kScrollWindowAdd(EngineState *s, int argc, reg_t *argv) {
	debug("kScrollWindowAdd");
	return kStubNull(s, argc, argv);
}

reg_t kScrollWindowWhere(EngineState *s, int argc, reg_t *argv) {
	debug("kScrollWindowWhere");
	return kStubNull(s, argc, argv);
}

reg_t kScrollWindowShow(EngineState *s, int argc, reg_t *argv) {
	debug("kScrollWindowShow");
	return kStubNull(s, argc, argv);
}

reg_t kScrollWindowDestroy(EngineState *s, int argc, reg_t *argv) {
	debug("kScrollWindowDestroy");
	return kStubNull(s, argc, argv);
}

#if 0
reg_t kScrollWindow(EngineState *s, int argc, reg_t *argv) {
	// Used by SQ6 and LSL6 hires for the text area in the bottom of the
	// screen. The relevant scripts also exist in Phantasmagoria 1, but they're
	// unused. This is always called by scripts 64906 (ScrollerWindow) and
	// 64907 (ScrollableWindow).

	reg_t kWindow = argv[1];
	uint16 op = argv[0].toUint16();
	switch (op) {
	case 0:	// Init
		// TODO: Init reads the nsLeft, nsTop, nsRight, nsBottom,
		//       borderColor, fore, back, mode, font, plane selectors
		//       from the window in argv[1].
		g_sci->_gfxFrameout->initScrollText(argv[2].toUint16());	// maxItems
		g_sci->_gfxFrameout->clearScrollTexts();
		return argv[1];	// kWindow
	case 1: // Show message, called by ScrollableWindow::addString
	case 14: // Modify message, called by ScrollableWindow::modifyString
		// TODO: The parameters in Modify are shifted by one: the first
		//       argument is the handle of the text to modify. The others
		//       are as Add.
		{
		Common::String text = s->_segMan->getString(argv[2]);
		uint16 x = 0;
		uint16 y = 0;
		// TODO: argv[3] is font
		// TODO: argv[4] is color
		// TODO: argv[5] is alignment (0 = left, 1 = center, 2 = right)
		//       font,color,alignment may also be -1. (Maybe same as previous?)
		// TODO: argv[6] is an optional bool, defaulting to true if not present.
		//       If true, the old contents are scrolled out of view.
		// TODO: Return a handle of the inserted text. (Used for modify/insert)
		//       This handle looks like it should also be usable by kString.
		g_sci->_gfxFrameout->addScrollTextEntry(text, kWindow, x, y, (op == 14));
		}
		break;
	case 2: // Clear, called by ScrollableWindow::erase
		g_sci->_gfxFrameout->clearScrollTexts();
		break;
	case 3: // Page up, called by ScrollableWindow::scrollTo
		// TODO
		kStub(s, argc, argv);
		break;
	case 4: // Page down, called by ScrollableWindow::scrollTo
		// TODO
		kStub(s, argc, argv);
		break;
	case 5: // Up arrow, called by ScrollableWindow::scrollTo
		g_sci->_gfxFrameout->prevScrollText();
		break;
	case 6: // Down arrow, called by ScrollableWindow::scrollTo
		g_sci->_gfxFrameout->nextScrollText();
		break;
	case 7: // Home, called by ScrollableWindow::scrollTo
		g_sci->_gfxFrameout->firstScrollText();
		break;
	case 8: // End, called by ScrollableWindow::scrollTo
		g_sci->_gfxFrameout->lastScrollText();
		break;
	case 9: // Resize, called by ScrollableWindow::resize and ScrollerWindow::resize
		// TODO: This reads the nsLeft, nsTop, nsRight, nsBottom
		//       selectors from the SCI object passed in argv[2].
		kStub(s, argc, argv);
		break;
	case 10: // Where, called by ScrollableWindow::where
		// TODO:
		// Gives the current relative scroll location as a fraction
		// with argv[2] as the denominator. (Return value is the numerator.)
		// Silenced the warnings because of the high amount of console spam
		//kStub(s, argc, argv);
		break;
	case 11: // Go, called by ScrollableWindow::scrollTo
		// TODO:
		// Two arguments provide a fraction: argv[2] is num., argv[3] is denom.
		// Scrolls to the relative location given by the fraction.
		kStub(s, argc, argv);
		break;
	case 12: // Insert, called by ScrollableWindow::insertString
		// 5 extra parameters here:
		// handle of insert location (new string takes that position).
		// text, font, color, alignment
		// TODO
		kStub(s, argc, argv);
		break;
	// case 13 (Delete) is handled below
	// case 14 (Modify) is handled above
	case 15: // Hide, called by ScrollableWindow::hide
		g_sci->_gfxFrameout->toggleScrollText(false);
		break;
	case 16: // Show, called by ScrollableWindow::show
		g_sci->_gfxFrameout->toggleScrollText(true);
		break;
	case 17: // Destroy, called by ScrollableWindow::dispose
		g_sci->_gfxFrameout->clearScrollTexts();
		break;
	case 13: // Delete, unused
	case 18: // Text, unused
	case 19: // Reconstruct, unused
		error("kScrollWindow: Unused subop %d invoked", op);
		break;
	default:
		error("kScrollWindow: unknown subop %d", op);
		break;
	}

	return s->r_acc;
}
#endif

reg_t kFont(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kSetFontHeight(EngineState *s, int argc, reg_t *argv) {
	// TODO: Setting font may have just been for side effect
	// of setting the fontHeight on the font manager, in
	// which case we could just get the font directly ourselves.
	g_sci->_gfxText32->setFont(argv[0].toUint16());
	g_sci->_gfxText32->_scaledHeight = (g_sci->_gfxText32->_font->getHeight() * g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight + g_sci->_gfxText32->_scaledHeight - 1) / g_sci->_gfxText32->_scaledHeight;
	return NULL_REG;
}

reg_t kSetFontRes(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxText32->_scaledWidth = argv[0].toUint16();
	g_sci->_gfxText32->_scaledHeight = argv[1].toUint16();
	return NULL_REG;
}

reg_t kBitmap(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kBitmapCreate(EngineState *s, int argc, reg_t *argv) {
	int16 width = argv[0].toSint16();
	int16 height = argv[1].toSint16();
	int16 skipColor = argv[2].toSint16();
	int16 backColor = argv[3].toSint16();
	int16 scaledWidth = argc > 4 ? argv[4].toSint16() : g_sci->_gfxText32->_scaledWidth;
	int16 scaledHeight = argc > 5 ? argv[5].toSint16() : g_sci->_gfxText32->_scaledHeight;
	bool useRemap = argc > 6 ? argv[6].toSint16() : false;

	BitmapResource bitmap(s->_segMan, width, height, skipColor, 0, 0, scaledWidth, scaledHeight, 0, useRemap);
	memset(bitmap.getPixels(), backColor, width * height);
	return bitmap.getObject();
}

reg_t kBitmapDestroy(EngineState *s, int argc, reg_t *argv) {
	s->_segMan->freeHunkEntry(argv[0]);
	return NULL_REG;
}

reg_t kBitmapDrawLine(EngineState *s, int argc, reg_t *argv) {
	// bitmapMemId, (x1, y1, x2, y2) OR (x2, y2, x1, y1), line color, unknown int, unknown int
	return kStubNull(s, argc + 1, argv - 1);
}

reg_t kBitmapDrawView(EngineState *s, int argc, reg_t *argv) {
	// viewId, loopNo, celNo, displace x, displace y, unused, view x, view y

	// called e.g. from TiledBitmap::resize() in Torin's Passage, script 64869
	// The tiled view seems to always have 2 loops.
	// These loops need to have 1 cel in loop 0 and 8 cels in loop 1.

	return kStubNull(s, argc + 1, argv - 1);

#if 0
	// tiled surface
	// 6 params, called e.g. from TiledBitmap::resize() in Torin's Passage,
	// script 64869
	reg_t hunkId = argv[1];	// obtained from kBitmap(0)
	// The tiled view seems to always have 2 loops.
	// These loops need to have 1 cel in loop 0 and 8 cels in loop 1.
	uint16 viewNum = argv[2].toUint16();	// vTiles selector
	uint16 loop = argv[3].toUint16();
	uint16 cel = argv[4].toUint16();
	uint16 x = argv[5].toUint16();
	uint16 y = argv[6].toUint16();

	byte *memoryPtr = s->_segMan->getHunkPointer(hunkId);
	// Get totalWidth, totalHeight
	uint16 totalWidth = READ_LE_UINT16(memoryPtr);
	uint16 totalHeight = READ_LE_UINT16(memoryPtr + 2);
	byte *bitmap = memoryPtr + BITMAP_HEADER_SIZE;

	GfxView *view = g_sci->_gfxCache->getView(viewNum);
	uint16 tileWidth = view->getWidth(loop, cel);
	uint16 tileHeight = view->getHeight(loop, cel);
	const byte *tileBitmap = view->getBitmap(loop, cel);
	uint16 width = MIN<uint16>(totalWidth - x, tileWidth);
	uint16 height = MIN<uint16>(totalHeight - y, tileHeight);

	for (uint16 curY = 0; curY < height; curY++) {
		for (uint16 curX = 0; curX < width; curX++) {
			bitmap[(curY + y) * totalWidth + (curX + x)] = tileBitmap[curY * tileWidth + curX];
		}
	}
#endif
}

reg_t kBitmapDrawText(EngineState *s, int argc, reg_t *argv) {
	// called e.g. from TextButton::createBitmap() in Torin's Passage, script 64894

	// bitmap, text, textLeft, textTop, textRight, textBottom, foreColor, backColor, skipColor, fontNo, alignment, borderColor, dimmed
	BitmapResource bitmap(argv[0]);
	Common::String text = s->_segMan->getString(argv[1]);
	Common::Rect textRect(
		argv[2].toSint16(),
		argv[3].toSint16(),
		argv[4].toSint16() + 1,
		argv[5].toSint16() + 1
	);
	int16 foreColor = argv[6].toSint16();
	int16 backColor = argv[7].toSint16();
	int16 skipColor = argv[8].toSint16();
	GuiResourceId fontId = (GuiResourceId)argv[9].toUint16();
	TextAlign alignment = (TextAlign)argv[10].toSint16();
	int16 borderColor = argv[11].toSint16();
	bool dimmed = argv[12].toUint16();

	// NOTE: Technically the engine checks these things:
	// textRect.bottom > 0
	// textRect.right > 0
	// textRect.left < bitmap.width
	// textRect.top < bitmap.height
	// Then clips. But this seems stupid.
	textRect.clip(Common::Rect(bitmap.getWidth(), bitmap.getHeight()));

	reg_t textBitmapObject = g_sci->_gfxText32->createFontBitmap(textRect.width(), textRect.height(), Common::Rect(textRect.width(), textRect.height()), text, foreColor, backColor, skipColor, fontId, alignment, borderColor, dimmed, false);
	Buffer bitmapBuffer(bitmap.getWidth(), bitmap.getHeight(), bitmap.getPixels());
	CelObjMem textCel(textBitmapObject);
	textCel.draw(bitmapBuffer, textRect, Common::Point(textRect.left, textRect.top), false);
	s->_segMan->freeHunkEntry(textBitmapObject);

	return NULL_REG;
}

reg_t kBitmapDrawColor(EngineState *s, int argc, reg_t *argv) {
	// bitmap, left, top, right, bottom, color

	// called e.g. from TextView::init() and TextView::draw() in Torin's Passage, script 64890
	return kStubNull(s, argc + 1, argv - 1);
#if 0
	reg_t hunkId = argv[1];	// obtained from kBitmap(0)
	uint16 x = argv[2].toUint16();
	uint16 y = argv[3].toUint16();
	uint16 fillWidth = argv[4].toUint16();	// width - 1
	uint16 fillHeight = argv[5].toUint16();	// height - 1
	uint16 back = argv[6].toUint16();

	byte *memoryPtr = s->_segMan->getHunkPointer(hunkId);
	// Get totalWidth, totalHeight
	uint16 totalWidth = READ_LE_UINT16(memoryPtr);
	uint16 totalHeight = READ_LE_UINT16(memoryPtr + 2);
	uint16 width = MIN<uint16>(totalWidth - x, fillWidth);
	uint16 height = MIN<uint16>(totalHeight - y, fillHeight);
	byte *bitmap = memoryPtr + BITMAP_HEADER_SIZE;

	for (uint16 curY = 0; curY < height; curY++) {
		for (uint16 curX = 0; curX < width; curX++) {
			bitmap[(curY + y) * totalWidth + (curX + x)] = back;
		}
	}
#endif
}

reg_t kBitmapDrawBitmap(EngineState *s, int argc, reg_t *argv) {
	// target bitmap, source bitmap, x, y, unknown boolean

	return kStubNull(s, argc + 1, argv - 1);
}

reg_t kBitmapInvert(EngineState *s, int argc, reg_t *argv) {
	// bitmap, left, top, right, bottom, foreColor, backColor

	return kStubNull(s, argc + 1, argv - 1);
}

reg_t kBitmapSetDisplace(EngineState *s, int argc, reg_t *argv) {
	// bitmap, x, y

	return kStubNull(s, argc + 1, argv - 1);
}

reg_t kBitmapCreateFromView(EngineState *s, int argc, reg_t *argv) {
	// viewId, loopNo, celNo, skipColor, backColor, useRemap, source overlay bitmap

	return kStub(s, argc + 1, argv - 1);
}

reg_t kBitmapCopyPixels(EngineState *s, int argc, reg_t *argv) {
	// target bitmap, source bitmap

	return kStubNull(s, argc + 1, argv - 1);
}

reg_t kBitmapClone(EngineState *s, int argc, reg_t *argv) {
	// bitmap

	return kStub(s, argc + 1, argv - 1);
}

reg_t kBitmapGetInfo(EngineState *s, int argc, reg_t *argv) {
	// bitmap

	// argc 1 = get width
	// argc 2 = pixel at row 0 col n
	// argc 3 = pixel at row n col n
	return kStub(s, argc + 1, argv - 1);
}

reg_t kBitmapScale(EngineState *s, int argc, reg_t *argv) {
	// TODO: SCI3
	return kStubNull(s, argc + 1, argv - 1);
}

reg_t kBitmapCreateFromUnknown(EngineState *s, int argc, reg_t *argv) {
	// TODO: SCI3
	return kStub(s, argc + 1, argv - 1);
}

// Used for edit boxes in save/load dialogs. It's a rewritten version of kEditControl,
// but it handles events on its own, using an internal loop, instead of using SCI
// scripts for event management like kEditControl does. Called by script 64914,
// DEdit::hilite().

reg_t kEditText(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_gfxControls32->kernelEditText(argv[0]);
}

reg_t kAddLine(EngineState *s, int argc, reg_t *argv) {
	return kStubNull(s, argc, argv); // return 0:0 for now, so that follow up calls won't create signature mismatches
#if 0
	reg_t plane = argv[0];
	Common::Point startPoint(argv[1].toUint16(), argv[2].toUint16());
	Common::Point endPoint(argv[3].toUint16(), argv[4].toUint16());
	// argv[5] is unknown (a number, usually 200)
	byte color = (byte)argv[6].toUint16();
	byte priority = (byte)argv[7].toUint16();
	byte control = (byte)argv[8].toUint16();
	// argv[9] is unknown (usually a small number, 1 or 2). Thickness, perhaps?
//	return g_sci->_gfxFrameout->addPlaneLine(plane, startPoint, endPoint, color, priority, control);
	return s->r_acc;
#endif
}

reg_t kUpdateLine(EngineState *s, int argc, reg_t *argv) {
	return kStub(s, argc, argv);

#if 0
	reg_t hunkId = argv[0];
	reg_t plane = argv[1];
	Common::Point startPoint(argv[2].toUint16(), argv[3].toUint16());
	Common::Point endPoint(argv[4].toUint16(), argv[5].toUint16());
	// argv[6] is unknown (a number, usually 200)
	byte color = (byte)argv[7].toUint16();
	byte priority = (byte)argv[8].toUint16();
	byte control = (byte)argv[9].toUint16();
	// argv[10] is unknown (usually a small number, 1 or 2). Thickness, perhaps?
//	g_sci->_gfxFrameout->updatePlaneLine(plane, hunkId, startPoint, endPoint, color, priority, control);
	return s->r_acc;
#endif
}
reg_t kDeleteLine(EngineState *s, int argc, reg_t *argv) {
	return kStub(s, argc, argv);
#if 0
	reg_t hunkId = argv[0];
	reg_t plane = argv[1];
//	g_sci->_gfxFrameout->deletePlaneLine(plane, hunkId);
	return s->r_acc;
#endif
}

reg_t kSetScroll(EngineState *s, int argc, reg_t *argv) {
	// Called in the intro of LSL6 hires (room 110)
	// The end effect of this is the same as the old screen scroll transition

	// 7 parameters
	reg_t planeObject = argv[0];
	//int16 x = argv[1].toSint16();
	//int16 y = argv[2].toSint16();
	uint16 pictureId = argv[3].toUint16();
	// param 4: int (0 in LSL6, probably scroll direction? The picture in LSL6 scrolls down)
	// param 5: int (first call is 1, then the subsequent one is 0 in LSL6)
	// param 6: optional int (0 in LSL6)

	// Set the new picture directly for now
	//writeSelectorValue(s->_segMan, planeObject, SELECTOR(left), x);
	//writeSelectorValue(s->_segMan, planeObject, SELECTOR(top), y);
	writeSelectorValue(s->_segMan, planeObject, SELECTOR(picture), pictureId);
	// and update our draw list
	g_sci->_gfxFrameout->kernelUpdatePlane(planeObject);

	// TODO
	return kStub(s, argc, argv);
}

// Used by SQ6, script 900, the datacorder reprogramming puzzle (from room 270)
reg_t kMorphOn(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxFrameout->_palMorphIsOn = true;
	return NULL_REG;
}

reg_t kPaletteSetFade(EngineState *s, int argc, reg_t *argv) {
	uint16 fromColor = argv[0].toUint16();
	uint16 toColor = argv[1].toUint16();
	uint16 percent = argv[2].toUint16();
	g_sci->_gfxPalette32->setFade(percent, fromColor, toColor);
	return NULL_REG;
}

reg_t kPalVarySetVary(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId paletteId = argv[0].toUint16();
	int time = argc > 1 ? argv[1].toSint16() * 60 : 0;
	int16 percent = argc > 2 ? argv[2].toSint16() : 100;
	int16 fromColor;
	int16 toColor;

	if (argc > 4) {
		fromColor = argv[3].toSint16();
		toColor = argv[4].toSint16();
	} else {
		fromColor = toColor = -1;
	}

	g_sci->_gfxPalette32->kernelPalVarySet(paletteId, percent, time, fromColor, toColor);
	return NULL_REG;
}

reg_t kPalVarySetPercent(EngineState *s, int argc, reg_t *argv) {
	int time = argc > 0 ? argv[0].toSint16() * 60 : 0;
	int16 percent = argc > 1 ? argv[1].toSint16() : 0;

	// TODO: GK1 adds a third optional parameter here, at the end of chapter 1
	// (during the sunset/sunrise sequence, the parameter is 1)

	g_sci->_gfxPalette32->setVaryPercent(percent, time, -1, -1);
	return NULL_REG;
}

reg_t kPalVaryGetPercent(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_gfxPalette32->getVaryPercent());
}

reg_t kPalVaryOff(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxPalette32->varyOff();
	return NULL_REG;
}

reg_t kPalVaryMergeTarget(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId paletteId = argv[0].toUint16();
	g_sci->_gfxPalette32->kernelPalVaryMergeTarget(paletteId);
	return make_reg(0, g_sci->_gfxPalette32->getVaryPercent());
}

reg_t kPalVarySetTime(EngineState *s, int argc, reg_t *argv) {
	int time = argv[0].toSint16() * 60;
	g_sci->_gfxPalette32->setVaryTime(time);
	return NULL_REG;
}

reg_t kPalVarySetTarget(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId paletteId = argv[0].toUint16();
	g_sci->_gfxPalette32->kernelPalVarySetTarget(paletteId);
	return make_reg(0, g_sci->_gfxPalette32->getVaryPercent());
}

reg_t kPalVarySetStart(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId paletteId = argv[0].toUint16();
	g_sci->_gfxPalette32->kernelPalVarySetStart(paletteId);
	return make_reg(0, g_sci->_gfxPalette32->getVaryPercent());
}

reg_t kPalVaryMergeStart(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId paletteId = argv[0].toUint16();
	g_sci->_gfxPalette32->kernelPalVaryMergeStart(paletteId);
	return make_reg(0, g_sci->_gfxPalette32->getVaryPercent());
}

enum {
	kSetCycle = 0,
	kDoCycle = 1,
	kCyclePause = 2,
	kCycleOn = 3,
	kCycleOff = 4
};

reg_t kPalCycle(EngineState *s, int argc, reg_t *argv) {
	// Examples: GK1 room 480 (Bayou ritual), LSL6 room 100 (title screen)

	switch (argv[0].toUint16()) {
	case kSetCycle: {
		uint16 fromColor = argv[1].toUint16();
		uint16 toColor = argv[2].toUint16();
		int16 direction = argv[3].toSint16();
		uint16 delay = (argc == 4 ? 0 : argv[4].toUint16());

		g_sci->_gfxPalette32->setCycle(fromColor, toColor, direction, delay);
		}
		break;
	case kDoCycle: {
		uint16 fromColor = argv[1].toUint16();
		int16 speed = (argc == 2) ? 1 : argv[2].toSint16();
		g_sci->_gfxPalette32->doCycle(fromColor, speed);
		}
		break;
	case kCyclePause: {
		if (argc == 1) {
			g_sci->_gfxPalette32->cycleAllPause();
		} else {
			uint16 fromColor = argv[1].toUint16();
			g_sci->_gfxPalette32->cyclePause(fromColor);
		}
		}
		break;
	case kCycleOn: {
		if (argc == 1) {
			g_sci->_gfxPalette32->cycleAllOn();
		} else {
			uint16 fromColor = argv[1].toUint16();
			g_sci->_gfxPalette32->cycleOn(fromColor);
		}
		}
		break;
	case kCycleOff: {
		if (argc == 1) {
			g_sci->_gfxPalette32->cycleAllOff();
		} else {
			uint16 fromColor = argv[1].toUint16();
			g_sci->_gfxPalette32->cycleOff(fromColor);
		}
		break;
		}
	default:
		// In SCI2.1 there are no values above 4, so should never get here;
		// SCI just returns early if this ever happens.
		assert(false);
		break;
	}

	return s->r_acc;
}

reg_t kRemapColors32(EngineState *s, int argc, reg_t *argv) {
	// TODO
#if 0
	uint16 operation = argv[0].toUint16();

	switch (operation) {
	case 0:	{ // turn remapping off
		// WORKAROUND: Game scripts in QFG4 erroneously turn remapping off in room
		// 140 (the character point allocation screen) and never turn it back on,
		// even if it's clearly used in that screen.
		if (g_sci->getGameId() == GID_QFG4 && s->currentRoomNumber() == 140)
			return s->r_acc;

		int16 color = (argc >= 2) ? argv[1].toSint16() : 0;
		if (color > 0)
			warning("kRemapColors(0) called with base %d", color);
		//g_sci->_gfxPalette32->resetRemapping();
		}
		break;
	case 1:	{ // remap by range
		uint16 color = argv[1].toUint16();
		uint16 from = argv[2].toUint16();
		uint16 to = argv[3].toUint16();
		uint16 delta = argv[4].toUint16();
		uint16 depth = (argc >= 6) ? argv[5].toUint16() : 0;
		if (depth > 0)
			warning("kRemapColors(1) called with 6 parameters, depth is %d", depth);
		//g_sci->_gfxPalette32->setRemappingRange(color, from, to, delta);
		}
		break;
	case 2:	{ // remap by percent
		uint16 color = argv[1].toUint16();
		uint16 percent = argv[2].toUint16(); // 0 - 100
		uint16 depth = (argc >= 4) ? argv[3].toUint16() : 0;
		if (depth >= 0)
			warning("RemapByPercent called with 4 parameters, depth is %d", depth);
		//g_sci->_gfxPalette32->setRemappingPercent(color, percent);
		}
		break;
	case 3:	{ // remap to gray
		// Example call: QFG4 room 490 (Baba Yaga's hut) - params are color 253, 75% and 0.
		// In this room, it's used for the cloud before Baba Yaga appears.
		uint16 color = argv[1].toUint16();
		uint16 percent = argv[2].toUint16(); // 0 - 100
		uint16 depth = (argc >= 4) ? argv[3].toUint16() : 0;
		if (depth >= 0)
			warning("RemapToGray called with 4 parameters, depth is %d", depth);
		//g_sci->_gfxPalette32->setRemappingPercentGray(color, percent);
		}
		break;
	case 4:	{ // remap to percent gray
		// Example call: QFG4 rooms 530/535 (swamp) - params are 253, 100%, 200
		uint16 color = argv[1].toUint16();
		uint16 percent = argv[2].toUint16(); // 0 - 100
		uint16 grayPercent = argv[3].toUint16();
		uint16 depth = (argc >= 5) ? argv[4].toUint16() : 0;
		if (argc >= 5)
			warning("RemapToGrayPercent called with 5 parameters, depth is %d", depth);
		//g_sci->_gfxPalette32->setRemappingPercentGray(color, percent);
		}
		break;
	case 5:	{ // don't map to range
		//uint16 start = argv[1].toSint16();
		//uint16 count = argv[2].toUint16();

		kStub(s, argc, argv);
		}
		break;
	default:
		break;
	}
#endif

	return s->r_acc;
}

#endif

} // End of namespace Sci
