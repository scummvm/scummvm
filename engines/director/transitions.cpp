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

void Frame::playTransition(Score *score) {
	TransParams t;

	t.type = _transType;
	t.duration = MAX<uint16>(250, _transDuration); // When duration is < 1/4s, make it 1/4
	t.chunkSize = MAX<uint>(1, _transChunkSize);

	Common::Rect clipRect(score->_movieRect);
	clipRect.moveTo(0, 0);

	Common::Rect r = clipRect;

	initTransParams(t, score, clipRect);

	switch (_transType) {
	case kTransCenterOutHorizontal: // 5
		{
			for (uint16 i = 0; i < t.steps; i++) {
				t.xpos += t.xStepSize;

				r.setWidth(t.xpos * 2);
				r.moveTo(clipRect.width() / 2 - t.xpos, 0);

				g_system->delayMillis(t.stepDuration);
				processQuitEvent();

				score->_backSurface->copyRectToSurface(*score->_surface, 0, 0, r);

				g_system->copyRectToScreen(score->_backSurface->getPixels(), score->_backSurface->pitch, r.left, r.top, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCenterOutVertical: // 7
		{
			for (uint16 i = 0; i < t.steps; i++) {
				t.ypos += t.yStepSize;

				r.setHeight(t.ypos * 2);
				r.moveTo(0, clipRect.height() / 2 - t.ypos);

				g_system->delayMillis(t.stepDuration);
				processQuitEvent();

				score->_backSurface->copyRectToSurface(*score->_surface, 0, 0, r);

				g_system->copyRectToScreen(score->_backSurface->getPixels(), score->_backSurface->pitch, r.left, r.top, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverDown:	// 29
		{
			for (uint16 i = 1; i < t.steps; i++) {
				r.setHeight(t.yStepSize * i);
				r.clip(clipRect);

				g_system->delayMillis(t.stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverDownLeft: // 30
		{
			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(t.xStepSize * i);
				r.setHeight(t.yStepSize * i);
				r.moveTo(clipRect.width() - t.xStepSize * i, 0);
				r.clip(clipRect);

				g_system->delayMillis(t.stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, r.left, r.top, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverDownRight: // 31
		{
			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(t.xStepSize * i);
				r.setHeight(t.yStepSize * i);
				r.clip(clipRect);

				g_system->delayMillis(t.stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverLeft:	// 32
		{
			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(t.xStepSize * i);
				r.moveTo(clipRect.width() - t.xStepSize * i, 0);
				r.clip(clipRect);

				g_system->delayMillis(t.stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, r.left, r.top, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverRight:	// 33
		{
			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(t.xStepSize * i);
				r.clip(clipRect);

				g_system->delayMillis(t.stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverUp:		// 34
		{
			for (uint16 i = 1; i < t.steps; i++) {
				r.setHeight(t.yStepSize * i);
				r.moveTo(0, clipRect.height() - t.yStepSize * i);
				r.clip(clipRect);

				g_system->delayMillis(t.stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, r.left, r.top, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverUpLeft:	// 35
		{
			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(t.xStepSize * i);
				r.setHeight(t.yStepSize * i);
				r.moveTo(clipRect.width() - t.xStepSize * i, clipRect.height() - t.yStepSize * i);
				r.clip(clipRect);

				g_system->delayMillis(t.stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, r.left, r.top, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverUpRight:	// 36
		{
			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(t.xStepSize * i);
				r.setHeight(t.yStepSize * i);
				r.moveTo(0, clipRect.height() - t.yStepSize * i);
				r.clip(clipRect);

				g_system->delayMillis(t.stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, r.left, r.top, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransDissolvePixels: // 51
		{
			warning("Frame::playTransition(): Unhandled transition type %s %d %d", transProps[_transType].name, t.duration, _transChunkSize);
		}
		break;

	default:
		warning("Frame::playTransition(): Unhandled transition type %s %d %d", transProps[_transType].name, t.duration, _transChunkSize);
		break;

	}
}

static void initTransParams(TransParams &t, Score *score, Common::Rect &clipRect) {
	int w = clipRect.width();
	int h = clipRect.height();
	int m = MIN(w, h);
	TransitionDirection d = transProps[t.type].dir;
	TransitionAlgo a = transProps[t.type].algo;

	if (a == kTransAlgoCenterOut || a == kTransAlgoEdgesIn) {
		w = (w + 1) >> 1;	// round up
		h = (h + 1) >> 1;
	}

	t.steps = m / t.chunkSize;
	t.stepDuration = t.duration / t.steps;

	if (d == kTransDirHorizontal || d == kTransDirBoth) {
		t.xStepSize = w / t.steps;
		t.xpos = w % t.steps;
	}
	if (d == kTransDirVertical || d == kTransDirBoth) {
		t.yStepSize = h / t.steps;
		t.ypos = h % t.steps;
	}
}


} // End of namespace Director
