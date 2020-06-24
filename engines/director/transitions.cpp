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
#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/util.h"
#include "director/lingo/lingo.h"

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
	kTransDirBlindsH
};

enum {
	kNumStrips = 16,
	kNumChecks = 16,
	kNumBlinds = 12
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
	TRANS(kTransDissolvePixelsFast,		kTransAlgoDissolve,	kTransDirNone),
	TRANS(kTransDissolveBoxyRects,		kTransAlgoDissolve,	kTransDirNone),
	TRANS(kTransDissolveBoxySquares,	kTransAlgoDissolve,	kTransDirNone),			// 25
	TRANS(kTransDissolvePatterns,		kTransAlgoDissolve,	kTransDirNone),
	TRANS(kTransRandomRows,				kTransAlgoDissolve,	kTransDirNone),
	TRANS(kTransRandomColumns,			kTransAlgoDissolve,	kTransDirNone),
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
	TRANS(kTransDissolveBitsFast,		kTransAlgoDissolve,	kTransDirNone),			// 50
	TRANS(kTransDissolvePixels,			kTransAlgoDissolve,	kTransDirNone),
	TRANS(kTransDissolveBits,			kTransAlgoDissolve,	kTransDirNone)
};

void Score::playTransition(uint16 transDuration, uint8 transArea, uint8 transChunkSize, TransitionType transType) {
	// Play a transition and return the number of subframes rendered

	// HACK: Prevent the tests from crashing
	if (!_backSurface || !_backSurface2)
		return;

	TransParams t;

	t.type = transType;
	t.duration = MAX<uint16>(250, transDuration); // When duration is < 1/4s, make it 1/4
	t.chunkSize = MAX<uint>(1, transChunkSize);
	t.area = MAX<uint>(0, transArea);

	// If we requested fast transitions, speed everything up
	if (debugChannelSet(-1, kDebugFast))
		t.duration = 250;

	if (t.area)
		warning("STUB: Transition over changed area transition");

	Common::Rect clipRect(_movieRect);
	clipRect.moveTo(0, 0);

	Common::Rect rfrom, rto;

	initTransParams(t, clipRect);

	Graphics::ManagedSurface *blitFrom;
	bool fullredraw = false;

	switch (transProps[t.type].algo) {
	case kTransAlgoDissolve:
		if (t.type == kTransDissolvePatterns)
			dissolvePatternsTrans(t, clipRect);
		else
			dissolveTrans(t, clipRect);
		return;

	case kTransAlgoChecker:
	case kTransAlgoStrips:
	case kTransAlgoBlinds:
		transMultiPass(t, clipRect);
		return;

	case kTransAlgoZoom:
		transZoom(t, clipRect);
		return;

	case kTransAlgoCenterOut:
	case kTransAlgoCover:
	case kTransAlgoWipe:
		blitFrom = _surface;
		break;

	case kTransAlgoEdgesIn:
	case kTransAlgoReveal:
	case kTransAlgoPush:
		blitFrom = _backSurface2;
		fullredraw = true;
		break;

	default:
		blitFrom = _surface;
		break;
	}

	uint w = clipRect.width();
	uint h = clipRect.height();

	for (uint16 i = 1; i < t.steps; i++) {
		bool stop = false;
		rto = clipRect;
		rfrom = clipRect;

		if (transProps[t.type].algo == kTransAlgoReveal ||
				transProps[t.type].algo == kTransAlgoEdgesIn) {
			_backSurface->copyFrom(*_surface);
		}

		switch (t.type) {
		case kTransWipeRight:								// 1
			rto.setWidth(t.xStepSize * i);
			rfrom = rto;
			break;

		case kTransWipeLeft:								// 2
			rto.setWidth(t.xStepSize * i);
			rto.moveTo(w - t.xStepSize * i, 0);
			rfrom = rto;
			break;

		case kTransWipeDown:								// 3
			rto.setHeight(t.yStepSize * i);
			rfrom = rto;
			break;

		case kTransWipeUp:									// 4
			rto.setHeight(t.yStepSize * i);
			rto.moveTo(0, h - t.yStepSize * i);
			rfrom = rto;
			break;

		case kTransCenterOutHorizontal:						// 5
			t.xpos += t.xStepSize;
			rto.setWidth(t.xpos * 2);
			rto.moveTo(w / 2 - t.xpos, 0);
			rfrom = rto;
			break;

		case kTransEdgesInHorizontal:						// 6
			rto.setWidth(w - t.xStepSize * i * 2);
			rto.moveTo(t.xStepSize * i, 0);
			rfrom = rto;
			break;

		case kTransCenterOutVertical:						// 7
			t.ypos += t.yStepSize;
			rto.setHeight(t.ypos * 2);
			rto.moveTo(0, h / 2 - t.ypos);
			rfrom = rto;
			break;

		case kTransEdgesInVertical:							// 8
			rto.setHeight(h - t.yStepSize * i * 2);
			rto.moveTo(0, t.yStepSize * i);
			rfrom = rto;
			break;

		case kTransCenterOutSquare: 						// 9
			t.ypos += t.yStepSize;
			rto.setHeight(t.ypos * 2);
			t.xpos += t.xStepSize;
			rto.setWidth(t.xpos * 2);
			rto.moveTo(w / 2 - t.xpos, h / 2 - t.ypos);
			rfrom = rto;
			break;

		case kTransEdgesInSquare:							// 10
			rto.setHeight(h - t.yStepSize * i * 2);
			rto.setWidth(w - t.xStepSize * i * 2);
			rto.moveTo(t.xStepSize * i, t.yStepSize * i);
			rfrom = rto;
			break;

		case kTransPushLeft:								// 11
			rto.moveTo(w - t.xStepSize * i, 0);
			_backSurface->blitFrom(*_surface, rfrom, Common::Point(rto.left, rto.top));

			rfrom.moveTo(t.xStepSize * i, 0);
			rfrom.setWidth(w - t.xStepSize * i);
			rto.moveTo(0, 0);
			break;

		case kTransPushRight:								// 12
			rfrom.moveTo(w - t.xStepSize * i, 0);
			rfrom.setWidth(t.xStepSize * i);
			_backSurface->blitFrom(*_surface, rfrom, Common::Point(rto.left, rto.top));

			rto.setWidth(w - t.xStepSize * i);
			rto.moveTo(t.xStepSize * i, 0);
			rfrom.moveTo(0, 0);
			rfrom.setWidth(w - t.xStepSize * i);
			break;

		case kTransPushDown:								// 13
			rfrom.moveTo(0, h - t.yStepSize * i);
			rfrom.setHeight(t.yStepSize * i);
			_backSurface->blitFrom(*_surface, rfrom, Common::Point(rto.left, rto.top));

			rto.setHeight(h - t.yStepSize * i);
			rto.moveTo(0, t.yStepSize * i);
			rfrom.moveTo(0, 0);
			rfrom.setHeight(h - t.yStepSize * i);
			break;

		case kTransPushUp:									// 14
			rto.moveTo(0, h - t.yStepSize * i);
			_backSurface->blitFrom(*_surface, rfrom, Common::Point(rto.left, rto.top));

			rfrom.moveTo(0, t.yStepSize * i);
			rfrom.setHeight(h - t.yStepSize * i);
			rto.moveTo(0, 0);
			break;

		case kTransRevealUp:								// 15
			rto.moveTo(0, -t.yStepSize * i);
			break;

		case kTransRevealUpRight:							// 16
			rto.moveTo(t.xStepSize * i, -t.yStepSize * i);
			break;

		case kTransRevealRight:								// 17
			rto.moveTo(t.xStepSize * i, 0);
			break;

		case kTransRevealDownRight:							// 18
			rto.moveTo(t.xStepSize * i, t.yStepSize * i);
			break;

		case kTransRevealDown:								// 19
			rto.moveTo(0, t.yStepSize * i);
			break;

		case kTransRevealDownLeft:							// 20
			rto.moveTo(-t.xStepSize * i, t.yStepSize * i);
			break;

		case kTransRevealLeft:								// 21
			rto.moveTo(-t.xStepSize * i, 0);
			break;

		case kTransRevealUpLeft:							// 22
			rto.moveTo(-t.xStepSize * i, -t.yStepSize * i);
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
			rto.moveTo(0, t.yStepSize * i - h);
			break;

		case kTransCoverDownLeft:							// 30
			rto.moveTo(w - t.xStepSize * i, t.yStepSize * i - h);
			break;

		case kTransCoverDownRight:							// 31
			rto.moveTo(t.xStepSize * i - w, t.yStepSize * i - h);
			break;

		case kTransCoverLeft:								// 32
			rto.moveTo(w - t.xStepSize * i, 0);
			break;

		case kTransCoverRight:								// 33
			rto.moveTo(t.xStepSize * i - w, 0);
			break;

		case kTransCoverUp:									// 34
			rto.moveTo(0, h - t.yStepSize * i);
			break;

		case kTransCoverUpLeft:								// 35
			rto.moveTo(w - t.xStepSize * i, h - t.yStepSize * i);
			break;

		case kTransCoverUpRight:							// 36
			rto.moveTo(t.xStepSize * i - w, h - t.yStepSize * i);
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
			warning("Score::playTransition(): Unhandled transition type %s %d %d", transProps[t.type].name, t.duration, t.chunkSize);
			stop = true;
			break;
		}

		if (stop)
			break;

		_backSurface->blitFrom(*blitFrom, rfrom, Common::Point(rto.left, rto.top));

		g_system->delayMillis(t.stepDuration);
		if (processQuitEvent(true))
			break;

		if (fullredraw) {
			g_system->copyRectToScreen(_backSurface->getPixels(), _backSurface->pitch, 0, 0, w, h);
		} else {
			rto.clip(clipRect);

			if (rto.height() > 0 && rto.width() > 0) {
				g_system->copyRectToScreen(_backSurface->getBasePtr(rto.left, rto.top), _backSurface->pitch, rto.left, rto.top, rto.width(), rto.height());
			}
		}

		g_system->updateScreen();

		g_lingo->executePerFrameHook(_currentFrame, i);
	}
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

void Score::dissolveTrans(TransParams &t, Common::Rect &clipRect) {
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

	case kTransDissolveBoxyRects:
		t.xStepSize = t.chunkSize;
		t.yStepSize = t.chunkSize;
		w = (w + t.chunkSize - 1) / t.chunkSize;
		h = (h + t.chunkSize - 1) / t.chunkSize;
		break;

	case kTransDissolveBoxySquares:
		t.xStepSize = w * t.chunkSize / h;
		t.yStepSize = h * t.chunkSize / w;

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

	// Get previous frame
	_backSurface->copyFrom(*_backSurface2);

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

	t.stepDuration = t.duration / t.steps;

	if (t.type == kTransDissolvePixelsFast ||
			t.type == kTransDissolveBitsFast)
		t.stepDuration = 0;						// No delay

	Common::Rect r(MAX(1, t.xStepSize), t.yStepSize);

	for (int i = 0; i < t.steps; i++) {
		uint32 pixPerStep = pixPerStepInit;
		do {
			uint32 x = rnd >> vShift;
			uint32 y = rnd & hMask;
			byte mask = 0;

			r.setWidth(MAX(1, t.xStepSize));
			r.setHeight(t.yStepSize);

			if (x < w && y < h) {
				if (t.xStepSize >= 1) {
					x = x * t.xStepSize;
					y = y * t.yStepSize;

					if (x < realw && y < realh) {
						r.moveTo(x, y);
						r.clip(clipRect);
						_backSurface->copyRectToSurface(*_surface, x, y, r);
					}
				} else {
					mask = pixmask[x % -t.xStepSize];
					x = x / -t.xStepSize;

					byte *color1 = (byte *)_backSurface->getBasePtr(x, y);
					byte *color2 = (byte *)_surface->getBasePtr(x, y);

					*color1 = ((*color1 & ~mask) | (*color2 & mask)) & 0xff;
				}
			}

			rnd = (rnd & 1) ? (rnd >> 1) ^ seed : rnd >> 1;

			if (pixPerStep > 0) {
				if (--pixPerStep == 0) {
					break;
				}
			}
		} while (rnd != seed);

		g_system->copyRectToScreen(_backSurface->getPixels(), _backSurface->pitch, 0, 0, realw, realh);
		g_system->updateScreen();

		g_lingo->executePerFrameHook(_currentFrame, i + 1);

		if (processQuitEvent(true))
			break;

		g_system->delayMillis(t.stepDuration);
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

void Score::dissolvePatternsTrans(TransParams &t, Common::Rect &clipRect) {
	uint w = clipRect.width();
	uint h = clipRect.height();

	// Get previous frame
	_backSurface->copyFrom(*_backSurface2);

	t.steps = 64;
	t.stepDuration = t.duration / t.steps;

	for (int i = 0; i < t.steps; i++) {
		for (uint y = 0; y < h; y++) {
			byte pat = dissolvePatterns[i][y % 8];
			byte *dst = (byte *)_backSurface->getBasePtr(0, y);
			byte *src = (byte *)_surface->getBasePtr(0, y);

			for (uint x = 0; x < w;) {
				byte mask = 0x80;
				for (int b = 0; b < 8 && x < w; b++, x++) {
					if (pat & mask)
						*dst = *src;

					dst++;
					src++;
					mask >>= 1;
				}
			}
		}

		g_system->copyRectToScreen(_backSurface->getPixels(), _backSurface->pitch, 0, 0, w, h);
		g_system->updateScreen();

		g_lingo->executePerFrameHook(_currentFrame, i + 1);

		if (processQuitEvent(true))
			break;

		g_system->delayMillis(t.stepDuration);
	}
}

void Score::transMultiPass(TransParams &t, Common::Rect &clipRect) {
	Common::Rect rto;
	uint w = clipRect.width();
	uint h = clipRect.height();
	bool flag = false;

	Common::Array<Common::Rect> rects;

	for (uint16 i = 1; i < t.steps; i++) {
		bool stop = false;
		rto = clipRect;

		switch (t.type) {
		case kTransVenetianBlind:							// 37
			rto.setHeight(t.yStepSize * i);
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
			rto.setWidth(t.xStepSize * i);
			for (int r = 0; r < kNumBlinds; r++) {
				rto.moveTo(r * t.stripSize, 0);
				rects.push_back(rto);
			}
			break;

		default:
			warning("Score::transMultiPass(): Unhandled transition type %s %d %d", transProps[t.type].name, t.duration, t.chunkSize);
			stop = true;
			break;
		}

		if (stop)
			break;

		for (uint r = 0; r < rects.size(); r++) {
			rto = rects[r];
			rto.clip(clipRect);

			if (rto.height() > 0 && rto.width() > 0) {
				_backSurface->blitFrom(*_surface, rto, Common::Point(rto.left, rto.top));
				g_system->copyRectToScreen(_backSurface->getBasePtr(rto.left, rto.top), _backSurface->pitch, rto.left, rto.top, rto.width(), rto.height());
			}
		}
		rects.clear();

		g_system->updateScreen();

		g_lingo->executePerFrameHook(_currentFrame, i);

		g_system->delayMillis(t.stepDuration);
		if (processQuitEvent(true))
			break;

	}
}

void Score::transZoom(TransParams &t, Common::Rect &clipRect) {
	Common::Rect r = clipRect;
	uint w = clipRect.width();
	uint h = clipRect.height();

	t.steps += 2;

	Graphics::MacPlotData pd(_backSurface, nullptr, &g_director->_wm->getPatterns(), Graphics::kPatternCheckers, 0, 0, 1, 0);

	for (uint16 i = 1; i < t.steps; i++) {
		_backSurface->copyFrom(*_backSurface2);

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

			Graphics::drawLine(r.left,  r.top,    r.right, r.top,    0xffff, Graphics::macDrawPixel, &pd);
			Graphics::drawLine(r.right, r.top,    r.right, r.bottom, 0xffff, Graphics::macDrawPixel, &pd);
			Graphics::drawLine(r.left,  r.bottom, r.right, r.bottom, 0xffff, Graphics::macDrawPixel, &pd);
			Graphics::drawLine(r.left,  r.top,    r.left,  r.bottom, 0xffff, Graphics::macDrawPixel, &pd);
		}

		r.setHeight(t.yStepSize * i * 2);
		r.setWidth(t.xStepSize * i * 2);
		r.moveTo(w / 2 - t.xStepSize * i, h / 2 - t.yStepSize * i);

		g_system->copyRectToScreen(_backSurface->getPixels(), _backSurface->pitch, 0, 0, w, h);
		g_system->updateScreen();

		g_lingo->executePerFrameHook(_currentFrame, i);

		g_system->delayMillis(t.stepDuration);
		if (processQuitEvent(true))
			break;
	}
}

void Score::initTransParams(TransParams &t, Common::Rect &clipRect) {
	int w = clipRect.width();
	int h = clipRect.height();
	int m = MIN(w, h);
	TransitionAlgo a = transProps[t.type].algo;

	if (a == kTransAlgoCenterOut || a == kTransAlgoEdgesIn || a == kTransAlgoZoom) {
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

	case kTransDirStepsH:
		t.xStepSize = t.chunkSize;
		t.yStepSize = (h + kNumStrips - 1) / kNumStrips;
		t.stripSize = (w + kNumStrips - 1) / kNumStrips;
		t.steps = ((w + t.xStepSize - 1) / t.xStepSize) * 2;
		break;

	case kTransDirStepsV:
		t.xStepSize = (w + kNumStrips - 1) / kNumStrips;
		t.yStepSize = t.chunkSize;
		t.stripSize = (h + kNumStrips - 1) / kNumStrips;
		t.steps = ((h + t.yStepSize - 1) / t.yStepSize) * 2;
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
		t.yStepSize = t.chunkSize;
		t.stripSize = (w + kNumBlinds - 1) / kNumBlinds;
		t.steps = (w + t.stripSize - 1) / t.stripSize;
		break;

	case kTransDirBlindsH:
		t.xStepSize = t.chunkSize;
		t.yStepSize = t.chunkSize;
		t.stripSize = (h + kNumBlinds - 1) / kNumBlinds;
		t.steps = (h + t.stripSize - 1) / t.stripSize;
		break;

	default:
		t.steps = 1;
	}

	t.stepDuration = t.duration / t.steps;
}

} // End of namespace Director
