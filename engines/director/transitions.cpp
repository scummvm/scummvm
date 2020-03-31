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
	TRANS(kTransDissolveBoxyRects,		kTransAlgoBoxy,		kTransDirBoth),
	TRANS(kTransDissolveBoxySquares,	kTransAlgoBoxy,		kTransDirBoth),			// 25
	TRANS(kTransDissolvePatterns,		kTransAlgoDissolve,	kTransDirNone),
	TRANS(kTransRandomRows,				kTransAlgoRandomLines,kTransDirHorizontal),
	TRANS(kTransRandomColumns,			kTransAlgoRandomLines,kTransDirVertical),
	TRANS(kTransCoverDown,				kTransAlgoCover,	kTransDirVertical),
	TRANS(kTransCoverDownLeft,			kTransAlgoCover,	kTransDirBoth),			// 30
	TRANS(kTransCoverDownRight,			kTransAlgoCover,	kTransDirBoth),
	TRANS(kTransCoverLeft,				kTransAlgoCover,	kTransDirHorizontal),
	TRANS(kTransCoverRight,				kTransAlgoCover,	kTransDirHorizontal),
	TRANS(kTransCoverUp,				kTransAlgoCover,	kTransDirVertical),
	TRANS(kTransCoverUpLeft,			kTransAlgoCover,	kTransDirBoth),			// 35
	TRANS(kTransCoverUpRight,			kTransAlgoCover,	kTransDirBoth),
	TRANS(kTransTypeVenitianBlind,		kTransAlgoBlinds,	kTransDirHorizontal),
	TRANS(kTransTypeCheckerboard,		kTransAlgoCheckerBoard, kTransDirBoth),
	TRANS(kTransTypeStripsBottomBuildLeft, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransTypeStripsBottomBuildRight, kTransAlgoBuildStrips, kTransDirBoth),	// 40
	TRANS(kTransTypeStripsLeftBuildDown, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransTypeStripsLeftBuildUp, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransTypeStripsRightBuildDown, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransTypeStripsRightBuildUp, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransTypeStripsTopBuildLeft,	kTransAlgoBuildStrips, kTransDirBoth),		// 45
	TRANS(kTransTypeStripsTopBuildRight, kTransAlgoBuildStrips, kTransDirBoth),
	TRANS(kTransZoomOpen,				kTransAlgoZoom,		kTransDirBoth),
	TRANS(kTransZoomClose,				kTransAlgoZoom,		kTransDirBoth),
	TRANS(kTransVerticalBinds,			kTransAlgoBlinds,	kTransDirBoth),
	TRANS(kTransDissolveBitsFast,		kTransAlgoDissolve,	kTransDirNone),			// 50
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
static void dissolvePatternsTrans(TransParams &t, Score *score, Common::Rect &clipRect);

void Frame::playTransition(Score *score) {
	TransParams t;

	t.type = _transType;
	t.duration = MAX<uint16>(250, _transDuration); // When duration is < 1/4s, make it 1/4
	t.chunkSize = MAX<uint>(1, _transChunkSize);

#if 0
	t.type = kTransDissolveBits;
	t.chunkSize = 4;
#endif

	Common::Rect clipRect(score->_movieRect);
	clipRect.moveTo(0, 0);

	Common::Rect rfrom, rto;

	initTransParams(t, score, clipRect);

	if (t.type == kTransDissolvePatterns) {
		dissolvePatternsTrans(t, score, clipRect);

		return;
	}

	if (transProps[t.type].algo == kTransAlgoDissolve) {
		dissolveTrans(t, score, clipRect);

		return;
	}

	Graphics::ManagedSurface *blitFrom;
	bool fullredraw = false;

	switch (transProps[t.type].algo) {
	case kTransAlgoDissolve:
		dissolveTrans(t, score, clipRect);
		return;

	case kTransAlgoCenterOut:
	case kTransAlgoCover:
	case kTransAlgoWipe:
		blitFrom = score->_surface;
		break;

	case kTransAlgoEdgesIn:
	case kTransAlgoReveal:
		blitFrom = score->_backSurface2;
		fullredraw = true;
		break;

	default:
		blitFrom = score->_surface;
		break;
	}

	rfrom = clipRect;
	rto = clipRect;

	for (uint16 i = 1; i < t.steps; i++) {
		bool stop = false;
		rto = clipRect;

		if (transProps[t.type].algo == kTransAlgoReveal ||
				transProps[t.type].algo == kTransAlgoEdgesIn) {
			score->_backSurface->copyFrom(*score->_surface);
		}

		switch (t.type) {
		case kTransWipeRight:								// 1
			rto.setWidth(t.xStepSize * i);
			rfrom = rto;
			break;

		case kTransWipeLeft:								// 2
			rto.setWidth(t.xStepSize * i);
			rto.moveTo(clipRect.width() - t.xStepSize * i, 0);
			rfrom = rto;
			break;

		case kTransWipeDown:								// 3
			rto.setHeight(t.yStepSize * i);
			rfrom = rto;
			break;

		case kTransWipeUp:									// 4
			rto.setHeight(t.yStepSize * i);
			rto.moveTo(0, clipRect.height() - t.yStepSize * i);
			rfrom = rto;
			break;

		case kTransCenterOutHorizontal:						// 5
			t.xpos += t.xStepSize;
			rto.setWidth(t.xpos * 2);
			rto.moveTo(clipRect.width() / 2 - t.xpos, 0);
			rfrom = rto;
			break;

		case kTransEdgesInHorizontal:						// 6
			rto.setWidth(clipRect.width() - t.xStepSize * i * 2);
			rto.moveTo(t.xStepSize * i, 0);
			rfrom = rto;
			break;

		case kTransCenterOutVertical:						// 7
			t.ypos += t.yStepSize;
			rto.setHeight(t.ypos * 2);
			rto.moveTo(0, clipRect.height() / 2 - t.ypos);
			rfrom = rto;
			break;

		case kTransEdgesInVertical:							// 8
			rto.setHeight(clipRect.height() - t.yStepSize * i * 2);
			rto.moveTo(0, t.yStepSize * i);
			rfrom = rto;
			break;

		case kTransCenterOutSquare: 						// 9
			t.ypos += t.yStepSize;
			rto.setHeight(t.ypos * 2);
			t.xpos += t.xStepSize;
			rto.setWidth(t.xpos * 2);
			rto.moveTo(clipRect.width() / 2 - t.xpos, clipRect.height() / 2 - t.ypos);
			rfrom = rto;
			break;

		case kTransEdgesInSquare:							// 10
			rto.setHeight(clipRect.height() - t.yStepSize * i * 2);
			rto.setWidth(clipRect.width() - t.xStepSize * i * 2);
			rto.moveTo(t.xStepSize * i, t.yStepSize * i);
			rfrom = rto;
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
			error("Frame::playTransition(): Fall through to dissolve transition");
			break;

		case kTransDissolvePatterns:						// 26
			error("Frame::playTransition(): Fall through to dissolve transition");
			break;

		case kTransCoverDown:								// 29
			rto.setHeight(clipRect.height());
			rto.moveTo(0, -clipRect.height() + t.yStepSize * i);
			break;

		case kTransCoverDownLeft:							// 30
			rto.moveTo(clipRect.width() - t.xStepSize * i, -clipRect.height() + t.yStepSize * i);
			break;

		case kTransCoverDownRight:							// 31
			rto.moveTo(-clipRect.width() + t.xStepSize * i, -clipRect.height() + t.yStepSize * i);
			break;

		case kTransCoverLeft:								// 32
			rto.moveTo(clipRect.width() - t.xStepSize * i, 0);
			break;

		case kTransCoverRight:								// 33
			rto.moveTo(-clipRect.width() + t.xStepSize * i, 0);
			break;

		case kTransCoverUp:									// 34
			rto.moveTo(0, clipRect.height() - t.yStepSize * i);
			break;

		case kTransCoverUpLeft:								// 35
			rto.moveTo(clipRect.width() - t.xStepSize * i, clipRect.height() - t.yStepSize * i);
			break;

		case kTransCoverUpRight:							// 36
			rto.moveTo(-clipRect.width() + t.xStepSize * i, clipRect.height() - t.yStepSize * i);
			break;

		case kTransDissolveBitsFast:						// 50
			error("Frame::playTransition(): Fall through to dissolve transition");
			break;

		case kTransDissolvePixels:							// 51
			error("Frame::playTransition(): Fall through to dissolve transition");
			break;

		case kTransDissolveBits:							// 52
			error("Frame::playTransition(): Fall through to dissolve transition");
			break;

		default:
			warning("Frame::playTransition(): Unhandled transition type %s %d %d", transProps[t.type].name, t.duration, _transChunkSize);
			stop = true;
			break;
		}

		if (stop)
			break;

		score->_backSurface->blitFrom(*blitFrom, rfrom, Common::Point(rto.left, rto.top));

		rto.clip(clipRect);

		g_system->delayMillis(t.stepDuration);
		if (processQuitEvent(true))
			break;

		if (fullredraw) {
			g_system->copyRectToScreen(score->_backSurface->getPixels(), score->_backSurface->pitch, 0, 0, clipRect.width(), clipRect.height());
		} else if (rto.height() > 0 && rto.width() > 0) {
			g_system->copyRectToScreen(score->_backSurface->getBasePtr(rto.left, rto.top), score->_backSurface->pitch, rto.left, rto.top, rto.width(), rto.height()); // transition
		}

		g_system->updateScreen();
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

static void dissolveTrans(TransParams &t, Score *score, Common::Rect &clipRect) {
	int numbytes = 1;
	uint w = clipRect.width();
	uint h = clipRect.height();
	uint realw = w, realh = h;
	byte pixmask[8];

	if (t.type == kTransDissolveBitsFast ||
			t.type == kTransDissolveBits) {

		if (t.chunkSize >= 32) {
			w = (w + 3) >> 2;
			numbytes = 4;
		} else if (t.chunkSize >= 16) {
			w = (w + 1) >> 1;
			numbytes = 2;
		} else if (t.chunkSize >= 8) {
			numbytes = 1;
		} else if (t.chunkSize >= 4) {
			w <<= 1;
			numbytes = -2;
			pixmask[0] = 0x0f;
			pixmask[1] = 0xf0;
		} else if (t.chunkSize >= 2) {
			w <<= 2;
			numbytes = -4;

			for (int i = 0; i < 4; i++)
				pixmask[i] = 0x3 << (i * 2);
		} else {
			w <<= 3;
			numbytes = -8;

			for (int i = 0; i < 8; i++)
				pixmask[i] = 1 << i;
		}
	}

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

	t.stepDuration = t.duration / t.steps;

	if (t.type == kTransDissolvePixelsFast ||
			t.type == kTransDissolveBitsFast)
		t.stepDuration = 0;						// No delay

	Common::Rect r(numbytes > 0 ? numbytes : 1, 1);

	while (t.steps) {
		uint32 pixPerStep = pixPerStepInit;
		do {
			uint32 x = rnd >> vShift;
			uint32 y = rnd & hMask;
			byte mask = 0;

			if (x < w && y < h) {
				if (numbytes >= 1) {
					x = MIN(x * numbytes, realw - numbytes);
					r.moveTo(x, y);

					score->_backSurface->copyRectToSurface(*score->_surface, x, y, r);
				} else if (numbytes < 1) {
					mask = pixmask[x % -numbytes];
					x = x / -numbytes;

					byte *color1 = (byte *)score->_backSurface->getBasePtr(x, y);
					byte *color2 = (byte *)score->_surface->getBasePtr(x, y);

					byte newcolor = ((*color1 & ~mask) | (*color2 & mask)) & 0xff;
					//warning("color1: %02x | %02x [%02x] -> %02x", *color1, *color2, mask, newcolor);
					*color1 = newcolor;
				}
			}

			rnd = (rnd & 1) ? (rnd >> 1) ^ seed : rnd >> 1;

			if (pixPerStep > 0) {
				if (--pixPerStep == 0) {
					break;
				}
			}
		} while (rnd != seed);

		g_system->copyRectToScreen(score->_backSurface->getPixels(), score->_backSurface->pitch, 0, 0, realw, realh);
		g_system->updateScreen();

		if (processQuitEvent(true))
			break;

		g_system->delayMillis(t.stepDuration);

		t.steps--;
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

static void dissolvePatternsTrans(TransParams &t, Score *score, Common::Rect &clipRect) {
	uint w = clipRect.width();
	uint h = clipRect.height();

	// Get previous frame
	score->_backSurface->copyFrom(*score->_backSurface2);

	t.steps = 64;
	t.stepDuration = t.duration / t.steps;

	for (int i = 0; i < t.steps; i++) {
		for (int y = 0; y < h; y++) {
			byte pat = dissolvePatterns[i][y % 8];
			byte *dst = (byte *)score->_backSurface->getBasePtr(0, y);
			byte *src = (byte *)score->_surface->getBasePtr(0, y);

			for (int x = 0; x < w;) {
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

		g_system->copyRectToScreen(score->_backSurface->getPixels(), score->_backSurface->pitch, 0, 0, w, h);
		g_system->updateScreen();

		if (processQuitEvent(true))
			break;

		g_system->delayMillis(t.stepDuration);
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

} // End of namespace Director
