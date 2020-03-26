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
	kTransDirBits,
	kTransDirBitsFast,
	kTransDirBottomBuildLeft,
	kTransDirBottomBuildRight,
	kTransDirEast,
	kTransDirHorizontal,
	kTransDirIn,
	kTransDirLeftBuildDown,
	kTransDirLeftBuildUp,
	kTransDirNormal,
	kTransDirNorth,
	kTransDirNorthEast,
	kTransDirNorthWest,
	kTransDirOut,
	kTransDirPattern,
	kTransDirPixels,
	kTransDirPixelsFast,
	kTransDirRectangular,
	kTransDirRightBuildDown,
	kTransDirRightBuildUp,
	kTransDirSouth,
	kTransDirSouthEast,
	kTransDirSouthWest,
	kTransDirSquare,
	kTransDirSymmetrical,
	kTransDirTopBuildLeft,
	kTransDirTopBuildRight,
	kTransDirVertical,
	kTransDirWest
};

#define TRANS(t,a,d) {t,#t,a,d}

struct {
	TransitionType type;
	const char *name;
	TransitionAlgo algo;
	TransitionDirection dir;
} static const transProps[] = {
	TRANS(kTransNone, 					kTransAlgoWipe,		kTransDirEast),
	TRANS(kTransWipeRight, 				kTransAlgoWipe,		kTransDirEast),
	TRANS(kTransWipeLeft,				kTransAlgoWipe,		kTransDirWest),
	TRANS(kTransWipeDown,				kTransAlgoWipe,		kTransDirSouth),
	TRANS(kTransWipeUp,					kTransAlgoWipe,		kTransDirNorth),
	TRANS(kTransCenterOutHorizontal, 	kTransAlgoCenterOut,kTransDirHorizontal),	// 5
	TRANS(kTransEdgesInHorizontal, 		kTransAlgoEdgesIn,	kTransDirHorizontal),
	TRANS(kTransCenterOutVertical,		kTransAlgoCenterOut,kTransDirVertical),
	TRANS(kTransEdgesInVertical,		kTransAlgoEdgesIn,	kTransDirVertical),
	TRANS(kTransCenterOutSquare,		kTransAlgoCenterOut,kTransDirSymmetrical),
	TRANS(kTransEdgesInSquare,			kTransAlgoEdgesIn,	kTransDirSymmetrical),	// 10
	TRANS(kTransPushLeft,				kTransAlgoPush,		kTransDirWest),
	TRANS(kTransPushRight,				kTransAlgoPush,		kTransDirEast),
	TRANS(kTransPushDown,				kTransAlgoPush,		kTransDirSouth),
	TRANS(kTransPushUp,					kTransAlgoPush,		kTransDirNorth),
	TRANS(kTransRevealUp,				kTransAlgoReveal,	kTransDirNorth),		// 15
	TRANS(kTransRevealUpRight,			kTransAlgoReveal,	kTransDirNorthEast),
	TRANS(kTransRevealRight,			kTransAlgoReveal,	kTransDirEast),
	TRANS(kTransRevealDownRight,		kTransAlgoReveal,	kTransDirSouthEast),
	TRANS(kTransRevealDown,				kTransAlgoReveal,	kTransDirSouth),
	TRANS(kTransRevealDownLeft,			kTransAlgoReveal,	kTransDirSouthWest),	// 20
	TRANS(kTransRevealLeft,				kTransAlgoReveal,	kTransDirWest),
	TRANS(kTransRevealUpLeft,			kTransAlgoReveal,	kTransDirNorthWest),
	TRANS(kTransDissolvePixelsFast,		kTransAlgoDissolve,	kTransDirPixelsFast),
	TRANS(kTransDissolveBoxyRects,		kTransAlgoBoxy,		kTransDirRectangular),
	TRANS(kTransDissolveBoxySquares,	kTransAlgoBoxy,		kTransDirSquare),		// 25
	TRANS(kTransDissolvePatterns,		kTransAlgoDissolve,	kTransDirPattern),
	TRANS(kTransRandomRows,				kTransAlgoRandomLines,kTransDirHorizontal),
	TRANS(kTransRandomColumns,			kTransAlgoRandomLines,kTransDirVertical),
	TRANS(kTransCoverDown,				kTransAlgoCover,	kTransDirSouth),
	TRANS(kTransCoverDownLeft,			kTransAlgoCover,	kTransDirSouthWest),	// 30
	TRANS(kTransCoverDownRight,			kTransAlgoCover,	kTransDirSouthEast),
	TRANS(kTransCoverLeft,				kTransAlgoCover,	kTransDirWest),
	TRANS(kTransCoverRight,				kTransAlgoCover,	kTransDirEast),
	TRANS(kTransCoverUp,				kTransAlgoCover,	kTransDirNorth),
	TRANS(kTransCoverUpLeft,			kTransAlgoCover,	kTransDirNorthWest),	// 35
	TRANS(kTransCoverUpRight,			kTransAlgoCover,	kTransDirNorthEast),
	TRANS(kTransTypeVenitianBlind,		kTransAlgoBlinds,	kTransDirHorizontal),
	TRANS(kTransTypeCheckerboard,		kTransAlgoCheckerBoard, kTransDirNormal),
	TRANS(kTransTypeStripsBottomBuildLeft, kTransAlgoBuildStrips, kTransDirBottomBuildLeft),
	TRANS(kTransTypeStripsBottomBuildRight, kTransAlgoBuildStrips, kTransDirBottomBuildRight), // 40
	TRANS(kTransTypeStripsLeftBuildDown, kTransAlgoBuildStrips, kTransDirLeftBuildDown),
	TRANS(kTransTypeStripsLeftBuildUp, kTransAlgoBuildStrips, kTransDirLeftBuildUp),
	TRANS(kTransTypeStripsRightBuildDown, kTransAlgoBuildStrips, kTransDirRightBuildDown),
	TRANS(kTransTypeStripsRightBuildUp, kTransAlgoBuildStrips, kTransDirRightBuildUp),
	TRANS(kTransTypeStripsTopBuildLeft,	kTransAlgoBuildStrips, kTransDirTopBuildLeft),// 45
	TRANS(kTransTypeStripsTopBuildRight, kTransAlgoBuildStrips, kTransDirTopBuildRight),
	TRANS(kTransZoomOpen,				kTransAlgoZoom,		kTransDirIn),
	TRANS(kTransZoomClose,				kTransAlgoZoom,		kTransDirOut),
	TRANS(kTransVerticalBinds,			kTransAlgoBlinds,	kTransDirVertical),
	TRANS(kTransDissolveBitsFast,		kTransAlgoDissolve,	kTransDirBitsFast),		// 50
	TRANS(kTransDissolvePixels,			kTransAlgoDissolve,	kTransDirPixels),
	TRANS(kTransDissolveBits,			kTransAlgoDissolve,	kTransDirBits)
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
			uint16 stepSize = clipRect.height() / t.steps;

			for (uint16 i = 1; i < t.steps; i++) {
				r.setHeight(stepSize * i);
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
			uint16 stepSize = clipRect.width() / t.steps;

			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);
				r.moveTo(clipRect.width() - stepSize * i, 0);
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
			uint16 stepSize = clipRect.width() / t.steps;

			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);
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
			uint16 stepSize = clipRect.width() / t.steps;

			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(stepSize * i);
				r.moveTo(clipRect.width() - stepSize * i, 0);
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
			uint16 stepSize = clipRect.width() / t.steps;

			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(stepSize * i);
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
			uint16 stepSize = clipRect.height() / t.steps;

			for (uint16 i = 1; i < t.steps; i++) {
				r.setHeight(stepSize * i);
				r.moveTo(0, clipRect.height() - stepSize * i);
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
			uint16 xStepSize = clipRect.width() / t.steps;
			uint16 yStepSize = clipRect.height() / t.steps;

			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(xStepSize * i);
				r.setHeight(yStepSize * i);
				r.moveTo(clipRect.width() - xStepSize * i, clipRect.height() - yStepSize * i);
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
			uint16 xStepSize = clipRect.width() / t.steps;
			uint16 yStepSize = clipRect.height() / t.steps;

			for (uint16 i = 1; i < t.steps; i++) {
				r.setWidth(xStepSize * i);
				r.setHeight(yStepSize * i);
				r.moveTo(0, clipRect.height() - yStepSize * i);
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
	if (transProps[t.type].dir == kTransDirHorizontal) {
		int w = clipRect.width() / 2;

		t.steps = w / t.chunkSize;
		t.xStepSize = w / t.steps;
		t.xpos = w % t.steps;
	} else if (transProps[t.type].dir == kTransDirVertical) {
		int h = clipRect.height() / 2;

		t.steps = h / t.chunkSize;
		t.yStepSize = h / t.steps;
		t.ypos = h % t.steps;
	} else {
		t.steps = clipRect.width() / t.chunkSize;
	}

	t.stepDuration = t.duration / t.steps;
}


} // End of namespace Director
