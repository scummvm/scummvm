/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-2013 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "pegasus/pegasus.h"
#include "pegasus/gamestate.h"
#include "pegasus/neighborhood/norad/alpha/subchase.h"
#include "pegasus/neighborhood/norad/alpha/noradalpha.h"
#include "pegasus/neighborhood/norad/constants.h"

namespace Pegasus {

static const TimeScale kSubChaseScale = 600;

static const DisplayOrder kSubChaseOrder = 27000;

// Segment start and end points.

static const TimeValue kIntroStart = 0;
static const TimeValue kIntroEnd = 2400;
static const TimeValue kDialogStart = kIntroEnd;
static const TimeValue kDialogEnd = 20920;
static const TimeValue kBranch1Start = kDialogEnd;
static const TimeValue kBranch1End = 32120;
static const TimeValue kBranch2LeftStart = kBranch1End;
static const TimeValue kBranch2LeftEnd = 48080;
static const TimeValue kBranch3Start = kBranch2LeftEnd;
static const TimeValue kBranch3End = 61080;
static const TimeValue kBranch4Start = kBranch3End;
static const TimeValue kBranch4End = 84080;
static const TimeValue kBranch5Start = kBranch4End;
static const TimeValue kBranch5End = 94840;
static const TimeValue kBranch6Start = kBranch5End;
static const TimeValue kBranch6End = 106040;
static const TimeValue kBranch7LeftStart = kBranch6End;
static const TimeValue kBranch7LeftEnd = 118840;
static const TimeValue kExitStart = kBranch7LeftEnd;
static const TimeValue kExitEnd = 133200;
static const TimeValue kBranch2RightStart = 133200;
static const TimeValue kBranch2RightEnd = 149160;
static const TimeValue kBranch7RightStart = 168000;
static const TimeValue kBranch7RightEnd = 180800;

// Death start and end points.

static const TimeValue kDeath4Start = 149160;
static const TimeValue kDeath4End = 158040;
static const TimeValue kDeath5Start = kDeath4End;
static const TimeValue kDeath5End = 163760;
static const TimeValue kDeath6Start = kDeath5End;
static const TimeValue kDeath6End = 168000;
static const TimeValue kDeath7Start = 180800;
static const TimeValue kDeath7End = 187040;

// Chase state.

enum {
	kSubDialog,
	kSubBranch1,
	kSubBranch2Left,
	kSubBranch2Right,
	kSubBranch3,
	kSubBranch4,
	kSubBranch5,
	kSubBranch6,
	kSubBranch7Left,
	kSubBranch7Right,
	kSubExit
};

void HintTimerEvent::fire() {
	subChase->hintTimerExpired(*this);
}

void BlinkTimerEvent::fire() {
	subChase->blinkTimerExpired(*this);
}

SubChase::SubChase(Neighborhood *handler) : ChaseInteraction(kNoradSubChaseInteractionID, handler,
						kNoradSubChaseNotificationID, (PegasusEngine *)g_engine), _subMovie(kNoDisplayElement),
						_hintPict(kNoDisplayElement), _blinkPict(kNoDisplayElement), _canSteerSub(true) {
}

void SubChase::setSoundFXLevel(const uint16 fxLevel) {
	_subMovie.setVolume(fxLevel);
}

void SubChase::openInteraction() {
	_subMovie.initFromMovieFile("Images/Norad Alpha/Sub Chase Movie");
	_subMovie.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());
	_subMovie.moveElementTo(0, 0);
	_subMovie.setDisplayOrder(kSubChaseOrder);
	_subMovie.startDisplaying();
	_subMovie.show();

	_subCallBack.setNotification(&_chaseNotification);
	_subCallBack.initCallBack(&_subMovie, kCallBackAtExtremes);

	ChaseInteraction::openInteraction();

	_steerPict.setDisplayOrder(kSubChaseOrder + 1);
	_steerPict.moveElementTo(kNoradSubSteerLeft, kNoradSubSteerTop);

	_hintPict.initFromPICTFile("Images/Norad Alpha/Sub Chase steerk1.pict", true);
	_hintPict.setDisplayOrder(kSubChaseOrder + 1);
	_hintPict.moveElementTo(kNoradSubHintLeft, kNoradSubHintTop);
	_blinkPict.initFromPICTFile("Images/Norad Alpha/Sub Chase steerk0.pict", true);
	_blinkPict.setDisplayOrder(kSubChaseOrder + 1);
	_blinkPict.moveElementTo(kNoradSubHintLeft, kNoradSubHintTop);
}

void SubChase::initInteraction() {
	_canSteerSub = !GameState.getWalkthroughMode();

	_owner->playMovieSegment(&_subMovie, kIntroStart, kIntroEnd);

	_subCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	if (_canSteerSub) {
		_steerPict.startDisplaying();
		_hintPict.startDisplaying();
		_blinkPict.startDisplaying();
		startHintTimer(20000 - kDialogStart, kSubChaseScale, kStartedHint);
		_subCallBack.setCallBackFlag(kChaseEnteredBranchZone);
		_subMovie.setSegment(kDialogStart, kBranch1End - kDecisionTime);
	} else {
		_subCallBack.setCallBackFlag(kChaseFinished);
		_subMovie.setSegment(kDialogStart, kExitEnd);
	}

	_subState = kSubDialog;
	_subMovie.setTime(kDialogStart);
	_subMovie.start();

	ChaseInteraction::initInteraction();
}

void SubChase::closeInteraction() {
	_subMovie.stop();
	_subMovie.stopDisplaying();
	_subMovie.releaseMovie();
	_subCallBack.releaseCallBack();

	_hintPict.hide();
	_hintPict.deallocateSurface();

	_blinkPict.hide();
	_blinkPict.deallocateSurface();

	ChaseInteraction::closeInteraction();
}

void SubChase::receiveNotification(Notification *notification, const NotificationFlags flags) {
	if (notification == &_chaseNotification && flags == kChaseFinished) {
		if (_subState != kSubDialog && _subState != kSubExit) {
			// We died
			((NoradAlpha *)_owner)->die(kDeathSubDestroyed);
		} else {
			_subMovie.stopDisplaying();
			((PegasusEngine *)g_engine)->_gfx->enableErase();
			((PegasusEngine *)g_engine)->_gfx->updateDisplay();
			((PegasusEngine *)g_engine)->_gfx->disableErase();
			((PegasusEngine *)g_engine)->jumpToNewEnvironment(kNoradDeltaID, kNorad41, kEast);
		}
	}
	ChaseInteraction::receiveNotification(notification, flags);
}

void SubChase::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (_subMovie.getTime() < kBranch1Start && input.anyInput()) {
		if (_canSteerSub) {
			if (!_blinkFuse.isFuseLit()) {
				// If the hint is not blinking then it must not be running, so display it
				_hintPict.show();
				startBlinkTimer(10, 10, kEnteredBlinkState);
			}
			_subState = kSubBranch1;
			startHintTimer(3000, kSubChaseScale, kEndedHint);
		} else {
			_subState = kSubExit;
		}
		_subMovie.setTime(kBranch1Start);
	} else {
		ChaseInteraction::handleInput(input, cursorSpot);
	}
}

void SubChase::setUpBranch() {
	TimeValue branchStart, branchEnd;

	branchStart = 0;
	branchEnd = 0;
	switch (_subState) {
	case kSubDialog:
	case kSubBranch1:
		branchStart = kBranch1End - kDecisionTime;
		branchEnd = kBranch1End;
		break;
	case kSubBranch2Left:
		branchStart = kBranch2LeftEnd - kDecisionTime;
		branchEnd = kBranch2LeftEnd;
		break;
	case kSubBranch2Right:
		branchStart = kBranch2RightEnd - kDecisionTime;
		branchEnd = kBranch2RightEnd;
		break;
	case kSubBranch3:
		branchStart = kBranch3End - kDecisionTime;
		branchEnd = kBranch3End;
		break;
	case kSubBranch4:
		branchStart = kBranch4End - kDecisionTime;
		branchEnd = kBranch4End;
		break;
	case kSubBranch5:
		branchStart = kBranch5End - kDecisionTime;
		branchEnd = kBranch5End;
		break;
	case kSubBranch6:
		branchStart = kBranch6End - kDecisionTime;
		branchEnd = kBranch6End;
		break;
	case kSubBranch7Left:
		branchStart = kBranch7LeftEnd - kDecisionTime;
		branchEnd = kBranch7LeftEnd;
		break;
	case kSubBranch7Right:
		branchStart = kBranch7RightEnd - kDecisionTime;
		branchEnd = kBranch7RightEnd;
		break;
	default:
		break;
	}

	_subMovie.setSegment(branchStart, branchEnd);

	_subCallBack.setCallBackFlag(kChaseExitedBranchZone);
	_subCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
}

void SubChase::branchLeft() {
	TimeValue branchStart, branchEnd;
	NotificationFlags flag;

	branchStart = 0;
	branchEnd = 0;
	flag = 0;
	switch (_subState) {
	case kSubDialog:
	case kSubBranch1:
		branchStart = kBranch2LeftStart;
		branchEnd = kBranch2LeftEnd;
		// Don't show the controls hint when we approach the whale
		// since the branch segments here are identical
		flag = kChaseExitedBranchZone;
		_subState = kSubBranch2Left;
		break;
	case kSubBranch2Left:
	case kSubBranch2Right:
		branchStart = kBranch3Start;
		branchEnd = kBranch3End - kDecisionTime;
		flag = kChaseEnteredBranchZone;
		_subState = kSubBranch3;
		break;
	case kSubBranch3:
		branchStart = kBranch4Start;
		branchEnd = kBranch4End - kDecisionTime;
		flag = kChaseEnteredBranchZone;
		_subState = kSubBranch4;
		break;
	case kSubBranch4:
		branchStart = kDeath5Start;
		branchEnd = kDeath5End;
		flag = kChaseFinished;
		_subState = kSubBranch5;
		break;
	case kSubBranch5:
		branchStart = kBranch6Start;
		branchEnd = kBranch6End - kDecisionTime;
		flag = kChaseEnteredBranchZone;
		_subState = kSubBranch6;
		break;
	case kSubBranch6:
		branchStart = kBranch7LeftStart;
		branchEnd = kBranch7LeftEnd;
		flag = kChaseExitedBranchZone;
		_subState = kSubBranch7Left;
		break;
	case kSubBranch7Left:
	case kSubBranch7Right:
		branchStart = kExitStart;
		branchEnd = kExitEnd;
		flag = kChaseFinished;
		_subState = kSubExit;
		break;
	default:
		break;
	}

	_subMovie.setSegment(branchStart, branchEnd);
	_subMovie.setTime(branchStart);

	_subCallBack.setCallBackFlag(flag);
	_subCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
}

void SubChase::branchRight() {
	TimeValue branchStart, branchEnd;
	NotificationFlags flag;

	branchStart = 0;
	branchEnd = 0;
	flag = 0;
	switch (_subState) {
	case kSubDialog:
	case kSubBranch1:
		branchStart = kBranch2RightStart;
		branchEnd = kBranch2RightEnd;
		// Don't show the controls hint when we approach the whale
		// since the branch segments here are identical
		flag = kChaseExitedBranchZone;
		_subState = kSubBranch2Right;
		break;
	case kSubBranch2Left:
	case kSubBranch2Right:
		branchStart = kBranch3Start;
		branchEnd = kBranch3End - kDecisionTime;
		flag = kChaseEnteredBranchZone;
		_subState = kSubBranch3;
		break;
	case kSubBranch3:
		branchStart = kDeath4Start;
		branchEnd = kDeath4End;
		flag = kChaseFinished;
		_subState = kSubBranch4;
		break;
	case kSubBranch4:
		branchStart = kBranch5Start;
		branchEnd = kBranch5End - kDecisionTime;
		flag = kChaseEnteredBranchZone;
		_subState = kSubBranch5;
		break;
	case kSubBranch5:
		branchStart = kDeath6Start;
		branchEnd = kDeath6End;
		flag = kChaseFinished;
		_subState = kSubBranch6;
		break;
	case kSubBranch6:
		if (((PegasusEngine *)g_engine)->getRandomBit()) {
			branchStart = kBranch7RightStart;
			branchEnd = kBranch7RightEnd;
			flag = kChaseExitedBranchZone;
		} else {
			branchStart = kDeath7Start;
			branchEnd = kDeath7End;
			flag = kChaseFinished;
		}
		_subState = kSubBranch7Right;
		break;
	case kSubBranch7Left:
	case kSubBranch7Right:
		branchStart = kExitStart;
		branchEnd = kExitEnd;
		flag = kChaseFinished;
		_subState = kSubExit;
		break;
	default:
		break;
	}

	_subMovie.setSegment(branchStart, branchEnd);
	_subMovie.setTime(branchStart);

	_subCallBack.setCallBackFlag(flag);
	_subCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
}

void SubChase::dontBranch() {
	if (((PegasusEngine *)g_engine)->getRandomBit())
		branchLeft();
	else
		branchRight();
}

void SubChase::startHintTimer(TimeValue time, TimeScale scale, HintTimerCode code) {
	if (_canSteerSub) {
		_hintFuse.primeFuse(time, scale);
		_hintEvent.subChase = this;
		_hintEvent.theEvent = code;
		_hintFuse.setFunctor(new Common::Functor0Mem<void, HintTimerEvent>(&_hintEvent, &HintTimerEvent::fire));
		_hintFuse.lightFuse();
	}
}

void SubChase::hintTimerExpired(HintTimerEvent &event) {
	switch (event.theEvent) {
	case kStartedHint:
		_hintPict.show();
		startBlinkTimer(10, 10, kEnteredBlinkState);
		startHintTimer(3920, kSubChaseScale, kEndedHint);
		break;
	case kEndedHint:
		_hintPict.hide();
		_blinkPict.hide();
		_blinkFuse.stopFuse();
		break;
	default:
		break;
	}
}

void SubChase::startBlinkTimer(TimeValue time, TimeScale scale, BlinkTimerCode code) {
	_blinkFuse.primeFuse(time, scale);
	_blinkEvent.subChase = this;
	_blinkEvent.theEvent = code;
	_blinkFuse.setFunctor(new Common::Functor0Mem<void, BlinkTimerEvent>(&_blinkEvent, &BlinkTimerEvent::fire));
	_blinkFuse.lightFuse();
}

void SubChase::blinkTimerExpired(BlinkTimerEvent &event) {
	switch (event.theEvent) {
	case kEnteredBlinkState:
		_hintPict.hide();
		_blinkPict.show();
		startBlinkTimer(5, 10, kExitedBlinkState);
		break;
	case kExitedBlinkState:
		_blinkPict.hide();
		_hintPict.show();
		startBlinkTimer(10, 10, kEnteredBlinkState);
		break;
	default:
		break;
	}
}

} // End of namespace Pegasus
