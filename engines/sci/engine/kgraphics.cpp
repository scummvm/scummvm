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

#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/video/avi_decoder.h"
#include "graphics/video/qt_decoder.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/debug.h"	// for g_debug_sleeptime_factor
#include "sci/resource.h"
#include "sci/video/seq_decoder.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/controls.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/view.h"
#ifdef ENABLE_SCI32
#include "sci/video/vmd_decoder.h"
#endif

namespace Sci {

void _k_dirloop(reg_t object, uint16 angle, EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = readSelectorValue(s->_segMan, object, SELECTOR(view));
	uint16 signal = readSelectorValue(s->_segMan, object, SELECTOR(signal));
	int16 loopNo;
	int16 maxLoops;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (signal & kSignalDoesntTurn)
		return;

	angle %= 360;

	if (!oldScriptHeader) {
		if (angle < 45)
			loopNo = 3;
		else if (angle < 136)
			loopNo = 0;
		else if (angle < 225)
			loopNo = 2;
		else if (angle < 316)
			loopNo = 1;
		else
			loopNo = 3;
	} else {
		if (angle >= 330 || angle <= 30)
			loopNo = 3;
		else if (angle <= 150)
			loopNo = 0;
		else if (angle <= 210)
			loopNo = 2;
		else if (angle < 330)
			loopNo = 1;
		else loopNo = -1;
	}

	maxLoops = g_sci->_gfxCache->kernelViewGetLoopCount(viewId);
		

	if ((loopNo > 1) && (maxLoops < 4))
		return;

	writeSelectorValue(s->_segMan, object, SELECTOR(loop), loopNo);
}

static reg_t kSetCursorSci0(EngineState *s, int argc, reg_t *argv) {
	Common::Point pos;
	GuiResourceId cursorId = argv[0].toSint16();

	// Set pointer position, if requested
	if (argc >= 4) {
		pos.y = argv[3].toSint16();
		pos.x = argv[2].toSint16();
		g_sci->_gfxCursor->kernelSetPos(pos);
	}

	if ((argc >= 2) && (argv[1].toSint16() == 0)) {
		cursorId = -1;
	}

	g_sci->_gfxCursor->kernelSetShape(cursorId);
	return s->r_acc;
}

static reg_t kSetCursorSci11(EngineState *s, int argc, reg_t *argv) {
	Common::Point pos;
	Common::Point *hotspot = NULL;

	switch (argc) {
	case 1:
		switch (argv[0].toSint16()) {
		case 0:
			g_sci->_gfxCursor->kernelHide();
			break;
		case -1:
			// TODO: Special case at least in kq6, check disassembly
			break;
		case -2:
			// TODO: Special case at least in kq6, check disassembly
			break;
		default:
			g_sci->_gfxCursor->kernelShow();
			break;
		}
		break;
	case 2:
		pos.y = argv[1].toSint16();
		pos.x = argv[0].toSint16();
		
		g_sci->_gfxCursor->kernelSetPos(pos);
		break;
	case 4: {
		int16 top = argv[0].toSint16();
		int16 left = argv[1].toSint16();
		int16 bottom = argv[2].toSint16();
		int16 right = argv[3].toSint16();

		// In SCI32, the right parameter seems to be divided by 2
		if (getSciVersion() >= SCI_VERSION_2)
			right *= 2;

		if ((right >= left) && (bottom >= top)) {
			Common::Rect rect = Common::Rect(left, top, right, bottom);
			g_sci->_gfxCursor->kernelSetMoveZone(rect);
		} else {
			warning("kSetCursor: Ignoring invalid mouse zone (%i, %i)-(%i, %i)", left, top, right, bottom);
		}
		break;
	}
	case 5:
	case 9:
		hotspot = new Common::Point(argv[3].toSint16(), argv[4].toSint16());
		// Fallthrough
	case 3:
		if (g_sci->getPlatform() == Common::kPlatformMacintosh)
			g_sci->_gfxCursor->kernelSetMacCursor(argv[0].toUint16(), argv[1].toUint16(), argv[2].toUint16(), hotspot);
		else
			g_sci->_gfxCursor->kernelSetView(argv[0].toUint16(), argv[1].toUint16(), argv[2].toUint16(), hotspot);
		break;
	default :
		error("kSetCursor: Unhandled case: %d arguments given", argc);
		break;
	}
	return s->r_acc;
}

reg_t kSetCursor(EngineState *s, int argc, reg_t *argv) {
	switch (g_sci->_features->detectSetCursorType()) {
	case SCI_VERSION_0_EARLY:
		return kSetCursorSci0(s, argc, argv);
	case SCI_VERSION_1_1:
		return kSetCursorSci11(s, argc, argv);
	default:
		error("Unknown SetCursor type");
		return NULL_REG;
	}
}

reg_t kMoveCursor(EngineState *s, int argc, reg_t *argv) {
	Common::Point pos;
	if (argc == 2) {
		pos.y = argv[1].toSint16();
		pos.x = argv[0].toSint16();
		g_sci->_gfxCursor->kernelSetPos(pos);
	}
	return s->r_acc;
}

reg_t kPicNotValid(EngineState *s, int argc, reg_t *argv) {
	int16 newPicNotValid = (argc > 0) ? argv[0].toUint16() : -1;

	return make_reg(0, g_sci->_gfxScreen->kernelPicNotValid(newPicNotValid));
}

static Common::Rect getGraphRect(reg_t *argv) {
	int16 x = argv[1].toSint16();
	int16 y = argv[0].toSint16();
	int16 x1 = argv[3].toSint16();
	int16 y1 = argv[2].toSint16();
	if (x > x1) SWAP(x, x1);
	if (y > y1) SWAP(y, y1);
	return Common::Rect(x, y, x1, y1);
}

static Common::Point getGraphPoint(reg_t *argv) {
	int16 x = argv[1].toSint16();
	int16 y = argv[0].toSint16();
	return Common::Point(x, y);
}

reg_t kGraph(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kGraphGetColorCount(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getResMan()->isAmiga32color())
		return make_reg(0, 32);
	return make_reg(0, !g_sci->getResMan()->isVGA() ? 16 : 256);
}

reg_t kGraphDrawLine(EngineState *s, int argc, reg_t *argv) {
	int16 color = argv[4].toSint16();
	int16 priority = (argc > 5) ? argv[5].toSint16() : -1;
	int16 control = (argc > 6) ? argv[6].toSint16() : -1;

	// TODO: Find out why we get >15 for color in EGA
	if (!g_sci->getResMan()->isVGA() && !g_sci->getResMan()->isAmiga32color())
		color &= 0x0F;

	g_sci->_gfxPaint16->kernelGraphDrawLine(getGraphPoint(argv), getGraphPoint(argv + 2), color, priority, control);
	return s->r_acc;
}

reg_t kGraphSaveBox(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect = getGraphRect(argv);
	uint16 screenMask = (argc > 4) ? argv[4].toUint16() : 0;
	return g_sci->_gfxPaint16->kernelGraphSaveBox(rect, screenMask);
}

reg_t kGraphRestoreBox(EngineState *s, int argc, reg_t *argv) {
	// This may be called with a memoryhandle from SAVE_BOX or SAVE_UPSCALEDHIRES_BOX
	g_sci->_gfxPaint16->kernelGraphRestoreBox(argv[0]);
	return s->r_acc;
}

reg_t kGraphFillBoxBackground(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect = getGraphRect(argv);
	g_sci->_gfxPaint16->kernelGraphFillBoxBackground(rect);
	return s->r_acc;
}

reg_t kGraphFillBoxForeground(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect = getGraphRect(argv);
	g_sci->_gfxPaint16->kernelGraphFillBoxForeground(rect);
	return s->r_acc;
}

reg_t kGraphFillBoxAny(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect = getGraphRect(argv);
	int16 colorMask = argv[4].toUint16();
	int16 color = argv[5].toSint16();
	int16 priority = (argc > 6) ? argv[6].toSint16() : -1;
	int16 control = (argc > 7) ? argv[7].toSint16() : -1;

	g_sci->_gfxPaint16->kernelGraphFillBox(rect, colorMask, color, priority, control);
	return s->r_acc;
}

reg_t kGraphUpdateBox(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect = getGraphRect(argv);
	// argv[4] is the map (1 for visual, etc.)
	// argc == 6 on upscaled hires
	bool hiresMode = (argc > 5) ? true : false;
	g_sci->_gfxPaint16->kernelGraphUpdateBox(rect, hiresMode);
	return s->r_acc;
}

reg_t kGraphRedrawBox(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect = getGraphRect(argv);
	g_sci->_gfxPaint16->kernelGraphRedrawBox(rect);
	return s->r_acc;
}

// Seems to be only implemented for SCI0/SCI01 games
reg_t kGraphAdjustPriority(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxPorts->kernelGraphAdjustPriority(argv[0].toUint16(), argv[1].toUint16());
	return s->r_acc;
}

reg_t kGraphSaveUpscaledHiresBox(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect = getGraphRect(argv);
	return g_sci->_gfxPaint16->kernelGraphSaveUpscaledHiresBox(rect);
}

reg_t kTextSize(EngineState *s, int argc, reg_t *argv) {
	int16 textWidth, textHeight;
	Common::String text = s->_segMan->getString(argv[1]);
	reg_t *dest = s->_segMan->derefRegPtr(argv[0], 4);
	int maxwidth = (argc > 3) ? argv[3].toUint16() : 0;
	int font_nr = argv[2].toUint16();

	Common::String sep_str;
	const char *sep = NULL;
	if ((argc > 4) && (argv[4].segment)) {
		sep_str = s->_segMan->getString(argv[4]);
		sep = sep_str.c_str();
	}

	dest[0] = dest[1] = NULL_REG;

	if (text.empty() || !dest) { // Empty text
		dest[2] = dest[3] = make_reg(0, 0);
		debugC(2, kDebugLevelStrings, "GetTextSize: Empty string");
		return s->r_acc;
	}

	textWidth = dest[3].toUint16(); textHeight = dest[2].toUint16();

#ifdef ENABLE_SCI32
	if (!g_sci->_gfxText16) {
		// TODO: Implement this
		textWidth = 0; textHeight = 0;
	} else
#endif
		g_sci->_gfxText16->kernelTextSize(g_sci->strSplit(text.c_str(), sep).c_str(), font_nr, maxwidth, &textWidth, &textHeight);
	
	debugC(2, kDebugLevelStrings, "GetTextSize '%s' -> %dx%d", text.c_str(), textWidth, textHeight);
	dest[2] = make_reg(0, textHeight);
	dest[3] = make_reg(0, textWidth);
	return s->r_acc;
}

reg_t kWait(EngineState *s, int argc, reg_t *argv) {
	int sleep_time = argv[0].toUint16();

	s->wait(sleep_time);

	return s->r_acc;
}

reg_t kCoordPri(EngineState *s, int argc, reg_t *argv) {
	int16 y = argv[0].toSint16();

	if ((argc < 2) || (y != 1)) {
		return make_reg(0, g_sci->_gfxPorts->kernelCoordinateToPriority(y));
	} else {
		int16 priority = argv[1].toSint16();
		return make_reg(0, g_sci->_gfxPorts->kernelPriorityToCoordinate(priority));
	}
}

reg_t kPriCoord(EngineState *s, int argc, reg_t *argv) {
	int16 priority = argv[0].toSint16();

	return make_reg(0, g_sci->_gfxPorts->kernelPriorityToCoordinate(priority));
}

reg_t kDirLoop(EngineState *s, int argc, reg_t *argv) {
	_k_dirloop(argv[0], argv[1].toUint16(), s, argc, argv);

	return s->r_acc;
}

reg_t kCanBeHere(EngineState *s, int argc, reg_t *argv) {
	reg_t curObject = argv[0];
	reg_t listReference = (argc > 1) ? argv[1] : NULL_REG;

	reg_t canBeHere = g_sci->_gfxCompare->kernelCanBeHere(curObject, listReference);
	return make_reg(0, canBeHere.isNull() ? 1 : 0);
}

reg_t kCantBeHere(EngineState *s, int argc, reg_t *argv) {
	reg_t curObject = argv[0];
	reg_t listReference = (argc > 1) ? argv[1] : NULL_REG;
	
	reg_t canBeHere = g_sci->_gfxCompare->kernelCanBeHere(curObject, listReference);
	return canBeHere;
}

reg_t kIsItSkip(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = argv[0].toSint16();
	int16 loopNo = argv[1].toSint16();
	int16 celNo = argv[2].toSint16();
	Common::Point position(argv[4].toUint16(), argv[3].toUint16());

	bool result = g_sci->_gfxCompare->kernelIsItSkip(viewId, loopNo, celNo, position);
	return make_reg(0, result);
}

reg_t kCelHigh(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = argv[0].toSint16();
	if (viewId == -1)	// Happens in SCI32
		return NULL_REG;
	int16 loopNo = argv[1].toSint16();
	int16 celNo = (argc >= 3) ? argv[2].toSint16() : 0;
	int16 celHeight;

	celHeight = g_sci->_gfxCache->kernelViewGetCelHeight(viewId, loopNo, celNo);

	return make_reg(0, celHeight);
}

reg_t kCelWide(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = argv[0].toSint16();
	if (viewId == -1)	// Happens in SCI32
		return NULL_REG;
	int16 loopNo = argv[1].toSint16();
	int16 celNo = (argc >= 3) ? argv[2].toSint16() : 0;
	int16 celWidth;

	celWidth = g_sci->_gfxCache->kernelViewGetCelWidth(viewId, loopNo, celNo);

	if (getSciVersion() == SCI_VERSION_2_1)
		celWidth = celWidth / 2; // half the width returned here, fixes lsl6 action icon placements
	// the scripts work low-res and add the returned value from here to the coordinate
	// TODO: check, if this is actually right. I'm slightly confused by this, but even GK1CD has some idivs in this
	//  code, so it seems plausible.
	return make_reg(0, celWidth);
}

reg_t kNumLoops(EngineState *s, int argc, reg_t *argv) {
	reg_t object = argv[0];
	GuiResourceId viewId = readSelectorValue(s->_segMan, object, SELECTOR(view));
	int16 loopCount;

	loopCount = g_sci->_gfxCache->kernelViewGetLoopCount(viewId);

	debugC(2, kDebugLevelGraphics, "NumLoops(view.%d) = %d", viewId, loopCount);

	return make_reg(0, loopCount);
}

reg_t kNumCels(EngineState *s, int argc, reg_t *argv) {
	reg_t object = argv[0];
	GuiResourceId viewId = readSelectorValue(s->_segMan, object, SELECTOR(view));
	int16 loopNo = readSelectorValue(s->_segMan, object, SELECTOR(loop));
	int16 celCount;

	celCount = g_sci->_gfxCache->kernelViewGetCelCount(viewId, loopNo);

	debugC(2, kDebugLevelGraphics, "NumCels(view.%d, %d) = %d", viewId, loopNo, celCount);

	return make_reg(0, celCount);
}

reg_t kOnControl(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect;
	byte screenMask;
	int argBase = 0;

	if ((argc == 2) || (argc == 4)) {
		screenMask = GFX_SCREEN_MASK_CONTROL;
	} else {
		screenMask = argv[0].toUint16();
		argBase = 1;
	}
	rect.left = argv[argBase].toSint16();
	rect.top = argv[argBase + 1].toSint16();
	if (argc > 3) {
		rect.right = argv[argBase + 2].toSint16();
		rect.bottom = argv[argBase + 3].toSint16();
	} else {
		rect.right = rect.left + 1;
		rect.bottom = rect.top + 1;
	}
	uint16 result = g_sci->_gfxCompare->kernelOnControl(screenMask, rect);
	return make_reg(0, result);
}

#define K_DRAWPIC_FLAGS_MIRRORED			(1 << 14)
#define K_DRAWPIC_FLAGS_ANIMATIONBLACKOUT	(1 << 15)

reg_t kDrawPic(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId pictureId = argv[0].toUint16();
	uint16 flags = 0;
	int16 animationNr = -1;
	bool animationBlackoutFlag = false;
	bool mirroredFlag = false;
	bool addToFlag = false;
	int16 EGApaletteNo = 0; // default needs to be 0

	if (argc >= 2) {
		flags = argv[1].toUint16();
		if (flags & K_DRAWPIC_FLAGS_ANIMATIONBLACKOUT)
			animationBlackoutFlag = true;
		animationNr = flags & 0xFF;
		if (flags & K_DRAWPIC_FLAGS_MIRRORED)
			mirroredFlag = true;
	}
	if (argc >= 3) {
		if (!argv[2].isNull())
			addToFlag = true;
		if (!g_sci->_features->usesOldGfxFunctions())
			addToFlag = !addToFlag;
	}
	if (argc >= 4)
		EGApaletteNo = argv[3].toUint16();

	g_sci->_gfxPaint16->kernelDrawPicture(pictureId, animationNr, animationBlackoutFlag, mirroredFlag, addToFlag, EGApaletteNo);

	return s->r_acc;
}

reg_t kBaseSetter(EngineState *s, int argc, reg_t *argv) {
	reg_t object = argv[0];

	g_sci->_gfxCompare->kernelBaseSetter(object);
	return s->r_acc;
}

reg_t kSetNowSeen(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxCompare->kernelSetNowSeen(argv[0]);

	return s->r_acc;
}

// we are called on EGA/amiga games as well, this doesnt make sense.
//  doing this would actually break the system EGA/amiga palette
reg_t kPalette(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kPaletteSetFromResource(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getResMan()->isVGA()) {
		GuiResourceId resourceId = argv[0].toUint16();
		bool force = false;
		if (argc == 2)
			force = argv[1].toUint16() == 2 ? true : false;
		g_sci->_gfxPalette->kernelSetFromResource(resourceId, force);
	}
	return s->r_acc;
}

reg_t kPaletteSetFlag(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getResMan()->isVGA()) {
		uint16 fromColor = CLIP<uint16>(argv[0].toUint16(), 1, 255);
		uint16 toColor = CLIP<uint16>(argv[1].toUint16(), 1, 255);
		uint16 flags = argv[2].toUint16();
		g_sci->_gfxPalette->kernelSetFlag(fromColor, toColor, flags);
	}
	return s->r_acc;
}

reg_t kPaletteUnsetFlag(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getResMan()->isVGA()) {
		uint16 fromColor = CLIP<uint16>(argv[0].toUint16(), 1, 255);
		uint16 toColor = CLIP<uint16>(argv[1].toUint16(), 1, 255);
		uint16 flags = argv[2].toUint16();
		g_sci->_gfxPalette->kernelUnsetFlag(fromColor, toColor, flags);
	}
	return s->r_acc;
}

reg_t kPaletteSetIntensity(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getResMan()->isVGA()) {
		uint16 fromColor = CLIP<uint16>(argv[0].toUint16(), 1, 255);
		uint16 toColor = CLIP<uint16>(argv[1].toUint16(), 1, 255);
		uint16 intensity = argv[2].toUint16();
		bool setPalette = (argc < 4) ? true : (argv[3].isNull()) ? true : false;

		g_sci->_gfxPalette->kernelSetIntensity(fromColor, toColor, intensity, setPalette);
	}
	return s->r_acc;
}

reg_t kPaletteFindColor(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getResMan()->isVGA()) {
		uint16 r = argv[0].toUint16();
		uint16 g = argv[1].toUint16();
		uint16 b = argv[2].toUint16();
		return make_reg(0, g_sci->_gfxPalette->kernelFindColor(r, g, b));
	}
	return NULL_REG;
}

reg_t kPaletteAnimate(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getResMan()->isVGA()) {
		int16 argNr;
		bool paletteChanged = false;
		for (argNr = 0; argNr < argc; argNr += 3) {
			uint16 fromColor = argv[argNr].toUint16();
			uint16 toColor = argv[argNr + 1].toUint16();
			int16 speed = argv[argNr + 2].toSint16();
			if (g_sci->_gfxPalette->kernelAnimate(fromColor, toColor, speed))
				paletteChanged = true;
		}
		if (paletteChanged)
			g_sci->_gfxPalette->kernelAnimateSet();
	}
	return s->r_acc;
}

reg_t kPaletteSave(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getResMan()->isVGA()) {
		warning("kPalette(7), save palette to heap STUB");
	}
	return NULL_REG;
}

reg_t kPaletteRestore(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getResMan()->isVGA()) {
		warning("kPalette(8), restore palette from heap STUB");
	}
	return s->r_acc;
}

reg_t kPalVary(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kPalVaryInit(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId paletteId = argv[0].toUint16();
	uint16 ticks = argv[1].toUint16();
	uint16 stepStop = argc >= 3 ? argv[2].toUint16() : 64;
	uint16 direction = argc >= 4 ? argv[3].toUint16() : 1;
	if (g_sci->_gfxPalette->kernelPalVaryInit(paletteId, ticks, stepStop, direction))
		return SIGNAL_REG;
	return NULL_REG;
}

reg_t kPalVaryReverse(EngineState *s, int argc, reg_t *argv) {
	int16 ticks = argc >= 1 ? argv[0].toUint16() : -1;
	int16 stepStop = argc >= 2 ? argv[1].toUint16() : 0;
	int16 direction = argc >= 3 ? argv[2].toSint16() : -1;

	return make_reg(0, g_sci->_gfxPalette->kernelPalVaryReverse(ticks, stepStop, direction));
}

reg_t kPalVaryGetCurrentStep(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_gfxPalette->kernelPalVaryGetCurrentStep());
}

reg_t kPalVaryDeinit(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxPalette->kernelPalVaryDeinit();
	return NULL_REG;
}

reg_t kPalVaryChangeTarget(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId paletteId = argv[0].toUint16();
	int16 currentStep = g_sci->_gfxPalette->kernelPalVaryChangeTarget(paletteId);
	return make_reg(0, currentStep);
}

reg_t kPalVaryChangeTicks(EngineState *s, int argc, reg_t *argv) {
	uint16 ticks = argv[0].toUint16();
	g_sci->_gfxPalette->kernelPalVaryChangeTicks(ticks);
	return NULL_REG;
}

reg_t kPalVaryPauseResume(EngineState *s, int argc, reg_t *argv) {
	bool pauseState = !argv[0].isNull();
	g_sci->_gfxPalette->kernelPalVaryPause(pauseState);
	return NULL_REG;
}

reg_t kPalVaryUnknown(EngineState *s, int argc, reg_t *argv) {
	// Unknown (seems to be SCI32 exclusive)
	return NULL_REG;
}

reg_t kAssertPalette(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId paletteId = argv[0].toUint16();

	g_sci->_gfxPalette->kernelAssertPalette(paletteId);
	return s->r_acc;
}

// Used to show hires character portraits in the Windows CD version of KQ6
reg_t kPortrait(EngineState *s, int argc, reg_t *argv) {
	uint16 operation = argv[0].toUint16();

	switch (operation) {
	case 0: { // load
		if (argc == 2) {
			Common::String resourceName = s->_segMan->getString(argv[1]);
			s->r_acc = g_sci->_gfxPaint16->kernelPortraitLoad(resourceName);
		} else {
			error("kPortrait(loadResource) called with unsupported argc %d", argc);
		}
		break;
	}
	case 1: { // show
		if (argc == 10) {
			Common::String resourceName = s->_segMan->getString(argv[1]);
			Common::Point position = Common::Point(argv[2].toUint16(), argv[3].toUint16());
			uint resourceNum = argv[4].toUint16();
			uint noun = argv[5].toUint16() & 0xff;
			uint verb = argv[6].toUint16() & 0xff;
			uint cond = argv[7].toUint16() & 0xff;
			uint seq = argv[8].toUint16() & 0xff;
			// argv[9] is usually 0??!!

			g_sci->_gfxPaint16->kernelPortraitShow(resourceName, position, resourceNum, noun, verb, cond, seq);
			return SIGNAL_REG;
		} else {
			error("kPortrait(show) called with unsupported argc %d", argc);
		}
		break;
	}
	case 2: { // unload
		if (argc == 2) {
			uint16 portraitId = argv[1].toUint16();
			g_sci->_gfxPaint16->kernelPortraitUnload(portraitId);
		} else {
			error("kPortrait(unload) called with unsupported argc %d", argc);
		}
		break;
	}
	default:
		error("kPortrait(%d), not implemented (argc = %d)", operation, argc);
	}

	return s->r_acc;
}

// Original top-left must stay on kControl rects, we adjust accordingly because sierra sci actually wont draw rects that
//  are upside down (example: jones, when challenging jones - one button is a duplicate and also has lower-right which is 0, 0)
Common::Rect kControlCreateRect(int16 x, int16 y, int16 x1, int16 y1) {
	if (x > x1) x1 = x;
	if (y > y1) y1 = y;
	return Common::Rect(x, y, x1, y1);
}

void _k_GenericDrawControl(EngineState *s, reg_t controlObject, bool hilite) {
	int16 type = readSelectorValue(s->_segMan, controlObject, SELECTOR(type));
	int16 style = readSelectorValue(s->_segMan, controlObject, SELECTOR(state));
	int16 x = readSelectorValue(s->_segMan, controlObject, SELECTOR(nsLeft));
	int16 y = readSelectorValue(s->_segMan, controlObject, SELECTOR(nsTop));
	GuiResourceId fontId = readSelectorValue(s->_segMan, controlObject, SELECTOR(font));
	reg_t textReference = readSelector(s->_segMan, controlObject, SELECTOR(text));
	Common::String text;
	Common::Rect rect;
	TextAlignment alignment;
	int16 mode, maxChars, cursorPos, upperPos, listCount, i;
	int16 upperOffset, cursorOffset;
	GuiResourceId viewId;
	int16 loopNo;
	int16 celNo;
	int16 priority;
	reg_t listSeeker;
	Common::String *listStrings = NULL;
	const char **listEntries = NULL;
	bool isAlias = false;

	rect = kControlCreateRect(x, y,
				readSelectorValue(s->_segMan, controlObject, SELECTOR(nsRight)),
				readSelectorValue(s->_segMan, controlObject, SELECTOR(nsBottom)));

	if (!textReference.isNull())
		text = s->_segMan->getString(textReference);

	switch (type) {
	case SCI_CONTROLS_TYPE_BUTTON:
		debugC(2, kDebugLevelGraphics, "drawing button %04x:%04x to %d,%d", PRINT_REG(controlObject), x, y);
		g_sci->_gfxControls->kernelDrawButton(rect, controlObject, g_sci->strSplit(text.c_str(), NULL).c_str(), fontId, style, hilite);
		return;

	case SCI_CONTROLS_TYPE_TEXT:
		alignment = readSelectorValue(s->_segMan, controlObject, SELECTOR(mode));
		debugC(2, kDebugLevelGraphics, "drawing text %04x:%04x ('%s') to %d,%d, mode=%d", PRINT_REG(controlObject), text.c_str(), x, y, alignment);
		g_sci->_gfxControls->kernelDrawText(rect, controlObject, g_sci->strSplit(text.c_str()).c_str(), fontId, alignment, style, hilite);
		return;

	case SCI_CONTROLS_TYPE_TEXTEDIT:
		mode = readSelectorValue(s->_segMan, controlObject, SELECTOR(mode));
		maxChars = readSelectorValue(s->_segMan, controlObject, SELECTOR(max));
		cursorPos = readSelectorValue(s->_segMan, controlObject, SELECTOR(cursor));
		if (cursorPos > (int)text.size()) {
			// if cursor is outside of text, adjust accordingly
			cursorPos = text.size();
			writeSelectorValue(s->_segMan, controlObject, SELECTOR(cursor), cursorPos);
		}
		debugC(2, kDebugLevelGraphics, "drawing edit control %04x:%04x (text %04x:%04x, '%s') to %d,%d", PRINT_REG(controlObject), PRINT_REG(textReference), text.c_str(), x, y);
		g_sci->_gfxControls->kernelDrawTextEdit(rect, controlObject, g_sci->strSplit(text.c_str(), NULL).c_str(), fontId, mode, style, cursorPos, maxChars, hilite);
		return;

	case SCI_CONTROLS_TYPE_ICON:
		viewId = readSelectorValue(s->_segMan, controlObject, SELECTOR(view));
		{
			int l = readSelectorValue(s->_segMan, controlObject, SELECTOR(loop));
			loopNo = (l & 0x80) ? l - 256 : l;
			int c = readSelectorValue(s->_segMan, controlObject, SELECTOR(cel));
			celNo = (c & 0x80) ? c - 256 : c;
			// Check if the control object specifies a priority selector (like in Jones)
			if (lookupSelector(s->_segMan, controlObject, SELECTOR(priority), NULL, NULL) == kSelectorVariable)
				priority = readSelectorValue(s->_segMan, controlObject, SELECTOR(priority));
			else
				priority = -1;
		}
		debugC(2, kDebugLevelGraphics, "drawing icon control %04x:%04x to %d,%d", PRINT_REG(controlObject), x, y - 1);
		g_sci->_gfxControls->kernelDrawIcon(rect, controlObject, viewId, loopNo, celNo, priority, style, hilite);
		return;

	case SCI_CONTROLS_TYPE_LIST:
	case SCI_CONTROLS_TYPE_LIST_ALIAS:
		if (type == SCI_CONTROLS_TYPE_LIST_ALIAS)
			isAlias = true;

		maxChars = readSelectorValue(s->_segMan, controlObject, SELECTOR(x)); // max chars per entry
		cursorOffset = readSelectorValue(s->_segMan, controlObject, SELECTOR(cursor));
		if (SELECTOR(topString) != -1) {
			// Games from early SCI1 onwards use topString
			upperOffset = readSelectorValue(s->_segMan, controlObject, SELECTOR(topString));
		} else {
			// Earlier games use lsTop or brTop
			if (lookupSelector(s->_segMan, controlObject, SELECTOR(brTop), NULL, NULL) == kSelectorVariable)
				upperOffset = readSelectorValue(s->_segMan, controlObject, SELECTOR(brTop));
			else
				upperOffset = readSelectorValue(s->_segMan, controlObject, SELECTOR(lsTop));
		}

		// Count string entries in NULL terminated string list
		listCount = 0; listSeeker = textReference;
		while (s->_segMan->strlen(listSeeker) > 0) {
			listCount++;
			listSeeker.offset += maxChars;
		}

		// TODO: This is rather convoluted... It would be a lot cleaner
		// if sciw_new_list_control would take a list of Common::String
		cursorPos = 0; upperPos = 0;
		if (listCount) {
			// We create a pointer-list to the different strings, we also find out whats upper and cursor position
			listSeeker = textReference;
			listEntries = (const char**)malloc(sizeof(char *) * listCount);
			listStrings = new Common::String[listCount];
			for (i = 0; i < listCount; i++) {
				listStrings[i] = s->_segMan->getString(listSeeker);
				listEntries[i] = listStrings[i].c_str();
				if (listSeeker.offset == upperOffset)
					upperPos = i;
				if (listSeeker.offset == cursorOffset)
					cursorPos = i;
				listSeeker.offset += maxChars;
			}
		}

		debugC(2, kDebugLevelGraphics, "drawing list control %04x:%04x to %d,%d, diff %d", PRINT_REG(controlObject), x, y, SCI_MAX_SAVENAME_LENGTH);
		g_sci->_gfxControls->kernelDrawList(rect, controlObject, maxChars, listCount, listEntries, fontId, style, upperPos, cursorPos, isAlias, hilite);
		free(listEntries);
		delete[] listStrings;
		return;

	case SCI_CONTROLS_TYPE_DUMMY:
		// Actually this here does nothing at all, its required by at least QfG1/EGA that we accept this type
		return;

	default:
		error("unsupported control type %d", type);
	}
}

reg_t kDrawControl(EngineState *s, int argc, reg_t *argv) {
	reg_t controlObject = argv[0];
	Common::String objName = s->_segMan->getObjectName(controlObject);

	// Disable the "Change Directory" button, as we don't allow the game engine to
	// change the directory where saved games are placed
	if (objName == "changeDirI") {
		int state = readSelectorValue(s->_segMan, controlObject, SELECTOR(state));
		writeSelectorValue(s->_segMan, controlObject, SELECTOR(state), (state | SCI_CONTROLS_STYLE_DISABLED) & ~SCI_CONTROLS_STYLE_ENABLED);
	}
	if (objName == "DEdit") {
		reg_t textReference = readSelector(s->_segMan, controlObject, SELECTOR(text));
		if (!textReference.isNull()) {
			Common::String text = s->_segMan->getString(textReference);
			if ((text == "a:hq1_hero.sav") || (text == "a:glory1.sav") || (text == "a:glory2.sav") || (text == "a:glory3.sav")) {
				// Remove "a:" from hero quest / quest for glory export default filenames
				text.deleteChar(0);
				text.deleteChar(0);
				s->_segMan->strcpy(textReference, text.c_str());
			}
		}
	}

	_k_GenericDrawControl(s, controlObject, false);
	return NULL_REG;
}

reg_t kHiliteControl(EngineState *s, int argc, reg_t *argv) {
	reg_t controlObject = argv[0];

	_k_GenericDrawControl(s, controlObject, true);
	return s->r_acc;
}

reg_t kEditControl(EngineState *s, int argc, reg_t *argv) {
	reg_t controlObject = argv[0];
	reg_t eventObject = argv[1];

	if (!controlObject.isNull()) {
		int16 controlType = readSelectorValue(s->_segMan, controlObject, SELECTOR(type));

		switch (controlType) {
		case SCI_CONTROLS_TYPE_TEXTEDIT:
			// Only process textedit controls in here
			g_sci->_gfxControls->kernelTexteditChange(controlObject, eventObject);
		}
	}
	return s->r_acc;
}

reg_t kAddToPic(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId;
	int16 loopNo;
	int16 celNo;
	int16 leftPos, topPos, priority, control;

	switch (argc) {
	// Is this ever really gets called with 0 parameters, we need to set _picNotValid!!
	//case 0:
	//	break;
	case 1:
		if (argv[0].isNull())
			return s->r_acc;
		g_sci->_gfxAnimate->kernelAddToPicList(argv[0], argc, argv);
		break;
	case 7:
		viewId = argv[0].toUint16();
		loopNo = argv[1].toSint16();
		celNo = argv[2].toSint16();
		leftPos = argv[3].toSint16();
		topPos = argv[4].toSint16();
		priority = argv[5].toSint16();
		control = argv[6].toSint16();
		g_sci->_gfxAnimate->kernelAddToPicView(viewId, loopNo, celNo, leftPos, topPos, priority, control);
		break;
	default:
		error("kAddToPic with unsupported parameter count %d", argc);
	}
	return s->r_acc;
}

reg_t kGetPort(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_gfxPorts->kernelGetActive();
}

reg_t kSetPort(EngineState *s, int argc, reg_t *argv) {
	uint16 portId;
	Common::Rect picRect;
	int16 picTop, picLeft;
	bool initPriorityBandsFlag = false;

	switch (argc) {
	case 1:
		portId = argv[0].toSint16();
		g_sci->_gfxPorts->kernelSetActive(portId);
		break;

	case 7:
		initPriorityBandsFlag = true;
	case 6:
		picRect.top = argv[0].toSint16();
		picRect.left = argv[1].toSint16();
		picRect.bottom = argv[2].toSint16();
		picRect.right = argv[3].toSint16();
		picTop = argv[4].toSint16();
		picLeft = argv[5].toSint16();
		g_sci->_gfxPorts->kernelSetPicWindow(picRect, picTop, picLeft, initPriorityBandsFlag);
		break;

	default:
		error("SetPort was called with %d parameters", argc);
		break;
	}
	return NULL_REG;
}

reg_t kDrawCel(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = argv[0].toSint16();
	int16 loopNo = argv[1].toSint16();
	int16 celNo = argv[2].toSint16();
	uint16 x = argv[3].toUint16();
	uint16 y = argv[4].toUint16();
	int16 priority = (argc > 5) ? argv[5].toSint16() : -1;
	uint16 paletteNo = (argc > 6) ? argv[6].toUint16() : 0;
	bool hiresMode = (argc > 7) ? true : false;
	reg_t upscaledHiresHandle = (argc > 7) ? argv[7] : NULL_REG;

	g_sci->_gfxPaint16->kernelDrawCel(viewId, loopNo, celNo, x, y, priority, paletteNo, hiresMode, upscaledHiresHandle);

	return s->r_acc;
}

reg_t kDisposeWindow(EngineState *s, int argc, reg_t *argv) {
	int windowId = argv[0].toSint16();
	bool reanimate = false;
	if ((argc != 2) || (argv[1].isNull()))
		reanimate = true;

	g_sci->_gfxPorts->kernelDisposeWindow(windowId, reanimate);
	return s->r_acc;
}

reg_t kNewWindow(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect1 (argv[1].toSint16(), argv[0].toSint16(), argv[3].toSint16(), argv[2].toSint16());
	Common::Rect rect2;
	int argextra = argc >= 13 ? 4 : 0; // Triggers in PQ3 and SCI1.1 games, argc 13 for DOS argc 15 for mac
	int	style = argv[5 + argextra].toSint16();
	int	priority = (argc > 6 + argextra) ? argv[6 + argextra].toSint16() : -1;
	int colorPen = (argc > 7 + argextra) ? argv[7 + argextra].toSint16() : 0;
	int colorBack = (argc > 8 + argextra) ? argv[8 + argextra].toSint16() : 255;

	//	const char *title = argv[4 + argextra].segment ? kernel_dereference_char_pointer(s, argv[4 + argextra], 0) : NULL;
	if (argc>=13) {
		rect2 = Common::Rect (argv[5].toSint16(), argv[4].toSint16(), argv[7].toSint16(), argv[6].toSint16());
	}

	Common::String title;
	if (argv[4 + argextra].segment) {
		title = s->_segMan->getString(argv[4 + argextra]);
		title = g_sci->strSplit(title.c_str(), NULL);
	}

	return g_sci->_gfxPorts->kernelNewWindow(rect1, rect2, style, priority, colorPen, colorBack, title.c_str());
}

reg_t kAnimate(EngineState *s, int argc, reg_t *argv) {
	reg_t castListReference = (argc > 0) ? argv[0] : NULL_REG;
	bool cycle = (argc > 1) ? ((argv[1].toUint16()) ? true : false) : false;

	g_sci->_gfxAnimate->kernelAnimate(castListReference, cycle, argc, argv);

	return s->r_acc;
}

reg_t kShakeScreen(EngineState *s, int argc, reg_t *argv) {
	int16 shakeCount = (argc > 0) ? argv[0].toUint16() : 1;
	int16 directions = (argc > 1) ? argv[1].toUint16() : 1;

	g_sci->_gfxPaint->kernelShakeScreen(shakeCount, directions);
	return s->r_acc;
}

reg_t kDisplay(EngineState *s, int argc, reg_t *argv) {
	reg_t textp = argv[0];
	int index = (argc > 1) ? argv[1].toUint16() : 0;

	Common::String text;

	if (textp.segment) {
		argc--; argv++;
		text = s->_segMan->getString(textp);
	} else {
		argc--; argc--; argv++; argv++;
		text = g_sci->getKernel()->lookupText(textp, index);
	}

	return g_sci->_gfxPaint16->kernelDisplay(g_sci->strSplit(text.c_str()).c_str(), argc, argv);
}

void playVideo(Graphics::VideoDecoder *videoDecoder) {
	if (!videoDecoder)
		return;

	byte *scaleBuffer = 0;
	uint16 width = videoDecoder->getWidth();
	uint16 height = videoDecoder->getHeight();
	uint16 screenWidth = g_system->getWidth();
	uint16 screenHeight = g_system->getHeight();

	if (screenWidth == 640 && width <= 320 && height <= 240) {
		assert(videoDecoder->getPixelFormat().bytesPerPixel == 1);
		width *= 2;
		height *= 2;
		scaleBuffer = new byte[width * height];
	}

	uint16 x = (screenWidth - width) / 2;
	uint16 y = (screenHeight - height) / 2;
	bool skipVideo = false;

	while (!g_engine->shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			Graphics::Surface *frame = videoDecoder->decodeNextFrame();
			if (frame) {
				if (scaleBuffer) {
					// TODO: Probably should do aspect ratio correction in e.g. GK1 Windows 
					g_sci->_gfxScreen->scale2x((byte *)frame->pixels, scaleBuffer, videoDecoder->getWidth(), videoDecoder->getHeight());
					g_system->copyRectToScreen(scaleBuffer, width, x, y, width, height);
				} else
					g_system->copyRectToScreen((byte *)frame->pixels, frame->pitch, x, y, width, height);

				if (videoDecoder->hasDirtyPalette())
					videoDecoder->setSystemPalette();

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}

	delete[] scaleBuffer;
	delete videoDecoder;
}

reg_t kShowMovie(EngineState *s, int argc, reg_t *argv) {
	// Hide the cursor if it's showing and then show it again if it was
	// previously visible.
	bool reshowCursor = g_sci->_gfxCursor->isVisible();
	if (reshowCursor)
		g_sci->_gfxCursor->kernelHide();

	uint16 screenWidth = g_system->getWidth();
	uint16 screenHeight = g_system->getHeight();
		
	Graphics::VideoDecoder *videoDecoder = 0;

	if (argv[0].segment != 0) {
		Common::String filename = s->_segMan->getString(argv[0]);

		if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
			// Mac QuickTime
			// The only argument is the string for the video

			// HACK: Switch to 16bpp graphics for Cinepak.
			initGraphics(screenWidth, screenHeight, screenWidth > 320, NULL);

			if (g_system->getScreenFormat().bytesPerPixel == 1) {
				error("This video requires >8bpp color to be displayed, but could not switch to RGB color mode.");
				return NULL_REG;
			}

			videoDecoder = new Graphics::QuickTimeDecoder();
			if (!videoDecoder->loadFile(filename))
				error("Could not open '%s'", filename.c_str());
		} else {
			// DOS SEQ
			// SEQ's are called with no subops, just the string and delay
			SeqDecoder *seqDecoder = new SeqDecoder();
			seqDecoder->setFrameDelay(argv[1].toUint16()); // Time between frames in ticks
			videoDecoder = seqDecoder;

			if (!videoDecoder->loadFile(filename)) {
				warning("Failed to open movie file %s", filename.c_str());
				delete videoDecoder;
				videoDecoder = 0;
			}
		}
	} else {
		// Windows AVI
		// TODO: This appears to be some sort of subop. case 0 contains the string
		// for the video, so we'll just play it from there for now.

#ifdef ENABLE_SCI32
		if (getSciVersion() >= SCI_VERSION_2_1) {
			// SCI2.1 always has argv[0] as 1, the rest of the arguments seem to
			// follow SCI1.1/2.
			if (argv[0].toUint16() != 1)
				error("SCI2.1 kShowMovie argv[0] not 1");
			argv++;
			argc--;
		}
#endif
		switch (argv[0].toUint16()) {
		case 0: {
			Common::String filename = s->_segMan->getString(argv[1]);
			videoDecoder = new Graphics::AviDecoder(g_system->getMixer());

			if (!videoDecoder->loadFile(filename.c_str())) {
				warning("Failed to open movie file %s", filename.c_str());
				delete videoDecoder;
				videoDecoder = 0;
			}
			break;
		}
		default:
			warning("Unhandled SCI kShowMovie subop %d", argv[1].toUint16());
		}
	}

	if (videoDecoder) {
		playVideo(videoDecoder);

		// HACK: Switch back to 8bpp if we played a QuickTime video.
		// We also won't be copying the screen to the SCI screen...
		if (g_system->getScreenFormat().bytesPerPixel != 1)
			initGraphics(screenWidth, screenHeight, screenWidth > 320);
		else {
			g_sci->_gfxScreen->kernelSyncWithFramebuffer();
			g_sci->_gfxPalette->kernelSyncScreenPalette();
		}
	}

	if (reshowCursor)
		g_sci->_gfxCursor->kernelShow();

	return s->r_acc;
}

#ifdef ENABLE_SCI32
reg_t kRobot(EngineState *s, int argc, reg_t *argv) {

	int16 subop = argv[0].toUint16();

	switch (subop) {
		case 0: { // init
			int id = argv[1].toUint16();
			reg_t obj = argv[2];
			int16 flag = argv[3].toSint16();
			int16 x = argv[4].toUint16();
			int16 y = argv[5].toUint16();
			warning("kRobot(init), id %d, obj %04x:%04x, flag %d, x=%d, y=%d", id, PRINT_REG(obj), flag, x, y);
			}
			break;
		case 1:	// LSL6 hires (startup)
			// TODO
			return NULL_REG;	// an integer is expected
		case 4: {	// start
				int id = argv[1].toUint16();
				warning("kRobot(start), id %d", id);
			}
			break;
		case 8: // sync
			//warning("kRobot(sync), obj %04x:%04x", PRINT_REG(argv[1]));
			break;
		default:
			warning("kRobot(%d)", subop);
			break;
	}

	return s->r_acc;
}

reg_t kPlayVMD(EngineState *s, int argc, reg_t *argv) {
	uint16 operation = argv[0].toUint16();
	Graphics::VideoDecoder *videoDecoder = 0;
	bool reshowCursor = g_sci->_gfxCursor->isVisible();
	Common::String fileName, warningMsg;

	switch (operation) {
	case 0:	// init
		// This is actually meant to init the video file, but we play it instead
		fileName = s->_segMan->derefString(argv[1]);
		// TODO: argv[2] (usually null). When it exists, it points to an "Event" object,
		// that holds no data initially (e.g. in the intro of Phantasmagoria 1 demo).
		// Perhaps it's meant for syncing
		if (argv[2] != NULL_REG)
			warning("kPlayVMD: third parameter isn't 0 (it's %04x:%04x - %s)", PRINT_REG(argv[2]), s->_segMan->getObjectName(argv[2]));

		videoDecoder = new VMDDecoder(g_system->getMixer());

		if (reshowCursor)
			g_sci->_gfxCursor->kernelHide();

		if (videoDecoder && videoDecoder->loadFile(fileName))
			playVideo(videoDecoder);

		if (reshowCursor)
			g_sci->_gfxCursor->kernelShow();
		break;
	case 1:
	{
		// Set VMD parameters. Called with a maximum of 6 parameters:
		//
		// x, y, flags, gammaBoost, gammaFirst, gammaLast
		//
		// Flags are as follows:
		// bit 0		doubled
		// bit 1		"drop frames"?
		// bit 2		insert black lines
		// bit 3		unknown
		// bit 4		gamma correction
		// bit 5		hold black frame
		// bit 6		hold last frame
		// bit 7		unknown
		// bit 8		stretch

		// gammaBoost boosts palette colors in the range gammaFirst to
		// gammaLast, but only if bit 4 in flags is set. Percent value such that
		// 0% = no amplification These three parameters are optional if bit 4 is
		// clear. Also note that the x, y parameters play subtle games if used
		// with subfx 21. The subtleness has to do with creation of temporary
		// planes and positioning relative to such planes.

		int flags = argv[3].offset;
		Common::String flagspec;

		if (argc > 3) {
			if (flags & 1)
				flagspec += "doubled ";
			if (flags & 2)
				flagspec += "dropframes ";
			if (flags & 4)
				flagspec += "blacklines ";
			if (flags & 8)
				flagspec += "bit3 ";
			if (flags & 16)
				flagspec += "gammaboost ";
			if (flags & 32)
				flagspec += "holdblack ";
			if (flags & 64)
				flagspec += "holdlast ";
			if (flags & 128)
				flagspec += "bit7 ";
			if (flags & 256)
				flagspec += "stretch";

			warning("VMDFlags: %s", flagspec.c_str());
		}

		warning("x, y: %d, %d", argv[1].offset, argv[2].offset);

		if (argc > 4 && flags & 16)
			warning("gammaBoost: %d%% between palette entries %d and %d", argv[4].offset, argv[5].offset, argv[6].offset);
		break;
	}
	case 6:
		// Play, perhaps? Or stop? This is the last call made, and takes no extra parameters
	case 14:
		// Takes an additional integer parameter (e.g. 3)
	case 16:
		// Takes an additional parameter, usually 0
	case 21:
		// Looks to be setting the video size and position. Called with 4 extra integer
		// parameters (e.g. 86, 41, 235, 106)
	default:
		warningMsg = "PlayVMD - unsupported subop. Params: " +
									Common::String::printf("%d", argc) + " (";

		for (int i = 0; i < argc; i++) {
			warningMsg +=  Common::String::printf("%04x:%04x", PRINT_REG(argv[i]));
			warningMsg += (i == argc - 1 ? ")" : ", ");
		}

		warning("%s", warningMsg.c_str());
		break;
	}

	return s->r_acc;
}

#endif

reg_t kSetVideoMode(EngineState *s, int argc, reg_t *argv) {
	// This call is used for KQ6's intro. It has one parameter, which is 1 when
	// the intro begins, and 0 when it ends. It is suspected that this is
	// actually a flag to enable video planar memory access, as the video
	// decoder in KQ6 is specifically written for the planar memory model.
	// Planar memory mode access was used for VGA "Mode X" (320x240 resolution,
	// although the intro in KQ6 is 320x200).
	// Refer to http://en.wikipedia.org/wiki/Mode_X

	//warning("STUB: SetVideoMode %d", argv[0].toUint16());
	return s->r_acc;
}

// New calls for SCI11. Using those is only needed when using text-codes so that
// one is able to change font and/or color multiple times during kDisplay and
// kDrawControl
reg_t kTextFonts(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxText16->kernelTextFonts(argc, argv);
	return s->r_acc;
}

reg_t kTextColors(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxText16->kernelTextColors(argc, argv);
	return s->r_acc;
}

} // End of namespace Sci
