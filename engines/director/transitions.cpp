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

#include "graphics/managed_surface.h"

#include "director/director.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/util.h"

namespace Director {

enum TransitionAlgo {
	kTransAlgoBlinds,
	kTransAlgoBoxy,
	kTransAlgoBuildStrips,
	kTransAlgoCenterOut,
	kTransAlgoCheckerBoard,
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
	kTransDirBoth
};

#define TRANS(t,a,d) {t,#t,a,d}

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
	TRANS(kTransEdgesInSquare,			kTransAlgoEdgesIn,	kTransDirBoth),	// 10
	TRANS(kTransPushLeft,				kTransAlgoPush,		kTransDirHorizontal),
	TRANS(kTransPushRight,				kTransAlgoPush,		kTransDirHorizontal),
	TRANS(kTransPushDown,				kTransAlgoPush,		kTransDirVertical),
	TRANS(kTransPushUp,					kTransAlgoPush,		kTransDirVertical),
	TRANS(kTransRevealUp,				kTransAlgoReveal,	kTransDirVertical),		// 15
	TRANS(kTransRevealUpRight,			kTransAlgoReveal,	kTransDirBoth),
	TRANS(kTransRevealRight,			kTransAlgoReveal,	kTransDirHorizontal),
	TRANS(kTransRevealDownRight,		kTransAlgoReveal,	kTransDirBoth),
	TRANS(kTransRevealDown,				kTransAlgoReveal,	kTransDirVertical),
	TRANS(kTransRevealDownLeft,			kTransAlgoReveal,	kTransDirBoth),	// 20
	TRANS(kTransRevealLeft,				kTransAlgoReveal,	kTransDirHorizontal),
	TRANS(kTransRevealUpLeft,			kTransAlgoReveal,	kTransDirBoth),
	TRANS(kTransDissolvePixelsFast,		kTransAlgoDissolve,	kTransDirNone),
	TRANS(kTransDissolveBoxyRects,		kTransAlgoBoxy,		kTransDirBoth),
	TRANS(kTransDissolveBoxySquares,	kTransAlgoBoxy,		kTransDirBoth),		// 25
	TRANS(kTransDissolvePatterns,		kTransAlgoDissolve,	kTransDirNone),
	TRANS(kTransRandomRows,				kTransAlgoRandomLines,kTransDirHorizontal),
	TRANS(kTransRandomColumns,			kTransAlgoRandomLines,kTransDirVertical),
	TRANS(kTransCoverDown,				kTransAlgoCover,	kTransDirVertical),
	TRANS(kTransCoverDownLeft,			kTransAlgoCover,	kTransDirBoth),	// 30
	TRANS(kTransCoverDownRight,			kTransAlgoCover,	kTransDirBoth),
	TRANS(kTransCoverLeft,				kTransAlgoCover,	kTransDirHorizontal),
	TRANS(kTransCoverRight,				kTransAlgoCover,	kTransDirHorizontal),
	TRANS(kTransCoverUp,				kTransAlgoCover,	kTransDirVertical),
	TRANS(kTransCoverUpLeft,			kTransAlgoCover,	kTransDirBoth),	// 35
	TRANS(kTransCoverUpRight,			kTransAlgoCover,	kTransDirBoth),
	TRANS(kTransTypeVenitianBlind,		kTransAlgoBlinds,	kTransDirHorizontal),
	TRANS(kTransTypeCheckerboard,		kTransAlgoCheckerBoard, kTransDirBoth),
	TRANS(kTransTypeStripsBottomBuildLeft, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransTypeStripsBottomBuildRight, kTransAlgoBuildStrips, kTransDirBoth), // 40
	TRANS(kTransTypeStripsLeftBuildDown, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransTypeStripsLeftBuildUp, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransTypeStripsRightBuildDown, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransTypeStripsRightBuildUp, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransTypeStripsTopBuildLeft,	kTransAlgoBuildStrips, kTransDirBoth),// 45
	TRANS(kTransTypeStripsTopBuildRight, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransZoomOpen,				kTransAlgoZoom,		kTransDirBoth),
	TRANS(kTransZoomClose,				kTransAlgoZoom,		kTransDirBoth),
	TRANS(kTransVerticalBinds,			kTransAlgoBlinds,	kTransDirBoth),
	TRANS(kTransDissolveBitsFast,		kTransAlgoDissolve,	kTransDirNone),		// 50
	TRANS(kTransDissolvePixels,			kTransAlgoDissolve,	kTransDirNone),
	TRANS(kTransDissolveBits,			kTransAlgoDissolve,	kTransDirNone)
};

struct TransParams {
	TransitionType type;
	uint duration;
	uint chunkSize;

	int steps;
	int stepDuration;

	int xStepSize;
	int yStepSize;

	int xpos, ypos;

	TransParams() {
		type = kTransNone;
		duration = 250;
		chunkSize = 1;
		steps = 0;
		stepDuration = 0;

		xStepSize = yStepSize = 0;
		xpos = ypos = 0;
	}
};

static void initTransParams(TransParams &t, Score *score, Common::Rect &clipRect);
static void dissolveTrans(TransParams &t, Score *score, Common::Rect &clipRect);

void Frame::playTransition(Score *score) {
	TransParams t;

	t.type = _transType;
	t.duration = MAX<uint16>(250, _transDuration); // When duration is < 1/4s, make it 1/4
	t.chunkSize = MAX<uint>(1, _transChunkSize);

	Common::Rect clipRect(score->_movieRect);
	clipRect.moveTo(0, 0);

	Common::Rect r = clipRect;

	initTransParams(t, score, clipRect);

	if (transProps[t.type].algo == kTransAlgoDissolve) {
		dissolveTrans(t, score, clipRect);

		return;
	}

	for (uint16 i = 1; i < t.steps; i++) {
		bool stop = false;

		switch (t.type) {
		case kTransCenterOutHorizontal: // 5
			t.xpos += t.xStepSize;
			r.setWidth(t.xpos * 2);
			r.moveTo(clipRect.width() / 2 - t.xpos, 0);
			break;

		case kTransCenterOutVertical: // 7
			t.ypos += t.yStepSize;
			r.setHeight(t.ypos * 2);
			r.moveTo(0, clipRect.height() / 2 - t.ypos);
			break;

		case kTransCoverDown:	// 29
			r.setHeight(t.yStepSize * i);
			break;

		case kTransCoverDownLeft: // 30
			r.setWidth(t.xStepSize * i);
			r.setHeight(t.yStepSize * i);
			r.moveTo(clipRect.width() - t.xStepSize * i, 0);
			break;

		case kTransCoverDownRight: // 31
			r.setWidth(t.xStepSize * i);
			r.setHeight(t.yStepSize * i);
			break;

		case kTransCoverLeft:	// 32
			r.setWidth(t.xStepSize * i);
			r.moveTo(clipRect.width() - t.xStepSize * i, 0);
			break;

		case kTransCoverRight:	// 33
			r.setWidth(t.xStepSize * i);
			break;

		case kTransCoverUp:		// 34
			r.setHeight(t.yStepSize * i);
			r.moveTo(0, clipRect.height() - t.yStepSize * i);
			break;

		case kTransCoverUpLeft:	// 35
			r.setWidth(t.xStepSize * i);
			r.setHeight(t.yStepSize * i);
			r.moveTo(clipRect.width() - t.xStepSize * i, clipRect.height() - t.yStepSize * i);
			break;

		case kTransCoverUpRight:	// 36
			r.setWidth(t.xStepSize * i);
			r.setHeight(t.yStepSize * i);
			r.moveTo(0, clipRect.height() - t.yStepSize * i);

		case kTransDissolvePixels: // 51
			warning("Frame::playTransition(): Unhandled transition type %s %d %d", transProps[t.type].name, t.duration, _transChunkSize);
			stop = true;
			break;

		default:
			warning("Frame::playTransition(): Unhandled transition type %s %d %d", transProps[t.type].name, t.duration, _transChunkSize);
			stop = true;
			break;
		}

		if (stop)
			break;

		r.clip(clipRect);

		g_system->delayMillis(t.stepDuration);
		if (processQuitEvent(true))
			break;

		score->_backSurface->copyRectToSurface(*score->_surface, 0, 0, r);

		g_system->copyRectToScreen(score->_backSurface->getPixels(), score->_backSurface->pitch, r.left, r.top, r.width(), r.height()); // transition
		g_system->updateScreen();
	}
}

static void initTransParams(TransParams &t, Score *score, Common::Rect &clipRect) {
	int w = clipRect.width();
	int h = clipRect.height();
	int m = MIN(w, h);
	TransitionAlgo a = transProps[t.type].algo;

	if (a == kTransAlgoCenterOut || a == kTransAlgoEdgesIn) {
		w = (w + 1) >> 1;	// round up
		h = (h + 1) >> 1;
	}

	switch (transProps[t.type].dir) {
	case kTransDirHorizontal:
		t.steps = w / t.chunkSize;
		t.xStepSize = w / t.steps;
		t.xpos = w % t.steps;
		break;

	case kTransDirVertical:
		t.steps = h / t.chunkSize;
		t.yStepSize = h / t.steps;
		t.ypos = h % t.steps;
		break;

	case kTransDirBoth:
		t.steps = m / t.chunkSize;

		t.xStepSize = w / t.steps;
		t.xpos = w % t.steps;
		t.yStepSize = h / t.steps;
		t.ypos = h % t.steps;
		break;

	default:
		t.steps = 1;
	}

	t.stepDuration = t.duration / t.steps;
}

static int getLog2(int n) {
	int res;

	for (res = 0; n != 0; res++)
		n >>= 1;

	return res;
}

uint32 randomSeed[33] = {
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

static void dissolveTrans(TransParams &t, Score *score, Common::Rect &clipRect) {
	uint w = clipRect.width();
	uint h = clipRect.height();
	int vBits = getLog2(w);
	int hBits = getLog2(h);
	uint32 rnd, seed;

	if (hBits <= 0 || vBits <= 0)
		return;

	// Get previous frame
	score->_backSurface->copyFrom(*score->_backSurface2);

	rnd = seed = randomSeed[hBits + vBits];
	int hMask = (1L << hBits) - 1;
	int vShift = hBits;

	// Calculate steps
	uint32 pixPerStepInit = 1;
	t.steps = (1 << (hBits + vBits)) - 1;

	while (t.steps > 64) {
		pixPerStepInit <<= 1;
		t.steps >>= 1;
	}
	t.steps++;

	Common::Rect r(1, 1);

	while (t.steps) {
		uint32 pixPerStep = pixPerStepInit;
		do {
			uint32 x = rnd & hMask;
			uint32 y = rnd >> vShift;

			if (x < w && y < h) {
				r.moveTo(x, y);
				score->_backSurface->copyRectToSurface(*score->_surface, x, y, r);
			}

			rnd = (rnd & 1) ? (rnd >> 1) ^ seed : rnd >> 1;

			if (pixPerStep > 0) {
				if (--pixPerStep == 0) {
					break;
				}
			}
		} while (rnd != seed);

		r.moveTo(0, 0);
		score->_backSurface->copyRectToSurface(*score->_surface, 0, 0, r);

		g_system->copyRectToScreen(score->_backSurface->getPixels(), score->_backSurface->pitch, 0, 0, score->_backSurface->w, score->_backSurface->h);

		g_system->delayMillis(t.stepDuration);
		if (processQuitEvent(true))
			break;

		g_system->updateScreen();

		t.steps--;
	}
}

} // End of namespace Director
