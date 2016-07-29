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
#include "sci/graphics/remap.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/view.h"
#ifdef ENABLE_SCI32
#include "sci/graphics/celobj32.h"
#include "sci/graphics/controls32.h"
#include "sci/graphics/font.h"	// TODO: remove once kBitmap is moved in a separate class
#include "sci/graphics/frameout.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap32.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/transitions32.h"
#endif

namespace Sci {
#ifdef ENABLE_SCI32

extern void showScummVMDialog(const Common::String &message);

reg_t kIsHiRes(EngineState *s, int argc, reg_t *argv) {
	const Buffer &buffer = g_sci->_gfxFrameout->getCurrentBuffer();
	if (buffer.screenWidth < 640 || buffer.screenHeight < 400)
		return make_reg(0, 0);

	return make_reg(0, 1);
}

reg_t kAddScreenItem(EngineState *s, int argc, reg_t *argv) {
	debugC(6, kDebugLevelGraphics, "kAddScreenItem %x:%x (%s)", PRINT_REG(argv[0]), s->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelAddScreenItem(argv[0]);
	return s->r_acc;
}

reg_t kUpdateScreenItem(EngineState *s, int argc, reg_t *argv) {
	debugC(7, kDebugLevelGraphics, "kUpdateScreenItem %x:%x (%s)", PRINT_REG(argv[0]), s->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelUpdateScreenItem(argv[0]);
	return s->r_acc;
}

reg_t kDeleteScreenItem(EngineState *s, int argc, reg_t *argv) {
	debugC(6, kDebugLevelGraphics, "kDeleteScreenItem %x:%x (%s)", PRINT_REG(argv[0]), s->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelDeleteScreenItem(argv[0]);
	return s->r_acc;
}

reg_t kAddPlane(EngineState *s, int argc, reg_t *argv) {
	debugC(6, kDebugLevelGraphics, "kAddPlane %x:%x (%s)", PRINT_REG(argv[0]), s->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelAddPlane(argv[0]);
	return s->r_acc;
}

reg_t kUpdatePlane(EngineState *s, int argc, reg_t *argv) {
	debugC(7, kDebugLevelGraphics, "kUpdatePlane %x:%x (%s)", PRINT_REG(argv[0]), s->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelUpdatePlane(argv[0]);
	return s->r_acc;
}

reg_t kDeletePlane(EngineState *s, int argc, reg_t *argv) {
	debugC(6, kDebugLevelGraphics, "kDeletePlane %x:%x (%s)", PRINT_REG(argv[0]), s->_segMan->getObjectName(argv[0]));
	g_sci->_gfxFrameout->kernelDeletePlane(argv[0]);
	return s->r_acc;
}

reg_t kMovePlaneItems(EngineState *s, int argc, reg_t *argv) {
	const reg_t plane = argv[0];
	const int16 deltaX = argv[1].toSint16();
	const int16 deltaY = argv[2].toSint16();
	const bool scrollPics = argc > 3 ? argv[3].toUint16() : false;

	g_sci->_gfxFrameout->kernelMovePlaneItems(plane, deltaX, deltaY, scrollPics);
	return s->r_acc;
}

reg_t kAddPicAt(EngineState *s, int argc, reg_t *argv) {
	reg_t planeObj = argv[0];
	GuiResourceId pictureId = argv[1].toUint16();
	int16 x = argv[2].toSint16();
	int16 y = argv[3].toSint16();
	bool mirrorX = argc > 4 ? argv[4].toSint16() : false;
	bool deleteDuplicate = argc > 5 ? argv[5].toSint16() : true;

	g_sci->_gfxFrameout->kernelAddPicAt(planeObj, pictureId, x, y, mirrorX, deleteDuplicate);
	return s->r_acc;
}

reg_t kGetHighPlanePri(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_gfxFrameout->kernelGetHighPlanePri());
}

reg_t kFrameOut(EngineState *s, int argc, reg_t *argv) {
	bool showBits = argc > 0 ? argv[0].toUint16() : true;
	g_sci->_gfxFrameout->kernelFrameOut(showBits);
	return s->r_acc;
}

reg_t kSetPalStyleRange(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxTransitions32->kernelSetPalStyleRange(argv[0].toUint16(), argv[1].toUint16());
	return s->r_acc;
}

reg_t kObjectIntersect(EngineState *s, int argc, reg_t *argv) {
	Common::Rect objRect1 = g_sci->_gfxCompare->getNSRect(argv[0]);
	Common::Rect objRect2 = g_sci->_gfxCompare->getNSRect(argv[1]);
	return make_reg(0, objRect1.intersects(objRect2));
}

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
		return g_sci->_gfxText32->createFontBitmap(width, height, rect, text, foreColor, backColor, skipColor, fontId, alignment, borderColor, dimmed, true, true);
	} else {
		CelInfo32 celInfo;
		celInfo.type = kCelTypeView;
		celInfo.resourceId = readSelectorValue(segMan, object, SELECTOR(view));
		celInfo.loopNo = readSelectorValue(segMan, object, SELECTOR(loop));
		celInfo.celNo = readSelectorValue(segMan, object, SELECTOR(cel));
		return g_sci->_gfxText32->createFontBitmap(celInfo, rect, text, foreColor, backColor, fontId, skipColor, borderColor, dimmed, true);
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
	if (rect == nullptr) {
		error("kTextSize: %04x:%04x cannot be dereferenced", PRINT_REG(argv[0]));
	}

	Common::String text = s->_segMan->getString(argv[1]);
	int16 maxWidth = argc > 3 ? argv[3].toSint16() : 0;
	bool doScaling = argc > 4 ? argv[4].toSint16() : true;

	Common::Rect textRect = g_sci->_gfxText32->getTextSize(text, maxWidth, doScaling);
	rect[0] = make_reg(0, textRect.left);
	rect[1] = make_reg(0, textRect.top);
	rect[2] = make_reg(0, textRect.right - 1);
	rect[3] = make_reg(0, textRect.bottom - 1);
	return s->r_acc;
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

reg_t kMessageBox(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_gfxControls32->kernelMessageBox(s->_segMan->getString(argv[0]), s->_segMan->getString(argv[1]), argv[2].toUint16());
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
	g_sci->_gfxTransitions32->kernelSetShowStyle(argc, planeObj, type, seconds, back, priority, animate, refFrame, pFadeArray, divisions, blackScreen);

	return s->r_acc;
}

reg_t kCelHigh32(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId resourceId = argv[0].toUint16();
	int16 loopNo = argv[1].toSint16();
	int16 celNo = argv[2].toSint16();
	CelObjView celObj(resourceId, loopNo, celNo);
	return make_reg(0, mulru(celObj._height, Ratio(g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight, celObj._scaledHeight)));
}

reg_t kCelWide32(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId resourceId = argv[0].toUint16();
	int16 loopNo = argv[1].toSint16();
	int16 celNo = argv[2].toSint16();
	CelObjView celObj(resourceId, loopNo, celNo);
	return make_reg(0, mulru(celObj._width, Ratio(g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth, celObj._scaledWidth)));
}

reg_t kCelInfo(EngineState *s, int argc, reg_t *argv) {
	// Used by Shivers 1, room 23601 to determine what blocks on the red door puzzle board
	// are occupied by pieces already

	CelObjView view(argv[1].toUint16(), argv[2].toSint16(), argv[3].toSint16());

	int16 result = 0;

	switch (argv[0].toUint16()) {
	case 0:
		result = view._displace.x;
		break;
	case 1:
		result = view._displace.y;
		break;
	case 2:
	case 3:
		// null operation
		break;
	case 4:
		result = view.readPixel(argv[4].toSint16(), argv[5].toSint16(), view._mirrorX);
		break;
	}

	return make_reg(0, result);
}

reg_t kScrollWindow(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kScrollWindowCreate(EngineState *s, int argc, reg_t *argv) {
	const reg_t object = argv[0];
	const uint16 maxNumEntries = argv[1].toUint16();

	SegManager *segMan = s->_segMan;
	const int16 borderColor = readSelectorValue(segMan, object, SELECTOR(borderColor));
	const TextAlign alignment = (TextAlign)readSelectorValue(segMan, object, SELECTOR(mode));
	const GuiResourceId fontId = (GuiResourceId)readSelectorValue(segMan, object, SELECTOR(font));
	const int16 backColor = readSelectorValue(segMan, object, SELECTOR(back));
	const int16 foreColor = readSelectorValue(segMan, object, SELECTOR(fore));
	const reg_t plane = readSelector(segMan, object, SELECTOR(plane));

	Common::Rect rect;
	rect.left = readSelectorValue(segMan, object, SELECTOR(nsLeft));
	rect.top = readSelectorValue(segMan, object, SELECTOR(nsTop));
	rect.right = readSelectorValue(segMan, object, SELECTOR(nsRight)) + 1;
	rect.bottom = readSelectorValue(segMan, object, SELECTOR(nsBottom)) + 1;
	const Common::Point position(rect.left, rect.top);

	return g_sci->_gfxControls32->makeScrollWindow(rect, position, plane, foreColor, backColor, fontId, alignment, borderColor, maxNumEntries);
}

reg_t kScrollWindowAdd(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	const Common::String text = s->_segMan->getString(argv[1]);
	const GuiResourceId fontId = argv[2].toSint16();
	const int16 color = argv[3].toSint16();
	const TextAlign alignment = (TextAlign)argv[4].toSint16();
	const bool scrollTo = argc > 5 ? (bool)argv[5].toUint16() : true;

	return scrollWindow->add(text, fontId, color, alignment, scrollTo);
}

reg_t kScrollWindowWhere(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	const uint16 where = (argv[1].toUint16() * scrollWindow->where()).toInt();

	return make_reg(0, where);
}

reg_t kScrollWindowGo(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	const Ratio scrollTop(argv[1].toSint16(), argv[2].toSint16());
	scrollWindow->go(scrollTop);

	return s->r_acc;
}

reg_t kScrollWindowModify(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	const reg_t entryId = argv[1];
	const Common::String newText = s->_segMan->getString(argv[2]);
	const GuiResourceId fontId = argv[3].toSint16();
	const int16 color = argv[4].toSint16();
	const TextAlign alignment = (TextAlign)argv[5].toSint16();
	const bool scrollTo = argc > 6 ? (bool)argv[6].toUint16() : true;

	return scrollWindow->modify(entryId, newText, fontId, color, alignment, scrollTo);
}

reg_t kScrollWindowHide(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	scrollWindow->hide();

	return s->r_acc;
}

reg_t kScrollWindowShow(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	scrollWindow->show();

	return s->r_acc;
}

reg_t kScrollWindowPageUp(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	scrollWindow->pageUp();

	return s->r_acc;
}

reg_t kScrollWindowPageDown(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	scrollWindow->pageDown();

	return s->r_acc;
}

reg_t kScrollWindowUpArrow(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	scrollWindow->upArrow();

	return s->r_acc;
}

reg_t kScrollWindowDownArrow(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	scrollWindow->downArrow();

	return s->r_acc;
}

reg_t kScrollWindowHome(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	scrollWindow->home();

	return s->r_acc;
}

reg_t kScrollWindowEnd(EngineState *s, int argc, reg_t *argv) {
	ScrollWindow *scrollWindow = g_sci->_gfxControls32->getScrollWindow(argv[0]);

	scrollWindow->end();

	return s->r_acc;
}

reg_t kScrollWindowDestroy(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxControls32->destroyScrollWindow(argv[0]);

	return s->r_acc;
}

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
	return make_reg(0, g_sci->_gfxText32->_scaledHeight);
}

reg_t kSetFontRes(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxText32->_scaledWidth = argv[0].toUint16();
	g_sci->_gfxText32->_scaledHeight = argv[1].toUint16();
	return s->r_acc;
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

	reg_t bitmapId;
	SciBitmap &bitmap = *s->_segMan->allocateBitmap(&bitmapId, width, height, skipColor, 0, 0, scaledWidth, scaledHeight, 0, useRemap, true);
	memset(bitmap.getPixels(), backColor, width * height);
	return bitmapId;
}

reg_t kBitmapDestroy(EngineState *s, int argc, reg_t *argv) {
	s->_segMan->freeBitmap(argv[0]);
	return s->r_acc;
}

reg_t kBitmapDrawLine(EngineState *s, int argc, reg_t *argv) {
	// bitmapMemId, (x1, y1, x2, y2) OR (x2, y2, x1, y1), line color, unknown int, unknown int
	return kStubNull(s, argc + 1, argv - 1);
}

reg_t kBitmapDrawView(EngineState *s, int argc, reg_t *argv) {
	SciBitmap &bitmap = *s->_segMan->lookupBitmap(argv[0]);
	CelObjView view(argv[1].toUint16(), argv[2].toSint16(), argv[3].toSint16());

	const int16 x = argc > 4 ? argv[4].toSint16() : 0;
	const int16 y = argc > 5 ? argv[5].toSint16() : 0;
	const int16 alignX = argc > 7 ? argv[7].toSint16() : -1;
	const int16 alignY = argc > 8 ? argv[8].toSint16() : -1;

	Common::Point position(
		x == -1 ? bitmap.getDisplace().x : x,
		y == -1 ? bitmap.getDisplace().y : y
	);

	position.x -= alignX == -1 ? view._displace.x : alignX;
	position.y -= alignY == -1 ? view._displace.y : alignY;

	Common::Rect drawRect(
		position.x,
		position.y,
		position.x + view._width,
		position.y + view._height
	);
	drawRect.clip(Common::Rect(bitmap.getWidth(), bitmap.getHeight()));
	view.draw(bitmap.getBuffer(), drawRect, position, view._mirrorX);
	return s->r_acc;
}

reg_t kBitmapDrawText(EngineState *s, int argc, reg_t *argv) {
	// called e.g. from TextButton::createBitmap() in Torin's Passage, script 64894

	SciBitmap &bitmap = *s->_segMan->lookupBitmap(argv[0]);
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

	reg_t textBitmapObject = g_sci->_gfxText32->createFontBitmap(textRect.width(), textRect.height(), Common::Rect(textRect.width(), textRect.height()), text, foreColor, backColor, skipColor, fontId, alignment, borderColor, dimmed, false, false);
	CelObjMem textCel(textBitmapObject);
	textCel.draw(bitmap.getBuffer(), textRect, Common::Point(textRect.left, textRect.top), false);
	s->_segMan->freeBitmap(textBitmapObject);

	return s->r_acc;
}

reg_t kBitmapDrawColor(EngineState *s, int argc, reg_t *argv) {
	// called e.g. from TextView::init() and TextView::draw() in Torin's Passage, script 64890

	SciBitmap &bitmap = *s->_segMan->lookupBitmap(argv[0]);
	Common::Rect fillRect(
		argv[1].toSint16(),
		argv[2].toSint16(),
		argv[3].toSint16() + 1,
		argv[4].toSint16() + 1
	);

	bitmap.getBuffer().fillRect(fillRect, argv[5].toSint16());
	return s->r_acc;
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
	SciBitmap &bitmap = *s->_segMan->lookupBitmap(argv[0]);
	bitmap.setDisplace(Common::Point(argv[1].toSint16(), argv[2].toSint16()));
	return s->r_acc;
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

reg_t kEditText(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_gfxControls32->kernelEditText(argv[0]);
}

reg_t kAddLine(EngineState *s, int argc, reg_t *argv) {
	const reg_t plane = argv[0];
	const Common::Point startPoint(argv[1].toSint16(), argv[2].toSint16());
	const Common::Point endPoint(argv[3].toSint16(), argv[4].toSint16());

	int16 priority;
	uint8 color;
	LineStyle style;
	uint16 pattern;
	uint8 thickness;

	if (argc == 10) {
		priority = argv[5].toSint16();
		color = (uint8)argv[6].toUint16();
		style = (LineStyle)argv[7].toSint16();
		pattern = argv[8].toUint16();
		thickness = (uint8)argv[9].toUint16();
	} else {
		priority = 1000;
		color = 255;
		style = kLineStyleSolid;
		pattern = 0;
		thickness = 1;
	}

	return g_sci->_gfxPaint32->kernelAddLine(plane, startPoint, endPoint, priority, color, style, pattern, thickness);
}

reg_t kUpdateLine(EngineState *s, int argc, reg_t *argv) {
	const reg_t screenItemObject = argv[0];
	const reg_t planeObject = argv[1];
	const Common::Point startPoint(argv[2].toSint16(), argv[3].toSint16());
	const Common::Point endPoint(argv[4].toSint16(), argv[5].toSint16());

	int16 priority;
	uint8 color;
	LineStyle style;
	uint16 pattern;
	uint8 thickness;

	Plane *plane = g_sci->_gfxFrameout->getPlanes().findByObject(planeObject);
	if (plane == nullptr) {
		error("kUpdateLine: Plane %04x:%04x not found", PRINT_REG(planeObject));
	}

	ScreenItem *screenItem = plane->_screenItemList.findByObject(screenItemObject);
	if (screenItem == nullptr) {
		error("kUpdateLine: Screen item %04x:%04x not found", PRINT_REG(screenItemObject));
	}

	if (argc == 11) {
		priority = argv[6].toSint16();
		color = (uint8)argv[7].toUint16();
		style = (LineStyle)argv[8].toSint16();
		pattern = argv[9].toUint16();
		thickness = (uint8)argv[10].toUint16();
	} else {
		priority = screenItem->_priority;
		color = screenItem->_celInfo.color;
		style = kLineStyleSolid;
		pattern = 0;
		thickness = 1;
	}

	g_sci->_gfxPaint32->kernelUpdateLine(screenItem, plane, startPoint, endPoint, priority, color, style, pattern, thickness);

	return s->r_acc;
}

reg_t kDeleteLine(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxPaint32->kernelDeleteLine(argv[0], argv[1]);
	return s->r_acc;
}

// Used by LSL6hires intro (room 110)
reg_t kSetScroll(EngineState *s, int argc, reg_t *argv) {
	const reg_t plane = argv[0];
	const int16 deltaX = argv[1].toSint16();
	const int16 deltaY = argv[2].toSint16();
	const GuiResourceId pictureId = argv[3].toUint16();
	const bool animate = argv[4].toUint16();
	// NOTE: speed was accepted as an argument, but then never actually used
	// const int16 speed = argc > 5 ? (bool)argv[5].toSint16() : -1;
	const bool mirrorX = argc > 6 ? (bool)argv[6].toUint16() : false;

	g_sci->_gfxTransitions32->kernelSetScroll(plane, deltaX, deltaY, pictureId, animate, mirrorX);
	return s->r_acc;
}

// Used by SQ6, script 900, the datacorder reprogramming puzzle (from room 270)
reg_t kMorphOn(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxFrameout->_palMorphIsOn = true;
	return s->r_acc;
}

reg_t kPaletteSetFromResource32(EngineState *s, int argc, reg_t *argv) {
	const GuiResourceId paletteId = argv[0].toUint16();
	g_sci->_gfxPalette32->loadPalette(paletteId);
	return s->r_acc;
}

reg_t kPaletteFindColor32(EngineState *s, int argc, reg_t *argv) {
	const uint8 r = argv[0].toUint16();
	const uint8 g = argv[1].toUint16();
	const uint8 b = argv[2].toUint16();
	return make_reg(0, g_sci->_gfxPalette32->matchColor(r, g, b));
}

reg_t kPaletteSetFade(EngineState *s, int argc, reg_t *argv) {
	uint16 fromColor = argv[0].toUint16();
	uint16 toColor = argv[1].toUint16();
	uint16 percent = argv[2].toUint16();
	g_sci->_gfxPalette32->setFade(percent, fromColor, toColor);
	return s->r_acc;
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
	return s->r_acc;
}

reg_t kPalVarySetPercent(EngineState *s, int argc, reg_t *argv) {
	int time = argc > 0 ? argv[0].toSint16() * 60 : 0;
	int16 percent = argc > 1 ? argv[1].toSint16() : 0;
	g_sci->_gfxPalette32->setVaryPercent(percent, time, -1, -1);
	return s->r_acc;
}

reg_t kPalVaryGetPercent(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_gfxPalette32->getVaryPercent());
}

reg_t kPalVaryOff(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxPalette32->varyOff();
	return s->r_acc;
}

reg_t kPalVaryMergeTarget(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId paletteId = argv[0].toUint16();
	g_sci->_gfxPalette32->kernelPalVaryMergeTarget(paletteId);
	return make_reg(0, g_sci->_gfxPalette32->getVaryPercent());
}

reg_t kPalVarySetTime(EngineState *s, int argc, reg_t *argv) {
	int time = argv[0].toSint16() * 60;
	g_sci->_gfxPalette32->setVaryTime(time);
	return s->r_acc;
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

reg_t kPalCycle(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kPalCycleSetCycle(EngineState *s, int argc, reg_t *argv) {
	const uint16 fromColor = argv[0].toUint16();
	const uint16 toColor = argv[1].toUint16();
	const int16 direction = argv[2].toSint16();
	const uint16 delay = argc > 3 ? argv[3].toUint16() : 0;

	g_sci->_gfxPalette32->setCycle(fromColor, toColor, direction, delay);
	return s->r_acc;
}

reg_t kPalCycleDoCycle(EngineState *s, int argc, reg_t *argv) {
	const uint16 fromColor = argv[0].toUint16();
	const int16 speed = argc > 1 ? argv[1].toSint16() : 1;

	g_sci->_gfxPalette32->doCycle(fromColor, speed);
	return s->r_acc;
}

reg_t kPalCyclePause(EngineState *s, int argc, reg_t *argv) {
	if (argc == 0) {
		g_sci->_gfxPalette32->cycleAllPause();
	} else {
		const uint16 fromColor = argv[0].toUint16();
		g_sci->_gfxPalette32->cyclePause(fromColor);
	}
	return s->r_acc;
}

reg_t kPalCycleOn(EngineState *s, int argc, reg_t *argv) {
	if (argc == 0) {
		g_sci->_gfxPalette32->cycleAllOn();
	} else {
		const uint16 fromColor = argv[0].toUint16();
		g_sci->_gfxPalette32->cycleOn(fromColor);
	}
	return s->r_acc;
}

reg_t kPalCycleOff(EngineState *s, int argc, reg_t *argv) {
	if (argc == 0) {
		g_sci->_gfxPalette32->cycleAllOff();
	} else {
		const uint16 fromColor = argv[0].toUint16();
		g_sci->_gfxPalette32->cycleOff(fromColor);
	}
	return s->r_acc;
}

reg_t kRemapColors32(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kRemapColorsOff(EngineState *s, int argc, reg_t *argv) {
	if (argc == 0) {
		g_sci->_gfxRemap32->remapAllOff();
	} else {
		const uint8 color = argv[0].toUint16();
		g_sci->_gfxRemap32->remapOff(color);
	}
	return s->r_acc;
}

reg_t kRemapColorsByRange(EngineState *s, int argc, reg_t *argv) {
	const uint8 color = argv[0].toUint16();
	const int16 from = argv[1].toSint16();
	const int16 to = argv[2].toSint16();
	const int16 base = argv[3].toSint16();
	// NOTE: There is an optional last parameter after `base`
	// which was only used by the priority map debugger, which
	// does not exist in release versions of SSCI
	g_sci->_gfxRemap32->remapByRange(color, from, to, base);
	return s->r_acc;
}

reg_t kRemapColorsByPercent(EngineState *s, int argc, reg_t *argv) {
	const uint8 color = argv[0].toUint16();
	const int16 percent = argv[1].toSint16();
	// NOTE: There is an optional last parameter after `percent`
	// which was only used by the priority map debugger, which
	// does not exist in release versions of SSCI
	g_sci->_gfxRemap32->remapByPercent(color, percent);
	return s->r_acc;
}

reg_t kRemapColorsToGray(EngineState *s, int argc, reg_t *argv) {
	const uint8 color = argv[0].toUint16();
	const int16 gray = argv[1].toSint16();
	// NOTE: There is an optional last parameter after `gray`
	// which was only used by the priority map debugger, which
	// does not exist in release versions of SSCI
	g_sci->_gfxRemap32->remapToGray(color, gray);
	return s->r_acc;
}

reg_t kRemapColorsToPercentGray(EngineState *s, int argc, reg_t *argv) {
	const uint8 color = argv[0].toUint16();
	const int16 gray = argv[1].toSint16();
	const int16 percent = argv[2].toSint16();
	// NOTE: There is an optional last parameter after `percent`
	// which was only used by the priority map debugger, which
	// does not exist in release versions of SSCI
	g_sci->_gfxRemap32->remapToPercentGray(color, gray, percent);
	return s->r_acc;
}

reg_t kRemapColorsBlockRange(EngineState *s, int argc, reg_t *argv) {
	const uint8 from = argv[0].toUint16();
	const uint8 count = argv[1].toUint16();
	g_sci->_gfxRemap32->blockRange(from, count);
	return s->r_acc;
}

#endif

} // End of namespace Sci
