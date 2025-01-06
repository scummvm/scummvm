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

#include "common/system.h"

#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/window.h"

namespace Director {

enum TransitionAlgo {
	kTransAlgoBlinds,
	kTransAlgoBoxy,
	kTransAlgoStrips,
	kTransAlgoCenterOut,
	kTransAlgoChecker,
	kTransAlgoCover,
	kTransAlgoDissolve,
	kTransAlgoEdgesIn,
	kTransAlgoPush,
	kTransAlgoRandomLines,
	kTransAlgoReveal,
	kTransAlgoWipe,
	kTransAlgoZoom
};

enum TransitionDirection {
	kTransDirNone,
	kTransDirHorizontal,
	kTransDirVertical,
	kTransDirBoth,
	kTransDirStepsH,
	kTransDirStepsV,
	kTransDirCheckers,
	kTransDirBlindsV,
	kTransDirBlindsH,
	kTransDirDissolve
};

enum {
	kNumStrips = 16,
	kNumChecks = 16,
	kNumBlinds = 12
};

#define TRANS(t,a,d) {t,#t,a,d}

// cap transition framerate to 60fps
#define MAX_STEPS(duration) ((duration)*60/1000)

struct {
	TransitionType type;
	const char *name;
	TransitionAlgo algo;
	TransitionDirection dir;
} static const transProps[] = {
	TRANS(kTransNone, 					kTransAlgoWipe,		kTransDirNone),
	TRANS(kTransWipeRight, 				kTransAlgoWipe,		kTransDirHorizontal),
	TRANS(kTransWipeLeft,				kTransAlgoWipe,		kTransDirHorizontal),
	TRANS(kTransWipeDown,				kTransAlgoWipe,		kTransDirVertical),
	TRANS(kTransWipeUp,					kTransAlgoWipe,		kTransDirVertical),
	TRANS(kTransCenterOutHorizontal, 	kTransAlgoCenterOut,kTransDirHorizontal),	// 5
	TRANS(kTransEdgesInHorizontal, 		kTransAlgoEdgesIn,	kTransDirHorizontal),
	TRANS(kTransCenterOutVertical,		kTransAlgoCenterOut,kTransDirVertical),
	TRANS(kTransEdgesInVertical,		kTransAlgoEdgesIn,	kTransDirVertical),
	TRANS(kTransCenterOutSquare,		kTransAlgoCenterOut,kTransDirBoth),
	TRANS(kTransEdgesInSquare,			kTransAlgoEdgesIn,	kTransDirBoth),			// 10
	TRANS(kTransPushLeft,				kTransAlgoPush,		kTransDirHorizontal),
	TRANS(kTransPushRight,				kTransAlgoPush,		kTransDirHorizontal),
	TRANS(kTransPushDown,				kTransAlgoPush,		kTransDirVertical),
	TRANS(kTransPushUp,					kTransAlgoPush,		kTransDirVertical),
	TRANS(kTransRevealUp,				kTransAlgoReveal,	kTransDirVertical),		// 15
	TRANS(kTransRevealUpRight,			kTransAlgoReveal,	kTransDirBoth),
	TRANS(kTransRevealRight,			kTransAlgoReveal,	kTransDirHorizontal),
	TRANS(kTransRevealDownRight,		kTransAlgoReveal,	kTransDirBoth),
	TRANS(kTransRevealDown,				kTransAlgoReveal,	kTransDirVertical),
	TRANS(kTransRevealDownLeft,			kTransAlgoReveal,	kTransDirBoth),			// 20
	TRANS(kTransRevealLeft,				kTransAlgoReveal,	kTransDirHorizontal),
	TRANS(kTransRevealUpLeft,			kTransAlgoReveal,	kTransDirBoth),
	TRANS(kTransDissolvePixelsFast,		kTransAlgoDissolve,	kTransDirDissolve),
	TRANS(kTransDissolveBoxyRects,		kTransAlgoDissolve,	kTransDirDissolve),
	TRANS(kTransDissolveBoxySquares,	kTransAlgoDissolve,	kTransDirDissolve),			// 25
	TRANS(kTransDissolvePatterns,		kTransAlgoDissolve,	kTransDirDissolve),
	TRANS(kTransRandomRows,				kTransAlgoDissolve,	kTransDirDissolve),
	TRANS(kTransRandomColumns,			kTransAlgoDissolve,	kTransDirDissolve),
	TRANS(kTransCoverDown,				kTransAlgoCover,	kTransDirVertical),
	TRANS(kTransCoverDownLeft,			kTransAlgoCover,	kTransDirBoth),			// 30
	TRANS(kTransCoverDownRight,			kTransAlgoCover,	kTransDirBoth),
	TRANS(kTransCoverLeft,				kTransAlgoCover,	kTransDirHorizontal),
	TRANS(kTransCoverRight,				kTransAlgoCover,	kTransDirHorizontal),
	TRANS(kTransCoverUp,				kTransAlgoCover,	kTransDirVertical),
	TRANS(kTransCoverUpLeft,			kTransAlgoCover,	kTransDirBoth),			// 35
	TRANS(kTransCoverUpRight,			kTransAlgoCover,	kTransDirBoth),
	TRANS(kTransVenetianBlind,			kTransAlgoBlinds,	kTransDirBlindsH),
	TRANS(kTransCheckerboard,			kTransAlgoChecker,	kTransDirCheckers),
	TRANS(kTransStripsBottomBuildLeft, 	kTransAlgoStrips,	kTransDirStepsV),
	TRANS(kTransStripsBottomBuildRight,	kTransAlgoStrips,	kTransDirStepsV),		// 40
	TRANS(kTransStripsLeftBuildDown, 	kTransAlgoStrips,	kTransDirStepsH),
	TRANS(kTransStripsLeftBuildUp,		kTransAlgoStrips,	kTransDirStepsH),
	TRANS(kTransStripsRightBuildDown,	kTransAlgoStrips,	kTransDirStepsH),
	TRANS(kTransStripsRightBuildUp,		kTransAlgoStrips,	kTransDirStepsH),
	TRANS(kTransStripsTopBuildLeft,		kTransAlgoStrips,	kTransDirStepsV),		// 45
	TRANS(kTransStripsTopBuildRight,	kTransAlgoStrips,	kTransDirStepsV),
	TRANS(kTransZoomOpen,				kTransAlgoZoom,		kTransDirBoth),
	TRANS(kTransZoomClose,				kTransAlgoZoom,		kTransDirBoth),
	TRANS(kTransVerticalBinds,			kTransAlgoBlinds,	kTransDirBlindsV),
	TRANS(kTransDissolveBitsFast,		kTransAlgoDissolve,	kTransDirDissolve),			// 50
	TRANS(kTransDissolvePixels,			kTransAlgoDissolve,	kTransDirDissolve),
	TRANS(kTransDissolveBits,			kTransAlgoDissolve,	kTransDirDissolve)
};

void Window::exitTransition(TransParams &t, Graphics::ManagedSurface *nextFrame, Common::Rect clipRect) {
	_composeSurface->blitFrom(*nextFrame, clipRect, Common::Point(clipRect.left, clipRect.top));
	stepTransition(t, t.steps);
}

void Window::stepTransition(TransParams &t, int step) {
	_contentIsDirty = true;

	if (t.sourcePal != t.targetPal) {
		for (int i = 0; i < 768; i++) {
			t.tempPal[i] = lerpByte(
				(i < t.sourcePalLength * 3 ? t.sourcePal[i] : 0),
				(i < t.targetPalLength * 3 ? t.targetPal[i] : 0),
				step, t.steps);
		}
		g_director->setPalette(t.tempPal, 256);
	}
	g_director->draw();
}

void Window::playTransition(uint frame, RenderMode mode, uint16 transDuration, uint8 transArea, uint8 transChunkSize, TransitionType transType, CastMemberID paletteId) {
	// Play a transition and return the number of subframes rendered
	TransParams t;

	if (transType < 1 || transType > ARRAYSIZE(transProps) - 1) {
		warning("playTransition(): transType is not in [1..%d]: %d", ARRAYSIZE(transProps) - 1, transType);
		return;
	}

	t.type = transType;
	t.duration = MAX<uint16>(250, transDuration); // When duration is < 1/4s, make it 1/4
	t.frame = frame;
	t.chunkSize = MAX<uint>(1, transChunkSize);
	t.area = MAX<uint>(0, transArea);

	debugC(2, kDebugImages, "Window::playTransition(): Playing transition %d", t.type);

	// If we requested fast transitions, speed everything up
	if (debugChannelSet(-1, kDebugFast))
		t.duration = 250;

	// Copy palette information
	t.sourcePal = g_director->getPalette();
	t.sourcePalLength = g_director->getPaletteColorCount();
	t.targetPal = g_director->getPalette();
	t.targetPalLength = g_director->getPaletteColorCount();

	if (!paletteId.isNull()) {
		PaletteV4 *target = g_director->getPalette(paletteId);
		if (target) {
			t.targetPal = target->palette;
			t.targetPalLength = target->length;
		}
	}

	// Cache a copy of the frame before the transition.
	Graphics::ManagedSurface currentFrame(Graphics::ManagedSurface(_composeSurface->w, _composeSurface->h, g_director->_pixelformat));
	currentFrame.copyFrom(*_composeSurface);

	// If a transition is being played, render the frame after the transition.
	Graphics::ManagedSurface nextFrame(Graphics::ManagedSurface(_composeSurface->w, _composeSurface->h, g_director->_pixelformat));

	Common::Rect clipRect;
	Score *score = g_director->getCurrentMovie()->getScore();
	if (t.area) {
		// Changed area transition
		score->updateSprites(mode);

		if (_dirtyRects.size() == 0)
			return;

		clipRect = *_dirtyRects.begin();

		for (auto &i : _dirtyRects)
			clipRect.extend(i);

		// Ensure we redraw any other sprites intersecting the non-clip area.
		_dirtyRects.clear();

		// Some transitions depend upon an even clipRect size
		if (clipRect.width() % 2 == 1)
			clipRect.right += 1;

		if (clipRect.height() % 2 == 1)
			clipRect.bottom += 1;

		clipRect.clip(Common::Rect(_innerDims.width(), _innerDims.height()));
		_dirtyRects.push_back(clipRect);

		render(false, &nextFrame);
	} else {
		// Full stage transition
		score->updateSprites(mode);
		render(true, &nextFrame);

		clipRect = _innerDims;
		clipRect.moveTo(0, 0);
	}

	Common::Rect rfrom, rto;

	initTransParams(t, clipRect);

	Graphics::ManagedSurface *blitFrom;
	bool fullredraw = false;

	uint32 transStartTime = g_system->getMillis();

	debugC(2, kDebugImages, "Window::playTransition(): type: %d, duration: %d, area: %d, chunkSize: %d, steps: %d, stepDuration: %d, xpos: %d, ypos: %d, xStepSize: %d, yStepSize: %d, stripSize: %d, clipRect: %d %d %d %d", t.type, t.duration, t.area, t.chunkSize, t.steps, t.stepDuration, t.xpos, t.ypos, t.xStepSize, t.yStepSize, t.stripSize, clipRect.left, clipRect.top, clipRect.right, clipRect.bottom);

	switch (transProps[t.type].algo) {
	case kTransAlgoDissolve:
		if (t.type == kTransDissolvePatterns)
			dissolvePatternsTrans(t, clipRect, &nextFrame);
		else
			dissolveTrans(t, clipRect, &nextFrame);
		debugC(2, kDebugImages, "Window::playTransition(): Transition %d finished in %d ms", t.type, g_system->getMillis() - transStartTime);
		return;

	case kTransAlgoChecker:
	case kTransAlgoStrips:
	case kTransAlgoBlinds:
		transMultiPass(t, clipRect, &nextFrame);
		debugC(2, kDebugImages, "Window::playTransition(): Transition %d finished in %d ms", t.type, g_system->getMillis() - transStartTime);
		return;

	case kTransAlgoZoom:
		transZoom(t, clipRect, &currentFrame, &nextFrame);
		debugC(2, kDebugImages, "Window::playTransition(): Transition %d finished in %d ms", t.type, g_system->getMillis() - transStartTime);
		return;

	case kTransAlgoCenterOut:
	case kTransAlgoCover:
	case kTransAlgoWipe:
		blitFrom = &nextFrame;
		break;

 	case kTransAlgoEdgesIn:
	case kTransAlgoReveal:
	case kTransAlgoPush:
		blitFrom = &currentFrame;
		fullredraw = true;
		break;

	default:
		blitFrom = &nextFrame;
		break;
	}

	uint w = clipRect.width();
	uint h = clipRect.height();

	for (uint16 i = 1; i < t.steps + 1; i++) {
		uint32 startTime = g_system->getMillis();
		bool stop = false;
		rto = clipRect;
		rfrom = clipRect;

		if (transProps[t.type].algo == kTransAlgoReveal ||
 				transProps[t.type].algo == kTransAlgoEdgesIn) {
			_composeSurface->copyRectToSurface(nextFrame, clipRect.left, clipRect.top, clipRect);
		}

		switch (t.type) {
		case kTransWipeRight:								// 1
			rto.setWidth(MAX((int16)0, (int16)(t.xpos + t.xStepSize * i)));
			rfrom = rto;
			break;

		case kTransWipeLeft:								// 2
			rto.setWidth(MAX((int16)0, (int16)(t.xpos + t.xStepSize * i)));
			rto.translate(w - t.xpos - t.xStepSize * i, 0);
			rfrom = rto;
			break;

		case kTransWipeDown:								// 3
			rto.setHeight(MAX((int16)0, (int16)(t.ypos + t.yStepSize * i)));
			rfrom = rto;
			break;

		case kTransWipeUp:									// 4
			rto.setHeight(MAX((int16)0, (int16)(t.ypos + t.yStepSize * i)));
			rto.translate(0, h - t.ypos - t.yStepSize * i);
			rfrom = rto;
			break;

		case kTransCenterOutHorizontal:						// 5
			t.xpos += t.xStepSize;
			rto.setWidth(MAX((int16)0, (int16)(t.xpos * 2)));
			rto.translate(w / 2 - t.xpos, 0);
			rfrom = rto;
			break;

		case kTransEdgesInHorizontal:						// 6
			rto.setWidth(MAX((int16)0, (int16)(w - t.xStepSize * i * 2)));
			rto.translate(t.xStepSize * i, 0);
			rfrom = rto;
			break;

		case kTransCenterOutVertical:						// 7
			t.ypos += t.yStepSize;
			rto.setHeight(MAX((int16)0, (int16)(t.ypos * 2)));
			rto.translate(0, h / 2 - t.ypos);
			rfrom = rto;
			break;

		case kTransEdgesInVertical:							// 8
			rto.setHeight(MAX((int16)0, (int16)(h - t.yStepSize * i * 2)));
			rto.translate(0, t.yStepSize * i);
			rfrom = rto;
			break;

		case kTransCenterOutSquare: 						// 9
			t.ypos += t.yStepSize;
			rto.setHeight(MAX((int16)0, (int16)(t.ypos * 2)));
			t.xpos += t.xStepSize;
			rto.setWidth(MAX((int16)0, (int16)(t.xpos * 2)));
			rto.translate(w / 2 - t.xpos, h / 2 - t.ypos);
			rfrom = rto;
			break;

		case kTransEdgesInSquare:							// 10
			rto.setHeight(MAX((int16)0, (int16)(h - t.yStepSize * i * 2)));
			rto.setWidth(MAX((int16)0, (int16)(w - t.xStepSize * i * 2)));
			rto.moveTo(t.xStepSize * i, t.yStepSize * i);
			rfrom = rto;
			break;

		case kTransPushLeft:								// 11
			rto.translate(w - t.xStepSize * i, 0);
			rfrom.right -= w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			_composeSurface->blitFrom(nextFrame, rfrom, Common::Point(rto.left, rto.top));

			rfrom.translate(t.xStepSize * i, 0);
			rfrom.setWidth(MAX((int16)0, (int16)(w - t.xStepSize * i)));
			rto.moveTo(clipRect.left, clipRect.top);
			break;

		case kTransPushRight:								// 12
			rfrom.translate(w - t.xStepSize * i, 0);
			rfrom.setWidth(MAX((int16)0, (int16)(t.xStepSize * i)));
			_composeSurface->blitFrom(nextFrame, rfrom, Common::Point(rto.left, rto.top));

			rto.setWidth(MAX((int16)0, (int16)(w - t.xStepSize * i)));
			rto.translate(t.xStepSize * i, 0);
			rfrom.moveTo(clipRect.left, clipRect.top);
			rfrom.setWidth(MAX((int16)0, (int16)(w - t.xStepSize * i)));
			break;

		case kTransPushDown:								// 13
			rfrom.translate(0, h - t.yStepSize * i);
			rfrom.setHeight(MAX((int16)0, (int16)(t.yStepSize * i)));
			_composeSurface->blitFrom(nextFrame, rfrom, Common::Point(rto.left, rto.top));

			rto.setHeight(MAX((int16)0, (int16)(h - t.yStepSize * i)));
			rto.translate(0, t.yStepSize * i);
			rfrom.moveTo(clipRect.left, clipRect.top);
			rfrom.setHeight(MAX((int16)0, (int16)(h - t.yStepSize * i)));
			break;

		case kTransPushUp:									// 14
			rto.translate(0, h - t.yStepSize * i);
			rfrom.bottom -= h - clipRect.findIntersectingRect(rto).height();
			rto.clip(clipRect);
			_composeSurface->blitFrom(nextFrame, rfrom, Common::Point(rto.left, rto.top));

			rfrom.translate(0, t.yStepSize * i);
			rfrom.setHeight(MAX((int16)0, (int16)(h - t.yStepSize * i)));
			rto.moveTo(clipRect.left, clipRect.top);
			break;

		case kTransRevealUp:								// 15
			rto.translate(0, -t.yStepSize * i);
			rfrom.top += h - clipRect.findIntersectingRect(rto).height();
			rto.clip(clipRect);
			break;

		case kTransRevealUpRight:							// 16
			rto.translate(t.xStepSize * i, -t.yStepSize * i);
			rfrom.top += h - clipRect.findIntersectingRect(rto).height();
			rfrom.right -= w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransRevealRight:								// 17
			rto.translate(t.xStepSize * i, 0);
			rfrom.right -= w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransRevealDownRight:							// 18
			rto.translate(t.xStepSize * i, t.yStepSize * i);
			rfrom.bottom -= h - clipRect.findIntersectingRect(rto).height();
			rfrom.right -= w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransRevealDown:								// 19
			rto.translate(0, t.yStepSize * i);
			rfrom.bottom -= h - clipRect.findIntersectingRect(rto).height();
			rto.clip(clipRect);
			break;

		case kTransRevealDownLeft:							// 20
			rto.translate(-t.xStepSize * i, t.yStepSize * i);
			rfrom.bottom -= h - clipRect.findIntersectingRect(rto).height();
			rfrom.left += w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransRevealLeft:								// 21
			rto.translate(-t.xStepSize * i, 0);
			rfrom.left += w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransRevealUpLeft:							// 22
			rto.moveTo(-t.xStepSize * i, -t.yStepSize * i);
			rfrom.top += h - clipRect.findIntersectingRect(rto).height();
			rfrom.left += w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransDissolvePixelsFast:						// 23
		case kTransDissolveBoxyRects:						// 24
		case kTransDissolveBoxySquares:						// 25
		case kTransDissolvePatterns:						// 26
		case kTransRandomRows:								// 27
		case kTransRandomColumns:							// 28
			// Dissolve
			break;

		case kTransCoverDown:								// 29
			rto.setHeight(h);
			rto.translate(0, t.yStepSize * i - h);
			rfrom.top += h - clipRect.findIntersectingRect(rto).height();
			rto.clip(clipRect);
			break;

		case kTransCoverDownLeft:							// 30
			rto.translate(w - t.xStepSize * i, t.yStepSize * i - h);
			rfrom.top += h - clipRect.findIntersectingRect(rto).height();
			rfrom.right -= w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransCoverDownRight:							// 31
			rto.translate(t.xStepSize * i - w, t.yStepSize * i - h);
			rfrom.top += h - clipRect.findIntersectingRect(rto).height();
			rfrom.left += w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransCoverLeft:								// 32
			rto.translate(w - t.xStepSize * i, 0);
			rfrom.right -= w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransCoverRight:								// 33
			rto.translate(t.xStepSize * i - w, 0);
			rfrom.left += w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransCoverUp:									// 34
			rto.translate(0, h - t.yStepSize * i);
			rfrom.bottom -= h - clipRect.findIntersectingRect(rto).height();
			rto.clip(clipRect);
			break;

		case kTransCoverUpLeft:								// 35
			rto.translate(w - t.xStepSize * i, h - t.yStepSize * i);
			rfrom.bottom -= h - clipRect.findIntersectingRect(rto).height();
			rfrom.right -= w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransCoverUpRight:							// 36
			rto.translate(t.xStepSize * i - w, h - t.yStepSize * i);
			rfrom.bottom -= h - clipRect.findIntersectingRect(rto).height();
			rfrom.right -= w - clipRect.findIntersectingRect(rto).width();
			rto.clip(clipRect);
			break;

		case kTransVenetianBlind:							// 37
		case kTransCheckerboard:							// 38
		case kTransStripsBottomBuildLeft:					// 39
		case kTransStripsBottomBuildRight:					// 40
		case kTransStripsLeftBuildDown:						// 41
		case kTransStripsLeftBuildUp:						// 42
		case kTransStripsRightBuildDown:					// 43
		case kTransStripsRightBuildUp:						// 44
		case kTransStripsTopBuildLeft:						// 45
		case kTransStripsTopBuildRight:						// 46
			// Multipass
			break;

		case kTransZoomOpen:								// 47
		case kTransZoomClose:								// 48
			// Zoom
			break;

		case kTransVerticalBinds:							// 49
			// Multipass
			break;

		case kTransDissolveBitsFast:						// 50
		case kTransDissolvePixels:							// 51
		case kTransDissolveBits:							// 52
			// Dissolve
			break;

		default:
			warning("Window::playTransition(): Unhandled transition type %s %d %d", transProps[t.type].name, t.duration, t.chunkSize);
			// fallthrough

		case kTransNone:
			stop = true;
			break;
		}

		if (stop)
			break;

		_composeSurface->blitFrom(*blitFrom, rfrom, Common::Point(rto.left, rto.top));

		if (_vm->processEvents(true)) {
			exitTransition(t, &nextFrame, clipRect);
			break;
		}

		if (fullredraw) {
			stepTransition(t, i);
		} else {
			rto.clip(clipRect);

			if (rto.height() > 0 && rto.width() > 0)
				stepTransition(t, i);
		}

		uint32 endTime = g_system->getMillis();
		int diff = MAX(0, (int)t.stepDuration - (int)(endTime - startTime));
		debugC(6, kDebugImages, "Window::playTransition(): delaying for %d", diff);
		g_director->delayMillis(diff);

		g_lingo->executePerFrameHook(t.frame, i);
	}

	debugC(2, kDebugImages, "Window::playTransition(): Transition %d finished in %d ms", t.type, g_system->getMillis() - transStartTime);
}

static int getLog2(int n) {
	int res;

	for (res = 0; n != 0; res++)
		n >>= 1;

	return res;
}

static uint32 randomSeed[33] = {
	0x00000000UL,
	0x00000000UL, 0x00000003UL, 0x00000006UL, 0x0000000cUL,
	0x00000014UL, 0x00000030UL, 0x00000060UL, 0x000000b8UL,
	0x00000110UL, 0x00000240UL, 0x00000500UL, 0x00000ca0UL,
	0x00001b00UL, 0x00003500UL, 0x00006000UL, 0x0000b400UL,
	0x00012000UL, 0x00020400UL, 0x00072000UL, 0x00090000UL,
	0x00140000UL, 0x00300000UL, 0x00420000UL, 0x00d80000UL,
	0x01200000UL, 0x03880000UL, 0x07200000UL, 0x09000000UL,
	0x14000000UL, 0x32800000UL, 0x48000000UL, 0xa3000000UL
};

void Window::dissolveTrans(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *nextFrame) {
	uint w = clipRect.width();
	uint h = clipRect.height();
	uint realw = w, realh = h;
	byte pixmask[8];

	memset(pixmask, 0, 8);

	t.xStepSize = 1;
	t.yStepSize = 1;

	switch (t.type) {
	case kTransDissolveBitsFast:
	case kTransDissolveBits:
		if (t.chunkSize >= 32) {
			w = (w + 3) >> 2;
			t.xStepSize = 4;
		} else if (t.chunkSize >= 16) {
			w = (w + 1) >> 1;
			t.xStepSize = 2;
		} else if (t.chunkSize >= 8) {
			t.xStepSize = 1;
		} else if (t.chunkSize >= 4) {
			w <<= 1;
			t.xStepSize = -2;
			pixmask[0] = 0x0f;
			pixmask[1] = 0xf0;
		} else if (t.chunkSize >= 2) {
			w <<= 2;
			t.xStepSize = -4;

			for (int i = 0; i < 4; i++)
				pixmask[i] = 0x3 << (i * 2);
		} else {
			w <<= 3;
			t.xStepSize = -8;

			for (int i = 0; i < 8; i++)
				pixmask[i] = 1 << i;
		}
		break;

	case kTransRandomRows:
		t.xStepSize = realw;
		t.yStepSize = t.chunkSize;
		w = 1;
		h = (h + t.chunkSize - 1) / t.chunkSize;
		break;

	case kTransRandomColumns:
		t.xStepSize = t.chunkSize;
		t.yStepSize = realh;
		w = (w + t.chunkSize - 1) / t.chunkSize;
		h = 1;
		break;

	case kTransDissolveBoxySquares:
		t.xStepSize = t.chunkSize;
		t.yStepSize = t.chunkSize;
		w = (w + t.chunkSize - 1) / t.chunkSize;
		h = (h + t.chunkSize - 1) / t.chunkSize;
		break;

	case kTransDissolveBoxyRects:
		if (w > h) {
			t.xStepSize = MAX(w * t.chunkSize / h, (uint)1);
			t.yStepSize = t.chunkSize;
		} else {
			t.xStepSize = t.chunkSize;
			t.yStepSize = MAX(h * t.chunkSize / w, (uint)1);
		}

		w = (w + t.xStepSize - 1) / t.xStepSize;
		h = (h + t.yStepSize - 1) / t.yStepSize;
		break;

	default:
		break;
	}

	int vBits = getLog2(w);
	int hBits = getLog2(h);
	uint32 rnd, seed;

	if (hBits <= 0 || vBits <= 0)
		return;

	rnd = seed = randomSeed[hBits + vBits];
	int hMask = (1L << hBits) - 1;
	int vShift = hBits;

	// Calculate steps
	uint32 pixPerStepInit = 1;
	int bitSteps = (1 << (hBits + vBits)) - 1;

	while (bitSteps > 64) {
		pixPerStepInit <<= 1;
		bitSteps >>= 1;
	}
	bitSteps++;

	Common::Rect r(MAX(1, t.xStepSize), t.yStepSize);

	int bitIndex = -1;
	for (int i = 0; i < t.steps; i++) {
		uint32 startTime = g_system->getMillis();
		int bitEndIndex = (bitSteps - 1) * (i + 1) / t.steps;

		while (bitIndex < bitEndIndex) {
			bitIndex++;
			uint32 pixPerStep = pixPerStepInit;
			do {
				uint32 x = (rnd - 1) >> vShift;
				uint32 y = (rnd - 1) & hMask;
				byte mask = 0;

				r.setWidth(MAX(1, t.xStepSize));
				r.setHeight(t.yStepSize);

				if (x < w && y < h) {
					if (t.xStepSize >= 1) {
						x = x * t.xStepSize;
						y = y * t.yStepSize;

						if (x < realw && y < realh) {
							x += clipRect.left;
							y += clipRect.top;
							r.moveTo(x, y);
							r.clip(clipRect);

							if (!r.isEmpty())
								_composeSurface->copyRectToSurface(*nextFrame, x, y, r);
						}
					} else {
						mask = pixmask[x % -t.xStepSize];
						x = x / -t.xStepSize;

						x += clipRect.left;
						y += clipRect.top;

						if (g_director->_pixelformat.bytesPerPixel == 1) {
							byte *dst = (byte *)_composeSurface->getBasePtr(x, y);
							byte *src = (byte *)nextFrame->getBasePtr(x, y);

							*dst = ((*dst & ~mask) | (*src & mask)) & 0xff;
						} else {
							uint32 *dst = (uint32 *)_composeSurface->getBasePtr(x, y);
							uint32 *src = (uint32 *)nextFrame->getBasePtr(x, y);

							*dst = ((*dst & ~mask) | (*src & mask)) & 0xff;

						}
					}
				}

				rnd = (rnd & 1) ? (rnd >> 1) ^ seed : rnd >> 1;

				if (pixPerStep > 0) {
					if (--pixPerStep == 0) {
						break;
					}
				}
			} while (rnd != seed);
		}
		stepTransition(t, i);

		g_lingo->executePerFrameHook(t.frame, i + 1);

		if (_vm->processEvents(true)) {
			exitTransition(t, nextFrame, clipRect);
			break;
		}

		uint32 endTime = g_system->getMillis();
		int diff = MAX(0, (int)t.stepDuration - (int)(endTime - startTime));
		debugC(6, kDebugImages, "Window::dissolveTrans(): delaying for %d", diff);
		g_director->delayMillis(diff);
	}
}

static byte dissolvePatterns[][8] = {
	{ 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x80, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 },
	{ 0x88, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 },
	{ 0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00 },
	{ 0x88, 0x00, 0x20, 0x00, 0x88, 0x00, 0x00, 0x00 },
	{ 0x88, 0x00, 0x20, 0x00, 0x88, 0x00, 0x02, 0x00 },
	{ 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x02, 0x00 },
	{ 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 },
	{ 0xa8, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 },
	{ 0xa8, 0x00, 0x22, 0x00, 0x8a, 0x00, 0x22, 0x00 },
	{ 0xaa, 0x00, 0x22, 0x00, 0x8a, 0x00, 0x22, 0x00 },
	{ 0xaa, 0x00, 0x22, 0x00, 0xaa, 0x00, 0x22, 0x00 },
	{ 0xaa, 0x00, 0xa2, 0x00, 0xaa, 0x00, 0x22, 0x00 },
	{ 0xaa, 0x00, 0xa2, 0x00, 0xaa, 0x00, 0x2a, 0x00 },
	{ 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00, 0x2a, 0x00 },
	{ 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00 },
	{ 0xaa, 0x40, 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00 },
	{ 0xaa, 0x40, 0xaa, 0x00, 0xaa, 0x04, 0xaa, 0x00 },
	{ 0xaa, 0x44, 0xaa, 0x00, 0xaa, 0x04, 0xaa, 0x00 },
	{ 0xaa, 0x44, 0xaa, 0x00, 0xaa, 0x44, 0xaa, 0x00 },
	{ 0xaa, 0x44, 0xaa, 0x10, 0xaa, 0x44, 0xaa, 0x00 },
	{ 0xaa, 0x44, 0xaa, 0x10, 0xaa, 0x44, 0xaa, 0x01 },
	{ 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x01 },
	{ 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 },
	{ 0xaa, 0x54, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 },
	{ 0xaa, 0x54, 0xaa, 0x11, 0xaa, 0x45, 0xaa, 0x11 },
	{ 0xaa, 0x55, 0xaa, 0x11, 0xaa, 0x45, 0xaa, 0x11 },
	{ 0xaa, 0x55, 0xaa, 0x11, 0xaa, 0x55, 0xaa, 0x11 },
	{ 0xaa, 0x55, 0xaa, 0x51, 0xaa, 0x55, 0xaa, 0x11 },
	{ 0xaa, 0x55, 0xaa, 0x51, 0xaa, 0x55, 0xaa, 0x15 },
	{ 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x15 },
	{ 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 },
	{ 0xea, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 },
	{ 0xea, 0x55, 0xaa, 0x55, 0xae, 0x55, 0xaa, 0x55 },
	{ 0xee, 0x55, 0xaa, 0x55, 0xae, 0x55, 0xaa, 0x55 },
	{ 0xee, 0x55, 0xaa, 0x55, 0xee, 0x55, 0xaa, 0x55 },
	{ 0xee, 0x55, 0xba, 0x55, 0xee, 0x55, 0xaa, 0x55 },
	{ 0xee, 0x55, 0xba, 0x55, 0xee, 0x55, 0xab, 0x55 },
	{ 0xee, 0x55, 0xbb, 0x55, 0xee, 0x55, 0xab, 0x55 },
	{ 0xee, 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55 },
	{ 0xfe, 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55 },
	{ 0xfe, 0x55, 0xbb, 0x55, 0xef, 0x55, 0xbb, 0x55 },
	{ 0xff, 0x55, 0xbb, 0x55, 0xef, 0x55, 0xbb, 0x55 },
	{ 0xff, 0x55, 0xbb, 0x55, 0xff, 0x55, 0xbb, 0x55 },
	{ 0xff, 0x55, 0xfb, 0x55, 0xff, 0x55, 0xbb, 0x55 },
	{ 0xff, 0x55, 0xfb, 0x55, 0xff, 0x55, 0xbf, 0x55 },
	{ 0xff, 0x55, 0xff, 0x55, 0xff, 0x55, 0xbf, 0x55 },
	{ 0xff, 0x55, 0xff, 0x55, 0xff, 0x55, 0xff, 0x55 },
	{ 0xff, 0xd5, 0xff, 0x55, 0xff, 0x55, 0xff, 0x55 },
	{ 0xff, 0xd5, 0xff, 0x55, 0xff, 0x5d, 0xff, 0x55 },
	{ 0xff, 0xdd, 0xff, 0x55, 0xff, 0x5d, 0xff, 0x55 },
	{ 0xff, 0xdd, 0xff, 0x55, 0xff, 0xdd, 0xff, 0x55 },
	{ 0xff, 0xdd, 0xff, 0x75, 0xff, 0xdd, 0xff, 0x55 },
	{ 0xff, 0xdd, 0xff, 0x75, 0xff, 0xdd, 0xff, 0x57 },
	{ 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff, 0x57 },
	{ 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff, 0x77 },
	{ 0xff, 0xfd, 0xff, 0x77, 0xff, 0xdd, 0xff, 0x77 },
	{ 0xff, 0xfd, 0xff, 0x77, 0xff, 0xdf, 0xff, 0x77 },
	{ 0xff, 0xff, 0xff, 0x77, 0xff, 0xdf, 0xff, 0x77 },
	{ 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff, 0x77 },
	{ 0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0x77 },
	{ 0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0x7f },
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f },
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
};

void Window::dissolvePatternsTrans(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *nextFrame) {
	int patternSteps = 64;

	for (int i = 0; i < t.steps; i++) {
		int patternIndex = (patternSteps - 1) * (i + 1) / t.steps;
		uint32 startTime = g_system->getMillis();
		for (int y = clipRect.top; y < clipRect.bottom; y++) {
			byte pat = dissolvePatterns[patternIndex][y % 8];
			if (g_director->_pixelformat.bytesPerPixel == 1) {

				byte *dst = (byte *)_composeSurface->getBasePtr(clipRect.left, y);
				byte *src = (byte *)nextFrame->getBasePtr(clipRect.left, y);

				for (int x = clipRect.left; x < clipRect.right;) {
					byte mask = 0x80;
					for (int b = 0; b < 8 && x < clipRect.right; b++, x++) {
						if (pat & mask)
							*dst = *src;

						dst++;
						src++;
						mask >>= 1;
					}
				}
			} else {
				uint32 *dst = (uint32 *)_composeSurface->getBasePtr(clipRect.left, y);
				uint32 *src = (uint32 *)nextFrame->getBasePtr(clipRect.left, y);

				for (int x = clipRect.left; x < clipRect.right;) {
					byte mask = 0x80;
					for (int b = 0; b < 8 && x < clipRect.right; b++, x++) {
						if (pat & mask)
							*dst = *src;

						dst++;
						src++;
						mask >>= 1;
					}
				}
			}
		}

		stepTransition(t, i);

		g_lingo->executePerFrameHook(t.frame, i + 1);

		if (_vm->processEvents(true)) {
			exitTransition(t, nextFrame, clipRect);
			break;
		}

		uint32 endTime = g_system->getMillis();
		int diff = MAX(0, (int)t.stepDuration - (int)(endTime - startTime));
		debugC(6, kDebugImages, "Window::dissolvePatternsTrans(): delaying for %d", diff);
		g_director->delayMillis(diff);
	}
}

void Window::transMultiPass(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *nextFrame) {
	Common::Rect rto;
	uint w = clipRect.width();
	uint h = clipRect.height();
	bool flag = false;

	Common::Array<Common::Rect> rects;
	rects.reserve(128);

	for (uint16 i = 0; i < t.steps; i++) {
		uint32 startTime = g_system->getMillis();

		bool stop = false;
		rto = clipRect;

		switch (t.type) {
		case kTransVenetianBlind:							// 37
			rto.setHeight(t.yStepSize * (i + 1));
			for (int r = 0; r < kNumBlinds; r++) {
				rto.moveTo(0, r * t.stripSize);
				rects.push_back(rto);
			}
			break;

		case kTransCheckerboard:							// 38
			rto.setWidth(t.stripSize);
			rto.setHeight((i % ((t.steps + 1) / 2)) * t.chunkSize);

			flag = i + i > t.steps;

			for (int y = 0; y < t.yStepSize; y++) {
				for (int x = 0; x < t.xStepSize; x++) {
					if ((x & 2) ^ (y & 2) ^ (int)flag) {
						rto.moveTo(x * t.stripSize, y * t.stripSize);
						rects.push_back(rto);
					}
				}
			}
			break;

		case kTransStripsBottomBuildLeft:					// 39
			for (int r = 0; r < kNumStrips; r++) {
				int len = t.yStepSize * i - (kNumStrips - r - 1) * t.stripSize;
				if (len > 0) {
					rto.setHeight(len);
					rto.setWidth(t.xStepSize);
					rto.moveTo(t.xStepSize * r, h - len);
					rects.push_back(rto);
				}
			}
			break;

		case kTransStripsBottomBuildRight:					// 40
			for (int r = 0; r < kNumStrips; r++) {
				int len = t.yStepSize * i - r * t.stripSize;
				if (len > 0) {
					rto.setHeight(len);
					rto.setWidth(t.xStepSize);
					rto.moveTo(t.xStepSize * r, h - len);
					rects.push_back(rto);
				}
			}
			break;

		case kTransStripsLeftBuildDown:						// 41
			for (int r = 0; r < kNumStrips; r++) {
				int len = t.xStepSize * i - r * t.stripSize;
				if (len > 0) {
					rto.setWidth(len);
					rto.setHeight(t.yStepSize);
					rto.moveTo(0, t.yStepSize * r);
					rects.push_back(rto);
				}
			}
			break;

		case kTransStripsLeftBuildUp:						// 42
			for (int r = 0; r < kNumStrips; r++) {
				int len = t.xStepSize * i - (kNumStrips - r - 1) * t.stripSize;
				if (len > 0) {
					rto.setWidth(len);
					rto.setHeight(t.yStepSize);
					rto.moveTo(0, t.yStepSize * r);
					rects.push_back(rto);
				}
			}
			break;

		case kTransStripsRightBuildDown:					// 43
			for (int r = 0; r < kNumStrips; r++) {
				int len = t.xStepSize * i - r * t.stripSize;
				if (len > 0) {
					rto.setWidth(len);
					rto.setHeight(t.yStepSize);
					rto.moveTo(w - len, t.yStepSize * r);
					rects.push_back(rto);
				}
			}
			break;

		case kTransStripsRightBuildUp:						// 44
			for (int r = 0; r < kNumStrips; r++) {
				int len = t.xStepSize * i - (kNumStrips - r - 1) * t.stripSize;
				if (len > 0) {
					rto.setWidth(len);
					rto.setHeight(t.yStepSize);
					rto.moveTo(w - len, t.yStepSize * r);
					rects.push_back(rto);
				}
			}
			break;

		case kTransStripsTopBuildLeft:						// 45
			for (int r = 0; r < kNumStrips; r++) {
				int len = t.yStepSize * i - (kNumStrips - r - 1) * t.stripSize;
				if (len > 0) {
					rto.setHeight(len);
					rto.setWidth(t.xStepSize);
					rto.moveTo(t.xStepSize * r, 0);
					rects.push_back(rto);
				}
			}
			break;

		case kTransStripsTopBuildRight:						// 46
			for (int r = 0; r < kNumStrips; r++) {
				int len = t.yStepSize * i - r * t.stripSize;
				if (len > 0) {
					rto.setHeight(len);
					rto.setWidth(t.xStepSize);
					rto.moveTo(t.xStepSize * r, 0);
					rects.push_back(rto);
				}
			}
			break;

		case kTransVerticalBinds:							// 49
			rto.setWidth(t.xStepSize * (i + 1));
			for (int r = 0; r < kNumBlinds; r++) {
				rto.moveTo(r * t.stripSize, 0);
				rects.push_back(rto);
			}
			break;

		default:
			warning("Window::transMultiPass(): Unhandled transition type %s %d %d", transProps[t.type].name, t.duration, t.chunkSize);
			stop = true;
			break;
		}

		if (stop)
			break;

		for (uint r = 0; r < rects.size(); r++) {
			rto = rects[r];
			rto.translate(clipRect.left, clipRect.top);
			rto.clip(clipRect);

			if (rto.height() > 0 && rto.width() > 0) {
				_composeSurface->blitFrom(*nextFrame, rto, Common::Point(rto.left, rto.top));
			}
		}
		stepTransition(t, i);
		rects.clear();

		g_lingo->executePerFrameHook(t.frame, i);

		uint32 endTime = g_system->getMillis();
		int diff = MAX(0, (int)t.stepDuration - (int)(endTime - startTime));
		debugC(6, kDebugImages, "Window::transMultiPass(): delaying for %d", diff);
		g_director->delayMillis(diff);

		if (_vm->processEvents(true)) {
			exitTransition(t, nextFrame, clipRect);
			break;
		}

	}
}

void Window::transZoom(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *currentFrame, Graphics::ManagedSurface *nextFrame) {
	Common::Rect r = clipRect;
	uint w = clipRect.width();
	uint h = clipRect.height();
	t.steps >>= 1;
	t.xStepSize <<= 1;
	t.yStepSize <<= 1;
	t.steps += 1;

	DirectorPlotData pd(g_director, kLineTopBottomSprite, kInkTypeReverse, 0, _wm->_colorWhite, _wm->_colorBlack);
	pd.destRect = clipRect;
	pd.dst = _composeSurface;

	Graphics::Primitives *primitives = g_director->getInkPrimitives();

	for (uint16 i = 1; i < t.steps; i++) {
		uint32 startTime = g_system->getMillis();

		// FIXME: figure out the bounding box of the drawn bits
		_composeSurface->copyRectToSurface(*currentFrame, clipRect.left, clipRect.top, clipRect);

		for (int s = 2; s >= 0; s--) {
			if (i - s < 0 || i - s > t.steps - 2)
				continue;

			if (t.type == kTransZoomOpen) {
				r.setHeight(t.yStepSize * (i - s) * 2);
				r.setWidth(t.xStepSize * (i - s) * 2);
				r.moveTo(w / 2 - t.xStepSize * (i - s), h / 2 - t.yStepSize * (i - s));
			} else {
				r.setHeight(h - t.yStepSize * (i - s) * 2);
				r.setWidth(w - t.xStepSize * (i - s) * 2);
				r.moveTo(t.xStepSize * (i - s), t.yStepSize * (i - s));
			}

			primitives->drawLine(r.left,  r.top,    r.right, r.top,    _wm->_colorBlack, &pd);
			primitives->drawLine(r.right, r.top,    r.right, r.bottom, _wm->_colorBlack, &pd);
			primitives->drawLine(r.left,  r.bottom, r.right, r.bottom, _wm->_colorBlack, &pd);
			primitives->drawLine(r.left,  r.top,    r.left,  r.bottom, _wm->_colorBlack, &pd);
		}

		r.setHeight(t.yStepSize * i * 2);
		r.setWidth(t.xStepSize * i * 2);
		r.moveTo(w / 2 - t.xStepSize * i, h / 2 - t.yStepSize * i);

		if (_vm->processEvents(true)) {
			exitTransition(t, nextFrame, clipRect);
			break;
		}

		stepTransition(t, i);

		uint32 endTime = g_system->getMillis();
		int diff = MAX(0, (int)t.stepDuration - (int)(endTime - startTime));
		debugC(6, kDebugImages, "Window::transZoom(): delaying for %d", diff);
		g_director->delayMillis(diff);

		g_lingo->executePerFrameHook(t.frame, i);
	}

	render(true, _composeSurface);
	_contentIsDirty = true;
	g_director->draw();
}

void Window::initTransParams(TransParams &t, Common::Rect &clipRect) {
	int w = clipRect.width();
	int h = clipRect.height();
	int m = MIN(w, h);
	TransitionAlgo a = transProps[t.type].algo;

	if (a == kTransAlgoCenterOut || a == kTransAlgoEdgesIn || a == kTransAlgoZoom) {
		w = (w + 1) >> 1;	// round up
		h = (h + 1) >> 1;
	}

	// If we requested fast transitions, speed everything up
	// Ensure the chunksize isn't larger than the amount of pixels.
	if (debugChannelSet(-1, kDebugFast))
		t.chunkSize = MIN((uint) m, t.chunkSize*16);

	uint maxSteps = MAX_STEPS(t.duration);

	switch (transProps[t.type].dir) {
	case kTransDirHorizontal:
		t.steps = MIN(MAX(w / t.chunkSize, (uint)1), maxSteps);
		t.xStepSize = w / t.steps;
		t.xpos = w % t.steps;
		break;

	case kTransDirVertical:
		t.steps = MIN(MAX(h / t.chunkSize, (uint)1), maxSteps);
		t.yStepSize = h / t.steps;
		t.ypos = h % t.steps;
		break;

	case kTransDirBoth:
		t.steps = MIN(MAX(m / t.chunkSize, (uint)1), maxSteps);

		t.xStepSize = w / t.steps;
		t.xpos = w % t.steps;
		t.yStepSize = h / t.steps;
		t.ypos = h % t.steps;
		break;

	case kTransDirStepsH:
		{
			uint minChunkSize = (w - 1)/((maxSteps/2) - 1);
			t.xStepSize = MAX(t.chunkSize, minChunkSize);
			t.yStepSize = (h + kNumStrips - 1) / kNumStrips;
			t.stripSize = (w + kNumStrips - 1) / kNumStrips;
			t.steps = ((w + t.xStepSize - 1) / t.xStepSize) * 2;
		}
		break;

	case kTransDirStepsV:
		{
			uint minChunkSize = (h - 1)/((maxSteps/2) - 1);
			t.xStepSize = (w + kNumStrips - 1) / kNumStrips;
			t.yStepSize = MAX(t.chunkSize, minChunkSize);
			t.stripSize = (h + kNumStrips - 1) / kNumStrips;
			t.steps = ((h + t.yStepSize - 1) / t.yStepSize) * 2;
		}
		break;

	case kTransDirCheckers:
		if (w > h)
			t.stripSize = (w + kNumStrips - 1) / kNumStrips;
		else
			t.stripSize = (h + kNumStrips - 1) / kNumStrips;

		t.steps = ((t.stripSize + t.chunkSize - 1) / t.chunkSize) * 2 + 2;
		t.xStepSize = (w + t.stripSize - 1) / t.stripSize;		// number of checkers
		t.yStepSize = (h + t.stripSize - 1) / t.stripSize;		// number of checkers
		break;

	case kTransDirBlindsV:
		t.xStepSize = t.chunkSize;
		t.stripSize = (w + kNumBlinds - 1) / kNumBlinds;
		t.steps = (t.stripSize + t.xStepSize - 1) / t.xStepSize;
		break;

	case kTransDirBlindsH:
		t.yStepSize = t.chunkSize;
		t.stripSize = (h + kNumBlinds - 1) / kNumBlinds;
		t.steps = (t.stripSize + t.yStepSize - 1) / t.yStepSize;
		break;

	case kTransDirDissolve:
		if (t.type == kTransDissolvePixelsFast ||
			t.type == kTransDissolveBitsFast)
			t.duration = 250;						// Smallest possible delay
		t.steps = MIN<int>(MAX_STEPS(t.duration), 64);
		t.stepDuration = t.duration / t.steps;
		break;

	default:
		t.steps = 1;
	}

	if (debugChannelSet(-1, kDebugFast))
		t.steps = 1;

	t.stepDuration = t.duration / t.steps;
}

} // End of namespace Director
